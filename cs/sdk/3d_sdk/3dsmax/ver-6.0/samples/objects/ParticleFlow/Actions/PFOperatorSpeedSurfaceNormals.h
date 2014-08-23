/**********************************************************************
 *<
	FILE:			PFOperatorSpeedSurfaceNormals.h

	DESCRIPTION:	SpeedSurfaceNormals Operator header
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORSPEEDSURFACENORMALS_H_
#define _PFOPERATORSPEEDSURFACENORMALS_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

#include <map>

namespace PFActions {

class PFOperatorSpeedBySurfaceLocalData
{
public:
	~PFOperatorSpeedBySurfaceLocalData() { FreeAll(); }
	void FreeAll();
	void InitNodeGeometry(int num);
	int NodeGeometryCount() const;
	// get nodeGeometry by index
	PFNodeGeometry* GetNodeGeometry(int i);
	void SetNodeGeometry(int i, PFNodeGeometry* nodeGeom);

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

class PFOperatorSpeedSurfaceNormals:	public PFSimpleOperator,
										public HitByNameDlgCallback 
{
public:
	PFOperatorSpeedSurfaceNormals();

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
	bool Release(IObject* pCont);
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	// From HitByNameDlgCallback
	TCHAR *dialogTitle()	{ return GetString(IDS_SELECTSURFACEOBJECTS); }
	TCHAR *buttonText()		{ return GetString(IDS_SELECT); }
	BOOL useFilter()		{ return TRUE; }
	int filter(INode *node);
	BOOL useProc()			{ return TRUE; }
	void proc(INodeTab &nodeTab);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

	bool	SupportRand() const { return true; }
	int		GetRand();
	void	SetRand(int seed);
protected:
	// returns actual number of surface objects
	int getNumSurfaces(); 
	// update surfacesMaxscript info on basis of surfaces info
	bool updateFromRealSurfaces();
	// update surfaces info on basis of surfacesMaxscript info
	bool updateFromMXSSurfaces();
	// update object names in UI and dynamic names
	bool updateObjectNames(int pblockID);

	// const access to class members
	int numSurfaces() const { return m_numSurfaces; }
	const Tab<INode*>& objectsToShow()	const	{ return m_objectsToShow; }
	INode* objectToShow(int i)			const	{ return m_objectsToShow[i]; }

	// access to class members
	std::map<IObject*, PFOperatorSpeedBySurfaceLocalData>& _localData()	{ return m_localData; }
	PFOperatorSpeedBySurfaceLocalData& _localData(IObject* pCont) { return m_localData[pCont]; }
	int& _numSurfaces() { return m_numSurfaces; }
	Tab<INode*>&		_objectsToShow()	{ return m_objectsToShow; }
	INode*&				_objectToShow(int i){ return m_objectsToShow[i]; }

private:
	static GeomObjectValidatorClass validator;
	static bool validatorInitiated;
	std::map<IObject*, PFOperatorSpeedBySurfaceLocalData>	m_localData;
	int m_numSurfaces;
	Tab<INode*> m_objectsToShow;
};

} // end of namespace PFActions

#endif // _PFOPERATORSPEEDSURFACENORMALS_H_