/**
  * @file syscalls.c
  * @brief This file implement the libc wrapper functions
  * 
  * @details
  *
  * @copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *
  * @par Revision history
  *
  * @par 1.0.0 : 2020/10/01 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup platform
 * @ingroup bsp
 * @{
 */

/******************************************************************************/

/*!
 * @cond INTERNAL
 * @{
 */

void __init_sys_calls__(void)
{
	return;
}

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/* Libc time wrapper functions */
/******************************************************************************/

#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

/*!
 * @cond INTERNAL
 * @{
 */

extern void BSP_Rtc_Time_ReadMicro(struct timeval * tp);
__attribute__((used)) int
_gettimeofday (struct timeval * tp, void * tzvp)
{
	struct timezone *tzp = (struct timezone *)tzvp;
	if (tp)
	{
		/* Ask the host for the Unix epoch.  */
		BSP_Rtc_Time_ReadMicro(tp);
	}
	/* Return fixed data for the timezone.  */
	if (tzp)
	{
		tzp->tz_minuteswest = 0;
		tzp->tz_dsttime = 0;
	}
	return 0;
}

extern void BSP_Rtc_Time_Write(time_t t);
__attribute__((used)) int
_settimeofday(const struct timeval *tp, const struct timezone *tzp)
{
	(void)tzp;
	if (tp)
	{
		/* Ask the host for the seconds since the Unix epoch.  */
		BSP_Rtc_Time_Write( tp->tv_sec );
	}
	return 0;
}

int _times(struct tms *buf)
{
	return -1;
}

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/* Libc sbrk wrapper functions */
/******************************************************************************/
#include <errno.h>
#include <stdio.h>

/*!
 * @cond INTERNAL
 * @{
 */

extern int errno;
register char * stack_ptr asm("sp");

__attribute__((used)) caddr_t
_sbrk(int incr)
{
	extern char end asm("end");
	extern uint32_t _Min_Heap_Size;
	const uint8_t *max_heap = (uint8_t *)((uint32_t)&end + (uint32_t)&_Min_Heap_Size);

	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0)
		heap_end = &end;

	prev_heap_end = heap_end;
	if (heap_end + incr > (char *)max_heap)
	{
		errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/* Libc wrapper functions */
/******************************************************************************/
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>

/*!
 * @cond INTERNAL
 * @{
 */

char *__env[1] = { 0 };
char **environ = __env;

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

void initialise_monitor_handles()
{
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = __io_getchar();
	}

return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}

int _open(char *path, int flags, ...)
{
	/* Pretend like we always fail */
	return -1;
}

int _close(int file)
{
	return -1;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void _exit (int status)
{
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}

/*!
 * @}
 * @endcond
 */

/*! @} */
