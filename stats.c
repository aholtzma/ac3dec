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
#include "config.h"
#include "ac3.h"
#include "decode.h"
#include "stats.h"
#include "debug.h"


static char *service_ids[] = {"CM","ME","VI","HI","D","C","E","VO"};

struct mixlev_s
{
	float clev;
	char *desc;
};

struct mixlev_s cmixlev_tbl[4] =  {{0.707, "(-3.0 dB)"}, {0.595, "(-4.5 dB)"},
                                 {0.500, "(-6.0 dB)"}, {1.0,  "Invalid"}};

struct mixlev_s smixlev_tbl[4] =  {{0.707, "(-3.0 dB)"}, {0.500, "(-6.0 dB)"},
                                   {0.0,   "off    "}, {1.0,   "Invalid"}};

static const char *language[128] = 
{
	"unknown", "Albanian", "Breton", "Catalan", "Croatian", "Welsh", "Czech", "Danish", 
	"German", "English", "Spanish", "Esperanto", "Estonian", "Basque", "Faroese", "French", 
	"Frisian", "Irish", "Gaelic", "Galician", "Icelandic", "Italian", "Lappish", "Latin", 
	"Latvian", "Luxembourgian", "Lithuanian", "Hungarian", "Maltese", "Dutch", "Norwegian", "Occitan", 
	"Polish", "Portugese", "Romanian", "Romansh", "Serbian", "Slovak", "Slovene", "Finnish", 
	"Swedish", "Turkish", "Flemish", "Walloon", "0x2c", "0x2d", "0x2e", "0x2f", 
	"0x30", "0x31", "0x32", "0x33", "0x34", "0x35", "0x36", "0x37", 
	"0x38", "0x39", "0x3a", "0x3b", "0x3c", "0x3d", "0x3e", "0x3f", 
	"background", "0x41", "0x42", "0x43", "0x44", "Zulu", "Vietnamese", "Uzbek", 
	"Urdu", "Ukrainian", "Thai", "Telugu", "Tatar", "Tamil", "Tadzhik", "Swahili", 
	"Sranan Tongo", "Somali", "Sinhalese", "Shona", "Serbo-Croat", "Ruthenian", "Russian", "Quechua", 
	"Pustu", "Punjabi", "Persian", "Papamiento", "Oriya", "Nepali", "Ndebele", "Marathi", 
	"Moldavian", "Malaysian", "Malagasay", "Macedonian", "Laotian", "Korean", "Khmer", "Kazakh",
	"Kannada", "Japanese", "Indonesian", "Hindi", "Hebrew", "Hausa", "Gurani", "Gujurati", 
	"Greek", "Georgian", "Fulani", "Dari", "Churash", "Chinese", "Burmese", "Bulgarian", 
	"Bengali", "Belorussian", "Bambora", "Azerbijani", "Assamese", "Armenian", "Arabic", "Amharic"
};

void stats_print_banner(syncinfo_t *syncinfo,bsi_t *bsi)
{
	printf(PACKAGE"-"VERSION" (C) 1999 Aaron Holtzman (aholtzma@ess.engr.uvic.ca)\n");

	printf("%d.%d Mode ",bsi->nfchans,bsi->lfeon);
	switch (syncinfo->fscod)
	{
		case 2:
			printf("32 KHz   ");
			break;
		case 1:
			printf("44.1 KHz ");
			break;
		case 0:
			printf("48 KHz   ");
			break;
		default:
			printf("Invalid sampling rate ");
			break;
	}
	printf("%4d kbps ",syncinfo->bit_rate);
	if (bsi->langcode && (bsi->langcod < 128))
		printf("%s ", language[bsi->langcod]);

	switch(bsi->bsmod)
	{
		case 0:
			printf("Complete Main Audio Service");
			break;
		case 1:
			printf("Music and Effects Audio Service");
		case 2:
			printf("Visually Impaired Audio Service");
			break;
		case 3:
			printf("Hearing Impaired Audio Service");
			break;
		case 4:
			printf("Dialogue Audio Service");
			break;
		case 5:
			printf("Commentary Audio Service");
			break;
		case 6:
			printf("Emergency Audio Service");
			break;
		case 7:
			printf("Voice Over Audio Service");
			break;
	}
	printf("\n");
}

void stats_print_syncinfo(syncinfo_t *syncinfo)
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
	
void stats_print_bsi(bsi_t *bsi)
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

void stats_print_audblk(audblk_t *audblk)
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
