/*  patchpanel.c

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

PANEL create_panel (int channel, int run, int size, float* in, float* out, float gain1, float gain2I, float gain2Q, int inselect, int copy)
{
	PANEL a = (PANEL) malloc0 (sizeof (panel));
	a->channel = channel;
	a->run = run;
	a->size = size;
	a->in = in;
	a->out = out;
	a->gain1 = gain1;
	a->gain2I = gain2I;
	a->gain2Q = gain2Q;
	a->inselect = inselect;
	a->copy = copy;
	return a;
}

void destroy_panel (PANEL a)
{
	free (a);
}

void flush_panel (PANEL a)
{

}

void xpanel (PANEL a)
{
	int i;
	float I, Q;
	float gainI = a->gain1 * a->gain2I;
	float gainQ = a->gain1 * a->gain2Q;
	// inselect is either 0(neither), 1(Q), 2(I), or 3(both)
	switch (a->copy)
	{
	case 0:	// no copy
		for (i = 0; i < a->size; i++)
		{
			I = a->in[2 * i + 0] * (a->inselect >> 1);		
			Q = a->in[2 * i + 1] * (a->inselect &  1);
			a->out[2 * i + 0] = gainI * I;
			a->out[2 * i + 1] = gainQ * Q;
		}
		break;
	case 1:	// copy I to Q (then Q == I)
		for (i = 0; i < a->size; i++)
		{
			I = a->in[2 * i + 0] * (a->inselect >> 1);
			Q = I;
			a->out[2 * i + 0] = gainI * I;
			a->out[2 * i + 1] = gainQ * Q;
		}
		break;
	case 2:	// copy Q to I (then I == Q)
		for (i = 0; i < a->size; i++)
		{
			Q = a->in[2 * i + 1] * (a->inselect &  1);
			I = Q;
			a->out[2 * i + 0] = gainI * I;
			a->out[2 * i + 1] = gainQ * Q;
		}
		break;
	case 3:	// reverse (I=>Q and Q=>I)
		for (i = 0; i < a->size; i++)
		{
			Q = a->in[2 * i + 0] * (a->inselect >> 1);
			I = a->in[2 * i + 1] * (a->inselect &  1);
			a->out[2 * i + 0] = gainI * I;
			a->out[2 * i + 1] = gainQ * Q;
		}
		break;
	}
}

/********************************************************************************************************
*																										*
*											RXA Properties												*
*																										*
********************************************************************************************************/

void SetRXAPanelRun (int channel, int run)
{
	rxa[channel].panel.p->run = run;
}

void SetRXAPanelSelect (int channel, int select)
{
	rxa[channel].panel.p->inselect = select;
}

void SetRXAPanelGain1 (int channel, float gain)
{
	rxa[channel].panel.p->gain1 = gain;
}

void SetRXAPanelGain2 (int channel, float gainI, float gainQ)
{
	rxa[channel].panel.p->gain2I = gainI;
	rxa[channel].panel.p->gain2Q = gainQ;
}

void SetRXAPanelPan (int channel, float pan)
{
	float gain1, gain2;
	if (pan <= 0.5)
	{
		gain1 = 1.0;
		gain2 = sin (pan * PI);
	}
	else
	{
		gain1 = sin (pan * PI);
		gain2 = 1.0;
	}
	rxa[channel].panel.p->gain2I = gain1;
	rxa[channel].panel.p->gain2Q = gain2;
}

void SetRXAPanelCopy (int channel, int copy)
{
	rxa[channel].panel.p->copy = copy;
}

void SetRXAPanelBinaural (int channel, int bin)
{
	rxa[channel].panel.p->copy = 1 - bin;
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

void SetTXAPanelRun (int channel, int run)
{
	txa[channel].panel.p->run = run;
}

void SetTXAPanelGain1 (int channel, float gain)
{
	txa[channel].panel.p->gain1 = gain;
}
