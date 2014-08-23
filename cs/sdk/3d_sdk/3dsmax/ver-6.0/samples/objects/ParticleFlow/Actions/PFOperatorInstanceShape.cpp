/**********************************************************************
 *<
	FILE:			PFOperatorInstanceShape.cpp

	DESCRIPTION:	InstanceShape Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-04-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "max.h"
#include "meshdlib.h"
//#include "imtl.h"
#include "stdmat.h"

#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorInstanceShape.h"

#include "PFOperatorInstanceShape_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "IPFActionList.h"

namespace PFActions {

// Creates a non-transferable particle channel to store a local index (to indicate which
// shape is used if there're multiple shapes) for each particle
// when "Animated Shape" is turned ON.
// So that when Proceed is called the same shape can be assigned to those old particles.
#define PARTICLECHANNELLOCALINDEXR_INTERFACE Interface_ID(0x1b6b260c, 0x1eb34500) 
#define PARTICLECHANNELLOCALINDEXW_INTERFACE Interface_ID(0x1b6b260c, 0x1eb34501) 

#define GetParticleChannelLocalIndexRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELLOCALINDEXR_INTERFACE)) 
#define GetParticleChannelLocalIndexWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELLOCALINDEXW_INTERFACE)) 

// Creates a non-transferable particle channel to store a time offset for each particle
// when "Rand Offset" is enabled.
#define PARTICLECHANNELLOCALOFFSETR_INTERFACE Interface_ID(0x12ec5d1d, 0x1eb34500) 
#define PARTICLECHANNELLOCALOFFSETW_INTERFACE Interface_ID(0x12ec5d1d, 0x1eb34501) 

#define GetParticleChannelLocalOffsetRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELLOCALOFFSETR_INTERFACE)) 
#define GetParticleChannelLocalOffsetWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELLOCALOFFSETW_INTERFACE)) 

// Initialize static data member
const float PFOperatorInstanceShape::kMaxVariation = 0.995f;

// Pick node call back
PFOperatorInstanceShapePickNodeCallback ThePFOperatorInstanceShapePickNodeCallback;
// Pick mode call back
PFOperatorInstanceShapePickModeCallback ThePFOperatorInstanceShapePickModeCallback;

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorInstanceShapeState				 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorInstanceShapeState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorInstanceShapeState::IObject					 |
//+--------------------------------------------------------------------------+
void PFOperatorInstanceShapeState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorInstanceShapeState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorInstanceShapeState::GetClassID() 
{ 
	return PFOperatorInstanceShapeState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorInstanceShapeState::IPFActionState			 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorInstanceShapeState::Save(ISave* isave) const
{
	ULONG nb;
	IOResult res;

	isave->BeginChunk(IPFActionState::kChunkActionHandle);
	ULONG handle = actionHandle();
	if ((res = isave->Write(&handle, sizeof(ULONG), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkRandGen2);
	if ((res = isave->Write(randGenVar(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkRandGen3);
	if ((res = isave->Write(randGenOrd(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkRandGen4);
	if ((res = isave->Write(randGenSyn(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData);
	int index = shapeIndex();
	if ((res = isave->Write(&index, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorInstanceShapeState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorInstanceShapeState::Load(ILoad* iload)
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

		case IPFActionState::kChunkRandGen2:
			res=iload->Read(_randGenVar(), sizeof(RandGenerator), &nb);
			break;

		case IPFActionState::kChunkRandGen3:
			res=iload->Read(_randGenOrd(), sizeof(RandGenerator), &nb);
			break;

		case IPFActionState::kChunkRandGen4:
			res=iload->Read(_randGenSyn(), sizeof(RandGenerator), &nb);
			break;

		case IPFActionState::kChunkData:
			res=iload->Read(&_shapeIndex(), sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorInstanceShape					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorInstanceShape::PFOperatorInstanceShape()
{ 
	_object() = NULL;
	_material() = NULL;
	_meshNeedsUpdate() = true;
	_registeredMtlChangeNotification() = false;
	_resetingObjectList() = false;
	GetClassDesc()->MakeAutoParamBlocks(this);
}

PFOperatorInstanceShape::~PFOperatorInstanceShape()
{ 
	updateRegMtlChangeNotification(true);
	for (int i = 0; i < meshData().Count(); i++)
		if (meshData(i))	delete _meshData(i);
}

FPInterfaceDesc* PFOperatorInstanceShape::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &instanceShape_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &instanceShape_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &instanceShape_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorInstanceShape::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_INSTANCESHAPE_CLASS_NAME);
}

Class_ID PFOperatorInstanceShape::ClassID()
{
	return PFOperatorInstanceShape_Class_ID;
}

void PFOperatorInstanceShape::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorInstanceShape::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefTargetHandle PFOperatorInstanceShape::GetReference(int i)
{
	switch (i)
	{
	case kInstanceShape_reference_pblock:	return (RefTargetHandle)pblock();
	case kInstanceShape_reference_object:	return (RefTargetHandle)object();
	case kInstanceShape_reference_material:	return (RefTargetHandle)material();
	}
	return NULL;
}

void PFOperatorInstanceShape::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case kInstanceShape_reference_pblock:	
		_pblock() = (IParamBlock2*)rtarg;	
		updateRegMtlChangeNotification();
		break;
	case kInstanceShape_reference_object:	
		_object() = (INode *)rtarg;			
		if (!theHold.RestoreOrRedoing()) {
			resetObjectList();
			BuildMeshes(GetCOREInterface()->GetTime(), true);
			updateFromRealRefObject();
		}
		updateRegMtlChangeNotification();
		RefreshUI(kInstanceShape_message_name);
		RefreshUI(kInstanceShape_message_type);
		NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
		NotifyDependents(FOREVER, 0, REFMSG_CHANGE, NOTIFY_ALL, TRUE);
		break;
	case kInstanceShape_reference_material:	
		_material() = (Mtl *)rtarg;			
		if (theHold.Redoing()) {
			BuildMeshes(GetCOREInterface()->GetTime(), true);
		}
		break;
	}
}

RefResult PFOperatorInstanceShape::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
													 PartID& partID, RefMessage message)
{
	if ( hTarget == pblock() ) {
		int lastUpdateID;
		switch (message)
		{
		case REFMSG_CHANGE:
			lastUpdateID = pblock()->LastNotifyParamID();
			if (lastUpdateID == kInstanceShape_objectMaxscript) {
				if (IsIgnoringRefNodeChange()) return REF_STOP;
				updateFromMXSRefObject();
				return REF_STOP;
			}
			if (lastUpdateID == kInstanceShape_objectList) {
				if (resetingObjectList()) return REF_STOP;
				if (IsIgnoringRefNodeChange()) return REF_STOP;
//				if (theHold.Holding()) {
//					if ((partID == PART_TM) || (partID == PART_OBJECT_TYPE) ||
//						(partID == PART_TOPO) || (partID == PART_GEOM))  {
//						BuildMeshes(GetCOREInterface()->GetTime(), true);
//					} else if ((partID == PART_MTL) || (partID == PART_TEXMAP)) {
//						if (((partID == PART_MTL) && (pblock()->GetInt(kInstanceShape_material, 0) != 0))
//							|| ((partID == PART_TEXMAP) && (pblock()->GetInt(kInstanceShape_mapping, 0) != 0)))
//						BuildMeshes(GetCOREInterface()->GetTime(), true);
//					} else {
//						_meshNeedsUpdate() = true;
//					}
//				} else {
					_meshNeedsUpdate() = true;
//				}
				return REF_SUCCEED;
			}
			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			switch ( lastUpdateID )
			{
			case kInstanceShape_sepGroup:
			case kInstanceShape_sepHierarchy:
			case kInstanceShape_sepElements:
				BuildMeshes(GetCOREInterface()->GetTime(), true);
				RefreshUI(kInstanceShape_message_type);
				break;
			case kInstanceShape_mapping:
				BuildMeshes(GetCOREInterface()->GetTime(), true);
				break;
			case kInstanceShape_material:
				if (pblock()->GetInt(kInstanceShape_material, 0)) {
					BuildMeshes(GetCOREInterface()->GetTime(), true);
					updateRegMtlChangeNotification();
				} else {
					SetMaterial(NULL);
					updateRegMtlChangeNotification(true);
					NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
				}
				// no need to refresh UI since only material or mapping is changed
				break;
			case kInstanceShape_animatedShape:
			case kInstanceShape_syncRandom:
				RefreshUI(kInstanceShape_message_animate);
				break;
			case kInstanceShape_setScale:
				RefreshUI(kInstanceShape_message_setScale);
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealRefObject();
			break;
		case REFMSG_NODE_MATERIAL_CHANGED:
			if (IsIgnoringRefNodeChange()) return REF_STOP;
			if (resetingObjectList()) return REF_STOP;
			if (!theHold.Holding()) return REF_STOP;
			if (pblock()->GetInt(kInstanceShape_material, 0) != 0) {
				Mtl* oldMtl = GetMaterial();
				BuildMeshes(GetCOREInterface()->GetTime(), true);
				Mtl* newMtl = GetMaterial();
				if (oldMtl != newMtl) // big change; may need re-proceed particles
					NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
			break;
		// Initialization of Dialog
		case kInstanceShape_RefMsg_InitDlg:
			RefreshUI(kInstanceShape_message_name, (IParamMap2*)partID);
			RefreshUI(kInstanceShape_message_type, (IParamMap2*)partID);
			RefreshUI(kInstanceShape_message_animate, (IParamMap2*)partID);
			RefreshUI(kInstanceShape_message_setScale, (IParamMap2*)partID);
			return REF_STOP;
		// Pick Geometry Object for Particle Shape
		case kInstanceShape_RefMsg_PickObj:
			ThePFOperatorInstanceShapePickModeCallback.Init(this);
//			ip()->SetPickMode(&ThePFOperatorInstanceShapePickModeCallback);
			GetCOREInterface()->SetPickMode(&ThePFOperatorInstanceShapePickModeCallback);
			return REF_STOP;
		case kInstanceShape_RefMsg_UpdateShape:
//			resetObjectList(); // not sure if we need that here (bayboro 03-12-2003
			BuildMeshes(GetCOREInterface()->GetTime(), true);
			RefreshUI(kInstanceShape_message_type);
			NotifyDependents(FOREVER, 0, REFMSG_CHANGE, NOTIFY_ALL, TRUE);
			return REF_STOP;
		// New Random Seed
		case kInstanceShape_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		}
	}

	if ( hTarget == const_cast <INode*> (object()) ) {
		switch (message)
		{
		case REFMSG_CHANGE:
			if (IsIgnoringRefNodeChange()) return REF_STOP;
//			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;

//			if (theHold.Holding()) {
//				if ((partID == PART_TM) || (partID == PART_OBJECT_TYPE) ||
//					/*(partID == PART_TOPO) ||*/ (partID == PART_GEOM))  {
//					BuildMeshes(GetCOREInterface()->GetTime(), true);
//				} else if ((partID == PART_MTL) || (partID == PART_TEXMAP)) {
//					if (((partID == PART_MTL) && (pblock()->GetInt(kInstanceShape_material, 0) != 0))
//						|| ((partID == PART_TEXMAP) && (pblock()->GetInt(kInstanceShape_mapping, 0) != 0)))
//					BuildMeshes(GetCOREInterface()->GetTime(), true);
//				} else {
//					_meshNeedsUpdate() = true;
//				}
//			} else {
				_meshNeedsUpdate() = true;
//			}

//			BuildMeshes(GetCOREInterface()->GetTime(), true);
			break;
		case REFMSG_NODE_LINK:
			if (IsIgnoringRefNodeChange()) return REF_STOP;
			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			_meshNeedsUpdate() = true;
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			break;
		case REFMSG_NODE_MATERIAL_CHANGED:
			if (IsIgnoringRefNodeChange()) return REF_STOP;
			if (!(theHold.Holding())) return REF_STOP;
			if (pblock()->GetInt(kInstanceShape_material, 0) != 0) {
				Mtl* oldMtl = GetMaterial();
				BuildMeshes(GetCOREInterface()->GetTime(), true);
				Mtl* newMtl = GetMaterial();
				if (oldMtl != newMtl) // big change; may need re-proceed particles
					NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			RefreshUI(kInstanceShape_message_name);
			break;
		case REFMSG_TARGET_DELETED:
			_object() = NULL;
			RefreshUI(kInstanceShape_message_name);
			updateRegMtlChangeNotification(true);
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			break;
		}
	}

	if ( hTarget == const_cast <Mtl*> (material()) ) {
		switch (message)
		{
		case REFMSG_CHANGE:
//			if (ignoreRefMsgNotify()) return REF_STOP;
			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			break;
		case REFMSG_TARGET_DELETED:
			_material() = NULL;
			break;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorInstanceShape::Clone(RemapDir &remap)
{
	PFOperatorInstanceShape* newOp = new PFOperatorInstanceShape();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	if (object()) newOp->ReplaceReference(kInstanceShape_reference_object, _object());
	if (material()) newOp->ReplaceReference(kInstanceShape_reference_material, _material());
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorInstanceShape::GetObjectName()
{
	return GetString(IDS_OPERATOR_INSTANCESHAPE_OBJECT_NAME);
}

bool PFOperatorInstanceShape::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	IPFActionList* iList = PFActionListInterface(pSystem);
	if (iList != NULL) {
		if (iList->IsActivated()) {
			BOOL animatedShape = pblock()->GetInt(kInstanceShape_animatedShape);
			BOOL acquireShape = pblock()->GetInt(kInstanceShape_acquireShape);
			// initialize mesh data
			if ((!animatedShape && !acquireShape) || meshNeedsUpdate()) 
				BuildMeshes(0, meshNeedsUpdate());
		}
	}
	_shapeIndex(pCont) = 0;

	// initialize random number generators
	return	_randLinkerVar().Init( pCont, GetRand() ) &&
			_randLinkerOrd().Init( pCont, GetRand() ) &&
			_randLinkerSyn().Init( pCont, GetRand() );
}

bool PFOperatorInstanceShape::Release(IObject* pCont)
{
	// release random number generators
	_randLinkerVar().Release( pCont );
	_randLinkerOrd().Release( pCont );
	_randLinkerSyn().Release( pCont );
	return true;
}

const ParticleChannelMask& PFOperatorInstanceShape::ChannelsUsed(const Interval& time) const
{
								// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart,
								// write channels
									PCU_EventStart|PCU_Scale|PCU_Shape);
	return mask;
}

int	PFOperatorInstanceShape::GetRand()
{
	return pblock()->GetInt(kInstanceShape_randomSeed);
}

void PFOperatorInstanceShape::SetRand(int seed)
{
	pblock()->SetValue(kInstanceShape_randomSeed, 0, seed);
}

Mtl* PFOperatorInstanceShape::GetMaterial()
{
	return _material();
}

bool PFOperatorInstanceShape::SetMaterial(Mtl* mtl)
{
	if (mtl == NULL) {
		return DeleteReference(kInstanceShape_reference_material) == REF_SUCCEED;
	} else {
		if (material())
			return ReplaceReference(kInstanceShape_reference_material, mtl) == REF_SUCCEED;
		else
			return MakeRefByID(FOREVER, kInstanceShape_reference_material, mtl) == REF_SUCCEED;
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorInstanceShape::GetCurrentState(IObject* pContainer)
{
	PFOperatorInstanceShapeState* actionState = (PFOperatorInstanceShapeState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorInstanceShapeState_Class_ID);
	RandGenerator* randGenVar = randLinkerVar().GetRandGenerator(pContainer);
	if (randGenVar != NULL)
		memcpy(actionState->_randGenVar(), randGenVar, sizeof(RandGenerator));
	RandGenerator* randGenOrd = randLinkerOrd().GetRandGenerator(pContainer);
	if (randGenOrd != NULL)
		memcpy(actionState->_randGenOrd(), randGenOrd, sizeof(RandGenerator));
	RandGenerator* randGenSyn = randLinkerSyn().GetRandGenerator(pContainer);
	if (randGenSyn != NULL)
		memcpy(actionState->_randGenSyn(), randGenSyn, sizeof(RandGenerator));
	actionState->_shapeIndex() = _shapeIndex(pContainer);
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorInstanceShape::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorInstanceShapeState* actionState = (PFOperatorInstanceShapeState*)aSt;
	RandGenerator* randGen = randLinkerVar().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenVar(), sizeof(RandGenerator));
	}
	randGen = randLinkerOrd().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenOrd(), sizeof(RandGenerator));
	}
	randGen = randLinkerSyn().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenSyn(), sizeof(RandGenerator));
	}
	_shapeIndex(pContainer) = actionState->shapeIndex();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorInstanceShape::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_INSTANCESHAPE_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorInstanceShape::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_INSTANCESHAPE_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorInstanceShape::HasDynamicName(TSTR& nameSuffix)
{
	if (object() == NULL) nameSuffix = GetString(IDS_NONE);
	else nameSuffix = _object()->GetName();
	return true;
}

