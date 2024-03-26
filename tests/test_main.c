#include <stdio.h>

#include "test_public.h"

static void run_suite(struct suite *suite, struct test_environment *env, const u64 verbose)
{
	if (verbose) { fprintf(stdout, ":::::::::: Running suite %s ::::::::::\n", suite->id); }

	u64 success_count = 0;
	for (u64 i = 0; i < suite->test_count; ++i)
	{
		struct arena record_1 = *env->mem_1;
		struct arena record_2 = *env->mem_2;
		struct arena record_3 = *env->mem_3;
		struct arena record_4 = *env->mem_4;
		struct arena record_5 = *env->mem_5;
		struct arena record_6 = *env->mem_6;

		struct test_output out = suite->tests[i](env);
		if (out.success)
		{
			success_count += 1;
		}
		else if (verbose)
		{
			fprintf(stdout, "\ttest %s failed: %s:%lu\n", out.id, out.file, out.line);
		}

		*env->mem_1 = record_1;
		*env->mem_2 = record_2;
		*env->mem_3 = record_3;
		*env->mem_4 = record_4;
		*env->mem_5 = record_5;
		*env->mem_6 = record_6;
	}
	
	if (verbose) { fprintf(stdout, "\ttests passed: (%lu/%lu)\n",  success_count, suite->test_count); }
}

int main(int argc, char *argv[])
{
	struct arena global_memory = arena_alloc(1024*1024);
	struct arena mem_1 = arena_alloc(1024*1024);
	struct arena mem_2 = arena_alloc(1024*1024);
	struct arena mem_3 = arena_alloc(1024*1024);
	struct arena mem_4 = arena_alloc(1024*1024);
	struct arena mem_5 = arena_alloc(1024*1024);
	struct arena mem_6 = arena_alloc(1024*1024);

	struct test_environment env = 
	{
		.mem_1 = &mem_1,
		.mem_2 = &mem_2,
		.mem_3 = &mem_3,
		.mem_4 = &mem_4,
		.mem_5 = &mem_5,
		.mem_6 = &mem_6,
		.seed = 2984395893,
	};

	run_suite(array_list_suite, &env, 1);
	run_suite(math_suite, &env, 1);
	
	return 0;
}
