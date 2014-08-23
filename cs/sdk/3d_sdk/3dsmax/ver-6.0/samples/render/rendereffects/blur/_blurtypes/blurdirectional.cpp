/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurDirectional.cpp

	 DESCRIPTION: directional blur type - class definitions

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

// precompiled header
//#include "pch.h"
#include "..\pch.h" // Martell 1/10/00: Proton can't find pch.h without this.


// local includes
#include "blurDirectional.h"
#include "..\blurMgr.h"

#ifdef INTEL_COMPILER
  #include "xmmintrin.h"
#endif

BlurDirectional::~BlurDirectional()
{
	if (mp_blurCache)
		delete[] mp_blurCache;
	if (mp_scratchMap)
		delete[] mp_scratchMap;
}

void BlurDirectional::notifyPrmChanged(int prmID)
{
	switch (prmID)
	{
		case prmDirRot:
			m_rotValid = false;
		case prmDirUPixRad:
		case prmDirVPixRad:
		case prmDirUTrail:
		case prmDirVTrail:
			m_blurValid = false;
		case prmDirAlpha:
			break;
	}
}

bool BlurDirectional::calcRotUPixOffsets()
{
	double c( cos(m_rot) ), s( sin(m_rot) );
	int x1(0), y1(0), x2(0), y2(0);

//	if (m_leftRad) // count pixels left of center
	if (m_leftRad>0) // count pixels left of center // mjm - 10.18.99
	{
		x1 = -round(m_leftRad*c); y1 = -round(m_leftRad*s);
		m_leftRot = findRotPixels(x1, y1, 0, 0) - 1; // don't count origin
	}
	else
		m_leftRot = 0;

//	if (m_rightRad) // count pixels right of center
	if (m_rightRad>0) // count pixels right of center // mjm - 10.18.99
	{
		x2 = round(m_rightRad*c); y2 = round(m_rightRad*s);
		m_rightRot = findRotPixels(0, 0, x2, y2) - 1; // don't count origin
	}
	else
		m_rightRot = 0;

	m_numURotPix = m_leftRot+m_rightRot+1;
	if ( m_szURotPix < m_numURotPix )
	{
		if (mp_uRotPix)
			delete[] mp_uRotPix;
		m_szURotPix = m_numURotPix;
		mp_uRotPix = new IPoint2[m_szURotPix];
		if (!mp_uRotPix)
			return false;
	}

	if (m_numURotPix == 1)
	{
		mp_uRotPix[0].x = mp_uRotPix[0].y = 0;
		return true;
	}

//	if (m_leftRad) // record pixels left of center
	if (m_leftRad>0) // record pixels left of center // mjm - 10.18.99
		findRotPixels(x1, y1, 0, 0, mp_uRotPix, m_numURotPix); // last pixel recorded will be origin

//	if (m_rightRad) // record pixels right of center
	if (m_rightRad>0) // record pixels right of center // mjm - 10.18.99
		findRotPixels(0, 0, x2, y2, mp_uRotPix+m_leftRot, m_numURotPix-m_leftRot); // first pixel recorded will be origin

	return true;
}

bool BlurDirectional::calcRotVPixOffsets()
{
	float c( (float)cos(m_rot) ), s( (float)sin(m_rot) );
	int x1(0), y1(0), x2(0), y2(0);

//	if (m_upRad) // count pixels above center
	if (m_upRad>0) // count pixels above center // mjm - 10.18.99
	{
		x1 = round(m_upRad*s); y1 = -round(m_upRad*c);
		m_upRot = findRotPixels(x1, y1, 0, 0) - 1; // don't count origin
	}
	else
		m_upRot = 0;

//	if (m_downRad) // count pixels below center
	if (m_downRad>0) // count pixels below center // mjm - 10.18.99
	{
		x2 = -round(m_downRad*s); y2 = round(m_downRad*c);
		m_downRot = findRotPixels(0, 0, x2, y2) - 1; // don't count origin
	}
	else
		m_downRot = 0;

	m_numVRotPix = m_upRot+m_downRot+1;
	if ( m_szVRotPix < m_numVRotPix )
	{
		if (mp_vRotPix)
			delete[] mp_vRotPix;
		m_szVRotPix = m_numVRotPix;
		mp_vRotPix = new IPoint2[m_szVRotPix];
		if (!mp_vRotPix)
			return false;
	}

	if (m_numVRotPix == 1)
	{
		mp_vRotPix[0].x = mp_vRotPix[0].y = 0;
		return true;
	}

//	if (m_upRad) // record pixels above center
	if (m_upRad>0) // record pixels above center // mjm - 10.18.99
		findRotPixels(x1, y1, 0, 0, mp_vRotPix, m_numVRotPix); // last pixel recorded will be origin

//	if (m_downRad) // record pixels below center
	if (m_downRad>0) // record pixels below center // mjm - 10.18.99
		findRotPixels(0, 0, x2, y2, mp_vRotPix+m_upRot, m_numVRotPix-m_upRot); // first pixel recorded will be origin

	return true;
}

