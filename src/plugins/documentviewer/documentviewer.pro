TEMPLATE = lib
DESTDIR = ../../

TARGET = office-tools-viewer

OBJECTS_DIR = ./.build
MOC_DIR = ./.build

QT += dbus
QT += xml


PKGCONFIG += meegotouch poppler-qt4 QtSparql

QMAKE_CXXFLAGS += -DNO_PIGMENT

#CONFIG += link_pkgconfig share-ui-service
CONFIG += link_pkgconfig shareuiinterface-maemo-meegotouch libsaveas libcontextsubscriber

CONFIG += release plugin
#CONFIG += debug

LIBS += -lmeegotouchcore
LIBS += -L../.. -loffice-tools-common
LIBS += -lmeegotouchcore  -lkok
LIBS += -lkomain \
        -lkopageapp \
        -lcalligratablescommon \
        -lwordsprivate

INCLUDEPATH += /usr/include/KDE/ \
               /usr/include/poppler/qt4/ \
               /usr/include/koffice \
               /usr/include/tables \
               /usr/include/tables/part \
               /usr/include/words/part \
               /usr/include \
               ../../common \
               ../..

# Input
HEADERS += \
    Limits.h \
    officepage.h \
    officeviewer.h \
    officeviewerpresentation.h \
    officeviewerspreadsheet.h \
    officeviewerword.h \
    officeviewereventfilter.h \
    pannablescrollbars.h \
    pdfimagecache.h \
    pdfloader.h \
    pdfloaderthread.h \
    pdfpage.h \
    pdfpagewidget.h \
    pdfsearch.h \
    pdfthumbprovider.h \
    searchresult.h \
    officefind.h \
    slideanimator.h \
    spreadsheetcommon.h \
    documentviewer.h

SOURCES += \
    officepage.cpp \
    officeviewer.cpp \
    officeviewerpresentation.cpp \
    officeviewerspreadsheet.cpp \
    officeviewerword.cpp \
    officeviewereventfilter.cpp \
    pannablescrollbars.cpp \
    pdfimagecache.cpp \
    pdfloader.cpp \
    pdfloaderthread.cpp \
    pdfpage.cpp \
    pdfpagewidget.cpp \
    pdfsearch.cpp \
    pdfthumbprovider.cpp \
    officefind.cpp \
    slideanimator.cpp \
    spreadsheetcommon.cpp \
    documentviewer.cpp


# Install instructions
target.path = /usr/lib/office-tools/viewer

INSTALLS += target 

QMAKE_CXXFLAGS += -Wall #-Werror -Wno-long-long -pedantic
QMAKE_CXXFLAGS_DEBUG += -O0 -g3

#include(../../../gcov.pri)
