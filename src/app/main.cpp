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

#include <iostream>

#include <QFile>
#include <QGestureRecognizer>

#include <MApplication>
#include <MTheme>
#include <MLocale>
#include <MExport>
#include <MComponentCache>

#include "officetoolsserviceifadaptor.h"
#include "applicationwindow.h"
#include "applicationservice.h"

static QString parse_filename(int argc, char** argv)
{
    const QRegExp r(".*[.](pdf|odt|doc|docx|docm|txt|rtf|dotx|dotm|dot|ott|odp|ppt|pptx|pptm|pps|ppsx|ppsm|ppam|otp|pot|potx|potm|ods|xls|xlsx|xlsm|xlsb|xlam|ots|xlt|xltx|xltm)");

    for(int i=1; i < argc; ++i) {
        if(r.exactMatch(argv[i])) {
            return argv[i];
        }
    }

    return QString();
}

static bool frontpage_in_args(int argc, char** argv)
{
    QString strfp("-showfrontpage");
    for(int i=1; i < argc; ++i) {
        if(strfp == argv[i]) {
            return true;
        }
    }
    return false;
}

#if 0
static void message_handler(QtMsgType type, const char *msg)
{
    FILE *filep;
    filep = fopen ("/home/user/MyDocs/office.log","a");
    if (NULL != msg) {
        const char * temp="??????? : ";

        switch (type) {

        case QtDebugMsg:
            temp = "Debug: ";
            std::cout << temp<<  msg << "\n";
            break;

        case QtWarningMsg:
            temp = "Warning: ";
            break;

        case QtCriticalMsg:
            temp = "Critical: ";
            break;

        case QtFatalMsg:
            temp = "Fatal: ";
            break;
        }

        fprintf(filep, "%d - %s - %s\n",getpid(),temp,msg);
    }
    fclose(filep);
}
#endif

M_EXPORT int main(int argc, char** argv)
{
    QString file = "";
    bool showfrontpage = false;

    if(argc > 1) {
        file = parse_filename(argc, argv);
        showfrontpage = frontpage_in_args(argc,argv);
    }
#if 0
    qInstallMsgHandler(message_handler);
#endif

    ApplicationService *applicationService = new ApplicationService("com.nokia.OfficeToolsService", file);

    MLocale loc;
    loc.installTrCatalog("officetools");
    MLocale::setDefault(loc);


    MApplication *app  = MComponentCache::mApplication(argc, argv, "office-tools", applicationService);
    app->setApplicationName(qtTrId("qtn_comm_appname_offi"));
    MApplicationWindow *appWindow = MComponentCache::mApplicationWindow();
    appWindow->setWindowTitle(qtTrId("qtn_comm_appname_offi"));
    ApplicationWindow *window = new ApplicationWindow(appWindow);

    QGestureRecognizer::unregisterRecognizer(Qt::SwipeGesture);

    new OfficeServiceIfAdaptor(window);
    QDBusConnection connection = QDBusConnection::sessionBus();
    bool ret = connection.registerService("com.nokia.OfficeToolsService");
    if(ret)
        connection.registerObject("/", window);

    window->setIsFirstInstance(ret);
    applicationService->setConnection(&connection, ret);
    connection.connect(Service, Path, Interface, QString("lookForPopup"), applicationService, SLOT(lookForPopup(QString)));
    applicationService->setApplicationWindow(window,showfrontpage);

    if (!showfrontpage) {
        connection.connect(Service, Path, Interface, QString("closeFile"), applicationService, SLOT(closeFile(QString)));
    }
    else {
        applicationService->setFrontPageLaunched(true);
    }

    int result = app->exec();

    delete window;
    delete appWindow;
    delete app;

    return result;
}
