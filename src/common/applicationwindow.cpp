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

#include <QApplication>
#include <QFileInfo>
#include <QDebug>
#include <QTime>
#include <QDesktopServices>
#include <QSettings>
#include <QPluginLoader>

#include <MLocale>
#include <MSceneManager>
#include <MMessageBox>
#include <MApplication>
#include <MBanner>
#include <MAction>
#include <MProgressIndicator>
#include <MSaveAsDialog>
#include <MPannableViewport>

#include <maemo-meegotouch-interfaces/shareuiinterface.h>
#include <KMimeType>
#include "applicationwindow.h"
#include "applicationservice.h"
#include "allpagespage.h"
#include "actionpool.h"
#include "documentlistpage.h"
#include "documentlistmodel.h"
#include "documentdetailview.h"
#include "misc.h"
#include "ViewerInterface.h"
#include "trackerutils.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

ApplicationWindow::ApplicationWindow(MApplicationWindow *p_appWindow) :
    page(0),
    pageLoaded(false),
    allPagesPage(0),
    docDetail(0),
    viewMode(NoView),
    pageList(0),
    shareIf(0),
    showPagesPending(true),
    appWindow(p_appWindow),
    firstInstance(false),
    frontPageLaunched(false),
    applicationService(0),
    viewerInterface(0),
    infoBanner(0)
{
    setObjectName("applicationwindow");

    connect(appWindow, SIGNAL(pageChanged(MApplicationPage*)), this, SLOT(slotPageChanged(MApplicationPage*)));
    connect(ActionPool::instance(), SIGNAL(destroyed(QObject *)), this, SLOT(removeActions()));
    connect(appWindow, SIGNAL(displayEntered()), this, SLOT(showPages()));

    QDir path;
    path.setPath(QDir::homePath() + "/.config/office-tools/");

    if(!path.exists()) {
        if(!path.mkpath(QDir::homePath() + "/.config/office-tools/")) {
            qWarning() << "Some serious error in creating the required path" << QDir::homePath() + "/.config/office-tools";
            return;
        }
    }

    Qt::WindowStates state = appWindow->windowState();
    appWindow->setWindowState( state | Qt::WindowFullScreen );

    officeToolsSettings = new QSettings(path.filePath("office-tools.cfg"),QSettings::NativeFormat);
    appWindow->setCloseOnLazyShutdown(true);
}

ApplicationWindow::~ApplicationWindow()
{
    qDebug() << __PRETTY_FUNCTION__;

    officeToolsSettings->sync();

    if(!filePath.isEmpty()) {
        officeToolsSettings->setValue(filePath, false);
    }

    if(firstInstance) {
        officeToolsSettings->setValue("FirstInstanceRunning", false);
    }

    if(frontPageLaunched) {
        officeToolsSettings->setValue("FrontPageLaunched", false);
    }

    delete officeToolsSettings;

    if (infoBanner) {
        delete infoBanner;
    }

    hidePages();
    removeActions();

    QThreadPool *threadPool = QThreadPool::globalInstance();

    if(threadPool->activeThreadCount() > 1) {
        threadPool->waitForDone();
    }

    if(pageList) {
        delete pageList;
        pageList = 0;
    }

    if(allPagesPage) {
        delete allPagesPage;
        allPagesPage = 0;
    }

    if(page) {
        delete page;
        page = 0;
    }

    if(docDetail) {
        delete docDetail;
        docDetail = 0;
    }
}

void ApplicationWindow::slotPageChanged(MApplicationPage *page)
{
    Q_UNUSED(page);

    if(0 != page) {
        qDebug() << __PRETTY_FUNCTION__ << " page :" << page->title();
    } else {
        qDebug() << __PRETTY_FUNCTION__ << " page : ????" ;
    }
}


