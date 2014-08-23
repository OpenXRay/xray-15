/**********************************************************************
 *<
	FILE:			PFOperatorSimplePosition.h

	DESCRIPTION:	SimplePosition Operator header
					Simple Operator to place particles on PF System icon

	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSIMPLEPOSITION_H_
#define _PFOPERATORSIMPLEPOSITION_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorSimplePositionLocalData
{
public:
	RandGenerator&		RandGenerator()		{ return m_randGenerator; }
	Tab<Point3>&		DistinctPoints()	{ return m_distinctPoints; }

private:
	::RandGenerator		m_randGenerator;
	Tab<Point3>			m_distinctPoints;
};

class PFOperatorSimplePositionState:	public IObject, 
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
	friend class PFOperatorSimplePosition;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGen()	const	{ return &m_randGen; }
		const PFOperatorSimplePositionLocalData* data() const { return &m_data; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGen()			{ return &m_randGen; }
		PFOperatorSimplePositionLocalData* _data() { return &m_data; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGen;		
		PFOperatorSimplePositionLocalData m_data;
};

class PFOperatorSimplePosition:	public PFSimpleOperator 
{
public:
	PFOperatorSimplePosition();

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
	bool IsEmitterTMDependent() const { return true; }
	bool IsEmitterPropDependent() const { return true; }

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

private:
	static void InterpolateObjectTM(PreciseTimeValue time, TimeValue timeValueStart,
									Tab<Matrix3>& cachedTM, Matrix3& nodeTM);
	static int AllowedDistinctPoints(int locationType, int emitterType, int numDistinctPoints);
	static void GenDistinctPoints(RandGenerator* randGen, int locationType, int numDistinctPoints,
									Tab<Point3>& distinctPoints, int emitterType);
	static Point3 GetRandomPosition(RandGenerator* randGen, int numDistinctPoints, Tab<Point3>& distinctPoints,
									int emitterType, Tab<float>& emitterDimensions);
	static Point3 GetRandomPosition(RandGenerator* randGen, int locationType, int emitterType, Tab<float>& emitterDimensions);
	static Point3 GetRandomPositionInRectangle(RandGenerator* randGen, int locationType, float xdim, float ydim);
	static Point3 GetRandomPositionInCircle(RandGenerator* randGen, int locationType, float radius);
	static Point3 GetRandomPositionInBox(RandGenerator* randGen, int locationType, float xdim, float ydim, float zdim);
	static Point3 GetRandomPositionInSphere(RandGenerator* randGen, int locationType, float radius);

	// access to class members
	std::map<IObject*, PFOperatorSimplePositionLocalData>& _pointsMap()	{ return m_pointsMap; }
	PFOperatorSimplePositionLocalData& _pointsMap(IObject* pCont) { return m_pointsMap[pCont]; }

protected:
	std::map<IObject*, PFOperatorSimplePositionLocalData>	m_pointsMap;
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEPOSITION_H_