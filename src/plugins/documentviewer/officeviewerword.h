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
#ifndef OFFICEVIEWERWORD_H
#define OFFICEVIEWERWORD_H

#include <QTextCharFormat>

#include <KWPage.h>

#include "officeviewer.h"
#include "searchresult.h"
#include "documentviewer_export.h"
#include "officefind.h"

class QTextDocument;

class QAction;

class KActionCollection;

class KoDocument;

class KoCanvasController;

class KoZoomController;

class KWCanvasItem;
class KWView;
/*!
 * \class OfficeViewerWord
 * \brief The class is an #OfficeViewer class for showing KWord document.
 * The class creates a MWidget where all pages are shown using signle KWord's
 * view.
 */

class DOCUMENTVIEWER_EXPORT OfficeViewerWord : public OfficeViewer
{
    Q_OBJECT

public:

    OfficeViewerWord(QGraphicsWidget *parent= 0);
    virtual ~OfficeViewerWord();

    bool createKoWidget();

    QGraphicsLayoutItem *getGraphicsLayoutItem();

    void zoom(const ZoomLevel &newlevel);

    void startSearch(const QString & searchString);

//    QList<QImage*> getPreviews();

    void getCurrentVisiblePages(ThumbProvider *thumbProvider);

    virtual void pinchStarted(QPointF &center);
    virtual qreal pinchUpdated(qreal zoomFactor);
    virtual void pinchFinished(const QPointF &center, qreal scale);

    void clearSearchResults();

    void nextWord();

    void previousWord();

public slots:

    void showPage(int pageIndex);

protected slots:

    void updateSizes();
    void orientationChanged();
    void resourceChanged(int key, const QVariant &value);
    void updatePageNumbers();
    void shortTap(const QPointF &point, QObject *object);
    void setCurrentPage(int pageIndex);

    /*!
     * \brief Slot for handling selection or activation of new tool.
     * \param canvas  the currently active canvas.
     * \param uniqueToolId    a random but unique code for the new tool
     */
    void activeToolChanged(KoCanvasController* canvas, int uniqueToolId);


protected:

    /*!
    * \brief putting search result to center
    *
    */
    void centerToResult();

    void prepareThumbnailer();

    virtual QImage * getThumbnail(int page);

private slots:

    void offsetInDocumentMoved(int yOffset);

    /*!
     * \brief sets the document offset to the given \param point
     */
    void setDocumentOffset(const QPoint &point);

private:

    virtual QSizeF currentDocumentSize();
    void setCurrentPage(const KWPage &page);
    void goToPage(const KWPage &page);
    void goToPageOffset(const KWPage &page, const QPointF &offset);
    void previousPage();
    void nextPage();
    /*!
     * \brief Get minimum zoom factor
     *
     * The factor is depending on the orientation
     */
    qreal minimumZoomFactor() const;

private:
    KWCanvasItem *m_canvasItem;

    KoZoomController *m_zoomController;
    KWPage m_currentPage;

    KActionCollection *m_actionCollection;

    int m_currentPageNr;
    int m_pageCount;
    ZoomLevel m_zoomLevel;
    qreal m_lastUserDefinedFactor;
    OfficeFind m_find;
    QPoint m_translatedPoint;

    KWView *m_thumbnailView;

};

#endif // OFFICEVIEWERWORD_H

