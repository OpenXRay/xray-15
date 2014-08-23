/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selIgnBack.h

	 DESCRIPTION: ignore background selection type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_SEL_IGN_BACK_H_INCLUDED_)
#define _SEL_IGN_BACK_H_INCLUDED_

#include "SelLum.h"

class SelIgnBack : public SelLum
{
protected:
	virtual bool doSelect();

public:
	SelIgnBack() { }
	SelIgnBack(BlurMgr *const mgr) : SelLum(mgr) { }
	~SelIgnBack();
	virtual void notifyPrmChanged(int prmID);
	virtual void select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_SEL_IGN_BACK_H_INCLUDED_)
