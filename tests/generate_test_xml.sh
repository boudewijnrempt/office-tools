#!/bin/sh

# create test difinition file
testhelper -d /usr/lib/office-tools-tests -p office-tools-tests -o tests.xml

# use tidy -program to format tests.xml for better readability
# tidy -indent -xml -wrap 1000 tests.xml > tidy.xml
# mv tidy.xml tests.xml

# validate test difinition XML file
# xmllint --noout --schema testdefinition.xsd tests.xml

# run tests and format results
# /usr/bin/testrunner.py -f tests.xml  -o result.xml -e scratchbox
# /usr/bin/testrunner.py -f /usr/share/office-tools-tests/tests.xml -o result.xml -e scratchbox
# tidy -indent -xml -wrap 1000 result.xml > tidy.xml
# mv tidy.xml result.xml
