/* 
 *  timing.c
 *
 *	Aaron Holtzman - May 1999
 *
 */


#define HAVE_TSC


#include <stdio.h>
#include <sys/time.h>
#include "ac3.h"
#include "timing.h"


#if (!defined(HAVE_TSC) && !defined(HAVE_HRTIME))

uint_64 (*get_time)(void);
uint_64 timing_init(void) { }
double timing_once_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3) { }
void timing_test_2(void (*func)(void*,void*),void *arg_1,void *arg_2,char name[]) {}
void timing_test_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3,char name[]){}

#else

#	ifdef HAVE_TSC
/* Read Pentium timestamp counter. */
inline uint_64
get_time()
{
	long long d;

	__asm__ __volatile__ ("rdtsc" : "=&A" (d));

	return d;
}

uint_64
timing_init(void)
{
	uint_64 start,end,elapsed;	
	long i;
	long iters = 1000;
	uint_64 mean = 0;

	for (i = 0; i < iters; i++)
	{
		start = get_time();
		end = get_time();

		if(i>0)
		{
			elapsed = end - start ;
			mean += elapsed;
		}

	}
	
	mean /= iters;

	return mean;
}


#	else

uint_64 (*get_time)(void);

void
timing_init(void)
{
	hrtime_t start;

	start = gethrvtime();

	//Use the virtual microstate counter if it's available
	if(start == 0LL)
		get_time = gethrtime;
	else
		get_time = gethrvtime;

}
#	endif


void
timing_test_2(void (*func)(void*,void*),void *arg_1,void *arg_2,char name[])
{
	uint_64 start, end;
	uint_64 start_i, end_i;
	int i, iters = 10;

	printf("\nTiming %s 10 times\n",name);
	start = get_time();
	for (i = 0; i < iters; i++)
	{
		start_i = get_time();
		func(arg_1,arg_2);
		end_i = get_time();
		printf("Iteration %d - %lld nsec\n",i,end_i - start_i);
	}
	end = get_time();

	printf("Avg %s time = %lld nsec\n", name,(end - start) / iters);
}


void
timing_test_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3,char name[])
{
	uint_64 start, end;
	uint_64 start_i, end_i;
	int i, iters = 10;

	printf("\nTiming %s 10 times\n",name);
	start = get_time();
	for (i = 0; i < iters; i++)
	{
		start_i = get_time();
		func(arg_1,arg_2,arg_3);
		end_i = get_time();
		printf("Iteration %d - %lld nsec\n",i,end_i - start_i);
	}
	end = get_time();

	printf("Avg %s time = %lld nsec\n", name,(end - start) / iters);
}

double timing_once_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3)
{
	uint_64 start, end;

	start = get_time();
		func(arg_1,arg_2,arg_3);
	end = get_time();

	return (end - start);
}

#endif
