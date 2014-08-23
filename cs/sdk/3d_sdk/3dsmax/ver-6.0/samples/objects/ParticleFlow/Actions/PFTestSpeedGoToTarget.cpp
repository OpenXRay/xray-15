/**********************************************************************
 *<
	FILE: PFTestSpeedGoToTarget.h

	DESCRIPTION: SpeedGoToTarget Test implementation
				 The test directs particles to reach a goal(target)
				 and once the target has been reached sends particles
				 to the next ActionList

	CREATED BY: Oleg Bayborodin

	HISTORY: created 07-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_Icon.h"

#include "PFTestSpeedGoToTarget.h"

#include "PFTestSpeedGoToTarget_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// SpeedGoToTarget Test creates a particle channel to keep track if the particle is initialized or not
#define PARTICLECHANNELGOTOINITR_INTERFACE Interface_ID(0x269f560f, 0x1eb34500) 
#define PARTICLECHANNELGOTOINITW_INTERFACE Interface_ID(0x269f560f, 0x1eb34501) 
#define GetParticleChannelGoToInitRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELGOTOINITR_INTERFACE)) 
#define GetParticleChannelGoToInitWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELGOTOINITW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to store a reference to a target node the particle is aiming at
// The channel is created if there are more than 1 target
#define PARTICLECHANNELTARGETNODER_INTERFACE Interface_ID(0x269f5610, 0x1eb34500) 
#define PARTICLECHANNELTARGETNODEW_INTERFACE Interface_ID(0x269f5610, 0x1eb34501) 
#define GetParticleChannelTargetNodeRInterface(obj) ((IParticleChannelINodeR*)obj->GetInterface(PARTICLECHANNELTARGETNODER_INTERFACE)) 
#define GetParticleChannelTargetNodeWInterface(obj) ((IParticleChannelINodeW*)obj->GetInterface(PARTICLECHANNELTARGETNODEW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to store coordinates
// of a target point
// The channel is always created
// If Follow Target is OFF, it's world coordinates
// If Follow Target is ON, then its local coordinates
//			If Animated Shape is OFF, it's local target coordinates
//			If Animated Shape is ON, it's local coordinates in the target face space
#define PARTICLECHANNELTARGETPOINTR_INTERFACE Interface_ID(0x269f5611, 0x1eb34500) 
#define PARTICLECHANNELTARGETPOINTW_INTERFACE Interface_ID(0x269f5611, 0x1eb34501) 
#define GetParticleChannelTargetPointRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELTARGETPOINTR_INTERFACE)) 
#define GetParticleChannelTargetPointWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELTARGETPOINTW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to store an index of the face
// the particle is aiming at.
// The channel is created if target is a mesh, and the shape is animated, and follow target is ON
#define PARTICLECHANNELFACEINDEXR_INTERFACE Interface_ID(0x269f5612, 0x1eb34500) 
#define PARTICLECHANNELFACEINDEXW_INTERFACE Interface_ID(0x269f5612, 0x1eb34501) 
#define GetParticleChannelFaceIndexRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELFACEINDEXR_INTERFACE)) 
#define GetParticleChannelFaceIndexWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELFACEINDEXW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to store cruise speed magnitude
// The channel is created if control by speed, and Use Cruise Speed is OFF
// Then when a particle enters the event, its speed is stored as a desirable cruise speed
// Otherwise the cruise speed is calculated according to Cruise Speed and Cruise Speed Variation spinners
// The channel is also used to store docking value for Control by Time when there is a docking direction
// but docking speed is not used. Then for the speed magnitude of a particle when it enters the event, 
// is used as a docking value.
#define PARTICLECHANNELCRUISESPEEDR_INTERFACE Interface_ID(0x269f5613, 0x1eb34500) 
#define PARTICLECHANNELCRUISESPEEDW_INTERFACE Interface_ID(0x269f5613, 0x1eb34501) 
#define GetParticleChannelCruiseSpeedRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELCRUISESPEEDR_INTERFACE)) 
#define GetParticleChannelCruiseSpeedWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELCRUISESPEEDW_INTERFACE)) 

// SpeedGoToTarget test creates a particle channel to store random float [0,1] per particle
// This float is used to calculate a cruise speed, if cruise speed variation is greater than zero
// The channel is created if Control By Speed and Use Cruise Speed is ON
#define PARTICLECHANNELCRUISEVARR_INTERFACE Interface_ID(0x269f5614, 0x1eb34500) 
#define PARTICLECHANNELCRUISEVARW_INTERFACE Interface_ID(0x269f5614, 0x1eb34501) 
#define GetParticleChannelCruiseVarRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELCRUISEVARR_INTERFACE)) 
#define GetParticleChannelCruiseVarWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELCRUISEVARW_INTERFACE)) 

// SpeedGoToTarget test creates a particle channel to store time for a particle to reach a target
// The channel is created if Control By Time
#define PARTICLECHANNELTARGETTIMER_INTERFACE Interface_ID(0x269f5615, 0x1eb34500) 
#define PARTICLECHANNELTARGETTIMEW_INTERFACE Interface_ID(0x269f5615, 0x1eb34501) 
#define GetParticleChannelTargetTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELTARGETTIMER_INTERFACE)) 
#define GetParticleChannelTargetTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELTARGETTIMEW_INTERFACE)) 

/* // obsolete channel
// SpeedGoToTarget test creates a particle channel to store control type for each particle: speed or time
// The channel is created only if the control type is "Speed Then Time"
#define PARTICLECHANNELCONTROLTYPER_INTERFACE Interface_ID(0x269f5616, 0x1eb34500) 
#define PARTICLECHANNELCONTROLTYPEW_INTERFACE Interface_ID(0x269f5616, 0x1eb34501) 
#define GetParticleChannelControlTypeRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELCONTROLTYPER_INTERFACE)) 
#define GetParticleChannelControlTypeWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELCONTROLTYPEW_INTERFACE)) 
*/
// SpeedGoToTarget Test creates a particle channel to store docking speed magnitude
// The channel is created if control by time, and Use Docking Speed is ON
// Then when a particle enters the event, its docking speed is calculated and stored stored as a desirable docking speed
#define PARTICLECHANNELDOCKINGSPEEDR_INTERFACE Interface_ID(0x269f5617, 0x1eb34500) 
#define PARTICLECHANNELDOCKINGSPEEDW_INTERFACE Interface_ID(0x269f5617, 0x1eb34501) 
#define GetParticleChannelDockingSpeedRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELDOCKINGSPEEDR_INTERFACE)) 
#define GetParticleChannelDockingSpeedWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELDOCKINGSPEEDW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to store docking direction vector
// The channel is created if docking direction is used and FollowTarget is OFF 
// The channel stores normalized vector in world coordinates
#define PARTICLECHANNELDOCKINGVECTORR_INTERFACE Interface_ID(0x269f5618, 0x1eb34500) 
#define PARTICLECHANNELDOCKINGVECTORW_INTERFACE Interface_ID(0x269f5618, 0x1eb34501) 
#define GetParticleChannelDockingVectorRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELDOCKINGVECTORR_INTERFACE)) 
#define GetParticleChannelDockingVectorWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELDOCKINGVECTORW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to keep track if the particle already reached its goal
// Once a particle reaches the goal, it is no more under control of the operator
#define PARTICLECHANNELFINISHEDR_INTERFACE Interface_ID(0x269f5619, 0x1eb34500) 
#define PARTICLECHANNELFINISHEDW_INTERFACE Interface_ID(0x269f5619, 0x1eb34501) 
#define GetParticleChannelFinishedRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELFINISHEDR_INTERFACE)) 
#define GetParticleChannelFinishedWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELFINISHEDW_INTERFACE)) 

// SpeedGoToTarget Test creates a particle channel to store distance from a particle to its target
// when the particle enters the event. 
// The channel is created if Ease In is greater then zero. Then the distance is used
// to calculated the desirable particle speed as a function of the current distance to the target
#define PARTICLECHANNELINITDISTANCER_INTERFACE Interface_ID(0x269f561a, 0x1eb34500) 
#define PARTICLECHANNELINITDISTANCEW_INTERFACE Interface_ID(0x269f561a, 0x1eb34501) 
#define GetParticleChannelInitDistanceRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELINITDISTANCER_INTERFACE)) 
#define GetParticleChannelInitDistanceWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELINITDISTANCEW_INTERFACE)) 



//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFOperatorSpeedGoToTargetLocalData				 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFOperatorSpeedGoToTargetLocalData::FreeAll()
{
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) delete _geomData(i);
	_geomData().ZeroCount();
}

void PFOperatorSpeedGoToTargetLocalData::InitNodeGeometry(int num)
{
	_geomData().SetCount(num);
	for(int i=0; i<num; i++) _geomData(i) = NULL;
}

int PFOperatorSpeedGoToTargetLocalData::NodeGeometryCount() const
{
	return geomData().Count();
}

PFNodeGeometry* PFOperatorSpeedGoToTargetLocalData::GetNodeGeometry(int i)
{
	if ((i < 0) || (i >= geomData().Count())) return NULL;
	return geomData(i);
}

void PFOperatorSpeedGoToTargetLocalData::SetNodeGeometry(int i, PFNodeGeometry* nodeG)
{
	if ((i>=0) && (i<geomData().Count())) _geomData(i) = nodeG;
}

PFNodeGeometry* PFOperatorSpeedGoToTargetLocalData::GetNodeGeometry(TimeValue time, float randValue)
{
	double totalShare = 0.0;
	int i;
	for(i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL)
			totalShare += _geomData(i)->GetProbabilityShare(time);
	if (totalShare == 0.0f) return NULL;
	double curShare = 0.0;
	for(i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL) {
			curShare += _geomData(i)->GetProbabilityShare(time)/totalShare;
			if (curShare >= randValue) return geomData(i);
		}
	return NULL;
}

PFNodeGeometry* PFOperatorSpeedGoToTargetLocalData::GetNodeGeometry(INode* node)
{
	for(int i=0; i<geomData().Count(); i++)
		if (geomData(i) != NULL)
			if (geomData(i)->GetNode() == node)
				return geomData(i);
	return NULL;
}

PFNodeGeometry* PFOperatorSpeedGoToTargetLocalData::GetClosestNodeSurface(
									TimeValue time, const Point3& toPoint,
									Point3& worldLocation, Point3& localLocation, int& faceIndex)
{
	Point3 curWorldPoint, curLocalPoint;
	int curFaceIndex, nodeIndex;
	float minDistance2 = 1.0f;
	bool hasMin = false;
	for(int j=0; j<geomData().Count(); j++) {
		if (!geomData(j)->GetClosestPoint(time, toPoint, curWorldPoint, curLocalPoint, curFaceIndex))
				continue;
		Point3 difPoint = toPoint - curWorldPoint;
		float curDistance2 = DotProd(difPoint, difPoint);
		if ((curDistance2 < minDistance2) || !hasMin) {
			hasMin = true;
			minDistance2 = curDistance2;
			nodeIndex = j;
			worldLocation = curWorldPoint;
			localLocation = curLocalPoint;
			faceIndex = curFaceIndex;
		}
	}
	if (hasMin) return geomData(nodeIndex);
	return NULL;
}

