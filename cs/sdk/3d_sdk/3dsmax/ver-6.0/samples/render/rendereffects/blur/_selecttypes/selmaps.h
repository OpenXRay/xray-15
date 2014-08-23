/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selMaps.h

	 DESCRIPTION: maps selection type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created November 4, 1998

	 Copyright (c) 1998, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_SEL_MAPS_H_INCLUDED_)
#define _SEL_MAPS_H_INCLUDED_

#include "SelLum.h"
#include "scTex.h"

class SelMaps : public SelLum
{
enum { red, green, blue, alpha, luminance };

protected:
	SCTex m_shadeContext;
	Texmap *m_brightenMap;
	int m_channel;

	bool testPixel(int index);
	virtual bool doSelect();

public:
	SelMaps() : m_brightenMap(NULL) { }
	SelMaps(BlurMgr *const mgr) : m_brightenMap(NULL), SelLum(mgr) { }
	~SelMaps();
	virtual void notifyPrmChanged(int prmID);
	virtual void select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_SEL_MAPS_H_INCLUDED_)
