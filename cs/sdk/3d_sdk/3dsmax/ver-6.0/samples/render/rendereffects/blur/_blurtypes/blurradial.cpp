/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurRadial.cpp

	 DESCRIPTION: radial blur type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
#include "pch.h"

// local includes
#include "blurRadial.h"
#include "..\blurMgr.h"


BlurRadial::~BlurRadial()
{
	if (mp_blurCache)
		delete[] mp_blurCache;
	if (mp_scratchMap)
		delete[] mp_scratchMap;
}

void BlurRadial::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmRadialPixRad:
		case prmRadialTrail:
		case prmRadialXOrig:
		case prmRadialYOrig:
		case prmRadialUseNode:
		case prmRadialNode:
			m_blurValid = false;
		case prmRadialAlpha:
			break;
	}
}

bool BlurRadial::calcRotPixOffsets(double theta)
{
	double c( cos(theta) ), s( sin(theta) );
	int x1(0), y1(0), x2(0), y2(0);

	if (m_inRad) // count pixels left of center
	{
		x1 = -round(m_inRad*c); y1 = -round(m_inRad*s);
		m_inRot = findRotPixels(x1, y1, 0, 0) - 1; // don't count origin
	}
	else
		m_inRot = 0;

	if (m_outRad) // count pixels right of center
	{
		x2 = round(m_outRad*c); y2 = round(m_outRad*s);
		m_outRot = findRotPixels(0, 0, x2, y2) - 1; // don't count origin
	}
	else
		m_outRot = 0;

	m_numRotPix = m_inRot+m_outRot+1;
	if ( m_szRotPix < m_numRotPix )
	{
		if (mp_RotPix)
			delete[] mp_RotPix;
		m_szRotPix = m_numRotPix;
		mp_RotPix = new IPoint2[m_szRotPix];
		if (!mp_RotPix)
			return false;
	}

	if (m_numRotPix == 1)
	{
		mp_RotPix[0].x = mp_RotPix[0].y = 0;
		return true;
	}

	if (m_inRad) // record pixels left of center
		findRotPixels(x1, y1, 0, 0, mp_RotPix, m_numRotPix); // last pixel recorded will be origin

	if (m_outRad) // record pixels right of center
		findRotPixels(0, 0, x2, y2, mp_RotPix+m_inRot, m_numRotPix-m_inRot); // first pixel recorded will be origin

	return true;
}

