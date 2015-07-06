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
//Include QT stuff
#include <QGraphicsLinearLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QScrollBar>
#include <QTextBlock>
#include <QGraphicsSceneMouseEvent>
#include <QTextLayout>
#include <QPropertyAnimation>
#include <QtDBus>

//Include M stuff
#include <MPannableViewport>
#include <MPositionIndicator>

#include <mce/mode-names.h>
#include <mce/dbus-names.h>

#include <contextsubscriber/contextproperty.h>

//Include Koffice stuff
#include <KoCanvasControllerWidget.h>
#include <KoView.h>
#include <KoToolManager.h>
#include <KoViewConverter.h>
#include <KoZoomController.h>
#include <KoCanvasBase.h>
#include <KoTextShapeData.h>
#include <KoPADocument.h>
#include <KoPAPageBase.h>
#include <KoZoomHandler.h>
#include <KoPACanvasBase.h>
#include <KoPACanvasItem.h>
#include <KoPageApp.h>
#include <KoPAViewModeNormal.h>
#include <KoPAUtil.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoShapeLayer.h>
#include <KoSelection.h>
#include <KoPAPage.h>
#include <KoPAMasterPage.h>
#include <KoZoomAction.h>
#include <KoPAView.h>

//Include application stuff
#include "officeviewerpresentation.h"
#include "definitions.h"
#include "applicationwindow.h"
#include "actionpool.h"
#include "thumbprovider.h"
#include "misc.h"
#include "pannablescrollbars.h"
#include "officeviewereventfilter.h"

OfficeViewerPresentation::OfficeViewerPresentation(SlideAnimator *slideAnimator, QGraphicsWidget *parent)
    : OfficeViewer(parent)
    , m_canvasItem(0)
    , m_zoomController(0)
    , m_currentPage(0)
    , m_actionCollection(new KActionCollection(this))
    , m_currentPageNr(0)
    , m_pageCount(0)
    , m_lastUserDefinedFactor(1.0)
    , m_searchIndex(0)
    , m_slideAnimator(slideAnimator)
{
    setObjectName("officeviewerpresentation");
    m_pannableScrollbars = 0;
    //TODO these values aren't defined in the UI specification yet
    //once they are decided, put proper values here.
    m_highlight.setBackground(QBrush(highlightColor));
    m_highlightCurrent.setBackground(QBrush(highlightColorCurrent));

    QObject::connect(ApplicationWindow::GetSceneManager(), SIGNAL(orientationChangeFinished(const M::Orientation &)),
                     this, SLOT(orientationChanged()));

    m_zoomLevel = ZoomLevel(ZoomLevel::FitToWidth, 1.0, false);

    m_actionCollection->addAction(KStandardAction::Prior, "page_previous", this, SLOT(goToPreviousPage()));
    m_actionCollection->addAction(KStandardAction::Next, "page_next", this, SLOT(goToNextPage()));
    m_actionCollection->addAction(KStandardAction::FirstPage, "page_first", this, SLOT(goToFirstPage()));
    m_actionCollection->addAction(KStandardAction::LastPage, "page_last", this, SLOT(goToLastPage()));

    tvout = new ContextProperty("/com/nokia/policy/video_route", this);
    QObject::connect(tvout, SIGNAL(valueChanged()), this, SLOT(tvoutConnected()));
    tvoutConnected();

    preventBlankTimer.setInterval(60000);
    QObject::connect(&preventBlankTimer, SIGNAL(timeout()), this, SLOT(preventBlanking()));

    QObject::connect(m_slideAnimator, SIGNAL(animationNextFinished()), this, SLOT(animationNextFinished()));
    QObject::connect(m_slideAnimator, SIGNAL(animationPreviousFinished()), this, SLOT(animationPreviousFinished()));
    QObject::connect(m_slideAnimator, SIGNAL(animationCanceled()), this, SLOT(animationCanceled()));
}


OfficeViewerPresentation::~OfficeViewerPresentation()
{
    if(m_document) {
        delete m_document;
        m_document= 0;
    }

    delete m_actionCollection;

    m_actionCollection = 0;
}

KoViewConverter * OfficeViewerPresentation::viewConverter(KoPACanvasBase * canvas)
{
    Q_ASSERT(viewMode());
    return viewMode()->viewConverter(canvas);
}


KoPACanvasBase * OfficeViewerPresentation::kopaCanvas() const
{
    return m_canvasItem;
}

KoPADocument * OfficeViewerPresentation::kopaDocument() const
{
    return static_cast<KoPADocument*>(m_document);
}

KoZoomController * OfficeViewerPresentation::zoomController() const
{
    return m_zoomController;
}

