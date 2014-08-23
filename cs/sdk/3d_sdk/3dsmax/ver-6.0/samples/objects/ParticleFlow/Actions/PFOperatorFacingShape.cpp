/**********************************************************************
 *<
	FILE:			PFOperatorFacingShape.cpp

	DESCRIPTION:	FacingShape Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include <limits>

#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorFacingShape.h"

#include "PFOperatorFacingShape_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Creates a transferable particle channel to store a scale factor for each particle
// when it's in screen space.
#define PARTICLECHANNELLOCALSCALEFACTORR_INTERFACE Interface_ID(0x1e7e7444, 0x1eb34500) 
#define PARTICLECHANNELLOCALSCALEFACTORW_INTERFACE Interface_ID(0x1e7e7444, 0x1eb34501) 

#define GetParticleChannelLocalScaleFactorRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELLOCALSCALEFACTORR_INTERFACE)) 
#define GetParticleChannelLocalScaleFactorWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELLOCALSCALEFACTORW_INTERFACE)) 

// Creates a transferable particle channel to store a random orientation offset
#define PARTICLECHANNELORIENTOFFSETR_INTERFACE Interface_ID(0x2e8c5dbf, 0x1eb34500) 
#define PARTICLECHANNELORIENTOFFSETW_INTERFACE Interface_ID(0x2e8c5dbf, 0x1eb34501) 

#define GetParticleChannelOrientOffsetRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELORIENTOFFSETR_INTERFACE)) 
#define GetParticleChannelOrientOffsetWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELORIENTOFFSETW_INTERFACE)) 


// Pick node call back
PFOperatorFacingShapePickNodeCallback	ThePFOperatorFacingShapePickNodeCallback;
// Pick mode call back
PFOperatorFacingShapePickModeCallback	ThePFOperatorFacingShapePickModeCallback;

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorFacingShapeState					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorFacingShapeState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorFacingShapeState::IObject					 |
//+--------------------------------------------------------------------------+
void PFOperatorFacingShapeState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorFacingShapeState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorFacingShapeState::GetClassID() 
{ 
	return PFOperatorFacingShapeState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorFacingShapeState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorFacingShapeState::Save(ISave* isave) const
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
	if ((res = isave->Write(randGenRot(), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorFacingShapeState::IPFActionState					 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorFacingShapeState::Load(ILoad* iload)
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
			res=iload->Read(_randGenRot(), sizeof(RandGenerator), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorFacingShape						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorFacingShape::PFOperatorFacingShape()
{ 
	_object() = NULL;
	_material() = NULL;
	GetClassDesc()->MakeAutoParamBlocks(this);
	CreateMesh();
}

FPInterfaceDesc* PFOperatorFacingShape::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &facingShape_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &facingShape_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &facingShape_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorFacingShape::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_FACINGSHAPE_CLASS_NAME);
}

Class_ID PFOperatorFacingShape::ClassID()
{
	return PFOperatorFacingShape_Class_ID;
}

void PFOperatorFacingShape::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorFacingShape::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefTargetHandle PFOperatorFacingShape::GetReference(int i)
{
	switch (i)
	{
	case kFacingShape_reference_pblock:		return (RefTargetHandle)pblock();
	case kFacingShape_reference_object:		return (RefTargetHandle)object();
	case kFacingShape_reference_material:	return (RefTargetHandle)material();
	}
	return NULL;
}

void PFOperatorFacingShape::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case kFacingShape_reference_pblock:		
		_pblock() = (IParamBlock2*)rtarg;	
		break;
	case kFacingShape_reference_object:		
		_object() = (INode *)rtarg;			
		RefreshUI(kFacingShape_message_name);
		RefreshUI(kFacingShape_message_size);
		NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
		updateFromRealRefObject();
		break;
	case kFacingShape_reference_material:	
		_material() = (Mtl *)rtarg;			
		break;
	}
}

RefResult PFOperatorFacingShape::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
													 PartID& partID, RefMessage message)
{
	if ( hTarget == pblock() ) {
		int lastUpdateID;
		switch (message)
		{
		case REFMSG_CHANGE:
			lastUpdateID = pblock()->LastNotifyParamID();
			if (lastUpdateID == kFacingShape_objectMaxscript) {
				if (IsIgnoringRefNodeChange()) return REF_STOP;
				if (updateFromMXSRefObject()) {
					RefreshUI(kFacingShape_message_name);
					RefreshUI(kFacingShape_message_size);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				}
				return REF_STOP;
			}
			if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
			switch ( lastUpdateID )
			{
			case kFacingShape_parallel:
				if (pblock()->GetInt(kFacingShape_parallel) &&
					pblock()->GetInt(kFacingShape_sizeSpace) == kFacingShape_sizeSpace_screen)
					pblock()->SetValue(kFacingShape_sizeSpace, 0, kFacingShape_sizeSpace_world);
				// continue here to refresh the size group UI
			case kFacingShape_sizeSpace:
				RefreshUI(kFacingShape_message_size);
				break;
			case kFacingShape_pivotAt:
			case kFacingShape_WHRatio:
				BuildMesh(GetCOREInterface()->GetTime());
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealRefObject();
			break;
		// Initialization of Dialog
		case kFacingShape_RefMsg_InitDlg:
			RefreshUI(kFacingShape_message_name, (IParamMap2*)partID);
			RefreshUI(kFacingShape_message_size, (IParamMap2*)partID);
			return REF_STOP;
		// Pick Look At Object
		case kFacingShape_RefMsg_PickObj:
			ThePFOperatorFacingShapePickModeCallback.Init(this);
			GetCOREInterface()->SetPickMode(&ThePFOperatorFacingShapePickModeCallback);
			return REF_STOP;
			// New Random Seed
		case kFacingShape_RefMsg_NewRand:
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
			if (theHold.Holding()) {
				if (pblock()->GetInt(kFacingShape_sizeSpace) == kFacingShape_sizeSpace_screen) {
					// check that the object is camera
					if (!IsCameraObject(_object()))
						pblock()->SetValue(kFacingShape_sizeSpace, 0, kFacingShape_sizeSpace_world);
				}
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			RefreshUI(kFacingShape_message_name);
			break;
		case REFMSG_TARGET_DELETED:
			_object() = NULL;
			RefreshUI(kFacingShape_message_name);
			RefreshUI(kFacingShape_message_size);
			break;
		}
	}

	if ( hTarget == const_cast <Mtl*> (material()) ) {
		switch (message)
		{
		case REFMSG_CHANGE:
			break;
		case REFMSG_TARGET_DELETED:
			_material() = NULL;
			break;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorFacingShape::Clone(RemapDir &remap)
{
	PFOperatorFacingShape* newOp = new PFOperatorFacingShape();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	if (object()) newOp->ReplaceReference(kFacingShape_reference_object, _object());
	if (material()) newOp->ReplaceReference(kFacingShape_reference_material, _material());
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorFacingShape::GetObjectName()
{
	return GetString(IDS_OPERATOR_FACINGSHAPE_OBJECT_NAME);
}

bool PFOperatorFacingShape::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	// build mesh
	BuildMesh(0);

	// initialize random number generators
	return	_randLinkerVar().Init( pCont, GetRand() ) &&
			_randLinkerRot().Init( pCont, GetRand() );
}

bool PFOperatorFacingShape::Release(IObject* pCont)
{
	// release random number generators
	_randLinkerVar().Release( pCont );
	_randLinkerRot().Release( pCont );
	return true;
}

const ParticleChannelMask& PFOperatorFacingShape::ChannelsUsed(const Interval& time) const
{
								// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_Position|PCU_Orientation|PCU_Spin,
								// write channels
									PCU_EventStart|PCU_Orientation|PCU_Spin|PCU_Scale|PCU_Shape|PCU_MtlIndex);
	return mask;
}

int	PFOperatorFacingShape::GetRand()
{
	return pblock()->GetInt(kFacingShape_randomSeed);
}

void PFOperatorFacingShape::SetRand(int seed)
{
	pblock()->SetValue(kFacingShape_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorFacingShape::GetCurrentState(IObject* pContainer)
{
	PFOperatorFacingShapeState* actionState = (PFOperatorFacingShapeState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorFacingShapeState_Class_ID);
	RandGenerator* randGenVar = randLinkerVar().GetRandGenerator(pContainer);
	if (randGenVar != NULL)
		memcpy(actionState->_randGenVar(), randGenVar, sizeof(RandGenerator));
	RandGenerator* randGenRot = randLinkerRot().GetRandGenerator(pContainer);
	if (randGenRot != NULL)
		memcpy(actionState->_randGenRot(), randGenRot, sizeof(RandGenerator));
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorFacingShape::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorFacingShapeState* actionState = (PFOperatorFacingShapeState*)aSt;
	RandGenerator* randGen = randLinkerVar().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenVar(), sizeof(RandGenerator));
	}
	randGen = randLinkerRot().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGenRot(), sizeof(RandGenerator));
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorFacingShape::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_FACINGSHAPE_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorFacingShape::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_FACINGSHAPE_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorFacingShape::HasDynamicName(TSTR& nameSuffix)
{
	if (object() == NULL) nameSuffix = GetString(IDS_NONE);
	else nameSuffix = _object()->GetName();
	return true;
}

bool PFOperatorFacingShape::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	const BOOL parallelDir = pblock()->GetInt(kFacingShape_parallel);
	const int sizeSpace = pblock()->GetInt(kFacingShape_sizeSpace);
	const float units = pblock()->GetFloat(kFacingShape_units);
	const float inheritScl = pblock()->GetFloat(kFacingShape_inheritedScale);
	const float proportion = pblock()->GetFloat(kFacingShape_proportion);
	const float variation = pblock()->GetFloat(kFacingShape_variation);
	int orientType = pblock()->GetInt(kFacingShape_orientation);

	const bool isCameraObj = IsCameraObject(_object());
	const bool okScreen = isCameraObj && !parallelDir;
	bool inWorld = (sizeSpace == kFacingShape_sizeSpace_world);
	bool inLocal = (sizeSpace == kFacingShape_sizeSpace_local);
	bool inScreen = (sizeSpace == kFacingShape_sizeSpace_screen);

	if (!okScreen && inScreen) {
		inScreen = false;
		inWorld = true;
	}

	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't find particle times in the container
	// may not have the position channel. Then the exact orientation is not guaranteed
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	// may not have the spinning channel. If it is not present then it is not needed
	IParticleChannelAngAxisR* chSpinR = GetParticleChannelSpinRInterface(pCont);
	IParticleChannelPoint3R* chSpeed = NULL;
	if (orientType == kFacingShape_orientation_speed) {
		chSpeed = GetParticleChannelSpeedRInterface(pCont);
		if (chSpeed == NULL) orientType = kFacingShape_orientation_horizon;
	}

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	IParticleChannelFloatR* chOrientOffsetR = NULL;
	IParticleChannelFloatW* chOrientOffsetW = NULL;
	bool initOrientOffset = false;
	if (orientType == kFacingShape_orientation_random) {
		chOrientOffsetR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELORIENTOFFSETR_INTERFACE,
																	ParticleChannelFloat_Class_ID,
																	true, PARTICLECHANNELORIENTOFFSETR_INTERFACE,
																	PARTICLECHANNELORIENTOFFSETW_INTERFACE, true,
																	actionNode, (Object*)this, &initOrientOffset);
		if (chOrientOffsetR == NULL) return false; // can't read orient offset channel
		chOrientOffsetW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELORIENTOFFSETW_INTERFACE, (Object*)this);
		if (chOrientOffsetW == NULL) return false; // can't write orient offset channel
	}

	IParticleChannelQuatR* chOrientR = NULL;
	IParticleChannelQuatW* chOrientW = NULL;
	if (object()) {
		chOrientW = (IParticleChannelQuatW*)chCont->EnsureInterface(PARTICLECHANNELORIENTATIONW_INTERFACE,
																ParticleChannelQuat_Class_ID,
																true, PARTICLECHANNELORIENTATIONR_INTERFACE,
																PARTICLECHANNELORIENTATIONW_INTERFACE, true );
		if (chOrientW == NULL) return false;	// can't modify Orientation channel in the container
		chOrientR = GetParticleChannelOrientationRInterface(pCont);
		if (chOrientR == NULL) return false; // can't read current orientation
	}

	bool initScale = false;
	IParticleChannelPoint3W* chScaleW = NULL;
	IParticleChannelPoint3R* chScaleR = NULL;
	chScaleW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSCALEW_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSCALER_INTERFACE,
																PARTICLECHANNELSCALEW_INTERFACE, true,
																actionNode, NULL, &initScale );
	if (chScaleW == NULL) return false;	// can't modify Scale channel in the container
	chScaleR = GetParticleChannelScaleRInterface(pCont);
	if (chScaleR == NULL) return false;

	IParticleChannelMeshW* chMeshW = NULL;
	IParticleChannelMeshR* chMeshR = NULL;
	chMeshW = (IParticleChannelMeshW*)chCont->EnsureInterface(PARTICLECHANNELSHAPEW_INTERFACE,
																ParticleChannelMesh_Class_ID,
																true, PARTICLECHANNELSHAPER_INTERFACE,
																PARTICLECHANNELSHAPEW_INTERFACE, true);
	if (chMeshW == NULL) return false; // can't modify Shape channel in the container
	chMeshR = GetParticleChannelShapeRInterface(pCont);
	if (chMeshR == NULL) return false;

	IParticleChannelFloatR* chLocalSclR = NULL;
	IParticleChannelFloatW* chLocalSclW = NULL;
	bool initLocalScale = false;
	if (inScreen && variation != 0.0) {
		// acquire LocalScaleFactor particle channel; if not present then create it.		
		chLocalSclR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELLOCALSCALEFACTORR_INTERFACE,
																ParticleChannelFloat_Class_ID,
																true, PARTICLECHANNELLOCALSCALEFACTORR_INTERFACE,
																PARTICLECHANNELLOCALSCALEFACTORW_INTERFACE, true,
																actionNode, (Object*)this, &initLocalScale);
		if (chLocalSclR == NULL) return false;
		chLocalSclW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALSCALEFACTORW_INTERFACE, (Object*)this);
		if (chLocalSclW == NULL) return false;
	}
	
	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();
	bool wasIgnoringEmitterTMChange = IsIgnoringEmitterTMChange();
	if (!wasIgnoringEmitterTMChange) SetIgnoreEmitterTMChange();

	// cache target position & PF position
	TimeValue timeValueStart = floor(float(timeStart) - 1.0f);
	TimeValue timeValueEnd = ceil(float(timeEnd));
	bool isTargetAnimated = false, isEdpAnimated = false;
	Tab<Point3> cachedTargetPos, cachedEdpPos;
	Point3 targetPos, pfPos, dirVector;
	if (object()) {
		isTargetAnimated = IsNodeAnimated(_object());
		if (isTargetAnimated)
			CacheNodePosition(_object(), timeValueStart, timeValueEnd, cachedTargetPos);
		else
			targetPos = _object()->GetObjectTM(timeValueStart).GetTrans();
	}
	if (object() && parallelDir) {
		isEdpAnimated = IsNodeAnimated(pNode);
		if (isEdpAnimated)
			CacheNodePosition(pNode, timeValueStart, timeValueEnd, cachedEdpPos);
		else
			pfPos = pNode->GetObjectTM(timeValueStart).GetTrans();
	}

	RandGenerator* randVarGen = randLinkerVar().GetRandGenerator(pCont);
	RandGenerator* randRotGen = randLinkerRot().GetRandGenerator(pCont);
	int amount = chAmount->Count();
	float scale, screenPro = proportion;
	
	// set scale for all particles if it's in world space & there's no variation
//	if (!chNew->IsAllOld()) {
//		if (inWorld && variation == 0.0f)
//			chScaleW->SetValue(Point3(units, units, units));
//	}

	Quat resRot;
	AngAxis curSpin, revSpin;
	Tab<int> toSetShape;
	for (int i = 0; i < amount; i++)
	{
		bool isNew = chNew->IsNew(i);
		PreciseTimeValue time = chTime->GetValue(i);
		
		if (isNew) {
			if ((orientType == kFacingShape_orientation_random) && (initOrientOffset))
				chOrientOffsetW->SetValue(i, randRotGen->Rand01()*TWOPI);
			if (initScale) chScaleW->SetValue(i, Point3(1.0f, 1.0f, 1.0f));
			toSetShape.Append(1, &i, 1024);
		}

		// Set particle orientation according to object location and parallel parameter
		if (object()) {
			// compute a direction vector
			Matrix3 orientTM;
			if (isTargetAnimated)	// compute target position
				InterpolatePosition(time, timeValueStart, cachedTargetPos, targetPos);
			if (parallelDir) {		// compute PF/particle position
				if (isEdpAnimated)
					InterpolatePosition(time, timeValueStart, cachedEdpPos, pfPos);
			} else {
				pfPos = (chPos != NULL) ? chPos->GetValue(i) : Point3::Origin;
			}
			dirVector = targetPos - pfPos;
			Point3 speed, xAxis, yAxis, zAxis;
			Quat q;
			// compute an orientation from the direction vector (align to Z+)
			switch(orientType) {
			case kFacingShape_orientation_horizon:
				ArbAxis(dirVector.Unify(), orientTM);
				chOrientW->SetValue(i, Quat(orientTM));
				break;
			case kFacingShape_orientation_speed:
				speed = chSpeed->GetValue(i);
				zAxis = dirVector.Unify();
				yAxis = CrossProd(zAxis, speed);
				if (LengthSquared(yAxis) > 0.0) {
					yAxis = yAxis.Unify();
					xAxis = CrossProd(yAxis, zAxis);
					orientTM.Set(xAxis, yAxis, zAxis, Point3::Origin);
					chOrientW->SetValue(i, Quat(orientTM));
				}
				break;
			case kFacingShape_orientation_random:
				ArbAxis(dirVector.Unify(), orientTM);
				orientTM.PreRotateZ(chOrientOffsetR->GetValue(i));
				chOrientW->SetValue(i, Quat(orientTM));
				break;
			case kFacingShape_orientation_spin:
				curSpin = (chSpinR != NULL) ? chSpinR->GetValue(i) : AngAxis(Point3::ZAxis, 0.0f);
				curSpin.angle *= float(timeEnd - time);
				revSpin = curSpin;
				revSpin.angle *= -1.0f;
				resRot = chOrientR->GetValue(i);
				// do a spin projection to the timeEnd;
				resRot = resRot + Quat(curSpin);
				resRot.MakeMatrix(orientTM);
				zAxis = dirVector.Unify();
				xAxis = CrossProd(orientTM.GetRow(2), zAxis);
				if (xAxis.LengthSquared() > 0) {
					xAxis = Normalize(xAxis);
					float theDotProd = DotProd(orientTM.GetRow(2),zAxis);
					float angle = 0.0f;
					if (theDotProd >= 1.0f) angle = 0.0f;
					else if (theDotProd <= -1.0f) angle = PI;
					else angle = acos(theDotProd);
					orientTM = orientTM*RotAngleAxisMatrix(xAxis, angle);
					// do a rollback in orientation to foresee the offset
					// that will be done in orientation by integrator while
					// using the spinning component
					chOrientW->SetValue(i, Quat(orientTM) + Quat(revSpin));
				}
				break;
			}
		}

		// Set particle size according to sizeSpace, units, proportion & variation parameters
		if (inWorld) {
			// set individual scale for each new particle if variation is not 0
			if (isNew) {
				scale = units * GetScaleFactor(randVarGen, variation);
				chScaleW->SetValue(i, Point3(scale, scale, scale));
			}
		}
		if (inLocal) {
			if (isNew) {
				const Point3 orgScale = (chScaleR) ? chScaleR->GetValue(i) : Point3(1, 1, 1);
				const Mesh* orgMesh = (chMeshR) ? chMeshR->GetValue(i) : NULL;
				scale = InheritedWidth(orgMesh, orgScale) * inheritScl;
				if (variation != 0.0f)
					scale *= GetScaleFactor(randVarGen, variation);
				chScaleW->SetValue(i, Point3(scale, scale, scale));
			}
		}
		if (inScreen) {
			// keep variation in local channel for new particle
			if (variation != 0.0f) {
				if (isNew) chLocalSclW->SetValue(i, GetScaleFactor(randVarGen, variation));
				screenPro = proportion * chLocalSclR->GetValue(i);
			}
			// compute size of each particle according to its position & camera's parameters
			scale = ProportionWidth(_object(), time, screenPro, (chPos != NULL) ? chPos->GetValue(i) : Point3::Origin);
			chScaleW->SetValue(i, Point3(scale, scale, scale));
		}
	}

	// modify shape for "new" particles
	if (chNew->IsAllNew())
		chMeshW->SetValue(&_mesh());
	else if (!chNew->IsAllOld())
		chMeshW->SetValue(toSetShape, &_mesh());

	if (!wasIgnoringEmitterTMChange) ClearIgnoreEmitterTMChange();
	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

ClassDesc* PFOperatorFacingShape::GetClassDesc() const
{
	return GetPFOperatorFacingShapeDesc();
}

bool PFOperatorFacingShape::IsNodeAnimated(INode *iNode)
{
	if (iNode == NULL) return FALSE;

	BOOL isNodeAnimated = FALSE;
	for (INode* pn = iNode; !(pn->IsRootNode()); pn = pn->GetParentNode())
		if ((isNodeAnimated = pn->GetTMController()->IsAnimated()) == TRUE) break;
	return isNodeAnimated == TRUE;
}

bool PFOperatorFacingShape::IsCameraObject(INode* iNode)
{
	if (iNode == NULL) return false;
	Object* obj = iNode->GetObjectRef();
	if (obj == NULL) return false;
	obj = GetPFObject(obj);
	return (obj->SuperClassID() == CAMERA_CLASS_ID);
}

float PFOperatorFacingShape::InheritedWidth(const Mesh* orgMesh, const Point3& orgScale)
{
	if (orgMesh == NULL) return 0.0f;

	Box3 bbox = (const_cast <Mesh*> (orgMesh))->getBoundingBox();
	Point3 dim = (bbox.Max() - bbox.Min()) * orgScale;
	return (dim.x + dim.y + dim.z) / 3.0f;
}

float PFOperatorFacingShape::ProportionWidth(INode* camNode, TimeValue t, float proportion, const Point3 &pos)
{
	if (camNode == NULL) return 0.0f;

	Interval iv;
	const ObjectState& os = camNode->EvalWorldState(t);
	if (os.obj->SuperClassID() != CAMERA_CLASS_ID) return 0.0f;

	CameraObject *camObj = (CameraObject *)os.obj;
	CameraState camState;
	camObj->EvalCameraState(t, iv, &camState);
	float dist;
	if (camState.isOrtho) {
		if(camNode->GetTarget()) 
			dist = Length(camNode->GetNodeTM(t).GetTrans() - camNode->GetTarget()->GetNodeTM(t).GetTrans());
		else
			dist = camState.tdist;
	} else {
		dist = Length(camNode->GetNodeTM(t).GetTrans() - pos);
	}

	return 2.0f * dist * tan(0.5f * camState.fov * proportion);
}

void PFOperatorFacingShape::CacheNodePosition(INode *iNode, TimeValue timeValueStart, TimeValue timeValueEnd,
											   Tab<Point3> &cachedPos)
{
	if (iNode == NULL) return;

	cachedPos.SetCount(timeValueEnd - timeValueStart + 1);
	TimeValue t;
	int i;
	for (t = timeValueStart, i = 0; t <= timeValueEnd; t++, i++)
		cachedPos[i] = iNode->GetObjectTM(t).GetTrans();
}

void PFOperatorFacingShape::InterpolatePosition(PreciseTimeValue time, TimeValue timeValueStart,
												 Tab<Point3> &cachedPos, Point3 &pos)
{
	if (float(time) < timeValueStart)
		time = PreciseTimeValue(timeValueStart);
	if (float(time) > (timeValueStart + cachedPos.Count() - 1))
		time = PreciseTimeValue(timeValueStart + cachedPos.Count() - 1);
	int tmIndex = int(time) - timeValueStart;
	float ratio = float(time) - int(time);		// -0.5 <= ratio < 0.5
	// readjust tick and fraction for ratio to be positive
	if (ratio < 0.0f) { ratio += 1.0f; tmIndex -= 1; }
	if (ratio == 0.0f)
		pos = cachedPos[tmIndex];
	else
		pos = ((1.0f - ratio) * cachedPos[tmIndex]) + (ratio * cachedPos[tmIndex+1]);
}

// Create a square mesh (width == height == 1.0)
void PFOperatorFacingShape::CreateMesh()
{
	_mesh().setNumVerts(4);
	_mesh().setNumFaces(2);

	_mesh().setVert(0, Point3(-0.5f, -0.5f, 0.0f));
	_mesh().setVert(1, Point3( 0.5f, -0.5f, 0.0f));
	_mesh().setVert(2, Point3(-0.5f,  0.5f, 0.0f));
	_mesh().setVert(3, Point3( 0.5f,  0.5f, 0.0f));

	Face f;

	f.v[0] = 2;  f.v[1] = 0;  f.v[2] = 3;  	f.smGroup = 1;  f.flags = 5; _mesh().faces[0] = f;
	f.v[0] = 1;  f.v[1] = 3;  f.v[2] = 0;  	f.smGroup = 1;  f.flags = 5; _mesh().faces[1] = f;

	// dump from a plane with 4 extra texture vertices
	/*
	_mesh().setNumTVerts(8);
	_mesh().setNumTVFaces(2);

	_mesh().setTVert(0, 0.0f, 0.0f, 0.0f);
	_mesh().setTVert(1, 1.0f, 0.0f, 0.0f);
	_mesh().setTVert(2, 0.0f, 0.0f, 0.0f);
	_mesh().setTVert(3, 1.0f, 0.0f, 0.0f);
	_mesh().setTVert(4, 0.0f, 0.0f, 0.0f);
	_mesh().setTVert(5, 1.0f, 0.0f, 0.0f);
	_mesh().setTVert(6, 0.0f, 1.0f, 0.0f);
	_mesh().setTVert(7, 1.0f, 1.0f, 0.0f);
	_mesh().tvFace[0].setTVerts(6, 4, 7);
	_mesh().tvFace[1].setTVerts(5, 7, 4);
	*/

	_mesh().setNumTVerts(4);
	_mesh().setNumTVFaces(2);

	_mesh().setTVert(0, UVVert(0.0f, 0.0f, 0.0f));
	_mesh().setTVert(1, UVVert(1.0f, 0.0f, 0.0f));
	_mesh().setTVert(2, UVVert(0.0f, 1.0f, 0.0f));
	_mesh().setTVert(3, UVVert(1.0f, 1.0f, 0.0f));
	_mesh().tvFace[0].setTVerts(2, 0, 3);
	_mesh().tvFace[1].setTVerts(1, 3, 0);
}

