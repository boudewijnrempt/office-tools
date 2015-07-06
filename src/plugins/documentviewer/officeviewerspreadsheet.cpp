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
#include <QGraphicsGridLayout>
#include <QGraphicsItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

//Include M stuff
#include <MApplication>
#include <MBanner>
#include <MGridLayoutPolicy>
#include <MLayout>
#include <MLabel>

//Include Koffice stuff
#include <KoToolManager.h>
#include <KoZoomController.h>
#include <KoZoomHandler.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoCanvasController.h>
#include <KoGlobal.h>
#include <KoShapePainter.h>

#include <tables/Sheet.h>
#include <tables/part/Doc.h>
#include <tables/Map.h>
#include <tables/part/View.h>
#include <tables/part/CanvasItem.h>
#include <tables/part/HeaderItems.h>
#include <tables/PrintSettings.h>
#include <tables/ui/SheetView.h>
#include <tables/part/ToolRegistry.h>

//Include application stuff
#include "officeviewerspreadsheet.h"
#include "definitions.h"
#include "applicationwindow.h"
#include "thumbprovider.h"
#include "pannablescrollbars.h"
#include "actionpool.h"
#include "spreadsheetcommon.h"
#include "officeviewereventfilter.h"

SpreadsheetPannableScrollBars::SpreadsheetPannableScrollBars(QGraphicsItem *parent)
    : PannableScrollBars(parent),
      canvasItem(0),
      indicatorsVisible(true)
{
}

SpreadsheetPannableScrollBars::~SpreadsheetPannableScrollBars()
{
}

void SpreadsheetPannableScrollBars::setCanvas(KoCanvasBase *canvas)
{
    qDebug() << __PRETTY_FUNCTION__ << canvas;
    PannableScrollBars::setCanvas(canvas);

    if(canvas) {
        canvasItem = dynamic_cast<Calligra::Tables::CanvasItem *>(canvas);
        Q_ASSERT(canvasItem);
    } else {
        canvasItem = 0;
    }
}

void SpreadsheetPannableScrollBars::restoreStartPoint()
{
    QPointF position;
    setPosition(position);
}

void SpreadsheetPannableScrollBars::setIndicatorsStatus(bool status)
{
    indicatorsVisible = status;
}

QSize SpreadsheetPannableScrollBars::viewportSize() const
{
    QSize size(PannableScrollBars::viewportSize());
    if (indicatorsVisible && canvasItem) {
        size.setWidth(size.width() - canvasItem->zoomHandler()->zoomItX(YBORDER_WIDTH));
        size.setHeight(size.height() - canvasItem->zoomHandler()->zoomItY(KoGlobal::defaultFont().pointSizeF() + 3));
    }
    return size;
}

/*!
 * \class OfficeViewerSpreadsheetPrivateData
 * \brief The class contains private data for #OfficeViewerSpreadsheet
 * The class contains UI items needed to create Spreadsheet widget.
 */

class OfficeViewerSpreadsheet::Private
{

public:
    Private()
        : pannableScrollbars(0)
        , search(0)
        , utils(0)
        , canvasItem(0)
        , rowHeader(0)
        , columnHeader(0)
        , showIndicatorsLayout(0)
        , hideIndicatorsLayout(0)
        , rootWidget(0)
        , label(0) // used for making the pan area smaller when quick viewer is visible 
    {
    }

    SpreadsheetPannableScrollBars *pannableScrollbars;
    SpreadsheetSearch             *search;
    SpreadsheetUtils              *utils;
    Calligra::Tables::CanvasItem           *canvasItem;
    Calligra::Tables::RowHeaderItem        *rowHeader;
    Calligra::Tables::ColumnHeaderItem     *columnHeader;
    MGridLayoutPolicy             *showIndicatorsLayout;
    MGridLayoutPolicy             *hideIndicatorsLayout;
    QGraphicsWidget               *rootWidget;
    QSizeF                         unscaledDocSize;
    MLabel *label;
};

OfficeViewerSpreadsheet::OfficeViewerSpreadsheet(QGraphicsWidget *parent)
    : OfficeViewer(parent)
    , d(new Private())
    , currentPage(0)
    , m_lastpageCount(0)
    , lastUserDefinedFactor(1.0)
    , minimumZoomFactor(1.0)
{
    qDebug() << __PRETTY_FUNCTION__ ;
    setObjectName("officeviewerspreadsheet");

    zoomLevel.setUserDefined(false);
    zoomLevel.setMode(ZoomLevel::FactorMode);
    //Default document scale factor for Spreadsheet
    zoomLevel.setFactor(1.2);

    connectActions();

    connect(ApplicationWindow::GetSceneManager(), SIGNAL(orientationChangeFinished(const M::Orientation &)),
            this, SLOT(orientationChanged()));
}

OfficeViewerSpreadsheet::~OfficeViewerSpreadsheet()
{
    qDebug() << __PRETTY_FUNCTION__ ;

    delete d->utils;

    delete d->search;
    delete d;
    delete m_document;
    qDebug() << __PRETTY_FUNCTION__  << "After deleting....." ;
}

