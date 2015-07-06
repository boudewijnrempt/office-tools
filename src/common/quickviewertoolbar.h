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

#ifndef QUICKVIEWERTOOLBAR_H
#define QUICKVIEWERTOOLBAR_H

#include <MOverlay>

class MButton;
class QGraphicsGridLayout;

class QuickViewerToolbar : public MOverlay
{
    Q_OBJECT;

public:
    QuickViewerToolbar(QGraphicsItem *parent = 0);

    QGraphicsWidget * createSpacer();

signals:
    void documentCloseEvent();
    void saveDocumentAs();

private slots:
    /*!
     * \brief This slot is used for updating possition of the indicator.
     */
    void updatePosition(const M::Orientation &orientation);

private:
    MButton             *saveButton;
    MButton             *closeButton;
    QGraphicsGridLayout *toolbarLayout;
};

#endif // QUICKVIEWERTOOLBAR_H
