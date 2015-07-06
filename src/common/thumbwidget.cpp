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

#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneResizeEvent>
#include <QDebug>

#include <MLabel>
#include <MLocale>
#include <MSceneManager>
#include <MWidgetStyle>
#include <MProgressIndicator>

#include "thumbwidget.h"
#include "thumbprovider.h"
#include "definitions.h"
#include "officethumbprovider.h"

ThumbWidget::ThumbWidget(ThumbProvider *thumbProvider, bool thumbForSpreadSheet, QGraphicsItem *parent)
    : MWidgetController(parent)
    , BasePageWidget()
    , thumbProvider(thumbProvider)
    , label(0)
    , m_spinner(0)
{
    label = new MLabel(this);
    label->setPos(0,0);
    label->setAlignment(Qt::AlignCenter);
    isSpreadsheet = thumbForSpreadSheet;
}

ThumbWidget::~ThumbWidget()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void ThumbWidget::setPageIndex(int newPageIndex, int pageCount)
{
    Q_UNUSED(pageCount);
    BasePageWidget::setPageIndex(newPageIndex);

    MLocale myLocale; // get the current locale
    QString formattedCurrentPage = myLocale.formatNumber(pageIndex+1);

    if(!isSpreadsheet) {
        QString indicator = QString("%L1").arg(formattedCurrentPage);

        label->setText(indicator);
    } else {
        OfficeThumbProvider *spreadsheetThumbProvider = dynamic_cast<OfficeThumbProvider *>(thumbProvider);

        if(spreadsheetThumbProvider) {
            QString sheetName = spreadsheetThumbProvider->spreadsheetSheetName(pageIndex);
            QString indicator = QString("%1.%2").arg(formattedCurrentPage).arg(sheetName);

            label->setText(indicator);
        }
    }
}

void ThumbWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    qDebug() << "in Paint method ---------------" << pageIndex;
    currentpageIndex = thumbProvider->getPageIndex();
    QPen pen = painter->pen();
    pen.setWidth(2);
    QString objName = QString("AllPages%1Columns").arg(columnCnt);
    if(pageIndex+1 == currentpageIndex) {
        setHighlightNames(objName);
    } else {
        setNames(objName);
    }

    pen.setColor(style()->backgroundColor());

    //Lets paint the visible page areas
    QRectF rect(QPointF(0, 1), QSizeF(requestedSize));
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
    thumbProvider->paint(painter, option, this);

}

void ThumbWidget::setSize(const QSizeF newsize,int nCols)
{
    if (requestedSize != newsize) {
        requestedSize = newsize;
        requestedSize.rheight() = requestedSize.height() - 1;
        columnCnt = nCols;
        updateGeometry();
    }
}

QSizeF ThumbWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return requestedSize;
}

void ThumbWidget::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    if(!isSpreadsheet) {
        //Move label into top right corner
        label->setPos(event->newSize().width() - label->size().width(), 0);
    }
}

MWidgetStyleContainer& ThumbWidget::style()
{
    return MWidgetController::style();
}

void ThumbWidget::setNames(QString prefix)
{
    if(isSpreadsheet) {
        label->setObjectName(prefix + "LabelSpreadsheet");
    } else {
        label->setObjectName(prefix + "Label");
    }

    setObjectName(prefix + "Widget");
}

void ThumbWidget::setHighlightNames(QString prefix)
{
    if(isSpreadsheet) {
        label->setObjectName(prefix + "LabelHighlightSpreadsheet");
    } else {
        label->setObjectName(prefix + "LabelHighlight");
    }

    setObjectName(prefix + "Widget");
}

void ThumbWidget::startSpinner(const QPointF &pos)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!m_spinner) {
        m_spinner = new MProgressIndicator(this, MProgressIndicator::spinnerType);
        m_spinner->setStyleName("CommonThumbnailSpinner");
    }
    m_spinner->setUnknownDuration(true);
    m_spinner->setPos(pos - QPointF(m_spinner->size().width()/2, m_spinner->size().height()/2));
}

void ThumbWidget::stopSpinner()
{
    if (m_spinner) {
        m_spinner->reset();
        delete m_spinner;
        m_spinner = 0;
    }
}
