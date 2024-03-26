#ifndef __MG_SORT_H__
#define __MG_SORT_H__

#include "mg_common.h"
#include "mg_mempool.h"

/* If mem != NULL, allocate tmp memory using arena, push/pop, sort low to high */
void mergesort(struct arena *mem, void *array, const u64 len, const size_t element_size, i32 (*compare)(const void *, const void *));

#endif
