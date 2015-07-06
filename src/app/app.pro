TEMPLATE = app
DESTDIR = ..

TARGET = office-tools

OBJECTS_DIR = ./.build
MOC_DIR = ./.build

QT += dbus
QT += xml

#CONFIG += link_pkgconfig share-ui-service
CONFIG += link_pkgconfig shareuiinterface-maemo-meegotouch libsaveas meegotouch-boostable libcontextsubscriber

CONFIG += release
#CONFIG += debug
LIBS += -L.. -loffice-tools-common
LIBS += -lmeegotouchcore 

PKGCONFIG += meegotouch QtSparql

INCLUDEPATH += /usr/include ../common ..

system(qdbusxml2cpp -c OfficeServiceIfAdaptor -a officetoolsserviceifadaptor.h:officetoolsserviceidadaptor.cpp com.nokia.maemo.meegotouch.OfficeToolsInterface.xml)

# Input
HEADERS += \
    officetoolsserviceifadaptor.h

SOURCES += main.cpp \
    officetoolsserviceidadaptor.cpp

# Install instructions
css.path = /usr/share/themes/base/meegotouch/office-tools/style/
css.files = style/office-tools.css

language.path = /usr/share/l10n/meegotouch/
language.files = ../../translations/officetools.qm

language2.path = /usr/share/doc/office-tools-l10n-engineering-english/
language2.files = ../../translations/officetools.ts

conf.path = /usr/share/themes/base/meegotouch/office-tools/
conf.files = style/office-tools.conf

desktop_entry.path = /usr/share/applications
desktop_entry.files = style/office-tools.desktop style/office-tools-frontpage.desktop

examplefile.path = /usr/share/office-tools/data/
examplefile.files = ../../testdata/pdf/link.pdf

examplefile1.path = /usr/share/office-tools-tests/data/
examplefile1.files = ../../testdata/office/spreadsheet.ods

dbusservice.path = /usr/share/dbus-1/services/
dbusservice.files = com.nokia.officetools.service

target.path = /usr/bin

images.path = /usr/share/themes/base/meegotouch/office-tools/images/
images.files = images/document.png images/pdf.png images/presentation.png images/spreadsheet.png images/meegotouch-office-tools-raster-pattern.png

INSTALLS += target css conf desktop_entry examplefile dbusservice language language2 images examplefile1

QMAKE_CXXFLAGS += -Wall #-Werror -Wno-long-long -pedantic
QMAKE_CXXFLAGS_DEBUG += -O0 -g3

#include(../../gcov.pri)
