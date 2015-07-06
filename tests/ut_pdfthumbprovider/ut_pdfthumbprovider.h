#ifndef UT__PDFLOADER_H
#define UT__PDFLOADER_H

#include <QtTest/QtTest>
#include <QObject>

#include <pdfloader.h>

class PdfThumbProvider;

class Ut_PdfThumbProvider : public QObject
{
    Q_OBJECT

public:
    Ut_PdfThumbProvider();
    virtual ~Ut_PdfThumbProvider();

private Q_SLOTS:
    void testCreation();
    void testGetPageCount();
    void testGetThumbSize();

private:
    PdfThumbProvider *pdfThumbProvider;
    PdfLoader pdfLoader;
};

#endif
