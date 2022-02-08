/**
  * @file: rs.c
  * @brief: This file implement the functions to encode and decode the
  * Reed-Solomon error correction code.
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2020/01/01[SCO]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup RS
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "rs.h"

/*! @par */
/*!
 * @static
 * @brief This variable hold the irreducible polynomial coefficients (1+x^^2+x^^3+x^^4+x^^8)
 */
static const uint8_t pp[mm+1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1};

/*!
 * @static
 * @brief This variable hold the index values
 */
static int16_t index_of [nn+1];

/*!
 * @static
 * @brief This variable hold the alpha values
 */
static uint8_t alpha_to [nn+1];

/*!
 * @static
 * @brief This variable hold the Galois Field values
 */
static uint8_t gg[nn-kk+1];


static void _generate_gf_(void);
static void _gen_poly_(void);

/*!
  * @fn const uint8_t* RS_GetGG_ptr(uint32_t *u32_ggSz)
  * @brief  This function give the generator polynomial pointer.
  *
  * @param[in,out] *u32_ggSz This pointer will hold the generator polynomial size.
  *
  * @return The generator polynomial pointer.
  */
const uint8_t* RS_GetGG_ptr(uint32_t *u32_ggSz)
{
	*u32_ggSz = sizeof(gg);
	return (const uint8_t*)(&gg[0]);
}

/*!
  * @fn const uint8_t* RS_GetAlphaOf_ptr(uint32_t *u32_alphaOfSz)
  * @brief  This function give the alpha_to pointer.
  *
  * @param[in,out] *u32_alphaOfSz This pointer will hold the alpha_to size.
  *
  * @return The alpha_to pointer.
  */
const uint8_t* RS_GetAlphaOf_ptr(uint32_t *u32_alphaOfSz)
{
	*u32_alphaOfSz = (sizeof(alpha_to)/sizeof(uint8_t));
	return (const uint8_t*)(&alpha_to[0]);
}

/*!
  * @fn const int16_t* RS_GetIndexOf_ptr(uint32_t *u32_indexOfSz)
  * @brief  This function give the index_of pointer.
  *
  * @param[in,out] *u32_indexOfSz This pointer will hold the index_of size.
  *
  * @return The index_of pointer.
  */
const int16_t* RS_GetIndexOf_ptr(uint32_t *u32_indexOfSz)
{
	*u32_indexOfSz = (sizeof(index_of)/sizeof(int16_t));
	return (const int16_t*)(&index_of[0]);
}

/*!
  * @fn uint32_t RS_GetMsgSize(void)
  * @brief  This function give the message size (in byte).
  *
  * @return The message size.
  */
uint32_t RS_GetMsgSize(void)
{
	return (uint32_t)kk;
}

/*!
  * @fn uint32_t RS_GetParitySize(void)
  * @brief  This function give the parity word size (in byte).
  *
  * @return The parity word size.
  */
uint32_t RS_GetParitySize(void)
{
	return (uint32_t)(tt*2);
}

/*!
  * @static
  *
  * @brief  This private function generate the Galois-Field
  *
  * @detail Generate GF(2**mm) from the irreducible polynomial p(X) in
  * pp[0]..pp[mm] lookup tables:
  *    - index->polynomial form  alpha_to[] contains j=alpha**i;
  *    - polynomial form -> index form  index_of[j=alpha**i] = i alpha=2 is the
  *    primitive element of GF(2**mm).
  *
  */
static void _generate_gf_(void)
{
   register int32_t i, mask;

   mask = 1;
   alpha_to[mm] = 0;
   for (i=0; i<mm; i++)
   {
      alpha_to[i] = mask;
      index_of[alpha_to[i]] = i;
      if (pp[i]!=0)
      {
         alpha_to[mm] ^= mask;
      }
      mask <<= 1;
   }

   index_of[alpha_to[mm]] = mm;
   mask >>= 1;
   for (i=mm+1; i<nn; i++)
   {
      if (alpha_to[i-1] >= mask)
      {
         alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i-1]^mask)<<1);
      }
      else
      {
         alpha_to[i] = alpha_to[i-1]<<1;
      }
      index_of[alpha_to[i]] = i;
   }
   index_of[0] = -1;
}

/*!
  * @static
  *
  * @brief  This private function compute the generator polynomial.
  *
  * @detail Obtain the generator polynomial of the tt-error correcting, length
  * nn=(2**mm -1) Reed Solomon code  from the product of (X+alpha**i), i=1..2*tt
  *
  */