PFNodeGeometry* PFOperatorSpeedGoToTargetLocalData::GetClosestNodePivot(
									TimeValue time, const Point3& toPoint)
{
	int nodeIndex;
	float minDistance2 = 1.0f;
	bool hasMin = false;
	for(int j=0; j<geomData().Count(); j++) {
		Matrix3* tm = geomData(j)->GetTM(time);
		if (tm == NULL) continue;
		Point3 difPoint = toPoint - tm->GetTrans();
		float curDistance2 = DotProd(difPoint, difPoint);
		if ((curDistance2 < minDistance2) || !hasMin) {
			hasMin = true;
			nodeIndex = j;
			minDistance2 = curDistance2;
		}
	}
	if (hasMin) return geomData(nodeIndex);
	return NULL;
}

PFNodeGeometry* PFOperatorSpeedGoToTargetLocalData::GetLeastDeviatedNode(
									TimeValue time, const Point3& fromPoint, const Point3& speed)
{
	int nodeIndex;
	float bestDeviation = 1.0f;
	bool hasMin = false;
	if (DotProd(speed, speed) <= 0.0f) return GetClosestNodePivot(time, fromPoint);
	Point3 normSpeed = Normalize(speed);
	for(int j=0; j<geomData().Count(); j++) {
		Matrix3* tm = geomData(j)->GetTM(time);
		if (tm == NULL) continue;
		Point3 dirVector = tm->GetTrans() - fromPoint;
		if (DotProd(dirVector, dirVector) <= 0.0f) return geomData(j);
		dirVector = Normalize(dirVector);
		float curDeviation = DotProd(dirVector, normSpeed);
		if ((curDeviation > bestDeviation) || !hasMin) {
			hasMin = true;
			nodeIndex = j;
			bestDeviation = curDeviation;
		}
	}
	if (hasMin) return geomData(nodeIndex);
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							PFOperatorSpeedGoToTarget						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+

// static members
Object*		PFTestSpeedGoToTarget::m_editOb				= NULL;
IObjParam*	PFTestSpeedGoToTarget::m_ip					= NULL;

GeomObjectValidatorClass PFTestSpeedGoToTarget::validator;
bool PFTestSpeedGoToTarget::validatorInitiated = false;

// constructors/destructors
PFTestSpeedGoToTarget::PFTestSpeedGoToTarget()
{ 
	_iconShape() = NULL; // lazy initialized in ::Display(...)
	_iconMesh() = NULL; // lazy initialized in ::Display(...)
	_validIcon() = NEVER;
	_numTargets() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kSpeedGoToTarget_targets;
		speedGoToTarget_paramBlockDesc.ParamOption(kSpeedGoToTarget_targets,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this);
}

PFTestSpeedGoToTarget::~PFTestSpeedGoToTarget()
{
	if (iconShape()) 
	{
		iconShape()->NewShape(); // to clear all data
		delete _iconShape();
		_iconShape() = NULL; 
	}
	
	if (iconMesh())
	{
		iconMesh()->FreeAll();
		delete _iconMesh();
		_iconMesh() = NULL;
	}
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestSpeedGoToTarget::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &speedGoToTarget_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &speedGoToTarget_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &speedGoToTarget_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestSpeedGoToTarget::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SPEEDGOTOTARGET_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSpeedGoToTarget::ClassID()
{
	return PFTestSpeedGoToTarget_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PFTestSpeedGoToTarget::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	static int colorRequest = -1;
	static INode* actionNode;

	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kSpeedGoToTarget_targetsMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSTargets();
					return REF_STOP;
				} 
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch ( lastUpdateID )
				{
				case kSpeedGoToTarget_type:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// break is omitted on purpose (bayboro 11-19-02)
				case kSpeedGoToTarget_useCruiseSpeed:
				case kSpeedGoToTarget_cruiseSpeedVariation:
				case kSpeedGoToTarget_timeVariation:
				case kSpeedGoToTarget_useDockingSpeed:
				case kSpeedGoToTarget_dockingSpeedVariation:
				case kSpeedGoToTarget_targetType:
				case kSpeedGoToTarget_follow:
				case kSpeedGoToTarget_pointType:
				case kSpeedGoToTarget_assignment:
					RefreshUI(kSpeedGoToTarget_message_update);
					break;
				case kSpeedGoToTarget_dockingType:
					RefreshUI(kSpeedGoToTarget_message_update);
					// the break is omitted on purpose (bayboro 12-18-02)
				case kSpeedGoToTarget_iconSize:
					InvalidateIcon();
					break;
				case kSpeedGoToTarget_colorType:
					NotifyDependents( FOREVER, (PartID)colorRequest, kPFMSG_UpdateWireColor );
					break;
				case kSpeedGoToTarget_targets:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealTargets();
					if (updateObjectNames(kSpeedGoToTarget_targets)) {
						RefreshUI(kSpeedGoToTarget_message_update);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				} 
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealTargets();
			break;
		// Initialization of Dialog
		case kSpeedGoToTarget_RefMsg_InitDlg:
			RefreshUI(kSpeedGoToTarget_message_update, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSpeedGoToTarget_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kSpeedGoToTarget_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kSpeedGoToTarget_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFTestSpeedGoToTarget::Clone(RemapDir &remap)
{
	PFTestSpeedGoToTarget* newTest = new PFTestSpeedGoToTarget();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
static PFActionCreateCallback speedGoTargetCallback;

CreateMouseCallBack* PFTestSpeedGoToTarget::GetCreateMouseCallBack()
{
	speedGoTargetCallback.init(this, pblock(), kSpeedGoToTarget_iconSize);
	return &speedGoTargetCallback;
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
int PFTestSpeedGoToTarget::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	int selected = inode->Selected();
	GraphicsWindow* gw = vpt->getGW();
	Material* curMtl;
	int numMtls;

	if (selected) {
		curMtl = SysUtil::GetWhiteMtl();
		numMtls = 1;
	} else {
		curMtl = inode->Mtls();
		numMtls = inode->NumMtls();
	}

	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_TWO_SIDED| (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0) );
	if (selected)
		gw->setColor( LINE_COLOR, GetSelColor());
	else if (inode->IsFrozen())
		gw->setColor( LINE_COLOR, GetFreezeColor());
	else {
		COLORREF rgb = COLORREF(inode->GetWireColor());
		gw->setColor( LINE_COLOR, Point3((int)GetRValue(rgb), (int)GetGValue(rgb), (int)GetBValue(rgb)));
	}

	Matrix3 mat = inode->GetObjTMAfterWSM(t);
	gw->setTransform(mat);

	GetIconShape(t)->Render(gw, curMtl, 
			(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL, numMtls);
	GetIconMesh(t)->render(gw, curMtl, 
			(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL, numMtls);

	gw->setRndLimits(rlim);
	return 0;
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
int PFTestSpeedGoToTarget::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	int shapeSelect=FALSE, meshSelect=FALSE;
	HitRegion hitRegion;
	GraphicsWindow* gw = vpt->getGW();
	DWORD rlim = gw->getRndLimits();
	MakeHitRegion( hitRegion, type, crossing, 4, p);
	gw->setRndLimits(GW_WIREFRAME|GW_BACKCULL|GW_TWO_SIDED| (rlim&GW_Z_BUFFER?GW_Z_BUFFER:0) );
	Matrix3 mat = inode->GetObjTMBeforeWSM(t);
	gw->setTransform(mat);
	meshSelect = GetIconMesh(t)->select(gw, inode->Mtls(), &hitRegion, flags & HIT_ABORTONHIT, inode->NumMtls());
	if (!meshSelect) // if Hit for mesh there is no need to check Hit for shape
		shapeSelect = GetIconShape(t)->Select(gw, inode->Mtls(), &hitRegion, flags & HIT_ABORTONHIT);
	gw->setRndLimits(rlim);

	return shapeSelect||meshSelect;
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{
	SysUtil::NeedToImplementLater();
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::GetWorldBoundBox(TimeValue t, INode * inode, ViewExp* vp, Box3& box )
{
	GetIconBoundBox(t, box);
	if (!box.IsEmpty()) {
		Matrix3 mat = inode->GetObjTMAfterWSM(t);
		box = box * mat;
	}
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp, Box3& box )
{
	GetIconBoundBox(t, box);	
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
TCHAR* PFTestSpeedGoToTarget::GetObjectName()
{
	return GetString(IDS_TEST_SPEEDGOTOTARGET_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
int	PFTestSpeedGoToTarget::GetRand()
{
	return pblock()->GetInt(kSpeedGoToTarget_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::SetRand(int seed)
{
	pblock()->SetValue(kSpeedGoToTarget_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void CollectPFNodeTargets(INode* inode, Tab<INode*>& collectedNodes)
{
	if (inode == NULL) return;
	// check out if the node is already collected
	bool alreadyCollected = false;
	int i;
	for(i=0; i<collectedNodes.Count(); i++) {
		if (inode == collectedNodes[i]) {
			alreadyCollected = true;
			break;
		}
	}

	if (!alreadyCollected) collectedNodes.Append(1, &inode);
	for(i=0; i<inode->NumberOfChildren(); i++)
		CollectPFNodeTargets(inode->GetChildNode(i), collectedNodes);
}

bool PFTestSpeedGoToTarget::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool initRes = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	// collect all nodes with surface geometry
	Tab<INode*> nodeList;
	int i;
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	for(i=0; i<pblock()->Count(kSpeedGoToTarget_targets); i++)
		CollectPFNodeTargets(pblock()->GetINode(kSpeedGoToTarget_targets, 0, i), nodeList);
	if (nodeList.Count() > 0) {
		_localData(pCont).InitNodeGeometry(nodeList.Count());
		for(i=0; i<nodeList.Count(); i++) {
			PFNodeGeometry* nodeG = new PFNodeGeometry();
			nodeG->Init(nodeList[i], pblock()->GetInt(kSpeedGoToTarget_animated, 0),
									false,
									kLocationType_surface, randGen);
			_localData(pCont).SetNodeGeometry(i, nodeG);
		}
	}
	return initRes;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestSpeedGoToTarget::Release(IObject* pCont)
{
	PFSimpleAction::Release(pCont);
	_localData(pCont).FreeAll();
	_localData().erase( pCont );
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFTestSpeedGoToTarget::ChannelsUsed(const Interval& time) const
{
								//  read									  &	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime||PCU_EventStart|PCU_Position|PCU_Speed|PCU_MXSInteger, // read channels
									PCU_Time|PCU_EventStart|PCU_Position|PCU_Speed); // write channels
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		// private channels
		mask.AddChannel(PARTICLECHANNELTARGETNODER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELTARGETNODEW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELTARGETPOINTR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELTARGETPOINTW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELFACEINDEXR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELFACEINDEXW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELCRUISESPEEDR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELCRUISESPEEDW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELCRUISEVARR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELCRUISEVARW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELTARGETTIMER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELTARGETTIMEW_INTERFACE); // write channel
//		mask.AddChannel(PARTICLECHANNELCONTROLTYPER_INTERFACE); // read channel
//		mask.AddChannel(PARTICLECHANNELCONTROLTYPEW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELDOCKINGSPEEDR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELDOCKINGSPEEDW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELDOCKINGVECTORR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELDOCKINGVECTORW_INTERFACE); // write channel
	}
	return mask;
}

int PFTestSpeedGoToTarget::IsColorCoordinated() const
{
	return pblock()->GetInt(kSpeedGoToTarget_colorType, 0);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpeedGoToTarget::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDGOTOTARGET_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpeedGoToTarget::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDGOTOTARGET_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpeedGoToTarget::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDGOTOTARGET_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestSpeedGoToTarget::HasDynamicName(TSTR& nameSuffix)
{
	int type = pblock()->GetInt(kSpeedGoToTarget_type, 0);
	switch(type) {
	case kSpeedGoToTarget_type_speed:
//		nameSuffix = GetString(IDS_ABSOLUTE_ABBR);
		nameSuffix = GetString(IDS_SPEED);
		break;
	case kSpeedGoToTarget_type_time:
		nameSuffix = GetString(IDS_TIME);
		break;
	case kSpeedGoToTarget_type_none:
		nameSuffix = GetString(IDS_NOCONTROL);
		break;
	}
	return true;
}

// returns world coordinates of the closest point on the surface of the icon
Point3 GetClosestIconPoint(float iconSize, const Matrix3& iconTM, const Point3& toPoint, Point3& localLocation)
{
	Matrix3 inverseTM = Inverse(iconTM);
	localLocation = 0.5f*iconSize*Normalize(toPoint*inverseTM);
	Point3 worldLocation = localLocation*iconTM;
	if ((iconTM.GetIdentFlags() & SCL_IDENT) == 0) { // non-uniform scaled sphere
		for(int i=0; i<10; i++) { // it's enough to have 10 iterations
			Point3 difVector = toPoint - worldLocation;
			if (DotProd(difVector, difVector) <= 0.0f) break; // point is on the surface
			Point3 tangent1;
			switch (MinComponent(localLocation)) {
			case 0: 
				tangent1 = Point3(0.0f, localLocation.z, -localLocation.y);
				break;
			case 1:
				tangent1 = Point3(localLocation.z, 0.0f, -localLocation.x);
				break;
			case 2:
				tangent1 = Point3(localLocation.y, -localLocation.x, 0.0f);
				break;
			}
			Point3 tangent2 = CrossProd(localLocation, tangent1);
			tangent1 = tangent1*iconTM;
			tangent2 = tangent2*iconTM;
			Point3 normal = Normalize(CrossProd(tangent1, tangent2));
			float normalDeviation = DotProd(normal, Normalize(difVector));
			if (normalDeviation > 0.9999f) break; // found extremum
			Point3 newToPoint = toPoint - normal*DotProd(difVector, normal);
			localLocation = 0.5f*iconSize*Normalize(newToPoint*inverseTM);
			worldLocation = localLocation*iconTM;
		}
	}

	return worldLocation;
}

// returns a normal (in world coordinates) at the closest point on the surface of the icon
Point3 GetClosestIconNormal(float iconSize, const Matrix3& iconTM, const Point3& toPoint)
{
	Matrix3 inverseTM = Inverse(iconTM);
	Point3 localNormal = Normalize(toPoint*inverseTM);
	Point3 normal = Normalize(iconTM.VectorTransform(localNormal));
	if ((iconTM.GetIdentFlags() & SCL_IDENT) == 0) { // non-uniform scaled sphere
		Point3 localLocation = 0.5f*iconSize*localNormal;
		Point3 worldLocation = localLocation*iconTM;
		for(int i=0; i<10; i++) { // it's enough to have 10 iterations
			Point3 tangent1;
			switch (MinComponent(localLocation)) {
			case 0: 
				tangent1 = Point3(0.0f, localLocation.z, -localLocation.y);
				break;
			case 1:
				tangent1 = Point3(localLocation.z, 0.0f, -localLocation.x);
				break;
			case 2:
				tangent1 = Point3(localLocation.y, -localLocation.x, 0.0f);
				break;
			}
			Point3 tangent2 = CrossProd(localLocation, tangent1);
			tangent1 = tangent1*iconTM;
			tangent2 = tangent2*iconTM;
			Point3 normal = Normalize(CrossProd(tangent1, tangent2));
			Point3 difVector = toPoint - worldLocation;
			if (DotProd(difVector, difVector) <= 0.0f) break;
			float normalDeviation = DotProd(normal, Normalize(difVector));
			if (normalDeviation > 0.9999f) break; // found extremum
			Point3 newToPoint = toPoint - normal*DotProd(difVector, normal);
			localLocation = 0.5f*iconSize*Normalize(newToPoint*Inverse(iconTM));
			worldLocation = localLocation*iconTM;
		}
	}
	return normal;
}

// calculate speed for "control by speed" without docking direction
Point3 CalcNewSpeedWithoutDocking(	const Point3& targetPoint,
									const Point3& currentPosition,
									const Point3& currentSpeed,
									float cruiseSpeed,
									float accelLimit, // per tick
									PreciseTimeValue currentTime,
									PreciseTimeValue timeEnd,
									bool& isFinished)
{
	isFinished = false;
	if (timeEnd <= currentTime) return currentSpeed; // no time to accel or decel
	if (accelLimit <= 0.0f) return currentSpeed; // nothing we can do
	// check for test condition
	Point3 distanceOffset = targetPoint - currentPosition;
	float timeDif = float(timeEnd - currentTime);
	Point3 newSpeed = Point3::Origin;
	if (LengthSquared(distanceOffset) <= 0.0f) { // the particle reached the target
		isFinished = true;
		// nothing to change except maybe the speed magnitude
		float currentSpeedMagnitude = Length(currentSpeed);
		float speedDif = currentSpeedMagnitude - cruiseSpeed;
		float accelToDo = speedDif/timeDif;
		if (currentSpeedMagnitude > 0.0f) {
			if (accelLimit >= fabs(accelToDo)) { // accel limit is not a problem
				newSpeed = cruiseSpeed*currentSpeed/currentSpeedMagnitude;
			} else {
				if (currentSpeedMagnitude > cruiseSpeed) 
					newSpeed = (1.0f - timeDif*accelLimit/currentSpeedMagnitude)*currentSpeed;
				else
					newSpeed = (1.0f + timeDif*accelLimit/currentSpeedMagnitude)*currentSpeed;
			}
		}
		return newSpeed;
	}
	Point3 bestDirection = Normalize(distanceOffset);
	Point3 steeringSpeed = bestDirection*DotProd(bestDirection, currentSpeed) - currentSpeed;
	float lenSq = LengthSquared(steeringSpeed/timeDif);
	float accelSq = accelLimit*accelLimit;
	if ((lenSq <= accelSq) && (DotProd(bestDirection, currentSpeed) >= 0.0f)) { // able to pull to the target direction
		Point3 steeredSpeed = currentSpeed + steeringSpeed;
		float steeredSpeedValue = Length(steeredSpeed);
		float margin = sqrt(accelSq - lenSq);
		float maxSteeredSpeedValue = steeredSpeedValue + margin*timeDif;
		float minSteeredSpeedValue = steeredSpeedValue - margin*timeDif;
		if (minSteeredSpeedValue < 0.0f) minSteeredSpeedValue = 0.0f;
		if (cruiseSpeed < minSteeredSpeedValue) {
			newSpeed = bestDirection*minSteeredSpeedValue;
		} else if (maxSteeredSpeedValue < cruiseSpeed) {
			newSpeed = bestDirection*maxSteeredSpeedValue;
		} else {
			newSpeed = bestDirection*cruiseSpeed;
		}
		// check if the particle is able to reach the target in the current integration step
		if (LengthSquared(distanceOffset) <= LengthSquared(newSpeed)*timeDif*timeDif)
			isFinished = true;
	} else { // not able yet to pull to the target direction
		Point3 bestSpeed = cruiseSpeed*bestDirection;
		steeringSpeed = bestSpeed - currentSpeed;
		if (LengthSquared(steeringSpeed) > double(accelLimit*accelLimit)*timeDif*timeDif)
			newSpeed = currentSpeed + accelLimit*timeDif*Normalize(steeringSpeed);
		else
			newSpeed = cruiseSpeed*bestDirection;
	}
	return newSpeed;
}

// calculate speed for "control by speed" with docking direction
Point3 CalcNewSpeedWithDocking(	const Point3& targetPoint,
									const Point3& currentPosition,
									const Point3& currentSpeed,
									float cruiseSpeed,
									float accelLimit, // per tick
									const Point3& dockingDirection, // normalized vector
									float dockingDistance,
									PreciseTimeValue currentTime,
									PreciseTimeValue timeEnd,
									bool& isFinished)
{
	if (timeEnd <= currentTime) return currentSpeed; // no time to accel or decel
	if (accelLimit <= 0.0f) return currentSpeed; // nothing we can do
	isFinished = false;
	float timeToTravel = float(timeEnd - currentTime);

	float turningRadiusCoef = 1.0f;
	// should be 0.5f but 1.0f to give a margin for an error

	Point3 newSpeed = Point3::Origin;

	Point3 aimPoint = targetPoint;
	Point3 worldOffset = targetPoint - currentPosition;
	float worldOffsetLenSq = LengthSquared(worldOffset);
	float distanceToTargetSq = worldOffsetLenSq; 
	Point3 predictedNextPosition = currentPosition + currentSpeed*float(timeEnd - currentTime);
	Point3 predictedWorldOffset = targetPoint - predictedNextPosition;
	float predictedWorldOffsetLenSq = LengthSquared(predictedWorldOffset);
//	if (worldOffsetLenSq > dockingDistance*dockingDistance) { // the dockingDistance target point may jerk particle way from the real target point
															  // therefore the decision to offset the target point by the docking distance
															  // should be done if particle won't reach the dockingDistance target point during next frame
	if (predictedWorldOffsetLenSq > dockingDistance*dockingDistance) {
		aimPoint -= dockingDistance*dockingDirection; // offset the target point by docking distance
		worldOffset = aimPoint - currentPosition;
		worldOffsetLenSq = LengthSquared(worldOffset);
	}

	if (worldOffsetLenSq <= 0.0f) { // the particle reached the target, the speed doesn't matter
		// the particle reached the target, then the aim point is set
		// to have the desired speed as cruise with docking direction
		isFinished = true;
		worldOffset = cruiseSpeed*timeToTravel*dockingDirection;
		worldOffsetLenSq = LengthSquared(worldOffset);
	}

	// check particle position for a half-space location: either in front of targetPoint or behind it
	bool inFront = (DotProd(worldOffset, dockingDirection) > 0.0f);

	// check if able to snap to the target point in the given time;
	// if it's possible than the direction doesn't matter
	float maxSpeed = Length(currentSpeed) + accelLimit*timeToTravel;
	float maxDistance = timeToTravel*maxSpeed;
	if (inFront && (maxDistance*maxDistance > worldOffsetLenSq)) { // need more checking
		Point3 bestDirection = Normalize(worldOffset);
		Point3 steeringSpeed = bestDirection*DotProd(bestDirection, currentSpeed) - currentSpeed;
		float lenSq = LengthSquared(steeringSpeed);
		float accelSq = accelLimit*accelLimit*timeToTravel*timeToTravel;
		if (lenSq <= accelSq) { // able to pull to the target direction
			Point3 steeredSpeed = currentSpeed + steeringSpeed;
			float steeredSpeedValue = Length(steeredSpeed);
			float margin = sqrt(accelSq - lenSq);
			float maxSteeredSpeedValue = steeredSpeedValue + margin*timeToTravel;
			float minSteeredSpeedValue = steeredSpeedValue - margin*timeToTravel;
			if (minSteeredSpeedValue < 0.0f) minSteeredSpeedValue = 0.0f;
			if (cruiseSpeed < minSteeredSpeedValue) {
				newSpeed = bestDirection*minSteeredSpeedValue;
			} else if (maxSteeredSpeedValue < cruiseSpeed) {
				newSpeed = bestDirection*maxSteeredSpeedValue;
			} else {
				newSpeed = bestDirection*cruiseSpeed;
			}
			// check if the particle is able to reach the target in the current integration step
			if (distanceToTargetSq <= LengthSquared(newSpeed)*timeToTravel*timeToTravel)
				isFinished = true;
			return newSpeed;
		}
	}

	Point3 normalOffset = worldOffset - dockingDirection*DotProd(worldOffset, dockingDirection);
	float normalOffsetLenSq = LengthSquared(normalOffset);
	Point3 steeringCenter = 0.5f*(targetPoint + currentPosition);
	float maxAccelToDo = accelLimit*timeToTravel;
	double maxAccelToDoSq = accelLimit*accelLimit*timeToTravel*timeToTravel;
	if (normalOffsetLenSq <= 0.0f) { // the particle location is on the line with targetPoint and dockingDirection
		// check current speed to construct a steering plane
		Point3 speedOffset = currentSpeed - dockingDirection*DotProd(currentSpeed, dockingDirection);
		if (LengthSquared(speedOffset) <= 0.0) { // particle moves in line with docking direciton (either inbound or outbound)
			if (inFront) { // adjust current speed towards cruiseSpeed
				Point3 bestSpeed = cruiseSpeed*dockingDirection;
				Point3 accelToDo = bestSpeed - currentSpeed;
				if (LengthSquared(accelToDo) < maxAccelToDoSq)
					newSpeed = bestSpeed;
				else
					newSpeed = currentSpeed + maxAccelToDo*Normalize(accelToDo);
			} else { // push particle up (luckily outward the docking direction)
				newSpeed = currentSpeed + maxAccelToDo*Point3::ZAxis;
			}
		} else { // particle will leave the docking direciton; if particle is inbound then move it toward the docking direction
				 // otherwise push it further out
			if (inFront) {
				newSpeed = currentSpeed + maxAccelToDo*Normalize(speedOffset);
			} else {
				newSpeed = currentSpeed - maxAccelToDo*Normalize(speedOffset);
			}
		}
		if (inFront)
			if (distanceToTargetSq <= LengthSquared(newSpeed)*timeToTravel*timeToTravel)
				isFinished = true;
	} else {
		float currentSpeedValue = Length(currentSpeed);
		float nextSpeedValue = currentSpeedValue;
		if (nextSpeedValue >= cruiseSpeed + maxAccelToDo) nextSpeedValue -= maxAccelToDo;
		else if (nextSpeedValue <= cruiseSpeed - maxAccelToDo) nextSpeedValue += maxAccelToDo;
		else nextSpeedValue = cruiseSpeed;
		float turningRadius = turningRadiusCoef*(nextSpeedValue*nextSpeedValue/accelLimit - accelLimit);
		if (turningRadius <= 0.0f) { // can turn on a dime; move directly to the target point
			Point3 bestSpeed = cruiseSpeed*dockingDirection;
			Point3 accelToDo = bestSpeed - currentSpeed;
			if (LengthSquared(accelToDo) < maxAccelToDoSq)
				newSpeed = bestSpeed;
			else
				newSpeed = currentSpeed + maxAccelToDo*Normalize(accelToDo);
			if (distanceToTargetSq <= LengthSquared(newSpeed)*timeToTravel*timeToTravel)
				isFinished = true;
		} else {
			Point3 bestSpeed = Point3::Origin;
			Point3 normalizedNormalOffset = Normalize(normalOffset);
			Point3 steeringCenter = aimPoint - turningRadius*normalizedNormalOffset;
			float leashLength = Length(currentPosition - steeringCenter);

			float accelRefactor = 1.0f;
			if (leashLength < 1.25f*turningRadius) {
				float oldTurningRadius = turningRadius;
				turningRadius = 0.5f*worldOffsetLenSq/Length(normalOffset);
				Point3 steeringCenter = aimPoint - turningRadius*normalizedNormalOffset;
				Point3 normLeash = Normalize(currentPosition - steeringCenter);
				bestSpeed = cruiseSpeed*( dockingDirection*DotProd(normLeash,normalizedNormalOffset) -
												normalizedNormalOffset*DotProd(normLeash, dockingDirection) );
				if (leashLength > 0.8f*oldTurningRadius) {
					Point3 nextPosition = currentPosition + bestSpeed*timeToTravel;
					nextPosition = steeringCenter + turningRadius*Normalize(nextPosition - steeringCenter);
					bestSpeed = (nextPosition - currentPosition)/timeToTravel;
				
					float timeToTarget = Length(worldOffset)/Length(bestSpeed) - timeToTravel;
					if (timeToTarget < TIME_TICKSPERSEC/3 ) {
						accelRefactor = 4.0f*(1.0f - timeToTarget/(TIME_TICKSPERSEC/3)) + 1.0f;
					}
				}
			} else {
				Point3 vectorFromCenter = currentPosition - steeringCenter;
				Point3 normVectorFromCenter = Normalize(vectorFromCenter);
				Point3 upSpeed = dockingDirection*DotProd(normVectorFromCenter,normalizedNormalOffset) -
							normalizedNormalOffset*DotProd(normVectorFromCenter, dockingDirection);
				float coef1 = turningRadius/Length(vectorFromCenter);
				float coef2 = 1 - coef1*coef1;
				coef2 = (coef2 <= 0.0f) ? 0.0f : sqrt(coef2);
				bestSpeed = cruiseSpeed*(coef1*upSpeed - coef2*normVectorFromCenter);
			}

			Point3 accelToDo = bestSpeed - currentSpeed;
			if (LengthSquared(accelToDo) < maxAccelToDoSq*accelRefactor*accelRefactor)
				newSpeed = bestSpeed;
			else
				newSpeed = currentSpeed + maxAccelToDo*accelRefactor*Normalize(accelToDo);
		}
	}

	return newSpeed;
} 

// calculate speed for "control by time" with docking direction
Point3 CalcNewSpeedWithoutDocking(	const Point3& targetPoint,
									const Point3& currentPosition,
									const Point3& currentSpeed,
									const PreciseTimeValue& targetTime,
									PreciseTimeValue currentTime,
									PreciseTimeValue timeEnd)
{
	float timeToTravel = float(timeEnd - currentTime);
	if (timeToTravel <= 0.0f) return currentSpeed; // no time to do anything
	if (targetTime <= currentTime) return currentSpeed; // to late to do anything

	Point3 newSpeed = Point3::Origin;

	// check if we need to snap the particle in the current frame
	if (targetTime <= timeEnd) {
		newSpeed = (targetPoint - currentPosition)/float(targetTime - currentTime);
	} else { // correct particle speed to aim at target
		float timeToTarget = float(targetTime - currentTime);
		Point3 accel = (targetPoint - currentPosition - currentSpeed*timeToTarget)/(timeToTarget*timeToTarget);
		newSpeed = currentSpeed + accel*timeToTravel;
	}

	return newSpeed;
}

// calculate speed for "control by time" with docking direction
Point3 CalcNewSpeedWithDockingValue(	const Point3& targetPoint,
										const Point3& currentPosition,
										const Point3& currentSpeed,
										const PreciseTimeValue& targetTime,
										float dockingValue,
										PreciseTimeValue currentTime,
										PreciseTimeValue timeEnd)
{
	float timeToTravel = float(timeEnd - currentTime);
	if (timeToTravel <= 0.0f) return currentSpeed; // no time to do anything
	if (targetTime <= currentTime) return currentSpeed; // too late to do anything

	Point3 newSpeed = currentSpeed;

	// check if we need to snap the particle in the current frame
	if (targetTime <= timeEnd) {
		newSpeed = (targetPoint - currentPosition)/float(targetTime - currentTime);
	} else {
		// correct particle speed to aim at target
		float timeToTarget = float(targetTime - currentTime);
		Point3 targetOffset = targetPoint - currentPosition;
		float distanceToTarget = Length(targetOffset);
		if (distanceToTarget > 0.0f) {
			Point3 dockingSpeed = dockingValue*targetOffset/distanceToTarget;
			Point3 a1 = 3.0f*(targetPoint - currentPosition) - timeToTarget*(2.0f*currentSpeed + dockingSpeed);
			Point3 a2 = 2.0f*(currentPosition - targetPoint) + timeToTarget*(dockingSpeed + currentSpeed);
			newSpeed = currentSpeed + (a1 + a2*timeToTravel/timeToTarget)*timeToTravel/(timeToTarget*timeToTarget);
		}
	}

	return newSpeed;
}

// calculate speed for "control by speed" with docking direction
Point3 CalcNewSpeedWithDockingValueAndVector(	const Point3& targetPoint,
												const Point3& currentPosition,
												const Point3& currentSpeed,
												const PreciseTimeValue& targetTime,
												float dockingValue,
												const Point3& dockingVector,
												PreciseTimeValue currentTime,
												PreciseTimeValue timeEnd)
{
	float timeToTravel = float(timeEnd - currentTime);
	if (timeToTravel <= 0.0f) return currentSpeed; // no time to do anything
	if (targetTime <= currentTime) return currentSpeed; // to late to do anything

	Point3 newSpeed = currentSpeed;

	// check if we need to snap the particle in the current frame
	if (targetTime <= timeEnd) {
		newSpeed = (targetPoint - currentPosition)/float(targetTime - currentTime);
	} else {
		// correct particle speed to aim at target
		float timeToTarget = float(targetTime - currentTime);
		Point3 targetOffset = targetPoint - currentPosition;
		Point3 dockingSpeed = dockingValue*dockingVector;
		Point3 a1 = 3.0f*targetOffset - timeToTarget*(2.0f*currentSpeed + dockingSpeed);
		Point3 a2 = -2.0f*targetOffset + timeToTarget*(dockingSpeed + currentSpeed);
		newSpeed = currentSpeed + (a1 + a2*timeToTravel/timeToTarget)*timeToTravel/(timeToTarget*timeToTarget);
	}

	return newSpeed;
}

// calculate time when a particle meets the test requirement
bool CalcTestCondition( const Point3& targetPoint,
						const Point3& currentPosition,
						const Point3& currentSpeed,
						float testDistance,
						PreciseTimeValue currentTime,
						PreciseTimeValue timeEnd,
						float& timeToSatisfyTest)
{
	// check if a particle is already in the test distance proximity
	float currentLenSq = LengthSquared(targetPoint - currentPosition);
	if ((currentLenSq < testDistance*testDistance) || (currentLenSq == 0.0f)) {
		timeToSatisfyTest = 0.0f;
		return true;
	}
	
	float timeToTravel = float(timeEnd - currentTime);
	// check if the target is out of reach if given the time in the current interval
	float testDistanceSq = testDistance*testDistance;
	if (currentLenSq > 2.0f*(LengthSquared(currentSpeed)*timeToTravel*timeToTravel + testDistanceSq))
		// the sufficient condition is satisfied: particle is guaranteed to not reach target in time
		return false;

	// not calculate the exact time when a particle can reach the test distance sphere
	float a = LengthSquared(currentSpeed);
	if (a <= 0.0f) return false; // particle is static and cannot reach the target at all
	Point3 worldOffset = currentPosition - targetPoint;
	float b = DotProd(currentSpeed, worldOffset);
	float c = LengthSquared(worldOffset) - testDistanceSq;
	float det = b*b - a*c;
	if (det < 0.0f) return false; // particle will never reach the target sphere since the course is outside the target
	timeToSatisfyTest = (-b - sqrt(det))/a;
	if (timeToSatisfyTest < 0.0f) return false; // the test condition was in the past
	if (timeToSatisfyTest > timeToTravel) return false; // maybe next frame but not now
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSpeedGoToTarget::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	if (pblock() == NULL) return false;
	if (actionNode == NULL) return false;
	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count;
	count = chAmount->Count();
	if (count == 0) return true; // no particles to modify
	IParticleChannelPTVR* chTimeR = GetParticleChannelTimeRInterface(pCont);
	IParticleChannelPTVW* chTimeW = GetParticleChannelTimeWInterface(pCont);
	if ((chTimeR == NULL) || (chTimeW == NULL)) return false; // can't read/modify timing info for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for speedGoToTarget calculation
	IParticleChannelPoint3R* chPosR = GetParticleChannelPositionRInterface(pCont);
	IParticleChannelPoint3W* chPosW = GetParticleChannelPositionWInterface(pCont);
	if ((chPosR == NULL) || (chPosW == NULL)) return false; // can't read/modify current particle position

	IParticleChannelPoint3R* chSpeedR = NULL;
	IParticleChannelPoint3W* chSpeedW = NULL;
	bool initSpeed = false;
	chSpeedR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSPEEDR_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSPEEDR_INTERFACE,
																PARTICLECHANNELSPEEDW_INTERFACE, true,
																actionNode, NULL, &initSpeed);
	chSpeedW = GetParticleChannelSpeedWInterface(pCont);
	if ((chSpeedR == NULL) || (chSpeedW == NULL)) return false; // can't read/modify speed

	// acquire static parameters
	int controlType = pblock()->GetInt(kSpeedGoToTarget_type, timeStart);
	bool useCruiseSpeed = (pblock()->GetInt(kSpeedGoToTarget_useCruiseSpeed, timeStart) != 0);
	int syncType = pblock()->GetInt(kSpeedGoToTarget_syncBy, timeStart);
	int targetSync = pblock()->GetInt(kSpeedGoToTarget_targetSync, timeStart);
	int timingType = pblock()->GetInt(kSpeedGoToTarget_timingType, timeStart);
	bool useDockingSpeed = (pblock()->GetInt(kSpeedGoToTarget_useDockingSpeed, timeStart) != 0);
	bool useBirthTime = ((syncType == kSpeedGoToTarget_syncBy_age) 
					|| (timingType == kSpeedGoToTarget_timingType_age) 
					|| (targetSync == kSpeedGoToTarget_syncBy_age));
	bool useEventStart = ((syncType == kSpeedGoToTarget_syncBy_event) 
					|| (timingType == kSpeedGoToTarget_timingType_event)
					|| (targetSync == kSpeedGoToTarget_syncBy_event));
	int targetType = pblock()->GetInt(kSpeedGoToTarget_targetType, timeStart);
	// doesn't give the real amount of target objects due to possible presence of hierarchical targets [bayboro 04-04-2003]
	//int i, targetNum = 1;
	//if (targetType == kSpeedGoToTarget_targetType_objects) {
	//	for(i=0, targetNum=0; i<pblock()->Count(kSpeedGoToTarget_targets); i++)
	//		if (pblock()->GetINode(kSpeedGoToTarget_targets, timeStart, i) != NULL)
	//			targetNum++;
	//}
	PFOperatorSpeedGoToTargetLocalData* localData = &(_localData(pCont));
	int i, targetNum = 1;
	if (targetType == kSpeedGoToTarget_targetType_objects) targetNum = localData->NodeGeometryCount();

	bool animatedShape = (targetType == kSpeedGoToTarget_targetType_objects) ? 
						( pblock()->GetInt(kSpeedGoToTarget_animated, timeStart) != 0) : false;
	bool followTarget = ( pblock()->GetInt(kSpeedGoToTarget_follow, timeStart) != 0);
	bool lockOnTarget = ( pblock()->GetInt(kSpeedGoToTarget_lock, timeStart) != 0);
	bool lockTarget = ( pblock()->GetInt(kSpeedGoToTarget_lock, timeStart ) != 0);
	int pointType = pblock()->GetInt(kSpeedGoToTarget_pointType, timeStart );
	int useMXSVector = (pointType == kSpeedGoToTarget_pointType_vector);
	int assignmentType = pblock()->GetInt(kSpeedGoToTarget_assignment, timeStart);
	bool useMXSInteger = ((targetNum > 1) && (assignmentType == kSpeedGoToTarget_assignment_integer));
	int dockingType = pblock()->GetInt(kSpeedGoToTarget_dockingType, timeStart);
	int useDockingVector = ((dockingType != kSpeedGoToTarget_dockingType_none) && !followTarget);
	float iconSize = pblock()->GetFloat( kSpeedGoToTarget_iconSize, timeStart );
	bool iconSizeAnimated = false;
	Control* iconSizeCtrl = pblock()->GetController(kSpeedGoToTarget_iconSize);
	if (iconSizeCtrl != NULL)
		iconSizeAnimated = (iconSizeCtrl->IsAnimated() != 0);
	bool easeInAnimated = false;
	Control* easeInCtrl = pblock()->GetController(kSpeedGoToTarget_easeIn);
	if (easeInCtrl != NULL)
		easeInAnimated = (easeInCtrl->IsAnimated() != 0);
	float easeIn = GetPFFloat( pblock(), kSpeedGoToTarget_easeIn, timeStart);
	bool useEaseIn = easeInAnimated ? true : (easeIn != 0.0f);
	int testType = pblock()->GetInt(kSpeedGoToTarget_testType, timeStart);
	bool dockingDistanceAnimated = false;
	Control* dockingDistanceCtrl = pblock()->GetController(kSpeedGoToTarget_dockingDistance);
	if (dockingDistanceCtrl != NULL)
		dockingDistanceAnimated = (dockingDistanceCtrl->IsAnimated() != 0);
	float dockingDistance = GetPFFloat( pblock(), kSpeedGoToTarget_dockingDistance, timeStart);

	// acquire more particle channels according to parameter conditions
	IParticleChannelPTVR* chBirthTime = NULL;
	if (useBirthTime)
	{
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirthTime == NULL) return false; // can't read particle age
	}

	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (useEventStart) {
		chEventStartR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		ParticleChannelPTV_Class_ID,
																		true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																		PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																		actionNode, NULL, &initEventStart);
		if (chEventStartR == NULL) return false; // can't read event start time
		if (initEventStart) {
			chEventStartW = GetParticleChannelEventStartWInterface(pCont);
			if (chEventStartW == NULL) return false; // can't write event start time
		}
	}

	bool initInited = false;
	IParticleChannelBoolR* chInitedR = (IParticleChannelBoolR*)chCont->EnsureInterface(PARTICLECHANNELGOTOINITR_INTERFACE,
																		ParticleChannelBool_Class_ID,
																		true, PARTICLECHANNELGOTOINITR_INTERFACE,
																		PARTICLECHANNELGOTOINITW_INTERFACE, false,
																		actionNode, (Object*)this, &initInited);
	IParticleChannelBoolW* chInitedW = (IParticleChannelBoolW*)chCont->GetPrivateInterface(PARTICLECHANNELGOTOINITW_INTERFACE, (Object*)this);
	if ((chInitedR == NULL) || (chInitedW == NULL)) return false;

	bool initFinished = false;
	IParticleChannelBoolR* chFinishedR = (IParticleChannelBoolR*)chCont->EnsureInterface(PARTICLECHANNELFINISHEDR_INTERFACE,
																		ParticleChannelBool_Class_ID,
																		true, PARTICLECHANNELFINISHEDR_INTERFACE,
																		PARTICLECHANNELFINISHEDW_INTERFACE, false,
																		actionNode, (Object*)this, &initFinished);
	IParticleChannelBoolW* chFinishedW = (IParticleChannelBoolW*)chCont->GetPrivateInterface(PARTICLECHANNELFINISHEDW_INTERFACE, (Object*)this);
	if ((chFinishedR == NULL) || (chFinishedW == NULL)) return false;

	IParticleChannelINodeR* chTargetNodeR = NULL;
	IParticleChannelINodeW* chTargetNodeW = NULL;
	bool initTargetNode = false;
	if (targetNum > 1) {
		chTargetNodeR = (IParticleChannelINodeR*)chCont->EnsureInterface(PARTICLECHANNELTARGETNODER_INTERFACE,
																		ParticleChannelINode_Class_ID,
																		true, PARTICLECHANNELTARGETNODER_INTERFACE,
																		PARTICLECHANNELTARGETNODEW_INTERFACE, false,
																		actionNode, (Object*)this, &initTargetNode);
		chTargetNodeW = (IParticleChannelINodeW*)chCont->GetPrivateInterface(PARTICLECHANNELTARGETNODEW_INTERFACE, (Object*)this);
		if ((chTargetNodeR == NULL) || (chTargetNodeW == NULL)) return false; // can't read/modify target node channel
	}

	bool initTargetPoint = false;
	IParticleChannelPoint3R* chTargetPointR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELTARGETPOINTR_INTERFACE,
																		ParticleChannelPoint3_Class_ID,
																		true, PARTICLECHANNELTARGETPOINTR_INTERFACE,
																		PARTICLECHANNELTARGETPOINTW_INTERFACE, false,
																		actionNode, (Object*)this, &initTargetNode);
	IParticleChannelPoint3W* chTargetPointW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELTARGETPOINTW_INTERFACE, (Object*)this);
	if ((chTargetPointR == NULL) || (chTargetPointW == NULL)) return false;
	
	IParticleChannelIntR* chFaceIndexR = NULL;
	IParticleChannelIntW* chFaceIndexW = NULL;
	bool initFaceIndex = false;
	if ((targetType == kSpeedGoToTarget_targetType_objects)	&& animatedShape && followTarget) {
		chFaceIndexR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELFACEINDEXR_INTERFACE,
																		ParticleChannelInt_Class_ID,
																		true, PARTICLECHANNELFACEINDEXR_INTERFACE,
																		PARTICLECHANNELFACEINDEXW_INTERFACE, false,
																		actionNode, (Object*)this, &initFaceIndex);
		chFaceIndexW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELFACEINDEXW_INTERFACE, (Object*)this);
		if ((chFaceIndexR == NULL) || (chFaceIndexW == NULL)) return false; // can't read/modify face index channel
	}

	IParticleChannelFloatR* chCruiseSpeedR = NULL;
	IParticleChannelFloatW* chCruiseSpeedW = NULL;
	IParticleChannelFloatR* chCruiseVarR = NULL;
	IParticleChannelFloatW* chCruiseVarW = NULL;
	bool initCruiseSpeed = false;
	if (controlType == kSpeedGoToTarget_type_speed) {
		if (useCruiseSpeed) {
			chCruiseVarR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELCRUISEVARR_INTERFACE,
																		ParticleChannelFloat_Class_ID,
																		true, PARTICLECHANNELCRUISEVARR_INTERFACE,
																		PARTICLECHANNELCRUISEVARW_INTERFACE, false,
																		actionNode, (Object*)this, &initCruiseSpeed);
			chCruiseVarW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELCRUISEVARW_INTERFACE, (Object*)this);
			if ((chCruiseVarR == NULL) || (chCruiseVarW == NULL)) return false; // can't read/modify cruise variation channel
		} else {
			chCruiseSpeedR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELCRUISESPEEDR_INTERFACE,
																		ParticleChannelFloat_Class_ID,
																		true, PARTICLECHANNELCRUISESPEEDR_INTERFACE,
																		PARTICLECHANNELCRUISESPEEDW_INTERFACE, false,
																		actionNode, (Object*)this, &initCruiseSpeed);
			chCruiseSpeedW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELCRUISESPEEDW_INTERFACE, (Object*)this);
			if ((chCruiseSpeedR == NULL) || (chCruiseSpeedW == NULL)) return false; // can't read/modify cruise speed channel
		}
	}

	if (chCruiseSpeedW == NULL) {
		if ((controlType == kSpeedGoToTarget_type_time) && useDockingVector && !useDockingSpeed) {
			chCruiseSpeedR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELCRUISESPEEDR_INTERFACE,
																		ParticleChannelFloat_Class_ID,
																		true, PARTICLECHANNELCRUISESPEEDR_INTERFACE,
																		PARTICLECHANNELCRUISESPEEDW_INTERFACE, false,
																		actionNode, (Object*)this, &initCruiseSpeed);
			chCruiseSpeedW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELCRUISESPEEDW_INTERFACE, (Object*)this);
			if ((chCruiseSpeedR == NULL) || (chCruiseSpeedW == NULL)) return false; // can't read/modify cruise speed channel
		}
	}

	IParticleChannelPTVR* chTargetTimeR = NULL;
	IParticleChannelPTVW* chTargetTimeW = NULL;
	bool initTargetTime = false;
	if (controlType == kSpeedGoToTarget_type_time) {
		chTargetTimeR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELTARGETTIMER_INTERFACE,
																		ParticleChannelPTV_Class_ID,
																		true, PARTICLECHANNELTARGETTIMER_INTERFACE,
																		PARTICLECHANNELTARGETTIMEW_INTERFACE, false,
																		actionNode, (Object*)this, &initTargetTime);
		chTargetTimeW = (IParticleChannelPTVW*)chCont->GetPrivateInterface(PARTICLECHANNELTARGETTIMEW_INTERFACE, (Object*)this);
		if ((chTargetTimeR == NULL) || (chTargetTimeW == NULL)) return false; // can't read/modify target time channel
	}	

	IParticleChannelIntR* chControlTypeR = NULL;
	IParticleChannelIntW* chControlTypeW = NULL;
/*	if (controlType == kSpeedGoToTarget_type_speedThenTime) {
		chControlTypeR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELCONTROLTYPER_INTERFACE,
																		ParticleChannelInt_Class_ID,
																		true, PARTICLECHANNELCONTROLTYPER_INTERFACE,
																		PARTICLECHANNELCONTROLTYPEW_INTERFACE, true,
																		actionNode, (Object*)this);
		chControlTypeW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELCONTROLTYPEW_INTERFACE, (Object*)this);
		if ((chControlTypeR == NULL) || (chControlTypeW == NULL)) return false; // can't read/modify face index channel
	}
*/
	IParticleChannelFloatR* chDockingSpeedR = NULL;
	IParticleChannelFloatW* chDockingSpeedW = NULL;
	bool initDockingSpeed = false;
	if ((controlType == kSpeedGoToTarget_type_time) && useDockingSpeed) {
		chDockingSpeedR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELDOCKINGSPEEDR_INTERFACE,
																		ParticleChannelFloat_Class_ID,
																		true, PARTICLECHANNELDOCKINGSPEEDR_INTERFACE,
																		PARTICLECHANNELDOCKINGSPEEDW_INTERFACE, false,
																		actionNode, (Object*)this, &initDockingSpeed);
		chDockingSpeedW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELDOCKINGSPEEDW_INTERFACE, (Object*)this);
		if ((chDockingSpeedR == NULL) || (chDockingSpeedW == NULL)) return false; // can't read/modify cruise speed channel
	}

	IParticleChannelPoint3R* chDockingVectorR = NULL;
	IParticleChannelPoint3W* chDockingVectorW = NULL;
	bool initDockingVector = false;
	if (useDockingVector) {
		chDockingVectorR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELDOCKINGVECTORR_INTERFACE,
																		ParticleChannelPoint3_Class_ID,
																		true, PARTICLECHANNELDOCKINGVECTORR_INTERFACE,
																		PARTICLECHANNELDOCKINGVECTORW_INTERFACE, false,
																		actionNode, (Object*)this, &initDockingVector);
		chDockingVectorW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELDOCKINGVECTORW_INTERFACE, (Object*)this);
		if ((chDockingVectorR == NULL) || (chDockingVectorW == NULL)) return false;
	}

	IParticleChannelFloatR* chInitDistanceR = NULL;
	IParticleChannelFloatW* chInitDistanceW = NULL;
	bool initInitDistance = false;
	if (useEaseIn) {
		chInitDistanceR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELINITDISTANCER_INTERFACE,
																		ParticleChannelFloat_Class_ID,
																		true, PARTICLECHANNELINITDISTANCER_INTERFACE,
																		PARTICLECHANNELINITDISTANCEW_INTERFACE, false,
																		actionNode, (Object*)this, &initInitDistance);
		chInitDistanceW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELINITDISTANCEW_INTERFACE, (Object*)this);
		if ((chInitDistanceR == NULL) || (chInitDistanceW == NULL)) return false; // can't read/modify initDistance channel
	}

	IParticleChannelPoint3R* chMXSVector = NULL;
	if (useMXSVector)
		chMXSVector = GetParticleChannelMXSVectorRInterface(pCont);

	IParticleChannelIntR* chMXSInteger = NULL;
	if (useMXSInteger)
		chMXSInteger = GetParticleChannelMXSIntegerRInterface(pCont);
	
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	if (randGen == NULL) return false;

	PreciseTimeValue time, syncTime, targetTime;
	TimeValue timeValue, timeVariation;
	int subframeSampling = pblock()->GetInt(kSpeedGoToTarget_subframe, timeStart);
	float testDistance = GetPFFloat(pblock(), kSpeedGoToTarget_distance, timeStart);
	PreciseTimeValue curTimeValue;
	int tpf = GetTicksPerFrame();
	PFNodeGeometry* nodeGeom = NULL;
	Point3 worldLocation, objectLocation, localLocation;
	int currentFaceIndex;
	Matrix3 targetTM;
	targetTM.IdentityMatrix();
	Point3 dockingVector = Point3::XAxis, currentPosition, currentSpeed;
	float cruiseSpeed, accelLimit, dockingSpeed;
	Point3 newSpeed;
	PreciseTimeValue finishTime;

	float timeToSatisfyTest;
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	BitArray particlesToAdvance;
	particlesToAdvance.SetSize(count);
	particlesToAdvance.ClearAll();

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	for(i=0; i<count; i++) {

		if (chNew->IsNew(i)) {
			if (initEventStart)
				chEventStartW->SetValue(i, chTimeR->GetValue(i));
			if (initInited)
				chInitedW->SetValue(i, false);
		}

		time = chTimeR->GetValue(i);
		switch(syncType) {
		case kSpeedGoToTarget_syncBy_time:
			syncTime = time;
			break;
		case kSpeedGoToTarget_syncBy_age:
			syncTime = time - chBirthTime->GetValue(i);
			break;
		case kSpeedGoToTarget_syncBy_event:
			syncTime = time - chEventStartR->GetValue(i);
			break;
		}
		switch(targetSync) {
		case kSpeedGoToTarget_syncBy_time:
			targetTime = time;
			break;
		case kSpeedGoToTarget_syncBy_age:
			targetTime = time - chBirthTime->GetValue(i);
			break;
		case kSpeedGoToTarget_syncBy_event:
			targetTime = time - chEventStartR->GetValue(i);
			break;
		}

		currentPosition = chPosR->GetValue(i);
		currentSpeed = chSpeedR->GetValue(i);
		currentFaceIndex = -1;

		if (chInitedR->GetValue(i) == false) { // particle needs initialization
			// each new particle is not finished yet
			chFinishedW->SetValue(i, false); 

			// if CruiseSpeed channel is not NULL then the particle is off Speed control type
			// and useCruiseSpeed is OFF. It means that the cruise speed is the speed of the particle
			// when it enters the event
			if (chCruiseSpeedW != NULL) {
				chCruiseSpeedW->SetValue(i, Length(chSpeedR->GetValue(i)));
			}
			// if CruiseVar channel is not NULL then the particle is off Speed control type
			// and useCruiseSpeed is ON. It means that the cruise speed is calculated for each frame and
			// we need to initialize random component for variation
			if (chCruiseVarW != NULL)
				chCruiseVarW->SetValue(i, randGen->Rand01());
			else randGen->Rand01(); // to keep random gen balance
			// if TargetTime channel is present then particle is off Time control type
			// and we need to initialize time when a particle reaches its target
			if (chTargetTimeW != NULL) {
				timeValue = pblock()->GetTimeValue(kSpeedGoToTarget_timeValue, time);
				timeVariation = pblock()->GetTimeValue(kSpeedGoToTarget_timeVariation, time);
				curTimeValue = PreciseTimeValue( timeValue );
				if (timeVariation != 0)
					curTimeValue += PreciseTimeValue( randGen->Rand11()*timeVariation );
				else
					randGen->Rand11();
				if (timingType == kSpeedGoToTarget_timingType_age)
					curTimeValue += chBirthTime->GetValue(i);
				else if (timingType == kSpeedGoToTarget_timingType_event)
					curTimeValue += chEventStartR->GetValue(i);
				if (!subframeSampling)
					curTimeValue = PreciseTimeValue(int(floor(TimeValue(curTimeValue)/float(tpf) + 0.5f) * tpf));
				chTargetTimeW->SetValue(i, curTimeValue);
			} else randGen->Rand11();
			// if DockingSpeed channel is present then particle is off Time control type
			// and we need to initialize docking speed for a particle
			if (chDockingSpeedW != NULL) {
				float curSpeed = GetPFFloat(pblock(), kSpeedGoToTarget_dockingSpeed, time);
				curSpeed += randGen->Rand11()*GetPFFloat(pblock(), kSpeedGoToTarget_dockingSpeedVariation, time);
				if (curSpeed <= 0.0f) curSpeed = 0.0f;
				curSpeed /= TIME_TICKSPERSEC;
				chDockingSpeedW->SetValue(i, curSpeed);
			} else randGen->Rand11();
			// if TargetNode channel is present that a particle has to choose a target node
			bool locationIsCalculated = false;
			nodeGeom = NULL;
			if (chTargetNodeW != NULL) {
				switch (assignmentType) {
				case kSpeedGoToTarget_assignment_random:
					nodeGeom = localData->GetNodeGeometry(targetTime, randGen->Rand01());										
					break;
				case kSpeedGoToTarget_assignment_distance:
					nodeGeom = localData->GetClosestNodePivot(targetTime, currentPosition);
					break;
				case kSpeedGoToTarget_assignment_surface:
					nodeGeom = localData->GetClosestNodeSurface(targetTime, currentPosition, worldLocation, localLocation, currentFaceIndex);					
					locationIsCalculated = true;
					break;
				case kSpeedGoToTarget_assignment_deviation:
					nodeGeom = localData->GetLeastDeviatedNode(targetTime, currentPosition, currentSpeed);
					break;
				case kSpeedGoToTarget_assignment_integer:
					nodeGeom = localData->GetNodeGeometry(0); // fall-back plan
					if (chMXSInteger != NULL) {
						int index = chMXSInteger->GetValue(i);
						if ((index >= 0) && (index < localData->NodeGeometryCount()))
							nodeGeom = localData->GetNodeGeometry(index);
					} else {
						goto invalidTarget;
					}
					break;
				}
				chTargetNodeW->SetValue(i, (nodeGeom != NULL) ? nodeGeom->GetNode() : NULL);
			} else {
				if (targetType == kSpeedGoToTarget_targetType_objects)
					nodeGeom = localData->GetNodeGeometry(0);
			}

			if ((targetType ==  kSpeedGoToTarget_targetType_objects) && (nodeGeom == NULL))
				goto invalidTarget;

			if (nodeGeom != NULL) {
				if (nodeGeom->GetTM(time) != NULL)
					targetTM = *(nodeGeom->GetTM(targetTime));
				switch (pointType) {
				case kSpeedGoToTarget_pointType_random:
					if (!nodeGeom->GetLocationPoint(targetTime, worldLocation, localLocation, currentFaceIndex, true))
						goto invalidTarget;
					objectLocation = worldLocation*Inverse(targetTM);
					break;
				case kSpeedGoToTarget_pointType_surface:
					if (!locationIsCalculated)
						if (!nodeGeom->GetClosestPoint(targetTime, currentPosition, worldLocation, localLocation, currentFaceIndex))
							goto invalidTarget;
					objectLocation = worldLocation*Inverse(targetTM);
					break;
				case kSpeedGoToTarget_pointType_vector:
					if (chMXSVector != NULL) {
						currentFaceIndex = 0;
						localLocation = chMXSVector->GetValue(i);
						worldLocation = localLocation*targetTM;
					} else {
						goto invalidTarget;
					}
					objectLocation = localLocation;
					break;
				}
			} else { // set by the test icon
				targetTM = actionNode->GetObjTMAfterWSM(targetTime);
				if (iconSizeAnimated)
					iconSize = GetPFFloat(pblock(), kSpeedGoToTarget_iconSize, targetTime.TimeValue());
				switch (pointType) {
				case kSpeedGoToTarget_pointType_random:
					localLocation = 0.5f*RandSphereSurface(randGen);
					worldLocation = iconSize*localLocation*targetTM;
					break;
				case kSpeedGoToTarget_pointType_surface:
					worldLocation = GetClosestIconPoint(iconSize, targetTM, currentPosition, localLocation);
					break;
				case kSpeedGoToTarget_pointType_vector:
					if (chMXSVector != NULL) {
						currentFaceIndex = 0;
						worldLocation = localLocation = chMXSVector->GetValue(i);
					} else {
						goto invalidTarget;
					}
					break;
				}
				objectLocation = localLocation;
			}

			// set TargetPoint channel value
			if (followTarget) {
				if (animatedShape && (targetType == kSpeedGoToTarget_targetType_objects)) {
					chTargetPointW->SetValue(i, localLocation);
					chFaceIndexW->SetValue(i, currentFaceIndex);
				} else {
					if ((nodeGeom == NULL) && (iconSize != 0.0f)) objectLocation *= 1.0f/iconSize;
					chTargetPointW->SetValue(i, objectLocation);
				}
			} else {
				chTargetPointW->SetValue(i, worldLocation);
			}

			// set docking vector
			if (useDockingVector) {
				switch ( dockingType ) {
				case kSpeedGoToTarget_dockingType_parallel:
					dockingVector = -1.0f*(actionNode->GetObjTMAfterWSM(targetTime)).GetRow(2);
					break;
				case kSpeedGoToTarget_dockingType_spherical:
					dockingVector = targetTM.GetTrans() - worldLocation;
					break;
				case kSpeedGoToTarget_dockingType_cylindrical: {
					dockingVector = targetTM.GetTrans() - worldLocation;
					Point3 zAxis = Normalize(targetTM.GetRow(2));
					dockingVector -= zAxis*DotProd(dockingVector, zAxis); }
					break;
				case kSpeedGoToTarget_dockingType_normal:
					if (nodeGeom == NULL) { // use icon for the normal calculations
						dockingVector = GetClosestIconNormal(iconSize, 
															actionNode->GetObjTMAfterWSM(targetTime), 
															worldLocation);
					} else {
						Mesh* mesh = nodeGeom->GetMesh(time);
						if (mesh == NULL) goto invalidTarget;
						if (currentFaceIndex >= 0) {
							dockingVector = targetTM.VectorTransform(mesh->FaceNormal(currentFaceIndex));
						} else {
							Point3 tempWL, tempLL;
							if (nodeGeom->GetClosestPoint(targetTime, worldLocation, tempWL, tempLL, currentFaceIndex))
								goto invalidTarget;
							if (currentFaceIndex >= 0)
								dockingVector = targetTM.VectorTransform(mesh->FaceNormal(currentFaceIndex));
							else
								dockingVector = -1.0f*Point3::ZAxis;
						}
					}
					break;
				default: DbgAssert(0);
				}
				chDockingVectorW->SetValue(i, Normalize(dockingVector));
			}

			// if uses Ease In then we need distance from a particle to its goal
			// at the initial moment. The ratio currentDistance/InitialDistance is
			// used to calculate slow down speed value for ease in effect
			if (initInitDistance) {
				float initialProximity = Length(worldLocation - currentPosition);
				chInitDistanceW->SetValue(i, initialProximity);
			}

			chInitedW->SetValue(i, true);
		} else {
			// commented because a particle needs to know its target to verify test condition
//			if (chFinishedR->GetValue(i) == true) {
//				continue; // don't control finished particles
//			}

			INode* targetNode = NULL;
			bool targetSwitch = false;
			bool locationIsCalculated = false;
			nodeGeom = NULL;
			if (chTargetNodeR != NULL) {
				INode* targetNode = chTargetNodeR->GetValue(i);
				if (lockOnTarget) {
					nodeGeom = localData->GetNodeGeometry(targetNode);
				} else {
					switch (assignmentType) {
					case kSpeedGoToTarget_assignment_random:
						nodeGeom = localData->GetNodeGeometry(targetNode);
						break;
					case kSpeedGoToTarget_assignment_distance:
						nodeGeom = localData->GetClosestNodePivot(targetTime, currentPosition);
						break;
					case kSpeedGoToTarget_assignment_surface:
						nodeGeom = localData->GetClosestNodeSurface(targetTime, currentPosition, worldLocation, localLocation, currentFaceIndex);					
						locationIsCalculated = true;
						break;
					case kSpeedGoToTarget_assignment_deviation:
						nodeGeom = localData->GetLeastDeviatedNode(targetTime, currentPosition, currentSpeed);
						break;
					case kSpeedGoToTarget_assignment_integer:
						nodeGeom = localData->GetNodeGeometry(0); // fall-back plan
						if (chMXSInteger != NULL) {
							int index = chMXSInteger->GetValue(i);
							if ((index >= 0) && (index < localData->NodeGeometryCount()))
								nodeGeom = localData->GetNodeGeometry(index);
						} else {
							goto invalidTarget;
						}
						break;
					}
					if (nodeGeom != NULL) {
						if (targetNode != nodeGeom->GetNode()) {
							targetNode = nodeGeom->GetNode();
							chTargetNodeW->SetValue(i, targetNode);
							targetSwitch = true;
						}
					}
				}
			} else {
				if (targetType == kSpeedGoToTarget_targetType_objects)
					nodeGeom = localData->GetNodeGeometry(0);
			}

			if ((targetType ==  kSpeedGoToTarget_targetType_objects) && (nodeGeom == NULL))
				goto invalidTarget;

			if (nodeGeom == NULL) {
				targetTM = actionNode->GetObjTMAfterWSM(targetTime);
			} else {
				targetTM.IdentityMatrix();
				if (nodeGeom->GetTM(targetTime) != NULL)
					targetTM = *(nodeGeom->GetTM(targetTime));
			}

			if (targetSwitch || (pointType == kSpeedGoToTarget_pointType_vector)) {
				if (nodeGeom != NULL) {
					if (nodeGeom->GetTM(time) != NULL)
						targetTM = *(nodeGeom->GetTM(targetTime));
					switch (pointType) {
					case kSpeedGoToTarget_pointType_random:
						if (!nodeGeom->GetLocationPoint(targetTime, worldLocation, localLocation, currentFaceIndex, true))
							goto invalidTarget;
						objectLocation = worldLocation*Inverse(targetTM);
						break;
					case kSpeedGoToTarget_pointType_surface:
						if (!locationIsCalculated)
							if (!nodeGeom->GetClosestPoint(targetTime, currentPosition, worldLocation, localLocation, currentFaceIndex))
								goto invalidTarget;
						objectLocation = worldLocation*Inverse(targetTM);
						break;
					case kSpeedGoToTarget_pointType_vector:
						if (chMXSVector != NULL) {
							currentFaceIndex = 0;
							localLocation = chMXSVector->GetValue(i);
							worldLocation = localLocation*targetTM;
						} else {
							goto invalidTarget;
						}
						objectLocation = localLocation;
						break;
					}
				} else { // set by the test icon
					targetTM = actionNode->GetObjTMAfterWSM(targetTime);
					if (iconSizeAnimated)
						iconSize = GetPFFloat(pblock(), kSpeedGoToTarget_iconSize, targetTime.TimeValue());
					switch (pointType) {
					case kSpeedGoToTarget_pointType_random:
						localLocation = 0.5f*RandSphereSurface(randGen);
						worldLocation = iconSize*localLocation*targetTM;
						break;
					case kSpeedGoToTarget_pointType_surface:
						worldLocation = GetClosestIconPoint(iconSize, targetTM, currentPosition, localLocation);
						break;
					case kSpeedGoToTarget_pointType_vector:
						if (chMXSVector != NULL) {
							currentFaceIndex = 0;
							worldLocation = localLocation = chMXSVector->GetValue(i);
						} else {
							goto invalidTarget;
						}	
						break;
					}
					objectLocation = localLocation;
				}

				// set TargetPoint channel value
				if (followTarget) {
					if (animatedShape && (targetType == kSpeedGoToTarget_targetType_objects)) {
						chTargetPointW->SetValue(i, localLocation);
						chFaceIndexW->SetValue(i, currentFaceIndex);
					} else {
						if ((nodeGeom == NULL) && (iconSize != 0.0f)) objectLocation *= 1.0f/iconSize;
						chTargetPointW->SetValue(i, objectLocation);
					}
				} else {
					chTargetPointW->SetValue(i, worldLocation);
				}
			}

			// find target point
			objectLocation = Point3::Origin; // fall-back plan
			if (followTarget) {
				if (chFaceIndexR != NULL) {
					if (nodeGeom != NULL) {
						Mesh* mesh = nodeGeom->GetMesh(targetTime);
						if (mesh == NULL) goto invalidTarget;
						currentFaceIndex = chFaceIndexR->GetValue(i);
						if ((currentFaceIndex >= 0) && (currentFaceIndex < mesh->getNumFaces())) {
							localLocation = chTargetPointR->GetValue(i);
							Point3 v0 = mesh->verts[mesh->faces[currentFaceIndex].v[0]];
							Point3 v1 = mesh->verts[mesh->faces[currentFaceIndex].v[1]];
							Point3 v2 = mesh->verts[mesh->faces[currentFaceIndex].v[2]];
							objectLocation = v0 + localLocation.x*(v1-v0) + localLocation.y*(v2-v0);
						}
					}
				} else {
					objectLocation = chTargetPointR->GetValue(i);
				}
				if (nodeGeom == NULL) {
					if (iconSizeAnimated)
						iconSize = GetPFFloat(pblock(), kSpeedGoToTarget_iconSize, targetTime.TimeValue());
					objectLocation *= iconSize;
				}
				worldLocation = objectLocation*targetTM;
			} else {
				worldLocation = chTargetPointR->GetValue(i);
			}

			// find docking vector
			if (chDockingVectorR) {
				dockingVector = chDockingVectorR->GetValue(i);
			} else {
				switch ( dockingType ) {
				case kSpeedGoToTarget_dockingType_none:
					dockingVector = -1.0f*Point3::ZAxis;
					break;
				case kSpeedGoToTarget_dockingType_parallel:
					dockingVector = -1.0f*(actionNode->GetObjTMAfterWSM(targetTime)).GetRow(2);
					break;
				case kSpeedGoToTarget_dockingType_spherical:
					dockingVector = targetTM.GetTrans() - worldLocation;
					break;
				case kSpeedGoToTarget_dockingType_cylindrical: {
					dockingVector = targetTM.GetTrans() - worldLocation;
					Point3 zAxis = Normalize(targetTM.GetRow(2));
					dockingVector -= zAxis*DotProd(dockingVector, zAxis); }
					break;
				case kSpeedGoToTarget_dockingType_normal:
					if (nodeGeom == NULL) { // use icon for the normal calculations
						dockingVector = GetClosestIconNormal(iconSize, 
															actionNode->GetObjTMAfterWSM(targetTime), 
															worldLocation);
					} else {	
						Mesh* mesh = nodeGeom->GetMesh(targetTime);
						if (mesh == NULL) goto invalidTarget;
						if (currentFaceIndex >= 0) {
							dockingVector = targetTM.VectorTransform(mesh->FaceNormal(currentFaceIndex));
						} else {
							Point3 tempWL, tempLL;
							nodeGeom->GetClosestPoint(targetTime, worldLocation, tempWL, tempLL, currentFaceIndex);
							if (currentFaceIndex >= 0)
								dockingVector = targetTM.VectorTransform(mesh->FaceNormal(currentFaceIndex));
							else
								dockingVector = -1.0f*Point3::ZAxis;
						}
					}
					break;
				default: DbgAssert(0);
				}
			}
		}

		// calculate the particle speed to achieve the goal
		newSpeed = currentSpeed; // for no control type
		if (!chFinishedR->GetValue(i)) {
			if (controlType == kSpeedGoToTarget_type_speed) { // control by speed
				if (chCruiseSpeedR != NULL) {
					cruiseSpeed = chCruiseSpeedR->GetValue(i);		
				} else {
					cruiseSpeed = GetPFFloat(pblock(), kSpeedGoToTarget_cruiseSpeed, syncTime);
					if (chCruiseVarR != NULL)
						cruiseSpeed += chCruiseVarR->GetValue(i)*GetPFFloat(pblock(), kSpeedGoToTarget_cruiseSpeedVariation, syncTime);
					if (cruiseSpeed < 0.0f) cruiseSpeed = 0.0f;
					cruiseSpeed /= TIME_TICKSPERSEC;
				}
				accelLimit = GetPFFloat(pblock(), kSpeedGoToTarget_accelLimit, syncTime);
				accelLimit /= TIME_TICKSPERSEC*TIME_TICKSPERSEC;

				if (easeInAnimated)
					easeIn = GetPFFloat(pblock(), kSpeedGoToTarget_easeIn, syncTime);
				if (chInitDistanceR != NULL) {
					float initialProximity = chInitDistanceR->GetValue(i);
					float currentProximity = Length(worldLocation - currentPosition);
					if ((initialProximity > currentProximity) && (initialProximity > 0.0f)) {
						float distanceRatio = currentProximity/initialProximity;
						cruiseSpeed *= 1.0f - (1.0f-distanceRatio)*easeIn;
					}
				}			

				bool isFinished = false;
				if (dockingType == kSpeedGoToTarget_dockingType_none) {
					newSpeed = CalcNewSpeedWithoutDocking(worldLocation, currentPosition, currentSpeed, 
														cruiseSpeed, accelLimit, time, timeEnd, isFinished);
				} else {
					newSpeed = CalcNewSpeedWithDocking(worldLocation, currentPosition, currentSpeed, cruiseSpeed,
													accelLimit, dockingVector, dockingDistance, time, timeEnd, isFinished);
				}
				if (isFinished) chFinishedW->SetValue(i, true);
			} else if (controlType == kSpeedGoToTarget_type_time) { // control by time
				DbgAssert(chTargetTimeR);
				targetTime = chTargetTimeR->GetValue(i);

				if (targetTime <= time) { // particle is out of control: the time is gone
					chFinishedW->SetValue(i, true);
				} else { // we have time to direct particle
					if (useDockingSpeed) {
						dockingSpeed = (chDockingSpeedR != NULL) ? chDockingSpeedR->GetValue(i) : 0.0f;
						if (dockingType == kSpeedGoToTarget_dockingType_none) {
							newSpeed = CalcNewSpeedWithDockingValue(worldLocation, currentPosition, currentSpeed,
															targetTime, dockingSpeed, time, timeEnd);
						} else {
							newSpeed = CalcNewSpeedWithDockingValueAndVector(worldLocation, currentPosition, currentSpeed,
															targetTime, dockingSpeed, dockingVector, time, timeEnd);
						}
					} else {
						if (dockingType == kSpeedGoToTarget_dockingType_none) {
							newSpeed = CalcNewSpeedWithoutDocking(worldLocation, currentPosition, currentSpeed,
															targetTime, time, timeEnd);
						} else {
							dockingSpeed = (chCruiseSpeedR != NULL) ? chCruiseSpeedR->GetValue(i) : 0.0f;
							newSpeed = CalcNewSpeedWithDockingValueAndVector(worldLocation, currentPosition, currentSpeed,
															targetTime, dockingSpeed, dockingVector, time, timeEnd);
						}
					}
				}
			}
		}

		if (testType == kSpeedGoToTarget_testType_pivot)
			worldLocation = targetTM.GetTrans();

		if (CalcTestCondition(worldLocation, currentPosition, newSpeed, testDistance,
			time, timeEnd, timeToSatisfyTest)) {
			if (timeToSatisfyTest > 0.0f)
				particlesToAdvance.Set(i);
			testResult.Set(i);
			testTime[i] = float(time - timeStart) + timeToSatisfyTest;
		}
				
		chSpeedW->SetValue(i, newSpeed);
	}

	// advance particles in time if they satisfy the condition and need to be pushed forward
	if (integrator != NULL)
	{
		Tab<PreciseTimeValue> timeToAdvance;
		timeToAdvance.SetCount(count);
		for(i=0; i<count; i++)
			if (particlesToAdvance[i] != 0)
				timeToAdvance[i] = timeStart + testTime[i];
		integrator->Proceed(pCont, timeToAdvance, particlesToAdvance);
	}

	// snap particle for their final target position if testDistance
	if ((testDistance <= 0.0f) && (controlType == kSpeedGoToTarget_type_time)) {
		for(i=0; i<count; i++) {
			if (particlesToAdvance[i] != 0) {
				if (useDockingSpeed) {
					dockingSpeed = (chDockingSpeedR != NULL) ? chDockingSpeedR->GetValue(i) : 0.0f;
					if (dockingType == kSpeedGoToTarget_dockingType_none) {
						currentSpeed = chSpeedR->GetValue(i);
						if (LengthSquared(currentSpeed) > 0.0)
							chSpeedW->SetValue(i, dockingSpeed*Normalize(currentSpeed));
					} else {
						if (chDockingVectorR != NULL) {
							dockingVector = chDockingVectorR->GetValue(i);
							chSpeedW->SetValue(i, dockingSpeed*dockingVector);
						}
					}
				} else {
					dockingSpeed = (chCruiseSpeedR != NULL) ? chCruiseSpeedR->GetValue(i) : 0.0f;
					if (dockingType != kSpeedGoToTarget_dockingType_none) {
						if (chDockingVectorR != NULL) {
							dockingVector = chDockingVectorR->GetValue(i);
							chSpeedW->SetValue(i, dockingSpeed*dockingVector);
						}
					}
				}
			}
		}
	}

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;

