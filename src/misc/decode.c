#include "decode.h"


#define UTF8_BAD_SEQUENCE	-1
#define DECODER_RETURN_ON_OVERFLOW(dc, readsize, err_code) 								\
if (!bs_can_read_byte_count(&(dc)->stream, readsize))								\
{														\
	fprintf(stderr, "Error %s:%d - About to Buffer-Overflow decoder, exceeding dump by %luBits\n",		\
		       	__FILE__, __LINE__, 8*(readsize) - (dc)->stream.length - (dc)->stream.offset);		\
	return err_code;										\
}

u32 read_utf8(struct decoder *dc)
{
	DECODER_RETURN_ON_OVERFLOW(dc, sizeof(u8), UTF8_BAD_SEQUENCE);
	const u8 b = bs_read_u8(&dc->stream, 8);

	u32 decoded = 0;
	u64 byte_count;
	if      ((b & 0x80) == 0x00) { byte_count = 1; decoded |= (u32) (b & 0x7f); }
	else if ((b & 0xe0) == 0xc0) { byte_count = 2; decoded |= (u32) (b & 0x1f) << 6; }
	else if ((b & 0xf0) == 0xe0) { byte_count = 3; decoded |= (u32) (b & 0x0f) << 12; }
	else if ((b & 0xf8) == 0xf0) { byte_count = 4; decoded |= (u32) (b & 0x07) << 18; }
	else
	{
		fprintf(stderr, "Error%s:%d - Invalid utf8 sequence\n", __FILE__, __LINE__);
		return UTF8_BAD_SEQUENCE;
	}

	DECODER_RETURN_ON_OVERFLOW(dc, byte_count-1, UTF8_BAD_SEQUENCE);
	for (u32 i = 0; i < byte_count-1; ++i)
	{
		decoded |= (u32) (bs_read_u8(&dc->stream, 8) & 0x3f) << (6*i);	
	}

	return decoded;
}

u32 *decode_utf8(struct arena *frame, u8 *utf8, const u32 utf8_len, u32 *str_len)
{
	*str_len = 0;
	u32 *str = (u32 *)frame->stack_ptr;
	struct decoder dc =
	{
		.stream = bs_at(utf8, 8*utf8_len, 0),
	};

	while (dc.stream.offset < dc.stream.length)
	{
		str[*str_len] = read_utf8(&dc);
		*str_len += 1;
	}

	arena_push(frame, NULL, *str_len*sizeof(u32));
	return str;
}
