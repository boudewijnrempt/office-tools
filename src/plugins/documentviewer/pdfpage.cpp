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
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QtGlobal>
#include <QDir>
#include <QPluginLoader>
#include <QReadWriteLock>

#include <MLayout>
#include <MSceneManager>
#include <MPannableViewport>
#include <MFlowLayoutPolicy>
#include <MLocale>
#include <MLabel>
#include <MApplication>

#include "pdfpage.h"
#include "pdfpagewidget.h"
#include "definitions.h"
#include "applicationwindow.h"
#include "actionpool.h"
#include "pdfthumbprovider.h"
#include "misc.h"
#include "pdfsearch.h"

#include "OfficeInterface.h"

class PdfPage::Private
{
public:
    Private()
        :  viewport(0)
        , hWidget(0)
        , innerWidget(0)
        , spacer(0)
        , innerLayout(0)
        , thumbProvider(&this->loader)
        , search(0)
    {
        lastVisibleSceneSize = ApplicationWindow::visibleSizeCorrect();
    };

    virtual ~Private() {};

    MPannableViewport     *viewport;
    MWidget               *hWidget;
    MWidget               *innerWidget;
    MLabel                *spacer;
    QGraphicsLinearLayout   *innerLayout;
    QList<PdfPageWidget *>  widgetList;
    PdfLoader               loader;
    QSize                   lastVisibleSceneSize;
    PdfThumbProvider        thumbProvider;
    QSizeF                  lastViewportSize;
    PdfSearch               *search;
};

PdfPage::PdfPage(const QString& filename, QGraphicsItem *parent)
    : DocumentPage(filename, parent)
    , d(new Private())
    , mDocument(0)
    , enable(false)
{
    connect(ApplicationWindow::GetSceneManager(), SIGNAL(orientationChangeFinished(const M::Orientation &)),
            this, SLOT(orientationChanged()));

    d->loader.setHighlightData(&searchData);

    connect(this, SIGNAL(verticalCenterOnPagePoint(int, qreal, int)),
            this, SLOT(setVerticalCenterOnPagePoint(int, qreal, int)), Qt::QueuedConnection);

    connect(this, SIGNAL(visibleAreaChanged()), this, SLOT(updateRange()));
}

PdfPage::~PdfPage()
{
    stopSearchThreads();

    qDebug() << __PRETTY_FUNCTION__;
    if (d->search) {
        delete d->search;
    }

    delete d;
    qDebug() << __PRETTY_FUNCTION__ << "END";
}


void PdfPage::createContent()
{
    MApplicationPage::createContent();

    setObjectName("pdfpage");
    createPdfView();

    initUI();

    const QDir pluginDir("/usr/lib/office-tools/plugins");
    const QStringList plugins = pluginDir.entryList(QDir::Files);

    for (int i = 0; i < plugins.size(); ++i) {
        QPluginLoader test(pluginDir.absoluteFilePath(plugins.at(i)));
        QObject *plug = test.instance();
        if (plug != 0) {

            OfficeInterface* inter = qobject_cast<OfficeInterface*>(plug);

            if (inter) {
                plug->setParent(this);

                if (inter && inter->pluginType() == "pdf") { // Other types not supported here
                    MAction *pluginAction = new MAction(inter->pluginName(), this);
                    connect(pluginAction, SIGNAL(triggered()), plug, SLOT(emitOpenSignal()));
                    connect(plug, SIGNAL(openMe(OfficeInterface*)), this, SLOT(openPlugin(OfficeInterface*)));
                    pluginAction->setLocation(MAction::ApplicationMenuLocation);
                    addAction(pluginAction);
                }
                else {
                    delete plug;
                }
            }
        }
    }
}

