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

#include <math.h>

#include <QTimer>
#include <QDebug>
#include <QGraphicsItem>
#include <QScrollBar>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneResizeEvent>
#include <QGesture>

#include <MApplication>
#include <MPositionIndicator>

#include <KoShape.h>
#include <KoCanvasBase.h>
#include <KoZoomHandler.h>
#include <KoPACanvasItem.h>

#include "pannablescrollbars.h"
#include "applicationwindow.h"

PannableScrollBars::PannableScrollBars(QGraphicsItem *parent)
    : MPannableViewport(parent)
    , KoCanvasController(0)
    , m_zoomHandler(0)
    , m_canvas(0)
    , m_canvasItem(0)
    , m_ignoreScrollSignals(false)
    , pinchInProgress(false)
    , updatePositionTimerRunning(false)
    , m_updatingPosition(false)
    , m_panDirection(PanDown)
    , m_panning(false)
{

    setEnabled(true);
    setAutoRange(false);
    setPanDirection(Qt::Vertical);
    setClipping(false);
    setContentsMargins(0, 0, 0, 0);
    connect(proxyObject, SIGNAL(moveDocumentOffset(QPoint)), this, SLOT(documentOffsetMoved(QPoint)));
#if QT_VERSION  >= 0x040700
    setAutoFillBackground(false);
#endif
    setStyleName("viewerBackground");
}


PannableScrollBars::~PannableScrollBars()
{
    if (m_canvas) {
        proxyObject->emitCanvasRemoved(this);
    }

    setWidget(0);

    if (m_canvasItem) {
        m_canvasItem->removeEventFilter(this);
        delete m_canvasItem;
    }
}


void PannableScrollBars::scrollContentsBy(int dx, int dy)
{
    //qDebug() << "PannableScrollBars::scrollContentsBy()" << dx << dy;
    Q_UNUSED(dx);
    Q_UNUSED(dy);
}

QSize PannableScrollBars::viewportSize() const
{
    QSize sz = ApplicationWindow::visibleSize();
    qDebug() << "PannableScrollBars::viewportSize()" << sz;
    return sz;
}


void PannableScrollBars::setDrawShadow(bool drawShadow)
{
    Q_UNUSED(drawShadow);
}

void PannableScrollBars::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__ << event->newSize().toSize();
    MPannableViewport::resizeEvent(event);
    proxyObject->emitSizeChanged(event->newSize().toSize());
    resetLayout();
}

void PannableScrollBars::setCanvas(KoCanvasBase *canvas)
{
    if(m_canvas) {
        proxyObject->emitCanvasRemoved(this);
    }

    if(!canvas) {
        setWidget(0);
    }

    if(m_canvasItem) {
        m_canvasItem->removeEventFilter(this);
        m_canvasItem->hide();
        delete m_canvasItem;
        m_canvasItem = 0;
    }

    if(!canvas) {
        return;
    }

    canvas->setCanvasController(this);

    // XXX: This won't work with Krita, where KoCanvasBase
    //      isn't a superclass of the actual canvas widgets
    QGraphicsWidget *canvasItem = canvas->canvasItem();
    Q_ASSERT(canvasItem);


    m_canvasItem = canvasItem;

    m_canvasItem->setParent(this);

    m_canvasItem->show();

    setWidget(m_canvasItem);

    resetLayout();

    m_canvasItem->installEventFilter(this);

    m_canvas = canvas;

    proxyObject->emitCanvasSet(this);

    connect(this,SIGNAL(panningStopped()),this,SLOT(panningStopped()));
}

KoCanvasBase *PannableScrollBars::canvas() const
{
    return m_canvas;
}

int PannableScrollBars::visibleHeight() const
{
    //qDebug() << "PannableScrollBars::visibleHeight()" << size().toSize().height();
    if(m_canvasItem == 0) {
        return 0;
    }

    QSize sz = size().toSize();

    int height1;

    if(m_canvasItem == 0) {
        height1 = sz.height();
    } else {
        height1 = qMin(sz.height(), m_canvasItem->size().toSize().height());
    }

    int height2 = sz.height();

    return qMin(height1, height2);
}

int PannableScrollBars::visibleWidth() const
{
    //qDebug() << "PannableScrollBars::visibleWidth()" << size().toSize().width();
    if(m_canvasItem == 0) {
        return 0;
    }

    QSize sz = size().toSize();

    int width1;

    if(m_canvasItem == 0) {
        width1 = sz.width();
    } else {
        width1 = qMin(sz.width(), m_canvasItem->size().toSize().width());
    }

    int width2 = sz.width();

    return qMin(width1, width2);
}

