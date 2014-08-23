/**********************************************************************
 *<
	FILE:			PFOperatorSimpleScale.h

	DESCRIPTION:	SimpleScale Operator header
					Operator to effect speed unto particles

	CREATED BY:		David C. Thompson

	HISTORY:		created 07-22-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSIMPLESCALE_H_
#define _PFOPERATORSIMPLESCALE_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorSimpleScale:	public PFSimpleOperator,
								public TimeChangeCallback
{
public:
	PFOperatorSimpleScale();

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

	// from TimeChangeCallback
	void TimeChanged(TimeValue t); // to update current scale values

	// From IPFAction interface
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);

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

	bool bAbsPresent, bRFPresent, bRSPresent;

protected:
	bool DoScaleConstrain();
	void XScaleChanged();
	void YScaleChanged();
	void ZScaleChanged();
	void DuplicateScale(TimeValue time);
	bool DoScaleVarConstrain();
	void XScaleVarChanged();
	void YScaleVarChanged();
	void ZScaleVarChanged();
	void DuplicateScaleVar(TimeValue time);

	// const access to class members
	bool calculatingConstrain() const { return m_calculatingConstrain; }
	Point3 scaleDup() const { return m_scaleDup; }
	Point3 scaleVarDup() const { return m_scaleVarDup; }
	bool registeredTimeChangeCallback() { return m_registeredTimeChangeCallback; }

	// access to class members
	bool& _calculatingConstrain() { return m_calculatingConstrain; }
	Point3& _scaleDup() { return m_scaleDup; }
	Point3& _scaleVarDup() { return m_scaleVarDup; }
	bool& _registeredTimeChangeCallback() { return m_registeredTimeChangeCallback; }

private:
	bool m_calculatingConstrain;
	Point3 m_scaleDup, m_scaleVarDup;
	// if registered TimeChange callback
	bool m_registeredTimeChangeCallback;
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESCALE_H_