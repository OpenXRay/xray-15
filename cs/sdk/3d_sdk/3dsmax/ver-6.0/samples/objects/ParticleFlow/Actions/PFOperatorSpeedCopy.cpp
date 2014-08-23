/**********************************************************************
 *<
	FILE:			PFOperatorSpeedCopy.cpp

	DESCRIPTION:	SpeedCopy Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_Icon.h"

#include "PFOperatorSpeedCopy.h"

#include "PFOperatorSpeedCopy_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

// SpeedCopy creates a particle channel to store a random float for each particle
// when using speed variation
// This way the random variation value is calculated only once when a particle enters the event.
#define PARTICLECHANNELRNDFLOATR_INTERFACE Interface_ID(0x38410659, 0x1eb34500) 
#define PARTICLECHANNELRNDFLOATW_INTERFACE Interface_ID(0x38410659, 0x1eb34501) 

#define GetParticleChannelRndFloatRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELRNDFLOATR_INTERFACE)) 
#define GetParticleChannelRndFloatWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELRNDFLOATW_INTERFACE)) 


// SpeedCopy creates a particle channel to store sync time for each particle
// The time is used to get the speed of the icon at a specific sync time
// The channel is created if Control By Time
#define PARTICLECHANNELSYNCTIMER_INTERFACE Interface_ID(0x677f0399, 0x1eb34500) 
#define PARTICLECHANNELSYNCTIMEW_INTERFACE Interface_ID(0x677f0399, 0x1eb34501) 
#define GetParticleChannelSyncTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELSYNCTIMER_INTERFACE)) 
#define GetParticleChannelSyncTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELSYNCTIMEW_INTERFACE)) 

PFOperatorSpeedCopy::PFOperatorSpeedCopy()
{ 
	_iconShape() = NULL; // lazy initialized in ::Display(...)
	_iconMesh() = NULL; // lazy initialized in ::Display(...)
	_validIcon() = NEVER;
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSpeedCopy::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &speedCopy_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &speedCopy_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &speedCopy_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSpeedCopy::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SPEEDCOPY_CLASS_NAME);
}

Class_ID PFOperatorSpeedCopy::ClassID()
{
	return PFOperatorSpeedCopy_Class_ID;
}

void PFOperatorSpeedCopy::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSpeedCopy::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSpeedCopy::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	static int colorRequest = -1;
	float speedMin, speedMax;
	TimeValue currentTime;

	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSpeedCopy_syncByParams:
				case kSpeedCopy_syncByIcon:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kSpeedCopy_useSpeedVariation:
				case kSpeedCopy_steer:
					RefreshUI(kSpeedCopy_message_update);
					break;
				case kSpeedCopy_iconSize:
					InvalidateIcon();
					break;
				case kSpeedCopy_colorType:
					NotifyDependents( FOREVER, (PartID)colorRequest, kPFMSG_UpdateWireColor );
					break;
				case kSpeedCopy_speedMin:
					currentTime = GetCOREInterface()->GetTime();
					speedMin = pblock()->GetFloat(kSpeedCopy_speedMin, currentTime);
					speedMax = pblock()->GetFloat(kSpeedCopy_speedMax, currentTime);
					if (speedMin > speedMax)
						pblock()->SetValue(kSpeedCopy_speedMax, currentTime, speedMin);
					break;
				case kSpeedCopy_speedMax:
					currentTime = GetCOREInterface()->GetTime();
					speedMin = pblock()->GetFloat(kSpeedCopy_speedMin, currentTime);
					speedMax = pblock()->GetFloat(kSpeedCopy_speedMax, currentTime);
					if (speedMin > speedMax)
						pblock()->SetValue(kSpeedCopy_speedMin, currentTime, speedMax);
					break;
				} 
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kSpeedCopy_RefMsg_InitDlg:
			RefreshUI(kSpeedCopy_message_update, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSpeedCopy_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSpeedCopy::Clone(RemapDir &remap)
{
	PFOperatorSpeedCopy* newOp = new PFOperatorSpeedCopy();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
static PFActionCreateCallback speedCopyCallback;

CreateMouseCallBack* PFOperatorSpeedCopy::GetCreateMouseCallBack()
{
	speedCopyCallback.init(this, pblock(), kSpeedCopy_iconSize);
	return &speedCopyCallback;
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
int PFOperatorSpeedCopy::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
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
int PFOperatorSpeedCopy::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
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
void PFOperatorSpeedCopy::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{
	SysUtil::NeedToImplementLater();
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedCopy::GetWorldBoundBox(TimeValue t, INode * inode, ViewExp* vp, Box3& box )
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
void PFOperatorSpeedCopy::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp, Box3& box )
{
	GetIconBoundBox(t, box);	
}

//+--------------------------------------------------------------------------+
//|							From BaseObject									 |
//+--------------------------------------------------------------------------+
TCHAR* PFOperatorSpeedCopy::GetObjectName()
{
	return GetString(IDS_OPERATOR_SPEEDCOPY_OBJECT_NAME);
}

bool PFOperatorSpeedCopy::Release(IObject* pCont)
{
	PFObjectTMCacher* cacher = _tmCacher(pCont);
	if (cacher != NULL) delete cacher;
	_tmCacher(pCont) = NULL;
	return PFSimpleAction::Release(pCont);
}

const ParticleChannelMask& PFOperatorSpeedCopy::ChannelsUsed(const Interval& time) const
{
								//  read						 &	write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Position|PCU_Amount, PCU_Speed);
	return mask;
}

int PFOperatorSpeedCopy::IsColorCoordinated() const
{
	return pblock()->GetInt(kSpeedCopy_colorType, 0);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSpeedCopy::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDCOPY_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSpeedCopy::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDCOPY_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSpeedCopy::HasDynamicName(TSTR& nameSuffix)
{
	int paramSync = pblock()->GetInt(kSpeedCopy_syncByParams, 0);
	int iconSync = pblock()->GetInt(kSpeedCopy_syncByIcon, 0);
	switch(paramSync) {
	case kSpeedCopy_syncBy_time:
		nameSuffix = GetString(IDS_TIME);
		break;
	case kSpeedCopy_syncBy_age:
		nameSuffix = GetString(IDS_AGE);
		break;
	case kSpeedCopy_syncBy_event:
		nameSuffix = GetString(IDS_EVENT);
		break;
	}
	if (paramSync != iconSync) {
		nameSuffix += TSTR("/");
		switch(iconSync) {
		case kSpeedCopy_syncBy_time:
			nameSuffix += GetString(IDS_TIME);
			break;
		case kSpeedCopy_syncBy_age:
			nameSuffix += GetString(IDS_AGE);
			break;
		case kSpeedCopy_syncBy_event:
			nameSuffix += GetString(IDS_EVENT);
			break;
		}	
	}
	return true;
}

Matrix3 GetObjectTM(PreciseTimeValue time, INode* node)
{
	int tick1 = time.tick;
	float fraction = time.fraction;
	if (fraction < 0.0f) {
		tick1 -= 1;
		fraction += 1.0f;
	}
	int tick2 = tick1 + 1;
	Matrix3 tm1 = node->GetObjTMAfterWSM(tick1);
	Matrix3 tm2 = node->GetObjTMAfterWSM(tick2);
	float prefraction = 1.0f - fraction;
	Matrix3 tm;
	for(int i=0; i<4; i++)
		tm.SetRow(i, prefraction*tm1.GetRow(i) + fraction*tm2.GetRow(i));
	return tm;
}

Point3 GetObjectPosition(PreciseTimeValue time, INode* node)
{
	int tick1 = time.tick;
	float fraction = time.fraction;
	if (fraction < 0.0f) {
		tick1 -= 1;
		fraction += 1.0f;
	}
	int tick2 = tick1 + 1;
	Matrix3 tm1 = node->GetObjTMAfterWSM(tick1);
	Matrix3 tm2 = node->GetObjTMAfterWSM(tick2);
	Point3 pos = (1.0f-fraction)*tm1.GetTrans() + fraction*tm2.GetTrans();
	return pos;
}

Matrix3 GetObjectTM(PreciseTimeValue time, PFObjectTMCacher* cacher)
{
	int tick1 = time.tick;
	float fraction = time.fraction;
	if (fraction < 0.0f) {
		tick1 -= 1;
		fraction += 1.0f;
	}
	int tick2 = tick1 + 1;
	Matrix3* tm1 = cacher->GetObjectTM(tick1);
	Matrix3 tm;
	if (fabs(fraction) < 0.000001f) { tm = (*tm1); }
	else {
		Matrix3* tm2 = cacher->GetObjectTM(tick2);
		float prefraction = 1.0f - fraction;
		for(int i=0; i<4; i++)
			tm.SetRow(i, prefraction*tm1->GetRow(i) + fraction*tm2->GetRow(i));
	}
	return tm;
}

Point3 GetObjectPosition(PreciseTimeValue time, PFObjectTMCacher* cacher)
{
	int tick1 = time.tick;
	float fraction = time.fraction;
	if (fraction < 0.0f) {
		tick1 -= 1;
		fraction += 1.0f;
	}
	int tick2 = tick1 + 1;
	Matrix3* tm1 = cacher->GetObjectTM(tick1);
	Point3 pos;
	if (fabs(fraction) < 0.000001f) { pos = tm1->GetTrans(); }
	else {
		Matrix3* tm2 = cacher->GetObjectTM(tick2);
		pos = (1.0f-fraction)*tm1->GetTrans() + fraction*tm2->GetTrans();
	}
	return pos;
}

bool PFOperatorSpeedCopy::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (timeStart >= timeEnd) return true; // can't modify speed channel if zero time interval

	// acquire all necessary channels, create additional if needed
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if(chNew == NULL) return false;
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	if(chPos == NULL) return false;
	//IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
	//if(chSpeed == NULL) return false;
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;
	int count = chAmount->Count();
	if (count <= 0) return true;

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

	float accelLimit = GetPFFloat(pblock(), kSpeedCopy_accelLimit, timeStart)/TIME_TICKSPERSEC;
	float influence = GetPFFloat(pblock(), kSpeedCopy_influence, timeStart);
	if (influence <= 0.0f) return true;
	int useSpeedVar = pblock()->GetInt(kSpeedCopy_useSpeedVariation, timeStart);
	float speedMin = GetPFFloat(pblock(), kSpeedCopy_speedMin, timeStart);
	float speedMax = GetPFFloat(pblock(), kSpeedCopy_speedMax, timeStart);
	int useOrient = pblock()->GetInt(kSpeedCopy_useOrient, timeStart);
	int useSteer = pblock()->GetInt(kSpeedCopy_steer, timeStart);
	float distance = GetPFFloat(pblock(), kSpeedCopy_distance, timeStart);
	int syncByParam = pblock()->GetInt(kSpeedCopy_syncByParams, timeStart);
	int syncByIcon = pblock()->GetInt(kSpeedCopy_syncByIcon, timeStart);
	bool useBirthTime = ((syncByIcon == kSpeedCopy_syncBy_age) || (syncByParam == kSpeedCopy_syncBy_age));
	bool useEventStart = ((syncByIcon == kSpeedCopy_syncBy_event) || (syncByParam == kSpeedCopy_syncBy_event)); 

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	if (randGen == NULL) return false;

	bool initRndFloat = false;
	IParticleChannelFloatR* chRndFloatR = NULL;
	IParticleChannelFloatW* chRndFloatW = NULL;
	if (useSpeedVar) {
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

	IParticleChannelPTVR* chBirthTime = NULL;
	if (useBirthTime) {
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if(chBirthTime == NULL) return false;
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
	
	IParticleChannelPTVR* chSyncTimeR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELSYNCTIMER_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELSYNCTIMER_INTERFACE,
																	PARTICLECHANNELSYNCTIMEW_INTERFACE, false,
																	actionNode, (Object*)this);
	IParticleChannelPTVW* chSyncTimeW = (IParticleChannelPTVW*)chCont->GetPrivateInterface(PARTICLECHANNELSYNCTIMEW_INTERFACE, (Object*)this);
	if ((chSyncTimeR == NULL) || (chSyncTimeW == NULL)) return false; // can't read/modify sync time channel

	PFObjectTMCacher* cacher = _tmCacher(pCont);
	if (cacher == NULL) {
		cacher = new PFObjectTMCacher(actionNode);
		_tmCacher(pCont) = cacher;
	}
	if (cacher == NULL) return false; // not able to construct cacher

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	int tpf = GetTicksPerFrame();
	for(int i=0; i<count; i++) {
		PreciseTimeValue syncTimeStart, syncTimeFinish;
		PreciseTimeValue time = chTime->GetValue(i);
		bool isNew = chNew->IsNew(i);
		if (isNew) {
			if (initEventStart)
				chEventStartW->SetValue(i, time);
			if (initRndFloat) 
				chRndFloatW->SetValue(i, randGen->Rand01());
			else
				randGen->Rand01(); // to keep random gen balance
			switch(syncByIcon) {
			case kSpeedCopy_syncBy_time:
				syncTimeStart = timeStart;
				break;
			case kSpeedCopy_syncBy_age:
				syncTimeStart = time - chBirthTime->GetValue(i);
				break;
			case kSpeedCopy_syncBy_event:
				syncTimeStart = time - chEventStartR->GetValue(i);
				break;
			}
			chSyncTimeW->SetValue(i, syncTimeStart);			
		}

		if (syncByParam != kSpeedCopy_syncBy_time) {
			PreciseTimeValue syncTime = time;
			switch(syncByParam) {
			case kSpeedCopy_syncBy_age:
				syncTime = time - chBirthTime->GetValue(i);
				break;
			case kSpeedCopy_syncBy_event:
				syncTime = time - chEventStartR->GetValue(i);
				break;
			}
			accelLimit = GetPFFloat(pblock(), kSpeedCopy_accelLimit, syncTime)/TIME_TICKSPERSEC;
			influence = GetPFFloat(pblock(), kSpeedCopy_influence, syncTime);
			if (useSpeedVar) {
				speedMin = GetPFFloat(pblock(), kSpeedCopy_speedMin, syncTime);
				speedMax = GetPFFloat(pblock(), kSpeedCopy_speedMax, syncTime);
			}
			if (useSteer)
				distance = GetPFFloat(pblock(), kSpeedCopy_distance, syncTime);
		}
		
		syncTimeStart = chSyncTimeR->GetValue(i);
		if (useSpeedVar) {
			float randomFactor = chRndFloatR->GetValue(i);
			float speedFactor = speedMin + randomFactor*(speedMax - speedMin);
			syncTimeFinish = syncTimeStart + speedFactor*float(timeEnd - time);
		} else {
			syncTimeFinish = syncTimeStart + (timeEnd - time);
		}

		chSyncTimeW->SetValue(i, syncTimeFinish);
		if (syncTimeFinish <= syncTimeStart) continue; // lacking time to do anything

		Point3 currentPos = chPos->GetValue(i);
		Point3 nextIconPos, startPos, finishPos;

		if (useOrient) {
//			Matrix3 tmStart = GetObjectTM(syncTimeStart, actionNode);
//			Matrix3 tmFinish = GetObjectTM(syncTimeFinish, actionNode);
			Matrix3 tmStart = GetObjectTM(syncTimeStart, cacher);
			Matrix3 tmFinish = GetObjectTM(syncTimeFinish, cacher);
			startPos = currentPos;
			Point3 locPos = currentPos*Inverse(tmStart);
			finishPos = locPos*tmFinish;
			if (useSteer) nextIconPos = tmFinish.GetTrans();
		} else {
//			startPos = GetObjectPosition(syncTimeStart, actionNode);
//			finishPos = GetObjectPosition(syncTimeFinish, actionNode);
			startPos = GetObjectPosition(syncTimeStart, cacher);
			finishPos = GetObjectPosition(syncTimeFinish, cacher);
			if (useSteer) nextIconPos = finishPos;
		}

		Point3 newSpeed = (finishPos - startPos)/float(timeEnd - time);
		if (useSteer) {
			Point3 nextPos = currentPos + finishPos - startPos;
			float proximity = Length(nextPos - nextIconPos);
			if (proximity > distance) {
				Point3 altSpeed = Length(newSpeed)*Normalize(nextIconPos - currentPos);
				if (proximity > 2.0f*distance) {
					newSpeed = altSpeed;
				} else {
					float frac = (proximity - distance)/distance;
					newSpeed = (1.0f-frac)*newSpeed + frac*altSpeed;
				}
			}
		}

		Point3 currentSpeed = chSpeedR->GetValue(i);
		float timeAdj = float(timeEnd - time)/tpf;

		if (influence < 1.0f) {
			float adjustedInfluence = influence*timeAdj;
			newSpeed = (1.0f-adjustedInfluence)*currentSpeed + adjustedInfluence*newSpeed;
		}

		Point3 accel = newSpeed - currentSpeed;
		float currentAccelLimit = accelLimit*timeAdj;
		if (Length(accel) > currentAccelLimit) {
			accel = currentAccelLimit*Normalize(accel);
			newSpeed = currentSpeed + accel;
		}
		
		chSpeedW->SetValue(i, newSpeed);
	}

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

ClassDesc* PFOperatorSpeedCopy::GetClassDesc() const
{
	return GetPFOperatorSpeedCopyDesc();
}

int PFOperatorSpeedCopy::GetRand()
{
	return pblock()->GetInt(kSpeedCopy_seed);
}

void PFOperatorSpeedCopy::SetRand(int seed)
{
	_pblock()->SetValue(kSpeedCopy_seed, 0, seed);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFOperatorSpeedCopy						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void BuildLogoSpeedCopy( PolyShape* shape)
{
	enum {	logo_lines = 9,
			num_points = 28 
	};
	static int lineNum[logo_lines] = { 2, 5, 7, 10, 12, 15, 17, 20, num_points };

	static Point3 LogoContur[ num_points ] = {

	Point3(-0.35f,0.15f,0.0f), Point3(-0.1f,0.4f,0.0f), 

	Point3(-0.3f,0.25f,0.0f), Point3(-0.35f,0.15f,0.0f), Point3(-0.25f,0.2f,0.0f), 

	Point3(-0.25f,0.05f,0.0f), Point3(0.0f,0.3f,0.0f), 

	Point3(-0.2f,0.15f,0.0f), Point3(-0.25f,0.05f,0.0f), Point3(-0.15f,0.1f,0.0f), 

	Point3(0.15f,-0.35f,0.0f), Point3(0.4f,-0.1f,0.0f), 

	Point3(0.25f,-0.3f,0.0f), Point3(0.15f,-0.35f,0.0f), Point3(0.2f,-0.25f,0.0f), 

	Point3(0.05f,-0.25f,0.0f), Point3(0.3f,0.0f,0.0f), 

	Point3(0.15f,-0.2f,0.0f), Point3(0.05f,-0.25f,0.0f), Point3(0.1f,-0.15f,0.0f), 

	Point3(-0.4f,-0.4f,0.0f), Point3(-0.3f,-0.15f,0.0f), Point3(-0.25f,-0.2f,0.0f), Point3(0.35f, 0.4f,0.0f), 
	Point3(0.4f, 0.35f,0.0f), Point3(-0.2f,-0.25f,0.0f), Point3(-0.15f,-0.3f,0.0f), Point3(-0.4f,-0.4f,0.0f),

	};

	shape->NewShape();
	int pointIndex = 0;
	for (register lineIndex = 0; lineIndex < logo_lines; lineIndex++ )
	{
		PolyLine* line = shape->NewLine();
		if ( line == NULL ) return;
		while ( pointIndex < lineNum[ lineIndex ] )
			line->Append( PolyPt( LogoContur[ pointIndex++], POLYPT_KNOT ) );
		line->Open();
	}
}

PolyShape* PFOperatorSpeedCopy::GetIconShape(TimeValue time)
// lazy initialization for logo shape
{
	UpdateIcon(time);
	return iconShape();
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorSpeedCopy						 |
//+--------------------------------------------------------------------------+
Mesh* PFOperatorSpeedCopy::GetIconMesh(TimeValue time)
// lazy initialization for emitter mesh
{
	UpdateIcon(time);
	return iconMesh();
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorSpeedCopy						 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedCopy::UpdateIcon(TimeValue time)
// update logo size
{
	if (_validIcon().InInterval(time)) return;

	if (iconShape() == NULL) _iconShape() = new PolyShape();
	if (_iconMesh() == NULL) _iconMesh() = new Mesh();

	float size;
	_validIcon() = FOREVER;
	pblock()->GetValue(kSpeedCopy_iconSize, time, size, _validIcon() );

	int i, j;
	BuildLogoSpeedCopy(_iconShape());
	for(i=0; i<iconShape()->numLines; i++)
		for(j=0; j<iconShape()->lines[i].numPts; j++)
			iconShape()->lines[i].pts[j].p *= size;
	iconShape()->BuildBoundingBox();
	
	PFActionIcon::BuildRectangle(_iconMesh());
	for(i=0; i<iconMesh()->getNumVerts(); i++)
		iconMesh()->verts[i] *= size;
	iconMesh()->buildBoundingBox();
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorSpeedCopy						 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedCopy::InvalidateIcon()
{
	_validIcon() = NEVER;
}

//+--------------------------------------------------------------------------+
//|							From PFOperatorSpeedCopy						 |
//+--------------------------------------------------------------------------+
void PFOperatorSpeedCopy::GetIconBoundBox(TimeValue t, Box3& box ) 
{
	box += GetIconShape(t)->GetBoundingBox();
	box += GetIconMesh(t)->getBoundingBox();
}

//+--------------------------------------------------------------------------+
//|							From PFObjectTMCacher							 |
//+--------------------------------------------------------------------------+

PF_TMBlock::PF_TMBlock() { assert(0); }
PF_TMBlock::PF_TMBlock(INode* node, TimeValue timeOffset)
{
	m_node = node;
	m_timeOffset = timeOffset;
	for(int i=0; i<256; i++) m_acquired[i] = 0;
}
Matrix3* PF_TMBlock::GetObjectTM(TimeValue time)
{
	int index = int(time - m_timeOffset);
	if (m_acquired[index]== 0) {
		m_acquired[index] = 1;
		m_tm[index] = m_node->GetObjTMAfterWSM(time);
	}
	return &(m_tm[index]);
}

PF_TMBlock2::PF_TMBlock2() { assert(0); }
PF_TMBlock2::PF_TMBlock2(INode* node, TimeValue timeOffset)
{
	m_node = node;
	m_timeOffset = timeOffset;
	for(int i=0; i<256; i++) m_tmBlock[i] = NULL;
}
PF_TMBlock2::~PF_TMBlock2()
{
	for(int i=0; i<256; i++)
		if (m_tmBlock[i] != NULL) delete m_tmBlock[i];
}
Matrix3* PF_TMBlock2::GetObjectTM(TimeValue time)
{
	int index = (int(time - m_timeOffset))>>8;
	if (m_tmBlock[index] == NULL) 
		m_tmBlock[index] = new PF_TMBlock(m_node, m_timeOffset + (index<<8));
	return m_tmBlock[index]->GetObjectTM(time);
}

PF_TMBlock3::PF_TMBlock3() { assert(0); }
PF_TMBlock3::PF_TMBlock3(INode* node, TimeValue timeOffset)
{
	m_node = node;
	m_timeOffset = timeOffset;
	for(int i=0; i<256; i++) m_tmBlock2[i] = NULL;
}
PF_TMBlock3::~PF_TMBlock3()
{
	for(int i=0; i<256; i++)
		if (m_tmBlock2[i] != NULL) delete m_tmBlock2[i];
}
Matrix3* PF_TMBlock3::GetObjectTM(TimeValue time)
{
	int index = (int(time - m_timeOffset))>>16;
	if (m_tmBlock2[index] == NULL) 
		m_tmBlock2[index] = new PF_TMBlock2(m_node, m_timeOffset + (index<<16));
	return m_tmBlock2[index]->GetObjectTM(time);
}

PFObjectTMCacher::PFObjectTMCacher() { assert(0); }
PFObjectTMCacher::PFObjectTMCacher(INode* node)
{
	m_node = node;
	for(int i=0; i<256; i++) m_tmBlock3[i] = NULL;
}
PFObjectTMCacher::~PFObjectTMCacher() { Release(); }
Matrix3* PFObjectTMCacher::GetObjectTM(TimeValue time)
{
	int index = UINT(time)>>24;
	if (m_tmBlock3[index] == NULL)
		m_tmBlock3[index] = new PF_TMBlock3(m_node, (index > 127) ? (index<<24) : ((256-index)<<24));
	return m_tmBlock3[index]->GetObjectTM(time);
}
void PFObjectTMCacher::Release()
{
	for(int i=0; i<256; i++) {
		if (m_tmBlock3[i] != NULL) delete m_tmBlock3[i];
		m_tmBlock3[i] = NULL;
	}
}

} // end of namespace PFActions