int PannableScrollBars::canvasOffsetX() const
{
    //qDebug() << "PannableScrollBars::canvasOffsetX()" << position().x();
    int offset = 0;

    if(m_canvasItem) {
        offset = m_canvasItem->x();
    }

    return offset - position().x();
}

int PannableScrollBars::canvasOffsetY() const
{
    //qDebug() << "PannableScrollBars::canvasOffsetY()" << position().y();
    int offset = 0;

    if(m_canvasItem) {
        offset = m_canvasItem->y();
    }

    return offset - position().y();
}

void PannableScrollBars::ensureVisible(const QRectF &rect, bool smooth)
{
    qDebug() << __PRETTY_FUNCTION__ << rect << smooth;
    QRect currentVisible(qMax(0, -canvasOffsetX()), qMax(0, -canvasOffsetY()), visibleWidth(), visibleHeight());
    qDebug() << "currently visible" << currentVisible << -canvasOffsetX() << -canvasOffsetY() << visibleWidth() << visibleHeight() << ApplicationWindow::visibleSizeCorrect();

    MApplicationWindow *window = MApplication::activeApplicationWindow();
    if (window && window->currentPage()) {
        DocumentPage *page = qobject_cast<DocumentPage*>(window->currentPage());
        if (page) {
            // this takes the findtoolbar into account
            QRectF vRect = page->visibleRect();
            // for spreadsheets the area where to content is is smaller when the row/column header is shown
            // so this has to be taken into account
            QRectF geometry = this->geometry();
            QRectF contentArea = vRect.intersected(geometry);
            qDebug() << __PRETTY_FUNCTION__ << vRect << geometry << contentArea << position();
            currentVisible = contentArea.translated(position()).toRect();
            qDebug() << __PRETTY_FUNCTION__ << currentVisible;
        }
    }

    QRect viewRect = rect.toRect();
    viewRect.translate(m_canvas->documentOrigin());

    qDebug() << "viewRect" << viewRect << m_canvas->documentOrigin();

    if(!viewRect.isValid() || currentVisible.contains(viewRect)) {
        qDebug() << "it is visible";
        return; // its visible. Nothing to do.
    }

    // if we move, we move a little more so the amount of times we have to move is less.
    int jumpWidth = smooth ? 0 : currentVisible.width() / 5;

    int jumpHeight = smooth ? 0 : currentVisible.height() / 5;

    if(!smooth && viewRect.width() + jumpWidth > currentVisible.width())
        jumpWidth = 0;

    if(!smooth && viewRect.height() + jumpHeight > currentVisible.height())
        jumpHeight = 0;

    qDebug() << __PRETTY_FUNCTION__ << jumpWidth << jumpHeight << currentVisible << smooth << viewRect;
    int horizontalMove = 0;

    if(currentVisible.width() <= viewRect.width()) {      // center view
        horizontalMove = viewRect.center().x() - currentVisible.center().x();
    }
    else if(currentVisible.x() > viewRect.x()) {          // move left
        horizontalMove = viewRect.x() - currentVisible.x() - jumpWidth;
    }
    else if(currentVisible.right() < viewRect.right()) {  // move right
        horizontalMove = viewRect.right() - qMax(0, currentVisible.right() - jumpWidth);
    }

    int verticalMove = 0;

    if(currentVisible.height() <= viewRect.height()) {     // center view
        verticalMove = viewRect.center().y() - currentVisible.center().y();
    }

    if(currentVisible.y() > viewRect.y()) {                // move up
        verticalMove = viewRect.y() - currentVisible.y() - jumpHeight;
    }
    else if(currentVisible.bottom() < viewRect.bottom()) { // move down
        verticalMove = viewRect.bottom() - qMax(0, currentVisible.bottom() - jumpHeight);
    }

    qDebug() << "panning to" << horizontalMove << verticalMove;
    QPointF pos = position() + QPointF(horizontalMove, verticalMove);

    setPosition(pos);
}

