/**********************************************************************
 *<
	FILE: PFOperatorSimpleBirth.h

	DESCRIPTION: SimpleBirth Operator header
				 Simple Operator to generate particles

	CREATED BY: Chung-An Lin

	HISTORY: created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSIMPLEBIRTH_H_
#define _PFOPERATORSIMPLEBIRTH_H_

#include <map>

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "IPFActionState.h"

namespace PFActions {

class PFOperatorSimpleBirthState:	public IObject, 
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
	friend class PFOperatorSimpleBirth;

	// const access to class members
	ULONG	actionHandle()		const	{ return m_actionHandle; }		
	float	accumLevel()		const	{ return m_accumLevel; }
	int		particlesGenerated()const	{ return m_particlesGenerated; }

	// access to class members
	ULONG&	_actionHandle()			{ return m_actionHandle; }
	float&	_accumLevel()			{ return m_accumLevel; }
	int&	_particlesGenerated()	{ return m_particlesGenerated; }

private:
	ULONG m_actionHandle;
	float m_accumLevel;		
	int m_particlesGenerated;
};

class PFOperatorSimpleBirth:	public PFSimpleOperator 
{
public:
	PFOperatorSimpleBirth();

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

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const;
	bool IsFertile() const { return true; }
	bool IsEmitterPropDependent() const { return true; }
	IObject* GetCurrentState(IObject* pContainer);
	void SetCurrentState(IObject* actionState, IObject* pContainer);

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

private:
	int EmitTotalAmount() const;
	void RefreshBirthTypeUI() const;
	void RefreshTotalAmountUI() const;

	void invalidateTotalAmount() { _emitTotalAmount() = -1; }

	// const access to class members
	int emitTotalAmount() const { return m_emitTotalAmount; }

	// access to class members
	int& _emitTotalAmount() { return m_emitTotalAmount; }
	std::map<IObject*, float>& _accumLevel()	{ return m_accumLevel; }
	float& _accumLevel(IObject* pCont) { return m_accumLevel[pCont]; }
	std::map<IObject*, int>& _particlesGenerated()	{ return m_particlesGenerated; }
	int& _particlesGenerated(IObject* pCont) { return m_particlesGenerated[pCont]; }

	mutable int m_emitTotalAmount;
	std::map<IObject*, float>	m_accumLevel; // indicate accumulated rate growth for a particle generation
	std::map<IObject*, int> m_particlesGenerated; // amount of particles currently generated
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEBIRTH_H_