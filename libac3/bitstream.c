/* 
 *  bitstream.c
 *
 *	Copyright (C) Aaron Holtzman - Dec 1999
 *
 *  This file is part of ac3dec, a free AC-3 audio decoder
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
 */

#include <stdlib.h>
#include <stdio.h>

#include "ac3.h"
#include "ac3_internal.h"
#include "bitstream.h"

uint_32 bits_left;
uint_32 current_word;
uint_8 *buffer_start;
uint_8 *buffer_end;
uint_32 total_bits_read;

void (*bitstream_fill_buffer)(uint_8**,uint_8**);


static inline void
bitstream_fill_current()
{
	//fprintf(stderr,"(fill) buffer_start %p, buffer_end %p, current_word 0x%08x, bits_left %d\n",buffer_start,buffer_end,current_word,bits_left);

	current_word = *((uint_32*)buffer_start)++;
	current_word = swab32(current_word);

	if(buffer_start >= buffer_end)
		bitstream_fill_buffer(&buffer_start,&buffer_end);
}

//
// The fast paths for _get is in the
// bitstream.h header file so it can be inlined.
//
// The "bottom half" of this routine is suffixed _bh
//
// -ah
//

uint_32
bitstream_get_bh(uint_32 num_bits)
{
	uint_32 result;

	num_bits -= bits_left;
	result = (current_word << (32 - bits_left)) >> (32 - bits_left);

	bitstream_fill_current();

	if(num_bits != 0)
		result = (result << num_bits) | (current_word >> (32 - num_bits));
	
	bits_left = 32 - num_bits;

	return result;
}

void 
bitstream_byte_align(void)
{
	//byte align the bitstream
	bits_left = bits_left & ~7;
	if(!bits_left)
	{
		bits_left = 32;
		bitstream_fill_current();
	}
}

void
bitstream_init(void(*fill_function)(uint_8**,uint_8**))
{
	//fprintf(stderr,"(fill_buffer) buffer buffer_start %p, buffer_end %p, current_word 0x%08x, bits_left %d\n",buffer_start,buffer_end,current_word,bits_left);
	// Setup the buffer fill callback 
	bitstream_fill_buffer = fill_function;

	bitstream_fill_buffer(&buffer_start,&buffer_end);

	bits_left = 32;
	current_word = *((uint_32*)buffer_start)++;
	current_word = swab32(current_word);
}

uint_32 bitstream_done()
{
	//FIXME
	return 0;
}


uint_32 bitstream_get_total_bits(void)
{
	return total_bits_read;
}

void bitstream_set_total_bits(uint_32 x)
{
	total_bits_read = x;
}
