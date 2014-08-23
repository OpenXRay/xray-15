/**********************************************************************
 *<
	FILE:			PFOperatorSpeedKeepApart.cpp

	DESCRIPTION:	SpeedKeepApart Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSpeedKeepApart.h"

#include "PFOperatorSpeedKeepApart_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "IParticleGroup.h"
#include "PFMessages.h"

#include "IPFActionListPool.h"
#include "IPFSystemPool.h"

#include "macrorec.h"

namespace PFActions {

// SpeedKeepApart creates a particle channel to store a random float for each particle
// when using force range variation
// This way the random variation value is calculated only once when a particle enters the event.
#define PARTICLECHANNELRNDFLOATR_INTERFACE Interface_ID(0x38410659, 0x1eb34500) 
#define PARTICLECHANNELRNDFLOATW_INTERFACE Interface_ID(0x38410659, 0x1eb34501) 

#define GetParticleChannelRndFloatRInterface(obj) ((IParticleChannelIFloatR*)obj->GetInterface(PARTICLECHANNELRNDFLOATR_INTERFACE)) 
#define GetParticleChannelRndFloatWInterface(obj) ((IParticleChannelIFloatW*)obj->GetInterface(PARTICLECHANNELRNDFLOATW_INTERFACE)) 

PFOperatorSpeedKeepApart::PFOperatorSpeedKeepApart()
{ 
	_scriptPBlock() = NULL;
	_activeProgress() = false;
	_notifyAListRegistered() = false;
	_notifyPSystemRegistered() = false;

	GetClassDesc()->MakeAutoParamBlocks(this); 
}

PFOperatorSpeedKeepApart::~PFOperatorSpeedKeepApart()
{
	ReleaseSelectionTracking();
}

FPInterfaceDesc* PFOperatorSpeedKeepApart::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &speedKeepApart_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &speedKeepApart_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &speedKeepApart_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSpeedKeepApart::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SPEEDKEEPAPART_CLASS_NAME);
}

Class_ID PFOperatorSpeedKeepApart::ClassID()
{
	return PFOperatorSpeedKeepApart_Class_ID;
}

void PFOperatorSpeedKeepApart::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSpeedKeepApart::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFOperatorSpeedKeepApart::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	case 1: return _scriptPBlock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFOperatorSpeedKeepApart::SubAnimName(int i)
{
	if (i == 0) return GetString(IDS_PARAMETERS);
//	if (i == 1) return GetString(IDS_SCRIPTWIRING);
	return GetString(IDS_UNDEFINED);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFOperatorSpeedKeepApart::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorSpeedKeepApart::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	if (i==1) return _scriptPBlock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorSpeedKeepApart::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	if (id == 1) return _scriptPBlock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFOperatorSpeedKeepApart::GetReference(int i)
{
	switch (i) {
	case kSpeedKeepApart_mainPBlockIndex:
		return (RefTargetHandle)pblock();
	case kSpeedKeepApart_scriptPBlockIndex:
		return (RefTargetHandle)scriptPBlock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedKeepApart::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==kSpeedKeepApart_mainPBlockIndex) {
		_pblock() = (IParamBlock2*)rtarg;
	}
	if (i==kSpeedKeepApart_scriptPBlockIndex) {
		_scriptPBlock() = (IParamBlock2*)rtarg;
	}
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedKeepApart::UpdatePViewUI(IParamBlock2* pblock, int updateID)
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		if (pblock != map->GetParamBlock()) continue;
		map->Invalidate(updateID);
		Interval currentTimeInterval;
		currentTimeInterval.SetInstant(GetCOREInterface()->GetTime());
		map->Validity() = currentTimeInterval;
	}
}

RefResult PFOperatorSpeedKeepApart::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				if (scriptPBlock() == NULL) break; // parameters aren't initialized completely
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSpeedKeepApart_rangeType:
					{
					bool disableAbsRange = (((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_absRange))
								|| (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_relative));
					if (disableAbsRange) RefreshUI(kSpeedKeepApart_message_disableAbsRange);
					else RefreshUI(kSpeedKeepApart_message_enableAbsRange);
					bool disableRelRange = (((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_relRange))
								|| (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_absolute));
					if (disableRelRange) RefreshUI(kSpeedKeepApart_message_disableRelRange);
					else RefreshUI(kSpeedKeepApart_message_enableRelRange);
					}
					break;
				case kSpeedKeepApart_scopeType:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// break is omitted intentionally (bayboro 11-19-02)
				case kSpeedKeepApart_groups:
					RefreshUI(kSpeedKeepApart_message_update);
					SetSelectionTracking();
					break;
				case kSpeedKeepApart_systems:
					RefreshUI(kSpeedKeepApart_message_update);
					SetSelectionTracking();
					break;
				case kSpeedKeepApart_useAccelLimit:
				case kSpeedKeepApart_useSpeedLimit:
					RefreshUI(kSpeedKeepApart_message_update);
					break;
				} 
				UpdatePViewUI(pblock(), lastUpdateID);
			} else if (hTarget == scriptPBlock() ) {
				if (pblock() == NULL) break; // parameters aren't initialized completely
				int lastUpdateID = scriptPBlock()->LastNotifyParamID();
				bool disableForce = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
							&& (scriptPBlock()->GetInt(kSpeedKeepApart_useFloat, 0) == kSpeedKeepApart_useFloat_force));
				if (disableForce) RefreshUI(kSpeedKeepApart_message_disableForce);
				else RefreshUI(kSpeedKeepApart_message_enableForce);
				bool disableAbsRange = (((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
							&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_absRange))
							|| (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_relative));
				if (disableAbsRange) RefreshUI(kSpeedKeepApart_message_disableAbsRange);
				else RefreshUI(kSpeedKeepApart_message_enableAbsRange);
				bool disableRelRange = (((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
							&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_relRange))
							|| (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_absolute));
				if (disableRelRange) RefreshUI(kSpeedKeepApart_message_disableRelRange);
				else RefreshUI(kSpeedKeepApart_message_enableRelRange);
				bool disableVariation = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
							&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) != kSpeedKeepApart_useVector_none));
				if (disableVariation) RefreshUI(kSpeedKeepApart_message_disableVariation);
				else RefreshUI(kSpeedKeepApart_message_enableVariation);
				UpdatePViewUI(scriptPBlock(), lastUpdateID);
			}
			break;

		case REFMSG_NODE_HANDLE_CHANGED:
			if (hTarget == NULL) // direct notification
				updateNodeHandles((IMergeManager*)partID);
			return REF_STOP;

		case kSpeedKeepApart_RefMsg_InitDlg:
			{
				if ((pblock() != NULL) && (scriptPBlock() != NULL)) {
					bool disableForce = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useFloat, 0) == kSpeedKeepApart_useFloat_force));
					if (disableForce) RefreshUI(kSpeedKeepApart_message_disableForce, (IParamMap2*)partID);
					else RefreshUI(kSpeedKeepApart_message_enableForce, (IParamMap2*)partID);
					bool disableAbsRange = (((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_absRange))
								|| (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_relative));
					if (disableAbsRange) RefreshUI(kSpeedKeepApart_message_disableAbsRange, (IParamMap2*)partID);
					else RefreshUI(kSpeedKeepApart_message_enableAbsRange, (IParamMap2*)partID);
					bool disableRelRange = (((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_relRange))
								|| (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_absolute));
					if (disableRelRange) RefreshUI(kSpeedKeepApart_message_disableRelRange, (IParamMap2*)partID);
					else RefreshUI(kSpeedKeepApart_message_enableRelRange, (IParamMap2*)partID);
					bool disableVariation = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) != kSpeedKeepApart_useVector_none));
					if (disableVariation) RefreshUI(kSpeedKeepApart_message_disableVariation, (IParamMap2*)partID);
					else RefreshUI(kSpeedKeepApart_message_enableVariation, (IParamMap2*)partID);
				}
				RefreshUI(kSpeedKeepApart_message_update, (IParamMap2*)partID);
			}
			return REF_STOP;

		case kSpeedKeepApart_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;

		case kSpeedKeepApart_RefMsg_MacrorecGroups:
			macrorecGroups();
			return REF_STOP;

		case kSpeedKeepApart_RefMsg_MacrorecSystems:
			macrorecSystems();
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSpeedKeepApart::Clone(RemapDir &remap)
{
	PFOperatorSpeedKeepApart* newOp = new PFOperatorSpeedKeepApart();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	newOp->ReplaceReference(1, remap.CloneRef(scriptPBlock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSpeedKeepApart::GetObjectName()
{
	return GetString(IDS_OPERATOR_SPEEDKEEPAPART_OBJECT_NAME);
}

bool PFOperatorSpeedKeepApart::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool initRes = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	_activeProgress() = true;
	SetSelectionTracking();
	return initRes;
}

bool PFOperatorSpeedKeepApart::Release(IObject* pCont)
{
	PFSimpleAction::Release(pCont);
	_activeProgress() = false;
	return true;
}

const ParticleChannelMask& PFOperatorSpeedKeepApart::ChannelsUsed(const Interval& time) const
{
								//  read						 &	write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Position|PCU_Amount, PCU_Speed);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSpeedKeepApart::GetUseScriptWiring() const
{
	if (scriptPBlock() == NULL) return false;
	return (scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedKeepApart::SetUseScriptWiring(bool useScriptWiring)
{
	if (scriptPBlock() == NULL) return;
	scriptPBlock()->SetValue(kSpeedKeepApart_useScriptWiring, 0, (int)useScriptWiring);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
int PFOperatorSpeedKeepApart::NumPViewParamBlocks() const
{
	return (GetUseScriptWiring() ? 2 : 1);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorSpeedKeepApart::GetPViewParamBlock(int i) const
{
	if (i==0) return pblock();
	if (i==1) return scriptPBlock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSpeedKeepApart::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDKEEPAPART_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSpeedKeepApart::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDKEEPAPART_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSpeedKeepApart::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kSpeedKeepApart_scopeType, 0);
	switch(type) {
	case kSpeedKeepApart_scopeType_currentGroup:
		nameSuffix = GetString(IDS_GROUP);
		break;
	case kSpeedKeepApart_scopeType_currentSystem:
		nameSuffix = GetString(IDS_SYSTEM);
		break;
	case kSpeedKeepApart_scopeType_selectedGroups:
		nameSuffix = GetString(IDS_SELECTEDGROUPS_ABBR);
		break;
	case kSpeedKeepApart_scopeType_selectedSystems:
		nameSuffix = GetString(IDS_SELECTEDSYSTEMS_ABBR);
		break;
	}
	return true;
}

bool PFOperatorSpeedKeepApart::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	if(chPos == NULL) return false;
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;
	int count = chAmount->Count();
	if (count <= 0) return true; // no particles to modify

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// the channel of interest
	IParticleChannelPoint3W* chSpeedW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELSPEEDR_INTERFACE,
																			PARTICLECHANNELSPEEDW_INTERFACE, true );
	if (chSpeedW == NULL) return false; // can't modify Speed channel in the container
	IParticleChannelPoint3R* chSpeedR = GetParticleChannelSpeedRInterface(pCont);
	if (chSpeedR == NULL) return false; // can't read particle speed

	bool useScriptForce = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useFloat, 0) == kSpeedKeepApart_useFloat_force));
	IParticleChannelFloatR* chFloat = NULL;
	if (useScriptForce) {
		chFloat = GetParticleChannelMXSFloatRInterface(pCont);
		if (chFloat == NULL) return false;
	}

	bool useScriptAbsRange = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_absRange)
								&& (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_absolute));
	bool useScriptRelRange = ((scriptPBlock()->GetInt(kSpeedKeepApart_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kSpeedKeepApart_useVector, 0) == kSpeedKeepApart_useVector_relRange)
								&& (pblock()->GetInt(kSpeedKeepApart_rangeType, 0) == kSpeedKeepApart_rangeType_relative));
	IParticleChannelPoint3R* chVector = NULL;
	if (useScriptAbsRange || useScriptRelRange) {
		chVector = GetParticleChannelMXSVectorRInterface(pCont);
		if (chVector == NULL) return false;
	}

	Tab<IObject*> selConts;
	bool useOwn = false;
	collectPContainers(timeEnd, pCont, pNode, selConts, useOwn);

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	if (randGen == NULL) return false;
	
	int tpf = GetTicksPerFrame();
	float force = GetPFFloat(pblock(), kSpeedKeepApart_force, timeStart);
	bool useAccelLimit = (pblock()->GetInt(kSpeedKeepApart_useAccelLimit, timeStart) != 0);
	bool useSpeedLimit = (pblock()->GetInt(kSpeedKeepApart_useSpeedLimit, timeStart) != 0);
	float accelLimit = GetPFFloat(pblock(), kSpeedKeepApart_accelLimit, timeStart)/TIME_TICKSPERSEC;
	float speedLimit = GetPFFloat(pblock(), kSpeedKeepApart_speedLimit, timeStart)/TIME_TICKSPERSEC;
	int rangeType = pblock()->GetInt(kSpeedKeepApart_rangeType, timeStart);
	float coreSize = GetPFFloat(pblock(), kSpeedKeepApart_coreSize, timeStart);
	float falloffSize = GetPFFloat(pblock(), kSpeedKeepApart_falloffSize, timeStart);
	float coreRel = GetPFFloat(pblock(), kSpeedKeepApart_coreRatio, timeStart);
	float falloffRel = GetPFFloat(pblock(), kSpeedKeepApart_falloffRatio, timeStart);
	float variation = GetPFFloat(pblock(), kSpeedKeepApart_variation, timeStart);
	bool useRndFloat = (variation > 0.0f);
	bool useRelSize = (rangeType == kSpeedKeepApart_rangeType_relative);
	float totalSize2 = coreSize + falloffSize;
	totalSize2 = totalSize2*totalSize2;

	bool initRndFloat = false;
	IParticleChannelFloatR* chRndFloatR = NULL;
	IParticleChannelFloatW* chRndFloatW = NULL;
	if (useRndFloat) {
		chRndFloatR = (IParticleChannelFloatR*) chCont->EnsureInterface(PARTICLECHANNELRNDFLOATR_INTERFACE,
																	ParticleChannelFloat_Class_ID,
																	true, PARTICLECHANNELRNDFLOATR_INTERFACE,
																	PARTICLECHANNELRNDFLOATW_INTERFACE, true,
																	actionNode, (Object*)this, &initRndFloat);
		if (chRndFloatR == NULL) return false; // can't read/create RndFloat channel
		if (initRndFloat) {
			chRndFloatW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELRNDFLOATW_INTERFACE, (Object*)this);
			if (chRndFloatW == NULL) return false; // can't init RndFloat channel
		}
	}

	IParticleChannelMeshR* chShape = NULL;
	IParticleChannelPoint3R* chScale = NULL;
	if (useRelSize) {
		chShape = GetParticleChannelShapeRInterface(pCont);
		chScale = GetParticleChannelScaleRInterface(pCont);
	}

	const float kCenterWeight = 10.0f;

	for(int i=0; i<count; i++) {
		if (useScriptForce) force = chFloat->GetValue(i);
		if (useScriptAbsRange) {
			Point3 vec = chVector->GetValue(i);
			coreSize = vec.x;
			falloffSize = vec.y;
			variation = 0.0f;
		}
		if (useScriptRelRange) {
			Point3 vec = chVector->GetValue(i);
			coreRel = vec.x;
			falloffRel = vec.y;
			variation = 0.0f;
		}

		PreciseTimeValue timeDif = timeEnd - chTime->GetValue();
		if (float(timeDif) <= 0.0f) continue; // no time to modify the speed

		if (useRelSize) {
			float shapeSize = 1.0f;
			if (chShape) {
				Mesh* mesh = ((Mesh*)(chShape->GetValue(i)));
				if (mesh != NULL) {
					Box3 box = mesh->getBoundingBox();
					Point3 diam3D = box.pmax - box.pmin;
					shapeSize = diam3D[MaxComponent(diam3D)];
				}
			}
			if (chScale) {
				Point3 scale3D = chScale->GetValue(i);
				shapeSize *= scale3D[MaxComponent(scale3D)];
			}
			coreSize = shapeSize*coreRel;
			falloffSize = shapeSize*falloffRel;
			totalSize2 = coreSize + falloffSize;
			totalSize2 = totalSize2*totalSize2;
		}
		
		if (useRndFloat) {
			float curRand = 0.0f;
			if (initRndFloat) {
				if (chNew->IsNew(i)) {
					curRand = randGen->Rand11();
					if (variation > 1.0f) {
						curRand = 0.5f*(variation + 1.0f)*(curRand + 1.0f);
					} else {
						curRand = 1.0f + variation*curRand;
					}
					chRndFloatW->SetValue(i, curRand);
				} else {
					curRand = chRndFloatR->GetValue(i);
				}
			} else {
				curRand = chRndFloatR->GetValue(i);
			}
			coreSize *= curRand;
			falloffSize *= curRand;
			totalSize2 *= curRand*curRand;
		}

		Point3 curPos = chPos->GetValue(i);

		double resForceX=0.0, resForceY=0.0, resForceZ=0.0, resWeight=0.0;
		Point3 curForce;
		float curWeight, forceFactor;
		if (useOwn) {
			for(int j=0; j<count; j++) {
				if (j == i) continue; // no interaction with itself
				Point3 pos = chPos->GetValue(j);
				Point3 posDif = curPos - pos;
				float len = DotProd(posDif, posDif);
				if (len >= totalSize2) continue; // particles too far apart
				if (len <= 0.0f) continue; // bull's eye: no position difference to calculate force vector
				len = sqrt(len);
				forceFactor = force;
				if (len < coreSize) {
					curWeight = kCenterWeight*(1.0f - len/coreSize) + 1.0f;
					if (force < 0.0f)
						forceFactor *= 1.0f - len/coreSize;
				} else {
					len -= coreSize;
					if (len >= falloffSize) continue; // particles too far apart
					curWeight = 1.0f - len/falloffSize;
					if (force < 0.0f)
						forceFactor *= len/falloffSize;
					else
						forceFactor *= curWeight;
				}

				curForce = curWeight*forceFactor*(posDif/len);
				resWeight += curWeight;
				resForceX += curForce.x;
				resForceY += curForce.y;
				resForceZ += curForce.z;
			}
		}

		for(int k=0; k<selConts.Count(); k++) {
			IParticleChannelPoint3R* chCurPos = GetParticleChannelPositionRInterface(selConts[k]);
			if(chCurPos == NULL) continue;
			IParticleChannelAmountR* chCurAmount = GetParticleChannelAmountRInterface(selConts[k]);
			if(chCurAmount == NULL) continue;
			int curCount = chCurAmount->Count();
			if (curCount <= 0) continue; // no particles to interact with

			for(int j=0; j<curCount; j++) {
				Point3 pos = chCurPos->GetValue(j);
				Point3 posDif = curPos - pos;
				float len = DotProd(posDif, posDif);
				if (len >= totalSize2) continue; // particles too far apart
				if (len <= 0.0f) continue; // bull's eye: no position difference to calculate force vector
				len = sqrt(len);
				forceFactor = force;
				if (len < coreSize) {
					curWeight = kCenterWeight*(1.0f - len/coreSize) + 1.0f;
					if (force < 0.0f)
						forceFactor *= 1.0f - len/coreSize;
				} else {
					len -= coreSize;
					if (len >= falloffSize) continue; // particles too far apart
					curWeight = 1.0f - len/falloffSize;
					if (force < 0.0f)
						forceFactor *= len/falloffSize;
					else
						forceFactor *= curWeight;
				}

				curForce = curWeight*forceFactor*(posDif/len);
				resWeight += curWeight;
				resForceX += curForce.x;
				resForceY += curForce.y;
				resForceZ += curForce.z;
			}
		}

		if (resWeight <= 0.0) continue; // the particle is far away from all other particles
		Point3 resForce = Point3(float(resForceX/resWeight), float(resForceY/resWeight), float(resForceZ/resWeight));
		if (useAccelLimit) {
			float newAccelLen = Length(resForce);
			if (newAccelLen > accelLimit)
				resForce = accelLimit*Normalize(resForce);
		}

		Point3 newSpeed = chSpeedR->GetValue(i);
		newSpeed += float(timeDif)*resForce/(tpf*tpf);
		if (useSpeedLimit) {
			float newSpeedLen = Length(newSpeed);
			if (newSpeedLen > speedLimit)
				newSpeed = speedLimit*Normalize(newSpeed);
		}
		chSpeedW->SetValue(i, newSpeed);
	}

	return true;
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorSpeedKeepApart					 |
//+--------------------------------------------------------------------------+
static void UpdateSelectedSet(void *param, NotifyInfo *info) 
{
	PFOperatorSpeedKeepApart* oper = (PFOperatorSpeedKeepApart*)param;
	oper->VerifySelectionTracking();
}

void PFOperatorSpeedKeepApart::SetSelectionTracking()
{
	if (!activeProgress()) return;

	int scopeType = pblock()->GetInt(kSpeedKeepApart_scopeType, 0);
	bool useGroups = (scopeType == kSpeedKeepApart_scopeType_selectedGroups);
	bool useSystems = (scopeType == kSpeedKeepApart_scopeType_selectedSystems);
	
	if (notifyAListRegistered() && !useGroups) {
		GetPFActionListPool()->UnRegisterNotification(UpdateSelectedSet, this);
		_notifyAListRegistered() = false;
	}

	if (notifyPSystemRegistered() && !useSystems) {
		GetPFSystemPool()->UnRegisterNotification(UpdateSelectedSet, this);
		_notifyPSystemRegistered() = false;
	}

	_selectedALists().ZeroCount();
	if (useGroups) {
		if (!notifyAListRegistered()) {
			GetPFActionListPool()->RegisterNotification(UpdateSelectedSet, this);
			_notifyAListRegistered() = true;
		}
		IPFActionListPool* pool = GetPFActionListPool();
		for(int i=0; i<pool->NumActionLists(); i++) {
			INode* aListNode = pool->GetActionList(i);
			if (SKAIsSelectedActionList(aListNode, pblock()))
				_selectedALists().Append(1, &aListNode);
		}
	}

	_selectedPSystems().ZeroCount();
	if (useSystems) {	
		if (!notifyPSystemRegistered()) {
			GetPFSystemPool()->RegisterNotification(UpdateSelectedSet, this);
			_notifyPSystemRegistered() = true;
		}
		IPFSystemPool* pool = GetPFSystemPool();
		for(int i=0; i<pool->NumPFSystems(); i++) {
			INode* pSystemNode = pool->GetPFSystem(i);
			if (SKAIsSelectedPSystem(pSystemNode, pblock()))
				_selectedPSystems().Append(1, &pSystemNode);
		}
	}
}

void PFOperatorSpeedKeepApart::VerifySelectionTracking()
{
	if (!activeProgress()) return;
	if (pblock() == NULL) return;

	int scopeType = pblock()->GetInt(kSpeedKeepApart_scopeType, 0);
	bool useGroups = (scopeType == kSpeedKeepApart_scopeType_selectedGroups);
	bool useSystems = (scopeType == kSpeedKeepApart_scopeType_selectedSystems);

	Tab<INode*> selALists;
	Tab<INode*> selPSystems;

	if (useGroups) {
		IPFActionListPool* pool = GetPFActionListPool();
		for(int i=0; i<pool->NumActionLists(); i++) {
			INode* aListNode = pool->GetActionList(i);
			if (SKAIsSelectedActionList(aListNode, pblock()))
				selALists.Append(1, &aListNode);
		}
	}

	if (useSystems) {	
		IPFSystemPool* pool = GetPFSystemPool();
		for(int i=0; i<pool->NumPFSystems(); i++) {
			INode* pSystemNode = pool->GetPFSystem(i);
			if (SKAIsSelectedPSystem(pSystemNode, pblock()))
				selPSystems.Append(1, &pSystemNode);
		}
	}

	bool changedAList = false, changedPSystem = false;

	int num = selALists.Count();
	if (num == _selectedALists().Count()) {
		for(int i=0; i<num; i++)
			if (selALists[i] != _selectedAList(i)) {
				changedAList = true;
				break;
			}
	} else {
		changedAList = true;
	}
	if (changedAList) {
		_selectedALists().SetCount(num);
		for(int i=0; i<num; i++)
			_selectedAList(i) = selALists[i];
	}

	num = selPSystems.Count();
	if (num == _selectedPSystems().Count()) {
		for(int i=0; i<num; i++)
			if (selPSystems[i] != _selectedPSystem(i)) {
				changedPSystem = true;
				break;
			}
	} else {
		changedPSystem = true;
	}
	if (changedPSystem) {
		_selectedPSystems().SetCount(num);
		for(int i=0; i<num; i++)
			_selectedPSystem(i) = selPSystems[i];
	}

	if (changedAList || changedPSystem) NotifyDependents( FOREVER, PART_ALL, REFMSG_CHANGE );
}

void PFOperatorSpeedKeepApart::ReleaseSelectionTracking()
{
	if (notifyAListRegistered()) GetPFActionListPool()->UnRegisterNotification(UpdateSelectedSet, this);
	_notifyAListRegistered() = false;
	if (notifyPSystemRegistered()) GetPFSystemPool()->UnRegisterNotification(UpdateSelectedSet, this);
	_notifyPSystemRegistered() = false;
}

void CollectParticleGroups(INode* node, INodeTab& pGroupNodes)
{
	if (node == NULL) return;
	Object* obj = node->GetObjectRef();
	if (obj != NULL)
		if (obj->ClassID() == ParticleGroup_Class_ID)
			pGroupNodes.Append(1, &node);
	for(int i=0; i<node->NumberOfChildren(); i++)
		CollectParticleGroups(node->GetChildNode(i), pGroupNodes);
}

void PFOperatorSpeedKeepApart::collectPContainers(TimeValue time, IObject* pCont, INode* pSystemNode, Tab<IObject*>& selectedCont, bool& useOwn)
// collects all relevant particle containers
// if a container belongs to a different particle system then the container
// is updated to be in sync with the current time
{
	int i, scopeType = pblock()->GetInt(kSpeedKeepApart_scopeType, 0);

	if (scopeType == kSpeedKeepApart_scopeType_currentGroup) {
		selectedCont.ZeroCount();
		useOwn = true;
	} else {
		INodeTab pGroupNodes;
		IParticleGroup* iPGroup;
		IObject* pContObj;
		CollectParticleGroups(GetCOREInterface()->GetRootNode(), pGroupNodes);
		for(i=0; i<pGroupNodes.Count(); i++) {
			iPGroup = ParticleGroupInterface(pGroupNodes[i]);
			if (iPGroup != NULL) {
				if (iPGroup->GetParticleSystem() != pSystemNode) // outsider group: needs update
					pGroupNodes[i]->NotifyDependents(FOREVER, PartID(&time), kPFMSG_UpdateToTime, NOTIFY_ALL, TRUE );
				bool goodGroup = false;
				switch (scopeType) {
				case kSpeedKeepApart_scopeType_currentSystem:
					goodGroup = (iPGroup->GetParticleSystem() == pSystemNode);
					break;
				case kSpeedKeepApart_scopeType_selectedGroups:
					goodGroup = SKAIsSelectedActionList(iPGroup->GetActionList(), pblock());
					break;
				case kSpeedKeepApart_scopeType_selectedSystems:
					goodGroup = SKAIsSelectedPSystem(iPGroup->GetParticleSystem(), pblock());
					break;
				}
				if (goodGroup) {
					pContObj = iPGroup->GetParticleContainer();
					if (pContObj == pCont) {
						useOwn = true;
					} else {
						if(pContObj != NULL) selectedCont.Append(1, &pContObj);
					}
				}
			}
		}
	}
}

void PFOperatorSpeedKeepApart::updateNodeHandles(IMergeManager* pMM)
{
	if (pMM == NULL) return;
	if (pblock() == NULL) return;
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();

	int i, eventCount = pblock()->Count(kSpeedKeepApart_groups);
	for(i=0; i<eventCount; i++) {
		ULONG eventHandle = pblock()->GetInt(kSpeedKeepApart_groups, 0, i);
		pblock()->SetValue(kSpeedKeepApart_groups, 0, int(pMM->GetNewHandle(eventHandle)), i);
	}
	int systemCount = pblock()->Count(kSpeedKeepApart_systems);
	for(i=0; i<systemCount; i++) {
		ULONG systemHandle = pblock()->GetInt(kSpeedKeepApart_systems, 0, i);
		pblock()->SetValue(kSpeedKeepApart_systems, 0, int(pMM->GetNewHandle(systemHandle)), i);
	}

	if (wasHolding) theHold.Resume();
}

void PFOperatorSpeedKeepApart::macrorecGroups()
{
	if (pblock() == NULL) return;
	TSTR selItemNames = _T("(");
	int numItems = pblock()->Count(kSpeedKeepApart_groups);
	bool nonFirst = false;
	Interface* ip = GetCOREInterface();
	for(int i=0; i<numItems; i++) {
		INode* selNode = ip->GetINodeByHandle(ULONG(pblock()->GetInt(kSpeedKeepApart_groups, 0, i)));
		if (selNode == NULL) continue;
		if (nonFirst) selItemNames = selItemNames + _T(", ");
		selItemNames = selItemNames + _T("$'");
		selItemNames = selItemNames + selNode->GetName();
		selItemNames = selItemNames + _T("'.handle");
		nonFirst = true;
	}
	selItemNames = selItemNames + _T(")");
	TCHAR selNames[8192];
	sprintf(selNames,"%s", selItemNames);
	macroRec->EmitScript();
	macroRec->SetProperty(this, _T("Selected_Events"), mr_name, selNames);
}

void PFOperatorSpeedKeepApart::macrorecSystems()
{
	if (pblock() == NULL) return;
	TSTR selItemNames = _T("(");
	int numItems = pblock()->Count(kSpeedKeepApart_systems);
	bool nonFirst = false;
	Interface* ip = GetCOREInterface();
	for(int i=0; i<numItems; i++) {
		INode* selNode = ip->GetINodeByHandle(ULONG(pblock()->GetInt(kSpeedKeepApart_systems, 0, i)));
		if (selNode == NULL) continue;
		if (nonFirst) selItemNames = selItemNames + _T(", ");
		selItemNames = selItemNames + _T("$'");
		selItemNames = selItemNames + selNode->GetName();
		selItemNames = selItemNames + _T("'.handle");
		nonFirst = true;
	}
	selItemNames = selItemNames + _T(")");
	TCHAR selNames[8192];
	sprintf(selNames,"%s", selItemNames);
	macroRec->EmitScript();
	macroRec->SetProperty(this, _T("Selected_Systems"), mr_name, selNames);
}

ClassDesc* PFOperatorSpeedKeepApart::GetClassDesc() const
{
	return GetPFOperatorSpeedKeepApartDesc();
}

int PFOperatorSpeedKeepApart::GetRand()
{
	return pblock()->GetInt(kSpeedKeepApart_seed);
}

void PFOperatorSpeedKeepApart::SetRand(int seed)
{
	_pblock()->SetValue(kSpeedKeepApart_seed, 0, seed);
}

} // end of namespace PFActions