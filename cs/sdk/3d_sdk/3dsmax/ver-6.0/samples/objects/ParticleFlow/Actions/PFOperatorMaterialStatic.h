/**********************************************************************
 *<
	FILE:			PFOperatorMaterialStatic.h

	DESCRIPTION:	MaterialStatic Operator header
					Operator to assign mark shape to particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORMATERIALSTATIC_H_
#define _PFOPERATORMATERIALSTATIC_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

// reference IDs
enum {	kMaterialStatic_reference_pblock,
		kMaterialStatic_reference_material,
		kMaterialStatic_reference_num = 2 };

// operator state
class PFOperatorMaterialStaticState:	public IObject, 
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
	friend class PFOperatorMaterialStatic;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGen()	const	{ return &m_randGen; }
		float cycleOffset()				const	{ return m_cycleOffset; }
		TimeValue offsetTime()			const	{ return m_offsetTime; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGen()			{ return &m_randGen; }
		float&			_cycleOffset()		{ return m_cycleOffset; }
		TimeValue&		_offsetTime()		{ return m_offsetTime; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;		
		float m_cycleOffset;		
		TimeValue m_offsetTime;
};

// MaterialStatic Operator
class PFOperatorMaterialStaticDADMgr;

class PFOperatorMaterialStatic:	public PFSimpleOperator 
{
public:
	PFOperatorMaterialStatic();
	~PFOperatorMaterialStatic();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	int NumRefs() { return kMaterialStatic_reference_num; }
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
	const PFOperatorMaterialStaticDADMgr* dadMgr()	const	{ return m_dadMgr; }
	TSTR							lastDynamicName() const	{ return m_lastDynamicName; }
	const Mtl*				material()			const	{ return m_material; }

	// access to class members
	PFOperatorMaterialStaticDADMgr*& _dadMgr()	{ return m_dadMgr; }
	float&						_cycleOffset(IObject* pCont)	{ return m_cycleOffset[pCont]; }
	std::map<IObject*, float>&	_cycleOffset()					{ return m_cycleOffset; }
	TimeValue&					_offsetTime(IObject* pCont)		{ return m_offsetTime[pCont]; }
	std::map<IObject*, TimeValue>& _offsetTime()				{ return m_offsetTime; }
	Tab<int>&					_randMtlIndex(IObject* pCont)	{ return m_randMtlIndex[pCont]; }
	std::map<IObject*, Tab<int> >& _randMtlIndex()				{ return m_randMtlIndex; }
	TSTR&						_lastDynamicName()	{ return m_lastDynamicName; }
	Mtl*&					_material()				{ return m_material; }

protected:
	PFOperatorMaterialStaticDADMgr*	m_dadMgr;	// materialStatic DAD manager
	std::map<IObject*, float> m_cycleOffset; // cycle offset for a current particle
	std::map<IObject*, TimeValue> m_offsetTime; // timing for the cycle offset
	std::map<IObject*, Tab<int> > m_randMtlIndex; 
	Mtl* m_material;			// material
	TSTR m_lastDynamicName;
};


// drag-and-drop manager
class PFOperatorMaterialStaticDADMgr : public DADMgr
{
public:
	PFOperatorMaterialStaticDADMgr(PFOperatorMaterialStatic* op) : m_operator(op) {}

	// called on the draggee to see what if anything can be dragged from this x,y
	SClass_ID GetDragType(HWND hwnd, POINT p) { return MATERIAL_CLASS_ID; }
	// called on potential dropee to see if can drop type at this x,y
	BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
	int SlotOwner() { return OWNER_SCENE; }  
	ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
	void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
	BOOL AutoTooltip() { return TRUE; }

private:
	PFOperatorMaterialStaticDADMgr() : m_operator(NULL) {}

	const PFOperatorMaterialStatic*	op() const	{ return m_operator; }
	PFOperatorMaterialStatic*&		_op()		{ return m_operator; }

	PFOperatorMaterialStatic *m_operator;
};


} // end of namespace PFActions

#endif // _PFOPERATORMATERIALSTATIC_H_