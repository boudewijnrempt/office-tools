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

#ifndef PAGEINDICATOR_H
#define PAGEINDICATOR_H

#include <QTimer>
#include <MLabel>
#include <MOverlay>

class QGraphicsSceneMouseEvent;
class QPropertyAnimation;
class QGraphicsGridLayout;

#include <common_export.h>

/*!
 * \class PageIndicator
 * \brief The page indicator UI class that shows current and total page amount
 * The page indicator is visible only for short time after current or total page is changed.
 */

class COMMON_EXPORT PageIndicator : public MOverlay
{
    Q_OBJECT
    Q_PROPERTY(int barOffset READ topBarOffset WRITE setTopBarOffset)

public:

    PageIndicator(const QString &documentName, QGraphicsItem *parent = 0);
    virtual ~PageIndicator();

    void setTopBarOffset(int offset);
    int topBarOffset() const;

    void setFileName(const QString &name);
    void setSheetName(const QString &name);

public slots:

    /*!
     * \brief Show current page and total amount of pages in the page indicator.
     * Note that if currentpage is less then one it hides page indicator.
     * \param total is the total amount of pages in a document
     * \param currentpage is the currently viewed page in a document
     */
    virtual void setPageCounters(int total, int currentpage);

    /*!
     * \brief Show the page indicator.
     * Page indicator will be visible for certain time.
     */
    void show();

    void hide();

private slots:
    /*!
     * \brief This slot is used for updating possition of the indicator.
     */
    void updatePosition(const M::Orientation &orientation);

    /*!
     * \brief This slot hides indicator after timeout.
     */
    void timeoutHide();

private:
    void startAnimation(int startValue, int endValue);

    QTimer              timer;
    MLabel              *fileNameLabel;
    int                 totalPages;
    int                 currentPage;
    MLabel              *pageNumberLabel;
    MLabel              *sheetNameLabel;
    QPropertyAnimation  *m_animation;
    int                 barOffset;
    bool                spreadsheetToolbar;
    QGraphicsGridLayout *topBarLayout;
};

#endif // PAGEINDICATOR_H