void ApplicationWindow::OpenListPage()
{
    officeToolsSettings->sync();
    if (officeToolsSettings->value("FrontPageLaunched", false).toBool()) {
        pid_t pid = officeToolsSettings->value("FrontPageLaunched/PID", 0).toInt();
        QDir path;
        path.setPath(QString("/proc/%1").arg(pid));

        if(path.exists()) {
           QDBusMessage msg = QDBusMessage::createSignal(Path, Interface, QString("showFrontPage"));
           applicationService->sendMessage(msg);
           QTimer::singleShot(300, QCoreApplication::instance(), SLOT(quit()));
           return;
        }
    }

//    appWindow->setWindowState( appWindow->windowState() & ~Qt::WindowFullScreen );
    if (0 == pageList) {
        pageList = new DocumentListPage();
        appWindow->show();
        pageList->appear(appWindow);

        connect(pageList,SIGNAL(DocumentDetailsView(QString)),this,SLOT(DocumentDetailsView(QString)));
        connect(pageList,SIGNAL(openShare()),this,SLOT(slotShare()));
        connect(pageList,SIGNAL(deleteDocuments()),this,SLOT(slotDelete()));

        pageList->setApplicationService(applicationService);
        applicationService->setFrontPageLaunched(true);

        frontPageLaunched = true;
        officeToolsSettings->sync();
        officeToolsSettings->setValue("FrontPageLaunched", true);
        officeToolsSettings->setValue("FrontPageLaunched/PID", getpid());
    } else {
        pageList->pannableViewport()->setPosition(QPoint(0,0));
    }
}

void ApplicationWindow::setApplicationService(ApplicationService *service)
{
    applicationService = service;
}

bool ApplicationWindow::launchFile(const QString& fileName)
{
    QUrl url(QUrl::fromPercentEncoding(fileName.toUtf8()));
    if ((TrackerUtils::Instance().urnFromUrl(QUrl::fromLocalFile(url.path()))).isEmpty()) {
        Atom atomWindowType = XInternAtom(QX11Info::display(),
                                          "_MEEGOTOUCH_NET_WM_WINDOW_TYPE_SHEET",
                                          False);
        XChangeProperty(QX11Info::display(), appWindow->effectiveWinId(),
                        XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE",
                        False),
                        XA_ATOM, 32, PropModeAppend, (unsigned char*)
                        &atomWindowType, 1);
    }

    //When launched from application grid, we get -showfrontpage as command line
    //and we show front page i.e. pageList !=0
    //When a file is opened from content manager we will get the file name
    //as an arg for launch function
    //In this scenario we dont open any thing and we wait for launch to get called
    //then pageList and page both of them will be null
    if(0 == pageList && 0 == page) {
        return OpenFile(fileName);
    }
    bool alreadyOpened = false;
    officeToolsSettings->sync();

    if(officeToolsSettings->value(url.path(), false).toBool()) {
        pid_t pid = officeToolsSettings->value(url.path() + "/PID", 0).toInt();
        QDir path;
        path.setPath(QString("/proc/%1").arg(pid));
        if(path.exists()) {
            alreadyOpened = true;
        }
    }

    Q_ASSERT(applicationService);
    if(alreadyOpened) {
        QDBusMessage msg = QDBusMessage::createSignal(Path, Interface, QString("lookForPopup"));
        msg << fileName;
        applicationService->sendMessage(msg);
    } else {
        QStringList args;
        args << "--type=m";
        args << "office-tools";
        if (fileName != "" &&
            fileName != "-showfrontpage") {
            args << fileName;
        } else {
            args << "-showfrontpage";

            if (officeToolsSettings->value("FrontPageLaunched", false).toBool()) {
                pid_t pid = officeToolsSettings->value("FrontPageLaunched/PID", 0).toInt();
                QDir path;
                path.setPath(QString("/proc/%1").arg(pid));

                if(path.exists()) {
                   QDBusMessage msg = QDBusMessage::createSignal(Path, Interface, QString("showFrontPage"));
                   applicationService->sendMessage(msg);
                   return true;
                }
            }
        }

        QProcess::startDetached("invoker", args);
    }

    return true;
}

void ApplicationWindow::setIsFirstInstance(bool isFirstInstance)
{
    firstInstance = isFirstInstance;

    if(firstInstance) {
        officeToolsSettings->sync();
        officeToolsSettings->setValue("FirstInstanceRunning", true);
        officeToolsSettings->setValue("FirstInstanceRunning/PID", getpid());
    }
}

bool ApplicationWindow::firstInstanceRunning()
{
    if(officeToolsSettings->value("FirstInstanceRunning", false).toBool()) {
        pid_t pid = officeToolsSettings->value("FirstInstanceRunning/PID", 0).toInt();
        QDir path;
        path.setPath(QString("/proc/%1").arg(pid));

        if(path.exists()) {
            return true;
        }
    }

    return false;
}

