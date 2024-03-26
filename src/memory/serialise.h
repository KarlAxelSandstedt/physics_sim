#ifndef __SERIALISE_H__
#define __SERIALISE_H__

#include "mg_common.h"

#define SERIAL_BUFFER_OVERFLOW	-1
#define SERIAL_INVALID_FORMAT	-2

union arg_f32
{
	i32 storage;
	f32 f;
};
#define ARG_F32(f) (((union arg_f32) (f)).storage)

/**
 * mg_logger_write_message() - serialise according to the given string from the input string and the following arguments and write to the buf. Returns the written length.
 * Format:
 * 	bi	- i8
 * 	si	- i16
 * 	i	- i32
 * 	I 	- i64
 * 	bu	- u8
 * 	su	- u16
 * 	u	- u32
 * 	U 	- u64
 * 	f	- f32
 * 	F	- F64
 * 	[...]	- start on array, format is [u:(len), str:(data)] where str is any sequence of types.
 * Note: 
 * 	As variadic arguments will promote themselves (or self-promote), dangers arise when the function expects 64 bit values and the user gives 32 bit values or less.
 * 	The smaller values such as char, unsigned int, floats, will be promoted to some type (perhaps self-promotions) and generate storage for that type. Thus, formatting
 * 	chars or perhaps ints as %lli will result in undefined behaviour as the function will grab 64 bits from a 32 bit storage, generating garbage. Explicit casting removes this error.
 */
i32 serial_write(void *buf, const i32 len, i8 *format, ...);
u8 serial_read_u8(void *buf, i32 *offset);
u16 serial_read_u16(void *buf, i32 *offset);
u32 serial_read_u32(void *buf, i32 *offset);
u64 serial_read_u64(void *buf, i32 *offset);
i8 serial_read_i8(void *buf, i32 *offset);
i16 serial_read_i16(void *buf, i32 *offset);
i32 serial_read_i32(void *buf, i32 *offset);
i64 serial_read_i64(void *buf, i32 *offset);
f32 serial_read_f32(void *buf, i32 *offset);
f64 serial_read_f64(void *buf, i32 *offset);

#endif
