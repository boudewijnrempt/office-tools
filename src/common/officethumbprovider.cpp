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

#include <QDebug>
#include <QImage>
#include <QRectF>

#include <MWidgetStyle>

#include "misc.h"
#include "officethumbprovider.h"
#include "officeviewerbase.h"

OfficeThumbProvider::OfficeThumbProvider(ThumbProvider::Type type, QObject * parent)
    : ThumbProvider(type, parent)
    , officeViewer(0)
{
}

OfficeThumbProvider::~OfficeThumbProvider()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void OfficeThumbProvider::init(OfficeViewerBase *newOfficeViewer)
{
    if(0 != newOfficeViewer) {
        officeViewer = newOfficeViewer;
    }

}

void OfficeThumbProvider::getPreviews()
{
//    emit pagesChanged();
}

bool OfficeThumbProvider::isInitilized()
{
    bool retval=true;

    if(0 == officeViewer) {
        retval=false;
    }

    return retval;
}


void OfficeThumbProvider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, ThumbWidget *widget)
{
    Q_UNUSED(option);
    if ( widget && painter ) {
        int pageIndex = widget->getPageIndex();
        QRectF expsRect = option->exposedRect;
        expsRect.setTopLeft(QPointF(0,0));

        if ((!painter->clipRegion().isEmpty()) && (!painter->clipRegion().contains(expsRect.toRect()))) {
            return;
        }

        qreal thumbWidth = widget->style()->preferredSize().width();

        QPixmap thumbnail;
        if ( !QPixmapCache::find(widget->key(), &thumbnail) ) {
            qDebug() <<  __PRETTY_FUNCTION__ << startLoading() << m_lastThumbanailSize;
            if (startLoading()) {
                qDebug() << "Stopped scrolling... now lets load the image";
                QImage *image = officeViewer->getThumbnail(pageIndex);
                if (image) {
                    QSizeF pageSize = image->size();
                    qreal scale = thumbWidth / pageSize.width();
                    m_lastThumbanailSize = QSizeF(thumbWidth, (pageSize.height() * scale));
                    widget->setSize(m_lastThumbanailSize, 2);
                    QPixmap temp(m_lastThumbanailSize.toSize());
                    temp.convertFromImage(image->scaledToWidth(widget->size().width(), Qt::SmoothTransformation));
                    widget->setKey(QPixmapCache::insert(temp));
                    widget->stopSpinner();
                    painter->drawPixmap(QPointF(0,0), temp, expsRect);
                    delete image;
                }
            } else if (m_lastThumbanailSize.height() > 0) {
                widget->setSize(m_lastThumbanailSize, 2);
                widget->startSpinner(expsRect.center());
            } else {
                widget->startSpinner(expsRect.center());
            }
        } else {
            if (thumbnail.width() != thumbWidth) {
                qDebug() << "Thumbnail is scaled to Thumbnail widget ";
                QPixmap temp(thumbnail.scaledToWidth(thumbWidth));
                QPixmapCache::replace(widget->key(), temp);
                thumbnail = temp;
                m_lastThumbanailSize = QSizeF(thumbnail.size());
                widget->setSize(m_lastThumbanailSize, 2);
            } else if (thumbnail.height() != widget->preferredSize().height()) {
                m_lastThumbanailSize = QSizeF(thumbnail.size());
                widget->setSize(m_lastThumbanailSize, 2);
            }
            painter->drawPixmap(QPointF(0,0), thumbnail, expsRect);
        }
    }
}

int OfficeThumbProvider::getPageCount()
{
    return officeViewer->pageCount();
}

QSizeF OfficeThumbProvider::getThumbSize(const int pageIndex, const QSizeF &maxSize)
{
    Q_UNUSED(pageIndex);
    return maxSize;
}

void OfficeThumbProvider::updateVisibleAreas()
{
    clearVisibleAreas();

    if(0 != officeViewer) {
        officeViewer->getCurrentVisiblePages(this);
    }
}

QString OfficeThumbProvider::spreadsheetSheetName(int pageIndex)
{
    return  officeViewer->sheetName(pageIndex);
}
