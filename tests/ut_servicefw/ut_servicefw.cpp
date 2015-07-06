#include <QtTest/QtTest>
//#include </usr/include/office-tools/service/officetoolsserviceif.h>
#include <maemo-meegotouch-interfaces/officetoolsinterface.h>


class TestOfficeService: public QObject
{
    Q_OBJECT

private slots:
    void services();
    void openFile();
};


void TestOfficeService::services()
{
    //Test if the interface is connected and usable
    OfficeToolsInterface *office = new OfficeToolsInterface();
    QVERIFY(office->isValid());

    //Test if we can find the service from a list of all available services
    QStringList services = office->serviceNames();
    bool found = false;
    QString name = "";
    foreach(name,services) {
        if(name == "com.nokia.OfficeToolsService") {
            found = true;
            break;
        }
    }

    QVERIFY(found);
}


void TestOfficeService::openFile()
{
    OfficeToolsInterface *office = new OfficeToolsInterface();
    QVERIFY(office->isValid());
    bool write = office->OpenFile("/usr/share/office-tools-tests/data/excerpts.pdf");
    QVERIFY(write);
}


QTEST_MAIN(TestOfficeService)
#include "ut_servicefw.moc"
