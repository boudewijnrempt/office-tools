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
#include <QGraphicsSceneMouseEvent>
#include <QPinchGesture>
#include <QGestureEvent>
#include <QFileInfo>
#include <QPropertyAnimation>

#include <MAction>
#include <MApplication>
#include <MApplicationWindow>
#include <MSceneManager>
#include <MLayout>
#include <MWidgetAction>
#include <MComboBox>
#include <MWidgetView>
#include <MBasicLayoutAnimation>
#include <MPannableViewport>

#include "thumbpagelayoutpolicy.h"
#include "allpagespage.h"
#include "thumbwidget.h"
#include "thumbprovider.h"
#include "officethumbprovider.h"
#include "actionpool.h"
#include "definitions.h"
#include "applicationwindow.h"
#include "pageindicator.h"

static const int NAVI_BAR_TIMEOUT = 5000;

class AllPagesPagePrivate
{

public:
    AllPagesPagePrivate();
    virtual ~AllPagesPagePrivate();

    QVector<ThumbPageLayoutPolicy*> policies;
    QList<ThumbWidget *>            widgets;
    ThumbProvider                   *thumbProvider;
    bool                            dirty;
    ActionPool::Id                  zoomOutLevel;
    bool                            zoomDone;
    MComboBox                       *combobox;
    QPropertyAnimation              *bounceAnimation;
    bool                            alreadyBounced;
};

AllPagesPagePrivate::AllPagesPagePrivate()
    : policies(4)
    , thumbProvider(0)
    , dirty(true)
    , zoomOutLevel(ActionPool::TwoThumbsPerColumn)
    , zoomDone(0)
    , combobox(0)
    , bounceAnimation(0)
    , alreadyBounced(false)
{
}

AllPagesPagePrivate::~AllPagesPagePrivate()
{
    qDebug() << __PRETTY_FUNCTION__;
    thumbProvider = 0;
}


AllPagesPage::AllPagesPage(const QString& document, const QString& urn, bool spreadSheet, QGraphicsItem *parent)
    : MApplicationPage(parent)
    , data(0)
    , documentUrn(urn)
    , isSpreadsheet(spreadSheet)
{
    Q_UNUSED(document);
    setObjectName("allpagespage");
    data = new AllPagesPagePrivate;
    connect(MApplication::activeApplicationWindow()->sceneManager(), SIGNAL(orientationChanged(const M::Orientation &)),
            this, SLOT(updateSizes()));

    connect(ActionPool::instance(), SIGNAL(destroyed(QObject *)),
            this, SLOT(removeActions()));

    qRegisterMetaType<ZoomLevel>("ZoomLevel");

    setAcceptTouchEvents(true);
    grabGesture(Qt::TapGesture);
}

AllPagesPage::~AllPagesPage()
{
    qDebug() << __PRETTY_FUNCTION__;
    removeActions();
    removeWidgets();
    delete data;
    data = 0;
}

void AllPagesPage::createContent()
{
    pannableViewport()->setStyleName("viewerBackground");
    MLayout *layout = new MLayout;
    Q_CHECK_PTR(layout);
    setObjectName("allpagespage_layout");
    new MBasicLayoutAnimation(layout);

    for(int i = 0 ; i < data->policies.size(); i++) {
        ThumbPageLayoutPolicy *policy = new ThumbPageLayoutPolicy(layout, i+2);
        data->policies[i] = policy;
        policy->MAbstractLayoutPolicy::setStyleName(QString("AllPagesPage%1PageLayoutPolicy").arg(i+2));
        connect(policy, SIGNAL(policyActivated()),
                this, SLOT(updateSizes()));
    }

    centralWidget()->setLayout(layout);
    data->zoomOutLevel = ActionPool::TwoThumbsPerColumn;
    ActionPool::instance()->getAction(ActionPool::TwoThumbsPerColumn)->trigger();
    prepareToAppear();
}

void AllPagesPage::showFindView()
{
    qDebug()<<__PRETTY_FUNCTION__;
    ActionPool::instance()->getAction(ActionPool::Find)->trigger();
    ActionPool::instance()->getAction(ActionPool::ShowNormalView)->trigger();
}

