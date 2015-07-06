#ifndef UT__ZOOMLEVEL_H
#define UT__ZOOMLEVEL_H

#include <QtTest/QtTest>
#include <QObject>

#include "zoomlevel.h"

class Ut_TestData
{

public:
    Ut_TestData(ZoomLevel::Mode mode, qreal factor=1.0, bool expectedTestResult=true);
    virtual ~Ut_TestData();

    ZoomLevel::Mode mode;
    qreal           factor;
    bool            expectedTestResult;
};


class Ut_ZoomLevel : public QObject
{
    Q_OBJECT

    /*
     * test methods
     */

private slots:
    void testModeConstructor();
    void testFactorConstructor();
    void testEqual();
    void testNotEqual();
    void testIsFitTo();



};

#endif //UT__ZOOMLEVEL_H

