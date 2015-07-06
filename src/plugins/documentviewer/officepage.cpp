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

#include <QGraphicsSceneMouseEvent>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

#include <MLayout>
#include <MLabel>
#include <MPannableViewport>
#include <MFlowLayoutPolicy>
#include <MLinearLayoutPolicy>
#include <MComboBox>
#include <MWidgetAction>
#include <MOverlay>
#include <MProgressIndicator>

#include "officepage.h"
#include "officeviewer.h"
#include "officeviewerpresentation.h"
#include "officeviewerspreadsheet.h"
#include "officeviewerword.h"
#include "applicationwindow.h"
#include "definitions.h"
#include "actionpool.h"
#include "officethumbprovider.h"
#include "slideanimator.h"

#include "OfficeInterface.h"

#include <KoProgressUpdater.h>
#include <KoProgressProxy.h>
#include <QApplication>

class OfficePagePrivateData
{

public:
    OfficePagePrivateData();
    virtual ~OfficePagePrivateData();

    OfficeViewer       *officeviewer;
    MFlowLayoutPolicy  *normalViewPolicy;
    OfficeThumbProvider provider;
    QString             filePath;
    MProgressIndicator *progressBar;
};

class DocumentProgress : public KoProgressProxy
{
public:
    DocumentProgress(MProgressIndicator *progressBar)
    : m_progressBar(progressBar),
      m_previousValue(0)
    {}

    virtual ~DocumentProgress() {}

    virtual int maximum() const
    {
        return 100;
    }

    virtual void setValue(int value)
    {
        qDebug() << "Progress" << value;
        //Temp arrangement to ensure progress bar doesn't go back
        if(m_previousValue < value)
        {
            m_progressBar->setValue(qBound(0,value,100));
            m_previousValue = value;
        }
        QCoreApplication::processEvents();
    }

    virtual void setRange(int /*minimum*/, int /*maximum*/ ) {}
    virtual void setFormat( const QString &/*format*/ ) {}

private:
    MProgressIndicator *m_progressBar;
    int m_previousValue;
};


OfficePagePrivateData::OfficePagePrivateData()
    : officeviewer(0)
    , normalViewPolicy(0)
    , progressBar(0)
{
}

OfficePagePrivateData::~OfficePagePrivateData()
{
    qDebug() << __PRETTY_FUNCTION__;
}

OfficePage::OfficePage(const QString& document)
    : DocumentPage(document), data(new OfficePagePrivateData)
{
    setPannable(false);

}

OfficePage::~OfficePage()
{
    qDebug() << __PRETTY_FUNCTION__;
    delete data;
    qDebug() << __PRETTY_FUNCTION__ << "After deleting";
}

void OfficePage::createContent()
{
    MApplicationPage::createContent();

    initUI();

    MLayout * layout = new MLayout;
    Q_CHECK_PTR(layout);
    MLinearLayoutPolicy *policy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    Q_CHECK_PTR(policy);

    MWidget * panel = qobject_cast<MWidget *>(centralWidget());
    Q_CHECK_PTR(panel);
    panel->setLayout(layout);
    data->progressBar = new MProgressIndicator(this, MProgressIndicator::barType);
    data->progressBar->setObjectName("CommonProgressBar");
    data->progressBar->setRange(0,100);

    // the two labels are needed so that the progressbar is aligned vertically
    MLabel *l1 = new MLabel("");
    MLabel *l2 = new MLabel("");
    policy->addItem(l1, Qt::AlignCenter);
    policy->addItem(data->progressBar, Qt::AlignCenter);
    policy->addItem(l2, Qt::AlignCenter);
    policy->MAbstractLayoutPolicy::setStyleName("ProgressBarMargin");
}

void OfficePage::setOpeningProgress(int value)
{
    if(data->progressBar) {
        data->progressBar->setValue(qBound(0,value,100));
        QApplication::processEvents();
    }
}

void OfficePage::loadDocument()
{
    QFileInfo fileInfo(documentName);
    //setOpeningProgress(5);
    switch(ApplicationWindow::checkMimeType(fileInfo.filePath())) {

    case ApplicationWindow::DOCUMENT_WORD:
        data->officeviewer = new OfficeViewerWord(this);
        emit updateZoomLevel(ActionPool::ZoomFitToWidth);
        Q_CHECK_PTR(data->officeviewer);

        setObjectName("officepage_TextDocuments");
        break;

    case ApplicationWindow::DOCUMENT_PRESENTATION:
        data->officeviewer = new OfficeViewerPresentation(new SlideAnimator(this), this);
        m_defaultZoomLevelAction = ActionPool::ZoomFitToPage;
        emit updateZoomLevel(ActionPool::ZoomFitToPage);
        Q_CHECK_PTR(data->officeviewer);
        setObjectName("officepage_presentations");
        data->provider.setType(ThumbProvider::Slide);
        break;

    case ApplicationWindow::DOCUMENT_SPREADSHEET:
        data->officeviewer = new OfficeViewerSpreadsheet(this);
        m_defaultZoomLevelAction = ActionPool::Zoom120percent;
        Q_CHECK_PTR(data->officeviewer);

        setObjectName("officepage_spreadsheets");
        data->provider.setType(ThumbProvider::Sheet);
        connect(dynamic_cast<OfficeViewerSpreadsheet *>(data->officeviewer), SIGNAL(showingSheet(QString)), this, SLOT(updatePageIndicatorName(QString)));
        break;

    default:
        emit(loadFailed(documentName,"What is this!!!"));
        return;
    }
    connect(data->officeviewer, SIGNAL(pageChanged(int, int)),
            this, SLOT(setPageCounters(int, int)));
    connect(this, SIGNAL(showPage(int)),
            data->officeviewer, SLOT(showPage(int)), Qt::QueuedConnection);
    connect(data->officeviewer, SIGNAL(documentLoaded(bool)), this, SLOT(createKoWidget(bool)));
    connect(data->officeviewer, SIGNAL(matchesFound(bool)), this, SLOT(matchesFound(bool)));
    connect(data->officeviewer, SIGNAL(updateZoomLevel(ActionPool::Id)), this, SIGNAL(updateZoomLevel(ActionPool::Id)));
    connect(this, SIGNAL(visibleAreaChanged()), data->officeviewer, SLOT(updateRange()));

    DocumentProgress dp(data->progressBar);
    data->officeviewer->loadDocument(documentName, &dp);

    const QDir pluginDir("/usr/lib/office-tools/plugins");
    const QStringList plugins = pluginDir.entryList(QDir::Files);

    for (int i = 0; i < plugins.size(); ++i) {
        QPluginLoader test(pluginDir.absoluteFilePath(plugins.at(i)));
        QObject *plug = test.instance();
        if (plug != 0) {

            OfficeInterface* inter = qobject_cast<OfficeInterface*>(plug);
            if (inter) {

                plug->setParent(this);

                if (inter && inter->pluginType() == "document") { // Other types not supported here
                    MAction *pluginAction = new MAction(inter->pluginName(), this);
                    connect(pluginAction, SIGNAL(triggered()), plug, SLOT(emitOpenSignal()));
                    connect(plug, SIGNAL(openMe(OfficeInterface*)), this, SLOT(openPlugin(OfficeInterface*)));
                    pluginAction->setLocation(MAction::ApplicationMenuLocation);
                    addAction(pluginAction);
                }
            }
            else {
                delete plug;
            }
        }
    }
}

