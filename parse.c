/* 
 *    parse.c
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

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "bitstream.h"
#include "stats.h"
#include "debug.h"
#include "parse.h"

/* Misc LUT */
static	uint_16 nfchans[] = {2,1,2,3,3,4,4,5};

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

/* Parse a syncinfo structure, minus the sync word */
void
parse_syncinfo(syncinfo_t *syncinfo,bitstream_t *bs)
{

	/* Get crc1 - we don't actually use this data though */
	bitstream_get(bs,16);

	/* Get the sampling rate */
	syncinfo->fscod  = bitstream_get(bs,2);

	/* Get the frame size code */
	syncinfo->frmsizecod = bitstream_get(bs,6);

	syncinfo->bit_rate = frmsizecod_tbl[syncinfo->frmsizecod].bit_rate;
	syncinfo->frame_size = frmsizecod_tbl[syncinfo->frmsizecod].frm_size[syncinfo->fscod];

	stats_printf_syncinfo(syncinfo);
}

/*
 * This routine fills a bsi struct from the AC3 stream
 */

void
parse_bsi(bsi_t *bsi,bitstream_t *bs)
{
	uint_32 i;

	/* Check the AC-3 version number */
	bsi->bsid = bitstream_get(bs,5);

	/* Get the audio service provided by the steram */
	bsi->bsmod = bitstream_get(bs,3);

	/* Get the audio coding mode (ie how many channels)*/
	bsi->acmod = bitstream_get(bs,3);
	/* Predecode the number of full bandwidth channels as we use this
	 * number a lot */
	bsi->nfchans = nfchans[bsi->acmod];

	/* If it is in use, get the centre channel mix level */
	if ((bsi->acmod & 0x1) && (bsi->acmod != 0x1))
		bsi->cmixlev = bitstream_get(bs,2);

	/* If it is in use, get the surround channel mix level */
	if (bsi->acmod & 0x4)
		bsi->surmixlev = bitstream_get(bs,2);

	/* Get the dolby surround mode if in 2/0 mode */
	if(bsi->acmod == 0x2)
		bsi->dsurmod= bitstream_get(bs,2);

	/* Is the low frequency effects channel on? */
	bsi->lfeon = bitstream_get(bs,1);

	/* Get the dialogue normalization level */
	bsi->dialnorm = bitstream_get(bs,5);

	/* Does compression gain exist? */
	bsi->compre = bitstream_get(bs,1);
	if (bsi->compre)
	{
		/* Get compression gain */
		bsi->compr = bitstream_get(bs,8);
	}

	/* Does language code exist? */
	bsi->langcode = bitstream_get(bs,1);
	if (bsi->langcode)
	{
		/* Get langauge code */
		bsi->langcod = bitstream_get(bs,8);
	}

	/* Does audio production info exist? */
	bsi->audprodie = bitstream_get(bs,1);
	if (bsi->audprodie)
	{
		/* Get mix level */
		bsi->mixlevel = bitstream_get(bs,5);

		/* Get room type */
		bsi->roomtyp = bitstream_get(bs,2);
	}

	/* If we're in dual mono mode then get some extra info */
	if (bsi->acmod ==0)
	{
		/* Get the dialogue normalization level two */
		bsi->dialnorm2 = bitstream_get(bs,5);

		/* Does compression gain two exist? */
		bsi->compr2e = bitstream_get(bs,1);
		if (bsi->compr2e)
		{
			/* Get compression gain two */
			bsi->compr2 = bitstream_get(bs,8);
		}

		/* Does language code two exist? */
		bsi->langcod2e = bitstream_get(bs,1);
		if (bsi->langcod2e)
		{
			/* Get langauge code two */
			bsi->langcod2 = bitstream_get(bs,8);
		}

		/* Does audio production info two exist? */
		bsi->audprodi2e = bitstream_get(bs,1);
		if (bsi->audprodi2e)
		{
			/* Get mix level two */
			bsi->mixlevel2 = bitstream_get(bs,5);

			/* Get room type two */
			bsi->roomtyp2 = bitstream_get(bs,2);
		}
	}

	/* Get the copyright bit */
	bsi->copyrightb = bitstream_get(bs,1);

	/* Get the original bit */
	bsi->origbs = bitstream_get(bs,1);
	
	/* Does timecode one exist? */
	bsi->timecod1e = bitstream_get(bs,1);

	if(bsi->timecod1e)
		bsi->timecod1 = bitstream_get(bs,14);

	/* Does timecode two exist? */
	bsi->timecod2e = bitstream_get(bs,1);

	if(bsi->timecod2e)
		bsi->timecod2 = bitstream_get(bs,14);

	/* Does addition info exist? */
	bsi->addbsie = bitstream_get(bs,1);

	if(bsi->addbsie)
	{
		/* Get how much info is there */
		bsi->addbsil = bitstream_get(bs,6);

		/* Get the additional info */
		for(i=0;i<(bsi->addbsil + 1);i++)
			bsi->addbsi[i] = bitstream_get(bs,8);
	}

	stats_printf_bsi(bsi);
}

