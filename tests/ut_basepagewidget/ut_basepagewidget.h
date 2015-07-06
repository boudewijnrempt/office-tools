#ifndef UT__MISC_H
#define UT__MISC_H

#include <QtTest/QtTest>
#include <QObject>

#include "basepagewidget.h"


class Ut_basepagewidget  : public QObject
{
    Q_OBJECT

public:
    Ut_basepagewidget();
    virtual ~Ut_basepagewidget();

private slots:
    void testCreation();
    void testPageIndex();



private:
    BasePageWidget  *widget;
};

#endif //UT__MISC_H

