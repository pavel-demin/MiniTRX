/*  wcpAGC.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2011, 2012, 2013 Warren Pratt, NR0V

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

or by paper mail at

Warren Pratt
11303 Empire Grade
Santa Cruz, CA  95060

*/

#include "comm.h"
WCPAGC create_wcpagc (	int run,
						int mode,
						int pmode,
						float* in,
						float* out,
						int io_buffsize,
						int sample_rate,
						float tau_attack,
						float tau_decay,
						int n_tau,
						float max_gain,
						float var_gain,
						float fixed_gain,
						float max_input,
						float out_targ,
						float tau_fast_backaverage,
						float tau_fast_decay,
						float pop_ratio,
						int hang_enable,
						float tau_hang_backmult,
						float hangtime,
						float hang_thresh,
						float tau_hang_decay
					)
{
	WCPAGC a;
	a = (WCPAGC) malloc0 (sizeof(wcpagc));
	//initialize per call parameters
	a->run = run;
	a->mode = mode;
	a->pmode = pmode;
	a->in = in;
	a->out = out;
	a->io_buffsize = io_buffsize;
	a->sample_rate = (float)sample_rate;
	a->tau_attack = tau_attack;
	a->tau_decay = tau_decay;
	a->n_tau = n_tau;
	a->max_gain = max_gain;
	a->var_gain = var_gain;
	a->fixed_gain = fixed_gain;
	a->max_input = max_input;
	a->out_targ = out_targ;
	a->tau_fast_backaverage = tau_fast_backaverage;
	a->tau_fast_decay = tau_fast_decay;
	a->pop_ratio = pop_ratio;
	a->hang_enable = hang_enable;
	a->tau_hang_backmult = tau_hang_backmult;
	a->hangtime = hangtime;
	a->hang_thresh = hang_thresh;
	a->tau_hang_decay = tau_hang_decay;
	//assign constants
	a->ring_buffsize = RB_SIZE;
	//do one-time initialization
	a->out_index = -1;
	a->ring_max = 0.0;
	a->volts = 0.0;
	a->save_volts = 0.0;
	a->fast_backaverage = 0.0;
	a->hang_backaverage = 0.0;
	a->hang_counter = 0;
	a->decay_type = 0;
	a->state = 0;
	a->ring = (float *) malloc0 (RB_SIZE * sizeof (complex));
	a->abs_ring = (float *) malloc0 (RB_SIZE * sizeof (float));
	loadWcpAGC (a);

	return a;
}

void loadWcpAGC (WCPAGC a)
{
	float tmp;
	//calculate internal parameters
	a->attack_buffsize = (int)ceil(a->sample_rate * a->n_tau * a->tau_attack);
	a->in_index = a->attack_buffsize + a->out_index;
	a->attack_mult = 1.0 - exp(-1.0 / (a->sample_rate * a->tau_attack));
	a->decay_mult = 1.0 - exp(-1.0 / (a->sample_rate * a->tau_decay));
	a->fast_decay_mult = 1.0 - exp(-1.0 / (a->sample_rate * a->tau_fast_decay));
	a->fast_backmult = 1.0 - exp(-1.0 / (a->sample_rate * a->tau_fast_backaverage));
	a->onemfast_backmult = 1.0 - a->fast_backmult;

	a->out_target = a->out_targ * (1.0 - exp(-(float)a->n_tau)) * 0.9999;
	a->min_volts = a->out_target / (a->var_gain * a->max_gain);
	a->inv_out_target = 1.0 / a->out_target;

	tmp = log10(a->out_target / (a->max_input * a->var_gain * a->max_gain));
	if (tmp == 0.0)
		tmp = 1e-16;
	a->slope_constant = (a->out_target * (1.0 - 1.0 / a->var_gain)) / tmp;

	a->inv_max_input = 1.0 / a->max_input;

	tmp = pow (10.0, (a->hang_thresh - 1.0) / 0.125);
	a->hang_level = (a->max_input * tmp + (a->out_target / 
		(a->var_gain * a->max_gain)) * (1.0 - tmp)) * 0.637;

	a->hang_backmult = 1.0 - exp(-1.0 / (a->sample_rate * a->tau_hang_backmult));
	a->onemhang_backmult = 1.0 - a->hang_backmult;

	a->hang_decay_mult = 1.0 - exp(-1.0 / (a->sample_rate * a->tau_hang_decay));
}

