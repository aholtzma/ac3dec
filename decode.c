/* 
 *    decode.c
 *
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "imdct.h"
#include "exponent.h"
#include "mantissa.h"
#include "bit_allocate.h"
#include "uncouple.h"
#include "parse.h"
#include "output.h"
#include "crc.h"
#include "rematrix.h"
#include "sys/time.h"
#include "debug.h"

static void decode_find_sync(bitstream_t *bs);
static void decode_print_banner(void);

static stream_coeffs_t stream_coeffs;
static stream_samples_t stream_samples;
static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;
static uint_32 frame_count = 0;


int main(int argc,char *argv[])
{
	int i;
	bitstream_t *bs;
	FILE *in_file;
	int done_banner = 0;


	/* If we get an argument then use it as a filename... otherwise use
	 * stdin */
	if(argc > 1)
	{
		in_file = fopen(argv[1],"r");	
		if(!in_file)
		{
			fprintf(stderr,"%s - Couldn't open file %s\n",strerror(errno),argv[1]);
			exit(1);
		}
	}
	else
		in_file = stdin;


	bs = bitstream_open(in_file);
	imdct_init();
	decode_sanity_check_init();
	output_open(16,48000,2);

	while(1)
	{
		decode_find_sync(bs);

		parse_syncinfo(&syncinfo,bs);

		parse_bsi(&bsi,bs);

		if(!done_banner)
		{
			decode_print_banner();
			done_banner = 1;
		}

		for(i=0; i < 6; i++)
		{

			/* Extract most of the audblk info from the bitstream
			 * (minus the mantissas */
			parse_audblk(&bsi,&audblk,bs);
			decode_sanity_check();

			/* Take the differential exponent data and turn it into
			 * absolute exponents */
			exponent_unpack(&bsi,&audblk,&stream_coeffs); 
			decode_sanity_check();

			/* Figure out how many bits per mantissa */
			bit_allocate(syncinfo.fscod,&bsi,&audblk);
			decode_sanity_check();

			/* Extract the mantissas from the data stream */
			mantissa_unpack(&bsi,&audblk,bs);
			decode_sanity_check();

			/* Uncouple the coupling channel if it exists and
			 * convert the mantissa and exponents to IEEE floating
			 * point format */
			uncouple(&bsi,&audblk,&stream_coeffs);
			decode_sanity_check();

			if(bsi.acmod == 0x2)
				rematrix(&audblk,&stream_coeffs);
#if 0
			/* Perform dynamic range compensation */
			dynamic_range(&bsi,&audblk,&stream_coeffs); 

#endif

			/* Convert the frequency data into time samples */
			imdct(&bsi,&audblk,&stream_coeffs,&stream_samples);
			decode_sanity_check();

			/* Send the samples to the output device */
			output_play(&stream_samples);
		}
		parse_auxdata(&syncinfo,bs);

		if(!crc_validate())
		{
			dprintf("(crc) CRC check failed\n");
		}
		else
		{
			dprintf("(crc) CRC check passed\n");
		}

		decode_sanity_check();

		if(bitstream_done(bs))
			break;
	}
	
	bitstream_close(bs);
	output_close();
	return 0;
}



static 
void decode_find_sync(bitstream_t *bs)
{
	uint_16 sync_word;
	uint_32 i = 0;

	sync_word = bitstream_get(bs,16);

	/* Make sure we sync'ed */
	while(1)
	{
		if(sync_word == 0x0b77)
			break;
		sync_word = sync_word * 2;
		sync_word |= bitstream_get(bs,1);
		i++;
	}
	dprintf("(sync) %ld bits skipped to synchronize\n",i);
	dprintf("(sync) begin frame %ld\n",frame_count);
	frame_count++;

	bs->total_bits_read = 16;
	crc_init();
}

