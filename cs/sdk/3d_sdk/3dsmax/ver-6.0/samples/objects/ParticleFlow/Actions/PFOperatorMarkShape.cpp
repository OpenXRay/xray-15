/**********************************************************************
 *<
	FILE:			PFOperatorMarkShape.cpp

	DESCRIPTION:	MarkShape Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-29-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include <limits>

#include "max.h"
#include "meshdlib.h"

#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorMarkShape.h"

#include "PFOperatorMarkShape_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Creates a transferable particle channel to store a transformation matrix for each particle
// in the contact object's space.
#define PARTICLECHANNELLOCALTMR_INTERFACE Interface_ID(0x42101061, 0x1eb34500) 
#define PARTICLECHANNELLOCALTMW_INTERFACE Interface_ID(0x42101061, 0x1eb34501) 
#define GetParticleChannelLocalTMRInterface(obj) ((IParticleChannelMatrix3R*)obj->GetInterface(PARTICLECHANNELLOCALTMR_INTERFACE)) 
#define GetParticleChannelLocalTMWInterface(obj) ((IParticleChannelMatrix3W*)obj->GetInterface(PARTICLECHANNELLOCALTMW_INTERFACE)) 

// Creates a transferable particle channel to store a transformation matrix for each particle
// in the contact face space.
#define PARTICLECHANNELFACETMR_INTERFACE Interface_ID(0x42101062, 0x1eb34500) 
#define PARTICLECHANNELFACETMW_INTERFACE Interface_ID(0x42101062, 0x1eb34501) 
#define GetParticleChannelFaceTMRInterface(obj) ((IParticleChannelMatrix3R*)obj->GetInterface(PARTICLECHANNELFACETMR_INTERFACE)) 
#define GetParticleChannelFaceTMWInterface(obj) ((IParticleChannelMatrix3W*)obj->GetInterface(PARTICLECHANNELFACETMW_INTERFACE)) 

// Creates a transferable particle channel to store an index of contact face for each particle
#define PARTICLECHANNELFACEINDEXR_INTERFACE Interface_ID(0x42101063, 0x1eb34500) 
#define PARTICLECHANNELFACEINDEXW_INTERFACE Interface_ID(0x42101063, 0x1eb34501) 
#define GetParticleChannelFaceIndexRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELFACEINDEXR_INTERFACE)) 
#define GetParticleChannelFaceIndexWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELFACEINDEXW_INTERFACE)) 

// Creates a transferable particle channel to store number of faces in the contact object
// to verify the consistency of the contact object topology
#define PARTICLECHANNELFACENUMR_INTERFACE Interface_ID(0x42101064, 0x1eb34500) 
#define PARTICLECHANNELFACENUMW_INTERFACE Interface_ID(0x42101064, 0x1eb34501) 
#define GetParticleChannelFaceNumRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELFACENUMR_INTERFACE)) 
#define GetParticleChannelFaceNumWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELFACENUMW_INTERFACE)) 

// Creates a transferable particle channel to store surface offset variation for each particle
#define PARTICLECHANNELOFFSETVARR_INTERFACE Interface_ID(0x42101065, 0x1eb34500) 
#define PARTICLECHANNELOFFSETVARW_INTERFACE Interface_ID(0x42101065, 0x1eb34501) 
#define GetParticleChannelOffsetVarRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELOFFSETVARR_INTERFACE)) 
#define GetParticleChannelOffsetVarWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELOFFSETVARW_INTERFACE)) 

BOOL ClosestPointOnMesh(Mesh &mesh, const Point3 &P, Point3 &C, Point3 &N, int& faceIndex );

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMarkShapeState					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorMarkShapeState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMarkShapeState::IObject						 |
//+--------------------------------------------------------------------------+
void PFOperatorMarkShapeState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMarkShapeState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorMarkShapeState::GetClassID() 
{ 
	return PFOperatorMarkShapeState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorMarkShapeState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMarkShapeState::Save(ISave* isave) const
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
	if ((res = isave->Write(randGenOrn(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkRandGen4);
	if ((res = isave->Write(randGenOffset(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorMarkShapeState::IPFActionState					 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorMarkShapeState::Load(ILoad* iload)
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
			res=iload->Read(_randGenOrn(), sizeof(RandGenerator), &nb);
			break;

		case IPFActionState::kChunkRandGen4:
			res=iload->Read(_randGenOffset(), sizeof(RandGenerator), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorMarkShape						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorMarkShape::PFOperatorMarkShape()
{
	_hasContactObj() = false;
	GetClassDesc()->MakeAutoParamBlocks(this);
	CreateRectangleMesh();
	CreateBoxMesh();
}

PFOperatorMarkShape::~PFOperatorMarkShape()
{
	;
}

FPInterfaceDesc* PFOperatorMarkShape::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &markShape_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &markShape_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &markShape_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorMarkShape::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_MARKSHAPE_CLASS_NAME);
}

Class_ID PFOperatorMarkShape::ClassID()
{
	return PFOperatorMarkShape_Class_ID;
}

void PFOperatorMarkShape::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorMarkShape::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorMarkShape::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
													 PartID& partID, RefMessage message)
{
	IParamMap2* map;
	bool updateDynamicName;

	if ( hTarget == pblock() ) {
		int lastUpdateID;
		switch (message)
		{
		case REFMSG_CHANGE:
			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			lastUpdateID = pblock()->LastNotifyParamID();
			switch ( lastUpdateID )
			{
			case kMarkShape_contactObj:
				if (IsIgnoringRefNodeChange()) return REF_STOP;
				updateDynamicName = false;
				if (hasContactObj()) {
					if (pblock()->GetINode(kMarkShape_contactObj, 0) == NULL) {
						_hasContactObj() = false;
						updateDynamicName = true;
					}
				} else {
					if (pblock()->GetINode(kMarkShape_contactObj, 0) != NULL) {
						_hasContactObj() = true;
						updateDynamicName = true;
					}
				}
				if (updateDynamicName)
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				break;
			case kMarkShape_alignTo:
				RefreshUI(kMarkShape_message_alignTo);
				break;
			case kMarkShape_sizeSpace:
				RefreshUI(kMarkShape_message_sizeSpace);
				break;
			case kMarkShape_angleDistortion:
				RefreshUI(kMarkShape_message_angleDistort);
				break;
			case kMarkShape_markType:
			case kMarkShape_constantUpdate:
				RefreshUI(kMarkShape_message_markType);
				break;
			case kMarkShape_pivotOffset:
//			case kMarkShape_surfaceOffset:
			case kMarkShape_generateMapping:
				BuildRectangleMesh();
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			UpdatePViewUI(kMarkShape_contactObj);
			break;
		// Initialization of Dialog
		case kMarkShape_RefMsg_InitDlg:
			// Set ICustButton properties for Material DAD button
			map = (IParamMap2*)partID;
			// Refresh UI
			RefreshUI(kMarkShape_message_alignTo, map);
			RefreshUI(kMarkShape_message_sizeSpace, map);
			// RefreshUI(kMarkShape_message_angleDistort, map);
			RefreshUI(kMarkShape_message_markType, map);
			return REF_STOP;
		// New Random Seed
		case kMarkShape_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorMarkShape::Clone(RemapDir &remap)
{
	PFOperatorMarkShape* newOp = new PFOperatorMarkShape();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorMarkShape::GetObjectName()
{
	return GetString(IDS_OPERATOR_MARKSHAPE_OBJECT_NAME);
}

bool PFOperatorMarkShape::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	BuildRectangleMesh();

	// initialize random number generators
	return	_randLinkerOrn().Init( pCont, GetRand() ) &&
			_randLinkerVar().Init( pCont, GetRand() ) &&
			_randLinkerOffset().Init( pCont, GetRand() );
}

bool PFOperatorMarkShape::Release(IObject* pCont)
{
	// release random number generators
	_randLinkerOrn().Release( pCont );
	_randLinkerVar().Release( pCont );
	_randLinkerOffset().Release( pCont );
	return true;
}

const ParticleChannelMask& PFOperatorMarkShape::ChannelsUsed(const Interval& time) const
{
								// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_EventStart|PCU_Position|PCU_Orientation|PCU_Scale|PCU_Speed|PCU_Shape,
								// write channels
									PCU_EventStart|PCU_Position|PCU_Orientation|PCU_Scale|PCU_Speed|PCU_Shape|PCU_MtlIndex);
	return mask;
}

int	PFOperatorMarkShape::GetRand()
{
	return pblock()->GetInt(kMarkShape_randomSeed);
}

void PFOperatorMarkShape::SetRand(int seed)
{
	pblock()->SetValue(kMarkShape_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorMarkShape::GetCurrentState(IObject* pContainer)
{
	PFOperatorMarkShapeState* actionState = (PFOperatorMarkShapeState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorMarkShapeState_Class_ID);
	RandGenerator* randGenVar = randLinkerVar().GetRandGenerator(pContainer);
	if (randGenVar != NULL)
		memcpy(actionState->_randGenVar(), randGenVar, sizeof(RandGenerator));
	RandGenerator* randGenOrn = randLinkerOrn().GetRandGenerator(pContainer);
	if (randGenOrn != NULL)
		memcpy(actionState->_randGenOrn(), randGenOrn, sizeof(RandGenerator));
	RandGenerator* randGenOffset = randLinkerOffset().GetRandGenerator(pContainer);
	if (randGenOffset != NULL)
		memcpy(actionState->_randGenOffset(), randGenOffset, sizeof(RandGenerator));
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorMarkShape::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorMarkShapeState* actionState = (PFOperatorMarkShapeState*)aSt;
	RandGenerator* randGen = randLinkerVar().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenVar(), sizeof(RandGenerator));
	}
	randGen = randLinkerOrn().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenOrn(), sizeof(RandGenerator));
	}
	randGen = randLinkerOffset().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenOffset(), sizeof(RandGenerator));
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMarkShape::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MARKSHAPE_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorMarkShape::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_MARKSHAPE_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorMarkShape::HasDynamicName(TSTR& nameSuffix)
{
	INode* contactNode = pblock()->GetINode(kMarkShape_contactObj);
	if (contactNode == NULL) nameSuffix = GetString(IDS_NONE);
	else nameSuffix = contactNode->GetName();
	return true;
}

bool PFOperatorMarkShape::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	INode* object = pblock()->GetINode(kMarkShape_contactObj);
	if (object == NULL) return true;

	const BOOL surfAnim = pblock()->GetInt(kMarkShape_surfAnimate);
	const int alignTo = pblock()->GetInt(kMarkShape_alignTo);
	const int sizeSpace = pblock()->GetInt(kMarkShape_sizeSpace);
	const float length = pblock()->GetFloat(kMarkShape_length);
	const float width = pblock()->GetFloat(kMarkShape_width);
	const float inheritScl = pblock()->GetFloat(kMarkShape_inheritedScale);
	const float variation = pblock()->GetFloat(kMarkShape_variation);
	const BOOL angleDistort = pblock()->GetInt(kMarkShape_angleDistortion);
	const float maxDistort = pblock()->GetFloat(kMarkShape_maxDistortion);
	const int markType = pblock()->GetInt(kMarkShape_markType);
	const float height = pblock()->GetFloat(kMarkShape_height);
	const float surfaceOffset = pblock()->GetFloat(kMarkShape_surfaceOffset);
	const float surfaceOffsetVar = pblock()->GetFloat(kMarkShape_surfaceOffsetVar);
	const float vertexNoise = pblock()->GetFloat(kMarkShape_vertexNoise);
	const BOOL multiElms = pblock()->GetInt(kMarkShape_multiElements);
	const BOOL constantUpdate = pblock()->GetInt(kMarkShape_constantUpdate);
	const BOOL generateMapping = pblock()->GetInt(kMarkShape_generateMapping);

	bool alignSpeed = (alignTo == kMarkShape_alignTo_speed);
	bool inWorld = (sizeSpace == kMarkShape_sizeSpace_world);
	bool inLocal = (sizeSpace == kMarkShape_sizeSpace_local);
	bool markRect = (markType == kMarkShape_markType_rectangle);
	bool markBox = (markType == kMarkShape_markType_boxIntersect);

	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't find particle times in the container
	IParticleChannelPoint3R* chPosR = GetParticleChannelPositionRInterface(pCont);
	if (chPosR == NULL) return false; // can't find particle position in the container
	IParticleChannelPoint3W* chPosW = GetParticleChannelPositionWInterface(pCont);
	if (chPosW == NULL) return false; // can't find particle position in the container

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	// ensure presence of the Speed channel
	bool initSpeed = false;
	IParticleChannelPoint3R* chSpeedR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSPEEDR_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSPEEDR_INTERFACE,
																PARTICLECHANNELSPEEDW_INTERFACE, true,
																actionNode, NULL, &initSpeed);
	if (chSpeedR == NULL) return false; // can't read/create Speed channel
	IParticleChannelPoint3W* chSpeedW = GetParticleChannelSpeedWInterface(pCont);
	if (chSpeedW == NULL) return false; // can't modify Speed channel
	
	// ensure presence of the Orientation channel
	bool initOrientation = false;
	IParticleChannelQuatR* chOrientR = (IParticleChannelQuatR*)chCont->EnsureInterface(PARTICLECHANNELORIENTATIONR_INTERFACE,
																ParticleChannelQuat_Class_ID,
																true, PARTICLECHANNELORIENTATIONR_INTERFACE,
																PARTICLECHANNELORIENTATIONW_INTERFACE, true,
																actionNode, NULL, &initOrientation);
	if (chOrientR == NULL) return false; // can't read/create Orientation channel
	IParticleChannelQuatW* chOrientW = GetParticleChannelOrientationWInterface(pCont);
	if (chOrientW == NULL) return false; // can't modify Orientation channel

	// ensure presence of the Rotation Spin channel
	bool initSpin = false;
	IParticleChannelAngAxisR* chSpinR = (IParticleChannelAngAxisR*)chCont->EnsureInterface(PARTICLECHANNELSPINR_INTERFACE,
																ParticleChannelAngAxis_Class_ID,
																true, PARTICLECHANNELSPINR_INTERFACE,
																PARTICLECHANNELSPINW_INTERFACE, true,
																actionNode, NULL, &initSpin);
	if (chSpinR == NULL) return false; // can't read/create Spin channel
	IParticleChannelAngAxisW* chSpinW = GetParticleChannelSpinWInterface(pCont);
	if (chSpinW == NULL) return false; // can't modify Spin channel

	// ensure presence of the Scale channel
	bool initScale = false;
	IParticleChannelPoint3R* chScaleR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSCALER_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSCALER_INTERFACE,
																PARTICLECHANNELSCALEW_INTERFACE, true,
																actionNode, NULL, &initScale);
	if (chScaleR == NULL) return false;	// can't read/create Scale channel
	IParticleChannelPoint3W* chScaleW = GetParticleChannelScaleWInterface(pCont);
	if (chScaleW == NULL) return false; // can't modify Scale channel

	// ensure presence of the Shape channel
	bool initShape = false;
	IParticleChannelMeshR* chMeshR = (IParticleChannelMeshR*)chCont->EnsureInterface(PARTICLECHANNELSHAPER_INTERFACE,
																ParticleChannelMesh_Class_ID,
																true, PARTICLECHANNELSHAPER_INTERFACE,
																PARTICLECHANNELSHAPEW_INTERFACE, true,
																actionNode, NULL, &initShape);
	if (chMeshR == NULL) return false; // can't read/create Shape channel
	IParticleChannelMeshW* chMeshW = GetParticleChannelShapeWInterface(pCont);
	if (chMeshW == NULL) return false; // can't modify Shape channel

	//------ Private Channels ---------//

	// ensure local transformation channel
	bool initLocalTM = false;
	IParticleChannelMatrix3R* chLocalTMR = (IParticleChannelMatrix3R*)chCont->EnsureInterface(PARTICLECHANNELLOCALTMR_INTERFACE,
																ParticleChannelMatrix3_Class_ID,
																true, PARTICLECHANNELLOCALTMR_INTERFACE,
																PARTICLECHANNELLOCALTMW_INTERFACE, true,
																actionNode, (Object*)this, &initLocalTM);
	if (chLocalTMR == NULL) return false; // can't read/create local transformation channel
	IParticleChannelMatrix3W* chLocalTMW = (IParticleChannelMatrix3W*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALTMW_INTERFACE, (Object*)this);
	if (chLocalTMW == NULL) return false; // can't modify local transformation channel

	// ensure face transformation channel
	bool initFaceTM = false;
	IParticleChannelMatrix3R* chFaceTMR = NULL;
	IParticleChannelMatrix3W* chFaceTMW = NULL;
	bool initFaceIndex = false;
	IParticleChannelIntR* chFaceIndexR = NULL;
	IParticleChannelIntW* chFaceIndexW = NULL;
	bool initFaceNum = false;
	IParticleChannelIntR* chFaceNumR = NULL;
	IParticleChannelIntW* chFaceNumW = NULL;
	if (surfAnim) {
		chFaceTMR = (IParticleChannelMatrix3R*)chCont->EnsureInterface(PARTICLECHANNELFACETMR_INTERFACE,
																ParticleChannelMatrix3_Class_ID,
																true, PARTICLECHANNELFACETMR_INTERFACE,
																PARTICLECHANNELFACETMW_INTERFACE, true,
																actionNode, (Object*)this, &initFaceTM);
		if (chFaceTMR == NULL) return false; // can't read/create face transformation channel
		chFaceTMW = (IParticleChannelMatrix3W*)chCont->GetPrivateInterface(PARTICLECHANNELFACETMW_INTERFACE, (Object*)this);
		if (chFaceTMW == NULL) return false; // can't modify face transformation channel

		chFaceIndexR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELFACEINDEXR_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELFACEINDEXR_INTERFACE,
																PARTICLECHANNELFACEINDEXW_INTERFACE, true,
																actionNode, (Object*)this, &initFaceIndex);
		if (chFaceIndexR == NULL) return false; // can't read/create face index channel
		chFaceIndexW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELFACEINDEXW_INTERFACE, (Object*)this);
		if (chFaceIndexW == NULL) return false; // can't modify face index channel
																
		chFaceNumR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELFACENUMR_INTERFACE,
																ParticleChannelInt_Class_ID,
																true, PARTICLECHANNELFACENUMR_INTERFACE,
																PARTICLECHANNELFACENUMW_INTERFACE, true,
																actionNode, (Object*)this, &initFaceNum);
		if (chFaceNumR == NULL) return false; // can't read/create face index channel
		chFaceNumW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELFACENUMW_INTERFACE, (Object*)this);
		if (chFaceNumW == NULL) return false; // can't modify face index channel
	}

	bool initOffsetVar = false;
	IParticleChannelFloatR* chOffsetVarR = NULL;
	IParticleChannelFloatW* chOffsetVarW = NULL;
	if (surfaceOffsetVar > 0.0f) {
		chOffsetVarR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELOFFSETVARR_INTERFACE,
																ParticleChannelFloat_Class_ID,
																true, PARTICLECHANNELOFFSETVARR_INTERFACE,
																PARTICLECHANNELOFFSETVARW_INTERFACE, true,
																actionNode, (Object*)this, &initOffsetVar);
		if (chOffsetVarR == NULL) return false; // can't read/create OffsetVariation channel
		chOffsetVarW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELOFFSETVARW_INTERFACE, (Object*)this);
		if (chOffsetVarW == NULL) return false; // can't modify OffsetVariation channel
	}

	RandGenerator* randOrnGen = randLinkerOrn().GetRandGenerator(pCont);
	RandGenerator* randVarGen = randLinkerVar().GetRandGenerator(pCont);
	RandGenerator* randOffsetGen = randLinkerOffset().GetRandGenerator(pCont);
	int amount = chAmount->Count();
	PreciseTimeValue interval;
	Matrix3 shapeTM, nextTM;
	Quat orgOrient, orientation;
	Point3 orgPosition, orgScale, orgSpeed, position, scale, speed, locScale, contactPoint, contactNormal, alignDir;
	AngAxis nullAngAxis(Point3::XAxis, 0.0f);
	Quat nullQuat(nullAngAxis);
	
	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	// cache contact object transformation matrix
	TimeValue timeValueStart = (int)floor(float(timeStart) - 1.0f);
	TimeValue timeValueEnd = (int)ceil(float(timeEnd));
	bool isNodeAnimated = false;
	Tab<Matrix3> cachedNodeTM;
	Matrix3 nodeTM, faceTM;
	if (!(chNew->IsAllOld())) { // initialize all new particles
		isNodeAnimated = IsNodeAnimated(object);
		if (isNodeAnimated)
			CacheObjectTM(object, timeValueStart, timeValueEnd, cachedNodeTM);
		else
			nodeTM = object->GetObjectTM(timeValueEnd);

		Tab<int> newIndices;
		for (int i = 0; i < amount; i++)
		{
			if (!(chNew->IsNew(i))) continue; // skip the old particles
			PreciseTimeValue time = chTime->GetValue(i);
			TimeValue t = TimeValue(time);
		
			// initiate all public channels for a new particle
			if (initSpeed) chSpeedW->SetValue(i, Point3::Origin);
			if (initOrientation) chOrientW->SetValue(i, nullQuat);
			if (initSpin) chSpinW->SetValue(i, nullAngAxis);
			if (initScale) chScaleW->SetValue(i, Point3(1.0f, 1.0f, 1.0f));

			// Set particle orientation according to contact point surface normal and align to parameter
			// compute the contact object's transformation matrix
			if (isNodeAnimated)
				InterpolateObjectTM(time, timeValueStart, cachedNodeTM, nodeTM);
			// build contact mesh 
			BuildContactMesh(t);
			if (_contactMesh().numFaces <= 0) continue;
			// compute a local transformation for each new particle
			orgPosition = chPosR->GetValue(i);
			orgSpeed = chSpeedR->GetValue(i);
			orgOrient = chOrientR->GetValue(i);
			orgScale = chScaleR->GetValue(i);
			int faceIndex;
			ClosestPointOnMesh(_contactMesh(), orgPosition,
								contactPoint, contactNormal, faceIndex);
			// compute align to direction
			alignDir = GetAlignDirection(t, contactNormal, orgSpeed, orgOrient, randOrnGen);
			// compute position and orientation of mark shape
			SetAlignTM(contactPoint, contactNormal, alignDir, shapeTM);
			Matrix3 prescaleShapeTM = shapeTM;

			// Set particle size according to sizeSpace, units, proportion & variation parameters
			// get length & width
			float heightScale = (markBox) ? height : 1.0f;
			float lengthScale = length;
			float widthScale = width;
			if (inLocal && !initShape) {
				const Mesh* orgMesh = chMeshR->GetValue(i);
				Point2 dims = ProjectedSize(orgMesh, orgOrient, orgScale, shapeTM);
				lengthScale = inheritScl * dims[0];
				widthScale = inheritScl * dims[1];
			}
			// take into account scale variation
			float scaleFactor = GetScaleFactor(randVarGen, variation);
			if (scaleFactor < 0.0001f) scaleFactor = 0.0001f; // to avoid zero-sized particles
			if (variation != 0.0f) {
				lengthScale *= scaleFactor;
				widthScale *= scaleFactor;
			}			
			// apply angle distortion on length
			if (alignSpeed && angleDistort && (LengthSquared(orgSpeed) > 0.0f)) {
				float lengthFactor = DotProd(orgSpeed.Normalize(),contactNormal);
				if (lengthFactor == 0.0f) {
					lengthFactor = maxDistort;
				} else {
					if (lengthFactor < 0.0f) lengthFactor = -lengthFactor;
					lengthFactor = min(maxDistort, (1.0f / lengthFactor));
				}
				lengthScale *= lengthFactor;
			}

			shapeTM.SetRow(0, shapeTM.GetRow(0)*lengthScale);
			shapeTM.SetRow(1, shapeTM.GetRow(1)*widthScale);
			shapeTM.SetRow(2, shapeTM.GetRow(2)*heightScale);
			chLocalTMW->SetValue(i, shapeTM * Inverse(nodeTM));
			if (chFaceIndexW != NULL) chFaceIndexW->SetValue(i, faceIndex);
			if (chFaceNumW != NULL) chFaceNumW->SetValue(i, _contactMesh().numFaces);
			if (chFaceTMW != NULL) {
				Face baseFace = _contactMesh().faces[faceIndex];
				Point3 v0 = _contactMesh().verts[baseFace.v[0]];
				Point3 v1 = _contactMesh().verts[baseFace.v[1]];
				Point3 v2 = _contactMesh().verts[baseFace.v[2]];
				faceTM.SetRow(0, v1 - v0);
				faceTM.SetRow(1, v2 - v0);
				faceTM.SetRow(2, contactNormal);
				faceTM.SetTrans(v0);
				chFaceTMW->SetValue(i, shapeTM*Inverse(faceTM));
			}
			float curOffsetVar = 0.0f;
			if (chOffsetVarW != NULL) {
				curOffsetVar = surfaceOffsetVar*randOffsetGen->Rand11();
				chOffsetVarW->SetValue(i, curOffsetVar);
			} 

			if (markBox && !constantUpdate) { // box cut-out
				Point3 localScale = Point3(lengthScale, widthScale, heightScale);
				if ((localScale.x == 0.0f) || (localScale.y == 0.0f) || (localScale.z == 0.0f)) {
					chMeshW->SetValue(i, NULL);
				} else {
					BuildBoxMesh(localScale);
					ChipOffMesh(contactMesh(), boxMesh(), prescaleShapeTM, _markMesh(), generateMapping);
					if (!multiElms)
						ContactElementMesh(_markMesh(), prescaleShapeTM, orgPosition, orgSpeed);
					Point3 invertScale(1.0f/localScale.x, 1.0f/localScale.y, 1.0f/localScale.z);
					for(int j=0; j<markMesh().numVerts; j++) {
						Point3 vertexOffset = Point3::Origin;
						if (vertexNoise > 0.0f) vertexOffset = vertexNoise*RandSphereSurface(randOffsetGen);
//						_markMesh().verts[j] = markMesh().verts[j]*invertScale + vertexOffset;
						_markMesh().verts[j] = (markMesh().verts[j]+vertexOffset)*invertScale;
					}
					chMeshW->SetValue(i, &_markMesh());
				}
			}
			if (markRect) newIndices.Append(1, &i, 10);

			DecomposeMatrix(shapeTM, position, orientation, scale);
			// set position, orientation & scale channel
			chPosW->SetValue(i, position + contactNormal*(surfaceOffset+curOffsetVar));
			chOrientW->SetValue(i, orientation);
			chScaleW->SetValue(i, scale);
		}

		if (markRect) { // flat rectangle
			chMeshW->SetValue(newIndices, &_rectMesh());
		}
	}

	nodeTM = object->GetObjectTM(timeValueEnd);
	if (constantUpdate || surfAnim) BuildContactMesh(timeValueEnd);
	if (_contactMesh().numFaces <= 0) { // early get out
		if (!wasIgnoring) ClearIgnoreRefNodeChange();
		return true;
	}

	for(int i=0; i<amount; i++) {

		PreciseTimeValue time = chTime->GetValue(i);
		float timeDif = float(timeEnd - time);

		if (surfAnim) {
			int faceNum = chFaceNumR->GetValue(i);
			if (faceNum == contactMesh().numFaces) {
				int faceIndex = chFaceIndexR->GetValue(i);
				Face baseFace = _contactMesh().faces[faceIndex];
				Point3 v0 = _contactMesh().verts[baseFace.v[0]];
				Point3 v1 = _contactMesh().verts[baseFace.v[1]];
				Point3 v2 = _contactMesh().verts[baseFace.v[2]];
				faceTM.SetRow(0, v1 - v0);
				faceTM.SetRow(1, v2 - v0);
				faceTM.SetRow(2, Normalize((v1-v0)^(v2-v1)));
				faceTM.SetTrans(v0);
				shapeTM = chFaceTMR->GetValue(i) * faceTM;
			} else {
				shapeTM = chLocalTMR->GetValue(i) * nodeTM;
			}
		} else {
			shapeTM = chLocalTMR->GetValue(i) * nodeTM;
		}
	
		position = chPosR->GetValue(i);
		orientation = chOrientR->GetValue(i);

		Point3 toBePosition;
		Quat toBeOrientation;
		OrthogonalizeByZthenX(shapeTM);
		DecomposeMatrix(shapeTM, toBePosition, toBeOrientation, scale);
		float curSurfaceOffsetVar = 0.0f;
		if (chOffsetVarR != NULL) curSurfaceOffsetVar = chOffsetVarR->GetValue(i);
		toBePosition += (surfaceOffset+curSurfaceOffsetVar)*Normalize(shapeTM.GetRow(2));
		chScaleW->SetValue(i, scale);
		speed = chSpeedR->GetValue(i);

		if (timeDif > 0.0f) {
			speed = (toBePosition - position)/timeDif;
			chSpeedW->SetValue(i, speed);
			AngAxis toSpin;
			toSpin.angle = QangAxis(orientation, toBeOrientation, toSpin.axis)/timeDif;
			chSpinW->SetValue(i, toSpin);
		} else {
			chPosW->SetValue(toBePosition);
			chOrientW->SetValue(toBeOrientation);
		}

		if (constantUpdate && markBox) {
			if ((scale.x == 0.0f) || (scale.y == 0.0f) || (scale.z == 0.0f)) {
				chMeshW->SetValue(i, NULL);
			} else {
				shapeTM.NoScale();
				BuildBoxMesh(scale);
				ChipOffMesh(contactMesh(), boxMesh(), shapeTM, _markMesh(), generateMapping);
				if (!multiElms)
					ContactElementMesh(_markMesh(), shapeTM, toBePosition, speed);
				Point3 invertScale(1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z);
				for(int j=0; j<markMesh().numVerts; j++)
					_markMesh().verts[j] = markMesh().verts[j]*invertScale;
				chMeshW->SetValue(i, &_markMesh());
			}
		}
	}

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

ClassDesc* PFOperatorMarkShape::GetClassDesc() const
{
	return GetPFOperatorMarkShapeDesc();
}

bool PFOperatorMarkShape::IsNodeAnimated(INode *iNode)
{
	if (iNode == NULL) return FALSE;

	BOOL isNodeAnimated = FALSE;
	for (INode* pn = iNode; !(pn->IsRootNode()); pn = pn->GetParentNode())
		if ((isNodeAnimated = pn->GetTMController()->IsAnimated()) == TRUE) break;
	return isNodeAnimated == TRUE;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* PFOperatorMarkShape::GetTriObjectFromNode(INode *iNode, const TimeValue t, bool &deleteIt)
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
		return tri;
	} else {
		return NULL;
	}
}

void PFOperatorMarkShape::CacheObjectTM(INode *iNode, TimeValue timeValueStart, TimeValue timeValueEnd, Tab<Matrix3> &cachedTM)
{
	if (iNode == NULL) return;

	cachedTM.SetCount(timeValueEnd - timeValueStart + 1);
	int i = 0;
	for (TimeValue t = timeValueStart; t <= timeValueEnd; t++, i++)
		cachedTM[i] = iNode->GetObjectTM(t);
}

void PFOperatorMarkShape::InterpolateObjectTM(PreciseTimeValue time, TimeValue timeValueStart,
									Tab<Matrix3>& cachedTM, Matrix3& nodeTM)
{
	if (float(time) < timeValueStart)
		time = PreciseTimeValue(timeValueStart);
	if (float(time) > (timeValueStart + cachedTM.Count() - 1))
		time = PreciseTimeValue(timeValueStart + cachedTM.Count() - 1);
	int tmIndex = TimeValue(time) - timeValueStart;
	float ratio = float(time) - (tmIndex + timeValueStart);
	if (ratio < 0.0f) {
		ratio += 1.0f;
		tmIndex--;
	}
	if (ratio == 0.0f) {
		nodeTM = cachedTM[tmIndex];
	} else {
		Matrix3 prevObjTM = cachedTM[tmIndex];
		Matrix3 nextObjTM = cachedTM[tmIndex+1];
		prevObjTM *= (1.0f - ratio);
		nextObjTM *= ratio;
		nodeTM = prevObjTM + nextObjTM;
	}
}

// align z axis to zDir and align x axis to the plane defined by zDir and xDir
void PFOperatorMarkShape::SetAlignTM(const Point3 &pivot, const Point3 &zDir, const Point3 &xDir, Matrix3 &alignTM)
{
	Point3 zAxis = zDir;
	if (zAxis.LengthUnify() == 0.0f)
		zAxis = Point3::ZAxis;
	Point3 xAxis, yAxis = zAxis ^ xDir;
	if (yAxis.LengthUnify() == 0.0f) {
		yAxis = zAxis ^ Point3::XAxis;
		if (yAxis.LengthUnify() == 0.0f) {
			yAxis = zAxis ^ Point3::YAxis;
			yAxis.Unify();
		}
	}
	xAxis = yAxis ^ zAxis;
	xAxis.Unify();

	alignTM.SetRow(0, xAxis);
    alignTM.SetRow(1, yAxis);
    alignTM.SetRow(2, zAxis);
    alignTM.SetRow(3, pivot);
    alignTM.ValidateFlags();
}

Point2 PFOperatorMarkShape::ProjectedSize(const Mesh* orgMesh, const Quat& orgOrient, const Point3& orgScale, const Matrix3& shapeTM)
{
	Point2 dims(Point2::Origin);
	if (orgMesh == NULL) return dims;
	Matrix3 orientTM;
	orientTM.SetRotate(orgOrient);
	Point3 xAxis = Normalize(shapeTM.GetRow(0));
	Point3 yAxis = Normalize(shapeTM.GetRow(1));
	for(int i=0; i<orgMesh->numVerts; i++) {
		Point3 p = orientTM.VectorTransform(orgMesh->verts[i]*orgScale);
		float lenX = fabs(DotProd(p, xAxis));
		float lenY = fabs(DotProd(p, yAxis));
		if (lenX > dims[0]) dims[0] = lenX;
		if (lenY > dims[1]) dims[1] = lenY;
	}
	dims *= 2.0f;
	return dims;
}

bool PFOperatorMarkShape::IntersectMesh(Mesh &mesh, const Point3 &point, const Point3 &vector, Point3 &contactPoint, Point3 &contactNormal)
{
	Ray speedRay;
	float at;
	speedRay.p = point;
	speedRay.dir = vector;
	speedRay.dir.Unify();
	if (mesh.IntersectRay(speedRay, at, contactNormal)) {
		contactPoint = speedRay.p + (at * speedRay.dir);
		return true;
	}
	return false;
}

DWORD PFOperatorMarkShape::ClosestFaceOnMesh(Mesh &mesh, const Point3 &point)
{
	float minDist;
	DWORD fi = 0;

	for (int i = 0; i < mesh.getNumFaces(); i++) {
		Point3 vec = mesh.FaceCenter(i) - point;
		float dist = vec.Length();
		if (i == 0 || dist < minDist) {
			fi = i;
			minDist = dist;
		}
	}

	return fi;
}

// temp straight-forward solution on finding a closest point on mesh
// requires some optimization later (Bayboro 03-12-2002)
BOOL ClosestPointOnMesh(Mesh &mesh, const Point3 &P, // point position
									Point3 &C, // closest contact point
									Point3 &N, // normal at contact point
									int& faceIndex ) // index of the closest face
{
	int i,j,numVerts,numFaces;
	Face *faces;
	Point3 *verts;
	BOOL gotVertex,gotEdge,gotFace;
	float vdist,edist,fdist,curdist,extdist;
	int vext,eext0,eext1,fext0,fext1,fext2;
	float elam,flam0,flam1,lam,lam0,lam1;
	Point3 Dif,v0,v1,v2,w0,w1;
	Point3 vec0,vec1;
	float a11,a12,a21,a22,b1,b2,D,D1,D2;
	int i0,i1,i2;
	Tab<int> faceAdj;
	Point3 SN;

	mesh.checkNormals(TRUE); // build normals

	numVerts = mesh.getNumVerts();
	numFaces = mesh.getNumFaces();
	if ((numVerts == 0) || (numFaces == 0)) return FALSE;

	gotVertex = FALSE;
	faces = mesh.faces;
	for(i=0; i<numFaces; i++, faces++)
	{
		for(j=0; j<3; j++) {
			Dif = P - mesh.verts[faces->v[j]];
			curdist = Dif.x*Dif.x + Dif.y*Dif.y + Dif.z*Dif.z;
			if (gotVertex) {
				if (vext == faces->v[j]) {
					faceAdj.Append(1, &i, 4);
				} else if(curdist < vdist) { 
					vdist = curdist; vext = faces->v[j]; 
					faceAdj.ZeroCount();
					faceAdj.Append(1, &i, 4);
				}
			} else { 
				gotVertex = TRUE; 
				vdist = curdist; 
				vext = faces->v[j]; 
				faceAdj.Append(1, &i, 4);
			}
		}
	}
	extdist = vdist;

	gotEdge = FALSE;
	verts = mesh.verts;
	faces = mesh.faces;
	for(i=0; i<numFaces; i++,faces++)
	{
		for(j=0; j<3; j++)
		{
			switch(j)
			{
			case 0: i0 = faces->v[0]; i1 = faces->v[1]; break;
			case 1: i0 = faces->v[1]; i1 = faces->v[2]; break;
			case 2: i0 = faces->v[2]; i1 = faces->v[0]; break;
			}

			v0 = verts[i0]; v1 = verts[i1];
			w0 = v1-v0; w1 = P-v0;
			lam = (w0.x*w1.x + w0.y*w1.y + w0.z*w1.z)/(w0.x*w0.x + w0.y*w0.y + w0.z*w0.z);
			if ((lam <= 0.0f) || (lam >= 1.0f)) continue;
			Dif = w1 - lam*w0;
			curdist = Dif.x*Dif.x + Dif.y*Dif.y + Dif.z*Dif.z;
			if (curdist >= extdist) continue;
			if (gotEdge) {
				if (((eext0 == i0) && (eext1 == i1)) 
					|| ((eext0 == i1) && (eext1 == i0))) {
					faceAdj.Append(1, &i, 4); 
				} else if (curdist < edist) { 
					edist = curdist; eext0 = i0; eext1 = i1; elam = lam; 
					faceAdj.ZeroCount(); faceAdj.Append(1, &i, 4);
				} 
			} else { 
				gotEdge = TRUE; edist = curdist; eext0 = i0; eext1 = i1; elam = lam; 
				faceAdj.ZeroCount(); faceAdj.Append(1, &i, 4);
			}
		}
	}
	if (gotEdge) extdist = edist;

	gotFace = FALSE;
	faces = mesh.faces;
	for(i=0; i<numFaces; i++,faces++)
	{
		i0 = faces->v[0]; i1 = faces->v[1]; i2 = faces->v[2];
		v0 = verts[i0]; v1 = verts[i1]; v2 = verts[i2];
		vec0 = v1-v0; vec1 = v2-v0; w0 = P-v0;
		a11 = vec0.x*vec0.x + vec0.y*vec0.y + vec0.z*vec0.z;
		a12 = a21 = vec0.x*vec1.x + vec0.y*vec1.y + vec0.z*vec1.z;
		a22 = vec1.x*vec1.x + vec1.y*vec1.y + vec1.z*vec1.z;
		b1 = vec0.x*w0.x + vec0.y*w0.y + vec0.z*w0.z;
		b2 = vec1.x*w0.x + vec1.y*w0.y + vec1.z*w0.z;
		D = a11*a22 - a12*a21;
		if (D == 0.0f) continue;
		D1 = b1*a22 - b2*a12; lam0 = D1/D;
		if ((lam0 <= 0.0f) || (lam0 >= 1.0f)) continue;
		D2 = a11*b2 - b1*a21; lam1 = D2/D;
		if ((lam1 <= 0.0f) || (lam1 >= 1.0f)) continue;
		if (lam0 + lam1 >= 1.0f) continue;
		Dif = P - v0 - lam0*vec0 - lam1*vec1;
		curdist = Dif.x*Dif.x + Dif.y*Dif.y + Dif.z*Dif.z;
		if (curdist >= extdist) continue;
		if (gotFace)
			{ if (curdist < fdist) { faceIndex = i; fdist = curdist; fext0 = i0; fext1 = i1; fext2 = i2; flam0 = lam0; flam1 = lam1; } }
		 else { gotFace = TRUE; faceIndex = i; fdist = curdist; fext0 = i0; fext1 = i1; fext2 = i2; flam0 = lam0; flam1 = lam1; }
	}

	faces = mesh.faces;
	if (gotFace)
	{
		v0 = verts[fext0]; v1 = verts[fext1]; v2 = verts[fext2];
		C = v0 + flam0*(v1-v0) + flam1*(v2-v0);
		N = Normalize((v1-v0)^(v2-v1));
		return TRUE;
	} else if (gotEdge)
	{
		v0 = verts[eext0]; v1 = verts[eext1];
		C = (1.0f-elam)*v0 + elam*v1;
		v0 = Normalize(mesh.getNormal(eext0)); v1 = Normalize(mesh.getNormal(eext1));
		N = SN = (1.0f-elam)*v0 + elam*v1;
		faceIndex = faceAdj[0];
		float dev = -2.0f;
		for(i=0; i<faceAdj.Count(); i++) {
			v0 = verts[faces[faceAdj[i]].v[0]]; v1 = verts[faces[faceAdj[i]].v[1]]; v2 = verts[faces[faceAdj[i]].v[2]];
			Point3 curSN = Normalize((v1-v0)^(v2-v1));
			float curDev = DotProd(N, curSN);
			if (curDev > dev) {
				faceIndex = faceAdj[i];
				dev = curDev;
				SN = curSN;
			}
		}
		N = SN;
		return TRUE;
	} else if (gotVertex)
	{
		C = verts[vext];
		Point3 gNormal = mesh.getNormal(vext);
		float lenSq = LengthSquared(gNormal);
		if (!((lenSq > 0.0f) && (lenSq < 2.0f))) gNormal = Point3::XAxis;
		N = SN = Normalize(gNormal);
		faceIndex = faceAdj[0];
		float dev = -2.0f;
		for(i=0; i<faceAdj.Count(); i++) {
			v0 = verts[faces[faceAdj[i]].v[0]]; v1 = verts[faces[faceAdj[i]].v[1]]; v2 = verts[faces[faceAdj[i]].v[2]];
			Point3 curSN = Normalize((v1-v0)^(v2-v1));
			float curDev = DotProd(N, curSN);
			if (curDev > dev) {
				faceIndex = faceAdj[i];
				dev = curDev;
				SN = curSN;
			}
		}
		N = SN;
		return TRUE;
	}

	return FALSE;
}

// chip off mesh using the box mesh in global space and then transform back to local space
bool PFOperatorMarkShape::ChipOffMesh(const Mesh &mesh, const Mesh &boxMesh, const Matrix3 &boxTM, Mesh &markMesh, BOOL generateMapping)
{
	int i, j;
	MeshDelta meshDelta;
	markMesh = boxMesh;

	Point3 org = markMesh.verts[0];
	Point3 uVec = markMesh.verts[1] - org;
	Point3 vVec = markMesh.verts[2] - org;
	float uLen = uVec.LengthUnify();
	float vLen = vVec.LengthUnify();
	if (uLen > 0.0f) uVec /= uLen;
	if (vLen > 0.0f) vVec /= vLen;

	// transform bounding box to global space
	for (i = 0; i < 8; i++)
		markMesh.verts[i] = boxTM.PointTransform(markMesh.verts[i]);

	Tab<Point3> normal;
	Tab<float> offset;
	normal.SetCount(6);
	offset.SetCount(6);
	
	// slice the mesh by 6 planes
	for (i = j = 0; i < 6; i++, j += 2) {
		normal[i] = -(markMesh.FaceNormal(j, TRUE));
		offset[i] = markMesh.verts[markMesh.faces[j].v[0]] % normal[i];
	}
	markMesh = mesh;

	// filter faces outside the box to optimize slice operation (bayboro 12-19-02)
	BitArray fdel; // array of faces to be deleted
	BitArray inRange, inRangeSet; // array for vertices to indicate that a vertex is under the plane
	int numF = markMesh.getNumFaces();
	fdel.SetSize(numF);
	fdel.ClearAll();
	int numV = markMesh.getNumVerts();
	inRange.SetSize(numV);
	inRangeSet.SetSize(numV);
	for(i = 0; i < 6; i++) {
		inRange.ClearAll();
		inRangeSet.ClearAll();
		for(j=0; j<numF; j++) {
			if (fdel[j]) continue;
			bool removeFace = true;
			for(int k=0; k<3; k++) {
				int vertIndex = markMesh.faces[j].v[k];
				if (!inRangeSet[vertIndex]) {
					inRangeSet.Set(vertIndex);
					if (DotProd(markMesh.verts[vertIndex], normal[i]) >= offset[i]) inRange.Set(vertIndex);
				}
				if (inRange[vertIndex]) {
					removeFace = false;
					break;
				}
			}
			if (removeFace)	fdel.Set(j);
		} 
	}
	meshDelta.InitToMesh(markMesh);
	meshDelta.FDelete(fdel);
	meshDelta.Apply(markMesh);
	meshDelta.ClearAllOps();
	// end of optimization 

	for (i = 0; i < 6; i++) {
		// AdjEdgeList manipulation is a workaround for the problem in MeshDelta::Slice module
		// see e-mail on 1/10/2003 from Andy. The source of the problem should be fixed in the core later
		AdjEdgeList* ae = new AdjEdgeList(markMesh);
		meshDelta.Slice(markMesh, normal[i], offset[i], FALSE, TRUE, NULL, ae);
//		meshDelta.Slice(markMesh, normal[i], offset[i], FALSE, TRUE);
		meshDelta.Apply(markMesh);
		delete ae;
	}

	// transform the chipped off mesh back to local space
	Matrix3 inverseTM = Inverse(boxTM);
	inverseTM.TransformPoints(markMesh.verts, markMesh.getNumVerts());

	// set mapping coordinates
	if (generateMapping) {
		markMesh.setNumTVerts(markMesh.getNumVerts());
		markMesh.setNumTVFaces(markMesh.getNumFaces());
		for (i = 0; i < markMesh.getNumVerts(); i++) {
			Point3 vec = markMesh.verts[i] - org;
			markMesh.setTVert(i, UVVert((vec % uVec), (vec % vVec), 0.0f));
		}
		for (i = 0; i < markMesh.getNumFaces(); i++) {
			Face &f = markMesh.faces[i];
			markMesh.tvFace[i].setTVerts(f.v[0], f.v[1], f.v[2]);
		}
	} else {
		markMesh.setNumTVerts(0);
		markMesh.setNumTVFaces(0);
	}

	return true;
}

void PFOperatorMarkShape::ContactElementMesh(Mesh &mesh, const Matrix3 &meshTM, const Point3 &point, const Point3 &vector)
{
	AdjEdgeList aeList(mesh);
	AdjFaceList afList(mesh, aeList);
	FaceElementList elemList(mesh, afList);
	if (elemList.count <= 1) return;	// single element

	Matrix3 inverseTM = Inverse(meshTM);
	Ray vecRay;
	DWORD elem, fi;
	Point3 normal, bary;
	vecRay.p = inverseTM.PointTransform(point);
	vecRay.dir = inverseTM.VectorTransform(vector);
	vecRay.dir.Unify();
	// find the element that's intersected by the vector
	// changed calcs to the closest point (Bayboro 03-12-2002)
/*	float at;
	if (!mesh.IntersectRay(vecRay, at, normal, fi, bary)) {
		Mesh flippedMesh = mesh;
		for (int i = 0; i < flippedMesh.getNumFaces(); i++)
			flippedMesh.FlipNormal(i);
		vecRay.dir = -(vecRay.dir);
		if (!flippedMesh.IntersectRay(vecRay, at, normal, fi, bary))
			fi = ClosestFaceOnMesh(mesh, vecRay.p);
	} */
	fi = ClosestFaceOnMesh(mesh, vecRay.p);
	// end of change (Bayboro 03-12-2002)

	elem = elemList[fi];
	// clone faces in the intersected element
	MeshDelta meshDelta;
	Mesh newMesh;
	BitArray fset;
	fset.SetSize(mesh.getNumFaces());
	for (int i = 0; i < mesh.getNumFaces(); i++)
		fset.Set(i, (elemList[i] == elem));
	meshDelta.Detach(mesh, &newMesh, fset, TRUE, FALSE, FALSE);
	mesh = newMesh;
}

