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
#include <QPinchGesture>
#include <QGestureEvent>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <TrackerLiveQuery>
#include <QApplication>
#include <MApplication>
#include <MTextEdit>
#include <maemo-meegotouch-interfaces/shareuiinterface.h>
#include <MToolBar>
#include <MEscapeButtonPanel>
#include <MLinearLayoutPolicy>
#include <MLayout>
#include <MWidgetAction>
#include <MComboBox>
#include <MPannableWidget>
#include <MPannableViewport>
#include <mapplicationpageview.h>
#include <MInputMethodState>
#include <MOverlay>
#include <MButton>
#include <MBanner>
#include <MImageWidget>
#include <QStyleOptionGraphicsItem>
#include <QDesktopServices>

#include "documentpage.h"
#include "definitions.h"
#include "pageindicator.h"
#include "actionpool.h"
#include "applicationwindow.h"
#include "documentlistmodel.h"
#include "jumptotoolbar.h"
#include "findtoolbar.h"
#include "trackerutils.h"
#include "quickviewertoolbar.h"

namespace
{
const qreal ZoomOutRelativeFactor = 0.70;
const qreal ZoomInRelativeFactor = 1.30;
const qreal ZoomDefaultFactor = 1.0;

const qreal WaitTimeOut = 1000;
}

static const int NAVI_BAR_TIMEOUT = 5000;
static const int DOUBLETAP_INTERVAL = 325;

QString getDocumentUrn(QString documentName)
{
    QString documentUrn = TrackerUtils::Instance().urnFromUrl(QUrl::fromLocalFile(documentName));
    if (!documentUrn.isEmpty()) {
        TrackerUtils::Instance().updateContentAccessedProperty(documentUrn);
    }
    return documentUrn;
}

// This should be removed once [Harmattan - Bug 247337] [TASK] CSS background-repeat should be supported
class ZoomBackground : public MApplicationPageView
{
public:
    ZoomBackground(MApplicationPage *controller)
    : MApplicationPageView(controller)
    , m_zoom(false)
    {
    }

    virtual ~ZoomBackground()
    {}

    void setZoomBackground(bool zoom)
    {
        m_zoom = zoom;
    }

    void drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const
    {
        if (m_zoom) {
            painter->fillRect(option->exposedRect, QBrush("#6b6e70"));
        }
        else {
            MApplicationPageView::drawBackground(painter, option);
        }
    }
private:
    bool m_zoom;
};


DocumentPage::DocumentPage(const QString& filePath, QGraphicsItem *parent)
    : MApplicationPage(parent)
    , currentPage(1)
    , m_pinchInProgress(false)
    , m_endScale(1.0)
    , m_blockRecenter(false)
    , pageLoaded(false)
    , m_defaultZoomLevelAction(ActionPool::ZoomFitToWidth)
    , pageIndicator(0)
    , m_infoBanner(0)
    , shareIf(0)
    , searchstring("")
    , searchingTimeout(false)
    , zoomAction(0)
    , indicatorAction(0)
    , zoomCombobox(0)
    , indicatorCombobox(0)
    , searchStarted(false)
    , noMatches(false)
    , totalPage(1)
    , liveDocument(0)
    , bounceAnimation(0)
    , jumpToPageOverlay(0)
    , findtoolbar(0)
    , quickViewToolbar(0)
    , quickViewer(false)
    , m_pageView(new ZoomBackground(this))
    , m_lastZoomFactor(1.0)
{
    setView(m_pageView);
    documentName = filePath;
    setObjectName("documentpage");

    // double click interval setting to 325ms
    QApplication::setDoubleClickInterval(DOUBLETAP_INTERVAL);

    setAutoMarginsForComponentsEnabled(false);
    setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
    setEscapeMode(MApplicationPageModel::EscapeCloseWindow);
    qRegisterMetaType<ZoomLevel>("ZoomLevel");
    m_autoHideTimer.setSingleShot(true);
    m_autoHideTimer.setInterval(NAVI_BAR_TIMEOUT);

    m_shortTapTimer.setSingleShot(true);
    m_shortTapTimer.setInterval(QApplication::doubleClickInterval());

    searchTimer.setSingleShot(true);
    searchTimer.setInterval(searchDelay);

    connect(&searchTimer, SIGNAL(timeout()), this, SLOT(searchTimeout()));
    connect(&m_shortTapTimer, SIGNAL(timeout()), this, SLOT(shortTapEvent()));
    connect(&m_autoHideTimer, SIGNAL(timeout()), this, SLOT(autoHideToolbar()));
    connect(this, SIGNAL(backButtonClicked()), this, SLOT(onClose()));
    connect(ActionPool::instance(), SIGNAL(destroyed(QObject *)), this, SLOT(removeActions()));
    connect(this, SIGNAL(loadSuccess(QString)), SLOT(updateViewerType()));
    connect(MInputMethodState::instance(), SIGNAL(inputMethodAreaChanged(const QRect &)), this, SLOT(sendVisibleAreayChanged()));
}