static void _gen_poly_(void)
{
   register int32_t i,j;

   gg[0] = 2;    /* primitive element alpha = 2  for GF(2**mm)  */
   gg[1] = 1;    /* g(x) = (X+alpha) initially */

   for (i=2; i<=nn-kk; i++)
   {
      gg[i] = 1;
      for (j=i-1; j>0; j--)
      {
         if (gg[j] != 0)
         {
            gg[j] = gg[j-1]^ alpha_to[(index_of[gg[j]]+i)%nn];
         }
         else
         {
            gg[j] = gg[j-1];
         }

      }
      gg[0] = alpha_to[(index_of[gg[0]]+i)%nn];     /* gg[0] can never be zero */
   }
   /* convert gg[] to index form for quicker encoding */
   for (i=0; i<=nn-kk; i++)
   {
      gg[i] = index_of[gg[i]];
   }
}

/*!
  * @fn void RS_Init(void)
  * @brief  This function initialize the Galois field and polynomial generator
  * tables.
  *
  */
void RS_Init(void)
{
	/* generate the Galois Field GF(2**mm) */
	_generate_gf_();
	/* compute the generator polynomial for this RS code */
	_gen_poly_();

}

/*!
  * @fn  uint8_t RS_Decode(uint8_t b_recd [RS_MESSAGE_SZ + RS_PARITY_SZ])
  * @brief  This function detect and correct the given message.
  *
  * @param [in,out] b_recd  Pointer on source and destination data (Message
  * and Parity). b_recd[0 to 222] contains message data, while
  * b_recd[223 to 223 + 32] contains the parity bytes).
  * @retval  0: Error;
  * @retval  1: Success
  *
  */
