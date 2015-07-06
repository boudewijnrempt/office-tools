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
 */

//Include QT stuff
#include <QGraphicsLinearLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextBlock>
#include <QApplication>
#include <QtAlgorithms>
#include <QScrollBar>
#include <QAction>
#include <QGraphicsSceneMouseEvent>

//Include M stuff
#include <MBanner>

// KDE stuff

#include <kactioncollection.h>

//Include Koffice stuff
#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoZoomController.h>
#include <KoShapeManager.h>
#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoTextLayoutRootArea.h>
#include <KoShapeContainer.h>
#include <KWCanvasItem.h>
#include <KoZoomHandler.h>
#include <KWCanvas.h>
#include <KWView.h>
#include <KoCanvasControllerWidget.h>
#include <KoSelection.h>

//Include application stuff
#include "officeviewerword.h"
#include "definitions.h"
#include "pannablescrollbars.h"
#include "applicationwindow.h"
#include "actionpool.h"
#include "thumbprovider.h"
#include "officeviewereventfilter.h"

OfficeViewerWord::OfficeViewerWord(QGraphicsWidget *parent)
    : OfficeViewer(parent)
    , m_canvasItem(0)
    , m_zoomController(0)
    , m_actionCollection(new KActionCollection(this))
    , m_currentPageNr(0)
    , m_pageCount(0)
    , m_lastUserDefinedFactor(1.0)
    , m_thumbnailView(0)
{
    setObjectName("officeviewerword");
    m_pannableScrollbars = NULL;
    //TODO these values aren't defined in the UI specification yet
    //once they are decided, put proper values here.
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(QBrush(highlightColor));
    KoFindText::setFormat(KoFindText::HighlightFormat, highlightFormat);

    QTextCharFormat currentMatchFormat;
    currentMatchFormat.setBackground(QBrush(highlightColorCurrent));
    KoFindText::setFormat(KoFindText::CurrentMatchFormat, currentMatchFormat);

    QObject::connect(ApplicationWindow::GetSceneManager(), SIGNAL(orientationChangeFinished(const M::Orientation &)),
                     this, SLOT(orientationChanged()));

    m_zoomLevel = ZoomLevel(ZoomLevel::FitToWidth, 1.0, false);

}

OfficeViewerWord::~OfficeViewerWord()
{
    if (m_thumbnailView) {
        m_document->removeView(m_thumbnailView);
        delete m_thumbnailView;
        m_thumbnailView = 0;
    }

    if(m_document) {
        delete m_document;
        m_document = 0;
    }
}

bool OfficeViewerWord::createKoWidget()
{
    if(!m_document) return false;

    // move to start
    KWDocument *document = qobject_cast<KWDocument*>(m_document);

    if(!document) return false;

    m_currentPage = document->pageManager()->begin();

    m_currentPageNr = 1;

    // setup canvas controller
    m_pannableScrollbars = new PannableScrollBars(this);

    m_pannableScrollbars->setEnabled(true);

    m_pannableScrollbars->setPanDirection(Qt::Horizontal | Qt::Vertical);

    m_pannableScrollbars->setClipping(false);

    m_pannableScrollbars->setCanvasMode(KoCanvasController::AlignTop);

    // Get the canvas
    m_canvasItem = dynamic_cast<KWCanvasItem*>(document->canvasItem());
    if(!m_canvasItem || !m_canvasItem->resourceManager()) {
        return false;
    }

    m_canvasItem->installEventFilter(new OfficeViewerEventFilter(this));
    m_canvasItem->setCacheEnabled(true, 32*1024*1024, 5.0);

    m_canvasItem->viewMode()->setGap(10); // wider gap because otherwise it'll get scaled out

    //m_canvasItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    m_canvasItem->setCacheMode(QGraphicsItem::NoCache);
    //m_canvasItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    m_pannableScrollbars->setCanvas(m_canvasItem);
    m_pannableScrollbars->setZoomHandler(m_canvasItem->zoomHandler());

    KoToolManager::instance()->addController(m_pannableScrollbars);

    connect(m_pannableScrollbars->proxyObject, SIGNAL(moveDocumentOffset(QPoint)),
            this, SLOT(setDocumentOffset(QPoint)));

    // Zoomcontroller
    m_zoomController = new KoZoomController(m_pannableScrollbars,
                                            m_canvasItem->zoomHandler(),
                                            m_actionCollection,
                                            0,
                                            this);
    m_zoomController->setPageSize(m_currentPage.rect().size());
    KoZoomMode::Modes modes = KoZoomMode::ZOOM_WIDTH;

    if(m_canvasItem->viewMode()->hasPages()) {
        modes |= KoZoomMode::ZOOM_PAGE;
    }

    m_zoomController->zoomAction()->setZoomModes(modes);

    connect(m_canvasItem, SIGNAL(documentSize(const QSizeF &)), m_zoomController, SLOT(setDocumentSize(const QSizeF&)));

    m_canvasItem->updateSize(); // to emit the doc size at least once

    // Document size
    QSizeF documentSize = m_canvasItem->viewMode()->contentsSize();
    m_pannableScrollbars->updateDocumentSize(documentSize.toSize(), true);

    // Connect the tool manager
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*, int)),
            this, SLOT(activeToolChanged(KoCanvasController*, int)));
    KoToolManager::instance()->switchToolRequested(PanToolID);

    connect(m_canvasItem->resourceManager(), SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(resourceChanged(int, const QVariant &)));

    updateSizes();
    updatePageNumbers();

    zoom(m_zoomLevel);

    // the addDocuments should only be done onces
    QList<QTextDocument*> texts;
    KoFindText::findTextInShapes(m_canvasItem->shapeManager()->shapes(), texts);
    m_find.addDocuments(texts);

    return true;
}


