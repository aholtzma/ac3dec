/* 
 *  timing.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdio.h>
#include <sys/time.h>

hrtime_t (*get_time)(void);

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
void
timing_test_2(void (*func)(void*,void*),void *arg_1,void *arg_2,char name[])
{
	hrtime_t start, end;
	hrtime_t start_i, end_i;
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

double timing_once_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3)
{
	hrtime_t start, end;

	start = get_time();
		func(arg_1,arg_2,arg_3);
	end = get_time();

	return (end - start);
}
