CXX = g++
CXXFLAGS ?= -g -Wall -W -ansi # -pedantic
LDFLAGS ?= 
SED = sed
MV = mv
RM = rm

.SUFFIXES: .o .cpp

lib = libUnitTest++.a
test = TestUnitTest++

src = unittest/AssertException.cpp \
	unittest/Test.cpp \
	unittest/Checks.cpp \
	unittest/TestRunner.cpp \
	unittest/TestResults.cpp \
	unittest/TestReporter.cpp \
	unittest/TestReporterStdout.cpp \
	unittest/ReportAssert.cpp \
	unittest/TestList.cpp \
	unittest/TimeConstraint.cpp \
	unittest/TestDetails.cpp \
	unittest/MemoryOutStream.cpp \
	unittest/DeferredTestReporter.cpp \
	unittest/DeferredTestResult.cpp \
	unittest/XmlTestReporter.cpp \
	unittest/CurrentTest.cpp
	
ifeq ($(MSYSTEM), MINGW32)
  src += unittest/Win32/TimeHelpers.cpp
else
  src += unittest/Posix/SignalTranslator.cpp \
	unittest/Posix/TimeHelpers.cpp
endif

test_src = unittest/tests/Main.cpp \
	unittest/tests/TestAssertHandler.cpp \
	unittest/tests/TestChecks.cpp \
	unittest/tests/TestUnitTest++.cpp \
	unittest/tests/TestTest.cpp \
	unittest/tests/TestTestResults.cpp \
	unittest/tests/TestTestRunner.cpp \
	unittest/tests/TestCheckMacros.cpp \
	unittest/tests/TestTestList.cpp \
	unittest/tests/TestTestMacros.cpp \
	unittest/tests/TestTimeConstraint.cpp \
	unittest/tests/TestTimeConstraintMacro.cpp \
	unittest/tests/TestMemoryOutStream.cpp \
	unittest/tests/TestDeferredTestReporter.cpp \
	unittest/tests/TestXmlTestReporter.cpp \
	unittest/tests/TestCurrentTest.cpp

objects = $(patsubst %.cpp, %.o, $(src))
test_objects = $(patsubst %.cpp, %.o, $(test_src))
dependencies = $(subst .o,.d,$(objects))
test_dependencies = $(subst .o,.d,$(test_objects))

define make-depend
  $(CXX) $(CXXFLAGS) -M $1 | \
  $(SED) -e 's,\($(notdir $2)\) *:,$(dir $2)\1: ,' > $3.tmp
  $(SED) -e 's/#.*//' \
      -e 's/^[^:]*: *//' \
      -e 's/ *\\$$//' \
      -e '/^$$/ d' \
      -e 's/$$/ :/' $3.tmp >> $3.tmp
  $(MV) $3.tmp $3
endef


all: $(test)


$(lib): $(objects) 
	@echo Creating $(lib) library...
	@ar cr $(lib) $(objects)
    
$(test): $(lib) $(test_objects)
	@echo Linking $(test)...
	@$(CXX) $(LDFLAGS) -o $(test) $(test_objects) $(lib)
	@echo Running unit tests...
	@./$(test)

clean:
	-@$(RM) $(objects) $(test_objects) $(dependencies) $(test_dependencies) $(test) $(lib) 2> /dev/null

%.o : %.cpp
	@echo $<
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@$(CXX) $(CXXFLAGS) -c $< -o $(patsubst %.cpp, %.o, $<)


ifneq "$(MAKECMDGOALS)" "clean"
-include $(dependencies)
-include $(test_dependencies)
endif