void destroy_wcpagc (WCPAGC a)
{
	free (a->abs_ring);
	free (a->ring);
	free (a);
}

void flush_wcpagc (WCPAGC a)
{
	memset ((void *)a->ring, 0, sizeof(float) * RB_SIZE * 2);
	a->ring_max = 0.0;
	memset ((void *)a->abs_ring, 0, sizeof(float)* RB_SIZE);
}

void xwcpagc (WCPAGC a)
{
	int i, j, k;
	float mult;
	if (a->run)
	{
		if (a->mode == 0)
		{
			for (i = 0; i < a->io_buffsize; i++)
			{
				a->out[2 * i + 0] = a->fixed_gain * a->in[2 * i + 0];
				a->out[2 * i + 1] = a->fixed_gain * a->in[2 * i + 1];
			}
			return;
		}
	
		for (i = 0; i < a->io_buffsize; i++)
		{
			if (++a->out_index >= a->ring_buffsize)
				a->out_index -= a->ring_buffsize;
			if (++a->in_index >= a->ring_buffsize)
				a->in_index -= a->ring_buffsize;
	
			a->out_sample[0] = a->ring[2 * a->out_index + 0];
			a->out_sample[1] = a->ring[2 * a->out_index + 1];
			a->abs_out_sample = a->abs_ring[a->out_index];
			a->ring[2 * a->in_index + 0] = a->in[2 * i + 0];
			a->ring[2 * a->in_index + 1] = a->in[2 * i + 1];
			if (a->pmode == 0)
				a->abs_ring[a->in_index] = max(fabs(a->ring[2 * a->in_index + 0]), fabs(a->ring[2 * a->in_index + 1]));
			else
				a->abs_ring[a->in_index] = sqrt(a->ring[2 * a->in_index + 0] * a->ring[2 * a->in_index + 0] + a->ring[2 * a->in_index + 1] * a->ring[2 * a->in_index + 1]);

			a->fast_backaverage = a->fast_backmult * a->abs_out_sample + a->onemfast_backmult * a->fast_backaverage;
			a->hang_backaverage = a->hang_backmult * a->abs_out_sample + a->onemhang_backmult * a->hang_backaverage;

			if ((a->abs_out_sample >= a->ring_max) && (a->abs_out_sample > 0.0))
			{
				a->ring_max = 0.0;
				k = a->out_index;
				for (j = 0; j < a->attack_buffsize; j++)
				{
					if (++k == a->ring_buffsize)
						k = 0;
					if (a->abs_ring[k] > a->ring_max)
						a->ring_max = a->abs_ring[k];
				}
			}
			if (a->abs_ring[a->in_index] > a->ring_max)
				a->ring_max = a->abs_ring[a->in_index];

			if (a->hang_counter > 0)
				--a->hang_counter;

			switch (a->state)
			{
			case 0:
				{
					if (a->ring_max >= a->volts)
					{
						a->volts += (a->ring_max - a->volts) * a->attack_mult;
					}
					else
					{
						if (a->volts > a->pop_ratio * a->fast_backaverage)
						{
							a->state = 1;
							a->volts += (a->ring_max - a->volts) * a->fast_decay_mult;
						}
						else
						{
							if (a->hang_enable && (a->hang_backaverage > a->hang_level))
							{
								a->state = 2;
								a->hang_counter = (int)(a->hangtime * a->sample_rate);
								a->decay_type = 1;
							}
							else
							{
								a->state = 3;
								a->volts += (a->ring_max - a->volts) * a->decay_mult;
								a->decay_type = 0;
							}
						}
					}
					break;
				}
			case 1:
				{
					if (a->ring_max >= a->volts)
					{
						a->state = 0;
						a->volts += (a->ring_max - a->volts) * a->attack_mult;
					}
					else
					{
						if (a->volts > a->save_volts)
						{
							a->volts += (a->ring_max - a->volts) * a->fast_decay_mult;
						}
						else
						{
							if (a->hang_counter > 0)
							{
								a->state = 2;
							}
							else
							{
								if (a->decay_type == 0)
								{
									a->state = 3;
									a->volts += (a->ring_max - a->volts) * a->decay_mult;
								}
								else
								{
									a->state = 4;
									a->volts += (a->ring_max - a->volts) * a->hang_decay_mult;
								}
							}
						}
					}
					break;
				}
			case 2:
				{
					if (a->ring_max >= a->volts)
					{
						a->state = 0;
						a->save_volts = a->volts;
						a->volts += (a->ring_max - a->volts) * a->attack_mult;
					}
					else
					{
						if (a->hang_counter == 0)
						{
							a->state = 4;
							a->volts += (a->ring_max - a->volts) * a->hang_decay_mult;
						}
					}
					break;
				}
			case 3:
				{
					if (a->ring_max >= a->volts)
					{
						a->state = 0;
						a->save_volts = a->volts;
						a->volts += (a->ring_max - a->volts) * a->attack_mult;
					}
					else
					{
						a->volts += (a->ring_max - a->volts) * a->decay_mult;
					}
					break;
				}
			case 4:
				{
					if (a->ring_max >= a->volts)
					{
						a->state = 0;
						a->save_volts = a->volts;
						a->volts += (a->ring_max - a->volts) * a->attack_mult;
					}
					else
					{
						a->volts += (a->ring_max - a->volts) * a->hang_decay_mult;
					}
					break;
				}
			}

			if (a->volts < a->min_volts)
				a->volts = a->min_volts;
			a->gain = a->volts * a->inv_out_target;
			mult = (a->out_target - a->slope_constant * min (0.0, log10(a->inv_max_input * a->volts))) / a->volts;
			a->out[2 * i + 0] = a->out_sample[0] * mult;
			a->out[2 * i + 1] = a->out_sample[1] * mult;
		}
	}
	else if (a->out != a->in)
		memcpy(a->out, a->in, a->io_buffsize * sizeof (complex));
}

