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

#ifndef PDFSEARCH_H
#define PDFSEARCH_H

#include <poppler-qt4.h>
#include <QThread>
#include <QHash>

#include "documentviewer_export.h"

class DOCUMENTVIEWER_EXPORT PdfSearch : public QThread
{
    Q_OBJECT

public:
    PdfSearch(const Poppler::Document *document, QHash<int, QList<QRectF> > &results);
    virtual ~PdfSearch();

    void setData(const QString &searchText, int currentPageIndex);

    void run();
    void cancel();

signals:
    void showPage(int pageIndex);
    void searchFinish();

private:
    void search();
    void searchPage(int pageIndex, bool &hit);

    const Poppler::Document   *m_document;
    QHash<int, QList<QRectF> > &m_result;
    QString m_searchText;
    int m_currentPage;
    bool m_canceled;
};

#endif // PDFSEARCH_H
