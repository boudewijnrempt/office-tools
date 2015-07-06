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

#include <QDBusInterface>
#include <QUrl>
#include <QDebug>

#include "applicationwindow.h"
#include "applicationservice.h"
#include "trackerutils.h"

ApplicationService::ApplicationService(const QString &serviceName, const QString &fileName, QObject *parent)
    : MApplicationService(serviceName, parent)
    , filename(fileName)
    , firstInstance(false)
    , applicationPageOpened(false)
    , applicationwindow(0)
    , connection(0)
{
    qDebug() << __PRETTY_FUNCTION__;
}

ApplicationService::~ApplicationService()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void ApplicationService::sendMessage(const QDBusMessage &msg)
{
    if(connection) {
        connection->send(msg);
    }
}

bool ApplicationService::callMethod(const QDBusMessage &msg)
{
    if(applicationwindow &&
       !applicationwindow->firstInstanceRunning()) {
        return false;
    }

    QList<QVariant> arg = msg.arguments();

    if(!firstInstance && connection) {
        connection->call(msg);
    } else if(!arg[0].toString().compare(filename)) {
        launch();
    } else {
        QList<QVariant> arg = msg.arguments();
        applicationwindow->launchFile(arg[0].toString());
    }

    return true;
}

QString ApplicationService::getFilename(QString uri)
{
    bool containsFileUri;

    containsFileUri = uri.startsWith("file://");

    if(containsFileUri) {
        return uri;
    } else {
        return TrackerUtils::Instance().urlFromUrn(uri).toString();
    }
}

void ApplicationService::setApplicationWindow(ApplicationWindow *window, bool showfrontpage)
{
    applicationwindow = window;
    applicationwindow->setApplicationService(this);

    applicationPageOpened = true;
    if((filename != "")) {
        applicationwindow->OpenFile(filename);
    } else if (showfrontpage){
        applicationwindow->OpenListPage();
    } else {
        applicationPageOpened = false;
    }
}

void ApplicationService::launch()
{
    MApplicationService::launch();
}

void ApplicationService::handleServiceRegistrationFailure()
{
    incrementAndRegister();
}

void ApplicationService::lookForPopup(const QString &file)
{
    if (!(file.compare(filename))) {
        launch();
    } else {
        QUrl fileUrl(file);
        if(!(fileUrl.path()).compare(filename)) {
            launch();
        }
    }
}

void ApplicationService::closeFile(const QString &documentUrn)
{
    QString file = getFilename(documentUrn);

    if(!file.isEmpty() &&
       !file.compare(filename) &&
       applicationwindow) {
        applicationwindow->exitApplication();
    }
}

void ApplicationService::showFrontPage()
{
    qDebug() << __PRETTY_FUNCTION__;

    launch();
}

void ApplicationService::setFrontPageLaunched(bool launched)
{
    Q_UNUSED(launched);

    if(connection) {
        // OBS: This is a temporary solution to make sure we are able to listen to the signal from all instances
        connection->connect("com.nokia.office-tools1", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools2", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools3", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools4", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools5", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools6", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools7", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools8", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools9", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
        connection->connect("com.nokia.office-tools110", "/", Interface, QString("showFrontPage"), this, SLOT(showFrontPage()));
    }
}

void ApplicationService::setConnection(QDBusConnection *curConnection, bool isFirstInstance)
{
    connection = curConnection;
    firstInstance = isFirstInstance;
}

void ApplicationService::launch(const QStringList &parameters)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString curFilename = "";
    if(parameters.size() > 0) {
        curFilename = getFilename(parameters.at(0));
    }

    if (applicationwindow) {
        if (curFilename != filename) {
            if (!applicationPageOpened) {
                applicationPageOpened = true;
                filename = curFilename;
            }
            applicationwindow->launchFile(curFilename);
            return;
        } else if (curFilename.isEmpty()) {
            applicationPageOpened = true;
            applicationwindow->OpenListPage();
        }
    }

    MApplicationService::launch(parameters);
}
