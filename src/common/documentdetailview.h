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

#ifndef DOCUMENTDETAILVIEW_H
#define DOCUMENTDETAILVIEW_H

#include "documentlistpage.h"

#include <common_export.h>

class MBasicListItem;

class COMMON_EXPORT DocumentDetailView: public MApplicationPage
{
    Q_OBJECT

public:
    DocumentDetailView(QString documentName);
    virtual ~DocumentDetailView();
    virtual void createContent();

private:
    void loadDocumentDetails();
    void parseDocumentDetail();
    void getAuthorName();
    void calculateByteSize(double filesize);
    QString documentFormat();
    void setDocumentIcon(MBasicListItem*, const QString&);

    bool m_pixmapsLoaded;
    QString m_fileTitle;
    QString m_documentPath;
    QString m_downloadDate;
    QString m_authorName;
    QString m_subject;
    QString m_mimeType;

    QString m_strByteSize;
    QString m_lastAccessedDate;

private slots:
    void pixmapLoaded();

};

#endif