bool ApplicationWindow::OpenFile(const QString& fileName)
{
    //If we have old page, remove it and disconnect the signals
    if(page) {
        delete page;
        page = 0;
    }

    if(allPagesPage) {
        delete allPagesPage;
        allPagesPage = 0;
    }

    if(docDetail) {
        delete docDetail;
        docDetail = 0;
    }

    QUrl url(QUrl::fromPercentEncoding(fileName.toUtf8()));
    filePath = url.path();

    if ((TrackerUtils::Instance().urnFromUrl(QUrl::fromLocalFile(url.path()))).isEmpty()) {
        Atom atomWindowType = XInternAtom(QX11Info::display(),
                                          "_MEEGOTOUCH_NET_WM_WINDOW_TYPE_SHEET",
                                          False);
        XChangeProperty(QX11Info::display(), appWindow->effectiveWinId(),
                        XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE",
                        False),
                        XA_ATOM, 32, PropModeAppend, (unsigned char*)
                        &atomWindowType, 1);
    }

//    appWindow->setWindowState( appWindow->windowState() | Qt::WindowFullScreen );
    officeToolsSettings->sync();
    officeToolsSettings->setValue(filePath, true);
    officeToolsSettings->setValue(filePath + "/PID", getpid());

    QFileInfo fileInfo(filePath);
    DocumentType fileType = checkMimeType(fileInfo.filePath());

    QString error("Unknown Error");
    if (!viewerInterface) {
        //load plugin
        const QDir pluginDir("/usr/lib/office-tools/viewer");
        const QStringList plugins = pluginDir.entryList(QDir::Files);
        if (plugins.size() == 0) {
            error = "No plugins oufnd in /usr/lib/office-tools";
        }
        for (int i = 0; i < plugins.size(); ++i) {
            QPluginLoader *loader = new QPluginLoader(pluginDir.absoluteFilePath(plugins.at(i)));
	    qDebug() << "pluginloader" << loader;
	    loader->load();
            QObject *plugin = loader->instance();
            if (plugin != 0) {
                viewerInterface = qobject_cast<ViewerInterface*>(plugin);
                if (viewerInterface) {
                    plugin->setParent(this);
                }
                else {
                    delete plugin;
                }
                break;
            }
            error = loader->errorString();
	    delete loader;
        }

    }
    if (!viewerInterface) {
        error = "Fatal: could not loading viewer plugin " + error;
        qFatal(error.toAscii());
        return false;
    }
    page = viewerInterface->createDocumentPage(fileType, filePath);

    if(page) {
        appWindow->show();
        GetSceneManager()->appearSceneWindow(page);
        //connect(page,SIGNAL(displayEntered()),this,SLOT(documentPageDisplayEntered()));
        //Using the timer approch as neither displayEntered not appeared signal is getting
        //triggered on the page.
        QTimer::singleShot(0,this,SLOT(documentPageDisplayEntered()));
    } else {
        loadFailed(filePath, qtTrId("qtn_offi_error_corrupt"));
        return false;
    }

    return true;
}

void ApplicationWindow::documentPageDisplayEntered()
{
    static int leftOverWait = 10; //This should allow events to process
    if(leftOverWait == 0)
        return;
    if(page->sceneWindowState() != MSceneWindow::Appeared)
    {
        --leftOverWait;
        QTimer::singleShot(10,this,SLOT(documentPageDisplayEntered()));
        return;
    }
    leftOverWait = 0;
    
    //disconnect(page,SIGNAL(appeared()),this,SLOT(documentPageDisplayEntered()));
    page->createFinalContent();
    appWindow->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    connect(page,SIGNAL(loadFailed(const QString &, const QString &)),
            this,SLOT(loadFailed(const QString &, const QString &)));
    connect(page,SIGNAL(loadSuccess(const QString &)),
            this,SLOT(loadSuccess(const QString &)));
    connect(page, SIGNAL(closeDocumentPage()), this, SLOT(closeDocumentPage()), Qt::QueuedConnection);
    connect(page, SIGNAL(openShare()), this, SLOT(slotShare()));
    connect(page, SIGNAL(deleteDocument()), this, SLOT(slotDelete()));
    connect(page, SIGNAL(toggleFavorite()), this, SLOT(slotFavourite()));
    connect(page, SIGNAL(showDetails()), this, SLOT(DocumentDetailsView()));
    connect(page, SIGNAL(showFrontPageView()), this, SLOT(showFrontPageView()));
    connect(page, SIGNAL(showAllPagesView()), this, SLOT(showAllPagesView()));
    connect(page, SIGNAL(showNormalView()), this, SLOT(showNormalView()));
    connect(page, SIGNAL(saveDocumentAs()), this, SLOT(slotSaveAs()));
    connect(page, SIGNAL(documentCloseEvent()), this, SLOT(exitApplication()));
    QTimer::singleShot(0, this, SLOT(slotOpenDocument()));
}


