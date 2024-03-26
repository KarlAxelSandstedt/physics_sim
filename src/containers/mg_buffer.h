#ifndef __MG_BUFFER_H__
#define __MG_BUFFER_H__

#include "mg_common.h"

struct mg_buffer
{
	u64	size;
	void*	data;
};

#endif
