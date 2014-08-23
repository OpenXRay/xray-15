/**********************************************************************
 *<
	FILE:			PFOperatorSimpleMapping.h

	DESCRIPTION:	SimpleMapping Operator header
					Operator to assign mark shape to particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSIMPLEMAPPING_H_
#define _PFOPERATORSIMPLEMAPPING_H_

#include <map>

#include "IPFOperator.h"
#include "PFSimpleOperator.h"

namespace PFActions {

// reference IDs
enum {	kSimpleMapping_reference_pblock,
		kSimpleMapping_reference_num = 1 };

class PFOperatorSimpleMapping:	public PFSimpleOperator 
{
public:
	PFOperatorSimpleMapping();
	~PFOperatorSimpleMapping();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);

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
};


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEMAPPING_H_