void OfficeViewerWord::updateSizes()
{
    QSizeF size=ApplicationWindow::visibleSizeCorrect();

    if(m_pannableScrollbars) {
        QPointF position = m_pannableScrollbars->position();
        m_pannableScrollbars->updateRange();
        m_pannableScrollbars->setMinimumSize(size);
        m_pannableScrollbars->setMaximumSize(size);
        m_pannableScrollbars->setPosition(position);
    }
}

void OfficeViewerWord::orientationChanged()
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

QGraphicsLayoutItem *OfficeViewerWord::getGraphicsLayoutItem()
{
    return m_pannableScrollbars;
}

void OfficeViewerWord::updatePageNumbers()
{
    if(0 != m_document) {
        int newPageCount = m_document->pageCount();

        emit pageChanged(newPageCount, m_currentPageNr);

        if(newPageCount != m_pageCount) {
            //Lets try reading page numbers little bit later as KOffice uses lazy loading.
            QTimer::singleShot(KofficePageNumberUpdateIntervalTime, this, SLOT(updatePageNumbers()));
        }

        m_pageCount = newPageCount;
    }
}

void OfficeViewerWord::zoom(const ZoomLevel &newLevel)
{
    if(m_zoomController && m_document) {

        qreal factor = 0;
        int nCurPage = m_currentPageNr-1; // since pageIndex+1 is the currentPageNo
        KoZoomAction *zAction = m_zoomController->zoomAction();

        newLevel.getFactor(factor);

        switch(newLevel.getMode()) {

        case ZoomLevel::FitToPage:
//            qDebug() << "Zooming fit to page";
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
            break;

        case ZoomLevel::FitToHeight:
//            qDebug() << "Zooming fit to height";
            //No Fit to height support so lets use best available
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
            break;

        case ZoomLevel::FitToWidth:
//            qDebug() << "Zooming fit to width";
            m_zoomController->setZoomMode(KoZoomMode::ZOOM_WIDTH);
            break;

        case ZoomLevel::FactorMode:
            //TODO Add checking of minimum and maximum
//            qDebug() << "Zooming factormode";

            if(factor >= minimumZoomFactor() && factor <= MaxZoomFactor) {
                zAction->setEffectiveZoom(factor);
            }

        case ZoomLevel::Relative:
            qDebug() << "relative zoom level";
            if(newLevel.getFactor(factor)) {
                qDebug() << "relative zoom level" << m_zoomController->zoomAction()->effectiveZoom() << factor;
                m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, m_zoomController->zoomAction()->effectiveZoom() * factor);
            }
            break;
        }

        qDebug() << __PRETTY_FUNCTION__ << "ZOOM" << m_zoomController->zoomAction()->effectiveZoom() << factor;
#if 0
        if (m_zoomController->zoomAction()->effectiveZoom() >= 2.0) {
            m_canvasItem->setCacheEnabled(false);
        } else {
            m_canvasItem->setCacheEnabled(true);
        }
#endif

        if(m_zoomLevel.isUserDefined()) {
            qreal current = zAction->effectiveZoom();

            if (newLevel.getMode() == ZoomLevel::Relative) {
                current =  zAction->effectiveZoom() * factor;
            }

            qreal fitToPageZoomFactor = minimumZoomFactor();

            qDebug() << __PRETTY_FUNCTION__ << current << fitToPageZoomFactor << MaxZoomFactor;
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
            m_lastUserDefinedFactor = zAction->effectiveZoom() * factor;
            ActionPool::instance()->setUserDefinedZoomFactor(m_lastUserDefinedFactor);
        }

        showPage(nCurPage);

        m_canvasItem->update(m_canvasItem->rect());
    }
}

