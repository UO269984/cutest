#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "CuTest.h"

/*-------------------------------------------------------------------------*
 * CuStr
 *-------------------------------------------------------------------------*/

char* CuStrAlloc(size_t size) {
	return (char*) malloc(sizeof(char) * (size));
}

char* CuStrCopy(const char* old) {
	char* newStr = CuStrAlloc(strlen(old) + 1);
	strcpy(newStr, old);
	return newStr;
}

/*-------------------------------------------------------------------------*
 * CuString
 *-------------------------------------------------------------------------*/

void CuStringInit(CuString* str) {
	str->length = 0;
	str->size = STRING_MAX;
	
	str->buffer = (char*) malloc(sizeof(char) * str->size);
	str->buffer[0] = 0;
}

CuString* CuStringNew() {
	CuString* str = (CuString*) malloc(sizeof(CuString));
	CuStringInit(str);
	return str;
}

void CuStringDelete(CuString *str) {
	if (str != NULL) {
		free(str->buffer);
		free(str);
	}
}

void CuStringClear(CuString* str) {
	str->length = 0;
	str->buffer[0] = 0;
}

void CuStringResize(CuString* str, size_t newSize) {
	str->buffer = (char*) realloc(str->buffer, sizeof(char) * newSize);
	str->size = newSize;
}

void CuStringAppend(CuString* str, const char* text) {
	size_t length;
	if (text == NULL)
		text = "NULL";
	
	length = strlen(text);
	if (str->length + length + 1 >= str->size)
		CuStringResize(str, str->length + length + 1 + STRING_INC);
	
	str->length += length;
	strcat(str->buffer, text);
}

void CuStringAppendChar(CuString* str, char ch) {
	char text[] = {ch, 0};
	CuStringAppend(str, text);
}

void CuStringAppendFormat(CuString* str, const char* format, ...) {
	va_list argp;
	char buf[HUGE_STRING_LEN];
	va_start(argp, format);
	vsprintf(buf, format, argp);
	va_end(argp);
	CuStringAppend(str, buf);
}

void CuStringInsert(CuString* str, const char* text, size_t pos) {
	size_t length = strlen(text);
	if (pos > str->length)
		pos = str->length;
	
	if (str->length + length + 1 >= str->size)
		CuStringResize(str, str->length + length + 1 + STRING_INC);
	
	memmove(str->buffer + pos + length, str->buffer + pos, (str->length - pos) + 1);
	str->length += length;
	memcpy(str->buffer + pos, text, length);
}

/*-------------------------------------------------------------------------*
 * CuTest
 *-------------------------------------------------------------------------*/

void CuTestInit(CuTest* t, const char* name, TestFunction function) {
	t->name = CuStrCopy(name);
	t->function = function;
	t->failed = 0;
	t->ran = 0;
	t->message = NULL;
	t->jumpBuf = NULL;
}

CuTest* CuTestNew(const char* name, TestFunction function) {
	CuTest* tc = CU_ALLOC(CuTest);
	CuTestInit(tc, name, function);
	return tc;
}

void CuTestDelete(CuTest *t) {
	if (t != NULL) {
		#ifdef CPP_SUPPORT
		t->function.~TestFunction();
		#endif
		
		CuStringDelete(t->message);
		free(t->name);
		free(t);
	}
}

#ifdef CPP_SUPPORT
void CuTestRun(CuTest* tc) {
	try {
		tc->ran = 1;
		tc->function(tc);
	}
	catch (int ex) {}
}
#else
void CuTestRun(CuTest* tc) {
	jmp_buf buf;
	tc->jumpBuf = &buf;
	if (setjmp(buf) == 0) {
		tc->ran = 1;
		tc->function(tc);
	}
	tc->jumpBuf = 0;
}
#endif

static void CuFailInternal(CuTest* tc, const char* file, int line, const char* string) {
	char buf[HUGE_STRING_LEN];
	sprintf(buf, "%s:%d: %s", file, line, string);
	
	tc->failed = 1;
	CuStringDelete(tc->message);
	tc->message = CuStringNew();
	CuStringAppend(tc->message, buf);
	
	#ifdef CPP_SUPPORT
		throw 1;
	#else
	if (tc->jumpBuf != 0)
		longjmp(*(tc->jumpBuf), 0);
	#endif
}

void CuFail_Line(CuTest* tc, const char* file, int line, const char* message2, const char* message) {
	char buf[HUGE_STRING_LEN];
	
	if (message2 != NULL)
		sprintf(buf, "%s: %s", message2, message);
	else
		sprintf(buf, "%s", message);
	
	CuFailInternal(tc, file, line, buf);
}