void OfficeViewerPresentation::doUpdateActivePage(KoPAPageBase * page)
{
    qDebug() << "UpdateActivePage. width" << page->pageLayout().width << "height" << page->pageLayout().height;

    // save the old offset into the page so we can use it also on the new page
    QPoint scrollValue(m_pannableScrollbars->scrollBarValue());
    qDebug() << "scrollValue" << scrollValue;

    bool pageChanged = page != m_currentPage;
    setActivePage(page);

    m_canvasItem->updateSize();
    KoPageLayout &layout = m_currentPage->pageLayout();

    QSizeF pageSize(layout.width, layout.height);

    qDebug() << "pagesize" << pageSize;
    qDebug() << "canvas size" << m_canvasItem->size();
    qDebug() << "document origin" << m_canvasItem->documentOrigin();
    qDebug() << "document offset" << m_canvasItem->documentOffset();

    m_canvasItem->setDocumentOrigin(QPointF(0,0));
    // the page is in the center of the canvas
    m_zoomController->setDocumentSize(pageSize);
    m_zoomController->setPageSize(pageSize);

    m_canvasItem->resourceManager()->setResource(KoCanvasResource::PageSize, pageSize);
    m_canvasItem->update();

    updatePageNavigationActions();

    if(pageChanged) {
        proxyObject->emitActivePageChanged();
    }

    QSize sz = m_canvasItem->viewConverter()->documentToView(pageSize).toSize();

    qDebug() << "Size in pixels" << sz;
    m_pannableScrollbars->setScrollBarValue(scrollValue);
    m_canvasItem->updateSize();
}

void OfficeViewerPresentation::setActivePage(KoPAPageBase * page)
{
    if(!page)
        return;

    if(m_currentPage) {
        m_canvasItem->shapeManager()->removeAdditional(m_currentPage);
    }

    m_currentPage = page;

    m_canvasItem->shapeManager()->addAdditional(m_currentPage);
    QList<KoShape*> shapes = page->shapes();
    m_canvasItem->shapeManager()->setShapes(shapes, KoShapeManager::AddWithoutRepaint);
    //Make the top most layer active

    if(!shapes.isEmpty()) {
        KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>(shapes.last());
        m_canvasItem->shapeManager()->selection()->setActiveLayer(layer);
    }

    // if the page is not a master page itself set shapes of the master page
    KoPAPage * paPage = dynamic_cast<KoPAPage *>(page);

    if(paPage) {
        KoPAMasterPage * masterPage = paPage->masterPage();
        QList<KoShape*> masterShapes = masterPage->shapes();
        m_canvasItem->masterShapeManager()->setShapes(masterShapes, KoShapeManager::AddWithoutRepaint);
        //Make the top most layer active

        if(!masterShapes.isEmpty()) {
            KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>(masterShapes.last());
            m_canvasItem->masterShapeManager()->selection()->setActiveLayer(layer);
        }
    } else {
        // if the page is a master page no shapes are in the masterShapeManager
        m_canvasItem->masterShapeManager()->setShapes(QList<KoShape*>());
    }

    // Set the current page number in the canvas resource provider
    KoPADocument *doc = qobject_cast<KoPADocument*>(m_document);

    m_canvasItem->resourceManager()->setResource(KoCanvasResource::CurrentPage, doc->pageIndex(m_currentPage)+1);
}

KoPAPageBase* OfficeViewerPresentation::activePage() const
{
    return m_currentPage;
}

void OfficeViewerPresentation::navigatePage(KoPageApp::PageNavigation pageNavigation)
{
    KoPADocument *doc = qobject_cast<KoPADocument*>(m_document);
    KoPAPageBase * newPage = doc->pageByNavigation(m_currentPage, pageNavigation);

    if(newPage != m_currentPage) {
        proxyObject->updateActivePage(newPage);
    }
}

void OfficeViewerPresentation::setActionEnabled(int actions, bool enable)
{
    Q_UNUSED(actions);
    Q_UNUSED(enable);
}

void OfficeViewerPresentation::updatePageNavigationActions()
{
    KoPADocument *doc = qobject_cast<KoPADocument*>(m_document);
    int index = doc->pageIndex(activePage());
    int pageCount = doc->pages(viewMode()->masterMode()).count();

    m_actionCollection->action("page_previous")->setEnabled(index > 0);
    m_actionCollection->action("page_first")->setEnabled(index > 0);
    m_actionCollection->action("page_next")->setEnabled(index < pageCount - 1);
    m_actionCollection->action("page_last")->setEnabled(index < pageCount - 1);
}

void OfficeViewerPresentation::insertPage()
{
    // XXX: not needed yet
}

void OfficeViewerPresentation::pagePaste()
{
    // XXX: not needed yet
}

void OfficeViewerPresentation::editPaste()
{
    // XXX: not needed yet
}

void OfficeViewerPresentation::setShowRulers(bool show)
{
    Q_UNUSED(show);
}


void OfficeViewerPresentation::goToPreviousPage()
{
    navigatePage(KoPageApp::PagePrevious);
}

void OfficeViewerPresentation::goToNextPage()
{
    navigatePage(KoPageApp::PageNext);
}

void OfficeViewerPresentation::goToFirstPage()
{
    navigatePage(KoPageApp::PageFirst);
}

void OfficeViewerPresentation::goToLastPage()
{
    navigatePage(KoPageApp::PageLast);
}

void OfficeViewerPresentation::animationNextFinished()
{
    navigatePage(KoPageApp::PageNext);
    ZoomLevel zoomLevel(ZoomLevel::FitToPage, 1.0, false);
    zoom(zoomLevel);
}

void OfficeViewerPresentation::animationPreviousFinished()
{
    navigatePage(KoPageApp::PagePrevious);
    ZoomLevel zoomLevel(ZoomLevel::FitToPage, 1.0, false);
    zoom(zoomLevel);
}

