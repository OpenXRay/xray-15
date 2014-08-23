/**********************************************************************
 *<
	FILE: PFTestSplitByAmount.h

	DESCRIPTION: SplitByAmount Test header
				 The Test checks particles by different numerical
				 properties

	CREATED BY: Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTSPLITBYAMOUNT_H_
#define _PFTESTSPLITBYAMOUNT_H_

#include "max.h"
#include "iparamb2.h"

#include "PFSimpleTest.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "IPFActionState.h"
#include "PFClassIDs.h"

#include <map>

namespace PFActions {

class PFTestSplitByAmountState:	public IObject, 
								public IPFActionState
{		
public:
	// from IObject interface
	int NumInterfaces() const { return 1; }
	BaseInterface* GetInterfaceAt(int index) const { return ((index == 0) ? (IPFActionState*)this : NULL); }
	BaseInterface* GetInterface(Interface_ID id);
	void DeleteIObject();

	// From IPFActionState
	Class_ID GetClassID();
	ULONG GetActionHandle() const { return actionHandle(); }
	void SetActionHandle(ULONG handle) { _actionHandle() = handle; }
	IOResult Save(ISave* isave) const;
	IOResult Load(ILoad* iload);

protected:
	friend class PFTestSplitByAmount;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator*	randGen()	const	{ return &m_randGen; }
		int						wentThru()	const	{ return m_wentThru; }
		int					wentThruAccum()	const	{ return m_wentThruAccum; }
		int					wentThruTotal()	const	{ return m_wentThruTotal; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGen()			{ return &m_randGen; }
		int&			_wentThru()			{ return m_wentThru; }
		int&			_wentThruAccum()	{ return m_wentThruAccum; }
		int&			_wentThruTotal()	{ return m_wentThruTotal; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;		
		int	m_wentThru, m_wentThruAccum, m_wentThruTotal;
};

class PFTestSplitByAmount:	public PFSimpleTest 
{
public:
	// constructor/destructor
	PFTestSplitByAmount();
	
	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next);
	int RenderBegin(TimeValue t, ULONG flags);
	int RenderEnd(TimeValue t);

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);
	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
		// SplitByAmount Test supports randomness
	bool	SupportRand() const	{ return true; }
	int		GetRand();
	void	SetRand(int seed);

	IObject* GetCurrentState(IObject* pContainer);
	void SetCurrentState(IObject* actionState, IObject* pContainer);

	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);
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
	bool hasParticleContainer(IObject* pCont, int& index);
	bool addParticleContainer(IObject* pCont, INode* systemNode);
	bool removeParticleContainer(IObject* pCont);
	int wentThruAccum(INode* systemNode);
	int wentThruTotal(INode* systemNode);
	int wentThruAccum();
	int wentThruTotal();

	// const access to class members
	bool					renderMode()				const { return m_renderMode; }
	const Tab<IObject*>&	allParticleContainers()		const { if (renderMode()) return m_allParticleContainersRN; return m_allParticleContainersVP; }
	IObject*				allParticleContainer(int i)	const { if (renderMode()) return m_allParticleContainersRN[i]; return m_allParticleContainersVP[i];}
	const Tab<INode*>&		allSystemNodes()			const { if (renderMode()) return m_allSystemNodesRN; return m_allSystemNodesVP;}
	INode*					allSystemNode(int i)		const { if (renderMode()) return m_allSystemNodesRN[i]; return m_allSystemNodesVP[i];}
	const Tab<int>&			wentThruAccums()			const { if (renderMode()) return m_wentThruAccumRN; return m_wentThruAccumVP;}
	int						wentThruAccum(int i)		const { if (renderMode()) return m_wentThruAccumRN[i]; return m_wentThruAccumVP[i];}
	const Tab<int>&			wentThruTotals()			const { if (renderMode()) return m_wentThruTotalRN; return m_wentThruTotalVP;}
	int						wentThruTotal(int i)		const { if (renderMode()) return m_wentThruTotalRN[i]; return m_wentThruTotalVP[i];}
	const Tab<TimeValue>&	lastUpdates()				const { if (renderMode()) return m_lastUpdateRN; return m_lastUpdateVP;}
	TimeValue				lastUpdate(int i)			const { if (renderMode()) return m_lastUpdateRN[i]; return m_lastUpdateVP[i];}

	// access to class members
	bool&			_renderMode()					{ return m_renderMode; }
	Tab<IObject*>&	_allParticleContainers()		{ if (renderMode()) return m_allParticleContainersRN; return m_allParticleContainersVP;}
	IObject*&		_allParticleContainer(int i)	{ if (renderMode()) return m_allParticleContainersRN[i]; return m_allParticleContainersVP[i];}
	Tab<INode*>&	_allSystemNodes()				{ if (renderMode()) return m_allSystemNodesRN; return m_allSystemNodesVP;}
	INode*&			_allSystemNode(int i)			{ if (renderMode()) return m_allSystemNodesRN[i]; return m_allSystemNodesVP[i];}
	Tab<int>&		_wentThruAccums()				{ if (renderMode()) return m_wentThruAccumRN; return m_wentThruAccumVP;}
	int&			_wentThruAccum(int i)			{ if (renderMode()) return m_wentThruAccumRN[i]; return m_wentThruAccumVP[i];}
	Tab<int>&		_wentThruTotals()				{ if (renderMode()) return m_wentThruTotalRN; return m_wentThruTotalVP;}
	int&			_wentThruTotal(int i)			{ if (renderMode()) return m_wentThruTotalRN[i]; return m_wentThruTotalVP[i];}
	Tab<TimeValue>&	_lastUpdates()					{ if (renderMode()) return m_lastUpdateRN; return m_lastUpdateVP;}
	TimeValue&		_lastUpdate(int i)				{ if (renderMode()) return m_lastUpdateRN[i]; return m_lastUpdateVP[i];}

private:
	bool m_renderMode;
	Tab<IObject*> m_allParticleContainersVP;
	Tab<IObject*> m_allParticleContainersRN;
	Tab<INode*> m_allSystemNodesVP;
	Tab<INode*> m_allSystemNodesRN;
	Tab<int> m_wentThruAccumVP; // used to keep track of amount of particles for every Nth
	Tab<int> m_wentThruAccumRN; // used to keep track of amount of particles for every Nth
	Tab<int> m_wentThruTotalVP; // used to keep track of amount of particles for first N
	Tab<int> m_wentThruTotalRN; // used to keep track of amount of particles for first N
	Tab<TimeValue> m_lastUpdateVP;
	Tab<TimeValue> m_lastUpdateRN;
};

} // end of namespace PFActions

#endif // _PFTESTSPLITBYAMOUNT_H_