void PdfPage::createPdfView()
{
    d->innerWidget =new MWidget;
    Q_CHECK_PTR(d->innerWidget);
    connect(d->innerWidget, SIGNAL(visibleChanged()),
            this, SLOT(pagesVisibilityChanged()));

    d->innerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->innerWidget->setMinimumWidth(10);

    d->innerLayout = new QGraphicsLinearLayout(Qt::Vertical);
    Q_CHECK_PTR(d->innerLayout);
    d->innerLayout->setContentsMargins(0, 0, 0, 0);
    d->innerLayout->setSpacing(PixelsBetweenPages);

    d->spacer = new MLabel("");
    d->spacer->setMinimumHeight(0);
    d->spacer->setMaximumHeight(0);
    d->spacer->hide();

    d->innerWidget->setLayout(d->innerLayout);

    d->hWidget= Misc::createHorizontalWidget(d->innerWidget);
    d->viewport = pannableViewport();
    setCentralWidget(d->hWidget);
    d->viewport->setStyleName("viewerBackground");

    connect(d->viewport, SIGNAL(rangeChanged(const QRectF &)),
            this, SLOT(viewPortRangeChanged(const QRectF &)));

    connect(d->viewport, SIGNAL(positionChanged(const QPointF &)),
            this, SLOT(updatePosition(const QPointF &)));

    connect(d->viewport, SIGNAL(geometryChanged()),
            this, SLOT(geometryChanged()));
}

void PdfPage::loadDocument()
{
    d->widgetList.clear();
    d->loader.setScene(d->innerWidget->scene());
    d->loader.setWidgetName(PDFPAGEWIDGET);

    if(false == d->loader.load(documentName,mDocument)) {
        if(0 != mDocument && mDocument->isLocked())
            emit(loadFailed(documentName, qtTrId("qtn_offi_error_pass_protect")));
        else
            emit(loadFailed(documentName, qtTrId("qtn_offi_error_corrupt")));

        return;
    }

    d->search = new PdfSearch(mDocument, searchData);

    connect(&d->loader, SIGNAL(pageChanged(int, int)), this, SLOT(setPageCounters(int, int)), Qt::QueuedConnection);
    connect(d->search, SIGNAL(showPage(int)), this, SLOT(highlightResult(int)));//, Qt::DirectConnection);
    connect(d->search, SIGNAL(searchFinish()), this, SLOT(searchFinished()));

    d->widgetList.reserve(d->loader.numberOfPages());
    for(int i = 0; i < d->loader.numberOfPages(); i++) {
        PdfPageWidget *w = new PdfPageWidget(&d->loader, i);
        Q_CHECK_PTR(w);
        d->widgetList.append(w);
        connect(w, SIGNAL(showPage(int, QPointF)), this, SLOT(showPage(int, QPointF)));
        connect(w, SIGNAL(changZoomLevel(ZoomLevel)), this, SLOT(zoom(ZoomLevel)), Qt::QueuedConnection);

        connect(w, SIGNAL(requestApplicationQuit()), this, SLOT(requestApplicationQuit()));
        connect(w, SIGNAL(requestApplicationClose()), this, SLOT(requestApplicationClose()));
        connect(w, SIGNAL(requestSearch()), this, SLOT(requestSearch()));
        w->setObjectName(PDFPAGEWIDGET);
        d->innerLayout->addItem(w);
        d->innerLayout->setAlignment(w, Qt::AlignHCenter);
    }

    //Lest set default start zooming level
    ActionPool::instance()->getAction(ActionPool::ZoomFitToWidth)->trigger();

    d->thumbProvider.init(scene(), PDFPAGEWIDGET);
    d->loader.setCurrentPage(0);

    emit(loadSuccess(documentName));

    pageLoaded = true;
}

void PdfPage::zoom(ZoomLevel level)
{
    zoom(level, !m_blockRecenter);
}

void PdfPage::zoom(ZoomLevel level, bool keepCenter)
{
    qDebug() << __PRETTY_FUNCTION__  << keepCenter;
    if (level != m_lastZoom) {
        QSize size = ApplicationWindow::visibleSizeCorrect();

        qreal yRelativePos= 0;
        int currentPage=-1;
        int offset= 0;

        getVerticalCenterPagePoint(currentPage, yRelativePos, offset);

        m_lastZoom = level;

        qreal maxWidth = 0;
        foreach(PdfPageWidget *w, d->widgetList) {
            w->updateSize(size, m_lastZoom);
            qreal width = w->sizeHint(Qt::PreferredSize).width();
            if (width > maxWidth) {
                maxWidth = width;
            }
        }

        if(maxWidth  <= size.width()) {
            d->viewport->setPanDirection(Qt::Vertical);
        } else {
            d->viewport->setPanDirection(Qt::Vertical | Qt::Horizontal);
        }

        invalidatePdfPageLayouts();
        updateGeometry();
        // this is needed to update the geometry so that the range is correct when we set the 
        // zoom in pinchFinished
        d->innerLayout->activate();
        d->hWidget->layout()->activate();
        d->viewport->layout()->activate();
        d->viewport->layout()->activate();
        layout()->activate();
        qDebug() << "innerLayout" << d->innerLayout->isActivated();
        qDebug() << "hWidget" << d->hWidget->layout()->isActivated();
        qDebug() << "viewport" << d->viewport->layout()->isActivated();
        qDebug() << "this" << layout()->isActivated();
        if (0 <= currentPage && keepCenter) {
            qDebug() << __PRETTY_FUNCTION__ << "verticalCenterOnPagePoint" << currentPage << yRelativePos << offset;
            emit verticalCenterOnPagePoint(currentPage, yRelativePos, offset);
        }
    }
}


