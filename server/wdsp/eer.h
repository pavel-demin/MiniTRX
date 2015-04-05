/*  eer.h

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

#ifndef _eer_h
#define _eer_h

typedef struct _eer
{
	int run;
	int amiq;
	int size;
	float* in;
	float* out;
	float* outM;
	int rate;
	float mgain;
	float pgain;
	int rundelays;
	float mdelay;
	float pdelay;
	DELAY mdel;
	DELAY pdel;
	float *legacy;																										////////////  legacy interface - remove
	float *legacyM;																									////////////  legacy interface - remove
} eer, *EER;

 EER create_eer (int run, int size, float* in, float* out, float* outM, int rate, float mgain, float pgain, int rundelays, float mdelay, float pdelay, int amiq);

 void destroy_eer (EER a);

 void flush_eer (EER a);

 void xeer (EER a);

 void pSetEERRun (EER a, int run);

 void pSetEERAMIQ (EER a, int amiq);

 void pSetEERMgain (EER a, float gain);

 void pSetEERPgain (EER a, float gain);

 void pSetEERRunDelays (EER a, int run);

 void pSetEERMdelay (EER a, float delay);

 void pSetEERPdelay (EER a, float delay);

 void pSetEERSize (EER a, int size);

 void pSetEERSamplerate (EER a, int rate);

#endif