void OfficeViewerPresentation::animationCanceled()
{
    m_pannableScrollbars->updateRange();
}

bool OfficeViewerPresentation::createKoWidget()
{
    if(!m_document) return false;

    KoPADocument *document = qobject_cast<KoPADocument*>(m_document);

    if(!document) return false;

    if(!document->pageCount() > 0) return false;

    m_currentPage = document->pageByIndex(0, false);

    m_currentPageNr = 1;

    // setup canvas controller
    m_pannableScrollbars = new PannableScrollBars(this);

    m_pannableScrollbars->setEnabled(true);

    m_pannableScrollbars->setPanDirection(Qt::Horizontal | Qt::Vertical);

    m_pannableScrollbars->physics()->setBorderSpringK(0.9);
    m_pannableScrollbars->physics()->setBorderFriction(0.8);
    m_pannableScrollbars->physics()->setSlidingFriction(0.01);

    m_pannableScrollbars->setClipping(true);

    //m_pannableScrollbars->setMargin(10);

    connect(m_pannableScrollbars, SIGNAL(topReached(const QPointF &)),
            this, SLOT(topReached(const QPointF &)));
    connect(m_pannableScrollbars, SIGNAL(bottomReached(const QPointF &)),
            this, SLOT(bottomReached(const QPointF &)));

    // Get the canvas
    m_canvasItem = dynamic_cast<KoPACanvasItem*>(document->canvasItem());
    if(!m_canvasItem || !m_canvasItem->resourceManager()) {
        return false;
    }

    m_canvasItem->setAttribute(Qt::WA_OpaquePaintEvent, true);
    m_canvasItem->setAutoFillBackground(false);
    m_canvasItem->installEventFilter(new OfficeViewerEventFilter(this));

    m_canvasItem->setView(this);

    m_canvasItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);

    KoPAViewMode * viewMode = new KoPAViewModeNormal(this, m_canvasItem);
    setViewMode(viewMode);
    m_pannableScrollbars->setCanvasMode(KoCanvasController::Infinite);

    m_zoomController = new KoZoomController(m_pannableScrollbars, zoomHandler(), m_actionCollection, 0, this);

    m_pannableScrollbars->setCanvas(m_canvasItem);
    const KoZoomHandler *zoomHandler = dynamic_cast<const KoZoomHandler*>(m_canvasItem->viewConverter());
    m_pannableScrollbars->setZoomHandler(zoomHandler);

    KoToolManager::instance()->addController(m_pannableScrollbars);

    connect(m_pannableScrollbars->proxyObject, SIGNAL(moveDocumentOffset(QPoint)),
            this, SLOT(setDocumentOffset(QPoint)));

    m_zoomController->zoomAction()->setZoomModes(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE);

    // Connect the tool manager
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*, int)), this, SLOT(activeToolChanged(KoCanvasController*, int)));
    KoToolManager::instance()->switchToolRequested(PanToolID);

    connect(m_canvasItem->resourceManager(), SIGNAL(resourceChanged(int, const QVariant &)),  this, SLOT(resourceChanged(int, const QVariant &)));

    updateSizes();
    updatePageNumbers();

    zoom(m_zoomLevel);

    doUpdateActivePage(m_currentPage);
    return true;
}


void OfficeViewerPresentation::updateSizes()
{
    QSizeF size=ApplicationWindow::visibleSize();
    qDebug() << __PRETTY_FUNCTION__ << size;

    if(m_pannableScrollbars) {
        QPointF position = m_pannableScrollbars->position();
        m_pannableScrollbars->updateRange();
        m_pannableScrollbars->setMinimumSize(size);
        m_pannableScrollbars->setMaximumSize(size);
        m_pannableScrollbars->setPosition(position);
    }
    qDebug() << __PRETTY_FUNCTION__ << "finished";
}

void OfficeViewerPresentation::orientationChanged()
{
    updateSizes();
    if (m_zoomController) {
        qreal effectiveZoomFactor = m_zoomController->zoomAction()->effectiveZoom();
        qreal fitToPageZoomFactor = minimumZoomFactor();
        qDebug() << __PRETTY_FUNCTION__ << effectiveZoomFactor << fitToPageZoomFactor;
        if (effectiveZoomFactor < fitToPageZoomFactor) {
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
            emit updateZoomLevel(ActionPool::ZoomFitToPage);
        }
    }
}

QGraphicsLayoutItem *OfficeViewerPresentation::getGraphicsLayoutItem()
{
    return m_pannableScrollbars;
}

void OfficeViewerPresentation::updatePageNumbers()
{
    int newPageCount = m_document->pageCount();
    emit pageChanged(newPageCount, m_currentPageNr);

    if(newPageCount != m_pageCount) {
        //Lets try reading page numbers little bit later
        QTimer::singleShot(KofficePageNumberUpdateIntervalTime, this, SLOT(updatePageNumbers()));
    }

    m_pageCount = newPageCount;
}

