/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: blurUniform.h

	 DESCRIPTION: uniform blur type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_BLUR_UNIFORM_H_INCLUDED_)
#define _BLUR_UNIFORM_H_INCLUDED_

#include "blurDirectional.h"

class BlurUniform : public BlurDirectional
{
protected:
	int m_pixRad;

	virtual bool doBlur(CompMap *pCompMap, WORD *mapFrom, WORD *mapTo, WORD *alphaFrom, WORD *alphaTo);

public:
	BlurUniform() : BlurDirectional() { }
	BlurUniform(BlurMgr *const mgr) : BlurDirectional(mgr) { }
	~BlurUniform();
	virtual void notifyPrmChanged(int prmID);
	virtual void blur(TimeValue t, CompMap *compositeMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_BLUR_UNIFORM_H_INCLUDED_)
