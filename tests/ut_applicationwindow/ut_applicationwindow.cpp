#include <mapplication.h>
#include <MApplicationWindow>
#include <MList>

#define protected public
#define private public
#include <applicationwindow.h>
#include <documentlistpage.h>
#undef protected
#undef private
#include <documentlistmodel.h>
#include <shareuiinterface.h>
#include "applicationservice.h"

#include <QItemSelection>
#include <QItemSelectionModel>
//#include <QScreen>

#include <QSignalSpy>
#include "ut_applicationwindow.h"
#include "trackerutils.h"

const QString testDocument = "/usr/share/office-tools-tests/data/excerpts.pdf";
const QString bigTestDocument = "/usr/share/office-tools-tests/data/presentation.pdf";

const QString appService("com.nokia.OfficeToolsService");


void Ut_ApplicationWindow::initTestCase()
{
}

bool Ut_ApplicationWindow::indexTestFiles()
{
    bool indexed = true;
    // Track some files
    system("rm -Rf /tmp/Ut_ApplicationWindow");
    system("mkdir -p /tmp/Ut_ApplicationWindow/");

    QTest::qWait(2000);

    system("cp -f /usr/share/office-tools-tests/data/excerpts.pdf /tmp/Ut_ApplicationWindow/");
    system("cp -f /usr/share/office-tools-tests/data/presentation.pdf /tmp/Ut_ApplicationWindow/");
    system("cp -f /usr/share/office-tools-tests/data/spreadsheet.ods /tmp/Ut_ApplicationWindow/");

    system("tracker-control -s");

    system("tracker-control --index-file=/tmp/Ut_ApplicationWindow/excerpts.pdf");
    system("tracker-control --index-file=/tmp/Ut_ApplicationWindow/presentation.pdf");
    system("tracker-control --index-file=/tmp/Ut_ApplicationWindow/spreadsheet.ods");

    QTest::qWait(2000);

    system("tracker-search -t");

    trackedTestDocument = "/tmp/Ut_ApplicationWindow/excerpts.pdf";
    trackedTestDocument2 = "/tmp/Ut_ApplicationWindow/presentation.pdf";
    trackedTestDocument3 = "/tmp/Ut_ApplicationWindow/spreadsheet.ods";

    if(!QFile::exists(trackedTestDocument) || !QFile::exists(trackedTestDocument2) || !QFile::exists(trackedTestDocument3))
        return false;

    return indexed;
}

void Ut_ApplicationWindow::cleanupTestCase()
{
}

void Ut_ApplicationWindow::testConstructor()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);
    delete window;
}

void Ut_ApplicationWindow::testDestructor()
{
    QThreadPool::globalInstance()->reserveThread();
    QThreadPool::globalInstance()->reserveThread();
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);
    ApplicationService *applicationService = new ApplicationService(appService, testDocument, window);
    window->setApplicationService(applicationService);
    // window->addActions(); // Not yet implemented
    QVERIFY(window->OpenFile(bigTestDocument));
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("pdfpage"));
    window->OpenListPage();
    applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("documentlistpage"));
    // Test the destructor when multiple threads are active
    delete window;
    QThreadPool::globalInstance()->releaseThread();
    QThreadPool::globalInstance()->releaseThread();

    //Verify that the page is unloaded correctly
    QVERIFY((uint)appW.currentPage() != (uint)applicationPage);
    TrackerUtils::Shutdown();
}

void Ut_ApplicationWindow::testOpenNormalView()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);
    QVERIFY(window->OpenFile(testDocument));
    // Let it actually load the document
    QCoreApplication::processEvents();
    window->showNormalView();
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("pdfpage"));
    delete window;
    QTest::qWait(1000);
    TrackerUtils::Shutdown();
}

void Ut_ApplicationWindow::testShowAllPagesView()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);
    QVERIFY(window->OpenFile(testDocument));
    // Let it actually load the document
    QCoreApplication::processEvents();
    window->showAllPagesView();
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("allpagespage_layout"));
    delete window;
}

void Ut_ApplicationWindow::testShowAllPagesViewFailure()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);
    window->showAllPagesView();
    MApplicationPage * applicationPage = appW.currentPage();
    QVERIFY(!applicationPage);
    delete window;
}

void Ut_ApplicationWindow::testDocumentDetailsView()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);
    // Open details view with document path
    window->DocumentDetailsView(testDocument);
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("documentdetailview"));

    // Then load other document
    QVERIFY(window->OpenFile(bigTestDocument));
    QCoreApplication::processEvents();
    applicationPage = appW.currentPage();
    QVERIFY(applicationPage->objectName() != QString("documentdetailview"));

    // and test if it can show it's details
    window->DocumentDetailsView();
    applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("documentdetailview"));
    delete window;
}

