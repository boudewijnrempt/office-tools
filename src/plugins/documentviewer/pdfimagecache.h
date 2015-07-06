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
#ifndef PDFIMAGECACHE_H
#define PDFIMAGECACHE_H

#include <QObject>

#include <QImage>

class PdfPageWidget;
class PdfImageData;

class PdfImageCache : public QObject
{
    Q_OBJECT
public:
    PdfImageCache(int pageCount);
    virtual ~PdfImageCache();

    QImage getImage(int pageIndex, qreal scale, PdfPageWidget *pageWidget);

    // called by pdf loader
    void setImage(int pageIndex, qreal scale, const QImage &image);

    QImage getThumbnail(int pageIndex, qreal scale);
    void setThumbnail(int pageIndex, const QImage &image);

signals:
    void loadPage(int pageIndex, qreal scale);
    void loadThumbnail(int pageIndex, qreal scale);
    void updatePageWidget(PdfPageWidget *pageWidget);
    void thumbnailLoaded(int pageindex);

private:
    void updateUseCount(class PdfImageData &data);
    bool cleanupCacheEntry(int index);
    void cleanupCache();

    class Private;
    Private * const d;
};

#endif /* PDFIMAGECACHE_H */
