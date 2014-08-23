/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurBase.cpp

	 DESCRIPTION: uniform blur type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// system includes
#include <assert.h>

// local includes
#include "blurBase.h"

int BlurBase::findRotPixels(int x1, int y1, int x2, int y2, IPoint2* buf, int sz)
{
	int num(0);
	int x(x1), dx(x2-x1), ax(abs(dx)<<1), sx = sgn(dx);
	int y(y1), dy(y2-y1), ay(abs(dy)<<1), sy = sgn(dy);
	int d;

    if (ax>ay) // slope < 1
	{
		d = ay-(ax>>1);
		for (;;)
		{
			if (buf)
			{
				if (num > sz)
					return -1;
				buf[num].x = x;
				buf[num].y = y;
			}
			num++;
			if (x==x2)
				return num;
			if (d>=0)
			{
				y += sy;
				d -= ax;
			}
			x += sx;
			d += ay;
		}
	}
	else // slope >= 1
	{
		d = ax-(ay>>1);
		for (;;) {
			if (buf)
			{
				if (num > sz)
					return -1;
				buf[num].x   = x;
				buf[num].y = y;
			}
			num++;
			if (y==y2)
				return num;
			if (d>=0)
			{
				x += sx;
				d -= ay;
			}
			y += sy;
			d += ax;
		}
	}
}

void BlurBase::calcGaussWts(float *buf, int bufSz)
{
	assert(buf);

	int xDist, rad = (bufSz-1)/2;
	float sigma;
	
	for (int i=0; i<bufSz; i++)
	{
		xDist = i-rad;
		sigma = rad/2.0f;
		buf[i] = (float)exp(-(sqr(xDist))/(2.0*sqr(sigma)));
	}
}

void BlurBase::calcGaussWts(float *buf, int radA, int radB)
{
	assert(buf);

	int index(0);
	float sigma(radA/2.0f);
	for (int i=radA; i>0; i--)
		buf[index++] = (float)exp(-(sqr(i))/(2.0*sqr(sigma)));

	buf[index++] = 1.0f;

	sigma = radB/2.0f;
	for (i=1; i<=radB; i++)
		buf[index++] = (float)exp(-(sqr(i))/(2.0*sqr(sigma)));
}

void BlurBase::blendPixel(int index, WORD *mapFrom, AColor blendCol, float brighten, float blend, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	static int srcIdx;
	static float notBlend;

	srcIdx = index * 3;
	notBlend = (1 - blend) * WORD2NORMFLT; // pre-multiply the conversion from WORD to float

// begin - mjm - 8.3.99
	if (m_brightenType == idBrightenAdd)
	{
		// added this option to increase 'glow' quality of feathered selection
		blendCol.r += brighten;
		blendCol.g += brighten;
		blendCol.b += brighten;
	}
	else if (m_brightenType == idBrightenMult)
	{
		blend *= brighten+1.0f; // pre-multiply brighten (+ 1.0f makes it a multiplier)
	}
// end - mjm - 8.3.99

	blendCol.r = blendCol.r*blend + mapFrom[srcIdx  ]*notBlend;
	blendCol.g = blendCol.g*blend + mapFrom[srcIdx+1]*notBlend;
	blendCol.b = blendCol.b*blend + mapFrom[srcIdx+2]*notBlend;
	if (alphaTo && alphaFrom && (blend > 0.0f) )
		blendCol.a = blendCol.a*blend + alphaFrom[index]*notBlend;

	blendCol.ClampMinMax();

	mapTo[srcIdx]	= (USHORT)(blendCol.r * MAX_COL16);
	mapTo[srcIdx+1] = (USHORT)(blendCol.g * MAX_COL16);
	mapTo[srcIdx+2] = (USHORT)(blendCol.b * MAX_COL16);
	if (alphaTo && alphaFrom && (blend > 0.0f) )
			alphaTo[index] = (USHORT)(blendCol.a * MAX_COL16);
}
