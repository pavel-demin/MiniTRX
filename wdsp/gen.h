/*  gen.h

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

#ifndef _gen_h
#define _gen_h

typedef struct _gen
{
	int run;					// run
	int size;					// number of samples per buffer
	float* in;					// input buffer (retained in case I want to mix in a generated signal)
	float* out;				// output buffer
	float rate;				// sample rate
	int mode;					
	struct _tone
	{
		float mag;
		float freq;
		float phs;
		float delta;
		float cosdelta;
		float sindelta;
	} tone;
	struct _tt
	{
		float mag1;
		float mag2;
		float f1;
		float f2;
		float phs1;
		float phs2;
		float delta1;
		float delta2;
		float cosdelta1;
		float cosdelta2;
		float sindelta1;
		float sindelta2;
	} tt;
	struct _noise
	{
		float mag;
	} noise;
	struct _sweep
	{
		float mag;
		float f1;
		float f2;
		float sweeprate;
		float phs;
		float dphs;
		float d2phs;
		float dphsmax;
	} sweep;
	struct _saw
	{
		float mag;
		float f;
		float period;
		float delta;
		float t;
	} saw;
	struct _tri
	{
		float mag;
		float f;
		float period;
		float half;
		float delta;
		float t;
		float t1;
	} tri;
	struct _pulse
	{
		float mag;
		float pf;
		float pdutycycle;
		float ptranstime;
		float* ctrans;
		int pcount;
		int pnon;
		int pntrans;
		int pnoff;
		float pperiod;
		float tf;
		float tphs;
		float tdelta;
		float tcosdelta;
		float tsindelta;
		int state;
	} pulse;
} gen, *GEN;

extern GEN create_gen (int run, int size, float* in, float* out, int rate, int mode);

extern void destroy_gen (GEN a);

extern void flush_gen (GEN a);

extern void xgen (GEN a);

// TXA Properties

extern __declspec (dllexport) void SetTXAgenRun (int channel, int run);

#endif