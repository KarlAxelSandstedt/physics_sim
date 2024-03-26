#include "mg_common.h"
#include "thread.h"

#if __OS__ == __LINUX__
#include <stdlib.h>
#include <pthread.h>
#include "unix_public.h"

struct thread thread_default(void *(*start)(void *), void *args)
{
	thread thr;
	pthread_attr_t attr;
	i32 status;
	if ((status = pthread_attr_init(&attr)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
	}

	if ((status = pthread_create(&thr.p_thread, &attr, start, args)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
	}

	if ((status = pthread_attr_destroy(&attr)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
	}


	return thr;
}

void thread_exit(void *retval)
{
	pthread_exit(retval);
}

void thread_join(thread *thr, void **retval)
{
	i32 status = pthread_join(thr->p_thread, retval);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

mutex mutex_default(void)
{
	mutex mtx;
	pthread_mutexattr_t attr;

	i32 status;
	if ((status = pthread_mutexattr_init(&attr)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
#ifdef MUTEX_DEBUG
	if ((status = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
#endif
	if ((status = pthread_mutex_init(&mtx.p_mutex, &attr)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}

	if ((status = pthread_mutexattr_destroy(&attr)) != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}

	return mtx;
}

void mutex_destroy(mutex *mtx)
{
	i32 status = pthread_mutex_destroy(&mtx->p_mutex);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

void mutex_lock(mutex *mtx)
{
	i32 status = pthread_mutex_lock(&mtx->p_mutex);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

void mutex_unlock(mutex *mtx)
{
	i32 status = pthread_mutex_unlock(&mtx->p_mutex);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

struct mutex_condition mutex_condition_default(void)
{
	struct mutex_condition cnd;
	i32 status = pthread_cond_init(&cnd.p_condition, NULL);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}

	return cnd;
}

void mutex_condition_destroy(mutex_condition *cnd)
{
	i32 status = pthread_cond_destroy(&cnd->p_condition);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

void mutex_condition_signal(mutex_condition *cnd)
{
	i32 status = pthread_cond_signal(&cnd->p_condition);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

void mutex_condition_broadcast(mutex_condition *cnd)
{
	i32 status = pthread_cond_broadcast(&cnd->p_condition);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

void mutex_condition_wait(mutex *mtx, mutex_condition *cnd)
{
	i32 status = pthread_cond_wait(&cnd->p_condition, &mtx->p_mutex);
	if (status != 0)
	{
		print_error(stderr, status, __FILE__, __LINE__);
		exit(1);
	}
}

#elif __OS__ == __WIN64__

struct thread thread_default(void *(*start)(void *), void *args)
{
	thread thr;

	return thr;
}

void thread_exit(void *retval)
{
}

void thread_join(thread *thr, void **retval)
{
}

mutex mutex_default(void)
{
	mutex mtx;

	return mtx;
}

void mutex_destroy(mutex *mtx)
{
}

void mutex_lock(mutex *mtx)
{
}

void mutex_unlock(mutex *mtx)
{
}

struct mutex_condition mutex_condition_default(void)
{
	struct mutex_condition cnd;
	
	return cnd;
}

void mutex_condition_destroy(mutex_condition *cnd)
{
}

void mutex_condition_signal(mutex_condition *cnd)
{
}

void mutex_condition_broadcast(mutex_condition *cnd)
{
}

void mutex_condition_wait(mutex *mtx, mutex_condition *cnd)
{
}

#endif
