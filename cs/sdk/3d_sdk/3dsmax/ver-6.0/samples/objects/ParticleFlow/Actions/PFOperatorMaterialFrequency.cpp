/**********************************************************************
 *<
	FILE:			PFOperatorMaterialFrequency.cpp

	DESCRIPTION:	MaterialFrequency Operator implementation
											 
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

#include "PFOperatorMaterialFrequency.h"

#include "PFOperatorMaterialFrequency_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Creates a non-transferable particle channel to store a float offset for materialFrequency index
#define PARTICLECHANNELIDOFFSETR_INTERFACE Interface_ID(0x52b76217, 0x1eb34500) 
#define PARTICLECHANNELIDOFFSETW_INTERFACE Interface_ID(0x52b76217, 0x1eb34501) 

#define GetParticleChannelIDOffsetRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELIDOFFSETR_INTERFACE)) 
#define GetParticleChannelIDOffsetWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELIDOFFSETW_INTERFACE)) 


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMaterialFrequencyState					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorMaterialFrequencyState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialFrequencyState::IObject						 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterialFrequencyState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialFrequencyState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorMaterialFrequencyState::GetClassID() 
{ 
	return PFOperatorMaterialFrequencyState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialFrequencyState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMaterialFrequencyState::Save(ISave* isave) const
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
//|			From PFOperatorMaterialFrequencyState::IPFActionState					 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMaterialFrequencyState::Load(ILoad* iload)
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
//|							From PFOperatorMaterialFrequency						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorMaterialFrequency::PFOperatorMaterialFrequency()
{
	_material() = NULL;
	_dadMgr() = new PFOperatorMaterialFrequencyDADMgr(this);
	_lastNumSubs() = 0;
	GetClassDesc()->MakeAutoParamBlocks(this);
}

PFOperatorMaterialFrequency::~PFOperatorMaterialFrequency()
{
	if (dadMgr()) delete _dadMgr();
}

FPInterfaceDesc* PFOperatorMaterialFrequency::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &materialFrequency_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &materialFrequency_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &materialFrequency_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorMaterialFrequency::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_MATERIALFREQUENCY_CLASS_NAME);
}

Class_ID PFOperatorMaterialFrequency::ClassID()
{
	return PFOperatorMaterialFrequency_Class_ID;
}

void PFOperatorMaterialFrequency::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorMaterialFrequency::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefTargetHandle PFOperatorMaterialFrequency::GetReference(int i)
{
	switch (i)
	{
	case kMaterialFrequency_reference_pblock:	return (RefTargetHandle)pblock();
	case kMaterialFrequency_reference_material:	return (RefTargetHandle)material();
	}
	return NULL;
}

void PFOperatorMaterialFrequency::SetReference(int i, RefTargetHandle rtarg)
{
	bool doUIRefresh = false;

	switch (i)
	{
	case kMaterialFrequency_reference_pblock:	
		_pblock() = (IParamBlock2*)rtarg;	
		break;
	case kMaterialFrequency_reference_material:	
		_material() = (Mtl *)rtarg;			
		if (updateFromRealMtl()) {
			NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
		}
		break;
	}
}

RefResult PFOperatorMaterialFrequency::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
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
			if (lastUpdateID == kMaterialFrequency_material) {
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
			case kMaterialFrequency_assignMaterial:
				RefreshUI(kMaterialFrequency_message_assignMaterial);
				NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
				break;
			case kMaterialFrequency_assignID:
				RefreshUI(kMaterialFrequency_message_assignID);
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealMtl();
			break;
		// Initialization of Dialog
		case kMaterialFrequency_RefMsg_InitDlg:
			// Set ICustButton properties for MaterialFrequency DAD button
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
			RefreshUI(kMaterialFrequency_message_assignMaterial);
			RefreshUI(kMaterialFrequency_message_numSubMtls);
			RefreshUI(kMaterialFrequency_message_assignID);
			return REF_STOP;
		// New Random Seed
		case kMaterialFrequency_RefMsg_NewRand:
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
			{
				if (pblock() != NULL) {
					Mtl* curMtl = pblock()->GetMtl(kMaterialFrequency_material);
					int curNumSubs = (curMtl != NULL) ? curMtl->NumSubMtls() : 0;
					if (curNumSubs != lastNumSubs()) {
						_lastNumSubs() = curNumSubs;
						RefreshUI(kMaterialFrequency_message_numSubMtls);
					}
				}
			}
			return REF_STOP;
//			if (ignoreRefMsgNotify()) return REF_STOP;
//			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			break;
		case REFMSG_NODE_NAMECHANGE:
			if (HasDynamicName(dynamicNameSuffix)) {
				if (lastDynamicName() != dynamicNameSuffix) {
					_lastDynamicName() = dynamicNameSuffix;
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					UpdatePViewUI(kMaterialFrequency_material);
				}
			}
			return REF_STOP;
		case REFMSG_TARGET_DELETED:
			_material() = NULL;
			if (pblock() != NULL)
				pblock()->SetValue(kMaterialFrequency_material, 0, NULL);
			if (theHold.Holding()) {
				if (HasDynamicName(dynamicNameSuffix)) {
					if (lastDynamicName() != dynamicNameSuffix) {
						_lastDynamicName() = dynamicNameSuffix;
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(kMaterialFrequency_material);
					}
				}
			}
			break;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorMaterialFrequency::Clone(RemapDir &remap)
{
	PFOperatorMaterialFrequency* newOp = new PFOperatorMaterialFrequency();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	if (GetMaterial() != NULL) newOp->SetMaterial(GetMaterial());
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorMaterialFrequency::GetObjectName()
{
	return GetString(IDS_OPERATOR_MATERIALFREQUENCY_OBJECT_NAME);
}

bool PFOperatorMaterialFrequency::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool res = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);

	return res;
}

bool PFOperatorMaterialFrequency::Release(IObject* pCont)
{
	return PFSimpleAction::Release(pCont);
}

const ParticleChannelMask& PFOperatorMaterialFrequency::ChannelsUsed(const Interval& time) const
{
								// read channels				// write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time, PCU_MtlIndex);
	return mask;
}

int	PFOperatorMaterialFrequency::GetRand()
{
	return pblock()->GetInt(kMaterialFrequency_randomSeed);
}

void PFOperatorMaterialFrequency::SetRand(int seed)
{
	pblock()->SetValue(kMaterialFrequency_randomSeed, 0, seed);
}

bool PFOperatorMaterialFrequency::IsMaterialHolder() const
{
	return (pblock()->GetInt(kMaterialFrequency_assignMaterial, 0) != 0);
}

Mtl* PFOperatorMaterialFrequency::GetMaterial()
{
	return _material();
}

bool PFOperatorMaterialFrequency::SetMaterial(Mtl* mtl)
{
	if (mtl == NULL) {
		return DeleteReference(kMaterialFrequency_reference_material) == REF_SUCCEED;
	} else {
		if (material())
			return ReplaceReference(kMaterialFrequency_reference_material, mtl) == REF_SUCCEED;
		else
			return MakeRefByID(FOREVER, kMaterialFrequency_reference_material, mtl) == REF_SUCCEED;
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorMaterialFrequency::GetCurrentState(IObject* pContainer)
{
	PFOperatorMaterialFrequencyState* actionState = (PFOperatorMaterialFrequencyState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorMaterialFrequencyState_Class_ID);
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL)
		memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterialFrequency::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorMaterialFrequencyState* actionState = (PFOperatorMaterialFrequencyState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMaterialFrequency::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMaterialFrequency::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterialFrequency::HasDynamicName(TSTR& nameSuffix)
{
	bool assign = (pblock()->GetInt(kMaterialFrequency_assignMaterial, 0) != 0);
	if (!assign) return false;
	Mtl* mtl = GetMaterial();
	if (mtl == NULL) {
		nameSuffix = GetString(IDS_NONE);
	} else {
		nameSuffix = mtl->GetName();
	}
	return true;
}

bool PFOperatorMaterialFrequency::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (pblock() == NULL) return false;
	int assignID = pblock()->GetInt(kMaterialFrequency_assignID, timeEnd);
	if (assignID == 0) return true; // nothing to assign

	int showInViewport = pblock()->GetInt(kMaterialFrequency_showInViewport, timeEnd);
	if (!showInViewport) { // check if the system is in render; if not then return
		IPFSystem* iSystem = GetPFSystemInterface(pSystem);
		if (iSystem == NULL) return false;
		if (!iSystem->IsRenderState()) return true; // nothing to show in viewport
	}

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// ensure material index channel
	IParticleChannelIntW* chMtlIDW = (IParticleChannelIntW*)chCont->EnsureInterface(PARTICLECHANNELMTLINDEXW_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELMTLINDEXR_INTERFACE,
																PARTICLECHANNELMTLINDEXW_INTERFACE, true);
	if (chMtlIDW == NULL) return false; // can't modify Material Index channel in the container
	IParticleChannelIntR* chMtlIDR = GetParticleChannelMtlIndexRInterface(pCont);

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);

	int i, j, count = chAmount->Count();
	int mtlID;
	float idShare[10], slideShare[10];
	int pblockIDShare[] = { kMaterialFrequency_id1, kMaterialFrequency_id2, kMaterialFrequency_id3, kMaterialFrequency_id4, kMaterialFrequency_id5, kMaterialFrequency_id6, kMaterialFrequency_id7, kMaterialFrequency_id8, kMaterialFrequency_id9, kMaterialFrequency_id10 };

	for(i=0; i<count; i++) {
		if (!chNew->IsNew(i)) continue; // the ID is already set
		TimeValue curTime = chTime->GetValue(i).TimeValue();
		float totalShare = 0.0f;
		for(j=0; j<10; j++) {
			totalShare += (idShare[j] = GetPFFloat(pblock(), pblockIDShare[j], curTime));
			slideShare[j] = totalShare;
		}
		float randomShare = totalShare*randGen->Rand01();
		for(j=0; j<10; j++) {
			mtlID = j;
			if (randomShare < slideShare[j]) break;
		}
		chMtlIDW->SetValue(i, mtlID);
	}
	
	return true;
}

ClassDesc* PFOperatorMaterialFrequency::GetClassDesc() const
{
	return GetPFOperatorMaterialFrequencyDesc();
}

bool PFOperatorMaterialFrequency::verifyMtlsMXSSync()
{
	if (pblock() == NULL) return true;
	return (_material() == pblock()->GetMtl(kMaterialFrequency_material, 0));
}

bool gUpdateFrequencyMtlInProgress = false;
bool PFOperatorMaterialFrequency::updateFromRealMtl()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return true;
	if (gUpdateFrequencyMtlInProgress) return false;
	if (verifyMtlsMXSSync()) return false;
	gUpdateFrequencyMtlInProgress = true;
	pblock()->SetValue(kMaterialFrequency_material, 0, _material());
	int curNumSubs = (material() != NULL) ? _material()->NumSubMtls() : 0;
	if (curNumSubs != lastNumSubs()) {
		_lastNumSubs() = curNumSubs;
		RefreshUI(kMaterialFrequency_message_numSubMtls);
	}
	RefreshUI(kMaterialFrequency_message_numSubMtls);
	gUpdateFrequencyMtlInProgress = false;
	return true;
}

bool PFOperatorMaterialFrequency::updateFromMXSMtl()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return true;
	if (gUpdateFrequencyMtlInProgress) return false;
	if (verifyMtlsMXSSync()) return false;
	gUpdateFrequencyMtlInProgress = true;
	SetMaterial(pblock()->GetMtl(kMaterialFrequency_material, 0));	
	gUpdateFrequencyMtlInProgress = false;
	return true;
}

//+--------------------------------------------------------------------------+
//|							Drag-And-Drop Manager							 |
//+--------------------------------------------------------------------------+
BOOL PFOperatorMaterialFrequencyDADMgr::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew)
{
	if (hfrom == hto) return FALSE;
	return (type==MATERIAL_CLASS_ID) ? TRUE : FALSE;
}

ReferenceTarget *PFOperatorMaterialFrequencyDADMgr::GetInstance(HWND hwnd, POINT p, SClass_ID type)
{
	if (op() == NULL) return NULL;

	return _op()->GetMaterial();
}

void PFOperatorMaterialFrequencyDADMgr::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type)
{
	Mtl *mtl = (Mtl *)dropThis;

	if (op() && mtl) {
		theHold.Begin();
		_op()->SetMaterial(mtl);
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
	}
}


} // end of namespace PFActions