void OfficeViewerPresentation::zoom(const ZoomLevel &newLevel)
{
    if(m_zoomController && m_document) {
        qDebug() << __PRETTY_FUNCTION__;

        qreal factor = 0;
        KoZoomAction *zAction = m_zoomController->zoomAction();

        switch(newLevel.getMode()) {

        case ZoomLevel::FitToPage:
            qDebug() << "Zooming fit to page";
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
            m_pannableScrollbars->setPosition(QPointF(0, 0));
            break;

        case ZoomLevel::FitToHeight:
            qDebug() << "Zooming fit to height";
            //No Fit to height support so lets use best available
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
            break;

        case ZoomLevel::FitToWidth:
            qDebug() << "Zooming fit to width";
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_WIDTH);
            break;

        case ZoomLevel::FactorMode:
            qDebug() << "Zooming factormode";

            if(newLevel.getFactor(factor)) {
                if(factor >= minimumZoomFactor() && factor <= MaxZoomFactor) {
                    zAction->setEffectiveZoom(factor);
                }
            }
        case ZoomLevel::Relative:

            qDebug() << "relative zoom level";
            qDebug() << "document offset" << m_canvasItem->documentOffset();
            qDebug() << "PREFFERED CENTER " << m_pannableScrollbars->preferredCenter();

            if(newLevel.getFactor(factor)) {
                m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, m_zoomController->zoomAction()->effectiveZoom() * factor);
            }

            break;
        }

        if(m_zoomLevel.isUserDefined()) {
            qreal current = zAction->effectiveZoom();
            qDebug() << "effectiveZoom" << current;

            if (newLevel.getMode() == ZoomLevel::Relative) {
                current =  zAction->effectiveZoom() * factor;
            }

            qreal fitToPageZoomFactor = minimumZoomFactor();

            if(current < fitToPageZoomFactor) {
                //We are under zoom out limit so we set minimum scale
                zAction->setEffectiveZoom(fitToPageZoomFactor);
                ActionPool::instance()->getAction(ActionPool::ShowAllPagesView)->trigger();
            } else if(current > MaxZoomFactor) {
                //We are over zoom in limit so we set maximum scale
                zAction->setEffectiveZoom(MaxZoomFactor);
            }
        }

        m_zoomLevel = newLevel;

        if(m_zoomLevel.isUserDefined()) {
            m_lastUserDefinedFactor = zAction->effectiveZoom();
            ActionPool::instance()->setUserDefinedZoomFactor(m_lastUserDefinedFactor);
        }
    }
}

void OfficeViewerPresentation::resourceChanged(int key, const QVariant &value)
{
    if(KoCanvasResource::CurrentPage == key) {
        m_currentPageNr = value.toInt();
        updatePageNumbers();
    }
}

void OfficeViewerPresentation::centerToResult(int index)
{
    if (index < 0 || index >= m_searchResults.size()) {
        return;
    }

    KoPADocument* padoc = qobject_cast<KoPADocument*>(m_document);

    KoShape *shape = m_searchResults[index].shape;
    KoPAPageBase* page = m_searchResults[index].page;

    if (!padoc || !shape  || !page) {
        return;
    }

    doUpdateActivePage(page);
    QRectF visibleRect(textSelectionRect(m_searchResults[index].shape, m_searchResults[index].startPosition, m_searchResults[index].length));
    m_pannableScrollbars->ensureVisible(m_canvasItem->viewConverter()->documentToView(visibleRect));
}


void OfficeViewerPresentation::scrollTo(int pageIndex, const QPointF &position)
{
    Q_UNUSED(position);
    KoPADocument *doc = qobject_cast<KoPADocument*>(m_document);
    KoPAPageBase* page = doc->pageByIndex(pageIndex, false);
    doUpdateActivePage(page);
}

void OfficeViewerPresentation::showPage(int pageIndex)
{
    scrollTo(pageIndex,QPointF());
}

void OfficeViewerPresentation::bottomReached(const QPointF &lastPosition)
{
    prepareAnimation(lastPosition, KoPageApp::PageNext, SlideAnimator::Next);
}

void OfficeViewerPresentation::animateSlideTop()
{
    disconnect(m_pannableScrollbars, SIGNAL(panningStopped()), this, SLOT(animateSlideTop()));
    qDebug() << __PRETTY_FUNCTION__ << m_pannableScrollbars->position().y() << m_slideAnimator->paintOffset() << m_pannableScrollbars->panDirection();
    if (m_pannableScrollbars->panDirection() == PannableScrollBars::PanDown) {
        m_slideAnimator->slide(m_pannableScrollbars, SlideAnimator::Next);
    }
    else {
        const KoPageLayout &layout = m_currentPage->pageLayout();
        QSizeF pageSize(layout.width, layout.height );
        QSize sz = m_canvasItem->viewConverter()->documentToView(pageSize).toSize();
        QPixmap currentPage = m_currentPage->thumbnail(sz);

        qDebug() << __PRETTY_FUNCTION__ << m_canvasItem->pos() << m_pannableScrollbars->position() << m_canvasItem->pos().isNull() << m_pannableScrollbars->hasOffset() << m_pannableScrollbars->paintOffset();
        QPointF offset(0, m_pannableScrollbars->hasOffset() ? -(m_canvasItem->pos().y() + m_pannableScrollbars->position().y()) : -m_canvasItem->pos().y());
        if (m_canvasItem->pos().x() > 0) {
            offset.setX(m_canvasItem->pos().x());
            qDebug() << __PRETTY_FUNCTION__ << "1" <<offset;
        }
        else {
            offset.setX(-m_pannableScrollbars->position().x());
            qDebug() << __PRETTY_FUNCTION__ << "2" <<offset;
        }

        m_slideAnimator->slideCancel(m_pannableScrollbars, currentPage, offset, SlideAnimator::Next);
    }
}

