/*
 *
 *  output_wav.c
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
 *  This file is based on output_linux.c by Aaron Holtzman.
 *  All .wav modifications were done by Jorgen Lundman <lundman@lundman.net>
 *  Any .wav bugs and errors should be reported to him.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


#include "ac3.h"
#include "decode.h"
#include "debug.h"
#include "output.h"
#include "downmix.h"
#include "ring_buffer.h"

#include "wav.h"

#define BUFFER_SIZE 1024 

static char output_file[] = "output.wav";
static int fd;

static struct wave_header wave;

/*
 * open the file for writing to
 */
int output_open(int bits, int rate, int channels)
{

  /*  int tmp;*/
  
  /*
   * Open the output file
   */

  fd=open(output_file,O_WRONLY | O_TRUNC | O_CREAT, 0644);
 
  if(fd < 0) 
    {
      dprintf("%s: Opening audio output %s\n",
	      strerror(errno), output_file);
      goto ERR;
    }
  dprintf("Opened audio output \"%s\"\n",output_file);

  /* Write out a ZEROD wave header first */
  memset(&wave, 0, sizeof(wave));

  /* Store information */
  wave.common.wChannels = channels;
  wave.common.wBitsPerSample = bits;
  wave.common.dwSamplesPerSec = rate;

  if (write(fd, &wave, sizeof(wave)) != sizeof(wave)) {
    dprintf("failed to write wav-header: %s\n", strerror(errno));
    goto ERR;
  }

  return 1;
  
 ERR:
  if(fd >= 0) { close(fd); }
  return 0;
}


/*
 * play the sample to the already opened file descriptor
 */
void output_play(bsi_t *bsi,stream_samples_t *samples)
{
  int i;
	float *left,*right;
	float norm = 1.0;
	float left_tmp = 0.0;
	float right_tmp = 0.0;
	sint_16 *out_buf;
  
  if(fd < 0)
    return;
  
	//Downmix if necessary 
	downmix(bsi,samples);

	//Determine a normalization constant if the signal exceeds 
	//100% digital [-1.0,1.0]
	//
	//perhaps use the dynamic range info to do this instead
	for(i=0; i< 256;i++)
	{
    left_tmp = samples->channel[0][i];
    right_tmp = samples->channel[1][i];

		if(left_tmp > norm)
			norm = left_tmp;
		if(left_tmp < -norm)
			norm = -left_tmp;

		if(right_tmp > norm)
			norm = right_tmp;
		if(right_tmp < -norm)
			norm = -right_tmp; 
	}
	norm = 32000.0/norm;

	/* Take the floating point audio data and convert it into
	 * 16 bit signed PCM data */
	left = samples->channel[0];
	right = samples->channel[1];

	for(i=0; i < 256; i++)
	{
		out_buf[i * 2 ]    = (sint_16) (*left++  * norm);
		out_buf[i * 2 + 1] = (sint_16) (*right++ * norm);
	}

	write(fd, out_buf,BUFFER_SIZE);

}


void
output_close(void)
{
  off_t offset;

  /* Find how long our file is in total, including header */
  offset = lseek(fd, 0, SEEK_CUR);

  if (offset < 0) 
	{
    dprintf("lseek failed - wav-header is corrupt\n");
    goto ERR;
  }

  /* Rewind file */
  if (lseek(fd, 0, SEEK_SET) < 0) 
	{
    dprintf("rewind failed - wav-header is corrupt\n");
    goto ERR;
  }


  /* Fill out our wav-header with some information. */
  offset -= 8;

  strcpy(wave.riff.id, "RIFF");
  wave.riff.len = offset + 24;
  strcpy(wave.riff.wave_id, "WAVE");
  offset -= 4;

  offset -= 8;
  strcpy(wave.format.id, "fmt ");
  wave.format.len = sizeof(struct common_struct);

  wave.common.wFormatTag = WAVE_FORMAT_PCM;
  wave.common.dwAvgBytesPerSec = 
    wave.common.wChannels * wave.common.dwSamplesPerSec *
    (wave.common.wBitsPerSample >> 4);

  wave.common.wBlockAlign = wave.common.wChannels * 
    (wave.common.wBitsPerSample >> 4);

  strcpy(wave.data.id, "data");

  offset -= sizeof(struct common_struct);
  wave.data.len = offset;

  offset -= 8;
  strcpy(wave.riffdata.id, "RIFF");
  wave.riffdata.len = offset;
  strcpy(wave.riffdata.wave_id, "WAVE");
  offset -= 4;

  offset -= 8;
  strcpy(wave.dataformat.id, "DATA");
  wave.dataformat.len = offset;

  if (write(fd, &wave, sizeof(wave)) < sizeof(wave)) 
	{
    dprintf("wav-header write failed -- file is corrupt\n");
    goto ERR;
  }


 ERR:
  close(fd);
}

