#ifndef __MG_THREAD_H__
#define __MG_THREAD_H__


/************************************************************************/
/* 				Threads					*/
/************************************************************************/

#include "mg_common.h"

#ifdef MG_DEBUG
#define MUTEX_DEBUG
#endif

#if __OS__ == __LINUX__ 
#include <pthread.h>

typedef struct mutex
{
	pthread_mutex_t p_mutex;
} mutex;

typedef struct mutex_condition
{
	pthread_cond_t p_condition;
} mutex_condition;

typedef struct thread
{
	pthread_t p_thread;
} thread;

#elif __OS__ == __WIN64__

typedef struct mutex
{
	i32 tmp;
} mutex;

typedef struct mutex_condition
{
	i32 tmp;
} mutex_condition;

typedef struct thread
{
	i32 tmp;
} thread;

#endif

struct thread thread_default(void *(*start)(void *), void *args);
void thread_exit(void *retval);
void thread_join(thread *thr, void **retval);

struct mutex mutex_default(void);
void mutex_destroy(mutex *mtx);
void mutex_lock(mutex *mtx);
void mutex_unlock(mutex *mtx);

struct mutex_condition mutex_condition_default(void);
void mutex_condition_destroy(mutex_condition *cnd);
void mutex_condition_signal(mutex_condition *cnd);
void mutex_condition_broadcast(mutex_condition *cnd);
void mutex_condition_wait(mutex *mtx, mutex_condition *cnd);

#endif
