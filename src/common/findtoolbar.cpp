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
#include <MGridLayoutPolicy>

#include <QClipboard>
#include <QDebug>
#include <QPropertyAnimation>
#include <QGraphicsLinearLayout>

#include <MApplication>
#include <MTextEdit>
#include <MStylableWidget>
#include <MImageWidget>
#include <MButton>
#include <MInputMethodState>
#include <MLayout>
#include <MLinearLayoutPolicy>
#include <MProgressIndicator>

#include <float.h>
#include "applicationwindow.h"
#include "findtoolbar.h"
#include "definitions.h"

FindToolbar::FindToolbar(QGraphicsItem *parent)
    : MOverlay(parent)
    , m_edit(0)
    , m_iconLayout(0)
    , m_clear(0)
    , m_magnifier(0)
    , m_searchIndicator(0)
    , m_next(0)
    , m_previous(0)
    , m_registerdAttributeExtension(-1)
    , m_matchFound(false)
    , m_animation(new QPropertyAnimation(this, "paintOffset", this))
{
    m_animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(m_animation, SIGNAL(finished()), this, SLOT(slotAnimationCompleted()));

    m_registerdAttributeExtension = MInputMethodState::instance()->registerAttributeExtension();
    MInputMethodState::instance()->setExtendedAttribute(m_registerdAttributeExtension, "/keys", "actionKey", "label", qtTrId("qtn_comm_command_done"));

    MLayout *layout = new MLayout();
    MLinearLayoutPolicy *policy = new MLinearLayoutPolicy(layout, Qt::Horizontal);
    policy->setContentsMargins(0, 0, 0, 0);
    policy->setSpacing(0);
    setStyleName(FINDTOOLBAR);

    MStylableWidget *searchBar = new MStylableWidget(this);
    searchBar->setStyleName("searchBar");
    searchBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    MStylableWidget *inputBar = new MStylableWidget(searchBar);
    inputBar->setStyleName("inputBar");
    inputBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QGraphicsLinearLayout *inputLayout = new QGraphicsLinearLayout(Qt::Horizontal, inputBar);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(0);

    m_edit = new MTextEdit(MTextEditModel::SingleLine, QString(), inputBar);
    m_edit->setPrompt(qtTrId("qtn_comm_search"));
    m_edit->setStyleName("CommonSingleInputFieldInverted");
    m_edit->attachToolbar("/usr/share/meegoimframework/imtoolbars/xulrunner.xml");
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_edit->attachToolbar(m_registerdAttributeExtension);
    inputLayout->addItem(m_edit);
    inputBar->setLayout(inputLayout);
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));

    MStylableWidget *iconBar = new MStylableWidget(searchBar);
    iconBar->setStyleName("iconBar");
    iconBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_iconLayout = new QGraphicsLinearLayout(Qt::Horizontal, iconBar);
    m_iconLayout->setContentsMargins(0, 0, 0, 0);
    m_iconLayout->setSpacing(0);

    m_magnifier = new MImageWidget("icon-m-common-search", iconBar);
    m_magnifier->setStyleName("searchMagnifier");

    m_clear = new MButton(iconBar);
    m_clear->setViewType(MButton::iconType);
    m_clear->setStyleName("searchClear");
    m_clear->setIconID(qtTrId("icon-m-input-clear"));
    m_clear->setVisible(false);
    connect(m_clear, SIGNAL(clicked()), this, SLOT(slotClear()));

    m_searchIndicator = new MProgressIndicator(iconBar);
    m_searchIndicator->setStyleName("CommonViewHeaderSpinner");
    m_searchIndicator->setVisible(false);

    m_iconLayout->addStretch();
    m_iconLayout->addItem(m_magnifier);
    iconBar->setLayout(m_iconLayout);

    policy->addItem(searchBar, Qt::AlignCenter);

    m_previous = new MButton(qtTrId(""), this);
    m_previous->setIconID(qtTrId("icon-m-toolbar-previous-white"));
    m_previous->setStyleName("findButton");
    m_previous->setFocusPolicy(Qt::NoFocus);
    policy->addItem(m_previous, Qt::AlignCenter);

    m_next = new MButton(qtTrId(""), this);
    m_next->setIconID(qtTrId("icon-m-toolbar-next-white"));
    m_next->setStyleName("findButton");
    m_next->setFocusPolicy(Qt::NoFocus);
    policy->addItem(m_next, Qt::AlignCenter);

    layout->setPolicy(policy);

    setLayout(layout);

    setZValue(FLT_MAX);

    connect(m_previous, SIGNAL(clicked()), this, SLOT(slotFindPrevious()));
    connect(m_next, SIGNAL(clicked()), this, SLOT(slotFindNext()));
}

