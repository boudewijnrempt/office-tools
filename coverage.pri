# Coverage analysis

contains(CONFIG, coverage) {

  QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage

  QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage

  LIBS += -lgov
  QMAKE_CLEAN += $(OBJECTS_DIR)*.gcda $(OBJECTS_DIR)*.gcno

  gcov.target = gcov
  gcov.CONFIG = recursive
  gcov.commands = for d in $$SOURCES; do (gcov -a -c -o $(OBJECTS_DIR) \$$$$d >> gcov.analysis ); done; echo "See gcov.analysis for details"
  gcov.depends = $(TARGET)

  QMAKE_EXTRA_TARGETS += gcov

  QMAKE_CLEAN += *.gcov gcov.analysis

  gcovsummary.target = gcovsummary
  gcovsummary.commands = find ContentServices Plugins Src -name "gcov.analysis" -exec cat {} \; | ./gcov.py > gcov.analysis.short; echo "See gcov.analysis.short for details"
  gcovsummary.depends = FORCE

  QMAKE_EXTRA_TARGETS += gcovsummary

}
