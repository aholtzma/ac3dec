/*
 *
 *  wav.h
 *    
 *	Copyright (C) Jorgen Lundman <lundman@argonaut.com> - May 1999 
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
 

#define WAVE_FORMAT_PCM  0x0001
#define FORMAT_MULAW     0x0101
#define IBM_FORMAT_ALAW  0x0102
#define IBM_FORMAT_ADPCM 0x0103


struct riff_struct 
{
  unsigned char id[4];   /* RIFF */
  unsigned long len;
  unsigned char wave_id[4]; /* WAVE */
};


struct chunk_struct 
{
  unsigned char id[4];
  unsigned long len;
};

struct common_struct 
{
  unsigned short wFormatTag;
  unsigned short wChannels;
  unsigned long dwSamplesPerSec;
  unsigned long dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;  /* Only for PCM */
};

struct wave_header 
{
  struct riff_struct   riff;
  struct chunk_struct  format;
  struct common_struct common;
  struct chunk_struct  data;
 
  struct riff_struct   riffdata;
  struct chunk_struct  dataformat;
};
