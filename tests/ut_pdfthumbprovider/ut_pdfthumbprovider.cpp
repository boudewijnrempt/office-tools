#include <mapplication.h>
#include <QSignalSpy>

#include "ut_pdfthumbprovider.h"
#include "pdfthumbprovider.h"


const QString testDocument = "/usr/share/office-tools/data/link.pdf";


Ut_PdfThumbProvider::Ut_PdfThumbProvider() :
    pdfThumbProvider(0)
{
    Poppler::Document *doc = 0;
    pdfLoader.load(testDocument, doc);

    pdfThumbProvider = new PdfThumbProvider(&pdfLoader);
}


Ut_PdfThumbProvider::~Ut_PdfThumbProvider()
{
    delete pdfThumbProvider;
    pdfThumbProvider = 0;
}


void Ut_PdfThumbProvider::testCreation()
{
    QVERIFY(pdfThumbProvider);
}


void Ut_PdfThumbProvider::testGetPageCount()
{
    QCOMPARE(5, pdfThumbProvider->getPageCount());
}

void Ut_PdfThumbProvider::testGetThumbSize()
{
    pdfThumbProvider->init(0, "what ever");
    QCOMPARE(5, pdfThumbProvider->getPageCount());
    /*
     for (int i = 0; i < pdfThumbProvider->getPageCount(); i++) {
         QSizeF size = pdfThumbProvider->getThumbSize(i, QSizeF(300,300));
         QCOMPARE(size, QSizeF(231.818,300));
     }
     */
    delete pdfThumbProvider;

    pdfThumbProvider = 0;
    pdfThumbProvider = new PdfThumbProvider(&pdfLoader);
}



int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_PdfThumbProvider test;
    return QTest::qExec(&test, argc, argv);
}