void OfficeViewerWord::resourceChanged(int key, const QVariant &value)
{
    if(KoCanvasResource::CurrentPage == key) {
        m_currentPageNr = value.toInt();
        updatePageNumbers();
    }
}

void OfficeViewerWord::startSearch(const QString & searchString)
{
    qDebug() << "startSearch" << searchString;

    if(!m_canvasItem) {
        return;
    }

    m_find.find(searchString);

    emit matchesFound(m_find.hasMatches());
    centerToResult();
}

void OfficeViewerWord::centerToResult()
{
    KoFindMatch match = m_find.currentMatch();
    if (!match.isValid() || !match.location().canConvert<QTextCursor>() || !match.container().canConvert<QTextDocument*>() ) {
        return;
    }

    QTextDocument *doc = match.container().value<QTextDocument *>();
    if (doc) {
        KoTextDocumentLayout *documentLayout = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
        if (documentLayout) {
            QTextCursor cursor = match.location().value<QTextCursor>();
            KoTextLayoutRootArea *area = documentLayout->rootAreaForPosition(cursor.position());
            if (area) {
                KoShape *shape = area->associatedShape();
                if (shape) {
                    QRectF visibleRect(textSelectionRect(shape, cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart()));
                    m_pannableScrollbars->ensureVisible(m_canvasItem->viewConverter()->documentToView(visibleRect));
                }
            }
        }
    }

}

void OfficeViewerWord::setCurrentPage(int pageIndex)
{
    qDebug() << "setCurrentPage" << pageIndex;

    if((1+pageIndex) != m_currentPageNr) {
        m_currentPageNr = pageIndex+1;
        updatePageNumbers();
    }
}

QSizeF OfficeViewerWord::currentDocumentSize()
{
    if (!m_document ||
        !((static_cast<KWDocument*>(m_document))->pageManager())) {
        return QSizeF();
    }

    KWPage page = (static_cast<KWDocument*>(m_document))->pageManager()->page(m_currentPageNr);

    qreal height = page.height();// + page.topMargin() + page.bottomMargin();
    qreal width = page.width();// + page.leftMargin() + page.rightMargin();

    width = m_canvasItem->zoomHandler()->zoomItX(width);
    height = m_canvasItem->zoomHandler()->zoomItY(height);

    return QSizeF(width, height);
}

void OfficeViewerWord::setDocumentOffset(const QPoint &point)
{
    qDebug() << "OfficeViewerWord::setDocumentOffset 1" << point;
    m_canvasItem->setDocumentOffset(point);
    offsetInDocumentMoved(point.y());
}

void OfficeViewerWord::showPage(int pageIndex)
{
    qDebug() << ">>>>>>>>>>>>>>>>> showPage" << pageIndex;
    KWPage page = (static_cast<KWDocument*>(m_document))->pageManager()->page(pageIndex + 1);
    goToPage(page);
}

