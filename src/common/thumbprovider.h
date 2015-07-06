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
#ifndef THUMBPROVIDER_H
#define THUMBPROVIDER_H

#include <QHash>
#include <QPainter>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

#include "thumbwidget.h"

#include <common_export.h>

/*!
 * \class ThumbProvider
 * \brief An abstract class that defines interface for page tumb provider.
 * The #AllPagesPage and #ThumbWidget uses this interface to get page images, current visible page areas.
 *
 * The visible page areas are stored as relative are, that is QRectF(QPointF(0,0), QSizeF(1,1))
 * is the whole page visible.
 */

class COMMON_EXPORT ThumbProvider: public QObject
{
    Q_OBJECT

signals:
    /*!
    * \brief Signal to inform that number of pages or aspect rations has changed
    */
    void pagesChanged();

public:

    /*!
    * \brief Definitions of thumb type
    */
    enum Type {
        //! Thumbs are pages as in pdf or word
        Page,
        //! Thumbs are slides as in presentation
        Slide,
        //! Thumbs are sheets as in spreadsheet
        Sheet
    };

public:
    ThumbProvider(Type type=Page, QObject * parent = 0);
    virtual ~ThumbProvider();

    /*!
     * \brief paints the thumbnail on the specified QPainter
     * \param painter is the painter
     * \param option is the options for painting
     * \param widget is the widget where it is painted
     */
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, ThumbWidget *widget) = 0;

    /*!
     * \brief Returns number of pages/slides/sheets in document.
     * \return number of pages/slides/sheets
     */
    virtual int getPageCount() = 0;

    /*!
     * \brief Provides the visible area of given page index.
     * \param pageIndex is the given page index
     * \param size is the size of the #ThumbWidget
     * \return the visible area in #ThumbWidget size
     */
    virtual QRectF getVisibleArea(const int pageIndex, const QSizeF &size);

    /*!
     * \brief Provides size of given page thumb.
     * \param pageIndex is page index hows size is returned
     * \param maxSize is max thumb size
     * \return page thumb size with correct aspect ration
     */
    virtual QSizeF getThumbSize(const int pageIndex, const QSizeF &maxSize) = 0;

    /*!
     * \brief The all pages view gives view data.
     * The defaul implementation does nothing.
     * \param scene is scene where thumb widgets are
     * \param widgetName is the name of the thumb widgets
     */
    virtual void setViewData(QGraphicsScene *scene, const QString &widgetName);

    /*!
    * \brief Clear all items in visibleAreas hash
    */
    virtual void clearVisibleAreas();

    /*!
     * \brief Appends item into visibleAreas hash
     * \param pageIndex is the page index of the visible page
     * \param area is the visible area of the page
     * \param pageSize is the size of the page
     */
    virtual void addVisibleAreas(const int pageIndex, const QRectF &area, const QSizeF &pageSize);

    /*!
     * \brief Set thumb type
     * \param newType is the thumb type
     */
    void setType(Type newType);

    void setPageIndex(int  nPageIndex);

    int getPageIndex();

    virtual void getPreviews()
    {
        emit pagesChanged();
    }


    /*!
     * \brief Get current thumb type
     * \returns the thumb type
     */
    Type getType();

public slots:
    /*!
     * \brief This slot is called to update hash of visible pages.
     * This slot is called before thumbs gets visible.
     */
    virtual void updateVisibleAreas() = 0;

    /*!
     * \brief The slot is called when #ThumbWidget objects are hidden or show.
     * The default implementation does nothing
     * \param visibility true if #ThumbWidget objects are show. If false then
     * all images can be deleted.
     */
    virtual void thumbsVisibilityChanged();

    virtual void panningStopped()
    {
        qDebug() << __PRETTY_FUNCTION__ << "  Panning stopped................";
        m_startLoadingThumbnail = true;
    }

    virtual void panningStarted(const QPointF &position)
    {
        qDebug() << __PRETTY_FUNCTION__ << "  Panning started................" << position;
        if (position.y() > 0) {
            m_startLoadingThumbnail = false;
        }
    }

    bool startLoading()
    {
        return m_startLoadingThumbnail;
    }

protected:
    QSizeF               m_lastThumbanailSize;

private:
    //! Contains relative areas of the visible pages (page index and area)
    QHash<int, QRectF> visibleAreas;
    bool m_startLoadingThumbnail;
    Type                type;
    int currentPage;

};

#endif //end of THUMBPROVIDER_H