void PannableScrollBars::recenterPreferred()
{
    //qDebug() << "PannableScrollBars::recenterPreferred" << preferredCenterFractionX() << preferredCenterFractionY();
    QSize sz = viewportSize();

    if(sz.width() >= documentSize().width()
       && sz.height() >= documentSize().height()) {
        return; // no need to center when image is smaller than viewport
    }

    const bool oldIgnoreScrollSignals = m_ignoreScrollSignals;

    m_ignoreScrollSignals = true;

    QPoint center = QPoint(int(documentSize().width() * preferredCenterFractionX()),
                           int(documentSize().height() * preferredCenterFractionY()));

    // convert into a viewport based point
    center.rx() += m_canvasItem->x();

    center.ry() +=  m_canvasItem->y();

    // calculate the difference to the viewport centerpoint
    QPoint topLeft = center - 0.5 * QPoint(sz.width(), sz.height());

    QRect rc = range().toRect();

    // try to centralize the centerpoint which we want to make visible
    topLeft.rx() = qMax(topLeft.x(), 0);

    topLeft.rx() = qMin(topLeft.x(), rc.width());

    topLeft.ry() = qMax(topLeft.y(), 0);

    topLeft.ry() = qMin(topLeft.y(), rc.height());

    setPosition(topLeft);

    m_ignoreScrollSignals = oldIgnoreScrollSignals;
}

void PannableScrollBars::setPreferredCenter(const QPoint &viewPoint)
{
    qDebug() << "PannableScrollBars::setPreferredCenter" << viewPoint;
    setPreferredCenterFractionX(1.0 * viewPoint.x() / documentSize().width());
    setPreferredCenterFractionY(1.0 * viewPoint.y() / documentSize().height());
    recenterPreferred();
}

QPoint PannableScrollBars::preferredCenter() const
{
    //qDebug() << "PannableScrollBars::preferredCenter";
    QPoint center;
    center.setX(qRound(preferredCenterFractionX() * documentSize().width()));
    center.setY(qRound(preferredCenterFractionY() * documentSize().height()));
    qDebug() << "DOCUMENT SIZE " << documentSize();
    qDebug() << "VIEW SIZE " << size();
    return center;
}


void PannableScrollBars::ensureVisible(KoShape *shape)
{
    //qDebug() << "PannableScrollBars::ensureVisible()" << shape;
    ensureVisible(m_canvas->viewConverter()->documentToView(shape->boundingRect()));
}

void PannableScrollBars::zoomIn(const QPoint &center)
{
    //qDebug() << "PannableScrollBars::zoomIn()" << center;
    zoomBy(center, sqrt(2.0));
}

void PannableScrollBars::zoomOut(const QPoint &center)
{
    //qDebug() << "PannableScrollBars::zoomOut" << center;
    zoomBy(center, sqrt(0.5));
}

void PannableScrollBars::zoomBy(const QPoint &center, qreal zoom)
{
    //qDebug() << "PannableScrollBars::zoomBy" << center << zoom;
    setPreferredCenterFractionX(1.0 * center.x() / documentSize().width());
    setPreferredCenterFractionY(1.0 * center.y() / documentSize().height());

    const bool oldIgnoreScrollSignals = m_ignoreScrollSignals;
    m_ignoreScrollSignals = true;
    proxyObject->emitZoomBy(zoom);
    m_ignoreScrollSignals = oldIgnoreScrollSignals;
    recenterPreferred();
    //m_canvasItem->update();
}

void PannableScrollBars::zoomTo(const QRect &viewRect)
{
    //qDebug() << "PannableScrollBars::zoomTo()" << rect;
    qreal scale;
    QSize sz = viewportSize();

    if(1.0 * sz.width() / viewRect.width() > 1.0 * sz.height() / viewRect.height())
        scale = 1.0 * sz.height() / viewRect.height();
    else
        scale = 1.0 * sz.width() / viewRect.width();

    const qreal preferredCenterFractionX = 1.0 * viewRect.center().x() / documentSize().width();

    const qreal preferredCenterFractionY = 1.0 * viewRect.center().y() / documentSize().height();

    proxyObject->emitZoomBy(scale);

    setPreferredCenterFractionX(preferredCenterFractionX);

    setPreferredCenterFractionY(preferredCenterFractionY);

    recenterPreferred();

    //m_canvasItem->update();
}

void PannableScrollBars::pan(const QPoint &distance)
{
    Q_UNUSED(distance);
    setPosition(position() + distance);
}

void PannableScrollBars::setMargin(int margin)
{
    //qDebug() << "PannableScrollBars::setMargin" << margin;
    m_margin = margin;
}

