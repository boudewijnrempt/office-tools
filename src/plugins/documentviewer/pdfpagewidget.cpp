#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QProcess>
#include <QTemporaryFile>
#include <QSound>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDesktopServices>
#include <QUrl>
#include <QThread>

#include <MApplication>
#include <MSceneManager>
#include <MProgressIndicator>

#include <math.h>
#include <poppler-qt4.h>

#include "pdfpagewidget.h"
#include "definitions.h"
#include "zoomlevel.h"
#include "pdfloader.h"
#include "applicationwindow.h"
#include "actionpool.h"

const qreal PdfPageWidget::maximumScale = PdfLoader::DPIPerInch * MaxZoomFactor;

PdfPageWidget::PdfPageWidget(PdfLoader *loader, int pageIndex, MWidget *parent)
    : MWidget(parent)
    , BasePageWidget(pageIndex)
    , loader(loader)
    , scale(0)
    , lastUserDefinedFactor(1)
    , m_updateCachedImage(false)
{

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    spinner = new MProgressIndicator(this, MProgressIndicator::spinnerType);
    spinner->setVisible(false);
    QSizeF spinnerSize = spinner->sizeHint(Qt::PreferredSize);
    spinnerCenter = QPointF(spinnerSize.width()/2 , spinnerSize.height()/2);

//    setCacheMode(QGraphicsItem::ItemCoordinateCache);
    connect(this, SIGNAL(displayExited()), this, SLOT(clearCachedImage()));
}


PdfPageWidget::~PdfPageWidget()
{
#ifdef SELECT_TEXT
    qDeleteAll(textBoxList.begin(), textBoxList.end());
    textBoxList.clear();
#endif
}

void PdfPageWidget::update(const QRectF & rect)
{
    qDebug() << __PRETTY_FUNCTION__ << rect;
    MWidget::update(rect);
}

void PdfPageWidget::updateupdate (qreal x, qreal y, qreal width, qreal height)
{
    qDebug() << __PRETTY_FUNCTION__ << x << y << width << height;
    MWidget::update(x, y, width, height);
}

void PdfPageWidget::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget);
    QRectF expsRect = option->exposedRect;

    static int i = 0;
    if (++i % 10 == 0) {
        qDebug() << "10";
    }
    if ((!painter->clipRegion().isEmpty()) && (!painter->clipRegion().contains(expsRect.toRect()))) {
        return;
    }
    qDebug() << __PRETTY_FUNCTION__ << painter->clipRegion().isEmpty() << painter->clipRegion().rects() << expsRect.toRect();

    if (m_cachedImage.isNull() || m_updateCachedImage) {
        QImage image = loader->getPageImage(pageIndex, scale, this);
        qDebug() << __PRETTY_FUNCTION__ << "loader->getPageImage" << pageIndex << scale << image.isNull() << image.size();
        if (!image.isNull()) {
            m_cachedImage = image;
            m_updateCachedImage = false;
        }
    }

    if (m_cachedImage.isNull()) {
        qDebug() << __PRETTY_FUNCTION__ << "show spinner" << pageIndex << scale;
        spinner->setPos(expsRect.center() - spinnerCenter);
        if(spinnerCenter.y() * 2 > expsRect.height()) {
            spinner->reset();
            spinner->setVisible(false);
        }
        else {
            spinner->setUnknownDuration(true);
            spinner->setVisible(true);
        }
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << size() << m_cachedImage.size() << (size() == m_cachedImage.size());
        spinner->reset();
        spinner->setVisible(false);
        // there are some documents where the size() == m_cachedImage.size() does not work.
        // however the width seems to be always correct.
        if (m_cachedImage.size().width() == size().width()) {
            qDebug() << __PRETTY_FUNCTION__ << "big as it should be" << pageIndex << scale;
            // the different handling for bigger 2000 is needed as otherwise the quality is quite bad.
            if (m_cachedImage.height() > 2000 || m_cachedImage.width() > 2000) {
                painter->drawImage(expsRect, m_cachedImage.copy(expsRect.toRect()));
            }
            else {
                painter->drawImage(expsRect, m_cachedImage, expsRect);
            }
        }
        else {
            // only trigger update if we had not triggered before
            if (!m_updateCachedImage) {
                qDebug() << __PRETTY_FUNCTION__ << "trigger update" << pageIndex << scale;
                loader->getPageImage(pageIndex, scale, this);
                m_updateCachedImage = true;
            }

            // zoom what we have at the moment to show until the update is done
            qreal zoom = m_cachedImage.size().width() / size().width();
            QRectF sourceRect(expsRect.topLeft() * zoom, expsRect.bottomRight() * zoom);
            qDebug() << __PRETTY_FUNCTION__ << sourceRect << expsRect << zoom;
            painter->drawImage(expsRect, m_cachedImage, sourceRect);
        }
    }

    /// Draw the search result highlights if any
    const QHash<int, QList<QRectF> > *searchData = loader->getHighlightData();
    if(searchData) {
        int highlightPageIndex = 0;
        int currentHighlightPostion = 0;

        //Getting the cuurent highlight pageindex and currentHighlight word of that page.
        loader->getCurrentHighlight(highlightPageIndex, currentHighlightPostion);

        QList <QRectF> pageSearchResults = searchData->value(pageIndex);

        painter->setOpacity(0.5);
        painter->setPen(QPen(highlightColor));
        painter->setBrush(QBrush(highlightColor));

        for(int textIndex = 0; textIndex < pageSearchResults.size(); ++textIndex) {
            // Result rectangles are for the library renderer's default
            // page size. Scale them according to our image width
            qreal scaleRatio = scale / PdfLoader::DPIPerInch;
            QRectF dHl(pageSearchResults.at(textIndex).x() * scaleRatio, pageSearchResults.at(textIndex).y() * scaleRatio,
                       pageSearchResults.at(textIndex).width() * scaleRatio, pageSearchResults.at(textIndex).height() * scaleRatio);

            if((highlightPageIndex == pageIndex) && (textIndex == currentHighlightPostion)) {
                painter->setPen(QPen(highlightColorCurrent));
                painter->setBrush(QBrush(highlightColorCurrent));
                painter->drawRect(dHl);
                painter->setPen(QPen(highlightColor));
                painter->setBrush(QBrush(highlightColor));
            } else {
                painter->drawRect(dHl);
            }

        }
    }
    static const int sceneLHeight(ApplicationWindow::visibleSize(M::Landscape).height());
    static const int scenePHeight(ApplicationWindow::visibleSize(M::Portrait).height());

    if (ApplicationWindow::GetSceneManager()) {
        int height = M::Landscape == ApplicationWindow::GetSceneManager()->orientation() ? sceneLHeight : scenePHeight;
        if ((!m_cachedImage.isNull() && m_cachedImage.height()==expsRect.height()) ||
            expsRect.height() >= height / 2) {
            loader->setCurrentPage(pageIndex);
        }
    }
}


