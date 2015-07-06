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

#include "documentdetailview.h"
#include "misc.h"
#include "trackerutils.h"

#include <QGraphicsLinearLayout>
#include <QFileInfo>
#include <QUrl>

#include <MContainer>
#include <MGridLayoutPolicy>
#include <MLayout>
#include <MLabel>
#include <MLocale>
#include <MBasicListItem>
#include <MImageWidget>
#include <MSeparator>

#define GB (1024*1024*1024)
#define MB (1024*1024)
#define KB 1024

DocumentDetailView::DocumentDetailView(QString path)
{
    m_documentPath = QUrl(QUrl::fromPercentEncoding(path.toUtf8())).path();
    m_pixmapsLoaded = false;
}

DocumentDetailView::~DocumentDetailView()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void DocumentDetailView::createContent()
{
    MApplicationPage::createContent();
    setTitle(qtTrId("qtn_offi_details_vm_details"));
    setObjectName("documentdetailview");
    connect(MTheme::instance(),SIGNAL(pixmapRequestsFinished()), this, SLOT(pixmapLoaded()));
    parseDocumentDetail();

}

void DocumentDetailView::pixmapLoaded()
{
    if(!m_pixmapsLoaded) {
        update();

        if(MTheme::hasPendingRequests()) {
            qWarning() << "some pending requests remain to Load pixmap";
        } else {
            m_pixmapsLoaded = true;
            disconnect(MTheme::instance(),SIGNAL(pixmapRequestsFinished()), this, SLOT(pixmapLoaded()));
        }
    }
}

void DocumentDetailView::parseDocumentDetail()
{
    QString filePath="file://";
    (m_documentPath.contains(filePath)) ? (filePath = m_documentPath) : (filePath.append(m_documentPath));
    QFileInfo fileInfo(m_documentPath);
    m_fileTitle = fileInfo.completeBaseName();

    DocumentDetails *details = TrackerUtils::Instance().documentDetailsFromUrl(filePath);
    if (details) {
        MLocale locale;
        m_downloadDate = locale.formatDateTime(details->created, MLocale::DateShort, MLocale::TimeShort);

        m_subject = details->subject;

        calculateByteSize(details->size);

        m_lastAccessedDate = locale.formatDateTime(details->lastAccessed, MLocale::DateShort, MLocale::TimeShort);

        if (!details->author.isNull()) {
            m_authorName = details->author;
        } else if (!details->publisher.isNull()) {
            m_authorName = details->publisher;
        }

        m_mimeType = qtTrId(Misc::getFileTypeFromMime(details->mimeType, fileInfo.suffix()).toLatin1().data());
        delete details;
    }

    loadDocumentDetails();
}

void DocumentDetailView::calculateByteSize(double fileSize)
{
    double Kbytes = (fileSize < KB) ? 1 : ((fileSize)/KB);
    double Mbytes = (Kbytes < KB) ? Kbytes : ((Kbytes)/KB);
    double Gbytes = (Mbytes < KB) ? Mbytes : ((Mbytes)/KB);

    m_strByteSize = MLocale::createSystemMLocale()->formatNumber(Gbytes);
    m_strByteSize.remove(4,10);
    if (!m_strByteSize.at(m_strByteSize.count()-1).isNumber()) {
        m_strByteSize.remove(m_strByteSize.count()-1, 1);
    }
    QString byte;
    byte = (Gbytes == Mbytes) ? ((Mbytes == Kbytes) ? qtTrId("qtn_comm_kilobytes") : qtTrId("qtn_comm_megabytes")) : qtTrId("qtn_comm_gigabytes");
    m_strByteSize = QString(byte).arg(m_strByteSize);
}

void DocumentDetailView::setDocumentIcon(MBasicListItem *contentItem, const QString &mime_type)
{
    if((QString::compare(qtTrId("qtn_comm_filetype_doc"), mime_type) == 0) ||
       (QString::compare(qtTrId("qtn_comm_filetype_rtf"), mime_type) == 0) ||
       (QString::compare(qtTrId("qtn_comm_filetype_docx"), mime_type) == 0))
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-word", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_odt"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-open-document-text", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_txt"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-text", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_pdf"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-pdf", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_ppt"), mime_type) == 0 ||
            QString::compare(qtTrId("qtn_comm_filetype_pps"), mime_type) == 0 ||
            QString::compare(qtTrId("qtn_comm_filetype_pptx"), mime_type) == 0 ||
            QString::compare(qtTrId("qtn_comm_filetype_ppsx"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-powerpoint", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_odp"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-open-document-presentation", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_xls"), mime_type) == 0 ||
            QString::compare(qtTrId("qtn_comm_filetype_xlsx"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-excel", QSize(88, 88))));
    else if(QString::compare(qtTrId("qtn_comm_filetype_ods"), mime_type) == 0)
        contentItem->imageWidget()->setPixmap(*(MTheme::pixmap("icon-m-content-open-document-spreadsheet", QSize(88, 88))));

    m_pixmapsLoaded = !MTheme::hasPendingRequests();
}

void DocumentDetailView::loadDocumentDetails()
{
    QGraphicsWidget *panel = centralWidget();
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    panel->setLayout(layout);

    MBasicListItem *contentItem = new MBasicListItem(MBasicListItem::IconWithTitleAndSubtitle, panel);
    Q_ASSERT(contentItem);

    setDocumentIcon(contentItem, m_mimeType);
    contentItem->setTitle(m_fileTitle);
    contentItem->setSubtitle(m_mimeType);
    layout->addItem(contentItem);

    layout->addItem(new MSeparator(panel));

    MLabel *detailsLabel = new MLabel(panel);
    detailsLabel->setWordWrap(true);
    detailsLabel->setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    detailsLabel->setAlignment(Qt::AlignTop);
    detailsLabel->setStyleName("CommonBodyText");
    QString detailsText;

    if (!m_authorName.isEmpty()) {
        detailsText = qtTrId("qtn_offi_details_vm_author").split(QChar(0x9c)).first() + " " + m_authorName + "\n";
    }

    if (!m_subject.isEmpty()) {
        detailsText += qtTrId("qtn_offi_details_vm_description").split(QChar(0x9c)).first() + " " + m_subject + "\n";
    }

    detailsText += qtTrId("qtn_offi_details_view_size").split(QChar(0x9c)).first() + " " + m_strByteSize + "\n";

    detailsText += qtTrId("qtn_offi_details_downloaded").split(QChar(0x9c)).first() + "\n" + "\t" + m_downloadDate + "\n";

    if (0 != QString::compare(m_downloadDate,m_lastAccessedDate)) {
        detailsText += qtTrId("qtn_offi_details_last_accessed").split(QChar(0x9c)).first() + "\n" + "\t" + m_lastAccessedDate + "\n";
    }

    detailsLabel->setText(detailsText);
    layout->addItem(detailsLabel);
}

