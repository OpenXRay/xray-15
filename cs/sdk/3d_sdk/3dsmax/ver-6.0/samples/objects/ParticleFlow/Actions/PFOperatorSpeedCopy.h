/**********************************************************************
 *<
	FILE:			PFOperatorSpeedCopy.h

	DESCRIPTION:	SpeedCopy Operator header
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSPEEDCOPY_H_
#define _PFOPERATORSPEEDCOPY_H_

#include <map>

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PF_TMBlock
{
public:
	PF_TMBlock(INode* node, TimeValue timeOffset);
	Matrix3* GetObjectTM(TimeValue time);		

protected:
	PF_TMBlock(); // no default constructor

private:
	INode* m_node;
	TimeValue m_timeOffset;
	int m_acquired[256];
	Matrix3 m_tm[256];
};

class PF_TMBlock2
{
public:
	PF_TMBlock2(INode* node, TimeValue timeOffset);
	~PF_TMBlock2();
	Matrix3* GetObjectTM(TimeValue time);		

protected:
	PF_TMBlock2(); // no default constructor

private:
	INode* m_node;
	TimeValue m_timeOffset;
	PF_TMBlock* m_tmBlock[256];
};

class PF_TMBlock3
{
public:
	PF_TMBlock3(INode* node, TimeValue timeOffset);
	~PF_TMBlock3();
	Matrix3* GetObjectTM(TimeValue time);		

protected:
	PF_TMBlock3(); // no default constructor

private:
	INode* m_node;
	TimeValue m_timeOffset;
	PF_TMBlock2* m_tmBlock2[256];
};

class PFObjectTMCacher 
{
public:
	PFObjectTMCacher(INode* node);
	~PFObjectTMCacher();

	Matrix3* GetObjectTM(TimeValue time);
	void Release(); // free cached TM values

protected:
	PFObjectTMCacher(); // no default constructor

private:
	INode* m_node;
	PF_TMBlock3* m_tmBlock3[256];
};

class PFOperatorSpeedCopy:	public PFSimpleOperator 
{
public:
	PFOperatorSpeedCopy();

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
	bool Release(IObject* pCont);
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }
	bool Has3dIcon() const { return true; }
	int	IsColorCoordinated() const;

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

	bool	SupportRand() const { return true; }
	int		GetRand();
	void	SetRand(int seed);

	// from PFOperatorSpeedCopy
	PolyShape*	GetIconShape(TimeValue time);
	Mesh*		GetIconMesh(TimeValue time);
	void		UpdateIcon(TimeValue time);
	void		InvalidateIcon();
	void		GetIconBoundBox(TimeValue t, Box3& box ); 

protected:
		// const access to class members
		PolyShape*		iconShape()	const	{ return m_iconShape; }
		Mesh*			iconMesh()	const	{ return m_iconMesh; }
		const Interval&	validIcon()	const	{ return m_validIcon; }

		// access to class members
		PolyShape*&		_iconShape()		{ return m_iconShape; }
		Mesh*&			_iconMesh()			{ return m_iconMesh; }
		Interval&		_validIcon()		{ return m_validIcon; }
		PFObjectTMCacher*& _tmCacher(IObject* pCont) { return m_tmCacher[pCont]; }
		std::map<IObject*, PFObjectTMCacher*>& _tmCacher() { return m_tmCacher; }

private:
		PolyShape*	m_iconShape;
		Mesh*		m_iconMesh;
		Interval	m_validIcon;
		std::map<IObject*, PFObjectTMCacher*> m_tmCacher;
};

} // end of namespace PFActions

#endif // _PFOPERATORSPEEDCOPY_H_