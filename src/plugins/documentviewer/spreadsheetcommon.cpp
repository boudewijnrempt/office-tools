//Include QT stuff
#include <QDebug>
#include <QMutex>
#include <QTimer>
#include <QCoreApplication>

//Include Koffice stuff
#include <Sheet.h>
#include <Doc.h>
#include <Map.h>
#include <Cell.h>
#include <CellStorage.h>
#include <tables/ui/SheetView.h>
#include <tables/RectStorage.h>
#include <tables/ValueConverter.h>
#include <tables/ValueStorage.h>

//Include application stuff
#include "spreadsheetcommon.h"
#include "definitions.h"


SpreadsheetSearch::SpreadsheetSearch(SpreadsheetUtils *utils, Calligra::Tables::Doc *doc, int count)
    : spreadSheetUtils(utils)
    , spreadDoc(doc)
    , slideCount(count)

{
    qDebug() << __PRETTY_FUNCTION__ ;

}

SpreadsheetSearch::~SpreadsheetSearch()
{
    //qDebug() << __PRETTY_FUNCTION__;
}

void SpreadsheetSearch::run()
{
    qDebug() << __PRETTY_FUNCTION__ ;
    QTimer::singleShot(10, this, SLOT(startSearch()));
    exec();
}

void SpreadsheetSearch::startSearch()
{
    Calligra::Tables::Sheet* sheet = 0;
    int index = currentSheetIndex;

    while(index < slideCount) {
        sheet = spreadDoc->map()->sheet(index);
        int count = searchSheet(sheet, index);

        if(count > 0) {
            emit setResults(index, count);
        }

        index++;
    }

    //We reach the end of the document, we search from the beginning until we reach the current page.
    index = 0;

    while(index < currentSheetIndex) {
        sheet = spreadDoc->map()->sheet(index);
        int count = searchSheet(sheet, index);

        if(count > 0) {
            emit setResults(index, count);
        }

        index++;
    }

    emit searchFinished();

    qDebug()<<"Run finish emit signal";
}

void SpreadsheetSearch::mSleep(unsigned long msecs)
{
    msleep(msecs);
}


void SpreadsheetSearch::setData(QString searchtext, int index)
{
    searchString = searchtext;
    currentSheetIndex = index;
}

static bool operator<(const SheetResult& a, const SheetResult& b) {
    if (a.row < b.row) return true;
    if (a.row > b.row) return false;
    return a.column < b.column;
}

int SpreadsheetSearch::searchSheet(Calligra::Tables::Sheet *sheet, int sheetIndex)
{
    Q_UNUSED(sheetIndex);
    qDebug() << __PRETTY_FUNCTION__;
    QList<SheetResult> searchResults;
    Calligra::Tables::CellStorage *storage = sheet->cellStorage();

    if((0 == sheet) || (0 == storage)) {
        return 0;
    }

    // collect points in a set to eliminate duplicates for cells that match multiple criteria
    QSet<QPoint> resultPositions;
    // first search through values
    Calligra::Tables::ValueConverter* converter = sheet->map()->converter();
    const Calligra::Tables::ValueStorage* values = storage->valueStorage();
    for (int i = 0; i < values->count(); ++i) {
        if (converter->asString(values->data(i)).asString().contains(searchString, Qt::CaseInsensitive)) {
            resultPositions.insert(QPoint(values->col(i), values->row(i)));
        }
    }
    // next search through userinput
    const Calligra::Tables::UserInputStorage* userInput = storage->userInputStorage();
    for (int i = 0; i < userInput->count(); ++i) {
        if (userInput->data(i).contains(searchString, Qt::CaseInsensitive)) {
            resultPositions.insert(QPoint(userInput->col(i), userInput->row(i)));
        }
    }
    // next search through comments
    const Calligra::Tables::CommentStorage* commentStorage = storage->commentStorage();
    QList<QPair<QRectF, QString> > comments = commentStorage->intersectingPairs(Calligra::Tables::Region(QRect(0, 0, KS_colMax, KS_rowMax), sheet));
    typedef QPair<QRectF, QString> RectStringPair;
    foreach (const RectStringPair& p, comments) {
        if (p.second.contains(searchString, Qt::CaseInsensitive)) {
            resultPositions.insert(p.first.topLeft().toPoint());
        }
    }
    // finally search through links
    const Calligra::Tables::LinkStorage* links = storage->linkStorage();
    for (int i = 0; i < links->count(); ++i) {
        if (links->data(i).contains(searchString, Qt::CaseInsensitive)) {
            resultPositions.insert(QPoint(links->col(i), links->row(i)));
        }
    }

    // now convert set of positions to list of actual search results
    foreach (const QPoint& p, resultPositions) {
        SheetResult result;
        result.row = p.y();
        result.column = p.x();
        qDebug() << "+++++++++++++++++  Searched string found in at >> " << result.row << result.column;
        result.background = Calligra::Tables::Cell(sheet, result.column, result.row).style().backgroundColor();
        searchResults << result;
    }

    // and sort the list of search results (a QSet is not sorted)
    qSort(searchResults);

    if(searchResults.size() > 0) {
        searchLock.lockForWrite();
        sheetsResults.insert(sheetIndex, searchResults);
        searchLock.unlock();
    }

    qDebug()<<"++++++++++++++++ sheetsearch Finish";

    return searchResults.size();
}

