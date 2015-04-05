/*  TXA.c

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

#include "comm.h"

void create_txa (int channel)
{
	txa[channel].mode = TXA_LSB;
	txa[channel].inbuff  = (float *) malloc0 (1 * ch[channel].dsp_insize  * sizeof (complex));
	txa[channel].outbuff = (float *) malloc0 (1 * ch[channel].dsp_outsize * sizeof (complex));
	txa[channel].midbuff = (float *) malloc0 (2 * ch[channel].dsp_size    * sizeof (complex));

	txa[channel].rsmpin.p = create_resample (
		1,											// run
		ch[channel].dsp_insize,						// input buffer size
		txa[channel].inbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer
		ch[channel].in_rate,						// input sample rate
		ch[channel].dsp_rate, 						// output sample rate
		0.0,										// select cutoff automatically
		0,											// select ncoef automatically
		1.0);										// gain

	txa[channel].gen0.p = create_gen (
		1,											// run
		ch[channel].dsp_size,						// buffer size
		txa[channel].midbuff,						// input buffer
		txa[channel].midbuff,						// output buffer
		ch[channel].dsp_rate,						// sample rate
		2);											// mode
	
	txa[channel].panel.p = create_panel (
		channel,									// channel number
		1,											// run
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer
		1.0,										// gain1
		1.0,										// gain2I
		1.0,										// gain2Q
		2,											// 1 to use Q, 2 to use I for input
		0);											// 0, no copy

	txa[channel].micmeter.p = create_meter (
		1,											// run
		0,											// optional pointer to another 'run'
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to buffer
		ch[channel].dsp_rate,						// samplerate
		0.100,										// averaging time constant
		0.100,										// peak decay time constant
		txa[channel].meter,							// result vector
		TXA_MIC_AV,									// index for average value
		TXA_MIC_PK,									// index for peak value
		-1,											// index for gain value
		0);											// pointer for gain computation

	txa[channel].amsq.p = create_amsq (
		1,											// run
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// input buffer
		txa[channel].midbuff,						// output buffer
		txa[channel].midbuff,						// trigger buffer
		ch[channel].dsp_rate,						// sample rate
		0.010,										// time constant for averaging signal
		0.004,										// up-slew time
		0.004,										// down-slew time
		0.180,										// signal level to initiate tail
		0.200,										// signal level to initiate unmute
		0.000,										// minimum tail length
		0.025,										// maximum tail length
		0.200);										// muted gain
	
	txa[channel].preemph.p = create_emph (
		1,											// run
		1,											// position
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// input buffer
		txa[channel].midbuff,						// output buffer,
		ch[channel].dsp_rate,						// sample rate
		0,											// pre-emphasis type
		300.0,										// f_low
		3000.0);									// f_high

	txa[channel].leveler.p = create_wcpagc (
		1,											// run - OFF by default
		5,											// mode
		0,											// 0 for max(I,Q), 1 for envelope
		txa[channel].midbuff,						// input buff pointer
		txa[channel].midbuff,						// output buff pointer
		ch[channel].dsp_size,						// io_buffsize
		ch[channel].dsp_rate,						// sample rate
		0.001,										// tau_attack
		0.500,										// tau_decay
		6,											// n_tau
		1.778,										// max_gain
		1.0,										// var_gain
		1.0,										// fixed_gain
		1.0,										// max_input
		1.05,										// out_targ
		0.250,										// tau_fast_backaverage
		0.005,										// tau_fast_decay
		5.0,										// pop_ratio
		0,											// hang_enable
		0.500,										// tau_hang_backmult
		0.500,										// hangtime
		2.000,										// hang_thresh
		0.100);										// tau_hang_decay

	txa[channel].lvlrmeter.p = create_meter (
		1,											// run
		&(txa[channel].leveler.p->run),				// pointer to leveler 'run'
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to buffer
		ch[channel].dsp_rate,						// samplerate
		0.100,										// averaging time constant
		0.100,										// peak decay time constant
		txa[channel].meter,							// result vector
		TXA_LVLR_AV,								// index for average value
		TXA_LVLR_PK,								// index for peak value
		TXA_LVLR_GAIN,								// index for gain value
		&txa[channel].leveler.p->gain);				// pointer for gain computation

	txa[channel].bp0.p = create_bandpass (
		1,											// always runs
		0,											// position
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer 
		32.0,										// low freq cutoff
		23000.0,									// high freq cutoff
		ch[channel].dsp_rate,						// samplerate
		1,											// wintype
		2.0);										// gain

	txa[channel].pfgain0.p = create_gain (
		1,											// run - depends upon mode
		0,											// 
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer
		0.5,										// Igain
		0.0);										// Qgain

	txa[channel].compressor.p = create_compressor (
		1,											// run - OFF by default
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer
		3.0);										// gain

	txa[channel].bp1.p = create_bandpass (
		1,											// ONLY RUNS WHEN COMPRESSOR IS USED
		0,											// position
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer 
		32.0,										// low freq cutoff
		23000.0,									// high freq cutoff
		ch[channel].dsp_rate,						// samplerate
		1,											// wintype
		2.0);										// gain	

	txa[channel].pfgain1.p = create_gain (
		1,											// run - depends upon mode
		&(txa[channel].compressor.p->run),			// run only if compressor is on
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer
		0.5,										// Igain
		0.0);										// Qgain


	txa[channel].osctrl.p = create_osctrl (
		1,											// run
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// input buffer
		txa[channel].midbuff,						// output buffer
		ch[channel].dsp_rate,						// sample rate
		1.95);										// gain for clippings

	txa[channel].bp2.p = create_bandpass (
		1,											// ONLY RUNS WHEN COMPRESSOR IS USED
		0,											// position
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer 
		32.0,										// low freq cutoff
		23000.0,									// high freq cutoff
		ch[channel].dsp_rate,						// samplerate
		1,											// wintype
		1.0);										// gain

	txa[channel].compmeter.p = create_meter (
		1,											// run
		&(txa[channel].compressor.p->run),			// pointer to compressor 'run'
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to buffer
		ch[channel].dsp_rate,						// samplerate
		0.100,										// averaging time constant
		0.100,										// peak decay time constant
		txa[channel].meter,							// result vector
		TXA_COMP_AV,								// index for average value
		TXA_COMP_PK,								// index for peak value
		-1,											// index for gain value
		0);											// pointer for gain computation

	txa[channel].alc.p = create_wcpagc (
		1,											// run - always ON
		5,											// mode
		1,											// 0 for max(I,Q), 1 for envelope
		txa[channel].midbuff,						// input buff pointer
		txa[channel].midbuff,						// output buff pointer
		ch[channel].dsp_size,						// io_buffsize
		ch[channel].dsp_rate,						// sample rate
		0.001,										// tau_attack
		0.010,										// tau_decay
		6,											// n_tau
		1.0,										// max_gain
		1.0,										// var_gain
		1.0,										// fixed_gain
		1.0,										// max_input
		1.0,										// out_targ
		0.250,										// tau_fast_backaverage
		0.005,										// tau_fast_decay
		5.0,										// pop_ratio
		0,											// hang_enable
		0.500,										// tau_hang_backmult
		0.500,										// hangtime
		2.000,										// hang_thresh
		0.100);										// tau_hang_decay

	txa[channel].ammod.p = create_ammod (
		1,											// run - OFF by default
		0,											// mode:  0=>AM, 1=>DSB
		ch[channel].dsp_size,						// size
		txa[channel].midbuff,						// pointer to input buffer
		txa[channel].midbuff,						// pointer to output buffer
		0.5);										// carrier level

	txa[channel].gen1.p = create_gen (
		0,											// run
		ch[channel].dsp_size,						// buffer size
		txa[channel].midbuff,						// input buffer
		txa[channel].midbuff,						// output buffer
		ch[channel].dsp_rate,						// sample rate
		0);											// mode

	txa[channel].outmeter.p = create_meter (
		1,											// run
		0,											// optional pointer to another 'run'
		ch[channel].dsp_outsize,					// size
		txa[channel].outbuff,						// pointer to buffer
		ch[channel].out_rate,						// samplerate
		0.100,										// averaging time constant
		0.100,										// peak decay time constant
		txa[channel].meter,							// result vector
		TXA_OUT_AV,									// index for average value
		TXA_OUT_PK,									// index for peak value
		-1,											// index for gain value
		0);											// pointer for gain computation

}

void destroy_txa (int channel)
{
	// in reverse order, free each item we created
	destroy_meter (txa[channel].outmeter.p);
	destroy_gen (txa[channel].gen1.p);
	destroy_ammod (txa[channel].ammod.p);
	destroy_wcpagc (txa[channel].alc.p);
	destroy_meter (txa[channel].compmeter.p);
	destroy_bandpass (txa[channel].bp2.p);
	destroy_osctrl (txa[channel].osctrl.p);
	destroy_gain (txa[channel].pfgain1.p);
	destroy_bandpass (txa[channel].bp1.p);
	destroy_compressor (txa[channel].compressor.p);
	destroy_gain (txa[channel].pfgain0.p);
	destroy_bandpass (txa[channel].bp0.p);
	destroy_meter (txa[channel].lvlrmeter.p);
	destroy_wcpagc (txa[channel].leveler.p);
	destroy_emph (txa[channel].preemph.p);
	destroy_amsq (txa[channel].amsq.p);
	destroy_meter (txa[channel].micmeter.p);
	destroy_panel (txa[channel].panel.p);
	destroy_gen (txa[channel].gen0.p);
	destroy_resample (txa[channel].rsmpin.p);
	free (txa[channel].midbuff);
	free (txa[channel].outbuff);
	free (txa[channel].inbuff);
}

void flush_txa (int channel)
{
	memset (txa[channel].inbuff,  0, 1 * ch[channel].dsp_insize  * sizeof (complex));
	memset (txa[channel].outbuff, 0, 1 * ch[channel].dsp_outsize * sizeof (complex));
	memset (txa[channel].midbuff, 0, 2 * ch[channel].dsp_size    * sizeof (complex));
	flush_resample (txa[channel].rsmpin.p);
	flush_gen (txa[channel].gen0.p);
	flush_panel (txa[channel].panel.p);
	flush_meter (txa[channel].micmeter.p);
	flush_amsq (txa[channel].amsq.p);
	flush_emph (txa[channel].preemph.p);
	flush_wcpagc (txa[channel].leveler.p);
	flush_meter (txa[channel].lvlrmeter.p);
	flush_bandpass (txa[channel].bp0.p);
	flush_gain (txa[channel].pfgain0.p);
	flush_compressor (txa[channel].compressor.p);
	flush_bandpass (txa[channel].bp1.p);
	flush_gain (txa[channel].pfgain1.p);
	flush_osctrl (txa[channel].osctrl.p);
	flush_bandpass (txa[channel].bp2.p);
	flush_meter (txa[channel].compmeter.p);
	flush_wcpagc (txa[channel].alc.p);
	flush_ammod (txa[channel].ammod.p);
	flush_gen (txa[channel].gen1.p);
	flush_meter (txa[channel].outmeter.p);
}

void xtxa (int channel)
{
	xresample (txa[channel].rsmpin.p);
	xgen (txa[channel].gen0.p);
	xpanel (txa[channel].panel.p);
	xmeter (txa[channel].micmeter.p);
	xamsqcap (txa[channel].amsq.p);
	xamsq (txa[channel].amsq.p);
	xemph (txa[channel].preemph.p, 0);
	xwcpagc (txa[channel].leveler.p);
	xmeter (txa[channel].lvlrmeter.p);
	xbandpass (txa[channel].bp0.p, 0);
	xgain (txa[channel].pfgain0.p);
	xcompressor (txa[channel].compressor.p);
	xbandpass (txa[channel].bp1.p, 0);
	xgain (txa[channel].pfgain1.p);
	xosctrl (txa[channel].osctrl.p);
	xbandpass (txa[channel].bp2.p, 0);
	xmeter (txa[channel].compmeter.p);
	xwcpagc (txa[channel].alc.p);
	xammod (txa[channel].ammod.p);
	xemph (txa[channel].preemph.p, 1);
	xgen (txa[channel].gen1.p);
	xmeter (txa[channel].outmeter.p);
}

/********************************************************************************************************
*																										*
*											TXA Properties												*
*																										*
********************************************************************************************************/

void SetTXAMode (int channel, int mode)
{
	txa[channel].mode = mode;
	txa[channel].ammod.p->run   = 0;
	txa[channel].pfgain0.p->run = 0;
	txa[channel].pfgain1.p->run = 0;
	txa[channel].preemph.p->run = 0;
	switch (mode)
	{
	case TXA_AM:
	case TXA_SAM:
		txa[channel].ammod.p->run   = 1;
		txa[channel].ammod.p->mode  = 0;
		txa[channel].pfgain0.p->run = 1;
		txa[channel].pfgain1.p->run = 1;
		break;
	case TXA_DSB:
		txa[channel].ammod.p->run   = 1;
		txa[channel].ammod.p->mode  = 1;
		txa[channel].pfgain0.p->run = 1;
		txa[channel].pfgain1.p->run = 1;
		break;
	default:

		break;
	}
}

