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
#include "decode.h"
#include "downmix.h"
#include "debug.h"


//Pre-scaled downmix coefficients
static float cmixlev_lut[4] = { 0.2928, 0.2468, 0.2071, 0.2468 };
static float smixlev_lut[4] = { 0.2928, 0.2071, 0.0   , 0.2071 };


/* Downmix into _two_ channels...other downmix modes aren't implemented
 * to reduce complexity. Realistically, there aren't many machines around
 * with > 2 channel output anyways */

void downmix(bsi_t* bsi, stream_samples_t* stream_sample)
{
	int j;
	float right_tmp;
	float left_tmp;
	float clev,slev;
	float *centre = 0, *left = 0, *right = 0, *left_sur = 0, *right_sur = 0;

	if(bsi->acmod > 7)
		dprintf("(downmix) invalid acmod number\n"); 

	//There are two main cases, with or without Dolby Surround
	if(global_prefs.use_dolby_surround)
	{
		switch(bsi->acmod)
		{
			// 3/2
			case 7:
				left      = stream_sample->channel[0];
				centre    = stream_sample->channel[1];
				right     = stream_sample->channel[2];
				left_sur  = stream_sample->channel[3];
				right_sur = stream_sample->channel[4];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.2265f * *left_sur++ + 0.2265f * *right_sur++;
					left_tmp  = -1 * right_tmp;
					right_tmp += 0.3204f * *right++ + 0.2265f * *centre;
					left_tmp  += 0.3204f * *left++  + 0.2265f * *centre++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}

			break;

			// 2/2
			case 6:
				left      = stream_sample->channel[0];
				right     = stream_sample->channel[1];
				left_sur  = stream_sample->channel[2];
				right_sur = stream_sample->channel[3];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.2265f * *left_sur++ + 0.2265f * *right_sur++;
					left_tmp  = -1 * right_tmp;
					right_tmp += 0.3204f * *right++;
					left_tmp  += 0.3204f * *left++ ;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}
			break;

			// 3/1
			case 5:
				left      = stream_sample->channel[0];
				centre    = stream_sample->channel[1];
				right     = stream_sample->channel[2];
				//Mono surround
				right_sur = stream_sample->channel[3];

				for (j = 0; j < 256; j++) 
				{
					right_tmp =  0.2265f * *right_sur++;
					left_tmp  = - right_tmp;
					right_tmp += 0.3204f * *right++ + 0.2265f * *centre;
					left_tmp  += 0.3204f * *left++  + 0.2265f * *centre++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}
			break;

			// 2/1
			case 4:
				left      = stream_sample->channel[0];
				right     = stream_sample->channel[1];
				//Mono surround
				right_sur = stream_sample->channel[2];

				for (j = 0; j < 256; j++) 
				{
					right_tmp =  0.2265f * *right_sur++;
					left_tmp  = - right_tmp;
					right_tmp += 0.3204f * *right++; 
					left_tmp  += 0.3204f * *left++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}
			break;

			// 3/0
			case 3:
				left      = stream_sample->channel[0];
				centre    = stream_sample->channel[1];
				right     = stream_sample->channel[2];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.3204f * *right++ + 0.2265f * *centre;
					left_tmp  = 0.3204f * *left++  + 0.2265f * *centre++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}
			break;

			// 2/0
			case 2:
			//Do nothing!
			break;

			// 1/0
			case 1:
				//Mono program!
				right = stream_sample->channel[0];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = right_tmp;
				}
				
			break;

			// 1+1
			case 0:
				//Dual mono, output selected by user
				right = stream_sample->channel[global_prefs.dual_mono_channel_select];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = right_tmp;
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
				left      = stream_sample->channel[0];
				centre    = stream_sample->channel[1];
				right     = stream_sample->channel[2];
				left_sur  = stream_sample->channel[3];
				right_sur = stream_sample->channel[4];

				clev = cmixlev_lut[bsi->cmixlev];
				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + clev * *centre   + slev * *right_sur++;
					left_tmp = 0.4142f * *left++  + clev * *centre++ + slev * *left_sur++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}

			break;

			// 2/2
			case 6:
				left      = stream_sample->channel[0];
				right     = stream_sample->channel[1];
				left_sur  = stream_sample->channel[2];
				right_sur = stream_sample->channel[3];

				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + slev * *right_sur++;
					left_tmp = 0.4142f * *left++  + slev * *left_sur++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}

			// 3/1
			case 5:
				left      = stream_sample->channel[0];
				centre    = stream_sample->channel[1];
				right     = stream_sample->channel[2];
				//Mono surround
				right_sur = stream_sample->channel[3];

				clev = cmixlev_lut[bsi->cmixlev];
				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + clev * *centre   + slev * *right_sur;
					left_tmp = 0.4142f * *left++  + clev * *centre++ + slev * *right_sur++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}
				
				break;

			// 2/1
			case 4:
				left      = stream_sample->channel[0];
				right     = stream_sample->channel[1];
				//Mono surround
				right_sur = stream_sample->channel[2];

				slev = smixlev_lut[bsi->surmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + slev * *right_sur;
					left_tmp = 0.4142f * *left++  + slev * *right_sur++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}

			// 3/0
			case 3:
				left      = stream_sample->channel[0];
				centre    = stream_sample->channel[1];
				right     = stream_sample->channel[2];

				clev = cmixlev_lut[bsi->cmixlev];

				for (j = 0; j < 256; j++) 
				{
					right_tmp= 0.4142f * *right++ + clev * *centre;   
					left_tmp = 0.4142f * *left++  + clev * *centre++; 

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = left_tmp;
				}
				
			break;

			case 2:
			//Do nothing!
			break;

			// 1/0
			case 1:
				//Mono program!
				right = stream_sample->channel[0];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = right_tmp;
				}
				
			break;

			// 1+1
			case 0:
				//Dual mono, output selected by user
				right = stream_sample->channel[global_prefs.dual_mono_channel_select];

				for (j = 0; j < 256; j++) 
				{
					right_tmp = 0.7071f * *right++;

					stream_sample->channel[1][j] = right_tmp;
					stream_sample->channel[0][j] = right_tmp;
				}
			break;

		}
	}
}