QPoint PannableScrollBars::scrollBarValue() const
{
    //qDebug() << "PannableScrollBars::scrollBarValue()";
    return position().toPoint();
}

void PannableScrollBars::setScrollBarValue(const QPoint &value)
{
    //qDebug() << "PannableScrollBars::setScrollBarValue()" << value;
    setPosition(value);
}

void PannableScrollBars::updateDocumentSize(const QSize &sz, bool recalculateCenter)
{
    //qDebug() << "PannableScrollBars::updateDocumentSize(" << sz << recalculateCenter;
    if(!recalculateCenter) {
        // assume the distance from the top stays equal and recalculate the center.
        setPreferredCenterFractionX(documentSize().width() * preferredCenterFractionX() / sz.width());
        setPreferredCenterFractionY(documentSize().height() * preferredCenterFractionY() / sz.height());
    }

    bool oldIgnoreScrollSignals = m_ignoreScrollSignals;

    m_ignoreScrollSignals = true;

    // the document size needs to be set before setRange is called so that it contains the correct data
    KoCanvasController::setDocumentSize(sz);

    updateRange();

    m_ignoreScrollSignals = oldIgnoreScrollSignals;
    qDebug() << __PRETTY_FUNCTION__ << sz;
}

void PannableScrollBars::updateRange()
{
    QSize vpSize(viewportSize());

    QRectF fullRect(geometry());
    QRectF vRect(fullRect);
    MApplicationWindow *window = MApplication::activeApplicationWindow();
    if (window && window->currentPage()) {
        DocumentPage *page = qobject_cast<DocumentPage*>(window->currentPage());
        if (page) {
            vRect = page->visibleRect();
        }
    }

    QSize sz(documentSize());
    qreal heightRange = sz.height() - vpSize.height();
    qreal beforeRange = 0.0;

    //qDebug() << __PRETTY_FUNCTION__ << sz << vpSize << vRect << (canvasMode() == KoCanvasController::AlignTop);
    // if it is a presentation only update the range if the size is bigger then the screen.
    if (canvasMode() == KoCanvasController::AlignTop || (vpSize.height() - sz.height()) / 2.0 < vRect.top()) {
        // the top for spreadsheet is different therfor fullRect.top cannot be used. Use 0.0 instead
        //if (vRect.top() > fullRect.top()) {
        if (vRect.top() > 0.0) {
            qDebug() << __PRETTY_FUNCTION__ << "top hidden" << vRect.top() << fullRect.top() << vRect.top() - fullRect.top();
            beforeRange = -(vRect.top() - 0.0);
            heightRange += -beforeRange;
        }
        if (vRect.bottom() < fullRect.bottom()) {
            qDebug() << __PRETTY_FUNCTION__ << "bottom hidden" << vRect.bottom() << fullRect.bottom() << vRect.bottom() - fullRect.bottom();
            heightRange = heightRange + fullRect.bottom() - vRect.bottom();
        }
    }

    qDebug() << "vpSize" << vpSize;
    setRange(QRectF(0.0, beforeRange, sz.width() - vpSize.width(), heightRange));
    qDebug() << "RANGE:" << range() << sz << vpSize << geometry().size() << (m_canvasItem ? m_canvasItem->size() : QSizeF(0, 0));

    resetLayout();
}

void PannableScrollBars::documentOffsetMoved(const QPoint& point)
{
    qDebug() << "PannableScrollBars::documentOffsetMoved(" << point;
    Q_UNUSED(point);
    resetLayout();
}

