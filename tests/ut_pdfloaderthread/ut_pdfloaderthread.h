#ifndef UT__PDFLOADERTHREAD_H
#define UT__PDFLOADERTHREAD_H

#include <QtTest/QtTest>
#include <QObject>

class PdfImageCache;

class PdfLoaderThread;

class Ut_PdfLoaderThread : public QObject
{
    Q_OBJECT

public:
    Ut_PdfLoaderThread();
    virtual ~Ut_PdfLoaderThread();

private Q_SLOTS:
    void testCreation();
    void testLoadPage();
    void testPdfLoaderGetPageImage();
    void testloadPdfImage(int pageIndex);

private:
    PdfImageCache *cache;
    PdfLoaderThread *thread;
    int scale;
};

#endif
