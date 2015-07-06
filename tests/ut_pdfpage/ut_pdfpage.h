#ifndef UT__PDFPAGE_H
#define UT__PDFPAGE_H

#include <QtTest/QtTest>
#include <QObject>


class PdfPage;
class ApplicationWindow;
class MApplicationWindow;


const int PAGES = 6;


class Ut_PdfPage : public QObject
{
    Q_OBJECT

public:
    Ut_PdfPage();
    virtual ~Ut_PdfPage();

private Q_SLOTS:
    void testCreation();
    void testCreateContent();
    void testCrashing();
    void testSearchText();
    //void testPainter();
    //void testSelectText();
    //void testClearSelectedText();

private:
    PdfPage *pdfpages[PAGES];
    ApplicationWindow *window;
    MApplicationWindow *appW;
};

#endif