void PannableScrollBars::resetLayout()
{
    qDebug() << "PannableScrollBars::resetLayout();";
    // Determine the area we have to show
    const QSize vpSize = viewportSize();
    const int viewH = vpSize.height();
    const int viewW = vpSize.width();

    const int docH = documentSize().height();
    const int docW = documentSize().width();

    int moveX = 0;
    int moveY = 0;

    int resizeW = viewW;
    int resizeH = viewH;

    qDebug() << "resetLayout\n\tviewH:" << viewH << "\tdocH: " << docH << "\tviewW: " << viewW << "\tdocW: " << docW << range();

    if(viewH == docH && viewW == docW) {
        qDebug() << "1";
        // Do nothing
        resizeW = docW;
        resizeH = docH;
    } else if(viewH >= docH && viewW >= docW) {
        // Show entire canvas centered
        qDebug() << "2";
        moveX = (viewW - docW) / 2;
        moveY = (viewH - docH) / 2;
        resizeW = docW;
        resizeH = docH;
    } else  if(viewW > docW) {
        // Center canvas horizontally
        qDebug() << "3";
        moveX = (viewW - docW) / 2;
        resizeW = docW;

        int marginTop = margin() - documentOffset().y();
        int marginBottom = viewH  - (documentSize().height() - documentOffset().y());

        if(marginTop > 0) moveY = marginTop;

        if(marginTop > 0) resizeH = viewH - marginTop;

        if(marginBottom > 0) resizeH = viewH - marginBottom;
    } else  if(viewH > docH) {
        qDebug() << "4";
        // Center canvas vertically
        moveY = (viewH - docH) / 2;
        resizeH = docH;

        int marginLeft = margin() - documentOffset().x();
        int marginRight = viewW - (documentSize().width() - documentOffset().x());

        if(marginLeft > 0) moveX = marginLeft;

        if(marginLeft > 0) resizeW = viewW - marginLeft;

        if(marginRight > 0) resizeW = viewW - marginRight;
    } else {
        // Take care of the margin around the canvas
        int marginTop = margin() - documentOffset().y();
        int marginLeft = margin() - documentOffset().x();
        int marginRight = viewW - (documentSize().width() - documentOffset().x());
        int marginBottom = viewH  - (documentSize().height() - documentOffset().y());
        qDebug() << "5" << marginTop << marginLeft << marginRight << marginBottom << margin() << documentOffset();

        if(marginTop > 0) moveY = marginTop;

        if(marginLeft > 0) moveX = marginLeft;

        if(marginTop > 0) resizeH = viewH - marginTop;

        if(marginLeft > 0) resizeW = viewW - marginLeft;

        if(marginRight > 0) resizeW = viewW - marginRight;

        if(marginBottom > 0) resizeH = viewH - marginBottom;
    }

    if(canvasMode() == KoCanvasController::AlignTop) {
        // have up to m_margin pixels at top.
        moveY = qMin(margin(), moveY);
    }

    m_hasOffset = false;
    if(m_canvasItem) {
        QRect geom;

        if (canvasMode() == KoCanvasController::Spreadsheet) {
            geom = QRect(0, 0, viewW, viewH);
        }
        else {
            // this code is needed to make the screen not hide part of the document
            // e.g. when there is only one page and when dragged down in fit to page in portait mode
            //      when not to clip the slide when moving in a presentation
            QRectF org(moveX, moveY, resizeW, resizeH);
            if (resizeH < viewH) {
                qreal y = m_lastPosition.y();
                if (y < 0) {
                    resizeH -= y;
                }
                else {
                    moveY -= y;
                    m_hasOffset = true;
                }
            }
            geom = QRect(moveX, moveY, resizeW, resizeH);
            qDebug() << "geo update" << org << geom << m_lastPosition;
        }

        qDebug() << "geometry" << geom << m_canvasItem->geometry();
        if(m_canvasItem->geometry() != geom) {
            m_canvasItem->setGeometry(geom);
            qDebug() << "geometry now" << m_canvasItem->geometry();
        }
    }
}

QCursor PannableScrollBars::setCursor(const QCursor &cursor)
{
    QCursor oldCursor = QGraphicsWidget::cursor();
    QGraphicsWidget::setCursor(cursor);
    return oldCursor;
}

//-------------------------------------------------------------------------------

