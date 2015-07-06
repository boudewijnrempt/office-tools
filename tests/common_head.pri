TEMPLATE = app

DEPENDPATH += . ../../src ../../src/.build ../../src/app ../../src/app/.build  ../../src/common ../../src/common/.build ../../src/plugins/documentviewer ../../src/plugins/documentviewer/.build
QMAKE_CLEAN += Makefile

INCLUDEPATH += ../../src \
               ../../src/common \
               ../../src/plugins/documentviewer \
               /usr/include/meegotouch \
               /usr/include/office-tools/service  \
               /usr/include/KDE \
               /usr/include/koffice \
               /usr/include/poppler/qt4/ \
               /usr/include/qmsystem2/

QMAKE_LIBDIR += ../../src \
                /usr/lib \
                /usr/lib/office-tools/viewer/
                
LIBS += -lkomain \
        -lkopageapp \
        -lcalligratablescommon \
        -lwordsprivate \
        -loffice-tools-common \
        -loffice-tools-viewer

OBJECTS_DIR = .build
MOC_DIR = .build

CONFIG += debug link_pkgconfig shareuiinterface-maemo-meegotouch thread libsaveas
QT += dbus testlib xml

# installation
target.path = /usr/lib/office-tools-tests
INSTALLS   += target

PKGCONFIG += meegotouch poppler-qt4 QtSparqlTrackerLive QtSparql

include(../gcov.pri)