QSizeF PdfPageWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);
    return widgetSize;
}

void PdfPageWidget::updateSize(const QSizeF & viewSize, const ZoomLevel & zoom)
{
    if(zoom == lastZoomLevel &&
       viewSize == lastViewSize &&
       zoom.isUserDefined() == lastZoomLevel.isUserDefined()) {  //Nothing has changed
        return;
    }

    bool belowMinScale = false;
    QSize pageSize = loader->pageSize(pageIndex);

    qreal newScale = zoomToScale(viewSize, zoom, pageSize);

    qreal minScaleWidth = calcScale(viewSize.width(), pageSize.width());
    qreal minScaleHeight = calcScale(viewSize.height(), pageSize.height());
    qreal minScale = qMin(minScaleWidth, minScaleHeight);
    //Minimum is fit to page or 100 % (the smaller one)
    minScale = qMin(MinZoomFactor*PdfLoader::DPIPerInch, minScale);

    if(zoom.isUserDefined()) {
        if(newScale < minScale) {
            //We are under zoom out limit so we set minimum scale
            newScale =  minScale;
            belowMinScale = true;
            //this->updateSize(viewSize, ZoomLevel(ZoomLevel::FactorMode, minimumScaleFactor));
        } else if(newScale > maximumScale) {
            //We are over zoom in limit so we set maximum scale
            newScale =  maximumScale;
            //this->updateSize(viewSize, ZoomLevel(ZoomLevel::FactorMode, MaxZoomFactor));
        }

        lastUserDefinedFactor = newScale / PdfLoader::DPIPerInch;

        if(pageIndex == loader->getCurrentPageIndex()) {
            //Lets use only current page zooming factor
            ActionPool::instance()->setUserDefinedZoomFactor(lastUserDefinedFactor);
        }
    }

    lastZoomLevel = zoom;

//    qDebug()<<"newScale"<<newScale<<"minScale"<<minScale;

    if(!belowMinScale) {
        if(scale != newScale || viewSize != lastViewSize) {
            lastViewSize = viewSize;
            scale = newScale;

            qreal newWidth = calcScaledSized(scale,  pageSize.width());
            qreal newHeight = calcScaledSized(scale, pageSize.height());

            widgetSize.setWidth(newWidth);
            widgetSize.setHeight(newHeight);

#ifdef SELECT_TEXT
            //Lets clear textBoxList each time zoom changes
            clearFullSelectedText();
#endif

            updateGeometry();
        }
    }
}

