/*
 * mlogtest.c
 *
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * Copyright (C) 2012, 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#undef NDEBUG

#include <librfn.h>

bool compare_line(int n, const char *val)
{
	char *logval = mlog_get_line(n);
	bool result;

	if (logval == NULL || val == NULL)
		result = logval == val;
	else
		result = 0 == strcmp(logval, val);

	if (!result) {
		printf("failed\n");
		printf("Expected: \"%s\"\n", val ? val : "<null>");
		printf("Got:      \"%s\"\n", logval ? logval : "<null>");
	}

	if (logval)
		free(logval);

	return result;
}

int main()
{
	/* very verbose... don't verify this */
	for (int i=0; i<1024; i++)
		assert(compare_line(i, NULL));

	mlog("logtest\n");
	verify(compare_line(0, "logtest\n"));
	verify(compare_line(1, NULL));

	mlog("char %c", 'A');
	verify(compare_line(1, "char A"));
	verify(compare_line(2, NULL));

	mlog("int %d", 1000000);
	verify(compare_line(2, "int 1000000"));
	verify(compare_line(3, NULL));

	mlog("long %ld", 1000000);
	verify(compare_line(3, "long 1000000"));
	verify(compare_line(4, NULL));

	mlog("ptr %p", 0x1234);
	verify(compare_line(4, "ptr 0x1234"));
	verify(compare_line(5, NULL));

	/* very verbose... don't verify this */
	mlog_clear();
	for (int i = 0; i < 1000 + 256; i++)
		assert(compare_line(i, NULL));

	/* overflow the buffer */
	for (int i = 0; i < 1000 + 256; i++)
		mlog("%d", i);
	verify(compare_line(0, "1000"));
	verify(compare_line(1, "1001"));
	verify(compare_line(254, "1254"));
	verify(compare_line(255, "1255"));
	verify(compare_line(256, NULL));

	/* very verbose final check */
	for (int i=0; i<256; i++) {
		char check[10];
		sprintf(check, "%d", i+1000);
		assert(compare_line(i, check));
	}

	/* nice logging doesn't do anything (because the buffer is in overflow
	 * already)
	 */
	mlog_nice("This message won't be logged");
	verify(compare_line(0, "1000"));
	verify(compare_line(255, "1255"));

	/* check that mlog_nice() does not overflow */
	mlog_clear();
	for (int i = 0; i < 1000 + 256; i++)
		mlog_nice("%d", i);
	verify(compare_line(0, "0"));
	verify(compare_line(1, "1"));
	verify(compare_line(254, "254"));
	verify(compare_line(255, "255"));
	verify(compare_line(256, NULL));

	return 0;
}