void OfficePage::openPlugin(OfficeInterface *plugin)
{
    plugin->setDocument(data->officeviewer->document());
    MApplicationPage *pluginView = plugin->createView();
    pluginView->appear(scene(), MSceneWindow::DestroyWhenDismissed);
}

void OfficePage::createKoWidget(bool status)
{
    if (data && data->progressBar) {
        data->progressBar->setVisible(false);
    }
    setStyleName("viewerBackground");
    if(!status || (true != data->officeviewer->createKoWidget())) {
        if(data->officeviewer->docOpenError.isEmpty())
            emit(loadFailed(documentName, qtTrId("qtn_offi_error_corrupt")));
        else
            emit(loadFailed(documentName,data->officeviewer->docOpenError));
    } else {
        MLayout * layout = new MLayout;
        Q_CHECK_PTR(layout);
        layout->setContentsMargins(0, 0, 0, 0);

        data->normalViewPolicy = new MFlowLayoutPolicy(layout);
        Q_CHECK_PTR(data->normalViewPolicy);
        data->normalViewPolicy->setContentsMargins(0,0,0,0);
        data->normalViewPolicy->setSpacing(0);

        MWidget * panel = qobject_cast<MWidget *>(centralWidget());
        Q_CHECK_PTR(panel);
        panel->setLayout(layout);

        emit(loadSuccess(documentName));
        data->normalViewPolicy->addItem(data->officeviewer->getGraphicsLayoutItem());

        ActionPool::instance()->getAction(m_defaultZoomLevelAction)->trigger();
        ActionPool::instance()->getAction(ActionPool::SpreadSheetFixedIndicators)->trigger();
        pageLoaded = true;
    }
}

void OfficePage::zoom(ZoomLevel level)
{
    if (data->officeviewer) {
        data->officeviewer->zoom(level);
        m_lastZoom = level;
    }
}

void OfficePage::shortTap(const QPointF &point, QObject *object)
{
    DocumentPage::shortTap(point, object);
    if (data->officeviewer) {
        data->officeviewer->shortTap(point, this);
    }
}

void OfficePage::searchText(DocumentPage::SearchMode mode, const QString &searchText)
{
    if(!pageLoaded) {
        return;
    }

    if(data->officeviewer) {
        switch(mode) {

        case DocumentPage::SearchFirst:
            data->officeviewer->startSearch(searchText);
            break;

        case DocumentPage::SearchNext:
            data->officeviewer->nextWord();
            break;

        case DocumentPage::SearchPrevious:
            data->officeviewer->previousWord();
            break;
        }
    }
}

void OfficePage::clearSearchTexts()
{
    if(data->officeviewer) {
        data->officeviewer->clearSearchResults();
    }
}

ThumbProvider* OfficePage::getThumbProvider()
{
    if(false == data->provider.isInitilized()) {
        data->provider.init(data->officeviewer);
    }

    return &data->provider;
}

void OfficePage::showPageIndexInternal(int pageIndex)
{
    qDebug() << __PRETTY_FUNCTION__ << " pageIndex:" << pageIndex;

    if (data->officeviewer) {
        data->officeviewer->showPage(pageIndex);
    }
}

void OfficePage::pinchStarted(QPointF &center)
{
    if (data->officeviewer) {
        data->officeviewer->pinchStarted(center);
    }
}

qreal OfficePage::pinchUpdated(qreal zoomFactor)
{
    return data->officeviewer ? data->officeviewer->pinchUpdated(zoomFactor) : zoomFactor;
}

void OfficePage::pinchFinished(const QPointF &center, qreal scale)
{
    if (data->officeviewer) {
        data->officeviewer->pinchFinished(center, scale);
    }
}

QGraphicsWidget *OfficePage::pinchWidget()
{
    return centralWidget();
}

void OfficePage::timedPinchFinished()
{
    MPannableWidget * pw = dynamic_cast<MPannableWidget *>(data->officeviewer->getGraphicsLayoutItem());

    if(pw) {
        pw->physics()->setEnabled(true);
        pw->setEnabled(true);
    }
}
