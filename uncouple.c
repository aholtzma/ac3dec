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
		uint_16 cplstrtmant;

		cplstrtmant = audblk->cplstrtmant;

		for(i=0; i< bsi->nfchans; i++)
		{
			if(audblk->chincpl[i])
			{

				//FIXME do the conversion once in a local buffer
				//and then memcpy
				/* ncplmant is equal to 12 * ncplsubnd */
				for(j=0; j < 12 * audblk->ncplsubnd; j++)
					 convert_to_float(audblk->cpl_exp[j],audblk->cplmant[j],
							(uint_32*) &coeffs->fbw[i][j + cplstrtmant]);
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
	/*
	 * This code is a little faster than the original code:
	 *
		for(i = 0; i < 16; i++)
		{
			if((mantissa << i) & 0x8000)
				break;
		}
		i++;
	 *
	 * The advantage is we remove the serial shift and add
	 * which helps a lot. We make up for it in increased code size
	 */
#if 0
		for(i = 0; i < 16; i++)
		{
			if((mantissa << i) & 0x8000)
				break;
		}
		i++;
#endif
		i = 0;
		//FIXME is this any better? 
	if (mantissa & 0x8000)
		i = 1;
	else if (mantissa & 0x4000)
		i = 2;
	else if (mantissa & 0x2000)
		i = 3;
	else if (mantissa & 0x1000)
		i = 4;
	else if (mantissa & 0x0800)
		i = 5;
	else if (mantissa & 0x0400)
		i = 6;
	else if (mantissa & 0x0200)
		i = 7;
	else if (mantissa & 0x0100)
		i = 8;
	else if (mantissa & 0x0080)
		i = 9;
	else if (mantissa & 0x0040)
		i = 10;
	else if (mantissa & 0x0020)
		i = 11;
	else if (mantissa & 0x0010)
		i = 12;
	else if (mantissa & 0x0008)
		i = 13;
	else if (mantissa & 0x0004)
		i = 14;
	else if (mantissa & 0x0002)
		i = 15;
	else if (mantissa & 0x0001)
		i = 16;

	/* Exponent is offset by 127 in IEEE format minus the shift to
	 * align the mantissa to 1.f */
	exponent = 0xff & (127 - exp - i);
	
	*dest = (sign << 31) | (exponent << 23) | 
		((0x007fffff) & (mantissa << (7 + i)));

}