uint8_t RS_Decode(uint8_t b_recd [RS_MESSAGE_SZ + RS_PARITY_SZ])
/* assume we have received bits grouped into mm-bit symbols in recd[i],
   i=0..(nn-1),  and recd[i] is index form (ie as powers of alpha).
   We first compute the 2*tt syndromes by substituting alpha**i into rec(X) and
   evaluating, storing the syndromes in s[i], i=1..2tt (leave s[0] zero) .
   Then we use the Berlekamp iteration to find the error location polynomial
   elp[i].   If the degree of the elp is >tt, we cannot correct all the errors
   and hence just put out the information symbols uncorrected. If the degree of
   elp is <=tt, we substitute alpha**i , i=1..n into the elp to get the roots,
   hence the inverse roots, the error location numbers. If the number of errors
   located does not equal the degree of the elp, we have more than tt errors
   and cannot correct them.  Otherwise, we then solve for the error value at
   the error location and correct the error.  The procedure is that found in
   Lin and Costello. For the cases where the number of errors is known to be too
   large to correct, the information symbols as received are output (the
   advantage of systematic encoding is that hopefully some of the information
   symbols will be okay and that if we are in luck, the errors are in the
   parity part of the transmitted codeword).  Of course, these insoluble cases
   can be returned as error flags to the calling routine if desired.   */
{
	int16_t i,j,u,q;
	int16_t elp[nn-kk+2][nn-kk];
	int16_t d[nn-kk+2];
	int16_t u_lu[nn-kk+2];

	int16_t recd [nn];

	uint8_t err[nn];
	uint8_t l[nn-kk+2];
	uint8_t s[nn-kk+1];
	uint8_t z[tt+1];
	uint8_t reg[tt+1];
	uint8_t root[tt];
	uint8_t loc[tt];

	uint8_t count=0;
	uint8_t syn_error=0;
	uint8_t ret = 1;

	/* put a_recd[i] into index form */
	for (i=0; i<nn; i++){
		recd[i] = index_of[b_recd[i]];
	}

	/* first form the syndromes */
	for (i=1; i<=nn-kk; i++)
	{
		s[i] = 0;
		for (j=0; j<nn; j++)
		{
			if (recd[j]!=-1) /* recd[j] in index form */
			{
				s[i] ^= alpha_to[(recd[j]+i*j)%nn];
			}
		}
		/* convert syndrome from polynomial form to index form  */
		if (s[i]!=0)
		{
			syn_error=1;/* set flag if non-zero syndrome => error */
		}
		s[i] = index_of[s[i]];
	};

	if (syn_error) /* if errors, try and correct */
	{
	/* compute the error location polynomial via the Berlekamp iterative algorithm,
	   following the terminology of Lin and Costello :   d[u] is the 'mu'th
	   discrepancy, where u='mu'+1 and 'mu' (the Greek letter!) is the step number
	   ranging from -1 to 2*tt (see L&C),  l[u] is the
	   degree of the elp at that step, and u_l[u] is the difference between the
	   step number and the degree of the elp.
	*/

		/* initialise table entries */
		d[0] = 0;           /* index form */
		d[1] = s[1];        /* index form */
		elp[0][0] = 0;      /* index form */
		elp[1][0] = 1;      /* polynomial form */
		for (i=1; i<nn-kk; i++)
		{
			elp[0][i] = -1;   /* index form */
			elp[1][i] = 0;   /* polynomial form */
		}
		l[0] = 0;
		l[1] = 0;
		u_lu[0] = -1;
		u_lu[1] = 0;
		u = 0;

		do
		{
			u++;
			if (d[u]==-1)
			{
				l[u+1] = l[u];
				for (i=0; i<=l[u]; i++)
				{
					elp[u+1][i] = elp[u][i];
					elp[u][i] = index_of[elp[u][i]];
				}
			}
			else /* search for words with greatest u_lu[q] for which d[q]!=0 */
			{
				q = u-1;
				while ((d[q]==-1) && (q>0)) { q--;}
				/* have found first non-zero d[q]  */
				if (q>0)
				{
					j=q;
					do
					{
						j--;
						if ((d[j]!=-1) && (u_lu[q]<u_lu[j])) { q = j; }
					}while (j>0);
				};
				/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
				/* store degree of new elp polynomial */
				if (l[u]>l[q]+u-q) { l[u+1] = l[u]; }
				else { l[u+1] = l[q]+u-q; }

				/* form new elp(x) */
				for (i=0; i<nn-kk; i++)  {  elp[u+1][i] = 0; }
				for (i=0; i<=l[q]; i++)
				{
					if (elp[q][i]!=-1)
					{
						elp[u+1][i+u-q] = alpha_to[(d[u]+nn-d[q]+elp[q][i])%nn];
					}
				}
				for (i=0; i<=l[u]; i++)
				{
					elp[u+1][i] ^= elp[u][i];
					elp[u][i] = index_of[elp[u][i]];  /*convert old elp value to index*/
				}
			}
			u_lu[u+1] = u-l[u+1];

			/* form (u+1)th discrepancy */
			if (u<nn-kk)    /* no discrepancy computed on last iteration */
			{
				if (s[u+1]!=-1) { d[u+1] = alpha_to[s[u+1]]; }
				else { d[u+1] = 0; }
				for (i=1; i<=l[u+1]; i++)
				{
					if ((s[u+1-i]!=-1) && (elp[u+1][i]!=0))
					{
						d[u+1] ^= alpha_to[(s[u+1-i]+index_of[elp[u+1][i]])%nn];
					}
				}
            	d[u+1] = index_of[d[u+1]];    /* put d[u+1] into index form */
			}
		} while ((u<nn-kk) && (l[u+1]<=tt));

		u++;
		if (l[u]<=tt) /* can correct error */
		{
			/* put elp into index form */
			for (i=0; i<=l[u]; i++) {  elp[u][i] = index_of[elp[u][i]]; }
			/* find roots of the error location polynomial */
			for (i=1; i<=l[u]; i++)
			{
				reg[i] = elp[u][i];
			}
			count = 0;
			for (i=1; i<=nn; i++)
			{
				q = 1;
				for (j=1; j<=l[u]; j++)
				{
					if (reg[j]!=-1)
					{
						reg[j] = (reg[j]+j)%nn;
						q ^= alpha_to[reg[j]];
					};
				}
				if (!q)        /* store root and error location number indices */
				{
					root[count] = i;
					loc[count] = nn-i;
					count++;
				};
			};
			if (count==l[u])    /* no. roots = degree of elp hence <= tt errors */
			{
				/* form polynomial z(x) */
				for (i=1; i<=l[u]; i++)        /* Z[0] = 1 always - do not need */
				{
					if ((s[i]!=-1) && (elp[u][i]!=-1))
					{
						z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]];
					}
					else
					{
						if ((s[i]!=-1) && (elp[u][i]==-1))
						{
							z[i] = alpha_to[s[i]];
						}
						else
						{
							if ((s[i]==-1) && (elp[u][i]!=-1))
							{
								z[i] = alpha_to[elp[u][i]];
							}
							else
                            {
								z[i] = 0;
                            }
						}
					}
					for (j=1; j<i; j++)
					{
						if ((s[j]!=-1) && (elp[u][i-j]!=-1))
						{
							z[i] ^= alpha_to[(elp[u][i-j] + s[j])%nn];
						}
					}
					z[i] = index_of[z[i]];         /* put into index form */
				};
				/* evaluate errors at locations given by error location numbers loc[i] */
				for (i=0; i<nn; i++)
				{
					err[i] = 0;
					if (recd[i]!=-1) /* convert recd[] to polynomial form */
					{
						recd[i] = alpha_to[recd[i]];
					}
					else
					{
						recd[i] = 0;
					}
				}
				for (i=0; i<l[u]; i++) /* compute numerator of error term first */
				{
					err[loc[i]] = 1;  /* accounts for z[0] */
					for (j=1; j<=l[u]; j++)
					{
						if (z[j]!=-1)
						{
							err[loc[i]] ^= alpha_to[(z[j]+j*root[i])%nn];
						}
					}
					if (err[loc[i]]!=0)
					{
						err[loc[i]] = index_of[err[loc[i]]];
						q = 0;     /* form denominator of error term */
						for (j=0; j<l[u]; j++)
						{
							if (j!=i)
							{
								q += index_of[1^alpha_to[(loc[j]+root[i])%nn]];
							}
						}
						q = q % nn;
						err[loc[i]] = alpha_to[(err[loc[i]]-q+nn)%nn];
						 /*recd[i] must be in polynomial form */
						recd[loc[i]] ^= err[loc[i]];
					}
				}
			}
			else /* no. roots != degree of elp => >tt errors and cannot solve */
			{
				for (i=0; i<nn; i++)
				{
					/* could return error flag if desired */
					if (recd[i]!=-1)
					{
						/* convert recd[] to polynomial form */
						recd[i] = alpha_to[recd[i]];
					}
					else /* just output received codeword as is */
					{
						recd[i] = 0;
					}
				}
				ret = 0;
			}
		}
		else /* elp has degree has degree >tt hence cannot solve */
		{
			for (i=0; i<nn; i++) /* could return error flag if desired */
			{
				if (recd[i]!=-1) /* convert recd[] to polynomial form */
				{
					recd[i] = alpha_to[recd[i]];
				}
				else /* just output received codeword as is */
				{
					recd[i] = 0;
				}
			}
			ret = 0;
		}
    }
	else /* no non-zero syndromes => no errors: output received codeword */
	{
		for (i=0; i<nn; i++)
		{
			if (recd[i]!=-1) /* convert recd[] to polynomial form */
			{
				recd[i] = alpha_to[recd[i]];
			}
			else
			{
				recd[i] = 0;
			}
		}
	}

    for (i = 0; i < nn; i++)
	{
		b_recd[i] = (uint8_t)(recd[i]);
	}
    return ret;
}

