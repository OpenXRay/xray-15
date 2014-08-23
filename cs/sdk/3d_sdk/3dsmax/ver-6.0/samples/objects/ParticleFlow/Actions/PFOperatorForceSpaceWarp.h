/**********************************************************************
 *<
	FILE:			PFOperatorForceSpaceWarp.h

	DESCRIPTION:	ForceSpaceWarp Operator header
					Operator to let older max space warps interact with the system

	CREATED BY:		Peter Watje

	HISTORY:		created 02-06-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef _PFOPERATORFORCESPACEWARP_H_
#define _PFOPERATORFORCESPACEWARP_H_

#include "IPFOperator.h"
#include "PFSimpleOperator.h"
#include "RandObjLinker.h"

namespace PFActions {

class PFOperatorForceSpaceWarp:	public PFSimpleOperator,
								public HitByNameDlgCallback 
{
public:
	PFOperatorForceSpaceWarp();

	// From Animatable
	void GetClassName(TSTR& s);
	Class_ID ClassID();
	void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);

	int NumSubs() { return 2; } // the paramBlocks
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	ParamDimension* GetParamDimension(int i);
	int NumParamBlocks() { return 2; } // the paramBlocks
	IParamBlock2* GetParamBlock(int i);
	IParamBlock2* GetParamBlockByID(short id);

	// From ReferenceMaker
	int NumRefs() { return 2; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// From ReferenceTarget
	RefTargetHandle Clone(RemapDir &remap);

	// From BaseObject
	TCHAR *GetObjectName();

	// from FPMixinInterface
	FPInterfaceDesc* GetDescByID(Interface_ID id);

	// from IPFAction
	const ParticleChannelMask& ChannelsUsed(const Interval& time) const;
	const Interval ActivityInterval() const { return FOREVER; }

	bool SupportScriptWiring() const { return true; }
	bool GetUseScriptWiring() const;
	void SetUseScriptWiring(bool useScriptWiring);

	// From IPFOperator interface
	bool Proceed(IObject* pCont, PreciseTimeValue timeStart, PreciseTimeValue& timeEnd, Object* pSystem, INode* pNode, INode* actionNode, IPFIntegrator* integrator);

	// from IPViewItem interface
	int NumPViewParamBlocks() const;
	IParamBlock2* GetPViewParamBlock(int i) const;

	bool HasCustomPViewIcons() { return true; }
	HBITMAP GetActivePViewIcon();
	HBITMAP GetInactivePViewIcon();
	bool HasDynamicName(TSTR& nameSuffix);

	void UpdatePViewUI(IParamBlock2* pblock, int updateID);
	
	// From HitByNameDlgCallback
	TCHAR *dialogTitle()	{ return GetString(IDS_SELECTFORCESPACEWARPS); }
	TCHAR *buttonText()		{ return GetString(IDS_SELECT); }
	BOOL useFilter()		{ return TRUE; }
	int filter(INode *node);
	BOOL useProc()			{ return TRUE; }
	void proc(INodeTab &nodeTab);

	// supply ClassDesc descriptor for the concrete implementation of the operator
	ClassDesc* GetClassDesc() const;

protected:
	// returns actual number of referenced force SWs
	int getNumForces();
	// update forcesMaxscript info on basis of forces info
	bool updateFromRealForces();
	// update forces info on basis of forcesMaxscript info
	bool updateFromMXSForces();
	// update object names in UI and dynamic names
	bool updateObjectNames(int pblockID);

	// const access to class members
	IParamBlock2*	scriptPBlock()		const	{ return m_scriptPBlock; }
	int				numForces() const { return m_numForces; }
	const Tab<INode*>& objectsToShow()	const	{ return m_objectsToShow; }
	INode* objectToShow(int i)			const	{ return m_objectsToShow[i]; }

	// access to class members
	IParamBlock2*&	_scriptPBlock()			{ return m_scriptPBlock; }
	int&			_numForces()			{ return m_numForces; }
	Tab<INode*>&		_objectsToShow()	{ return m_objectsToShow; }
	INode*&				_objectToShow(int i){ return m_objectsToShow[i]; }

private:
	// script wiring parameters
	IParamBlock2*	m_scriptPBlock;

	static ForceSpaceWarpValidatorClass validator;
	static bool validatorInitiated;
	int m_numForces;
	Tab<INode*> m_objectsToShow;
};

// for quick fix in Character Studio Vector Field SW
#define	CS_VFIELDOBJECT_CLASS_ID Class_ID(0x73413e89, 0x4f685b58)
class CS_VectorField : public ForceField 
{
public:
	void *obj; // VFieldObject *obj;
	ParticleObject *partobj;
	INode *node;
	Matrix3 tm;
	Interval tmValid;
	Point3 force;
	Interval fValid;
	int type;
	Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel,int index) { return Point3::Origin; }
};

} // end of namespace PFActions

#endif // _PFOPERATORFORCESPACEWARP_H_