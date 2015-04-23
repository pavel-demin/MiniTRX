/*  eq.c

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

float* eq_mults (int size, int nfreqs, float* F, float* G, float samplerate, float scale, int ctfmode, int method)
{
	float* fp = (float *) malloc0 ((nfreqs + 2)   * sizeof (float));
	float* gp = (float *) malloc0 ((nfreqs + 2)   * sizeof (float));
	float* A  = (float *) malloc0 ((size / 2 + 1) * sizeof (float));
	float gpreamp, f, frac;
	float* impulse;
	float* mults;
	int i, j, mid;
	fp[0] = 0.0;
	fp[nfreqs + 1] = 1.0;
	gp[0] = G[1];
	gp[nfreqs + 1] = G[nfreqs];
	gpreamp = G[0];
	for (i = 1; i <= nfreqs; i++)
	{
		fp[i] = 2.0 * F[i] / samplerate;
		gp[i] = G[i];
	}
	mid = size / 2;
	j = 0;
	for (i = 0; i <= mid; i++)
	{
		f = (float)i / (float)mid;
		while (f > fp[j + 1]) j++;
		frac = (f - fp[j]) / (fp[j + 1] - fp[j]);
		A[i] = pow (10.0, 0.05 * (frac * gp[j + 1] + (1.0 - frac) * gp[j] + gpreamp)) * scale;
	}
	if (ctfmode == 0)
	{
		int k;
		int low = (int)(fp[1] * mid);
		int high = (int)(fp[nfreqs] * mid + 0.5);
		float lowmag = A[low];
		float highmag = A[high];
		float flow4 = pow((float)low / (float)mid, 4.0);
		float fhigh4 = pow((float)high / (float)mid, 4.0);
		k = low;
		while (--k >= 0)
		{
			f = (float)k / (float)mid;
			lowmag *= (f * f * f * f) / flow4;
			if (lowmag < 1.0e-100) lowmag = 1.0e-100;
			A[k] = lowmag;
		}
		k = high;
		while (++k <= mid)
		{
			f = (float)k / (float)mid;
			highmag *= fhigh4 / (f * f * f * f);
			if (highmag < 1.0e-100) highmag = 1.0e-100;
			A[k] = highmag;
		}
	}
	if (method == 0)
		impulse = fir_fsamp(size + 1, A, 1, 1.0);
	else
		impulse = fir_fsamp_odd(size + 1, A, 1, 1.0);
	mults = fftcv_mults(2 * size, impulse);
	_aligned_free (impulse);
	_aligned_free (A);
	_aligned_free (gp);
	_aligned_free (fp);
	return mults;
}

EQ create_eq (int run, int size, float *in, float *out, int nfreqs, float* F, float* G, int ctfmode, int method, int samplerate)
{
	EQ a = (EQ) malloc0 (sizeof (eq));
	a->run = run;
	a->size = size;
	a->in = in;
	a->out = out;
	a->nfreqs = nfreqs;
	a->F = F;
	a->G = G;
	a->ctfmode = ctfmode;
	a->method = method;
	a->samplerate = (float)samplerate;
	a->scale = 1.0 / (float)(2 * a->size);
	a->infilt  = (float *) malloc0 (2 * a->size * sizeof (complex));
	a->product = (float *) malloc0 (2 * a->size * sizeof (complex));
	a->CFor = fftwf_plan_dft_1d(2 * a->size, (fftwf_complex *)a->infilt,  (fftwf_complex *)a->product, FFTW_FORWARD,  FFTW_PATIENT);
	a->CRev = fftwf_plan_dft_1d(2 * a->size, (fftwf_complex *)a->product, (fftwf_complex *)a->out, FFTW_BACKWARD, FFTW_PATIENT);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	return a;
}

void destroy_eq (EQ a)
{
	fftwf_destroy_plan (a->CRev);
	fftwf_destroy_plan (a->CFor);
	_aligned_free (a->mults);
	_aligned_free (a->product);
	_aligned_free (a->infilt);
	_aligned_free (a);
}

void flush_eq (EQ a)
{
	memset (a->infilt, 0, 2 * a->size * sizeof (complex));
}

void xeq (EQ a)
{
	int i;
	float I, Q;
	if (a->run)
	{
		memcpy (&(a->infilt[2 * a->size]), a->in, a->size * sizeof (complex));
		fftwf_execute (a->CFor);
		for (i = 0; i < 2 * a->size; i++)
		{
			I = a->product[2 * i + 0];
			Q = a->product[2 * i + 1];
			a->product[2 * i + 0] = I * a->mults[2 * i + 0] - Q * a->mults[2 * i + 1];
			a->product[2 * i + 1] = I * a->mults[2 * i + 1] + Q * a->mults[2 * i + 0];
		}
		fftwf_execute (a->CRev);
		memcpy (a->infilt, &(a->infilt[2 * a->size]), a->size * sizeof(complex));
	}
	else if (a->in != a->out)
		memcpy (a->out, a->in, a->size * sizeof (complex));
}

/********************************************************************************************************
*																										*
*											RXA Properties												*
*																										*
********************************************************************************************************/

