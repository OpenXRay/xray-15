/**********************************************************************
 *<
	FILE: PFTestScale.h

	DESCRIPTION: Scale Test header
				 The Test checks particles for either scale/size
				 properties of a particle

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTSCALE_H_
#define _PFTESTSCALE_H_

#include "max.h"
#include "iparamb2.h"

#include "PFSimpleTest.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "PFClassIDs.h"

namespace PFActions {

class PFTestScale:	public PFSimpleTest 
{
public:
	// constructor/destructor
	PFTestScale();
	
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

		// Scale Test supports randomness
	bool	SupportRand() const	{ return true; }
	int		GetRand();
	void	SetRand(int seed);

	const	ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const	Interval ActivityInterval() const { return FOREVER; }

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetTruePViewIcon();
	HBITMAP GetFalsePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// From IPFTest interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, BitArray& testResult, Tab<float>& testTime);

	// supply ClassDesc descriptor for the concrete implementation of the test
	ClassDesc* GetClassDesc() const;
};

} // end of namespace PFActions

#endif // _PFTESTSCALE_H_

