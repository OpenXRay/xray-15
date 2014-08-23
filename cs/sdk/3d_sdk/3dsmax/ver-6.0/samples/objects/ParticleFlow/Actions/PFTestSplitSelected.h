/**********************************************************************
 *<
	FILE: PFTestSplitSelected.h

	DESCRIPTION: SplitSelected Test header
				 The Test checks if a particles is selected or not

	CREATED BY: Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTSPLITSELECTED_H_
#define _PFTESTSPLITSELECTED_H_

#include "max.h"
#include "iparamb2.h"

#include "PFSimpleTest.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "PFClassIDs.h"

namespace PFActions {

class PFTestSplitSelected:	public PFSimpleTest 
{
public:
	// constructor/destructor
	PFTestSplitSelected();
	
	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next);

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);
	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	const	ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const	Interval ActivityInterval() const { return FOREVER; }

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetTruePViewIcon();
	HBITMAP GetFalsePViewIcon();

	// From IPFTest interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, BitArray& testResult, Tab<float>& testTime);

	// supply ClassDesc descriptor for the concrete implementation of the test
	ClassDesc* GetClassDesc() const;
};

} // end of namespace PFActions

#endif // _PFTESTSPLITSELECTED_H_

