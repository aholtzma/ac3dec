/* 
 *  uncouple_timing.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdio.h>
#include "timing.h"
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"

void convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest);

// This test seems broken. I think the duration of convert_to_float is
// too short to give accurate performance info.
void main(int argc,char *argv[])
{
	int i;
	float foo;
	double time;
	double time_acc;

	
	timing_init();

	for(i=0;i < 256; i++)
	{
		time = timing_once_3(convert_to_float,4,rand(),&foo);
		time_acc += time;
		printf("%3d - %6.1f ns  ",i, time);
		if(!(i%4))
			printf("\n");
	}
	printf("\nAverage time - %f ns\n\n",time_acc/256);

	for(i=0;i < 16; i++)
	{
		time = timing_once_3(convert_to_float,4,1 << i,&foo);
		time_acc += time;
		printf("%3d - %6.1f ns \n",i, time);
	}
	printf("\nAverage time - %f ns\n",time_acc/256);
}
