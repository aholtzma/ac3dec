/*
 *
 *  output_linux.c
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
#  include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#if defined(__OpenBSD__)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>


#include "ac3.h"
#include "decode.h"
#include "debug.h"
#include "output.h"
#include "downmix.h"
#include "ring_buffer.h"


#define BUFFER_SIZE 1024 

static char dev[] = "/dev/dsp";
static int fd;
static sint_16 out_buf[BUFFER_SIZE];

/*
 * open the audio device for writing to
 */
int output_open(int bits, int rate, int channels)
{
  int tmp;
  
  /*
   * Open the device driver
   */

	fd=open(dev,O_WRONLY);
  if(fd < 0) 
  {
    dprintf("%s: Opening audio device %s\n",
        strerror(errno), dev);
    goto ERR;
  }
	dprintf("Opened audio device \"%s\"\n",dev);

	tmp = BUFFER_SIZE;
  ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&tmp);

  tmp = bits;
  ioctl(fd,SNDCTL_DSP_SAMPLESIZE,&tmp);

  tmp = channels == 2 ? 1 : 0;
  ioctl(fd,SNDCTL_DSP_STEREO,&tmp);

  tmp = rate;
  ioctl(fd,SNDCTL_DSP_SPEED, &tmp);

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
	float norm;
	float left_tmp = 0.0;
	float right_tmp = 0.0;

	if(fd < 0)
		return;

	//Downmix if necessary 
	downmix(bsi,samples);

	//Determine a normalization constant if the signal exceeds 
	//100% digital [-1.0,1.0]
	//
	//perhaps use the dynamic range info to do this instead
	
	norm = 1.0;

	for(i=0; i< 256;i++)
	{
    left_tmp = samples->channel[0][i];
    right_tmp = samples->channel[1][i];

		if(left_tmp > norm)
			norm = left_tmp;
		else if(left_tmp < -norm)
			norm = -left_tmp;

		if(right_tmp > norm)
			norm = right_tmp;
		else if(right_tmp < -norm)
			norm = -right_tmp; 
	}
	norm = 32766.0f/norm;

	/* Take the floating point audio data and convert it into
	 * 16 bit signed PCM data */
	left = samples->channel[0];
	right = samples->channel[1];

	for(i=0; i < 256; i++)
	{
		out_buf[i * 2 ]    = (sint_16) (*left++  * norm);
		out_buf[i * 2 + 1] = (sint_16) (*right++ * norm);

	}

	write(fd,out_buf,BUFFER_SIZE);
}


void
output_close(void)
{
	close(fd);
}