void OfficeViewerPresentation::topReached(const QPointF &lastPosition)
{
    prepareAnimation(lastPosition, KoPageApp::PagePrevious, SlideAnimator::Previous);
}

void OfficeViewerPresentation::animateSlideBottom()
{
    disconnect(m_pannableScrollbars, SIGNAL(panningStopped()), this, SLOT(animateSlideBottom()));
    qDebug() << __PRETTY_FUNCTION__ << m_pannableScrollbars->position() << m_pannableScrollbars << m_slideAnimator->paintOffset() << m_pannableScrollbars->panDirection();
    if (m_pannableScrollbars->panDirection() == PannableScrollBars::PanUp) {
        m_slideAnimator->slide(m_pannableScrollbars, SlideAnimator::Previous);
    }
    else {
        const KoPageLayout &layout = m_currentPage->pageLayout();
        QSizeF pageSize(layout.width, layout.height );
        QSize sz = m_canvasItem->viewConverter()->documentToView(pageSize).toSize();
        QPixmap currentPage = m_currentPage->thumbnail(sz);

        QPointF offset(0, m_canvasItem->pos().y());
        if (m_canvasItem->pos().x() > 0) {
            offset.setX(m_canvasItem->pos().x());
        }
        else {
            offset.setX(-m_pannableScrollbars->position().x());
        }

        m_slideAnimator->slideCancel(m_pannableScrollbars, currentPage, offset, SlideAnimator::Previous);
    }
}

void OfficeViewerPresentation::prepareAnimation(const QPointF &lastPosition, KoPageApp::PageNavigation navigation, SlideAnimator::Direction direction)
{
    //check if there is a next page
    KoPADocument *doc = qobject_cast<KoPADocument*>(m_document);
    KoPAPageBase *newPage = doc->pageByNavigation(m_currentPage, navigation);

    if (newPage == m_currentPage) {
        return;
    }

    qDebug() << __PRETTY_FUNCTION__;
    QSize size = ApplicationWindow::visibleSize();
    // TODO make sure the thumbnail is only as big as the page

    KoZoomHandler zoomHandler;
    const KoPageLayout &layout = newPage->pageLayout();
    KoPAUtil::setZoom(layout, size, zoomHandler);
    QRect pageRect(KoPAUtil::pageRect(layout, size, zoomHandler));
    QPixmap slidePixmap = newPage->thumbnail(pageRect.size());
    QPixmap pixmap(size);
    pixmap.fill("#6b6e70");
    QPainter painter(&pixmap);
    painter.drawPixmap(pageRect, slidePixmap, slidePixmap.rect());

    m_slideAnimator->setPixmap(pixmap);
    m_slideAnimator->setDirection(direction);
    m_slideOffset = lastPosition;
    m_slideAnimator->updatePaintOffset(m_slideOffset, m_slideOffset);
    m_slideAnimator->setVisible(true);
    QRectF range = m_pannableScrollbars->range();
    m_pannableScrollbars->physics()->setEnabled(false);
    m_pannableScrollbars->positionIndicator()->setEnabled(false);
    if (direction == SlideAnimator::Next) {
        range.setHeight(range.height() + size.height() + SlideAnimator::animationGap);
    }
    else {
        range.setTop(-(size.height() + SlideAnimator::animationGap));
    }
    m_pannableScrollbars->setRange(range);
    // we need to stop the animation here as otherwise the slide goes out to fast
    m_pannableScrollbars->physics()->stop();

    // if panning already stopped we don't need the connections and can execute the code directly
    if (m_pannableScrollbars->isPanning()) {
        if (direction == SlideAnimator::Next) {
            connect(m_pannableScrollbars, SIGNAL(panningStopped()), this, SLOT(animateSlideTop()));
        }
        else {
            connect(m_pannableScrollbars, SIGNAL(panningStopped()), this, SLOT(animateSlideBottom()));
        }
    }
    else {
        // the single shot it needed that the offset of the slide has been set correctly at the time the 
        // animation is started, which is not the case then calling the method directly.
        if (direction == SlideAnimator::Next) {
            QTimer::singleShot(0, this, SLOT(animateSlideTop()));
        }
        else {
            QTimer::singleShot(0, this, SLOT(animateSlideBottom()));
        }
    }
}

void OfficeViewerPresentation::setCurrentPage(int pageIndex)
{
    if((1 + pageIndex) != m_currentPageNr) {
        m_currentPageNr = pageIndex + 1;
        updatePageNumbers();

    }
}

QSizeF OfficeViewerPresentation::currentDocumentSize()
{
    if (!m_currentPage) {
        return QSize();
    }

    KoPageLayout &layout = m_currentPage->pageLayout();
    QSizeF pageSize(layout.width, layout.height);
    QSize sz = m_canvasItem->viewConverter()->documentToView(pageSize).toSize();
    return sz;
}

