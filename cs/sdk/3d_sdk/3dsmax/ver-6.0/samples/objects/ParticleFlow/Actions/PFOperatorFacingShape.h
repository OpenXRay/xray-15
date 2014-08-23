/**********************************************************************
 *<
	FILE:			PFOperatorFacingShape.h

	DESCRIPTION:	FacingShape Operator header
					Operator to assign facing shape to particles

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORFACINGSHAPE_H_
#define _PFOPERATORFACINGSHAPE_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

// reference IDs
enum {	kFacingShape_reference_pblock,
		kFacingShape_reference_object,
		kFacingShape_reference_material,
		kFacingShape_reference_num = 3 };

class PFOperatorFacingShapeDlgProc;
class PFOperatorFacingShapePickModeCallback;

class PFOperatorFacingShapeState:	public IObject, 
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
	friend class PFOperatorFacingShape;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGenVar()	const	{ return &m_randGenVar; }
		const RandGenerator* randGenRot()	const	{ return &m_randGenRot; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGenVar()		{ return &m_randGenVar; }
		RandGenerator*	_randGenRot()		{ return &m_randGenRot; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGenVar;		
		RandGenerator m_randGenRot;		
};

class PFOperatorFacingShape:	public PFSimpleOperator 
{
	friend PFOperatorFacingShapeDlgProc;
	friend PFOperatorFacingShapePickModeCallback;

public:
	PFOperatorFacingShape();
	// ~PFOperatorFacingShape();		// no need at the moment

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	int NumRefs() { return kFacingShape_reference_num; }
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

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

protected:
	static bool IsNodeAnimated(INode *iNode);
	static bool IsCameraObject(INode* iNode);
	static float InheritedWidth(const Mesh* orgMesh, const Point3& orgScale);
	static float ProportionWidth(INode* camNode, TimeValue t, float proportion, const Point3 &pos);
	static void CacheNodePosition(INode *iNode, TimeValue timeValueStart, TimeValue timeValueEnd, Tab<Point3> &cachedPos);
	static void InterpolatePosition(PreciseTimeValue time, TimeValue timeValueStart, Tab<Point3> &cachedPos, Point3 &pos);

	// Build Mesh
	void CreateMesh();
	void BuildMesh(const TimeValue t);

	// Scale factor
	static float GetScaleFactor(RandGenerator* randGen, float variation);

	// UI refresh
	void RefreshUI(WPARAM message, IParamMap2* map=NULL) const;

	bool verifyRefObjectMXSSync();
	bool updateFromRealRefObject();
	bool updateFromMXSRefObject();

	// const access to class members
	const RandObjLinker&	randLinkerVar()	const	{ return m_randLinkerVar; }
	const RandObjLinker&	randLinkerRot()	const	{ return m_randLinkerRot; }
	const INode*				object()		const	{ return m_object; }
	const Mtl*					material()		const	{ return m_material; }
	const Mesh&					mesh()			const	{ return m_mesh; }

	// access to class members
	RandObjLinker&	_randLinkerVar()	{ return m_randLinkerVar; }
	RandObjLinker&	_randLinkerRot()	{ return m_randLinkerRot; }
	INode*&			_object()			{ return m_object; }
	Mtl*&			_material()			{ return m_material; }
	Mesh&			_mesh()				{ return m_mesh; }

private:
	RandObjLinker	m_randLinkerVar;	// for size variation
	RandObjLinker	m_randLinkerRot;	// for random rotation offset
	INode*				m_object;			// look at object
	Mtl*				m_material;			// material
	Mesh				m_mesh;				// rectangle mesh
};

// pick node call back
class PFOperatorFacingShapePickNodeCallback : public PickNodeCallback
{
public:
	BOOL Filter(INode *iNode);
};

// pick mode call back
class PFOperatorFacingShapePickModeCallback : public PickModeCallback
{
public:
	PFOperatorFacingShapePickModeCallback() : m_operator(NULL) {}

	// From PickModeCallback
	BOOL HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags);
	BOOL Pick(IObjParam *ip, ViewExp *vpt);
	// BOOL PickAnimatable(Animatable* anim) { return TRUE; }
	BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }

	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);

	PickNodeCallback *GetFilter();

	void Init(PFOperatorFacingShape* op) { _op() = op; }

private:
	const PFOperatorFacingShape*	op() const	{ return m_operator; }
	PFOperatorFacingShape*&		_op()		{ return m_operator; }

	PFOperatorFacingShape *m_operator;
};


} // end of namespace PFActions

#endif // _PFOPERATORFACINGSHAPE_H_