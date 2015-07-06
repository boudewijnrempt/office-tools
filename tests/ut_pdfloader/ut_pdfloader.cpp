#include <mapplication.h>
#include <pdfloader.h>
#include <pdfpagewidget.h>
#include <QSignalSpy>
#include "ut_pdfloader.h"


const QString testDocument = "/usr/share/office-tools/data/link.pdf";


Ut_PdfLoader::Ut_PdfLoader() :
    pdfloader(0)
{
    pdfloader = new PdfLoader();
}


Ut_PdfLoader::~Ut_PdfLoader()
{
    delete pdfloader;
    pdfloader = 0;
}


void Ut_PdfLoader::testCreation()
{
    QVERIFY(pdfloader);
}


void Ut_PdfLoader::testDocumentLoadAndSizes_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("canopen");
    QTest::addColumn<int>("pages");
    QTest::addColumn<QSize>("pagesize");

    QTest::newRow("link") << "/usr/share/office-tools/data/link.pdf" << true << 5 << QSize(612, 792);
    //QTest::newRow("sonnets") << "office-tools-test-document-encrypted(passwordIspassword).pdf" << true << 187 << QSize(433, 388);
    //QTest::newRow("presentation") << "/usr/share/office-tools-tests/data/presentation.pdf" << true << 70 << QSize(617, 404);
    //QTest::newRow("nonexists") << "nonexists.pdf" << false << 0 << QSize(-1, -1);
}


void Ut_PdfLoader::testDocumentLoadAndSizes()
{
    Poppler::Document *mDocument;

    QFETCH(QString, filename);
    QFETCH(bool, canopen);
    QCOMPARE(pdfloader->load(filename, mDocument), canopen);

    QFETCH(int, pages);
    QCOMPARE(pdfloader->numberOfPages(), pages);

    QFETCH(QSize, pagesize);
    QCOMPARE(pdfloader->pageSize(0), pagesize);
}


void Ut_PdfLoader::testCurrentPage()
{
    Poppler::Document *mDocument;
    QVERIFY(pdfloader->load(testDocument, mDocument));

    // load pages and check if pdfloader sends exactly one pageChanged(int, int) signal for each page
    QSignalSpy spy(pdfloader, SIGNAL(pageChanged(int, int)));

    for(int pageIndex=1; pageIndex<pdfloader->numberOfPages(); pageIndex++) {
        pdfloader->setCurrentPage(pageIndex);
        QCOMPARE(pdfloader->getCurrentPageIndex(), pageIndex);
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(1).toInt(), pageIndex+1);
    }

    // load last page again. No signal should be received
    pdfloader->setCurrentPage(pdfloader->numberOfPages()-1);

    QCOMPARE(spy.count(), 0);
}


// This test case is moved to pdfloaderthread unit tests
void Ut_PdfLoader::testGetPageImage()
{
    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));

    Poppler::Document *doc = Poppler::Document::load(testDocument);
    doc->setRenderHint(Poppler::Document::Antialiasing, true);
    doc->setRenderHint(Poppler::Document::TextAntialiasing, true);

    Poppler::Document *mDocument;
    QVERIFY(pdfloader->load(testDocument, mDocument));

    QImage *image = 0;

    for(int page=0; page<pdfloader->numberOfPages(); page++) {
#if 0
        int scale = (qrand() % 190) + 10;

        image = pdfloader->getPageImage(page, scale);
        QImage popplerImage = doc->page(page)->renderToImage(scale, scale);
        popplerImage = popplerImage.convertToFormat(QImage::Format_RGB16, Qt::AutoColor);

        if(image != 0) {
            //QCOMPARE(*image, popplerImage);
        }

        delete image;

        image = 0;
#endif
    }
}


/*
void Ut_PdfLoader::testPageImageScaleChangedSignal() {
    //Poppler::Document *mDocument;
    //pdfloader->load(testDocument, mDocument);

    QImage *image = 0;
    PdfPageWidget *widget = new PdfPageWidget(pdfloader, 1);
    QRectF rect;

    QSignalSpy spy(pdfloader, SIGNAL(pageImageScaleChanged(int)));
    qreal scale = 33.3;
    image = pdfloader->getPageImage(widget, 2, scale, rect);
    QCOMPARE(spy.count(), 1);

    delete widget;
}
*/

/*
void Ut_PdfLoader::testLoadPageSignal() {
    Poppler::Document *mDocument;
    delete pdfloader;
    pdfloader = new PdfLoader();

    QSignalSpy spy(pdfloader, SIGNAL(loadPage(int)));
    pdfloader->load(testDocument, mDocument);
    pdfloader->setCurrentPage(10);

    QTest::qWait(500);
    qDebug() << "spy.COUNT: " << spy.count();
}
*/


int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_PdfLoader test;
    return QTest::qExec(&test, argc, argv);
}
