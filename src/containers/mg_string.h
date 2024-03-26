#ifndef __MG_STRING_H__
#define __MG_STRING_H__

#include <stdlib.h>
#include <stdint.h>

#include "mg_common.h"
#include "mg_mempool.h"

/* MOSSY GROTTO SOFTWARE STRING IMPLEMENTATION */

typedef struct mg_string {
	uint64_t len;			/* Length of string, not including EOF */
	char* str;				/* string must end with EOF */
} mg_string;

mg_string	mg_string_empty(void); /* Return nonallocated string "", 0 length */
mg_string	mg_string_duplicate_c_string(const uint64_t len, const char *str);
void		mg_string_free(mg_string* str);
void		mg_string_append(mg_string* str1, const mg_string* str2);
mg_string	mg_string_concatenate(const mg_string* str1, const mg_string* str2);
int32_t		mg_string_compare(const mg_string* str1, const mg_string* str2);	/* 1 is identical, 0 otherwise */
int32_t		mg_string_compare_substring(const mg_string* str, const uint64_t start, const mg_string* sub_str); /* 1 if str + start == sub_str , 0 otherwise*/
int32_t		mg_string_hash(const mg_string* str);
uint64_t	mg_string_find_substring(const mg_string* str, const mg_string* substring);				/* Return index at start of first occuring substring in string, UINT64_MAX if not found */
uint64_t	mg_string_find_substring_last(const mg_string* str, const mg_string* substring);		/* Return index at start of last occuring substring in string, UINT64_MAX if not found */
mg_string	mg_string_substring(const mg_string* str, const uint64_t start, const uint64_t end);	/* Return substring starting and ending at given indices, empty string if failure */
mg_string	mg_string_substring_alias(const mg_string* str, const uint64_t start, const uint64_t end); /* Return a substring aliasing the original string */
mg_string	mg_string_tail(const mg_string* str, const mg_string* substring); /* Allocate and store string indentical to the smallest tail of (str) containing substring (Think of extensions .png, ...) (empty if fail!) */
mg_string	mg_string_tail_alias(const mg_string* str, const mg_string* substring);
/**
 * Format:
 * 	%s   	- String
 * 	%i	- Int32
 * 	%u	- Uint32
 * 	%lli 	- Int64
 * 	%llu 	- Uint64
 * 	%f(d)  	- Real64 (double), optionally replace (d) with number of decimals
 * 	%p	- Pointer
 * 	%x(.)	- Read anything that comes after x (such as p) as hexadecimal.
 * Note: 
 * 	As variadic arguments will promote themselves (or self-promote), dangers arise when the function expects 64 bit values and the user gives 32 bit values or less.
 * 	The smaller values such as char, unsigned int, floats, will be promoted to some type (perhaps self-promotions) and generate storage for that type. Thus, formatting
 * 	chars or perhaps ints as %lli will result in undefined behaviour as the function will grab 64 bits from a 32 bit storage, generating garbage. Explicit casting removes this error.
 */

mg_string	mg_string_format(char *buf, const size_t buf_size, const char *format, ... );


union bit64 {
	uint64_t u;
	int64_t	i;
};

union bit32 {
	uint32_t u;
	int32_t i;
};

mg_string	mg_string_from_double(const double d, uint64_t num_decimals, char *buf, const size_t buf_size);	/* NOTE: Only works for doubles within +-INT64_MAX range */
mg_string	mg_string_from_float(const float f,   uint64_t num_decimals, char *buf, const size_t buf_size);	/* NOTE: Only works for doubles within +-INT64_MAX range */
mg_string	mg_string_from_uint(const uint64_t u, char *buf, const size_t buf_size);
mg_string	mg_string_from_int(int64_t i,   char *buf, const size_t buf_size);
mg_string 	mg_string_from_hexadecimal_64(const union bit64 bits, char *buf, const size_t buf_size);
mg_string	mg_string_from_hexadecimal_32(const union bit32 bits, char *buf, const size_t buf_size);

mg_string	arena_push_mg_string(struct arena* stack, const char* str);									/* Push mg_string data, return empty mg_string on failure */
#endif