bool PFOperatorInstanceShape::Proceed(IObject* pCont, 
									   PreciseTimeValue timeStart, 
									   PreciseTimeValue& timeEnd,
									   Object* pSystem,
									   INode* pNode,
									   INode* actionNode,
									   IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int amount = chAmount->Count();
	if (amount <= 0) return true; // no particles to proceed

	float scale = pblock()->GetFloat(kInstanceShape_scale);
	float variation = pblock()->GetFloat(kInstanceShape_variation);
	BOOL animatedShape = pblock()->GetInt(kInstanceShape_animatedShape);
	BOOL acquireShape = pblock()->GetInt(kInstanceShape_acquireShape);
	BOOL randomOrder = pblock()->GetInt(kInstanceShape_randomShape);
	int syncType = pblock()->GetInt(kInstanceShape_syncType);
	BOOL syncRandom = pblock()->GetInt(kInstanceShape_syncRandom);
	TimeValue randOffset = pblock()->GetTimeValue(kInstanceShape_randomOffset);
	bool setScale = (pblock()->GetInt(kInstanceShape_setScale) != 0);
	variation = min(variation, kMaxVariation);
	int useSepGroups = pblock()->GetInt(kInstanceShape_sepGroup);
	int useSepHierarchy = pblock()->GetInt(kInstanceShape_sepHierarchy);
	int useSepElements = pblock()->GetInt(kInstanceShape_sepElements);
	bool useMultipleShapes = ((useSepGroups != 0) || (useSepHierarchy != 0) || (useSepElements != 0));

	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false;	// can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false;	// can't find particle times in the container

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;	// can't get access to ChannelContainer interface

	IParticleChannelPTVR* chBirthTime = NULL;
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (animatedShape) {
		if (syncType == kInstanceShape_syncBy_particleAge) {
			chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
			if (chBirthTime == NULL) return false; // can't read particle age
		}
		if (syncType == kInstanceShape_syncBy_eventStart) {
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
	}

	IParticleChannelPoint3W* chScale = NULL;
	if (setScale) {
		chScale = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSCALEW_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSCALER_INTERFACE,
																PARTICLECHANNELSCALEW_INTERFACE, true );
		if (chScale == NULL) return false;	// can't modify Scale channel in the container
	}

	IParticleChannelMeshW* chMesh = NULL;
	chMesh = (IParticleChannelMeshW*)chCont->EnsureInterface(PARTICLECHANNELSHAPEW_INTERFACE,
																ParticleChannelMesh_Class_ID,
																true, PARTICLECHANNELSHAPER_INTERFACE,
																PARTICLECHANNELSHAPEW_INTERFACE, true );
	if (chMesh == NULL) return false;	// can't modify Shape channel in the container

	IParticleChannelIntR* chLocalIdxR = NULL;
	IParticleChannelIntW* chLocalIdxW = NULL;
	IParticleChannelIntR* chLocalOffR = NULL;
	IParticleChannelIntW* chLocalOffW = NULL;
	bool initLocalIdx = false;
	bool initLocalOff = false;
	if (animatedShape || useMultipleShapes) {
		// acquire LocalIndex particle channel; if not present then create it.		
		chLocalIdxR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELLOCALINDEXR_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELLOCALINDEXR_INTERFACE,
																PARTICLECHANNELLOCALINDEXW_INTERFACE, true,
																actionNode, (Object*)this, &initLocalIdx);
		if (chLocalIdxR == NULL) return false;
		chLocalIdxW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALINDEXW_INTERFACE, (Object*)this);
		if (chLocalIdxW == NULL) return false;
	}
	if (animatedShape) {
		// acquire LocalOffset particle channel; if not present then create it.
		if (syncRandom) {
			chLocalOffR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELLOCALOFFSETR_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELLOCALOFFSETR_INTERFACE,
																PARTICLECHANNELLOCALOFFSETW_INTERFACE, true,
																actionNode, (Object*)this, &initLocalOff);
			if (chLocalOffR == NULL) return false;
			chLocalOffW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALOFFSETW_INTERFACE, (Object*)this);
			if (chLocalOffW == NULL) return false;
		}
	}

	if (chNew->IsAllOld() && !animatedShape) return true;

	// modify shape for "new" particles
	RandGenerator* randVarGen = randLinkerVar().GetRandGenerator(pCont);
	RandGenerator* randOrdGen = randLinkerOrd().GetRandGenerator(pCont);
	RandGenerator* randSynGen = randLinkerSyn().GetRandGenerator(pCont);
	int tpf = GetTicksPerFrame();

	int numMesh = meshData().Count();
	Tab<int> nullMeshes;
	int i, numNew = 0;
	for (i=0; i<amount; i++)
		if (chNew->IsNew(i)) numNew++;
	int* numIndices = NULL;
	int** corrTable = NULL;
	if (!(animatedShape || acquireShape)) {
		if (numMesh > 0) {
			numIndices = new int[numMesh];
			corrTable = new int*[numMesh];
			for(i=0; i<numMesh; i++) {
				numIndices[i] = 0;
				corrTable[i] = new int[numNew];
			}
		}
	}

	// assign mesh to each particle
	for (i = 0; i < amount; i++)
	{
		bool isNew = chNew->IsNew(i);
		if (!animatedShape && !isNew) continue;
		
		// set eventStart time for new particles
		if (isNew && initEventStart)
			chEventStartW->SetValue(i, chTime->GetValue(i));

		// get new shape from the source
		if (animatedShape || acquireShape) {
			PreciseTimeValue time = chTime->GetValue(i);
			if (animatedShape) {
				switch(syncType)
				{
				case kInstanceShape_syncBy_absoluteTime:
					break;
				case kInstanceShape_syncBy_particleAge:
					time -= chBirthTime->GetValue(i);
					break;
				case kInstanceShape_syncBy_eventStart:
					time -= chEventStartR->GetValue(i);
					break;
				}
				if (syncRandom) {
					if (isNew && initLocalOff)
						chLocalOffW->SetValue(i, randSynGen->Rand0X(randOffset));
					time += chLocalOffR->GetValue(i);
				}
			}
			BuildMeshes(TimeValue(time));
		}
		// compute an index for new particles
		int index = -1;
		numMesh = meshData().Count();
		if (isNew) {
			if (numMesh == 1) {
				index = 0;
			} else if (numMesh > 1) {
				if (randomOrder) {
					index = randOrdGen->Rand0X(numMesh-1);
				} else {
					index = sortIndex(_shapeIndex(pCont) % numMesh);
					_shapeIndex(pCont)++;
				}
			}
		}
		// use a local channel to store old particle's mesh index when animatedShape
		if (animatedShape || useMultipleShapes) {
			if (isNew && initLocalIdx) {	// save index to local channel
				chLocalIdxW->SetValue(i, index);
			} else {		// retrieve index from local channel
				index = chLocalIdxR->GetValue(i);
			}
		}
		
		if (animatedShape || acquireShape) {
			// set mesh data
			if (index < 0 || index >= numMesh)
				chMesh->SetValue(i, NULL);
			else
				chMesh->SetValue(i, _meshData(index));
		} else {
			if (index < 0 || index >= numMesh)
				nullMeshes.Append(1, &i, numNew);
			else {
				(corrTable[index])[numIndices[index]] = i;
				numIndices[index] += 1;
			}
		}

		// set scale for individual particle if there's variation
		if (setScale && isNew) {
			float varScale = scale * GetScaleFactor(randVarGen, variation);
			chScale->SetValue(i, Point3(varScale, varScale, varScale));
		}
	}

	// set meshes in compact form for non-animated meshes
	if (!(animatedShape || acquireShape)) {
		if (nullMeshes.Count() > 0)
			chMesh->SetValue(nullMeshes, NULL);
		Tab<int> meshIndex;
		for(i=0; i<numMesh; i++) {
			if (numIndices[i] > 0) {
				meshIndex.ZeroCount();
				meshIndex.Append(numIndices[i], corrTable[i]);
				chMesh->SetValue(meshIndex, _meshData(i));
			}
		}
		if (numMesh > 0) {
			delete [] numIndices;
			for(i=0; i<numMesh; i++)
				delete [] corrTable[i];
			delete [] corrTable;
		}
	}

	return true;
}

