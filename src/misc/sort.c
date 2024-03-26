#include "sort.h"
#include "string.h"

static void mergesort_internal(void *array, void *tmp, const u64 len, const size_t element_size, i32 (*compare)(const void *, const void *))
{
	const u64 len_left = len / 2;
	const u64 len_right = len - len_left;
	const size_t right_offset = len_left * element_size; 

	if (len_left > 1) {
		mergesort_internal(array, tmp, len_left, element_size, compare);
		mergesort_internal(((u8 *) array) + right_offset, ((u8 *) tmp) + right_offset, len_right, element_size, compare);
	} else if (len_right > 1) {
		mergesort_internal(((u8 *) array) + right_offset, ((u8 *) tmp) + right_offset, len_right, element_size, compare);
	}

	//Merge
	u64 il = 0;
	u64 ir = 0;
	size_t merge_left_offset = 0;
	size_t merge_right_offset = right_offset;
	size_t tmp_offset = 0;
	while (il < len_left && ir < len_right) {
		const i32 result = compare(((u8 *) array) + merge_left_offset, ((u8 *) array) + merge_right_offset);
		if (result < 0) {
			memcpy(((u8 *) tmp) + tmp_offset, ((u8 *) array) + merge_left_offset, element_size); 
			merge_left_offset += element_size;
			il += 1;
		} else {
			memcpy(((u8 *) tmp) + tmp_offset, ((u8 *) array) + merge_right_offset, element_size); 
			merge_right_offset += element_size;
			ir += 1;
		}
		tmp_offset += element_size;
	}

	if (il < len_left) {
		memcpy(((u8 *) tmp) + tmp_offset, ((u8 *) array) + merge_left_offset, (len_left - il) * element_size); 
	} else {
		memcpy(((u8 *) tmp) + tmp_offset, ((u8 *) array) + merge_right_offset, (len_right - ir) * element_size); 
	}

	memcpy(array, tmp, len * element_size);
}

void mergesort(struct arena *mem, void *array, const u64 len, const size_t element_size, i32 (*compare)(const void *, const void *))
{
	u8 *tmp;
	const size_t size = element_size * len;
	if (mem != NULL) {
		tmp = arena_push(mem, NULL, size);	
		mergesort_internal(array, tmp, len, element_size, compare);
		arena_pop(mem, size);
	} else {
		tmp = malloc(size);
		mergesort_internal(array, tmp, len, element_size, compare);
		free(tmp);
	}
}
