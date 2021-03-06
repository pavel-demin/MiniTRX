/*  calcc.c

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

#define _CRT_SECURE_NO_WARNINGS
#include "comm.h"

CALCC create_calcc (int channel, int runcal, int size, int rate, int ints, int spi, float hw_scale, float moxdelay, float loopdelay, float ptol, int mox, int solidmox)
{
	int i;
	CALCC a = (CALCC) malloc0 (sizeof (calcc));
	a->channel = channel;
	a->runcal = runcal;
	a->size = size;
	a->rate = rate;
	a->ints = ints;
	a->spi = spi;
	a->nsamps = a->ints * a->spi;
	a->hw_scale = hw_scale;
	a->ctrl.moxdelay = moxdelay;
	a->ctrl.loopdelay = loopdelay;
	a->ptol = ptol;
	a->mox = mox;
	a->solidmox = solidmox;

	a->t =	(float *) malloc0 ((a->ints + 1) * sizeof(float));
	a->t1 = (float *) malloc0 ((a->ints + 1) * sizeof(float));
	for (i = 0; i <= a->ints; i++)
	{
		a->t1[i] = (float)i / (a->hw_scale * (float)a->ints);
		a->t[i] = (float)i / (float)a->ints;
	}

	a->cm = (float *) malloc0 (a->ints * 4 * sizeof(float));
	a->cc = (float *) malloc0 (a->ints * 4 * sizeof(float));
	a->cs = (float *) malloc0 (a->ints * 4 * sizeof(float));

	a->rxs = (float *) malloc0 (a->nsamps * sizeof (complex));
	a->txs = (float *) malloc0 (a->nsamps * sizeof (complex));

	a->info  = (int *) malloc0 (16 * sizeof (int));
	a->binfo = (int *) malloc0 (16 * sizeof (int));

	a->ctrl.cpi = (int *) malloc0 (a->ints * sizeof (int));
	a->ctrl.sindex = (int *) malloc0 (a->ints * sizeof (int));
	a->ctrl.sbase = (int *) malloc0 (a->ints * sizeof (int));

	a->ctrl.state = 0;
	a->ctrl.reset = 0;
	a->ctrl.automode = 0;
	a->ctrl.mancal = 0;
	a->ctrl.turnon = 0;
	a->ctrl.moxsamps = (int)(a->rate * a->ctrl.moxdelay);
	a->ctrl.moxcount = 0;
	a->ctrl.count = 0;
	for (i = 0; i < a->ints; i++)
	{
		a->ctrl.cpi[i] = 0;
		a->ctrl.sindex[i] = 0;
		a->ctrl.sbase[i] = i * a->spi;
	}
	a->ctrl.full_ints = 0;
	a->ctrl.calcinprogress = 0;
	a->ctrl.calcdone = 0;
	a->ctrl.waitsamps = (int)(a->rate * a->ctrl.loopdelay);
	a->ctrl.waitcount = 0;
	a->ctrl.running = 0;
	InitializeCriticalSectionAndSpinCount (&txa[a->channel].calcc.cs_update, 2500);
	a->rxdelay = create_delay (
		1,											// run
		0,											// size				[stuff later]
		0,											// input buffer		[stuff later]
		0,											// output buffer	[stuff later]
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		0.0);										// delay
	a->txdelay = create_delay (
		1,											// run
		0,											// size				[stuff later]
		0,											// input buffer		[stuff later]
		0,											// output buffer	[stuff later]
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		0.0);										// delay
	a->disp.x  = (float *) malloc0 (a->nsamps * sizeof (float));
	a->disp.ym = (float *) malloc0 (a->nsamps * sizeof (float));
	a->disp.yc = (float *) malloc0 (a->nsamps * sizeof (float));
	a->disp.ys = (float *) malloc0 (a->nsamps * sizeof (float));
	a->disp.cm = (float *) malloc0 (a->ints * 4 * sizeof(float));
	a->disp.cc = (float *) malloc0 (a->ints * 4 * sizeof(float));
	a->disp.cs = (float *) malloc0 (a->ints * 4 * sizeof(float));
	InitializeCriticalSectionAndSpinCount (&a->disp.cs_disp, 2500);
	a->util.ints = a->ints;
	a->util.channel = a->channel;
	a->temprx = (float *)malloc0 (2048 * sizeof (complex));														// remove later
	a->temptx = (float *)malloc0 (2048 * sizeof (complex));														// remove later
	return a;
}

void destroy_calcc (CALCC a)
{
	_aligned_free (a->temptx);																						// remove later
	_aligned_free (a->temprx);																						// remove later
	DeleteCriticalSection (&a->disp.cs_disp);
	_aligned_free (a->disp.cs);
	_aligned_free (a->disp.cc);
	_aligned_free (a->disp.cm);
	_aligned_free (a->disp.ys);
	_aligned_free (a->disp.yc);
	_aligned_free (a->disp.ym);
	destroy_delay (a->txdelay);
	destroy_delay (a->rxdelay);
	DeleteCriticalSection (&txa[a->channel].calcc.cs_update);
	_aligned_free (a->ctrl.sbase);
	_aligned_free (a->ctrl.sindex);
	_aligned_free (a->ctrl.cpi);
	_aligned_free (a->binfo);
	_aligned_free (a->info);
	_aligned_free (a->rxs);
	_aligned_free (a->txs);

	_aligned_free (a->cm);
	_aligned_free (a->cc);
	_aligned_free (a->cs);

	_aligned_free (a->t1);
	_aligned_free (a->t);

	_aligned_free (a);
}

void flush_calcc (CALCC a)
{
	flush_delay (a->rxdelay);
	flush_delay (a->txdelay);
}

int fcompare (const void * a, const void * b)
{
	if (*(float*)a < *(float*)b)
		return -1;
	else if (*(float*)a == *(float*)b)
		return 0;
	else
		return 1;
}

void decomp(int n, float *a, int *piv, int *info)
{
	int i, j, k;
	int t_piv;
	float m_row, mt_row, m_col, mt_col;
	float *wrk = (float *)malloc0(n * sizeof(float));
	*info = 0;
	for (i = 0; i < n; i++)
	{
		piv[i] = i;
		m_row = 0.0;
		for (j = 0; j < n; j++)
		{
			mt_row = a[n * i + j];
			if (mt_row < 0.0)  mt_row = - mt_row;
			if (mt_row > m_row)  m_row = mt_row;
		}
		if (m_row == 0.0)
		{
			*info = i;
			goto cleanup;
		}
		wrk[i] = m_row;
	}
	for (k = 0; k < n - 1; k++)
	{
		j = k;
		m_col = a[n * piv[k] + k] / wrk[piv[k]];
		if (m_col < 0)  m_col = - m_col;
		for (i = k + 1; i < n; i++)
		{
			mt_col = a[n * piv[i] + k] / wrk[piv[k]];
			if (mt_col < 0.0)  mt_col = - mt_col;
			if (mt_col > m_col)
			{
				m_col = mt_col;
				j = i;
			}
		}
		if (m_col == 0)
		{
			*info = - k;
			goto cleanup;
		}
		t_piv = piv[k];
		piv[k] = piv[j];
		piv[j] = t_piv;
		for (i = k + 1; i < n; i++)
		{
			a[n * piv[i] + k] /= a[n * piv[k] + k];
			for (j = k + 1; j < n; j++)
				a[n * piv[i] + j] -= a[n * piv[i] + k] * a[n * piv[k] + j];
		}
	}
	if (a[n * n - 1] == 0.0)
		*info = - n;
cleanup:
	_aligned_free (wrk);
}

void dsolve(int n, float *a, int *piv, float *b, float *x)
{
	int j, k;
	float sum;

	for (k = 0; k < n; k++)
	{
		sum = 0.0;
		for (j = 0; j < k; j++)
			sum += a[n * piv[k] + j] * x[j];
		x[k] = b[piv[k]] - sum;
	}

	for (k = n - 1; k >= 0; k--)
	{
		sum = 0.0;
		for (j = k + 1; j < n; j++)
			sum += a[n * piv[k] + j] * x[j];
		x[k] = (x[k] - sum) / a[n * piv[k] + k];
	}
}

void cull (int* n, int ints, float* x, float* t, float ptol)
{
	int k = 0;
	int i = *n;	
	int ntopint;
	int npx;

	while (x[i - 1] > t[ints - 1])
		i--;
	ntopint = *n - i;	
	npx = (int)(ntopint * (1.0 - ptol));
	i = *n;
	while ((k < npx) && (x[--i] > t[ints]))
		k++;
	*n -= k;
}

void builder (int points, float *x, float *y, int ints, float *t, int *info, float *c, float ptol)
{
	float *catxy = (float *)malloc0(2 * points * sizeof(float));
	float *sx =	(float *)malloc0(points * sizeof(float));
	float *sy =	(float *)malloc0(points * sizeof(float));
	float *h =		(float *)malloc0(ints * sizeof(float));
	int *p =		(int *)   malloc0(ints * sizeof(int));
	int *np =	    (int *)   malloc0(ints * sizeof(int));
	float u, v, alpha, beta, gamma, delta;
	float *taa =   (float *)malloc0(ints * sizeof(float));
	float *tab =   (float *)malloc0(ints * sizeof(float));
	float *tag =   (float *)malloc0(ints * sizeof(float));
	float *tad =   (float *)malloc0(ints * sizeof(float));
	float *tbb =   (float *)malloc0(ints * sizeof(float));
	float *tbg =   (float *)malloc0(ints * sizeof(float));
	float *tbd =   (float *)malloc0(ints * sizeof(float));
	float *tgg =   (float *)malloc0(ints * sizeof(float));
	float *tgd =   (float *)malloc0(ints * sizeof(float));
	float *tdd =   (float *)malloc0(ints * sizeof(float));
	int nsize = 3*ints + 1;
	int intp1 = ints + 1;
	int intm1 = ints - 1;
	float *A =     (float *)malloc0(intp1 * intp1 * sizeof(float));
	float *B =     (float *)malloc0(intp1 * intp1 * sizeof(float));
	float *C =     (float *)malloc0(intm1 * intp1 * sizeof(float));
	float *D =     (float *)malloc0(intp1 * sizeof(float));
	float *E =     (float *)malloc0(intp1 * intp1 * sizeof(float));
	float *F =     (float *)malloc0(intm1 * intp1 * sizeof(float));
	float *G =     (float *)malloc0(intp1 * sizeof(float));
	float *MAT =   (float *)malloc0(nsize * nsize * sizeof(float));
	float *RHS =   (float *)malloc0(nsize * sizeof(float));
	float *SLN =   (float *)malloc0(nsize * sizeof(float));
	float *z =	    (float *)malloc0(intp1 * sizeof(float));
	float *zp =    (float *)malloc0(intp1 * sizeof(float));
	int i, j, k, m;
	int dinfo;
	int *ipiv =        (int *)malloc0(nsize * sizeof(int));

	for (i = 0; i < points; i++)
	{
		catxy[2 * i + 0] = x[i];
		catxy[2 * i + 1] = y[i];
	}
	qsort(catxy, points, 2 * sizeof(float), fcompare);
	for (i = 0; i < points; i++)
	{
		sx[i] = catxy[2 * i + 0];
		sy[i] = catxy[2 * i + 1];
	}
	cull (&points, ints, sx, t, ptol);
	if (sx[points - 1] > t[ints])
	{
		*info = -1000;
		goto cleanup;
	}
	else *info = 0;

	for(j = 0; j < ints; j++)
		h[j] = t[j + 1] - t[j];
	p[0] = 0;
	j = 0;
	for (i = 0; i < points; i++)
	{
		if(sx[i] <= t[j + 1])
			np[j]++;
		else
		{
			p[++j] = i;
			while (sx[i] > t[j + 1])
				p[++j] = i;
			np[j] = 1;
		}
	}
	for (i = 0; i < ints; i++)
		for (j = p[i]; j < p[i] + np[i]; j++)
		{
			u = (sx[j] - t[i]) / h[i];
			v = u - 1.0;
			alpha = (2.0 * u + 1.0) * v * v;
			beta = u * u * (1.0 - 2.0 * v);
			gamma = h[i] * u * v * v;
			delta = h[i] * u * u * v;
			taa[i] += alpha * alpha;
			tab[i] += alpha * beta;
			tag[i] += alpha * gamma;
			tad[i] += alpha * delta;
			tbb[i] += beta * beta;
			tbg[i] += beta * gamma;
			tbd[i] += beta * delta;
			tgg[i] += gamma * gamma;
			tgd[i] += gamma * delta;
			tdd[i] += delta * delta;
			D[i + 0] += 2.0 * sy[j] * alpha;
			D[i + 1] += 2.0 * sy[j] * beta;
			G[i + 0] += 2.0 * sy[j] * gamma;
			G[i + 1] += 2.0 * sy[j] * delta;
		}
	for (i = 0; i < ints; i++)
	{
		A[(i + 0) * intp1 + (i + 0)] += 2.0 * taa[i];
		A[(i + 1) * intp1 + (i + 1)] =  2.0 * tbb[i];
		A[(i + 0) * intp1 + (i + 1)] =  2.0 * tab[i];
		A[(i + 1) * intp1 + (i + 0)] =  2.0 * tab[i];
		B[(i + 0) * intp1 + (i + 0)] += 2.0 * tag[i];
		B[(i + 1) * intp1 + (i + 1)] =  2.0 * tbd[i];
		B[(i + 0) * intp1 + (i + 1)] =  2.0 * tbg[i];
		B[(i + 1) * intp1 + (i + 0)] =  2.0 * tad[i];
		E[(i + 0) * intp1 + (i + 0)] += 2.0 * tgg[i];
		E[(i + 1) * intp1 + (i + 1)] =  2.0 * tdd[i];
		E[(i + 0) * intp1 + (i + 1)] =  2.0 * tgd[i];
		E[(i + 1) * intp1 + (i + 0)] =  2.0 * tgd[i];
	}
	for (i = 0; i < intm1; i++)
	{
		C[i * intp1 + (i + 0)] = +3.0 * h[i + 1] / h[i];
		C[i * intp1 + (i + 2)] = -3.0 * h[i] / h[i + 1];
		C[i * intp1 + (i + 1)] = -C[i * intp1 + (i + 0)] - C[i * intp1 + (i + 2)];
		F[i * intp1 + (i + 0)] =  h[i + 1];
		F[i * intp1 + (i + 1)] = 2.0 * (h[i] + h[i + 1]);
		F[i * intp1 + (i + 2)] = h[i];
	}
	for (i = 0, k = 0; i < intp1; i++, k++)
	{
		for (j = 0, m = 0; j < intp1; j++, m++)
			MAT[k*nsize + m] = A[i * intp1 + j];
		for (j = 0, m = intp1; j < intp1; j++, m++)
			MAT[k*nsize + m] = B[j * intp1 + i];
		for (j = 0, m = 2 * intp1; j < intm1; j++, m++)
			MAT[k*nsize + m] = C[j * intp1 + i];
		RHS[k] = D[i];
	}
	for (i = 0, k = intp1; i < intp1; i++, k++)
	{
		for (j = 0, m = 0; j < intp1; j++, m++)
			MAT[k*nsize + m] = B[i * intp1 + j];
		for (j = 0, m = intp1; j < intp1; j++, m++)
			MAT[k*nsize + m] = E[i * intp1 + j];
		for (j = 0, m = 2 * intp1; j < intm1; j++, m++)
			MAT[k*nsize + m] = F[j * intp1 + i];
		RHS[k] = G[i];
	}
	for (i = 0, k = 2 * intp1; i < intm1; i++, k++)
	{
		for (j = 0, m = 0; j < intp1; j++, m++)
			MAT[k*nsize + m] = C[i * intp1 + j];
		for (j = 0, m = intp1; j < intp1; j++, m++)
			MAT[k*nsize + m] = F[i * intp1 + j];
		for (j = 0, m = 2 * intp1; j < intm1; j++, m++)
			MAT[k*nsize + m] = 0.0;
		RHS[k] = 0.0;
	}
	decomp(nsize, MAT, ipiv, &dinfo);
	dsolve(nsize, MAT, ipiv, RHS, SLN);
	if (dinfo != 0)
	{
		*info = dinfo;
		goto cleanup;
	}

	for (i = 0; i <= ints; i++)
	{
		z[i] = SLN[i];
		zp[i] = SLN[i + ints + 1];
	}
	for (i = 0; i < ints; i++)
	{
		c[4 * i + 0] = z[i];
		c[4 * i + 1] = zp[i];
		c[4 * i + 2] = -3.0 / (h[i] * h[i]) * (z[i] - z[i + 1]) - 1.0 / h[i] * (2.0 * zp[i] + zp[i + 1]);
		c[4 * i + 3] = 2.0 / (h[i] * h[i] * h[i]) * (z[i] - z[i + 1]) + 1.0 / (h[i] * h[i]) * (zp[i] + zp[i + 1]);
	}
cleanup:
	_aligned_free (ipiv);
	_aligned_free (catxy);
	_aligned_free (sx);
	_aligned_free (sy);
	_aligned_free (h);
	_aligned_free (p);
	_aligned_free (np);

	_aligned_free (taa);
	_aligned_free (tab);
	_aligned_free (tag);
	_aligned_free (tad);
	_aligned_free (tbb);
	_aligned_free (tbg);
	_aligned_free (tbd);
	_aligned_free (tgg);
	_aligned_free (tgd);
	_aligned_free (tdd);

	_aligned_free (A);
	_aligned_free (B);
	_aligned_free (C);
	_aligned_free (D);
	_aligned_free (E);
	_aligned_free (F);
	_aligned_free (G);

	_aligned_free (MAT);
	_aligned_free (RHS);
	_aligned_free (SLN);

	_aligned_free (z);
	_aligned_free (zp);
}

void scheck(CALCC a)
{
	int i, j, k;
	float v, dx, out;
	int intm1 = a->ints - 1;
	a->binfo[6] = 0x0000;
	if (a->cm[0] != a->cm[0]) a->binfo[6] |= 0x0001;
	if ((a->cm[4 * intm1 + 0] == 0.0) && (a->cm[4 * intm1 + 1] == 0.0) &&
		(a->cm[4 * intm1 + 2] == 0.0) && (a->cm[4 * intm1 + 3] == 0.0)) a->binfo[6] |= 0x0002;
	for (i = 0; i < a->ints; i++)
	{
		for (j = 0; j < 4; j++)
		{
			k = 4 * i + j;
			v = (float)k / (4.0 * (float)a->ints);
			dx = (a->t[i + 1] - a->t[i]) * (float)j / 4.0;
			out = v * (a->cm[4 * i + 0] + dx * (a->cm[4 * i + 1] + dx * (a->cm[4 * i + 2] + dx * a->cm[4 * i + 3])));
			if (out > 1.0)
				a->binfo[6] |= 0x0004;
			if (out < 0.0) a->binfo[6] |= 0x0010;
		}
	}
	dx = a->t[a->ints] - a->t[a->ints - 1];
	out = a->cm[4 * intm1 + 0] + dx * (a->cm[4 * intm1 + 1] + dx * (a->cm[4 * intm1 + 2] + dx * a->cm[4 * intm1 + 3]));
	if (out > 1.01)	// VALUE
		a->binfo[6] |= 0x0008;
	if (out < 0.0) a->binfo[6] |= 0x0020;
}

void calc (CALCC a)
{
	int i;
	float *env_TX =	(float *)malloc0(a->nsamps * sizeof(float));
	float *env_RX =	(float *)malloc0(a->nsamps * sizeof(float));
	float *txrxcoefs = (float *)malloc0(a->ints * 4 * sizeof(float));
	float *x =			(float *)malloc0(a->nsamps * sizeof(float));
	float *ym =		(float *)malloc0(a->nsamps * sizeof(float));
	float *yc =		(float *)malloc0(a->nsamps * sizeof(float));
	float *ys =		(float *)malloc0(a->nsamps * sizeof(float));
	float rx_scale;
	float norm;

	for (i = 0; i < a->nsamps; i++)
	{
		env_TX[i] = sqrt (a->txs[2 * i + 0] * a->txs[2 * i + 0] + a->txs[2 * i + 1] * a->txs[2 * i + 1]);
		env_RX[i] = sqrt (a->rxs[2 * i + 0] * a->rxs[2 * i + 0] + a->rxs[2 * i + 1] * a->rxs[2 * i + 1]);
	}
	{
		int rints = 2;
		int ix = rints - 1;
		float tvec[3] = { 0.0, 0.5 / a->hw_scale, 1.0 / a->hw_scale };
		float dx = tvec[rints] - tvec[rints - 1];
		builder(a->nsamps, env_TX, env_RX, rints, tvec, &(a->binfo[0]), txrxcoefs, a->ptol);
		if (a->binfo[0] != 0)
			goto cleanup;
		rx_scale = 1.0 / (txrxcoefs[4 * ix + 0] + dx * (txrxcoefs[4 * ix + 1] + dx * (txrxcoefs[4 * ix + 2] + dx * txrxcoefs[4 * ix + 3])));
	}

	a->binfo[4] = (int)(256.0 * (a->hw_scale / rx_scale));
	a->binfo[5]++;

	for (i = 0; i < a->nsamps; i++)
	{
		norm = env_TX[i] * env_RX[i];
		x[i]  = rx_scale * env_RX[i];
		ym[i] = (a->hw_scale * env_TX[i]) / (rx_scale * env_RX[i]);
		yc[i] = (+ a->txs[2 * i + 0] * a->rxs[2 * i + 0] + a->txs[2 * i + 1] * a->rxs[2 * i + 1]) / norm;
		ys[i] = (- a->txs[2 * i + 0] * a->rxs[2 * i + 1] + a->txs[2 * i + 1] * a->rxs[2 * i + 0]) / norm;
	}

	builder (a->nsamps, x, ym, a->ints, a->t, &(a->binfo[1]), a->cm, a->ptol);
	builder (a->nsamps, x, yc, a->ints, a->t, &(a->binfo[2]), a->cc, a->ptol);
	builder (a->nsamps, x, ys, a->ints, a->t, &(a->binfo[3]), a->cs, a->ptol);
	scheck (a);
	EnterCriticalSection (&a->disp.cs_disp);
	memcpy(a->disp.x,  x,  a->nsamps * sizeof (float));
	memcpy(a->disp.ym, ym, a->nsamps * sizeof (float));
	memcpy(a->disp.yc, yc, a->nsamps * sizeof (float));
	memcpy(a->disp.ys, ys, a->nsamps * sizeof (float));
	if ((a->binfo[0] == 0) && (a->binfo[1] == 0) && (a->binfo[2] == 0) && (a->binfo[3] == 0) && (a->binfo[6] == 0))
	{
		memcpy(a->disp.cm, a->cm, a->ints * 4 * sizeof (float));
		memcpy(a->disp.cc, a->cc, a->ints * 4 * sizeof (float));
		memcpy(a->disp.cs, a->cs, a->ints * 4 * sizeof (float));
	}
	else
	{
		memset(a->disp.cm, 0, a->ints * 4 * sizeof (float));
		memset(a->disp.cc, 0, a->ints * 4 * sizeof (float));
		memset(a->disp.cs, 0, a->ints * 4 * sizeof (float));
	}
	LeaveCriticalSection (&a->disp.cs_disp);
cleanup:
	_aligned_free (x);
	_aligned_free (ym);
	_aligned_free (yc);
	_aligned_free (ys);

	_aligned_free (env_TX);
	_aligned_free (env_RX);
	_aligned_free (txrxcoefs);
}

void __cdecl doCalcCorrection (void *arg)
{
	CALCC a = (CALCC)arg;
	calc (a);
	if ((a->binfo[0] == 0) && (a->binfo[1] == 0) && (a->binfo[2] == 0) && (a->binfo[3] == 0) && (a->binfo[6] == 0))
	{
		if (!InterlockedBitTestAndSet (&a->ctrl.running, 0))
			SetTXAiqcStart (a->channel, a->cm, a->cc, a->cs);
		else
			SetTXAiqcSwap  (a->channel, a->cm, a->cc, a->cs);
	}
	InterlockedBitTestAndSet (&a->ctrl.calcdone, 0);
	_endthread();
}

void __cdecl doTurnoff (void *arg)
{
	CALCC a = (CALCC)arg;
	SetTXAiqcEnd (a->channel);
	_endthread();
}

enum _calcc_state
{
	LRESET,
	LWAIT,
	LMOXDELAY,
	LSETUP,
	LCOLLECT,
	MOXCHECK,
	LCALC,
	LDELAY,
	LSTAYON,
	LTURNON
};

void __cdecl SaveCorrection (void *pargs)
{
	int i, k;
	CALCC a = (CALCC)pargs;
	float* pm = (float *)malloc0 (4 * a->util.ints * sizeof (float));
	float* pc = (float *)malloc0 (4 * a->util.ints * sizeof (float));
	float* ps = (float *)malloc0 (4 * a->util.ints * sizeof (float));
	FILE* file = fopen(a->util.savefile, "w");
	GetTXAiqcValues (a->util.channel, pm, pc, ps);
	for (i = 0; i < a->util.ints; i++)
	{
		for (k = 0; k < 4; k++)
			fprintf (file, "%.17e\t", pm[4 * i + k]);
		fprintf (file, "\n");
		for (k = 0; k < 4; k++)
			fprintf (file, "%.17e\t", pc[4 * i + k]);
		fprintf (file, "\n");
		for (k = 0; k < 4; k++)
			fprintf (file, "%.17e\t", ps[4 * i + k]);
		fprintf (file, "\n\n");
	}
	fflush (file);
	fclose (file);
	_aligned_free (ps);
	_aligned_free (pc);
	_aligned_free (pm);
	_endthread();
}

void __cdecl RestoreCorrection(void *pargs)
{
	int i, k;
	CALCC a = (CALCC)pargs;
	float* pm = (float *)malloc0 (4 * a->util.ints * sizeof (float));
	float* pc = (float *)malloc0 (4 * a->util.ints * sizeof (float));
	float* ps = (float *)malloc0 (4 * a->util.ints * sizeof (float));
	FILE* file = fopen (a->util.restfile, "r");
	for (i = 0; i < a->util.ints; i++)
	{
		for (k = 0; k < 4; k++)
			fscanf (file, "%le", &(pm[4 * i + k]));
		for (k = 0; k < 4; k++)
			fscanf (file, "%le", &(pc[4 * i + k]));
		for (k = 0; k < 4; k++)
			fscanf (file, "%le", &(ps[4 * i + k]));
	}
	fclose (file);
	if (!InterlockedBitTestAndSet (&a->ctrl.running, 0))
		SetTXAiqcStart (a->channel, pm, pc, ps);
	else
		SetTXAiqcSwap  (a->channel, pm, pc, ps);
	_aligned_free (ps);
	_aligned_free (pc);
	_aligned_free (pm);
	_endthread();
}

/********************************************************************************************************
*																										*
*										  Public Functions												*
*																										*
********************************************************************************************************/

