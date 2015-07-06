#ifndef UT__MISC_H
#define UT__MISC_H

#include <QtTest/QtTest>
#include <QObject>

#include "misc.h"


class Ut_misc  : public QObject
{
    Q_OBJECT

    /*
     * test methods
     */

private slots:
    void testgetRelativePoint();
    void testgetRelativeSize();
    void testgetRelativeRect();
    void testtranslateRelativePoint();
    void testtranslateRelativeSize();
    void testtranslateRelativeRect();
    void testgetMaxSizeInGivenSize();
    void testgetFileTypeFromFile();
};

#endif //UT__MISC_H