void OfficeViewerPresentation::setDocumentOffset(const QPoint &point)
{
    qDebug() << __PRETTY_FUNCTION__ << point << m_slideOffset << m_pannableScrollbars->hasOffset();
    m_canvasItem->setDocumentOffset(QPoint(point.x(), m_pannableScrollbars->hasOffset() ? 0 : point.y()));

    if (m_slideAnimator->isVisible()) {
        m_slideAnimator->updatePaintOffset(point, m_slideOffset);
    }
}

QImage * OfficeViewerPresentation::getThumbnail(int pageNumber)
{
    KoPADocument *document = qobject_cast<KoPADocument*>(m_document);
    KoViewConverter viewConverter;
    viewConverter.setZoom(1.0);

    KoPAPageBase *page = document->pageByIndex(pageNumber, false);

    return (new QImage(page->thumbnail(page->size().toSize()).toImage()));
}

void OfficeViewerPresentation::startSearch(const QString & searchString)
{

    m_searchIndex = 0;
    clearSearchResults();

    if(!m_document || !m_pannableScrollbars || !m_canvasItem) {
        return;
    }

    KoPADocument* padoc = qobject_cast<KoPADocument*>(m_document);

    if(padoc) {
        // loop over all pages starting from current page to get
        // search results in the right order
        int curPage = m_canvasItem->resourceManager()->resource(KoCanvasResource::CurrentPage).toInt()-1;
        QList<QPair<KoPAPageBase*, KoShape*> > textShapes;
        QList<QTextDocument*> textDocs;

        for(int page = 0; page < padoc->pageCount(); page++) {
            KoPAPageBase* papage = padoc->pageByIndex(page, false);
            findTextShapesRecursive(papage, papage, textShapes, textDocs);
        };

        findText(textDocs, textShapes, searchString);

        // now find the first search result in the list of positions counting from the current page
        // this is not very efficient...
        bool foundIt = false;

        for(int page = curPage; page < padoc->pageCount(); page++) {
            for(int i = 0; i < m_searchResults.size(); i++) {
                if(m_searchResults[i].page == padoc->pageByIndex(page, false)) {
                    foundIt = true;
                    m_searchIndex = i;
                    highlightText(m_searchIndex,true);
                    centerToResult(i);
                    break;
                }
            }

            if(foundIt) break;
        }

        if(!foundIt) {
            for(int page = 0; page < curPage; page++) {
                for(int i = 0; i < m_searchResults.size(); i++) {
                    if(m_searchResults[i].page == padoc->pageByIndex(page, false)) {
                        foundIt = true;
                        m_searchIndex = i;
                        highlightText(m_searchIndex,true);
                        centerToResult(i);
                        break;
                    }
                }

                if(foundIt) break;
            }
        }
    }

    emit matchesFound(m_searchResults.size() > 0);
}

void OfficeViewerPresentation::getCurrentVisiblePages(ThumbProvider *thumbProvider)
{
    if(0 != thumbProvider && 0 != m_pannableScrollbars && 0 != m_canvasItem) {
        thumbProvider->clearVisibleAreas();
        thumbProvider->addVisibleAreas(m_currentPageNr, m_canvasItem->contentsRect(), m_canvasItem->size());
    }
}

void OfficeViewerPresentation::pinchStarted(QPointF &center)
{
    m_pannableScrollbars->pinchStarted();
    QSize size = ApplicationWindow::visibleSize();

    if (size.width() > currentDocumentSize().width()) {
        center.rx() = size.width() / 2;
    }

    if (size.height() > currentDocumentSize().height()) {
        center.ry() = size.height() / 2;
    }

    m_pinchCenterDocument = m_canvasItem->viewConverter()->viewToDocument(center + m_pannableScrollbars->position() - m_canvasItem->pos());
}

qreal OfficeViewerPresentation::pinchUpdated(qreal zoomFactor)
{
    qreal effectiveZoomFactor = m_zoomController->zoomAction()->effectiveZoom() * zoomFactor;

    qreal fitToPageZoomFactor = minimumZoomFactor();

    qDebug() << __PRETTY_FUNCTION__ << zoomFactor << effectiveZoomFactor << fitToPageZoomFactor << m_zoomController->zoomAction()->effectiveZoom() << fitToPageZoomFactor / m_zoomController->zoomAction()->effectiveZoom();
    if (fitToPageZoomFactor > effectiveZoomFactor) {
        return fitToPageZoomFactor / m_zoomController->zoomAction()->effectiveZoom();
    }
    else if (effectiveZoomFactor > MaxZoomFactor) {
        return MaxZoomFactor / m_zoomController->zoomAction()->effectiveZoom();
    }
    return zoomFactor;
}

