#ifndef UT__PDFLOADER_H
#define UT__PDFLOADER_H

#include <QtTest/QtTest>
#include <QObject>
#include <pdfloader.h>
#include <pdfpagewidget.h>


class Ut_PdfLoader : public QObject
{
    Q_OBJECT

public:
    Ut_PdfLoader();
    virtual ~Ut_PdfLoader();

private Q_SLOTS:
    void testCreation();
    void testCurrentPage();
    void testGetPageImage();
    void testDocumentLoadAndSizes();
    void testDocumentLoadAndSizes_data();

private:
    PdfLoader* pdfloader;
    Poppler::Document *mDocument;
};

#endif
