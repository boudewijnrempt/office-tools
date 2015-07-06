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

#ifndef OFFICETHUMBPROVIDER_H
#define OFFICETHUMBPROVIDER_H

#include "thumbprovider.h"

class OfficeViewerBase;

#include <common_export.h>

class COMMON_EXPORT OfficeThumbProvider: public ThumbProvider
{
    Q_OBJECT

public:
    OfficeThumbProvider(ThumbProvider::Type type=ThumbProvider::Page, QObject * parent = 0);
    virtual ~OfficeThumbProvider();

    /*!
     * \brief Takes owner ship of the list and uses it to provide thumbs
     * \param newOfficeViewer the object handling the pages
     */
    void init(OfficeViewerBase *newOfficeViewer);

    /*!
     * \brief Checkes if has page images or not
     * \returns true if init has been called
     */
    bool isInitilized();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, ThumbWidget *widget);

    int getPageCount();

    QSizeF getThumbSize(const int pageIndex, const QSizeF &maxSize);

    QString spreadsheetSheetName(int pageIndex);

    virtual void getPreviews();

public slots:

    void updateVisibleAreas();

private:
    OfficeViewerBase    *officeViewer;
};

#endif //end of OFFICETHUMBPROVIDER_H