void PdfPage::searchText(DocumentPage::SearchMode mode, const QString &searchText)
{
    if(!pageLoaded) {
        return;
    }

    switch(mode) {

    case DocumentPage::SearchFirst:
        qDebug() <<"searchText";
        startSearch(searchText);
        break;

    case DocumentPage::SearchNext:

        if(enable) {
            searchNext();
        }

        break;

    case DocumentPage::SearchPrevious:

        if(enable) {
            searchPrev();
        }

        break;

    default :
        qDebug()<<"\nInvalid search type";
        break;
    }


}

void PdfPage::startSearch(const QString &searchText)
{
    qDebug()<<"startSearch**";
    int currentPage = d->loader.getCurrentPageIndex();

    //Clearing data
    clearSearchTexts();
    d->loader.setCurrentHighlight(0, 0);
    enable = false;

    if(!searchText.isEmpty()) {
        stopSearchThreads();

        d->search->setData(searchText, currentPage);

        qDebug()<<"d->search start**";
        d->search->start();
    }//if
}


void PdfPage::highlightResult(int pageIndex)
{
    int textXPos = 0;
    int textYPos = 0;
    qreal zoomRatio = 0.0;

    //int numOfPages = mDocument->numPages();

    qDebug()<<"*searchData.contains(pageIndex)"<<searchData.contains(pageIndex);

    if(searchData.contains(pageIndex)) {

        qDebug()<<"qRect:"<<searchData[pageIndex];
        d->loader.setCurrentHighlight(pageIndex, 0);

        zoomRatio = d->widgetList[pageIndex]->calcZoomFactor();

        QList <QRectF> pageHitResults = searchData.value(pageIndex);

        textXPos = pageHitResults.at(0).x()*zoomRatio;
        textYPos = pageHitResults.at(0).y()*zoomRatio;

        showPage(pageIndex,QPointF(textXPos, textYPos), false);
    }

    enable = true;
}

void PdfPage::searchFinished()
{
    qDebug()<<"searchFinished called";
    //exit the thread

    stopSearchThreads();

    matchesFound(searchData.count() > 0);
}


void PdfPage::searchNext()
{
    qDebug() << "SearchNext with searchText";

    qreal zoomRatio = 0.0;
    int textXPos = 0;
    int textYPos = 0;
    int pageIndex = 0;
    int currentHighlight = 0;
    int count = 0;

    //Getting the cuurent highlight pageindex and currentHighlight word of that page.
    d->loader.getCurrentHighlight(pageIndex, currentHighlight);

    if(searchData.contains(pageIndex)) {
        //Count the total number of hit for that pageIndex
        count = (searchData.value(pageIndex)).size();

        //Checking  to increase the pageIndex or not
        currentHighlight++;

        if(currentHighlight >= count) {
            int nextPageIndex = (pageIndex+1) % mDocument->numPages();

            while(nextPageIndex != pageIndex) {
                if(searchData.contains(nextPageIndex)) {
                    break;
                }

                nextPageIndex = (nextPageIndex+1) % mDocument->numPages();
            }

            //Making it zero as the first highlight value of the nextPage
            currentHighlight = 0;

            //Saving nextPageIndex to pageIndex
            pageIndex = nextPageIndex;
        }

        //Setting currentHighligted text and pageIndex
        d->loader.setCurrentHighlight(pageIndex, currentHighlight);

        zoomRatio = d->widgetList[pageIndex]->calcZoomFactor();

        //Getting the page hit results
        QList <QRectF> pagehitResults = searchData.value(pageIndex);

        qDebug()<< "Found at position " << currentHighlight << "rect:"<<pagehitResults.at(currentHighlight)<<endl;


        if(currentHighlight < pagehitResults.size()) {
            textXPos = pagehitResults.at(currentHighlight).x()*zoomRatio;
            textYPos = pagehitResults.at(currentHighlight).y()*zoomRatio;
        }

        showPage(pageIndex,QPointF(textXPos, textYPos), false);
    }
}

