#ifndef __MG_DEBUG_H__
#define __MG_DEBUG_H__

/* Should be included at top of file */

#define MG_DEBUG	/* general debugging, asserts ... */
#define MGL_DEBUG	/* graphics debugging */

#ifdef MG_DEBUG
	#include <stdio.h>
	#include <assert.h>
#else
	#define NDEBUG
#endif

#endif
