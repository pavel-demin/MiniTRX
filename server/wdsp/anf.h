/*  anf.h

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

#ifndef _anf_h
#define _anf_h

#define ANF_DLINE_SIZE 2048

typedef struct _anf
{
	int run;
	int position;
	int buff_size;
	float *in_buff;
	float *out_buff;
	int dline_size;
	int mask;
	int n_taps;
	int delay;
	float two_mu;
	float gamma;
	float d [ANF_DLINE_SIZE];
	float w [ANF_DLINE_SIZE];
	int in_idx;

	float lidx;
	float lidx_min;
	float lidx_max;
	float ngamma;
	float den_mult;
	float lincr;
	float ldecr;
} anf, *ANF;

extern ANF create_anf	(
				int run,
				int position,
				int buff_size,
				float *in_buff,
				float *out_buff,
				int dline_size,
				int n_taps,
				int delay,
				float two_mu,
				float gamma,

				float lidx,
				float lidx_min,
				float lidx_max,
				float ngamma,
				float den_mult,
				float lincr,
				float ldecr
			);

extern void destroy_anf (ANF a);

extern void flush_anf (ANF a);

extern void xanf (ANF a, int position);

// RXA Properties

extern  void SetRXAANFRun (int channel, int setit);

extern  void SetRXAANFVals (int channel, int taps, int delay, float gain, float leakage);

extern  void SetRXAANFTaps (int channel, int taps);

extern  void SetRXAANFDelay (int channel, int delay);

extern  void SetRXAANFGain (int channel, float gain);

extern  void SetRXAANFLeakage (int channel, float leakage);

extern  void SetRXAANFPosition (int channel, int position);

#endif