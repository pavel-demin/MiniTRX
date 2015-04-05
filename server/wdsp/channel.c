/*  channel.c

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

#include "comm.h"

void build_channel (int channel)
{
	if (ch[channel].in_rate  >= ch[channel].dsp_rate)
		ch[channel].dsp_insize  = ch[channel].dsp_size * (ch[channel].in_rate  / ch[channel].dsp_rate);
	else
		ch[channel].dsp_insize  = ch[channel].dsp_size / (ch[channel].dsp_rate /  ch[channel].in_rate);

	if (ch[channel].out_rate >= ch[channel].dsp_rate)
		ch[channel].dsp_outsize = ch[channel].dsp_size * (ch[channel].out_rate / ch[channel].dsp_rate);
	else
		ch[channel].dsp_outsize = ch[channel].dsp_size / (ch[channel].dsp_rate / ch[channel].out_rate);

	switch (ch[channel].type)
	{
	case 0:
		create_rxa (channel);
		break;
	case 1:
		create_txa (channel);
		break;
	}
}

void OpenChannel (int channel, int dsp_size, int input_samplerate, int dsp_rate, int output_samplerate, int type, int state, float tdelayup, float tslewup, float tdelaydown, float tslewdown, int bfo)
{
	ch[channel].dsp_size = dsp_size;
	ch[channel].in_rate = input_samplerate;
	ch[channel].dsp_rate = dsp_rate;
	ch[channel].out_rate = output_samplerate;
	ch[channel].type = type;
	ch[channel].state = state;
	ch[channel].tdelayup = tdelayup;
	ch[channel].tslewup = tslewup;
	ch[channel].tdelaydown = tdelaydown;
	ch[channel].tslewdown = tslewdown;
	ch[channel].bfo = bfo;
	build_channel (channel);
}

void CloseChannel (int channel)
{
	switch (ch[channel].type)
	{
	case 0:
		destroy_rxa (channel);
		break;
	case 1:
		destroy_txa (channel);
		break;
	}
}

void flushChannel (void* p)
{
	int channel = (int)p;
	switch (ch[channel].type)
	{
	case 0:
		flush_rxa (channel);
		break;
	case 1:
		flush_txa (channel);
		break;
	}
}

/********************************************************************************************************
*																										*
*										Channel Properties												*
*																										*
********************************************************************************************************/

void SetType (int channel, int type)
{	// no need to rebuild buffers; but we did anyway
	if (type != ch[channel].type)
	{
		CloseChannel (channel);
		ch[channel].type = type;
		build_channel (channel);
	}
}

void SetInputSamplerate (int channel, int in_rate)
{
	if (in_rate != ch[channel].in_rate)
	{
		CloseChannel (channel);
		ch[channel].in_rate = in_rate;
		build_channel (channel);
	}
}

void SetDSPSamplerate (int channel, int dsp_rate)
{
	if (dsp_rate != ch[channel].dsp_rate)
	{
		CloseChannel (channel);
		ch[channel].dsp_rate = dsp_rate;
		build_channel (channel);
	}
}

void SetOutputSamplerate (int channel, int out_rate)
{
	if (out_rate != ch[channel].out_rate)
	{
		CloseChannel (channel);
		ch[channel].out_rate = out_rate;
		build_channel (channel);
	}
}

void SetAllRates (int channel, int in_rate, int dsp_rate, int out_rate)
{
	if ((in_rate != ch[channel].in_rate) || (dsp_rate != ch[channel].dsp_rate) || (out_rate != ch[channel].out_rate))
	{
		CloseChannel (channel);
		ch[channel].in_rate  = in_rate;
		ch[channel].dsp_rate = dsp_rate;
		ch[channel].out_rate = out_rate;
		build_channel (channel);
	}
}