PORT
void pscc (int channel, int size, float* tx, float* rx)
{
	int i, n, m;
	float env;
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	if (a->runcal)
	{
		a->size = size;
		if (a->mox && a->txdelay->tdelay != 0.0)
		{
			SetDelayBuffs (a->rxdelay, a->size, rx, rx);
			xdelay (a->rxdelay);
			SetDelayBuffs (a->txdelay, a->size, tx, tx);
			xdelay (a->txdelay);
		}
		a->info[15] = a->ctrl.state;

		switch (a->ctrl.state)
		{
			case LRESET:
				a->ctrl.reset = 0;
				if (!a->ctrl.turnon)
					if (InterlockedBitTestAndReset (&a->ctrl.running, 0))
						_beginthread (doTurnoff, 0, (void *)a);
				a->info[14] = 0;
				a->ctrl.env_maxtx = 0.0;
				if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if (a->ctrl.automode || a->ctrl.mancal)
					a->ctrl.state = LWAIT;
				break;
			case LWAIT:
				a->ctrl.mancal = 0;
				a->ctrl.moxcount = 0;
				if (a->ctrl.reset)
					a->ctrl.state = LRESET;
				else if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if (a->mox)
					a->ctrl.state = LMOXDELAY;
				break;
			case LMOXDELAY:
				a->ctrl.moxcount += a->size;
				if (a->ctrl.reset)
					a->ctrl.state = LRESET;
				else if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if (!a->mox || !a->solidmox)
					a->ctrl.state = LWAIT;
				else if ((a->ctrl.moxcount - a->size) >= a->ctrl.moxsamps)
					a->ctrl.state = LSETUP;
				break;
			case LSETUP:
				a->ctrl.count = 0;
				for (i = 0; i < a->ints; i++)
				{
					a->ctrl.cpi[i] = 0;
					a->ctrl.sindex[i] = 0;
				}
				a->ctrl.full_ints = 0;
				a->ctrl.waitcount = 0;
				
				if (a->ctrl.reset)
					a->ctrl.state = LRESET;
				else if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if (a->mox && a->solidmox)
				{
					a->ctrl.state = LCOLLECT;
					SetTXAiqcDogCount (channel, a->info[13] = 0);
				}
				else
					a->ctrl.state = LWAIT;
				break;
			case LCOLLECT:
				for (i = 0; i < a->size; i++)
				{
					env = sqrt(tx[2 * i + 0] * tx[2 * i + 0] + tx[2 * i + 1] * tx[2 * i + 1]);
					if (env > a->ctrl.env_maxtx)
						a->ctrl.env_maxtx = env;
					if ((env *= a->hw_scale * (float)a->ints) <= (float)a->ints)
					{
						if (env == (float)a->ints)
							n = a->ints - 1;
						else
							n = (int)env;
						m = a->ctrl.sbase[n] + a->ctrl.sindex[n];
						a->txs[2 * m + 0] = tx[2 * i + 0];
						a->txs[2 * m + 1] = tx[2 * i + 1];
						a->rxs[2 * m + 0] = rx[2 * i + 0];
						a->rxs[2 * m + 1] = rx[2 * i + 1];
						if (++a->ctrl.sindex[n] == a->spi) a->ctrl.sindex[n] = 0;
						if (a->ctrl.cpi[n] != a->spi)
							if (++a->ctrl.cpi[n] == a->spi) a->ctrl.full_ints++;
						++a->ctrl.count;
					}
				}
				GetTXAiqcDogCount (channel, &a->info[13]);
				if (a->ctrl.reset)
					a->ctrl.state = LRESET;
				else if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if (!a->mox || !a->solidmox)
					a->ctrl.state = LWAIT;
				else if (a->ctrl.full_ints == a->ints)
					a->ctrl.state = MOXCHECK;
				else if (a->info[13] >= 6)
					a->ctrl.state = LRESET;
				else if (a->ctrl.count >= (int)a->rate)
					a->ctrl.state = LSETUP;
				break;
			case MOXCHECK:
				if (a->ctrl.reset)
					a->ctrl.state = LRESET;
				else if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if (!a->mox || !a->solidmox)
					a->ctrl.state = LWAIT;
				else
					a->ctrl.state = LCALC;
				break;
			case LCALC:
				if (!a->ctrl.calcinprogress)	
				{
					a->ctrl.calcinprogress = 1;
					_beginthread(doCalcCorrection, 0, (void *)a);
				}

				if (InterlockedBitTestAndReset(&a->ctrl.calcdone, 0))
				{
					memcpy (a->info, a->binfo, 7 * sizeof (int));
					a->info[14] = _InterlockedAnd (&a->ctrl.running, 1);
					a->ctrl.calcinprogress = 0;
					if (a->ctrl.reset)
						a->ctrl.state = LRESET;
					else if (a->ctrl.turnon)
						a->ctrl.state = LTURNON;
					else if ((a->info[0] == 0) && (a->info[1] == 0) && (a->info[2] == 0) && (a->info[3] == 0) && (a->info[6] == 0))
					{
						a->ctrl.state = LDELAY;
					}
					else
					{
						if (a->mox && a->solidmox) a->ctrl.state = LSETUP;
						else a->ctrl.state = LWAIT;
					}
				}
				break;
			case LDELAY:
				a->ctrl.waitcount += a->size;
				if (a->ctrl.reset)
					a->ctrl.state = LRESET;
				else if (a->ctrl.turnon)
					a->ctrl.state = LTURNON;
				else if ((a->ctrl.waitcount - a->size) >= a->ctrl.waitsamps)
				{
					if (a->ctrl.automode)
					{
						if (a->mox && a->solidmox)
							a->ctrl.state = LSETUP;
						else
							a->ctrl.state = LWAIT;
					}
					else
						a->ctrl.state = LSTAYON;
				}
				break;
			case LSTAYON:
				if (a->ctrl.reset || a->ctrl.automode || a->ctrl.mancal)
					a->ctrl.state = LRESET;
				break;
			case LTURNON:
				a->ctrl.turnon = 0;
				a->ctrl.automode = 0;
				a->info[14] = 1;
				a->ctrl.state = LSTAYON;
				break;
		}
	}
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT 
void psccF (int channel, int size, float *Itxbuff, float *Qtxbuff, float *Irxbuff, float *Qrxbuff, int mox, int solidmox)
{
	int i;
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->mox = mox;
	a->solidmox = solidmox;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
	for (i = 0; i < size; i++)
	{
		a->temptx[2 * i + 0] = (float)Itxbuff[i];
		a->temptx[2 * i + 1] = (float)Qtxbuff[i];
		a->temprx[2 * i + 0] = (float)Irxbuff[i];
		a->temprx[2 * i + 1] = (float)Qrxbuff[i];
	}
	pscc (channel, size, a->temptx, a->temprx);
}

PORT
void PSSaveCorr (int channel, char* filename)
{
	CALCC a;
	int i = 0;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	while (a->util.savefile[i++] = *filename++);
	_beginthread(SaveCorrection, 0, (void *)a);
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void PSRestoreCorr (int channel, char* filename)
{
	CALCC a;
	int i = 0;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	while (a->util.restfile[i++] = *filename++);
	a->ctrl.turnon = 1;
	_beginthread(RestoreCorrection, 0, (void *)a);
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

/********************************************************************************************************
*																										*
*											  Properties												*
*																										*
********************************************************************************************************/

PORT
void SetPSRunCal (int channel, int run)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->runcal = run;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSInSize (int channel, int size)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->size = size;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSMox (int channel, int mox)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->mox = mox;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSSolidmox (int channel, int solidmox)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->solidmox = solidmox;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT 
void GetPSInfo (int channel, int *info)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	memcpy (info, a->info, 16 * sizeof(int));
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSReset (int channel, int reset)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->ctrl.reset = reset;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSMancal (int channel, int mancal)
{
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	txa[channel].calcc.p->ctrl.mancal = mancal;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSAutomode (int channel, int automode)
{
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	txa[channel].calcc.p->ctrl.automode = automode;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSTurnon (int channel, int turnon)
{
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	txa[channel].calcc.p->ctrl.turnon = turnon;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSControl (int channel, int reset, int mancal, int automode, int turnon)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->ctrl.reset = reset;
	a->ctrl.mancal = mancal;
	a->ctrl.automode = automode;
	a->ctrl.turnon = turnon;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSLoopDelay (int channel, float delay)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->ctrl.loopdelay = delay;
	a->ctrl.waitsamps = (int)(a->rate * a->ctrl.loopdelay);
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSMoxDelay (int channel, float delay)
{
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->ctrl.moxdelay = delay;
	a->ctrl.moxsamps = (int)(a->rate * a->ctrl.moxdelay);
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
float SetPSTXDelay (int channel, float delay)
{
	CALCC a;
	float adelay;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->txdel = delay;
	adelay = SetDelayValue (a->txdelay, a->txdel);
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
	return adelay;
}

PORT
void SetPSHWPeak (int channel, float peak)
{
	int i;
	CALCC a;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a = txa[channel].calcc.p;
	a->hw_scale = 1.0 / peak;
	for (i = 0; i <= a->ints; i++)
		a->t1[i] = (float)i / (a->hw_scale * (float)a->ints);
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void GetPSHWPeak (int channel, float* peak)
{
EnterCriticalSection (&txa[channel].calcc.cs_update);
*peak = 1.0 / txa[channel].calcc.p->hw_scale;
LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void GetPSMaxTX (int channel, float* maxtx)
{
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	*maxtx = txa[channel].calcc.p->ctrl.env_maxtx;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void SetPSPtol (int channel, float ptol)
{
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	txa[channel].calcc.p->ptol = ptol;
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}

PORT
void GetPSDisp (int channel, float* x, float* ym, float* yc, float* ys, float* cm, float* cc, float* cs)
{
	CALCC a = txa[channel].calcc.p;
	EnterCriticalSection (&a->disp.cs_disp);
	memcpy (x,  a->disp.x,  a->nsamps * sizeof (float));
	memcpy (ym, a->disp.ym, a->nsamps * sizeof (float));
	memcpy (yc, a->disp.yc, a->nsamps * sizeof (float));
	memcpy (ys, a->disp.ys, a->nsamps * sizeof (float));
	memcpy (cm, a->disp.cm, a->ints * 4 * sizeof (float));
	memcpy (cc, a->disp.cc, a->ints * 4 * sizeof (float));
	memcpy (cs, a->disp.cs, a->ints * 4 * sizeof (float));
	LeaveCriticalSection (&a->disp.cs_disp);
}

PORT
void SetPSFeedbackRate (int channel, int rate)
{
	CALCC a = txa[channel].calcc.p;
	EnterCriticalSection (&txa[channel].calcc.cs_update);
	a->rate = rate;
	a->ctrl.moxsamps = (int)(a->rate * a->ctrl.moxdelay);
	a->ctrl.waitsamps = (int)(a->rate * a->ctrl.loopdelay);
	destroy_delay (a->txdelay);
	destroy_delay (a->rxdelay);
	a->rxdelay = create_delay (
		1,											// run
		0,											// size				[stuff later]
		0,											// input buffer		[stuff later]
		0,											// output buffer	[stuff later]
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		0.0);										// delay
	a->txdelay = create_delay (
		1,											// run
		0,											// size				[stuff later]
		0,											// input buffer		[stuff later]
		0,											// output buffer	[stuff later]
		a->rate,									// sample rate
		20.0e-09,									// delta (delay stepsize)
		a->txdel);									// delay
	LeaveCriticalSection (&txa[channel].calcc.cs_update);
}
