#include "bitstream.h"

struct bitstream bs_at(void *data, u64 length, u64 offset)
{
	assert(offset < length);

	struct bitstream stream =
	{
		.data = (u8 *) data,
		.length = length,
		.offset = offset,
	};

	return stream;
}

void bs_advance_bit_count(struct bitstream *stream, const u64 bit_count)
{
	stream->offset += bit_count;
}

void bs_advance_byte_count(struct bitstream *stream, const u64 byte_count)
{
	stream->offset += 8*byte_count;
}

void bs_read_reverse(void *buf, const u64 bytes, struct bitstream *stream)
{
	assert(stream->offset + bytes*8 <= stream->length);

	u8 *write_ptr = buf;
	u8 *read_ptr = stream->data + stream->offset / 8;

	const u8 shift_l = stream->offset % 8;
	const u8 shift_r = 8 - shift_l;
	const u8 mask_1 = 0xff >> shift_l;
	const u8 mask_2 = ~mask_1;

	if (shift_l != 0)
	{
		for (u64 i = 0; i < bytes; ++i)
		{
			write_ptr[bytes-i-1] = (read_ptr[i] & mask_1) << shift_l | (read_ptr[i+1] & mask_2) >> shift_r;
		}
	}
	else
	{
		for (u64 i = 0; i < bytes; ++i)
		{
			write_ptr[bytes-i-1] = read_ptr[i] & mask_1;
		}
	}

	stream->offset += bytes*8;
}

void bs_read(void *buf, const u64 bytes, struct bitstream *stream)
{
	assert(stream->offset + bytes*8 <= stream->length);

	u8 *write_ptr = buf;
	u8 *read_ptr = stream->data + stream->offset / 8;

	const u8 shift_l = stream->offset % 8;
	const u8 shift_r = 8 - shift_l;
	const u8 mask_1 = 0xff >> shift_l;
	const u8 mask_2 = ~mask_1;

	if (shift_l != 0)
	{
		for (u64 i = 0; i < bytes; ++i)
		{
			write_ptr[i] = (read_ptr[i] & mask_1) << shift_l | (read_ptr[i+1] & mask_2) >> shift_r;
		}
	}
	else
	{
		for (u64 i = 0; i < bytes; ++i)
		{
			write_ptr[i] = read_ptr[i] & mask_1;
		}
	}

	stream->offset += bytes*8;
}

i32 bs_can_read_byte_count(const struct bitstream *stream, const u64 byte_count)
{
	return (stream->offset + byte_count*8 <= stream->length) ? BITSTREAM_TRUE : BITSTREAM_FALSE;
}

i32 bs_can_read_bit_count(const struct bitstream *stream, const u64 bit_count)
{
	return (stream->offset + bit_count <= stream->length) ? BITSTREAM_TRUE : BITSTREAM_FALSE;
}

u8 bs_read_u8(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 8);

	const u8 shift = stream->offset % 8;
	const u8 mask_size = 8 - shift;
	const u8 *ptr = stream->data + stream->offset / 8;
	u8 mask = 0xff >> shift;

	stream->offset += bit_count;
	return (bit_count <= mask_size) 
		? (ptr[0] & mask) >> (mask_size - bit_count) 
		: (ptr[0] & mask) << (bit_count - mask_size) | (ptr[1] & (~mask)) >> (8 - bit_count + mask_size);
}

u16 bs_read_u16_le(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 16);

	const u8 mod = stream->offset % 8;
	const u8 mask_size = 8 - mod;
	const u8 *ptr = stream->data + stream->offset / 8;
	u8 mask = 0xff >> mod;
	u16 val = 0;
	u8 shift = 0;

	if (mod)
	{
		u8 bits_left = 0;
		if (bit_count >= 8)
		{
			val = (u16) (ptr[0] & mask) << mod | (u16) (ptr[1] & (~mask)) >> (8-mod);
			shift = 8;
		  	bits_left = bit_count - 8;
		}
		else
		{
		  	bits_left = bit_count;
		}

		if (bits_left > 0)
		{
			if (bits_left <= mask_size)
			{
				val |= (u16) ((ptr[1] & mask) >> (mask_size - bits_left)) << shift;
			}
			else
			{
				val |= (u16) ((ptr[1] & mask) << (shift + bits_left - mask_size)
				     | (u16) ((ptr[2] & (~mask)) >> (8 - bits_left + mask_size))) << shift;
			}
		}
	}
	else
	{
		if (bit_count >= 8)
		{
			val = (u16) ptr[0];
			shift = 8;
		}

		const u8 bits_left = bit_count - 8;
		if (bits_left > 0)
		{
			val |= (u16) ptr[1] >> (8 - bits_left) << shift;
		}

	}

	stream->offset += bit_count;
	return val;
}