void Ut_ApplicationWindow::testDocumentDetailsViewFailure()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window);

    // Try to show details without loading the document
    window->DocumentDetailsView();
    MApplicationPage * applicationPage = appW.currentPage();
    QVERIFY(!applicationPage);
    delete window;
    TrackerUtils::Shutdown();
}

void Ut_ApplicationWindow::testOpenFile()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(window->OpenFile(testDocument));
    // This is needed for the single-shot timer to execute a slot before the window gets destroyed
    QCoreApplication::processEvents();
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("pdfpage"));

    // Test opening more than one file
    // and when AllPagesView is on
    window->showAllPagesView();
    applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("allpagespage_layout"));
    QVERIFY(window->OpenFile(bigTestDocument));
    QCoreApplication::processEvents();
    applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("pdfpage"));
    delete window;
}


void Ut_ApplicationWindow::testOpenFileFailure()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    ApplicationService *applicationService = new ApplicationService(appService, testDocument, window);
    window->setApplicationService(applicationService);
    QVERIFY(!window->OpenFile("NonExits"));

    // Test load failure when there is already a previous file loaded and DocumentListPage is loaded
    QVERIFY(window->OpenFile(testDocument));
    QCoreApplication::processEvents();
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("pdfpage"));
    window->OpenListPage();
    applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("documentlistpage"));

    window->loadFailed("NonExists", "Unknown reason");
//    sleep(3); // Wait until the banner disappears
    applicationPage = appW.currentPage();
    QVERIFY(!applicationPage);

    delete window;
    QTest::qWait(1000);
    TrackerUtils::Shutdown();
}

void Ut_ApplicationWindow::testOpenListPage()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    ApplicationService *applicationService = new ApplicationService(appService, testDocument, window);
    window->setApplicationService(applicationService);
    window->OpenListPage();
    MApplicationPage * applicationPage = appW.currentPage();
    QCOMPARE(applicationPage->objectName(), QString("documentlistpage"));
    window->closeDocumentPage();
    delete window;
    TrackerUtils::Shutdown();
}


void Ut_ApplicationWindow::testCheckMimeType()
{
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.pdf"),  ApplicationWindow::DOCUMENT_PDF);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.odt"),  ApplicationWindow::DOCUMENT_WORD);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.doc"),  ApplicationWindow::DOCUMENT_WORD);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.docx"), ApplicationWindow::DOCUMENT_WORD);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.odp"),  ApplicationWindow::DOCUMENT_PRESENTATION);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.ppt"),  ApplicationWindow::DOCUMENT_PRESENTATION);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.pptx"), ApplicationWindow::DOCUMENT_PRESENTATION);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.ods"),  ApplicationWindow::DOCUMENT_SPREADSHEET);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.xls"),  ApplicationWindow::DOCUMENT_SPREADSHEET);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.xlsx"), ApplicationWindow::DOCUMENT_SPREADSHEET);
    QCOMPARE(ApplicationWindow::checkMimeType(""),     ApplicationWindow::DOCUMENT_UNKOWN);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.txt"),  ApplicationWindow::DOCUMENT_WORD);
    QCOMPARE(ApplicationWindow::checkMimeType("excerpts.rtf"),  ApplicationWindow::DOCUMENT_WORD);
}


//void Ut_ApplicationWindow::testVisibleOrientationSize()
//{
//    MApplicationWindow appW;
//    ApplicationWindow *window = new ApplicationWindow(&appW);
////    int height = QScreen::instance()->deviceHeight();
////    int width = QScreen::instance()->deviceWidth();
//    QCOMPARE(window->visibleSize(M::Portrait), QSize(480, 854));
//    QCOMPARE(window->visibleSize(M::Landscape), QSize(854, 480));
//    delete window;
//}


//void Ut_ApplicationWindow::testGetSceneManager()
//{
//    MApplicationWindow appW;
//    ApplicationWindow *window = new ApplicationWindow(&appW);
//    QCOMPARE(window->visibleSize(M::Portrait), QSize(480, 854));
//    QCOMPARE(window->visibleSize(M::Landscape), QSize(854, 480));
//    QVERIFY(window->GetSceneManager());
//    delete window;
//}

