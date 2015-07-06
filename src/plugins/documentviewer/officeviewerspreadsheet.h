/*
 * This file is part of Meego Office UI for KOffice
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#ifndef OFFICEVIEWERSPREADSHEET_H
#define OFFICEVIEWERSPREADSHEET_H

#include <QTextCharFormat>

#include "officeviewer.h"
#include "searchresult.h"
#include "pannablescrollbars.h"
#include "documentviewer_export.h"
class QTextDocument;

class KoDocument;

class KoCanvasController;

namespace Calligra
{
namespace Tables
{

class Doc;

class View;

class Cell;

class Sheet;

class CanvasItem;
}
}

/*!
 * \typedef SpreadSheetResult
 * \brief The struct for single search result within spreadsheet.
 * The #SpreadSheetResult::count is the count of matched text in a sheet or
 * the current index.
 */

typedef struct {
    int     sheetIndex;
    int     searchIndex;
    int     count;
} SpreadSheetResult;


/*!
 * \class OfficeViewerSpreadsheet
 * \brief The class is an #OfficeViewer class for showing Spreadsheet document.
 * The class creates a MPannableViewport where are each sheet is shown on
 * it own koffice view.
 */

class DOCUMENTVIEWER_EXPORT OfficeViewerSpreadsheet : public OfficeViewer
{
    Q_OBJECT

public:

    OfficeViewerSpreadsheet(QGraphicsWidget *parent = 0);
    virtual ~OfficeViewerSpreadsheet();

    /*!
     * \brief To createKoWidget.
     * \return boolean
     */
    bool createKoWidget();

    /*!
     * \brief To get graphics Layout Item.
     * \param zoomLevel
     * \return QGraphicsLayoutItem
     */
    QGraphicsLayoutItem *getGraphicsLayoutItem();

    /*!
     * \brief Zooming function.
     * \param zoomLevel
     * \return None
     */
    void zoom(const ZoomLevel &newlevel);

    /*!
     * \brief To clear search results.
     * \param SearchString
     * \return None
     */
    void clearSearchResults();

    /*!
     * \brief To start search.
     * \param SearchString
     * \return None
     */
    void startSearch(const QString & searchString);

    /*!
     * \brief To move next search result .
     * \param None
     * \return None
     */
    void nextWord();

    /*!
     * \brief To move to previous search result .
     * \param None
     * \return None
     */
    void previousWord();

    /*!
     * \brief To get previews of spreadsheet used in allpages view.
     * \param None
     * \return List of images of sheets
     */
//    QList<QImage*> getPreviews();

    QImage * getThumbnail(int page);

    virtual int pageCount();

    /*!
     * \brief To get currentVisiblePage in case of all pages view.
     * \param ThumbProvider
     * \return None
     */
    void getCurrentVisiblePages(ThumbProvider *thumbProvider);

    virtual void pinchStarted(QPointF &center);
    virtual qreal pinchUpdated(qreal zoomFactor);
    virtual void pinchFinished(const QPointF &center, qreal scale);

    /*!
     * \brief To put result in center
     * \param pageIndex
     * \return None
     */
    void centerToResult(int index);

    /*!
     * \brief To get the sheet name of the given sheet
     * \param sheetIndex
     * \return QString sheet name
     */
    QString sheetName(int sheetIndex);

signals:

    void updateWidgetSize();

    void showingSheet(const QString &sheetName);

public slots:

    /*!
     * \brief To showPage
     * \param PageIndex
     */
    void showPage(int pageIndex);

    virtual void updateRange();

protected slots:

    /*!
     * \brief this is called when resouce changed. Used in to update pagenumber and showpage
     */
    void resourceChanged(int key, const QVariant &value);

    /*!
     * \brief Updates widget sizes for example when orientation changes
     */
    void updateSizes();

    /*!
     * \brief Slot for getting sheet number changed signals.
     */
    void setCurrentPage(int pageIndex);

