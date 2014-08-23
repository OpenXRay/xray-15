/**********************************************************************
 *<
	FILE: PFTestSplitByAmount.h

	DESCRIPTION: SplitByAmount Test implementation
				 The Test checks particles by different numerical
				 properties

	CREATED BY: Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFTestSplitByAmount.h"

#include "PFTestSplitByAmount_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// SplitByAmount Test creates a particle channel to store a test value for each particle
// This way the value is calculated only once when a particle enters the event
#define PARTICLECHANNELTESTSPLITBYAMOUNTR_INTERFACE Interface_ID(0x4f535065, 0x1eb34500) 
#define PARTICLECHANNELTESTSPLITBYAMOUNTW_INTERFACE Interface_ID(0x4f535065, 0x1eb34501) 

#define GetParticleChannelTestSplitByAmountRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELTESTSPLITBYAMOUNTR_INTERFACE)) 
#define GetParticleChannelTestSplitByAmountWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELTESTSPLITBYAMOUNTW_INTERFACE)) 

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFTestSplitByAmountState					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFTestSplitByAmountState::GetInterface(Interface_ID id)
{
	if (id == PFACTIONSTATE_INTERFACE) return (IPFActionState*)this;
	return IObject::GetInterface(id);
}

//+--------------------------------------------------------------------------+
//|				From PFTestSplitByAmountState::IObject						 |
//+--------------------------------------------------------------------------+
void PFTestSplitByAmountState::DeleteIObject()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|				From PFTestSplitByAmountState::IPFActionState			 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSplitByAmountState::GetClassID() 
{ 
	return PFTestSplitByAmountState_Class_ID; 
}

//+--------------------------------------------------------------------------+
//|				From PFTestSplitByAmountState::IPFActionState				 |
//+--------------------------------------------------------------------------+
IOResult PFTestSplitByAmountState::Save(ISave* isave) const
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
	int wT = wentThru();
	if ((res = isave->Write(&wT, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData2);
	int wTA = wentThruAccum();
	if ((res = isave->Write(&wTA, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(IPFActionState::kChunkData3);
	int wTT = wentThruTotal();
	if ((res = isave->Write(&wTT, sizeof(int), &nb)) != IO_OK) return res;
	isave->EndChunk();

	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|			From PFTestSplitByAmountState::IPFActionState					 |
//+--------------------------------------------------------------------------+
IOResult PFTestSplitByAmountState::Load(ILoad* iload)
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
			res=iload->Read(&_wentThru(), sizeof(int), &nb);
			break;

		case IPFActionState::kChunkData2:
			res=iload->Read(&_wentThruAccum(), sizeof(int), &nb);
			break;

		case IPFActionState::kChunkData3:
			res=iload->Read(&_wentThruTotal(), sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFTestSplitByAmount						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
// constructors/destructors
PFTestSplitByAmount::PFTestSplitByAmount()
{ 
	_renderMode() = false;
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestSplitByAmount::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &splitByAmount_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &splitByAmount_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &splitByAmount_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestSplitByAmount::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SPLITBYAMOUNT_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSplitByAmount::ClassID()
{
	return PFTestSplitByAmount_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSplitByAmount::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSplitByAmount::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
int PFTestSplitByAmount::RenderBegin(TimeValue t, ULONG flags)
{
	_renderMode() = true;
	return 0;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
int PFTestSplitByAmount::RenderEnd(TimeValue t)
{
	_renderMode() = false;
	return 0;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PFTestSplitByAmount::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSplitByAmount_testType:
					RefreshUI(kSplitByAmount_message_type);
					// break is omitted on purpose (bayboro 11-19-02)
				case kSplitByAmount_fraction:
				case kSplitByAmount_everyN:
				case kSplitByAmount_firstN:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kSplitByAmount_RefMsg_InitDlg:
			RefreshUI(kSplitByAmount_message_type, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSplitByAmount_RefMsg_NewRand:
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
RefTargetHandle PFTestSplitByAmount::Clone(RemapDir &remap)
{
	PFTestSplitByAmount* newTest = new PFTestSplitByAmount();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestSplitByAmount::GetObjectName()
{
	return GetString(IDS_TEST_SPLITBYAMOUNT_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
int	PFTestSplitByAmount::GetRand()
{
	return pblock()->GetInt(kSplitByAmount_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestSplitByAmount::SetRand(int seed)
{
	pblock()->SetValue(kSplitByAmount_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFTestSplitByAmount::ChannelsUsed(const Interval& time) const
{
								//  read						  &	write channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New, 0);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELTESTSPLITBYAMOUNTR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELTESTSPLITBYAMOUNTW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestSplitByAmount::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	bool initRes = PFSimpleAction::Init(pCont, pSystem, node, actions, actionNodes);
	int index;
	if (!hasParticleContainer(pCont, index)) {
		if (!addParticleContainer(pCont, node)) return false;
		if (!hasParticleContainer(pCont, index)) return false;
	}
	_allSystemNode(index) = node; // necessary step because it may be not initialized after SetCurrentState
	_wentThruAccum(index) = 0;
	_wentThruTotal(index) = 0;
	_lastUpdate(index) = TIME_NegInfinity;
	return initRes;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestSplitByAmount::Release(IObject* pCont)
{
	removeParticleContainer(pCont);
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFTestSplitByAmount::GetCurrentState(IObject* pContainer)
{
	PFTestSplitByAmountState* actionState = (PFTestSplitByAmountState*)CreateInstance(REF_TARGET_CLASS_ID, PFTestSplitByAmountState_Class_ID);
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL)
		memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
	int index;
	if (hasParticleContainer(pContainer, index)) {
		actionState->_wentThruAccum() = wentThruAccum(index);
		actionState->_wentThruTotal() = wentThruTotal(index);
	} else {
		actionState->_wentThruAccum() = 0;
		actionState->_wentThruTotal() = 0;
	}
	actionState->_wentThru() = 0; // obsolete
	return actionState;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestSplitByAmount::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFTestSplitByAmountState* actionState = (PFTestSplitByAmountState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
	}
	int index = -1;
	if (!hasParticleContainer(pContainer, index)) {
		addParticleContainer(pContainer, NULL);
		hasParticleContainer(pContainer, index);
	}
	if (index >= 0) {
		_wentThruAccum(index) = actionState->wentThruAccum();
		_wentThruTotal(index) = actionState->wentThruTotal();
	}
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSplitByAmount::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPLITBYAMOUNT_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSplitByAmount::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPLITBYAMOUNT_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSplitByAmount::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPLITBYAMOUNT_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestSplitByAmount::HasDynamicName(TSTR& nameSuffix)
{
	Control* ctrl;
	bool isAnimated;
	int type = pblock()->GetInt(kSplitByAmount_testType, 0);
	TCHAR buf[32];
	switch(type) {
	case kSplitByAmount_testType_fraction:
		ctrl = pblock()->GetController(kSplitByAmount_fraction);
		isAnimated = (ctrl != NULL) ? (ctrl->IsAnimated() != 0) : false;
		if (isAnimated) {
			nameSuffix = GetString(IDS_FRACTION);
		} else {
			sprintf(buf,"%d%%",int(pblock()->GetFloat(kSplitByAmount_fraction, 0)*100.0f + 0.5f));
			nameSuffix = TSTR(buf);
		}
		break;
	case kSplitByAmount_testType_everyN:
		ctrl = pblock()->GetController(kSplitByAmount_everyN);
		isAnimated = (ctrl != NULL) ? (ctrl->IsAnimated() != 0) : false;
		if (isAnimated) {
			nameSuffix = GetString(IDS_EVERY);
			nameSuffix += TSTR(" N");
			nameSuffix += GetString(IDS_TH_LikeInNth);
		} else {
			int n = pblock()->GetInt(kSplitByAmount_everyN, 0);
			sprintf(buf," %d",n);
			nameSuffix += buf;
			if (n == 1) {
				nameSuffix = GetString(IDS_ALL);
			} else {
				nameSuffix = GetString(IDS_EVERY);
				sprintf(buf," %d",pblock()->GetInt(kSplitByAmount_everyN, 0));
				nameSuffix += TSTR(buf);
				if ((n%100 > 10) && (n%100 < 20)) {
					nameSuffix += GetString(IDS_TH_LikeInNth);
				} else {
					switch(n%10) {
					case 1: nameSuffix += GetString(IDS_ST_LikeIn1st); break;
					case 2: nameSuffix += GetString(IDS_ND_LikeIn2nd); break;
					case 3: nameSuffix += GetString(IDS_RD_LikeIn3rd); break;
					default: nameSuffix += GetString(IDS_TH_LikeInNth); break;
					}
				}
			}
		}
		break;
	case kSplitByAmount_testType_firstN:
		nameSuffix = GetString(IDS_FIRST);
		sprintf(buf," %d",pblock()->GetInt(kSplitByAmount_firstN, 0));
		nameSuffix += buf;
		break;
	case kSplitByAmount_testType_afterFirstN:
		nameSuffix = GetString(IDS_AFTER);
		sprintf(buf," %d",pblock()->GetInt(kSplitByAmount_firstN, 0));
		nameSuffix += buf;
		break;
	}
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSplitByAmount::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	int contIndex;
	if (!hasParticleContainer(pCont, contIndex)) return false;
	_lastUpdate(contIndex) = timeEnd.TimeValue();
	bool exactStep = IsExactIntegrationStep(timeEnd, pSystem);

	// update all other systems to the current time; everybody should be in sync
	// for proper accumulation amounts
	int i;
	for(i=0; i<allParticleContainers().Count(); i++) {
		if (allParticleContainer(i) == pCont) continue;
		if (allSystemNode(i) == pNode) continue;
		if (lastUpdate(i) == timeEnd.TimeValue()) continue;
		TimeValue timeToUpdateTo = timeEnd.TimeValue();
		allSystemNode(i)->NotifyDependents(FOREVER, PartID(&timeToUpdateTo), kPFMSG_UpdateToTime, NOTIFY_ALL, TRUE );
	}

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find "new" property of particles in the container

	// acquire TestSplitByAmount private particle channel; if not present then create it		
	IParticleChannelBoolW* chTestW = (IParticleChannelBoolW*)chCont->EnsureInterface(PARTICLECHANNELTESTSPLITBYAMOUNTW_INTERFACE,
																			ParticleChannelBool_Class_ID,
																			true, PARTICLECHANNELTESTSPLITBYAMOUNTR_INTERFACE,
																			PARTICLECHANNELTESTSPLITBYAMOUNTW_INTERFACE, false,
																			actionNode, (Object*)this);
	IParticleChannelBoolR* chTestR = (IParticleChannelBoolR*)chCont->GetPrivateInterface(PARTICLECHANNELTESTSPLITBYAMOUNTR_INTERFACE, (Object*)this);
	if ((chTestR == NULL) || (chTestW == NULL)) return false; // can't set test value for newly entered particles

	int count = chAmount->Count();
	
	// check if all particles are "old". If some particles are "new" then we
	// have to calculate test values for those.
	if (!chNew->IsAllOld())
	{
		RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
		if (randGen == NULL) return false;

		int testType	= pblock()->GetInt(kSplitByAmount_testType, timeStart);
		float fraction = GetPFFloat(pblock(), kSplitByAmount_fraction, timeStart);
		int everyN = GetPFInt(pblock(), kSplitByAmount_everyN, timeStart);
		int firstN = pblock()->GetInt(kSplitByAmount_firstN, timeStart);
		bool perSource = (pblock()->GetInt(kSplitByAmount_perSource, timeStart) != 0);
		int curWentThru = perSource ? wentThruTotal(pNode) : wentThruTotal();

		// number of "first N" particles is adjusted by multiplier coefficient
		// of the master particle system. This is done to make "first N"
		// parameter to be consistent to "total" number of particles acclaimed
		// by a birth operator
		IPFSystem* pfSys = PFSystemInterface(pSystem);
		if (pfSys == NULL) return false; // no handle for PFSystem interface
		firstN *= pfSys->GetMultiplier(timeStart); 

		for(i=0; i<count; i++) {
			if (chNew->IsNew(i)) { // calculate test value only for new particles
				bool sendOut = false;
				switch(testType) {
				case kSplitByAmount_testType_fraction:
					sendOut = (randGen->Rand01() <= fraction);
					break;
				case kSplitByAmount_testType_everyN:
					_wentThruAccum(contIndex) += 1;
					if (wentThruAccum(contIndex) >= everyN) {
						sendOut = true;
						_wentThruAccum(contIndex) = 0;
					}
					break;
				case kSplitByAmount_testType_firstN:
					_wentThruTotal(contIndex) += 1;
					if (curWentThru++ < firstN) sendOut = true;
					break;
				case kSplitByAmount_testType_afterFirstN:
					_wentThruTotal(contIndex) += 1;
					if (curWentThru++ >= firstN) sendOut = true;
					break;
				}
				chTestW->SetValue(i, sendOut);
			}
		}
	}

	// check all particles by predefined test channel
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	if (exactStep) {
		for(i=0; i<count; i++)
		{	
			if (chTestR->GetValue(i)) {
				testResult.Set(i);
				testTime[i] = 0.0f;
			}
		}
	}
	return true;
}

ClassDesc* PFTestSplitByAmount::GetClassDesc() const
{
	return GetPFTestSplitByAmountDesc();
}


bool PFTestSplitByAmount::hasParticleContainer(IObject* pCont, int& index)
{
	index = 0;
	for(int i=0; i<allParticleContainers().Count(); i++, index++) {
		if (allParticleContainer(i) == pCont) return true;
	}
	return false;
}

bool PFTestSplitByAmount::addParticleContainer(IObject* pCont, INode* systemNode)
{
	_allParticleContainers().Append(1, &pCont);
	_allSystemNodes().Append(1, &systemNode);
	int num = 0;
	_wentThruAccums().Append(1, &num);
	_wentThruTotals().Append(1, &num);
	TimeValue t = TIME_NegInfinity;
	_lastUpdates().Append(1, &t);
	return true;
}

bool PFTestSplitByAmount::removeParticleContainer(IObject* pCont)
{
	int index;
	if (!hasParticleContainer(pCont, index)) return false;
	_allParticleContainers().Delete(index, 1);
	_allSystemNodes().Delete(index, 1);
	_wentThruAccums().Delete(index, 1);
	_wentThruTotals().Delete(index, 1);
	_lastUpdates().Delete(index, 1);
	return true;
}

int PFTestSplitByAmount::wentThruAccum(INode* systemNode)
{
	int num = 0;
	for(int i=0; i<allSystemNodes().Count(); i++)
		if (allSystemNode(i) == systemNode)
			num += wentThruAccum(i);
	return num;
}

int PFTestSplitByAmount::wentThruTotal(INode* systemNode)
{
	int num = 0;
	for(int i=0; i<allSystemNodes().Count(); i++)
		if (allSystemNode(i) == systemNode)
			num += wentThruTotal(i);
	return num;
}

int PFTestSplitByAmount::wentThruAccum()
{
	int num = 0;
	for(int i=0; i<wentThruAccums().Count(); i++)
		num += wentThruAccum(i);
	return num;
}

int PFTestSplitByAmount::wentThruTotal()
{
	int num = 0;
	for(int i=0; i<wentThruTotals().Count(); i++)
		num += wentThruTotal(i);
	return num;
}

} // end of namespace PFActions