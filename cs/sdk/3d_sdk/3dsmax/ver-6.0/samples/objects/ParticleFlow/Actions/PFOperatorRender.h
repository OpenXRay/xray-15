/**********************************************************************
 *<
	FILE: PFOperatorRender.h

	DESCRIPTION: Render Operator header
				 The Operator is used to define appearance of the particles
				 in the current Event (or globally if it's a global
				 Operator) during render

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORRENDER_H_
#define _PFOPERATORRENDER_H_

#include <map>

#include "max.h"
#include "iparamb2.h"
#include "chkmtlapi.h"

#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPFIntegrator.h"
#include "IPFRender.h"
#include "IPViewItem.h"
#include "IPFSystem.h"
#include "OneClickCreateCallBack.h"
#include "PFClassIDs.h"
#include "IParticleChannels.h"

namespace PFActions {

class ParticleGroup;

class PFOperatorRender:	public HelperObject, 
						public IPFAction,
						public IPFOperator,
						public IPFRender,
						public IPViewItem,
						public IChkMtlAPI
{
public:
	// constructor/destructor
	PFOperatorRender();
	~PFOperatorRender();
	
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
	void* GetInterface(ULONG id);

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

		// Render Operator doesn't support randomness
	bool	SupportRand() const	{ return false; }
	int		GetRand()			{ return 0; }
	void	SetRand(int seed)	{ ; }
	int		NewRand()			{ return 0; }

	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	bool IsInstant() const { return false; }
	const Interval ActivityInterval() const { return FOREVER; }

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);
	
	// From IPFRender interface
	int HasRenderableGeometry();
	Mesh* GetRenderMesh(IObject* pCont, TimeValue t, Object* pSystem, INode *inode, View& view, BOOL& needDelete);
	int NumberOfRenderMeshes(IObject* pCont, TimeValue t, Object* pSystem);
	Mesh* GetMultipleRenderMesh(IObject* pCont, TimeValue t, Object* pSystem, INode *inode, View& view, BOOL& needDelete, int meshNumber);
	void GetMultipleRenderMeshTM(IObject* pCont, TimeValue t, Object* pSystem, INode *inode, View& view, int meshNumber, 
					Matrix3& meshTM, Interval& meshTMValid);

	// From IPViewItem interface
	int NumPViewParamBlocks() const { return 1; }
	IParamBlock2* GetPViewParamBlock(int i) const { if (i==0) return pblock(); else return NULL; }
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// from IChkMtlAPI interface
	BOOL SupportsParticleIDbyFace();
	int  GetParticleFromFace(int faceID);
		// doesn't support the rest; using the default implementation
	BOOL SupportsIndirMtlRefs() { return FALSE; }
	int NumberOfMtlsUsed() { return 0; }
	Mtl *GetNthMtl(int n) { return NULL; }
	int  GetNthMaxMtlID(int n) { return 0; }
	Mtl *GetMaterialFromFace(int faceID) { return NULL; }

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

protected:
		void RefreshUI(WPARAM message, IParamMap2* map=NULL) const;
		void UpdatePViewUI(int updateID) const;

		bool getParticleNumData(IParticleChannelMeshR* chShape,
										  int particleIndex,
										  int displayType,
										  int& vertNum, int& faceNum);
		Mesh* getParticleMesh(IParticleChannelMeshR* chShape,
										IParticleChannelIntR* chMtlIndex,
										IParticleChannelMeshMapR* chMapping,
										int particleIndex, int displayType, BOOL& needDelete);
		bool isInvisible(int index);
		void recalcVisibility(int amount = 0);

		// const access to class members
		Object*				editOb()	const	{ return m_editOb; }
		IObjParam*			ip()		const	{ return m_ip; }
		IParamBlock2*		pblock()	const	{ return m_pblock; }
		HBITMAP				activeIcon()	const	{ return m_activeIcon; }
		HBITMAP				inactiveIcon()	const	{ return m_inactiveIcon; }
		const Tab<int>&		faceToParticle()const	{ return m_faceToParticle; }
		int					faceToParticle(int i) const { return m_faceToParticle[i]; }
		const BitArray&		invisibleParticles() const { return m_invisibleParticles; }

		// access to class members
		Object*&			_editOb()			{ return m_editOb; }
		IObjParam*&			_ip()				{ return m_ip; }
		IParamBlock2*&		_pblock()			{ return m_pblock; }
		HBITMAP&		_activeIcon()			{ return m_activeIcon; }
		HBITMAP&		_inactiveIcon()			{ return m_inactiveIcon; }
		Tab<int>&		_faceToParticle()		{ return m_faceToParticle; }
		int&			_faceToParticle(int i)	{ return m_faceToParticle[i]; }
		std::map<IObject*, int>& _totalParticles() { return m_totalParticles; }
		int&			_totalParticles(IObject* pCont) { return m_totalParticles[pCont]; }
		BitArray&		_invisibleParticles()	{ return m_invisibleParticles; }

private:
		static Object *m_editOb;
		static IObjParam *m_ip;

		IParamBlock2*	m_pblock;

		HBITMAP m_activeIcon;
		HBITMAP m_inactiveIcon;

		// mapping from face number to particle index
		Tab<int> m_faceToParticle;

		std::map<IObject*, int> m_totalParticles;
		BitArray m_invisibleParticles;
};


} // end of namespace PFActions

#endif // _PFOPERATORRENDER_H_

