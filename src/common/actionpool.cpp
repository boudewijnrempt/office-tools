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
#include <MComboBox>
#include <MLocale>
#include <MAction>
#include <QDebug>
#include "actionpool.h"
#include "definitions.h"


ActionPool ActionPool::singleton;

class ActionPoolPrivate
{

public:
    ActionPoolPrivate();
    virtual ~ActionPoolPrivate();

    QHash<ActionPool::Id, MAction*> actions;
    qreal userDefinedZoomFactor;
};

ActionPoolPrivate::ActionPoolPrivate()
    : userDefinedZoomFactor(1.0)
{

}

ActionPoolPrivate::~ActionPoolPrivate()
{
    qDebug() << __PRETTY_FUNCTION__;
}

ActionPool::ActionPool(QObject * parent)
    :QObject(parent)
    , data(0)
{
}

ActionPool::~ActionPool()
{
    qDebug() << __PRETTY_FUNCTION__;
}

ActionPool * ActionPool::instance()
{
    singleton.init();
    return &singleton;
}

//This must called after MApplication is created
//That is this won't work if called in creater when we have singleton
void ActionPool::init()
{
    if(0 == data) {
        data = new ActionPoolPrivate;

        const struct actionStruct {
            const char              *iconID;
            const char              *text;
            const char              *textId;
            Id                       id;
            MAction::Location        location;
            const char              *objectName;
        } actions[] = {

            //Views
            { NO_ICON, "Show normal view", "",                         ShowNormalView,         MAction::NoLocation,    "action_normal_view"}

            //View modes
            ,{ NO_ICON, "Fullscreen mode",  "",                         FullScreenNormalView,   MAction::NoLocation,    "action_fullscreen_mode"}
            ,{ NO_ICON, "Normal screen mode", "",                       NormalScreenNormalView, MAction::NoLocation,    "action_normal_mode"}

            // Application Menu actions
            //,{ NO_ICON, "View comments",    "qtn_offi_vm_view_comments",      EditComments,       MAction::NoLocation,              "action_view_comments"}
            ,{ NO_ICON, "Mark As Favorite", "qtn_comm_command_favorite",      MarkFavorite,       MAction::ApplicationMenuLocation, "action_mark_favorite"}
            ,{ NO_ICON, "UnMark As Favorite","qtn_comm_command_unmark_favorite", UnMarkFavorite,  MAction::ApplicationMenuLocation, "action_unmark_favorite"}
            //,{ NO_ICON, "Export comments",  "qtn_offi_exportcomments",        ExportComments,     MAction::NoLocation,              "action_export_comments"}
            ,{ NO_ICON, "Details",          "qtn_comm_object_details",        ViewDetails,        MAction::ApplicationMenuLocation, "action_view_details"}
            ,{ NO_ICON, "Delete",           "qtn_comm_delete",                Delete,             MAction::ApplicationMenuLocation, "action_toolbar_delete"}
            //,{ NO_ICON, "Help",             "qtn_comm_help",                  Help,               MAction::NoLocation,              "action_help"}
            ,{ NO_ICON, "Share",            "qtn_comm_command_share",         Share,              MAction::ApplicationMenuLocation, "action_view_share"}
            ,{ NO_ICON, "Save As",          "qtn_comm_content_save_as",       SaveAs,             MAction::ApplicationMenuLocation, "action_toolbar_save"}
            //,{ NO_ICON, "Two Pages",        "qtn_offi_vm_2_columns",          TwoPages,           MAction::NoLocation,              "action_two_pages"}

            // Command area actions
            ,{ FRONT_PAGE_ICON,"FrontPageView",    "", ShowFrontPageView, MAction::ToolBarLocation,    "action_toolbar_frontpageview"}
            ,{ JUMP_ICON,      "Enter page number","" ,EnterPageNumber,   MAction::ToolBarLocation,    "action_toolbar_pagenumber"}
            ,{ JUMP_ICON,      "Enter page number","", AllPageJump,       MAction::ToolBarLocation,    "action_toolbar_pagenumber"}
            ,{ ALL_PAGE_ICON,  "AllPagesView",     "", ShowAllPagesView,  MAction::ToolBarLocation,    "action_toolbar_allpageview"}
            ,{ SEARCH_ICON,    "Find",             "", Find,              MAction::ToolBarLocation,    "action_toolbar_search_document"}
            ,{ SEARCH_ICON,    "Find",             "", AllPageFind,       MAction::ToolBarLocation,    "action_toolbar_search_document"}
            ,{ PREVIOUS_ICON,  "Previous",         "", FindPrevious,      MAction::ToolBarLocation,    "action_toolbar_previous"}
            ,{ NEXT_ICON,      "Next",             "", FindNext,          MAction::ToolBarLocation,    "action_toolbar_next"}

            //All pages thumb per column actions
            ,{ NO_ICON, "qtn_offi_vm_layout_options",   "qtn_offi_vm_layout_options", LayoutOptions,        MAction::NoLocation,    "action_layout_option"}
            ,{ NO_ICON, "qtn_offi_vm_2_columns",        "qtn_offi_vm_2_columns",      TwoThumbsPerColumn,   MAction::NoLocation,    "action_two_column"}
            ,{ NO_ICON, "qtn_offi_vm_3_columns",        "qtn_offi_vm_3_columns",      ThreeThumbsPerColumn, MAction::NoLocation,    "action_three_column"}
            ,{ NO_ICON, "qtn_offi_vm_4_columns",        "qtn_offi_vm_4_columns",      FourThumbsPerColumn,  MAction::NoLocation,    "action_four_column"}
            ,{ NO_ICON, "5 pages",                      "qtn_offi_fivepages",         FiveThumbsPerColumn,  MAction::NoLocation,    "action_five_column"}

            //Zooming
            ,{ NO_ICON, "qtn_offi_vm_zoom_levels",      "qtn_offi_vm_zoom_levels",  Zoomlevels,             MAction::NoLocation,    "action_zoom_level"}
            ,{ NO_ICON, "qtn_offi_vm_fit_to_width",     "qtn_offi_vm_fit_to_width", ZoomFitToWidth,         MAction::NoLocation,    "action_fit_to_width"}
            ,{ NO_ICON, "qtn_offi_vm_fit_to_page",      "qtn_offi_vm_fit_to_page",  ZoomFitToPage,          MAction::NoLocation,    "action_fit_to_page"}
            ,{ NO_ICON, "qtn_offi_user_defined",        "qtn_offi_user_defined",    ZoomLastUserDefined,    MAction::NoLocation,    "action_current_scale"}
            ,{ NO_ICON, "qtn_offi_vm_actual_size",      "qtn_offi_vm_actual_size",  Zoom100percent,         MAction::NoLocation,    "action_actual_size"}
            ,{ NO_ICON, "",                             "",                         Zoom120percent,         MAction::NoLocation,    "action_zoom_120"}
            ,{ NO_ICON, "",                             "",                         ZoomToogleZoom,         MAction::NoLocation,    "action_toggle_zoom"}

            //spread sheet indicator actions
            ,{ NO_ICON, "qtn_offi_vm_indicators ",        "qtn_offi_vm_indicators",  Indicators,                     MAction::NoLocation,    "action_indicators"}
            ,{ NO_ICON, "qtn_offi_vm_floating_indicators","qtn_offi_vm_floating_indicators",         SpreadSheetFloatingIndicators,  MAction::NoLocation,    "action_floating_indicators"}
            ,{ NO_ICON, "qtn_offi_vm_fixed_indicators ",  "qtn_offi_vm_fixed_indicators",               SpreadSheetFixedIndicators,     MAction::NoLocation,    "action_fixed_indicator"}
            ,{ NO_ICON, "qtn_offi_vm_no_indicators ",     "qtn_offi_vm_no_indicators",                 SpreadSheetNoIndicators,        MAction::NoLocation,    "action_no_indicators"}

            //spread sheet indicator actions
            ,{ NO_ICON, "",                 "",                          FindFirst,              MAction::NoLocation,    "action_find_first"}

            //Other
            ,{ NO_ICON, "Open ContextMenu", "",                         ContextMenu,            MAction::NoLocation,    "action_context_menu"}
        };

        for(unsigned int i = 0; i < (sizeof(actions) / sizeof actions[0]); i++) {
            MAction *action;
            if (0 == QString::compare("", QString(actions[i].iconID))) {
                action = new MAction(qtTrId(actions[i].textId), this);
            } else {
                action = new MAction(actions[i].iconID, qtTrId(actions[i].textId), this);
            }
            Q_CHECK_PTR(action);
            action->setObjectName(actions[i].objectName);
            action->setLocation(actions[i].location);
            //The id must be uniq so check the table if asserts
            Q_ASSERT(!data->actions.contains(actions[i].id));
            data->actions[actions[i].id] = action;
        }
    }
}

MAction * ActionPool::getAction(ActionPool::Id id) const
{
    MAction *action = 0;

    if(data->actions.contains(id)) {
        action = data->actions[id];
    }

    Q_ASSERT_X(0 != action, "Missing action", "Please update the Actions::init()");

    return action;
}

void ActionPool::setUserDefinedZoomFactor(qreal factor)
{
    if(data->userDefinedZoomFactor != factor) {
        data->userDefinedZoomFactor = factor;
        updateUserDefinedZoomFactortext();
    }
}

void ActionPool::updateUserDefinedZoomFactortext()
{
#if 0
    MButton *button = data->zoomButtonGroup.button(3);

    if(0 != button) {
        MLocale myLocale; // get the current locale
        QString formattedprocent = myLocale.formatNumber(static_cast<int>(data->userDefinedZoomFactor*100));

        button->setText(qtTrId(QString("qtn_offi_user_defined %1%").arg(formattedprocent).toLatin1().data()));
    }

#endif
}
