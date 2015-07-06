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

#include <QDebug>

#include "pdfsearch.h"

#include <sys/time.h>

PdfSearch::PdfSearch(const Poppler::Document *document, QHash<int, QList<QRectF> > &results)
    : m_document(document)
    , m_result(results)
    , m_currentPage(0)
    , m_canceled(false)
{
    setTerminationEnabled(true);
}

PdfSearch::~PdfSearch()
{
}

void PdfSearch::setData(const QString &searchText, int currentPageIndex)
{
    m_searchText = searchText;
    m_currentPage = currentPageIndex;
}

void PdfSearch::run()
{
    m_canceled = false;

    search();

    emit searchFinish();

    exec();
}

void PdfSearch::cancel()
{
    m_canceled = true;
}

void PdfSearch::search()
{
    qDebug() << "search";
    int pageIndex = m_currentPage;
    bool signalEmitted = false;
    bool hit = false;

    while(!m_canceled && pageIndex < m_document->numPages()) {
        qDebug() << __PRETTY_FUNCTION__ << "search page" << pageIndex;
        searchPage(pageIndex, hit);

        if ((false == signalEmitted) && (true == hit)) {
            signalEmitted = true;
            emit showPage(pageIndex);
        }

        pageIndex += 1;
    }

    //We reach the end of the document, we search from the beginning until we reach the current page.
    pageIndex = 0;

    while(!m_canceled && pageIndex < m_currentPage) {
        qDebug() << __PRETTY_FUNCTION__ << "search page" << pageIndex;
        searchPage(pageIndex, hit);

        if((false == signalEmitted) && (true == hit)) {
            signalEmitted = true;
            emit showPage(pageIndex);
            qDebug() << __PRETTY_FUNCTION__ << "showPage";
        }

        pageIndex += 1;
    }
}



void PdfSearch::searchPage(int pageIndex, bool &hit)
{
    double top = 0;
    double bottom = 0;
    double right = 0;
    double left = 0;
    Poppler::Page *page = m_document->page(pageIndex);

    while(!m_canceled && page->search(m_searchText, left, top, right, bottom, Poppler::Page::NextResult, Poppler::Page::CaseInsensitive)) {
        QRectF searchHit(QPointF(left, top), QPointF(right, bottom));
        qDebug() << "**********Page:" << pageIndex+1 << "qrect:" <<searchHit << "searchText:" << m_searchText;

        //Append the co-ordinates
        m_result[pageIndex].append(searchHit);

        hit = true;
    }

    delete page;
}
