/* 
 *  stats.c
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
#include "stats.h"
#include "debug.h"

/* Misc LUTs that will go elsewhere soon */

static char *service_ids[] = {"CM","ME","VI","HI","D","C","E","VO"};

struct mixlev_s
{
	float clev;
	char *desc;
};

struct mixlev_s cmixlev_tbl[4] =  {{0.707, "(-3.0 dB)"}, {0.596, "(-4.5 dB)"},
                                 {0.500, "(-6.0 dB)"}, {1.0,  "Invalid"}};

struct mixlev_s smixlev_tbl[4] =  {{0.707, "(-3.0 dB)"}, {0.500, "(-6.0 dB)"},
                                   {0.0,   "off    "}, {1.0,   "Invalid"}};


void stats_printf_syncinfo(syncinfo_t *syncinfo)
{
	dprintf("(syncinfo) ");
	
	switch (syncinfo->fscod)
	{
		case 2:
			dprintf("32 KHz   ");
			break;
		case 1:
			dprintf("44.1 KHz ");
			break;
		case 0:
			dprintf("48 KHz   ");
			break;
		default:
			dprintf("Invalid sampling rate ");
			break;
	}

	dprintf("%4d kbps %4d words per frame\n",syncinfo->bit_rate, 
			syncinfo->frame_size);

}
	
void stats_printf_bsi(bsi_t *bsi)
{
	dprintf("(bsi) ");
	dprintf("%s",service_ids[bsi->bsmod]);
	dprintf(" %d.%d Mode ",bsi->nfchans,bsi->lfeon);
	if ((bsi->acmod & 0x1) && (bsi->acmod != 0x1))
		dprintf(" Centre Mix Level %s ",cmixlev_tbl[bsi->cmixlev].desc);
	if (bsi->acmod & 0x4)
		dprintf(" Sur Mix Level %s ",smixlev_tbl[bsi->cmixlev].desc);
	dprintf("\n");

}

char *exp_strat_tbl[4] = {"R   ","D15 ","D25 ","D45 "};

void stats_printf_audblk(audblk_t *audblk)
{
	dprintf("(audblk) ");
	dprintf("%s ",audblk->cplinu ? "cpl on " : "cpl off");
	dprintf("%s ",audblk->baie? "bai " : "    ");
	dprintf("%s ",audblk->snroffste? "snroffst " : "         ");
	dprintf("%s ",audblk->deltbaie? "deltba " : "       ");
	dprintf("(%s %s %s %s %s) ",exp_strat_tbl[audblk->chexpstr[0]],
		exp_strat_tbl[audblk->chexpstr[1]],exp_strat_tbl[audblk->chexpstr[2]],
		exp_strat_tbl[audblk->chexpstr[3]],exp_strat_tbl[audblk->chexpstr[4]]);
	dprintf("\n");
}
