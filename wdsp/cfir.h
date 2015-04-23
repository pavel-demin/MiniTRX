/*  cfir.h

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

#ifndef _cfir_h
#define _cfir_h

typedef struct _cfir
{
	int run;
	int size;
	float* in;
	float* out;
	int runrate;
	int cicrate; 
	int DD; 
	int R; 
	int Pairs; 
	float cutoff;
	float scale;
	int xtype;
	float xbw;
	float* infilt;
	float* product;
	fftwf_plan CFor;
	fftwf_plan CRev;
	float* mults;
} cfir, *CFIR;

extern CFIR create_cfir (int run, int size, float* in, float* out, int runrate, int cicrate, int DD, int R, int Pairs, float cutoff, int xtype, float xbw);

extern void destroy_cfir (CFIR a);

extern void flush_cfir (CFIR a);

extern void xcfir (CFIR a);

extern float *fir_read (int N, const char *filename, int rtype, float scale);

extern float* cfir_impulse (int N, int DD, int R, int Pairs, float runrate, float cicrate, float cutoff, int xtype, float xbw, int rtype, float scale);

extern float* cfir_mults (int size, int runrate, int cicrate, float scale, int DD, int R, int Pairs, float cutoff, int xtype, float xbw);

#endif