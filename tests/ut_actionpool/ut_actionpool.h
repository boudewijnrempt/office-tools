#ifndef UT__MISC_H
#define UT__MISC_H

#include <QtTest/QtTest>
#include <QObject>


class Ut_ActionPool  : public QObject
{
    Q_OBJECT

    /*
     * test methods
     */

private slots:
    void testSingelton();
    void testGetAction();
    void testNegativeGetAction();

};

#endif //UT__MISC_H