bool OfficeViewerSpreadsheet::createKoWidget()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    if(!m_document) return false;

    d->pannableScrollbars = new SpreadsheetPannableScrollBars(this);

    Q_CHECK_PTR(d->pannableScrollbars);

    d->pannableScrollbars->setEnabled(true);

    d->pannableScrollbars->setPanDirection(Qt::Horizontal | Qt::Vertical);

    d->pannableScrollbars->setClipping(true);

    d->pannableScrollbars->setCanvasMode(KoCanvasController::Spreadsheet);

    d->pannableScrollbars->setContentsMargins(0, 0, 0, 0);

    Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*>(m_document);

    d->canvasItem = dynamic_cast<Calligra::Tables::CanvasItem*>(spreadDoc->canvasItem());

    if(!d->canvasItem || !d->canvasItem->resourceManager())  {
        return false;
    }

    d->canvasItem->installEventFilter(new OfficeViewerEventFilter(this));
    d->rowHeader = static_cast<Calligra::Tables::RowHeaderItem*>(d->canvasItem->rowHeader());
    d->columnHeader = static_cast<Calligra::Tables::ColumnHeaderItem*>(d->canvasItem->columnHeader());

    d->canvasItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    d->rowHeader->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    d->columnHeader->setCacheMode(QGraphicsItem::ItemCoordinateCache);
    // set zValue to beabove canvas so that charts on canvas ar not shown above the headers
    qreal zValue = d->canvasItem->zValue();
    d->rowHeader->setZValue(zValue + 1);
    d->columnHeader->setZValue(zValue + 2);

    d->pannableScrollbars->setCanvas(d->canvasItem);
    d->pannableScrollbars->setZoomHandler(d->canvasItem->zoomHandler());

    MLayout* layout = new MLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    d->label = new MLabel("");
    d->label->setMinimumHeight(0);
    d->label->setMaximumHeight(0);

    d->showIndicatorsLayout = new MGridLayoutPolicy(layout);
    d->showIndicatorsLayout->setContentsMargins(0, 0, 0, 0);
    d->showIndicatorsLayout->setSpacing(0);
    d->showIndicatorsLayout->addItem(d->label, 0, 0, 1, 2);
    d->showIndicatorsLayout->addItem(d->rowHeader, 2, 0);
    d->showIndicatorsLayout->addItem(d->columnHeader, 1, 1);
    d->showIndicatorsLayout->addItem(d->pannableScrollbars, 2, 1);

    d->hideIndicatorsLayout = new MGridLayoutPolicy(layout);
    d->hideIndicatorsLayout->setContentsMargins(0, 0, 0, 0);
    d->hideIndicatorsLayout->setSpacing(0);
    d->hideIndicatorsLayout->addItem(d->pannableScrollbars, 0, 0);
    d->hideIndicatorsLayout->addItem(d->rowHeader, 0, 1);
    d->hideIndicatorsLayout->addItem(d->columnHeader, 1, 0);

    d->rootWidget = new QGraphicsWidget();
    d->rootWidget->setLayout(layout);

    connect(d->canvasItem->resourceManager(), SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(resourceChanged(int, const QVariant &)));
    connect(d->pannableScrollbars->proxyObject, SIGNAL(moveDocumentOffset(QPoint)),
            this, SLOT(setDocumentOffset(QPoint)));
    connect(d->canvasItem, SIGNAL(obscuredRangeChanged(const Calligra::Tables::Sheet*,QSize)),
            this, SLOT(updateObscuredRange(const Calligra::Tables::Sheet*)));

    int slideCount = pageCount();

    // Creating utils
    d->utils = new SpreadsheetUtils();
    Q_CHECK_PTR(d->utils);

    setDocumentSize(contentRect(d->canvasItem->activeSheet()));

    setHeaderDefault();
    QSizeF size = viewportSize();
    d->rowHeader->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    d->columnHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    d->canvasItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create search utility
    d->search = new SpreadsheetSearch(d->utils, spreadDoc, slideCount);
    Q_CHECK_PTR(d->search);
    connect(d->search, SIGNAL(setResults(int, int)), this, SLOT(setSearchResults(int, int)));//, Qt::DirectConnection);
    connect(d->search, SIGNAL(searchFinished()), this, SLOT(searchFinished()));

    Calligra::Tables::ToolRegistry::instance()->loadTools();

    // Connect the tool manager
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*, int)),
            this, SLOT(activeToolChanged(KoCanvasController*, int)));
    KoToolManager::instance()->addController(d->pannableScrollbars);
    KoToolManager::instance()->switchToolRequested(PanToolID);

    setCurrentPage(0);
    updatePageNumbers();
    updateSizes();

    zoom(zoomLevel);

    return true;

}

QSizeF OfficeViewerSpreadsheet::viewportSize() const
{
    QSizeF size = ApplicationWindow::visibleSize();
    if (d->showIndicatorsLayout->isActive()) {
        size.setHeight(size.height() - d->columnHeader->height());
        size.setWidth(size.width() - d->rowHeader->width());
    }
    qDebug() << "viewportSize" << size;
    return size;
}

void OfficeViewerSpreadsheet::orientationChanged()
{
    if(zoomLevel.getMode() == ZoomLevel::FitToWidth ||
       zoomLevel.getMode() == ZoomLevel::FitToPage) {
        zoom(zoomLevel);
    }

    updateSizes();
}

