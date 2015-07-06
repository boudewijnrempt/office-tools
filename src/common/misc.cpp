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

#include <QRectF>
#include <QGraphicsLinearLayout>

#include <MSceneManager>
#include <MLayout>
#include <MFlowLayoutPolicy>
#include <MPannableViewport>
#include <KMimeType>

#include "applicationwindow.h"
#include "misc.h"

QPointF Misc::getRelativePoint(const QPointF &point, const QSizeF &size)
{
    qreal x = 0;
    qreal y = 0;

    if(0 != size.width()) {
        x = point.x() / size.width() ;
    }

    if(0 != size.height()) {
        y =  point.y() / size.height() ;
    }

    return QPointF(x,y);
}

QSizeF Misc::getRelativeSize(const QSizeF &partsize, const QSizeF &size)
{
    qreal width = 0;
    qreal height =  0;

    if(0 != size.width()) {
        width = partsize.width() / size.width();
    }

    if(0 != size.height()) {
        height = partsize.height() / size.height();
    }

    return QSizeF(width, height);
}

QRectF Misc::getRelativeRect(const QRectF &rect, const QSizeF &size)
{
    QPointF point = getRelativePoint(rect.topLeft(), size);
    QSizeF  newsize = getRelativeSize(rect.size(), size);
    return QRectF(point, newsize);
}

QPointF Misc::translateRelativePoint(const QPointF &rpoint, const QSizeF &size)
{
    qreal x = rpoint.x() * size.width();
    qreal y = rpoint.y() * size.height();

    return QPointF(x,y);
}

QSizeF Misc::translateRelativeSize(const QSizeF &rsize, const QSizeF &size)
{
    qreal width = rsize.width() * size.width();
    qreal height = rsize.height() * size.height();
    return QSizeF(width, height);
}

QRectF Misc::translateRelativeRect(const QRectF &rect, const QSizeF &size)
{
    QPointF point = translateRelativePoint(rect.topLeft(), size);
    QSizeF  newsize = translateRelativeSize(rect.size(), size);
    return QRectF(point, newsize);
}

QSizeF Misc::getMaxSizeInGivenSize(const QSizeF &size, const QSizeF &maxSize)
{
    QSizeF newSize;
    qreal limit;

    limit = qMin(maxSize.width()/size.width(), maxSize.height()/size.height());
    newSize.setWidth(size.width() * limit);
    newSize.setHeight(size.height() * limit);

    return newSize;
}

MWidget * Misc::createHorizontalWidget(MWidget * child)
{
    MWidget *widget = 0;

    widget=new MWidget;
    Q_CHECK_PTR(widget);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widget->setContentsMargins(0, 0, 0, 0);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Horizontal);
    Q_CHECK_PTR(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addStretch(1);
    layout->addItem(child);
    layout->addStretch(1);
    widget->setLayout(layout);

    return widget;
}

QString Misc::getFileTypeFromFile(QString filePath)
{
    return qtTrId(getFileTypeFromMime(KMimeType::findByPath(filePath)->name()).toLatin1().data());
}

QString Misc::getFileTypeFromMime(QString mimeType, QString extension)
{
    if((QString::compare("application/msword", mimeType) == 0) ||
       (QString::compare("application/x-mswrite", mimeType) == 0))
        return "qtn_comm_filetype_doc";

    if(QString::compare("text/plain", mimeType) == 0)
        return "qtn_comm_filetype_txt";

    if(QString::compare("application/pdf", mimeType) == 0)
        return "qtn_comm_filetype_pdf";

    if((QString::compare("application/mspowerpoint", mimeType) == 0) ||
       (QString::compare("application/vnd.ms-powerpoint", mimeType) == 0)) {
        if (0 == QString::compare("pps", extension)) {
            return "qtn_comm_filetype_pps";
        } else {
            return "qtn_comm_filetype_ppt";
        }
    }

    if(QString::compare("application/vnd.ms-powerpoint.slideshow.macroEnabled.12", mimeType) == 0)
        return "qtn_comm_filetype_pps";

    if(QString::compare("application/vnd.ms-excel", mimeType) == 0)
        return "qtn_comm_filetype_xls";

    if(QString::compare("application/vnd.oasis.opendocument.text", mimeType) == 0)
        return "qtn_comm_filetype_odt";

    if((QString::compare("application/vnd.oasis.opendocument.presentation", mimeType) == 0) ||
       (QString::compare("application/vnd.oasis.opendocument.presentation-template", mimeType) == 0))
        return "qtn_comm_filetype_odp";

    if(QString::compare("application/vnd.oasis.opendocument.spreadsheet", mimeType) == 0)
        return "qtn_comm_filetype_ods";

    if(QString::compare("application/vnd.openxmlformats-officedocument.wordprocessingml.document", mimeType) == 0)
        return "qtn_comm_filetype_docx";

    if(QString::compare("application/vnd.openxmlformats-officedocument.presentationml.presentation", mimeType) == 0)
        return "qtn_comm_filetype_pptx";

    if(QString::compare("application/vnd.openxmlformats-officedocument.presentationml.slideshow", mimeType) == 0)
        return "qtn_comm_filetype_ppsx";

    if(QString::compare("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", mimeType) == 0)
        return "qtn_comm_filetype_xlsx";

    if(QString::compare("application/rtf", mimeType) == 0)
        return "qtn_comm_filetype_rtf";

    return QString();
}
