/*
 * This file is part of Meego Office UI for KOffice
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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
#include "pdfimagecache.h"

#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

#include "pdfpagewidget.h"

static const int MAX_CACHE_SIZE = 60000000;

struct PdfImageData
{
    PdfImageData()
    : scale(-20)
    , scaled(false)
    , updating(false)
    , useCount(-1)
    , pageWidget(0)
    , updatingThumbnail(false)
    {}

    void update(const QImage &newImage, qreal newScale)
    {
        image = newImage;
        qDebug() << __PRETTY_FUNCTION__ << "updating scale" << scale << newScale;
        scale = newScale;
        scaled = false;
        updating = false;
    }

    void scaleImage(qreal newScale)
    {
        qreal tempScale = newScale / scale;
        int width  = image.size().width() * tempScale;
        int height = image.size().height() * tempScale;
        image = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qDebug() << __PRETTY_FUNCTION__ << "updating scale" << scale << newScale << image.size();
        scale = newScale;
        scaled = true;
        updating = false;
    }

    void updateThumbnail(const QImage &newThumbnail)
    {
        thumbnail = newThumbnail;
        updatingThumbnail = false;
    }

    QImage image;
    qreal scale;
    bool scaled;
    bool updating;
    int useCount;
    PdfPageWidget *pageWidget;

    QImage thumbnail;
    bool updatingThumbnail;
};

class PdfImageCache::Private
{
public:
    Private(int pageCount)
    : images(pageCount)
    , size(0)
    , count(0)
    , currentUseCount(0)
    , cleanupPosition(0)
    {
    }

    ~Private()
    {
    }

    QVector<PdfImageData> images;
    QMutex mutex;
    int size;
    int count;
    int currentUseCount;
    int cleanupPosition;
};

PdfImageCache::PdfImageCache(int pageCount)
: d(new Private(pageCount))
{
}

PdfImageCache::~PdfImageCache()
{
}

QImage PdfImageCache::getImage(int pageIndex, qreal scale, PdfPageWidget *pageWidget)
{
    if (pageIndex < 0 || pageIndex >= d->images.size()) {
        return QImage();
    }

    QMutexLocker lock(&d->mutex);
    PdfImageData &data = d->images[pageIndex];
    d->images[pageIndex].pageWidget = pageWidget;

    if (data.scale == scale) {
        qDebug() << __PRETTY_FUNCTION__ << "image in cache" << data.scaled << data.scale << scale << qAbs(data.scale - scale);
        updateUseCount(data);
        return data.image;
    }
    else if (!data.scaled && qAbs(data.scale - scale) < 20) {
        // use scale current image and use that.
        qDebug() << __PRETTY_FUNCTION__ << "scaled" << data.scaled << data.scale << scale << qAbs(data.scale - scale);
        QSize oldSize(data.image.size());
        int oldImageSize(oldSize.width() * oldSize.height());

        data.scaleImage(scale);
        updateUseCount(data);

        QSize size(data.image.size());
        int imageSize = size.width() * size.height();
        d->size = d->size + imageSize - oldImageSize;

        return data.image;
    }
    else {
        QImage image = data.image;
        if (!data.updating) {
            qDebug() << __PRETTY_FUNCTION__ << "update scale" << data.scaled << data.scale << scale << qAbs(data.scale - scale);
            data.updating = true;
            lock.unlock();
            // trigger loading of page
            emit loadPage(pageIndex, scale);
        }
        else {
            lock.unlock();
        }
        return image;
    }
}

void PdfImageCache::setImage(int pageIndex, qreal scale, const QImage &image)
{
    QSize size(image.size());
    int imageSize = size.width() * size.height();
    QMutexLocker lock(&d->mutex);
    PdfImageData &data = d->images[pageIndex];
    if (data.image.isNull() && imageSize > 0) {
        d->count++;
    }
    QSize oldSize(data.image.size());
    int oldImageSize(oldSize.width() * oldSize.height());
    d->size = d->size + imageSize - oldImageSize;
    data.update(image, scale);
    updateUseCount(data);
    PdfPageWidget *pageWidget = data.pageWidget;
    if (d->size > MAX_CACHE_SIZE) {
        cleanupCache();
    }
    lock.unlock();
    qDebug() << __PRETTY_FUNCTION__ << pageIndex << scale << pageWidget << image.size() << d->size;

    // trigger repainting of page
    if (pageWidget) {
        emit updatePageWidget(pageWidget);
    }
}

void PdfImageCache::updateUseCount(PdfImageData &data)
{
    // we already have the lock when this function is called
    if (data.useCount < d->currentUseCount) {
        data.useCount = ++d->currentUseCount;
    }
}

void PdfImageCache::cleanupCache()
{
    // we already have the lock when this function is called
    qDebug() << __PRETTY_FUNCTION__ << d->size << d->cleanupPosition;
    bool finished = false;
    int newCleanupPosition = d->cleanupPosition;
    for (int i = d->cleanupPosition; i < d->images.size() && !finished; ++i) {
        finished = cleanupCacheEntry(i);
        newCleanupPosition = i;
    }
    for (int i = 0; i < d->cleanupPosition && !finished; ++i) {
        finished = cleanupCacheEntry(i);
        newCleanupPosition = i;
    }
    d->cleanupPosition = newCleanupPosition;
    qDebug() << __PRETTY_FUNCTION__ << d->size << d->cleanupPosition;
}

bool PdfImageCache::cleanupCacheEntry(int index)
{
    PdfImageData &data = d->images[index];
    // keep at least the last 3 images
    QSize size(data.image.size());
    int imageSize = size.width() * size.height();

    qDebug() << __PRETTY_FUNCTION__ << "check" << index << imageSize << data.useCount << d->currentUseCount - 2 << data.updating;

    if (data.useCount < d->currentUseCount - 2 && imageSize > 0 && data.updating == false) {
        qDebug() << __PRETTY_FUNCTION__ << "removing" << index << imageSize;
        data.image = QImage();
        data.scale = -20;
        data.scaled = 0;
        d->size -= imageSize;
        if (d->size < MAX_CACHE_SIZE) {
            return true;
        }
    }
    return false;
}

QImage PdfImageCache::getThumbnail(int pageIndex, qreal scale)
{
    qDebug() << __PRETTY_FUNCTION__ << pageIndex << scale;
    if (pageIndex < 0 || pageIndex >= d->images.size()) {
        return QImage();
    }

    QMutexLocker lock(&d->mutex);
    PdfImageData &data = d->images[pageIndex];

    if (data.thumbnail.isNull() && !data.updatingThumbnail) {
        qDebug() << __PRETTY_FUNCTION__ << "loadThumbnail" << pageIndex << scale;
        data.updatingThumbnail = true;
        lock.unlock();
        emit loadThumbnail(pageIndex, scale);
    }
    return data.thumbnail;
}

void PdfImageCache::setThumbnail(int pageIndex, const QImage &image)
{
    qDebug() << __PRETTY_FUNCTION__ << pageIndex;
    QMutexLocker lock(&d->mutex);
    PdfImageData &data = d->images[pageIndex];
    data.updateThumbnail(image);

    emit thumbnailLoaded(pageIndex);
}