void decode_sanity_check_init(void)
{
	syncinfo.magic = DECODE_MAGIC_NUMBER;
	bsi.magic = DECODE_MAGIC_NUMBER;
	audblk.magic1 = DECODE_MAGIC_NUMBER;
	audblk.magic2 = DECODE_MAGIC_NUMBER;
	audblk.magic3 = DECODE_MAGIC_NUMBER;
}

void decode_sanity_check(void)
{
	int i;

	if(syncinfo.magic != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- syncinfo magic number **");
	
	if(bsi.magic != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- bsi magic number **");

	if(audblk.magic1 != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- audblk magic number 1 **"); 

	if(audblk.magic2 != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- audblk magic number 2 **"); 

	if(audblk.magic3 != DECODE_MAGIC_NUMBER)
		fprintf(stderr,"\n** Sanity check failed -- audblk magic number 3 **"); 

	for(i = 0;i < 5 ; i++)
	{
		if (audblk.fbw_exp[i][255] !=0 || audblk.fbw_exp[i][254] !=0 || 
				audblk.fbw_exp[i][253] !=0)
			fprintf(stderr,"\n** Sanity check failed -- fbw_exp out of bounds **"); 

		if (audblk.fbw_bap[i][255] !=0 || audblk.fbw_bap[i][254] !=0 || 
				audblk.fbw_bap[i][253] !=0)
			fprintf(stderr,"\n** Sanity check failed -- fbw_bap out of bounds **"); 

		if (audblk.chmant[i][255] !=0 || audblk.chmant[i][254] !=0 || 
				audblk.chmant[i][253] !=0)
			fprintf(stderr,"\n** Sanity check failed -- chmant out of bounds **"); 
	}

	if (audblk.cpl_exp[255] !=0 || audblk.cpl_exp[254] !=0 || 
			audblk.cpl_exp[253] !=0)
		fprintf(stderr,"\n** Sanity check failed -- cpl_exp out of bounds **"); 

	if (audblk.cpl_bap[255] !=0 || audblk.cpl_bap[254] !=0 || 
			audblk.cpl_bap[253] !=0)
		fprintf(stderr,"\n** Sanity check failed -- cpl_bap out of bounds **"); 

	if (audblk.cplmant[255] !=0 || audblk.cplmant[254] !=0 || 
			audblk.cplmant[253] !=0)
		fprintf(stderr,"\n** Sanity check failed -- cpl_mant out of bounds **"); 

	if ((audblk.cplinu == 1) && (audblk.cplbegf > (audblk.cplendf+2)))
		fprintf(stderr,"\n** Sanity check failed -- cpl params inconsistent **"); 

	for(i=0; i < bsi.nfchans; i++)
	{
		if((audblk.chincpl[i] == 0) && (audblk.chbwcod[i] > 60))
			fprintf(stderr,"\n** Sanity check failed -- chbwcod too big **"); 
	}

	return;
}	


void decode_print_banner(void)
{
	printf(PACKAGE"-"VERSION" (C) 1999 Aaron Holtzman (aholtzma@engr.uvic.ca)\n");

	printf("%d.%d Mode ",bsi.nfchans,bsi.lfeon);
	switch (syncinfo.fscod)
	{
		case 2:
			printf("32 KHz   ");
			break;
		case 1:
			printf("44.1 KHz ");
			break;
		case 0:
			printf("48 KHz   ");
			break;
		default:
			printf("Invalid sampling rate ");
			break;
	}
	printf("%4d kbps ",syncinfo.bit_rate);
	switch(bsi.bsmod)
	{
		case 0:
			printf("Complete Main Audio Service");
			break;
		case 1:
			printf("Music and Effects Audio Service");
		case 2:
			printf("Visually Impaired Audio Service");
			break;
		case 3:
			printf("Hearing Impaired Audio Service");
			break;
		case 4:
			printf("Dialogue Audio Service");
			break;
		case 5:
			printf("Commentary Audio Service");
			break;
		case 6:
			printf("Emergency Audio Service");
			break;
		case 7:
			printf("Voice Over Audio Service");
			break;
	}
	printf("\n");
}
