/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selLum.h

	 DESCRIPTION: luminance selection type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_SEL_LUM_H_INCLUDED_)
#define _SEL_LUM_H_INCLUDED_

#include "selBase.h"

class SelLum : public SelBase
{
protected:
	virtual bool doSelect();
	virtual bool doFeather();
	virtual float calcBrighten(TimeValue t, const float &normRadius);
	virtual float calcBlend(TimeValue t, const float &normRadius);
	virtual bool doComposite(TimeValue t, CompMap &compMap);

public:
	SelLum() { }
	SelLum(BlurMgr *const mgr) : SelBase(mgr) { }
	~SelLum();
	virtual void notifyPrmChanged(int prmID);
	virtual void select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_SEL_LUM_H_INCLUDED_)