void CuAssert_Line(CuTest* tc, const char* file, int line, const char* message, int condition) {
	if (! condition)
		CuFail_Line(tc, file, line, NULL, message);
}

void CuAssertStrEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message,
	const char* expected, const char* actual) {
	
	char buf[HUGE_STRING_LEN];
	if ((expected == NULL && actual == NULL) ||
		(expected != NULL && actual != NULL && strcmp(expected, actual) == 0)) {
		
		return;
	}
	if (expected == NULL)
		expected = "NULL";
	if (actual == NULL)
		actual = "NULL";
	
	if (message != NULL)
		sprintf(buf, "%s: expected <%s> but was <%s>", message, expected, actual);
	else
		sprintf(buf, "expected <%s> but was <%s>", expected, actual);
	
	CuFailInternal(tc, file, line, buf);
}

void CuAssertIntEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message,
	int expected, int actual) {
	
	char buf[STRING_MAX];
	if (expected != actual) {
		sprintf(buf, "expected <%d> but was <%d>", expected, actual);
		CuFail_Line(tc, file, line, message, buf);
	}
}

void CuAssertDblEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message,
	double expected, double actual, double delta) {
	
	char buf[STRING_MAX];
	if (fabs(expected - actual) > delta) {
		sprintf(buf, "expected <%f> but was <%f>", expected, actual);
		CuFail_Line(tc, file, line, message, buf);
	}
}

void CuAssertPtrEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message,
	const void* expected, const void* actual) {
	
	char buf[STRING_MAX];
	if (expected != actual) {
		sprintf(buf, "expected pointer <0x%p> but was <0x%p>", expected, actual);
		CuFail_Line(tc, file, line, message, buf);
	}
}


/*-------------------------------------------------------------------------*
 * CuSuite
 *-------------------------------------------------------------------------*/

void CuSuiteInit(CuSuite* testSuite) {
	testSuite->count = 0;
	testSuite->failCount = 0;
	memset(testSuite->list, 0, sizeof(testSuite->list));
}

CuSuite* CuSuiteNew() {
	CuSuite* testSuite = CU_ALLOC(CuSuite);
	CuSuiteInit(testSuite);
	return testSuite;
}

void CuSuiteDelete(CuSuite *testSuite) {
	for (unsigned int n = 0; n < MAX_TEST_CASES; n++) {
		if (testSuite->list[n])
			CuTestDelete(testSuite->list[n]);
	}
	free(testSuite);
}

void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase) {
	assert(testSuite->count < MAX_TEST_CASES);
	testSuite->list[testSuite->count] = testCase;
	testSuite->count++;
}

void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2) {
	for (int i = 0; i < testSuite2->count; ++i)
		CuSuiteAdd(testSuite, testSuite2->list[i]);
}

void CuSuiteRun(CuSuite* testSuite) {
	for (int i = 0; i < testSuite->count; ++i) {
		CuTest* testCase = testSuite->list[i];
		
		CuTestRun(testCase);
		if (testCase->failed)
			testSuite->failCount += 1;
	}
}

void CuSuiteSummary(CuSuite* testSuite, CuString* summary) {
	for (int i = 0; i < testSuite->count; ++i)
		CuStringAppend(summary, testSuite->list[i]->failed ? "F" : ".");
	
	CuStringAppend(summary, "\n\n");
}

void CuSuiteDetails(CuSuite* testSuite, CuString* details) {
	int failCount = 0;
	
	if (testSuite->failCount == 0) {
		int passCount = testSuite->count - testSuite->failCount;
		CuStringAppendFormat(details, "OK (%d %s)\n", passCount, passCount == 1 ? "test" : "tests");
	}
	else {
		if (testSuite->failCount == 1)
			CuStringAppend(details, "There was 1 failure:\n");
		else
			CuStringAppendFormat(details, "There were %d failures:\n", testSuite->failCount);
		
		for (int i = 0; i < testSuite->count; ++i) {
			CuTest* testCase = testSuite->list[i];
			
			if (testCase->failed) {
				failCount++;
				CuStringAppendFormat(details, "%d) %s: %s\n",
					failCount, testCase->name, testCase->message->buffer);
			}
		}
		CuStringAppend(details, "\n!!!FAILURES!!!\n");
		
		CuStringAppendFormat(details, "Runs: %d ", testSuite->count);
		CuStringAppendFormat(details, "Passes: %d ", testSuite->count - testSuite->failCount);
		CuStringAppendFormat(details, "Fails: %d\n", testSuite->failCount);
	}
}