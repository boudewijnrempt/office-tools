#ifndef UT__SPREADSHEET_H
#define UT__SPREADSHEET_H

#include <QtTest/QtTest>
#include <QObject>

class OfficeViewerSpreadsheet;

class Ut_SpreadSheet : public QObject
{
    Q_OBJECT

public:
    Ut_SpreadSheet();
    virtual ~Ut_SpreadSheet();

private Q_SLOTS:
    void testCreation();
    void testCreateContent();
    void testPreview();
    void testSearch();
    void testFixedIndicators();
    void testFloatingIndicators();
    void testNoIndicators();
    void testSearchText();

private:
    OfficeViewerSpreadsheet *spreadsheet;
};

#endif
