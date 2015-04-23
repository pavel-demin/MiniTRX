/*  bandpass.h

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

#ifndef _bandpass_h
#define _bandpass_h

typedef struct _bandpass
{
	int run;
	int position;
	int size;
	float* in;
	float* out;
	float f_low;
	float f_high;
	float* infilt;
	float* product;
	float* mults;
	float samplerate;
	int wintype;
	float gain;
	fftwf_plan CFor;
	fftwf_plan CRev;
}bandpass, *BANDPASS;

extern BANDPASS create_bandpass (int run, int position, int size, float* in, float* out, 
	float f_low, float f_high, int samplerate, int wintype, float gain);

extern void destroy_bandpass (BANDPASS a);

extern void flush_bandpass (BANDPASS a);

extern void xbandpass (BANDPASS a, int pos);

// RXA Prototypes

extern __declspec (dllexport) void SetRXABandpassRun (int channel, int run);

extern __declspec (dllexport) void SetRXABandpassFreqs (int channel, float low, float high);

// TXA Prototypes

extern __declspec (dllexport) void SetTXABandpassRun (int channel, int run);

extern __declspec (dllexport) void SetTXABandpassFreqs (int channel, float low, float high);

#endif