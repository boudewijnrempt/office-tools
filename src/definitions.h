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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <QColor>

/*!
 * \brief Definitions of icon and css item names.
 *
 */
//! TODO Lets just use some icons for now

#define NO_ICON                 ""
#define LABEL_ICON              "Icon-contacts"
#define SHARE_ICON              "Icon-email"
#define SEARCH_ICON             "icon-m-toolbar-search-white"
#define NEXT_ICON               "icon-m-toolbar-next"
#define PREVIOUS_ICON           "icon-m-toolbar-previous"
#define FRONT_PAGE_ICON         "icon-l-documents-main-view"
#define ALL_PAGE_ICON           "icon-m-toolbar-pages-all-white"
#define JUMP_ICON               "icon-m-toolbar-jump-to-white"
/*!
 * \brief Definitions of object / css item names.
 *
 */
#define JUMPTOTOOLBAR       "jumpToToolbar"
#define JUMPTOOLBARBUTTON   "imToolbarButton"
#define FINDLEFTICONBUTTON  "findLeftIconButton"
#define FINDRIGHTICONBUTTON "findRightIconButton"
#define FINDTOOLBAR         "findToolbar"
#define FINDTOOLBARBUTTON   "findToolbarButton"
#define OFFICEWIDGET        "officeWidget"
#define PDFPAGEWIDGET       "pdfPageWidget"
#define THUMBWIDGET         "ThumbWidget"
#define THUMBPAGEINDICATOR  "thumbpageCounterLabel" //Keep first char differ from THUMBWIDGET
#define LANDSCAPELAYOUT     "landscapeSearchLayout"
#define PORTRAITLAYOUT      "portraitSearchLayout"
#define SEARCHDIALOG        "searchDialogStyle"
#define SEARCHNOMATCHES     "searchNoMatches"
/*!
 * \brief Timeouts in milliseconds
 */
const int SearchBarVisibletime                = 30000;
const int searchDelay                         = 2000;
const int PageIndicatorVisibletime            = 1000;
const int FullscreenModeTime                  = 5000;
const int ImageRemovingPeriod                 = 15000;
const int MaxWaitTimeForSecondTap             = 400;
const int ShortTapTimeLimit                   = 300;
const int KofficePageNumberUpdateIntervalTime = 1500;
const int KofficeScrollAreaReUpdateTimeout    = 500;
const int MaxThreadWaitTime                   = 1000;
const int SpreadSheetInitIntervalTime         = 500;


/*!
 * \brief Z order values
 */
const qreal ZValuePageIndicator     = 3;

/*!
 * \brief Hard code size in pixels
 */
const int ThumbnailImageWidth               = 350;
const int PixelsBetweenPages                = 10;


/*!
 * \brief Zoom limits
 */
const qreal MinZoomFactor = 1.0; //! Mimimum is 100% or fit to page
const qreal MaxZoomFactor = 5.0; //! 500% When using a value higher then 500% there are changes needed so that the backend allows that.
const qreal MinSpreadSheetZoomFactor = 0.4; //! Mimimum is 40% or fit to page
const qreal MaxSpreadSheetZoomFactor = 3.0; //! Mimimum is 200% or fit to page


/*!
 * \brief Maximum size of tapping area
 */
const QSizeF maxTapAreaSize(50,50);  //! TODO For now the area is big, check if smaller can be used in later flash releases?

const QColor highlightColor(255,255,0);
const QColor highlightColorCurrent(255,127,0);

/*!
 * \brief KOffice specific constants
 */
const QString PanToolID = "PanTool";
const QString TextToolID = "TextToolFactory_ID";
const QString InteractionToolID = "InteractionTool";
const QString CellToolID = "KSpreadCellToolId";
#define KoFingerScrollable "FingerScrollable"

/*!
 * \brief KOffice action names
 */
#define KoNextPageAction "page_next"
#define KoPrevPageAction "page_previous"


#endif //DEFINITIONS_H
