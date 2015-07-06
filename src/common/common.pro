TEMPLATE = lib
TARGET = office-tools-common
DESTDIR = ..
OBJECTS_DIR = ./.build
MOC_DIR = ./.build
DEFINES += COMMON_LIBRARY

QT += dbus
QT += xml

#CONFIG += link_pkgconfig share-ui-service
CONFIG += link_pkgconfig \
          shareuiinterface-maemo-meegotouch \
          libsaveas \
          libcontextsubscriber \
          QmSystem2 \
          qtsparql-tracker-live

CONFIG += release
#CONFIG += debug
LIBS += -lmeegotouchcore
LIBS += -lkok

PKGCONFIG += meegotouch poppler-qt4 QtSparql

INCLUDEPATH += /usr/include \
               /usr/include/KDE/ \
               ..

# Input
HEADERS +=  \
    actionpool.h \
    allpagespage.h \
    applicationservice.h \
    applicationwindow.h \
    basepagewidget.h \
    documentdetailview.h \
    documentlistitem.h \
    documentlistmodel.h \
    documentlistpage.h \
    documentpage.h \
    findtoolbar.h \
    jumptotoolbar.h \
    misc.h \
    officethumbprovider.h \
    officeviewerbase.h \
    pageindicator.h \
    thumbpagelayoutpolicy.h \
    thumbprovider.h \
    thumbwidget.h \
    trackerutils.h \
    zoomlevel.h \
    quickviewertoolbar.h

SOURCES += \
    actionpool.cpp \
    allpagespage.cpp \
    applicationservice.cpp \
    applicationwindow.cpp \
    basepagewidget.cpp \
    documentdetailview.cpp \
    documentlistitem.cpp \
    documentlistmodel.cpp \
    documentlistpage.cpp \
    documentpage.cpp \
    findtoolbar.cpp \
    jumptotoolbar.cpp \
    misc.cpp \
    officethumbprovider.cpp \
    pageindicator.cpp \
    thumbprovider.cpp \
    thumbwidget.cpp \
    trackerutils.cpp \
    zoomlevel.cpp \
    quickviewertoolbar.cpp

# Install instructions
target.path = /usr/lib


INSTALLS += target

QMAKE_CXXFLAGS += -Wall #-Werror -Wno-long-long -pedantic
QMAKE_CXXFLAGS_DEBUG += -O0 -g3

#include(../gcov.pri)
