#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include "mg_common.h"

#define BITSTREAM_OUT_OF_BOUNDS -1
#define BITSTREAM_SUCCESS 0
#define BITSTREAM_FALSE 0
#define BITSTREAM_TRUE 1

struct bitstream
{
	u8 *data;
	u64 length;	/* length in bits */
	u64 offset;	/* offset in bits */
};

struct bitstream bs_at(void *data, u64 length, u64 offset);
i32 bs_can_read_byte_count(const struct bitstream *stream, const u64 byte_count);
i32 bs_can_read_bit_count(const struct bitstream *stream, const u64 bit_count);
void bs_advance_bit_count(struct bitstream *stream, const u64 bit_count);
void bs_advance_byte_count(struct bitstream *stream, const u64 byte_count);

void bs_read(void *buf, const u64 bytes, struct bitstream *stream);
void bs_read_reverse(void *buf, const u64 bytes, struct bitstream *stream);	/* writes bytes in reverse order as they are being read */

u8 bs_read_u8(struct bitstream *stream, const u8 bit_count);
u16 bs_read_u16_le(struct bitstream *stream, const u8 bit_count);
u16 bs_read_u16_be(struct bitstream *stream, const u8 bit_count);
u32 bs_read_u32_le(struct bitstream *stream, const u8 bit_count);
u32 bs_read_u32_be(struct bitstream *stream, const u8 bit_count);
u64 bs_read_u64_le(struct bitstream *stream, const u8 bit_count);
u64 bs_read_u64_be(struct bitstream *stream, const u8 bit_count);

i8 bs_read_i8(struct bitstream *stream, const u8 bit_count);
i16 bs_read_i16_le(struct bitstream *stream, const u8 bit_count);
i16 bs_read_i16_be(struct bitstream *stream, const u8 bit_count);
i32 bs_read_i32_le(struct bitstream *stream, const u8 bit_count);
i32 bs_read_i32_be(struct bitstream *stream, const u8 bit_count);
i64 bs_read_i64_le(struct bitstream *stream, const u8 bit_count);
i64 bs_read_i64_be(struct bitstream *stream, const u8 bit_count);

#endif