void OfficeViewerSpreadsheet::updateSizes()
{
    QSizeF size = ApplicationWindow::visibleSize();
    qDebug() << __PRETTY_FUNCTION__ << size;

    if(d->canvasItem) {
        if ( d->pannableScrollbars->indicatorsVisible ) {
            size.rwidth() -= d->rowHeader->minimumWidth();
            size.rheight() -= d->columnHeader->minimumHeight();
        }
        d->rowHeader->setMinimumHeight(size.height());
        d->columnHeader->setMinimumWidth(size.width());
        d->canvasItem->setMinimumSize(size);

        QPointF position = d->pannableScrollbars->position();
        d->pannableScrollbars->updateRange();
        d->pannableScrollbars->setPosition(position);
    }

    if(d->rootWidget) {
        d->rootWidget->layout()->invalidate();
        d->rootWidget->layout()->activate();
    }

}

QGraphicsLayoutItem *OfficeViewerSpreadsheet::getGraphicsLayoutItem()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    return d->rootWidget;
}

void OfficeViewerSpreadsheet::zoom(const ZoomLevel &newLevel)
{
    qDebug() << __PRETTY_FUNCTION__  << newLevel.getMode();

    if(d->canvasItem) {
        qreal factor = 0;
        //qreal curScaleFactor = 0;
        qreal fitToPageZoomFactor = 0;
        Calligra::Tables::Sheet *currentSheet = d->canvasItem->activeSheet();
        QSizeF actualSheetSize(contentRect(currentSheet));

        QSizeF size = viewportSize();

        fitToPageZoomFactor = (size.width() - 2 * d->pannableScrollbars->margin()) /
                              (d->canvasItem->zoomHandler()->resolutionX() * actualSheetSize.width());
        fitToPageZoomFactor = qMin(fitToPageZoomFactor, ((size.height() - 2 * d->pannableScrollbars->margin()) /
                                                        (d->canvasItem->zoomHandler()->resolutionY() * actualSheetSize.height())));

         //Clamping the zoom factor between min and max zoom limits
        fitToPageZoomFactor = qMin(fitToPageZoomFactor, MaxSpreadSheetZoomFactor);
        fitToPageZoomFactor = qMax(fitToPageZoomFactor, minimumZoomFactor);

        switch(newLevel.getMode()) {

        case ZoomLevel::FactorMode:
            newLevel.getFactor(factor);

            if(factor <= MaxSpreadSheetZoomFactor) {
                d->canvasItem->zoomHandler()->setZoom(factor);
            }

            break;
        case ZoomLevel::FitToWidth:
            d->canvasItem->zoomHandler()->setZoomMode(KoZoomMode::ZOOM_WIDTH);
            factor = (size.width() - 2 * d->pannableScrollbars->margin()) /
                     (d->canvasItem->zoomHandler()->resolutionX() * actualSheetSize.width());

            //Clamping the zoom factor between min and max zoom limits
            factor = qMin(factor, MaxSpreadSheetZoomFactor);
            factor = qMax(factor, minimumZoomFactor);

            d->canvasItem->zoomHandler()->setZoom(factor);
            break;

        case ZoomLevel::FitToPage:
            d->canvasItem->zoomHandler()->setZoomMode(KoZoomMode::ZOOM_PAGE);
            d->canvasItem->zoomHandler()->setZoom(fitToPageZoomFactor);
            break;

        case ZoomLevel::Relative: {
            if (newLevel.getFactor(factor)) {
                qDebug() << __PRETTY_FUNCTION__ << d->canvasItem->zoomHandler()->zoom() << factor;
                d->canvasItem->zoomHandler()->setZoom(factor * d->canvasItem->zoomHandler()->zoom());
            }

            qreal current = d->canvasItem->zoomHandler()->zoom();
            //if((type == ZoomLevel::ZoomOut) && (current < minimumZoomFactor)) {
            if(current < minimumZoomFactor) {
                d->canvasItem->zoomHandler()->setZoom(minimumZoomFactor);
                //We are under zoom out limit so we set minimum scale
            } else if(current > MaxSpreadSheetZoomFactor) {
                //We are over zoom in limit so we set maximum scale
                d->canvasItem->zoomHandler()->setZoom(MaxSpreadSheetZoomFactor);
            }

            break;
        }

        case ZoomLevel::FitToHeight:
            //Doing nothing
            break;

        }//switch

        zoomLevel = newLevel;

        if(zoomLevel.isUserDefined()) {
            lastUserDefinedFactor = d->canvasItem->zoomHandler()->zoom();
            //qDebug() << __PRETTY_FUNCTION__ << "lastUserDefinedFactor : " << lastUserDefinedFactor;
        }

        setHeaderDefault();
        setDocumentSize();
        d->pannableScrollbars->resetLayout();
        updateSizes();
    }
}

void OfficeViewerSpreadsheet::resourceChanged(int key, const QVariant &value)
{
    if(KoCanvasResource::CurrentPage == key) {
        currentPage = value.toInt();
        updatePageNumbers();
    }
}


int OfficeViewerSpreadsheet::pageCount()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

    if(spreadDoc && spreadDoc->map()) {
        return spreadDoc->map()->count();
    }

    return -1;
}

