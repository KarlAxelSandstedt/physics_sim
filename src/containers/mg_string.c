#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "mg_string.h"

const char hexadecimal_table[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

mg_string mg_string_empty(void)
{
	mg_string empty = { .len = 0, .str = "" };
	return empty;
}

mg_string mg_string_duplicate_c_string(const uint64_t len, const char* str)
{
	mg_string mg_str = { .len = len };
	if (len > 0) {
		const size_t str_size = (len + 1) * sizeof(char);
		mg_str.str = malloc(str_size);
		memcpy(mg_str.str, str, str_size);
		mg_str.str[len] = '\0';
	} else {
		mg_str.str = "";
	}

	return mg_str;
}

void mg_string_free(mg_string* str)
{
	free(str->str);
}

void mg_string_append(mg_string* str1, const mg_string* str2)
{
	char* new_block;
	if ((new_block = realloc(str1->str, (str1->len + str2->len + 1) * sizeof(char)))) {
		str1->str = new_block;
		memcpy(str1->str + str1->len, str2->str, (str2->len + 1) * sizeof(char));
		str1->len += str2->len;
	} else {
		printf("WARNING: Failed to reallocate in mg_string append method\n");
	}
}

mg_string mg_string_concatenate(const mg_string* str1, const mg_string* str2)
{
	mg_string concat = { .len = str1->len + str2->len, .str = malloc((str1->len + str2->len + 1) * sizeof(char)) };
	memcpy(concat.str, str1->str, str1->len * sizeof(char));
	memcpy(concat.str + str1->len, str2->str, (str2->len + 1) * sizeof(char));

	return concat;
}

int	mg_string_compare(const mg_string* str1, const mg_string* str2)
{
	if (str1->len != str2->len)
		return 0;

	for (uint64_t i = 0; i < str1->len; ++i) {
		if (str1->str[i] != str2->str[i]) {
			return 0;
		}
	}

	return 1;
}

int mg_string_hash(const mg_string* str)
{
	int hash = 0;
	for (uint64_t i = 0; i < str->len; ++i) {
		hash += str->str[i] * (i + 119);
	}
	
	return hash;
}

int	mg_string_compare_substring(const mg_string* str, const uint64_t start, const mg_string* substring)
{
	if (str->len <= start || (str->len - start) < substring->len) {
		return 0;
	}

	for (uint64_t i = 0; i < substring->len; ++i) {
		if (str->str[start + i] != substring->str[i]) {
			return 0;
		}
	}

	return 1;
}

uint64_t mg_string_find_substring(const mg_string* str, const mg_string* substring)
{
	if (str->len < substring->len) {
		return UINT64_MAX;
	}

	const uint64_t num_substrings = str->len - substring->len + 1;
	uint64_t substring_index;
	for (substring_index = 0; substring_index < num_substrings; ++substring_index) {
		int substring_found = 1;
		for (uint64_t i = 0; i < substring->len; ++i) {
			if (str->str[substring_index + i] != substring->str[i]) {
				substring_found = 0;
			}
		}
		if (substring_found) {
			break;
		}
	}

	return (substring_index != num_substrings) ? substring_index : UINT64_MAX;
}

uint64_t mg_string_find_substring_last(const mg_string* str, const mg_string* substring)
{
	if (str->len < substring->len) {
		return UINT64_MAX;
	}

	int substring_found = 0;
	const uint64_t num_substrings = str->len - substring->len + 1;
	uint64_t substring_index;
	for (substring_index = num_substrings - 1; 0 < substring_index; --substring_index) {
		substring_found = 1;
		for (uint64_t i = 0; i < substring->len; ++i) {
			if (str->str[substring_index + i] != substring->str[i]) {
				substring_found = 0;
			}
		}
		if (substring_found) {
			break;
		}
	}

	if (substring_index == 0) {
		substring_found = 1;
		for (uint64_t i = 0; i < substring->len; ++i) {
			if (str->str[substring_index + i] != substring->str[i]) {
				substring_found = 0;
			}
		}
	}

	return (substring_found) ? substring_index : UINT64_MAX;
}

mg_string mg_string_substring(const mg_string* str, const uint64_t start, const uint64_t end)
{
	if (end < start || str->len <= end) {
		mg_string empty = { .len = 0, .str = "" };
		return empty;
	}

	return mg_string_duplicate_c_string(end - start + 1, str->str + start);
}


mg_string mg_string_substring_alias(const mg_string* str, const uint64_t start, const uint64_t end)
{
	if (end < start || str->len <= end) {
		mg_string empty = { .len = 0, .str = "" };
		return empty;
	}

	const mg_string substring = { .len = end - start + 1, .str = str->str + start };
	return substring;
}

mg_string mg_string_tail(const mg_string* str, const mg_string* substring)
{
	mg_string tail;

	if (str->len < substring->len) {
		tail = mg_string_empty();
	} else {
		const uint64_t start = mg_string_find_substring_last(str, substring);
		if (start == UINT64_MAX) {
			tail = mg_string_empty();
		} else {
			tail = mg_string_substring(str, start, str->len - 1);
		}
	}

	return tail;
}

mg_string mg_string_tail_alias(const mg_string* str, const mg_string* substring)
{
	mg_string tail;

	if (str->len < substring->len) {
		tail = mg_string_empty();
	} else {
		const uint64_t start = mg_string_find_substring_last(str, substring);
		if (start == UINT64_MAX) {
			tail = mg_string_empty();
		} else {
			tail = mg_string_substring_alias(str, start, str->len - 1);
		}
	}

	return tail;
}

union d_u {
	double d;
	uint64_t bits;
};

mg_string mg_string_from_double(const double d, uint64_t num_decimals, char *buf, const size_t buf_size)
{
	assert(buf_size > 0);

	union d_u du;
	du.d = d;

	if (num_decimals > 0)
	{
		const int64_t ints = (int64_t) d;	/* Rounding towards 0 hopefully */
		mg_string int_part = mg_string_from_int(ints, buf, buf_size);
		if (int_part.len > 0 && int_part.len + num_decimals + 1 < buf_size)
		{	
			uint64_t dec_mul = 1;
			for(uint64_t i = num_decimals; i; i -= 1, dec_mul *= 10);
			const double absolute_decimals = (1.0 - 2.0 * (du.bits >> 63)) * (d - ints);
			const uint64_t decs = (uint64_t) (absolute_decimals * dec_mul);

			uint64_t num_leading_zeroes = 0;
			double zeroes_mul = 10.0;
			buf[int_part.len] = '.';
			for(uint64_t i = 0; i < num_decimals; ++i, zeroes_mul *= 10.0)
			{
				if (!((uint64_t) (absolute_decimals * zeroes_mul)))
				{
					num_leading_zeroes += 1;
					buf[int_part.len + num_leading_zeroes] = '0';
				}
				else
				{
					break;
				}
			}

			mg_string d_str = { .str = buf };

			if (num_leading_zeroes != num_decimals)
			{
				mg_string dec_part = mg_string_from_uint(decs, buf + int_part.len  + num_leading_zeroes + 1, buf_size - int_part.len - num_leading_zeroes - 1);
				d_str.len = int_part.len + num_leading_zeroes + dec_part.len + 1;
			}
			else
			{
				buf[int_part.len + 2] = '\0';
				d_str.len = int_part.len + 2;
			}
				
			return d_str;
		}
	}
	else
	{
		return mg_string_from_int((int64_t) d, buf, buf_size);
	}

	mg_string empty = { .len = 0, .str = buf };
	buf[0] = '\0';
	return empty;
}

union f_u {
	float f;
	uint32_t bits;
};

mg_string mg_string_from_float(const float f, uint64_t num_decimals, char *buf, const size_t buf_size)
{
	assert(buf_size > 0);

	union f_u fu;
	fu.f = f;

	if (num_decimals > 0)
	{
		const int64_t ints = (int64_t) f;	/* Rounding towards 0 hopefully */
		mg_string int_part = mg_string_from_int(ints, buf, buf_size);
		if (int_part.len > 0 && int_part.len + num_decimals + 1 < buf_size)
		{	
			uint64_t dec_mul = 1;
			for(uint64_t i = num_decimals; i; i -= 1, dec_mul *= 10);
			const float absolute_decimals = (1.0 - 2.0 * (fu.bits >> 31)) * (f - ints);
			const uint64_t decs = (uint64_t) (absolute_decimals * dec_mul);

			uint64_t num_leading_zeroes = 0;
			double zeroes_mul = 10.0;
			buf[int_part.len] = '.';
			for(uint64_t i = 0; i < num_decimals; ++i, zeroes_mul *= 10.0)
			{
				if (!((uint64_t) (absolute_decimals * zeroes_mul)))
				{
					num_leading_zeroes += 1;
					buf[int_part.len + num_leading_zeroes] = '0';
				}
				else
				{
					break;
				}
			}

			mg_string f_str = { .str = buf };

			if (num_leading_zeroes != num_decimals)
			{
				mg_string dec_part = mg_string_from_uint(
						decs, buf + int_part.len + num_leading_zeroes + 1,
						buf_size - int_part.len - num_leading_zeroes - 1);
				f_str.len = int_part.len + num_leading_zeroes + dec_part.len + 1;
			}
			else
			{
				buf[int_part.len + 2] = '\0';
				f_str.len = int_part.len + 2;
			}
				
			return f_str;
		}
	}
	else
	{
		return mg_string_from_int((int64_t) f, buf, buf_size);
	}

	mg_string empty = { .len = 0, .str = buf };
	buf[0] = '\0';
	return empty;
}

mg_string mg_string_from_uint(const uint64_t u, char *buf, const size_t buf_size)
{
	assert(buf_size > 0);

	uint64_t len = 1;
	for (uint64_t div = 10; (u / div) && (len < buf_size-1); div *= 10, len += 1);

	if (len < buf_size)
	{
		int64_t index = len-1;
		uint64_t div = 1;
		for (; index >= 0; index -= 1, div *= 10)
		{ 
			buf[index] = '0' + (u / div) % 10;
		}

	}
	else
	{
		len = 0;
	}

	buf[len] = '\0';
	mg_string u_str = { .len = len, .str = buf };
	return u_str;
}

mg_string mg_string_from_int(int64_t i, char *buf, const size_t buf_size)
{
	assert(buf_size > 0);

	uint64_t len = 1;
	uint8_t neg = 0;
	if (i < 0)
	{
		len += 1;
		neg = 1;
		i *= -1;
	}

	for (uint64_t div = 10; (i / div) && (len < buf_size-1); div *= 10, len += 1);

	if (len < buf_size)
	{
		int64_t number_start = 0;
		if (neg)
		{
			number_start += 1;
			buf[0] = '-';
		}
		for (int64_t index = len-1, div = 1; index >= number_start; --index, div *= 10)
		{ 
			buf[index] = '0' + (i / div) % 10;
		}

	}
	else
	{
		len = 0;
	}

	buf[len] = '\0';
	mg_string i_str = { .len = len, .str = buf };
	return i_str;
}

mg_string mg_string_from_hexadecimal_64(const union bit64 bits, char *buf, const size_t buf_size)
{
	uint64_t len = 18;
	if (len < buf_size)
	{
		buf[0] = '0';
		buf[1] = 'x';
		uint64_t hex_index;
		for (uint64_t i = 0; i < 16; i += 2)
		{
			hex_index = (bits.u >> (i * 4)) & 0xF;
			buf[2 + 15 - i] = hexadecimal_table[hex_index];
			hex_index = (bits.u >> ((i + 1) * 4)) & 0xF;
			buf[2 + 14 - i] = hexadecimal_table[hex_index];
		}
	}
	else
	{
		len = 0;
	}
	buf[len] = '\0';
	mg_string hex_rep = { .len = len, .str = buf };
	return hex_rep;

}

mg_string mg_string_from_hexadecimal_32(const union bit32 bits, char *buf, const size_t buf_size)
{
	uint64_t len = 10;
	if (len < buf_size)
	{
		buf[0] = '0';
		buf[1] = 'x';
		uint32_t hex_index;
		for (uint32_t i = 0; i < 8; i += 2)
		{
			hex_index = (bits.u >> (i * 4)) & 0xF;
			buf[2 + 7 - i] = hexadecimal_table[hex_index];
			hex_index = (bits.u >> ((i + 1) * 4)) & 0xF;
			buf[2 + 6 - i] = hexadecimal_table[hex_index];
		}
	}
	else
	{
		len = 0;
	}
	buf[len] = '\0';
	mg_string hex_rep = { .len = len, .str = buf };
	return hex_rep;
}

enum mg_string_format {
	FORMAT_STRING,
	FORMAT_POINTER,
	FORMAT_UINT32,
	FORMAT_INT32,
	FORMAT_UINT64,
	FORMAT_INT64,
	FORMAT_REAL64,
	FORMAT_COUNT,
	FORMAT_BAD,
};

#define STANDARD    0
#define HEXADECIMAL 1

/* format_len - Length of format string (i.e. length of '%...' of a valid input.
 * extra:
 *	REAL64 - decimals	
 *	Integers and Pointers - hexadecimal
 */
enum mg_string_format mg_string_internal_determine_input(const char *format, uint64_t *format_len, uint64_t *extra)
{
	*format_len = 1;
	*extra = STANDARD;
	if (format[0] == 'x')
	{
		*format_len += 1;
		*extra = HEXADECIMAL;	
	}

	const uint64_t type_index = *extra; 
	switch (format[type_index])
	{
		case 's':
		{
			if (*extra == STANDARD)
			{
				*format_len += 1;
				return FORMAT_STRING;
			}
		} break;

		case 'p':
		{
			*format_len += 1;
			return FORMAT_POINTER;
		}

		case 'i':
		{
			*format_len += 1;
			return FORMAT_INT32;
		} break;

		case 'u':
		{
			*format_len += 1;
			return FORMAT_UINT32;
		} break;

		case 'l':
		{
			if (format[type_index + 1] == 'l')
			{
				if (format[type_index + 2] == 'i')
				{
					*format_len += 3;
					return FORMAT_INT64;
				}
				else if (format[type_index + 2] == 'u')
				{
					*format_len += 3;
					return FORMAT_UINT64;
				}
			}
		} break;

		case 'f':
		{
			if (*extra == STANDARD)
			{
				uint32_t dec_len = 0;
				for (char next = format[type_index + 1]; next != '\0' &&  '0' <= next && next <= '9' ; )
				{
					*extra = *extra * 10 + next - '0';
					dec_len += 1;
					next = format[type_index + dec_len + 1];	
				}

				if (*extra == STANDARD)
				{
					*extra = 4;
				}

				*format_len += 1 + dec_len;
				return FORMAT_REAL64;
			}
		} break;

		default:
		{

		} break;
	}

	return FORMAT_BAD;
}

mg_string mg_string_format(char *buf, const size_t buf_size, const char *format, ... )
{
	va_list args;
	va_start(args, format);

	size_t offset = 0;
	int32_t success = 1;

	for (const char *str = format; success && *str != '\0' && offset < (buf_size-1); )
	{
		switch (*str)
		{
			case '%':
			{
				uint64_t format_len;
				uint64_t extra;
				const enum mg_string_format arg_type = mg_string_internal_determine_input(str + 1, &format_len, &extra);	
				mg_string str_from_arg = mg_string_empty();
				switch (arg_type)
				{
					case FORMAT_STRING:
					{
						char *str_arg = va_arg(args, char *);
						str_from_arg.len = strlen(str_arg);
						str_from_arg.str = str_arg;
						if (str_from_arg.len < buf_size - offset)
						{
							memcpy(buf + offset, str_from_arg.str, str_from_arg.len);
						}
					} break;

					case FORMAT_POINTER:
					{
						if (extra == STANDARD)
						{
							str_from_arg = mg_string_from_uint(va_arg(args, uintptr_t), buf + offset, buf_size - offset);
						}
						else if (extra == HEXADECIMAL)
						{
							const union bit64 bits = { .u = va_arg(args, uintptr_t) };
							str_from_arg = mg_string_from_hexadecimal_64(bits, buf + offset, buf_size - offset);
						}
					} break;

					case FORMAT_INT32:
					{
						if (extra == STANDARD)
						{
							str_from_arg = mg_string_from_int(va_arg(args, int32_t), buf + offset, buf_size - offset);
						}
						else if (extra == HEXADECIMAL)
						{
							
							const union bit32 bits = { .i = va_arg(args, int32_t) };
							str_from_arg = mg_string_from_hexadecimal_32(bits, buf + offset, buf_size - offset);
						}
					} break;

					case FORMAT_UINT32:
					{
						if (extra == STANDARD)
						{
							str_from_arg = mg_string_from_uint(va_arg(args, uint32_t), buf + offset, buf_size - offset);
						}
						else if (extra == HEXADECIMAL)
						{
							const union bit32 bits = { .u = va_arg(args, uint32_t) };
							str_from_arg = mg_string_from_hexadecimal_32(bits, buf + offset, buf_size - offset);

						}
					} break;

					case FORMAT_INT64:
					{
						if (extra == STANDARD)
						{
							str_from_arg = mg_string_from_int(va_arg(args, int64_t), buf + offset, buf_size - offset);
						}
						else if (extra == HEXADECIMAL)
						{
							const union bit64 bits = { .i = va_arg(args, int64_t) };
							str_from_arg = mg_string_from_hexadecimal_64(bits, buf + offset, buf_size - offset);
						}
					} break;

					case FORMAT_UINT64:
					{
						if (extra == STANDARD)
						{
							str_from_arg = mg_string_from_uint(va_arg(args, uint64_t), buf + offset, buf_size - offset);
						}
						else if (extra == HEXADECIMAL)
						{
							const union bit64 bits = { .u = va_arg(args, uint64_t) };
							str_from_arg = mg_string_from_hexadecimal_64(bits, buf + offset, buf_size - offset);
						}
					} break;

					case FORMAT_REAL64:
					{
						str_from_arg = mg_string_from_double(va_arg(args, double), extra, buf + offset, buf_size - offset);
					} break;

					case FORMAT_BAD:
					{
						str_from_arg = mg_string_empty();
					} break;

					default:
					{

					} break;
				}

				/* Failed to fit argument in log message, exit processing and send full buf to logger */
				if (str_from_arg.len == 0)
				{
					success = 0;
					break;
				}
				else
				{
					str += format_len;
					offset += str_from_arg.len;
				}
			} break;

			default:
			{
				buf[offset] = *str;
				offset += 1;
				str += 1;
			} break;
		}
	}
	va_end(args);

	buf[offset] = '\0';
	mg_string mstr = { .len = offset, .str = buf };
	return mstr;
}

mg_string arena_push_mg_string(struct arena* stack, const char* str)
{
	const size_t len = strlen(str);
	u8* arena_str = arena_push(stack, str, len + 1);
	mg_string string;
	if (arena_str) {
		string.len = len;
		string.str = (char *) arena_str;
	} else {
		string = mg_string_empty();
	}

	return string;
}
