/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selImage.h

	 DESCRIPTION: whole image selection type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_SEL_IMAGE_H_INCLUDED_)
#define _SEL_IMAGE_H_INCLUDED_

#include "selBase.h"

class SelImage : public SelBase
{
protected:
	void updateParams();

public:
	SelImage() { }
	SelImage(BlurMgr *const mgr) : SelBase(mgr) { }
	~SelImage() { }
	virtual void notifyPrmChanged(int prmID) { }
	virtual float getBrighten(TimeValue t);
	virtual float getBlend(TimeValue t);
};

#endif // !defined(_SEL_IMAGE_H_INCLUDED_)
