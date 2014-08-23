/**********************************************************************
 *<
	FILE:			PFOperatorMarkShape.h

	DESCRIPTION:	MarkShape Operator header
					Operator to assign mark shape to particles

	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-29-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORMARKSHAPE_H_
#define _PFOPERATORMARKSHAPE_H_

#include <map>

#include "IPFOperator.h"
#include "IPFActionState.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

// reference IDs
enum {	kMarkShape_reference_pblock,
		kMarkShape_reference_num = 1 };


class PFOperatorMarkShapeState:	public IObject, 
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
	friend class PFOperatorMarkShape;

		// const access to class members
		ULONG		actionHandle()	const	{ return m_actionHandle; }		
		const RandGenerator* randGenVar()	const	{ return &m_randGenVar; }
		const RandGenerator* randGenOrn()	const	{ return &m_randGenOrn; }
		const RandGenerator* randGenOffset()const	{ return &m_randGenOffset; }

		// access to class members
		ULONG&			_actionHandle()		{ return m_actionHandle; }
		RandGenerator*	_randGenVar()		{ return &m_randGenVar; }
		RandGenerator*	_randGenOrn()		{ return &m_randGenOrn; }
		RandGenerator*	_randGenOffset()	{ return &m_randGenOffset; }

private:
		ULONG m_actionHandle;
		RandGenerator m_randGenVar;		
		RandGenerator m_randGenOrn;		
		RandGenerator m_randGenOffset;
};

class PFOperatorMarkShape:	public PFSimpleOperator 
{
public:
	PFOperatorMarkShape();
	~PFOperatorMarkShape();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

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

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

protected:
	static bool IsNodeAnimated(INode *iNode);
	static TriObject* GetTriObjectFromNode(INode *iNode, const TimeValue t, bool &deleteIt);
	static void CacheObjectTM(INode *iNode, TimeValue timeValueStart, TimeValue timeValueEnd, Tab<Matrix3> &cachedTM);
	static void InterpolateObjectTM(PreciseTimeValue time, TimeValue timeValueStart,
									Tab<Matrix3>& cachedTM, Matrix3& nodeTM);
	static void SetAlignTM(const Point3 &pivot, const Point3 &zDir, const Point3 &xDir, Matrix3 &alignTM);
	static Point2 ProjectedSize(const Mesh* orgMesh, const Quat& orgOrient, const Point3& orgScale, const Matrix3& shapeTM);
	static bool IntersectMesh(Mesh &mesh, const Point3 &point, const Point3 &vector, Point3 &contactPoint, Point3 &contactNormal);
	static DWORD ClosestFaceOnMesh(Mesh &mesh, const Point3 &point);
	static bool ChipOffMesh(const Mesh &mesh, const Mesh &boxMesh, const Matrix3 &boxTM, Mesh &markMesh, BOOL generateMapping);
	static void ContactElementMesh(Mesh &mesh, const Matrix3 &meshTM, const Point3 &point, const Point3 &vector);
	static void SurfaceOffsetMesh(Mesh &mesh, float offset);

	// Build Mesh
	void BuildContactMesh(const TimeValue t);
	void CreateRectangleMesh();
	void CreateBoxMesh();
	void BuildRectangleMesh();
	void BuildBoxMesh(Point3 &dimensions);

	// Shape orientation
	Point3 GetAlignDirection(const TimeValue t, const Point3& normal, const Point3& speed, const Quat& orientation, RandGenerator* randGen);
	void GetWidthLength(const TimeValue t, float &width, float &length);

	// Scale factor
	static float GetScaleFactor(RandGenerator* randGen, float variation);

	// const access to class members
	const RandObjLinker&	randLinkerOrn()	const	{ return m_randLinkerOrn; }
	const RandObjLinker&	randLinkerVar()	const	{ return m_randLinkerVar; }
	const RandObjLinker&	randLinkerOffset()const	{ return m_randLinkerOffset; }
	const Mesh&					contactMesh()	const	{ return m_contactMesh; }
	const Mesh&					flippedMesh()	const	{ return m_flippedMesh; }
	const Mesh&					rectMesh()		const	{ return m_rectMesh; }
	const Mesh&					boxMesh()		const	{ return m_boxMesh; }
	const Mesh&					markMesh()		const	{ return m_markMesh; }
	bool						hasContactObj()	const	{ return m_hasContactObj; }

	// access to class members
	RandObjLinker&	_randLinkerOrn()	{ return m_randLinkerOrn; }
	RandObjLinker&	_randLinkerVar()	{ return m_randLinkerVar; }
	RandObjLinker&	_randLinkerOffset()	{ return m_randLinkerOffset; }
	Mesh&				_contactMesh()		{ return m_contactMesh; }
	Mesh&				_flippedMesh()		{ return m_flippedMesh; }
	Mesh&				_rectMesh()			{ return m_rectMesh; }
	Mesh&				_boxMesh()			{ return m_boxMesh; }
	Mesh&				_markMesh()			{ return m_markMesh; }
	bool&				_hasContactObj()	{ return m_hasContactObj; }

private:
	RandObjLinker	m_randLinkerOrn;	// for random orientation or orientation divergence
	RandObjLinker	m_randLinkerVar;	// for size variation
	RandObjLinker	m_randLinkerOffset; // for offset variations
	Mesh				m_contactMesh;		// contact mesh
	Mesh				m_flippedMesh;		// flipped contact mesh
	Mesh				m_rectMesh;			// rectangle mesh
	Mesh				m_boxMesh;			// box mesh
	Mesh				m_markMesh;			// mark mesh
	bool m_hasContactObj;
};

void OrthogonalizeByZthenX(Matrix3& tm);

} // end of namespace PFActions

#endif // _PFOPERATORMARKSHAPE_H_