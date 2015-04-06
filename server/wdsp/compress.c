/*  compress.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2011, 2013 Warren Pratt, NR0V

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The author can be reached by email at  

warren@wpratt.com

This software is based upon the algorithm described by Peter Martinez, G3PLX,
in the January 2010 issue of RadCom magazine.

*/

#include "comm.h"

COMPRESSOR create_compressor (
				int run,
				int buffsize,
				float* inbuff,
				float* outbuff,
				float gain )
{
	COMPRESSOR a;
	a = (COMPRESSOR) malloc0 (sizeof (compressor));
	a->run = run;
	a->inbuff = inbuff;
	a->outbuff = outbuff;
	a->buffsize = buffsize;
	a->gain = gain;

	return a;
}

void destroy_compressor (COMPRESSOR a)
{
	free (a);
}

void flush_compressor (COMPRESSOR a)
{

}

void xcompressor (COMPRESSOR a)
{
	int i;
	float mag;
	if (a->run)
		for (i = 0; i < a->buffsize; i++)
		{
			mag = sqrt(a->inbuff[2 * i + 0] * a->inbuff[2 * i + 0] + a->inbuff[2 * i + 1] * a->inbuff[2 * i + 1]);
			if (a->gain * mag > 1.0)
				a->outbuff[2 * i + 0] = a->inbuff[2 * i + 0] / mag;
			else
				a->outbuff[2 * i + 0] = a->inbuff[2 * i + 0] * a->gain;
			a->outbuff[2 * i + 1] = 0.0;
		}
	else if (a->inbuff != a->outbuff)
		memcpy(a->outbuff, a->inbuff, a->buffsize * sizeof (complex));
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

void
SetTXACompressorRun (int channel, int run)
{
	txa[channel].compressor.p->run = run;
	txa[channel].bp1.p->run = run;
}

void
SetTXACompressorGain (int channel, float gain)
{
	txa[channel].compressor.p->gain = pow (10.0, gain / 20.0);
}