/**********************************************************************
 *<
	FILE: PFTestCollisionSpaceWarp.h

	DESCRIPTION: CollisionSpaceWarp Test header

	CREATED BY: Peter Watje

	HISTORY: created 2-07-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTCOLLISIONSPACEWARP_H_
#define _PFTESTCOLLISIONSPACEWARP_H_

#include <map>

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

namespace PFActions {

class PFTestCollisionSpaceWarp:	public HelperObject, 
								public IPFAction,
								public IPFTest,
								public IPViewItem,
								public IPFIntegrator,
								public HitByNameDlgCallback 
{
public:
	// constructor/destructor
	PFTestCollisionSpaceWarp();
	~PFTestCollisionSpaceWarp();
	
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

		// CollisionSpaceWarp Test supports randomness
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

	// From IPFIntegrator interface
	bool Proceed(IObject* pCont, PreciseTimeValue time, int index);
	bool Proceed(IObject* pCont, PreciseTimeValue time);
	bool Proceed(IObject* pCont, Tab<PreciseTimeValue>& times);
	bool Proceed(IObject* pCont, PreciseTimeValue time, BitArray& selected);
	bool Proceed(IObject* pCont, Tab<PreciseTimeValue>& times, BitArray& selected);

	// From HitByNameDlgCallback
	TCHAR *dialogTitle()	{ return GetString(IDS_SELECTDEFLECTORS); }
	TCHAR *buttonText()		{ return GetString(IDS_SELECT); }
	BOOL useFilter()		{ return TRUE; }
	int filter(INode *node);
	BOOL useProc()			{ return TRUE; }
	void proc(INodeTab &nodeTab);

	// supply ClassDesc descriptor for the concrete implementation of the test
	ClassDesc* GetClassDesc() const;

public:
		void RefreshUI(WPARAM message, IParamMap2* map=NULL) const;
		void UpdatePViewUI(int updateID) const;
		// returns actual number of deflectors
		int getNumDeflectors();
		// update deflectorsMaxscript info on basis of deflectors info
		bool updateFromRealDeflectors();
		// update deflectors info on basis of deflectorsMaxscript info
		bool updateFromMXSDeflectors();
		// update object names in UI and dynamic names
		bool updateObjectNames(int pblockID);

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

		int numDeflectors()		const	{ return m_numDeflectors; }
		IObject* prevContainer()const	{ return m_prevContainer; }
		const Tab<INode*>& objectsToShow()	const	{ return m_objectsToShow; }
		INode* objectToShow(int i)			const	{ return m_objectsToShow[i]; }

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

		IPFIntegrator*&	_givenIntegrator(IObject* pCont) { return m_givenIntegrator[pCont]; }

		HBITMAP&	_activeIcon()		{ return m_activeIcon; }
		HBITMAP&	_trueIcon()			{ return m_trueIcon; }
		HBITMAP&	_falseIcon()		{ return m_falseIcon; }

		int&		_numDeflectors()	{ return m_numDeflectors; }
		IObject*&	_prevContainer()	{ return m_prevContainer; }
		Tab<INode*>&		_objectsToShow()	{ return m_objectsToShow; }
		INode*&				_objectToShow(int i){ return m_objectsToShow[i]; }

protected:
		static CollisionSpaceWarpValidatorClass validator;
		static bool validatorInitiated;

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

		// the test stores ptr to integrator given in the Proceed method
		// the integrator is used to calculate angular components for integration
		// position/speed component is overwritten by the test
		std::map<IObject*, IPFIntegrator*>	m_givenIntegrator;

		// for custom icons in ParticleView
		HBITMAP m_activeIcon;
		HBITMAP m_trueIcon;
		HBITMAP m_falseIcon;

		int m_numDeflectors;
		Tab<INode*> m_objectsToShow;

		IObject* m_prevContainer;
};

} // end of namespace PFActions

#endif // _PFTESTCOLLISIONSPACEWARP_H_

