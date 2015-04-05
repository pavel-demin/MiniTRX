/*  nob.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013, 2014 Warren Pratt, NR0V

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

*/

#include "comm.h"

#define MAX_TAU			(0.002)		// maximum transition time, signal<->zero
#define MAX_ADVTIME		(0.002)		// maximum deadtime (zero output) in advance of detected noise
#define MAX_SAMPLERATE  (384000)

void initBlanker(ANB a)
{
    int i;
	a->trans_count = (int)(a->tau * a->samplerate);
    a->hang_count = (int)(a->hangtime * a->samplerate);
    a->adv_count = (int)(a->advtime * a->samplerate);
    a->count = 0;
    a->in_idx = a->trans_count + a->adv_count;
    a->out_idx = 0;
    a->coef = PI / a->trans_count;
    a->state = 0;
    a->avg = 1.0;
    a->power = 1.0;
    a->backmult = exp(-1.0 / (a->samplerate * a->backtau));
    a->ombackmult = 1.0 - a->backmult;
    for (i = 0; i <= a->trans_count; i++)
        a->wave[i] = 0.5 * cos(i * a->coef);
	memset(a->dline, 0, a->dline_size * sizeof(complex));
}

ANB create_anb	(
	int run,
	int buffsize,
	float* in,
	float* out,
	float samplerate,
	float tau,
	float hangtime,
	float advtime,
	float backtau,
	float threshold
				)
{
	ANB a;
	a = (ANB) malloc0 (sizeof(anb));
	a->run = run;
	a->buffsize = buffsize;
	a->in = in;
	a->out = out;
	a->samplerate = samplerate;
	a->tau = tau;
	a->hangtime = hangtime;
	a->advtime = advtime;
	a->backtau = backtau;
	a->threshold = threshold;
	a->wave = (float *) malloc0 (((int)(MAX_SAMPLERATE * MAX_TAU) + 1) * sizeof(float));
	a->dline_size = (int)((MAX_TAU + MAX_ADVTIME) * MAX_SAMPLERATE) + 1;
	a->dline = (float *) malloc0 (a->dline_size * sizeof(complex));
	initBlanker(a);
	a->legacy = (float *) malloc0 (2048 * sizeof (complex));														/////////////// legacy interface - remove
	return a;
}

void destroy_anb (ANB a)
{ 
	free (a->legacy);																						/////////////// legacy interface - remove
	free (a->dline);
	free (a->wave);
	free (a);
}

void flush_anb (ANB a)
{
	initBlanker (a);
}

void xanb (ANB a)
{
    float scale;
    float mag;
	int i;
    if (a->run)
	{
		for (i = 0; i < a->buffsize; i++)
		{
			mag = sqrt(a->in[2 * i + 0] * a->in[2 * i + 0] + a->in[2 * i + 1] * a->in[2 * i + 1]);
			a->avg = a->backmult * a->avg + a->ombackmult * mag;
			a->dline[2 * a->in_idx + 0] = a->in[2 * i + 0];
			a->dline[2 * a->in_idx + 1] = a->in[2 * i + 1];
			if (mag > (a->avg * a->threshold))
				a->count = a->trans_count + a->adv_count;

			switch (a->state)
			{
				case 0:
					a->out[2 * i + 0] = a->dline[2 * a->out_idx + 0];
					a->out[2 * i + 1] = a->dline[2 * a->out_idx + 1];
					if (a->count > 0)
					{
						a->state = 1;
						a->dtime = 0;
						a->power = 1.0;
					}
					break;
				case 1:
					scale = a->power * (0.5 + a->wave[a->dtime]);
					a->out[2 * i + 0] = a->dline[2 * a->out_idx + 0] * scale;
					a->out[2 * i + 1] = a->dline[2 * a->out_idx + 1] * scale;
					if (++a->dtime > a->trans_count)
					{
						a->state = 2;
						a->atime = 0;
					}
					break;
				case 2:
					a->out[2 * i + 0] = 0.0;
					a->out[2 * i + 1] = 0.0;
					if (++a->atime > a->adv_count)
						a->state = 3;
					break;
				case 3:
					if (a->count > 0)
						a->htime = -a->count;
                                
					a->out[2 * i + 0] = 0.0;
					a->out[2 * i + 1] = 0.0;
					if (++a->htime > a->hang_count)
					{
						a->state = 4;
						a->itime = 0;
					}
					break;
				case 4:
					scale = 0.5 - a->wave[a->itime];
					a->out[2 * i + 0] = a->dline[2 * a->out_idx + 0] * scale;
					a->out[2 * i + 1] = a->dline[2 * a->out_idx + 1] * scale;
					if (a->count > 0)
					{
						a->state = 1;
						a->dtime = 0;
						a->power = scale;
					}
					else if (++a->itime > a->trans_count)
						a->state = 0;
					break;
			}
			if (a->count > 0) a->count--;
			if (++a->in_idx == a->dline_size) a->in_idx = 0; 
			if (++a->out_idx == a->dline_size) a->out_idx = 0;
		}
	}
	else if (a->in != a->out)
		memcpy (a->out, a->in, a->buffsize * sizeof (complex));
}