void OfficeViewerSpreadsheet::updatePageNumbers()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    int newPageCount = pageCount();

    emit pageChanged(newPageCount, currentPage);
    emit showingSheet(sheetName(currentPage - 1));

    if(newPageCount != m_lastpageCount) {
        //Lets try reading page numbers little bit later
        QTimer::singleShot(KofficePageNumberUpdateIntervalTime, this, SLOT(updatePageNumbers()));
    }

    m_lastpageCount = newPageCount;
}

void OfficeViewerSpreadsheet::showPage(int pageIndex)
{
    qDebug() << __PRETTY_FUNCTION__ ;
    if(m_document && d->canvasItem) {
        Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

        Calligra::Tables::Sheet* sheet = spreadDoc->map()->sheet(pageIndex);
        d->canvasItem->setActiveSheet(sheet);
        d->pannableScrollbars->restoreStartPoint();
        zoom(ZoomLevel(ZoomLevel::FactorMode, d->canvasItem->zoomHandler()->zoom()));

        emit showingSheet(sheet->sheetName());
        //Taking the contentRect of page index
        QSizeF size = contentRect(sheet);
        qDebug() << "size" << size.height() << "size.width()" << size.width();
        setDocumentSize(size);
        updateSizes();
    }
}

void OfficeViewerSpreadsheet::scrollTo(QPointF point)
{
    qDebug() << __PRETTY_FUNCTION__ << point;

    if(false == point.isNull()) {
        QPointF pannableScrollbarsPosition = d->pannableScrollbars->position() + point;

        //Lets make sure that we don't go over range edges
        QRectF rect = d->pannableScrollbars->range();
        rect.setSize(rect.size() - ApplicationWindow::visibleSize());
        pannableScrollbarsPosition = normalizePoint(pannableScrollbarsPosition,  rect.bottomRight());
        d->pannableScrollbars->setPosition(pannableScrollbarsPosition);
    }
}

QPointF OfficeViewerSpreadsheet::normalizePoint(const QPointF & point, const QPointF & maxPoint)
{
    qDebug() << __PRETTY_FUNCTION__ ;
    const qreal zero = 0;
    QPointF retval = point;
    qreal &x = retval.rx();
    x = qMax(x, zero);
    x = qMin(x, maxPoint.x());

    qreal &y = retval.ry();
    y = qMax(y, zero);
    y = qMin(y, maxPoint.y());
    return retval;
}


void OfficeViewerSpreadsheet::setCurrentPage(int pageIndex)
{
    //qDebug() << __PRETTY_FUNCTION__ << pageIndex;

    if((1 + pageIndex) != currentPage) {
        currentPage = pageIndex + 1;
        updatePageNumbers();
    }
}

QSizeF OfficeViewerSpreadsheet::currentDocumentSize()
{
    if(!d->canvasItem ||
       !d->pannableScrollbars) {
        return QSize();
    }

    Calligra::Tables::Sheet *currentSheet = d->canvasItem->activeSheet();

    if(!currentSheet) {
        return QSize();
    }

    QSizeF actualSheetSize(contentRect(currentSheet));

    actualSheetSize.rwidth() = d->canvasItem->zoomHandler()->zoomItX(actualSheetSize.width());
    actualSheetSize.rheight() = d->canvasItem->zoomHandler()->zoomItY(actualSheetSize.height());

    if(d->pannableScrollbars->indicatorsVisible) {
        actualSheetSize.rwidth() = actualSheetSize.width() +
                                   d->canvasItem->zoomHandler()->zoomItX(YBORDER_WIDTH);
        actualSheetSize.rheight() = actualSheetSize.height() +
                                    d->canvasItem->zoomHandler()->zoomItY(KoGlobal::defaultFont().pointSizeF() + XBORDER_HEIGHT);
    }

    actualSheetSize.rwidth() = actualSheetSize.width() + 3;
    actualSheetSize.rheight() = actualSheetSize.height() + 3;

    return actualSheetSize;
}

void OfficeViewerSpreadsheet::setDocumentOffset(const QPoint &point)
{
    QPoint p(point);
    if (d->pannableScrollbars) {
        qDebug() << __PRETTY_FUNCTION__ << point;
        QSizeF size(currentDocumentSize());
        QSize sz = d->pannableScrollbars->viewportSize();
        if (p.x() < 0) {
            p.setX(0);
        }
        if (p.y() < 0) {
            p.setY(0);
        }
    }

    d->canvasItem->setDocumentOffset(p);
}

static void paintContent(QPainter& painter, const QRect& rect, Calligra::Tables::Sheet* sheet)
{
    if(rect.isEmpty()) {
        return;
    }

    painter.fillRect(rect, Qt::white);

    Calligra::Tables::SheetView sheetView(sheet);

    qreal zoom = 0.5;
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom(zoom);
    painter.setClipRect(rect);
    painter.save();
    qreal zoomX, zoomY;
    zoomHandler.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    sheetView.setViewConverter(&zoomHandler);

    QRectF area = zoomHandler.viewToDocument(rect);
    QRect range = sheet->documentToCellCoordinates(area).adjusted(0, 0, 2, 2);
    sheetView.setPaintCellRange(range);
    sheetView.paintCells(painter, area, QPointF(0,0));

    painter.restore();
    const Qt::LayoutDirection direction = sheet->layoutDirection();

    KoShapePainter shapePainter(direction == Qt::LeftToRight ? (KoShapeManagerPaintingStrategy *)0 : (KoShapeManagerPaintingStrategy *)0 /*RightToLeftPaintingStrategy(shapeManager, d->canvas)*/);
    shapePainter.setShapes(sheet->shapes());
    shapePainter.paint(painter, zoomHandler);
}