u16 bs_read_u16_be(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 16);

	const u8 mod = stream->offset % 8;
	const u8 *ptr = stream->data + stream->offset / 8;
	u8 mask = 0xff >> mod;
	u16 val = 0;

	val = (u16) ptr[0] & mask;
	i8 bits_left = bit_count - (8-mod);
	if (bits_left)
	{
		const u8 shift = 8 - (bits_left % 8);
		mask = 0xff << shift;
		if (bits_left <= 8)
		{
			val = val << bits_left | (u16) ptr[1] >> (8-bits_left);
		}
		else
		{
			val = val << bits_left | (u16) ptr[1] << (bits_left-8) | (u16) (ptr[2] & mask) >> (16-bits_left);
		}
	}
	else
	{
		val >>= -bits_left;
	}

	stream->offset += bit_count;
	return val;
}

u32 bs_read_u32_le(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 32);

	const u8 mod = stream->offset % 8;
	const u8 *ptr = stream->data + stream->offset / 8;
	const u8 whole_bytes = bit_count / 8;
	u32 val = 0;
	u8 shift = 0;

	if (mod)
	{
		const u8 mask_1 = 0xff >> mod;
		const u8 mask_2 = ~mask_1;
		const u8 mask_1_size = 8 - mod;
		for (u32 i = 0; i < whole_bytes; ++i)
		{
			val |= ((u32) (ptr[i] & mask_1) << mod | (u32) (ptr[i+1] & mask_2) >> mask_1_size) << shift;
			shift += 8;
		}

		const u8 bits_left = bit_count - shift;
		if (bits_left)
		{
			if (bits_left <= mask_1_size)
			{
				val |= ((u32) (ptr[whole_bytes] & mask_1) >> (mask_1_size - bits_left)) << shift;
			}
			else
			{
				val |= ((u32) (ptr[whole_bytes] & mask_1) << (bits_left - mask_1_size)
				     | (u32) (ptr[whole_bytes+1] & mask_2) >> (8 - bits_left + mask_1_size)) << shift;
			}
		}
	}
	else
	{
		for (u32 i = 0; i < whole_bytes; ++i)
		{
			val |= (u32) ptr[i] << shift;
			shift += 8;
		}

		const u8 bits_left = bit_count - shift;
		if (bits_left)
		{
			val |= ((u32) (ptr[whole_bytes] & 0xff) >> (8-bits_left)) << shift;	
		}
	}
	
	stream->offset += bit_count;
	return val;
}

u32 bs_read_u32_be(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 32);

	const u8 shift = stream->offset % 8;
	const u8 *ptr = stream->data + stream->offset / 8;
	u32 val = 0;

	const u8 whole_bytes = bit_count / 8;
	const u8 bits_left = bit_count - 8*whole_bytes;
	if (shift)
	{
		const u8 mask_1 = 0xff >> shift;
		const u8 mask_2 = ~mask_1;
		for (u8 i = 0; i < whole_bytes; ++i)
		{
			val = val << 8 | (u32) (ptr[i] & mask_1) << shift |  (u32) (ptr[i+1] & mask_2) >> (8-shift);
		}

		if (bits_left)
		{
			const u8 mask_1_size = (8-shift);
			if (bits_left <= mask_1_size)
			{
				val = val << bits_left | (u32) (ptr[whole_bytes] & mask_1) >> (mask_1_size-bits_left);
			}
			else
			{
				val = val << bits_left | (u32) (ptr[whole_bytes] & mask_1) << (bits_left-mask_1_size)
						       | (u32) (ptr[whole_bytes+1] & mask_2) >> (8-bits_left+mask_1_size);
			}
		}
	}
	else
	{
		for (u8 i = 0; i < whole_bytes; ++i)
		{
			val = val << 8 | ptr[i];
		}

		if (bits_left)
		{
			val = val << bits_left | (u32) ptr[whole_bytes] >> (8 - bits_left);
		}
	}

	stream->offset += bit_count;
	return val;
}

u64 bs_read_u64_le(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 64);

	const u8 mod = stream->offset % 8;
	const u8 *ptr = stream->data + stream->offset / 8;
	const u8 whole_bytes = bit_count / 8;
	u64 val = 0;
	u8 shift = 0;

	if (mod)
	{
		const u8 mask_1 = 0xff >> mod;
		const u8 mask_2 = ~mask_1;
		const u8 mask_1_size = 8 - mod;
		for (u64 i = 0; i < whole_bytes; ++i)
		{
			val |= ((u64) (ptr[i] & mask_1) << mod | (u64) (ptr[i+1] & mask_2) >> mask_1_size) << shift;
			shift += 8;
		}

		const u8 bits_left = bit_count - shift;
		if (bits_left)
		{
			if (bits_left <= mask_1_size)
			{
				val |= ((u64) (ptr[whole_bytes] & mask_1) >> (mask_1_size - bits_left)) << shift;
			}
			else
			{
				val |= ((u64) (ptr[whole_bytes] & mask_1) << (bits_left - mask_1_size)
				     | (u64) (ptr[whole_bytes+1] & mask_2) >> (8 - bits_left + mask_1_size)) << shift;
			}
		}
	}
	else
	{
		for (u32 i = 0; i < whole_bytes; ++i)
		{
			val |= (u64) ptr[i] << shift;
			shift += 8;
		}

		const u8 bits_left = bit_count - shift;
		if (bits_left)
		{
			val |= ((u64) (ptr[whole_bytes] & 0xff) >> (8-bits_left)) << shift;	
		}
	}
	

	stream->offset += bit_count;
	return val;
}

