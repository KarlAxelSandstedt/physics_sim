#ifndef __MEM_UTILS_H__
#define __MEM_UTILS_H__

#include "mg_common.h"

#ifdef memcmp
#undef memcmp
#endif

i32 mem_cmp(const void *m1, const void *m2, const u64 len); /* return 1 on EQUALITY, 0 otherwise*/

#endif
