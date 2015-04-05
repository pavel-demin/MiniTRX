/*  eer.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2014 Warren Pratt, NR0V

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

NOTE:  THIS FILE IS CURRENTLY EXPERIMENTAL AND WILL CHANGE LATER.

*/

#include "comm.h"

EER create_eer (int run, int size, float* in, float* out, float* outM, int rate, float mgain, float pgain, int rundelays, float mdelay, float pdelay, int amiq)
{
	EER a = (EER) malloc0 (sizeof (eer));
	a->run = run;
	a->size = size;
	a->in = in;
	a->out = out;
	a->rate = rate;
	a->mgain = mgain;
	a->pgain = pgain;
	a->rundelays = rundelays;
	a->mdelay = mdelay;
	a->pdelay = pdelay;
	a->amiq = amiq;
	a->mdel = create_delay (
		a->rundelays,								// run
		a->size,									// size
		a->outM,									// input buffer
		a->outM,									// output buffer
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->mdelay);									// delay
	a->pdel = create_delay (
		a->rundelays,								// run
		a->size,									// size
		a->out,										// input buffer
		a->out,										// output buffer
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->pdelay);									// delay

	a->legacy  = (float *) malloc0 (2048 * sizeof (complex));														/////////////// legacy interface - remove
	a->legacyM = (float *) malloc0 (2048 * sizeof (complex));														/////////////// legacy interface - remove

	return a;
}

void destroy_eer (EER a)
{
	destroy_delay (a->pdel);
	destroy_delay (a->mdel);
	free (a);
}

void flush_eer (EER a)
{
	flush_delay (a->mdel);
	flush_delay (a->pdel);
}

void xeer (EER a)
{
	if (a->run)
	{
		int i;
		float I, Q, mag;
		for (i = 0; i < a->size; i++)
		{
			I = a->in[2 * i + 0];
			Q = a->in[2 * i + 1];
			a->outM[2 * i + 0] = I * a->mgain;
			a->outM[2 * i + 1] = Q * a->mgain;
			if(a->amiq)
			{
				a->out [2 * i + 0] = a->pgain * I;
				a->out [2 * i + 1] = a->pgain * Q;
			}
			else
			{
				mag = sqrt (I * I + Q * Q);
				a->out [2 * i + 0] = a->pgain * I / mag;
				a->out [2 * i + 1] = a->pgain * Q / mag;
			}
		}
		xdelay (a->mdel);
		xdelay (a->pdel);
	}
	else if (a->out != a->in)
		memcpy (a->out, a->in, a->size * sizeof (complex));
}

/********************************************************************************************************
*																										*
*									    CALLS FOR EXTERNAL USE											*
*																										*
********************************************************************************************************/

#define MAX_EXT_EERS	(2)							// maximum number of EERs called from outside wdsp
EER peer[MAX_EXT_EERS];		// array of pointers for EERs used EXTERNAL to wdsp


void create_eerEXT (int id, int run, int size, int rate, float mgain, float pgain, int rundelays, float mdelay, float pdelay, int amiq)
{
	peer[id] = create_eer (run, size, 0, 0, 0, rate, mgain, pgain, rundelays, mdelay, pdelay, amiq);
}

void destroy_eerEXT (int id)
{
	destroy_eer (peer[id]);
}

void flush_eerEXT (int id)
{
	flush_eer (peer[id]);
}

void SetEERRun (int id, int run)
{
	EER a = peer[id];
	a->run = run;
}

void SetEERAMIQ (int id, int amiq)
{
	EER a = peer[id];
	a->amiq = amiq;
}

void SetEERMgain (int id, float gain)
{
	EER a = peer[id];
	a->mgain = gain;
}

void SetEERPgain (int id, float gain)
{
	EER a = peer[id];
	a->pgain = gain;
}

void SetEERRunDelays (int id, int run)
{
	EER a = peer[id];
	a->rundelays = run;
	SetDelayRun (a->mdel, a->rundelays);
	SetDelayRun (a->pdel, a->rundelays);
}