qreal PdfPageWidget::zoomToScale(const QSizeF & viewSize, const ZoomLevel & zoom, const QSize &pageSize)
{
    qreal newScale = 0;

    switch(zoom.getMode()) {

    case ZoomLevel::FitToHeight:
        newScale = calcScale(viewSize.height(), pageSize.height());
        break;

    case ZoomLevel::FitToWidth:
        newScale = calcScale(viewSize.width(), pageSize.width());
        break;

    case ZoomLevel::FitToPage: {
        qreal scaleWidth = calcScale(viewSize.width(), pageSize.width());
        qreal scaleHeight = calcScale(viewSize.height(), pageSize.height());
        newScale = qMin(scaleWidth, scaleHeight);
    }

    break;

    case ZoomLevel::FactorMode: {
        qreal factor = 0;

        if(zoom.getFactor(factor)) {
            newScale = PdfLoader::DPIPerInch * factor;
        }
    }

    break;

    case ZoomLevel::Relative: {
        qreal factor = 0;

        if(zoom.getFactor(factor)) {
            newScale = scale*factor;
        }
    }

    break;

    }

    return newScale;
}

qreal PdfPageWidget::calcScale(const qreal targetSize, const qreal sourceSize)
{
    qreal retval = 0;
    Q_ASSERT(0 != sourceSize);
    retval = PdfLoader::DPIPerInch * (targetSize / sourceSize);

    return retval ;
}

qreal PdfPageWidget::calcZoomFactor()
{
    qreal ratio =  scale/(PdfLoader::DPIPerInch);
    return ratio;
}


qreal PdfPageWidget::calcScaledSized(const qreal factor, const qreal sourceSize)
{
    qreal retval = 0;

    retval = (factor * sourceSize) / PdfLoader::DPIPerInch;

    return qRound(retval) ;
}


bool PdfPageWidget::linkTaped(const QPointF &point)
{
    bool retval = false;
    Poppler::Link *link = 0;

    QList<Poppler::Link *> links = loader->getLinks(pageIndex);

    //Lets check if we have direct hit
    foreach(Poppler::Link *item, links) {
        QRectF temp = item->linkArea().normalized();
        QRectF linkRect(QPointF(temp.x() * size().width(), temp.y() * size().height()),
                        QSizeF(temp.width() * size().width(), temp.height() * size().height()));

        qDebug() << "Link:" << temp << linkRect << point << size();
        if (linkRect.contains(point)) {
            link = item;
            break;
        }
    }

    if(0 != link) {
        retval = handleLinkTypes(link);
    }

    qDeleteAll(links.begin(), links.end());

    links.clear();

    return retval;
}

bool PdfPageWidget::handleLinkTypes(const Poppler::Link *link)
{
    bool handled=false;

    if(!link) {
        return handled;
    }

    switch(link->linkType()) {

    case Poppler::Link::None:
        qDebug("Link is of type None");
        break;

    case Poppler::Link::Execute:
        //TODO security concern?
        qDebug("Link is of type Execute");
        //const Poppler::LinkExecute *linkExecute = static_cast<const Poppler::LinkExecute*>(link);
        //QProcess::execute(linkExecute->fileName(),QStringList(linkExecute->parameters()));
        break;

    case Poppler::Link::Browse: {
        qDebug("Link is of type Browse");
        QDesktopServices::openUrl(static_cast<const Poppler::LinkBrowse*>(link)->url());
        handled = true;
        break;
    }

    case Poppler::Link::Action: {
        qDebug("Link is of type Action");
        const Poppler::LinkAction *linkAction = static_cast<const Poppler::LinkAction*>(link);
        handleLinkAction(linkAction);
        handled = true;
    }

    break;

    case Poppler::Link::Sound: {
        qDebug("Link is of type Sound");
        const Poppler::LinkSound *linkSound = static_cast<const Poppler::LinkSound*>(link);
        Poppler::SoundObject *object = linkSound->sound();

        if(!object) {
            break;
        }

        //TODO I'm ignoring mix, repeat and volume from sound object

        switch(object->soundType()) {

        case Poppler::SoundObject::Embedded: {
            handled = true;
            QTemporaryFile *file=new QTemporaryFile(this);

            if(!file->open()) {
                qDebug()<<"Failed to create temporary file for embedded sound to"<<file->fileName();
                return handled;
            }

            {
                QDataStream out(file);
                out<<object->data();
            }

            QSound::play(file->fileName());
        }

        break;

        case Poppler::SoundObject::External:
            QSound::play(object->url());
            handled = true;
            break;
        }
    }

    break;

    case Poppler::Link::Movie:
        qDebug("Link is of type Movie");
        break;

    case Poppler::Link::Goto: {
        const Poppler::LinkGoto *linkGoto = static_cast<const Poppler::LinkGoto*>(link);
        handleLinkGoto(linkGoto);
        handled = true;
        break;
    }

    case Poppler::Link::JavaScript:
        qDebug("Link is of type JavaScript");
        break;

    default:
        break;
    }

    return handled;
}

