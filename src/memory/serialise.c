#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "serialise.h"

#define SERIAL_RETURN_ON_OVERFLOW(len, offset, readsize)									    \
	if ((len) < (offset) + (readsize))											    \
	{													    \
		fprintf(stderr, "Error %s:%d - About to Buffer-Overflow in serial writing, exceeding buffer by %iBits\n", \
		       	__FILE__, __LINE__,  (offset) + (readsize) - (len));		    					\
		return SERIAL_BUFFER_OVERFLOW;									    \
	}



enum serial_token
{
	TOKEN_BYTE,
	TOKEN_SHORT,
	TOKEN_SIGNED,
	TOKEN_SIGNED64,
	TOKEN_UNSIGNED,
	TOKEN_UNSIGNED64,
	TOKEN_ARRAY_BEGIN,
	TOKEN_ARRAY_END,
	TOKEN_FLOAT,
	TOKEN_FLOAT64,
	TOKEN_INVALID,
	TOKEN_END,
	TOKEN_COUNT,
};

static u8 token_map[] = 
{
	TOKEN_FLOAT64, TOKEN_INVALID, TOKEN_INVALID, TOKEN_SIGNED64, TOKEN_INVALID,
       	TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID,
	TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, 
	TOKEN_UNSIGNED64, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID,
	TOKEN_INVALID, TOKEN_ARRAY_BEGIN, TOKEN_INVALID, TOKEN_ARRAY_END, TOKEN_INVALID,
       	TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_BYTE, TOKEN_INVALID,
	TOKEN_INVALID, TOKEN_INVALID, TOKEN_FLOAT, TOKEN_INVALID, TOKEN_INVALID,
       	TOKEN_SIGNED, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID,
	TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID, TOKEN_INVALID,
       	TOKEN_SHORT, TOKEN_INVALID, TOKEN_UNSIGNED,
};

enum serial_token get_token(i8 **ptr)
{
	i8 c = **ptr;
	*ptr += 1;
	if (70 <= c && c <= 117)
	{
		return token_map[c-70];
	}
	else if (c == '\0')
	{
		return TOKEN_END;
	}
	else
	{
		return TOKEN_INVALID;
	}
}

i32 serial_write(void *buf, const i32 len, i8 *format, ... )
{
	i32 offset = 0;
	va_list args;
	va_start(args, format);
	{
		i8 *ptr = format;
		enum serial_token token;
		while ((token = get_token(&ptr)) != TOKEN_END)
		{
			switch (token)
			{
				case TOKEN_BYTE:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 1);
					token = get_token(&ptr);
					if (token == TOKEN_UNSIGNED)
					{
						*(u8 *)((u8 *) buf + offset) = (u8) va_arg(args, u32);
					}
					else if (token == TOKEN_SIGNED)
					{
						*(i8 *)((u8 *) buf + offset) = (i8) va_arg(args, i32);
					}
					else
					{
						fprintf(stderr, "Error %s:%d - Bad formating\n", __FILE__, __LINE__); 
						return SERIAL_INVALID_FORMAT;
					}
					offset += 1;
				} break;

				case TOKEN_SHORT:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 2);
					token = get_token(&ptr);
					if (token == TOKEN_UNSIGNED)
					{
						*(u16 *)((u8 *) buf + offset) = (u16) va_arg(args, u32);
					}
					else if (token == TOKEN_SIGNED)
					{
						*(i16 *)((u8 *) buf + offset) = (i16) va_arg(args, i32);
					}
					else
					{
						fprintf(stderr, "Error %s:%d - Bad formating\n", __FILE__, __LINE__); 
						return SERIAL_INVALID_FORMAT;
					}
					offset += 2;
				} break;

				case TOKEN_UNSIGNED:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 4);
					*(u32 *)((u8 *) buf + offset) = (u32) va_arg(args, u32);
					offset += 4;
				} break;

				case TOKEN_UNSIGNED64:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 8);
					*(u64 *)((u8 *) buf + offset) = (u64) va_arg(args, u64);
					offset += 8;
				} break;

				case TOKEN_SIGNED:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 4);
					*(i32 *)((u8 *) buf + offset) = (i32) va_arg(args, i32);
					offset += 4;
				} break;

				case TOKEN_SIGNED64:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 8);
					*(i64 *)((u8 *) buf + offset) = (i64) va_arg(args, i64);
					offset += 8;
				} break;

				case TOKEN_FLOAT:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 4);
					//*(f32 *)((u8 *) buf + offset) = ((union arg_f32) va_arg(args, i32)).f;
					*(f32 *)((u8 *) buf + offset) = (f32) va_arg(args, i32);
					offset += 4;
				} break;

				case TOKEN_FLOAT64:
				{
					SERIAL_RETURN_ON_OVERFLOW(len, offset, 8);
					*(f64 *)((u8 *) buf + offset) = (f64) va_arg(args, f64);
					offset += 8;
				} break;

				case TOKEN_ARRAY_BEGIN:
				{
					assert(0 && "To be implemented!");
				} break;

				case TOKEN_ARRAY_END:
				{
					assert(0 && "To be implemented!");
				} break;

				case TOKEN_INVALID:
				{
					fprintf(stderr, "Error %s:%d - Bad formating\n", __FILE__, __LINE__); 
					return SERIAL_INVALID_FORMAT;
				} break;

				default:
				{
					fprintf(stderr, "Error %s:%d - Bad formating\n", __FILE__, __LINE__); 
					return SERIAL_INVALID_FORMAT;
				} break;
			}
		}
	}
	va_end(args);
	return offset;
}

u8 serial_read_u8(void *buf, i32 *offset)
{
	const u8 res = *(u8 *)((u8 *) buf + *offset);
	*offset += 1;
	return res;
}

u16 serial_read_u16(void *buf, i32 *offset)
{
	const u16 res = *(u16 *)((u8 *) buf + *offset);
	*offset += 2;
	return res;
}

u32 serial_read_u32(void *buf, i32 *offset)
{
	const u32 res = *(u32 *)((u8 *) buf + *offset);
	*offset += 4;
	return res;
}

u64 serial_read_u64(void *buf, i32 *offset)
{
	const u64 res = *(u64 *)((u8 *) buf + *offset);
	*offset += 8;
	return res;
}

i8 serial_read_i8(void *buf, i32 *offset)
{
	const i8 res = *(i8 *)((u8 *) buf + *offset);
	*offset += 1;
	return res;
}

i16 serial_read_i16(void *buf, i32 *offset)
{
	const i16 res = *(i16 *)((u8 *) buf + *offset);
	*offset += 2;
	return res;
}

i32 serial_read_i32(void *buf, i32 *offset)
{
	const i32 res = *(i32 *)((u8 *) buf + *offset);
	*offset += 4;
	return res;
}

i64 serial_read_i64(void *buf, i32 *offset)
{
	const i64 res = *(i64 *)((u8 *) buf + *offset);
	*offset += 8;
	return res;
}

f32 serial_read_f32(void *buf, i32 *offset)
{
	const f32 res = *(f32 *)((u8 *) buf + *offset);
	*offset += 4;
	return res;
}

f64 serial_read_f64(void *buf, i32 *offset)
{
	const f64 res = *(f64 *)((u8 *) buf + *offset);
	*offset += 8;
	return res;
}
