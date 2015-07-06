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

#ifndef PDFPAGE_H
#define PDFPAGE_H

#include "documentpage.h"
#include <poppler-qt4.h>
#include <QTimer>
#include <QReadWriteLock>
#include "documentviewer_export.h"
class PDFPageData;

class MLayout;

class MPannableViewport;

class MWidget;

class PdfPageWidget;

class OfficeInterface;

class DOCUMENTVIEWER_EXPORT PdfPage : public DocumentPage
{
    Q_OBJECT

signals:
    /*!
     * \brief A signal that is emitted when page should be in center on screen
     * \param pageindex Page to be centered
     * \param relativeY The relative point in page to be centered on
     * \param offset The offset to page egde to center point
     */
    void verticalCenterOnPagePoint(int pageindex, qreal relativeY, int offset);

public:

    PdfPage(const QString& filename, QGraphicsItem *parent = 0);
    virtual ~PdfPage();
    void loadDocument();

    void shortTap(const QPointF &point, QObject *object);

    void showInfoBanner(const QString &message);

public slots:
    virtual void zoom(ZoomLevel level);
    virtual void searchText(DocumentPage::SearchMode mode,
                            const QString &searchText);
    virtual void clearSearchTexts();

    void createContent();
    ThumbProvider* getThumbProvider();

protected slots:
    void geometryChanged();
    void updatePosition(const QPointF &position);
    /*!
     * \brief Slot to update widget sizes when screen orientation changes.
     */
    void orientationChanged();

    /*!
     * \brief Updates the view show page in given point.
     * \param pageindex The index of page to be shown (starts from zero)
     * \param relativeY The relative point to be shown (from 0.0 to 1.0)
     * \param offset The distance of page to center point
     */
    void setVerticalCenterOnPagePoint(int pageindex, qreal relativeY, int offset);

    /*!
     * \brief Show given page at given point.
     * \param pageindex The index of page to be shown (starts from zero)
     * \param rPoint    The point in page to be shown.
     * \param relativePoint True if rPoint is relative (from QPointF(0.0, 0.0) to QPointF(1.0, 1.0)
     */
    void showPage(int pageindex, QPointF rPoint, bool relativePoint=true);

    /*!
     * \brief Handling of Application Quit pdf link type
     */
    void requestApplicationQuit();

    /*!
     * \brief Handling of Application Close pdf link type
     */
    void requestApplicationClose();

    /*!
     * \brief Handling of Search request pdf link type
     */
    void requestSearch();

    /*!
     * \brief The slot handles clearing of loaded images when
     * normal page view is hiden
     * \param visibility Indicates if normal page view is hidden or shown
     */
    void pagesVisibilityChanged();

    /*!
     * \brief Slot for updating #PdfPageWidget alignment when viewport size changes.
     * \param range See MPannableViewport::rangeChanged
     */
    void viewPortRangeChanged(const QRectF &range);


    /*!
     * \brief to highlight the search result.
     */
    void highlightResult(int);

    /*!
     * \brief to to exit the search thread.
     */
    void searchFinished();

    void openPlugin(OfficeInterface *plugin);

    /*!
     * This updates the available space for search
     */
    virtual void updateRange();

protected:
    void showPageIndexInternal(int pageIndex);
    /*!
     * \brief Helper method for creation normal view widget
     * \returns the new widget for normal view
     */
    void createPdfView();

    /*!
     * \brief Gets position of current page
     * \param pageIndex The current page index
     * \param relativeY The relative point in center of screen
     * \param offset The distance of page to center point of screen
     */
    void getVerticalCenterPagePoint(int &pageIndex, qreal &relativeY, int &offset) const;

    /*!
     * \brief Helper method for marking widgets size changed
     */
    void invalidatePdfPageLayouts();

    /*!
     * \brief Gets widget in given point in scene
     * \param point The point where to search widget
     * \param widgetName The name of widget to be searched
     * \return The pointer to found widget, null if not found
     */
    PdfPageWidget * getWidgetAt(QPointF point, const QString &widgetName);

    /*!
     * \brief Centers the view into given point in given widget.
     * \param widget The widget to be centered
     * \param centerPagePoint The point in page to be centered
     * \param screenSize The current screen size
     */
    void centerOnPage(const PdfPageWidget *widget, const QPointF & centerPagePoint, const QSizeF &screenSize);

    virtual void pinchStarted(QPointF &center);
    virtual qreal pinchUpdated(qreal zoomFactor);
    virtual void pinchFinished(const QPointF &center, qreal scale);
    virtual QGraphicsWidget *pinchWidget();

    /*!
     * \brief Forces a point to be positive and not to exceed given maximum
     * \param point The point to normalilize
     * \param maxPoint The maximum limit of point
     * \return A normalilize point
     */
    static QPointF normalilizePoint(const QPointF &point, const QPointF &maxPoint);


private :
    void zoom(ZoomLevel level, bool keepCenter);
    void searchNext();
    void searchPrev();
    void startSearch(const QString &searchText);
    qreal minimumZoomFactor() const;
    void stopSearchThreads();

private:
    class Private;
    Private * const d;
    Poppler::Document         *mDocument;
    QHash<int, QList<QRectF> > searchData;
    bool                      enable;
    QPointF m_pinchCenterDocument;
};

#endif // PDFPAGE_H