void OfficeViewerPresentation::pinchFinished(const QPointF &center, qreal scale)
{
    qreal effectiveZoomFactor = m_zoomController->zoomAction()->effectiveZoom() * scale;
    qDebug() << "New Zoom:" << effectiveZoomFactor << minimumZoomFactor();
    if (qFuzzyCompare(effectiveZoomFactor, minimumZoomFactor())) {
        m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
    }
    else {
        m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, m_zoomController->zoomAction()->effectiveZoom() * scale);
    }
    m_pannableScrollbars->pinchFinished();

    QPointF newPos = m_canvasItem->viewConverter()->documentToView(m_pinchCenterDocument) - center;
    qDebug() << "XXX finish" << m_pinchCenterDocument << newPos << center << m_zoomController->zoomAction()->effectiveZoom() << m_pannableScrollbars->range();
    // don't try to pan outside of the slide
    if (newPos.y() < 0) {
        newPos.setY(0);
    }
    if (newPos.x() < 0) {
        newPos.setX(0);
    }
    m_pannableScrollbars->setScrollBarValue(newPos.toPoint());
}

void OfficeViewerPresentation::shortTap(const QPointF &point, QObject *object)
{
    if (0 == object || !isLoaded) {
        return;
    }

    QPointF p(m_canvasItem->mapFromScene(point));
    QPointF currentPos(m_pannableScrollbars->position());
    QPointF documentPoint(m_canvasItem->viewConverter()->viewToDocument(p + currentPos - m_canvasItem->pos()));

    qDebug() << __PRETTY_FUNCTION__ << documentPoint;

    // find text shape at current position
    QRectF area(documentPoint, QSizeF(1,1));
    KoShape *selectedShape = 0;
    foreach (KoShape *shape, m_canvasItem->shapeManager()->shapesAt(area, true)) {
        if (qobject_cast<KoTextShapeData*>(shape->userData())) {
            selectedShape = shape;
            break;
        }
    }

    if (selectedShape == 0) {
        return;
    }

    KoSelection *selection = m_canvasItem->shapeManager()->selection();
    selection->select(selectedShape);

    //Lets select texttool
    KoToolManager::instance()->switchToolRequested(TextToolID);

    QGraphicsSceneMouseEvent *event;
    event = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    event->setPos(p);
    event->setButton(Qt::LeftButton);
    event->setButtons(Qt::LeftButton);
    event->setAccepted(false);
    m_canvasItem->mousePressEvent(event);
    delete event;

    event = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    event->setPos(p);
    event->setButton(Qt::LeftButton);
    event->setButtons(Qt::LeftButton);
    event->setAccepted(false);
    m_canvasItem->mouseReleaseEvent(event);
    delete event;

    //Lets select the default tool again
    KoToolManager::instance()->switchToolRequested(PanToolID);
}

QSizeF OfficeViewerPresentation::scaleTo(const QSizeF &original,
                                         qreal value,
                                         bool width)
{
    qreal scale = 0;
    QSizeF result;

    if(width) {
        //Calculate the scale between width and height
        scale = original.height() / original.width();
        //Set the value
        result.setWidth(value);
        //and scale the other value so that aspect ratio is kept intact
        result.setHeight(value * scale);
    } else {
        scale = original.width() / original.height();
        result.setHeight(value);
        result.setWidth(value * scale);
    }

    return result;
}

void OfficeViewerPresentation::highlightText(int index, bool current)
{
    if(index < 0 || index >= m_searchResults.size()) {
        return;
    }

    KoShape *shape = m_searchResults[index].shape;

    if(!shape) {
        return;
    }

    KoTextShapeData* tsd = qobject_cast<KoTextShapeData*> (shape->userData());

    if(!tsd || !tsd->document()) {
        return;
    }

    //Find QTextDocument from text shape's user data
    QTextDocument *doc = tsd->document();

    //Find correct text block
    QTextBlock block = doc->findBlock(m_searchResults[index].startPosition);

    QTextLayout *layout = block.layout();

    QTextLayout::FormatRange range;

    range.start = m_searchResults[index].startPosition - block.position();

    range.length = m_searchResults[index].length;

    //Get the list of additional text formatting that has been set for that
    //range
    QList<QTextLayout::FormatRange> ranges = layout->additionalFormats();

    //Create our own additional text formatting
    //and set the highlight color depending on if this is current result
    //or one of the results
    if(current) {
        range.format = m_highlightCurrent;
    } else {
        range.format = m_highlight;
    }

    //Add our range to the list
    ranges.append(range);

    //and set all those additional formats
    layout->setAdditionalFormats(ranges);

    //then mark contents dirty so it'll get redrawn
    doc->markContentsDirty(m_searchResults[index].startPosition, range.length);
}


void OfficeViewerPresentation::findTextShapesRecursive(KoShapeContainer *con,
                                                       KoPAPageBase *page,
                                                       QList<QPair<KoPAPageBase*, KoShape*> >  &shapes,
                                                       QList<QTextDocument*> &docs)
{
    foreach(KoShape* shape, con->shapes()) {
        KoTextShapeData* tsd = qobject_cast<KoTextShapeData*> (shape->userData());

        if(tsd) {
            shapes.append(qMakePair(page, shape));
            docs.append(tsd->document());
        }

        KoShapeContainer* child = dynamic_cast<KoShapeContainer*>(shape);

        if(child) {
            findTextShapesRecursive(child, page, shapes, docs);
        }
    }
}


