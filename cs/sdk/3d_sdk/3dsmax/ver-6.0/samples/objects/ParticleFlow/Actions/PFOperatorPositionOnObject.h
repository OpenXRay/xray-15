/**********************************************************************
 *<
	FILE:			PFOperatorPositionOnObject.h

	DESCRIPTION:	PositionOnObject Operator header
					Simple Operator to place particles on PF System icon

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORPOSITIONONOBJECT_H_
#define _PFOPERATORPOSITIONONOBJECT_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorPositionOnObjectLocalData
{
public:
	~PFOperatorPositionOnObjectLocalData() { FreeAll(); }
	void FreeAll();
	void InitDistinctPoints(int num);
	const LocationPoint& GetDistinctPoint(int i);
	void SetDistinctPoint(int i, const LocationPoint& p);
	void InitNodeGeometry(int num);
//	int NodeGeometryCount() const;
	// get random nodeGeometry according to probability weight
	PFNodeGeometry* GetNodeGeometry(TimeValue time, float randValue);
	// get nodeGeometry by INode passed
	PFNodeGeometry* GetNodeGeometry(INode* node);
	void SetNodeGeometry(int i, PFNodeGeometry* nodeGeom);

public:
	// const access to class members
	const LocationPoint& point(int i) const { return m_points[i]; }
	const Tab<LocationPoint>& points() const { return m_points; }

protected:
	PFNodeGeometry* geomData(int i) const { return m_geomData[i]; }
	const Tab<PFNodeGeometry*>& geomData() const { return m_geomData; }

	// access to class members
	LocationPoint& _point(int i) { return m_points[i]; }
	Tab<LocationPoint>& _points() { return m_points; }
	PFNodeGeometry*& _geomData(int i) const { return m_geomData[i]; }
	Tab<PFNodeGeometry*>& _geomData() { return m_geomData; }

private:
	Tab<LocationPoint> m_points;
	Tab<PFNodeGeometry*> m_geomData;
};

class PFOperatorPositionOnObjectState:	public IObject, 
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
	friend class PFOperatorPositionOnObject;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGen()		const	{ return &m_randGen; }
		const Tab<LocationPoint>& points()	const	{ return m_points; }

		// access to class members
		ULONG&				_actionHandle()		{ return m_actionHandle; }
		RandGenerator*		_randGen()			{ return &m_randGen; }
		Tab<LocationPoint>&	_points()			{ return m_points; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;		
		Tab<LocationPoint> m_points;		
};

class PFOperatorPositionOnObject:	public PFSimpleOperator,
									public HitByNameDlgCallback 
{
public:
	PFOperatorPositionOnObject();

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

	// From HitByNameDlgCallback
	TCHAR *dialogTitle()	{ return GetString(IDS_SELECTEMITTEROBJECTS); }
	TCHAR *buttonText()		{ return GetString(IDS_SELECT); }
	BOOL useFilter()		{ return TRUE; }
	int filter(INode *node);
	BOOL useProc()			{ return TRUE; }
	void proc(INodeTab &nodeTab);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

protected:
	//returns the actual number of emitters
	int getNumEmitters();
	// update emittersMaxscript info on basis of emitters info
	bool updateFromRealEmitters();
	// update emitters info on basis of emittersMaxscript info
	bool updateFromMXSEmitters();
	// update object names in UI and dynamic names
	bool updateObjectNames(int pblockID);

	// const access to class members
	int	numEmitters() const { return m_numEmitters; }
	const Tab<INode*>& objectsToShow()	const	{ return m_objectsToShow; }
	INode* objectToShow(int i)			const	{ return m_objectsToShow[i]; }

	// access to class members
	std::map<IObject*, PFOperatorPositionOnObjectLocalData>& _localData()	{ return m_localData; }
	PFOperatorPositionOnObjectLocalData& _localData(IObject* pCont) { return m_localData[pCont]; }
	int& _numEmitters() { return m_numEmitters; }
	Tab<INode*>&		_objectsToShow()	{ return m_objectsToShow; }
	INode*&				_objectToShow(int i){ return m_objectsToShow[i]; }

private:
	static GeomObjectValidatorClass validator;
	static bool validatorInitiated;
	std::map<IObject*, PFOperatorPositionOnObjectLocalData>	m_localData;
	int m_numEmitters; // to track change in number of emitters
	Tab<INode*> m_objectsToShow;
};

} // end of namespace PFActions

#endif // _PFOPERATORPOSITIONONOBJECT_H_