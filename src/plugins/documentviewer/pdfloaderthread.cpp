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

//Qt Headers
#include <QDebug>
#include <QStack>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QCoreApplication>
#include <QMutex>
#include <QQueue>

//Poppler Headers
#include <poppler-qt4.h>

#include "pdfimagecache.h"
#include "pdfloaderthread.h"
#include "definitions.h"

class PdfLoaderThread::Private
{
public:
    Private()
    : document(0)
    , imageCache(0)
    , stopLoading(false)
    {}

    ~Private()
    {
        delete document;
    }

    Poppler::Document *document;
    PdfImageCache *imageCache;
    bool stopLoading;

    QQueue<QPair<int, qreal> > queuedPages;
    QMutex queueMutex;

    QQueue<QPair<int, qreal> > queuedThumbnail;
    QMutex thumbnailMutex;
};

PdfLoaderThread::PdfLoaderThread(const QString & pdfFileName, PdfImageCache *imageCache)
: data(new Private())
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    setTerminationEnabled(true);
    QObject::moveToThread(this);
    data->imageCache = imageCache;
    data->document = Poppler::Document::load(pdfFileName);

    if(0 != data->document) {
        qDebug() << "setRenderHint";
        data->document->setRenderHint(Poppler::Document::Antialiasing, true);
        data->document->setRenderHint(Poppler::Document::TextAntialiasing, true);
    }

    connect(this, SIGNAL(pageQueued()), SLOT(loadQueuedPage()));
    connect(this, SIGNAL(thumbnailQueued()), SLOT(loadQueuedThumbnail()));
}

PdfLoaderThread::~PdfLoaderThread()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    //disconnect(this, SIGNAL(continueLoading()), this, SLOT(loadPendingItems()));
    delete data;
}

void PdfLoaderThread::stopBackgroundLoading()
{
    data->stopLoading = true;
}

void PdfLoaderThread::loadPage(int pageIndex, qreal scale)
{
    if (data->document == 0 || pageIndex >= data->document->numPages() || pageIndex < 0) {
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << pageIndex << scale << QThread::currentThread();

    QMutexLocker lock(&data->queueMutex);
    data->queuedPages.enqueue(QPair<int, qreal>(pageIndex, scale));
    lock.unlock();

    // trigger loading
    emit pageQueued();
}

void PdfLoaderThread::loadQueuedPage()
{
    qDebug() << __PRETTY_FUNCTION__ << QThread::currentThread();

    if (0 == data->document || data->stopLoading) {
        return;
    }

    QMutexLocker lock(&data->queueMutex);
    QPair<int, qreal> pageData = data->queuedPages.dequeue();
    lock.unlock();

    qDebug() << __PRETTY_FUNCTION__ << pageData.first << pageData.second;

    QImage image;
    Poppler::Page *page = data->document->page(pageData.first);
    image = page->renderToImage(pageData.second, pageData.second);
    delete page;
    if (data->imageCache) {
        // TODO is the convert needed?
        QImage tmpImage = image.convertToFormat(QImage::Format_RGB16, Qt::AutoColor);
        data->imageCache->setImage(pageData.first, pageData.second, tmpImage);
    }
}

void PdfLoaderThread::loadThumbnail(int pageIndex, qreal scale)
{
    if (data->document == 0 || pageIndex >= data->document->numPages() || pageIndex < 0) {
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << pageIndex << scale << QThread::currentThread();

    QMutexLocker lock(&data->thumbnailMutex);
    data->queuedThumbnail.enqueue(QPair<int, qreal>(pageIndex, scale));
    lock.unlock();

    // trigger loading
    emit thumbnailQueued();
}

void PdfLoaderThread::loadQueuedThumbnail()
{
    qDebug() << __PRETTY_FUNCTION__ << QThread::currentThread();

    if (0 == data->document || data->stopLoading) {
        return;
    }

    QMutexLocker lock(&data->thumbnailMutex);
    QPair<int, qreal> pageData = data->queuedThumbnail.dequeue();
    lock.unlock();

    qDebug() << __PRETTY_FUNCTION__ << pageData.first << pageData.second;

    QImage image;
    Poppler::Page *page = data->document->page(pageData.first);
    image = page->renderToImage(pageData.second, pageData.second);
    delete page;
    if (data->imageCache) {
        // TODO is the convert needed?
        QImage tmpImage = image.convertToFormat(QImage::Format_RGB16, Qt::AutoColor);
        data->imageCache->setThumbnail(pageData.first, tmpImage);
    }
}
