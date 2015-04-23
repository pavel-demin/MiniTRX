/*  nobII.h

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

#ifndef _nob_h
#define _nob_h

typedef struct _nob
{
	int run;
	int buffsize;					// size of input/output buffer
	float* in;						// input buffer
	float* out;					// output buffer
	int mode;
	int dline_size;					// length of delay line which is 'float dline[length][2]'
	float *dline;					// pointer to delay line
	int *imp;
	float samplerate;				// samplerate, used to convert times into sample counts
	float advslewtime;						// transition time, signal<->zero
	float advtime;					// deadtime (zero output) in advance of detected noise
	float hangslewtime;
	float hangtime;				// time to stay at zero after noise is no longer detected
	float max_imp_seq_time;
	int filterlen;
	float *fcoefs;
	float *bfbuff;
	int bfb_in_idx;
	float *ffbuff;
	int ffb_in_idx;
	float backtau;					// time constant used in averaging the magnitude of the input signal
	float threshold;				// triggers if (noise > threshold * average_signal_magnitude)
    float *awave;                   // pointer to array holding transition waveform
	float *hwave;
    int state;                      // state of the state machine
    float avg;                     // average value of the signal magnitude
    int time;                       // count when decreasing the signal magnitude
	int adv_slew_count;
    int adv_count;                  // number of samples to equal 'tau' time
    int hang_count;                 // number of samples to equal 'hangtime' time
    int hang_slew_count;            // number of samples to equal 'advtime' time
	int max_imp_seq;
	int blank_count;
    int in_idx;                     // ring buffer position into which new samples are inserted
	int scan_idx;
    int out_idx;                    // ring buffer position from which delayed samples are pulled
    float backmult;				// multiplier for waveform averaging
    float ombackmult;				// multiplier for waveform averaging
	float I1, Q1;
	float I2, Q2;
	float I, Q;
	float Ilast, Qlast;
	float deltaI, deltaQ;
	float Inext, Qnext;
	int overflow;
	CRITICAL_SECTION cs_update;
	float *legacy;																										////////////  legacy interface - remove
} nob, *NOB;

__declspec (dllexport) NOB create_nob	(
	int run,
	int buffsize,
	float* in,
	float* out,
	float samplerate,
	int mode,
	float advslewtime,
	float advtime,
	float hangslewtime,
	float hangtime,
	float max_imp_seq_time,
	float backtau,
	float threshold
						);

__declspec (dllexport) void destroy_nob (NOB a);

__declspec (dllexport) void flush_nob (NOB a);

__declspec (dllexport) void xnob (NOB a);

extern __declspec (dllexport) void create_nobEXT	(
	int id,
	int run,
	int mode,
	int buffsize,
	float samplerate,
	float slewtime,
	float hangtime,
	float advtime,
	float backtau,
	float threshold
					);

extern __declspec (dllexport) void destroy_nobEXT (int id);

extern __declspec (dllexport) void flush_nobEXT (int id);

extern __declspec (dllexport) void xnobEXT (int id, float* in, float* out);


extern __declspec (dllexport) void pSetRCVRNOBRun (NOB a, int run);

extern __declspec (dllexport) void pSetRCVRNOBMode (NOB a, int mode);

extern __declspec (dllexport) void pSetRCVRNOBBuffsize (NOB a, int size);

extern __declspec (dllexport) void pSetRCVRNOBSamplerate (NOB a, int size);

extern __declspec (dllexport) void pSetRCVRNOBTau (NOB a, float tau);

extern __declspec (dllexport) void pSetRCVRNOBHangtime (NOB a, float time);

extern __declspec (dllexport) void pSetRCVRNOBAdvtime (NOB a, float time);

extern __declspec (dllexport) void pSetRCVRNOBBacktau (NOB a, float tau);

extern __declspec (dllexport) void pSetRCVRNOBThreshold (NOB a, float thresh);

#endif