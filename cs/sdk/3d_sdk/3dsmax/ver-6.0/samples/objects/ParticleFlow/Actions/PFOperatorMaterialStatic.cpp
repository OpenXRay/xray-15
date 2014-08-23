/**********************************************************************
 *<
	FILE:			PFOperatorMaterialStatic.cpp

	DESCRIPTION:	MaterialStatic Operator implementation
											 
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

#include "PFOperatorMaterialStatic.h"

#include "PFOperatorMaterialStatic_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMaterialStaticState					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorMaterialStaticState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialStaticState::IObject						 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterialStaticState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialStaticState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorMaterialStaticState::GetClassID() 
{ 
	return PFOperatorMaterialStaticState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMaterialStaticState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMaterialStaticState::Save(ISave* isave) const
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

	isave->BeginChunk(IPFActionState::kChunkData);
	float offset = cycleOffset();
	if ((res = isave->Write(&offset, sizeof(float), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData2);
	TimeValue time = offsetTime();
	if ((res = isave->Write(&time, sizeof(TimeValue), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorMaterialStaticState::IPFActionState					 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMaterialStaticState::Load(ILoad* iload)
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

		case IPFActionState::kChunkData:
			res=iload->Read(&_cycleOffset(), sizeof(float), &nb);
			break;

		case IPFActionState::kChunkData2:
			res=iload->Read(&_offsetTime(), sizeof(TimeValue), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMaterialStatic						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorMaterialStatic::PFOperatorMaterialStatic()
{
	_dadMgr() = new PFOperatorMaterialStaticDADMgr(this);
	GetClassDesc()->MakeAutoParamBlocks(this);
	// set the initial value for sub-mtls rate to the number of frames per second
	pblock()->SetValue(kMaterialStatic_ratePerSecond, 0, float(TIME_TICKSPERSEC)/GetTicksPerFrame() );
	_material() = NULL;
}

PFOperatorMaterialStatic::~PFOperatorMaterialStatic()
{
	if (dadMgr()) delete _dadMgr();
}

FPInterfaceDesc* PFOperatorMaterialStatic::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &materialStatic_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &materialStatic_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &materialStatic_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorMaterialStatic::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_MATERIALSTATIC_CLASS_NAME);
}

Class_ID PFOperatorMaterialStatic::ClassID()
{
	return PFOperatorMaterialStatic_Class_ID;
}

void PFOperatorMaterialStatic::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorMaterialStatic::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefTargetHandle PFOperatorMaterialStatic::GetReference(int i)
{
	switch (i)
	{
	case kMaterialStatic_reference_pblock:	return (RefTargetHandle)pblock();
	case kMaterialStatic_reference_material:	return (RefTargetHandle)material();
	}
	return NULL;
}

void PFOperatorMaterialStatic::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case kMaterialStatic_reference_pblock:	
		_pblock() = (IParamBlock2*)rtarg;	
		break;
	case kMaterialStatic_reference_material:	
		_material() = (Mtl *)rtarg;	
		if (updateFromRealMtl()) {
			NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
		}
		break;
	}
}

RefResult PFOperatorMaterialStatic::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
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
			if (lastUpdateID == kMaterialStatic_material) {
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
			case kMaterialStatic_assignMaterial:
				RefreshUI(kMaterialStatic_message_assignMaterial);
				NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
				NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				break;
			case kMaterialStatic_assignID:
				RefreshUI(kMaterialStatic_message_assignID);
				break;
			case kMaterialStatic_type:
				RefreshUI(kMaterialStatic_message_type);
				break;
			case kMaterialStatic_rateType:
				RefreshUI(kMaterialStatic_message_rateType);
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealMtl();
			break;
		// Initialization of Dialog
		case kMaterialStatic_RefMsg_InitDlg:
			// Set ICustButton properties for MaterialStatic DAD button
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
			RefreshUI(kMaterialStatic_message_assignMaterial);
			RefreshUI(kMaterialStatic_message_assignID);
			return REF_STOP;
		// New Random Seed
		case kMaterialStatic_RefMsg_NewRand:
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
					UpdatePViewUI(kMaterialStatic_material);
				}
			}
			return REF_STOP;
		case REFMSG_TARGET_DELETED:
			_material() = NULL;
			if (pblock() != NULL)
				pblock()->SetValue(kMaterialStatic_material, 0, NULL);
			if (theHold.Holding()) {
				if (HasDynamicName(dynamicNameSuffix)) {
					if (lastDynamicName() != dynamicNameSuffix) {
						_lastDynamicName() = dynamicNameSuffix;
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(kMaterialStatic_material);
					}
				}
			}
			break;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorMaterialStatic::Clone(RemapDir &remap)
{
	PFOperatorMaterialStatic* newOp = new PFOperatorMaterialStatic();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	if (GetMaterial() != NULL) newOp->SetMaterial(GetMaterial());
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorMaterialStatic::GetObjectName()
{
	return GetString(IDS_OPERATOR_MATERIALSTATIC_OBJECT_NAME);
}

bool PFOperatorMaterialStatic::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_cycleOffset(pCont) = 0.0f;
	_offsetTime(pCont) = TIME_NegInfinity;
	bool res = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);

	if (pblock()->GetInt(kMaterialStatic_type, 0) == kMaterialStatic_type_random) {
		int numSubMtls = pblock()->GetInt(kMaterialStatic_numSubMtls, 0);
		if (numSubMtls <= 0) numSubMtls = 1;
		_cycleOffset(pCont) = randLinker().GetRandGenerator(pCont)->Rand0X(numSubMtls-1);
	}

	return res;
}

bool PFOperatorMaterialStatic::Release(IObject* pCont)
{
	return PFSimpleAction::Release(pCont);
}

const ParticleChannelMask& PFOperatorMaterialStatic::ChannelsUsed(const Interval& time) const
{
								// read channels				// write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time, PCU_MtlIndex);
	return mask;
}

int	PFOperatorMaterialStatic::GetRand()
{
	return pblock()->GetInt(kMaterialStatic_randomSeed);
}

void PFOperatorMaterialStatic::SetRand(int seed)
{
	pblock()->SetValue(kMaterialStatic_randomSeed, 0, seed);
}

bool PFOperatorMaterialStatic::IsMaterialHolder() const
{
	return (pblock()->GetInt(kMaterialStatic_assignMaterial, 0) != 0);
}

Mtl* PFOperatorMaterialStatic::GetMaterial()
{
	return _material();
}

bool PFOperatorMaterialStatic::SetMaterial(Mtl* mtl)
{
	if (mtl == NULL) {
		return DeleteReference(kMaterialStatic_reference_material) == REF_SUCCEED;
	} else {
		if (material())
			return ReplaceReference(kMaterialStatic_reference_material, mtl) == REF_SUCCEED;
		else
			return MakeRefByID(FOREVER, kMaterialStatic_reference_material, mtl) == REF_SUCCEED;
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorMaterialStatic::GetCurrentState(IObject* pContainer)
{
	PFOperatorMaterialStaticState* actionState = (PFOperatorMaterialStaticState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorMaterialStaticState_Class_ID);
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL)
		memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
	actionState->_cycleOffset() = _cycleOffset(pContainer);
	actionState->_offsetTime() = _offsetTime(pContainer);
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorMaterialStatic::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorMaterialStaticState* actionState = (PFOperatorMaterialStaticState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
	}
	_cycleOffset(pContainer) = actionState->cycleOffset();
	_offsetTime(pContainer) = actionState->offsetTime();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMaterialStatic::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMaterialStatic::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MATERIAL_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorMaterialStatic::HasDynamicName(TSTR& nameSuffix)
{
	bool assign = (pblock()->GetInt(kMaterialStatic_assignMaterial, 0) != 0);
	if (!assign) return false;
	Mtl* mtl = GetMaterial();
	if (mtl == NULL) {
		nameSuffix = GetString(IDS_NONE);
	} else {
		nameSuffix = mtl->GetName();
	}
	return true;
}

bool PFOperatorMaterialStatic::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (pblock() == NULL) return false;
	int assignID = pblock()->GetInt(kMaterialStatic_assignID, timeEnd);
	if (assignID == 0) return true; // nothing to assign

	int showInViewport = pblock()->GetInt(kMaterialStatic_showInViewport, timeEnd);
	if (!showInViewport) { // check if the system is in render; if not then return
		IPFSystem* iSystem = GetPFSystemInterface(pSystem);
		if (iSystem == NULL) return false;
		if (!iSystem->IsRenderState()) return true; // nothing to show in viewport
	}

	int type = pblock()->GetInt(kMaterialStatic_type, timeEnd);

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int i, count = chAmount->Count();
	if (count == 0) return true; // no particles to modify
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// ensure materialStatic index channel
	IParticleChannelIntW* chMtlIDW = (IParticleChannelIntW*)chCont->EnsureInterface(PARTICLECHANNELMTLINDEXW_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELMTLINDEXR_INTERFACE,
																PARTICLECHANNELMTLINDEXW_INTERFACE, true);
	if (chMtlIDW == NULL) return false; // can't modify MaterialStatic Index channel in the container
	IParticleChannelIntR* chMtlIDR = GetParticleChannelMtlIndexRInterface(pCont);

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);

	int rateType = pblock()->GetInt(kMaterialStatic_rateType, timeEnd);
	int numSubMtls = pblock()->GetInt(kMaterialStatic_numSubMtls, timeEnd);
	if (numSubMtls < 1) numSubMtls = 1;
	int cycleLoop = pblock()->GetInt(kMaterialStatic_loop, timeEnd);
	float rateSec = pblock()->GetFloat(kMaterialStatic_ratePerSecond, timeEnd);
	float ratePart = pblock()->GetFloat(kMaterialStatic_ratePerParticle, timeEnd);
	int mtlID;

	bool useRatePerSec = ((type != kMaterialStatic_type_id) && (rateType == kMaterialStatic_rateType_second));
	bool useRatePerPart = ((type != kMaterialStatic_type_id) && (rateType == kMaterialStatic_rateType_particle));

	bool initRands = false;
	// recalc offset if necessary
	if (_offsetTime(pCont) == TIME_NegInfinity) {
		_offsetTime(pCont) = timeStart.TimeValue();
		// find the earliest time of the coming particles
		PreciseTimeValue minTime = chTime->GetValue(0);
		for(i=1; i<count; i++)
			if (minTime > chTime->GetValue(i)) 
				minTime = chTime->GetValue(i);
		if (useRatePerSec)
			if (type == kMaterialStatic_type_cycle)
				_cycleOffset(pCont) = float(PreciseTimeValue(_offsetTime(pCont)) - minTime)*rateSec/TIME_TICKSPERSEC;
		initRands = true;
	} else if (_offsetTime(pCont) != timeStart.TimeValue()) {
		if (useRatePerSec) {
			if (type == kMaterialStatic_type_cycle) {
				float timeDelta = float(timeStart.TimeValue() - _offsetTime(pCont));
				float addOffset = timeDelta*rateSec/TIME_TICKSPERSEC;
				_cycleOffset(pCont) += addOffset;
				if (_cycleOffset(pCont) >= numSubMtls) {
					if (cycleLoop) _cycleOffset(pCont) -= numSubMtls*int(_cycleOffset(pCont)/numSubMtls);
					else _cycleOffset(pCont) = numSubMtls - 1.0f;
				}
			}
		}
		_offsetTime(pCont) = timeStart.TimeValue();
		initRands = true;
	}
	if (initRands && useRatePerSec && (type == kMaterialStatic_type_random)) {
		int intervalDelta = int(timeEnd.TimeValue() - timeStart.TimeValue()) + 1;
		_randMtlIndex(pCont).SetCount(intervalDelta);
		float curOffset = _cycleOffset(pCont);
		int curMtlID = int(curOffset);
		_randMtlIndex(pCont)[0] = curMtlID;
		for(int i=1; i<intervalDelta; i++) {
			float addOffset = rateSec/TIME_TICKSPERSEC;
			curOffset += addOffset;
			if (int(curOffset) != curMtlID) {
				curOffset = randGen->Rand0X(numSubMtls-1) + (curOffset - floor(curOffset));
				curMtlID = int(curOffset);
			}
			_randMtlIndex(pCont)[i] = curMtlID;
			_cycleOffset(pCont) = curOffset;
		}
	}

	float curCycleOffset = _cycleOffset(pCont);
	float ratePerPart = pblock()->GetFloat(kMaterialStatic_ratePerParticle, timeEnd);

	for(i=0; i<count; i++) {
		if (!chNew->IsNew(i)) continue; // the ID is already set
		switch(type) {
		case kMaterialStatic_type_id:
			mtlID = GetPFInt(pblock(), kMaterialStatic_materialID, chTime->GetValue(i).TimeValue());
			mtlID--;
			break;
		case kMaterialStatic_type_cycle:
			if (rateType == kMaterialStatic_rateType_second) {
				float timeDelta = float(chTime->GetValue(i) - timeStart);
				float addOffset = timeDelta*rateSec/TIME_TICKSPERSEC;
				mtlID = int(curCycleOffset + addOffset);
				if (mtlID >= numSubMtls) {
					if (cycleLoop) mtlID = mtlID%numSubMtls;
					else mtlID = numSubMtls - 1;
				}
			} else { // per particle rate type
				mtlID = int(curCycleOffset);
				if (mtlID >= numSubMtls) {
					if (cycleLoop) mtlID = mtlID%numSubMtls;
					else mtlID = numSubMtls - 1;
				}
				if (ratePart > 0.0f) {
					curCycleOffset += 1.0f/ratePart;
					if (curCycleOffset >= numSubMtls) {
						if (cycleLoop) curCycleOffset -= numSubMtls*int(curCycleOffset/numSubMtls);
						else curCycleOffset = numSubMtls - 1.0f;
					}
				}
			}
			break;
		case kMaterialStatic_type_random:
			if (rateType == kMaterialStatic_rateType_second) {
				int timeDelta = int( chTime->GetValue(i) - timeStart.TimeValue() );
				mtlID = _randMtlIndex(pCont)[timeDelta];
			} else { // per particle rate type
				mtlID = int(curCycleOffset);
				int oldMtlID = mtlID;
				if (mtlID >= numSubMtls) {
					if (cycleLoop) mtlID = mtlID%numSubMtls;
					else mtlID = numSubMtls - 1;
				}
				if (ratePart > 0.0f) {
					curCycleOffset += 1.0f/ratePart;
					if (int(curCycleOffset) != oldMtlID)
						curCycleOffset = randGen->Rand0X(numSubMtls-1) + (curCycleOffset - floor(curCycleOffset));
				}
			}
			break;
		default: DbgAssert(0);
		}
		if (mtlID < 0) mtlID = 0;
		chMtlIDW->SetValue(i, mtlID);
	}

	if (useRatePerPart)
		_cycleOffset(pCont) = curCycleOffset;
	
	return true;
}

ClassDesc* PFOperatorMaterialStatic::GetClassDesc() const
{
	return GetPFOperatorMaterialStaticDesc();
}

bool PFOperatorMaterialStatic::verifyMtlsMXSSync()
{
	if (pblock() == NULL) return true;
	return (_material() == pblock()->GetMtl(kMaterialStatic_material, 0));
}

bool gUpdateStaticMtlInProgress = false;
bool PFOperatorMaterialStatic::updateFromRealMtl()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return true;
	if (gUpdateStaticMtlInProgress) return false;
	if (verifyMtlsMXSSync()) return false;
	gUpdateStaticMtlInProgress = true;
	if (pblock()->GetMtl(kMaterialStatic_material) == NULL) {
		int numSubs = _material()->NumSubMtls();
		pblock()->SetValue(kMaterialStatic_numSubMtls, 0, numSubs);
	}
	pblock()->SetValue(kMaterialStatic_material, 0, _material());
	gUpdateStaticMtlInProgress = false;
	return true;
}

bool PFOperatorMaterialStatic::updateFromMXSMtl()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return true;
	if (gUpdateStaticMtlInProgress) return false;
	if (verifyMtlsMXSSync()) return false;
	gUpdateStaticMtlInProgress = true;
	SetMaterial(pblock()->GetMtl(kMaterialStatic_material, 0));	
	gUpdateStaticMtlInProgress = false;
	return true;
}

//+--------------------------------------------------------------------------+
//|							Drag-And-Drop Manager							 |
//+--------------------------------------------------------------------------+
BOOL PFOperatorMaterialStaticDADMgr::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew)
{
	if (hfrom == hto) return FALSE;
	return (type==MATERIAL_CLASS_ID) ? TRUE : FALSE;
}

ReferenceTarget *PFOperatorMaterialStaticDADMgr::GetInstance(HWND hwnd, POINT p, SClass_ID type)
{
	if (op() == NULL) return NULL;

	return _op()->GetMaterial();
}

void PFOperatorMaterialStaticDADMgr::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type)
{
	Mtl *mtl = (Mtl *)dropThis;

	if (op() && mtl) {
		theHold.Begin();
		_op()->SetMaterial(mtl);
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
	}
}

} // end of namespace PFActions