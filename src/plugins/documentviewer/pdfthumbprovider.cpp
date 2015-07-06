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
#include <QVector>

#include <MApplication>
#include <MWidgetStyle>

#include "pdfloader.h"
#include "pdfthumbprovider.h"
#include "misc.h"
#include "applicationwindow.h"

class PdfThumbProvider::Private
{

public:
    Private(PdfLoader *loader)
    : loader(loader)
    , scene(0)
    {}

    PdfLoader *loader;
    QGraphicsScene *scene;
    QString widgetName;
    QMap<int, QImage *> images;
};


PdfThumbProvider::PdfThumbProvider(PdfLoader *loader, ThumbProvider::Type type, QObject * parent)
    : ThumbProvider(type, parent)
    , data(new Private(loader))
{
}

PdfThumbProvider::~PdfThumbProvider()
{
    emit pagesChanged();
    data->images.clear();
    delete data;
}

void PdfThumbProvider::init(QGraphicsScene *scene, const QString &widgetName)
{
    data->scene = scene;
    data->widgetName = widgetName;
}


void PdfThumbProvider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, ThumbWidget *widget)
{
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
            qreal pageWidth = data->loader->pageSize(pageIndex).width();
            qreal scale = PdfLoader::DPIPerInch * (thumbWidth / pageWidth);

            qDebug() <<  __PRETTY_FUNCTION__ << "getThumbnail";
            QImage thumb = data->loader->getThumbnail(pageIndex, scale);
            if (thumb.isNull()) {
                widget->startSpinner(expsRect.center());
                return;
            }
            thumbnail = QPixmap::fromImage(thumb.scaledToWidth(thumbWidth));
            m_lastThumbanailSize = QSizeF(thumbnail.size());
            widget->setKey(QPixmapCache::insert(thumbnail));
            painter->drawPixmap(QPointF(0,0), thumbnail, expsRect);
            widget->stopSpinner();
        } else {
            if (thumbnail.width() != thumbWidth) {
                qDebug() << "Thumbnail is scaled to Thumbnail widget ";
                thumbnail = thumbnail.scaledToWidth(thumbWidth);
                QPixmapCache::replace(widget->key(), thumbnail);
                m_lastThumbanailSize = QSizeF(thumbnail.size());
            }
            painter->drawPixmap(QPointF(0,0), thumbnail, expsRect);
        }

        if (m_lastThumbanailSize.height() > 0) {
            widget->setSize(m_lastThumbanailSize, 2);
        }
    }
}

int PdfThumbProvider::getPageCount()
{
    return data->loader->numberOfPages();
}

void PdfThumbProvider::setViewData(QGraphicsScene *scene, const QString &widgetName)
{
    Q_UNUSED(scene)
    //data->loader->setScene(scene);
    data->loader->setWidgetName(widgetName);
}


QSizeF PdfThumbProvider::getThumbSize(const int pageIndex, const QSizeF &maxSize)
{
    QSizeF pageSize = data->loader->pageSize(pageIndex);
    qreal scale = PdfLoader::DPIPerInch * (maxSize.width() / pageSize.width());
    QSizeF thumbSize(maxSize.width(), (pageSize.height() * scale) / PdfLoader::DPIPerInch);

    return thumbSize;
}

void PdfThumbProvider::updateVisibleAreas()
{
    clearVisibleAreas();

    QRectF sceneRect;
    QList<QGraphicsItem *> visibleItems;

    if(0 != data->scene) {
        //MSceneManager * sceneManager = MApplication::activeApplicationWindow()->sceneManager();
        //QSizeF size = sceneManager->visibleSceneSize(M::Landscape);
        QSizeF size = ApplicationWindow::visibleSize(M::Landscape);
        sceneRect = QRectF(QPoint(0,0), size);
        visibleItems = data->scene->items(sceneRect, Qt::IntersectsItemBoundingRect);

    }

    foreach(QGraphicsItem * item, visibleItems) {

        ThumbWidget *newitem = qgraphicsitem_cast<ThumbWidget *>(item);

        if(0 != newitem &&  data->widgetName == newitem->objectName()) {
            //Lets get sceneRect in wigdets coodinate
            QRectF temp= newitem->mapRectFromScene(sceneRect);
            //Lets get intersecting between sceneRect and widgets area
            QRectF visibleArea = temp.intersected(newitem->rect());

            //If widget intersects with sceneRect then mark it as visible item

            if(visibleArea.isValid()) {
                addVisibleAreas(newitem->getPageIndex(), visibleArea, newitem->size());
            }
        }
    }
}

void PdfThumbProvider::updateLoadedThumbnail(int pageIndex)
{
    QRectF sceneRect;
    QList<QGraphicsItem *> visibleItems;

    if(0 != data->scene) {
        QSizeF size = ApplicationWindow::visibleSize(M::Landscape);
        sceneRect = QRectF(QPoint(0,0), size);
        visibleItems = data->scene->items(sceneRect, Qt::IntersectsItemBoundingRect);
    }

    foreach(QGraphicsItem * item, visibleItems) {
        ThumbWidget *newitem = qgraphicsitem_cast<ThumbWidget *>(item);
        if(0 != newitem &&  data->widgetName == newitem->objectName()) {
            if(newitem->getPageIndex() == pageIndex) {
                newitem->update();
                return;
            }
        }
    }
}

void PdfThumbProvider::thumbsVisibilityChanged()
{
    data->loader->loadNeighborPages();
}


