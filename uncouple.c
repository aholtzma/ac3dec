/* 
 *    uncouple.c
 *
 *	Aaron Holtzman - May 1999
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "decode.h"
#include "dither.h"
#include "uncouple.h"


void convert_to_float(uint_16 exp, uint_16 mantissa, uint_32 *dest);

void uncouple_channel(stream_coeffs_t *coeffs,audblk_t *audblk, uint_32 ch);

void
uncouple(bsi_t *bsi,audblk_t *audblk,stream_coeffs_t *coeffs)
{
	int i,j;

	for(i=0; i< bsi->nfchans; i++)
	{
		for(j=0; j < audblk->endmant[i]; j++)
			 convert_to_float(audblk->fbw_exp[i][j],audblk->chmant[i][j],
					 (uint_32*) &coeffs->fbw[i][j]);
	}

	if(audblk->cplinu)
	{
		for(i=0; i< bsi->nfchans; i++)
		{
			if(audblk->chincpl[i])
			{
				uncouple_channel(coeffs,audblk,i);
			}
		}

	}

	if(bsi->lfeon)
	{
		/* There are always 7 mantissas for lfe */
		for(j=0; j < 7 ; j++)
			 convert_to_float(audblk->lfe_exp[j],audblk->lfemant[j],
					 (uint_32*) &coeffs->lfe[j]);

	}

}

void
uncouple_channel(stream_coeffs_t *coeffs,audblk_t *audblk, uint_32 ch)
{
	uint_32 bnd = 0;
	uint_32 i,j;
	float cpl_coord;
	float coeff;
	uint_32 cpl_exp_tmp;
	uint_32 cpl_mant_tmp;

	for(i=audblk->cplstrtmant;i<audblk->cplendmant;)
	{
		if(!audblk->cplbndstrc[bnd])
		{
			cpl_exp_tmp = audblk->cplcoexp[ch][bnd] + 3 * audblk->mstrcplco[ch];
			if(audblk->cplcoexp[ch][bnd] == 15)
				cpl_mant_tmp = (audblk->cplcomant[ch][bnd]) << 12;
			else
				cpl_mant_tmp = ((0x10) | audblk->cplcomant[ch][bnd]) << 11;
			
			convert_to_float(cpl_exp_tmp,cpl_mant_tmp,(uint_32*)&cpl_coord);
		}
		bnd++;

		for(j=0;j < 12; j++)
		{
			//Get new dither values for each channel if necessary, so
			//the channels are uncorrelated
			if(audblk->dithflag[ch] && audblk->cpl_bap[i] == 0)
				convert_to_float(audblk->cpl_exp[i],dither_gen(),(uint_32*)&coeff);
			else
				convert_to_float(audblk->cpl_exp[i], audblk->cplmant[i],(uint_32*)&coeff);

			coeffs->fbw[ch][i]  = cpl_coord * coeff;

			i++;
		}
	}
}

static uint_8 first_bit_lut[256] = 
{
	0, 8, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

/* Converts an unsigned exponent in the range of 0-24 and a 16 bit mantissa
 * to an IEEE single precision floating point value */
void convert_to_float(uint_16 exp, uint_16 mantissa, uint_32 *dest)
{
	uint_32 num;
	uint_16 exponent;
	uint_8 i;

	/* If the mantissa is zero we can simply return zero */
	if(mantissa == 0)
	{
		*dest = 0;
		return;
	}

	/* Exponent is offset by 127 in IEEE format minus the shift to
	 * align the mantissa to 1.f (subtracted in the final result) */
	exponent = 127 - exp;

	/* Take care of the one asymmetric negative number */
	if(mantissa == 0x8000)
		mantissa++;

	/* Extract the sign bit, invert the mantissa if it's negative, and 
	   shift out the sign bit */
	if( mantissa & 0x8000 )
	{
		mantissa *= -2;
		num = 0x80000000 + (exponent << 23);
	}
	else
	{
		mantissa *= 2;
		num = exponent << 23;
	}

	/* Find the index of the most significant one bit */
	i = first_bit_lut[mantissa >> 8];

	if(i == 0)
		i = first_bit_lut[mantissa & 0xff] + 8;
	
	*dest = num - (i << 23) + (mantissa << (7 + i));

}

