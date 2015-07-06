#include <mapplication.h>
#include <QColor>
#include <QStyleOptionGraphicsItem>
#define private public
#define protected public
#include <pdfpage.h>
#undef private
#undef protected
#include <pdfsearch.h>
#include "ut_pdfpage.h"
#include "applicationwindow.h"


const QString testDocuments[] = {"/usr/share/office-tools-tests/data/link.pdf", "/usr/share/office-tools-tests/data/link.pdf", "/usr/share/office-tools-tests/data/link.pdf", "/usr/share/office-tools-tests/data/link.pdf", "/usr/share/office-tools-tests/data/link.pdf", "/usr/share/office-tools-tests/data/link.pdf"};
const QString testPdf = "/usr/share/office-tools-tests/data/excerpts.pdf";

Ut_PdfPage::Ut_PdfPage()
{
    appW = new MApplicationWindow();
    window = new ApplicationWindow(appW);

    for(int i=0; i<PAGES; i++)
        pdfpages[i] = new PdfPage(testDocuments[i]);
}


Ut_PdfPage::~Ut_PdfPage()
{
    for(int i=0; i<PAGES; i++) {
        delete pdfpages[i];
        pdfpages[i] = 0;
    }

    delete window;
}


void Ut_PdfPage::testCreation()
{
    for(int i=0; i<PAGES; i++)
        QVERIFY(pdfpages[i]);
}


void Ut_PdfPage::testCreateContent()
{
    for(int i=0; i<PAGES; i++)
        pdfpages[i]->createContent();
}


void Ut_PdfPage::testCrashing()
{
    for(int i=0; i<PAGES; i++) {
        pdfpages[i]->createContent();
        pdfpages[i]->loadDocument();
    }
}

void Ut_PdfPage::testSearchText()
{
    PdfPage *pdfPage = new PdfPage(testPdf);
    pdfPage->createContent();
    QSignalSpy spy(pdfPage, SIGNAL(loadSuccess(QString)));
    pdfPage->loadDocument();
    QCOMPARE(spy.count(), 1);
    pdfPage->createFinalContent();
    pdfPage->setVerticalCenterOnPagePoint(0, 0.5, 0);

    // Search first
    pdfPage->searchText(DocumentPage::SearchFirst, "Nautilus");
    QTest::qWait(1000);
    // Nautilus is on the second page
    QVERIFY(pdfPage->searchData.contains(1));

    // Search next
    pdfPage->searchText(DocumentPage::SearchNext, "Nautilus");
    // Next hit is in page 4
    QVERIFY(pdfPage->searchData.contains(3));

    // Search previous
    pdfPage->searchText(DocumentPage::SearchPrevious, "Nautilus");
    QTest::qWait(1000);
    QVERIFY(pdfPage->searchData.contains(1));

    delete pdfPage;
}


int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_PdfPage test;
    QTest::qExec(&test, argc, argv);
}
