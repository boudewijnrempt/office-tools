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

#ifndef OFFICEINTERFACE_H
#define OFFICEINTERFACE_H

#include <MApplicationPage>

#include <QtPlugin>

class OfficeInterface
{
public:

    virtual ~OfficeInterface() {}

    // This is used to give information about the currently active doc to the plugin
    // 0 if there is no document open
    virtual void setDocument(void *doc) = 0;

    // Asks the plugin to create it's view for the main program to use
    virtual MApplicationPage *createView() = 0;

    // Should return the name of the plugin
    virtual QString pluginName() = 0;

    // What kind of plugin this is
    // Possible supported values : generic, document, pdf
    // Plugins of type generic will be shown in the main menu
    // for those, setDocument is never called
    // for types document and pdf, setDocument is called before calling view()
    virtual QString pluginType() = 0;
};

Q_DECLARE_INTERFACE(OfficeInterface, "com.office.OfficeTools.OfficeInterface/1.0")

#endif
