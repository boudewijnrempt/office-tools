
#include <QCoreApplication>
#include <QDebug>
#include <QList>

#include "ut_zoomlevel.h"

Ut_TestData::Ut_TestData(ZoomLevel::Mode mode, qreal factor, bool expectedTestResult)
    :  mode(mode)
    ,  factor(factor)
    ,  expectedTestResult(expectedTestResult)
{
}

Ut_TestData::~Ut_TestData()
{

}

void Ut_ZoomLevel::testModeConstructor()
{
    QList<Ut_TestData> dataList;
    dataList << Ut_TestData(ZoomLevel::FitToHeight)
             << Ut_TestData(ZoomLevel::FitToWidth)
             << Ut_TestData(ZoomLevel::FitToPage)
             << Ut_TestData(ZoomLevel::Relative)
             << Ut_TestData(ZoomLevel::Relative, 2.0)
             << Ut_TestData(ZoomLevel::Relative, 0.1)
             << Ut_TestData(ZoomLevel::FactorMode)
             << Ut_TestData(ZoomLevel::FactorMode, 0.1)
             << Ut_TestData(ZoomLevel::FactorMode, 2.0)
             << Ut_TestData(ZoomLevel::FactorMode, 8.0);

    foreach(Ut_TestData data, dataList) {
        ZoomLevel zoom(data.mode, data.factor);
        QVERIFY(zoom.getMode() == data.mode);
    }
}

void Ut_ZoomLevel::testFactorConstructor()
{
    QList<Ut_TestData> dataList;
    dataList << Ut_TestData(ZoomLevel::FitToHeight, 2.0)
             << Ut_TestData(ZoomLevel::FitToWidth, 2.0)
             << Ut_TestData(ZoomLevel::FitToPage, 2.0)
             << Ut_TestData(ZoomLevel::Relative)
             << Ut_TestData(ZoomLevel::Relative, 2.0)
             << Ut_TestData(ZoomLevel::Relative, 0.1)
             << Ut_TestData(ZoomLevel::FactorMode)
             << Ut_TestData(ZoomLevel::FactorMode, 0.1)
             << Ut_TestData(ZoomLevel::FactorMode, 2.0)
             << Ut_TestData(ZoomLevel::FactorMode, 8.0);

    foreach(Ut_TestData data, dataList) {
        ZoomLevel zoom(data.mode, data.factor);
        qreal factor;
        bool ok= zoom.getFactor(factor);
        QVERIFY(ok == data.expectedTestResult);

        if(ok) {
            QVERIFY(factor == data.factor);
        }
    }
}

void Ut_ZoomLevel::testEqual()
{
    QList<Ut_TestData> src;
    src << Ut_TestData(ZoomLevel::FitToHeight)
        << Ut_TestData(ZoomLevel::FitToWidth)
        << Ut_TestData(ZoomLevel::FitToPage)
        << Ut_TestData(ZoomLevel::Relative)
        << Ut_TestData(ZoomLevel::FactorMode);

    QList< QList<Ut_TestData> > dest;
    QList<Ut_TestData> temp;
    //FitToHeight
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0)
         << Ut_TestData(ZoomLevel::FitToHeight, 2.0)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //FitToWidth
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0)
         << Ut_TestData(ZoomLevel::FitToWidth, 2.0)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //FitToPage
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0)
         << Ut_TestData(ZoomLevel::FitToPage, 2.0)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //Relative
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 2.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //FactorMode
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0)
         << Ut_TestData(ZoomLevel::FactorMode, 2.0, false);
    dest << temp;

    for(int i =0 ; i < src.size(); i++) {
        ZoomLevel a(src.at(i).mode, src.at(i).factor);
        foreach(Ut_TestData destItem, dest.at(i)) {
            ZoomLevel b(destItem.mode, destItem.factor);
            bool equal = (a == b);
            QVERIFY(destItem.expectedTestResult == equal);
        }
    }
}

void Ut_ZoomLevel::testNotEqual()
{
    QList<Ut_TestData> src;
    src << Ut_TestData(ZoomLevel::FitToHeight)
        << Ut_TestData(ZoomLevel::FitToWidth)
        << Ut_TestData(ZoomLevel::FitToPage)
        << Ut_TestData(ZoomLevel::Relative)
        << Ut_TestData(ZoomLevel::FactorMode);

    QList< QList<Ut_TestData> > dest;
    QList<Ut_TestData> temp;
    //FitToHeight
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0)
         << Ut_TestData(ZoomLevel::FitToHeight, 2.0)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //FitToWidth
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0)
         << Ut_TestData(ZoomLevel::FitToWidth, 2.0)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //FitToPage
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0)
         << Ut_TestData(ZoomLevel::FitToPage, 2.0)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //Relative
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 2.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false);
    dest << temp;
    temp.clear();
    //FactorMode
    temp << Ut_TestData(ZoomLevel::FitToHeight, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToWidth, 1.0, false)
         << Ut_TestData(ZoomLevel::FitToPage, 1.0, false)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0)
         << Ut_TestData(ZoomLevel::FactorMode, 2.0, false);
    dest << temp;

    for(int i =0 ; i < src.size(); i++) {
        ZoomLevel a(src.at(i).mode, src.at(i).factor);
        foreach(Ut_TestData destItem, dest.at(i)) {
            ZoomLevel b(destItem.mode, destItem.factor);
            bool equal = (a != b);
            QVERIFY(destItem.expectedTestResult != equal);
        }
    }
}

void Ut_ZoomLevel::testIsFitTo()
{
    QList<Ut_TestData> data;
    data << Ut_TestData(ZoomLevel::FitToHeight)
         << Ut_TestData(ZoomLevel::FitToWidth)
         << Ut_TestData(ZoomLevel::FitToPage)
         << Ut_TestData(ZoomLevel::Relative, 1.0, false)
         << Ut_TestData(ZoomLevel::FactorMode, 1.0, false)
         ;

    foreach(Ut_TestData item, data) {
        ZoomLevel a(item.mode, item.factor);
        QVERIFY(item.expectedTestResult == !a.isFitTo());
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    Ut_ZoomLevel test;
    return QTest::qExec(&test, argc, argv);
}
