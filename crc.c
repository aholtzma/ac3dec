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

static uint_16 state;

void
crc_init(void)
{
	state = 0;
}

void
crc_process(uint_32 data)
{
	uint_32 shift_reg;
	uint_32 num_bits;

	num_bits = 16;

	shift_reg = state;

	while(num_bits)
	{
		shift_reg <<= 1;

		if((shift_reg >> 16) ^ (data >> 31))
			shift_reg = (shift_reg ^ 0x8005);

		shift_reg &= 0xffff;

		data <<= 1;
		num_bits--;
	}

	state = shift_reg;
}
#if 0
void
crc_process(uint_32 data, uint_32 num_bits)
{
	uint_32 shift_reg;

	data <<= 32 - num_bits;

	shift_reg = state;

	while(num_bits)
	{
		shift_reg <<= 1;

		if((shift_reg >> 16) ^ (data >> 31))
			shift_reg = (shift_reg ^ 0x8005);

		shift_reg &= 0xffff;

		data <<= 1;
		num_bits--;
	}

	state = shift_reg;
}
#endif

int
crc_validate(void)
{
	return(state  == 0);
}