void PdfPage::searchPrev()
{
    qreal zoomRatio = 0.0;
    int textXPos = 0;
    int textYPos = 0;
    int pageIndex = 0;
    int currentHighlight =  0;

    //Getting the cuurent highlight pageindex and currentHighlight word of that page.
    d->loader.getCurrentHighlight(pageIndex, currentHighlight);

    if(searchData.contains(pageIndex)) {

        //Checking  to decerase the pageIndex or not
        currentHighlight--;

        if(currentHighlight < 0) {
            int nextPageIndex;

            if(pageIndex != 0) {
                nextPageIndex = pageIndex-1;
            } else {
                nextPageIndex = mDocument->numPages() -1 ;
            }

            while(nextPageIndex != pageIndex) {
                if(searchData.contains(nextPageIndex)) {
                    break;
                } else {
                    if(nextPageIndex != 0) {
                        nextPageIndex = nextPageIndex-1;
                    } else {
                        nextPageIndex = mDocument->numPages() -1 ;
                    }
                }
            }

            //We are decreasing the value as going in opposite direction
            currentHighlight = (searchData.value(nextPageIndex)).size() - 1;

            //Saving nextPageIndex to pageIndex
            pageIndex = nextPageIndex;
        }

        //Setting currentHighligted text and pageIndex
        d->loader.setCurrentHighlight(pageIndex, currentHighlight);

        zoomRatio = d->widgetList[pageIndex]->calcZoomFactor();

        //Getting the page hit results
        QList <QRectF> pagehitResults = searchData.value(pageIndex);

        if(currentHighlight >= 0) {
            //qDebug()<<"****XPos:"<<pagehitResults.at(currentHighlight).x()<<"YPos :"<<pagehitResults.at(currentHighlight).y()<<endl;
            textXPos = pagehitResults.at(currentHighlight).x()*zoomRatio;
            textYPos = pagehitResults.at(currentHighlight).y()*zoomRatio;
        }

        showPage(pageIndex,QPointF(textXPos, textYPos), false);
    }
}

void PdfPage::clearSearchTexts()
{
    stopSearchThreads();
    searchData.clear();
}

void PdfPage::stopSearchThreads()
{
    if (d->search && d->search->isRunning()) {
        d->search->cancel();
        d->search->quit();
        d->search->wait();
    }
}

void PdfPage::orientationChanged()
{
    if (m_lastZoom.isFitTo()) {
        ZoomLevel tmpZoom = m_lastZoom;
        m_lastZoom = ZoomLevel(ZoomLevel::FactorMode, 1.0, false);
        zoom(tmpZoom, true);
    }
    else {
        if (d->widgetList.size() >= currentPage && currentPage > 0) {
            PdfPageWidget *page = d->widgetList[currentPage - 1];
            qreal pageZoom = page->calcZoomFactor();
            if (pageZoom < minimumZoomFactor()) {
                ZoomLevel newZoom = ZoomLevel(ZoomLevel::FitToPage);
                zoom(newZoom, true);
            }
        }
    }

    if (d->viewport && d->viewport->range().width() == 0) {
        invalidatePdfPageLayouts();
        updateGeometry();
    }
}

void PdfPage::showPage(int pageIndex, QPointF rPoint, bool relativePoint)
{
    qDebug() << __PRETTY_FUNCTION__ << pageIndex << rPoint << relativePoint;
    QPointF         pagePoint = rPoint;
    PdfPageWidget   *widget   = 0;
    QSizeF         screenSize = ApplicationWindow::visibleSizeCorrect();

    if(0 <= pageIndex && d->widgetList.size() > pageIndex) {
        widget = d->widgetList[pageIndex];
    } else {
        return;
    }

    if(relativePoint) {
        //Lets not allow relative point to be out side of the page
        pagePoint = normalilizePoint(pagePoint, QPointF(1.0, 1.0));

        pagePoint = Misc::translateRelativePoint(pagePoint, widget->size());
    }

    if(QRectF(QPointF(0, 0), screenSize*0.6).contains(pagePoint)) {
        //If point is inside 60% of screen area then let show start from the page
        pagePoint = QRectF(QPointF(0, 0), screenSize).center();
    }

    centerOnPage(widget, pagePoint, screenSize);

    widget->update();
}