void ApplicationWindow::slotOpenDocument()
{
    if(page) {
        page->loadDocument();
    }
}

void ApplicationWindow::exitApplication()
{
    page->disappear();
    appWindow->hide();
    QCoreApplication::flush();
    QCoreApplication::quit();
}

ApplicationWindow::DocumentType ApplicationWindow::checkMimeType(const QString &fileName)
{
    ApplicationWindow::DocumentType type = ApplicationWindow::DOCUMENT_UNKOWN;
    QString mimetype = KMimeType::findByPath(fileName)->name();

    if((QString::compare("application/msword", mimetype) == 0) ||
       (QString::compare("application/x-mswrite", mimetype) == 0) ||
       (QString::compare("text/plain", mimetype) == 0) ||
       (QString::compare("application/vnd.oasis.opendocument.text", mimetype) == 0) ||
       (QString::compare("application/vnd.openxmlformats-officedocument.wordprocessingml.document", mimetype) == 0) ||
       (QString::compare("application/rtf", mimetype) == 0) ||
       (QString::compare("application/x-vnd.oasis.opendocument.text", mimetype) == 0) ||
       (QString::compare("application/vnd.ms-word.document.macroEnabled.12", mimetype) == 0) ||
       (QString::compare("application/vnd.openxmlformats-officedocument.wordprocessingml.template", mimetype) == 0) ||
       (QString::compare("application/vnd.ms-word.template.macroEnabled.12", mimetype) == 0) ||
       (QString::compare("application/vnd.oasis.opendocument.text-template", mimetype) == 0) ||
       (QString::compare("application/x-vnd.oasis.opendocument.text-template", mimetype) == 0)) {
        type = ApplicationWindow::DOCUMENT_WORD;
    } else if(QString::compare("application/pdf", mimetype) == 0) {
        type = ApplicationWindow::DOCUMENT_PDF;
    } else if((QString::compare("application/mspowerpoint", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-powerpoint", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-powerpoint.slideshow.macroEnabled.12", mimetype) == 0) ||
              (QString::compare("application/vnd.oasis.opendocument.presentation", mimetype) == 0) ||
              (QString::compare("application/vnd.oasis.opendocument.presentation-template", mimetype) == 0) ||
              (QString::compare("application/x-vnd.oasis.opendocument.presentation-template", mimetype) == 0) ||
              (QString::compare("application/vnd.openxmlformats-officedocument.presentationml.presentation", mimetype) == 0) ||
              (QString::compare("application/vnd.openxmlformats-officedocument.presentationml.slideshow", mimetype) == 0) ||
              (QString::compare("application/vnd.oasis.opendocument.presentation", mimetype) == 0) ||
              (QString::compare("application/x-vnd.oasis.opendocument.presentation", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-powerpoint.presentation.macroEnabled.12", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-powerpoint.addin.macroEnabled.12", mimetype) == 0) ||
              (QString::compare("application/vnd.openxmlformats-officedocument.presentationml.template", mimetype) == 0)) {
        type = ApplicationWindow::DOCUMENT_PRESENTATION;
    } else if((QString::compare("application/vnd.ms-excel", mimetype) == 0) ||
              (QString::compare("application/vnd.oasis.opendocument.spreadsheet", mimetype) == 0) ||
              (QString::compare("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", mimetype) == 0) ||
              (QString::compare("application/x-vnd.oasis.opendocument.spreadsheet", mimetype) == 0) ||
              (QString::compare("application/excel", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-excel.sheet.macroEnabled.12", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-excel.sheet.binary.macroEnabled.12", mimetype) == 0) ||
              (QString::compare("application/vnd.oasis.opendocument.spreadsheet-template", mimetype) == 0) ||
              (QString::compare("application/x-vnd.oasis.opendocument.spreadsheet-template", mimetype) == 0) ||
              (QString::compare("application/vnd.openxmlformats-officedocument.spreadsheetml.template", mimetype) == 0) ||
              (QString::compare("application/vnd.ms-excel.template.macroEnabled.12", mimetype) == 0) ||
              (QString::compare("application/x-excel", mimetype) == 0) ||
              (QString::compare("application/xlt", mimetype) == 0) ||
              (QString::compare("application/x-msexcel", mimetype) == 0) ||
              (QString::compare("application/msexcel", mimetype) == 0)) {
        type = ApplicationWindow::DOCUMENT_SPREADSHEET;
    }

    return type;
}