ClassDesc* PFOperatorInstanceShape::GetClassDesc() const
{
	return GetPFOperatorInstanceShapeDesc();
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* PFOperatorInstanceShape::GetTriObjectFromNode(INode *iNode, const TimeValue t, bool &deleteIt)
{
	deleteIt = false;
	if (iNode == NULL) return NULL;
	Object *obj = iNode->EvalWorldState(t).obj;
	if (obj == NULL) return NULL;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = true;
		if (tri->GetMesh().numVerts == 0)
			obj->InvalidateChannels(GEOM_CHANNEL); // to clean the cache for NURBS [bayboro 03-27-2003]
		return tri;
	} else {
		return NULL;
	}
}

void RemoveWSMOffset(Mesh* mesh, INode *node, const TimeValue t)
{
	if (node == NULL) return;
	if (mesh == NULL) return;
	Matrix3 tm = node->GetObjTMAfterWSM(t)*Inverse(node->GetNodeTM(t));
	for(int i=0; i<mesh->numVerts; i++)
		mesh->verts[i] = mesh->verts[i]*tm;
}

void MoveToCentroid(Mesh* mesh)
{
	double x=0.0, y=0.0, z=0.0;
	int i, num = mesh->numVerts;
	if (num == 0) return;
	for(i=0; i<num; i++) {
		Point3 pos = mesh->verts[i];
		x += pos.x;
		y += pos.y;
		z += pos.z;
	}
	Point3 offset = Point3(float(x/num), float(y/num), float(z/num));
	for(i=0; i<num; i++)
		mesh->verts[i] -= offset;
}

