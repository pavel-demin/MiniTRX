/*  amd.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2012, 2013 Warren Pratt, NR0V

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

#ifndef _amd_h
#define _amd_h

// ff defines for sbdemod
#ifndef STAGES
#define STAGES		7
#endif

#ifndef OUT_IDX
#define OUT_IDX		(3 * STAGES)
#endif

typedef struct _amd
{
	int run;
	int buff_size;						// buffer size
	float *in_buff;					// pointer to input buffer
	float *out_buff;					// pointer to output buffer
	int mode;							// demodulation mode
	float sample_rate;					// sample rate
	float dc;							// dc component in demodulated output
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
	float tauR;						// carrier removal time constant
	float tauI;						// carrier insertion time constant
	float mtauR;						// carrier removal multiplier
	float onem_mtauR;					// 1.0 - carrier_removal_multiplier
	float mtauI;						// carrier insertion multiplier
	float onem_mtauI;					// 1.0 - carrier_insertion_multiplier
	float a[3 * STAGES + 3];			// Filter a variables
	float b[3 * STAGES + 3];			// Filter b variables
	float c[3 * STAGES + 3];			// Filter c variables
	float d[3 * STAGES + 3];			// Filter d variables
	float c0[STAGES];					// Filter coefficients - path 0
	float c1[STAGES];					// Filter coefficients - path 1
	float dsI;							// delayed sample, I path
	float dsQ;							// delayed sample, Q path
	float dc_insert;					// dc component to insert in output
	int sbmode;							// sideband mode
	int levelfade;						// Fade Leveler switch

}amd, *AMD;

extern AMD create_amd 
	(
	int run,
	int buff_size,
	float *in_buff,
	float *out_buff,
	int mode,
	int levelfade,
	int sbmode,
	int sample_rate,
	float fmin,
	float fmax,
	float zeta,
	float omegaN,
	float tauR,
	float tauI
	);

extern void init_amd (AMD a);

extern void destroy_amd (AMD a);

extern void flush_amd (AMD a);

extern void xamd (AMD a);

// RXA Properties

extern __declspec (dllexport) void SetRXAAMDRun(int channel, int run);

extern __declspec (dllexport) void SetRXAAMDSBMode(int channel, int sbmode);

extern __declspec (dllexport) void SetRXAAMDFadeLevel(int channel, int levelfade);

#endif