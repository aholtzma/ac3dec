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

void imdct_do_512(float x[],float y[]);
void imdct_do_256(float x[],float y[]);

void main(int argc,char *argv[])
{
	i_buf[80] = 1.0;
	i_buf[40] = 0.8;
	i_buf[20] = 0.3;
	imdct_init();
	timing_init();

	timing_test_2(imdct_do_512,i_buf,o_buf,"imdct_do_512");
	timing_test_2(imdct_do_256,i_buf,o_buf,"imdct_do_256");
}
