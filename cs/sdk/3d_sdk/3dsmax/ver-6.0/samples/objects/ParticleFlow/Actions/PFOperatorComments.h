/**********************************************************************
 *<
	FILE:			PFOperatorComments.h

	DESCRIPTION:	Comments Operator header
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 08-27-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORCOMMENTS_H_
#define _PFOPERATORCOMMENTS_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorComments:	public PFSimpleOperator 
{
public:
	PFOperatorComments();

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
	bool IsNonExecutable() const { return true; }

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;
};

} // end of namespace PFActions

#endif // _PFOPERATORCOMMENTS_H_