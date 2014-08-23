/**********************************************************************
 *<
	FILE: PFTestSpeed.h

	DESCRIPTION: Speed Test implementation
				 The Test checks particles for their speed/acceleration

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFTestSpeed.h"

#include "PFTestSpeed_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Speed Test creates a particle channel to store a random value for each particle [-1.0f, 1.0f]
// The value is used to calculation test value with variation
#define PARTICLECHANNELRANDFLOATR_INTERFACE Interface_ID(0xf4133dc, 0x1eb34500) 
#define PARTICLECHANNELRANDFLOATW_INTERFACE Interface_ID(0xf4133dc, 0x1eb34501) 
#define GetParticleChannelRandFloatRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELRANDFLOATR_INTERFACE)) 
#define GetParticleChannelRandFloatWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELRANDFLOATW_INTERFACE)) 

// Speed Test creates a particle channel to store the speed at previous time moment
// This way is possible to calculate rate of change in speed value
#define PARTICLECHANNELPREVSPEEDR_INTERFACE Interface_ID(0x3f40042f, 0x1eb34500) 
#define PARTICLECHANNELPREVSPEEDW_INTERFACE Interface_ID(0x3f40042f, 0x1eb34501) 
#define GetParticleChannelPrevSpeedRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELPREVSPEEDR_INTERFACE)) 
#define GetParticleChannelPrevSpeedWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELPREVSPEEDW_INTERFACE)) 

// Speed Test creates a particle channel to store the time the previous speed has been acquired
// This way is possible to calculate rate of change in speed value
#define PARTICLECHANNELPREVTIMER_INTERFACE Interface_ID(0x51f31ac1, 0x1eb34500) 
#define PARTICLECHANNELPREVTIMEW_INTERFACE Interface_ID(0x51f31ac1, 0x1eb34501) 
#define GetParticleChannelPrevTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELPREVTIMER_INTERFACE)) 
#define GetParticleChannelPrevTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELPREVTIMEW_INTERFACE)) 

// constructors/destructors
PFTestSpeed::PFTestSpeed()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestSpeed::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &speedTest_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &speedTest_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &speedTest_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestSpeed::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SPEED_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSpeed::ClassID()
{
	return PFTestSpeed_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpeed::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpeed::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PFTestSpeed::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSpeedTest_testType:
				case kSpeedTest_unitVariation:
				case kSpeedTest_angleVariation:
					RefreshUI(kSpeedTest_message_update);
					// the break is omitted on purpose (bayboro 11-18-02)
				case kSpeedTest_conditionType:
				case kSpeedTest_unitValue:
				case kSpeedTest_angleValue:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kSpeedTest_RefMsg_InitDlg:
			RefreshUI(kSpeedTest_message_update, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSpeedTest_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFTestSpeed::Clone(RemapDir &remap)
{
	PFTestSpeed* newTest = new PFTestSpeed();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestSpeed::GetObjectName()
{
	return GetString(IDS_TEST_SPEED_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestSpeed::GetRand()
{
	return pblock()->GetInt(kSpeedTest_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestSpeed::SetRand(int seed)
{
	pblock()->SetValue(kSpeedTest_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFTestSpeed::ChannelsUsed(const Interval& time) const
{
								//  read													&	write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_Speed, PCU_EventStart);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELRANDFLOATR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELRANDFLOATW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELPREVSPEEDR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELPREVSPEEDW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELPREVTIMER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELPREVTIMEW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpeed::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDTEST_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpeed::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDTEST_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpeed::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPEEDTEST_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestSpeed::HasDynamicName(TSTR& nameSuffix)
{
	int testType	= pblock()->GetInt(kSpeedTest_testType, 0);
	switch(testType) {
	case kSpeedTest_testType_speed:
		nameSuffix = GetString(IDS_SPEED);
		break;
	case kSpeedTest_testType_speedX:
		nameSuffix = GetString(IDS_SPEEDX);
		break;
	case kSpeedTest_testType_speedY:
		nameSuffix = GetString(IDS_SPEEDY);
		break;
	case kSpeedTest_testType_speedZ:
		nameSuffix = GetString(IDS_SPEEDZ);
		break;
	case kSpeedTest_testType_accel:
		nameSuffix = GetString(IDS_ACCELERATION_ABBR);
		break;
	case kSpeedTest_testType_accelX:
		nameSuffix = GetString(IDS_ACCELERATIONX_ABBR);
		break;
	case kSpeedTest_testType_accelY:
		nameSuffix = GetString(IDS_ACCELERATIONY_ABBR);
		break;
	case kSpeedTest_testType_accelZ:
		nameSuffix = GetString(IDS_ACCELERATIONZ_ABBR);
		break;
	case kSpeedTest_testType_steering:
		nameSuffix = GetString(IDS_STEERING);
		break;
	case kSpeedTest_testType_whenAccels:
		nameSuffix = GetString(IDS_IFACCELERATES_ABBR);
		break;
	case kSpeedTest_testType_whenSlows:
		nameSuffix = GetString(IDS_IFDECELERATES_ABBR);
		break;
	}
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSpeed::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	bool exactStep = IsExactIntegrationStep(timeEnd, pSystem);

	// get the constant properties of the test
	int testType = pblock()->GetInt(kSpeedTest_testType, timeEnd);
	int condType = pblock()->GetInt(kSpeedTest_conditionType, timeEnd);
	int syncType = pblock()->GetInt(kSpeedTest_sync, timeEnd);
	int varParamID = (testType == kSpeedTest_testType_steering) ? kSpeedTest_angleVariation : kSpeedTest_unitVariation;
	bool hasTestVariation = (pblock()->GetFloat(varParamID, 0) != 0.0f);
	if (!hasTestVariation) {
		Control* ctrl = pblock()->GetController(varParamID);
		if (ctrl != NULL)
			hasTestVariation = (ctrl->IsAnimated() != 0);
	}
	if (testType >= kSpeedTest_testType_whenAccels) {
		hasTestVariation = false;
		syncType = kSpeedTest_sync_time;
	}
	bool needPrevValue = (testType >= kSpeedTest_testType_accel);

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count == 0) return true; // no particles to test
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing info for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
	if (chSpeed == NULL) return false; // can't read speed values

	// acquire more particle channels
	IParticleChannelPTVR* chBirthTime = NULL;
	if (syncType == kSpeedTest_sync_age && (testType < kSpeedTest_testType_whenAccels))
	{
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirthTime == NULL) return false; // can't read particle age
	}
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (syncType == kSpeedTest_sync_event && (testType < kSpeedTest_testType_whenAccels)) {
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
	IParticleChannelFloatR* chRandFloatR = NULL;
	IParticleChannelFloatW* chRandFloatW = NULL;
	bool initRandFloat = false;
	if (hasTestVariation) {
		chRandFloatW = (IParticleChannelFloatW*)chCont->EnsureInterface(PARTICLECHANNELRANDFLOATW_INTERFACE,
																		ParticleChannelFloat_Class_ID,
																		true, PARTICLECHANNELRANDFLOATR_INTERFACE,
																		PARTICLECHANNELRANDFLOATW_INTERFACE, true,
																		actionNode, (Object*)this, &initRandFloat);
		chRandFloatR = (IParticleChannelFloatR*)chCont->GetPrivateInterface(PARTICLECHANNELRANDFLOATR_INTERFACE, (Object*)this);
		if ((chRandFloatR == NULL) || (chRandFloatW == NULL)) return false; // can't set rand float value for newly entered particles
	}
	IParticleChannelPoint3R* chPrevSpeedR = NULL;
	IParticleChannelPoint3W* chPrevSpeedW = NULL;
	IParticleChannelPTVR* chPrevTimeR = NULL;
	IParticleChannelPTVW* chPrevTimeW = NULL;
	bool initPrevValue = false;
	if (needPrevValue) {
		chPrevSpeedW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELPREVSPEEDW_INTERFACE,
																		ParticleChannelPoint3_Class_ID,
																		true, PARTICLECHANNELPREVSPEEDR_INTERFACE,
																		PARTICLECHANNELPREVSPEEDW_INTERFACE, true,
																		actionNode, (Object*)this, &initPrevValue);
		chPrevSpeedR = (IParticleChannelPoint3R*)chCont->GetPrivateInterface(PARTICLECHANNELPREVSPEEDR_INTERFACE, (Object*)this);
		chPrevTimeW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELPREVTIMEW_INTERFACE,
																		ParticleChannelPTV_Class_ID,
																		true, PARTICLECHANNELPREVTIMER_INTERFACE,
																		PARTICLECHANNELPREVTIMEW_INTERFACE, true,
																		actionNode, (Object*)this, &initPrevValue);
		chPrevTimeR = (IParticleChannelPTVR*)chCont->GetPrivateInterface(PARTICLECHANNELPREVTIMER_INTERFACE, (Object*)this);
		if ((chPrevSpeedR == NULL) || (chPrevSpeedW == NULL) || (chPrevTimeR == NULL) || (chPrevTimeW == NULL)) return false; 
	}

	// grab the rand generator for test variation
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	if (randGen == NULL) return false;

	// check all particles
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	for(int i=0; i<count; i++)
	{
		if (chNew->IsNew(i)) { // initialize some channels
			if (initEventStart)
				chEventStartW->SetValue(i, chTime->GetValue(i));
			if (initRandFloat)
				chRandFloatW->SetValue(i, randGen->Rand11());
		}

		PreciseTimeValue prevTime;
		Point3 prevSpeed;
		Point3 currentSpeed = chSpeed->GetValue(i);
		PreciseTimeValue currentTime = chTime->GetValue(i);
		if (needPrevValue) {
			prevTime = chPrevTimeR->GetValue(i);
			prevSpeed = chPrevSpeedR->GetValue(i);
			chPrevTimeW->SetValue(i, currentTime);
			chPrevSpeedW->SetValue(i, currentSpeed);
			if (initPrevValue && chNew->IsNew(i))
				continue; // particle just came into the event and doesn't have previous value
		}

		PreciseTimeValue syncTime = currentTime;
		switch(syncType) {
		case kSpeedTest_sync_age:
			syncTime -= chBirthTime->GetValue(i);
			break;
		case kSpeedTest_sync_event:
			syncTime -= chEventStartR->GetValue(i);
			break;
		}
		TimeValue syncTimeTV = TimeValue(syncTime);

		float testValue = 0.0f;
		if (testType < kSpeedTest_testType_whenAccels) {
			if (testType == kSpeedTest_testType_steering) {
				testValue = GetPFFloat(pblock(), kSpeedTest_angleValue, syncTimeTV);
				if (hasTestVariation)
					testValue += chRandFloatR->GetValue(i)*GetPFFloat(pblock(), kSpeedTest_angleVariation, syncTimeTV);
			} else {
				testValue = GetPFFloat(pblock(), kSpeedTest_unitValue, syncTimeTV);
				if (hasTestVariation)
					testValue += chRandFloatR->GetValue(i)*GetPFFloat(pblock(), kSpeedTest_unitVariation, syncTimeTV);
			}
			testValue /= TIME_TICKSPERSEC;
		}

		float currentValue = 0.0f;
		bool testSatisfied = false;
		if (testType < kSpeedTest_testType_whenAccels) {
			if (testType < kSpeedTest_testType_accel) {
				switch(testType) {
				case kSpeedTest_testType_speed:
					currentValue = Length(currentSpeed);
					break;
				case kSpeedTest_testType_speedX:
					currentValue = currentSpeed.x;
					break;
				case kSpeedTest_testType_speedY:
					currentValue = currentSpeed.y;
					break;
				case kSpeedTest_testType_speedZ:
					currentValue = currentSpeed.z;
					break;
				}
			} else if (testType == kSpeedTest_testType_steering) {
				float timeDif = float(currentTime - prevTime);
				if (timeDif <= 0.0f) continue; // no time difference
				float normFactor = Length(currentSpeed)*Length(prevSpeed);
				if (normFactor <= 0.0f) continue; // steering rate is not calculatable
				float vv = DotProd(currentSpeed,prevSpeed)/normFactor;
				float uu = 0.0;
				if (vv >= 1.0f) uu = 0.0;
				else if (vv <= -1.0f) uu = PI;
				else uu = acos(vv);
				currentValue = uu/timeDif;
			} else { // acceleration
				float timeDif = float(currentTime - prevTime);
				if (timeDif <= 0.0f) continue; // no time difference
				Point3 curAccel;
				switch(testType) {
				case kSpeedTest_testType_accel:
					currentValue = Length((currentSpeed - prevSpeed)/timeDif);
					break;
				case kSpeedTest_testType_accelX:
					currentValue = (currentSpeed.x - prevSpeed.x)/timeDif;	
					break;
				case kSpeedTest_testType_accelY:
					currentValue = (currentSpeed.y - prevSpeed.y)/timeDif;	
					break;
				case kSpeedTest_testType_accelZ:
					currentValue = (currentSpeed.z - prevSpeed.z)/timeDif;	
					break;
				}
				testValue /= TIME_TICKSPERSEC; // acceleration is per second squared
			}
			testSatisfied = (condType == kSpeedTest_conditionType_less) ?
								(currentValue < testValue) : (currentValue > testValue);
		} else {
			if (testType == kSpeedTest_testType_whenAccels) {
				testSatisfied = (Length(currentSpeed) > Length(prevSpeed));
			} else {
				testSatisfied = (Length(currentSpeed) < Length(prevSpeed));
			}
		}

		if (testSatisfied && exactStep) {
			testResult.Set(i);
			testTime[i] = 0.0f;
		}
	}

	return true;
}

ClassDesc* PFTestSpeed::GetClassDesc() const
{
	return GetPFTestSpeedDesc();
}



} // end of namespace PFActions