void FindToolbar::setMatchFound(bool found)
{
    m_edit->setErrorHighlight(!found);
    m_matchFound = found;
    MInputMethodState::instance()->setExtendedAttribute(m_registerdAttributeExtension, "/keys", "actionKey", "highlighted", QVariant(found));

    if (text().size() > 0) {
        setItem(m_clear);
    }
    else {
        setItem(m_magnifier);
    }
}

bool FindToolbar::matchFound() const
{
    return m_matchFound;
}

QString FindToolbar::text() const
{
    return m_edit->text();
}

void FindToolbar::setText(const QString &text)
{
    m_edit->setText(text);
    m_edit->setFocus();
}

bool FindToolbar::hideVkb()
{
    bool retval = !MInputMethodState::instance()->inputMethodArea().isNull();
    if (retval) {
        m_edit->clearFocus();
    }
    return retval;
}

void FindToolbar::hide()
{
    m_edit->clearFocus();
    m_matchFound = false;
    startAnimation(0, -maximumSize().height());
    // keep the old search string
    // uncomment if we don't want to keep the search string between closing and opening the dialog
    //m_edit->clear();
}

void FindToolbar::show()
{
    qDebug() << __PRETTY_FUNCTION__ << size() << maximumSize();
    MOverlay::show();
    startAnimation(-maximumSize().height(), 0);
    m_edit->setFocus();

    if (m_edit->text().size() > 0) {
        textChanged();
    }
}

void FindToolbar::returnPressed()
{
    if (matchFound()) {
        hideVkb();
    }
}

void FindToolbar::textChanged()
{
    MInputMethodState::instance()->setExtendedAttribute(m_registerdAttributeExtension, "/keys", "actionKey", "highlighted", QVariant(false));
    m_edit->setErrorHighlight(false);
    m_matchFound = false;
    if (text().size() > 0) {
        qDebug() << __PRETTY_FUNCTION__ << "set clear";
        setItem(m_clear);
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << "set magnifier";
        setItem(m_magnifier);
    }
    emit findFirst();
}

void FindToolbar::slotFindNext()
{
    if (matchFound()) {
        emit findNext();
    }
}

void FindToolbar::slotFindPrevious()
{
    if (matchFound()) {
        emit findPrevious();
    }
}

void FindToolbar::slotAnimationCompleted()
{
    if (paintOffset() != QPointF(0, 0)) {
        MOverlay::hide();
    }
}

void FindToolbar::slotClear()
{
    setText("");
}

void FindToolbar::startAnimation(int startValue, int endValue)
{
    qDebug() << __PRETTY_FUNCTION__ << startValue << endValue;
    m_animation->stop();
    m_animation->setStartValue(QPointF(0, startValue));
    m_animation->setEndValue(QPointF(0, endValue));
    m_animation->setDuration(500);
    m_animation->start();
}

void FindToolbar::setItem(MWidget *item)
{
    MWidget *current = dynamic_cast<MWidget *>(m_iconLayout->itemAt(0));
    if (current && current != item) {
        m_searchIndicator->setUnknownDuration(item == m_searchIndicator);
        m_iconLayout->removeItem(current);
        m_iconLayout->addItem(item);
        m_iconLayout->activate();
        item->setVisible(true);
        current->setVisible(false);
    }
}

void FindToolbar::showSearchIndicator()
{
    setItem(m_searchIndicator);
}

bool FindToolbar::isActive() const
{
    return isVisible() && !(m_animation->state() == QAbstractAnimation::Running && m_animation->endValue() != QPointF(0, 0));
}
