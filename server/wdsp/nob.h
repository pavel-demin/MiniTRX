/*  nob.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013, 2014 Warren Pratt, NR0V

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

#ifndef _anb_h
#define _anb_h

typedef struct _anb
{
	int run;
	int buffsize;					// size of input/output buffer
	float* in;						// input buffer
	float* out;					// output buffer
	int dline_size;					// length of delay line which is 'float dline[length][2]'
	float *dline;					// pointer to delay line
	float samplerate;				// samplerate, used to convert times into sample counts
	float tau;						// transition time, signal<->zero
	float hangtime;				// time to stay at zero after noise is no longer detected
	float advtime;					// deadtime (zero output) in advance of detected noise
	float backtau;					// time constant used in averaging the magnitude of the input signal
	float threshold;				// triggers if (noise > threshold * average_signal_magnitude)
    float *wave;                   // pointer to array holding transition waveform
    int state;                      // state of the state machine
    float avg;                     // average value of the signal magnitude
    int dtime;                      // count when decreasing the signal magnitude
    int htime;                      // count when hanging
    int itime;                      // count when increasing the signal magnitude
    int atime;                      // count at zero before the noise burst (advance count)
    float coef;					// parameter in calculating transition waveform
    int trans_count;                // number of samples to equal 'tau' time
    int hang_count;                 // number of samples to equal 'hangtime' time
    int adv_count;                  // number of samples to equal 'advtime' time
    int in_idx;                     // ring buffer position into which new samples are inserted
    int out_idx;                    // ring buffer position from which delayed samples are pulled
    float power;					// level at which signal was increasing when a new decrease is started
    int count;						// set each time a noise sample is detected, counts down
    float backmult;				// multiplier for waveform averaging
    float ombackmult;				// multiplier for waveform averaging
	float *legacy;																										////////////  legacy interface - remove
} anb, *ANB;

 ANB create_anb	(
	int run,
	int buffsize,
	float* in,
	float* out,
	float samplerate,
	float tau,
	float hangtime,
	float advtime,
	float backtau,
	float threshold
						);

 void destroy_anb (ANB a);

 void flush_anb (ANB a);

 void xanb (ANB a);

extern  void create_anbEXT	(
	int id,
	int run,
	int buffsize,
	float samplerate,
	float tau,
	float hangtime,
	float advtime,
	float backtau,
	float threshold
					);

extern  void destroy_anbEXT (int id);

extern  void flush_anbEXT (int id);

extern  void xanbEXT (int id, float* in, float* out);


extern  void pSetRCVRANBRun (ANB a, int run);

extern  void pSetRCVRANBBuffsize (ANB a, int size);

extern  void pSetRCVRANBSamplerate (ANB a, int rate);

extern  void pSetRCVRANBTau (ANB a, float tau);

extern  void pSetRCVRANBHangtime (ANB a, float time);

extern  void pSetRCVRANBAdvtime (ANB a, float time);

extern  void pSetRCVRANBBacktau (ANB a, float tau);

extern  void pSetRCVRANBThreshold (ANB a, float thresh);

#endif