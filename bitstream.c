/* 
 *  bitstream.c
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
#include "decode.h"
#include "crc.h"
#include "bitstream.h"


//My vego-matic endian swapping routine
#define SWAP_ENDIAN32(x)  ((((uint_8*)&x)[0] << 24) |  \
                         (((uint_8*)&x)[1] << 16) |  \
                         (((uint_8*)&x)[2] << 8) |   \
                         ((uint_8*)&x)[3])           
#define SWAP_ENDIAN16(x) ((((uint_8*)&x)[0] << 8) |  \
                         ((uint_8*)&x)[1])           


static void bitstream_load(bitstream_t *bs);

/* Fetches 1-32 bits from the file opened in bitstream_open */
uint_16
bitstream_get(bitstream_t *bs,uint_32 num_bits)
{
	uint_16 result;
	uint_32 bits_read;
	uint_32 bits_to_go;


	if(num_bits == 0)
		return 0;

	bits_read = num_bits > bs->bits_left ? bs->bits_left : num_bits; 

	result = bs->current_word  >> (16 - bits_read);
	bs->current_word <<= bits_read;
	bs->bits_left -= bits_read;

	if(bs->bits_left == 0)
		bitstream_load(bs);

	if (bits_read < num_bits)
	{
		bits_to_go = num_bits - bits_read;
		result <<= bits_to_go;
		result |= bs->current_word  >> (16 - bits_to_go);
		bs->current_word <<= bits_to_go;
		bs->bits_left -= bits_to_go;
	}
	
	bs->total_bits_read += num_bits;
	return result;
}

static void
bitstream_load(bitstream_t *bs)
{
	int bytes_read;

	bytes_read = fread(&bs->current_word,1,2,bs->file);
	bs->current_word = SWAP_ENDIAN16(bs->current_word);
	bs->bits_left = bytes_read * 8;
	bs->done = !bytes_read;

	crc_process(bs->current_word);
}

/* Opens a bitstream for use in bitstream_get */
bitstream_t*
bitstream_open(FILE *file)
{
	bitstream_t *bs;
	
	if(!file)
		return 0;

	bs = malloc(sizeof(bitstream_t));
	if(!bs)
		return 0;

	bs->done = 0;

	/* Read in the first 32 bit word and initialize the structure */
	bs->file = file;
	bitstream_load(bs);

	return bs;
}

int bitstream_done(bitstream_t *bs)
{
	return (bs->done);
}

void 
bitstream_close(bitstream_t *bs)
{
	if(bs->file)
		fclose(bs->file);

	free(bs);
}