bool BlurDirectional::do1DBoxRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	float totWts;
	int x,y,p,mapIndex,filterIndex,taskDone;
	mp_nameCurProcess = GetString(IDS_PROCESS_BLUR);

	if (m_uPixRad) // blur only in u direction
	{
		// compute 1D gaussian filter in U direction
		int leftRad( round(m_uPixRad*(1-m_uTrail)) ), rightRad( round(m_uPixRad*(1+m_uTrail)) );
		if ( (leftRad != m_leftRad) || (rightRad != m_rightRad) || !m_rotValid )
		{
			m_leftRad = leftRad; m_rightRad = rightRad;
			if ( !calcRotUPixOffsets() )
				return false;

			if ( m_szUFilter < m_numURotPix )
			{
				if (mp_uFilter)
					delete[] mp_uFilter;
				m_szUFilter = m_numURotPix;
				mp_uFilter = new float[m_szUFilter];
			}
			calcGaussWts(mp_uFilter, m_leftRot, m_rightRot);
			m_rotValid = true;
		}

		mapIndex = -1;
		taskDone = m_imageH;
		// blur all pixels in x dir
		for (y=0; y<m_imageH; y++)
		{
			for (x=0; x<m_imageW; x++)
			{
				mapIndex++;

				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

				// average pixels
				for (filterIndex=0; filterIndex<m_numURotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+mp_uRotPix[filterIndex].x, y-mp_uRotPix[filterIndex].y, m_imageW, m_imageH) )
						continue;

					p = (mapIndex - (mp_uRotPix[filterIndex].y*m_imageW) + mp_uRotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_uFilter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_uFilter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_uFilter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_uFilter[filterIndex];
					totWts += mp_uFilter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}
			if ( ( (y%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, y, taskDone) )
				return false;
		}
	}
	else // blur only in v direction
	{
		// compute 1D gaussian filter in V direction
		int upRad( round(m_vPixRad*(1-m_vTrail)) ), downRad( round(m_vPixRad*(1+m_vTrail)) );
		if ( (upRad != m_upRad) || (downRad != m_downRad) )
		{
			m_upRad = upRad; m_downRad = downRad;
			if ( !calcRotVPixOffsets() )
				return false;

			if ( m_szVFilter < m_numVRotPix )
			{
				if (mp_vFilter)
					delete[] mp_vFilter;
				m_szVFilter = m_numVRotPix;
				mp_vFilter = new float[m_szVFilter];
			}
			calcGaussWts(mp_vFilter, m_upRot, m_downRot);
			m_rotValid = true;
		}

		taskDone = m_imageW;
		// blur all pixels in y dir
		for (x=0; x<m_imageW; x++)
		{
			for (y=0; y<m_imageH; y++)
			{
				mapIndex = x + y*m_imageW;

				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

				// average pixels
				for (filterIndex=0; filterIndex<m_numVRotPix; filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+mp_vRotPix[filterIndex].x, y-mp_vRotPix[filterIndex].y, m_imageW, m_imageH) )
						continue;
					p = (mapIndex - (mp_vRotPix[filterIndex].y*m_imageW) + mp_vRotPix[filterIndex].x)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_vFilter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_vFilter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_vFilter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_vFilter[filterIndex];
					totWts += mp_vFilter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}
			if ( ( (x%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, x, taskDone) )
				return false;
		}
	}
	if (mapTo)
	{
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

bool BlurDirectional::do1DBoxNoRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	float totWts;
	int x,y,u,v,p,mapIndex, filterIndex, taskDone;
	mp_nameCurProcess = GetString(IDS_PROCESS_BLUR);

	if (m_uPixRad) // blur only in u direction
	{
		// compute 1D gaussian filter in U direction
		int leftRad( round(m_uPixRad*(1-m_uTrail)) ), rightRad( round(m_uPixRad*(1+m_uTrail)) );
		if ( (leftRad != m_leftRad) || (rightRad != m_rightRad) )
		{
			m_leftRad = leftRad; m_rightRad = rightRad;
			if ( m_szUFilter < leftRad+rightRad+1 )
			{
				if (mp_uFilter)
					delete[] mp_uFilter;
				m_szUFilter = leftRad+rightRad+1;
				mp_uFilter = new float[m_szUFilter];
			}
			calcGaussWts(mp_uFilter, m_leftRad, m_rightRad);
		}

		mapIndex = -1;
		taskDone = m_imageH;
		// blur all pixels in x dir
		for (y=0; y<m_imageH; y++)
		{
			for (x=0; x<m_imageW; x++)
			{
				mapIndex++;

				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

				// average pixels
				for (u=-m_leftRad, filterIndex=0; u<=m_rightRad; u++, filterIndex++)

				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+u, y, m_imageW, m_imageH) )
						continue;

					// compute offsets
					p = (mapIndex+u)*3; // align with mapFrom's rgb triplet
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_uFilter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_uFilter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_uFilter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_uFilter[filterIndex];
					totWts += mp_uFilter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}
			if ( ( (y%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, y, taskDone) )
				return false;
		}
	}
	else // blur only in v direction
	{
		// compute 1D gaussian filter in V direction
		int upRad( round(m_vPixRad*(1-m_vTrail)) ), downRad( round(m_vPixRad*(1+m_vTrail)) );
		if ( (upRad != m_upRad) || (downRad != m_downRad) )
		{
			m_upRad = upRad; m_downRad = downRad;
			if ( m_szVFilter < upRad+downRad+1 )
			{
				if (mp_vFilter)
					delete[] mp_vFilter;
				m_szVFilter = upRad+downRad+1;
				mp_vFilter = new float[m_szVFilter];
			}
			calcGaussWts(mp_vFilter, upRad, downRad);
		}

		taskDone = m_imageW;
		// blur all pixels in y dir
		for (x=0; x<m_imageW; x++) {
			for (y=0; y<m_imageH; y++) {
				mapIndex = x + y*m_imageW;

				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

				// average pixels
				for (v=-upRad, filterIndex=0; v<=downRad; v++, filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x, y+v, m_imageW, m_imageH) )
						continue;

					// compute offsets
					p = (mapIndex + v*m_imageW)*3;
					mp_blurCache[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_vFilter[filterIndex];
					mp_blurCache[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_vFilter[filterIndex];
					mp_blurCache[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_vFilter[filterIndex];
					mp_blurCache[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_vFilter[filterIndex];
					totWts += mp_vFilter[filterIndex];
				}
				mp_blurCache[mapIndex].r /= totWts;
				mp_blurCache[mapIndex].g /= totWts;
				mp_blurCache[mapIndex].b /= totWts;
				mp_blurCache[mapIndex].a /= totWts;
			}
			if ( ( (x%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, x, taskDone) )
				return false;
		}
	}
	if (mapTo)
	{
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

bool BlurDirectional::do2DBoxRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	float totWts;
	int x,y,p,mapIndex,filterIndex,taskDone,srcIdx;
	mp_nameCurProcess = GetString(IDS_PROCESS_BLUR);

	// compute 1D gaussian filter in U direction
	int leftRad( round(m_uPixRad*(1-m_uTrail)) ), rightRad( round(m_uPixRad*(1+m_uTrail)) );
	if ( (leftRad != m_leftRad) || (rightRad != m_rightRad) || !m_rotValid )
	{
		m_leftRad = leftRad; m_rightRad = rightRad;
		if ( !calcRotUPixOffsets() )
			return false;

		if ( m_szUFilter < m_numURotPix )
		{
			if (mp_uFilter)
				delete[] mp_uFilter;
			m_szUFilter = m_numURotPix;
			mp_uFilter = new float[m_szUFilter];
		}
		calcGaussWts(mp_uFilter, m_leftRot, m_rightRot);
		// m_rotValid false until mp_vFilter is calculated
	}

	mapIndex = -1;
	taskDone = m_imageH + m_imageW;
	// blur all pixels in x dir
	for (y=0; y<m_imageH; y++)
	{
		for (x=0; x<m_imageW; x++)
		{
			mapIndex++;

			mp_scratchMap[mapIndex].r = mp_scratchMap[mapIndex].g = mp_scratchMap[mapIndex].b = mp_scratchMap[mapIndex].a = 0.0f;
			totWts = 0.0f;

			// average pixels
			for (filterIndex=0; filterIndex<m_numURotPix; filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+mp_uRotPix[filterIndex].x, y-mp_uRotPix[filterIndex].y, m_imageW, m_imageH) )
					continue;

				p = (mapIndex - (mp_uRotPix[filterIndex].y*m_imageW) + mp_uRotPix[filterIndex].x)*3;
				mp_scratchMap[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_uFilter[filterIndex];
				mp_scratchMap[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_uFilter[filterIndex];
				mp_scratchMap[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_uFilter[filterIndex];
				mp_scratchMap[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_uFilter[filterIndex];
				totWts += mp_uFilter[filterIndex];
			}
			mp_scratchMap[mapIndex].r /= totWts;
			mp_scratchMap[mapIndex].g /= totWts;
			mp_scratchMap[mapIndex].b /= totWts;
			mp_scratchMap[mapIndex].a /= totWts;
		}
		if ( ( (y%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, y, taskDone) )
			return false;
	}

	// compute 1D gaussian filter in V direction
	int upRad( round(m_vPixRad*(1-m_vTrail)) ), downRad( round(m_vPixRad*(1+m_vTrail)) );
	if ( (upRad != m_upRad) || (downRad != m_downRad) || !m_rotValid )
	{
		m_upRad = upRad; m_downRad = downRad;
		if ( !calcRotVPixOffsets() )
			return false;

		if ( m_szVFilter < m_numVRotPix )
		{
			if (mp_vFilter)
				delete[] mp_vFilter;
			m_szVFilter = m_numVRotPix;
			mp_vFilter = new float[m_szVFilter];
		}
		calcGaussWts(mp_vFilter, m_upRot, m_downRot);
		m_rotValid = true;
	}

	// blur all pixels in y dir
	for (x=0; x<m_imageW; x++)
	{
		for (y=0; y<m_imageH; y++)
		{
			mapIndex = x + y*m_imageW;

				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

			// average pixels
			for (filterIndex=0; filterIndex<m_numVRotPix; filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+mp_vRotPix[filterIndex].x, y-mp_vRotPix[filterIndex].y, m_imageW, m_imageH) )
					continue;
				p = (mapIndex - (mp_vRotPix[filterIndex].y*m_imageW) + mp_vRotPix[filterIndex].x);
				mp_blurCache[mapIndex].r += mp_scratchMap[p].r * mp_vFilter[filterIndex];
				mp_blurCache[mapIndex].g += mp_scratchMap[p].g * mp_vFilter[filterIndex];
				mp_blurCache[mapIndex].b += mp_scratchMap[p].b * mp_vFilter[filterIndex];
				mp_blurCache[mapIndex].a += mp_scratchMap[p].a * mp_vFilter[filterIndex];
				totWts += mp_vFilter[filterIndex];
			}
			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;

			if (mapTo)
			{
				// blend back to source
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
		if ( ( (x%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, m_imageH+x, taskDone) )
			return false;
	}
	return true;
}

extern BOOL gBlurSSE_Enabled; // Martell 2/19/01: SSE Support
bool BlurDirectional::do2DBoxNoRot(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
#ifdef INTEL_COMPILER
	if ( gBlurSSE_Enabled )
	{
		float totWts;
		int x,y,u,v,p,mapIndex,filterIndex,taskDone, srcIdx;
		__m128	xmm0,xmm1,xmm2,xmm3, xmm4, xmm7;
		float *ptr;
		int vadd; 

		mp_nameCurProcess = GetString(IDS_PROCESS_BLUR);

		// compute 1D gaussian filter in U direction
		int leftRad( round(m_uPixRad*(1-m_uTrail)) ), rightRad( round(m_uPixRad*(1+m_uTrail)) );
		if ( (leftRad != m_leftRad) || (rightRad != m_rightRad) )
		{
			m_leftRad = leftRad; m_rightRad = rightRad;
			if ( m_szUFilter < leftRad+rightRad+1 )
			{
				if (mp_uFilter)
					delete[] mp_uFilter;
				m_szUFilter = leftRad+rightRad+1;
				mp_uFilter = new float[m_szUFilter];
			}
			calcGaussWts(mp_uFilter, m_leftRad, m_rightRad);
		}

		mapIndex = -1;
		taskDone = m_imageH + m_imageW;

		__declspec (align(16)) static float word2normelt[4]={WORD2NORMFLT,WORD2NORMFLT,WORD2NORMFLT,WORD2NORMFLT};
		//xmm7 = _mm_set_ps1(WORD2NORMFLT);	// xmm7 = [WORD2NORMFLT,..,..,WORD2NORMFLT]

		// blur all pixels in x dir
		for (y=0; y<m_imageH; y++) {
			for (x=0; x<m_imageW; x++) {
				mapIndex++;

				mp_scratchMap[mapIndex].r = mp_scratchMap[mapIndex].g = mp_scratchMap[mapIndex].b = mp_scratchMap[mapIndex].a = 0.0f;
				totWts = 0.0f;

				xmm0 = _mm_setzero_ps();
				// average pixels
				for (u=-m_leftRad, filterIndex=0; u<=m_rightRad; u++, filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x+u, y, m_imageW, m_imageH) )
						continue;

					// compute offsets
					p = (mapIndex+u)*3; // align with mapFrom's rgb triplet
					xmm1 = _mm_set_ps1(mp_uFilter[filterIndex]);
					xmm7 = _mm_load_ps(word2normelt); 
					xmm2 = _mm_set_ps((float)alphaFrom[p/3],(float)mapFrom[p+2],(float)mapFrom[p+1],(float)mapFrom[p]);
					xmm1 = _mm_mul_ps(xmm1, xmm7);
					xmm1 = _mm_mul_ps(xmm1, xmm2);
					xmm0 = _mm_add_ps(xmm0, xmm1);

					//mp_scratchMap[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_uFilter[filterIndex];
					//mp_scratchMap[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_uFilter[filterIndex];
					//mp_scratchMap[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_uFilter[filterIndex];
					//mp_scratchMap[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_uFilter[filterIndex];
					totWts += mp_uFilter[filterIndex];
				}

				xmm3 = _mm_set_ss(totWts);	// xmm3 = [*,*,*,totWts]
				xmm3 = _mm_rcp_ss(xmm3);
				ptr = mp_scratchMap[mapIndex];
				xmm3 = _mm_unpacklo_ps(xmm3,xmm3); // xmm3 = [*,*,1/totWts, 1/totWts]
				xmm3 = _mm_unpacklo_ps(xmm3,xmm3); // xmm3 = [1/totWts,1/totWts,1/totWts, 1/totWts]
				xmm0 = _mm_mul_ps(xmm0,xmm3);
				
				//mp_scratchMap[mapIndex].r /= totWts;
				//mp_scratchMap[mapIndex].g /= totWts;
				//mp_scratchMap[mapIndex].b /= totWts;
				//mp_scratchMap[mapIndex].a /= totWts;
				_mm_storel_pi((__m64 *)ptr, xmm0);
				ptr +=2;
				_mm_storeh_pi((__m64 *)ptr, xmm0);
				
			}
			if ( ( (y%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, y, taskDone) )
				return false;
		}

		// compute 1D gaussian filter in V direction
		int upRad( round(m_vPixRad*(1-m_vTrail)) ), downRad( round(m_vPixRad*(1+m_vTrail)) );
		if ( (upRad != m_upRad) || (downRad != m_downRad) )
		{
			m_upRad = upRad; m_downRad = downRad;
			if ( m_szVFilter < upRad+downRad+1 )
			{
				if (mp_vFilter)
					delete[] mp_vFilter;
				m_szVFilter = upRad+downRad+1;
				mp_vFilter = new float[m_szVFilter];
			}
			calcGaussWts(mp_vFilter, upRad, downRad);
		}

		// blur all pixels in y dir
		for (x=0; x<m_imageW; x++) {
			for (y=0; y<m_imageH; y++) {
				mapIndex = x + y*m_imageW;

				mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
				totWts = 0.0f;

				xmm0 = _mm_set_ps(mp_blurCache[mapIndex].a,mp_blurCache[mapIndex].b,
					mp_blurCache[mapIndex].g,mp_blurCache[mapIndex].r);

				vadd = -upRad*m_imageW;
				// average pixels
				for (v=-upRad, filterIndex=0; v<=downRad; v++, filterIndex++)
				{
					// for speed, the filter size is reduced at edges of image
					// rather than extrapolating color values for non-existing pixels
					if ( !inBounds(x, y+v, m_imageW, m_imageH) ){
						vadd += m_imageW;
						continue;
					}

					// compute offsets
					//p = mapIndex + v*m_imageW;
					p = mapIndex + vadd;
					vadd += m_imageW;
					xmm1 = _mm_set_ps(mp_scratchMap[p].a,mp_scratchMap[p].b,
							mp_scratchMap[p].g,mp_scratchMap[p].r);
					xmm2 = _mm_set_ps1(mp_vFilter[filterIndex]);

					xmm1 = _mm_mul_ps(xmm1, xmm2);
					xmm0 = _mm_add_ps(xmm0, xmm1);

					//mp_blurCache[mapIndex].r += mp_scratchMap[p].r * mp_vFilter[filterIndex];
					//mp_blurCache[mapIndex].g += mp_scratchMap[p].g * mp_vFilter[filterIndex];
					//mp_blurCache[mapIndex].b += mp_scratchMap[p].b * mp_vFilter[filterIndex];
					//mp_blurCache[mapIndex].a += mp_scratchMap[p].a * mp_vFilter[filterIndex];
					totWts += mp_vFilter[filterIndex];
				}

				//one_totWts = 1.0/totWts;
				//xmm1 = _mm_set_ps1(one_totWts);
				xmm3 = _mm_set_ss(totWts);
				xmm3 = _mm_rcp_ss(xmm3);

				xmm3 = _mm_unpacklo_ps(xmm3,xmm3); // xmm3 = [*,*,1/totWts, 1/totWts]
				xmm3 = _mm_unpacklo_ps(xmm3,xmm3); 
				ptr = mp_blurCache[mapIndex];

				xmm0 = _mm_mul_ps(xmm0,xmm3);

				//mp_blurCache[mapIndex].r /= totWts;
				//mp_blurCache[mapIndex].g /= totWts;
				//mp_blurCache[mapIndex].b /= totWts;
				//mp_blurCache[mapIndex].a /= totWts;

				_mm_storel_pi((__m64 *)ptr, xmm0);
				ptr +=2;
				_mm_storeh_pi((__m64 *)ptr, xmm0);

				if (mapTo)
				{
					// blend back to source
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
			if ( ( (x%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, m_imageH+x, taskDone) )
				return false;
		}
		return true;
	}
	else
#endif //INTEL_COMPILER
	{
	float totWts;
	int x,y,u,v,p,mapIndex,filterIndex,taskDone, srcIdx;
	mp_nameCurProcess = GetString(IDS_PROCESS_BLUR);

	// compute 1D gaussian filter in U direction
	int leftRad( round(m_uPixRad*(1-m_uTrail)) ), rightRad( round(m_uPixRad*(1+m_uTrail)) );
	if ( (leftRad != m_leftRad) || (rightRad != m_rightRad) )
	{
		m_leftRad = leftRad; m_rightRad = rightRad;
		if ( m_szUFilter < leftRad+rightRad+1 )
		{
			if (mp_uFilter)
				delete[] mp_uFilter;
			m_szUFilter = leftRad+rightRad+1;
			mp_uFilter = new float[m_szUFilter];
		}
		calcGaussWts(mp_uFilter, m_leftRad, m_rightRad);
	}

	mapIndex = -1;
	taskDone = m_imageH + m_imageW;
	// blur all pixels in x dir
	for (y=0; y<m_imageH; y++) {
		for (x=0; x<m_imageW; x++) {
			mapIndex++;

			mp_scratchMap[mapIndex].r = mp_scratchMap[mapIndex].g = mp_scratchMap[mapIndex].b = mp_scratchMap[mapIndex].a = 0.0f;
			totWts = 0.0f;

			// average pixels
			for (u=-m_leftRad, filterIndex=0; u<=m_rightRad; u++, filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x+u, y, m_imageW, m_imageH) )
					continue;

				// compute offsets
				p = (mapIndex+u)*3; // align with mapFrom's rgb triplet
				mp_scratchMap[mapIndex].r += mapFrom[p]  *WORD2NORMFLT * mp_uFilter[filterIndex];
				mp_scratchMap[mapIndex].g += mapFrom[p+1]*WORD2NORMFLT * mp_uFilter[filterIndex];
				mp_scratchMap[mapIndex].b += mapFrom[p+2]*WORD2NORMFLT * mp_uFilter[filterIndex];
				mp_scratchMap[mapIndex].a += alphaFrom[p/3]*WORD2NORMFLT * mp_uFilter[filterIndex];
				totWts += mp_uFilter[filterIndex];
			}
			mp_scratchMap[mapIndex].r /= totWts;
			mp_scratchMap[mapIndex].g /= totWts;
			mp_scratchMap[mapIndex].b /= totWts;
			mp_scratchMap[mapIndex].a /= totWts;
		}
		if ( ( (y%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, y, taskDone) )
			return false;
	}

	// compute 1D gaussian filter in V direction
	int upRad( round(m_vPixRad*(1-m_vTrail)) ), downRad( round(m_vPixRad*(1+m_vTrail)) );
	if ( (upRad != m_upRad) || (downRad != m_downRad) )
	{
		m_upRad = upRad; m_downRad = downRad;
		if ( m_szVFilter < upRad+downRad+1 )
		{
			if (mp_vFilter)
				delete[] mp_vFilter;
			m_szVFilter = upRad+downRad+1;
			mp_vFilter = new float[m_szVFilter];
		}
		calcGaussWts(mp_vFilter, upRad, downRad);
	}

	// blur all pixels in y dir
	for (x=0; x<m_imageW; x++) {
		for (y=0; y<m_imageH; y++) {
			mapIndex = x + y*m_imageW;

			mp_blurCache[mapIndex].r = mp_blurCache[mapIndex].g = mp_blurCache[mapIndex].b = mp_blurCache[mapIndex].a = 0.0f;
			totWts = 0.0f;

			// average pixels
			for (v=-upRad, filterIndex=0; v<=downRad; v++, filterIndex++)
			{
				// for speed, the filter size is reduced at edges of image
				// rather than extrapolating color values for non-existing pixels
				if ( !inBounds(x, y+v, m_imageW, m_imageH) )
					continue;

				// compute offsets
				p = mapIndex + v*m_imageW;
				mp_blurCache[mapIndex].r += mp_scratchMap[p].r * mp_vFilter[filterIndex];
				mp_blurCache[mapIndex].g += mp_scratchMap[p].g * mp_vFilter[filterIndex];
				mp_blurCache[mapIndex].b += mp_scratchMap[p].b * mp_vFilter[filterIndex];
				mp_blurCache[mapIndex].a += mp_scratchMap[p].a * mp_vFilter[filterIndex];
				totWts += mp_vFilter[filterIndex];
			}

			mp_blurCache[mapIndex].r /= totWts;
			mp_blurCache[mapIndex].g /= totWts;
			mp_blurCache[mapIndex].b /= totWts;
			mp_blurCache[mapIndex].a /= totWts;

			if (mapTo)
			{
				// blend back to source
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
		if ( ( (x%3) == 0 ) && mp_blurMgr->progress(mp_nameCurProcess, m_imageH+x, taskDone) )
			return false;
	}
	return true;
}
}

bool BlurDirectional::doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo)
{
	if(m_blurValid && mapTo)
	{
		int srcIdx;
		for (int i=0; i<m_imageSz; i++)
			if (pCompMap)
			{
				blendPixel(i, mapFrom, mp_blurCache[i], (*pCompMap)[i].brighten, (*pCompMap)[i].blend, mapTo, alphaFrom, alphaTo);
			}
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

	// adjust so 0 <= angle < PI/2
	if (m_rot >= PIOVER2)
	{
		int iTemp(m_uPixRad);
		m_uPixRad = m_vPixRad;
		m_vPixRad = iTemp;

		float fTemp(m_uTrail);
		m_uTrail = m_vTrail;
		m_vTrail = fTemp;

		m_rot -= PIOVER2;
	}

	if ( m_uPixRad == m_vPixRad )
	{
		return do2DBoxNoRot(pCompMap, mapFrom, mapTo, alphaFrom, alphaTo);
	}

	if ( m_rot == 0.0f )
	{
		if ( (!m_uPixRad && m_vPixRad) || (m_uPixRad && !m_vPixRad) )
			return do1DBoxNoRot(pCompMap, mapFrom, mapTo, alphaFrom, alphaTo);
		else
			return do2DBoxNoRot(pCompMap, mapFrom, mapTo, alphaFrom, alphaTo);
	}
	else
	{
		if ( (!m_uPixRad && m_vPixRad) || (m_uPixRad && !m_vPixRad) )
			return do1DBoxRot(pCompMap, mapFrom, mapTo, alphaFrom, alphaTo);
		else
			return do2DBoxRot(pCompMap, mapFrom, mapTo, alphaFrom, alphaTo);
	}
}

void BlurDirectional::blur(TimeValue t, CompMap *pCompMap, Bitmap *bm, RenderGlobalContext *gc)
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
	int iTemp;
	float fTemp;
	mp_blurMgr->getBlurValue(prmDirUPixRad, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_uPixRad = (int)( 0.5*floor( max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC) ) );

	mp_blurMgr->getBlurValue(prmDirVPixRad, t, fTemp, FOREVER);
	LimitValue(fTemp, 0.0f, 1000.0f); // mjm - 9.30.99
	m_vPixRad = (int)( 0.5*floor( max(m_imageW, m_imageH)*(fTemp*PERCENT2DEC) ) );

	mp_blurMgr->getBlurValue(prmDirUTrail, t, fTemp, FOREVER);
	LimitValue(fTemp, -100.0f, 100.0f); // mjm - 9.30.99
	m_uTrail = fTemp*PERCENT2DEC;

	mp_blurMgr->getBlurValue(prmDirVTrail, t, fTemp, FOREVER);
	LimitValue(fTemp, -100.0f, 100.0f); // mjm - 9.30.99
	m_vTrail = fTemp*PERCENT2DEC;

	mp_blurMgr->getBlurValue(prmDirRot, t, iTemp, FOREVER);
	LimitValue(iTemp, -180, 180); // mjm - 9.30.99
	m_rot = (float)fmod(iTemp*DEG2RAD, PI);

	mp_blurMgr->getBlurValue(prmDirAlpha, t, m_affectAlpha, FOREVER);
	mp_blurMgr->getSelValue(prmGenBrightType, t, m_brightenType, FOREVER);

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