// Build a mesh according to whRatio (width == 1)
void PFOperatorFacingShape::BuildMesh(const TimeValue t)
{
	int pivotAt = pblock()->GetInt(kFacingShape_pivotAt, t);
	float whRatio = pblock()->GetFloat(kFacingShape_WHRatio, t);
	float hwRatio = (whRatio == 0.0f) ? std::numeric_limits<float>::infinity() : 1.0f / whRatio;
	float bottomY = 0.0f, topY = 0.0f;

	switch (pivotAt)
	{
	case kFacingShape_pivotAt_top:
		topY = 0.0f;			bottomY = -hwRatio;	
		break;
	case kFacingShape_pivotAt_center:
		topY = 0.5f * hwRatio;	bottomY = -topY;
		break;
	case kFacingShape_pivotAt_bottom:
		topY = hwRatio;			bottomY = 0.0f;
		break;
	}

	_mesh().setVert(0, Point3(-0.5f, bottomY, 0.0f));
	_mesh().setVert(1, Point3( 0.5f, bottomY, 0.0f));
	_mesh().setVert(2, Point3(-0.5f, topY,    0.0f));
	_mesh().setVert(3, Point3( 0.5f, topY,    0.0f));
}

float PFOperatorFacingShape::GetScaleFactor(RandGenerator* randGen, float variation)
{
	return 1.0f + variation * randGen->Rand11();
}

