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

	timing_test_3(convert_to_float,rand() % 24 ,rand(),&foo,"convert_to_float");

}
