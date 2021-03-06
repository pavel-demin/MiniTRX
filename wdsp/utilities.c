/*  utilities.c

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

#define _CRT_SECURE_NO_WARNINGS
#include "comm.h"

/********************************************************************************************************
*																										*
*											Required Utilities											*
*																										*
********************************************************************************************************/

PORT
void *malloc0 (int size)
{
	int alignment = 16;
	void* p = _aligned_malloc (size, alignment);
	if (p != 0) memset (p, 0, size);
	return p;
}

// Exported calls

#ifndef linux

PORT void
*NewCriticalSection()
{	// used by VAC
	LPCRITICAL_SECTION cs_ptr;
	cs_ptr = (LPCRITICAL_SECTION)calloc (1,sizeof (CRITICAL_SECTION));
	return (void *)cs_ptr;
}

PORT void
DestroyCriticalSection (LPCRITICAL_SECTION cs_ptr)
{	// used by VAC
	free ((char *)cs_ptr);
}

#endif

/********************************************************************************************************
*																										*
*										Test & Debug Utilities											*
*																										*
********************************************************************************************************/

void print_impulse (const char* filename, int N, float* impulse, int rtype, int pr_mode)
{
	int i;
	FILE* file;
	if (pr_mode == 0)
		file = fopen (filename, "w");
	else
		file = fopen (filename, "a");
	if (rtype == 0)
		for (i = 0; i < N; i++)
			fprintf (file, "%.17e\n", impulse[i]);
	else
		for (i = 0; i < N; i++)
			fprintf (file, "%.17e\t%.17e\n", impulse[2 * i + 0], impulse[2 * i + 1]);
	fprintf (file, "\n\n\n\n");
	fflush (file);
	fclose (file);
}

void print_peak_val (const char* filename, int N, float* buff, float thresh)
{
	int i;
	static unsigned int seqnum;
	float peak = 0.0;
	FILE* file;
	for (i = 0; i < N; i++)
		if (buff[i] > peak) peak = buff[i];
	if (peak >= thresh)
	{
		file = fopen(filename, "a");
		fprintf(file, "%d\t\t%.17e\n", seqnum, peak);
		fflush(file);
		fclose(file);
	}
	seqnum++;
}

void print_peak_env (const char* filename, int N, float* buff, float thresh)
{
	int i;
	static unsigned int seqnum;
	float peak = 0.0;
	float new_peak;
	FILE* file;
	for (i = 0; i < N; i++)
	{
		new_peak = sqrt (buff[2 * i + 0] * buff[2 * i + 0] + buff[2 * i + 1] * buff[2 * i + 1]);
		if (new_peak > peak) peak = new_peak;
	}
	if (peak >= thresh)
	{
		file = fopen (filename, "a");
		fprintf (file, "%d\t\t%.17e\n", seqnum, peak);
		fflush (file);
		fclose (file);
	}
	seqnum++;
}

void print_peak_env_f2 (const char* filename, int N, float* Ibuff, float* Qbuff)
{
	int i;
	float peak = 0.0;
	float new_peak;
	FILE* file = fopen (filename, "a");
	for (i = 0; i < N; i++)
	{
		new_peak = sqrt (Ibuff[i] * Ibuff[i] + Qbuff[i] * Qbuff[i]);
		if (new_peak > peak) peak = new_peak;
	}
	fprintf (file, "%.17e\n", peak);
	fflush (file);
	fclose (file);
}

void print_iqc_values (const char* filename, int state, float env_in, float I, float Q, float ym, float yc, float ys, float thresh)
{
	static unsigned int seqnum;
	float env_out;
	FILE* file;
	env_out = sqrt (I * I + Q * Q);
	if (env_out > thresh)
	{
		file = fopen(filename, "a");
		if (seqnum == 0)
			fprintf(file, "seqnum\tstate\tenv_in\t\tenv_out\t\tym\t\tyc\t\tys\n");
		fprintf(file, "%d\t%d\t%f\t%f\t%f\t%f\t%f\n", seqnum, state, env_in, env_out, ym, yc, ys);
		fflush(file);
		fclose(file);
		seqnum++;
	}
}