void PFOperatorFacingShape::RefreshUI(WPARAM message, IParamMap2* map) const
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

bool PFOperatorFacingShape::verifyRefObjectMXSSync()
{
	if (pblock() == NULL) return true;
	return (object() == pblock()->GetINode(kFacingShape_objectMaxscript, 0));
}

bool hUpdateFacingShapeInProgress = false;
bool PFOperatorFacingShape::updateFromRealRefObject()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return false;
	if (hUpdateFacingShapeInProgress) return false;
	if (verifyRefObjectMXSSync()) return false;
	hUpdateFacingShapeInProgress = true;
	pblock()->SetValue(kFacingShape_objectMaxscript, 0, _object());
	hUpdateFacingShapeInProgress = false;
	return true;
}

bool PFOperatorFacingShape::updateFromMXSRefObject()
{
	if (pblock == NULL) return false;
	if (theHold.RestoreOrRedoing()) return false;
	if (hUpdateFacingShapeInProgress) return false;
	if (verifyRefObjectMXSSync()) return false;
	hUpdateFacingShapeInProgress = true;
	INode* node = pblock()->GetINode(kFacingShape_objectMaxscript, 0);	
	RefResult refR;
	if (node == NULL) {
		refR = DeleteReference(kFacingShape_reference_object);
	} else {
		if (object())
			refR = ReplaceReference(kFacingShape_reference_object, node);
		else
			MakeRefByID(FOREVER, kFacingShape_reference_object, node);
	}
	// set to world space if non-camera object is picked
	if (!IsCameraObject(node) &&
		(pblock()->GetInt(kFacingShape_sizeSpace, 0) == kFacingShape_sizeSpace_screen))
		pblock()->SetValue(kFacingShape_sizeSpace, 0, kFacingShape_sizeSpace_world);
	hUpdateFacingShapeInProgress = false;
	return (refR == REF_SUCCEED);
}