QImage * OfficeViewerSpreadsheet::getThumbnail(int page)
{
    Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

    if(0 != spreadDoc && 0 != spreadDoc->map()) {
        Calligra::Tables::Sheet *sheet = spreadDoc->map()->sheet(page);

        QPixmap pix((int)ThumbnailImageWidth, (int)ThumbnailImageWidth);
        pix.fill(QColor(245, 245, 245));
        QRect rc(0, 0, pix.width(), pix.height());
        QPainter p(&pix);
        paintContent(p, rc, sheet);
        return (new QImage(pix.toImage()));
    }

    return 0;
}

void OfficeViewerSpreadsheet::getCurrentVisiblePages(ThumbProvider *thumbProvider)
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    Q_UNUSED(thumbProvider);

    /*if (0 != thumbProvider) {
        thumbProvider->clearVisibleAreas();

        QRectF sceneRect;
        QList<QGraphicsItem *> visibleItems;

        if (0 != d->innerWidget->scene()) {
            //QSizeF size = ApplicationWindow::visibleSize(M::Landscape);
            MSceneManager * sceneManager = MApplication::activeApplicationWindow()->sceneManager();
            QSizeF size = sceneManager->visibleSceneSize(M::Landscape);
            sceneRect = QRectF(QPoint(0,0), size);
            visibleItems = d->innerWidget->scene()->items(sceneRect, Qt::IntersectsItemBoundingRect);

        }

        foreach(QGraphicsItem * item, visibleItems) {

            OfficeViewerSpreadsheetView *newitem = qgraphicsitem_cast<OfficeViewerSpreadsheetView *>(item);

            if (0 != newitem) {
                //Lets get sceneRect in wigdets coodinate
                QRectF temp= newitem->mapRectFromScene(sceneRect);
                //Lets get intersecting between sceneRect and widgets area
                QRectF visibleArea = temp.intersected(newitem->rect());

                //If widget intersects with sceneRect then mark it as visible item

                if (visibleArea.isValid()) {
                    thumbProvider->addVisibleAreas(newitem->getPageIndex(), visibleArea, newitem->size());
                }
            }
        }
    }*/
}

void OfficeViewerSpreadsheet::pinchStarted(QPointF &center)
{
    d->pannableScrollbars->pinchStarted();
    m_pinchCenterDocument = d->canvasItem->viewConverter()->viewToDocument(center + d->pannableScrollbars->position() - d->canvasItem->pos());
}

qreal OfficeViewerSpreadsheet::pinchUpdated(qreal zoomFactor)
{
    qreal effectiveZoomFactor = d->canvasItem->zoomHandler()->zoom() * zoomFactor;

    Calligra::Tables::Sheet *currentSheet = d->canvasItem->activeSheet();
    QSizeF actualSheetSize(contentRect(currentSheet));
    QSizeF size = viewportSize();

    qDebug() << __PRETTY_FUNCTION__ << zoomFactor << effectiveZoomFactor << minimumZoomFactor << d->canvasItem->zoomHandler()->zoom() << minimumZoomFactor / d->canvasItem->zoomHandler()->zoom();
    if (minimumZoomFactor > effectiveZoomFactor) {
        return minimumZoomFactor / d->canvasItem->zoomHandler()->zoom();
    }
    else if (effectiveZoomFactor > MaxSpreadSheetZoomFactor) {
        return MaxSpreadSheetZoomFactor / d->canvasItem->zoomHandler()->zoom();
    }
    return zoomFactor;
}

void OfficeViewerSpreadsheet::pinchFinished(const QPointF &center, qreal scale)
{
    d->canvasItem->zoomHandler()->setZoom(d->canvasItem->zoomHandler()->zoom() * scale);
    d->pannableScrollbars->pinchFinished();

    if (d->pannableScrollbars->indicatorsVisible) {
        d->rowHeader->setVisible(true);
        d->columnHeader->setVisible(true);

        setHeaderDefault();
    }
    setDocumentSize();

    d->pannableScrollbars->resetLayout();
    updateSizes();

    QPointF newPos = d->canvasItem->viewConverter()->documentToView(m_pinchCenterDocument) - center;
    qDebug() << "XXX finish" << m_pinchCenterDocument << newPos << center;
    d->pannableScrollbars->setScrollBarValue(newPos.toPoint());
    qDebug() << "XXX finish 1";

    d->canvasItem->update();
    d->rowHeader->update();
    d->columnHeader->update();

    zoomLevel.setUserDefined(false);
    zoomLevel.setMode(ZoomLevel::FactorMode);
    zoomLevel.setFactor(d->canvasItem->zoomHandler()->zoom());
}

