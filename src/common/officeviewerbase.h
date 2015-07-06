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

#ifndef OFFICEVIEWERBASE_H
#define OFFICEVIEWERBASE_H

#include <QGraphicsWidget>
#include "zoomlevel.h"

class QGraphicsScene;
class QGraphicsLayoutItem;
class QGraphicsWidget;
class QTextDocument;

class MImageWidget;

class ThumbImage;
class ThumbProvider;

#include <common_export.h>

/*!
 * \class OfficeViewerBase
 * \brief The abstract class that defines the interface for KOffice document widgets.
 */

class COMMON_EXPORT OfficeViewerBase : public QGraphicsWidget
{
    Q_OBJECT
public:
    OfficeViewerBase(QGraphicsWidget *parent = 0)
        : QGraphicsWidget(parent)
        {}

signals:
    /*!
     * \brief The signal is sent first or new page is viewed.
     * \param total is the total amount of pages in a document
     * \param current is the currently viewed page in a document
     */
    void pageChanged(int total, int current);

    /*!
     * \brief the documentLoaded signal is emitted when the document
     * has finished loading.
     * \param status is true when loading was succesful, false if it failed.
     */
    void documentLoaded(bool status);

    /*!
     * \brief Signal which is send when search is finished and gives as a result if a match was found
     */
    void matchesFound(bool found);

    void openingProgress(int value);

public:
    /*!
     * \brief Load KOffice document and create a widget for it.
     * \param document the filename of the document
     * \param scene  the scene where viewer will be shown
     * \param tapEventHandler The mouse event handler
     * \return true if opening of document did success
     */
    virtual bool createKoWidget() = 0;

    /*!
     * \brief Get the widget showing the loaded document.
     * The widget is created in scene given in #OfficeViewer::createKoWidget
     * \return pointer to widget. Caller will get the owner ship.
     */
    virtual QGraphicsLayoutItem *getGraphicsLayoutItem() = 0;

    /*!
     * \brief Request for change zooming mode or zoom factor.
     * \param newlevel is the new mode or factor
     */
    virtual void zoom(const ZoomLevel &newlevel) = 0;

    /*!
     *
     */
    virtual void startSearch(const QString & searchString) = 0;

    /*!
     * \brief Clear search results (and un-highlight highlighted search results)
     */
    virtual void clearSearchResults() = 0;

    /*!
     * \brief Request for searching next string in document
     */
    virtual void nextWord() = 0;

    /*!
     * \brief Request for searching previous string in document
     */
    virtual void previousWord() = 0;

    virtual int pageCount() = 0;

    /*!
     * \brief Get current visible pages/slides/sheets areas.
     * This is called before all pages view is shown
     * \param thumbProvider is thumb provider requesting the visible areas
     */
    virtual void getCurrentVisiblePages(ThumbProvider *thumbProvider) = 0;

    virtual void pinchStarted(QPointF &center) = 0;
    virtual qreal pinchUpdated(qreal zoomFactor) = 0;
    virtual void pinchFinished(const QPointF &center, qreal scale) = 0;

    /*!
     *
     */
    virtual void shortTap(const QPointF &point, QObject *object) = 0;

    /*!
     * \brief To get the sheet name of the given sheet
     * \param sheetIndex
     * \return QString sheet name
     */
    virtual QString sheetName(int /*sheetIndex*/) { return QString::null; }

    virtual QImage * getThumbnail(int page) = 0;

public slots:

    /*!
     * \brief Show given page index.
     * \param pageIndex page index to be shown (from 0 - amountOfPages()-1)
     */
    virtual void showPage(int pageIndex) = 0;

};

#endif // OFFICEVIEWER_H
