#include <mapplication.h>
#include <QColor>
#include <QStyleOptionGraphicsItem>

#include <pdfpagewidget.h>
#include <pdfpage.h>
#include <pdfloader.h>
#include <zoomlevel.h>

#include "ut_pdfpagewidget.h"


//const QString testDocument = "/usr/share/office-tools-tests/data//sonnets.pdf";
const QString testDocument = "/usr/share/office-tools/data/link.pdf";
//const QString testDocument = "/usr/share/office-tools-tests/data//excerpts.pdf";
const int deviceScreenWidth  = 864;
const int deviceScreenHeight = 480;


Ut_PdfPageWidget::Ut_PdfPageWidget() :
    pdfpagewidget(0)
{
    Poppler::Document *popplerDocument;
    loader = new PdfLoader();
    loader->load(testDocument, popplerDocument);
    loader->setCurrentPage(1);
    pdfpagewidget = new PdfPageWidget(loader, 1);
}


Ut_PdfPageWidget::~Ut_PdfPageWidget()
{
    delete pdfpagewidget;
    pdfpagewidget = 0;
    delete loader;
    loader = 0;
}


void Ut_PdfPageWidget::testCreation()
{
    QVERIFY(pdfpagewidget);
}


void Ut_PdfPageWidget::testUpdateSize()
{
    QSizeF viewSize(deviceScreenWidth, deviceScreenHeight);
    ZoomLevel zoom(ZoomLevel::FitToWidth);
    pdfpagewidget->updateSize(viewSize, zoom);
    QSizeF widgetSize = pdfpagewidget->size();
    QCOMPARE((int)widgetSize.width(), deviceScreenWidth);

    zoom = ZoomLevel(ZoomLevel::FitToHeight);
    pdfpagewidget->updateSize(viewSize, zoom);
    widgetSize = pdfpagewidget->size();
    QCOMPARE((int)widgetSize.height(), deviceScreenHeight);
}


void Ut_PdfPageWidget::testPainter()
{
    QSizeF viewSize(deviceScreenWidth, deviceScreenHeight);
    ZoomLevel zoom(ZoomLevel::FitToWidth);
    pdfpagewidget->updateSize(viewSize, zoom);
    int i = 0;
    while (i++ < 20 && loader->getPageImage(1, pdfpagewidget->calcZoomFactor() * PdfLoader::DPIPerInch, 0).isNull()) {
        QTest::qWait(250);
    }

    QStyleOptionGraphicsItem styleoption;
    styleoption.exposedRect = QRectF(QPointF(0, 0), QSizeF(deviceScreenWidth, deviceScreenHeight));
    QImage image(deviceScreenWidth, deviceScreenHeight, QImage::Format_RGB16);

    QPainter painter(&image);
    QWidget *widget;
    pdfpagewidget->paint(&painter, &styleoption, widget);
    //image.save("painter.png");
    QCOMPARE(qBlue(image.pixel(QPoint(1,1))), 255);
}


void Ut_PdfPageWidget::testSelectText()
{
    QSizeF viewSize(deviceScreenWidth, deviceScreenHeight);
    ZoomLevel zoom(ZoomLevel::FitToWidth);
    pdfpagewidget->updateSize(viewSize, zoom);
    int i = 0;
    while (i++ < 20 && loader->getPageImage(1, pdfpagewidget->calcZoomFactor() * PdfLoader::DPIPerInch, 0).isNull()) {
        QTest::qWait(250);
    }

    QHash<int, QList<QRectF> > highlight;
    highlight.insert(1, QList<QRectF>() << QRectF(0,0,400,400));
    loader->setHighlightData(&highlight);

    QStyleOptionGraphicsItem styleoption;
    styleoption.exposedRect = QRectF(QPointF(0, 0), QSizeF(deviceScreenWidth, deviceScreenHeight));
    QImage image(deviceScreenWidth, deviceScreenHeight, QImage::Format_RGB16);
    QPainter painter(&image);
    QWidget *widget;
    pdfpagewidget->paint(&painter, &styleoption, widget);
    //image.save("painter.png");

    QVERIFY(qBlue(image.pixel(QPoint(100,25))) < qBlue(image.pixel(QPoint(700,25))));
}


/*
void Ut_PdfPageWidget::testLinkClick() {
    QMouseEvent press(QEvent::MouseButtonPress, QPoint(100,100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(pdfpagewidget, &press);

    QMouseEvent release(QEvent::MouseButtonRelease, QPoint(100,100), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(pdfpagewidget, &release);
}
*/


int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_PdfPageWidget test;
    QTest::qExec(&test, argc, argv);
}