void ApplicationWindow::loadFailed(const QString &file, const QString &reason)
{
    viewMode=NoView;
    appWindow->show();

    if (!file.isEmpty() && !reason.isEmpty()) {
        qDebug()<<"Failed to load document "<<file<<": "<<reason << " : " << filePath;
        showBannerInformation(reason);
    }
    closeDocumentPage();
    QTimer::singleShot(3000, QCoreApplication::instance(), SLOT(quit()));
}

void ApplicationWindow::loadSuccess(const QString &file)
{
    if(page) {
        if(showPagesPending)
            page->appear(appWindow);

        viewMode=NormalView;
        qDebug()<<file<<" loaded succesfully!";
        pageLoaded =true;
    }
}

void ApplicationWindow::closeDocumentPage()
{
    if(page) {
        page->disappear();
        delete page;
        page = 0;
    }

    if(pageList) {
        pageList->disappear();
        delete pageList;
        pageList = 0;
    }
}

QSize ApplicationWindow::visibleSize()
{
    if(MApplication::activeApplicationWindow())
        return MApplication::activeApplicationWindow()->visibleSceneSize();

    return QSize(0,0);
}

QSize ApplicationWindow::visibleSizeCorrect()
{
    MApplicationWindow *window = MApplication::activeApplicationWindow();
    if (window && window->currentPage()) {
        return window->currentPage()->exposedContentRect().size().toSize();
    }

    return QSize(0, 0);
}

QSize ApplicationWindow::visibleSize(M::Orientation orientation)
{
    MApplicationWindow *window = MApplication::activeApplicationWindow();

    if(window)
        return window->visibleSceneSize(orientation);

    return QSize(0,0);
}


MSceneManager* ApplicationWindow::GetSceneManager()
{
    if(MApplication::activeApplicationWindow())
        return MApplication::activeApplicationWindow()->sceneManager();

    return 0;
}

void ApplicationWindow::showPages()
{
    qDebug() << __PRETTY_FUNCTION__ ;
    //Lets make sure that widgets are shown when application window is entered.
    //For pdf it has impact on used memory

    if(0 != page && NormalView == viewMode) {
        if(pageLoaded) {
            qDebug() << __PRETTY_FUNCTION__ << "page apper";
            page->appear(appWindow);;
            showPagesPending = false;
        }
    } else if(0 != allPagesPage && AllPagesView == viewMode) {
        qDebug() << __PRETTY_FUNCTION__ << "all pages apper";
        allPagesPage->appear(appWindow);
    }
}

void ApplicationWindow::hidePages()
{
    qDebug() << __PRETTY_FUNCTION__ ;
    //Lets make sure that widgets are hiden so that images are released

    if(0 != allPagesPage) {
        allPagesPage->disappear();
    }

    if(0 != page) {
        page->disappear();
    }
}

void ApplicationWindow::showNormalView()
{
    viewMode=NormalView;
    showPages();
}

void ApplicationWindow::showFrontPageView()
{
    if(firstInstanceRunning() &&
       officeToolsSettings->value("FrontPageLaunched", false).toBool()) {
        pid_t pid = officeToolsSettings->value("FrontPageLaunched/PID", 0).toInt();
        QDir path;
        path.setPath(QString("/proc/%1").arg(pid));

        if(path.exists()) {
            QDBusMessage msg = QDBusMessage::createSignal(Path, Interface, QString("showFrontPage"));
            applicationService->sendMessage(msg);
            appWindow->hide();
            QCoreApplication::flush();
            QCoreApplication::quit();
            return;
        }
    }

    closeDocumentPage();

    if(!filePath.isEmpty()) {
        officeToolsSettings->setValue(filePath, false);
    }

    // Setting to the default service name for front page, it is possible
    // that this service was previous eg. com.nokia.officetools1
    applicationService->setServiceName("com.nokia.OfficeToolsService");
    applicationService->setFileName("");

    OpenListPage();
}

