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
#ifndef FINDTOOLBAR_H
#define FINDTOOLBAR_H

#include <MOverlay>
#include <common_export.h>

class MButton;
class MProgressIndicator;
class MImageWidget;
class MTextEdit;
class MStylableWidget;
class QGraphicsLinearLayout;
class QPropertyAnimation;

class COMMON_EXPORT FindToolbar : public MOverlay
{
    Q_OBJECT
public:
    explicit FindToolbar(QGraphicsItem * parent = 0);

    void setMatchFound(bool found);
    bool matchFound() const;
    QString text() const;
    void setText(const QString &text);

    bool hideVkb();
    bool isActive() const;

signals:
    void findFirst();
    void findPrevious();
    void findNext();

public slots:
    void hide();
    void show();
    void showSearchIndicator();

private slots:
    void textChanged();
    void returnPressed();
    void slotFindNext();
    void slotFindPrevious();
    void slotAnimationCompleted();
    void slotClear();

private:
    void startAnimation(int startValue, int endValue);
    void setItem(MWidget *widget);

    MTextEdit *m_edit;
    QGraphicsLinearLayout *m_iconLayout;
    MButton *m_clear;
    MImageWidget *m_magnifier;
    MProgressIndicator *m_searchIndicator;
    MButton *m_next;
    MButton *m_previous;

    int m_registerdAttributeExtension;
    bool m_matchFound;
    QPropertyAnimation *m_animation;
};

#endif // FINDTOOLBAR_H