void PFOperatorMarkShape::SurfaceOffsetMesh(Mesh &mesh, float offset)
{
	if (offset == 0.0f) return;

	Point3 *verts = mesh.verts;
	for (int i = 0; i < mesh.getNumVerts(); i++)
		verts[i].z += offset;
}

// build contact mesh in global space
void PFOperatorMarkShape::BuildContactMesh(const TimeValue t)
{
	_contactMesh() = Mesh();
	INode *iNode = pblock()->GetINode(kMarkShape_contactObj, t);
	if (iNode == NULL) return;
	
	bool deleteIt;
	MeshDelta meshDelta;
	Tab<INode*> stackNodes;
	
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	stackNodes.Append(1, &iNode, 10);
	for (int index = 0; index < stackNodes.Count(); index++)
	{
		INode *node = stackNodes[index];
		TriObject *triObj = GetTriObjectFromNode(node, t, deleteIt);
		if (triObj) {
			Matrix3 transTM = node->GetObjectTM(t);
			meshDelta.AttachMesh(_contactMesh(), triObj->GetMesh(), transTM, 0);
			meshDelta.Apply(_contactMesh());
		}
		if (deleteIt) triObj->DeleteMe();
		// add children nodes to the stack
		for (int i = 0; i < node->NumberOfChildren(); i++) {
			INode *childNode = node->GetChildNode(i);
			if (childNode)	stackNodes.Append(1, &childNode, 10);
		}
	}
	if (wasHolding) theHold.Resume();

	// make flipped contact mesh
	_flippedMesh() = _contactMesh();
	for (int i = 0; i < flippedMesh().getNumFaces(); i++)
		_flippedMesh().FlipNormal(i);
}