u64 bs_read_u64_be(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 64);

	const u8 shift = stream->offset % 8;
	const u8 *ptr = stream->data + stream->offset / 8;
	u8 mask_1 = 0xff >> shift;
	u8 mask_2 = ~mask_1;
	u64 val = 0;

	const u8 whole_bytes = bit_count / 8;
	const u8 bits_left = bit_count - 8*whole_bytes;
	if (shift)
	{
		for (u8 i = 0; i < whole_bytes; ++i)
		{
			val = val << 8 | (u64) (ptr[i] & mask_1) << shift | (u64) (ptr[i+1] & mask_2) >> (8-shift);
		}

		if (bits_left)
		{
			const u8 mask_1_size = (8-shift);
			if (bits_left <= mask_1_size)
			{
				val = val << bits_left | (u64) (ptr[whole_bytes] & mask_1) >> (mask_1_size-bits_left);
			}
			else
			{
				val = val << bits_left | (u64) (ptr[whole_bytes] & mask_1) << (bits_left-mask_1_size)
						       | (u64) (ptr[whole_bytes+1] & mask_2) >> (8-bits_left+mask_1_size);
			}
		}
	}
	else
	{
		for (u8 i = 0; i < whole_bytes; ++i)
		{
			val = val << 8 | (u64) (ptr[i] & mask_1);
		}

		if (bits_left)
		{
			val = val << bits_left | (u64) ptr[whole_bytes] >> (8 - bits_left);
		}
	}

	stream->offset += bit_count;
	return val;
}

i8 bs_read_i8(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 8);
	
	union { u8 bits; i8 val; } u;
	u.bits = bs_read_u8(stream, bit_count);
	const u8 sign_mask = 0x01 << (bit_count-1);
	const u8 sign_extension = (u.bits & sign_mask) ? 0xff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}

i16 bs_read_i16_le(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 16);

	union { u16 bits; i16 val; } u;
	u.bits = bs_read_u16_le(stream, bit_count);
	const u16 sign_mask = (u16) 0x01 << (bit_count-1);
	const u16 sign_extension = (u.bits & sign_mask) ? 0xffff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}

i16 bs_read_i16_be(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 16);

	union { u16 bits; i16 val; } u;
	u.bits = bs_read_u16_be(stream, bit_count);
	const u16 sign_mask = (u16) 0x01 << (bit_count-1);
	const u16 sign_extension = (u.bits & sign_mask) ? 0xffff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}

i32 bs_read_i32_le(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 32);

	union { u32 bits; i32 val; } u;
	u.bits = bs_read_u32_le(stream, bit_count);
	const u32 sign_mask = (u32) 0x01 << (bit_count-1);
	const u32 sign_extension = (u.bits & sign_mask) ? 0xffffffff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}

i32 bs_read_i32_be(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 32);

	union { u32 bits; i32 val; } u;
	u.bits = bs_read_u32_be(stream, bit_count);
	const u32 sign_mask = (u32) 0x01 << (bit_count-1);
	const u32 sign_extension = (u.bits & sign_mask) ? 0xffffffff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}

i64 bs_read_i64_le(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 64);

	union { u64 bits; i64 val; } u;
	u.bits = bs_read_u64_le(stream, bit_count);
	const u64 sign_mask = (u64) 0x01 << (bit_count-1);
	const u64 sign_extension = (u.bits & sign_mask) ? 0xffffffffffffffff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}

i64 bs_read_i64_be(struct bitstream *stream, const u8 bit_count)
{
	assert(stream->offset + bit_count <= stream->length);
	assert(0 < bit_count && bit_count <= 64);

	union { u64 bits; i64 val; } u;
	u.bits = bs_read_u64_be(stream, bit_count);
	const u64 sign_mask = (u64) 0x01 << (bit_count-1);
	const u64 sign_extension = (u.bits & sign_mask) ? 0xffffffffffffffff << (bit_count-1) : 0;
	u.bits |= sign_extension;

	return u.val;
}
