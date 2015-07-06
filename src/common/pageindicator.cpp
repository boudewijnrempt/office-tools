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

#include <QTimer>
#include <QTimer>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsGridLayout>
#include <QPropertyAnimation>

#include <MSceneManager>
#include <MLocale>
#include <MLabel>
#include <MApplication>
#include <MInputMethodState>

#include "definitions.h"
#include "pageindicator.h"
#include "applicationwindow.h"
#include "actionpool.h"

PageIndicator::PageIndicator(const QString &documentName, QGraphicsItem *parent)
    : MOverlay(parent)
    , totalPages(0)
    , currentPage(0)
    , spreadsheetToolbar(false)
{
    m_animation = new QPropertyAnimation(this, "barOffset");
    connect(&timer, SIGNAL(timeout()), this, SLOT(timeoutHide()));
    connect(ApplicationWindow::GetSceneManager(), SIGNAL(orientationChanged(const M::Orientation &)),
            this, SLOT(updatePosition(const M::Orientation &)));
    setStyleName("TopToolBarDocument");
    fileNameLabel = new MLabel(documentName, this);
    fileNameLabel->setStyleName("CommonTitleInverted");
    fileNameLabel->setTextElide(true);
    fileNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pageNumberLabel = new MLabel(this);
    pageNumberLabel->setStyleName("CommonItemInfoInverted");
    pageNumberLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    sheetNameLabel = new MLabel(this);
    sheetNameLabel->setStyleName("CommonSubTitleInverted");

    topBarLayout = new QGraphicsGridLayout(this);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->addItem(fileNameLabel, 0, 0, 1, 1, Qt::AlignLeft);
    topBarLayout->addItem(pageNumberLabel, 0, 1, 1, 1, Qt::AlignRight);
    setLayout(topBarLayout);

    //Lets make Page Indicator on top of other child items
    setZValue(ZValuePageIndicator);
    updatePosition(ApplicationWindow::GetSceneManager()->orientation());

    setPos(0, 0);
    setVisible(false);
}

PageIndicator::~PageIndicator()
{
    delete m_animation;
    m_animation = 0;
}

void PageIndicator::timeoutHide()
{
    setVisible(false);
}

void PageIndicator::updatePosition(const M::Orientation &orientation)
{
    Q_UNUSED(orientation);
    MWindow *win = MApplication::activeWindow();
    QSize size = win->visibleSceneSize();
    resize(size.width(), preferredSize().height());
}

void PageIndicator::setPageCounters(int total, int page)
{
    if(totalPages == total && currentPage == page) {
        return;
    }

    totalPages = total;

    currentPage = page;

    if(1 <= currentPage) {
        MLocale myLocale; // get the current locale
        QString formattedPageAmount = myLocale.formatNumber(totalPages);
        QString formattedCurrentPage = myLocale.formatNumber(currentPage);

        QString indicator = QString(qtTrId("%L1/%L2")).arg(formattedCurrentPage).arg(formattedPageAmount);
        pageNumberLabel->setText(indicator);

        if(fileNameLabel->text().isEmpty()) {
            setVisible(false);
        }
    } else {
        setVisible(false);
    }
}

void PageIndicator::setFileName(const QString &name)
{
    fileNameLabel->setText(name);
}

void PageIndicator::setSheetName(const QString &name)
{
    if ( !spreadsheetToolbar ) {
        topBarLayout->removeAt(1);
        topBarLayout->addItem(sheetNameLabel, 1, 0, 1, 1, Qt::AlignLeft);
        topBarLayout->addItem(pageNumberLabel, 1, 1, 1, 1, Qt::AlignRight);
        spreadsheetToolbar = true;
        //setPos(0, 30);
        setStyleName("TopToolBarSpreadsheet");
    }

    sheetNameLabel->setText(name);
}

void PageIndicator::show()
{
    DocumentPage *page = static_cast<DocumentPage *>(parentWidget());
    if (page && page->isQuickViewer()) {
        return;
    }

    startAnimation(-geometry().height(), 0);
    setVisible(true);
}

void PageIndicator::hide()
{
    startAnimation(0, -geometry().height());
    timer.setSingleShot(true);
    timer.start(100);
}

void PageIndicator::setTopBarOffset(int offset)
{
    barOffset = offset;
    setPos(0, offset);
}

int PageIndicator::topBarOffset() const
{
    return barOffset;
}

void PageIndicator::startAnimation(int startValue, int endValue)
{
    m_animation->stop();
    m_animation->setStartValue(startValue);
    m_animation->setEndValue(endValue);
    m_animation->setDuration(100);
    m_animation->start();
}