/********************************************************************************************************
*																										*
*											RXA Properties												*
*																										*
********************************************************************************************************/

void
SetRXAAGCMode (int channel, int mode)
{
	switch (mode)
	{
		case 0:	//agcOFF
			rxa[channel].agc.p->mode = 0;
			loadWcpAGC ( rxa[channel].agc.p );
			break;
		case 1:	//agcLONG
			rxa[channel].agc.p->mode = 1;
			rxa[channel].agc.p->hangtime = 2.000;
			rxa[channel].agc.p->tau_decay = 2.000;
			loadWcpAGC ( rxa[channel].agc.p );
			break;
		case 2:	//agcSLOW
			rxa[channel].agc.p->mode = 2;
			rxa[channel].agc.p->hangtime = 1.000;
			rxa[channel].agc.p->tau_decay = 0.500;
			loadWcpAGC ( rxa[channel].agc.p );
			break;
		case 3:	//agcMED
			rxa[channel].agc.p->mode = 3;
			rxa[channel].agc.p->hang_thresh = 1.0;
			rxa[channel].agc.p->hangtime = 0.000;
			rxa[channel].agc.p->tau_decay = 0.250;
			loadWcpAGC ( rxa[channel].agc.p );
			break;
		case 4:	//agcFAST
			rxa[channel].agc.p->mode = 4;
			rxa[channel].agc.p->hang_thresh = 1.0;
			rxa[channel].agc.p->hangtime = 0.000;
			rxa[channel].agc.p->tau_decay = 0.050;
			loadWcpAGC ( rxa[channel].agc.p );
			break;
		default:
			rxa[channel].agc.p->mode = 5;
			break;
	}
}

void
SetRXAAGCAttack (int channel, int attack)
{
	rxa[channel].agc.p->tau_attack = (float)attack / 1000.0;
	loadWcpAGC ( rxa[channel].agc.p );
}

void
SetRXAAGCDecay (int channel, int decay)
{
	rxa[channel].agc.p->tau_decay = (float)decay / 1000.0;
	loadWcpAGC ( rxa[channel].agc.p );
}

void
SetRXAAGCHang (int channel, int hang)
{
	rxa[channel].agc.p->hangtime = (float)hang / 1000.0;
	loadWcpAGC ( rxa[channel].agc.p );
}

void				
GetRXAAGCHangLevel(int channel, float *hangLevel)
//for line on bandscope
{
	*hangLevel = 20.0 * log10( rxa[channel].agc.p->hang_level / 0.637 );
}