/*!
  * @fn  void RS_Encode(uint8_t p_Data[RS_MESSAGE_SZ], uint8_t p_Out[RS_PARITY_SZ])
  * @brief  This function generate the parity word of the given message
  *
  * @param [in]  p_Data    Pointer on message (source data).
  *
  * @param [out] p_Out     Pointer on parity (destination).
  *
  */
void RS_Encode(uint8_t p_Data[RS_MESSAGE_SZ], uint8_t p_Out[RS_PARITY_SZ])
/* take the string of symbols in data[i], i=0..(k-1) and encode systematically
   to produce 2*tt parity symbols in output[0]..output[2*tt-1]
   data[] is input and output[] is output in polynomial form.
   Encoding is done by using a feedback shift register with appropriate
   connections specified by the elements of gg[], which was generated above.
   Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)          */
{
   register int32_t i,j;
   int32_t feedback;

   for (i = 0; i < (nn-kk); i++)
   {
	   p_Out[i] = 0;
   }

   for (i = (kk-1); i >= 0; i--)
   {
      feedback = index_of[ p_Data[i]^p_Out[nn-kk-1] ];

      if (feedback != -1)
      {
         for (j = (nn-kk-1); j > 0; j--)
         {
            if (gg[j] != -1)
            {
            	p_Out[j] = p_Out[j-1]^alpha_to[ (gg[j]+feedback)%nn ];
            }
            else
            {
            	p_Out[j] = p_Out[j-1];
            }
         }
         p_Out[0] = alpha_to[ (gg[0]+feedback)%nn ];
      }
      else
      {
         for (j = (nn-kk-1); j > 0; j--)
         {
        	 p_Out[j] = p_Out[j-1];
         }
         p_Out[0] = 0;
      }
   }
}

#ifdef __cplusplus
}
#endif

/*! @} */
/*! @} */
/*! @} */
