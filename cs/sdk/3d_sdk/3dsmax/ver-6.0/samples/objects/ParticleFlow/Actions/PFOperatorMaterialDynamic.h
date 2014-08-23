/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamic.h

	DESCRIPTION:	MaterialDynamic Operator header
					Operator to assign mark shape to particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORMATERIALDYNAMIC_H_
#define _PFOPERATORMATERIALDYNAMIC_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

// reference IDs
enum {	kMaterialDynamic_reference_pblock,
		kMaterialDynamic_reference_material,
		kMaterialDynamic_reference_num = 2 };

// operator state
class PFOperatorMaterialDynamicState:	public IObject, 
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
	friend class PFOperatorMaterialDynamic;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGen()	const	{ return &m_randGen; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGen()			{ return &m_randGen; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;		
};

// MaterialDynamic Operator
class PFOperatorMaterialDynamicDADMgr;

class PFOperatorMaterialDynamic:	public PFSimpleOperator 
{
public:
	PFOperatorMaterialDynamic();
	~PFOperatorMaterialDynamic();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	int NumRefs() { return kMaterialDynamic_reference_num; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
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
	const Interval ActivityInterval() const { return FOREVER; }

	bool	SupportRand() const	{ return true; }
	int		GetRand();
	void	SetRand(int seed);

	IObject* GetCurrentState(IObject* pContainer);
	void SetCurrentState(IObject* actionState, IObject* pContainer);

	bool	IsMaterialHolder() const;
	Mtl*	GetMaterial();
	bool	SetMaterial(Mtl* mtl);

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);
	bool HasPostProceed(IObject* pCont, PreciseTimeValue time);
	void PostProceedBegin(IObject* pCont, PreciseTimeValue time) { _postProceed() = true; }
	void PostProceedEnd(IObject* pCont, PreciseTimeValue time) { _postProceed() = false; }

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

private:
	bool verifyMtlsMXSSync();
	bool updateFromRealMtl();
	bool updateFromMXSMtl();

	// const access to class members
	const PFOperatorMaterialDynamicDADMgr* dadMgr()	const	{ return m_dadMgr; }
	bool							postProceed() const { return m_postProceed; }
	TSTR							lastDynamicName() const	{ return m_lastDynamicName; }
	const Mtl*						material()	const		{ return m_material; }

	// access to class members
	PFOperatorMaterialDynamicDADMgr*& _dadMgr()	{ return m_dadMgr; }
	bool&						_postProceed()	{ return m_postProceed; }
	TSTR&						_lastDynamicName()	{ return m_lastDynamicName; }
	Mtl*&						_material()		{ return m_material; }

protected:
	PFOperatorMaterialDynamicDADMgr*	m_dadMgr;	// materialDynamic DAD manager
	bool						m_postProceed;
	Mtl*						m_material;		// material
	TSTR m_lastDynamicName;
};


// drag-and-drop manager
class PFOperatorMaterialDynamicDADMgr : public DADMgr
{
public:
	PFOperatorMaterialDynamicDADMgr(PFOperatorMaterialDynamic* op) : m_operator(op) {}

	// called on the draggee to see what if anything can be dragged from this x,y
	SClass_ID GetDragType(HWND hwnd, POINT p) { return MATERIAL_CLASS_ID; }
	// called on potential dropee to see if can drop type at this x,y
	BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
	int SlotOwner() { return OWNER_SCENE; }  
	ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
	void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
	BOOL AutoTooltip() { return TRUE; }

private:
	PFOperatorMaterialDynamicDADMgr() : m_operator(NULL) {}

	const PFOperatorMaterialDynamic*	op() const	{ return m_operator; }
	PFOperatorMaterialDynamic*&		_op()		{ return m_operator; }

	PFOperatorMaterialDynamic *m_operator;
};


} // end of namespace PFActions

#endif // _PFOPERATORMATERIALDYNAMIC_H_