void SetEERMdelay (int id, float delay)
{
	EER a = peer[id];
	a->mdelay = delay;
	SetDelayValue (a->mdel, a->mdelay);
}

void SetEERPdelay (int id, float delay)
{
	EER a = peer[id];
	a->pdelay = delay;
	SetDelayValue (a->pdel, a->pdelay);
}

void SetEERSize (int id, int size)
{
	EER a = peer[id];
	a->size = size;
	SetDelayBuffs (a->mdel, a->size, a->outM, a->outM);
	SetDelayBuffs (a->pdel, a->size, a->out, a->out);
}

void SetEERSamplerate (int id, int rate)
{
	EER a = peer[id];
	a->rate = rate;
	destroy_delay (a->mdel);
	destroy_delay (a->pdel);
	a->mdel = create_delay (
		a->rundelays,								// run
		a->size,									// size
		a->outM,									// input buffer
		a->outM,									// output buffer
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->mdelay);									// delay
	a->pdel = create_delay (
		a->rundelays,								// run
		a->size,									// size
		a->out,										// input buffer
		a->out,										// output buffer
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->pdelay);									// delay
}

/********************************************************************************************************
*																										*
*								            POINTER-BASED PROPERTIES	    							*
*																										*
********************************************************************************************************/

void pSetEERRun (EER a, int run)
{
	a->run = run;
}

void pSetEERAMIQ (EER a, int amiq)
{
	a->amiq = amiq;
}

void pSetEERMgain (EER a, float gain)
{
	a->mgain = gain;
}

void pSetEERPgain (EER a, float gain)
{
	a->pgain = gain;
}

void pSetEERRunDelays (EER a, int run)
{
	a->rundelays = run;
	SetDelayRun (a->mdel, a->rundelays);
	SetDelayRun (a->pdel, a->rundelays);
}

void pSetEERMdelay (EER a, float delay)
{
	a->mdelay = delay;
	SetDelayValue (a->mdel, a->mdelay);
}

void pSetEERPdelay (EER a, float delay)
{
	a->pdelay = delay;
	SetDelayValue (a->pdel, a->pdelay);
}

void pSetEERSize (EER a, int size)
{
	a->size = size;
	SetDelayBuffs (a->mdel, a->size, a->outM, a->outM);
	SetDelayBuffs (a->pdel, a->size, a->out, a->out);
}

void pSetEERSamplerate (EER a, int rate)
{
	a->rate = rate;
	destroy_delay (a->mdel);
	destroy_delay (a->pdel);
	a->mdel = create_delay (
		a->rundelays,								// run
		a->size,									// size
		a->outM,									// input buffer
		a->outM,									// output buffer
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->mdelay);									// delay
	a->pdel = create_delay (
		a->rundelays,								// run
		a->size,									// size
		a->out,										// input buffer
		a->out,										// output buffer
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->pdelay);									// delay
}


/********************************************************************************************************
*																										*
*										   Legacy Interface												*
*																										*
********************************************************************************************************/

void xeerEXTF (int id, float* inI, float* inQ, float* outI, float* outQ, float* outMI, float* outMQ, int mox, int size)
{
	EER a = peer[id];
	if (mox && a->run)
	{
		int i;
		a->in   = a->legacy;
		a->out  = a->legacy;
		a->outM = a->legacyM;
		a->size = size;
		SetDelayBuffs (a->mdel, a->size, a->outM, a->outM);
		SetDelayBuffs (a->pdel, a->size, a->out, a->out);
		for (i = 0; i < a->size; i++)
		{
			a->legacy[2 * i + 0] = (float)inI[i];
			a->legacy[2 * i + 1] = (float)inQ[i];
		}
		xeer (a);
		for (i = 0; i < a->size; i++)
		{
			outI[i]  = (float)a->legacy [2 * i + 0];
			outQ[i]  = (float)a->legacy [2 * i + 1];
			outMI[i] = (float)a->legacyM[2 * i + 0];
			outMQ[i] = (float)a->legacyM[2 * i + 1];
		}
	}
}