//+--------------------------------------------------------------------------+
//|							Pick Node Callback								 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorFacingShapePickNodeCallback::Filter(INode *iNode)
{
	return TRUE;
}


//+--------------------------------------------------------------------------+
//|							Pick Mode Callback								 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorFacingShapePickModeCallback::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags)
{
	return (ip->PickNode(hWnd, m, GetFilter())) ? TRUE : FALSE;
}

BOOL PFOperatorFacingShapePickModeCallback::Pick(IObjParam *ip, ViewExp *vpt)
{
	INode *iNode = vpt->GetClosestHit();
	if (iNode == NULL) return FALSE;
	if (op() == NULL) return TRUE;

	// set object reference
	theHold.Begin();
	if (op()->object())
		_op()->ReplaceReference(kFacingShape_reference_object, iNode);
	else
		_op()->MakeRefByID(FOREVER, kFacingShape_reference_object, iNode);

	// set to world space if non-camera object is picked
	if (!PFOperatorFacingShape::IsCameraObject(iNode) &&
		(op()->pblock()->GetInt(kFacingShape_sizeSpace, 0) == kFacingShape_sizeSpace_screen))
		op()->pblock()->SetValue(kFacingShape_sizeSpace, 0, kFacingShape_sizeSpace_world);

	if (PFOperatorFacingShape::IsCameraObject(iNode))
		theHold.Accept(GetString(IDS_PICKCAMERA));
	else
		theHold.Accept(GetString(IDS_PICKOBJECT));
	
	return TRUE;
}

void PFOperatorFacingShapePickModeCallback::EnterMode(IObjParam *ip)
{
	if (op()) op()->RefreshUI(kFacingShape_message_enterPick);
	GetCOREInterface()->PushPrompt(GetString(IDS_PICKCAMERAOROBJECT));
}

void PFOperatorFacingShapePickModeCallback::ExitMode(IObjParam *ip)
{
	if (op()) op()->RefreshUI(kFacingShape_message_exitPick);
	GetCOREInterface()->PopPrompt();
}

PickNodeCallback *PFOperatorFacingShapePickModeCallback::GetFilter()
{
	return &ThePFOperatorFacingShapePickNodeCallback;
}



} // end of namespace PFActions