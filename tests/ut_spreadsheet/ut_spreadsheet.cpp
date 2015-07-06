#include "ut_spreadsheet.h"
#define private public
#include <officeviewerspreadsheet.h>
#undef private
#include <actionpool.h>
#include <definitions.h>
#include <mapplication.h>

const QString testDocuments = "/usr/share/office-tools-tests/data/spreadsheet.ods";

Ut_SpreadSheet::Ut_SpreadSheet()
{
    spreadsheet = new OfficeViewerSpreadsheet;
    spreadsheet->loadDocument(testDocuments, 0);
}

Ut_SpreadSheet::~Ut_SpreadSheet()
{
    delete spreadsheet;
    spreadsheet = 0;
}

void Ut_SpreadSheet::testCreation()
{
    QVERIFY(spreadsheet);
}

void Ut_SpreadSheet::testCreateContent()
{
    bool created = spreadsheet->createKoWidget();
    QVERIFY(created);
}

void Ut_SpreadSheet::testPreview()
{
    //QList <QImage *>thumbnailList = spreadsheet->getPreviews();
    //QVERIFY(thumbnailList != 0);
}

void Ut_SpreadSheet::testSearch()
{
    //spreadsheet->startSearch("sheet");
}

void Ut_SpreadSheet::testFixedIndicators()
{
    bool created = spreadsheet->createKoWidget();
    QVERIFY(created);

    ActionPool::instance()->getAction(ActionPool::SpreadSheetFixedIndicators)->trigger();
}

void Ut_SpreadSheet::testFloatingIndicators()
{
    bool created = spreadsheet->createKoWidget();
    QVERIFY(created);

    ActionPool::instance()->getAction(ActionPool::SpreadSheetFloatingIndicators)->trigger();
}

void Ut_SpreadSheet::testNoIndicators()
{
    bool created = spreadsheet->createKoWidget();
    QVERIFY(created);

    ActionPool::instance()->getAction(ActionPool::SpreadSheetNoIndicators)->trigger();
}

void Ut_SpreadSheet::testSearchText()
{
    bool created = spreadsheet->createKoWidget();
    QVERIFY(created);

    spreadsheet->startSearch("searchstring");
    QTest::qWait(1000);
    spreadsheet->nextWord();
    spreadsheet->previousWord();
    QVERIFY(spreadsheet->searchResults.count());
    QCOMPARE(spreadsheet->searchResults.at(0).count, 2);
}



int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_SpreadSheet test;
    QTest::qExec(&test, argc, argv);
}