void DocumentPage::createFinalContent()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!pageIndicator) {
        pageIndicator = new PageIndicator(QString(), this);
    }

    if (documentUrn.isEmpty()) {
        documentUrn = TrackerUtils::Instance().urnFromUrl(QUrl::fromLocalFile(documentName));
    }
    TrackerUtils::Instance().updateContentAccessedProperty(documentUrn);
    if (!documentUrn.isEmpty() && !liveDocument) {
        liveDocument = TrackerUtils::Instance().createDocumentLiveUpdate(QUrl(documentName));
        if (liveDocument) {
            connect(liveDocument->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(documentsChanged(QModelIndex, QModelIndex)));
            connect(liveDocument->model(), SIGNAL(rowsRemoved (QModelIndex, int, int)), SIGNAL(documentCloseEvent()));
        }
    }


    QFileInfo fileInfo(documentName);
    fileTitle = fileInfo.completeBaseName();

    pageIndicator->setFileName(fileTitle);
    setAcceptTouchEvents(true);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::TapGesture);
}
DocumentPage::~DocumentPage()
{
    qDebug() << __PRETTY_FUNCTION__;
    pageLoaded = false;
    delete shareIf;
    removeActions();
    if (liveDocument) {
        delete liveDocument;
    }
    qDebug() << __PRETTY_FUNCTION__ << "After Deleting";
}

void DocumentPage::updateViewerType()
{
    // set the background so that it is also correct while zooming out pdf documents
    m_pageView->setZoomBackground(true);
    if (documentUrn.isEmpty()) {
        ///FIXME: Hack to prevent quickviewer from opening when the file is not yet indexed by tracker
        if (documentName.contains("/home/user/MyDocs/", Qt::CaseInsensitive) && !documentName.contains("/home/user/MyDocs/Downloads", Qt::CaseInsensitive)) {
            QTimer::singleShot(2000, this, SLOT(waitForTrackerIndexing()));
            return;
        }
        quickViewToolbar = new QuickViewerToolbar(this);
        quickViewer = true;
        removeActions();
        //We don't want to go to All pages view. So disconnect the connection with all pages view action
        disconnect(ActionPool::instance()->getAction(ActionPool::ShowAllPagesView), SIGNAL(triggered()),
                   this, SIGNAL(showAllPagesView()));

        QTimer::singleShot(0, this, SLOT(sendVisibleAreayChanged()));
    }
}

void DocumentPage::sendVisibleAreayChanged()
{
    qDebug() << __PRETTY_FUNCTION__;
    emit visibleAreaChanged();
}

void DocumentPage::waitForTrackerIndexing()
{
    static int tries = 1;

    if (tries > 3)
        return;

    documentUrn = TrackerUtils::Instance().urnFromUrl(QUrl::fromLocalFile(documentName));
    if (documentUrn.isEmpty()) {
        QTimer::singleShot(tries*WaitTimeOut, this, SLOT(waitForTrackerIndexing()));
        return;
    }

    createFinalContent();
}

void DocumentPage::initUI()
{
    applicationWindow()->setStyleName("ViewerToolbar");
    applicationWindow()->setNavigationBarOpacity(0.8);

    connect(MApplication::activeWindow(), SIGNAL(orientationChanged(const M::Orientation &)),
            this, SLOT(changeOrientation(const M::Orientation &)));

    addActions();
    connectActions();
}

void DocumentPage::closeEvent(QCloseEvent *event)
{
    emit documentCloseEvent();
    event->accept();
}

