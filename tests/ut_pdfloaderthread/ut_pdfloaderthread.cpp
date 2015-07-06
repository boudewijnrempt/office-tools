#include <mapplication.h>
#include <QColor>
#include <QStyleOptionGraphicsItem>

#include <pdfimagecache.h>
#include <pdfloaderthread.h>
#include <pdfloader.h>
#include <zoomlevel.h>

#include "ut_pdfloaderthread.h"


const QString testDocument = "/usr/share/office-tools/data/link.pdf";
const int deviceScreenWidth  = 864;
const int deviceScreenHeight = 480;


Ut_PdfLoaderThread::Ut_PdfLoaderThread() :
    cache(0),
    thread(0)

{
    cache = new PdfImageCache(5);
    thread = new PdfLoaderThread(testDocument, cache);
    thread->start();
    scale = (qrand() % 190) + 10;
}


Ut_PdfLoaderThread::~Ut_PdfLoaderThread()
{
    while(thread->isRunning()) {
        sleep(0.5);
        thread->quit();
    }

    delete thread;
    delete cache;
}


void Ut_PdfLoaderThread::testCreation()
{
    QVERIFY(thread);
}


void Ut_PdfLoaderThread::testLoadPage()
{
    QCOMPARE(true, cache->getImage(1, 100.0, 0).isNull());
    thread->loadPage(1,100.0);
    // 1000 ms delay for the pdfloaderthread to load the page and to emit the signal, this
    // might vary if the input file changes
    QTest::qWait(1000);
    QCOMPARE(false, cache->getImage(1, 100.0, 0).isNull());
}


// Test case moved from pdfloader unit tests
void Ut_PdfLoaderThread::testPdfLoaderGetPageImage()
{
    delete cache;
    cache = new PdfImageCache(5);
    delete thread;
    thread = new PdfLoaderThread(testDocument, cache);
    thread->start();
    Poppler::Document *doc = Poppler::Document::load(testDocument);

    connect(cache, SIGNAL(loadPage(int, qreal)),
            thread,SLOT(loadPage(int, qreal)), Qt::QueuedConnection);

    for (int page=0; page < doc->numPages(); page++) {
        QImage image = cache->getImage( page, 100, 0 );
        qDebug() << "IMAGE:" << image.size() << image.isNull();
        printf( "%d: %d, %d, %d\n", page, image.size().width(), image.size().height(), image.isNull());
        QCOMPARE(image.isNull(), true);
    }

    QTest::qWait(3000);

    for (int page=0; page < doc->numPages(); page++) {
        testloadPdfImage(page);
    }
}


void Ut_PdfLoaderThread::testloadPdfImage(int pageIndex)
{
    Poppler::Document *doc = Poppler::Document::load(testDocument);
    doc->setRenderHint(Poppler::Document::Antialiasing, true);
    doc->setRenderHint(Poppler::Document::TextAntialiasing, true);

    QImage popplerImage = doc->page(pageIndex)->renderToImage(100.0, 100.0);
    popplerImage = popplerImage.convertToFormat(QImage::Format_RGB16, Qt::AutoColor);

    QCOMPARE(cache->getImage(pageIndex,100.0, 0), popplerImage);
}


int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_PdfLoaderThread test;
    QTest::qExec(&test, argc, argv);
}