// Create a square mesh (xdim == ydim == 1.0)
void PFOperatorMarkShape::CreateRectangleMesh()
{
	_rectMesh().setNumVerts(4);
	_rectMesh().setNumFaces(2);

	_rectMesh().setVert(0, Point3(-0.5f, -0.5f, 0.0f));
	_rectMesh().setVert(1, Point3( 0.5f, -0.5f, 0.0f));
	_rectMesh().setVert(2, Point3(-0.5f,  0.5f, 0.0f));
	_rectMesh().setVert(3, Point3( 0.5f,  0.5f, 0.0f));

	Face f;

	f.v[0] = 2;  f.v[1] = 0;  f.v[2] = 3;  	f.smGroup = 1;  f.flags = 5; _rectMesh().faces[0] = f;
	f.v[0] = 1;  f.v[1] = 3;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 5; _rectMesh().faces[1] = f;
}

// Create a cube mesh (xdim == ydim == zdim == 1.0)
void PFOperatorMarkShape::CreateBoxMesh()
{
	_boxMesh().setNumVerts(8);
	_boxMesh().setNumFaces(12);

	float hsize = 0.5f;
	float nhsize = -0.5f;
	_boxMesh().setVert(0, Point3(nhsize, nhsize, nhsize));
	_boxMesh().setVert(1, Point3(hsize,  nhsize, nhsize));
	_boxMesh().setVert(2, Point3(nhsize, hsize,  nhsize));
	_boxMesh().setVert(3, Point3(hsize,  hsize,  nhsize));
	_boxMesh().setVert(4, Point3(nhsize, nhsize, hsize));
	_boxMesh().setVert(5, Point3(hsize,  nhsize, hsize));
	_boxMesh().setVert(6, Point3(nhsize, hsize,  hsize));
	_boxMesh().setVert(7, Point3(hsize,  hsize,  hsize));

	Face f;

	f.v[0] = 0;  f.v[1] = 2;  f.v[2] = 3;  	f.smGroup = 2;   f.flags = 65603;  _boxMesh().faces[0] = f;
	f.v[0] = 3;  f.v[1] = 1;  f.v[2] = 0;  	f.smGroup = 2;   f.flags = 65603;  _boxMesh().faces[1] = f;
	f.v[0] = 4;  f.v[1] = 5;  f.v[2] = 7;  	f.smGroup = 4;   f.flags = 67;     _boxMesh().faces[2] = f;
	f.v[0] = 7;  f.v[1] = 6;  f.v[2] = 4;  	f.smGroup = 4;   f.flags = 67;     _boxMesh().faces[3] = f;
	f.v[0] = 0;  f.v[1] = 1;  f.v[2] = 5;  	f.smGroup = 8;   f.flags = 262211; _boxMesh().faces[4] = f;
	f.v[0] = 5;  f.v[1] = 4;  f.v[2] = 0;  	f.smGroup = 8;	 f.flags = 262211; _boxMesh().faces[5] = f;
	f.v[0] = 1;  f.v[1] = 3;  f.v[2] = 7;  	f.smGroup = 16;  f.flags = 196675; _boxMesh().faces[6] = f;
	f.v[0] = 7;  f.v[1] = 5;  f.v[2] = 1;  	f.smGroup = 16;  f.flags = 196675; _boxMesh().faces[7] = f;
	f.v[0] = 3;  f.v[1] = 2;  f.v[2] = 6;  	f.smGroup = 32;  f.flags = 327747; _boxMesh().faces[8] = f;
	f.v[0] = 6;  f.v[1] = 7;  f.v[2] = 3;  	f.smGroup = 32;  f.flags = 327747; _boxMesh().faces[9] = f;
	f.v[0] = 2;  f.v[1] = 0;  f.v[2] = 4;  	f.smGroup = 64;  f.flags = 131139; _boxMesh().faces[10] = f;
	f.v[0] = 4;  f.v[1] = 6;  f.v[2] = 2;  	f.smGroup = 64;  f.flags = 131139; _boxMesh().faces[11] = f;
}

