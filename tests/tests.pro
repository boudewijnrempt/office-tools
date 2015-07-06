TEMPLATE    = subdirs

SUBDIRS     =   \
	ut_applicationwindow \
	ut_pageindicator \
	ut_pdfloader \
	ut_pdfloaderthread \
	ut_pdfpage \
	ut_pdfpagewidget \
	ut_servicefw \
	ut_zoomlevel \
    ut_misc \
    ut_basepagewidget \
    ut_actionpool \
    ut_pdfthumbprovider \
    ut_spreadsheet
	
tests.path = /usr/share/office-tools-tests
tests.files = tests.xml

######################################################
#Test files
######################################################

#This is not working so installing one by one
#pdfsamplefiles.path = /usr/share/office-tools/data/
#pdfsamplefiles.files = ../src/testdata/pdf/*.pdf

examplefile.path = /usr/share/office-tools-tests/data/
examplefile.files = ../testdata/pdf/excerpts.pdf

examplefile1.path = /usr/share/office-tools-tests/data/
examplefile1.files = ../testdata/pdf/presentation.pdf

examplefile2.path = /usr/share/office-tools-tests/data/
examplefile2.files = ../testdata/pdf/sonnets.pdf

examplefile3.path = /usr/share/office-tools-tests/data/
examplefile3.files = ../testdata/pdf/link.pdf

examplefile4.path = /usr/share/office-tools-tests/data/
examplefile4.files = ../testdata/office/spreadsheet.ods

INSTALLS += tests examplefile examplefile1 examplefile2 examplefile3 examplefile4