void OfficeViewerWord::prepareThumbnailer()
{
    KWDocument *doc = qobject_cast<KWDocument*>(m_document);
    m_thumbnailView = static_cast<KWView *>(m_document->createView());

    Q_CHECK_PTR(m_thumbnailView);
    QList<KoCanvasControllerWidget*> koControllers = m_thumbnailView->findChildren<KoCanvasControllerWidget*>();
    KoCanvasControllerWidget *controller = koControllers.first();
    Q_CHECK_PTR(controller);
//    KWCanvas *canvas =  static_cast<KWCanvas*>(m_thumbnailView->canvasBase());
    controller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    controller->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    foreach (KoShape *shape, canvas->shapeManager()->shapes()) {
//        shape->waitUntilReady(*m_thumbnailView->viewConverter(), false);
//    }

    while(! doc->layoutFinishedAtleastOnce()) {
        QCoreApplication::processEvents();

        if(! QCoreApplication::hasPendingEvents())
            break;
    }
}

QImage * OfficeViewerWord::getThumbnail(int pageNumber)
{
    if (!m_thumbnailView) {
        prepareThumbnailer();
    }
    KWDocument *doc = qobject_cast<KWDocument*>(m_document);

    KWPageManager *manager = doc->pageManager();

    // In Words page number starts from 1
    KWPage page = manager->page(pageNumber+1);

    qreal height = page.height() + page.topMargin() + page.bottomMargin();
    qreal width = page.width() + page.leftMargin() + page.rightMargin();
    QSizeF pageSize(width, height);

    qreal zoom = 430 / width;
    qDebug() << __PRETTY_FUNCTION__ << zoom;
    zoom = qMax(qreal(1.0), zoom);

    QImage *image = new QImage(page.thumbnail((pageSize*zoom).toSize(), m_canvasItem->shapeManager()));
    return image;
}

void OfficeViewerWord::shortTap(const QPointF &point, QObject *object)
{
    if (0 == object || !isLoaded) {
        return;
    }

    QPointF p(m_canvasItem->mapFromScene(point));
    QPointF currentPos(m_pannableScrollbars->position());
    QPointF documentPoint(m_canvasItem->viewMode()->viewToDocument(p + currentPos - m_canvasItem->pos(), m_canvasItem->viewConverter()));

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


void OfficeViewerWord::clearSearchResults()
{
    qDebug() << "clearSearchResults();";

    m_find.finished();
}

void OfficeViewerWord::previousWord()
{
    qDebug() << "previousWord";

    m_find.findPrevious();
    centerToResult();
}

void OfficeViewerWord::nextWord()
{
    qDebug() << "nextWord";

    m_find.findNext();
    centerToResult();
}

void OfficeViewerWord::activeToolChanged(KoCanvasController* canvas, int uniqueToolId)
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

    //    canvas->setProperty(KoFingerScrollable, true);
}

void OfficeViewerWord::getCurrentVisiblePages(ThumbProvider *thumbProvider)
{
    qDebug() << "getCurrentVisiblePages";

    if(0 != thumbProvider) {
        thumbProvider->clearVisibleAreas();

        KWViewMode *viewMode = m_canvasItem->viewMode();

        QSizeF size = ApplicationWindow::visibleSizeCorrect();
        QRect rect = QRect(qAbs(m_pannableScrollbars->canvasOffsetX()),
                           qAbs(m_pannableScrollbars->canvasOffsetY()),
                           m_pannableScrollbars->visibleWidth(),
                           m_pannableScrollbars->visibleHeight());

        foreach(const KWViewMode::ViewMap map, viewMode->mapExposedRects(rect, m_canvasItem->viewConverter())) {
            //FIXME There is something wrong with the calculation
            QRectF clipRect = map.clipRect;
            QRectF pageRect = m_canvasItem->viewConverter()->documentToView(map.page.rect());

            if(rect.width() >= pageRect.width()) {
                clipRect.moveLeft(0);
            }

            QRectF visibleRect = pageRect.intersected(clipRect);

            visibleRect = QRectF(visibleRect.topLeft()- pageRect.topLeft(), visibleRect.size());
            thumbProvider->addVisibleAreas(map.page.pageNumber()-1,  visibleRect, pageRect.size());
        }
    }
}

void OfficeViewerWord::pinchStarted(QPointF &center)
{
    m_pannableScrollbars->pinchStarted();
    QSize size = ApplicationWindow::visibleSizeCorrect();

    if (size.width() > currentDocumentSize().width()) {
        center.rx() = size.width() / 2;
    }
    if (size.height() > currentDocumentSize().height()) {
        center.ry() = size.height() / 2;
    }

    m_pinchCenterDocument = m_canvasItem->viewConverter()->viewToDocument(center + m_pannableScrollbars->position() - m_canvasItem->pos());
}

qreal OfficeViewerWord::pinchUpdated(qreal zoomFactor)
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

void OfficeViewerWord::pinchFinished(const QPointF &center, qreal scale)
{
    qreal effectiveZoomFactor = m_zoomController->zoomAction()->effectiveZoom() * scale;
    if (qFuzzyCompare(effectiveZoomFactor, minimumZoomFactor())) {
        m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);
    }
    else {
        m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, effectiveZoomFactor);
    }
    m_pannableScrollbars->pinchFinished();

    qDebug() << __PRETTY_FUNCTION__ << "ZOOM" << m_zoomController->zoomAction()->effectiveZoom();

    // calculate the position in the view coordinates
    QPointF newPos = m_canvasItem->viewConverter()->documentToView(m_pinchCenterDocument) - center;
    qDebug() << "XXX finish" << m_pinchCenterDocument << newPos << center;
    if (newPos.y() < 0) {
        newPos.setY(0);
    }
    if (newPos.x() < 0) {
        newPos.setX(0);
    }
    // it might happen that the pos in the scroll bar has been updated to 0,0 while the document offset has not been
    // changed and the info therfore not updated. Therefore we need to trigger that.
    if (newPos == m_pannableScrollbars->position()) {
        m_pannableScrollbars->proxyObject->emitMoveDocumentOffset(newPos.toPoint());
    }
    else {
        m_pannableScrollbars->setPosition(newPos.toPoint());
    }
}

