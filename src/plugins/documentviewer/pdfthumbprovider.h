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

#ifndef PDFTHUMBPROVIDER_H
#define PDFTHUMBPROVIDER_H

#include "thumbprovider.h"
#include "documentviewer_export.h"

class PdfLoader;

class DOCUMENTVIEWER_EXPORT PdfThumbProvider: public ThumbProvider
{
    Q_OBJECT

public:
    PdfThumbProvider(PdfLoader *loader, ThumbProvider::Type type=ThumbProvider::Page, QObject * parent = 0);
    virtual ~PdfThumbProvider();

    /*!
     * \brief Opens the pdf document
     * \param scene is the scene of normal page view
     * \param widgetName is the name of normal page widgets
     */
    void init(QGraphicsScene *scene, const QString &widgetName);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, ThumbWidget *widget);

    int getPageCount();

    QSizeF getThumbSize(const int pageIndex, const QSizeF &maxSize);

    void setViewData(QGraphicsScene *scene, const QString &widgetName);

public slots:
    void updateVisibleAreas();
    void updateLoadedThumbnail(int pageIndex);
    void thumbsVisibilityChanged();

protected:
    class Private;
    Private * const data;
};

#endif //end of PDFTHUMBPROVIDER_H
