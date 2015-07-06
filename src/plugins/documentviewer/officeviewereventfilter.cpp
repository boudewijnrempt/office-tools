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

#include "officeviewereventfilter.h"

#include <QDebug>
#include <QEvent>

OfficeViewerEventFilter::OfficeViewerEventFilter(QObject *parent)
: QObject(parent)
{
}

OfficeViewerEventFilter::~OfficeViewerEventFilter()
{
}

bool OfficeViewerEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    qDebug() << __PRETTY_FUNCTION__ << event << event->type();
    // don't allow to select shapes
    return event->type() == QEvent::GraphicsSceneMouseDoubleClick || event->type() == QEvent::GraphicsSceneMouseMove;
}
