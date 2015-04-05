/*  meter.c

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

METER create_meter (int run, int* prun, int size, float* buff, int rate, float tau_av, float tau_decay, float* result, int enum_av, int enum_pk, int enum_gain, float* pgain)
{
	METER a = (METER) malloc0 (sizeof (meter));
	a->run = run;
	a->prun = prun;
	a->size = size;
	a->buff = buff;
	a->rate = (float)rate;
	a->tau_average = tau_av;
	a->tau_peak_decay = tau_decay;
	a->result = result;
	a->enum_av = enum_av;
	a->enum_pk = enum_pk;
	a->enum_gain = enum_gain;
	a->pgain = pgain;
	a->mult_average = exp (-1.0 / (a->rate * a->tau_average));
	a->mult_peak = exp (-1.0 / (a->rate * a->tau_peak_decay));
	flush_meter (a);
	return a;
}

void destroy_meter (METER a)
{
	free (a);
}

void flush_meter (METER a)
{
	a->avg = -400.0;
	a->peak = 0.0;
	a->result[a->enum_av] = -400.0;
	a->result[a->enum_pk] = -400.0;
	if ((a->pgain != 0) && (a->enum_gain >= 0))
		a->result[a->enum_gain] = -400.0;
}

void xmeter (METER a)
{
	int srun;
	if (a->prun != 0)
		srun = *(a->prun);
	else
		srun = 1;
	if (a->run && srun)
	{
		int i;
		float smag;
		float np = 0.0;
		for (i = 0; i < a->size; i++)
		{
			smag = a->buff[2 * i + 0] * a->buff[2 * i + 0] + a->buff[2 * i + 1] * a->buff[2 * i + 1];
			a->avg = a->avg * a->mult_average + (1.0 - a->mult_average) * 10.0 * mlog10 (smag + 1.0e-40);
			a->peak *= a->mult_peak;
			if (smag > np) np = smag;
		}
		if (np > a->peak) a->peak = np;
		a->result[a->enum_av] = a->avg;
		a->result[a->enum_pk] = 10.0 * mlog10 (a->peak + 1.0e-40);
		if ((a->pgain != 0) && (a->enum_gain >= 0))
			a->result[a->enum_gain] = 20.0 * mlog10 (*a->pgain + 1.0e-40);
	}
	else
	{
		if (a->enum_av   >= 0) a->result[a->enum_av]   = - 400.0;
		if (a->enum_pk   >= 0) a->result[a->enum_pk]   = - 400.0;
		if (a->enum_gain >= 0) a->result[a->enum_gain] = +   0.0;
	}
}

/********************************************************************************************************
*																										*
*											RXA Properties												*
*																										*
********************************************************************************************************/

float GetRXAMeter (int channel, int mt)
{
	float val;
	val = rxa[channel].meter[mt];
	return val;
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

float GetTXAMeter (int channel, int mt)
{
	float val;
	val = txa[channel].meter[mt];
	return val;
}