void Ut_ApplicationWindow::testFavourite()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(indexTestFiles());
    QVERIFY(window);
    QVERIFY(window->OpenFile(trackedTestDocument));
    QCoreApplication::processEvents();
    QTest::qWait(2000);
    QVERIFY(window->page);
    QVERIFY(window->pageLoaded);
    bool wasFavorite = DocumentListModel::documentIsFavorite(window->page->documentUrn);
    emit window->page->toggleFavorite();
    QCoreApplication::processEvents();
    QVERIFY(window->page);
    bool isFavorite = DocumentListModel::documentIsFavorite(window->page->documentUrn);
    delete window;
    QCOMPARE(isFavorite, !wasFavorite);
}

void Ut_ApplicationWindow::testSharing()
{
    MApplicationWindow appW;
    ApplicationWindow *window = new ApplicationWindow(&appW);
    QVERIFY(indexTestFiles());
    ApplicationService *applicationService = new ApplicationService(appService, testDocument, window);
    window->setApplicationService(applicationService);

    window->OpenListPage();
    QCoreApplication::processEvents();

    // Get selection model to hack it
    QItemSelectionModel *selectionModel = window->pageList->list->selectionModel();

    // Select all documents
    window->pageList->activateShareMultipleDocumentsView();
    QItemSelection *allItemsSelection = new QItemSelection(window->pageList->list->firstVisibleItem().child(0,0), window->pageList->list->lastVisibleItem());
    selectionModel->select(*allItemsSelection, QItemSelectionModel::Select);
    QCoreApplication::processEvents();

    window->pageList->openShare();
    QCoreApplication::processEvents();

    QVERIFY(window->shareIf->isValid());

    delete allItemsSelection;
    delete window;
}

//void Ut_ApplicationWindow::testDeletion()
//{
//    QVERIFY(indexTestFiles());

//    MApplicationWindow appW;
//    ApplicationWindow *window = new ApplicationWindow(&appW);
//    appW.show();
//    ApplicationService *applicationService = new ApplicationService(appService, testDocument, window);
//    window->setApplicationService(applicationService);
//    QVERIFY(window->OpenFile(trackedTestDocument));
//    emit window->page->toggleFavorite();
//    QCoreApplication::processEvents();
//    window->OpenListPage();
//    QCoreApplication::processEvents();

//    // Get selection model to hack it
//    QItemSelectionModel *selectionModel = window->pageList->list->selectionModel();

//    // Try to delete all documents
//    window->pageList->activateDeleteMultipleDocumentsView();
//    QItemSelection *allItemsSelection = new QItemSelection(window->pageList->list->firstVisibleItem().child(0,0), window->pageList->list->lastVisibleItem());
//    selectionModel->select(*allItemsSelection, QItemSelectionModel::Select);
//    QCoreApplication::processEvents();
//    emit window->pageList->deleteDocuments();
//    QTest::qWait(2000);
//    window->deleteConfirmationYes();
//    QVERIFY(!QFile::exists(trackedTestDocument) && !QFile::exists(trackedTestDocument2) && !QFile::exists(trackedTestDocument3));

//    // Try to delete only one document from list page
//    // Copy and index the documents again
//    QVERIFY(indexTestFiles());
//    QTest::qWait(2000);
//    // Fake long tap
//    window->pageList->longTappedRow = 1;
//    window->pageList->longTappedRowGroup = -1;
//    emit window->pageList->deleteDocuments();
//    window->deleteConfirmationYes();

//    // Because I don't know what is the order of the files on the list, and it changes randomly
//    // I don't know also what file I'm deleting. have to check by quantity
//    int howMany = 0;
//    if (QFile::exists(trackedTestDocument))
//        ++howMany;
//    if (QFile::exists(trackedTestDocument2))
//        ++howMany;
//    if (QFile::exists(trackedTestDocument3))
//        ++howMany;
//    QVERIFY(howMany == 2);

//    // Try to delete current opened document
//    QVERIFY(indexTestFiles()); //system("cp -f /usr/share/office-tools-tests/data/link.pdf /tmp/Ut_ApplicationWindow/");
//    QVERIFY(window->OpenFile(trackedTestDocument)); //"/tmp/Ut_ApplicationWindow/link.pdf"));
//    QCoreApplication::processEvents();
//    QVERIFY(window->page);
//    emit window->page->deleteDocument();
//    window->deleteConfirmationYes();

//    QVERIFY(!QFile::exists(trackedTestDocument)); //"/tmp/Ut_ApplicationWindow/link.pdf"));

//    delete allItemsSelection;
//    delete window;
//}

int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_ApplicationWindow test;
    return QTest::qExec(&test, argc, argv);
}
