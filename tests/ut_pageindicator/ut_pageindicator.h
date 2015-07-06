#ifndef UT__ZOOMACTIONWIDGET_H
#define UT__ZOOMACTIONWIDGET_H

#include <QtTest/QtTest>
#include <QObject>

class PageIndicator;
class ApplicationWindow;
class MApplicationWindow;

class Ut_PageIndicator : public QObject
{
    Q_OBJECT

public:
    Ut_PageIndicator();
    virtual ~Ut_PageIndicator();

private Q_SLOTS:
    void testCreation();
    void testPageCounters();

private:
    PageIndicator *pageindicator;
    ApplicationWindow *window;
    MApplicationWindow *appW;
};

#endif
