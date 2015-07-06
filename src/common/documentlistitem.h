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

#ifndef DOCUMENTLISTITEM_H
#define DOCUMENTLISTITEM_H

#include <MListItem>
#include <MBasicListItem>
#include <MLabel>

#include <common_export.h>

class MGridLayoutPolicy;
class MProgressIndicator;
class MImageWidget;
class MLabel;
class MLayout;
class DocumentListPage;
class QGraphicsSceneResizeEvent;

class COMMON_EXPORT DocumentHeaderItem : public MBasicListItem
{
    Q_OBJECT;
    MProgressIndicator *progressSpinner;

public:
    DocumentHeaderItem(QGraphicsWidget *parent = 0);
    void resizeEvent(QGraphicsSceneResizeEvent * event);
    void showSpinner();
    void hideSpinner();
};

class COMMON_EXPORT DocumentListItem : public MListItem
{
    Q_OBJECT

public:
    DocumentListItem();

    MLabel *titleWidget();
    MLabel *subtitleWidget();
    MLabel *sideBottomSubtitleWidget();
    MImageWidget *sideTopImageWidget();
    MImageWidget *imageWidget();
    MProgressIndicator *spinner();

    void setTitle(const QString &text);
    void setSubtitle(const QString &text);
    void setSideBottomTitle(const QString &text);

    void setPage(DocumentListPage *page);

protected:
    virtual MLayout * createLayout();
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);

public slots:
    void hideSpinner();
    void showSpinner();

private:
    MLayout *layout;
    MGridLayoutPolicy *layoutPolicy;
    MLabel *titleLabel;
    MLabel *subtitleLabel;
    MImageWidget *icon;
    MImageWidget *sideTopIcon;
    MLabel *sideBottomSubtitleLabel;
    MProgressIndicator *itemSpinner;
    DocumentListPage *m_page;
};

#endif // DOCUMENTLISTITEM_H
