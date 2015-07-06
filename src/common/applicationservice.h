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

#ifndef APPLICATION_SERVICE_H
#define APPLICATION_SERVICE_H

#include <MApplicationService>

class ApplicationWindow;
class QDBusInterface;
class QDBusMessage;
class QDBusConnection;

#include <common_export.h>

namespace
{
const QString Service("com.nokia.OfficeToolsService");
const QString Path("/");
const QString Interface("com.nokia.maemo.meegotouch.OfficeToolsInterface");
}

class COMMON_EXPORT ApplicationService : public MApplicationService
{
    Q_OBJECT

public:
    ApplicationService(const QString &serviceName, const QString &fileName, QObject *parent = 0);
    virtual ~ApplicationService();
    void setApplicationWindow(ApplicationWindow *window,bool showfrontpage);
    void setConnection(QDBusConnection *curConnection, bool isFirstInstance);
    void sendMessage(const QDBusMessage &msg);
    bool callMethod(const QDBusMessage &msg);
    void setFrontPageLaunched(bool launched);
    void setFileName(const QString &fileName) {  filename = fileName; }

public Q_SLOTS: // METHODS

    virtual void launch(const QStringList &parameters);
    virtual void launch();
    virtual void handleServiceRegistrationFailure();
    void lookForPopup(const QString &file);
    void closeFile(const QString &documentUrn);
    void showFrontPage();

protected:
    QString getFilename(QString uri);
    QString filename;
    bool firstInstance;
    bool applicationPageOpened;
    ApplicationWindow *applicationwindow;
    QDBusConnection *connection;
};

#endif
