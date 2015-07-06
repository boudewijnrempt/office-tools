
#include <QCoreApplication>
#include <QDebug>
#include <QList>

#include "ut_basepagewidget.h"



Ut_basepagewidget::Ut_basepagewidget() :
    widget(0)
{
    widget = new BasePageWidget();
}


Ut_basepagewidget::~Ut_basepagewidget()
{
    delete widget;
    widget = 0;
}


void Ut_basepagewidget::testCreation()
{
    QVERIFY(widget);
}


void Ut_basepagewidget::testPageIndex()
{
    QVERIFY(widget->getPageIndex() == -1);

    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));
    int random = qrand() % 1000000;
    widget->setPageIndex(random);
    QVERIFY(widget->getPageIndex() == random);
}


int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    Ut_basepagewidget test;
    return QTest::qExec(&test, argc, argv);
}
