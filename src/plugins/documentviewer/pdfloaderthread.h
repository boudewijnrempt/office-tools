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

#ifndef PdfLoaderThread_H
#define PdfLoaderThread_H

#include <QThread>
#include <QReadWriteLock>
#include "documentviewer_export.h"

class PdfImageCache;
/*!
 * \class PdfLoaderThread
 * \brief The class provides loading of pdf image in background
 *  The class just provides async. loading of page.
 */

class DOCUMENTVIEWER_EXPORT PdfLoaderThread: public QThread
{
    Q_OBJECT

public:
    PdfLoaderThread(const QString & pdfFileName, PdfImageCache *imageCache);
    ~PdfLoaderThread();

public slots:
    void loadPage(int pageIndex, qreal scale);
    void loadThumbnail(int pageIndex, qreal scale);

    void stopBackgroundLoading();

signals:
    void pageQueued();
    void thumbnailQueued();

private slots:
    void loadQueuedPage();
    void loadQueuedThumbnail();

private:
    class Private;
    Private * const data;
};

#endif // PdfLoaderThread_H
