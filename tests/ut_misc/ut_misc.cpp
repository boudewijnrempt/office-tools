
#include <QCoreApplication>
#include <QDebug>
#include <QList>

#include "ut_misc.h"




void Ut_misc::testgetRelativePoint()
{

    const struct testStruct {
        QPointF result;
        QPointF in;
        QSizeF size;
    } testData[] = {

        {  QPointF(-0.5,-0.5), QPointF(-100,-100), QSizeF(200,200)}
        ,{ QPointF(0.0,0.0), QPointF(0,0), QSizeF(0,0)}
        ,{ QPointF(0.0,0.0), QPointF(0,0), QSizeF(200,200)}
        ,{ QPointF(0.5,0.5), QPointF(100,100), QSizeF(200,200)}
        ,{ QPointF(1.0,1.0), QPointF(200,200), QSizeF(200,200)}
        ,{ QPointF(2.0,2.0), QPointF(400,400), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QPointF result = Misc::getRelativePoint(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testgetRelativeSize()
{

    const struct testStruct {
        QSizeF result;
        QSizeF in;
        QSizeF size;
    } testData[] = {

        {  QSizeF(-0.5,-0.5), QSizeF(-100,-100), QSizeF(200,200)}
        ,{ QSizeF(0.0,0.0), QSizeF(0,0), QSizeF(0,0)}
        ,{ QSizeF(0.0,0.0), QSizeF(0,0), QSizeF(200,200)}
        ,{ QSizeF(0.5,0.5), QSizeF(100,100), QSizeF(200,200)}
        ,{ QSizeF(1.0,1.0), QSizeF(200,200), QSizeF(200,200)}
        ,{ QSizeF(2.0,2.0), QSizeF(400,400), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QSizeF result = Misc::getRelativeSize(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testgetRelativeRect()
{

    const struct testStruct {
        QRectF result;
        QRectF in;
        QSizeF size;
    } testData[] = {

        {  QRectF(QPointF(-0.5,-0.5), QSizeF(-0.5,-0.5)),   QRectF(QPointF(-100,-100), QSizeF(-100,-100)), QSizeF(200,200)}
        ,{ QRectF(QPointF(0,0), QSizeF(0,0)),               QRectF(QPointF(0, 0), QSizeF(0,0)), QSizeF(0,0)}
        ,{ QRectF(QPointF(0.2, 0.3), QSizeF(0.4, 0.6)),     QRectF(QPointF(20,30), QSizeF(40,60)), QSizeF(100,100)}
        ,{ QRectF(QPointF(1,1), QSizeF(1,1)),               QRectF(QPointF(200,200), QSizeF(200,200)), QSizeF(200,200)}
        ,{ QRectF(QPointF(2,2), QSizeF(2,2)),               QRectF(QPointF(400,400), QSizeF(400,400)), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QRectF result = Misc::getRelativeRect(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testtranslateRelativePoint()
{

    const struct testStruct {
        QPointF in;
        QPointF result;
        QSizeF size;
    } testData[] = {

        {  QPointF(-0.5,-0.5), QPointF(-100,-100), QSizeF(200,200)}
        ,{ QPointF(0.0,0.0), QPointF(0,0), QSizeF(0,0)}
        ,{ QPointF(0.0,0.0), QPointF(0,0), QSizeF(200,200)}
        ,{ QPointF(0.5,0.5), QPointF(100,100), QSizeF(200,200)}
        ,{ QPointF(1.0,1.0), QPointF(200,200), QSizeF(200,200)}
        ,{ QPointF(2.0,2.0), QPointF(400,400), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QPointF result = Misc::translateRelativePoint(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testtranslateRelativeSize()
{

    const struct testStruct {
        QSizeF in;
        QSizeF result;
        QSizeF size;
    } testData[] = {

        {  QSizeF(-0.5,-0.5), QSizeF(-100,-100), QSizeF(200,200)}
        ,{ QSizeF(0.0,0.0), QSizeF(0,0), QSizeF(0,0)}
        ,{ QSizeF(0.0,0.0), QSizeF(0,0), QSizeF(200,200)}
        ,{ QSizeF(0.5,0.5), QSizeF(100,100), QSizeF(200,200)}
        ,{ QSizeF(1.0,1.0), QSizeF(200,200), QSizeF(200,200)}
        ,{ QSizeF(2.0,2.0), QSizeF(400,400), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QSizeF result = Misc::translateRelativeSize(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testtranslateRelativeRect()
{

    const struct testStruct {
        QRectF in;
        QRectF result;
        QSizeF size;
    } testData[] = {

        {  QRectF(QPointF(-0.5,-0.5), QSizeF(-0.5,-0.5)),   QRectF(QPointF(-100,-100), QSizeF(-100,-100)), QSizeF(200,200)}
        ,{ QRectF(QPointF(0,0), QSizeF(0,0)),               QRectF(QPointF(0, 0), QSizeF(0,0)), QSizeF(0,0)}
        ,{ QRectF(QPointF(0.2, 0.3), QSizeF(0.4, 0.6)),     QRectF(QPointF(20,30), QSizeF(40,60)), QSizeF(100,100)}
        ,{ QRectF(QPointF(1,1), QSizeF(1,1)),               QRectF(QPointF(200,200), QSizeF(200,200)), QSizeF(200,200)}
        ,{ QRectF(QPointF(2,2), QSizeF(2,2)),               QRectF(QPointF(400,400), QSizeF(400,400)), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QRectF result = Misc::translateRelativeRect(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testgetMaxSizeInGivenSize()
{

    const struct testStruct {
        QSizeF result;
        QSizeF in;
        QSizeF size;
    } testData[] = {

        {  QSizeF(200,200), QSizeF(1000,1000), QSizeF(200,200)}
        ,{ QSizeF(200,200), QSizeF(10,10), QSizeF(200,200)}
        ,{ QSizeF(200,100), QSizeF(2000,1000), QSizeF(200,200)}
        ,{ QSizeF(200,100), QSizeF(20,10), QSizeF(200,200)}
        ,{ QSizeF(100,200), QSizeF(1000,2000), QSizeF(200,200)}
    };


    for(unsigned int i = 0; i < (sizeof(testData) / sizeof testData[0]); i++) {

        QSizeF result = Misc::getMaxSizeInGivenSize(testData[i].in, testData[i].size);
        //qDebug() << __PRETTY_FUNCTION__  << result << testData[i].result;
        QVERIFY(result == testData[i].result);
    }
}

void Ut_misc::testgetFileTypeFromFile()
{
    QCOMPARE(QString("qtn_comm_filetype_pdf"), Misc::getFileTypeFromFile("/usr/share/office-tools-tests/data/presentation.pdf"));
    QCOMPARE(QString("qtn_comm_filetype_ods"), Misc::getFileTypeFromFile("/usr/share/office-tools-tests/data/spreadsheet.ods"));
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    Ut_misc test;
    return QTest::qExec(&test, argc, argv);
}
