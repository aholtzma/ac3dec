/* 
 *  bitstream_test.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "bitstream.h"

void print_bits(unsigned long data,unsigned long num_bits);
void print_bits(unsigned long data,unsigned long num_bits)
{
	long i;

	for (i=num_bits-1; i >= 0; i--)
	{
		printf("%1d",data & (1 << i) ? 1 : 0);
	}

}
#define BIT_SIZE (10 * 128)
int main(void)
{
	bitstream_t *bs;
	unsigned long num_bits = 0;
	unsigned long data;
	long i = 0;
	FILE *f;


	f = fopen("bitstream_test.dat","r");
	bs = bitstream_open(f);

	while (i < BIT_SIZE)
	{
		if(abs(i - BIT_SIZE) < 32)
			num_bits = abs(i - (BIT_SIZE));
		else
			num_bits = rand() % 32;
		data = bitstream_get(bs,num_bits);
		print_bits(data,num_bits);
		i += num_bits;
	}

	return 0;
}
