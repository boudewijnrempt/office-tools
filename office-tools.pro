TEMPLATE = subdirs

SUBDIRS = translations src/common src/plugins/documentviewer src/app tests

CONFIG += ordered

include(doc/doc.pri)

# Run tests (tests.xml)
QMAKE_EXTRA_TARGETS += test
test.target = test
	#test.commands = meego-run python2.3 /usr/bin/testrunner.py -v -f /usr/share/office-tools-tests/tests.xml -o /tmp/office-tools/office-tools-test-results.xml -e scratchbox
test.depends = FORCE