void SpreadsheetSearch::clearResults(int currentSheetIndex)
{

    Calligra::Tables::Sheet* sheet = 0;

    //Getting Read lock on sheet.
    bool lockResult = searchLock.tryLockForRead();

    sheet = spreadDoc->map()->sheet(currentSheetIndex);
    QList<SheetResult> sheetResults = sheetsResults.value(currentSheetIndex);

    foreach(SheetResult result, sheetResults) {
        setCellBackground(sheet,
                          result.row,
                          result.column,
                          result.background);
    }

    sheetsResults.clear();

    //Unlocking

    if(true == lockResult) {
        searchLock.unlock();
    }
}

void SpreadsheetSearch::setCellBackground(Calligra::Tables::Sheet* sheet,
                                          int row,
                                          int column,
                                          const QColor &color)
{
    Calligra::Tables::CellStorage *storage = 0;

    if(0 != sheet && 0 != (storage = sheet->cellStorage())) {
        Calligra::Tables::Region region(QPoint(column, row), sheet);

        Calligra::Tables::Style style = storage->style(column, row);
        style.setBackgroundColor(color);
        storage->setStyle(region, style);
    }
}

void SpreadsheetSearch::setWordsColor(Calligra::Tables::Sheet* currentSheet, int currentSheetIndex, int currentIndex)
{
    if(!currentSheet || (currentIndex < 0)) {
        return;
    }

    //Getting Read lock on sheet.
    bool lockResult = searchLock.tryLockForRead();

    QList<SheetResult> sheetResults = sheetsResults.value(currentSheetIndex);

    if((currentIndex > 0) && (currentIndex < sheetResults.size())) {
        setCellBackground(currentSheet,
                          sheetResults[currentIndex - 1].row,
                          sheetResults[currentIndex - 1].column,
                          highlightColor);
    } else if((currentIndex == 0) && (currentIndex < sheetResults.size())) {
        setCellBackground(currentSheet,
                          sheetResults[sheetResults.size() - 1].row,
                          sheetResults[sheetResults.size() - 1].column,
                          highlightColor);
    }

    setCellBackground(currentSheet,

                      sheetResults[currentIndex].row,
                      sheetResults[currentIndex].column,
                      highlightColorCurrent);

    if((currentIndex >= 0) && (currentIndex < (sheetResults.size() -1))) {
        setCellBackground(currentSheet,
                          sheetResults[currentIndex + 1].row,
                          sheetResults[currentIndex + 1].column,
                          highlightColor);
    } else if((currentIndex > 0) && (currentIndex == sheetResults.size() - 1)) {
        setCellBackground(currentSheet,
                          sheetResults[0].row,
                          sheetResults[0].column,
                          highlightColor);
    }

    //Unlocking
    if(true == lockResult) {
        searchLock.unlock();
    }

}