void PdfPage::centerOnPage(const PdfPageWidget *widget, const QPointF & centerPagePoint, const QSizeF &screenSize)
{
    qDebug() << __PRETTY_FUNCTION__ << centerPagePoint << screenSize << d->viewport->position() << d->viewport->geometry();
    QPointF pagePoint = centerPagePoint;

    QRectF sceneRect = QRectF(QPointF(0, 0), screenSize);

    if(sceneRect.width() >= widget->size().width()) {
        pagePoint.setX(0);
    }

    if(sceneRect.height() > widget->size().height()) {
        //If more then on page visible the center on the page center
        pagePoint.setY(widget->size().height() / 2);
    }

    pagePoint -= sceneRect.center();

    QPointF newPoint = d->viewport->mapFromItem(widget,pagePoint);

    QPointF viewportPosition = d->viewport->position() + newPoint;

    //Lets make sure that we don't go over range edges
    viewportPosition = normalilizePoint(viewportPosition,  d->viewport->range().bottomRight());

    // To avoid closing the search toolbar (if active) when the pdfpage changes the viewport position for
    // highlighting the result
    d->viewport->setPosition(viewportPosition);
    d->loader.setCurrentPage(widget->getPageIndex());
}

QPointF PdfPage::normalilizePoint(const QPointF & point, const QPointF & maxPoint)
{
    static const qreal zero = 0;
    QPointF retval = point;
    qreal &x = retval.rx();
    x= qMax(x, zero);
    x= qMin(x, maxPoint.x());

    qreal &y = retval.ry();
    y= qMax(y, zero);
    y= qMin(y, maxPoint.y());
    return retval;
}

void PdfPage::setVerticalCenterOnPagePoint(int pageIndex, qreal relativeY, int offset)
{
    qDebug() << __PRETTY_FUNCTION__ << pageIndex << " relativeY :" << relativeY << " offset:" << offset;

    if(0 <= pageIndex && d->widgetList.size() > pageIndex) {

        PdfPageWidget *widget = d->widgetList[pageIndex];

        QPointF pagePoint = QPointF(0.0, 0.0);
        pagePoint.setY(relativeY * widget->size().height());
        pagePoint.setX(0.5 * widget->size().width());

        QPointF newPoint = d->viewport->mapFromItem(widget,pagePoint);

        newPoint.setY(newPoint.y() - ApplicationWindow::visibleSize().height() / 2 + offset);

        QPointF viewportPosition = d->viewport->position();
        viewportPosition.setY(viewportPosition.y() + newPoint.y());
        qreal width = d->viewport->range().width();

        qDebug() << __PRETTY_FUNCTION__ << d->viewport->range();

        if (width > 0) {
            viewportPosition.setX(width / 2.0);
        }

        // To avoid closing the search toolbar (if active) when the pdfpage changes the viewport position for
        // highlighting the result
        d->viewport->setPosition(viewportPosition);
        d->loader.setCurrentPage(widget->getPageIndex());
    }
}

void PdfPage::getVerticalCenterPagePoint(int &pageIndex, qreal &relativeY, int &offset) const
{
    pageIndex = -1;
    relativeY = 0;
    offset = 0;

    int curPageIndex = d->loader.getCurrentPageIndex();

    qDebug() << __PRETTY_FUNCTION__ << curPageIndex;

    if(0 > curPageIndex || d->widgetList.size() <= curPageIndex) {
        return;
    }

    PdfPageWidget * pdfPageWidget = d->widgetList.at(curPageIndex);

    QSizeF size = ApplicationWindow::visibleSize();

    // this is needed as the points pressed on screen are not rotated in the scene.
    QPointF center = M::Landscape == ApplicationWindow::GetSceneManager()->orientation() ?
        QPointF(size.width()/2.0, size.height()/2.0) :
        QPointF(size.height()/2.0, size.width()/2.0);

    QPointF point = pdfPageWidget->mapFromScene(center);
    qreal y = point.y();

    pageIndex = pdfPageWidget->getPageIndex();

    qreal height = pdfPageWidget->size().height();

    if(0 > y) {
        offset = y;
        relativeY = 0;
    } else if(y > height) {
        offset = y - height;
        relativeY = 1;
    } else {
        relativeY = y / height;
    }

    qDebug() << __PRETTY_FUNCTION__ << center << point << pdfPageWidget->size() << pageIndex << relativeY << offset;
}