void DocumentPage::addActions()
{
    ActionPool * actions = ActionPool::instance();

    const ActionPool::Id actionIds[] = {
        ActionPool::MarkFavorite,
        ActionPool::UnMarkFavorite,
        ActionPool::ViewDetails,
        ActionPool::Share,
        ActionPool::ShowFrontPageView,
        ActionPool::Delete,
        ActionPool::EnterPageNumber,
        ActionPool::Find,
        ActionPool::ShowAllPagesView
    };

    unsigned int numberOfActions = (sizeof(actionIds) / sizeof actionIds[0]);

    QFileInfo fileInfo(documentName);
    if (ApplicationWindow::DOCUMENT_SPREADSHEET != ApplicationWindow::checkMimeType(fileInfo.filePath())) {
        // according to the spec zoom level combo should not be shown for spreadsheets
        createCombo(ActionPool::instance()->getAction(ActionPool::Zoomlevels), ActionPool::instance()->getAction(ActionPool::ZoomFitToWidth),
                ActionPool::instance()->getAction(ActionPool::ZoomFitToPage), ActionPool::instance()->getAction(ActionPool::Zoom100percent));
        connect(this, SIGNAL(updateZoomLevel(ActionPool::Id)), this, SLOT(updateZoomCombobox(ActionPool::Id)));

        if(zoomCombobox) {
            zoomCombobox->setCurrentIndex(0);
         }
    } else {
        createIndicatorCombo(ActionPool::instance()->getAction(ActionPool::Indicators),
                             ActionPool::instance()->getAction(ActionPool::SpreadSheetFixedIndicators),
                             ActionPool::instance()->getAction(ActionPool::SpreadSheetFloatingIndicators),
                             ActionPool::instance()->getAction(ActionPool::SpreadSheetNoIndicators));
    }

    for(unsigned int i = 0; i < numberOfActions; i++) {
        MAction *action = actions->getAction(actionIds[i]);
        Q_CHECK_PTR(action);

        if(actionIds[i] == ActionPool::MarkFavorite)
            appMenuMFAction = action;
        else if(actionIds[i] == ActionPool::UnMarkFavorite)
            appMenuUFAction = action;

        addAction(action);
    }

    changeMenus();
}

void DocumentPage::changeOrientation(const M::Orientation & orientation)
{
    switch(orientation) {

    case M::Landscape :
        break;

    case M::Portrait :
        break;
    }
}

void DocumentPage::fakeDocumentSaved()
{
    if (quickViewToolbar) {
        delete quickViewToolbar;
        quickViewToolbar = 0;

        emit visibleAreaChanged();
    }
    quickViewer = false;
    QFileInfo fileInfo(documentName);
    fileTitle = fileInfo.completeBaseName();
    pageIndicator->setFileName(fileTitle);

    addActions();
    //This action was disconnected in Quick Viewer mode.
    connect(ActionPool::instance()->getAction(ActionPool::ShowAllPagesView), SIGNAL(triggered()),
               this, SIGNAL(showAllPagesView()));
}

void DocumentPage::connectActions(bool onlyToolbar)
{
    ActionPool * actions = ActionPool::instance();

    const struct actionStruct {
        ActionPool::Id   id;
        QObject       *object;
        const char    *slot;
    } actionData[] = {

        {   ActionPool::Share,                  this,   SIGNAL(openShare())}
        ,{  ActionPool::Delete,                 this,   SIGNAL(deleteDocument())}
        ,{  ActionPool::Find,                   this,   SLOT(createSearchToolBar())}
        ,{  ActionPool::MarkFavorite,           this,   SIGNAL(toggleFavorite())}
        ,{  ActionPool::UnMarkFavorite,         this,   SIGNAL(toggleFavorite())}
        ,{  ActionPool::ViewDetails,            this,   SIGNAL(showDetails())}
        ,{  ActionPool::ShowFrontPageView,      this,   SIGNAL(showFrontPageView())}
        ,{  ActionPool::ShowAllPagesView,       this,   SIGNAL(showAllPagesView())}
        ,{  ActionPool::ShowNormalView,         this,   SIGNAL(showNormalView())}
#ifdef TESTING_PURPOSE
        ,{  ActionPool::ZoomOut,                this,   SLOT(onZoomOut())} //Only for testing
        ,{  ActionPool::ZoomIn,                 this,   SLOT(onZoomIn())} //Only for testing
#endif
        ,{  ActionPool::ZoomFitToWidth,         this,   SLOT(onZoomFitToWidth())}
        ,{  ActionPool::ZoomFitToPage,          this,   SLOT(onZoomFitToPage())}
        ,{  ActionPool::ZoomLastUserDefined,    this,   SLOT(zoomBy200percent())}
        ,{  ActionPool::Zoom100percent,         this,   SLOT(onZoom100percent())}
        ,{  ActionPool::NormalScreenNormalView, this,   SLOT(SetNormalscreen())}
        ,{  ActionPool::Zoom120percent,         this,   SLOT(set120percentZoom())}
        ,{  ActionPool::EnterPageNumber,        this,   SLOT(slotJumpToPage())}
    };

    for(unsigned int i = 0; i < (sizeof(actionData) / sizeof actionData[0]); i++) {
        MAction *action = actions->getAction(actionData[i].id);
        Q_CHECK_PTR(action);

        if(false == onlyToolbar) {
            connect(action, SIGNAL(triggered()), actionData[i].object, actionData[i].slot);
        } else if(0 != (MAction::ToolBarLocation && action->location())) {
            connect(action, SIGNAL(triggered()), actionData[i].object, actionData[i].slot);
        }
    }
}