bool BlurRadial::doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	// this function really needs to be cleaned up and modularized ... no time!
	float totWts, outMult=2.0f;
	int i,j,u,v,x,y,p,mapIndex,filterIndex,taskDone,doneSoFar(0);
	double pixRad,distRatio,dist,maxDist,theta;
	mp_nameCurProcess = GetString(IDS_PROCESS_BLUR);

	if(m_blurValid && mapTo)
	{
		int srcIdx;
		for (int i=0; i<m_imageSz; i++)
			if (pCompMap)
				blendPixel(i, mapFrom, mp_blurCache[i], (*pCompMap)[i].brighten, (*pCompMap)[i].blend, mapTo, alphaFrom, alphaTo);
			else
			{
				srcIdx = i*3;
				mp_srcMap[srcIdx]   = (USHORT)(mp_blurCache[i].r * MAX_COL16);
				mp_srcMap[srcIdx+1] = (USHORT)(mp_blurCache[i].g * MAX_COL16);
				mp_srcMap[srcIdx+2] = (USHORT)(mp_blurCache[i].b * MAX_COL16);
				if (alphaTo)
					alphaTo[i] = (USHORT)(mp_blurCache[i].a * MAX_COL16);
			}
		return true;
	}
	x = int(m_orig.x);
	y = int(m_orig.y);
	int maxX = max( abs(x), abs(m_imageW - x) );
	int maxY = max( abs(y), abs(m_imageH - y) );
	int maxXY = max(maxX, maxY);
	maxDist = sqrt( sqr(maxX) + sqr(maxY) );

	if ( inBounds(x, y, m_imageW, m_imageH) )
	{
		// ----------------------------------------------------------------------------------
		// do not blur origin
		// ----------------------------------------------------------------------------------
		mapIndex = x + y*m_imageW;
		p = mapIndex*3; // align with mapFrom's rgb triplet
		mp_blurCache[mapIndex].r = mapFrom[p]  *WORD2NORMFLT;
		mp_blurCache[mapIndex].g = mapFrom[p+1]*WORD2NORMFLT;
		mp_blurCache[mapIndex].b = mapFrom[p+2]*WORD2NORMFLT;
		mp_blurCache[mapIndex].a = alphaFrom[mapIndex]*WORD2NORMFLT;
	}

	// --------------------------------------------------------------------------------------
	// blur along axes
	// --------------------------------------------------------------------------------------
	int counter(0);
	taskDone = maxX + maxY + maxXY + 40*maxY; // weight the last maxY since it is a much slower loop
	for (i=1; i<=maxX; i++)
	{
		dist = (double)i;
		distRatio = dist/maxDist;
		pixRad = distRatio * m_pixRad;
		m_inRad = round(pixRad*(1-m_trail)); m_outRad = round(pixRad*outMult*(1+m_trail));
		if ( m_szFilter < m_inRad+m_outRad+1 )
		{
			if (mp_Filter)
				delete[] mp_Filter;
			m_szFilter = m_inRad+m_outRad+1;
			mp_Filter = new float[m_szFilter];
		}
		calcGaussWts(mp_Filter, m_inRad, m_outRad);

		// +x axis --------------------------------------------------------------------------
		x = int(m_orig.x)+i;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (u=-m_inRad, filterIndex=0; u<=m_outRad; u++, filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+u, y, m_imageW, m_imageH) )
					continue;

				// compute offsets
				p = (mapIndex+u)*3; // align with mapFrom's rgb triplet
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}

		// -x axis --------------------------------------------------------------------------
		x = int(m_orig.x)-i;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (u=-m_outRad, filterIndex=m_outRad+m_inRad; u<=m_inRad; u++, filterIndex--)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+u, y, m_imageW, m_imageH) )
					continue;

				// compute offsets
				p = (mapIndex+u)*3; // align with mapFrom's rgb triplet
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}
		if ( ( (i%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, i, taskDone) )
			return false;
	}
	doneSoFar += maxX;

	x = int(m_orig.x);
	for (j=1; j<=maxY; j++)
	{
		dist = (double)j;
		distRatio = dist/maxDist;
		pixRad = distRatio * m_pixRad;
		m_inRad = round(pixRad*(1-m_trail)); m_outRad = round(pixRad*outMult*(1+m_trail));
		if ( m_szFilter < m_inRad+m_outRad+1 )
		{
			if (mp_Filter)
				delete[] mp_Filter;
			m_szFilter = m_inRad+m_outRad+1;
			mp_Filter = new float[m_szFilter];
		}
		calcGaussWts(mp_Filter, m_inRad, m_outRad);

		// +y axis --------------------------------------------------------------------------
		y = int(m_orig.y)+j;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (v=-m_inRad, filterIndex=0; v<=m_outRad; v++, filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x, y+v, m_imageW, m_imageH) )
					continue;

				// compute offsets
				p = (mapIndex + v*m_imageW)*3;
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}

		// -y axis --------------------------------------------------------------------------
		y = int(m_orig.y)-j;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (v=-m_outRad, filterIndex=m_outRad+m_inRad; v<=m_inRad; v++, filterIndex--)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x, y+v, m_imageW, m_imageH) )
					continue;

				// compute offsets
				p = (mapIndex + v*m_imageW)*3;
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}
		if ( ( (j%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, j+doneSoFar, taskDone) )
			return false;
	}
	doneSoFar += maxY;

	// --------------------------------------------------------------------------------------
	// blur along 45 degrees
	// --------------------------------------------------------------------------------------
	theta = atan(1);
	for (i=1; i<=maxXY; i++) {
		dist = sqrt( 2*sqr(i) );
		distRatio = dist/maxDist;
		pixRad = distRatio * m_pixRad;
		m_inRad = round(pixRad*(1-m_trail)); m_outRad = round(pixRad*outMult*(1+m_trail));
		if ( !calcRotPixOffsets(theta) )
			return false;
		if ( m_szFilter < m_numRotPix )
		{
			if (mp_Filter)
				delete[] mp_Filter;
			m_szFilter = m_numRotPix;
			mp_Filter = new float[m_szFilter];
		}
		calcGaussWts(mp_Filter, m_inRot, m_outRot);

		// +x, +y -------------------------------------------------------------------
		x = int(m_orig.x)+i;
		y = int(m_orig.y)+i;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+mp_RotPix[filterIndex].x, y+mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
						continue;

				p = (mapIndex + (mp_RotPix[filterIndex].y*m_imageW) + mp_RotPix[filterIndex].x)*3;
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}

		// -x, +y -------------------------------------------------------------------
		x = int(m_orig.x)-i;
		y = int(m_orig.y)+i;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x-mp_RotPix[filterIndex].x, y+mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
						continue;

				p = (mapIndex + (mp_RotPix[filterIndex].y*m_imageW) - mp_RotPix[filterIndex].x)*3;
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}

		// -x, -y -------------------------------------------------------------------
		x = int(m_orig.x)-i;
		y = int(m_orig.y)-i;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x-mp_RotPix[filterIndex].x, y-mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
						continue;

				p = (mapIndex - (mp_RotPix[filterIndex].y*m_imageW) - mp_RotPix[filterIndex].x)*3;
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}

		// +x, -y -------------------------------------------------------------------
		x = int(m_orig.x)+i;
		y = int(m_orig.y)-i;
		if ( inBounds(x, y, m_imageW, m_imageH) )
		{
			// average pixels
			mapIndex = x + y*m_imageW;
			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;
			for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+mp_RotPix[filterIndex].x, y-mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
						continue;

				p = (mapIndex - (mp_RotPix[filterIndex].y*m_imageW) + mp_RotPix[filterIndex].x)*3;
				mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
				mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
				totWts += mp_Filter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;
		}
		if ( ( (i%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, i+doneSoFar, taskDone) )
			return false;
	}
	doneSoFar += maxXY;

	// --------------------------------------------------------------------------------------
	// blur inbetween
	// --------------------------------------------------------------------------------------
	int iTemp;
	for (j=1; j<=maxXY; j++) {
		for (i=j+1; i<=maxXY; i++) {
			theta = atan2( j, i );
			dist = sqrt( sqr(i) + sqr(j) );
			distRatio = dist/maxDist;
			pixRad = distRatio * m_pixRad;
			m_inRad = round(pixRad*(1-m_trail)); m_outRad = round(pixRad*outMult*(1+m_trail));
			if ( !calcRotPixOffsets(theta) )
				return false;
			if ( m_szFilter < m_numRotPix )
			{
				if (mp_Filter)
					delete[] mp_Filter;
				m_szFilter = m_numRotPix;
				mp_Filter = new float[m_szFilter];
			}
			calcGaussWts(mp_Filter, m_inRot, m_outRot);

			// +x, +y -------------------------------------------------------------------
			x = int(m_orig.x)+i;
			y = int(m_orig.y)+j;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+mp_RotPix[filterIndex].x, y+mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex + (mp_RotPix[filterIndex].y*m_imageW) + mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// -x, +y -------------------------------------------------------------------
			x = int(m_orig.x)-i;
			y = int(m_orig.y)+j;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x-mp_RotPix[filterIndex].x, y+mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex + (mp_RotPix[filterIndex].y*m_imageW) - mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// -x, -y -------------------------------------------------------------------
			x = int(m_orig.x)-i;
			y = int(m_orig.y)-j;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x-mp_RotPix[filterIndex].x, y-mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex - (mp_RotPix[filterIndex].y*m_imageW) - mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// +x, -y -------------------------------------------------------------------
			x = int(m_orig.x)+i;
			y = int(m_orig.y)-j;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+mp_RotPix[filterIndex].x, y-mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex - (mp_RotPix[filterIndex].y*m_imageW) + mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// flip xy in mp_RotPix
			for (int k=0; k<m_numRotPix; k++)
			{
				iTemp = mp_RotPix[k].x;
				mp_RotPix[k].x = mp_RotPix[k].y;
				mp_RotPix[k].y = iTemp;
			}

			// +x, +y -------------------------------------------------------------------
			x = int(m_orig.x)+j;
			y = int(m_orig.y)+i;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+mp_RotPix[filterIndex].x, y+mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex + (mp_RotPix[filterIndex].y*m_imageW) + mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// -x, +y -------------------------------------------------------------------
			x = int(m_orig.x)-j;
			y = int(m_orig.y)+i;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x-mp_RotPix[filterIndex].x, y+mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex + (mp_RotPix[filterIndex].y*m_imageW) - mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// -x, -y -------------------------------------------------------------------
			x = int(m_orig.x)-j;
			y = int(m_orig.y)-i;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

				// average pixels
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x-mp_RotPix[filterIndex].x, y-mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex - (mp_RotPix[filterIndex].y*m_imageW) - mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}

			// +x, -y -------------------------------------------------------------------
			x = int(m_orig.x)+j;
			y = int(m_orig.y)-i;
			if ( inBounds(x, y, m_imageW, m_imageH) )
			{
				// average pixels
				mapIndex = x + y*m_imageW;
				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;
				for (filterIndex=0; filterIndex<m_numRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+mp_RotPix[filterIndex].x, y-mp_RotPix[filterIndex].y, m_imageW, m_imageH) )
							continue;

					p = (mapIndex - (mp_RotPix[filterIndex].y*m_imageW) + mp_RotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_Filter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_Filter[filterIndex];
					totWts += mp_Filter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}
		}
		if ( ( (j%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, 40*j+doneSoFar, taskDone) )
			return false;
	}

	if (mapTo)
	{
		int checkInterval(9*m_imageW);
		// blend back to source
		int srcIdx;
		for (mapIndex=0; mapIndex<m_imageSz; mapIndex++)
		{
			if (pCompMap)
				blendPixel(mapIndex, mapFrom, mp_blurCache[mapIndex], (*pCompMap)[mapIndex].brighten, (*pCompMap)[mapIndex].blend, mapTo, alphaFrom, alphaTo);
			else
			{
				srcIdx = mapIndex*3;
				mapTo[srcIdx]   = (USHORT)(mp_blurCache[mapIndex].r * MAX_COL16);
				mapTo[srcIdx+1] = (USHORT)(mp_blurCache[mapIndex].g * MAX_COL16);
				mapTo[srcIdx+2] = (USHORT)(mp_blurCache[mapIndex].b * MAX_COL16);
				if (alphaTo)
					alphaTo[mapIndex] = (USHORT)(mp_blurCache[mapIndex].a * MAX_COL16);
			}
		}
	}
	return true;
}

