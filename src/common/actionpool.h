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

#ifndef ACTIONPOOL_H
#define ACTIONPOOL_H

#include <MAction>

class ActionPoolPrivate;

#include <common_export.h>

/*!
 * \class ActionPool
 * \brief A class that provides pool of sigelton MAction objects.
 */

class COMMON_EXPORT ActionPool: public QObject
{
    Q_OBJECT

public:
    /*!
    * \brief Defines the uniq id for actions used in office-tools.
    */
    enum Id {
        //! Action to show normall page/slide/sheet view
        ShowNormalView,
        //! Action to set normal view into full screen mode
        FullScreenNormalView,
        //! Action to set normal view into normal screen mode
        NormalScreenNormalView,
        //!Layout options
        LayoutOptions,
        //! Number of columns in all pages view
        TwoThumbsPerColumn,
        //! Number of columns in all pages view
        ThreeThumbsPerColumn,
        //! Number of columns in all pages view
        FourThumbsPerColumn,
        //! Number of columns in all pages view
        FiveThumbsPerColumn,
        //! Front Page View
        ShowFrontPageView,
        //!All Pages View
        ShowAllPagesView,
        //!Mark As Favorite
        MarkFavorite,
        //!UnMark As Favorite
        UnMarkFavorite,
        //! Edit commants action
        EditComments,
        //! Export commants action
        ExportComments,
        //! View commants action
        ViewDetails,
        //! Delete document action
        Delete,
        //! Launch office-tool help action
        Help,
        //! This is a dummy action. TODO Remove this
        Label,
        //! Save document as action.
        SaveAs,
        //! Launch Find in document dialog action.
        Find,
        //! Launch Find in All Page Find.
        AllPageFind,
        //! Share document action.
        Share,
        //Zoom level title
        Zoomlevels,
        //! Zoom fit to width action.
        ZoomFitToWidth,
        //! Zoom fit to page action.
        ZoomFitToPage,
        //! Zoom to last user defined zoom level action.
        ZoomLastUserDefined,
        //! Zoom to 100 % action.
        Zoom100percent,
        //! Zoom to 120 % action. The spread sheet default zoom
        Zoom120percent,
        //! Internal widget action with buttons ZoomFitToWidth and ZoomFitToPage
//        ZoomWidgetPart1,
        //! Internal widget action with buttons ZoomLastUserDefined and Zoom100percent
//         ZoomWidgetPart2,
        //! Toogle Zoom between last user defined and default zoom level action.
        ZoomToogleZoom,
        //Spreadsheet indicator title
        Indicators,
        //! Spread sheet indicator action
        SpreadSheetFloatingIndicators,
        //! Spread sheet indicator action
        SpreadSheetFixedIndicators,
        //! Spread sheet indicator action
        SpreadSheetNoIndicators,
        //! Find first match in document action
        FindFirst,
        //! Find previuos match in document action
        FindPrevious,
        //! Find next match in document action
        FindNext,
        //! Open context menu action
        ContextMenu,
        //! Jump to Page number
        EnterPageNumber,
        //! All PageView Jump to Page number
        AllPageJump,
        //! Two pages action for Viewer menu
        TwoPages
    };

public:
    static ActionPool * instance();
    virtual ~ActionPool();

    /*!
     * \brief Gets the action for given id.
     * The owner ship is not changed, that is ActionPool keeps the owner ship.
     * \returns pointer to action or null if action id not found or initialized.
     */
    MAction * getAction(ActionPool::Id id) const;

public slots:
    /*!
     * \brief Sets the last user defined zoom level.
     * The button text in ZoomWidgetPart2 MWidgetAction is updated.
     * \param factor is the new user defined zoom factor (1.0 = 100%)
     */
    void setUserDefinedZoomFactor(qreal factor);

protected:
    /*!
     * \brief Update the last user defined zoom level in ZoomWidgetPart2
     * MWidgetAction..
     */
    void updateUserDefinedZoomFactortext();

private:
    ActionPool(QObject * parent = 0);
    /*!
     * \brief Initializes the actions are there locations.
     */
    void init();

private:
    static ActionPool singleton;
    ActionPoolPrivate  *data;

};

#endif //end of ACTIONPOOL_H
