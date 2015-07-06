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

#include <QImage>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QVector>
#include <QTime>

#include <MSceneManager>

#include <poppler-qt4.h>

#include "pdfloader.h"
#include "definitions.h"
#include "pdfimagecache.h"
#include "pdfpagewidget.h"
#include "applicationwindow.h"
#include "pdfloaderthread.h"

class PdfLoaderPrivate
{

public:
    PdfLoaderPrivate();
    virtual ~PdfLoaderPrivate();

    Poppler::Page  *page;
};

PdfLoaderPrivate::PdfLoaderPrivate()
    : page(0)
{
}

PdfLoaderPrivate::~PdfLoaderPrivate()
{
    delete page;
    page = 0;
}

PdfLoader::PdfLoader(QObject *parent)
    : QObject(parent)
    , document(0)
    , currentPageIndex(-1)
    , scene(0)
    , mHighlights(0)
    , highlightPageIndex(0)
    , highlightCurrentPosition(0)
    , thread(0)
    , m_imageCache(0)
{
    qDebug() << __PRETTY_FUNCTION__ ;
    connect(this, SIGNAL(loadNeighborPagesRequest()), this, SLOT(loadNeighborPages()));
}

PdfLoader::~PdfLoader()
{
    emit stopBackGroundLoading();
    qDebug() << __PRETTY_FUNCTION__;
    clear();
}

void PdfLoader::clear()
{
    if(0 != thread) {
        while(thread->isRunning()) {
            sleep(0.5);
            thread->quit();
        }

        delete thread;
        thread = 0;
    }

    qDeleteAll(dataItems.begin(), dataItems.end());
    dataItems.clear();
    delete document;
    document = 0;
    currentPageIndex = -1;
}

bool PdfLoader::load(const QString &filename,Poppler::Document* &mDocument)
{
    qDebug() << __PRETTY_FUNCTION__ ;
    bool retval = false;
    clear();
    document = Poppler::Document::load(filename);

    if(0 != document) {
        //Fix in case of encrypted document poppler crashing in numpages/
        qDebug()<<"is encrypted"<<document->isLocked();

        mDocument = document;

        if(document->isLocked()) {
            return false;
        }

        int numberOfPages = document->numPages();

        m_imageCache = new PdfImageCache(numberOfPages);
        Q_CHECK_PTR(m_imageCache);

        thread = new PdfLoaderThread(filename, m_imageCache);
        connect(this, SIGNAL(stopBackGroundLoading()),
                thread, SLOT(stopBackgroundLoading()));
        connect(m_imageCache, SIGNAL(loadPage(int, qreal)),
                thread, SLOT(loadPage(int, qreal)), Qt::QueuedConnection);
        connect(m_imageCache, SIGNAL(updatePageWidget(PdfPageWidget*)),
                this, SLOT(updatePage(PdfPageWidget*)), Qt::QueuedConnection);
        connect(m_imageCache, SIGNAL(loadThumbnail(int, qreal)),
                thread, SLOT(loadThumbnail(int, qreal)), Qt::QueuedConnection);

        connect(m_imageCache, SIGNAL(thumbnailLoaded(int)),
                this, SIGNAL(thumbnailLoaded(int)), Qt::QueuedConnection);

        document->setRenderHint(Poppler::Document::Antialiasing, true);
        document->setRenderHint(Poppler::Document::TextAntialiasing, true);

        //Initialize list with empty private date for each page
        dataItems.resize(numberOfPages);

        for(int i = 0; i < numberOfPages; i++) {
            PdfLoaderPrivate *data= new PdfLoaderPrivate();
            Q_CHECK_PTR(data);
            data->page = document->page(i);
            if (data->page == 0) {
                /// This shouldn't happen, Anyway to fix coverity issues.
                /// Delete the pointer and assign it to zero.
                delete data;
                data = 0;
                return false;
            }
            dataItems[i] = data;
        }

        thread->start();

        retval = true;
    }


    return retval;
}

void PdfLoader::setCurrentPage(const int pageIndex)
{
    qDebug() << __PRETTY_FUNCTION__ << pageIndex << currentPageIndex;
    if(pageIndex != currentPageIndex) {
        //qDebug() << "PdfLoader::setCurrentPage : " << currentPageIndex << " <- " << pageIndex;
        currentPageIndex = pageIndex;
        emit pageChanged(dataItems.size(), currentPageIndex + 1);
        emit loadNeighborPagesRequest();
    }
}

int PdfLoader::getCurrentPageIndex() const
{
    return currentPageIndex;
}

int PdfLoader::numberOfPages() const
{
    return dataItems.size();
}

QSize PdfLoader::pageSize(int pageIndex) const
{
    QSize retval;
    PdfLoaderPrivate *data = getPageData(pageIndex);

    if(0 != data) {
        retval = data->page->pageSize();
    }

    return retval;
}

PdfLoaderPrivate * PdfLoader::getPageData(int pageIndex) const
{
    PdfLoaderPrivate    *data = 0;

    if(0 <= pageIndex && pageIndex < dataItems.size()) {
        data = dataItems[pageIndex];

        if(0 == data->page) {
            data->page = document->page(pageIndex);
        }
    }

    return data;
}

QList<int>  PdfLoader::getItemsAtSceneArea(QRectF rect) const
{
    QList<int> pageList;
    QList<QGraphicsItem *> visibleItems;

    if(0 != scene) {
        visibleItems = scene->items(rect, Qt::IntersectsItemBoundingRect);
    }

    foreach(QGraphicsItem * item, visibleItems) {

        PdfPageWidget *newitem = qgraphicsitem_cast<PdfPageWidget *>(item);

        if(0 != newitem &&  widgetName == newitem->objectName()) {
            if(newitem->isVisible()) {
                pageList.append(newitem->getPageIndex());
            }
        }
    }

    return pageList;
}