void AllPagesPage::showJumpToolbar()
{
    qDebug()<<__PRETTY_FUNCTION__;
    ActionPool::instance()->getAction(ActionPool::EnterPageNumber)->trigger();
    ActionPool::instance()->getAction(ActionPool::ShowNormalView)->trigger();
}

void AllPagesPage::removeWidgets()
{
    foreach(ThumbWidget *widget, data->widgets) {
        foreach(ThumbPageLayoutPolicy *policy, data->policies) {
            policy->removeItem(widget);
        }

        delete widget;
    }

    data->widgets.clear();
    data->dirty=true;
}

void AllPagesPage::addWidgets()
{
    if(0 != data->thumbProvider) {
        setTitle(title);
        int pageCount = data->thumbProvider->getPageCount();

        for(int pageCounter = 0; pageCounter < data->thumbProvider->getPageCount(); pageCounter++) {
            ThumbWidget *widget = new ThumbWidget(data->thumbProvider, isSpreadsheet);
            widget->setPageIndex(pageCounter, pageCount);
            data->widgets.append(widget);

            for(int policyCounter = 0 ; policyCounter < data->policies.size(); policyCounter++) {
                data->policies[policyCounter]->addItem(widget, pageCounter/(policyCounter+2), pageCounter%(policyCounter+2));
            }
        }

        updateSizes();
    }
}


void AllPagesPage::setCurrentPage(int nPageIndex)
{
    currentPage = nPageIndex;
    if (0 != data->thumbProvider) {
        data->thumbProvider->setPageIndex(nPageIndex);
    }
}

bool AllPagesPage::isDirty()
{
    bool dirty=true;

    if(0 != data->thumbProvider &&
       false == data->dirty &&
       data->widgets.size() == data->thumbProvider->getPageCount()) {
        dirty=false;
    }

    return dirty;
}

bool AllPagesPage::event(QEvent *e)
{
    if(e->type() == QEvent::TouchBegin) {
//        data->zoomDone = false;
        e->setAccepted(true);
        return true;
    }

    return MWidgetController::event(e);
}

void AllPagesPage::pinchGestureEvent(QGestureEvent *event, QPinchGesture *gesture)
{
    Q_UNUSED(event);
    Q_UNUSED(gesture);
#if 0
    static qreal lastZoomedFactor;
    QPointF centerPoint(mapFromScene(gesture->centerPoint()));
    qDebug()<<__PRETTY_FUNCTION__ << gesture->centerPoint() << centerPoint;

    if(gesture->state() == Qt::GestureStarted) {
        qDebug() << "pinchStarted" << centerPoint;
        data->alreadyBounced = false;
        lastZoomedFactor = 1.0;
    }

    if(gesture->state() == Qt::GestureFinished || gesture->state() == Qt::GestureCanceled) {
        qDebug() << "pinchFinished" << centerPoint;
        lastZoomedFactor = 1.0;
    }
    if(gesture->state() == Qt::GestureUpdated) {
        qDebug() << "pinch zoom" << gesture->totalScaleFactor() << lastZoomedFactor;
        if((lastZoomedFactor - gesture->totalScaleFactor() > 0.005) && (!data->alreadyBounced)) {
            if (data->bounceAnimation->state() != QAbstractAnimation::Running) {
                data->bounceAnimation->start();
                data->alreadyBounced = true;
            }
        } else if(lastZoomedFactor - gesture->totalScaleFactor() < -0.005) {
            lastZoomedFactor = gesture->totalScaleFactor();
            zoomIn(centerPoint);
        }
    }

    event->accept(gesture);
#endif
}

