#ifndef SPREADSHEETCOMMON_H
#define SPREADSHEETCOMMON_H

#include <QThread>
#include <QMap>
#include <QReadWriteLock>
#include <QColor>
#include <QRectF>
#include <QSizeF>
#include "documentviewer_export.h"

class SpreadsheetUtils;

namespace Calligra
{
namespace Tables
{

class Doc;

class View;

class Cell;

class Sheet;

class SheetView;

}
}

/*!
 * \typedef SheetResult
 * \brief The struct for single search result within sheet.
 */

typedef struct {
    int     row;
    int     column;
    QColor  background;
} SheetResult;

/*!
 * \class SpreadsheetSearch
 * \brief The class provides search for spreadsheet
 *
 */

class DOCUMENTVIEWER_EXPORT SpreadsheetSearch : public QThread
{
    Q_OBJECT

signals:
    /*!
     * \
     * \brief To show the first highlight page
     * \param PageIndex
     */
    void setResults(int, int);

    /*!
     * \
     * \brief Emit when search finish
     * \param
     */
    void searchFinished();

public:
    /*!
     * \
     * \brief Constructor
     */
    SpreadsheetSearch(SpreadsheetUtils *utils, Calligra::Tables::Doc *doc, int count);

    /*!
     * \
     * \brief Destructor
     */
    virtual ~SpreadsheetSearch();

    /*!
     * \brief Search given string from the sheet and appeds it
     * to existsing search result (does not clear or highlight the view).
     * \param sheet to be searched
     * \param sheetindex to store the result
     * \returns None
     */
    int searchSheet(Calligra::Tables::Sheet* sheet, int sheetIndex);

    /*!
     * \brief Clears both search result and highlighting for sheet.
     */
    void clearResults(int sheetIndex);

    /*!
     * \brief Highlighs text found in #search call.
     * \param sheet to be searched
     * \param sheetindex to store the result
     * \param currentIndex is the index of current highlighing. The -1 indicates
     * that current is not in this sheet
     */
    void highlightSheetResult(Calligra::Tables::Sheet* sheet, int sheetIndex, int currentIndex);

    /*!
     * \brief Gets area of given search index
     * \param sheet to be searched
     * \param sheetindex to store the result
     * \param index is the search index
     * \returns area in item's coordinate
     */
    QRectF mapSearchResult(Calligra::Tables::Sheet* sheet, int sheetIndex, int currentIndex);

    /*!
     * \brief Overriding QThread run method.
     */
    virtual void run();

    /*!
     * \brief To set the thread data which gonna be used
     * \param
     * \param
     * \param
     * \returns None
     */
    void setData(QString searchtext, int index);

    /*!
     * \brief To put in sleep for mili second.
     */
    void mSleep(unsigned long msecs);

    /*!
     * \brief Sets background color as per the position given
     * \param currentSheet- Sheet which we have to highlight current result.
     * \param currentSheetIndex - SheetIndex.
     * \param currentWordIndex - Current Word index to which we have to highlight.
     */
    void setWordsColor(Calligra::Tables::Sheet* currentSheet, int currentSheetIndex, int currentWordIndex);

private slots:
    void startSearch();

protected:

    /*!
     * \brief Sets background color of given cell
     * \param sheet to be searched
     * \param row is the row of the cell
     * \param column is the column of the cell
     * \param color is the new background color of the cell
     */
    void setCellBackground(Calligra::Tables::Sheet* sheet, int row, int column, const QColor &color);

private:

    /*!
    * \brief ReaderWriter lock
    */
    mutable QReadWriteLock    searchLock;

    /*!
    * \brief List of cells that have the searched text
    */
    QMap < int /*sheetIndex*/, QList<SheetResult> /*searchResults*/ > sheetsResults;

    /*!
    * \brief Txt to be search.
    */
    QString searchString;

    /*!
    * \brief Pointer of utils
    */
    SpreadsheetUtils *spreadSheetUtils;

    /*!
    * \brief spreadDoc
    */
    Calligra::Tables::Doc *spreadDoc;

    /*!
    * \brief CurrentPage from search to start.
    */
    int     currentSheetIndex;

    /*!
    * \brief slideCount
    */
    int slideCount;

};

class SpreadsheetUtils
{
public:
    SpreadsheetUtils();

    /*!
     * \brief Gets area of text contained in spreadsheet
     * \param sheet to be searched
     * \param sheetindex for which we want the size.
     * \returns None
     */
    QSizeF contentRect(const Calligra::Tables::Sheet *sheet, Calligra::Tables::SheetView *sheetView);
    void clearContentRectCache(const Calligra::Tables::Sheet *sheet);

private:
    QMap<const Calligra::Tables::Sheet *, QSizeF> m_contentRects;
};

#endif // SPREADSHEETCOMMON_H