void DocumentPage::onZoomFitToWidth()
{
    zoom(ZoomLevel(ZoomLevel::FitToWidth, 1.0, false));
}

void DocumentPage::onZoomFitToPage()
{
    zoom(ZoomLevel(ZoomLevel::FitToPage, 1.0, false));
}

void DocumentPage::onZoom100percent()
{
    zoom(ZoomLevel(ZoomLevel::FactorMode, 1.0, false));
}

#ifdef TESTING_PURPOSE
void DocumentPage::onZoomOut()
{
    ZoomLevel zoomLevel;
    zoomLevel.setRelativeFactor(ZoomOutRelativeFactor);
    zoomLevel.setZoomType(ZoomLevel::ZoomOut);
    zoom(zoomLevel);
}

void DocumentPage::onZoomIn()
{
    ZoomLevel zoomLevel;
    zoomLevel.setRelativeFactor(ZoomInRelativeFactor);
    zoomLevel.setZoomType(ZoomLevel::ZoomIn);
    zoom(zoomLevel);
}

#endif

void DocumentPage::zoomBy200percent()
{
    qDebug() << __PRETTY_FUNCTION__;
    // Zoom by 200% as discussed with Juha
    zoom(ZoomLevel(ZoomLevel::Relative, 2.0, true));
}

// Key handler
void DocumentPage::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (searchActive() || jumpActive()) {
        return;
    }

    if (event->matches(QKeySequence::Find)) {
        createSearchToolBar();
    }
    else if (!event->text().isEmpty()) {
        Qt::KeyboardModifiers modifier = event->modifiers();

        if(modifier == Qt::NoModifier ||
           modifier == Qt::ShiftModifier ||
           modifier == Qt::KeypadModifier) {
            createSearchToolBar();
            QString text(event->text());
            findtoolbar->setText(text);
        }
    }

    if(event->key() == Qt::Key_Left)
        showPrevPage();

    if(event->key() == Qt::Key_Right)
        showNextPage();

}


void DocumentPage::pinchGestureEvent(QGestureEvent *event, QPinchGesture *gesture)
{
    if (!pageLoaded || (bounceAnimation && bounceAnimation->state() == QAbstractAnimation::Running)) {
        return;
    }

    if (gesture->state() == Qt::GestureStarted && !m_pinchInProgress) {
        gesture->setGestureCancelPolicy(QGesture::CancelAllInContext);
        m_pinchInProgress = true;
        m_pinchCenter = mapFromScene(gesture->centerPoint());
        pinchStarted(m_pinchCenter);
        pinchWidget()->setTransformOriginPoint(m_pinchCenter);
        qDebug() << "pinchStarted" << m_pinchCenter << gesture << objectName();
        m_lastZoomFactor = 1.0;
    }

    if (gesture->state() == Qt::GestureFinished || gesture->state() == Qt::GestureCanceled) {
        m_endScale = pinchUpdated(pinchWidget()->scale());
        m_lastZoomFactor = 1.0;

        qDebug() << "pinchFinished" << m_pinchCenter << m_endScale << pinchWidget()->scale();
        // in case we zoomed over or under the min/max zoom level bounce back
        if (m_endScale != pinchWidget()->scale()) {
            setupBounceAnimation();
            if (m_endScale > pinchWidget()->scale()) {
                emit updateZoomLevel(ActionPool::ZoomFitToPage);
                pinchFinished(m_pinchCenter, m_endScale);
                bounceAnimation->setStartValue(pinchWidget()->scale());
                bounceAnimation->setEndValue(1.0);
                bounceAnimation->start();
                m_pinchInProgress = false;
            }
            else {
                emit updateZoomLevel(ActionPool::ZoomLastUserDefined);
                m_lastZoom.setUserDefined(true);
                bounceAnimation->setStartValue(pinchWidget()->scale());
                bounceAnimation->setEndValue(m_endScale);
                bounceAnimation->start();
                connect(bounceAnimation, SIGNAL(finished()), this, SLOT(bounceAnimationFinished()));
            }
        }
        else {
            emit updateZoomLevel(ActionPool::ZoomLastUserDefined);
            m_lastZoom.setUserDefined(true);
            finishZoom();
        }
    }

    if (gesture->state() == Qt::GestureUpdated) {
        if (qAbs(m_lastZoomFactor - gesture->totalScaleFactor()) > 0.005) {
            qDebug() << "pinch zoom" << gesture->totalScaleFactor() << m_lastZoomFactor << gesture->gestureCancelPolicy();
            qreal factor = gesture->totalScaleFactor();
            qreal updateFactor = pinchUpdated(factor);
            // go to all pages view after min zoom factor has been reached
            if (updateFactor != factor) {
                qDebug() << __PRETTY_FUNCTION__ << updateFactor << factor;
                // if the factor is below 
                if (updateFactor > factor) {
                    factor = updateFactor - (updateFactor - factor) / 3.0;
                }
                else if (factor > updateFactor) {
                    factor = updateFactor + (factor - updateFactor) / 5.0;
                }

                if (updateFactor > factor * 1.4) {
                    factor = updateFactor / 1.4;
                }
                else if (factor > updateFactor * 2.0) {
                    factor = updateFactor * 2.0;
                }
            }
            m_lastZoomFactor = gesture->totalScaleFactor();

            pinchWidget()->setScale(factor);
        }
    }

    startAutoHideTimer();

    event->accept(gesture);
}


