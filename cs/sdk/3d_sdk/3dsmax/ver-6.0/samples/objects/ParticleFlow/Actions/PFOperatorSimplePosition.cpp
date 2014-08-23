/**********************************************************************
 *<
	FILE:			PFOperatorSimplePosition.cpp

	DESCRIPTION:	SimplePosition Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-19-2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimplePosition.h"

#include "PFOperatorSimplePosition_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

// Simple Position creates a particle channel to store a local position for each particle
// when "Lock On Emitter" is turned ON.
// This way the value is calculated only once when a particle enters the event.
#define PARTICLECHANNELLOCALPOSITIONR_INTERFACE Interface_ID(0x7e2c3072, 0x1eb34500) 
#define PARTICLECHANNELLOCALPOSITIONW_INTERFACE Interface_ID(0x7e2c3072, 0x1eb34501) 

#define GetParticleChannelLocalPositionRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELLOCALPOSITIONR_INTERFACE)) 
#define GetParticleChannelLocalPositionWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELLOCALPOSITIONW_INTERFACE)) 


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorSimplePositionState				 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFOperatorSimplePositionState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorSimplePositionState::IObject					 |
//+--------------------------------------------------------------------------+
void PFOperatorSimplePositionState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorSimplePositionState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorSimplePositionState::GetClassID() 
{ 
	return PFOperatorSimplePositionState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFOperatorSimplePositionState::IPFActionState			 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorSimplePositionState::Save(ISave* isave) const
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

	PFOperatorSimplePositionLocalData* d = (PFOperatorSimplePositionLocalData*)data();

	isave->BeginChunk(IPFActionState::kChunkRandGen2);
	if ((res = isave->Write(&(d->RandGenerator()), sizeof(RandGenerator), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData);
	int num = d->DistinctPoints().Count();
	if ((res = isave->Write(&num, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	if (num > 0) {
		isave->BeginChunk(IPFActionState::kChunkData2);
		if ((res = isave->Write(d->DistinctPoints().Addr(0), num*sizeof(Point3), &nb)) != IO_OK) return res;
		isave->EndChunk();
	}

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFOperatorSimplePositionState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFOperatorSimplePositionState::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int num;

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

		case IPFActionState::kChunkRandGen2:
			res=iload->Read(&(_data()->RandGenerator()), sizeof(RandGenerator), &nb);
			break;

		case IPFActionState::kChunkData:
			num = 0;
			res=iload->Read(&num, sizeof(int), &nb);
			if (num > 0) {
				_data()->DistinctPoints().SetCount(num);
				for(int i=0; i<num; i++)
					_data()->DistinctPoints()[i] = Point3::Origin;
			}
			break;

		case IPFActionState::kChunkData2:
			res=iload->Read(_data()->DistinctPoints().Addr(0), sizeof(Point3)*_data()->DistinctPoints().Count(), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorSimplePosition					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
PFOperatorSimplePosition::PFOperatorSimplePosition()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSimplePosition::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simplePosition_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simplePosition_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simplePosition_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimplePosition::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLEPOSITION_CLASS_NAME);
}

Class_ID PFOperatorSimplePosition::ClassID()
{
	return PFOperatorSimplePosition_Class_ID;
}

void PFOperatorSimplePosition::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimplePosition::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimplePosition::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch (message)
	{
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSimplePosition_lock:
				case kSimplePosition_inherit:
					RefreshUI(kSimplePosition_message_lock);
					break;
				case kSimplePosition_type:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// the break is omitted on purpose (bayboro 01-17-2003)
				case kSimplePosition_distinctOnly:
					RefreshUI(kSimplePosition_message_type);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kSimplePosition_RefMsg_InitDlg:
			RefreshUI(kSimplePosition_message_lock, (IParamMap2*)partID);
			RefreshUI(kSimplePosition_message_type, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSimplePosition_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}
	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimplePosition::Clone(RemapDir &remap)
{
	PFOperatorSimplePosition* newOp = new PFOperatorSimplePosition();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimplePosition::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLEPOSITION_OBJECT_NAME);
}

bool PFOperatorSimplePosition::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_pointsMap(pCont).DistinctPoints().ZeroCount();
	_pointsMap(pCont).RandGenerator().srand( GetRand() );
	return PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
}

bool PFOperatorSimplePosition::Release(IObject* pCont)
{
	PFSimpleAction::Release(pCont);
	_pointsMap().erase( pCont );
	return true;
}

const ParticleChannelMask& PFOperatorSimplePosition::ChannelsUsed(const Interval& time) const
{
								//  read	                       &	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time, PCU_Position|PCU_Speed);
	return mask;
}

int	PFOperatorSimplePosition::GetRand()
{
	return pblock()->GetInt(kSimplePosition_randomSeed);
}

void PFOperatorSimplePosition::SetRand(int seed)
{
	pblock()->SetValue(kSimplePosition_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFOperatorSimplePosition::GetCurrentState(IObject* pContainer)
{
	PFOperatorSimplePositionState* actionState = (PFOperatorSimplePositionState*)CreateInstance(REF_TARGET_CLASS_ID, PFOperatorSimplePositionState_Class_ID);
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL)
		memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
	memcpy(&(actionState->_data()->RandGenerator()), &(_pointsMap(pContainer).RandGenerator()), sizeof(RandGenerator));
	actionState->_data()->DistinctPoints() = _pointsMap(pContainer).DistinctPoints();
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorSimplePosition::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFOperatorSimplePositionState* actionState = (PFOperatorSimplePositionState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
	}
	memcpy(&(_pointsMap(pContainer).RandGenerator()), &(actionState->_data()->RandGenerator()), sizeof(RandGenerator));
	_pointsMap(pContainer).DistinctPoints() = actionState->_data()->DistinctPoints();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimplePosition::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEPOSITION_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimplePosition::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEPOSITION_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimplePosition::HasDynamicName(TSTR& nameSuffix)
{
	int type = pblock()->GetInt(kSimplePosition_type, 0);
	int ids;
	switch(type) {
	case kSimplePosition_locationType_pivot:	ids = IDS_LOCATIONTYPE_PIVOT;	break;
	case kSimplePosition_locationType_vertex:	ids = IDS_LOCATIONTYPE_VERTEX;	break;
	case kSimplePosition_locationType_edge:		ids = IDS_LOCATIONTYPE_EDGE;	break;
	case kSimplePosition_locationType_surface:	ids = IDS_LOCATIONTYPE_SURFACE;	break;
	case kSimplePosition_locationType_volume:	ids = IDS_LOCATIONTYPE_VOLUME;	break;
	}
	nameSuffix = GetString(ids);
	return true;
}

bool PFOperatorSimplePosition::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	IPFSystem* pfSys = GetPFSystemInterface(pSystem);
	if (pfSys == NULL) return false; // no handle for PFSystem interface

// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't find particle times in the container

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false; // can't get access to ChannelContainer interface

	IParticleChannelPoint3W* chPos = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELPOSITIONW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELPOSITIONR_INTERFACE,
																			PARTICLECHANNELPOSITIONW_INTERFACE, true);
	if (chPos == NULL) return false; // can't modify Position channel in the container

	BOOL lockOnEmitter = pblock()->GetInt(kSimplePosition_lock);
	BOOL inheritSpeed = pblock()->GetInt(kSimplePosition_inherit);

	IParticleChannelPoint3W* chSpeed = NULL;
	if (lockOnEmitter || inheritSpeed) {
		chSpeed = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
															ParticleChannelPoint3_Class_ID,
															true, PARTICLECHANNELSPEEDR_INTERFACE,
															PARTICLECHANNELSPEEDW_INTERFACE, true );
		if (chSpeed == NULL) return false; // can't modify Speed channel in the container
	}

	IParticleChannelPoint3R* chLocalPosR = NULL;
	IParticleChannelPoint3W* chLocalPosW = NULL;
	if (lockOnEmitter) {
		// acquire LocalPosition particle channel; if not present then create it as a private channel
		chLocalPosR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELLOCALPOSITIONR_INTERFACE,
															ParticleChannelPoint3_Class_ID,
															true, PARTICLECHANNELLOCALPOSITIONR_INTERFACE,
															PARTICLECHANNELLOCALPOSITIONW_INTERFACE, false,
															actionNode, (Object*)this);
		if (chLocalPosR == NULL) return false;
		chLocalPosW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELLOCALPOSITIONW_INTERFACE, (Object*)this);
		if (chLocalPosW == NULL) return false;
	}

	// cache nodeTM at each tick
	BOOL isTMAnimated = FALSE;
	TimeValue timeValueStart = floor(float(timeStart) - 1.0f);
	TimeValue timeValueEnd = ceil(float(timeEnd));
	Tab<Matrix3> cachedTM;
	Matrix3 nodeTM;

	bool wasIgnoringEmitterTMChange = IsIgnoringEmitterTMChange();
	if (!wasIgnoringEmitterTMChange) SetIgnoreEmitterTMChange();
	bool wasIgnoringEmitterPropChange = IsIgnoringEmitterPropChange();
	if (!wasIgnoringEmitterPropChange) SetIgnoreEmitterPropChange();

	bool useSubframe = (pblock()->GetInt(kSimplePosition_subframe, timeValueStart) != 0);
	float time_dt = float(timeValueEnd - timeValueStart);
	for (INode* pn = pNode; !(pn->IsRootNode()); pn = pn->GetParentNode())
		if ((isTMAnimated = pn->GetTMController()->IsAnimated()) == TRUE) break;
	if (isTMAnimated) {
		if (useSubframe) {
			cachedTM.SetCount(timeValueEnd - timeValueStart + 1);
			int i = 0;
			for (TimeValue t = timeValueStart; t <= timeValueEnd; t++, i++)
				cachedTM[i] = pNode->GetObjectTM(t);
		} else {
			cachedTM.SetCount(2);
			cachedTM[0] = pNode->GetObjectTM(timeValueStart);
			cachedTM[1] = pNode->GetObjectTM(timeValueEnd);
		}
	} else {
		nodeTM = pNode->GetObjectTM(timeValueStart);
	}

	// modify position for "new" particles
	int locationType = pblock()->GetInt(kSimplePosition_type);
	BOOL useDistinctPositions = pblock()->GetInt(kSimplePosition_distinctOnly);
	int amount = chAmount->Count();

	Control* paramCtrl = pblock()->GetController(kSimplePosition_inheritAmount);
	bool isInheritAmountAnimated = (paramCtrl == NULL) ? false : (paramCtrl->IsAnimated() == TRUE);
	float inheritAmount = isInheritAmountAnimated ? 1.0f : GetPFFloat(pblock(), kSimplePosition_inheritAmount, timeStart);

	PreciseTimeValue time;
	TimeValue timeValue;
	Point3 localPosition;
	for (int i = 0; i < amount; i++)
	{
		bool isNew = chNew->IsNew(i);
		if (lockOnEmitter || isNew)
		{
			// update position
			time = chTime->GetValue(i);
			timeValue = TimeValue(time);
			if (isTMAnimated) {		// interpolate ObjectTM
				PreciseTimeValue interpTime = time;
				if (!useSubframe)
					interpTime = PreciseTimeValue(timeValueStart) + float(interpTime - PreciseTimeValue(timeValueStart))/time_dt;
				InterpolateObjectTM(interpTime, timeValueStart, cachedTM, nodeTM);
			}
			if (isNew) {			// compute local position
				if (locationType == kSimplePosition_locationType_pivot) {
					localPosition = Point3::Origin;
				} else {
					RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
					int emitterType = pfSys->GetEmitterType(timeValue);
					Tab<float> emitterDimensions;
					pfSys->GetEmitterDimensions(timeValue, emitterDimensions);
					if (useDistinctPositions) {
						Tab<Point3>& distinctPoints = _pointsMap(pCont).DistinctPoints();
						int numDistinctPoints = GetPFInt(pblock(), kSimplePosition_distinctTotal, timeValue);
						numDistinctPoints = AllowedDistinctPoints(locationType, emitterType, numDistinctPoints);
						if (distinctPoints.Count() < numDistinctPoints) {
							RandGenerator& randGen2 = _pointsMap(pCont).RandGenerator();
							GenDistinctPoints(&randGen2, locationType, numDistinctPoints,
												distinctPoints, emitterType);
						}
						localPosition = GetRandomPosition(randGen, numDistinctPoints, distinctPoints,
															emitterType, emitterDimensions);
					} else {
						localPosition = GetRandomPosition(randGen, locationType,
															emitterType, emitterDimensions);
					}
				}
			}
			if (lockOnEmitter) {
				if (isNew) {		// write local position
					chLocalPosW->SetValue(i, localPosition);
				} else {			// read local position
					localPosition = chLocalPosR->GetValue(i);
				}
			}
			chPos->SetValue(i, nodeTM.PointTransform(localPosition));

			// update speed
			if (lockOnEmitter || inheritSpeed) {
				Point3 speedVec(Point3::Origin);
				PreciseTimeValue interval(1);
				if (isTMAnimated) {
					Matrix3 nextTM;
					if (lockOnEmitter) {
						PreciseTimeValue interpTime = timeEnd;
						if (!useSubframe)
							interpTime = PreciseTimeValue(timeValueStart) + float(interpTime - PreciseTimeValue(timeValueStart))/time_dt;
						InterpolateObjectTM(interpTime, timeValueStart, cachedTM, nextTM);
						interval = timeEnd - time;
					} else {
						nextTM = nodeTM;
						PreciseTimeValue interpTime = time - interval;
						if (!useSubframe)
							interpTime = PreciseTimeValue(timeValueStart) + float(interpTime - PreciseTimeValue(timeValueStart))/time_dt;
						InterpolateObjectTM(interpTime, timeValueStart, cachedTM, nodeTM);
					}
					Point3 currPosition = nodeTM.PointTransform(localPosition);
					Point3 nextPosition = nextTM.PointTransform(localPosition);
					if (float(interval) != 0.0f) {
						speedVec = nextPosition - currPosition;
						speedVec /= float(interval);
					}
					if (inheritSpeed && !lockOnEmitter) {
						if (isInheritAmountAnimated) inheritAmount = GetPFFloat(pblock(), kSimplePosition_inheritAmount, timeValue);
						speedVec *= inheritAmount;
					}
				}
				chSpeed->SetValue(i, speedVec);
			}
		}
	}

	if (!wasIgnoringEmitterTMChange) ClearIgnoreEmitterTMChange();
	if (!wasIgnoringEmitterPropChange) ClearIgnoreEmitterPropChange();

	return true;
}

ClassDesc* PFOperatorSimplePosition::GetClassDesc() const
{
	return GetPFOperatorSimplePositionDesc();
}

void PFOperatorSimplePosition::InterpolateObjectTM(PreciseTimeValue time, TimeValue timeValueStart,
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
		tmIndex -= 1;
	}
	if (tmIndex < 0) {
		nodeTM = cachedTM[0];
	} else if (tmIndex >= cachedTM.Count()-1) {
		nodeTM = cachedTM[cachedTM.Count()-1];
	} else if (ratio == 0.0f) {
		nodeTM = cachedTM[tmIndex];
	} else {
		Matrix3 prevObjTM = cachedTM[tmIndex];
		Matrix3 nextObjTM = cachedTM[tmIndex+1];
		prevObjTM *= (1.0f - ratio);
		nextObjTM *= ratio;
		nodeTM = prevObjTM + nextObjTM;
	}
}

int PFOperatorSimplePosition::AllowedDistinctPoints(int locationType, int emitterType, int numDistinctPoints)
{
	if (numDistinctPoints < 1)	return 1;

	switch (locationType)
	{
	case kSimplePosition_locationType_pivot:
		return 1;
	case kSimplePosition_locationType_vertex:
		switch (emitterType)
		{
		case IPFSystem::kEmitterType_rectangle:
			return min(4, numDistinctPoints);
		case IPFSystem::kEmitterType_circle:
			return 1;
		case IPFSystem::kEmitterType_box:
			return min(8, numDistinctPoints);
		case IPFSystem::kEmitterType_sphere:
			return min(6, numDistinctPoints);
		default:
			return 1;
		}
	case kSimplePosition_locationType_edge:
	case kSimplePosition_locationType_surface:
	case kSimplePosition_locationType_volume:
		return numDistinctPoints;
	default:
		return 1;
	}
}

void PFOperatorSimplePosition::GenDistinctPoints(RandGenerator* randGen, int locationType, int numDistinctPoints,
													Tab<Point3>& distinctPoints, int emitterType)
{
	int count = distinctPoints.Count();
	if (count >= numDistinctPoints) return;

	// generate distinct points in cardinal dimensions
	Tab<float> emitterDimensions;
	emitterDimensions.SetCount(3);
	for (int i = 0; i < 3; i++) emitterDimensions[i] = 1.0f;
	distinctPoints.SetCount(numDistinctPoints);
	for (i = count; i < numDistinctPoints; i++)
	{
		Point3 newPoint;
		for (int j = -1; j < i;)
		{
			newPoint = GetRandomPosition(randGen, locationType, emitterType, emitterDimensions);
			if (locationType != kSimplePosition_locationType_vertex) break;
			for (j = 0; j < i; j++)
				if (distinctPoints[j] == newPoint) break;
		}
		distinctPoints[i] = newPoint;
	}
}

Point3 PFOperatorSimplePosition::GetRandomPosition(RandGenerator* randGen, int numDistinctPoints, Tab<Point3>& distinctPoints,
													int emitterType, Tab<float>& emitterDimensions)
{
	if (numDistinctPoints > distinctPoints.Count())
		numDistinctPoints = distinctPoints.Count();
	if (numDistinctPoints <= 0) return Point3::Origin;

	int index = (numDistinctPoints == 1) ? 0 : randGen->Rand0X(numDistinctPoints-1);
	Point3 newPoint = distinctPoints[index];

	// adjust the point by emitter's dimensions
	switch (emitterType)
	{
	case IPFSystem::kEmitterType_rectangle:
		if (emitterDimensions.Count() < 2) break;
		newPoint *= Point3(emitterDimensions[1], emitterDimensions[0], 0.0f);
		return newPoint;
	case IPFSystem::kEmitterType_box:
		if (emitterDimensions.Count() < 3) break;
		newPoint *= Point3(emitterDimensions[1], emitterDimensions[0], emitterDimensions[2]);
		return newPoint;
	case IPFSystem::kEmitterType_circle:
	case IPFSystem::kEmitterType_sphere:
		if (emitterDimensions.Count() < 1) break;
		newPoint *= emitterDimensions[0];
		return newPoint;
	}
	return Point3::Origin;
}

Point3 PFOperatorSimplePosition::GetRandomPosition(RandGenerator* randGen, int locationType,
													int emitterType, Tab<float>& emitterDimensions)
{
	switch (emitterType)
	{
	case IPFSystem::kEmitterType_rectangle:
		if (emitterDimensions.Count() < 2) break;
		return GetRandomPositionInRectangle(randGen, locationType, emitterDimensions[1],
											emitterDimensions[0]);
	case IPFSystem::kEmitterType_circle:
		if (emitterDimensions.Count() < 1) break;
		return GetRandomPositionInCircle(randGen, locationType, 0.5f * emitterDimensions[0]);
	case IPFSystem::kEmitterType_box:
		if (emitterDimensions.Count() < 3) break;
		return GetRandomPositionInBox(randGen, locationType, emitterDimensions[1],
											emitterDimensions[0], emitterDimensions[2]);
	case IPFSystem::kEmitterType_sphere:
		if (emitterDimensions.Count() < 1) break;
		return GetRandomPositionInSphere(randGen, locationType, 0.5f * emitterDimensions[0]);
	}
	return Point3::Origin;
}

Point3 PFOperatorSimplePosition::GetRandomPositionInRectangle(RandGenerator* randGen, int locationType,
															   float xdim, float ydim)
{
	float x, y;

	switch (locationType)
	{
	case kSimplePosition_locationType_vertex:
		// NOTE: explicitly assigned to variables, so that the order of evaluation is fixed.
		x = randGen->RandSign() * 0.5f * xdim;
		y = randGen->RandSign() * 0.5f * ydim;
		return Point3(x, y, 0.0f);
	case kSimplePosition_locationType_edge:
		// distribution is propotional to the length of the edges
		if (randGen->Rand01() * (xdim + ydim) < xdim) {
			x = randGen->Rand55() * xdim;
			y = randGen->RandSign() * 0.5f * ydim;
		} else {
			x = randGen->RandSign() * 0.5f * xdim;
			y = randGen->Rand55() * ydim;
		}
		return Point3(x, y, 0.0f);
	case kSimplePosition_locationType_surface:
	case kSimplePosition_locationType_volume:
		x = randGen->Rand55() * xdim;
		y = randGen->Rand55() * ydim;
		return Point3(x, y, 0.0f);
	}
	return Point3::Origin;
}

Point3 PFOperatorSimplePosition::GetRandomPositionInCircle(RandGenerator* randGen, int locationType,
															float radius)
{
	float theta, q1, q2, x, y;

	switch (locationType)
	{
	case kSimplePosition_locationType_vertex:
		return Point3::Origin;
	case kSimplePosition_locationType_surface:
	case kSimplePosition_locationType_volume:
		q1 = randGen->Rand01();
		q2 = randGen->Rand01();
		// the probability of points fall inside a circle of radius r = r^2 / R^2
		radius *= max(q1, q2);
		// NOTE: continue to the next case to compute a position
	case kSimplePosition_locationType_edge:
		theta = randGen->Rand01() * TWOPI;
		x = cos(theta) * radius;
		y = sin(theta) * radius;
		return Point3(x, y, 0.0f);
	}
	return Point3::Origin;
}

Point3 PFOperatorSimplePosition::GetRandomPositionInBox(RandGenerator* randGen, int locationType,
														 float xdim, float ydim, float zdim)
{
	float xTotal, xyTotal, xyzTotal, d, x, y, z;

	switch (locationType)
	{
	case kSimplePosition_locationType_vertex:
		x = randGen->RandSign() * 0.5f * xdim;
		y = randGen->RandSign() * 0.5f * ydim;
		z = randGen->RandSign() * 0.5f * zdim;
		return Point3(x, y, z);
	case kSimplePosition_locationType_edge:
		xTotal = xdim;						// lines parallel to X axis
		xyTotal = xTotal + ydim;			// + lines parallel to Y axis
		xyzTotal = xyTotal + zdim;			// + lines parallel to Z axis
		d = randGen->Rand01() * xyzTotal;
		x = randGen->Rand55();
		y = randGen->RandSign() * 0.5f;
		z = randGen->RandSign() * 0.5f;
		if (d < xTotal)
			return Point3(x*xdim, y*ydim, z*zdim);
		else if (d < xyTotal)
			return Point3(z*xdim, x*ydim, y*zdim);
		else
			return Point3(y*xdim, z*ydim, x*zdim);
	case kSimplePosition_locationType_surface:
		xTotal = ydim*zdim;					// faces perpendicual to X axis
		xyTotal = xTotal + zdim*xdim;		// + faces perpendicual to Y axis
		xyzTotal = xyTotal + xdim*ydim;		// + faces perpendicual to Z axis
		d = randGen->Rand01() * xyzTotal;
		x = randGen->RandSign() * 0.5f;
		y = randGen->Rand55();
		z = randGen->Rand55();
		if (d < xTotal)
			return Point3(x*xdim, y*ydim, z*zdim);
		else if (d < xyTotal)
			return Point3(z*xdim, x*ydim, y*zdim);
		else
			return Point3(y*xdim, z*ydim, x*zdim);
	case kSimplePosition_locationType_volume:
		x = randGen->Rand55() * xdim;
		y = randGen->Rand55() * ydim;
		z = randGen->Rand55() * zdim;
		return Point3(x, y, z);
	}
	return Point3::Origin;
}

Point3 PFOperatorSimplePosition::GetRandomPositionInSphere(RandGenerator* randGen, int locationType,
															float radius)
{
	float theta, q1, q2, x, y;

	switch (locationType)
	{
	case kSimplePosition_locationType_vertex:
		x = randGen->RandSign() * radius;
		switch (randGen->Rand0X(2)) {
		case 0:  return Point3(x, 0.0f, 0.0f);		// points on X axis
		case 1:  return Point3(0.0f, x, 0.0f);		// points on Y axis
		default: return Point3(0.0f, 0.0f, x);		// points on Z axis
		}
	case kSimplePosition_locationType_edge:
		theta = randGen->Rand01() * TWOPI;
		x = cos(theta) * radius;
		y = sin(theta) * radius;
		switch (randGen->Rand0X(2)) {
		case 0:  return Point3(0.0f, x, y);		// circle perpendicular to X axis
		case 1:  return Point3(y, 0.0f, x);		// circle perpendicular to Y axis
		default: return Point3(x, y, 0.0f);		// circle perpendicular to Z axis
		}
	case kSimplePosition_locationType_volume:
		q1 = randGen->Rand01();
		q2 = randGen->Rand01();
		q1 = max(q1, q2);
		q2 = randGen->Rand01();
		// the probability of points fall inside a sphere of radius r = r^3 / R^3
		radius *= max(q1, q2);
		// NOTE: continue to the next case to compute a position
	case kSimplePosition_locationType_surface:
		return radius*RandSphereSurface(randGen);
	}
	return Point3::Origin;
}

} // end of namespace PFActions