PORT
void print_buffer_parameters (const char* filename, int channel)
{
	IOB a = ch[channel].iob.pc;
	FILE* file = fopen (filename, "a");
	fprintf (file, "channel            = %d\n", channel);
	fprintf (file, "in_size            = %d\n", a->in_size);
	fprintf (file, "r1_outsize         = %d\n", a->r1_outsize);
	fprintf (file, "r1_size            = %d\n", a->r1_size);
	fprintf (file, "r2_size            = %d\n", a->r2_size);
	fprintf (file, "out_size           = %d\n", a->out_size);
	fprintf (file, "r2_insize          = %d\n", a->r2_insize);
	fprintf (file, "r1_active_buffsize = %d\n", a->r1_active_buffsize);
	fprintf (file, "f2_active_buffsize = %d\n", a->r2_active_buffsize);
	fprintf (file, "r1_inidx           = %d\n", a->r1_inidx);
	fprintf (file, "r1_outidx          = %d\n", a->r1_outidx);
	fprintf (file, "r1_unqueuedsamps   = %d\n", a->r1_unqueuedsamps);
	fprintf (file, "r2_inidx           = %d\n", a->r2_inidx);
	fprintf (file, "r2_outidx          = %d\n", a->r2_outidx);
	fprintf (file, "r2_havesamps       = %d\n", a->r2_havesamps);
	fprintf (file, "in_rate            = %d\n", ch[channel].in_rate);
	fprintf (file, "dsp_rate           = %d\n", ch[channel].dsp_rate);
	fprintf (file, "out_rate           = %d\n", ch[channel].out_rate);
	fprintf (file, "\n");
	fflush (file);
	fclose (file);
}

void print_meter (const char* filename, float* meter, int enum_av, int enum_pk, int enum_gain)
{
	FILE* file = fopen (filename, "a");
	if (enum_gain >= 0)
		fprintf (file, "%.4e\t%.4e\t%.4e\n", meter[enum_av], meter[enum_pk], meter[enum_gain]);
	else
		fprintf (file, "%.4e\t%.4e\n", meter[enum_av], meter[enum_pk]);
	fflush (file);
	fclose (file);
}

void print_message (const char* filename, const char* message, int p0, int p1, int p2)
{
	FILE* file = fopen (filename, "a");
	const char* msg = message;
	fprintf (file, "%s     %d     %d     %d\n", msg, p0, p1, p2);
	fflush (file);
	fclose (file);
}

void print_deviation (const char* filename, float dpmax, float rate)
{
	FILE* file = fopen (filename, "a");
	float peak = dpmax * rate / TWOPI;
	fprintf (file, "Peak Dev = %.4f\n", peak);
	fflush (file);
	fclose (file);
}

void __cdecl CalccPrintSamples (void *pargs)
{
	int i;
	float env_tx, env_rx;
	int channel = (int)pargs;
	CALCC a = txa[channel].calcc.p;
	FILE* file = fopen("samples.txt", "w");
	fprintf (file, "\n");
	for (i = 0; i < a->nsamps; i++)
	{
		env_tx = sqrt(a->txs[2 * i + 0] * a->txs[2 * i + 0] + a->txs[2 * i + 1] * a->txs[2 * i + 1]);
		env_rx = sqrt(a->rxs[2 * i + 0] * a->rxs[2 * i + 0] + a->rxs[2 * i + 1] * a->rxs[2 * i + 1]);
		fprintf(file, "%.12f  %.12f  %.12f      %.12f  %.12f  %.12f\n", 
			a->txs[2 * i + 0], a->txs[2 * i + 1], env_tx,
			a->rxs[2 * i + 0], a->rxs[2 * i + 1], env_rx);
	}
	fflush(file);
	fclose(file);
	_endthread();
}

void doCalccPrintSamples(int channel)
{	// no sample buffering - use in single cal mode
	_beginthread(CalccPrintSamples, 0, (void *)channel);
}

void print_anb_parms (const char* filename, ANB a)
{
	FILE* file = fopen (filename, "a");
	fprintf (file, "Run         = %d\n", a->run);
	fprintf (file, "Buffer Size = %d\n", a->buffsize);
	fprintf (file, "Sample Rate = %d\n", (int)a->samplerate);
	fprintf (file, "Threshold   = %.6f\n", a->threshold);
	fprintf (file, "BackTau     = %.6f\n", a->backtau);
	fprintf (file, "BackMult    = %.6f\n", a->backmult);
	fprintf (file, "Tau         = %.6f\n", a->tau);
	fflush (file);
	fclose (file);
}
