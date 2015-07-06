#ifndef UT__PDFLOADER_H
#define UT__PDFLOADER_H

#include <QtTest/QtTest>
#include <QObject>


class Ut_ApplicationWindow : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testDestructor();
    void testOpenNormalView();
    void testShowAllPagesView();
    void testShowAllPagesViewFailure();
    void testDocumentDetailsView();
    void testDocumentDetailsViewFailure();
    void testOpenFile();
    void testOpenFileFailure();
    void testOpenListPage();
    void testCheckMimeType();
//    void testVisibleOrientationSize();
//    void testGetSceneManager();
    void testFavourite();
    void testSharing();
//    void testDeletion();

private:
    bool indexTestFiles();
    // Test document indexed by tracker
    QString trackedTestDocument;
    QString trackedTestDocument2;
    QString trackedTestDocument3;
    bool myDocsExists;
    QString homeDir;
};

#endif