/********************************************************************************************************
*																										*
*											  RXA PROPERTIES											*
*																										*
********************************************************************************************************/

// The following is an example to follow for properties in the event that this function is used inside
// a channel.

// void SetRXAANBRun (int channel, int run)
// {
//	 ANB a = rxa[channel].anb.p;
//	 a->run = run;
// }

/********************************************************************************************************
*																										*
*										POINTER-BASED PROPERTIES										*
*																										*
********************************************************************************************************/

void pSetRCVRANBRun (ANB a, int run)
{
	a->run = run;
}

void pSetRCVRANBBuffsize (ANB a, int size)
{
	a->buffsize = size;
}

void pSetRCVRANBSamplerate (ANB a, int rate)
{
	a->samplerate = (float) rate;
	initBlanker (a);
}

void pSetRCVRANBTau (ANB a, float tau)
{
	a->tau = tau;
	initBlanker (a);
}

void pSetRCVRANBHangtime (ANB a, float time)
{
	a->hangtime = time;
	initBlanker (a);
}

void pSetRCVRANBAdvtime (ANB a, float time)
{
	a->advtime = time;
	initBlanker (a);
}

void pSetRCVRANBBacktau (ANB a, float tau)
{
	a->backtau = tau;
	initBlanker (a);
}

void pSetRCVRANBThreshold (ANB a, float thresh)
{
	a->threshold = thresh;
}

/********************************************************************************************************
*																										*
*									    CALLS FOR EXTERNAL USE											*
*																										*
********************************************************************************************************/

#define MAX_EXT_ANBS	(32)						// maximum number of NOBs called from outside wdsp
ANB panb[MAX_EXT_ANBS];		// array of pointers for NOBs used EXTERNAL to wdsp

void create_anbEXT	(
	int id,
	int run,
	int buffsize,
	float samplerate,
	float tau,
	float hangtime,
	float advtime,
	float backtau,
	float threshold
					)
{
	panb[id] = create_anb (run, buffsize, 0, 0,samplerate, tau, hangtime, advtime, backtau, threshold);
}

void destroy_anbEXT (int id)
{
	destroy_anb (panb[id]);
}

void flush_anbEXT (int id)
{
	flush_anb (panb[id]);
}

void xanbEXT (int id, float* in, float* out)
{
	ANB a = panb[id];
	a->in = in;
	a->out = out;
	xanb (a);
}

void SetEXTANBRun (int id, int run)
{
	ANB a = panb[id];
	a->run = run;
}

void SetEXTANBBuffsize (int id, int size)
{
	ANB a = panb[id];
	a->buffsize = size;
}

void SetEXTANBSamplerate (int id, int rate)
{
	ANB a = panb[id];
	a->samplerate = (float) rate;
	initBlanker (a);
}

void SetEXTANBTau (int id, float tau)
{
	ANB a = panb[id];
	a->tau = tau;
	initBlanker (a);
}

void SetEXTANBHangtime (int id, float time)
{
	ANB a = panb[id];
	a->hangtime = time;
	initBlanker (a);
}

void SetEXTANBAdvtime (int id, float time)
{
	ANB a = panb[id];
	a->advtime = time;
	initBlanker (a);
}

void SetEXTANBBacktau (int id, float tau)
{
	ANB a = panb[id];
	a->backtau = tau;
	initBlanker (a);
}

void SetEXTANBThreshold (int id, float thresh)
{
	ANB a = panb[id];
	a->threshold = thresh;
}

/********************************************************************************************************
*																										*
*											LEGACY INTERFACE											*
*																										*
********************************************************************************************************/

void xanbEXTF (int id, float *I, float *Q)
{
	int i;
	ANB a = panb[id];
	a->in = a->legacy;
	a->out = a->legacy;
	for (i = 0; i < a->buffsize; i++)
	{
		a->legacy[2 * i + 0] = (float)I[i];
		a->legacy[2 * i + 1] = (float)Q[i];
	}
	xanb (a);
	for (i = 0; i < a->buffsize; i++)
	{
		I[i] = (float)a->legacy[2 * i + 0];
		Q[i] = (float)a->legacy[2 * i + 1];
	}
}