void ApplicationWindow::showAllPagesView()
{
    if(!pageLoaded) {
        return;
    }

    if(0 == allPagesPage) {
        allPagesPage = new AllPagesPage(page->documentName, page->documentUrn, (page->objectName() == "officepage_spreadsheets"));

        connect(allPagesPage, SIGNAL(showPageIndexDefaultZoom(int)),
                page , SLOT(showPageIndexDefaultZoom(int)));
        allPagesPage->addThumbProvider(page->getThumbProvider());
    }

    allPagesPage->setCurrentPage(page->currentPage);
    allPagesPage->scrollToCurrentVisiblePage();
    viewMode=AllPagesView;
    showPages();
}

void ApplicationWindow::removeActions()
{
    foreach(QAction *action, appWindow->actions()) {
        appWindow->removeAction(action);
    }
}

void ApplicationWindow::slotFavourite()
{
    if(!pageLoaded) {
        return;
    }

    if(0 != page) {
        DocumentListModel::setFavourite(page->documentUrn);
    }
}

void ApplicationWindow::slotShare()
{
    QStringList uris;
    bool currentPage = false;

    if(0 != pageList && sender() == pageList) {
        if(pageList->getlongTappedRow() != -1 &&
           pageList->getCurrentSubview() != DocumentListPage::ShareMultipleView &&
           pageList->getCurrentSubview() != DocumentListPage::DeleteMultipleView) {
            uris << pageList->getLongTappedRowUrn();
        } else if(pageList->getCurrentSubview() == DocumentListPage::ShareMultipleView) {
            uris << pageList->getSelectedUrns();

            if(!uris.isEmpty())
                pageList->closeSubview();
        }
    } else if(0 != page) {
        uris << page->documentUrn;
        currentPage = true;
    }

    if(currentPage && !pageLoaded) {
        return;
    }
    if(uris.isEmpty())
    {
        return;
    }

    if(!shareIf)
        shareIf = new ShareUiInterface("com.nokia.ShareUi");

    if(!uris.isEmpty() && shareIf->isValid()) {
        shareIf->share(uris);
    }

    qDebug() << "Sharing the following URIs: " << uris;
}


void ApplicationWindow::slotDelete()
{
    int totalFiles = -1;
    bool currentPage = false;
    pathsToDelete.clear();

    if(0 != pageList && sender() == pageList) {
        if(pageList->getlongTappedRow() != -1 &&
           pageList->getCurrentSubview() != DocumentListPage::ShareMultipleView &&
           pageList->getCurrentSubview() != DocumentListPage::DeleteMultipleView) {
            pathsToDelete << pageList->getLongTappedRowPath();
        } else if(pageList->getCurrentSubview() == DocumentListPage::DeleteMultipleView) {
            totalFiles = pageList->getFileCount();
            pathsToDelete << pageList->getSelectedPaths();
        }
    } else if(0 != page) {
        QString documentPath = "file://";
        (page->documentName.contains(documentPath) ? documentPath = page->documentName : documentPath.append(page->documentName));
        pathsToDelete << documentPath;
        currentPage = true;
    }

    if(currentPage && !pageLoaded) {
        return;
    }

    qDebug() << "DELETING the following URIs: " << pathsToDelete;

    QString titleString, msgString;

    if(pathsToDelete.count() == 0) {
        qDebug() << "No files to delete returning";
        return;
    } else if(pathsToDelete.count() == totalFiles) {
        titleString = QString(qtTrId("qtn_offi_delete_all"));
    } else if(pathsToDelete.count() == 1) {
        titleString = QString(qtTrId("qtn_offi_delete_single"));
        msgString = QString(QFileInfo(QUrl(QUrl::fromPercentEncoding(pathsToDelete[0].toUtf8())).path()).fileName());
    } else if(pathsToDelete.count() > 1) {
        titleString = QString(qtTrId("qtn_offi_delete_multiple")).arg(pathsToDelete.count());
    }

    MMessageBox * confirmDialog = new MMessageBox(titleString, msgString, M::YesButton | M::NoButton);

    connect(
        confirmDialog,
        SIGNAL(accepted()),
        this,
        SLOT(deleteConfirmationYes()));

    connect(
        confirmDialog,
        SIGNAL(rejected()),
        this,
        SLOT(deleteConfirmationNo()));
    confirmDialog->appear(MSceneWindow::DestroyWhenDone);

}