void PdfPage::invalidatePdfPageLayouts()
{
    qDebug() << __PRETTY_FUNCTION__;
    d->innerWidget->layout()->invalidate();
    d->hWidget->layout()->invalidate();
    d->viewport->layout()->invalidate();
}


void PdfPage::shortTap(const QPointF &point, QObject *object)
{
    bool setNormalSceenMode=true;

    DocumentPage::shortTap(point, object);

    PdfPageWidget *widget=getWidgetAt(point, PDFPAGEWIDGET);
    qDebug() << __PRETTY_FUNCTION__ << widget;

    if(0 != widget) {
        qDebug() << __PRETTY_FUNCTION__ << widget->getPageIndex();
        if (widget->linkTaped(widget->mapFromScene(point))) {
            setNormalSceenMode = false;
        }
    }
}

void PdfPage::requestApplicationQuit()
{
    //notImplementedBanner("PDF LINK: requestApplicationQuit");
}

void PdfPage::requestApplicationClose()
{
    //notImplementedBanner("PDF LINK: requestApplicationClose");
}

void PdfPage::requestSearch()
{
    //notImplementedBanner("PDF LINK: requestSearch");
}

PdfPageWidget * PdfPage::getWidgetAt(QPointF point, const QString &widgetName)
{
    PdfPageWidget *widget= 0;

    QList<QGraphicsItem *> visibleItems;

    if(0 != scene()) {
        visibleItems = scene()->items(point);
    }

    qDebug() << __PRETTY_FUNCTION__ << visibleItems.size();

    foreach(QGraphicsItem * item, visibleItems) {

        PdfPageWidget *newitem = qgraphicsitem_cast<PdfPageWidget *>(item);

        if(0 != newitem &&  widgetName == newitem->objectName()) {
            widget = newitem;
            break;
        }
    }

    return widget;
}

void PdfPage::pagesVisibilityChanged()
{
    d->loader.loadNeighborPages();
}

ThumbProvider* PdfPage::getThumbProvider()
{
    return &d->thumbProvider;
}

void PdfPage::showPageIndexInternal(int pageIndex)
{
    showPage(pageIndex, QPointF(0,0));
}

void PdfPage::viewPortRangeChanged(const QRectF &range)
{
    if(d->lastViewportSize.width() != range.width()) {
        Qt::Alignment alignment = Qt::AlignHCenter;

        if(range.width() > ApplicationWindow::visibleSizeCorrect().width()) {
            alignment = Qt::AlignLeft;
        }

        foreach(PdfPageWidget *w, d->widgetList) {

            if(alignment != d->innerLayout->alignment(w)) {
                d->innerLayout->setAlignment(w, alignment);
            }
        }
    }

    d->lastViewportSize = range.size();
}

void PdfPage::openPlugin(OfficeInterface *plugin)
{
    plugin->setDocument(mDocument);
    MApplicationPage *pluginView = plugin->createView();
    pluginView->appear(scene(), MSceneWindow::DestroyWhenDismissed);
}

void PdfPage::pinchStarted(QPointF &center)
{
    QSize size = ApplicationWindow::visibleSizeCorrect();
    QSize documentSize = d->innerWidget->geometry().size().toSize();

    QPointF offset;
    if (size.width() > documentSize.width()) {
        center.rx() = size.width() / 2;
        offset.setX((documentSize.width() - size.width()) / 2.0);
    }

    if (size.height() > documentSize.height()) {
        center.ry() = size.height() / 2;
        offset.setY((documentSize.height() - size.height()) / 2.0);
    }

    m_pinchCenterDocument = center + d->viewport->position() + offset;
    qDebug() << "Start Point" << center << d->viewport->position() << m_pinchCenterDocument << offset;
    // autorange needs to be disabled otherwise setHorizontalPanningPolicy triggers a relayout which results in a setRange
    // which the set the position to 0,0 and we see the first page when zooming
    d->viewport->setAutoRange(false);
    d->viewport->setHorizontalPanningPolicy(MPannableWidget::PanningAlwaysOff);
    d->viewport->setVerticalPanningPolicy(MPannableWidget::PanningAlwaysOff);
}

