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

#ifndef OFFICEPAGE_H
#define OFFICEPAGE_H

#include "documentpage.h"
#include "documentviewer_export.h"
class OfficeInterface;

class OfficeViewer;

class OfficePagePrivateData;

/*!
 * \class OfficePage
 * \brief OfficePage is a #DocumentPage class that provides viewing of KOffice documents.
 */

class DOCUMENTVIEWER_EXPORT OfficePage : public DocumentPage
{
    Q_OBJECT

signals:
    void showPage(int pageIndex);

public:
    OfficePage(const QString& document);
    virtual ~OfficePage();

    void loadDocument();
    void createContent();
    ThumbProvider* getThumbProvider();

public slots:

    void zoom(ZoomLevel level);
    void searchText(DocumentPage::SearchMode mode, const QString &searchText);
    void clearSearchTexts();
    void createKoWidget(bool status);
    void timedPinchFinished();
    void shortTap(const QPointF &point, QObject *object);
    virtual void setOpeningProgress(int value);
protected slots:

    void openPlugin(OfficeInterface *plugin);

protected:
    virtual void showPageIndexInternal(int pageIndex);

    virtual void pinchStarted(QPointF &center);
    virtual qreal pinchUpdated(qreal zoomFactor);
    virtual void pinchFinished(const QPointF &center, qreal scale);
    virtual QGraphicsWidget *pinchWidget();

private:
    OfficePagePrivateData * const data;
};

#endif // OFFICEPAGE_H

