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
#include "uncouple.h"

void convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest);

static float cpl_tmp[256];

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

		for(j=audblk->cplstrtmant; j < audblk->cplendmant; j++)
			 convert_to_float(audblk->cpl_exp[j],audblk->cplmant[j],
					(uint_32*) &cpl_tmp[j]);

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
	uint_32 i;
	float cpl_coord;
	uint_32 cpl_exp_tmp;
	uint_32 cpl_mant_tmp;

	for(i=audblk->cplstrtmant;i<audblk->cplendmant;i+=12)
	{
		if(!audblk->cplbndstrc[bnd])
		{
			cpl_exp_tmp = audblk->cplcoexp[ch][bnd] + 3 * audblk->mstrcplco[ch];
			if(audblk->cplcoexp[ch][bnd] == 15)
				cpl_mant_tmp = (audblk->cplcomant[ch][bnd]) << 12;
			else
				cpl_mant_tmp = ((0x10) & audblk->cplcomant[ch][bnd]) << 11;
			
			convert_to_float(cpl_exp_tmp,cpl_mant_tmp,(uint_32*)&cpl_coord);
		}
		bnd++;

		coeffs->fbw[ch][i]   = cpl_coord * cpl_tmp[i];
		coeffs->fbw[ch][i+1] = cpl_coord * cpl_tmp[i+1];
		coeffs->fbw[ch][i+2] = cpl_coord * cpl_tmp[i+2];
		coeffs->fbw[ch][i+3] = cpl_coord * cpl_tmp[i+3];
		coeffs->fbw[ch][i+4] = cpl_coord * cpl_tmp[i+4];
		coeffs->fbw[ch][i+5] = cpl_coord * cpl_tmp[i+5];
		coeffs->fbw[ch][i+6] = cpl_coord * cpl_tmp[i+6];
		coeffs->fbw[ch][i+7] = cpl_coord * cpl_tmp[i+7];
		coeffs->fbw[ch][i+8] = cpl_coord * cpl_tmp[i+8];
		coeffs->fbw[ch][i+9] = cpl_coord * cpl_tmp[i+9];
		coeffs->fbw[ch][i+10] = cpl_coord * cpl_tmp[i+10];
		coeffs->fbw[ch][i+11] = cpl_coord * cpl_tmp[i+11];
	}
}

/* Converts an unsigned exponent in the range of 0-24 and a 16 bit mantissa
 * to an IEEE single precision floating point value */
void convert_to_float(uint_16 exp, uint_16 mant, uint_32 *dest)
{
	uint_16 sign;
	uint_16 exponent;
	uint_16 mantissa;
	int i;

	/* If the mantissa is zero we can simply return zero */
	if(mant == 0)
	{
		*dest = 0;
		return;
	}

	/* Take care of the one asymmetric negative number */
	if(mant == 0x8000)
		mant++;

	/* Extract the sign bit */
	sign = mant & 0x8000 ? 1 : 0;

	/* Invert the mantissa if it's negative */
	if(sign)
		mantissa = (~mant) + 1;
	else 
		mantissa = mant;

	/* Shift out the sign bit */
	mantissa <<= 1;

	/* Find the index of the most significant one bit */
	for(i = 0; i < 16; i++)
	{
		if((mantissa << i) & 0x8000)
			break;
	}
	i++;

	/* Exponent is offset by 127 in IEEE format minus the shift to
	 * align the mantissa to 1.f */
	exponent = 0xff & (127 - exp - i);
	
	*dest = (sign << 31) | (exponent << 23) | 
		((0x007fffff) & (mantissa << (7 + i)));

}