void AllPagesPage::tapGestureEvent(QGestureEvent *event, QTapGesture *gesture)
{
    static QPointF startPoint(0,0);
    static QTime time(0,0);

    QPointF position(mapFromScene(gesture->position()));

    qDebug() << __PRETTY_FUNCTION__ << gesture->position() << position << sceneManager()->orientation();

    if(gesture->state() == Qt::GestureStarted) {
        startPoint = position;
    }

    if(gesture->state() == Qt::GestureFinished) {
        QPointF diff = position - startPoint;

        if(diff.manhattanLength() < 5) {
            if(time.elapsed() > QApplication::doubleClickInterval()) {
                shortTap(QRectF(position, startPoint), this);
                time.start();
            } else {
//                doubleTap(QRectF(position, startPoint).center());
            }
        }
    }

    event->accept(gesture);
}

#if 0
void AllPagesPage::zoomOut()
{
    if(data->zoomOutLevel == ActionPool::TwoThumbsPerColumn) {
        data->zoomOutLevel = ActionPool::ThreeThumbsPerColumn;
        ActionPool::instance()->getAction(data->zoomOutLevel)->trigger();
    } else if(data->zoomOutLevel == ActionPool::ThreeThumbsPerColumn) {
        data->zoomOutLevel = ActionPool::FourThumbsPerColumn;
        ActionPool::instance()->getAction(data->zoomOutLevel)->trigger();
    } else {
        data->zoomOutLevel = ActionPool::FourThumbsPerColumn;
    }

}

void AllPagesPage::zoomIn(QPointF center)
{

    if(data->zoomOutLevel == ActionPool::FourThumbsPerColumn) {
        data->zoomOutLevel = ActionPool::ThreeThumbsPerColumn;
        ActionPool::instance()->getAction(data->zoomOutLevel)->trigger();
    } else if(data->zoomOutLevel == ActionPool::ThreeThumbsPerColumn) {
        data->zoomOutLevel = ActionPool::TwoThumbsPerColumn;
        ActionPool::instance()->getAction(data->zoomOutLevel)->trigger();
    } else if(data->zoomOutLevel == ActionPool::TwoThumbsPerColumn) {
        data->zoomOutLevel = ActionPool::TwoThumbsPerColumn;
        int pageIndex = getPageAt(center);
        if (-1 != pageIndex) {
            qDebug() << __PRETTY_FUNCTION__ << pageIndex;
            ActionPool::instance()->getAction(ActionPool::ShowNormalView)->trigger();
            emit showPageIndexDefaultZoom(pageIndex);
        }
    }
}
#endif

void AllPagesPage::updateSizes()
{
    QSizeF size = MApplication::activeApplicationWindow()->visibleSceneSize();
    centralWidget()->setMinimumWidth(size.width());
    centralWidget()->setMaximumWidth(size.width());
    ThumbPageLayoutPolicy *activePolicy = 0;
    foreach(ThumbPageLayoutPolicy *policy, data->policies) {
        if(policy->isActive()) {
            activePolicy = policy;
            break;
        }
    }

    if(0 != activePolicy) {
        int curCols = data->policies.indexOf(activePolicy) + 2;
        QString objName = QString("AllPages%1Columns").arg(curCols);
        foreach(ThumbWidget *widget, data->widgets) {
            widget->setNames(objName);
            widget->setSize(widget->style()->preferredSize(),curCols);
        }
    }
    centralWidget()->layout()->invalidate();
    update();
}

void AllPagesPage::prepareToAppear()
{
    if(data->combobox)
        data->combobox->setCurrentIndex(-1);

    //Note:- Dont delete this is required while pinch out from normal view to all pages view.
    // otherwise it will move directly to old store value.
    ActionPool::instance()->getAction(ActionPool::TwoThumbsPerColumn)->trigger();

    if(isDirty() && isContentCreated()) {
        removeWidgets();
        addWidgets();
    }

    if(0 != data->thumbProvider) {
        data->thumbProvider->updateVisibleAreas();
    }
}

void AllPagesPage::pagesChanged()
{
    data->dirty=true;
}