qreal PdfPage::pinchUpdated(qreal zoomFactor)
{
    if (d->widgetList.size() >= currentPage && currentPage > 0) {
        PdfPageWidget *page = d->widgetList[currentPage - 1];
        qreal pageZoom = page->calcZoomFactor();
        qreal effectiveZoomFactor = pageZoom * zoomFactor;

        qreal minScale = minimumZoomFactor();
        qDebug() << __PRETTY_FUNCTION__ << minScale;
        //Minimum is fit to page or 100 % (the smaller one)
        minScale = qMin(MinZoomFactor, minScale);

        qDebug() << __PRETTY_FUNCTION__ << effectiveZoomFactor << minScale << MaxZoomFactor;
        if (minScale > effectiveZoomFactor) {
            return minScale / pageZoom;
        }
        else if (effectiveZoomFactor > MaxZoomFactor) {
            return MaxZoomFactor / pageZoom;
        }
    }
    return zoomFactor;
}

void PdfPage::pinchFinished(const QPointF &center, qreal scale)
{
    if (d->widgetList.size() >= currentPage && currentPage > 0) {
        PdfPageWidget *page = d->widgetList[currentPage - 1];
        qreal pageZoom = page->calcZoomFactor() * scale;

        ZoomLevel level(qFuzzyCompare(pageZoom, minimumZoomFactor()) ? ZoomLevel::FitToPage : ZoomLevel::FactorMode, pageZoom);
        zoom(level, false);

        qreal appliedZoom = page->calcZoomFactor();

        if (pageZoom != appliedZoom) {
            qDebug() << __PRETTY_FUNCTION__ << "different zoom" << pageZoom << appliedZoom << scale;
            qreal factor = appliedZoom / pageZoom;
            scale *= factor;
        }

        QPointF newPos = m_pinchCenterDocument * scale - center;
        qDebug() << "XXX finish" << m_pinchCenterDocument << newPos << center << scale << d->viewport->range();
        d->viewport->setHorizontalPanningPolicy(MPannableWidget::PanningAlwaysOn);
        d->viewport->setVerticalPanningPolicy(MPannableWidget::PanningAlwaysOn);
        d->viewport->setAutoRange(true);
        // the set position needs to be done as last as setAutoRange triggers sometimes a set position 
        // and the position is not correct
        if (newPos.y() < 0) {
            newPos.setY(0);
        }
        if (newPos.x() < 0) {
            newPos.setX(0);
        }
        d->viewport->setPosition(newPos);
    }
}

QGraphicsWidget *PdfPage::pinchWidget()
{
    return d->viewport;
}

void PdfPage::updatePosition(const QPointF &position)
{
    Q_UNUSED(position)
    //qDebug() << __PRETTY_FUNCTION__ << position << d->viewport->range() << d->viewport->geometry() << d->hWidget->geometry() << d->innerLayout->geometry() << geometry();
}

void PdfPage::geometryChanged()
{
#if 0
    qDebug() << __PRETTY_FUNCTION__ << d->viewport->range() << d->viewport->geometry();
    if (d->viewport->pos().y() == 0) {
        qDebug() << __PRETTY_FUNCTION__;
    }
#endif
}

qreal PdfPage::minimumZoomFactor() const
{
    QSize size = ApplicationWindow::visibleSizeCorrect();
    QSize pageSize = d->loader.pageSize(currentPage -1);

    qreal minScaleWidth = PdfPageWidget::calcScale(size.width(), pageSize.width()) / PdfLoader::DPIPerInch;
    qreal minScaleHeight = PdfPageWidget::calcScale(size.height(), pageSize.height()) / PdfLoader::DPIPerInch;
    return qMin(minScaleWidth, minScaleHeight);
}

void PdfPage::updateRange()
{
    QRectF vRect(0, 0, 0, 0);
    MApplicationWindow *window = MApplication::activeApplicationWindow();
    if (window && window->currentPage()) {
        DocumentPage *page = qobject_cast<DocumentPage*>(window->currentPage());
        if (page) {
            vRect = page->visibleRect();
        }
    }
    if (vRect.top() > 0) {
        qreal spacerHeight = vRect.top() - PixelsBetweenPages;
        d->spacer->setMinimumHeight(spacerHeight);
        d->spacer->setMaximumHeight(spacerHeight);
        d->innerLayout->insertItem(0, d->spacer);
        d->spacer->show();
    }
    else {
        d->innerLayout->removeItem(d->spacer);
        d->spacer->hide();
    }
}
