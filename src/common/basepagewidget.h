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

#ifndef BASEPAGEWIDGET_H
#define BASEPAGEWIDGET_H

#include <common_export.h>

/*!
 * \class BasePageWidget
 * \brief The class is an MWidget class that provides page index handling.
 * Main puprose of #BasePageWidget is to provide interface for seeking pages in
 * scene.
 */

class COMMON_EXPORT BasePageWidget
{

public:
    BasePageWidget(int pageIndex = -1);
    virtual ~BasePageWidget();

    /*!
     * \brief Page index setter
     * \param newPageIndex is the page index set for widget
    */
    virtual void setPageIndex(int newPageIndex);

    /*!
     * \brief Page index getter
     * \returns the page index given in setPageIndex. Value is -1 if setPageIndex was not
     * called
     */
    virtual int getPageIndex() const;

protected:
    int pageIndex;

};

#endif //end of BASEPAGEWIDGET_H