void OfficeViewerSpreadsheet::connectActions()
{
    ActionPool * actions = ActionPool::instance();

    const struct actionStruct {
        ActionPool::Id   id;
        QObject       *object;
        const char    *slot;
    } actionData[] = {

        { ActionPool::SpreadSheetFixedIndicators,      this,  SLOT(setFixedIndicators())}
        ,{ ActionPool::SpreadSheetFloatingIndicators,   this,  SLOT(setFloatingIndicators())}
        , { ActionPool::SpreadSheetNoIndicators,         this,  SLOT(setNoIndicators())}
    };

    for(unsigned int i = 0; i < (sizeof(actionData) / sizeof actionData[0]); i++) {
        MAction *action = actions->getAction(actionData[i].id);
        Q_CHECK_PTR(action);
        connect(action, SIGNAL(triggered()), actionData[i].object, actionData[i].slot);
    }
}

void OfficeViewerSpreadsheet::setFloatingIndicators()
{
    connect(d->pannableScrollbars, SIGNAL(panWidgets(qreal, qreal)),
            this, SLOT(panIndicators(qreal, qreal)));

    d->showIndicatorsLayout->activate();
    d->pannableScrollbars->setIndicatorsStatus(true);

    if(zoomLevel.getMode() == ZoomLevel::FitToWidth ||
       zoomLevel.getMode() == ZoomLevel::FitToPage) {
        zoom(zoomLevel);
    }

    updateSizes();
}

void OfficeViewerSpreadsheet::setFixedIndicators()
{
    disconnect(d->pannableScrollbars, SIGNAL(panWidgets(qreal, qreal)), 0, 0);
    setHeaderDefault();
    d->rowHeader->show();
    d->columnHeader->show();

    d->showIndicatorsLayout->activate();
    d->pannableScrollbars->setIndicatorsStatus(true);

    if(zoomLevel.getMode() == ZoomLevel::FitToWidth ||
       zoomLevel.getMode() == ZoomLevel::FitToPage) {
        zoom(zoomLevel);
    }

    updateSizes();
}

void OfficeViewerSpreadsheet::setNoIndicators()
{
    disconnect(d->pannableScrollbars, SIGNAL(panWidgets(qreal, qreal)), 0, 0);

    d->hideIndicatorsLayout->activate();
    d->pannableScrollbars->setIndicatorsStatus(false);
    d->rowHeader->setMinimumWidth(0);
    d->rowHeader->setMaximumWidth(0);
    d->rowHeader->hide();
    d->columnHeader->setMinimumHeight(0);
    d->columnHeader->setMaximumHeight(0);
    d->columnHeader->hide();

    if(zoomLevel.getMode() == ZoomLevel::FitToWidth ||
       zoomLevel.getMode() == ZoomLevel::FitToPage) {
        zoom(zoomLevel);
    }

    updateSizes();
}

// panIndicators just hides/shows the indicators doesn't pan the indicators
void OfficeViewerSpreadsheet::panIndicators(qreal x, qreal y)
{
    qDebug() << __PRETTY_FUNCTION__ << x << y << d->rowHeader->minimumWidth() << d->columnHeader->minimumHeight();

    bool updated = false;

    if(x > 3) {
        if (d->rowHeader->minimumWidth() > 0) {
            d->rowHeader->setMinimumWidth(0);
            d->rowHeader->setMaximumWidth(0);
            d->rowHeader->hide();
            updated = true;
        }
    } else {
        if (d->rowHeader->minimumWidth() == 0) {
            d->rowHeader->setMinimumWidth(d->canvasItem->zoomHandler()->zoomItX(YBORDER_WIDTH));
            d->rowHeader->setMaximumWidth(d->canvasItem->zoomHandler()->zoomItX(YBORDER_WIDTH));
            d->rowHeader->show();
            updated = true;
        }
    }

    if(y > 3) {
        if (d->columnHeader->minimumHeight() > 0) {
            d->columnHeader->setMinimumHeight(0);
            d->columnHeader->setMaximumHeight(0);
            d->columnHeader->hide();
            updated = true;
        }
    } else {
        if (d->columnHeader->minimumHeight() == 0) {
            d->columnHeader->setMinimumHeight(d->canvasItem->zoomHandler()->zoomItY(KoGlobal::defaultFont().pointSizeF() + 3));
            d->columnHeader->setMaximumHeight(d->canvasItem->zoomHandler()->zoomItY(KoGlobal::defaultFont().pointSizeF() + 3));
            d->columnHeader->show();
            updated = true;
        }
    }

    if (updated) {
        updateSizes();
    }

    d->canvasItem->update();
    d->rowHeader->update();
    d->columnHeader->update();
}

void OfficeViewerSpreadsheet::clearSearchResults()
{
    qDebug() << __PRETTY_FUNCTION__;
    currentSearchResult.sheetIndex  = -1;
    currentSearchResult.count       = -1;
    currentSearchResult.searchIndex = -1;

    if (searchResults.size() > 0) {
        searchResults.clear();
        Calligra::Tables::Sheet *currentSheet = d->canvasItem->activeSheet();

        if(currentSheet) {
            Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);
            int currentSheetIndex = spreadDoc->map()->indexOf(currentSheet);
            d->search->clearResults(currentSheetIndex);
        }

        d->canvasItem->refreshSheetViews();

        d->canvasItem->update();
    }
}



