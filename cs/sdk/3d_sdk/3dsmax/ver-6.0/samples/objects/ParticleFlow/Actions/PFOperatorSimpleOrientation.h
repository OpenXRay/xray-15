/**********************************************************************
 *<
	FILE:			PFOperatorSimpleOrientation.h

	DESCRIPTION:	SimpleOrientation Operator header
					Operator to set initial orientation of particles

	CREATED BY:		David C. Thompson

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSIMPLEORIENTATION_H_
#define _PFOPERATORSIMPLEORIENTATION_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorSimpleOrientation:	public PFSimpleOperator 
{
public:
	PFOperatorSimpleOrientation();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

	bool	SupportRand() const { return true; }
	int		GetRand();
	void	SetRand(int seed);
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEORIENTATION_H_