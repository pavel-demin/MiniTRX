/*  shift.h

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

#ifndef _shift_h
#define _shift_h

typedef struct _shift
{
	int run;
	int size;
	float* in;
	float* out;
	float rate;
	float shift;
	float phase;
	float delta;
	float cos_delta;
	float sin_delta;
} shift, *SHIFT;

extern SHIFT create_shift (int run, int size, float* in, float* out, int rate, float fshift);

extern void destroy_shift (SHIFT a);

extern void flush_shift (SHIFT a);

extern void xshift (SHIFT a);

// RXA Properties

extern __declspec (dllexport) void SetRXAShiftRun (int channel, int run);

extern __declspec (dllexport) void SetRXAShiftFreq (int channel, float fshift);

#endif