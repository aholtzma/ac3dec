/*
 *
 *  matlab.c
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

#include <stdio.h>
#include <stdlib.h>
#include "matlab.h"


matlab_file_t *matlab_open(char name[])
{
	matlab_file_t *mf;	

	mf = malloc(sizeof(matlab_file_t));
	if(!mf)
		return 0;

	mf->f = fopen(name,"w");
	if(!mf->f)
	{
		free(mf);
		return 0;
	}

  fprintf(mf->f,"s = [\n");

	return mf;
}

void matlab_close(matlab_file_t *mf)
{
  fprintf(mf->f,"];\n");
	fclose(mf->f);
	free(mf);
}

void matlab_write(matlab_file_t *mf,float x[], int length)
{
  int i;

  for(i=0;i< length;i++)
    fprintf(mf->f,"%5f;\n ",x[i]);
}

