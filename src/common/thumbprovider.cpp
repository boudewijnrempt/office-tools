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

#include "thumbprovider.h"
#include "misc.h"

ThumbProvider::ThumbProvider(Type type, QObject * parent)
    : QObject(parent)
    , m_startLoadingThumbnail(true)
    , type(type)
    , currentPage(0)
{
}

ThumbProvider::~ThumbProvider()
{
    qDebug() << __PRETTY_FUNCTION__;
}


void ThumbProvider::clearVisibleAreas()
{
    visibleAreas.clear();
}

void ThumbProvider::addVisibleAreas(const int pageIndex, const QRectF &area, const QSizeF &pageSize)
{
    visibleAreas.insert(pageIndex, Misc::getRelativeRect(area, pageSize));
}

QRectF ThumbProvider::getVisibleArea(const int pageIndex, const QSizeF &pageSize)
{
    QRectF retval;

    if(visibleAreas.contains(pageIndex)) {
        retval = Misc::translateRelativeRect(visibleAreas.value(pageIndex), pageSize);
    }

    return retval;
}

void ThumbProvider::thumbsVisibilityChanged()
{
}

void ThumbProvider::setViewData(QGraphicsScene *scene, const QString &widgetName)
{
    Q_UNUSED(scene);
    Q_UNUSED(widgetName);
}

void ThumbProvider::setType(Type newType)
{
    type = newType;
}

void ThumbProvider::setPageIndex(int page)
{
    currentPage = page;
}

int ThumbProvider::getPageIndex()
{
    return currentPage;
}

ThumbProvider::Type ThumbProvider::getType()
{
    return type;
}