void PFOperatorMarkShape::BuildRectangleMesh()
{
	float pivotOffset = pblock()->GetFloat(kMarkShape_pivotOffset);
//	float surfaceOffset = pblock()->GetFloat(kMarkShape_surfaceOffset);
	float surfaceOffset = 0.0f;
	float maxX = 0.5f - pivotOffset;
	float minX = -0.5f - pivotOffset;
	float maxY = 0.5f;
	float minY = -0.5f;

	_rectMesh().setVert(0, Point3(minX, minY, surfaceOffset));
	_rectMesh().setVert(1, Point3(maxX, minY, surfaceOffset));
	_rectMesh().setVert(2, Point3(minX, maxY, surfaceOffset));
	_rectMesh().setVert(3, Point3(maxX, maxY, surfaceOffset));

	if (pblock()->GetInt(kMarkShape_generateMapping)) {
		_rectMesh().setNumTVerts(4);
		_rectMesh().setNumTVFaces(2);
		_rectMesh().setTVert(0, UVVert(0.0f, 0.0f, 0.0f));
		_rectMesh().setTVert(1, UVVert(1.0f, 0.0f, 0.0f));
		_rectMesh().setTVert(2, UVVert(0.0f, 1.0f, 0.0f));
		_rectMesh().setTVert(3, UVVert(1.0f, 1.0f, 0.0f));
		_rectMesh().tvFace[0].setTVerts(2, 0, 3);
		_rectMesh().tvFace[1].setTVerts(1, 3, 0);
	} else {
		_rectMesh().setNumTVerts(0);
		_rectMesh().setNumTVFaces(0);
	}
}