/* More pain inducing parsing */
void
parse_audblk(bsi_t *bsi,audblk_t *audblk,bitstream_t *bs)
{
	int i,j;

	for (i=0;i < bsi->nfchans; i++)
	{
		/* Is this channel an interleaved 256 + 256 block ? */
		audblk->blksw[i] = bitstream_get(bs,1);
	}

	for (i=0;i < bsi->nfchans; i++)
	{
		/* Should we dither this channel? */
		audblk->dithflag[i] = bitstream_get(bs,1);
	}

	/* Does dynamic range control exist? */
	audblk->dynrnge = bitstream_get(bs,1);
	if (audblk->dynrnge)
	{
		/* Get dynamic range info */
		audblk->dynrng = bitstream_get(bs,8);
	}

	/* If we're in dual mono mode then get the second channel DR info */
	if (bsi->acmod == 0)
	{
		/* Does dynamic range control two exist? */
		audblk->dynrng2e = bitstream_get(bs,1);
		if (audblk->dynrng2e)
		{
			/* Get dynamic range info */
			audblk->dynrng2 = bitstream_get(bs,8);
		}
	}

	/* Does coupling strategy exist? */
	audblk->cplstre = bitstream_get(bs,1);
	if (audblk->cplstre)
	{
		/* Is coupling turned on? */
		audblk->cplinu = bitstream_get(bs,1);
		if(audblk->cplinu)
		{
			for(i=0;i < bsi->nfchans; i++)
				audblk->chincpl[i] = bitstream_get(bs,1);
			if(bsi->acmod == 0x2)
				audblk->phsflginu = bitstream_get(bs,1);
			audblk->cplbegf = bitstream_get(bs,4);
			audblk->cplendf = bitstream_get(bs,4);
			audblk->ncplsubnd = (audblk->cplendf + 2) - audblk->cplbegf + 1;

			/* Calculate the start and end bins of the coupling channel */
			audblk->cplstrtmant = (audblk->cplbegf * 12) + 37 ; 
			audblk->cplendmant =  ((audblk->cplendf + 3) * 12) + 37;

			/* The number of combined subbands is ncplsubnd minus each combined
			 * band */
			audblk->ncplbnd = audblk->ncplsubnd; 

			for(i=1; i< audblk->ncplsubnd; i++)
			{
				audblk->cplbndstrc[i] = bitstream_get(bs,1);
				audblk->ncplbnd -= audblk->cplbndstrc[i];
			}
		}
	}

	if(audblk->cplinu)
	{
		/* Loop through all the channels and get their coupling co-ords */	
		for(i=0;i < bsi->nfchans;i++)
		{
			if(!audblk->chincpl[i])
				continue;

			/* Is there new coupling co-ordinate info? */
			audblk->cplcoe[i] = bitstream_get(bs,1);

			if(audblk->cplcoe[i])
			{
				audblk->mstrcplco[i] = bitstream_get(bs,2); 
				for(j=0;j < audblk->ncplbnd; j++)
				{
					audblk->cplcoexp[i][j] = bitstream_get(bs,4); 
					audblk->cplcomant[i][j] = bitstream_get(bs,4); 
				}
			}
		}

		/* If we're in dual mono mode, there's going to be some phase info */
		if( (bsi->acmod == 0x2) && audblk->phsflginu && 
				(audblk->cplcoe[0] || audblk->cplcoe[1]))
		{
			for(j=0;j < audblk->ncplbnd; j++)
				audblk->phsflg[j] = bitstream_get(bs,1); 

		}
	}

	/* If we're in dual mono mode, there may be a rematrix strategy */
	if(bsi->acmod == 0x2)
	{
		audblk->rematstr = bitstream_get(bs,1);
		if(audblk->rematstr)
		{
			if (audblk->cplinu == 0) 
			{ 
				for(i = 0; i < 4; i++) 
					audblk->rematflg[i] = bitstream_get(bs,1);
			}
			if((audblk->cplbegf > 2) && audblk->cplinu) 
			{
				for(i = 0; i < 4; i++) 
					audblk->rematflg[i] = bitstream_get(bs,1);
			}
			if((audblk->cplbegf <= 2) && audblk->cplinu) 
			{ 
				for(i = 0; i < 3; i++) 
					audblk->rematflg[i] = bitstream_get(bs,1);
			} 
			if((audblk->cplbegf == 0) && audblk->cplinu) 
				for(i = 0; i < 2; i++) 
					audblk->rematflg[i] = bitstream_get(bs,1);

		}
	}

	if (audblk->cplinu)
	{
		/* Get the coupling channel exponent strategy */
		audblk->cplexpstr = bitstream_get(bs,2);
		audblk->ncplgrps = (audblk->cplendmant - audblk->cplstrtmant) / 
				(3 << (audblk->cplexpstr-1));
	}

	for(i = 0; i < bsi->nfchans; i++)
		audblk->chexpstr[i] = bitstream_get(bs,2);

	/* Get the exponent strategy for lfe channel */
	if(bsi->lfeon) 
		audblk->lfeexpstr = bitstream_get(bs,1);

	/* Determine the bandwidths of all the fbw channels */
	for(i = 0; i < bsi->nfchans; i++) 
	{ 
		uint_16 grp_size;

		if(audblk->chexpstr[i] != EXP_REUSE) 
		{ 
			if (audblk->cplinu && audblk->chincpl[i]) 
			{
				audblk->endmant[i] = audblk->cplstrtmant;
			}
			else
			{
				audblk->chbwcod[i] = bitstream_get(bs,6); 
				audblk->endmant[i] = ((audblk->chbwcod[i] + 12) * 3) + 37;
			}

			/* Calculate the number of exponent groups to fetch */
			grp_size =  3 * (1 << (audblk->chexpstr[i] - 1));
			audblk->nchgrps[i] = (audblk->endmant[i] - 1 + (grp_size - 3)) / grp_size;
		}
	}

	/* Get the coupling exponents if they exist */
	if(audblk->cplinu && (audblk->cplexpstr != EXP_REUSE))
	{
		audblk->cplabsexp = bitstream_get(bs,4);
		for(i=0;i< audblk->ncplgrps;i++)
			audblk->cplexps[i] = bitstream_get(bs,7);
	}

	/* Get the fwb channel exponents */
	for(i=0;i < bsi->nfchans; i++)
	{
		if(audblk->chexpstr[i] != EXP_REUSE)
		{
			audblk->exps[i][0] = bitstream_get(bs,4);			
			for(j=1;j<=audblk->nchgrps[i];j++)
				audblk->exps[i][j] = bitstream_get(bs,7);
			audblk->gainrng[i] = bitstream_get(bs,2);
		}
	}

	/* Get the lfe channel exponents */
	if(bsi->lfeon && (audblk->lfeexpstr != EXP_REUSE))
	{
		audblk->lfeexps[0] = bitstream_get(bs,4);
		audblk->lfeexps[1] = bitstream_get(bs,7);
		audblk->lfeexps[2] = bitstream_get(bs,7);
	}

	/* Get the parametric bit allocation parameters */
	audblk->baie = bitstream_get(bs,1);

	if(audblk->baie)
	{
		audblk->sdcycod = bitstream_get(bs,2);
		audblk->fdcycod = bitstream_get(bs,2);
		audblk->sgaincod = bitstream_get(bs,2);
		audblk->dbpbcod = bitstream_get(bs,2);
		audblk->floorcod = bitstream_get(bs,3);
	}

	/* Get the SNR off set info if it exists */
	audblk->snroffste = bitstream_get(bs,1);

	if(audblk->snroffste)
	{
		audblk->csnroffst = bitstream_get(bs,6);

		if(audblk->cplinu)
		{
			audblk->cplfsnroffst = bitstream_get(bs,4);
			audblk->cplfgaincod = bitstream_get(bs,3);
		}

		for(i = 0;i < bsi->nfchans; i++)
		{
			audblk->fsnroffst[i] = bitstream_get(bs,4);
			audblk->fgaincod[i] = bitstream_get(bs,3);
		}
		if(bsi->lfeon)
		{

			audblk->lfefsnroffst = bitstream_get(bs,4);
			audblk->lfefgaincod = bitstream_get(bs,3);
		}
	}

	/* Get coupling leakage info if it exists */
	if(audblk->cplinu)
	{
		audblk->cplleake = bitstream_get(bs,1);	
		
		if(audblk->cplleake)
		{
			audblk->cplfleak = bitstream_get(bs,3);
			audblk->cplsleak = bitstream_get(bs,3);
		}
	}
	
	/* Get the delta bit alloaction info */
	audblk->deltbaie = bitstream_get(bs,1);	
	
	if(audblk->deltbaie)
	{
		if(audblk->cplinu)
			audblk->cpldeltbae = bitstream_get(bs,2);

		for(i = 0;i < bsi->nfchans; i++)
			audblk->deltbae[i] = bitstream_get(bs,2);

		if (audblk->cplinu && (audblk->cpldeltbae == DELTA_BIT_NEW))
		{
			audblk->cpldeltnseg = bitstream_get(bs,3);
			for(i = 0;i < audblk->cpldeltnseg + 1; i++)
			{
				audblk->cpldeltoffst[i] = bitstream_get(bs,5);
				audblk->cpldeltlen[i] = bitstream_get(bs,4);
				audblk->cpldeltba[i] = bitstream_get(bs,3);
			}
		}

		for(i = 0;i < bsi->nfchans; i++)
		{
			if (audblk->cplinu && (audblk->deltbae[i] == DELTA_BIT_NEW))
			{
				audblk->deltnseg[i] = bitstream_get(bs,3);
				for(j = 0; j < audblk->cpldeltnseg + 1; j++)
				{
					audblk->deltoffst[i][j] = bitstream_get(bs,5);
					audblk->deltlen[i][j] = bitstream_get(bs,4);
					audblk->deltba[i][j] = bitstream_get(bs,3);
				}
			}
		}
	}

	/* Check to see if there's any dummy info to get */
	if((audblk->skiple =  bitstream_get(bs,1)))
	{
		uint_16 skip_data;

		audblk->skipl = bitstream_get(bs,9);
		//FIXME remove
		//fprintf(stderr,"(parse) skipping %d bytes\n",audblk->skipl);

		for(i = 0; i < audblk->skipl ; i++)
		{
			skip_data = bitstream_get(bs,8);
			//FIXME remove
			//fprintf(stderr,"skipped data %2x\n",skip_data);
			//if(skip_data != 0)
			//{	
				//dprintf("(parse) Invalid skipped data %2x\n",skip_data);
				//exit(1);
			//}
		}
	}

	stats_printf_audblk(audblk);
}

void
parse_auxdata(syncinfo_t *syncinfo,bitstream_t *bs)
{
	int i;
	int skip_length;
	uint_16 crc;
	uint_16 auxdatae;

	skip_length = (syncinfo->frame_size * 16)  - bs->total_bits_read - 17 - 1;

	//FIXME remove
	//dprintf("(auxdata) skipping %d auxbits\n",skip_length);
	
	for(i=0; i <  skip_length; i++)
		//printf("Skipped bit %i\n",(uint_16)bitstream_get(bs,1));
		bitstream_get(bs,1);

	//get the auxdata exists bit
	auxdatae = bitstream_get(bs,1);	

	//FIXME remove
	//dprintf("auxdatae = %i\n",auxdatae);

	//Skip the CRC reserved bit
	bitstream_get(bs,1);

	//Get the crc
	crc = bitstream_get(bs,16);
}