void			
SetRXAAGCHangLevel(int channel, float hangLevel)
//for line on bandscope
{
	float convert, tmp;
	if (rxa[channel].agc.p->max_input > rxa[channel].agc.p->min_volts)
	{
		convert = pow (10.0, hangLevel / 20.0);
		tmp = max(1e-8, (convert - rxa[channel].agc.p->min_volts) / 
			(rxa[channel].agc.p->max_input - rxa[channel].agc.p->min_volts));
		rxa[channel].agc.p->hang_thresh = 1.0 + 0.125 * log10 (tmp);
	}
	else
		rxa[channel].agc.p->hang_thresh = 1.0;
	loadWcpAGC ( rxa[channel].agc.p );
}

void				
GetRXAAGCHangThreshold(int channel, int *hangthreshold)
//for slider in setup
{
	*hangthreshold = (int)(100.0 * rxa[channel].agc.p->hang_thresh);
}

void
SetRXAAGCHangThreshold (int channel, int hangthreshold)
//For slider in setup
{
	rxa[channel].agc.p->hang_thresh = (float)hangthreshold / 100.0;
	loadWcpAGC ( rxa[channel].agc.p );
}

void				
GetRXAAGCThresh(int channel, float *thresh, float size, float rate)
//for line on bandscope.
{
	float noise_offset;
	noise_offset = 10.0 * log10((rxa[channel].bp0.p->f_high - rxa[channel].bp0.p->f_low) 
		* size / rate);
	*thresh = 20.0 * log10( rxa[channel].agc.p->min_volts ) - noise_offset;
}

void				
SetRXAAGCThresh(int channel, float thresh, float size, float rate)
//for line on bandscope
{
	float noise_offset;
	noise_offset = 10.0 * log10((rxa[channel].bp0.p->f_high - rxa[channel].bp0.p->f_low) 
		* size / rate);
	rxa[channel].agc.p->max_gain = rxa[channel].agc.p->out_target / 
		(rxa[channel].agc.p->var_gain * pow (10.0, (thresh + noise_offset) / 20.0));
	loadWcpAGC ( rxa[channel].agc.p );
}

void			
GetRXAAGCTop(int channel, float *max_agc)
//for AGC Max Gain in setup
{
	*max_agc = 20 * log10 (rxa[channel].agc.p->max_gain);
}

void
SetRXAAGCTop (int channel, float max_agc)
//for AGC Max Gain in setup
{
	rxa[channel].agc.p->max_gain = pow (10.0, (float)max_agc / 20.0);
	loadWcpAGC ( rxa[channel].agc.p );
}

void
SetRXAAGCSlope (int channel, int slope)
{
	rxa[channel].agc.p->var_gain = pow (10.0, (float)slope / 20.0 / 10.0);
	loadWcpAGC ( rxa[channel].agc.p );
}

void
SetRXAAGCFixed (int channel, float fixed_agc)
{
	rxa[channel].agc.p->fixed_gain = pow (10.0, (float)fixed_agc / 20.0);
	loadWcpAGC ( rxa[channel].agc.p );
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

void
SetTXAALCSt (int channel, int state)
{
	txa[channel].alc.p->run = state;
}

void
SetTXAALCAttack (int channel, int attack)
{
	txa[channel].alc.p->tau_attack = (float)attack / 1000.0;
	loadWcpAGC(txa[channel].alc.p);
}
void
SetTXAALCDecay (int channel, int decay)
{
	txa[channel].alc.p->tau_decay = (float)decay / 1000.0;
	loadWcpAGC(txa[channel].alc.p);
}

void
SetTXAALCHang (int channel, int hang)
{
	txa[channel].alc.p->hangtime = (float)hang / 1000.0;
	loadWcpAGC(txa[channel].alc.p);
}

void
SetTXALevelerSt (int channel, int state)
{
	txa[channel].leveler.p->run = state;
}

void
SetTXALevelerAttack (int channel, int attack)
{
	txa[channel].leveler.p->tau_attack = (float)attack / 1000.0;
	loadWcpAGC(txa[channel].leveler.p);
}

void
SetTXALevelerDecay (int channel, int decay)
{
	txa[channel].leveler.p->tau_decay = (float)decay / 1000.0;
	loadWcpAGC(txa[channel].leveler.p);
}

void
SetTXALevelerHang (int channel, int hang)
{
	txa[channel].leveler.p->hangtime = (float)hang / 1000.0;
	loadWcpAGC(txa[channel].leveler.p);
}

void
SetTXALevelerTop (int channel, float maxgain)
{
	txa[channel].leveler.p->max_gain = pow (10.0,(float)maxgain / 20.0);
	loadWcpAGC(txa[channel].leveler.p);
}