void PannableScrollBars::updatePosition(const QPointF &position)
{
    // don't do anything if the position has not changed
    if (position == m_lastPosition || pinchInProgress || m_updatingPosition ) {
        qDebug() << "updatePosition canceled" << m_lastPosition << position << range() << physics()->inMotion();
        m_lostPanGesture = false;
        return;
    }

    m_updatingPosition = true;

    qDebug() << "updatePosition" << m_lastPosition << position << range() << physics()->inMotion();

    MPositionIndicator *pi = positionIndicator();
    if (pi) {
        pi->setPosition(position);
        qDebug() << pi->position() << pi->range() << pi->isEnabled() << pi->viewportSize() << pi->zValue();
    }

    if (m_canvasItem) {
        m_canvasItem->setCacheMode(QGraphicsItem::NoCache);

        qDebug() << "PannableScrollBars::updatePosition();" << position << m_lastPosition << canvasOffsetX() << canvasOffsetY()
            << m_canvasItem->pos() << this->position() << documentOffset() << range() << geometry() << autoFillBackground();

        //MPannableViewport::updatePosition(position);
        if (!updatePositionTimerRunning) {
            updatePositionTimerRunning = true;
            QTimer::singleShot(500, this, SLOT(updatePositionTimeout()));
        }

        QPointF p = position;
        QPointF center = QPointF(p.x() + size().width() / 2.0, p.y() + size().height() / 2.0);
        // disable horizontal panning if document is smaller then the view size
        if (range().width() == 0) {
            p.setX(0);
            center.setX(documentSize().width() / 2.0);
        }

        setPreferredCenterFractionX(center.x() / documentSize().width());
        setPreferredCenterFractionY(center.y() / documentSize().height());

        // remember the direction we are panning
        if (m_lastPosition.y() < position.y()) {
            m_panDirection = PanDown;
        }
        else if (m_lastPosition.y() > position.y()) {
            m_panDirection = PanUp;
        }

        qDebug() << __PRETTY_FUNCTION__ << "panning" << isPanning();
        if (isPanning()) {
            qreal bottom = range().bottom();
            qreal top = range().top();
            qDebug() << __PRETTY_FUNCTION__ << top << bottom << p << m_lastPosition;
            if (p.y() < top && m_lastPosition.y() == top) {
                emit topReached(m_lastPosition);
            }
            else if (p.y() > bottom && m_lastPosition.y() == bottom) {
                emit bottomReached(m_lastPosition);
            }
        }

        m_lastPosition = position;
        proxyObject->emitMoveDocumentOffset(p.toPoint());
        emit panWidgets(position.x(), position.y());

        m_canvasItem->update();
    }
    qDebug() << "updatePosition finished";
    m_updatingPosition = false;
    m_lostPanGesture = false;
}

void PannableScrollBars::pinchStarted()
{
    pinchInProgress = true;
}

void PannableScrollBars::pinchFinished()
{
    pinchInProgress = false;
}

void PannableScrollBars::updatePositionTimeout()
{
    if(!physics()->inMotion()) {
        m_canvasItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    }

    updatePositionTimerRunning = false;
}

void PannableScrollBars::panningStopped()
{
    m_canvasItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);
}

PannableScrollBars::PanDirection PannableScrollBars::panDirection() const
{
    return m_panDirection;
}

// this is a HACK to cancel gesture events we get while pinch is in progress
// looks like it is a problem in qt that is not able to cancel event for qgraphicitem 
// based objects. Seem to work only for widgets.
void PannableScrollBars::gestureEvent(QGestureEvent *event)
{
    // get the active gestures
    QList<QGesture *> gestures = event->activeGestures();
    qDebug() << __PRETTY_FUNCTION__ << event << event->type() << event->gesture(Qt::PinchGesture) << gestures.size();
    foreach (QGesture * gesture, gestures) {
        //qDebug() << __PRETTY_FUNCTION__ << "    " << gesture << gesture->state();

        // if there is a new gesture started we clear the list of gestures
        // without that the gestures are the same and the == operator doesn't find out there was a change
        if (gesture->state() == Qt::GestureStarted) {
            m_gesturesToCancel = QList<QGesture *>();
        }
    }

    if (pinchInProgress) {
        qDebug() << __PRETTY_FUNCTION__ << "event ignored pinch in progress";
        event->ignore();
        m_gesturesToCancel = gestures;
    }
    else {
        if (gestures == m_gesturesToCancel) {
            qDebug() << __PRETTY_FUNCTION__ << "event ignored";
            event->ignore();
        }
        else {
            MPannableViewport::gestureEvent(event);
        }
    }
}

bool PannableScrollBars::hasOffset() const
{
    return m_hasOffset;
}

void PannableScrollBars::panGestureEvent(QGestureEvent *event, QPanGesture* panGesture)
{
    //qDebug() << __PRETTY_FUNCTION__ << panGesture->state();
    MPannableViewport::panGestureEvent(event, panGesture);
    switch (panGesture->state()) {
        case Qt::GestureStarted:
        case Qt::GestureUpdated:
            m_panning = true;
            break;
        case Qt::GestureFinished:
        case Qt::GestureCanceled:
            m_panning = false;
            // it is possible that the panGesture is already be finished before the first updatePosition is 
            // called due to the pan gesture. With the m_lostPanGesture we still emit bottomReached/topReached
            m_lostPanGesture = true;
            break;
        default:
            break;
    }
}

bool PannableScrollBars::isPanning() const
{
    return m_panning || m_lostPanGesture;
}