    /*!
     * \brief Slot for updating current and amount of sheets
     */
    void updatePageNumbers();

    /*!
     * \brief Slot for changing indicator mode
     */
    void setFloatingIndicators();

    /*!
     * \brief Slot for changing indicator mode
     */
    void setFixedIndicators();

    /*!
     * \brief Slot for changing indicator mode
     */
    void setNoIndicators();

    /*!
     * \brief Slot for hiding/showing the indicators
     */
    void panIndicators(qreal x, qreal y);

    /*!
     * \brief Slot for handling link taping
     */
    void shortTap(const QPointF &point, QObject *object);

    /*!
    * \brief Slot for handling selection or activation of new tool.
    * \param canvas  the currently active canvas.
    * \param uniqueToolId    a random but unique code for the new tool
    */
    void activeToolChanged(KoCanvasController* canvas, int uniqueToolId);

    /*!
     * \brief To set search Results
     */
    void setSearchResults(int, int);

    /*!
     * \brief To call when search finished
     */
    void searchFinished();

    /*!
     * \brief To call when orientation changes
     */
    void orientationChanged();

    /*!
     * \brief sets the document offset to the given \param point
     */
    void setDocumentOffset(const QPoint &point);

    /*!
     * \brief called when the area of the sheet that is covered by obscured cells changes.
     * We need to listen to this as only during rendering it will become clear how much space
     * the actual contents of the sheet might take.
     */
    void updateObscuredRange(const Calligra::Tables::Sheet* sheet);
protected:

    /*!
     * \brief Configures the sheet indicators
     */
    //void setIndicatorsMode(OfficeViewerSpreadsheetView::Mode mode);

    /*!
     * \brief Connects actions to slots
     */
    void connectActions();

    /*!
     * \brief Makes sure that given point is does not exeed given point
     * \param point is the point to be checked
     * \param maxPoint is the maximum value
     * \returns point that is not negative and not bigger then maxPoint
     */
    QPointF normalizePoint(const QPointF & point, const QPointF & maxPoint);

    /*!
     * \brief Updates viewport so that current search string is visible
     */
    void showCurrentSearchResult();

    /*!
     * \brief Scrolls the viewport to given point.
     */
    void scrollTo(QPointF point);

    void setDocumentSize(const QSizeF& size = QSizeF());
    QSizeF viewportSize() const;

    virtual QSizeF currentDocumentSize();

private:
    void setHeaderDefault();
    QSizeF contentRect(const Calligra::Tables::Sheet* sheet);

    /*!
    * \brief Private data for the spreadsheet
    *
    */

    class Private;
    Private * const d;

    /*!
    * \brief Currently shown sheet
    *
    */
    int                                 currentPage;

    /*!
    * \brief Number of sheets in the document
    *
    */
    int                                 m_lastpageCount;

    /*!
    * \brief Last used zoom level
    *
    */
    ZoomLevel                           zoomLevel;

    /*!
    * \brief Last user defined zoom factor
    *
    */
    qreal                               lastUserDefinedFactor;

    /*!
    * \brief minimum zoom factor
    *
    */
    qreal                               minimumZoomFactor;

    /*!
    * \brief List of cells that have the searched text
    *
    */
    QList<SpreadSheetResult>            searchResults;
    SpreadSheetResult                   currentSearchResult;

};

class SpreadsheetPannableScrollBars : public PannableScrollBars
{
    Q_OBJECT

public:

    SpreadsheetPannableScrollBars(QGraphicsItem *parent = 0);
    ~SpreadsheetPannableScrollBars();
    void setCanvas(KoCanvasBase *canvas);
    void restoreStartPoint();
    void setIndicatorsStatus(bool status);
    virtual QSize viewportSize() const;

public:
    Calligra::Tables::CanvasItem *canvasItem;
    bool indicatorsVisible;
};

#endif // OFFICEVIEWERSPREADSHEET_H
