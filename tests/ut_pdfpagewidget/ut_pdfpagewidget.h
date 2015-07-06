#ifndef UT__PDFPAGEWIDGET_H
#define UT__PDFPAGEWIDGET_H

#include <QtTest/QtTest>
#include <QObject>

class PdfPageWidget;

class PdfPage;

class PdfLoader;

class Ut_PdfPageWidget : public QObject
{
    Q_OBJECT

public:
    Ut_PdfPageWidget();
    virtual ~Ut_PdfPageWidget();

private Q_SLOTS:
    void testCreation();
    void testUpdateSize();
    void testPainter();
    void testSelectText();

private:
    PdfPageWidget *pdfpagewidget;
    PdfLoader *loader;
};

#endif
