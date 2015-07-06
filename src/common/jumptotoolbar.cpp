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
#include <QGraphicsLinearLayout>
#include <QRegExpValidator>
#include <QPropertyAnimation>
#include <QDebug>

#include <MTextEdit>
#include <MButton>
#include <MInputMethodState>
#include <MApplication>
#include <MApplicationWindow>

#include <float.h>

#include "jumptotoolbar.h"
#include "definitions.h"
#include "documentpage.h"

JumpToPageEventFilter::JumpToPageEventFilter(QObject *parent)
: QObject(parent)
{
}

JumpToPageEventFilter::~JumpToPageEventFilter()
{
}

bool JumpToPageEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    qDebug() << __PRETTY_FUNCTION__ << event << event->type();
    // filter out + and -
    if (event->type() == QEvent::InputMethod) {
        QInputMethodEvent *imEvent = dynamic_cast<QInputMethodEvent *>(event);
        qDebug() << __PRETTY_FUNCTION__ << imEvent;
        if (imEvent) {
            qDebug() << __PRETTY_FUNCTION__ << imEvent->attributes().size() << imEvent->commitString() << imEvent->preeditString();
        }
        return imEvent && (imEvent->commitString().startsWith('-') || imEvent->commitString().startsWith('+') || imEvent->commitString().startsWith('.') ||
                           imEvent->preeditString().startsWith('-') || imEvent->preeditString().startsWith('+') || imEvent->preeditString().startsWith('.'));
        //return keyEvent && (keyEvent->key() == Qt::Key_Plus || keyEvent->key() == Qt::Key_Minus);
    }
    return false;
}


JumpToToolbar::JumpToToolbar(DocumentPage *parent)
    : MOverlay(parent)
    , m_edit(0)
    , m_page(parent)
    , m_registerdAttributeExtension(-1)
    , m_animation(new QPropertyAnimation(this, "paintOffset", this))
{
    m_animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(m_animation, SIGNAL(finished()), this, SLOT(slotAnimationCompleted()));

    m_registerdAttributeExtension = MInputMethodState::instance()->registerAttributeExtension();
    MInputMethodState::instance()->setExtendedAttribute(m_registerdAttributeExtension, "/keys", "actionKey", "label", qtTrId("qtn_offi_go"));

    setStyleName(JUMPTOTOOLBAR);


    m_edit = new MTextEdit(MTextEditModel::SingleLine, QString(), this);
    m_edit->setPrompt(QString(qtTrId("qtn_offi_jump_page_number").arg(QString::number(1)).arg(QString::number(1))));
    m_edit->setContentType(M::NumberContentType);
    m_edit->setMaxLength(6);
    m_edit->setStyleName("CommonSingleInputFieldInverted");
    m_edit->installEventFilter(new JumpToPageEventFilter(this));
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(goToPage()));
    m_edit->attachToolbar(m_registerdAttributeExtension);

    QGraphicsLinearLayout *overlayLayout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->addItem(m_edit);
    overlayLayout->setAlignment(m_edit, Qt::AlignVCenter);

    setLayout(overlayLayout);
    setZValue(FLT_MAX);
}

void JumpToToolbar::hide()
{
    m_page->hideInfoBanner();
    m_edit->clearFocus();
    startAnimation(0, -maximumSize().height());
    m_edit->clear();
}

void JumpToToolbar::show()
{
    MOverlay::show();
    m_edit->setPrompt(QString(qtTrId("qtn_offi_jump_page_number")).arg(QString::number(1)).arg(QString::number(m_page->pageCount())));
    startAnimation(-maximumSize().height(), 0);
    m_edit->setFocus();
}

void JumpToToolbar::goToPage()
{
    if (!m_edit->text().isEmpty()) {
        bool ok = false;
        int pageIndex = m_edit->text().toInt(&ok);

        if (ok) {
            if (m_page->showPageIndex(pageIndex-1)) {
                hide();
                m_page->hidePageIndicator();
            }
        }
    }

}

void JumpToToolbar::textChanged()
{
    m_page->hideInfoBanner();
    MInputMethodState::instance()->setExtendedAttribute(m_registerdAttributeExtension, "/keys", "actionKey", "highlighted", QVariant(!m_edit->text().isEmpty()));
}

void JumpToToolbar::slotAnimationCompleted()
{
    if (paintOffset() != QPointF(0, 0)) {
        MOverlay::hide();
    }
}

void JumpToToolbar::startAnimation(int startValue, int endValue)
{
    qDebug() << __PRETTY_FUNCTION__ << startValue << endValue;
    m_animation->stop();
    m_animation->setStartValue(QPointF(0, startValue));
    m_animation->setEndValue(QPointF(0, endValue));
    m_animation->setDuration(500);
    m_animation->start();
}
