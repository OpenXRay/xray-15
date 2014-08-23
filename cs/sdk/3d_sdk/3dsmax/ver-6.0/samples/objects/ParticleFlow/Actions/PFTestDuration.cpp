/**********************************************************************
 *<
	FILE: PFTestDuration.h

	DESCRIPTION: Duration Test implementation
				 The Test checks particles for either absolute time
				 or particle age or time period particle is present
				 in the current Event

	CREATED BY: Oleg Bayborodin

	HISTORY: created 12-04-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFTestDuration.h"

#include "PFTestDuration_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Duration Test creates a particle channel to store a test value for each particle
// This way the value is calculated only once when a particle enters the event
#define PARTICLECHANNELTESTDURATIONR_INTERFACE Interface_ID(0x61324c5c, 0x1eb34500) 
#define PARTICLECHANNELTESTDURATIONW_INTERFACE Interface_ID(0x61324c5c, 0x1eb34501) 

#define GetParticleChannelTestDurationRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELTESTDURATIONR_INTERFACE)) 
#define GetParticleChannelTestDurationWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELTESTDURATIONW_INTERFACE)) 

// static members
Object*		PFTestDuration::m_editOb				= NULL;
IObjParam*	PFTestDuration::m_ip					= NULL;


// constructors/destructors
PFTestDuration::PFTestDuration()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestDuration::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &duration_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &duration_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &duration_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestDuration::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_DURATION_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestDuration::ClassID()
{
	return PFTestDuration_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestDuration::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestDuration::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PFTestDuration::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kDuration_variation:
					RefreshUI(kDuration_message_variation);
					// the break is omitted on purpose (bayboro 11-18-02)
				case kDuration_testType:
				case kDuration_conditionType:
				case kDuration_testValue:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kDuration_disparity:
					RefreshUI(kDuration_message_disparity);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kDuration_RefMsg_InitDlg:
			RefreshUI(kDuration_message_variation, (IParamMap2*)partID);
			RefreshUI(kDuration_message_disparity, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kDuration_RefMsg_NewRand:
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
RefTargetHandle PFTestDuration::Clone(RemapDir &remap)
{
	PFTestDuration* newTest = new PFTestDuration();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestDuration::GetObjectName()
{
	return GetString(IDS_TEST_DURATION_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestDuration::GetRand()
{
	return pblock()->GetInt(kDuration_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestDuration::SetRand(int seed)
{
	pblock()->SetValue(kDuration_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFTestDuration::ChannelsUsed(const Interval& time) const
{
								//  read									  &	write channels
	static ParticleChannelMask mask(PCU_New|PCU_ID|PCU_Time|PCU_BirthTime, 0);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELTESTDURATIONR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELTESTDURATIONW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestDuration::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DURATION_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestDuration::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DURATION_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestDuration::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DURATION_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestDuration::HasDynamicName(TSTR& nameSuffix)
{
	int testType	= pblock()->GetInt(kDuration_testType, 0);
	switch(testType) {
	case kDuration_testType_time:
//		nameSuffix = GetString(IDS_ABSOLUTE_ABBR);
		nameSuffix = GetString(IDS_TIME);
		break;
	case kDuration_testType_age:
		nameSuffix = GetString(IDS_AGE);
		break;
	case kDuration_testType_event:
		nameSuffix = GetString(IDS_EVENT);
		break;
	}
	int condType	= pblock()->GetInt(kDuration_conditionType, 0);
	if (condType == kDuration_conditionType_less)
		nameSuffix += "<";
	else
		nameSuffix += ">";
	TimeValue testValue	= pblock()->GetTimeValue(kDuration_testValue, 0);
	int tpf = GetTicksPerFrame();
	testValue /= tpf;
	TCHAR buf[32];
	sprintf(buf, "%d", testValue);
	nameSuffix += buf;
	TimeValue variation = pblock()->GetTimeValue(kDuration_variation, 0);
	variation /= tpf;
	if (variation > 0) {
		buf[0] = BYTE(177);
		buf[1] = 0;
		nameSuffix += buf;
		sprintf(buf, "%d", variation);
		nameSuffix += buf;
	}
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestDuration::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	TimeValue proceedTime = timeStart;
	int testType	= pblock()->GetInt(kDuration_testType, proceedTime);
	int disparity	= pblock()->GetInt(kDuration_disparity, proceedTime);

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing info for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation

	// acquire more particle channels
	IParticleChannelPTVR* chBirthTime = NULL;
	if (testType == kDuration_testType_age)
	{
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirthTime == NULL) return false; // can't read particle age
	}
	IParticleChannelIDR* chID = NULL;
	if (disparity != 0)
	{
		chID = GetParticleChannelIDRInterface(pCont);
		if (chID == NULL) return false; // can't read particle index for first-to-last disparity
	}

	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (testType == kDuration_testType_event) {
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

	// acquire TestDuration private particle channel; if not present then create it		
	IParticleChannelPTVW* chTestW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELTESTDURATIONW_INTERFACE,
																			ParticleChannelPTV_Class_ID,
																			true, PARTICLECHANNELTESTDURATIONR_INTERFACE,
																			PARTICLECHANNELTESTDURATIONW_INTERFACE, false,
																			actionNode, (Object*)this);
	IParticleChannelPTVR* chTestR = (IParticleChannelPTVR*)chCont->GetPrivateInterface(PARTICLECHANNELTESTDURATIONR_INTERFACE, (Object*)this);
	if ((chTestR == NULL) || (chTestW == NULL)) return false; // can't set test value for newly entered particles

	int i, count;
	PreciseTimeValue curTestValue;
	count = chAmount->Count();
	
	// check if all particles are "old". If some particles are "new" then we
	// have to calculate test values for those.
	if (!chNew->IsAllOld())
	{
		TimeValue testValue	= pblock()->GetTimeValue(kDuration_testValue, proceedTime);
		TimeValue variation = pblock()->GetTimeValue(kDuration_variation, proceedTime);
		int subframe		= pblock()->GetInt(kDuration_subframeSampling, proceedTime);
		TimeValue testFirst = pblock()->GetInt(kDuration_testFirst, proceedTime);
		TimeValue testLast	= pblock()->GetInt(kDuration_testLast, proceedTime);
		TimeValue lastIndex = pblock()->GetInt(kDuration_lastIndex, proceedTime);
		if (lastIndex <= 0) lastIndex = 1;
		int tpf = GetTicksPerFrame();

		RandGenerator* randGen = NULL;
		if (variation != 0) randGen = randLinker().GetRandGenerator(pCont);
		
		int index;
		for(i=0; i<count; i++) 
			if (chNew->IsNew(i)) // calculate test value only for new particles
			{
				if (disparity) {
					index = chID->GetParticleIndex(i);
					if (index > lastIndex) 
						curTestValue = PreciseTimeValue( testLast );
					else 
						curTestValue = PreciseTimeValue( testFirst + (testLast-testFirst)*(float(index)/lastIndex) );
				} else
					curTestValue = PreciseTimeValue( testValue );

				if (variation != 0)
					curTestValue += PreciseTimeValue( randGen->Rand11()*variation );

				// adjust test value according to test type
				if (testType == kDuration_testType_age)
					curTestValue += chBirthTime->GetValue(i);
				else if (testType == kDuration_testType_event) {
					if (initEventStart)
						chEventStartW->SetValue(i, chTime->GetValue(i));
					curTestValue += chEventStartR->GetValue(i);
				}

				if (!subframe) // round the test value to the nearest frame
					curTestValue = PreciseTimeValue(int(floor(TimeValue(curTestValue)/float(tpf) + 0.5f) * tpf));

				chTestW->SetValue(i, curTestValue);
			}
	}
	
	// test all particles
	PreciseTimeValue curParticleValue;
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	int condType = pblock()->GetInt(kDuration_conditionType, proceedTime);
	BitArray particlesToAdvance;
	particlesToAdvance.SetSize(count);
	particlesToAdvance.ClearAll();
	for(i=0; i<count; i++)
	{
		curTestValue = chTestR->GetValue(i);
		curParticleValue = chTime->GetValue(i);
		if (curParticleValue > timeEnd) continue; // particle has been proceeded beyond 
												  // the testing interval [timeStart,timeEnd]
		switch( condType )
		{
		case kDuration_conditionType_less:
			if (curParticleValue <= curTestValue)
			{ // particle doesn't need to be advanced in "time" since the current time value is the condition value
				testResult.Set(i);
				testTime[i] = float( curParticleValue - timeStart );
			}
			break;
		case kDuration_conditionType_greater:
			if (timeEnd < curTestValue) break;	// particle won't satisfy the condition 
												// even at the end of the test interval			
			if (curParticleValue >= curTestValue)
			{ // particle doesn't need to be advanced in "time" since the current time value is more than satisfactory
				testResult.Set(i);
				testTime[i] = float( curParticleValue - timeStart );
				break;
			}

			testResult.Set(i);
			testTime[i] = float( curTestValue - timeStart );
			// the particle needs to be advanced in time if possible
			particlesToAdvance.Set(i);
			break;
		default:
			DbgAssert(0);
			break;
		}
	}

	// advance particles in time if they satisfy the condition and need to be pushed forward
	if (integrator != NULL)
	{
		Tab<PreciseTimeValue> timeToAdvance;
		timeToAdvance.SetCount(count);
		for(i=0; i<count; i++)
			if (particlesToAdvance[i] != 0)
				timeToAdvance[i] = timeStart + testTime[i];
		integrator->Proceed(pCont, timeToAdvance, particlesToAdvance);
	}

	return true;
}

ClassDesc* PFTestDuration::GetClassDesc() const
{
	return GetPFTestDurationDesc();
}



} // end of namespace PFActions