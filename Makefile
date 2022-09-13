CC := gcc
CXX := gcc
CFLAGS = -Wall -c -o $@
DEFINES := -D CPP_SUPPORT

.PHONY: all
all: cuTest.a cuTestCPP.a CuTestTest

.PHONY: buildFolders
buildFolders:
	mkdir -p bin
	mkdir -p binCpp

.PHONY: test
test: CuTestTest
	@./CuTestTest

CuTestTest: bin/AllTests.o bin/CuTestTest.o cuTest.a
	$(CC) -o $@ $^

cuTest.a: bin/CuTest.o
	ar rcs $@ $^

cuTestCPP.a: binCpp/CuTest.o binCpp/CuTestCPP.o
	ar rcs $@ $^

bin/%.o: %.c
	$(CC) $(CFLAGS) $<

binCpp/CuTest.o: CuTest.c
	$(CXX) -x c++ $(DEFINES) $(CFLAGS) $<

binCpp/CuTestCPP.o: CuTestCPP.cpp
	$(CXX) $(DEFINES) $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f bin/*
	rm -f binCpp/*
	rm -f cuTest.a
	rm -f cuTestCPP.a
	rm -f CuTestTest