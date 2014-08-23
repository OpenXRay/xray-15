/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurBase.h

	 DESCRIPTION: base blur type - class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_BLURBASE_H_INCLUDED_)
#define _BLURBASE_H_INCLUDED_

#include "max.h"
#include "..\globals.h"

class BlurMgr;

// ----------------------------------------
// base blur type - class declaration
// ----------------------------------------
class BlurBase
{
protected:
	BlurMgr *const mp_blurMgr;
	WORD *mp_srcMap, *mp_srcAlpha;
	int m_bufSz, m_mapSz, m_imageSz, m_imageW, m_imageH, m_brightenType;
	bool m_blurValid;
	BOOL m_affectAlpha;
	AColor *mp_blurCache, *mp_scratchMap;
	const TCHAR *mp_nameCurProcess;
	DWORD m_lastBMModifyID;

	int findRotPixels(int x1, int y1, int x2, int y2, IPoint2* buf = NULL, int sz = 0);
	void calcGaussWts(float *buf, int bufSz);
	void calcGaussWts(float *buf, int radA, int radB);
	void blendPixel(int index, WORD *mapFrom, AColor blendCol, float brighten, float blend, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo = NULL);
	virtual bool doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo) { return false; }

public:
	BlurBase() : mp_blurMgr(NULL), mp_srcMap(NULL), mp_srcAlpha(NULL),
		m_bufSz(0), m_mapSz(0), m_imageSz(0), m_imageW(0), m_imageH(0), m_brightenType(0), m_blurValid(false), m_affectAlpha(FALSE),
		mp_blurCache(NULL), mp_scratchMap(NULL), mp_nameCurProcess(NULL), m_lastBMModifyID(0xFFFFFFFF) { }
	BlurBase(BlurMgr *const mgr) : mp_blurMgr(mgr), mp_srcMap(NULL), mp_srcAlpha(NULL),
		m_bufSz(0), m_mapSz(0), m_imageSz(0), m_imageW(0), m_imageH(0), m_brightenType(0), m_blurValid(false), m_affectAlpha(FALSE),
		mp_blurCache(NULL), mp_scratchMap(NULL), mp_nameCurProcess(NULL), m_lastBMModifyID(0xFFFFFFFF) { }
	virtual ~BlurBase() { }
	virtual void notifyPrmChanged(int prmID) { }
	virtual void blur(TimeValue t, CompMap *compositeMap, Bitmap *bm, RenderGlobalContext *gc) { return; };
};

#endif // !defined(_BLURBASE_H_INCLUDED_)
