#include "mem_utils.h"

i32 mem_cmp(const void *m1, const void *m2, const u64 len) /* return 1 on EQUALITY, 0 otherwise*/
{
	for (u64 i = 0; i < len; ++i)
	{
		if (((u8*)m1)[i] != ((u8*)m2)[i])
		{
			return 0;
		}
	}

	return 1;
}