void DocumentPage::tapAndHoldGestureEvent(QGestureEvent *event, QTapAndHoldGesture *gesture)
{
    if (!pageLoaded) {
        return;
    }

    if (searchActive()) {
        findtoolbar->hideVkb();
    }

    if(gesture->state() == Qt::GestureFinished)
        longTap(mapFromScene(gesture->position()));

    startAutoHideTimer();

    event->accept(gesture);
}

void DocumentPage::tapGestureEvent(QGestureEvent *event, QTapGesture *gesture)
{
    if (!pageLoaded) {
        return;
    }

    Q_UNUSED(event);

    // the scene has to be rotated and the position has th be updated to take to navigationbar into account
    // so that we get the coordinates correctly for the viewer
    //QPointF position = mapFromScene(gesture->position());// - exposedContentRect().topLeft();
    m_position = gesture->position();
    qDebug() << __PRETTY_FUNCTION__ << gesture->position() << m_position << exposedContentRect();

    if(gesture->state() == Qt::GestureStarted) {
        m_startPoint = m_position;
    }

    if(gesture->state() == Qt::GestureFinished) {
        if (searchActive()) {
            if (findtoolbar->matchFound()) {
                if (!findtoolbar->hideVkb()) {
                    hideFindToolbar();
                }
            }
            else {
                hideFindToolbar();
            }
            return;
        }

        if (jumpActive()) {
            jumpToPageOverlay->hide();
            pageIndicator->show();
            setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Show);
            return;
        }

        QPointF diff = m_position - m_startPoint;
        if(diff.manhattanLength() < 3) {
            if (m_shortTapTimer.isActive()) {
                m_shortTapTimer.stop();
                doubleTap(QRectF(m_position, m_startPoint).center());
             } else {
                m_shortTapTimer.start();
             }
        }
    }

    event->accept(gesture);
}

void DocumentPage::showNextPage()
{
//  decrement done to match page count with viewer
    if(currentPage <= totalPage-1)
        showPageIndex(currentPage);
}

void DocumentPage::showPrevPage()
{
    if(currentPage-2 >= 0) {
        showPageIndex(currentPage-2);
    }
}

bool DocumentPage::showPageIndex(int pageIndex)
{
    if (pageIndex >= totalPage || pageIndex < 0) {
        showInfoBanner(qtTrId("qtn_offi_type_correct_page").arg(QString::number(1)).arg(QString::number(totalPage)));
        return false;
    }
    hideInfoBanner();
    showPageIndexInternal(pageIndex);
    return true;
}

int DocumentPage::pageCount() const
{
    return totalPage;
}

void DocumentPage::showInfoBanner(const QString &message)
{
    if (!m_infoBanner) {
        m_infoBanner = new MBanner();
        m_infoBanner->setStyleName("InformationBanner");
        m_infoBanner->setTitle(message);
    }
    m_infoBanner->appear(MApplication::activeWindow());
}

void DocumentPage::hideInfoBanner()
{
    if (m_infoBanner && m_infoBanner->isVisible()) {
        m_infoBanner->disappear();
    }
}

void DocumentPage::set120percentZoom()
{
    ZoomLevel newZoom(ZoomLevel::FactorMode, 1.2, false);
    zoom(newZoom);
}

void DocumentPage::setPageCounters(int total, int page)
{
    Q_ASSERT(pageIndicator);
    if( pageLoaded && (1 == total)) {
        removeAction(ActionPool::instance()->getAction(ActionPool::EnterPageNumber));
        removeAction(ActionPool::instance()->getAction(ActionPool::ShowAllPagesView));
    }

    currentPage = page;

    totalPage = total;

    pageIndicator->setPageCounters(total, page);
}

void DocumentPage::SetFullscreen()
{
    qDebug() << __PRETTY_FUNCTION__ ;
}

void DocumentPage::SetNormalscreen()
{
    qDebug() << __PRETTY_FUNCTION__ ;
}

