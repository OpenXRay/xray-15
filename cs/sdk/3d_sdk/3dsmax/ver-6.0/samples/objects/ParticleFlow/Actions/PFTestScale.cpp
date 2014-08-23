/**********************************************************************
 *<
	FILE: PFTestScale.h

	DESCRIPTION: Scale Test implementation
				 The Test checks particles for either scale/size
				 properties of a particle

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

#include "PFTestScale.h"

#include "PFTestScale_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// Scale Test creates a particle channel to store a random value for each particle [-1.0f, 1.0f]
// The value is used to calculation test value with variation
#define PARTICLECHANNELRANDFLOATR_INTERFACE Interface_ID(0x4a627284, 0x1eb34500) 
#define PARTICLECHANNELRANDFLOATW_INTERFACE Interface_ID(0x4a627284, 0x1eb34501) 
#define GetParticleChannelRandFloatRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELRANDFLOATR_INTERFACE)) 
#define GetParticleChannelRandFloatWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELRANDFLOATW_INTERFACE)) 

// constructors/destructors
PFTestScale::PFTestScale()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestScale::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &scaleTest_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &scaleTest_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &scaleTest_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestScale::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SCALE_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestScale::ClassID()
{
	return PFTestScale_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestScale::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestScale::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PFTestScale::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kScaleTest_testType:
				case kScaleTest_sizeVariation:
				case kScaleTest_scaleVariation:
					RefreshUI(kScaleTest_message_update);
					// the break is omitted on purpose (bayboro 11-18-02)
				case kScaleTest_sizeValue:
				case kScaleTest_scaleValue:
				case kScaleTest_conditionType:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kScaleTest_RefMsg_InitDlg:
			RefreshUI(kScaleTest_message_update, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kScaleTest_RefMsg_NewRand:
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
RefTargetHandle PFTestScale::Clone(RemapDir &remap)
{
	PFTestScale* newTest = new PFTestScale();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestScale::GetObjectName()
{
	return GetString(IDS_TEST_SCALE_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestScale::GetRand()
{
	return pblock()->GetInt(kScaleTest_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestScale::SetRand(int seed)
{
	pblock()->SetValue(kScaleTest_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFTestScale::ChannelsUsed(const Interval& time) const
{
								//  read																&	write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_Shape|PCU_Scale, PCU_EventStart);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELRANDFLOATR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELRANDFLOATW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestScale::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SCALETEST_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestScale::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SCALETEST_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestScale::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SCALETEST_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestScale::HasDynamicName(TSTR& nameSuffix)
{
	int testType	= pblock()->GetInt(kScaleTest_testType, 0);
	switch(testType) {
	case kScaleTest_testType_preSize:
		nameSuffix = GetString(IDS_PRESIZE);
		break;
	case kScaleTest_testType_postSize:
		nameSuffix = GetString(IDS_POSTSIZE);
		break;
	case kScaleTest_testType_scale:
		nameSuffix = GetString(IDS_SCALE);
		Control* ctrl = pblock()->GetController(kScaleTest_scaleValue);
		bool isValueAnimated = (ctrl != NULL) ? (ctrl->IsAnimated() != 0) : false;
		int testValue = int(GetPFFloat(pblock(), kScaleTest_scaleValue, 0)*100.0f);
		ctrl = pblock()->GetController(kScaleTest_scaleVariation);
		bool isVarAnimated = (ctrl != NULL) ? (ctrl->IsAnimated() != 0) : false;
		int testVar = int(GetPFFloat(pblock(), kScaleTest_scaleVariation, 0)*100.0f);
		if ((!isValueAnimated) && (!isVarAnimated)) {
			int condType	= pblock()->GetInt(kScaleTest_conditionType, 0);
			if (condType == kScaleTest_conditionType_less)
				nameSuffix += "<";
			else
				nameSuffix += ">";
			TCHAR buf[32];
			sprintf(buf, "%d", testValue);
			nameSuffix += buf;
			if (testVar > 0) {
				buf[0] = BYTE(177);
				buf[1] = 0;
				nameSuffix += buf;
				sprintf(buf, "%d", testVar);
				nameSuffix += buf;
			}
			if (testType == kScaleTest_testType_scale)
				nameSuffix += TSTR("%");
		}
		break;
	}
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestScale::Proceed(IObject* pCont, 
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
	int testType = pblock()->GetInt(kScaleTest_testType, timeEnd);
	int axisType = pblock()->GetInt(kScaleTest_axisType, timeEnd);
	int condType = pblock()->GetInt(kScaleTest_conditionType, timeEnd);
	int syncType = pblock()->GetInt(kScaleTest_sync, timeEnd);
	int varParamID = (testType == kScaleTest_testType_scale) ? kScaleTest_scaleVariation : kScaleTest_sizeVariation;
	bool hasTestVariation = (pblock()->GetFloat(varParamID, 0) != 0.0f);
	if (!hasTestVariation) {
		Control* ctrl = pblock()->GetController(varParamID);
		if (ctrl != NULL)
			hasTestVariation = (ctrl->IsAnimated() != 0);
	}
	
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

	// acquire more particle channels
	IParticleChannelPTVR* chBirthTime = NULL;
	if (syncType == kScaleTest_sync_age)
	{
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirthTime == NULL) return false; // can't read particle age
	}
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (syncType == kScaleTest_sync_event) {
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
	IParticleChannelMeshR* chShape = NULL;
	if (testType != kScaleTest_testType_scale) {
		chShape = GetParticleChannelShapeRInterface(pCont);
		if (chShape == NULL) return false; // can't read particle shape to find bounding box
	}
	IParticleChannelPoint3R* chScale = NULL;
	if (testType != kScaleTest_testType_preSize) {
		chScale = GetParticleChannelScaleRInterface(pCont);
		if (chScale == NULL) return false; // can't read particle scale
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

		PreciseTimeValue syncTime = chTime->GetValue(i);
		switch(syncType) {
		case kScaleTest_sync_age:
			syncTime -= chBirthTime->GetValue(i);
			break;
		case kScaleTest_sync_event:
			syncTime -= chEventStartR->GetValue(i);
			break;
		}
		TimeValue syncTimeTV = TimeValue(syncTime);

		float testValue = 0.0f;
		if (testType == kScaleTest_testType_scale) {
			testValue = GetPFFloat(pblock(), kScaleTest_scaleValue, syncTimeTV);
			if (hasTestVariation)
				testValue += chRandFloatR->GetValue(i)*GetPFFloat(pblock(), kScaleTest_scaleVariation, syncTimeTV);
		} else {
			testValue = GetPFFloat(pblock(), kScaleTest_sizeValue, syncTimeTV);
			if (hasTestVariation)
				testValue += chRandFloatR->GetValue(i)*GetPFFloat(pblock(), kScaleTest_sizeVariation, syncTimeTV);
		}

		Point3 cur3DValue;
		if (testType == kScaleTest_testType_scale) {
			cur3DValue = chScale->GetValue(i);
		} else {
			Mesh* curMesh = const_cast <Mesh*>(chShape->GetValue(i));
			if (curMesh == NULL) continue;
			Box3 curBox = curMesh->getBoundingBox();
			cur3DValue = curBox.pmax - curBox.pmin;
			if (testType == kScaleTest_testType_postSize)
				cur3DValue *= chScale->GetValue(i);
		}

		float currentValue;
		switch(axisType) {
		case kScaleTest_axisType_average:
			currentValue = (cur3DValue.x + cur3DValue.y + cur3DValue.z)/3.0f;
			break;
		case kScaleTest_axisType_minimum:
			currentValue = min(cur3DValue.x, min(cur3DValue.y, cur3DValue.z));
			break;
		case kScaleTest_axisType_median:
			currentValue = max(min(cur3DValue.x, cur3DValue.y), 
								max(min(cur3DValue.x, cur3DValue.z), min(cur3DValue.y, cur3DValue.z)));
			break;
		case kScaleTest_axisType_maximum:
			currentValue = max(cur3DValue.x, max(cur3DValue.y, cur3DValue.z));
			break;
		case kScaleTest_axisType_x:
			currentValue = cur3DValue.x;
			break;
		case kScaleTest_axisType_y:
			currentValue = cur3DValue.y;
			break;
		case kScaleTest_axisType_z:
			currentValue = cur3DValue.z;
			break;
		}

		bool testSatisfied = (condType == kScaleTest_conditionType_less) ?
								(currentValue < testValue) : (currentValue > testValue);
		if (testSatisfied && exactStep) {
			testResult.Set(i);
			testTime[i] = 0.0f;
		}
	}

	return true;
}

ClassDesc* PFTestScale::GetClassDesc() const
{
	return GetPFTestScaleDesc();
}



} // end of namespace PFActions