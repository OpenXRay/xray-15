/**********************************************************************
 *<
	FILE: PFTestSpawn.h

	DESCRIPTION: Duration Test header
				 The tests generates new particles from the existing
				 ones and then sends new particles (and may also the
				 old ones) to the next action list

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-30-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTSPAWN_H_
#define _PFTESTSPAWN_H_

#include "max.h"
#include "iparamb2.h"

#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "OneClickCreateCallBack.h"
#include "PFClassIDs.h"
#include "RandObjLinker.h"
#include "IParticleChannelPTV.h"
#include "IParticleChannelFloat.h"
#include "IParticleChannelPoint3.h"

namespace PFActions {

class PFTestSpawn:	public HelperObject, 
					public IPFAction,
					public IPFTest,
					public IPViewItem
{
public:
	// constructor/destructor
	PFTestSpawn();
	~PFTestSpawn();
	
	// From InterfaceServer
	BaseInterface* GetInterface(Interface_ID id);

	// From Animatable
	void DeleteThis();
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next);
	int NumSubs() { return 1; } // the paramBlock
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	ParamDimension* GetParamDimension(int i);
	BOOL IsAnimated() { return _pblock()->IsAnimated(); }
	void FreeCaches() { ; }
	int NumParamBlocks() { return 1; } // the paramBlock
	IParamBlock2* GetParamBlock(int i);
	IParamBlock2* GetParamBlockByID(short id);

	// From ReferenceMaker
	int NumRefs() { return 1; } // the paramBlock
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);
	int RemapRefOnLoad(int iref);
	RefTargetHandle Clone(RemapDir &remap);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// From BaseObject
	TCHAR *GetObjectName();
	CreateMouseCallBack* GetCreateMouseCallBack() { return OneClickCreateCallBack::Instance(); }
	BOOL OKToChangeTopology(TSTR &modName) { return FALSE; }

	// From Object
	void InitNodeName(TSTR& s) { s = GetObjectName(); }
	ObjectState Eval(TimeValue t) { return ObjectState(this); }

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);

		// Duration Test supports randomness
	bool	SupportRand() const	{ return true; }
	int		GetRand();
	void	SetRand(int seed);

	IObject* GetCurrentState(IObject* pContainer);
	void SetCurrentState(IObject* actionState, IObject* pContainer);

	const	ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	bool	IsInstant() const { return false; }
	const	Interval ActivityInterval() const { return FOREVER; }

	// From IPFTest interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, BitArray& testResult, Tab<float>& testTime);
	void ProceedStep1(IObject* pCont, Object* pSystem, INode* pNode, INode* actionNode, FPInterface* integrator);
	bool ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime);
	
	// From IPViewItem interface
	int NumPViewParamBlocks() const { return 1; }
	IParamBlock2* GetPViewParamBlock(int i) const { if (i==0) return pblock(); else return NULL; }
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetTruePViewIcon();
	HBITMAP GetFalsePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the test
	ClassDesc* GetClassDesc() const;

public:
		float getSpawnRate(TimeValue time);
		float getSpawnRate(PreciseTimeValue time);
		float getStepSize(TimeValue time);
		float getStepSize(PreciseTimeValue time);
		PreciseTimeValue calculateNextSpawnTimeForFrames(int index, // particle index
														int syncType,
														PreciseTimeValue timeEnd,
														int paramAnim,
														float frameRate, // frame rate if constant
														IParticleChannelPTVR* chLastTimeR,
														IParticleChannelPTVW* chLastTimeW,
														IParticleChannelFloatR* chAccumDefR,
														IParticleChannelFloatW* chAccumDefW,
														IParticleChannelPTVR* chBirthTime,
														IParticleChannelPTVR* chEventStart );
		PreciseTimeValue calculateNextSpawnTimeForDistance(int index, // particle index
														int syncType,
														PreciseTimeValue timeEnd,
														int paramAnim,
														float stepSize, // step size if constant
														IParticleChannelPoint3R* chSpeed,
														IParticleChannelPTVR* chLastTimeR,
														IParticleChannelPTVW* chLastTimeW,
														IParticleChannelFloatR* chAccumDefR,
														IParticleChannelFloatW* chAccumDefW,
														IParticleChannelPTVR* chBirthTime,
														IParticleChannelPTVR* chEventStart );

		void RefreshUI(WPARAM message, IParamMap2* map=NULL) const;
		void UpdatePViewUI(int updateID) const;

		// const access to class members
		Object*			editOb()	const	{ return m_editOb; }
		IObjParam*		ip()		const	{ return m_ip; }
		IParamBlock2*	pblock()	const	{ return m_pblock; }

		const IObject*		containerFnPub()		const	{ return m_containerFnPub; }
		const Object*		particleSystemFnPub()	const	{ return m_particleSystemFnPub; }
		const INode*		particleNodeFnPub()		const	{ return m_particleNodeFnPub; }
		const INode*		actionNodeFnPub()		const	{ return m_actionNodeFnPub; }
		const FPInterface*	integratorFnPub()		const	{ return m_integratorFnPub; }

		const RandObjLinker& randLinker()	const	{ return m_randLinker; }

		HBITMAP	activeIcon()	const	{ return m_activeIcon; }
		HBITMAP	trueIcon()		const	{ return m_trueIcon; }
		HBITMAP	falseIcon()		const	{ return m_falseIcon; }

		// access to class members
		Object*&		_editOb()			{ return m_editOb; }
		IObjParam*&		_ip()				{ return m_ip; }
		IParamBlock2*&	_pblock()			{ return m_pblock; }

		IObject*&		_containerFnPub()		{ return m_containerFnPub; }
		Object*&		_particleSystemFnPub()	{ return m_particleSystemFnPub; }
		INode*&			_particleNodeFnPub()	{ return m_particleNodeFnPub; }
		INode*&			_actionNodeFnPub()		{ return m_actionNodeFnPub; }
		FPInterface*&	_integratorFnPub()		{ return m_integratorFnPub; }

		RandObjLinker&	_randLinker()	{ return m_randLinker; }

		HBITMAP&	_activeIcon()		{ return m_activeIcon; }
		HBITMAP&	_trueIcon()			{ return m_trueIcon; }
		HBITMAP&	_falseIcon()		{ return m_falseIcon; }

private:
		static Object *m_editOb;
		static IObjParam *m_ip;

		IParamBlock2*	m_pblock;

		// to support FnPub two-step Proceed
		IObject* m_containerFnPub;
		Object* m_particleSystemFnPub;
		INode* m_particleNodeFnPub;
		INode* m_actionNodeFnPub;
		FPInterface* m_integratorFnPub;

		// to keep track of client particle systems
		// the test may serve several particle systems at once
		// each particle system has its own randomization scheme
		RandObjLinker m_randLinker;

		// for custom icons in ParticleView
		HBITMAP m_activeIcon;
		HBITMAP m_trueIcon;
		HBITMAP m_falseIcon;
};

struct SpawnParticleData {
	PreciseTimeValue time;
	PreciseTimeValue birthTime;
	Point3 position;
	Point3 speed;
	Quat orientation;
	AngAxis spin;
	Point3 scale;
};

typedef Tab<SpawnParticleData> TabSpawnParticleData;



} // end of namespace PFActions

#endif // _PFTESTSPAWN_H_

