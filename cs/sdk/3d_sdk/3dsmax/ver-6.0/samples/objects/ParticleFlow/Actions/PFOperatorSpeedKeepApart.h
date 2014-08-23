/**********************************************************************
 *<
	FILE:			PFOperatorSpeedKeepApart.h

	DESCRIPTION:	SpeedKeepApart Operator header
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSPEEDKEEPAPART_H_
#define _PFOPERATORSPEEDKEEPAPART_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorSpeedKeepApart:	public PFSimpleOperator 
{
public:
	PFOperatorSpeedKeepApart();
	~PFOperatorSpeedKeepApart();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	int NumSubs() { return 2; } // the paramBlocks
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	ParamDimension* GetParamDimension(int i);
	int NumParamBlocks() { return 2; } // the paramBlocks
	IParamBlock2* GetParamBlock(int i);
	IParamBlock2* GetParamBlockByID(short id);

	// From ReferenceMaker
	int NumRefs() { return 2; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	bool SupportScriptWiring() const { return true; }
	bool GetUseScriptWiring() const;
	void SetUseScriptWiring(bool useScriptWiring);

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	int NumPViewParamBlocks() const;
	IParamBlock2* GetPViewParamBlock(int i) const;
	bool HasDynamicName(TSTR& nameSuffix);

	// from PFOperatorSpeedKeepApart
	void SetSelectionTracking();
	void VerifySelectionTracking();
	void ReleaseSelectionTracking();

	void UpdatePViewUI(IParamBlock2* pblock, int updateID);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

	bool	SupportRand() const { return true; }
	int		GetRand();
	void	SetRand(int seed);

protected:
	void collectPContainers(TimeValue time, IObject* pCont, INode* pSystemNode, Tab<IObject*>& selectedCont, bool& useOwn);
	void updateNodeHandles(IMergeManager* pMM);
	void macrorecGroups();
	void macrorecSystems();

	// const access to class members
	bool				activeProgress()			const	{ return m_activeProgress; }
	bool				notifyAListRegistered()		const	{ return m_notifyAListRegistered; }
	bool				notifyPSystemRegistered()	const	{ return m_notifyPSystemRegistered; }
	const Tab<INode*>&	selectedALists()			const	{ return m_selectedALists; }
	INode*				selectedAList(int i)		const	{ return m_selectedALists[i]; }
	const Tab<INode*>&	selectedPSystems()			const	{ return m_selectedPSystems; }
	INode*				selectedPSystem(int i)		const	{ return m_selectedPSystems[i]; }
	IParamBlock2*		scriptPBlock()				const	{ return m_scriptPBlock; }

	// access to class members
	bool&			_activeProgress()			{ return m_activeProgress; }
	bool&			_notifyAListRegistered()	{ return m_notifyAListRegistered; }
	bool&			_notifyPSystemRegistered()	{ return m_notifyPSystemRegistered; }
	Tab<INode*>&	_selectedALists()			{ return m_selectedALists; }
	INode*&			_selectedAList(int i)		{ return m_selectedALists[i]; }
	Tab<INode*>&	_selectedPSystems()			{ return m_selectedPSystems; }
	INode*&			_selectedPSystem(int i)		{ return m_selectedPSystems[i]; }
	IParamBlock2*&	_scriptPBlock()				{ return m_scriptPBlock; }

private:
	bool m_activeProgress;
	bool m_notifyAListRegistered;
	bool m_notifyPSystemRegistered;
	
	Tab<INode*> m_selectedALists;
	Tab<INode*> m_selectedPSystems;

	// script wiring parameters
	IParamBlock2*	m_scriptPBlock;
};

} // end of namespace PFActions

#endif // _PFOPERATORSPEEDKEEPAPART_H_