#include "mmath.h"

bool is_power_of_two(const int n)
{
	return (n & (n-1)) == 0 && n > 0;
}

uint32_t power_of_two_ceil(uint32_t n)
{
	uint32_t shift = 0;
	while (n >> shift) {
		shift += 1;
	}

	return 1 << shift;
}
