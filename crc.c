/* 
 *    crc.c
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
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "crc.h"

uint_16 state;

static inline void crc_process_bit(uint_32 bit);

void
crc_init(void)
{
	state = 0;
}

static inline void
crc_process_bit(uint_32 bit)
{
	uint_32 xor_val;

	xor_val = (state >> 15) ^ bit;

	state <<= 1;
	
	if(xor_val)
		state = (state ^ 0x8005);
}


void
crc_process(uint_32 data, uint_32 num_bits)
{
	data <<= 32 - num_bits;

	while(num_bits)
	{
		crc_process_bit(data >> 31);

		data <<= 1;
		num_bits--;
	}
}

int
crc_validate(void)
{
	return(state == 0);
}