void BlurRadial::blur(TimeValue t, CompMap *pCompMap, Bitmap *bm, RenderGlobalContext *gc)
{
	// get source bitmap data
	int type;
	mp_srcAlpha = (WORD*)bm->GetAlphaPtr(&type);
	mp_srcMap = (WORD*)bm->GetStoragePtr(&type);
	assert(type == BMM_TRUE_48);

	// if source bitmap has changed since last call
	if ( m_lastBMModifyID != bm->GetModifyID() )
	{
		m_lastBMModifyID = bm->GetModifyID();
		m_blurValid = false;
		if ( (bm->Width() != m_imageW) || (bm->Height() != m_imageH) )
		{
			m_imageW = bm->Width();
			m_imageH = bm->Height();
			m_imageSz = m_imageW * m_imageH;
		}
	}

	// get ui parameters
	Interval iValid = FOREVER;
	float fTemp;
	mp_blurMgr->getBlurValue(prmRadialPixRad, t, fTemp, iValid);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_pixRad = (int)( 0.5*floor( max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC) ) );

	mp_blurMgr->getBlurValue(prmRadialTrail, t, fTemp, iValid);
	LimitValue(fTemp, -100.0f, 100.0f); // mjm - 9.30.99
	m_trail = fTemp*PERCENT2DEC;

	mp_blurMgr->getBlurValue(prmRadialAlpha, t, m_affectAlpha, iValid);

	mp_blurMgr->getBlurValue(prmRadialUseNode, t, m_useNode, iValid);

	if (m_useNode)
	{
		mp_blurMgr->getBlurValue(prmRadialNode, 0, m_node, iValid); // not animatable -- use time = 0
		if (!m_node)
			return;
		RenderInfo* ri = bm->GetRenderInfo();
		Interval tmValid = FOREVER;
		Matrix3 nodeTM = m_node->GetObjTMAfterWSM(t, &tmValid);
		iValid &= tmValid;
		Point3 nodeOrigin = nodeTM.GetRow(3);
		m_orig = ri->MapWorldToScreen(nodeOrigin); // TODO: support fields
		// ensure integer values
		round(m_orig);
	}
	else
	{
		int tempInt;
		mp_blurMgr->getBlurValue(prmRadialXOrig, t, tempInt, iValid);
		LimitValue(tempInt, -999999, 999999); // mjm - 9.30.99
		m_orig.x = (float)tempInt;

		mp_blurMgr->getBlurValue(prmRadialYOrig, t, tempInt, iValid);
		LimitValue(tempInt, -999999, 999999); // mjm - 9.30.99
		m_orig.y = (float)tempInt;
	}
	mp_blurMgr->getSelValue(prmGenBrightType, t, m_brightenType, FOREVER);

	// if old parameter block values are no longer valid
	if ( !(iValid == FOREVER) && !(m_validInterval.InInterval(iValid)) )
		m_blurValid = false;
	m_validInterval = iValid;

	// setup buffers
	if (m_imageSz > m_mapSz)
	{
		if (mp_scratchMap)
			delete[] mp_scratchMap;
		mp_scratchMap = new AColor[m_imageSz];

		if (mp_blurCache)
			delete[] mp_blurCache;
		mp_blurCache = new AColor[m_imageSz];

		m_mapSz = m_imageSz;
	}

	m_blurValid = doBlur(pCompMap, mp_srcMap, mp_srcMap, mp_srcAlpha, (m_affectAlpha) ? mp_srcAlpha : NULL);
};
