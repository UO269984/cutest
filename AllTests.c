#include <stdio.h>
#include <stdlib.h>

#include "CuTest.h"

CuSuite* CuGetSuite(void);
CuSuite* CuStringGetSuite(void);

int RunAllTests(void) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	
	CuSuite* auxSuite = CuGetSuite();
	CuSuiteAddSuite(suite, auxSuite);
	free(auxSuite);
	
	auxSuite = CuStringGetSuite();
	CuSuiteAddSuite(suite, auxSuite);
	free(auxSuite);
	
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	
	int failCount = suite->failCount;
	CuSuiteDelete(suite);
	CuStringDelete(output);
	return failCount;
}

int main() {
	return RunAllTests();
}