void OfficeViewerSpreadsheet::startSearch(const QString & searchString)
{
    qDebug() << __PRETTY_FUNCTION__  << searchString;
    clearSearchResults();

    if(false == searchString.isEmpty()) {
        if(d->search->isRunning()) {
            d->search->exit();
        }

        Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

        Calligra::Tables::Sheet *currentSheet = d->canvasItem->activeSheet();
        int currentSheetIndex = 0;

        if(currentSheet) {
            currentSheetIndex = spreadDoc->map()->indexOf(currentSheet);
        }

        qDebug()<<"currentSheetIndex"<<currentSheetIndex;

        d->search->setData(searchString, currentSheetIndex);

        d->search->start();

    }//if

}

void OfficeViewerSpreadsheet::nextWord()
{
    qDebug() << __PRETTY_FUNCTION__ ;

    if(1 < searchResults.size() ||
       (1 == searchResults.size() && 1 < searchResults.at(0).count)) {

        //If we are not at end of current sheet search list
        if(currentSearchResult.count < (searchResults.at(currentSearchResult.searchIndex).count - 1)) {
            currentSearchResult.count++;
        } else if(currentSearchResult.searchIndex < (searchResults.size() - 1)) {
            //Lets get first search result from next sheet
            currentSearchResult = searchResults.at(currentSearchResult.searchIndex + 1);
            currentSearchResult.count = 0;
            showPage(currentSearchResult.sheetIndex);

        } else {
            //Default is first item
            SpreadSheetResult newCurrent = searchResults.first();
            newCurrent.count = 0;
            currentSearchResult = newCurrent;
            showPage(currentSearchResult.sheetIndex);
        }

        Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

        Calligra::Tables::Sheet* currentSheet = spreadDoc->map()->sheet(currentSearchResult.sheetIndex);

        //d->search->highlightSheetResult(currentSheet, currentSearchResult.sheetIndex, currentSearchResult.count);
        d->search->setWordsColor(currentSheet, currentSearchResult.sheetIndex, currentSearchResult.count);
        d->canvasItem->refreshSheetViews();
        d->canvasItem->update();

        showCurrentSearchResult();
    }
}

void OfficeViewerSpreadsheet::previousWord()
{
    qDebug() << __PRETTY_FUNCTION__ ;

    if(1 < searchResults.size() ||
       (1 == searchResults.size() && 1 < searchResults.at(0).count)) {

        //If we are not at first search item in current sheet
        if(0 < currentSearchResult.count) {
            currentSearchResult.count--;

        } else if(0 < currentSearchResult.searchIndex) {
            //Lets get last search result from previous sheet
            currentSearchResult = searchResults.at(currentSearchResult.searchIndex - 1);
            currentSearchResult.count--;
            showPage(currentSearchResult.sheetIndex);

        } else {
            //Default is last item
            SpreadSheetResult newCurrent = searchResults.last();
            newCurrent.count --;
            currentSearchResult = newCurrent;
            showPage(currentSearchResult.sheetIndex);
        }

        Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

        Calligra::Tables::Sheet* currentSheet = spreadDoc->map()->sheet(currentSearchResult.sheetIndex);

        //d->search->highlightSheetResult(currentSheet, currentSearchResult.sheetIndex, currentSearchResult.count);
        d->search->setWordsColor(currentSheet, currentSearchResult.sheetIndex, currentSearchResult.count);
        d->canvasItem->refreshSheetViews();
        d->canvasItem->update();

        showCurrentSearchResult();


    }
}

void OfficeViewerSpreadsheet::showCurrentSearchResult()
{
    qDebug() << __PRETTY_FUNCTION__;
    Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);
    Calligra::Tables::Sheet* sheet = 0;

    if(0 <= currentSearchResult.sheetIndex && currentSearchResult.sheetIndex < pageCount()) {
        sheet = spreadDoc->map()->sheet(currentSearchResult.sheetIndex);
        //Lets get rect of current search
        QRectF rect = d->search->mapSearchResult(sheet, currentSearchResult.sheetIndex, currentSearchResult.count);

        //Lets get rect in document coordinate into this coordinate
        rect = d->canvasItem->zoomHandler()->documentToView(rect);
        qDebug()<<"rect top"<<rect.top()<<"rect.bottom"<<rect.bottom();

        d->pannableScrollbars->ensureVisible(rect);
    }

    d->canvasItem->refreshSheetViews();

    d->canvasItem->update();
}

void OfficeViewerSpreadsheet::centerToResult(int index)
{
    Q_UNUSED(index);
}

void OfficeViewerSpreadsheet::shortTap(const QPointF &point, QObject *object)
{
    //FIXME See #154524 - this bug fixed but still crashing better we open the new bug ?
    if(0 == object || !isLoaded) {
        return;
    }


    //Lets select texttool
    KoToolManager::instance()->switchToolRequested(CellToolID);

    QPointF p(d->canvasItem->mapFromScene(point));
    qDebug() << __PRETTY_FUNCTION__ << " point: " << p << object;

    //QMouseEvent *event;
    QGraphicsSceneMouseEvent *event;
    //Lets send a right button press event
    //event = new QMouseEvent(QEvent::MouseButtonPress, p, Qt::LeftButton, Qt::RightButton, Qt::NoModifier);
    event = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    event->setPos(p);
    event->setButton(Qt::LeftButton);
    event->setButtons(Qt::LeftButton);
    event->setAccepted(false);
    d->canvasItem->mousePressEvent(event);
    delete event;

    //Lets send a right button release event
    //event = new QMouseEvent(QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::RightButton, Qt::NoModifier);
    event = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    event->setPos(p);
    event->setButton(Qt::LeftButton);
    event->setButtons(Qt::LeftButton);
    event->setAccepted(false);
    d->canvasItem->mouseReleaseEvent(event);
    delete event;

    //Lets select the default tool again
    KoToolManager::instance()->switchToolRequested(PanToolID);
}