invalidTarget:
	testResult.ClearAll();
	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return false;
}

ClassDesc* PFTestSpeedGoToTarget::GetClassDesc() const
{
	return GetPFTestSpeedGoToTargetDesc();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFTestSpeedGoToTarget						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PolyShape* PFTestSpeedGoToTarget::GetIconShape(TimeValue time)
// lazy initialization for logo shape
{
	UpdateIcon(time);
	return iconShape();
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpeedGoToTarget						 |
//+--------------------------------------------------------------------------+
Mesh* PFTestSpeedGoToTarget::GetIconMesh(TimeValue time)
// lazy initialization for emitter mesh
{
	UpdateIcon(time);
	return iconMesh();
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpeedGoToTarget						 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::UpdateIcon(TimeValue time)
// update logo size
{
	if (_validIcon().InInterval(time)) return;

	if (iconShape() == NULL) _iconShape() = new PolyShape();
	if (_iconMesh() == NULL) _iconMesh() = new Mesh();

	float size;
	_validIcon() = FOREVER;
	pblock()->GetValue(kSpeedGoToTarget_iconSize, time, size, _validIcon() );
	int dockingType = pblock()->GetInt(kSpeedGoToTarget_dockingType, time);
	switch(dockingType) {
	case kSpeedGoToTarget_dockingType_none:
		iconShape()->NewShape(); // to clear all data
		break;
	case kSpeedGoToTarget_dockingType_parallel:
		PFActionIcon::BuildLogo4ArrowsInParallel(_iconShape());
		break;
	case kSpeedGoToTarget_dockingType_spherical:
		PFActionIcon::BuildLogo6ArrowsInSphere(_iconShape());
		break;
	case kSpeedGoToTarget_dockingType_cylindrical:
		PFActionIcon::BuildLogo4ArrowsInCircle(_iconShape());
		break;
	case kSpeedGoToTarget_dockingType_normal:
		PFActionIcon::BuildLogo12ArrowsInSphereSurface(_iconShape());
		break;
	}
	int i, j;
	for(i=0; i<iconShape()->numLines; i++)
		for(j=0; j<iconShape()->lines[i].numPts; j++)
			iconShape()->lines[i].pts[j].p *= size;
	iconShape()->BuildBoundingBox();
	
	PFActionIcon::BuildSphere(_iconMesh());
	for(i=0; i<iconMesh()->getNumVerts(); i++)
		iconMesh()->verts[i] *= size;
	iconMesh()->buildBoundingBox();
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpeedGoToTarget						 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::InvalidateIcon()
{
	_validIcon() = NEVER;
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpeedGoToTarget						 |
//+--------------------------------------------------------------------------+
void PFTestSpeedGoToTarget::GetIconBoundBox(TimeValue t, Box3& box ) 
{
	box += GetIconShape(t)->GetBoundingBox();
	box += GetIconMesh(t)->getBoundingBox();
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpeedGoToTarget						 |
//+--------------------------------------------------------------------------+
int PFTestSpeedGoToTarget::getNumTargets()
{
	int num = pblock()->Count(kSpeedGoToTarget_targets);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kSpeedGoToTarget_targets, 0, i) != NULL)
			count++;
	return count;	
}

bool gUpdateTargetsInProgress = false;
bool PFTestSpeedGoToTarget::updateFromRealTargets()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kSpeedGoToTarget_targets, 
				kSpeedGoToTarget_targetsMaxscript, gUpdateTargetsInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kSpeedGoToTarget_targets, _T("Target_Objects"));
	return res;
}

bool PFTestSpeedGoToTarget::updateFromMXSTargets()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kSpeedGoToTarget_targets, 
				kSpeedGoToTarget_targetsMaxscript, gUpdateTargetsInProgress, this);
}

bool PFTestSpeedGoToTarget::updateObjectNames(int pblockID)
{
	if (pblock() == NULL) return false;
	bool needUpdate = false;
	int count = pblock()->Count(pblockID);
	if (count != objectsToShow().Count()) needUpdate = true;
	if (!needUpdate) {
		for(int i=0; i<count; i++)
			if (pblock()->GetINode(pblockID, 0, i) != objectToShow(i))
			{	needUpdate = true; 	break;	}
	}
	if (!needUpdate) return false;
	_objectsToShow().SetCount(count);
	for(int i=0; i<count; i++)
		_objectToShow(i) = pblock()->GetINode(pblockID, 0, i);
	return true;
}

//+--------------------------------------------------------------------------+
//|							From HitByNameDlgCallback						 |
//+--------------------------------------------------------------------------+
int PFTestSpeedGoToTarget::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PFTestSpeedGoToTarget::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kSpeedGoToTarget_targets, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}

} // end of namespace PFActions