void DocumentPage::onClose()
{
    if(!pageLoaded) {
        return;
    }

    setEscapeMode(MApplicationPageModel::EscapeCloseWindow);
    setTitle(qtTrId("qtn_comm_appname_offi"));
    emit closeDocumentPage();
}

void DocumentPage::createSearchToolBar()
{
    if (!findtoolbar) {
        findtoolbar = new FindToolbar(this);
        connect(findtoolbar, SIGNAL(findFirst()), this, SLOT(findFirst()));
        connect(findtoolbar, SIGNAL(findPrevious()), this, SLOT(findPrevious()));
        connect(findtoolbar, SIGNAL(findNext()), this, SLOT(findNext()));
    }
    pageIndicator->hide();
    setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
    findtoolbar->show();
    emit visibleAreaChanged();
}

void DocumentPage::findFirst()
{
    QString searchKey = findtoolbar->text();
    qDebug()<<"******** FindFirst";
    searchStarted = true;

    noMatches = false;
    if(searchKey != searchstring) {
        clearSearchTexts();
        searchstring = searchKey;
        if (searchKey.size() > 0) {
            searchingTimeout = false;
            searchTimer.start();
        }
        else {
            // kill the search timer when it is running
            searchTimer.stop();
        }
    }
}

void DocumentPage::findNext()
{
    qDebug()<<"\n****findNext"<<searchstring;
    //searchstring = textEdit->text();
    searchText(DocumentPage::SearchNext, searchstring);
}

void DocumentPage::findPrevious()
{
    //qDebug()<<"\n****findPrevious"<<searchstring;
    //searchstring = textEdit->text();
    searchText(DocumentPage::SearchPrevious, searchstring);
    searchTimer.stop();
}

void DocumentPage::searchTimeout()
{
    if(searchingTimeout == false) {
        //textEdit->clearFocus();
        searchstring = findtoolbar->text();
        findtoolbar->showSearchIndicator();
        qDebug()<<"*timeout"<<searchstring;
        searchText(DocumentPage::SearchFirst, searchstring);
        searchTimer.stop();
        searchingTimeout = true;
    }
}

void DocumentPage::matchesFound(bool found)
{
    if (searchActive()) {
        noMatches = !found;
        findtoolbar->setMatchFound(found);
    }
}

void DocumentPage::removeActions()
{
    foreach(QAction *action, actions()) {
        removeAction(action);
    }
}

void DocumentPage::setEscapeButtonCloseMode()
{
    setEscapeMode(MApplicationPageModel::EscapeCloseWindow);
}

void DocumentPage::createIndicatorCombo(MAction *label, MAction *one, MAction *two, MAction *three)
{
    indicatorCombobox = new MComboBox();
    indicatorCombobox->setObjectName("documentpage_indicatorcombobox");
    indicatorAction = new MWidgetAction(this);
    Q_CHECK_PTR(indicatorAction);
    indicatorAction->setObjectName("documentpage_indicatorcombobox_widgetaction");

    indicatorCombobox->setTitle(label->text());

    indicatorAction->setLocation(MAction::ApplicationMenuLocation);
    connect(indicatorCombobox, SIGNAL(activated(int)), this, SLOT(indicatorButtonClicked(int)));

    indicatorCombobox->addItem(one->text());
    indicatorCombobox->addItem(two->text());
    indicatorCombobox->addItem(three->text());
    indicatorCombobox->setCurrentIndex(0);

    indicatorAction->setWidget(indicatorCombobox);
    addAction(indicatorAction);

}

void DocumentPage::indicatorButtonClicked(int buttonId)
{
    if(!pageLoaded) {
        indicatorCombobox->setCurrentIndex(0);
        return;
    }

    indicatorCombobox->setCurrentIndex(buttonId);

    switch(indicatorCombobox->currentIndex()) {

    case 0: //SpreadSheetFixedIndicators
        ActionPool::instance()->getAction(ActionPool::SpreadSheetFixedIndicators)->trigger();
        break;

    case 1://SpreadSheetFloatingIndicators
        ActionPool::instance()->getAction(ActionPool::SpreadSheetFloatingIndicators)->trigger();
        break;

    case 2://SpreadSheetNoIndicators
        ActionPool::instance()->getAction(ActionPool::SpreadSheetNoIndicators)->trigger();
        break;
    }
}