void OfficeViewerSpreadsheet::activeToolChanged(KoCanvasController* canvas, int uniqueToolId)
{
    Q_UNUSED(canvas);
    //TODO Check if this is needed or not
    Q_UNUSED(uniqueToolId);
    QString newTool = KoToolManager::instance()->activeToolId();
    qDebug() << " newTool" << newTool;
    // only Pan tool or Text tool should ever be the active tool, so if
    // another tool got activated, switch back to pan tool

    if(newTool != PanToolID && newTool != TextToolID && newTool != InteractionToolID && newTool != CellToolID) {
        KoToolManager::instance()->switchToolRequested(PanToolID);
    }

    //    canvas->setProperty(KoFingerScrollable, true);
}

void OfficeViewerSpreadsheet::setSearchResults(int index, int count)
{
    qDebug()<<"setSearchResults index"<<index<<"count"<<count;
    Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);
    Calligra::Tables::Sheet* sheet = spreadDoc->map()->sheet(index);

    SpreadSheetResult res;
    res.sheetIndex = index;
    res.count = count;
    res.searchIndex = searchResults.size();
    searchResults.append(res);

    if(currentSearchResult.sheetIndex == -1) {
        //If first then lets make it current
        currentSearchResult.sheetIndex = index;
        currentSearchResult.searchIndex = 0;
        currentSearchResult.count = 0; //Note we are using count as current indexs

        //Let make thread to sleep .3 second
        d->search->highlightSheetResult(sheet, currentSearchResult.sheetIndex, currentSearchResult.count);
        d->search->mSleep(300);

        //IF result on different shhet show that sheet.
        Calligra::Tables::Sheet *currentSheet = d->canvasItem->activeSheet();
        int currentSheetIndex = 0;

        if(currentSheet) {
            currentSheetIndex = spreadDoc->map()->indexOf(currentSheet);
        }

        if(currentSearchResult.sheetIndex != currentSheetIndex) {
            showPage(currentSearchResult.sheetIndex);
        }

        showCurrentSearchResult();
    }
}

void OfficeViewerSpreadsheet::searchFinished()
{
    qDebug()<< __PRETTY_FUNCTION__;

    emit matchesFound(searchResults.size() > 0);

    if(d->search->isRunning()) {
        d->search->exit();
    }
}


void OfficeViewerSpreadsheet::setDocumentSize(const QSizeF& size)
{
    if(size.isValid())
        d->unscaledDocSize = size;

    const QSizeF vsize = d->canvasItem->zoomHandler()->documentToView(d->unscaledDocSize);

    d->canvasItem->setDocumentSize(d->unscaledDocSize);
    qDebug() << "setDocumentSize" << size << vsize.toSize();

    d->pannableScrollbars->updateDocumentSize(vsize.toSize(), true);
}

QString OfficeViewerSpreadsheet::sheetName(int sheetIndex)
{
    if(m_document) {
        Calligra::Tables::Doc *spreadDoc = qobject_cast<Calligra::Tables::Doc*> (m_document);

        Calligra::Tables::Sheet* currentSheet = spreadDoc->map()->sheet(sheetIndex);

        if(currentSheet) {
            return currentSheet->sheetName();
        }
    }

    return QString();
}

void OfficeViewerSpreadsheet::setHeaderDefault()
{
    qreal rowWidth = d->canvasItem->zoomHandler()->zoomItX(YBORDER_WIDTH);
    qreal colHeight = d->canvasItem->zoomHandler()->zoomItY(KoGlobal::defaultFont().pointSizeF() + 3);
    d->rowHeader->setMinimumWidth(rowWidth);
    d->rowHeader->setMaximumWidth(rowWidth);
    d->columnHeader->setMinimumHeight(colHeight);
    d->columnHeader->setMaximumHeight(colHeight);
}

QSizeF OfficeViewerSpreadsheet::contentRect(const Calligra::Tables::Sheet *sheet)
{
    return d->utils->contentRect(sheet, d->canvasItem->sheetView(sheet));
}

void OfficeViewerSpreadsheet::updateObscuredRange(const Calligra::Tables::Sheet *sheet)
{
    d->utils->clearContentRectCache(sheet);
    QSizeF size = contentRect(sheet);
    qDebug() << "size" << size.height() << "size.width()" << size.width();
    setDocumentSize(size);
    updateSizes();
}

void OfficeViewerSpreadsheet::updateRange()
{
    if (d->pannableScrollbars) {
        QRectF vRect(0, 0, 0, 0);
        MApplicationWindow *window = MApplication::activeApplicationWindow();
        if (window && window->currentPage()) {
            DocumentPage *page = qobject_cast<DocumentPage*>(window->currentPage());
            if (page) {
                vRect = page->visibleRect();
            }
        }
        d->label->setMinimumHeight(vRect.top());
        d->label->setMaximumHeight(vRect.top());
        d->pannableScrollbars->updateRange();
    }
}
