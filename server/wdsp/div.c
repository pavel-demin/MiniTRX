/*  div.c

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

*/

#include "comm.h"

#define MAX_NR	(8)		// maximum number of receivers to mix

MDIV create_div (int run, int nr, int size, float **in, float *out)
{
	int i;
	MDIV a = (MDIV) malloc0 (sizeof (mdiv));
	a->run = run;
	a->nr = nr;
	a->size = size;
	a->out = out;
	a->in = (float **) malloc0 ( MAX_NR * sizeof (float *));
	if (in != 0)
		for (i = 0; i < nr; i++) a->in[i] = in[i];
	a->Irotate = (float *) malloc0 (MAX_NR * sizeof (float));
	a->Qrotate = (float *) malloc0 (MAX_NR * sizeof (float));
	for (i = 0; i < 4; i++)																					///////////// legacy interface - remove
		a->legacy[i] = (float *) malloc0 (2048 * sizeof (complex));										///////////// legacy interface - remove
	return a;
}

void destroy_div (MDIV a)
{
	int i;																									///////////// legacy interface - remove
	for (i = 0; i < 4; i++)																					///////////// legacy interface - remove
		free (a->legacy[i]);																		///////////// legacy interface - remove
	free (a->Qrotate);
	free (a->Irotate);
	free (a->in);
}

void flush_div (MDIV a)
{

}

void xdiv (MDIV a)
{
	if (a->run)
	{
		if (a->output != a->nr)
		{
			if (a->out != a->in[a->output])
				memcpy (a->out, a->in[a->output], a->size * sizeof (complex));
		}
		else
		{
			int i, j;
			float I, Q;
			memset (a->out, 0, a->size * sizeof (complex));
			for (i = 0; i < a->nr; i++)
				for (j = 0; j < a->size; j++)
				{
					I = a->in[i][2 * j + 0];
					Q = a->in[i][2 * j + 1];
					a->out[2 * j + 0] += a->Irotate[i] * I - a->Qrotate[i] * Q;
					a->out[2 * j + 1] += a->Irotate[i] * Q + a->Qrotate[i] * I;
				}
		}
	}
	else
		memcpy (a->out, a->in[0], a->size * sizeof (complex));
}


/********************************************************************************************************
*																										*
*									    CALLS FOR EXTERNAL USE											*
*																										*
********************************************************************************************************/

#define MAX_EXT_DIVS	(2)							// maximum number of DIVs called from outside wdsp
MDIV pdiv[MAX_EXT_DIVS];	// array of pointers for DIVs used EXTERNAL to wdsp

void create_divEXT (int id, int run, int nr, int size)
{
	pdiv[id] = create_div (run, nr, size, 0, 0);
}

void destroy_divEXT (int id)
{
	destroy_div (pdiv[id]);
}

void flush_divEXT (int id)
{
	flush_div (pdiv[id]);
}

void xdivEXT (int id, int nsamples, float **in, float *out)
{
	int i;
	MDIV a = pdiv[id];
	a->size = nsamples;
	a->out = out;
	for (i = 0; i < a->nr; i++) a->in[i] = in[i];
	xdiv (a);
}

// 0 - does nothing; 1 - operates
void SetEXTDIVRun (int id, int run)
{
	MDIV a = pdiv[id];
	a->run = run;
}

// size of data buffer in complex samples
void SetEXTDIVBuffsize (int id, int size)
{
	MDIV a = pdiv[id];
	a->size = size;
}

// number of receivers being used for diversity
void SetEXTDIVNr (int id, int nr)
{
	MDIV a = pdiv[id];
	a->nr = nr;
}

// number of which receiver to output
//	if output==nr, mixing occurs
void SetEXTDIVOutput (int id, int output)
{
	MDIV a = pdiv[id];
	a->output = output;
}

// I and Q "rotate" multipliers for each receiver
//	can be set to 1.0 and 0.0 for "reference receiver"
void SetEXTDIVRotate (int id, int nr, float *Irotate, float *Qrotate)
{
	MDIV a = pdiv[id];
	memcpy (a->Irotate, Irotate, nr * sizeof (float));
	memcpy (a->Qrotate, Qrotate, nr * sizeof (float));
}

/********************************************************************************************************
*																										*
*									  LEGACY INTERFACE - REMOVE											*
*																										*
********************************************************************************************************/
void xdivEXTF (int id, int size, float **input, float *Iout, float *Qout)
{
	int i, j;
	MDIV a = pdiv[id];
	if (a->run)
	{
		a->size = size;
		for (i = 0; i < a->nr; i++)
		{
			for (j = 0; j < a->size; j++)
			{
				a->legacy[i][2 * j + 0] = (float)input[2 * i + 0][j];
				a->legacy[i][2 * j + 1] = (float)input[2 * i + 1][j];
			}
			a->in[i] = a->legacy[i];
		}
		a->out = a->legacy[3];
		xdiv (a);
		for (j = 0; j < a->size; j++)
		{
			Iout[j] = (float)a->legacy[3][2 * j + 0];
			Qout[j] = (float)a->legacy[3][2 * j + 1];
		}
	}
}