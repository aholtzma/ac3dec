/*
 *
 *  downmix.c
 *    
 *	Copyright (C) Aaron Holtzman - Sept 1999
 *
 *	Originally based on code by Yuqing Deng.
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ac3.h"
#include "ac3_internal.h"


#include "decode.h"
#include "downmix.h"
#include "debug.h"


//Pre-scaled downmix coefficients
static float cmixlev_lut[4] = { 0.2928, 0.2468, 0.2071, 0.2468 };
static float smixlev_lut[4] = { 0.2928, 0.2071, 0.0   , 0.2071 };

void 
downmix_normalize(stream_samples_t samples,sint_16 *output_samples)
{
	uint_32 i,j;
	float norm;
	float left_tmp = 0.0;
	float right_tmp = 0.0;
	float *left,*right;

	//Determine a normalization constant if the signal exceeds 
	//100% digital [-1.0,1.0]
	//
	//I wish we didn't have to do this. Maybe I'm doing something
	//wrong in the downmix -ah
	

	norm = 1.0;

	for(i=0; i< 256;i++)
	{
    left_tmp = samples[0][i];
    right_tmp = samples[1][i];

		if(left_tmp > norm)
			norm = left_tmp;
		else if(left_tmp < -norm)
			norm = -left_tmp;

		if(right_tmp > norm)
			norm = right_tmp;
		else if(right_tmp < -norm)
			norm = -right_tmp; 
	}

	if(norm > 100000.0)
	{
		printf("norm = %f\n",norm);
		for(j=0;j<256;j++)
			printf("%f %f\n",samples[0][j],samples[1][j]);
		printf("\n\n");
	}
	norm = 32766.0f/norm;

	/* Take the floating point audio data and convert it into
	 * 16 bit signed PCM data */
	left = samples[0];
	right = samples[1];

	for(i=0; i < 256; i++)
	{
		output_samples[i * 2 ]    = (sint_16) (*left++  * norm);
		output_samples[i * 2 + 1] = (sint_16) (*right++ * norm);
	}
}

/* Downmix into _two_ channels...other downmix modes aren't implemented
 * to reduce complexity. Realistically, there aren't many machines around
 * with > 2 channel output anyways */

void downmix(bsi_t* bsi, stream_samples_t samples,sint_16 *output_samples)
{
	int j;
	float right_tmp;
	float left_tmp;
	float clev,slev;
	float *centre = 0, *left = 0, *right = 0, *left_sur = 0, *right_sur = 0;

	if(bsi->acmod > 7)
		dprintf("(downmix) invalid acmod number\n"); 

#if 1
	//There are two main cases, with or without Dolby Surround
	if(ac3_config.flags & AC3_DOLBY_SURR_ENABLE)
	{
		exit(1);
		switch(bsi->acmod)
		{
			// 3/2
			case 7:
				left      = samples[0];
				centre    = samples[1];
				right     = samples[2];
				left_sur  = samples[3];
				right_sur = samples[4];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.2265f * *left_sur++ + 0.2265f * *right_sur++;
					left_tmp  = -1 * right_tmp;
					right_tmp += 0.3204f * *right++ + 0.2265f * *centre;
					left_tmp  += 0.3204f * *left++  + 0.2265f * *centre++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}

			break;

			// 2/2
			case 6:
				left      = samples[0];
				right     = samples[1];
				left_sur  = samples[2];
				right_sur = samples[3];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.2265f * *left_sur++ + 0.2265f * *right_sur++;
					left_tmp  = -1 * right_tmp;
					right_tmp += 0.3204f * *right++;
					left_tmp  += 0.3204f * *left++ ;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}
			break;

			// 3/1
			case 5:
				left      = samples[0];
				centre    = samples[1];
				right     = samples[2];
				//Mono surround
				right_sur = samples[3];

				for (j = 0; j < 256; j++) 
				{
					right_tmp =  0.2265f * *right_sur++;
					left_tmp  = -1 * right_tmp;
					right_tmp += 0.3204f * *right++ + 0.2265f * *centre;
					left_tmp  += 0.3204f * *left++  + 0.2265f * *centre++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}
			break;

			// 2/1
			case 4:
				left      = samples[0];
				right     = samples[1];
				//Mono surround
				right_sur = samples[2];

				for (j = 0; j < 256; j++) 
				{
					right_tmp =  0.2265f * *right_sur++;
					left_tmp  = -1 * right_tmp;
					right_tmp += 0.3204f * *right++; 
					left_tmp  += 0.3204f * *left++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}
			break;

			// 3/0
			case 3:
				left      = samples[0];
				centre    = samples[1];
				right     = samples[2];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.3204f * *right++ + 0.2265f * *centre;
					left_tmp  = 0.3204f * *left++  + 0.2265f * *centre++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}
			break;

			// 2/0
			case 2:
			//Do nothing!
			break;

			// 1/0
			case 1:
				//Mono program!
				right = samples[0];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					samples[1][j] = right_tmp;
					samples[0][j] = right_tmp;
				}
				
			break;

			// 1+1
			case 0:
				//Dual mono, output selected by user
				right = samples[ac3_config.dual_mono_ch_sel];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					samples[1][j] = right_tmp;
					samples[0][j] = right_tmp;
				}
			break;
		}
	}
	else
	{
		//Non-Dolby surround downmixes
		switch(bsi->acmod)
		{
			// 3/2
			case 7:
				left      = samples[0];
				centre    = samples[1];
				right     = samples[2];
				left_sur  = samples[3];
				right_sur = samples[4];

				clev = cmixlev_lut[bsi->cmixlev];
				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + clev * *centre   + slev * *right_sur++;
					left_tmp = 0.4142f * *left++  + clev * *centre++ + slev * *left_sur++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}

			break;

			// 2/2
			case 6:
				left      = samples[0];
				right     = samples[1];
				left_sur  = samples[2];
				right_sur = samples[3];

				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + slev * *right_sur++;
					left_tmp = 0.4142f * *left++  + slev * *left_sur++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}

			// 3/1
			case 5:
				left      = samples[0];
				centre    = samples[1];
				right     = samples[2];
				//Mono surround
				right_sur = samples[3];

				clev = cmixlev_lut[bsi->cmixlev];
				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + clev * *centre   + slev * *right_sur;
					left_tmp = 0.4142f * *left++  + clev * *centre++ + slev * *right_sur++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}
				
				break;

			// 2/1
			case 4:
				left      = samples[0];
				right     = samples[1];
				//Mono surround
				right_sur = samples[2];

				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + slev * *right_sur;
					left_tmp = 0.4142f * *left++  + slev * *right_sur++;

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}

			// 3/0
			case 3:
				left      = samples[0];
				centre    = samples[1];
				right     = samples[2];

				clev = cmixlev_lut[bsi->cmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + clev * *centre;   
					left_tmp = 0.4142f * *left++  + clev * *centre++; 

					samples[1][j] = right_tmp;
					samples[0][j] = left_tmp;
				}
				
			break;

			case 2:
			//Do nothing!
			break;

			// 1/0
			case 1:
				//Mono program!
				right = samples[0];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					samples[1][j] = right_tmp;
					samples[0][j] = right_tmp;
				}
				
			break;

			// 1+1
			case 0:
				//Dual mono, output selected by user
				right = samples[ac3_config.dual_mono_ch_sel];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					samples[1][j] = right_tmp;
					samples[0][j] = right_tmp;
				}
			break;

		}
	}

#endif
	downmix_normalize(samples,output_samples);
}
