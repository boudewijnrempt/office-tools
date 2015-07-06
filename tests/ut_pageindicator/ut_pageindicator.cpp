#include <mapplication.h>
#define private public
#include <pageindicator.h>
#undef private
#include <QSignalSpy>
#include "ut_pageindicator.h"
#include "applicationwindow.h"


Ut_PageIndicator::Ut_PageIndicator() :
    pageindicator(0)
{
    appW = new MApplicationWindow();
    window = new ApplicationWindow(appW);
    pageindicator = new PageIndicator("/usr/share/office-tools/data/link.pdf");
}


Ut_PageIndicator::~Ut_PageIndicator()
{
    delete pageindicator;
    pageindicator = 0;
    delete window;
}


void Ut_PageIndicator::testCreation()
{
    QVERIFY(pageindicator);
}


void Ut_PageIndicator::testPageCounters()
{
    pageindicator->show();
    QVERIFY(pageindicator->isVisible());
    pageindicator->setPageCounters(10,1);
    QVERIFY(pageindicator->pageNumberLabel->text().contains("1/10"));

    pageindicator->setPageCounters(10,0);
    QVERIFY(!pageindicator->isVisible());
}


int main(int argc, char* argv[])
{
    MApplication app(argc, argv);
    Ut_PageIndicator test;
    QTest::qExec(&test, argc, argv);
}