void OfficeViewerWord::setCurrentPage(const KWPage &currentPage)
{
    qDebug() << "setCurrentPage();" << currentPage.pageNumber();
    Q_ASSERT(currentPage.isValid());

    if(currentPage != m_currentPage) {
        m_currentPage = currentPage;
        m_canvasItem->resourceManager()->setResource(KoCanvasResource::CurrentPage, m_currentPage.pageNumber());
        m_zoomController->setPageSize(m_currentPage.rect().size());
    }
}

void OfficeViewerWord::nextPage()
{
    qDebug() << "nextPage";
    KWPage page = m_currentPage.next();

    if(page.isValid()) {
        goToPage(page);
    }
}

void OfficeViewerWord::previousPage()
{
    qDebug() << "previousPage()";
    KWPage page = m_currentPage.previous();

    if(page.isValid()) {
        goToPage(page);
    }
}

void OfficeViewerWord::goToPage(const KWPage &page)
{
    qDebug() << "goToPage" << page.pageNumber();

    QPoint origPos = m_pannableScrollbars->scrollBarValue();
    QPointF pos = m_canvasItem->viewMode()->documentToView(QPointF(0, page.offsetInDocument()), m_canvasItem->viewConverter());
    origPos.setY((int)pos.y());
    m_pannableScrollbars->setScrollBarValue(origPos);
}


void OfficeViewerWord::goToPageOffset(const KWPage &page, const QPointF &offset)
{
    qDebug() << "goToPageOffset" << page.pageNumber();

    QPoint origPos = m_pannableScrollbars->scrollBarValue();

    QPointF newOffest = m_canvasItem->viewMode()->documentToView(offset, m_canvasItem->viewConverter());

    int offsetx = newOffest.x();
    int offsety = newOffest.y();

    int zoomedWidth = currentDocumentSize().width();
    QSize size = ApplicationWindow::visibleSizeCorrect();
    int viewportWidth = size.width();
    bool smallerSize = false;

    if (zoomedWidth < viewportWidth) {
        smallerSize = true;
    }

    offsetx = qMax(offsetx, 0);
    offsety = qMax(offsety, 0);

    if (smallerSize) {
        offsetx = 0;
    }

    origPos.setX((int)offsetx);
    origPos.setY((int)offsety);
    m_pannableScrollbars->setScrollBarValue(origPos);
}


void OfficeViewerWord::offsetInDocumentMoved(int yOffset)
{
    const qreal offset = m_canvasItem->zoomHandler()->viewToDocumentY(yOffset);
    const qreal height = m_canvasItem->zoomHandler()->viewToDocumentY(m_pannableScrollbars->size().height());

    const qreal off = offset + height/2.0;

    KWPage page = qobject_cast<KWDocument*>(m_document)->pageManager()->page(off);

    qDebug() << "offsetInDocumentMoved();" << yOffset << height << off << page.pageNumber();

    if (page.isValid()) {
        setCurrentPage(page);
    }
}

qreal OfficeViewerWord::minimumZoomFactor() const
{
    const QSizeF pageSize(m_currentPage.rect().size());

    qreal fitToPageZoomFactor = (m_pannableScrollbars->viewportSize().width() - 2 * m_pannableScrollbars->margin())
                 / (m_canvasItem->zoomHandler()->resolutionX() * pageSize.width());
    fitToPageZoomFactor = qMin(fitToPageZoomFactor, (m_pannableScrollbars->viewportSize().height() - 2 * m_pannableScrollbars->margin())
                 / (m_canvasItem->zoomHandler()->resolutionY() * pageSize.height()));
    return fitToPageZoomFactor;
}
