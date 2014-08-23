/**********************************************************************
 *<
	FILE: PFOperatorDisplay.h

	DESCRIPTION: Display Operator header
				 The Operator is used to define appearance of the particles
				 in the current Event (or globally if it's a global
				 Operator) for viewports

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORDISPLAY_H_
#define _PFOPERATORDISPLAY_H_

#include <map>

#include "max.h"
#include "iparamb2.h"

#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPFIntegrator.h"
#include "IPFViewport.h"
#include "IPViewItem.h"
#include "IPFSystem.h"
#include "OneClickCreateCallBack.h"
#include "PFClassIDs.h"

namespace PFActions {

class ParticleGroup;

class PFOperatorDisplay:	public HelperObject, 
							public IPFAction,
							public IPFOperator,
							public IPFViewport,
							public IPViewItem
{
public:
	// constructor/destructor
	PFOperatorDisplay();
	~PFOperatorDisplay();
	
	// From InterfaceServer
	BaseInterface* GetInterface(Interface_ID id);

	// From Animatable
	void DeleteThis();
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next);
	int NumSubs() { return 1; } // the paramBlock
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	ParamDimension* GetParamDimension(int i);
	BOOL IsAnimated() { return _pblock()->IsAnimated(); }
	void FreeCaches() { ; }
	int NumParamBlocks() { return 1; } // the paramBlock
	IParamBlock2* GetParamBlock(int i);
	IParamBlock2* GetParamBlockByID(short id);

	// From ReferenceMaker
	int NumRefs() { return 1; } // the paramBlock
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);
	int RemapRefOnLoad(int iref);
	RefTargetHandle Clone(RemapDir &remap);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// From BaseObject
	TCHAR *GetObjectName();
	CreateMouseCallBack* GetCreateMouseCallBack() { return OneClickCreateCallBack::Instance(); }
	BOOL OKToChangeTopology(TSTR &modName) { return FALSE; }

	// From Object
	void InitNodeName(TSTR& s) { s = GetObjectName(); }
	ObjectState Eval(TimeValue t) { return ObjectState(this); }

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// From IPFAction interface
	bool Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	bool Release(IObject* pCont);
	bool IsNonExecutable() const { return true; }

		// Display Operator doesn't support randomness
	bool	SupportRand() const	{ return false; }
	int		GetRand()			{ return 0; }
	void	SetRand(int seed)	{ ; }
	int		NewRand()			{ return 0; }

	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	bool IsInstant() const { return false; }
	const Interval ActivityInterval() const { return FOREVER; }

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);
	
	// From IPFViewport interface
	int Display(IObject* pCont, TimeValue time, Object* pSystem, INode* psNode, INode* pgNode, ViewExp *vpt, int flags);
	int HitTest(IObject* pCont, TimeValue time, Object* pSystem, INode* psNode, INode* pgNode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void GetWorldBoundBox(IObject* pCont, TimeValue time, Object* pSystem, INode* pgNode, ViewExp* vp, Box3& box );
	void GetLocalBoundBox(IObject* pCont, TimeValue time, Object* pSystem, INode* pgNode, ViewExp* vp, Box3& box );
	DWORD GetWireColor() const;
	void SetWireColor(DWORD color);
	void MaybeEnlargeViewportRect(IObject* pCont, TimeValue time, GraphicsWindow *gw, Rect &rect);

	// From IPViewItem interface
	int NumPViewParamBlocks() const { return 1; }
	IParamBlock2* GetPViewParamBlock(int i) const { if (i==0) return pblock(); else return NULL; }
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

public:
		bool isInvisible(int index);
		void recalcVisibility(int amount = 0);

		// const access to class members
		Object*				editOb()	const	{ return m_editOb; }
		IObjParam*			ip()		const	{ return m_ip; }
		IParamBlock2*		pblock()	const	{ return m_pblock; }
		HBITMAP				activeIcon()	const	{ return m_activeIcon; }
		HBITMAP				inactiveIcon()	const	{ return m_inactiveIcon; }
		const BitArray&		invisibleParticles() const { return m_invisibleParticles; }

		// access to class members
		Object*&			_editOb()			{ return m_editOb; }
		IObjParam*&			_ip()				{ return m_ip; }
		IParamBlock2*&		_pblock()			{ return m_pblock; }
		HBITMAP&		_activeIcon()			{ return m_activeIcon; }
		HBITMAP&		_inactiveIcon()			{ return m_inactiveIcon; }
		std::map<IObject*, int>& _totalParticles() { return m_totalParticles; }
		int&			_totalParticles(IObject* pCont) { return m_totalParticles[pCont]; }
		BitArray&		_invisibleParticles()	{ return m_invisibleParticles; }

protected:
		void RefreshSelectionUI() const;
		void UpdatePViewUI(int updateID) const;

private:
		static Object *m_editOb;
		static IObjParam *m_ip;

		IParamBlock2*	m_pblock;

		HBITMAP m_activeIcon;
		HBITMAP m_inactiveIcon;

		std::map<IObject*, int> m_totalParticles;
		BitArray m_invisibleParticles;
};


} // end of namespace PFActions

#endif // _PFOPERATORDISPLAY_H_