void PdfPageWidget::handleLinkAction(const Poppler::LinkAction *link)
{
    if(!link) {
        return;
    }

    switch(link->actionType()) {

    case Poppler::LinkAction::PageFirst:
        emit showPage(0, QPointF(0,0));
        break;

    case Poppler::LinkAction::PagePrev:
        emit showPage(pageIndex - 1, QPointF(0,0));
        break;

    case Poppler::LinkAction::PageNext:
        emit showPage(pageIndex + 1, QPointF(0,0));
        break;

    case Poppler::LinkAction::PageLast:
        emit showPage(loader->numberOfPages() + 1, QPointF(0,0));
        break;

    case Poppler::LinkAction::HistoryBack:
        //! We don't have back action
        break;

    case Poppler::LinkAction::HistoryForward:
        //! We don't have forward action
        break;

    case Poppler::LinkAction::Quit:
        emit requestApplicationQuit();
        break;

    case Poppler::LinkAction::Presentation:
        //! TODO startPresentation();
        break;

    case Poppler::LinkAction::EndPresentation:
        //! TODO endPresentation();
        break;

    case Poppler::LinkAction::Find:
        emit requestSearch();
        break;

    case Poppler::LinkAction::GoToPage:
        //At the moment there are no other parameters, so there is no
        //page to go. This is also undocumented in qtpoppler. So there's nothing
        //we can do?
        break;

    default:
        break;
    }
}

void PdfPageWidget::handleLinkGoto(const Poppler::LinkGoto *link)
{
    if(!link) {
        return;
    }

    //Do we want the current document or a new document
    if(link->isExternal()) {
        qWarning("Opening other files from PDF not supported ('%s')", link->fileName().toLatin1().data());
        return;
    }

    Poppler::LinkDestination destination = link->destination();

    handleLinkDestination(destination);
}


void PdfPageWidget::handleLinkDestination(const Poppler::LinkDestination &destination)
{
    qDebug()<<__PRETTY_FUNCTION__<<"Link to page"<<destination.pageNumber();
    QPointF point(0,0);


    if(Poppler::LinkDestination::destXYZ == destination.kind()) {
        if(destination.isChangeLeft()) {
            qDebug()<<__PRETTY_FUNCTION__<<"destination.left()" << destination.left();
            point.setX(destination.left());
        }

        if(destination.isChangeTop()) {
            qDebug()<<__PRETTY_FUNCTION__<<"destination.top()" << destination.top();
            point.setY(destination.top());
        }
    }

    //Page number in LinkDestination is from 1 to n
    emit showPage(destination.pageNumber()-1, point);


    switch(destination.kind()) {

    case Poppler::LinkDestination::destXYZ: {
        if(destination.isChangeZoom()) {
            /*
                We don't want the zoom scale to be 0 since then we'd have nothing
                to show. I'm making the assumption that zoom scale 1 is the
                "as is" scale as the popplers documentation is a bit lacking at the
                moment.
            */
            if(destination.zoom() != 0 && destination.zoom() != 1) {
                qDebug()<<__PRETTY_FUNCTION__<<"Changing scale to"<<destination.zoom();
                emit changZoomLevel(ZoomLevel(ZoomLevel::FactorMode,destination.zoom()));
            }
        }
    }

    break;

    case Poppler::LinkDestination::destFit: {
        qDebug()<<__PRETTY_FUNCTION__<<"changZoomLevel destFit" ;
        emit changZoomLevel(ZoomLevel(ZoomLevel::FitToPage,1, false));
    }

    break;

    case Poppler::LinkDestination::destFitH: {
        qDebug()<<__PRETTY_FUNCTION__<<"changZoomLevel destFitH" ;
        emit changZoomLevel(ZoomLevel(ZoomLevel::FitToWidth,1, false));
    }

    break;

    case Poppler::LinkDestination::destFitV: {
        qDebug()<<__PRETTY_FUNCTION__<<"changZoomLevel destFitV" ;
        emit changZoomLevel(ZoomLevel(ZoomLevel::FitToHeight,1, false));
    }

    break;

    //TODO rest of these were uncommented in qt popplers documents/code,
    //they might be properly defined in poppler's own document

    case Poppler::LinkDestination::destFitR:
        break;

    case Poppler::LinkDestination::destFitB:
        break;

    case Poppler::LinkDestination::destFitBH:
        break;

    case Poppler::LinkDestination::destFitBV:
        break;

    default:
        break;

    }

}

