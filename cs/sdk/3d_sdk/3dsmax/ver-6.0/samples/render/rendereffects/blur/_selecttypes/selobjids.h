/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: selObjIds.h

	 DESCRIPTION: object id selection type -  class declaration

	 CREATED BY: michael malone (mjm)

	 HISTORY: created July 29, 1999

	 Copyright (c) 1999, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#if !defined(_SEL_OBJIDS_H_INCLUDED_)
#define _SEL_OBJIDS_H_INCLUDED_

#include "SelLum.h"

class SelObjIds : public SelLum
{
protected:
	virtual bool doSelect();

public:
	static const int maxID;

	WORD *mp_srcObjId;
//	UBYTE *mp_srcCoverage;
	Color24 *mp_srcColor;
	int *mp_idList;
	int m_idCount, m_idListSz;

	SelObjIds() { }
//	SelObjIds(BlurMgr *const mgr) : mp_srcObjId(NULL), mp_srcCoverage(NULL), mp_idList(NULL), m_idCount(0), m_idListSz(0), SelLum(mgr) { }
	SelObjIds(BlurMgr *const mgr) : mp_srcObjId(NULL), mp_srcColor(NULL), mp_idList(NULL), m_idCount(0), m_idListSz(0), SelLum(mgr) { }
	~SelObjIds();
	bool IdInList(int objId)
	{
		for (int i=0; i<m_idCount; i++)
			if (objId == mp_idList[i])
				return true;
			return false;
	}
	virtual void notifyPrmChanged(int prmID);
	virtual void select(TimeValue t, CompMap &compMap, Bitmap *bm, RenderGlobalContext *gc);
};

#endif // !defined(_SEL_OBJIDS_H_INCLUDED_)
