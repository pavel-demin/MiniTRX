/*  div.h

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

#ifndef _div_h
#define _div_h

typedef struct _div
{
	int run;
	int nr;							// number of receivers to mix
	int size;						// size of input/output buffers
	float **in;					// input buffers
	float *out;					// output buffer
	int output;						// which rcvr to output; ==nr for mix
	float *Irotate;
	float *Qrotate;
	float *legacy[4];																	///////////// legacy interface - remove
} mdiv, *MDIV;

extern MDIV create_div (int run, int nr, int size, float **in, float *out);

extern void destroy_div (MDIV pdiv);

extern void xdiv (MDIV pdiv);

extern  void xdivEXT (int id, int nsamples, float **in, float *out);

extern  void create_divEXT (int id, int run, int nr, int size);

extern  void destroy_divEXT (int id);

#endif