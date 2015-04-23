/*  fmmod.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013 Warren Pratt, NR0V

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

FMMOD create_fmmod (int run, int size, float* in, float* out, int rate, float dev, float f_low, float f_high, int ctcss_run, float ctcss_level, float ctcss_freq, int bp_run)
{
	FMMOD a = (FMMOD) malloc0 (sizeof (fmmod));
	float* impulse;
	// inputs
	a->run = run;
	a->size = size;
	a->in = in;
	a->out = out;
	a->samplerate = (float)rate;
	a->deviation = dev;
	a->f_low = f_low;
	a->f_high = f_high;
	a->ctcss_run = ctcss_run;
	a->ctcss_level = ctcss_level;
	a->ctcss_freq = ctcss_freq;
	// ctcss gen
	a->tscale = 1.0 / (1.0 + a->ctcss_level);
	a->tphase = 0.0;
	a->tdelta = TWOPI * a->ctcss_freq / a->samplerate;
	// mod
	a->sphase = 0.0;
	a->sdelta = TWOPI * a->deviation / a->samplerate;
	// bandpass
	a->bp_run = bp_run;
	a->bp_fc = a->deviation + a->f_high;
	impulse = fir_bandpass (a->size + 1, -a->bp_fc, +a->bp_fc, a->samplerate, 0, 1, 1.0 / (2 * a->size));
	a->bp_mults = fftcv_mults (2 * a->size, impulse);
	a->bp_infilt  = (float *) malloc0 (2 * a->size * sizeof (complex));
	a->bp_product = (float *) malloc0 (2 * a->size * sizeof (complex));
	a->bp_CFor = fftwf_plan_dft_1d(2 * a->size, (fftwf_complex *)a->bp_infilt,  (fftwf_complex *)a->bp_product, FFTW_FORWARD,  FFTW_PATIENT);
	a->bp_CRev = fftwf_plan_dft_1d(2 * a->size, (fftwf_complex *)a->bp_product, (fftwf_complex *)a->out, FFTW_BACKWARD, FFTW_PATIENT);
	_aligned_free (impulse);
	return a;
}

void destroy_fmmod (FMMOD a)
{
	fftwf_destroy_plan (a->bp_CRev);
	fftwf_destroy_plan (a->bp_CFor);
	_aligned_free (a->bp_product);
	_aligned_free (a->bp_infilt);
	_aligned_free (a->bp_mults);
	_aligned_free (a);
}

void flush_fmmod (FMMOD a)
{
	memset (a->bp_infilt, 0, 2 * a->size * sizeof (complex));
	a->tphase = 0.0;
	a->sphase = 0.0;
}

void xfmmod (FMMOD a)
{
	int i;
	float I, Q;
	float dp, magdp, peak;
	if (a->run)
	{
		peak = 0.0;
		for (i = 0; i < a->size; i++)
		{
			if (a->ctcss_run)
			{
				a->tphase += a->tdelta;
				if (a->tphase >= TWOPI) a->tphase -= TWOPI;
				a->out[2 * i + 0] = a->tscale * (a->in[2 * i + 0] + a->ctcss_level * cos (a->tphase));
			}
			dp = a->out[2 * i + 0] * a->sdelta;
			a->sphase += dp;
			if (a->sphase >= TWOPI) a->sphase -= TWOPI;
			if (a->sphase <   0.0 ) a->sphase += TWOPI;
			a->out[2 * i + 0] = 0.7071 * cos (a->sphase);
			a->out[2 * i + 1] = 0.7071 * sin (a->sphase);
			if ((magdp = dp) < 0.0) magdp = - magdp;
			if (magdp > peak) peak = magdp;
		}
		//print_deviation ("peakdev.txt", peak, a->samplerate);
		if (a->bp_run)
		{
			memcpy (&(a->bp_infilt[2 * a->size]), a->out, a->size * sizeof (complex));
			fftwf_execute (a->bp_CFor);
			for (i = 0; i < 2 * a->size; i++)
			{
				I = a->bp_product[2 * i + 0];
				Q = a->bp_product[2 * i + 1];
				a->bp_product[2 * i + 0] = I * a->bp_mults[2 * i + 0] - Q * a->bp_mults[2 * i + 1];
				a->bp_product[2 * i + 1] = I * a->bp_mults[2 * i + 1] + Q * a->bp_mults[2 * i + 0];
			}
			fftwf_execute (a->bp_CRev);
			memcpy (a->bp_infilt, &(a->bp_infilt[2 * a->size]), a->size * sizeof(complex));
		}
	}
	else if (a->in != a->out)
		memcpy (a->out, a->in, a->size * sizeof (complex));
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

PORT
void SetTXAFMDeviation (int channel, float deviation)
{
	float* impulse;
	FMMOD a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].fmmod.p;
	a->deviation = deviation;
	// mod
	a->sphase = 0.0;
	a->sdelta = TWOPI * a->deviation / a->samplerate;
	// bandpass
	a->bp_fc = a->deviation + a->f_high;
	_aligned_free (a->bp_mults);
	impulse = fir_bandpass (a->size + 1, -a->bp_fc, +a->bp_fc, a->samplerate, 0, 1, 1.0 / (2 * a->size));
	a->bp_mults = fftcv_mults (2 * a->size, impulse);
	_aligned_free (impulse);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXACTCSSFreq (int channel, float freq)
{
	FMMOD a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].fmmod.p;
	a->ctcss_freq = freq;
	a->tphase = 0.0;
	a->tdelta = TWOPI * a->ctcss_freq / a->samplerate;
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXACTCSSRun (int channel, int run)
{
	EnterCriticalSection (&ch[channel].csDSP);
	txa[channel].fmmod.p->ctcss_run = run;
	LeaveCriticalSection (&ch[channel].csDSP);
}