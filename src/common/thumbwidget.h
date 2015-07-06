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

#ifndef THUMBWIDGET_H
#define THUMBWIDGET_H

#include "basepagewidget.h"
#include <MWidgetController>
#include <QPixmapCache>
#include <common_export.h>

class ThumbProvider;

class MLabel;

class MProgressIndicator;
/*!
 * \class ThumbWidget
 * \brief A class for showing page thumb.
 * The #ThumbWidget has a label with page number and number of pages.
 * The #ThumbWidget uses #ThumbProvider to get page thumbs images and
 * keeps it size in same aspect ration as page thumbs image has.
 */

class COMMON_EXPORT ThumbWidget: public MWidgetController , public BasePageWidget
{
    Q_OBJECT

public:
    enum { Type = UserType + 624 };
    int type() const {
        return Type;
    }

    ThumbWidget(ThumbProvider *thumbProvider, bool thumbForSpreadSheet, QGraphicsItem *parent = 0);
    virtual ~ThumbWidget();

    /*!
     * \brief As #BasePageWidget::setPageIndex but updates also label text
     * \param newPageIndex As in #BasePageWidget::setPageIndex
     * \param pageCount Amount of pages in document
     */
    void setPageIndex(int newPageIndex, int pageCount);

    /*!
     * \brief Sets widget size
     * \param newsize is the requested widget size
     */
    void setSize(const QSizeF newsize,int nCols);

    MWidgetStyleContainer& style();

    void setNames(QString prefix);

    void setHighlightNames(QString prefix);

    QPixmapCache::Key key() { return cacheKey; }

    void setKey(const QPixmapCache::Key &key) {
        cacheKey = key;
    }

    void startSpinner(const QPointF &pos);

    void stopSpinner();

protected:

    /*!
     * \brief See QGraphicsItem::paint
     * \param painter As in QGraphicsItem::paint
     * \param option As in QGraphicsItem::paint
     * \param widget As in QGraphicsItem::paint
     */
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    /*!
     * \brief See QGraphicsWidget::resizeEvent
     * \param event As in QGraphicsWidget::resizeEvent
     */
    void resizeEvent(QGraphicsSceneResizeEvent * event);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;


private:
    ThumbProvider   *thumbProvider;
    MLabel        *label;
    QSizeF          requestedSize;
    int currentpageIndex;
    int columnCnt;
    QPixmapCache::Key cacheKey;
    bool isSpreadsheet;
    MProgressIndicator *m_spinner;
};

#endif //end of THUMBWIDGET_H


