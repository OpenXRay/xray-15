/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamic.cpp

	DESCRIPTION:	MaterialDynamic Operator implementation
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include <limits>

#include "max.h"
#include "meshdlib.h"

#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorMaterialDynamic.h"

#include "PFOperatorMaterialDynamic_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Creates a transferable particle channel to store a float offset for materialDynamic index
#define PARTICLECHANNELIDOFFSETR_INTERFACE Interface_ID(0x52b76217, 0x1eb34500) 
#define PARTICLECHANNELIDOFFSETW_INTERFACE Interface_ID(0x52b76217, 0x1eb34501) 

#define GetParticleChannelIDOffsetRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELIDOFFSETR_INTERFACE)) 
#define GetParticleChannelIDOffsetWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELIDOFFSETW_INTERFACE)) 

// Creates a transferable particle channel to store a time offset for each particle
// when "Rand Offset" is enabled.
#define PARTICLECHANNELLOCALOFFSETR_INTERFACE Interface_ID(0x4e541176, 0x1eb34500) 
#define PARTICLECHANNELLOCALOFFSETW_INTERFACE Interface_ID(0x4e541176, 0x1eb34501) 

#define GetParticleChannelLocalOffsetRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELLOCALOFFSETR_INTERFACE)) 
#define GetParticleChannelLocalOffsetWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELLOCALOFFSETW_INTERFACE)) 

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMaterialDynamicState					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorMaterialDynamicState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialDynamicState::IObject						 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterialDynamicState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialDynamicState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorMaterialDynamicState::GetClassID() 
{ 
	return PFOperatorMaterialDynamicState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialDynamicState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMaterialDynamicState::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;

	isave->BeginChunk(IPFActionState::kChunkActionHandle);
	ULONG handle = actionHandle();
	if ((res = isave->Write(&handle, sizeof(ULONG), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkRandGen);
	if ((res = isave->Write(randGen(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorMaterialDynamicState::IPFActionState					 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMaterialDynamicState::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch(iload->CurChunkID())
		{
		case IPFActionState::kChunkActionHandle:
			res=iload->Read(&_actionHandle(), sizeof(ULONG), &nb);
			break;

		case IPFActionState::kChunkRandGen:
			res=iload->Read(_randGen(), sizeof(RandGenerator), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMaterialDynamic						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorMaterialDynamic::PFOperatorMaterialDynamic()
{
	_postProceed() = false;
	_material() = NULL;
	_dadMgr() = new PFOperatorMaterialDynamicDADMgr(this);
	GetClassDesc()->MakeAutoParamBlocks(this);
	pblock()->SetValue(kMaterialDynamic_rate, 0, float(TIME_TICKSPERSEC)/GetTicksPerFrame() );
}

PFOperatorMaterialDynamic::~PFOperatorMaterialDynamic()
{
	if (dadMgr()) delete _dadMgr();
}

FPInterfaceDesc* PFOperatorMaterialDynamic::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &materialDynamic_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &materialDynamic_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &materialDynamic_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorMaterialDynamic::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_MATERIALDYNAMIC_CLASS_NAME);
}

Class_ID PFOperatorMaterialDynamic::ClassID()
{
	return PFOperatorMaterialDynamic_Class_ID;
}

void PFOperatorMaterialDynamic::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorMaterialDynamic::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefTargetHandle PFOperatorMaterialDynamic::GetReference(int i)
{
	switch (i)
	{
	case kMaterialDynamic_reference_pblock:	return (RefTargetHandle)pblock();
	case kMaterialDynamic_reference_material: return (RefTargetHandle)material();
	}
	return NULL;
}

void PFOperatorMaterialDynamic::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case kMaterialDynamic_reference_pblock:	
		_pblock() = (IParamBlock2*)rtarg;	
		break;
	case kMaterialDynamic_reference_material:	
		_material() = (Mtl *)rtarg;			
		if (updateFromRealMtl()) {
			NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
		}
		break;
	}
}

RefResult PFOperatorMaterialDynamic::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
													 PartID& partID, RefMessage message)
{
	IParamMap2* map;
	TSTR dynamicNameSuffix;

	if ( hTarget == pblock() ) {
		int lastUpdateID;
		switch (message)
		{
		case REFMSG_CHANGE:
			lastUpdateID = pblock()->LastNotifyParamID();
			if (lastUpdateID == kMaterialDynamic_material) {
				if (updateFromMXSMtl()) {
					NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				}
				UpdatePViewUI(lastUpdateID);
				return REF_STOP;
			}
			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			switch ( lastUpdateID )
			{
			case kMaterialDynamic_assignMaterial:
				RefreshUI(kMaterialDynamic_message_assignMaterial);
				NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
				break;
			case kMaterialDynamic_assignID:
			case kMaterialDynamic_type:
			case kMaterialDynamic_randomizeAge:
			case kMaterialDynamic_randomizeRotoscoping:
				RefreshUI(kMaterialDynamic_message_assignID);
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealMtl();
			break;
		// Initialization of Dialog
		case kMaterialDynamic_RefMsg_InitDlg:
			// Set ICustButton properties for MaterialDynamic DAD button
			map = (IParamMap2*)partID;
			if (map != NULL) {
				HWND hWnd = map->GetHWnd();
				if (hWnd) {
					ICustButton *iBut = GetICustButton(GetDlgItem(hWnd, IDC_MATERIAL));
					iBut->SetDADMgr(_dadMgr());
					ReleaseICustButton(iBut);
				}
			}
			// Refresh UI
			RefreshUI(kMaterialDynamic_message_assignMaterial, map);
			RefreshUI(kMaterialDynamic_message_assignID, map);
			return REF_STOP;
		// New Random Seed
		case kMaterialDynamic_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		}
	}
	if ( hTarget == const_cast <Mtl*> (material()) ) {
		switch (message)
		{
		case REFMSG_CHANGE:
			return REF_STOP;
//			if (ignoreRefMsgNotify()) return REF_STOP;
//			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			break;
		case REFMSG_NODE_NAMECHANGE:
			if (HasDynamicName(dynamicNameSuffix)) {
				if (lastDynamicName() != dynamicNameSuffix) {
					_lastDynamicName() = dynamicNameSuffix;
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					UpdatePViewUI(kMaterialDynamic_material);
				}
			}
			return REF_STOP;
		case REFMSG_TARGET_DELETED:
			_material() = NULL;
			if (pblock() != NULL)
				pblock()->SetValue(kMaterialDynamic_material, 0, NULL);
			if (theHold.Holding()) {
				if (HasDynamicName(dynamicNameSuffix)) {
					if (lastDynamicName() != dynamicNameSuffix) {
						_lastDynamicName() = dynamicNameSuffix;
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(kMaterialDynamic_material);
					}
				}
			}
			break;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorMaterialDynamic::Clone(RemapDir &remap)
{
	PFOperatorMaterialDynamic* newOp = new PFOperatorMaterialDynamic();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	if (GetMaterial() != NULL) newOp->SetMaterial(GetMaterial());
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorMaterialDynamic::GetObjectName()
{
	return GetString(IDS_OPERATOR_MATERIALDYNAMIC_OBJECT_NAME);
}

bool PFOperatorMaterialDynamic::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool res = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	return res;
}

bool PFOperatorMaterialDynamic::Release(IObject* pCont)
{
	return PFSimpleAction::Release(pCont);
}

const ParticleChannelMask& PFOperatorMaterialDynamic::ChannelsUsed(const Interval& time) const
{
								// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart,
								// write channels
									PCU_EventStart|PCU_MtlIndex);
	return mask;
}

int	PFOperatorMaterialDynamic::GetRand()
{
	return pblock()->GetInt(kMaterialDynamic_randomSeed);
}

void PFOperatorMaterialDynamic::SetRand(int seed)
{
	pblock()->SetValue(kMaterialDynamic_randomSeed, 0, seed);
}

bool PFOperatorMaterialDynamic::IsMaterialHolder() const
{
	return (pblock()->GetInt(kMaterialDynamic_assignMaterial, 0) != 0);
}

Mtl* PFOperatorMaterialDynamic::GetMaterial()
{
	return _material();
}

bool PFOperatorMaterialDynamic::SetMaterial(Mtl* mtl)
{
	if (mtl == NULL) {
		return DeleteReference(kMaterialDynamic_reference_material) == REF_SUCCEED;
	} else {
		if (material())
			return ReplaceReference(kMaterialDynamic_reference_material, mtl) == REF_SUCCEED;
		else
			return MakeRefByID(FOREVER, kMaterialDynamic_reference_material, mtl) == REF_SUCCEED;
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorMaterialDynamic::GetCurrentState(IObject* pContainer)
{
	PFOperatorMaterialDynamicState* actionState = (PFOperatorMaterialDynamicState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorMaterialDynamicState_Class_ID);
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL)
		memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterialDynamic::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorMaterialDynamicState* actionState = (PFOperatorMaterialDynamicState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMaterialDynamic::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMaterialDynamic::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterialDynamic::HasDynamicName(TSTR& nameSuffix)
{
	bool assign = (pblock()->GetInt(kMaterialDynamic_assignMaterial, 0) != 0);
	if (!assign) return false;
	Mtl* mtl = GetMaterial();
	if (mtl == NULL) {
		nameSuffix = GetString(IDS_NONE);
	} else {
		nameSuffix = mtl->GetName();
	}
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFOperator								 |
//+--------------------------------------------------------------------------+
float GetIntegralValue(IParamBlock2* pblock, int paramID, 
					   const PreciseTimeValue& startV, const PreciseTimeValue& endV,
					   bool isAnimated, float valueAtZero)
{
	if (!isAnimated) {
		double sum = valueAtZero*(endV.fraction - startV.fraction);
		sum += valueAtZero*(endV.tick - startV.tick);
		float res = float(sum/TIME_TICKSPERSEC);
		return res;
	}

	PreciseTimeValue start, end;
	float multSign = (float(end - start) >= 0.f) ? 1.0f : -1.0f;
	if (multSign < 0) {
		start = endV;
		end = startV;
	} else {
		start = startV;
		end = endV;
	}

	int startTick = start.tick;
	int endTick = end.tick;
	double sum = 0.0;
	if (startTick == endTick) {
		sum += (end.fraction - start.fraction)*GetPFFloat(pblock, paramID, startTick);
	} else {
		sum += (0.5 - start.fraction)*GetPFFloat(pblock, paramID, startTick);
		sum += (end.fraction + 0.5)*GetPFFloat(pblock, paramID, endTick);
		for(int i=startTick+1; i<endTick; i++)
			sum += GetPFFloat(pblock, paramID, i);
	}

	float res = float(sum/TIME_TICKSPERSEC);
	res *= multSign;
	return res;
}

bool PFOperatorMaterialDynamic::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (pblock() == NULL) return false;
	int assignID = pblock()->GetInt(kMaterialDynamic_assignID, timeEnd);
	if (assignID == 0) return true; // nothing to assign

	int showInViewport = pblock()->GetInt(kMaterialDynamic_showInViewport, timeEnd);
	if (!showInViewport) { // check if the system is in render; if not then return
		IPFSystem* iSystem = GetPFSystemInterface(pSystem);
		if (iSystem == NULL) return false;
	}

	int type = pblock()->GetInt(kMaterialDynamic_type, timeEnd);
	int resetAge = pblock()->GetInt(kMaterialDynamic_resetAge, timeEnd);
	int randomizeAge = pblock()->GetInt(kMaterialDynamic_randomizeAge, timeEnd);
	TimeValue maxAgeOffset = pblock()->GetTimeValue(kMaterialDynamic_maxAgeOffset, timeEnd);
	if (type == kMaterialDynamic_type_particleID) { // this type is for post proceeding only
		// check if we need to do anything in proceed time
		bool doSomething = ((resetAge != 0) || ((randomizeAge != 0) && (maxAgeOffset > 0)));
		if (!doSomething)
			if (!postProceed()) return true;
	}

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	IParticleChannelPTVR* chBirthR = GetParticleChannelBirthTimeRInterface(pCont);
	if (chBirthR == NULL) return false; // can't read birth time
	IParticleChannelPTVW* chBirthW = GetParticleChannelBirthTimeWInterface(pCont); // to reset age if required by a param
	if (chBirthW == NULL) return false; // can't modify birth time
	
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire EventStart channel; if not present then create it
	bool initEventStart = false;
	IParticleChannelPTVR* chEventStartR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																	PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																	actionNode, NULL, &initEventStart);
	if (chEventStartR == NULL) return false; // can't read/create EventStart channel
	IParticleChannelPTVW* chEventStartW = NULL;
	if (initEventStart) {
		chEventStartW = GetParticleChannelEventStartWInterface(pCont);
		if (chEventStartW == NULL) return false; // can't modify EventStart channel
	}

	// ensure materialDynamic index channel
	IParticleChannelIntW* chMtlIDW = (IParticleChannelIntW*)chCont->EnsureInterface(PARTICLECHANNELMTLINDEXW_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELMTLINDEXR_INTERFACE,
																PARTICLECHANNELMTLINDEXW_INTERFACE, true);
	if (chMtlIDW == NULL) return false; // can't modify MaterialDynamic Index channel in the container
	IParticleChannelIntR* chMtlIDR = GetParticleChannelMtlIndexRInterface(pCont);

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);

	int numSubMtls = pblock()->GetInt(kMaterialDynamic_numSubMtls, timeEnd);
	if (numSubMtls < 1) numSubMtls = 1;
	int cycleLoop = pblock()->GetInt(kMaterialDynamic_loop, timeEnd);
	int syncType = pblock()->GetInt(kMaterialDynamic_sync, timeEnd);
	bool randomizeOffset = (pblock()->GetInt(kMaterialDynamic_randomizeRotoscoping, timeEnd) != 0);
	TimeValue randOffset = pblock()->GetTimeValue(kMaterialDynamic_maxRandOffset, timeEnd);
	int i, count = chAmount->Count();
	int mtlID;

	IParticleChannelFloatR* chIDOffsetR = NULL;
	IParticleChannelFloatW* chIDOffsetW = NULL;
	bool initIDOffset = false;
	if ((type == kMaterialDynamic_type_cycle) || (type == kMaterialDynamic_type_random))
	{
		chIDOffsetR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELIDOFFSETR_INTERFACE,
																ParticleChannelFloat_Class_ID,
																true, PARTICLECHANNELIDOFFSETR_INTERFACE,
																PARTICLECHANNELIDOFFSETW_INTERFACE, true,
																actionNode, (Object*)this, &initIDOffset);
		if (chIDOffsetR == NULL) return false;
		chIDOffsetW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELIDOFFSETW_INTERFACE, (Object*)this);
		if (chIDOffsetW == NULL) return false;
	}

	IParticleChannelIntR* chRandOffsetR = NULL;
	IParticleChannelIntW* chRandOffsetW = NULL;
	bool initRandOffset = false;
	if (((type == kMaterialDynamic_type_cycle) || (type == kMaterialDynamic_type_materialID))
		&& randomizeOffset && (randOffset > 0)) 
	{
		chRandOffsetR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELLOCALOFFSETR_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELLOCALOFFSETR_INTERFACE,
																PARTICLECHANNELLOCALOFFSETW_INTERFACE, true,
																actionNode, (Object*)this, &initRandOffset);
		if (chRandOffsetR == NULL) return false;
		chRandOffsetW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALOFFSETW_INTERFACE, (Object*)this);
		if (chRandOffsetW == NULL) return false;
	}

	Control* rateCtrl = pblock()->GetController(kMaterialDynamic_rate);
	bool isRateAnimated = (rateCtrl == NULL) ? false : (rateCtrl->IsAnimated() == TRUE);
	float rateAtZero = GetPFFloat(pblock(), kMaterialDynamic_rate, 0);

	if (initEventStart && (!chNew->IsAllOld())) {
		for(i=0; i<count; i++)
			if (chNew->IsNew(i)) {
				chEventStartW->SetValue(i, chTime->GetValue(i));
			}
	}

	if (initRandOffset && (!chNew->IsAllOld())) {
		for(i=0; i<count; i++)
			if (chNew->IsNew(i)) {
				chRandOffsetW->SetValue(i, randGen->Rand0X(randOffset) );
			}
	}

	if (initIDOffset && (!chNew->IsAllOld())) {
		for(i=0; i<count; i++) {
			if (!chNew->IsNew(i)) continue;
			if (type == kMaterialDynamic_type_cycle) {
				PreciseTimeValue curTime = chTime->GetValue(i);
				if (chRandOffsetR != NULL)
					curTime += PreciseTimeValue(chRandOffsetR->GetValue(i));
				float idOffset = 0.0f;
				PreciseTimeValue timeOrigin(0); // for absolute time
				switch(syncType) {
				case kMaterialDynamic_sync_age:
					timeOrigin = chBirthR->GetValue(i);
					break;
				case kMaterialDynamic_sync_event:
					timeOrigin = chEventStartR->GetValue(i);
					break;
				}
				idOffset = GetIntegralValue(pblock(), kMaterialDynamic_rate, 
											timeOrigin, curTime, isRateAnimated, rateAtZero);
				chIDOffsetW->SetValue(i, idOffset);
			} else { // random
				chIDOffsetW->SetValue(i, float(randGen->Rand0X(numSubMtls-1)));
			}
		}
	}

	PreciseTimeValue syncTime, syncStartTime, syncEndTime;
	switch(type) {
	case kMaterialDynamic_type_particleID:
		if (postProceed()) {
			for(i=0; i<count; i++) {
				mtlID = i;
				chMtlIDW->SetValue(i, mtlID);
			}
		} else {
			if (!chNew->IsAllOld()) {
				for(i=0; i<count; i++) {
					if (chNew->IsNew(i)) {
						PreciseTimeValue birthTime = chBirthR->GetValue(i);
						if (resetAge) birthTime = chEventStartR->GetValue(i);
						if (randomizeAge && (maxAgeOffset > 0))
							birthTime -= PreciseTimeValue(randGen->Rand0X(maxAgeOffset));
						chBirthW->SetValue(i, birthTime);
					}
				}
			}
		}
		break;
	case kMaterialDynamic_type_materialID:
		for(i=0; i<count; i++) {
			syncTime = timeEnd;
			switch(syncType) {
			case kMaterialDynamic_sync_age:
				syncTime -= chBirthR->GetValue(i);
				break;
			case kMaterialDynamic_sync_event:
				syncTime -= chEventStartR->GetValue(i);
				break;
			}
			if (chRandOffsetR != NULL)
				syncTime += PreciseTimeValue(chRandOffsetR->GetValue(i));
			mtlID = GetPFInt(pblock(), kMaterialDynamic_materialID, syncTime) - 1;
			if (mtlID < 0) mtlID = 0;
			chMtlIDW->SetValue(i, mtlID);
		}
		break;
	case kMaterialDynamic_type_cycle:
	case kMaterialDynamic_type_random:
		for(i=0; i<count; i++) {
			syncStartTime = chTime->GetValue(i);
			syncEndTime = timeEnd;
			switch(syncType) {
			case kMaterialDynamic_sync_age:
				syncStartTime -= chBirthR->GetValue(i);
				syncEndTime -= chBirthR->GetValue(i);
				break;
			case kMaterialDynamic_sync_event:
				syncStartTime -= chEventStartR->GetValue(i);
				syncEndTime -= chEventStartR->GetValue(i);
				break;
			}
			if (chRandOffsetR != NULL) {
				syncStartTime += PreciseTimeValue(chRandOffsetR->GetValue(i));
				syncEndTime += PreciseTimeValue(chRandOffsetR->GetValue(i));
			}
			float idOffset = GetIntegralValue(pblock(), kMaterialDynamic_rate, 
								syncStartTime, syncEndTime, isRateAnimated, rateAtZero);
			idOffset += chIDOffsetR->GetValue(i);
			if (type == kMaterialDynamic_type_cycle) {
				if (cycleLoop) {
					if (idOffset < 0.f)
						idOffset += numSubMtls*(floor(-idOffset/numSubMtls) + 1);
					if (idOffset >= numSubMtls)
						idOffset -= numSubMtls*(floor(idOffset/numSubMtls));
					chIDOffsetW->SetValue(i, idOffset);
				} else {
					chIDOffsetW->SetValue(i, idOffset);
					if (idOffset < 0.f) idOffset = 0.f;
					else if (idOffset >= numSubMtls) idOffset = numSubMtls-0.5f;
				}
			} else { // type: random
				int oldMtlID = int(floor(chIDOffsetR->GetValue(i)));
				int newMtlID = int(floor(idOffset));
				if (newMtlID != oldMtlID) {
					idOffset -= floor(idOffset);
					idOffset += float(randGen->Rand0X(numSubMtls-1));
				}
				chIDOffsetW->SetValue(i, idOffset);
			}
			mtlID = int(floor(idOffset));
			chMtlIDW->SetValue(i, mtlID);
		}
		break;
	}
	
	return true;
}

bool PFOperatorMaterialDynamic::HasPostProceed(IObject* pCont, PreciseTimeValue time)
{
	int animType = pblock()->GetInt(kMaterialDynamic_type, time);
	return (animType == kMaterialDynamic_type_particleID);
}

ClassDesc* PFOperatorMaterialDynamic::GetClassDesc() const
{
	return GetPFOperatorMaterialDynamicDesc();
}

bool PFOperatorMaterialDynamic::verifyMtlsMXSSync()
{
	if (pblock() == NULL) return true;
	return (_material() == pblock()->GetMtl(kMaterialDynamic_material, 0));
}

bool gUpdateDynamicMtlInProgress = false;
bool PFOperatorMaterialDynamic::updateFromRealMtl()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return true;
	if (gUpdateDynamicMtlInProgress) return false;
	if (verifyMtlsMXSSync()) return false;
	gUpdateDynamicMtlInProgress = true;
	if (pblock()->GetMtl(kMaterialDynamic_material) == NULL) {
		int numSubs = _material()->NumSubMtls();
		pblock()->SetValue(kMaterialDynamic_numSubMtls, 0, numSubs);
	}
	pblock()->SetValue(kMaterialDynamic_material, 0, _material());
	gUpdateDynamicMtlInProgress = false;
	return true;
}

bool PFOperatorMaterialDynamic::updateFromMXSMtl()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return true;
	if (gUpdateDynamicMtlInProgress) return false;
	if (verifyMtlsMXSSync()) return false;
	gUpdateDynamicMtlInProgress = true;
	SetMaterial(pblock()->GetMtl(kMaterialDynamic_material, 0));	
	gUpdateDynamicMtlInProgress = false;
	return true;
}

//+--------------------------------------------------------------------------+
//|							Drag-And-Drop Manager							 |
//+--------------------------------------------------------------------------+
BOOL PFOperatorMaterialDynamicDADMgr::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew)
{
	if (hfrom == hto) return FALSE;
	return (type==MATERIAL_CLASS_ID) ? TRUE : FALSE;
}

ReferenceTarget *PFOperatorMaterialDynamicDADMgr::GetInstance(HWND hwnd, POINT p, SClass_ID type)
{
	if (op() == NULL) return NULL;

	return _op()->GetMaterial();
}

void PFOperatorMaterialDynamicDADMgr::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type)
{
	Mtl *mtl = (Mtl *)dropThis;

	if (op() && mtl) {
		theHold.Begin();
		_op()->SetMaterial(mtl);
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
	}
}


} // end of namespace PFActions