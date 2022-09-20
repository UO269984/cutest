#include "CuTestCPP.h"

const std::vector<TestFuncData>* BaseCuTest::getTestFuncs() const {
	return &testFunctions;
}

CuSuiteWrapper::CuSuiteWrapper() {
	suite = CuSuiteNew();
}

CuSuiteWrapper::~CuSuiteWrapper() {
	CuSuiteDelete(suite);
}

void CuSuiteWrapper::addTest(BaseCuTest* test) {
	const std::vector<TestFuncData>* testFuncs = test->getTestFuncs();
	
	if (testFuncs->size() != 0) {
		baseTests.push_back(test);
		
		for (const TestFuncData& funcData : *(testFuncs))
			SUITE_ADD_TEST_NAME(suite, [this](CuTest* tc) {this->runTest(tc);}, funcData.name);
	}
}

void CuSuiteWrapper::runTest(CuTest* tc) {
	if (testIndex == 0)
		(*testsIt)->testStart();
	
	(*testsIt)->tc = tc;
	(*testsIt)->before();
	try {
		(*(*testsIt)->getTestFuncs())[testIndex++].func();
	}
	catch (int ex) {
		nextTest();
		throw ex;
	}
	nextTest();
}

void CuSuiteWrapper::nextTest() {
	(*testsIt)->after();
	
	if (testIndex == (*testsIt)->getTestFuncs()->size()) {
		(*(testsIt++))->testEnd();
		testIndex = 0;
	}
}

bool CuSuiteWrapper::run() {
	testIndex = 0;
	testsIt = baseTests.begin();
	CuSuiteRun(suite);
	
	summary.clear();
	details.clear();
	CuString* cuStr = CuStringNew();
	
	CuSuiteSummary(suite, cuStr);
	summary.append(cuStr->buffer);
	CuStringClear(cuStr);
	
	CuSuiteDetails(suite, cuStr);
	details.append(cuStr->buffer);
	CuStringDelete(cuStr);
	
	testSuccess = suite->failCount == 0;
	return testSuccess;
}

CuTestResult CuSuiteWrapper::getTestResult() const {
	return {testSuccess, summary.c_str(), details.c_str()};
}

void CuSuiteWrapper::printResult() const {
	printf(summary.c_str());
	printf(details.c_str());
}