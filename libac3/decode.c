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
#include <sys/time.h>
#include "ac3.h"
#include "ac3_internal.h"
#include "bitstream.h"
#include "imdct.h"
#include "exponent.h"
#include "coeff.h"
#include "bit_allocate.h"
#include "parse.h"
#include "crc.h"
#include "stats.h"
#include "rematrix.h"
#include "sanity_check.h"
#include "downmix.h"
#include "debug.h"

//our global config structure
ac3_config_t ac3_config;

static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;
static uint_32 frame_count = 0;
static uint_32 done_banner;
static ac3_frame_t frame;
//the floating point samples for one audblk
static stream_samples_t samples;
//the integer samples for the entire frame (with enough space for 2 ch out)
static sint_16 output_samples[2 * 6 * 256];

void decode_find_sync(void)
{
	uint_16 sync_word;
	uint_32 i = 0;

	sync_word = bitstream_get(16);

	/* Make sure we sync'ed */
	while(1)
	{
		if(sync_word == 0x0b77)
			break;
		sync_word = sync_word * 2;
		sync_word |= bitstream_get(1);
		i++;
	}
	dprintf("(sync) %d bits skipped to synchronize\n",i);
	dprintf("(sync) begin frame %d\n",frame_count);

	bitstream_set_total_bits(16);
}

void
ac3_init(ac3_config_t *config)
{
	memcpy(&ac3_config,config,sizeof(ac3_config_t));
	bitstream_init(config->fill_buffer_callback);
	imdct_init();
	sanity_check_init(&syncinfo,&bsi,&audblk);
}

ac3_frame_t*
ac3_decode_frame(void)
{
	uint_32 i;

	decode_find_sync();
	dprintf("(decode) begin frame %d\n",frame_count++);

	parse_syncinfo(&syncinfo);

	parse_bsi(&bsi);

	if(!done_banner)
	{
		stats_print_banner(&syncinfo,&bsi);
		done_banner = 1;
	}

	for(i=0; i < 6; i++)
	{

		// Extract most of the audblk info from the bitstream
		// (minus the mantissas 
		parse_audblk(&bsi,&audblk);
		sanity_check(&syncinfo,&bsi,&audblk);

		// Take the differential exponent data and turn it into
		// absolute exponents 
		exponent_unpack(&bsi,&audblk); 
		sanity_check(&syncinfo,&bsi,&audblk);

		// Figure out how many bits per mantissa 
		bit_allocate(syncinfo.fscod,&bsi,&audblk);
		sanity_check(&syncinfo,&bsi,&audblk);

		// Extract the mantissas from the stream and
		// generate floating point frequency coefficients
		coeff_unpack(&bsi,&audblk,samples);
		sanity_check(&syncinfo,&bsi,&audblk);

		if(bsi.acmod == 0x2)
			rematrix(&audblk,samples);

		// Convert the frequency samples into time samples 
		imdct(&bsi,&audblk,samples);
		sanity_check(&syncinfo,&bsi,&audblk);

		// Downmix into the requested number of channels
		// and convert floating point to sint_16
		downmix(&bsi,samples,&output_samples[i * 2 * 256]);
		memset(samples,0,sizeof(stream_samples_t));
	}
	parse_auxdata(&syncinfo);

	frame.audio_data = output_samples;
	return &frame;	

}