void PFOperatorMarkShape::BuildBoxMesh(Point3 &dimensions)
{
	const float pivotOffset = pblock()->GetFloat(kMarkShape_pivotOffset);
	float maxX = (0.5f - pivotOffset) * dimensions.x;
	float minX = maxX - dimensions.x;
	float maxY = 0.5f * dimensions.y;
	float minY = -maxY;
	float maxZ = 0.5f * dimensions.z;
	float minZ = -maxZ;

	_boxMesh().setVert(0, Point3(minX, minY, minZ));
	_boxMesh().setVert(1, Point3(maxX, minY, minZ));
	_boxMesh().setVert(2, Point3(minX, maxY, minZ));
	_boxMesh().setVert(3, Point3(maxX, maxY, minZ));
	_boxMesh().setVert(4, Point3(minX, minY, maxZ));
	_boxMesh().setVert(5, Point3(maxX, minY, maxZ));
	_boxMesh().setVert(6, Point3(minX, maxY, maxZ));
	_boxMesh().setVert(7, Point3(maxX, maxY, maxZ));
}

// return an align direction that's perpendicular to the normal direction
Point3 PFOperatorMarkShape::GetAlignDirection(const TimeValue t, const Point3& normal, const Point3& speed, const Quat& orientation, RandGenerator* randGen)
{
	int alignTo = pblock()->GetInt(kMarkShape_alignTo, t);
	float divergence = (alignTo == kMarkShape_alignTo_random) ? 180.0f : GetPFFloat(pblock(), kMarkShape_divergence, t);
	Point3 alignDir;
	Matrix3 orientTM;

	switch (alignTo)
	{
	case kMarkShape_alignTo_speed:
	case kMarkShape_alignTo_random:
		alignDir = speed;
		alignDir.Unify();
		break;
	case kMarkShape_alignTo_particleX:
	case kMarkShape_alignTo_particleY:
	case kMarkShape_alignTo_particleZ:
		orientation.MakeMatrix(orientTM);
		switch (alignTo)
		{
		case kMarkShape_alignTo_particleX:
			alignDir = orientTM.GetRow(0);
			break;
		case kMarkShape_alignTo_particleY:
			alignDir = orientTM.GetRow(1);
			break;
		case kMarkShape_alignTo_particleZ:
			alignDir = orientTM.GetRow(2);
			break;
		}
		break;
	}

	if (divergence != 0.0f) {
		float theta = DegToRad(divergence) * randGen->Rand11();
		Point3 vertDir = normal ^ alignDir;
		vertDir.Unify();
		alignDir = (float)cos(theta) * alignDir + (float)sin(theta) * vertDir;
		alignDir.Unify();
	}

	return alignDir;
}

float PFOperatorMarkShape::GetScaleFactor(RandGenerator* randGen, float variation)
{
	return 1.0f + variation * randGen->Rand11();
}

void OrthogonalizeByZthenX(Matrix3& tm)
{
	Point3 x = tm.GetRow(0);
	Point3 y = tm.GetRow(1);
	Point3 z = tm.GetRow(2);
	Point3 zNorm = Normalize(z);
	x -= DotProd(x, zNorm)*zNorm;
	y -= DotProd(y, zNorm)*zNorm;
	Point3 xNorm = Normalize(x);
	y -= DotProd(y, xNorm)*xNorm;
	tm.SetRow(0, x);
	tm.SetRow(1, y);
}

} // end of namespace PFActions