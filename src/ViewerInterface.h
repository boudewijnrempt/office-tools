/*
 * This file is part of Office-tools
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Lassi Nieminen <lassniem@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef ViewerInterface_H
#define ViewerInterface_H

#include <QtPlugin>
#include "documentpage.h"
#include "applicationwindow.h"
/**
 * base interface class for the document viewer plugin
 */
class ViewerInterface
{
public:

    virtual ~ViewerInterface() {}
    virtual DocumentPage *createDocumentPage(ApplicationWindow::DocumentType documentType, const QString &filePath) = 0;

};

Q_DECLARE_INTERFACE(ViewerInterface, "com.office.OfficeTools.ViewerInterface/1.0")

#endif
