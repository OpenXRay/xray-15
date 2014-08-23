/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: globals.h

	 DESCRIPTION: global access to identifiers

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_GLOBALS_H_INCLUDED_)
#define _GLOBALS_H_INCLUDED_

namespace blurGlobals {

	// primary IDs
	enum { idMaster, idBlurData, idSelData, idSelCurveCtrl, numIDs };

	// parameter IDs
	enum // blur data parameters
	{
		prmBlurType,
		prmUnifPixRad, prmUnifAlpha,
		prmDirUPixRad, prmDirVPixRad, prmDirUTrail, prmDirVTrail, prmDirRot, prmDirAlpha,
		prmRadialPixRad, prmRadialTrail, prmRadialAlpha, prmRadialXOrig, prmRadialYOrig, prmRadialUseNode, prmRadialNode,
	};

	enum // selection data parameters
	{
		prmImageActive, prmIBackActive, prmLumActive, prmMaskActive,
		prmImageBrighten, prmImageBlend,
		prmIBackBrighten, prmIBackBlend, prmIBackFeathRad,
		prmLumBrighten, prmLumBlend, prmLumMin, prmLumMax, prmLumFeathRad,
		prmMaskMap, prmMaskChannel, prmMaskBrighten, prmMaskBlend, prmMaskMin, prmMaskMax, prmMaskFeathRad,
		prmObjIdsActive, prmObjIdsIds, prmObjIdsBrighten, prmObjIdsBlend, prmObjIdsFeathRad, prmObjIdsLumMin, prmObjIdsLumMax,
		prmMatIdsActive, prmMatIdsIds, prmMatIdsBrighten, prmMatIdsBlend, prmMatIdsFeathRad, prmMatIdsLumMin, prmMatIdsLumMax,
		prmGenBrightType,
	};

	// blur types
	enum { idBlurUnif, idBlurDir, idBlurRadial, numBlurs };

	// selection types
	enum { idSelImage, idSelIBack, idSelLum, idSelMask, idSelObjIds, idSelMatIds, numSels };

	// brighten types
	enum { idBrightenAdd, idBrightenMult, numBrightens };

	const int	MAX_COL16(65535);			// maximum 16 bit color value
	const float	PITIMES2(6.283185f);		// pi*2
	const float	PIOVER2(1.570796f);			// pi/2
	const float	PIOVER4(7.853982e-1f);		// pi/4
	const float	RT2OVER2(7.071068e-1f);		// (sqrt(2))/2
	const float	WORD2NORMFLT(1.525902e-5f);	// 16-bit WORD to normalized [0,1] float
	const float	BYTE2NORMFLT(3.921569e-3f);	// 8-bit UBYTE to normalized [0,1] float
	const float	DEG2RAD(1.745329e-2f);		// degrees to radians
	const float	RAD2DEG(5.729578e+1f);		// radians to degrees
	const float	PERCENT2DEC(0.01f);			// percent to decimal fraction

	inline int sgn(int x)
		{ return (x<0) ? -1 : 1; }
	inline int sqr(int i)
		{ return i*i; }
	inline float sqr(float f)
		{ return f*f; }
	inline double sqr(double d)
		{ return d*d; }
	inline void round(Point2 &pt)
		{ pt.x = (float)( (pt.x>0) ? (int)(pt.x+0.5f) : -(int)(0.5f-pt.x) );
		  pt.y = (float)( (pt.y>0) ? (int)(pt.y+0.5f) : -(int)(0.5f-pt.y) ); }
	inline int round(float x)
		{ return (x>0) ? (int)(x+0.5f) : -(int)(0.5f-x); }
	inline int round(double x)
		{ return (x>0) ? (int)(x+0.5) : -(int)(0.5-x); }
	inline bool inBounds(int x, int y, int w, int h)
		{ return (x>=0 && x<w && y>=0 && y<h); }
	inline WORD luminance16(WORD r, WORD g, WORD b)	// perceptually based luminance (from "A Technical Introduction to Digital Video", C. Poynton)
		{ return (WORD)(r*0.2125f + g*0.7154f + b*0.0721f); }
	inline float luminanceNormFloat(float r, float g, float b)
		{ return (r*0.2125f + g*0.7154f + b*0.0721f); }
	inline float luminanceNormFloat(WORD r, WORD g, WORD b)
		{ return (r*3.242542e-6f + g*1.091630e-5f + b*1.100175e-6f); } // lum factors pre-multiplied by WORD2NORMFLT
	inline float luminanceNormFloat(const Color24 &color)
		{ return (color.r*8.333333e-4f + color.g*2.805490e-3f + color.b*2.827451e-4f); } // lum factors pre-multiplied by BYTE2NORMFLT

	class CompWt
	{
	public:
		float brighten;
		float blend;
		CompWt() : brighten(0.0f), blend(0.0f) { }
		CompWt(float &_brighten, float &_blend) { brighten =_brighten; blend = _blend; }
		CompWt(CompWt &compWt) { brighten = compWt.brighten; blend = compWt.blend; }
	};

	class CompMap
	{
		CompWt *map;
		int size;

	public:
		CompMap() : map(NULL), size(0) { }

		~CompMap()
		{
			if (map)
				delete[] map;
		}

		CompWt& operator[](int i) { return map[i]; }

		void resize(int newSize)
		{
			if (newSize>size)
			{
				if (map)
					delete[] map;
				map = new CompWt[size = newSize];
			}
		}

		void set(float brighten, float blend, int num)
		{
			for (int i=0; i<num; i++)
			{
				map[i].brighten = brighten;
				map[i].blend = blend;
			}
		}
	};
}

using namespace blurGlobals;

#endif // !defined(_GLOBALS_H_INCLUDED_)
