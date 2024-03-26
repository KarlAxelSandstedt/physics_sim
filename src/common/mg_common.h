#ifndef __MG_COMMON_H__
#define __MG_COMMON_H__

/*** system definitions ***/
#define __X11__		0
#define __WAYLAND__	1

#define __WIN64__	2
#define __LINUX__	3

#define __GCC__ 	4
#define __MSVC__ 	5


/* CHOOSE: system graphics api */

/* OS api */
#if defined(__linux__)
#define __OS__ __LINUX__
#define __GAPI__ __X11__
#elif defined(_WIN64)
#define __OS__ __WIN64__
#define __GAPI__ __WIN64__ 
#endif

#if defined(__GNUC__)
#define __COMPILER__ == __GNUC__
#endif

#include "mg_types.h"
#include "mg_debug.h"

#define MEMCPY_AND_OFFSET(buf, data, data_size, offset_addr)		\
	memcpy((u8 *) (buf) + *(offset_addr), (data), (data_size));	\
	*(offset_addr) += (data_size);

#define GET_ADDRESS_AND_OFFSET(buf, addr_type, offset, offset_addr)	\
	(addr_type)((u8 *) (buf) + *(offset_addr));			\
	*(offset_addr) += offset;

#endif