BOOL PFOperatorInstanceShape::IsGroupObject(INode *iNode)
{
	return iNode && iNode->IsGroupHead();
}

BOOL PFOperatorInstanceShape::IsHierarchyObject(INode *iNode)
{
	return iNode && (iNode->NumberOfChildren() > 0);
}

BOOL PFOperatorInstanceShape::IsMultiElementsObject(INode *iNode)
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	bool deleteIt;
	TriObject *triObj = GetTriObjectFromNode(iNode, GetCOREInterface()->GetTime(), deleteIt);
	if (triObj == NULL) {
		if (wasHolding) theHold.Resume();
		return FALSE;
	}

	Mesh &triMesh = triObj->GetMesh();
	AdjEdgeList aeList(triMesh);
	AdjFaceList afList(triMesh, aeList);
	FaceElementList elemList(triMesh, afList);
	if (deleteIt) triObj->DeleteMe();
	if (wasHolding) theHold.Resume();

	return (elemList.count > 1);
}

void AppendToMaterialsForDelete(Mtl* mtl, Tab<Mtl*>& toDelete)
{
	if (mtl == NULL) return;
	bool alreadyAdded = false;
	for(int i=0; i<toDelete.Count(); i++)
		if (toDelete[i] == mtl) {
			alreadyAdded = true;
			break;
		}
	if (!alreadyAdded)
		toDelete.Append(1, &mtl);
}