void OfficeViewerPresentation::findText(QList<QTextDocument*> docs,
                                        QList<QPair<KoPAPageBase*, KoShape*> > shapes,
                                        const QString &text)
{
    if(docs.isEmpty()) {
        return;
    }

    for(int i = 0; i < docs.size(); i++) {
        QTextDocument* doc = docs.at(i);
        KoShape* shape = shapes.at(i).second;

        QTextCursor result(doc);

        do {
            result = doc->find(text, result);

            if(result.hasSelection()) {
                SearchResult pos;
                pos.page = shapes.at(i).first;
                pos.shape = shape;
                pos.startPosition = result.selectionStart();
                pos.length = result.selectionEnd() - result.selectionStart();
                m_searchResults<<pos;
                highlightText(m_searchResults.size()-1,false);
            }
        } while(!result.isNull());
    }
}

void OfficeViewerPresentation::previousWord()
{
    if(searchResultCount() > 0) {
        highlightText(m_searchIndex,false);

        if(m_searchIndex == 0) {
            m_searchIndex = searchResultCount() - 1;
        } else {
            m_searchIndex--;
        }

        highlightText(m_searchIndex,true);

        centerToResult(m_searchIndex);
    }
}

void OfficeViewerPresentation::nextWord()
{
    if(searchResultCount() > 0) {
        highlightText(m_searchIndex,false);

        if(m_searchIndex == searchResultCount() - 1) {
            m_searchIndex = 0;
        } else {
            m_searchIndex++;
        }

        highlightText(m_searchIndex,true);

        centerToResult(m_searchIndex);
    }
}

int OfficeViewerPresentation::searchResultCount()
{
    return m_searchResults.size();
}

void OfficeViewerPresentation::clearSearchResults()
{
    bool updated = false;
    for(int i= 0; i<m_searchResults.size(); i++) {

        KoShape *shape = m_searchResults[i].shape;

        if(!shape) {
            continue;
        }

        KoTextShapeData* tsd = qobject_cast<KoTextShapeData*> (shape->userData());

        if(!tsd || !tsd->document()) {
            continue;
        }

        //Get QTextDocument from text shape's userdata
        QTextDocument *doc = tsd->document();

        QTextBlock block = doc->findBlock(m_searchResults[i].startPosition);

        QTextLayout *layout = block.layout();

        if(!layout) {
            continue;
        }

        //Get the list of all additional formats in the search result's range
        QList<QTextLayout::FormatRange> ranges = block.layout()->additionalFormats();

        QList<QTextLayout::FormatRange> newRanges;

        //And remove our highlight formatting from the list
        foreach(QTextLayout::FormatRange range, ranges) {
            if(range.format != m_highlight && range.format != m_highlightCurrent) {
                newRanges<<range;
            }
        }

        //And set the list back
        if(ranges.count() != newRanges.count()) {
            block.layout()->setAdditionalFormats(newRanges);
        }

        // update the page so that the search results are no longer visible
        if (!updated && m_currentPage == m_searchResults[i].page) {
            m_searchResults[i].page->update();
            updated = true;
        }
    }

    m_searchResults.clear();

}

void OfficeViewerPresentation::activeToolChanged(KoCanvasController* canvas, int uniqueToolId)
{
    Q_UNUSED(canvas);
    Q_UNUSED(uniqueToolId);

    QString newTool = KoToolManager::instance()->activeToolId();
    qDebug() << __PRETTY_FUNCTION__ << "newTool" << newTool;

    // only Pan tool or Text tool should ever be the active tool, so if
    // another tool got activated, switch back to pan tool

    if(newTool != PanToolID && newTool != TextToolID && newTool != InteractionToolID) {
        KoToolManager::instance()->switchToolRequested(PanToolID);
    }

    //m_canvasItem->setProperty(KoFingerScrollable, true);

}

void OfficeViewerPresentation::tvoutConnected()
{
    qWarning() << "Some activity with AV Slot " << tvout->value();

    if((tvout->value().toString() == "tvout") || (tvout->value().toString() == "builtinandtvout")) {
        QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                                                         MCE_REQUEST_IF, MCE_PREVENT_BLANK_REQ));
        tvoutPluggedIn = true;
        preventBlankTimer.start();
    } else {
        tvoutPluggedIn = false;
        preventBlankTimer.stop();
        QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                                                         MCE_REQUEST_IF, MCE_CANCEL_PREVENT_BLANK_REQ));
    }
}

void OfficeViewerPresentation::preventBlanking()
{
    QDBusConnection::systemBus().call(QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH,
                                                                     MCE_REQUEST_IF, MCE_PREVENT_BLANK_REQ));
}

qreal OfficeViewerPresentation::minimumZoomFactor() const
{
    const KoPageLayout &layout = m_currentPage->pageLayout();
    const QSizeF pageSize(layout.width, layout.height);

    qreal fitToPageZoomFactor = (m_pannableScrollbars->viewportSize().width() - 2 * m_pannableScrollbars->margin())
                 / (zoomHandler()->resolutionX() * pageSize.width());
    fitToPageZoomFactor = qMin(fitToPageZoomFactor, (m_pannableScrollbars->viewportSize().height() - 2 * m_pannableScrollbars->margin())
                 / (zoomHandler()->resolutionY() * pageSize.height()));
    return fitToPageZoomFactor;
}
