/* 
 *    dither.c
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
#include "dither.h"

static uint_32 lfsr_state = 1;

/* 
 * Generate eight bits of pseudo-entropy using a 16 bit linear
 * feedback shift register (LFSR). The primitive polynomial used
 * is 1 + x^4 + x^14 + x^16.
 *
 * The distribution is uniform, over the range [-0.707,0.707]
 *
 */
uint_16 dither_gen(void)
{
	int i;
	uint_32 state;

	//explicitly bring the state into a local var as gcc > 3.0?
	//doesn't know how to optimize out the stores
	state = lfsr_state;

	//Generate eight pseudo random bits
	for(i=0;i<8;i++)
	{
		state <<= 1;	

		if(state & 0x10000)
			state ^= 0xa011;
	}

	lfsr_state = state;

	return (((((sint_32)state<<8)>>8) * (sint_32) (0.707106 * 256.0))>>16);
}