PORT
void SetRXAEQRun (int channel, int run)
{
	EnterCriticalSection (&ch[channel].csDSP);
	rxa[channel].eq.p->run = run;
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetRXAEQProfile (int channel, int nfreqs, float* F, float* G)
{
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = rxa[channel].eq.p;
	a->nfreqs = nfreqs;
	a->F = F;
	a->G = G;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetRXAEQCtfmode (int channel, int mode)
{
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = rxa[channel].eq.p;
	a->ctfmode = mode;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetRXAEQMethod (int channel, int method)
{
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = rxa[channel].eq.p;
	a->method = method;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetRXAGrphEQ (int channel, int *rxeq)
{	// three band equalizer (legacy compatibility)
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = rxa[channel].eq.p;
	a->nfreqs = 4;
	a->F = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->G = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->F[1] =  150.0;
	a->F[2] =  400.0;
	a->F[3] = 1500.0;
	a->F[4] = 6000.0;
	a->G[0] = (float)rxeq[0];
	a->G[1] = (float)rxeq[1];
	a->G[2] = (float)rxeq[1];
	a->G[3] = (float)rxeq[2];
	a->G[4] = (float)rxeq[3];
	a->ctfmode = 0;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	_aligned_free (a->F);
	_aligned_free (a->G);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetRXAGrphEQ10 (int channel, int *rxeq)
{	// ten band equalizer (legacy compatibility)
	EQ a;
	int i;
	EnterCriticalSection (&ch[channel].csDSP);
	a = rxa[channel].eq.p;
	a->nfreqs = 10;
	a->F = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->G = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->F[1]  =    32.0;
	a->F[2]  =    63.0;
	a->F[3]  =   125.0;
	a->F[4]  =   250.0;
	a->F[5]  =   500.0;
	a->F[6]  =  1000.0;
	a->F[7]  =  2000.0;
	a->F[8]  =  4000.0;
	a->F[9]  =  8000.0;
	a->F[10] = 16000.0;
	for (i = 0; i <= a->nfreqs; i++)
		a->G[i] = (float)rxeq[i];
	a->ctfmode = 0;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	_aligned_free (a->F);
	_aligned_free (a->G);
	LeaveCriticalSection (&ch[channel].csDSP);
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

PORT
void SetTXAEQRun (int channel, int run)
{
	EnterCriticalSection (&ch[channel].csDSP);
	txa[channel].eq.p->run = run;
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXAEQProfile (int channel, int nfreqs, float* F, float* G)
{
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].eq.p;
	a->nfreqs = nfreqs;
	a->F = F;
	a->G = G;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXAEQCtfmode (int channel, int mode)
{
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].eq.p;
	a->ctfmode = mode;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXAEQMethod (int channel, int method)
{
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].eq.p;
	a->method = method;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXAGrphEQ (int channel, int *txeq)
{	// three band equalizer (legacy compatibility)
	EQ a;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].eq.p;
	a->nfreqs = 4;
	a->F = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->G = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->F[1] =  150.0;
	a->F[2] =  400.0;
	a->F[3] = 1500.0;
	a->F[4] = 6000.0;
	a->G[0] = (float)txeq[0];
	a->G[1] = (float)txeq[1];
	a->G[2] = (float)txeq[1];
	a->G[3] = (float)txeq[2];
	a->G[4] = (float)txeq[3];
	a->ctfmode = 0;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	_aligned_free (a->F);
	_aligned_free (a->G);
	LeaveCriticalSection (&ch[channel].csDSP);
}

PORT
void SetTXAGrphEQ10 (int channel, int *txeq)
{	// ten band equalizer (legacy compatibility)
	EQ a;
	int i;
	EnterCriticalSection (&ch[channel].csDSP);
	a = txa[channel].eq.p;
	a->nfreqs = 10;
	a->F = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->G = (float *) malloc0 ((a->nfreqs + 1) * sizeof (float));
	a->F[1]  =    32.0;
	a->F[2]  =    63.0;
	a->F[3]  =   125.0;
	a->F[4]  =   250.0;
	a->F[5]  =   500.0;
	a->F[6]  =  1000.0;
	a->F[7]  =  2000.0;
	a->F[8]  =  4000.0;
	a->F[9]  =  8000.0;
	a->F[10] = 16000.0;
	for (i = 0; i <= a->nfreqs; i++)
		a->G[i] = (float)txeq[i];
	a->ctfmode = 0;
	_aligned_free (a->mults);
	a->mults = eq_mults (a->size, a->nfreqs, a->F, a->G, a->samplerate, a->scale, a->ctfmode, a->method);
	_aligned_free (a->F);
	_aligned_free (a->G);
	LeaveCriticalSection (&ch[channel].csDSP);
}