void ApplicationWindow::deleteConfirmationYes()
{
    foreach(QString path,pathsToDelete) {
        qDebug() << "Deleting file " << path;

        if(path.isEmpty())
            continue;

        QString documentUrn = TrackerUtils::Instance().urnFromUrl(QUrl(path));

        if(QFile::remove(QUrl(QUrl::fromPercentEncoding(path.toUtf8())).path())) {
            //Successfully deleted...
        }
    }

    if(0 != pageList) {
        pageList->docsDeleted(pathsToDelete);
        pageList->closeSubview();
        pageList->refreshList();
    } else if(0 != page) {
        QCoreApplication::quit();
    }
    pathsToDelete.clear();
}

void ApplicationWindow::deleteConfirmationNo()
{
    if(0 != pageList) {
        pageList->closeSubview();
    }
//    pathsToDelete.clear();
//    pageList->refreshList();
}

void ApplicationWindow::DocumentDetailsView(QString documentPath)
{
    docDetail = new DocumentDetailView(documentPath);
    viewMode = DocDetailView;
    docDetail->appear(appWindow);
}

void ApplicationWindow::DocumentDetailsView()
{
    QString documentPath;

    if(!pageLoaded) {
        return;
    }

    documentPath = page->documentName;

    docDetail = new DocumentDetailView(documentPath);

    viewMode = DocDetailView;

    docDetail->appear(appWindow);

    page->hidePageIndicator();
}

void ApplicationWindow::slotSaveAs()
{
    if(0 == page) {
        return;
    }

    QFileInfo fileInfo(QUrl(QUrl::fromPercentEncoding(page->documentName.toUtf8())).path());

    MSaveAsDialog *saveAs = new MSaveAsDialog;
    connect(saveAs, SIGNAL(saveFileAs(QString)), this, SLOT(documentSaved(QString)));
    QString newFileName = MSaveAsDialog::generateUniqueFileName(fileInfo.absolutePath(),fileInfo.baseName(),"." + fileInfo.suffix());
    newFileName.chop(fileInfo.suffix().length() + 1);
    saveAs->setDefaultFileName(newFileName);
    saveAs->setSourcePath(fileInfo.absoluteFilePath());
    saveAs->setAutoCopyMode(MSaveAsDialog::AutoCopyEnabled);
    QString newPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    if(newPath.isEmpty()) {
        newPath = QDir::homePath();
    }

    saveAs->setDestinationPath(newPath);
    saveAs->setDefaultExtension("." + fileInfo.suffix());
    saveAs->setContentSize(fileInfo.size());
    saveAs->setMimeType(Misc::getFileTypeFromFile(fileInfo.absoluteFilePath()));
    saveAs->appear(MSceneWindow::DestroyWhenDone);
}

void ApplicationWindow::documentSaved(const QString &fileName)
{
    QString newPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    if(newPath.isEmpty()) {
        newPath = QDir::homePath();
    }

    officeToolsSettings->sync();
    if(!filePath.isEmpty()) {
        officeToolsSettings->setValue(filePath, false);
    }
    QUrl url(QUrl::fromPercentEncoding((newPath + "/" + fileName).toUtf8()));
    filePath = url.path();
    applicationService->setFileName(filePath);
    officeToolsSettings->sync();
    officeToolsSettings->setValue(filePath, true);
    officeToolsSettings->setValue(filePath + "/PID", getpid());

    page->setDocumentName(newPath + "/" + fileName);
    page->fakeDocumentSaved();
    QTimer::singleShot(2000, page, SLOT(waitForTrackerIndexing()));
}

void ApplicationWindow::toNormalView()
{
    if (page) {
        QString documentUrn = TrackerUtils::Instance().urnFromUrl(QUrl::fromLocalFile(page->documentName));
        if (documentUrn.isEmpty()) {
            QTimer::singleShot(2000, this, SLOT(toNormalView()));
            return;
        }
        page->createFinalContent();
    }
}

void ApplicationWindow::showBannerInformation(const QString &message)
{
    if (!infoBanner) {
        infoBanner = new MBanner();
        infoBanner->setStyleName("InformationBanner");
    }

    infoBanner->setTitle(message);
    if (!infoBanner->isActive()) {
        infoBanner->appear(appWindow);
    }
}
