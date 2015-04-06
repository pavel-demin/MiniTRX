/*  comm.h

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

#ifndef _comm_h
#define _comm_h

// channel definitions
#define MAX_CHANNELS    2       // maximum number of supported channels

// math definitions
#define PI              3.1415926535897932
#define TWOPI           6.2831853071795864

#define min(x,y) (x<y?x:y)
#define max(x,y) (x<y?y:x)

typedef float complex[2];

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <fftw3.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "amd.h"
#include "ammod.h"
#include "amsq.h"
#include "anf.h"
#include "anr.h"
#include "bandpass.h"
#include "cblock.h"
#include "compress.h"
#include "delay.h"
#include "div.h"
#include "eer.h"
#include "emnr.h"
#include "emph.h"
#include "fcurve.h"
#include "fir.h"
#include "gain.h"
#include "gen.h"
#include "iir.h"
#include "meter.h"
#include "meterlog10.h"
#include "nob.h"
#include "nobII.h"
#include "osctrl.h"
#include "patchpanel.h"
#include "shift.h"
#include "utilities.h"
#include "wcpAGC.h"
#include "RXA.h"
#include "TXA.h"

#ifdef __cplusplus
}
#endif
#endif
