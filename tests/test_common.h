#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include "mg_common.h"
#include "mg_mempool.h"

#include "test_macro.h"

struct test_output
{
	const char *id;
	const char *file;
	u64 line;
	u64 success;
};

struct test_environment
{
	struct arena *mem_1;
	struct arena *mem_2;
	struct arena *mem_3;
	struct arena *mem_4;
	struct arena *mem_5;
	struct arena *mem_6;
	const u64 seed;
};

struct suite
{
	char *id;
	struct test_output (**tests)(struct test_environment *);
	const u64 test_count;	
};

#endif