void SpreadsheetSearch::highlightSheetResult(Calligra::Tables::Sheet* sheet, int sheetIndex, int currentIndex)
{
    qDebug() << __PRETTY_FUNCTION__ ;

    if(!sheet) {
        return;
    }

    //Getting Read lock on sheet.
    bool lockResult = searchLock.tryLockForRead();

    QList<SheetResult> sheetResults = sheetsResults.value(sheetIndex);

    //Unlocking
    if(true == lockResult) {
        searchLock.unlock();
    }

    if(sheetResults.size() < 500) {
        for(int i = 0; i < sheetResults.size(); i++) {
            if(i == currentIndex) {
                setCellBackground(sheet,
                                  sheetResults[i].row,
                                  sheetResults[i].column,
                                  highlightColorCurrent);
            } else {
                setCellBackground(sheet,
                                  sheetResults[i].row,
                                  sheetResults[i].column,
                                  highlightColor);
            }
        }
    } else {
        if(currentIndex < 0) {
            return;
        }

        if((currentIndex > 0) && (currentIndex < sheetResults.size())) {
            setCellBackground(sheet,
                              sheetResults[currentIndex - 1].row,
                              sheetResults[currentIndex - 1].column,
                              highlightColor);
        }

        setCellBackground(sheet,

                          sheetResults[currentIndex].row,
                          sheetResults[currentIndex].column,
                          highlightColorCurrent);

        if((currentIndex >= 0) && (currentIndex < (sheetResults.size() -1))) {
            setCellBackground(sheet,
                              sheetResults[currentIndex + 1].row,
                              sheetResults[currentIndex + 1].column,
                              highlightColor);
        }
    }
}

QRectF SpreadsheetSearch::mapSearchResult(Calligra::Tables::Sheet* sheet, int sheetIndex, int currentIndex)
{
    QRectF rect;

    if(sheet) {
        //Getting Read lock on sheet.
        bool lockResult = searchLock.tryLockForRead();

        QList<SheetResult> sheetResults = sheetsResults.value(sheetIndex);

        // Unlocking lock

        if(true == lockResult) {
            searchLock.unlock();
        }

        if(0 <= currentIndex && currentIndex < sheetResults.size()) {
            QRect cell(sheetResults[currentIndex].column, sheetResults[currentIndex].row, 1, 1);

            //Lets get cell coordinate into document coordinate
            rect = sheet->cellCoordinatesToDocument(cell);
        }

        qDebug() << __PRETTY_FUNCTION__  << rect;
    }

    return rect;

}


SpreadsheetUtils::SpreadsheetUtils()
{
}

QSizeF SpreadsheetUtils::contentRect(const Calligra::Tables::Sheet *sheet, Calligra::Tables::SheetView *sheetView)
{
    if (!sheet || !sheetView) {
        return QSizeF(0, 0);
    }

    QMap<const Calligra::Tables::Sheet *, QSizeF>::const_iterator usedArea(m_contentRects.find(sheet));
    if (usedArea != m_contentRects.constEnd()) {
        return usedArea.value();
    }
    QRect area = sheet->usedArea(true);
    QSize obscuredRange = sheetView->totalObscuredRange();
    if (obscuredRange.width() > area.right()) {
        area.setRight(obscuredRange.width());
    }
    if (obscuredRange.height() > area.bottom()) {
        area.setBottom(obscuredRange.height());
    }
    qDebug() << __PRETTY_FUNCTION__ << area.width() << area.height() << area.right() << area.bottom();
    // the 2 are there to see one row/column after the last row/column
    QSizeF size(sheet->columnPosition(area.right() + 2), sheet->rowPosition(area.bottom() + 2));
    m_contentRects.insert(sheet, size);
    return size;
}

void SpreadsheetUtils::clearContentRectCache(const Calligra::Tables::Sheet *sheet)
{
    m_contentRects.remove(sheet);
}