void AllPagesPage::addThumbProvider(ThumbProvider *newThumbProvider)
{
    removeWidgets();
    data->thumbProvider=newThumbProvider;
    connect(data->thumbProvider, SIGNAL(pagesChanged()),
            this, SLOT(pagesChanged()));
    connect(data->thumbProvider, SIGNAL(destroyed(QObject *)),
            this, SLOT(thumbProviderDestroyed()));
    connect(centralWidget(), SIGNAL(visibleChanged()),
            data->thumbProvider, SLOT(thumbsVisibilityChanged()));

    connect(pannableViewport(), SIGNAL(panningStopped()), data->thumbProvider, SLOT(panningStopped()));
    connect(pannableViewport(), SIGNAL(positionChanged(QPointF)), data->thumbProvider, SLOT(panningStarted(QPointF)));

    data->dirty=true;
}

void AllPagesPage::thumbProviderDestroyed()
{
    removeWidgets();
    data->thumbProvider = 0;
    data->dirty=true;

}

void AllPagesPage::removeActions()
{
    foreach(QAction *action, actions()) {
        removeAction(action);
    }
}

void AllPagesPage::shortTap(QRectF rect, QObject *object)
{
    Q_UNUSED(object);
    int pageIndex = getPageAt(rect.center());
    if (-1 != pageIndex) {
        qDebug() << __PRETTY_FUNCTION__ << pageIndex;
        emit showPageIndexDefaultZoom(pageIndex);
    }
}

void AllPagesPage::longTap(QRectF rect)
{
    //qDebug() << __PRETTY_FUNCTION__ << " point:" << point;
    Q_UNUSED(rect);
}

void AllPagesPage::doubleTap(QPointF point)
{
    Q_UNUSED(point);
}

int AllPagesPage::getPageAt(const QPointF &point)
{
    int pageIndex=-1;

    QList<QGraphicsItem *> visibleItems;

    if(0 != scene()) {
        visibleItems = scene()->items(mapToScene(point));
    }

    foreach(QGraphicsItem * item, visibleItems) {

        ThumbWidget *newitem = qgraphicsitem_cast<ThumbWidget *>(item);

        if(0 != newitem) {
            pageIndex = newitem->getPageIndex();
            break;
        }
    }

    return pageIndex;
}

void AllPagesPage::createCombo(MAction *label, MAction *one, MAction *two, MAction *three)
{
    data->combobox = new MComboBox();
    Q_CHECK_PTR(data->combobox);
    data->combobox->setObjectName("allpagespage_combobox");
    MWidgetAction *widgetAction = new MWidgetAction(this);
    Q_CHECK_PTR(widgetAction);
    data->combobox->setObjectName("allpagespage_combobox_widgetaction");
    data->combobox->setTitle(label->text());

    widgetAction->setLocation(MAction::ApplicationMenuLocation);
    connect(data->combobox, SIGNAL(activated(int)), this, SLOT(buttonClicked(int)));

    data->combobox->addItem(one->text());
    data->combobox->addItem(two->text());
    data->combobox->addItem(three->text());
    widgetAction->setWidget(data->combobox);
    addAction(widgetAction);
    data->combobox->setCurrentIndex(-1);
}

void AllPagesPage::buttonClicked(int buttonId)
{
    data->combobox->setCurrentIndex(buttonId);

    //Set the current zoomOutLevel and triger the action to set the zoom
    ActionPool::instance()->getAction(ActionPool::ShowNormalView)->trigger();

    switch(data->combobox->currentIndex()) {

    case 0: //Fit to Width
        ActionPool::instance()->getAction(ActionPool::ZoomFitToWidth)->trigger();
        break;

    case 1://Fit to Page
        ActionPool::instance()->getAction(ActionPool::ZoomFitToPage)->trigger();
        break;

    case 2://Zoom 100%
        ActionPool::instance()->getAction(ActionPool::Zoom100percent)->trigger();
        break;

    default://Normal View
        ActionPool::instance()->getAction(ActionPool::ShowNormalView)->trigger();
        break;
    }
}

void AllPagesPage::scrollToCurrentVisiblePage()
{
    int row = currentPage/2;

    if (data->widgets.count() > 0) {
        qreal widgetHeight = data->widgets.first()->preferredSize().height();
        pannableViewport()->setPosition(QPointF(0, (widgetHeight + 10)*row));
        qDebug() << __PRETTY_FUNCTION__ << QPointF(0, widgetHeight*row) << row;
    }
}
