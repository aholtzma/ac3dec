/* 
 *  bitstream_test.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"
#include "timing.h"

float i_buf[256];
float o_buf[512];
float delay[512];

void imdct_do_512(float x[],float y[],float delay[]);
void imdct_do_256(float x[],float y[],float delay[]);

void main(int argc,char *argv[])
{
	int i;
	float foo;
	double time;
	double time_acc;
	uint_64 start,end,elapsed,correction;	
	uint_32 iters = 1000;
	uint_16 exp;
	uint_16 mant;
	double mean = 0;
	double variance = 0;
	i_buf[80] = 1.0;
	i_buf[40] = 0.8;
	i_buf[20] = 0.3;

	imdct_init();
	correction = timing_init();

	printf("\nCorrection Factor = %lld\n",correction);
	printf("Timing imdct_512 %d times\n",iters);
	for (i = 0; i < iters; i++)
	{
		mant = 	rand();
		exp = rand() % 24;
		start = get_time();
		imdct_do_512(i_buf,o_buf,delay);
		end = get_time();
		//printf("Iteration %d - %lld nsec\n",i,end - start);
		if(i>0)
		{
			elapsed = end - start - correction;
			mean += elapsed;
			variance += elapsed * elapsed;
			
		}

	}
	
	mean /= iters;
	variance /= iters;
	variance -= mean * mean;

	printf("mean = %f\n",mean);
	printf("variance= %f\n",variance);

}