#ifdef SELECT_TEXT
void PdfPageWidget::selectText(QPointF mousePressPoint, QPointF mouseMovePoint)
{
    //qDebug()<<__PRETTY_FUNCTION__<< "startPoint " << mousePressPoint << "endPoint " << mouseMovePoint;

    if(textBoxList.isEmpty()) {
        calcTextBoxList();
    }


    QString selectedText = "";

    clearSelectedText();

    qreal topX, topY, downX, downY;

    if(mouseMovePoint.y() < mousePressPoint.y()) {
        topX = mouseMovePoint.x();
        topY = mouseMovePoint.y();
        downY = mousePressPoint.y();
        downX = mousePressPoint.x();
    } else {
        topY = mousePressPoint.y();
        topX = mousePressPoint.x();
        downY = mouseMovePoint.y();
        downX = mouseMovePoint.x();
    }


    //qDebug() << " topX: " << topX << " topY: " << topY << " downX: " << downX << " downY: " << downY;

    for(int i = 0; i<textBoxList.count(); i++) {
        QRectF textRect = textBoxList[i]->boundingBox();

        // textbox is below or above selected area

        if(textRect.bottom() < topY || textRect.top() > downY)
            continue;

        // textbox is on the left side of top left selection point
        if(textRect.top() < topY && textRect.bottom() > topY && textRect.right() < topX)
            continue;

        // textbox is on the right side of bottom right selection point
        if(textRect.top() < downY && textRect.bottom() > downY && textRect.left() > downX)
            continue;

        // if we get to here then text is between selection points
        selectedRectList << textRect;

        selectedText += textBoxList[i]->text() + " " ;
    }

    if(!selectedRectList.isEmpty()) {
        qDebug() << "selectedText: " << qPrintable(selectedText);
    }
}

void PdfPageWidget::clearSelectedText()
{
    selectedRectList.clear();
}

void PdfPageWidget::clearFullSelectedText()
{
    qDeleteAll(textBoxList.begin(), textBoxList.end());
    textBoxList.clear();
    clearSelectedText();
}

void PdfPageWidget::calcTextBoxList(void)
{
    //qDebug() << __PRETTY_FUNCTION__ << " pageIndex: " << pageIndex;

    QList<Poppler::TextBox*> popplerList = loader->getTextBoxList(pageIndex);

    if(popplerList.isEmpty())
        return;

    clearFullSelectedText();

    // scale rects from PDF default 1/72 to 1/scale
    qreal multipler = (1.0/72.0)/(1.0/scale);

    for(int i = 0; i<popplerList.count(); i++) {
        QRectF popplerRect(popplerList[i]->boundingBox());
        QRectF rect(popplerRect.x()*multipler, popplerRect.y()*multipler, popplerRect.width()*multipler, popplerRect.height()*multipler);
        textBoxList.append(new Poppler::TextBox(popplerList[i]->text(), rect));
    }

    const QList<Poppler::TextBox *> list = loader->getTextBoxList(pageIndex);

    /*
    qDebug()<<__PRETTY_FUNCTION__;
    int i = 0;
    foreach (Poppler::TextBox *item, popplerList)
    {
        qDebug()<< "\t[" << i << "] text()" << item->text();
        qDebug()<< "\t[" << i << "] boundingBox()" << item->boundingBox();
        qDebug()<< "\t[" << i << QString().sprintf("] pointer %p", item);
        qDebug()<< "\t[" << i << QString().sprintf("] nextWord pointer %p", item->nextWord());
        i++;
    }
    */
    qDeleteAll(popplerList.begin(), popplerList.end());

    popplerList.clear();
}
#endif

void PdfPageWidget::clearCachedImage()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_cachedImage = QImage();
}
