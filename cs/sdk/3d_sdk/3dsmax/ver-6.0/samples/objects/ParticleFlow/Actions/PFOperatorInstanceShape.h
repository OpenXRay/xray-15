/**********************************************************************
 *<
	FILE:			PFOperatorInstanceShape.h

	DESCRIPTION:	InstanceShape Operator header
					Operator to assign instance shape to particles

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-04-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORINSTANCESHAPE_H_
#define _PFOPERATORINSTANCESHAPE_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"
#include "notify.h"

namespace PFActions {

// reference IDs
enum {	kInstanceShape_reference_pblock,
		kInstanceShape_reference_object,
		kInstanceShape_reference_material,
		kInstanceShape_reference_num = 3 };

class PFOperatorInstanceShapeDlgProc;
class PFOperatorInstanceShapePickModeCallback;

class PFOperatorInstanceShapeState:	public IObject, 
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
	friend class PFOperatorInstanceShape;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGenVar()	const	{ return &m_randGenVar; }
		const RandGenerator* randGenOrd()	const	{ return &m_randGenOrd; }
		const RandGenerator* randGenSyn()	const	{ return &m_randGenSyn; }
		int shapeIndex() const { return m_shapeIndex; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGenVar()		{ return &m_randGenVar; }
		RandGenerator*	_randGenOrd()		{ return &m_randGenOrd; }
		RandGenerator*	_randGenSyn()		{ return &m_randGenSyn; }
		int&			_shapeIndex()		{ return m_shapeIndex; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGenVar;		
		RandGenerator m_randGenOrd;		
		RandGenerator m_randGenSyn;		
		int m_shapeIndex;
};

class PFOperatorInstanceShape:	public PFSimpleOperator 
{
	friend PFOperatorInstanceShapeDlgProc;
	friend PFOperatorInstanceShapePickModeCallback;

public:
	PFOperatorInstanceShape();
	~PFOperatorInstanceShape();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	int NumRefs() { return kInstanceShape_reference_num; }
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

	bool	IsMaterialHolder() const { return true; }
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

protected:
friend void CatchMaterialChange(void *param, NotifyInfo *info); 

	static TriObject* GetTriObjectFromNode(INode *iNode, const TimeValue t, bool &deleteIt);
	static BOOL IsGroupObject(INode *iNode);
	static BOOL IsHierarchyObject(INode *iNode);
	static BOOL IsMultiElementsObject(INode *iNode);
	
	// Build meshes
	void BuildMeshes(const TimeValue t, bool setMaterial = false);
	int BuildMeshesFromNode(const TimeValue t, INode *iNode);
	int BuildMeshesFromNode(const TimeValue t, INode *iNode, Tab<INode*>& childNodes);
	void CollectMergingNodes(INode *iNode, Tab<INode*>& childNodes);
	Mtl *AddMaterialFromNode(Mtl *multi, INode *iNode);
	void OrderMeshIndices();
	static void RemoveMapData(Mesh &mesh);
	static void AddMaterialOffset(Mesh &mesh, int matOffset);

	// update list of the reference objects
	void resetObjectList();
	
	// Mesh sorting
	typedef std::pair<float, int> FloatIntPair;
	static int CompareFloatIntPair(const void *elem1, const void *elem2);

	// Scale factor
	static float GetScaleFactor(RandGenerator* randGen, float variation);

	// UI refresh
	void RefreshUI(WPARAM message, IParamMap2* map=NULL) const;

	// update status of mtl change notification registration
	void updateRegMtlChangeNotification(bool unregister = false);
	void notifyMtlChanged(NotifyInfo* info);

	bool verifyRefObjectMXSSync();
	bool updateFromRealRefObject();
	bool updateFromMXSRefObject();

	// const access to class members
	const RandObjLinker&	randLinkerVar()	const	{ return m_randLinkerVar; }
	const RandObjLinker&	randLinkerOrd()	const	{ return m_randLinkerOrd; }
	const RandObjLinker&	randLinkerSyn()	const	{ return m_randLinkerSyn; }
	const INode*			object()			const	{ return m_object; }
	const Mtl*				material()			const	{ return m_material; }
	const Mesh*				meshData(int index)	const	{ return m_meshData[index]; }
	const Tab<Mesh*>&		meshData()			const	{ return m_meshData; }
	const Matrix3&			meshTM(int index)	const	{ return m_meshTM[index]; }
	const Tab<Matrix3>&		meshTM()			const	{ return m_meshTM; }
	int						sortIndex(int index) const	{ return m_sortIndex[index]; }
	const Tab<int>&			sortIndex()			const	{ return m_sortIndex; }
	bool					meshNeedsUpdate()	const	{ return m_meshNeedsUpdate; }
	bool					registeredMtlChangeNotification() const { return m_registeredMtlChangeNotification; }
	bool					resetingObjectList()const	{ return m_resetingObjectList; }
	
	// access to class members
	RandObjLinker&		_randLinkerVar()		{ return m_randLinkerVar; }
	RandObjLinker&		_randLinkerOrd()		{ return m_randLinkerOrd; }
	RandObjLinker&		_randLinkerSyn()		{ return m_randLinkerSyn; }
	INode*&					_object()				{ return m_object; }
	Mtl*&					_material()				{ return m_material; }
	Mesh*&					_meshData(int index)	{ return m_meshData[index]; }
	Tab<Mesh*>&				_meshData()				{ return m_meshData; }
	Matrix3&				_meshTM(int index)		{ return m_meshTM[index]; }
	Tab<Matrix3>&			_meshTM()				{ return m_meshTM; }
	int&					_sortIndex(int index)	{ return m_sortIndex[index]; }
	Tab<int>&				_sortIndex()			{ return m_sortIndex; }
	int&					_shapeIndex(IObject* pCont) { return m_shapeIndex[pCont]; }
	std::map<IObject*, int>& _shapeIndex()			{ return m_shapeIndex; }
	int&					_matOffset(INode* node) { return m_matOffset[node]; }
	std::map<INode*, int>&	_matOffset()			{ return m_matOffset; }
	bool&					_meshNeedsUpdate()		{ return m_meshNeedsUpdate; }
	bool&					_registeredMtlChangeNotification() { return m_registeredMtlChangeNotification; }
	bool&					_resetingObjectList()	{ return m_resetingObjectList; }

private:
	static const float	kMaxVariation;
	RandObjLinker	m_randLinkerVar;	// for scale factor variation
	RandObjLinker	m_randLinkerOrd;	// for multi-shape random order
	RandObjLinker	m_randLinkerSyn;	// for sync random offset
	INode*				m_object;			// particle's geometry object
	Mtl*				m_material;			// material
	Tab<Mesh*>			m_meshData;			// mesh data of the ith mesh
	Tab<Matrix3>		m_meshTM;			// ObjectTM of the ith mesh
	Tab<int>			m_sortIndex;		// sorted indices of the mesh data
	std::map<IObject*, int> m_shapeIndex;   // index to meshData for the current particle
	std::map<INode*, int> m_matOffset;		// material offset of an INode
	bool m_meshNeedsUpdate;
	bool m_registeredMtlChangeNotification;
	bool m_resetingObjectList;
};

// pick node call back
class PFOperatorInstanceShapePickNodeCallback : public PickNodeCallback
{
public:
	BOOL Filter(INode *iNode);

	void Init(PFOperatorInstanceShape* op) { _op() = op; }

private:
	const PFOperatorInstanceShape* op() const	{ return m_operator; }
	PFOperatorInstanceShape*&		_op()		{ return m_operator; }

	PFOperatorInstanceShape *m_operator;
};

// pick mode call back
class PFOperatorInstanceShapePickModeCallback : public PickModeCallback
{
public:
	PFOperatorInstanceShapePickModeCallback() : m_operator(NULL) {}

	// From PickModeCallback
	BOOL HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags);
	BOOL Pick(IObjParam *ip, ViewExp *vpt);
	// BOOL PickAnimatable(Animatable* anim) { return TRUE; }
	BOOL RightClick(IObjParam *ip, ViewExp *vpt) { return TRUE; }

	void EnterMode(IObjParam *ip);
	void ExitMode(IObjParam *ip);

	PickNodeCallback *GetFilter();

	void Init(PFOperatorInstanceShape* op) { _op() = op; }

private:
	const PFOperatorInstanceShape* op() const	{ return m_operator; }
	PFOperatorInstanceShape*&		_op()		{ return m_operator; }

	PFOperatorInstanceShape *m_operator;
};


} // end of namespace PFActions

#endif // _PFOPERATORINSTANCESHAPE_H_