void PFOperatorInstanceShape::BuildMeshes(const TimeValue t, bool setMaterial /* = false */)
{
	Tab<INode*> stackNodes, doneNodes, childNodes;
	INode *node = _object();
	Mtl *multi = NULL;
	Tab<Mtl*> mtlsToDelete;

	if (pblock() == NULL) return; // the pblock is not established

	bool acquireMaterial = (pblock()->GetInt(kInstanceShape_material) == TRUE);

	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	// TODO: this for loop can be removed once the cache is implemented.
	for (int i = 0; i < meshData().Count(); i++)
		if (meshData(i)) {
			delete _meshData(i);
			_meshData(i) = NULL;
		}
	_meshData().ZeroCount();
	_meshTM().ZeroCount();
	if (node)	stackNodes.Append(1, &node, 10);
	if (acquireMaterial && setMaterial)	{ _matOffset().clear(); }
	while (stackNodes.Count())
	{
		node = stackNodes[stackNodes.Count()-1];
		stackNodes.Delete(stackNodes.Count()-1, 1);
		// add children nodes to the stack
		for (int i = 0; i < node->NumberOfChildren(); i++) {
			INode *childNode = node->GetChildNode(i);
			if (childNode)	stackNodes.Append(1, &childNode, 10);
		}
		// skip processed nodes
		for (i = 0; i < doneNodes.Count(); i++)
			if (node == doneNodes[i]) break;
		if (i < doneNodes.Count()) continue;
		// collect nodes to merge
		CollectMergingNodes(node, childNodes);
		// add materials from nodes and compute material offset
		if (acquireMaterial && setMaterial) {
			for (i = -1; i < childNodes.Count(); i++) {
				Mtl *newMat = AddMaterialFromNode(multi, (i < 0) ? node : childNodes[i]);
// commented it: causes crash with more than two materials
				// delete the old material multi if no one reference it
//				if (multi && (newMat != multi) && !(multi->HasDependents()))
//					delete multi;
				if (newMat != multi) {
					int fff = 555;
					AppendToMaterialsForDelete(multi, mtlsToDelete);
				}
				multi = newMat;
			}
		}
		// build mesh(es) from node(s)
		if (childNodes.Count() == 0)
			BuildMeshesFromNode(t, node);
		else
			BuildMeshesFromNode(t, node, childNodes);
		// add childNodes to doneNodes
		doneNodes.Append(1, &node, 10);
		if (childNodes.Count() > 0)
			doneNodes.Append(childNodes.Count(), childNodes.Addr(0), 10);
	}
	
	// sort mesh data according to its max X coordinate and store the mapping into sortIndex
	if (!(pblock()->GetInt(kInstanceShape_randomShape)))
		OrderMeshIndices();

	if (meshNeedsUpdate()) {
		RefreshUI(kInstanceShape_message_type);
		_meshNeedsUpdate() = false;
	}
	if (!wasIgnoring) ClearIgnoreRefNodeChange();

	// Send Message (kPFMSG_UpdateMaterial) to PF System Engine to set material
	if (acquireMaterial && setMaterial) {
		if (theHold.Holding()) {
			if (multi != GetMaterial()) {
				GetPViewManager()->KeepMaterial(GetMaterial()); // to prevent from deletion from the scene and thus making trouble for undo/redo
//				AppendToMaterialsForDelete(GetMaterial(), mtlsToDelete);
				SetMaterial(multi);
			}
			NotifyDependents(FOREVER, PART_MTL, kPFMSG_UpdateMaterial, NOTIFY_ALL, TRUE);
		}
		Interface* ip = GetCOREInterface();
		for(i=0; i<mtlsToDelete.Count(); i++) {
			if (!(mtlsToDelete[i]->HasDependents())) {
				ip->GetSceneMtls()->Remove(mtlsToDelete[i]);
			}
		}
	}
}

void RescaleMesh(Mesh* mesh, Point3& scale)
{
	for(int i=0; i<mesh->getNumVerts(); i++)
		mesh->verts[i] = mesh->verts[i] * scale;
}

