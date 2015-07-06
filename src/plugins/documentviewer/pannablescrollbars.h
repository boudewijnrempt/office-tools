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

#ifndef PannableScrollBars_H
#define PannableScrollBars_H

#include <MPannableViewport>
#include <KoCanvasController.h>
#include "documentviewer_export.h"
class QScrollBar;

class KoZoomHandler;

/*!
 * \class PannableScrollBars
 * \brief The class makes MPannableViewport class to work with QScrollBar.
 * The QScrollBar scrolls updates the position indicator and panning from
 * MPannableViewport updates the QScrollBar position.
 * The panned widget position is not updated because it should be viewport to
 * QAbstractScrollArea and position is changed based on QScrollBar.
  */

class DOCUMENTVIEWER_EXPORT PannableScrollBars : public MPannableViewport, public KoCanvasController
{
    Q_OBJECT

public:
    enum PanDirection {
        PanUp,
        PanDown
    };

    PannableScrollBars(QGraphicsItem *parent = 0);
    ~PannableScrollBars();

    void setZoomHandler(const KoZoomHandler *zoomHandler) {
        m_zoomHandler = zoomHandler;
    }

// KoCanvasController implementation

    void scrollContentsBy(int dx, int dy);
    virtual QSize viewportSize() const;
    void setDrawShadow(bool drawShadow);
    virtual void setCanvas(KoCanvasBase *canvas);
    KoCanvasBase *canvas() const;
    int visibleHeight() const;
    int visibleWidth() const;
    int canvasOffsetX() const;
    int canvasOffsetY() const;
    void ensureVisible(const QRectF &rect, bool smooth = false);
    void ensureVisible(KoShape *shape);
    void zoomIn(const QPoint &center);
    void zoomOut(const QPoint &center);
    void zoomBy(const QPoint &center, qreal zoom);
    void zoomTo(const QRect &rect);
    void recenterPreferred();
    void setPreferredCenter(const QPoint &viewPoint);
    QPoint preferredCenter() const;
    void pan(const QPoint &distance);
    void setMargin(int margin);
    QPoint scrollBarValue() const;
    void setScrollBarValue(const QPoint &value);
    void updateDocumentSize(const QSize &sz, bool recalculateCenter = true);
    void updateRange();
    QCursor setCursor(const QCursor &cursor);
    void pinchStarted();
    void pinchFinished();
    void resetLayout();

    // Dummy Implementation of Abstract class KoCanvasController
    // to make compiler happy
    void setZoomWithWheel(bool value) {
        Q_UNUSED(value);
    }

    void setVastScrolling(qreal value) {
        Q_UNUSED(value);
    }

    PanDirection panDirection() const;

    bool hasOffset() const;

    bool isPanning() const;

signals:
    void panWidgets(qreal x, qreal y);
    void topReached(const QPointF &lastPosition);
    void bottomReached(const QPointF &lastPosition);

public slots:
    void documentOffsetMoved(const QPoint&);

public:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

protected:
    void panGestureEvent(QGestureEvent *event, QPanGesture* panGesture);

protected slots:

// MPannableViewport overrides

    void updatePosition(const QPointF &position);
    void updatePositionTimeout();
    void panningStopped();

    virtual void gestureEvent(QGestureEvent *event);

private:
    const KoZoomHandler *m_zoomHandler;
    KoCanvasBase *m_canvas;
    QGraphicsWidget *m_canvasItem;
    int m_margin;
    bool m_ignoreScrollSignals;
    bool pinchInProgress;
    bool updatePositionTimerRunning;
    QPointF m_lastPosition;
    bool m_updatingPosition;
    PanDirection m_panDirection;
    QList<QGesture *> m_gesturesToCancel;
    bool m_hasOffset;
    bool m_panning;
    bool m_lostPanGesture;
};

#endif //PannableScrollBars_H
