/* 
 *    verify_ac3.c
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
#include "parse.h"
#include "crc.h"
#include "sys/time.h"
#include "debug.h"

static void decode_find_sync(bitstream_t *bs);
static void decode_print_banner(void);

static bsi_t bsi;
static syncinfo_t syncinfo;
static uint_32 frame_count = 0;


int main(int argc,char *argv[])
{
	int i;
	bitstream_t *bs;
	FILE *in_file;
	int done_banner = 0;
	int bits_in_frame;


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

		bits_in_frame = (syncinfo.frame_size * 16)  - bs->total_bits_read;

		for(i=0; i < bits_in_frame ; i++)
			bitstream_get(bs,1);

		if(!crc_validate())
			printf("(crc) CRC check failed on frame %d\n",frame_count);
		else
			dprintf("(crc) CRC check passed\n");


		if(bitstream_done(bs))
			break;
	}

	printf("end of stream\n");
	bitstream_close(bs);
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
	dprintf("(sync) %d bits skipped to synchronize\n",i);
	dprintf("(sync) begin frame %d\n",frame_count);
	frame_count++;

	bs->total_bits_read = 16;
	crc_init();
}


void decode_print_banner(void)
{
	printf(PACKAGE"-"VERSION" (C) 1999 Aaron Holtzman (aholtzma@engr.uvic.ca)\n");
	printf("Verifying Stream\n");

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