int PFOperatorInstanceShape::BuildMeshesFromNode(const TimeValue t, INode *iNode)
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	bool deleteIt;
	TriObject *triObj = GetTriObjectFromNode(iNode, t, deleteIt);
	if (triObj == NULL) {
		if (wasHolding) theHold.Resume();	
		return 0;
	}

	MeshDelta meshDelta;
	Mesh &triMesh = triObj->GetMesh();
	Matrix3 nodeTM = iNode->GetObjectTM(t);
	Point3 pos, scaleFactor;
	Quat rotate;
	DecomposeMatrix(nodeTM, pos, rotate, scaleFactor);

	bool separateElements = (pblock()->GetInt(kInstanceShape_sepElements) == TRUE);
	bool acquireMapping = (pblock()->GetInt(kInstanceShape_mapping) == TRUE);
	bool acquireMaterial = (pblock()->GetInt(kInstanceShape_material) == TRUE);
	Mtl *mtl = NULL;
	int matOffset = 0, count = 0;

	if (acquireMaterial) {
		mtl = iNode->GetMtl();
		matOffset = (_matOffset().find(iNode) != _matOffset().end()) ? _matOffset(iNode) : 0;
	}
	
	if (separateElements) {		// separate elements into meshes
		AdjEdgeList aeList(triMesh);
		AdjFaceList afList(triMesh, aeList);
		FaceElementList elemList(triMesh, afList);
		int numFaces = triMesh.getNumFaces();
		BitArray fset(numFaces);
		for (int i = 0; i < elemList.count; i++) {
			Mesh *elmMesh = new Mesh;
			for (int j = 0; j < numFaces; j++)
				fset.Set(j, (elemList[j] == i));
			meshDelta.Detach(triMesh, elmMesh, fset, TRUE, FALSE, FALSE);
			RemoveWSMOffset(elmMesh, iNode, t);
			MoveToCentroid(elmMesh);
			if (!acquireMapping) RemoveMapData(*elmMesh);
			if (acquireMaterial) {
				FitMeshIDsToMaterial(*elmMesh, mtl);
				AddMaterialOffset(*elmMesh, matOffset);
			}
			if ((elmMesh->getNumFaces() == 0) || (elmMesh->getNumVerts() < 3)) continue;
			RescaleMesh(elmMesh, scaleFactor);
			_meshData().Append(1, &elmMesh, 10);
			_meshTM().Append(1, &nodeTM, 10);
			count++;
		}
	} else {					// whole object as a mesh
		Mesh *newMesh = new Mesh(triMesh);
		RemoveWSMOffset(newMesh, iNode, t);
		if (!acquireMapping) RemoveMapData(*newMesh);
		if (acquireMaterial) {
			FitMeshIDsToMaterial(*newMesh, mtl);
			AddMaterialOffset(*newMesh, matOffset);
		}
		RescaleMesh(newMesh, scaleFactor);
		_meshData().Append(1, &newMesh, 10);
		_meshTM().Append(1, &nodeTM, 10);
		count++;
	}

	if (deleteIt) triObj->DeleteMe();
	if (wasHolding) theHold.Resume();
	return count;
}

int PFOperatorInstanceShape::BuildMeshesFromNode(const TimeValue t, INode *iNode, Tab<INode*>& childNodes)
{
	bool acquireMapping = (pblock()->GetInt(kInstanceShape_mapping) == TRUE);
	bool acquireMaterial = (pblock()->GetInt(kInstanceShape_material) == TRUE);
	Mesh *newMesh = new Mesh;
	int matOffset = 0, count = 0;
	
	// get parent mesh
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	bool deleteIt;
	TriObject *triObj = GetTriObjectFromNode(iNode, t, deleteIt);
	if (triObj) {
		*newMesh = triObj->GetMesh();
//		RemoveWSMOffset(newMesh, iNode, t);
		if (acquireMaterial) {
			matOffset = (_matOffset().find(iNode) != _matOffset().end()) ? _matOffset(iNode) : 0;
			FitMeshIDsToMaterial(*newMesh, iNode->GetMtl());
			AddMaterialOffset(*newMesh, matOffset);
		}
		count = 1;
	}
	if (deleteIt) triObj->DeleteMe();
	Matrix3 nodeTM = iNode->GetObjectTM(t);
	Matrix3 offsetTM = iNode->GetObjTMAfterWSM(t)*Inverse(nodeTM);
	Point3 pos, scaleFactor;
	Quat rotate;
	DecomposeMatrix(nodeTM, pos, rotate, scaleFactor);
	// attach child meshes
	MeshDelta meshDelta;
	for (int i = 0; i < childNodes.Count(); i++)
	{
		INode *childNode = childNodes[i];
		triObj = GetTriObjectFromNode(childNode, t, deleteIt);
		if (triObj) {
			Mesh *childMesh = &(triObj->GetMesh());
			Mesh *triMesh = childMesh;
			Matrix3 transTM = childNode->GetObjectTM(t) * Inverse(nodeTM);
			if (acquireMaterial) {
				matOffset = (_matOffset().find(childNode) != _matOffset().end()) ? _matOffset(childNode) : 0;
				if (!deleteIt) triMesh = new Mesh(*childMesh);
				FitMeshIDsToMaterial(*triMesh, childNode->GetMtl());
			}

			if (newMesh->numVerts > 0) {
				meshDelta.AttachMesh(*newMesh, *triMesh, transTM, matOffset);
				meshDelta.Apply(*newMesh);
			} else {
				(*newMesh) = (*triMesh);
				for(int j=0; j<newMesh->numVerts; j++)
					newMesh->verts[j] = newMesh->verts[j]*transTM;
			}
			if (triMesh != childMesh) delete triMesh;
			if (count == 0) count = 1;
		}
		if (deleteIt) triObj->DeleteMe();
	}
	if (wasHolding) theHold.Resume();
	RemoveWSMOffset(newMesh, iNode, t);
	// store mesh data
	if (count == 0) {
		delete newMesh;
	} else {
		if (!acquireMapping) RemoveMapData(*newMesh);
		RescaleMesh(newMesh, scaleFactor);
		_meshData().Append(1, &newMesh, 10);
		_meshTM().Append(1, &nodeTM, 10);
	}

	return count;
}

void PFOperatorInstanceShape::CollectMergingNodes(INode *iNode, Tab<INode*>& childNodes)
{
	childNodes.ZeroCount();
	if (iNode == NULL) return;

	bool mrgGrp = !(pblock()->GetInt(kInstanceShape_sepGroup));
	bool mrgHry = !(pblock()->GetInt(kInstanceShape_sepHierarchy));
	INode *node = iNode;
	for (int index = 0; ; index++) {
		for (int i = 0; i < node->NumberOfChildren(); i++) {
			INode *childNode = node->GetChildNode(i);
			if (childNode == NULL) continue;
			if ((childNode->IsGroupMember()) ? mrgGrp : mrgHry)
				childNodes.Append(1, &childNode, 10);
		}
		if (index == childNodes.Count()) break;
		node = childNodes[index];
	}
}

