/**********************************************************************
 *<
	FILE: PFTestSpeedGoToTarget.h

	DESCRIPTION: SpeedGoToTarget Test header
				 The test directs particles to reach a goal(target)
				 and once the target has been reached sends particles
				 to the next ActionList

	CREATED BY: Oleg Bayborodin

	HISTORY: created 07-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFTESTSPEEDGOTOTARGET_H_
#define _PFTESTSPEEDGOTOTARGET_H_

#include "max.h"
#include "iparamb2.h"

#include "PFSimpleTest.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "PFClassIDs.h"

#include <map>

namespace PFActions {

class PFOperatorSpeedGoToTargetLocalData
{
public:
	~PFOperatorSpeedGoToTargetLocalData() { FreeAll(); }
	void FreeAll();
	void InitNodeGeometry(int num);
	int NodeGeometryCount() const;
	void SetNodeGeometry(int i, PFNodeGeometry* nodeGeom);
	// get nodeGeometry by index
	PFNodeGeometry* GetNodeGeometry(int i);
	// get random nodeGeometry according to probability weight
	PFNodeGeometry* GetNodeGeometry(TimeValue time, float randValue);
	// get nodeGeometry by INode passed
	PFNodeGeometry* GetNodeGeometry(INode* node);
	// get nodeGeometry with the closest surface
	PFNodeGeometry* GetClosestNodeSurface(TimeValue time, const Point3& toPoint,
									Point3& worldLocation, Point3& localLocation, int& faceIndex);
	// get nodeGeometry with the closest pivot
	PFNodeGeometry* GetClosestNodePivot(TimeValue time, const Point3& toPoint);
	// get nodeGeometry with the least deviation from the given position and speed vector
	PFNodeGeometry* GetLeastDeviatedNode(TimeValue time, const Point3& fromPoint, const Point3& speed);

protected:
	// const access to class members
	PFNodeGeometry* geomData(int i) const { return m_geomData[i]; }
	const Tab<PFNodeGeometry*>& geomData() const { return m_geomData; }

	// access to class members
	PFNodeGeometry*& _geomData(int i) const { return m_geomData[i]; }
	Tab<PFNodeGeometry*>& _geomData() { return m_geomData; }

private:
	Tab<PFNodeGeometry*> m_geomData;
};

class PFTestSpeedGoToTarget:	public PFSimpleTest, 
								public HitByNameDlgCallback 
{
public:
	// constructor/destructor
	PFTestSpeedGoToTarget();
	~PFTestSpeedGoToTarget();
	
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
	CreateMouseCallBack* GetCreateMouseCallBack();
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	void GetWorldBoundBox(TimeValue t, INode * inode, ViewExp* vp, Box3& box );
	void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp, Box3& box );
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	// SpeedGoToTarget Test supports randomness
	bool	SupportRand() const	{ return true; }
	int		GetRand();
	void	SetRand(int seed);

	// From IPFAction interface
	bool	Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool	Release(IObject* pCont);
	const	ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const	Interval ActivityInterval() const { return FOREVER; }
	bool	Has3dIcon() const { return true; }
	int		IsColorCoordinated() const;

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetTruePViewIcon();
	HBITMAP GetFalsePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// From IPFTest interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator, BitArray& testResult, Tab<float>& testTime);

	// From HitByNameDlgCallback
	TCHAR *dialogTitle()	{ return GetString(IDS_SELECTTARGETOBJECTS); }
	TCHAR *buttonText()		{ return GetString(IDS_SELECT); }
	BOOL useFilter()		{ return TRUE; }
	int filter(INode *node);
	BOOL useProc()			{ return TRUE; }
	void proc(INodeTab &nodeTab);

	// supply ClassDesc descriptor for the concrete implementation of the test
	ClassDesc* GetClassDesc() const;

	// from PFTestSpeedGoToTarget
	PolyShape*	GetIconShape(TimeValue time);
	Mesh*		GetIconMesh(TimeValue time);
	void		UpdateIcon(TimeValue time);
	void		InvalidateIcon();
	void		GetIconBoundBox(TimeValue t, Box3& box ); 

protected:
		int getNumTargets();
		// update targetsMaxscript info on basis of targets info
		bool updateFromRealTargets();
		// update targets info on basis of targetsMaxscript info
		bool updateFromMXSTargets();
		// update object names in UI and dynamic names
		bool updateObjectNames(int pblockID);

		// const access to class members
		Object*			editOb()	const	{ return m_editOb; }
		IObjParam*		ip()		const	{ return m_ip; }
		PolyShape*		iconShape()	const	{ return m_iconShape; }
		Mesh*			iconMesh()	const	{ return m_iconMesh; }
		const Interval&	validIcon()	const	{ return m_validIcon; }
		int				numTargets()const	{ return m_numTargets; }
		const Tab<INode*>& objectsToShow()	const	{ return m_objectsToShow; }
		INode* objectToShow(int i)			const	{ return m_objectsToShow[i]; }

		// access to class members
		Object*&		_editOb()			{ return m_editOb; }
		IObjParam*&		_ip()				{ return m_ip; }
		PolyShape*&		_iconShape()		{ return m_iconShape; }
		Mesh*&			_iconMesh()			{ return m_iconMesh; }
		Interval&		_validIcon()		{ return m_validIcon; }
		std::map<IObject*, PFOperatorSpeedGoToTargetLocalData>& _localData()	{ return m_localData; }
		PFOperatorSpeedGoToTargetLocalData& _localData(IObject* pCont) { return m_localData[pCont]; }
		int&			_numTargets()		{ return m_numTargets; }
		Tab<INode*>&		_objectsToShow()	{ return m_objectsToShow; }
		INode*&				_objectToShow(int i){ return m_objectsToShow[i]; }

private:
		static Object *m_editOb;
		static IObjParam *m_ip;
		PolyShape*	m_iconShape;
		Mesh*		m_iconMesh;
		Interval	m_validIcon;
		static GeomObjectValidatorClass validator;
		static bool validatorInitiated;
		std::map<IObject*, PFOperatorSpeedGoToTargetLocalData>	m_localData;
		int m_numTargets;
		Tab<INode*> m_objectsToShow;
};

} // end of namespace PFActions

#endif // _PFTESTSPEEDGOTOTARGET_H_
