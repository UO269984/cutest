CC := gcc
CXX := gcc
AR := ar
CFLAGS = -Wall -c -o $@

LIB_EXT := $(if $(filter-out $(OS),Windows_NT),a,lib)
CUTEST_LIB := cuTest.$(LIB_EXT)
CUTEST_CPP_LIB := cuTestCPP.$(LIB_EXT)

EXEC_EXT := $(if $(filter-out $(OS),Windows_NT),,.exe)
TEST_EXEC := CuTestTest$(EXEC_EXT)

.PHONY: all
all: $(CUTEST_LIB) $(CUTEST_CPP_LIB) $(TEST_EXEC)

.PHONY: buildFolders
buildFolders:
	mkdir -p bin
	mkdir -p binCpp

.PHONY: test
test: $(TEST_EXEC)
	@./$(TEST_EXEC)

$(TEST_EXEC): bin/AllTests.o bin/CuTestTest.o $(CUTEST_LIB)
	$(CC) -o $@ $^

$(CUTEST_LIB): bin/CuTest.o
	$(AR) rcs $@ $^

$(CUTEST_CPP_LIB): binCpp/CuTest.o binCpp/CuTestCPP.o
	$(AR) rcs $@ $^

bin/%.o: %.c
	$(CC) $(CFLAGS) $<

binCpp/CuTest.o: CuTest.c
	$(CXX) -x c++ -D CPP_SUPPORT $(CFLAGS) $<

binCpp/CuTestCPP.o: CuTestCPP.cpp
	$(CXX) $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f bin/*
	rm -f binCpp/*
	rm -f $(CUTEST_LIB)
	rm -f $(CUTEST_CPP_LIB)
	rm -f $(TEST_EXEC)