Mtl *AggregateMaterial( Mtl *mat1, Mtl *mat2, int &mat2Offset)
{	
	DbgAssert(mat1);
	if (mat1 == NULL) return mat2;
	DbgAssert(mat1->IsMultiMtl());
	if (!mat1->IsMultiMtl()) return mat1;
	
	mat2Offset = mat1->NumSubMtls();
	// Special case: If mat2 is present in mat1 then they 
	// are already combined. We just need to reevaluate the offset.		
	for (int i=0; i<mat2Offset; i++) {
		if (mat1->GetSubMtl(i)==mat2) {
			mat2Offset = i;
			return mat1;
		}
	}

	MultiMtl* multi = (MultiMtl*)mat1;
	// Copy the materials for the second
	if (mat2->IsMultiMtl()) {
		int c2 = mat2->NumSubMtls();
		for (i=0; i<c2; i++) {
			Mtl *m = mat2->GetSubMtl(i);
			if (m) {
				TSTR name;
				MultiMtl *m2 = (MultiMtl *)mat2;
				m2->GetSubMtlName(i,name);
				multi->SetSubMtlAndName(i+mat2Offset,m,name);
				}
			}
	} else {
		multi->SetSubMtl(mat2Offset,mat2);
	}
	return mat1;
}

Mtl *PFOperatorInstanceShape::AddMaterialFromNode(Mtl *multi, INode *iNode)
{
	Mtl *mat = iNode->GetMtl();
	if (multi && mat && (multi != mat)) {
		int multiNum = multi->NumSubMtls();
		int matNum = mat->NumSubMtls();
		if (multiNum && (matNum == multiNum)) {
			// check if all sub-materials are the same
			bool theSameMaterial = true;
			for(int i=0; i<multiNum; i++) {
				if (multi->GetSubMtl(i) != mat->GetSubMtl(i)) {
					theSameMaterial = false; break;
				}
			}
			if (theSameMaterial) {
				_matOffset(iNode) = 0;
				return multi;
			}
		} else if (multiNum && (matNum==0)) {
			// multi has sub-materials and mat does not; check out if mat is one of the sub-materials in multi
			for(int i=0; i<multiNum; i++) {
				if (mat == multi->GetSubMtl(i)) {
					_matOffset(iNode) = i;
					return multi;
				}
			}
		}
		if (multiNum == 0) {
			Mtl* newMat = CombineMaterials(multi, mat, _matOffset(iNode));
			if ((newMat != multi) && (newMat != mat)) {
				GetCOREInterface()->AssignNewName(newMat);
				((Animatable*)(newMat))->SetAFlag(A_PLUGIN1);
			}
			multi = newMat;
		} else {
			if (((Animatable*)(multi))->TestAFlag(A_PLUGIN1)) {
				multi = AggregateMaterial(multi, mat, _matOffset(iNode));
			} else {
				Mtl* newMat = CombineMaterials(multi, mat, _matOffset(iNode));
				if ((newMat != multi) && (newMat != mat)) {
					GetCOREInterface()->AssignNewName(newMat);
					((Animatable*)(newMat))->SetAFlag(A_PLUGIN1);
				}
				multi = newMat;
			}
		}
	} else {
		if (multi == NULL) multi = mat;
		_matOffset(iNode) = 0;
	}
	return multi;
}

void PFOperatorInstanceShape::OrderMeshIndices()
{
	Tab<FloatIntPair> xCoordIndex;
	int meshCount = meshData().Count();

	xCoordIndex.SetCount(meshCount);
	_sortIndex().SetCount(meshCount);
	for (int i = 0; i < meshCount; i++) {
		const Matrix3 &nodeTM = meshTM(i);
		xCoordIndex[i].first = nodeTM.PointTransform(_meshData(i)->getBoundingBox().Max()).x;
		xCoordIndex[i].second = i;
	}
	xCoordIndex.Sort(CompareFloatIntPair);
	for (i = 0; i < meshCount; i++)
		_sortIndex(i) = xCoordIndex[i].second;
}

void PFOperatorInstanceShape::RemoveMapData(Mesh &mesh)
{
	mesh.setNumMaps(2);
	for (int mp = -NUM_HIDDENMAPS; mp < 2; mp++)
		mesh.setMapSupport(mp, FALSE);
}

void PFOperatorInstanceShape::AddMaterialOffset(Mesh &mesh, int matOffset)
{
	for (int i = 0; i < mesh.getNumFaces(); i++) {
		mesh.setFaceMtlIndex(i, mesh.getFaceMtlIndex(i) + matOffset);
	}
}

int PFOperatorInstanceShape::CompareFloatIntPair(const void *elem1, const void *elem2)
{
	FloatIntPair &e1 = * (FloatIntPair *) elem1;
	FloatIntPair &e2 = * (FloatIntPair *) elem2;
	if (e1 < e2) return -1;
	if (e1 > e2) return 1;
	return 0;
}

float PFOperatorInstanceShape::GetScaleFactor(RandGenerator* randGen, float variation)
{
	variation = max(variation, 0.0f);
	variation = min(variation, kMaxVariation);
	variation *= 0.7f;
	float maxSF = 1.0f + variation;
	float scaleFactor = (1.0f - variation * variation) /
		sqrt((maxSF * maxSF) - (4.0f * variation * randGen->Rand01()));
	if (scaleFactor > 1.0f) scaleFactor = 1.0f + (scaleFactor-1.0f)/0.7f;
	else scaleFactor = (scaleFactor-0.3f)/0.7f;
	return scaleFactor;
}

void PFOperatorInstanceShape::RefreshUI(WPARAM message, IParamMap2* map) const
{
	HWND hWnd;

	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)this);
		}
		return;
	}

	if ( pblock() == NULL ) return;

	map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)this);
		}
	}
	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		SendMessage( hWnd, WM_COMMAND, message, (LPARAM)this);
	}
}

static void CatchMaterialChange(void *param, NotifyInfo *info) 
{
	DbgAssert(param);
	PFOperatorInstanceShape* op = (PFOperatorInstanceShape*)param;
	op->notifyMtlChanged(info);
}

