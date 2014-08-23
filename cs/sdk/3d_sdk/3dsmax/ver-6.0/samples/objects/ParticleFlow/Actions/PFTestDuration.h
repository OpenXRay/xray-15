/**********************************************************************
 *<
	FILE: PFTestDuration.h

	DESCRIPTION: Duration Test header
				 The Test is used to check particles for
				 a) absolute time; b) particle age; c) time interval for
				 a particle to be present in the current event

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-30-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTDURATION_H_
#define _PFTESTDURATION_H_

#include "max.h"
#include "iparamb2.h"

#include "PFSimpleTest.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "PFClassIDs.h"

namespace PFActions {

class PFTestDuration:	public PFSimpleTest 
{
public:
	// constructor/destructor
	PFTestDuration();
	
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

		// Duration Test supports randomness
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

protected:
		// const access to class members
		Object*			editOb()	const	{ return m_editOb; }
		IObjParam*		ip()		const	{ return m_ip; }

		// access to class members
		Object*&		_editOb()			{ return m_editOb; }
		IObjParam*&		_ip()				{ return m_ip; }

private:
		static Object *m_editOb;
		static IObjParam *m_ip;
};

} // end of namespace PFActions

#endif // _PFTESTDURATION_H_

