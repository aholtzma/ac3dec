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

struct mixlev_s
{
	float clev;
	char *desc;
};

struct mixlev_s cmixlev_tbl[4] =  {{0.707, "(-3.0 dB)"}, {0.596, "(-4.5 dB)"},
                                 {0.500, "(-6.0 dB)"}, {1.0,  "Invalid"}};

struct mixlev_s smixlev_tbl[4] =  {{0.707, "(-3 dB)"}, {0.500, "(-6 dB)"},
                                   {0.0,   "off    "}, {1.0,   "Invalid"}};
struct frmsize_s
{
	uint_16 bit_rate;
	uint_16 frm_size[3];
};

struct frmsize_s frmsizecod_tbl[] = {
      { 32  ,{64   ,69   ,96   } },
      { 32  ,{64   ,70   ,96   } },
      { 40  ,{80   ,87   ,120  } },
      { 40  ,{80   ,88   ,120  } },
      { 48  ,{96   ,104  ,144  } },
      { 48  ,{96   ,105  ,144  } },
      { 56  ,{112  ,121  ,168  } },
      { 56  ,{112  ,122  ,168  } },
      { 64  ,{128  ,139  ,192  } },
      { 64  ,{128  ,140  ,192  } },
      { 80  ,{160  ,174  ,240  } },
      { 80  ,{160  ,175  ,240  } },
      { 96  ,{192  ,208  ,288  } },
      { 96  ,{192  ,209  ,288  } },
      { 112 ,{224  ,243  ,336  } },
      { 112 ,{224  ,244  ,336  } },
      { 128 ,{256  ,278  ,384  } },
      { 128 ,{256  ,279  ,384  } },
      { 160 ,{320  ,348  ,480  } },
      { 160 ,{320  ,349  ,480  } },
      { 192 ,{384  ,417  ,576  } },
      { 192 ,{384  ,418  ,576  } },
      { 224 ,{448  ,487  ,672  } },
      { 224 ,{448  ,488  ,672  } },
      { 256 ,{512  ,557  ,768  } },
      { 256 ,{512  ,558  ,768  } },
      { 320 ,{640  ,696  ,960  } },
      { 320 ,{640  ,697  ,960  } },
      { 384 ,{768  ,835  ,1152 } },
      { 384 ,{768  ,836  ,1152 } },
      { 448 ,{896  ,975  ,1344 } },
      { 448 ,{896  ,976  ,1344 } },
      { 512 ,{1024 ,1114 ,1536 } },
      { 512 ,{1024 ,1115 ,1536 } },
      { 576 ,{1152 ,1253 ,1728 } },
      { 576 ,{1152 ,1254 ,1728 } },
      { 640 ,{1280 ,1393 ,1920 } },
      { 640 ,{1280 ,1394 ,1920 } }};

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

	dprintf("%4d kbps %4d words per frame\n",
			frmsizecod_tbl[syncinfo->frmsizecod].bit_rate,
			frmsizecod_tbl[syncinfo->frmsizecod].frm_size[syncinfo->fscod]);

}
	
void stats_printf_bsi(bsi_t *bsi)
{
	dprintf("(bsi) ");
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