void PFOperatorInstanceShape::updateRegMtlChangeNotification(bool unregister)
{
	if (unregister || (pblock() == NULL)) {
		if (registeredMtlChangeNotification()) {
			UnRegisterNotification(CatchMaterialChange, this, NOTIFY_NODE_POST_MTL);
			_registeredMtlChangeNotification() = false;
		}
		return;
	}
	bool needReg = (pblock()->GetInt(kInstanceShape_material, 0) != 0) && (object() != NULL);
	if (registeredMtlChangeNotification()) {
		if (!needReg) {
			UnRegisterNotification(CatchMaterialChange, this, NOTIFY_NODE_POST_MTL);
			_registeredMtlChangeNotification() = false;
		}			
	} else {
		if (needReg) {
			RegisterNotification(CatchMaterialChange, this, NOTIFY_NODE_POST_MTL);
			_registeredMtlChangeNotification() = true;
		}			
	}
}

bool HasNodeInHierarchy(INode* checkNode, INode* sampleNode)
{
	if (checkNode == sampleNode) return true;
	if (checkNode == NULL) return false;
	for(int i=0; i<checkNode->NumChildren(); i++)
		if (HasNodeInHierarchy(checkNode->GetChildNode(i), sampleNode))
			return true;
	return false;
}

void PFOperatorInstanceShape::notifyMtlChanged(NotifyInfo* info)
{
	if (IsIgnoringRefNodeChange()) return;

	INode* node = (INode*)(info->callParam); DbgAssert(node);
	// check if the node is referenced
	if (HasNodeInHierarchy(_object(), node)) {
		if (theHold.Holding()) {
			BuildMeshes(GetCOREInterface()->GetTime(), true);
			NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		} else {
			_meshNeedsUpdate() = true;
		}
	}
}

bool PFOperatorInstanceShape::verifyRefObjectMXSSync()
{
	if (pblock() == NULL) return true;
	return (object() == pblock()->GetINode(kInstanceShape_objectMaxscript, 0));
}

bool hUpdateInstanceShapeInProgress = false;
bool PFOperatorInstanceShape::updateFromRealRefObject()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return false;
	if (hUpdateInstanceShapeInProgress) return false;
	if (verifyRefObjectMXSSync()) return false;
	hUpdateInstanceShapeInProgress = true;
	pblock()->SetValue(kInstanceShape_objectMaxscript, 0, _object());
	hUpdateInstanceShapeInProgress = false;
	return true;
}

bool PFOperatorInstanceShape::updateFromMXSRefObject()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return false;
	if (hUpdateInstanceShapeInProgress) return false;
	if (verifyRefObjectMXSSync()) return false;
	hUpdateInstanceShapeInProgress = true;
	INode* node = pblock()->GetINode(kInstanceShape_objectMaxscript, 0);	
	RefResult refR;
	if (node == NULL) {
		refR = DeleteReference(kInstanceShape_reference_object);
	} else {
		if (object())
			refR = ReplaceReference(kInstanceShape_reference_object, node);
		else
			MakeRefByID(FOREVER, kInstanceShape_reference_object, node);
	}
	// Build Meshes
	BuildMeshes(GetCOREInterface()->GetTime(), true);
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	hUpdateInstanceShapeInProgress = false;
	return (refR == REF_SUCCEED);
}

void PFOpInstShapeCollectChildren(INode* node, Tab<INode*>& objList)
{
	if (node == NULL) return;
	for(int i=0; i<node->NumberOfChildren(); i++) {
		INode* childNode = node->GetChildNode(i);
		if (childNode != NULL) {
			objList.Append(1, &childNode);
			PFOpInstShapeCollectChildren(childNode, objList);
		}
	}
}

void PFOperatorInstanceShape::resetObjectList()
{
	if (pblock() == NULL) return;
	if (resetingObjectList()) return;
	_resetingObjectList() = true;
	Tab<INode*> objList;
	PFOpInstShapeCollectChildren(_object(), objList);
	pblock()->ZeroCount(kInstanceShape_objectList);
	if (objList.Count())
		pblock()->Append(kInstanceShape_objectList, objList.Count(), objList.Addr(0));
	_resetingObjectList() = false;
}

//+--------------------------------------------------------------------------+
//|							Pick Node Callback								 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorInstanceShapePickNodeCallback::Filter(INode *iNode)
{
	if (iNode == NULL) return FALSE;
	TimeValue t = GetCOREInterface()->GetTime();
	Tab<INode*> stack;
	
	if (op() != NULL)
		if (iNode->TestForLoop(FOREVER,_op())!=REF_SUCCEED) return FALSE;

	if (iNode->IsGroupMember())		// get highest closed group node
		iNode = GetHighestClosedGroupNode(iNode);

	stack.Append(1, &iNode, 10);
	while (stack.Count())
	{
		INode *node = stack[stack.Count()-1];
		stack.Delete(stack.Count()-1, 1);

		Object *obj = node->EvalWorldState(t).obj;
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
			return TRUE;

		// add children to the stack
		for (int i = 0; i < node->NumberOfChildren(); i++) {
			INode *childNode = node->GetChildNode(i);
			if (childNode)	stack.Append(1, &childNode, 10);
		}
	}
	return FALSE;
}

//+--------------------------------------------------------------------------+
//|							Pick Mode Callback								 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorInstanceShapePickModeCallback::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags)
{
	return (ip->PickNode(hWnd, m, GetFilter())) ? TRUE : FALSE;
}

BOOL PFOperatorInstanceShapePickModeCallback::Pick(IObjParam *ip, ViewExp *vpt)
{
	INode *iNode = vpt->GetClosestHit();
	if (iNode == NULL) return FALSE;
	if (op() == NULL) return TRUE;

	if (iNode->IsGroupMember())		// get highest closed group node
		iNode = GetHighestClosedGroupNode(iNode);

	theHold.Begin();
	// set object reference
	if (op()->object())
		_op()->ReplaceReference(kInstanceShape_reference_object, iNode);
	else
		_op()->MakeRefByID(FOREVER, kInstanceShape_reference_object, iNode);
	theHold.Accept(GetString(IDS_PICKOBJECT));

	return TRUE;
}

void PFOperatorInstanceShapePickModeCallback::EnterMode(IObjParam *ip)
{
	if (op()) op()->RefreshUI(kInstanceShape_message_enterPick);
	GetCOREInterface()->PushPrompt(GetString(IDS_PICKGEOMETRYOBJECT));
}

void PFOperatorInstanceShapePickModeCallback::ExitMode(IObjParam *ip)
{
	if (op()) op()->RefreshUI(kInstanceShape_message_exitPick);
	GetCOREInterface()->PopPrompt();
}

PickNodeCallback *PFOperatorInstanceShapePickModeCallback::GetFilter()
{
	ThePFOperatorInstanceShapePickNodeCallback.Init(_op());
	return &ThePFOperatorInstanceShapePickNodeCallback;
}

} // end of namespace PFActions