void DocumentPage::createCombo(MAction *label, MAction *one, MAction *two, MAction *three)
{
    zoomCombobox = new MComboBox();
    zoomCombobox->setObjectName("documentpage_zoomcombobox");
    zoomAction = new MWidgetAction(this);
    Q_CHECK_PTR(zoomAction);
    zoomAction->setObjectName("documentpage_zoomcombobox_widgetaction");

    zoomCombobox->setTitle(label->text());

    zoomAction->setLocation(MAction::ApplicationMenuLocation);
    connect(zoomCombobox, SIGNAL(activated(int)), this, SLOT(zoomButtonClicked(int)));
    connect(zoomCombobox, SIGNAL(clicked()), this, SLOT(zoomComboClicked()));
    connect(zoomCombobox, SIGNAL(dismissed()), this, SLOT(startTimerToRestoreZoomLevelText()));

    zoomCombobox->addItem(one->text());
    zoomCombobox->addItem(two->text());
    zoomCombobox->addItem(three->text());

    zoomCombobox->setCurrentIndex(0);

    zoomAction->setWidget(zoomCombobox);
    addAction(zoomAction);
}

void DocumentPage::zoomButtonClicked(int buttonId)
{
    if(!pageLoaded) {
        zoomCombobox->setCurrentIndex(0);
        return;
    }

    zoomCombobox->setCurrentIndex(buttonId);

    //Triger this widget action

    switch(zoomCombobox->currentIndex()) {

    case 0: //FitToWidth
        ActionPool::instance()->getAction(ActionPool::ZoomFitToWidth)->trigger();
        break;

    case 1://FitToPage
        ActionPool::instance()->getAction(ActionPool::ZoomFitToPage)->trigger();
        break;

    case 2://Zoom100Percent(Actual size)
        ActionPool::instance()->getAction(ActionPool::Zoom100percent)->trigger();
        break;
    }
}

void DocumentPage::zoomComboClicked()
{
    restoreZoomText = zoomCombobox->currentText();
    //Restoring original text
    zoomCombobox->setItemText(0, ActionPool::instance()->getAction(ActionPool::ZoomFitToWidth)->text());
    zoomCombobox->setItemText(1, ActionPool::instance()->getAction(ActionPool::ZoomFitToPage)->text());
    zoomCombobox->setItemText(2, ActionPool::instance()->getAction(ActionPool::Zoom100percent)->text());
}


void DocumentPage::updateZoomCombobox(ActionPool::Id item)
{
    int index = -1;

    switch(item) {

    case ActionPool::ZoomFitToWidth:
        index = 0;
        break;

    case ActionPool::ZoomFitToPage:
        index = 1;
        break;

    case ActionPool::ZoomLastUserDefined:
        index = zoomCombobox->currentIndex();
        break;

    default:
        return;
    }

    QString text = ActionPool::instance()->getAction(item)->text();
    qDebug() << __PRETTY_FUNCTION__ << text;

    zoomCombobox->setCurrentIndex(index);
    zoomCombobox->setItemText(index, text);
}

void DocumentPage::startTimerToRestoreZoomLevelText()
{
    QTimer::singleShot(500, this, SLOT(restoreZoomLevelText()));
}

void DocumentPage::restoreZoomLevelText()
{
    zoomCombobox->setItemText(zoomCombobox->currentIndex(), restoreZoomText);
}

void DocumentPage::shortTapEvent()
{
    qDebug() << __PRETTY_FUNCTION__ << "shortTap" << m_position;
    shortTap(m_position, this);
}

void DocumentPage::shortTap(const QPointF &point, QObject *object)
{
    Q_UNUSED(point);
    Q_UNUSED(object);
    qDebug() << __PRETTY_FUNCTION__ << searchActive();

    if(componentDisplayMode( MApplicationPage::AllComponents ) == MApplicationPageModel::Hide) {
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Show);
        pageIndicator->show();
        m_autoHideTimer.start();
    } else {
        m_autoHideTimer.stop();
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
        pageIndicator->hide();
    }

    SetNormalscreen();
}

void DocumentPage::longTap(QPointF point)
{
    //qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(point);
}

void DocumentPage::doubleTap(QPointF point)
{
    qDebug() << __PRETTY_FUNCTION__ << m_lastZoom.isUserDefined();
    Q_UNUSED(point);
    if(m_lastZoom.isUserDefined()) {
        emit updateZoomLevel(m_defaultZoomLevelAction);
        ActionPool::instance()->getAction(m_defaultZoomLevelAction)->trigger();
    } else {
        emit updateZoomLevel(ActionPool::ZoomLastUserDefined);
        ActionPool::instance()->getAction(ActionPool::ZoomLastUserDefined)->trigger();
    }
}

void DocumentPage::documentsChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
    Q_UNUSED(bottomRight);
    qDebug() << __PRETTY_FUNCTION__;

    if(topLeft.sibling(topLeft.row(), 1).data().toString().isNull()) {
        appMenuMFAction->setVisible(true);
        appMenuUFAction->setVisible(false);
    } else {
        appMenuMFAction->setVisible(false);
        appMenuUFAction->setVisible(true);
    }

    if (0 == liveDocument->model()->rowCount()) {
        qWarning() << "Document was deleted......";
        exit(0);
    }
}