void PdfLoader::removeUnused()
{
#if 0
    if(0 != dataItems.size()) {
        QRectF usedRect = getNeighborRect();

        QList<int> usedPages = getItemsAtSceneArea(usedRect);
        qSort(usedPages);
        if (usedPages.count() == 0) {
            return;
        }

        qreal imageScale;
        qreal requestedScale;
        container->getScales(usedPages.at(0), imageScale, requestedScale);

//        qDebug() << __PRETTY_FUNCTION__ << "  Requested Scale =   " << requestedScale;
//        qDebug() << __PRETTY_FUNCTION__ << widgetName << " numBytes : " << container->numBytes();

        if(usedPages.count() > 0) {
            int min = usedPages.at(0);
            int max = usedPages.at(usedPages.count() - 1);

            if (requestedScale > 150 && requestedScale < 250) {
                usedPages.prepend(min - 1);
                usedPages.append(max + 1);
            } else if (requestedScale > 75 && requestedScale <= 150) {
                usedPages.prepend(min - 2);
                usedPages.append(max + 2);
            } else if (requestedScale <= 75) {
                usedPages.prepend(min - 3);
                usedPages.append(max + 3);
            }
            container->removeImages(usedPages);
//            qDebug() << __PRETTY_FUNCTION__ << widgetName << " usedPages : " << usedPages.size();
        }

//        qDebug() << __PRETTY_FUNCTION__ << widgetName << " numBytes : " << container->numBytes();
    }
#endif
}

void PdfLoader::setScene(const QGraphicsScene *graphicsScene)
{
    scene = graphicsScene;
}


QList<Poppler::TextBox*> PdfLoader::getTextBoxList(int pageIndex)
{
    QList<Poppler::TextBox *> list;

    PdfLoaderPrivate *data = getPageData(pageIndex);

    if(0 != data) {
        list = data->page->textList();
    }

    return list;
}

void PdfLoader::setWidgetName(const QString & newName)
{
    widgetName=newName;
}

QList<Poppler::Link *> PdfLoader::getLinks(int pageIndex)
{
    QList<Poppler::Link *> list;
    PdfLoaderPrivate *data = getPageData(pageIndex);

    if(0 != data) {
        list = data->page->links();
    }

    return list;
}


void PdfLoader::setHighlightData(const  QHash<int, QList<QRectF> >* highlights)
{
    mHighlights = highlights;
}


const QHash<int, QList<QRectF> >* PdfLoader::getHighlightData()
{
    return mHighlights;
}


void PdfLoader::setCurrentHighlight(int pageIndex, int highlightPostion)
{
    highlightPageIndex = pageIndex;
    highlightCurrentPosition = highlightPostion;
}


void  PdfLoader::getCurrentHighlight(int &pageIndex, int &highlightPosition)
{
    pageIndex = highlightPageIndex;
    highlightPosition = highlightCurrentPosition;
}

QRectF PdfLoader::getNeighborRect()
{
    QSizeF size = ApplicationWindow::visibleSize();
    QRectF usedRect(QPointF(0,0), size * 4);
    usedRect.moveCenter(QRectF(QPointF(0,0), size).center());
    return usedRect;
}

void PdfLoader::loadNeighborPages()
{
#if 0
    QRectF usedRect = getNeighborRect();

    QList<int> usedPages = getItemsAtSceneArea(usedRect);

    int max= 0;
    int min=numberOfPages();
    foreach(int usePageIndex, usedPages) {
        qDebug() << "\tUsed pages : " << usePageIndex;
        max = qMax(max, usePageIndex);
        min = qMin(min, usePageIndex);
    }

    //Lets set next about middle of list
    int nextPage = min + (max-min) / 2;
    //Lets set prev page before next
    int prevPage = nextPage - 1;
    int pageIndex = -1;
    qreal imageScale = 0;
    qreal requestedScale = 0;

    bool contains=true;

    //Lets loop while next or prev are found in usedPages list

    while(-1 == pageIndex && contains) {
        //Lets assume that next or prev  are not found in usedPages list
        contains=false;
        //If next found in usedPages list

        if(usedPages.contains(nextPage)) {
            //The next is found in usedPages
            contains=true;

            if(true == container->getScales(nextPage, imageScale, requestedScale)) {
                if(0 != requestedScale && imageScale != requestedScale) {
                    //The next has image scale that is not same as the valid reqeusted scale
                    pageIndex = nextPage;
                    break;
                }
            }
        }

        //If prev found in usedPages list
        if(usedPages.contains(prevPage)) {
            //The prev is found in usedPages
            contains=true;

            if(true == container->getScales(prevPage, imageScale, requestedScale)) {
                if(0 != requestedScale && imageScale != requestedScale) {
                    //The prev has image scale that is not same as the valid reqeusted scale
                    pageIndex = prevPage;
                    break;
                }
            }
        }

        //Lets set next one page forward
        nextPage++;

        //Lets set prev one page backward
        prevPage--;
    }

    if( (-1 != pageIndex) && (pageIndex < (currentPageIndex + 2)) ) {
        emit loadPage(pageIndex);
    }
#endif
}

QImage PdfLoader::getPageImage(int pageIndex, qreal scale, PdfPageWidget *pageWidget)
{
    if (m_imageCache == 0) {
        return QImage();
    }

    return m_imageCache->getImage(pageIndex, scale, pageWidget);
}

void PdfLoader::updatePage(PdfPageWidget *pageWidget)
{
    pageWidget->update();
}

QImage PdfLoader::getThumbnail(int pageIndex, qreal scale)
{
    if (m_imageCache == 0) {
        return QImage();
    }

    return m_imageCache->getThumbnail(pageIndex, scale);
}
