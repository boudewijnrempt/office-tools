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
#ifndef JUMPTOTOOLBAR_H
#define JUMPTOTOOLBAR_H

#include <MOverlay>
#include <common_export.h>

class QRect;
class MButton;
class MTextEdit;
class DocumentPage;
class QPropertyAnimation;

class COMMON_EXPORT JumpToPageEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit JumpToPageEventFilter(QObject *parent = 0);
    virtual ~JumpToPageEventFilter();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};


class COMMON_EXPORT JumpToToolbar : public MOverlay
{
    Q_OBJECT
public:
    explicit JumpToToolbar(DocumentPage *parent = 0);

public slots:
    void hide();
    void show();

private slots:
    void goToPage();
    void textChanged();
    void slotAnimationCompleted();

private:
    void startAnimation(int startValue, int endValue);

    MTextEdit *m_edit;
    DocumentPage *m_page;
    int m_registerdAttributeExtension;
    QPropertyAnimation *m_animation;
};

#endif // JUMPTOTOOLBAR_H
