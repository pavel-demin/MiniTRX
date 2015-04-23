/*  fmd.h

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

#ifndef _fmd_h
#define _fmd_h
#include "iir.h"
typedef struct _fmd
{
	int run;
	int size;
	float* in;
	float* out;
	float rate;
	float f_low;						// audio low cutoff
	float f_high;						// audio high cutoff
	// pll
	float fmin;						// pll - minimum carrier freq to lock
	float fmax;						// pll - maximum carrier freq to lock
	float omega_min;					// pll - minimum lock check parameter
	float omega_max;					// pll - maximum lock check parameter
	float zeta;						// pll - damping factor; as coded, must be <=1.0
	float omegaN;						// pll - natural frequency
	float phs;							// pll - phase accumulator
	float omega;						// pll - locked pll frequency
	float fil_out;						// pll - filter output
	float g1, g2;						// pll - filter gain parameters
	float pllpole;						// pll - pole frequency
	// for dc removal
	float tau;
	float mtau;
	float onem_mtau;
	float fmdc;
	// pll audio gain
	float deviation;
	float again;
	float* audio;						// buffer to hold audio before de-emphasis
	// for de-emphasis filter
	float* infilt;
	float* product;
	float* mults;
	float* outfilt;
	fftwf_plan CFor;
	fftwf_plan CRev;
	// for audio filter
	float* ainfilt;
	float* aproduct;
	float* amults;
	float afgain;
	fftwf_plan aCFor;
	fftwf_plan aCRev;
	// CTCSS removal
	SNOTCH sntch;
	int sntch_run;
	float ctcss_freq;
} fmd, *FMD;

extern FMD create_fmd ( int run, int size, float* in, float* out, int rate, float deviation, float f_low, float f_high, float fmin, float fmax, float zeta, float omegaN, float tau, float afgain, int sntch_run, float ctcss_freq);

extern void destroy_fmd (FMD a);

extern void flush_fmd (FMD a);

extern void xfmd (FMD a);

// RXA Properties

extern __declspec (dllexport) void SetRXAFMDeviation (int channel, float deviation);

#endif