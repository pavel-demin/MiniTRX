/*  fmmod.h

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

#ifndef _fmmod_h
#define _fmmod_h

typedef struct _fmmod
{
	int run;
	int size;
	float* in;
	float* out;
	float samplerate;
	float deviation;
	float f_low;
	float f_high;
	int ctcss_run;
	float ctcss_level;
	float ctcss_freq;
	// for ctcss gen
	float tscale;
	float tphase;
	float tdelta;
	// mod
	float sphase;
	float sdelta;
	// bandpass
	int bp_run;
	float bp_fc;
	float* bp_infilt;
	float* bp_product;
	float* bp_mults;
	fftwf_plan bp_CFor;
	fftwf_plan bp_CRev;
}fmmod, *FMMOD;

extern FMMOD create_fmmod (int run, int size, float* in, float* out, int rate, float dev, float f_low, float f_high, int ctcss_run, float ctcss_level, float ctcss_freq, int bp_run);

extern void destroy_fmmod (FMMOD a);

extern void flush_fmmod (FMMOD a);

extern void xfmmod (FMMOD a);

// TXA Properties

extern __declspec (dllexport) void SetTXAFMDeviation (int channel, float deviation);

extern __declspec (dllexport) void SetTXACTCSSFreq (int channel, float freq);

extern __declspec (dllexport) void SetTXACTCSSRun (int channel, int run);

#endif