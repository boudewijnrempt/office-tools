
QMAKE_EXTRA_TARGETS += doc
doc.target = doc

doc.commands = ( cd doc ; doxygen $${IN_PWD}/Doxyfile  );

doc.depends = FORCE

