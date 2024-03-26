#include <stdio.h>

#include "test_common.h"
#include "array_list.h"

static struct test_output array_list_slot_size(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };
	struct array_list *list;

	list = array_list_new(env->mem_1, 4, 1);
	TEST_EQUAL(list->data_size, 1);
	TEST_EQUAL(list->slot_size, 8);

	list = array_list_new(env->mem_1, 4, 2);
	TEST_EQUAL(list->slot_size, 8);

	list = array_list_new(env->mem_1, 4, 4);
	TEST_EQUAL(list->slot_size, 8);

	list = array_list_new(env->mem_1, 4, 8);
	TEST_EQUAL(list->slot_size, 8);

	list = array_list_new(env->mem_1, 4, 12);
	TEST_EQUAL(list->slot_size, 12);

	TEST_EQUAL(list->length, 4);
	TEST_EQUAL(list->count, 0);

	return output;
}

static struct test_output array_list_try_overflow(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };
	struct array_list *list = array_list_new(env->mem_1, 4, 16);
	TEST_NOT_EQUAL(list, NULL);

	struct { u64 a; u64 b; } s;
	assert(sizeof(s) == 16);

	TEST_EQUAL((void *) (list->slot + 0 * 16), array_list_reserve(list));
	TEST_EQUAL((void *) (list->slot + 1 * 16), array_list_reserve(list));
	TEST_EQUAL((void *) (list->slot + 2 * 16), array_list_reserve(list));
	TEST_EQUAL((void *) (list->slot + 3 * 16), array_list_reserve(list));
	TEST_EQUAL(NULL, array_list_reserve(list));
	TEST_EQUAL(NULL, array_list_add(list, &s));

	return output;
}

static struct test_output array_list_add_remove_add(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };
	struct array_list *list = array_list_new(env->mem_1, 4, 16);

	array_list_reserve(list);
	TEST_EQUAL(list->max_count, 1);
	array_list_remove_index(list, 0);
	TEST_EQUAL((void *) (list->slot + 0 * 16), array_list_reserve(list));
	TEST_EQUAL(list->max_count, 1);

	array_list_reserve(list);
	array_list_reserve(list);
	TEST_EQUAL(list->max_count, 3);
	TEST_EQUAL(list->count, 3);

	array_list_remove_index(list, 0);
	TEST_EQUAL((void *) (list->slot + 0 * 16), array_list_reserve(list));
	TEST_EQUAL(list->max_count, 3);

	array_list_remove_index(list, 1);
	TEST_EQUAL((void *) (list->slot + 1 * 16), array_list_reserve(list));
	TEST_EQUAL(list->max_count, 3);

	array_list_remove_index(list, 2);
	TEST_EQUAL((void *) (list->slot + 2 * 16), array_list_reserve(list));
	TEST_EQUAL(list->max_count, 3);

	TEST_EQUAL((void *) (list->slot + 3 * 16), array_list_reserve(list));
	TEST_EQUAL(list->max_count, 4);

	array_list_remove(list, (list->slot + 0 * 16));
	TEST_EQUAL((void *) (list->slot + 0 * 16), array_list_reserve(list));

	array_list_remove(list, (list->slot + 1 * 16));
	TEST_EQUAL((void *) (list->slot + 1 * 16), array_list_reserve(list));

	array_list_remove(list, (list->slot + 2 * 16));
	TEST_EQUAL((void *) (list->slot + 2 * 16), array_list_reserve(list));

	array_list_remove(list, (list->slot + 3 * 16));
	TEST_EQUAL((void *) (list->slot + 3 * 16), array_list_reserve(list));

	return output;
}

static struct test_output gen_array_list_try_overflow(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };
	struct gen_array_list *list = gen_array_list_new(env->mem_1, 4, 16);
	TEST_NOT_EQUAL(list, NULL);

	u64 gen;
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	TEST_EQUAL(list->generation[0], 0);
	TEST_EQUAL(list->generation[1], 0);
	TEST_EQUAL(list->generation[2], 0);
	TEST_EQUAL(list->generation[3], 0);
	
	gen_array_list_remove_index(list, 0);
	gen_array_list_remove_index(list, 1);
	gen_array_list_remove_index(list, 2);
	gen_array_list_remove_index(list, 3);
	TEST_EQUAL(list->generation[0], 1);
	TEST_EQUAL(list->generation[1], 1);
	TEST_EQUAL(list->generation[2], 1);
	TEST_EQUAL(list->generation[3], 1);

	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	TEST_EQUAL(list->generation[0], 1);
	TEST_EQUAL(list->generation[1], 1);
	TEST_EQUAL(list->generation[2], 1);
	TEST_EQUAL(list->generation[3], 1);
	
	void *addr = gen_array_list_reserve(&gen, list);
	TEST_EQUAL(addr, NULL);
	TEST_EQUAL(list->generation[0], 1);
	TEST_EQUAL(list->generation[1], 1);
	TEST_EQUAL(list->generation[2], 1);
	TEST_EQUAL(list->generation[3], 1);

	return output;
}

static struct test_output gen_array_list_add_remove_add(struct test_environment *env)
{
	struct test_output output = { .success = 1, .id = __func__ };
	struct gen_array_list *list = gen_array_list_new(env->mem_1, 4, 16);
	TEST_NOT_EQUAL(list, NULL);

	u64 gen;
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	gen_array_list_reserve(&gen, list);
	TEST_EQUAL(list->generation[0], 0);
	TEST_EQUAL(list->generation[1], 0);
	TEST_EQUAL(list->generation[2], 0);
	
	gen_array_list_remove_index(list, 0);
	TEST_EQUAL(list->generation[0], 1);
	gen_array_list_remove_index(list, 1);
	TEST_EQUAL(list->generation[1], 1);
	gen_array_list_remove_index(list, 2);
	TEST_EQUAL(list->generation[2], 1);
	
	gen_array_list_remove_index(list, 0);
	TEST_EQUAL(list->generation[0], 2);
	gen_array_list_reserve(&gen, list);
	gen_array_list_remove_index(list, 0);
	TEST_EQUAL(list->generation[0], 3);
	TEST_EQUAL(list->generation[1], 1);
	TEST_EQUAL(list->generation[2], 1);

	gen_array_list_reserve(&gen, list);

	return output;
}

static struct test_output (*array_list_tests[])(struct test_environment *) =
{
	array_list_slot_size,
	array_list_try_overflow,
	array_list_add_remove_add,	
	gen_array_list_try_overflow,
	gen_array_list_add_remove_add,	
};

struct suite m_array_list_suite =
{
	.id = "array_list",
	.tests = array_list_tests,
	.test_count = sizeof(array_list_tests) / sizeof(array_list_tests[0]),
};

struct suite *array_list_suite = &m_array_list_suite;


