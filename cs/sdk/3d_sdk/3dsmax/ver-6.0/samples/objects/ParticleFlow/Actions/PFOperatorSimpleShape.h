/**********************************************************************
 *<
	FILE:			PFOperatorSimpleShape.h

	DESCRIPTION:	SimpleShape Operator header
					Operator to assign simple shape to particles

	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSIMPLESHAPE_H_
#define _PFOPERATORSIMPLESHAPE_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"

namespace PFActions {

class PFOperatorSimpleShape:	public PFSimpleOperator 
{
public:
	PFOperatorSimpleShape();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	// From ReferenceMaker
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);

	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

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
	// mesh shape builders
	void BuildMesh(int meshType, float size);
	void BuildMeshVertex();
	void BuildMeshPyramid(float size);
	void BuildMeshCube(float size);
	void BuildMeshSphere(float size);

	// const access to class members
	const Mesh&	mesh() const	{ return m_mesh; }

	// access to class members
	Mesh&		_mesh()			{ return m_mesh; }

protected:
	Mesh		m_mesh;
};

} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESHAPE_H_