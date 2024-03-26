#ifndef __DECODE_H__
#define __DECODE_H__

#include "mg_common.h"
#include "mg_mempool.h"
#include "bitstream.h"

struct decoder {
	struct bitstream stream;
};

u32 read_utf8(struct decoder *dc);
u32 *decode_utf8(struct arena *frame, u8 *utf8, const u32 utf8_len, u32 *str_len);

#endif