void DocumentPage::changeMenus()
{
    if(DocumentListModel::documentIsFavorite(getDocumentUrn(documentName))) {
        appMenuMFAction->setVisible(false);
        appMenuUFAction->setVisible(true);
    } else {
        appMenuMFAction->setVisible(true);
        appMenuUFAction->setVisible(false);
    }
}

void DocumentPage::slotJumpToPage()
{
    if (!jumpToPageOverlay) {
        jumpToPageOverlay = new JumpToToolbar(this);
    }
    jumpToPageOverlay->show();
    pageIndicator->hide();
    setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
    m_autoHideTimer.stop();
}

void DocumentPage::updatePageIndicatorName(const QString &name)
{
    if(pageIndicator)
        pageIndicator->setSheetName(name);
}
void DocumentPage::setOpeningProgress(int value)
{
    Q_UNUSED(value);
}

void DocumentPage::autoHideToolbar()
{
    if (! MApplication::activeApplicationWindow()->isMenuOpen()) {
        setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Hide);
        pageIndicator->hide();
    }
}

void DocumentPage::setupBounceAnimation()
{
    if (!bounceAnimation) {
        bounceAnimation = new QPropertyAnimation(this);
        bounceAnimation->setTargetObject(pinchWidget());
        bounceAnimation->setPropertyName("scale");
        bounceAnimation->setStartValue(1.0f);
        bounceAnimation->setEndValue(1.0f);
        bounceAnimation->setDuration(300);
        bounceAnimation->setEasingCurve(QEasingCurve(QEasingCurve::OutQuint));
    }
}

void DocumentPage::bounceAnimationFinished()
{
    qDebug() << __PRETTY_FUNCTION__;
    disconnect(bounceAnimation, SIGNAL(finished()), this, SLOT(bounceAnimationFinished()));
    finishZoom();
}

void DocumentPage::finishZoom()
{
    qDebug() << __PRETTY_FUNCTION__ << "pinchFinished" << m_pinchCenter << m_endScale;
    pinchWidget()->setScale(1.0);
    pinchFinished(m_pinchCenter, m_endScale);
    m_pinchInProgress = false;
}

QRectF DocumentPage::visibleRect() const
{
    // Depending on showing status bar
    QRectF rect = exposedContentRect();
    if (searchActive()) {
        QRectF findRect = findtoolbar->geometry();
        QRect inputArea(MInputMethodState::instance()->inputMethodArea());

        qDebug() << "visibleRect" << rect << findRect << findRect.topRight() << inputArea << mapFromScene(inputArea.topLeft());
        rect.setTopLeft(findRect.bottomLeft());

        if (inputArea.isValid()) {
            rect.setBottom(mapFromScene(inputArea.topLeft()).y());
        }
    }
    else if (quickViewToolbar) {
        qDebug() << "quickViewToolbar" << quickViewToolbar->geometry().bottomLeft() << quickViewToolbar->geometry();
        rect.setTopLeft(quickViewToolbar->geometry().bottomLeft());
    }
    qDebug() << "visibleRect" << rect << MInputMethodState::instance()->inputMethodArea();
    return rect;
}

void DocumentPage::hidePageIndicator()
{
    if (pageIndicator->isVisible()) {
        pageIndicator->hide();
    }
}

void DocumentPage::startAutoHideTimer()
{
    if (componentDisplayMode(NavigationBar) != MApplicationPageModel::Hide) {
        m_autoHideTimer.start();
    }
}

bool DocumentPage::searchActive() const
{
    return findtoolbar && findtoolbar->isActive();
}

void DocumentPage::hideFindToolbar()
{
    searchTimer.stop();
    clearSearchTexts();
    findtoolbar->hide();
    searchstring.clear();
    pageIndicator->show();
    setComponentsDisplayMode(MApplicationPage::AllComponents, MApplicationPageModel::Show);
    m_autoHideTimer.start();
    emit visibleAreaChanged();
}

bool DocumentPage::jumpActive() const
{
    return jumpToPageOverlay && jumpToPageOverlay->isVisible();
}

void DocumentPage::showPageIndexDefaultZoom(int pageIndex)
{
    m_blockRecenter = true;
    ActionPool::instance()->getAction(m_defaultZoomLevelAction)->trigger();
    m_blockRecenter = false;
    emit updateZoomLevel(m_defaultZoomLevelAction);
    showPageIndex(pageIndex);
    ActionPool::instance()->getAction(ActionPool::ShowNormalView)->trigger();
}
