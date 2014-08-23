/**********************************************************************
 *<
	FILE: PFTestGoToRotation.h

	DESCRIPTION: GoToRotation Test implementation
				 The Test makes a smooth transition in rotation
				 When the transition is complete the particles are
				 sent to the next event

	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-14-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFTestGoToRotation.h"

#include "PFTestGoToRotation_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "IParticleObjectExt.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "PFSimpleActionState.h"

namespace PFActions {

// the test creates several transferable private particle channels to store some data per particle
// channel stores start time for the transition
#define PARTICLECHANNELSTARTTIMER_INTERFACE Interface_ID(0x7f9128f1, 0x1eb34500) 
#define PARTICLECHANNELSTARTTIMEW_INTERFACE Interface_ID(0x7f9128f1, 0x1eb34501) 
#define GetParticleChannelStartTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELSTARTTIMER_INTERFACE)) 
#define GetParticleChannelStartTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELSTARTTIMEW_INTERFACE)) 

// channel stores end time for the transition
#define PARTICLECHANNELENDTIMER_INTERFACE Interface_ID(0x7f9128f2, 0x1eb34500) 
#define PARTICLECHANNELENDTIMEW_INTERFACE Interface_ID(0x7f9128f2, 0x1eb34501) 
#define GetParticleChannelEndTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELENDTIMER_INTERFACE)) 
#define GetParticleChannelEndTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELENDTIMEW_INTERFACE)) 

// channel stores time of the last proceed
// the data is used to rollback the effect of integration to find the desirable orientation
#define PARTICLECHANNELPROCEEDTIMER_INTERFACE Interface_ID(0x7f9128f3, 0x1eb34500) 
#define PARTICLECHANNELPROCEEDTIMEW_INTERFACE Interface_ID(0x7f9128f3, 0x1eb34501) 
#define GetParticleChannelProceedTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELPROCEEDTIMER_INTERFACE)) 
#define GetParticleChannelProceedTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELPROCEEDTIMEW_INTERFACE)) 

// channel stores if the end rotation state has been initialized
// the channel is used only if the End Rotation is Constant type, otherwise the end rotation is updated at each integration step
#define PARTICLECHANNELGOTINITR_INTERFACE Interface_ID(0x7f9128f4, 0x1eb34500) 
#define PARTICLECHANNELGOTINITW_INTERFACE Interface_ID(0x7f9128f4, 0x1eb34501) 
#define GetParticleChannelGotInitRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELGOTINITR_INTERFACE)) 
#define GetParticleChannelGotInitWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELGOTINITW_INTERFACE)) 

// channel stores start rotation
#define PARTICLECHANNELSTARTROTR_INTERFACE Interface_ID(0x7f9128f5, 0x1eb34500) 
#define PARTICLECHANNELSTARTROTW_INTERFACE Interface_ID(0x7f9128f5, 0x1eb34501) 
#define GetParticleChannelStartRotRInterface(obj) ((IParticleChannelQuatR*)obj->GetInterface(PARTICLECHANNELSTARTROTR_INTERFACE)) 
#define GetParticleChannelStartRotWInterface(obj) ((IParticleChannelQuatW*)obj->GetInterface(PARTICLECHANNELSTARTROTW_INTERFACE)) 

// channel stores end rotation
#define PARTICLECHANNELENDROTR_INTERFACE Interface_ID(0x7f9128f6, 0x1eb34500) 
#define PARTICLECHANNELENDROTW_INTERFACE Interface_ID(0x7f9128f6, 0x1eb34501) 
#define GetParticleChannelEndRotRInterface(obj) ((IParticleChannelQuatR*)obj->GetInterface(PARTICLECHANNELENDROTR_INTERFACE)) 
#define GetParticleChannelEndRotWInterface(obj) ((IParticleChannelQuatW*)obj->GetInterface(PARTICLECHANNELENDROTW_INTERFACE)) 

// channel stores start spin (rotation axis and spin rate as an angle)
#define PARTICLECHANNELSTARTSPINR_INTERFACE Interface_ID(0x7f9128f7, 0x1eb34500) 
#define PARTICLECHANNELSTARTSPINW_INTERFACE Interface_ID(0x7f9128f7, 0x1eb34501) 
#define GetParticleChannelStartSpinRInterface(obj) ((IParticleChannelAngAxisR*)obj->GetInterface(PARTICLECHANNELSTARTSPINR_INTERFACE)) 
#define GetParticleChannelStartSpinWInterface(obj) ((IParticleChannelAngAxisW*)obj->GetInterface(PARTICLECHANNELSTARTSPINW_INTERFACE)) 

// channel stores end spin (spin rate only as float)
#define PARTICLECHANNELENDSPINR_INTERFACE Interface_ID(0x7f9128f8, 0x1eb34500) 
#define PARTICLECHANNELENDSPINW_INTERFACE Interface_ID(0x7f9128f8, 0x1eb34501) 
#define GetParticleChannelEndSpinRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELENDSPINR_INTERFACE)) 
#define GetParticleChannelEndSpinWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELENDSPINW_INTERFACE)) 

// static members
Object*		PFTestGoToRotation::m_editOb				= NULL;
IObjParam*	PFTestGoToRotation::m_ip					= NULL;

// constructors/destructors
PFTestGoToRotation::PFTestGoToRotation()
				:IPFTest()
{ 
	RegisterParticleFlowNotification();
	_postProceed() = false;
	_pblock() = NULL;
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _trueIcon() = _falseIcon() = NULL;
}

PFTestGoToRotation::~PFTestGoToRotation()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFTestGoToRotation::GetInterface(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return (IPFAction*)this;
	if (id == PFOPERATOR_INTERFACE) return (IPFOperator*)this;
	if (id == PFTEST_INTERFACE) return (IPFTest*)this;
	if (id == PVIEWITEM_INTERFACE) return (IPViewItem*)this;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestGoToRotation::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &goToRotation_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &goToRotation_operator_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &goToRotation_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &goToRotation_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestGoToRotation::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_GOTOROTATION_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestGoToRotation::ClassID()
{
	return PFTestGoToRotation_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFTestGoToRotation::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFTestGoToRotation::SubAnimName(int i)
{
	return PFActions::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFTestGoToRotation::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestGoToRotation::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestGoToRotation::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFTestGoToRotation::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PFTestGoToRotation::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	IParamMap2* map;

	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) 
			{
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kGoToRotation_syncBy:
					RefreshUI(kGoToRotation_message_sync);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kGoToRotation_time:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kGoToRotation_variation:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					RefreshUI(kGoToRotation_message_variation);
					break;
				case kGoToRotation_spinVariation:
					RefreshUI(kGoToRotation_message_variation);
					break;
				case kGoToRotation_matchSpin:
					RefreshUI(kGoToRotation_message_match);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kGoToRotation_RefMsg_InitDlg:
			map = (IParamMap2*)partID;
			RefreshUI(kGoToRotation_message_sync, map);
			RefreshUI(kGoToRotation_message_variation, map);
			RefreshUI(kGoToRotation_message_match);
			return REF_STOP;

		// New Random Seed
		case kGoToRotation_RefMsg_NewRand:
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
RefTargetHandle PFTestGoToRotation::Clone(RemapDir &remap)
{
	PFTestGoToRotation* newTest = new PFTestGoToRotation();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PFTestGoToRotation::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestGoToRotation::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestGoToRotation::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestGoToRotation::GetObjectName()
{
	return GetString(IDS_TEST_GOTOROTATION_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestGoToRotation::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	return _randLinker().Init( pCont, GetRand() );
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestGoToRotation::Release(IObject* pCont)
{
	_randLinker().Release( pCont );
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestGoToRotation::GetRand()
{
	return pblock()->GetInt(kGoToRotation_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::SetRand(int seed)
{
	pblock()->SetValue(kGoToRotation_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFTestGoToRotation::GetCurrentState(IObject* pContainer)
{
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen != NULL) {
		PFSimpleActionState* actionState = (PFSimpleActionState*)CreateInstance(REF_TARGET_CLASS_ID, PFSimpleActionState_Class_ID);
		if (actionState->randGen() != NULL)
			memcpy(actionState->_randGen(), randGen, sizeof(RandGenerator));
		return actionState;
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::SetCurrentState(IObject* aSt, IObject* pContainer)
{
	if (aSt == NULL) return;
	PFSimpleActionState* actionState = (PFSimpleActionState*)aSt;
	RandGenerator* randGen = randLinker().GetRandGenerator(pContainer);
	if (randGen == NULL) return;
	memcpy(randGen, actionState->randGen(), sizeof(RandGenerator));
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFTestGoToRotation::ChannelsUsed(const Interval& time) const
{
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_BirthTime|PCU_Orientation|PCU_Spin, // read 
									PCU_Orientation|PCU_Spin); // write
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELSTARTTIMER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELSTARTTIMEW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELENDTIMER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELENDTIMEW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELPROCEEDTIMER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELPROCEEDTIMEW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELGOTINITR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELGOTINITW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELSTARTROTR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELSTARTROTW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELENDROTR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELENDROTW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELSTARTSPINR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELSTARTSPINW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELENDSPINR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELENDSPINW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestGoToRotation::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_GOTOROTATION_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestGoToRotation::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_GOTOROTATION_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestGoToRotation::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_GOTOROTATION_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestGoToRotation::HasDynamicName(TSTR& nameSuffix)
{
	int type = pblock()->GetInt(kGoToRotation_syncBy, 0);
	switch(type) {
	case kGoToRotation_syncBy_time:
		nameSuffix = GetString(IDS_TIME);
		break;
	case kGoToRotation_syncBy_age:
		nameSuffix = GetString(IDS_AGE);
		break;
	case kGoToRotation_syncBy_event:
		nameSuffix = GetString(IDS_EVENT);
		break;
	}
	nameSuffix += TSTR(" ");
	int tpf = GetTicksPerFrame();
	TimeValue time	= pblock()->GetTimeValue(kGoToRotation_time, 0)/tpf;
	TCHAR buf[32];
	sprintf(buf, "%d", time);
	nameSuffix += buf;
	TimeValue variation = pblock()->GetTimeValue(kGoToRotation_variation, 0)/tpf;
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
//|							From IPFOperator								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestGoToRotation::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (postProceed()) return doPostProceed(pCont, timeStart, timeEnd, pSystem, pNode, actionNode, integrator);

	if (pblock() == NULL) return false;
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int i, count = chAmount->Count();
	if (count == 0) return true; // no particles to modify
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing info for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for speedGoToTarget calculation
	IParticleChannelPTVR* chBirth = GetParticleChannelBirthTimeRInterface(pCont);
	if (chBirth == NULL) return false; // can't read birth time data
	IParticleChannelQuatR* chOrient = GetParticleChannelOrientationRInterface(pCont);
	if (chOrient == NULL) return false; // can't read current orientation for a particle

	// may create and initialize spin channel if it is not present
	IParticleChannelAngAxisR* chSpinR = NULL;
	IParticleChannelAngAxisW* chSpinW = NULL;
	bool initSpin = false;
	chSpinR = (IParticleChannelAngAxisR*)chCont->EnsureInterface(PARTICLECHANNELSPINR_INTERFACE,
																ParticleChannelAngAxis_Class_ID,
																true, PARTICLECHANNELSPINR_INTERFACE,
																PARTICLECHANNELSPINW_INTERFACE, true,
																actionNode, NULL, &initSpin);
	if (chSpinR == NULL) return false; // can't read spin data
	if (initSpin) {
		chSpinW = GetParticleChannelSpinWInterface(pCont);
		if (chSpinW == NULL) return false; // can't modify spin data
	}
	if (initSpin) {
		AngAxis aa(Point3::XAxis, 0.0f);
		if (!chNew->IsAllOld())
			for(i=0; i<count; i++) {
				if (chNew->IsNew(i))
					chSpinW->SetValue(i, aa);
			}
	}

	// create channel to store the start moment of the transition process
	// the time is when a particle enters the event
	IParticleChannelPTVW* chStartTimeW = NULL;
	bool initStartTime = false;
	chStartTimeW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELSTARTTIMEW_INTERFACE,
																ParticleChannelPTV_Class_ID,
																true, PARTICLECHANNELSTARTTIMER_INTERFACE,
																PARTICLECHANNELSTARTTIMEW_INTERFACE, true,
																actionNode, (Object*)this, &initStartTime);
	if (chStartTimeW == NULL) return false; // can't modify the start time
	
	// create channel to store the end moment of the transition process
	// the time is used to determine when a particle should go to the next event, and when to finish the transition process
	IParticleChannelPTVW* chEndTimeW = NULL;
	bool initEndTime = false;
	chEndTimeW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELENDTIMEW_INTERFACE,
																ParticleChannelPTV_Class_ID,
																true, PARTICLECHANNELENDTIMER_INTERFACE,
																PARTICLECHANNELENDTIMEW_INTERFACE, true,
																actionNode, (Object*)this, &initEndTime);
	if (chEndTimeW == NULL) return false; // can't modify the end time

	// create channel to store info about the last time for each particle for the proceed function
	// the data is used to rollback the effect of integration to find the desirable orientation
	IParticleChannelPTVW* chProceedTimeW = NULL;
	chProceedTimeW = (IParticleChannelPTVW*)chCont->EnsureInterface(PARTICLECHANNELPROCEEDTIMEW_INTERFACE,
																ParticleChannelPTV_Class_ID,
																true, PARTICLECHANNELPROCEEDTIMER_INTERFACE,
																PARTICLECHANNELPROCEEDTIMEW_INTERFACE, false,
																actionNode, (Object*)this);
	if (chProceedTimeW == NULL) return false; // can't modify the proceed time
	for(i=0; i<count; i++) chProceedTimeW->SetValue(i, chTime->GetValue(i));

	// create channel to store info if the final rotation has been initialized
	IParticleChannelBoolW* chGotInitW = NULL;
	bool initGotInit = false;
	chGotInitW = (IParticleChannelBoolW*)chCont->EnsureInterface(PARTICLECHANNELGOTINITW_INTERFACE,
															ParticleChannelBool_Class_ID,
															true, PARTICLECHANNELGOTINITR_INTERFACE,
															PARTICLECHANNELGOTINITW_INTERFACE, true,
															actionNode, (Object*)this, &initGotInit);
	if (chGotInitW == NULL) return false; // can't modify if init data

	// create channel to store initial rotation
	IParticleChannelQuatW* chStartRotW = NULL;
	bool initStartRot = false;
	chStartRotW = (IParticleChannelQuatW*)chCont->EnsureInterface(PARTICLECHANNELSTARTROTW_INTERFACE,
																ParticleChannelQuat_Class_ID,
																true, PARTICLECHANNELSTARTROTR_INTERFACE,
																PARTICLECHANNELSTARTROTW_INTERFACE, true,
																actionNode, (Object*)this, &initStartRot);
	if (chStartRotW == NULL) return false; // can't modify the start rotation

	// create channel to store end rotation
	IParticleChannelQuatW* chEndRotW = NULL;
	bool initEndRot = false;
	chEndRotW = (IParticleChannelQuatW*)chCont->EnsureInterface(PARTICLECHANNELENDROTW_INTERFACE,
																ParticleChannelQuat_Class_ID,
																true, PARTICLECHANNELENDROTR_INTERFACE,
																PARTICLECHANNELENDROTW_INTERFACE, true,
																actionNode, (Object*)this, &initEndRot);
	if (chEndRotW == NULL) return false; // can't modify the end rotation

	// create channel to store initial spin
	IParticleChannelAngAxisW* chStartSpinW = NULL;
	bool initStartSpin = false;
	chStartSpinW = (IParticleChannelAngAxisW*)chCont->EnsureInterface(PARTICLECHANNELSTARTSPINW_INTERFACE,
																ParticleChannelAngAxis_Class_ID,
																true, PARTICLECHANNELSTARTSPINR_INTERFACE,
																PARTICLECHANNELSTARTSPINW_INTERFACE, true,
																actionNode, (Object*)this, &initStartSpin);
	if (chStartSpinW == NULL) return false; // can't modify the start rotation
	
	// create channel to store final spin rate as a float
	IParticleChannelFloatW* chEndSpinW = NULL;
	bool initEndSpin = false;
	chEndSpinW = (IParticleChannelFloatW*)chCont->EnsureInterface(PARTICLECHANNELENDSPINW_INTERFACE,
																ParticleChannelFloat_Class_ID,
																true, PARTICLECHANNELENDSPINR_INTERFACE,
																PARTICLECHANNELENDSPINW_INTERFACE, true,
																actionNode, (Object*)this, &initEndSpin);
	if (chEndSpinW == NULL) return false; // can't modify the start rotation

	int sync = pblock()->GetInt(kGoToRotation_syncBy, timeEnd);
	TimeValue time = pblock()->GetTimeValue(kGoToRotation_time, timeEnd);
	TimeValue timeVar = pblock()->GetTimeValue(kGoToRotation_variation, timeEnd);
	int matchSpin = pblock()->GetInt(kGoToRotation_matchSpin, timeEnd);
	float spin = GetPFFloat(pblock(), kGoToRotation_spin, timeEnd.TimeValue())/TIME_TICKSPERSEC;
	float spinVar = GetPFFloat(pblock(), kGoToRotation_spinVariation, timeEnd.TimeValue())/TIME_TICKSPERSEC;

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	if (randGen == NULL) return false;

	if (!chNew->IsAllOld()) {
		for(i=0; i<count; i++) {
			if (!chNew->IsNew(i)) continue;
			if (initStartTime) 
				chStartTimeW->SetValue(i, chTime->GetValue(i) );
			if (initEndTime) {
				PreciseTimeValue endTime(time);
				switch(sync) {
				case kGoToRotation_syncBy_age:
					endTime += chBirth->GetValue(i);
					break;
				case kGoToRotation_syncBy_event:
					endTime += chTime->GetValue(i);
					break;
				}
				if (timeVar > 0) {
					int sign = randGen->RandSign();
					endTime += PreciseTimeValue(sign*randGen->Rand0X(timeVar));
				} else {
					randGen->RandSign();
					randGen->Rand0X(10);
				}
				chEndTimeW->SetValue(i, endTime);
			}
			if (initGotInit)
				chGotInitW->SetValue(i, false);
			if (initStartRot)
				chStartRotW->SetValue(i, chOrient->GetValue(i));
			if (initEndRot)
				chEndRotW->SetValue(i, chOrient->GetValue(i));
			if (initStartSpin)
				chStartSpinW->SetValue(i, chSpinR->GetValue(i));
			if (initEndSpin) {
				float endSpin = 0;
				if (matchSpin) {
					AngAxis aa = chSpinR->GetValue(i);
					endSpin = aa.angle;
				} else endSpin = spin;
				if (spinVar > 0.0f) endSpin += spinVar*randGen->Rand11();
				else randGen->Rand11();
				chEndSpinW->SetValue(i, endSpin);
			}
		}
	}

	return true;
}

void InterpolateRotation(Quat startRot, Quat endRot, AngAxis startSpin, float endSpin,
						 PreciseTimeValue startT, PreciseTimeValue endT, PreciseTimeValue curT,
						 float easyIn, bool targetConstant, Quat& resRot, AngAxis& resSpin)
{
	float timeDif = float(endT - startT);
	AngAxis totalSpin = startSpin;
	totalSpin.angle *= timeDif;
	Quat rot0 = startRot + Quat(totalSpin);
	AngAxis difSpin;
	difSpin.angle = QangAxis(rot0, endRot, difSpin.axis);
	AngAxis reverseSpin = difSpin;
	reverseSpin.angle = endSpin;
	AngAxis totalDifSpin = difSpin;
	totalDifSpin.angle = -endSpin*timeDif;
	Quat rot1 = endRot + Quat(totalDifSpin);
	AngAxis partialSpin = startSpin;
	partialSpin.angle *= float(curT - startT);
	Quat rot00 = startRot + Quat(partialSpin);
	AngAxis partialRevSpin = reverseSpin;
	partialRevSpin.angle *= float(curT - endT);
	Quat rot11 = endRot + Quat(partialRevSpin);
	if (!targetConstant)
		rot11.MakeClosest(rot00);
	float timeRatio = float(endT - curT)/timeDif;
	float adjRatio = timeRatio*(timeRatio*easyIn + 1.0f - easyIn);
	resRot = Slerp(rot11, rot00, adjRatio);

	curT.tick -= 1;
	partialSpin = startSpin;
	partialSpin.angle *= float(curT - startT);
	Quat rot20 = startRot + Quat(partialSpin);
	partialRevSpin = reverseSpin;
	partialRevSpin.angle *= float(curT - endT);
	Quat rot21 = endRot + Quat(partialRevSpin);
	if (!targetConstant)
		rot21.MakeClosest(rot20);
	timeRatio = float(endT - curT)/timeDif;
	float adjRatio2 = timeRatio*(timeRatio*easyIn + 1.0f - easyIn);
	Quat prevRot = Slerp(rot21, rot20, adjRatio2);
	resSpin.angle = QangAxis(prevRot, resRot, resSpin.axis);
	if (!targetConstant)
		resSpin.angle = timeRatio*startSpin.angle + (1.0f-timeRatio)*endSpin;
}

bool PFTestGoToRotation::doPostProceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (pblock() == NULL) return false;
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int i, count = chAmount->Count();
	if (count == 0) return true; // no particles to modify
	IParticleChannelQuatR* chOrientR = GetParticleChannelOrientationRInterface(pCont);
	if (chOrientR == NULL) return false; // can't read current orientation for a particle
	IParticleChannelQuatW* chOrientW = GetParticleChannelOrientationWInterface(pCont);
	if (chOrientW == NULL) return false; // can't modify current orientation for a particle
	IParticleChannelAngAxisR* chSpinR = GetParticleChannelSpinRInterface(pCont);
	if (chSpinR == NULL) return false; // can't read current spin for a particle
	IParticleChannelAngAxisW* chSpinW = GetParticleChannelSpinWInterface(pCont);
	if (chSpinW == NULL) return false; // can't modify current spin for a particle

	// acquire private channels
	IParticleChannelPTVR* chStartTime = (IParticleChannelPTVR*)(chCont->GetPrivateInterface(PARTICLECHANNELSTARTTIMER_INTERFACE, (Object*)this));
	if (chStartTime == NULL) return false;
	IParticleChannelPTVR* chEndTime = (IParticleChannelPTVR*)(chCont->GetPrivateInterface(PARTICLECHANNELENDTIMER_INTERFACE, (Object*)this));
	if (chEndTime == NULL) return false;
	IParticleChannelPTVR* chProceedTime = (IParticleChannelPTVR*)(chCont->GetPrivateInterface(PARTICLECHANNELPROCEEDTIMER_INTERFACE, (Object*)this));
	if (chProceedTime == NULL) return false;
		
	int targetType = pblock()->GetInt(kGoToRotation_targetType, timeEnd);
	bool initOnce = (targetType == kGoToRotation_targetType_constant);
	IParticleChannelBoolR* chGotInitR = (IParticleChannelBoolR*)(chCont->GetPrivateInterface(PARTICLECHANNELGOTINITR_INTERFACE, (Object*)this));
	IParticleChannelBoolW* chGotInitW = (IParticleChannelBoolW*)(chCont->GetPrivateInterface(PARTICLECHANNELGOTINITW_INTERFACE, (Object*)this));
	if ((chGotInitR == NULL) || (chGotInitW == NULL)) return false;

	IParticleChannelQuatR* chStartRot = (IParticleChannelQuatR*)(chCont->GetPrivateInterface(PARTICLECHANNELSTARTROTR_INTERFACE, (Object*)this));
	if (chStartRot == NULL) return false;
	IParticleChannelQuatR* chEndRotR = (IParticleChannelQuatR*)(chCont->GetPrivateInterface(PARTICLECHANNELENDROTR_INTERFACE, (Object*)this));
	if (chEndRotR == NULL) return false;
	IParticleChannelQuatW* chEndRotW = (IParticleChannelQuatW*)(chCont->GetPrivateInterface(PARTICLECHANNELENDROTW_INTERFACE, (Object*)this));
	if (chEndRotW == NULL) return false;
	
	IParticleChannelAngAxisR* chStartSpin = (IParticleChannelAngAxisR*)(chCont->GetPrivateInterface(PARTICLECHANNELSTARTSPINR_INTERFACE, (Object*)this));
	if (chStartSpin == NULL) return false;
	IParticleChannelFloatR* chEndSpin = (IParticleChannelFloatR*)(chCont->GetPrivateInterface(PARTICLECHANNELENDSPINR_INTERFACE, (Object*)this));
	if (chEndSpin == NULL) return false;
	
	float easyIn = GetPFFloat(pblock(), kGoToRotation_easeIn, timeEnd.TimeValue() );

	// particle properties modification
	for(i=0; i<count; i++) {
		// if the particle out of the transition period then do nothing
		PreciseTimeValue startT = chStartTime->GetValue(i);
		PreciseTimeValue endT = chEndTime->GetValue(i);
		if (endT < timeEnd) continue;
		if (endT <= startT) continue;

		// rollback the current rotation: remove the effect of the integration to know the real orientation value
		// need that if not initialized, or the target rotation is changing
		Quat curOrient = chOrientR->GetValue(i);
		bool needRollback = true;
		if (initOnce)
			if (chGotInitR->GetValue(i)) needRollback = false;
		if (needRollback) {
			AngAxis curSpin = chSpinR->GetValue(i);
			float timeDif = float(timeEnd - chProceedTime->GetValue(i));
			curSpin.angle *= -timeDif;
			curOrient += Quat(curSpin);
		}
		if (initOnce) {
			if (!chGotInitR->GetValue(i)) {
				chEndRotW->SetValue(i, curOrient);
			}
		} else {
			chEndRotW->SetValue(i, curOrient);
		}
		chGotInitW->SetValue(i, true);
	
		Quat resRot;
		AngAxis resSpin;
		InterpolateRotation(chStartRot->GetValue(i), chEndRotR->GetValue(i), chStartSpin->GetValue(i),
							chEndSpin->GetValue(i), startT, endT, timeEnd,
							easyIn, initOnce, resRot, resSpin);
		chOrientW->SetValue(i, resRot);
		chSpinW->SetValue(i, resSpin);
	}

	return true;
}
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestGoToRotation::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{

	if (pblock() == NULL) return false;
	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int i, count = chAmount->Count();
	if (count == 0) return true; // no particles to test
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing info for a particle
	IParticleChannelQuatW* chOrientW = GetParticleChannelOrientationWInterface(pCont);
	if (chOrientW == NULL) return false; // can't modify current orientation for a particle
	IParticleChannelAngAxisR* chSpinR = GetParticleChannelSpinRInterface(pCont);
	if (chSpinR == NULL) return false; // can't read current spin for a particle
	IParticleChannelAngAxisW* chSpinW = GetParticleChannelSpinWInterface(pCont);
	if (chSpinW == NULL) return false; // can't modify current spin for a particle

	// acquire private channels
	IParticleChannelPTVR* chEndTime = (IParticleChannelPTVR*)(chCont->GetPrivateInterface(PARTICLECHANNELENDTIMER_INTERFACE, (Object*)this));
	if (chEndTime == NULL) return false;
	IParticleChannelBoolR* chGotInitR = (IParticleChannelBoolR*)(chCont->GetPrivateInterface(PARTICLECHANNELGOTINITR_INTERFACE, (Object*)this));
	if (chGotInitR == NULL) return false;
	IParticleChannelQuatR* chEndRotR = (IParticleChannelQuatR*)(chCont->GetPrivateInterface(PARTICLECHANNELENDROTR_INTERFACE, (Object*)this));
	if (chEndRotR == NULL) return false;
	IParticleChannelFloatR* chEndSpin = (IParticleChannelFloatR*)(chCont->GetPrivateInterface(PARTICLECHANNELENDSPINR_INTERFACE, (Object*)this));
	if (chEndSpin == NULL) return false;

	bool sendOut = (pblock()->GetInt(kGoToRotation_sendOut, timeEnd) != 0);
	bool stopSpin = (pblock()->GetInt(kGoToRotation_stopSpin, timeEnd) != 0);

	// test all particles
	PreciseTimeValue curTime;
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	BitArray particlesToAdvance;
	particlesToAdvance.SetSize(count);
	particlesToAdvance.ClearAll();
	for(i=0; i<count; i++)
	{
		curTime = chEndTime->GetValue(i);
		if (curTime > timeEnd) continue;
		if (curTime < timeStart) {
			testTime[i] = float(chTime->GetValue(i) - timeStart);
		} else {
			testTime[i] = float(curTime - timeStart);
			particlesToAdvance.Set(i);
		}
		testResult.Set(i);
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

	for(i=0; i<count; i++) {
		if (particlesToAdvance[i] == 0) continue;

		Quat curRot = chEndRotR->GetValue(i);

		chOrientW->SetValue(i, chEndRotR->GetValue(i));
		AngAxis spin = chSpinR->GetValue(i);
		spin.angle = stopSpin ? 0.0f : chEndSpin->GetValue(i);
		chSpinW->SetValue(i, spin);
	}

	if (!sendOut) testResult.ClearAll();

	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFTest									 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::ProceedStep1(IObject* pCont, 
								   Object* pSystem, 
								   INode* pNode, 
								   INode* actionNode, 
								   FPInterface* integrator)
{
	_containerFnPub() = pCont;
	_particleSystemFnPub() = pSystem;
	_particleNodeFnPub() = pNode;
	_actionNodeFnPub() = actionNode;
	_integratorFnPub() = integrator;
}

//+--------------------------------------------------------------------------+
//|							From IPFTest									 |
//+--------------------------------------------------------------------------+
bool PFTestGoToRotation::ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime)
{
	PreciseTimeValue timeStart = PreciseTimeValue(timeStartTick, timeStartFraction);
	PreciseTimeValue timeEnd = PreciseTimeValue(timeEndTick, timeEndFraction);
	bool res = Proceed(	_containerFnPub(),
						timeStart,
						timeEnd,
						_particleSystemFnPub(),
						_particleNodeFnPub(),
						_actionNodeFnPub(),
						(IPFIntegrator*)_integratorFnPub(),
						testResult,
						testTime);
	timeEndTick = timeEnd.tick;
	timeEndFraction = timeEnd.fraction;
	return res;
}



ClassDesc* PFTestGoToRotation::GetClassDesc() const
{
	return GetPFTestGoToRotationDesc();
}

//+--------------------------------------------------------------------------+
//|							From PFTestGoToRotation					 |
//+--------------------------------------------------------------------------+
void PFTestGoToRotation::RefreshUI(WPARAM message, IParamMap2* map) const
{
	HWND hWnd;
	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
		}
		return;
	}

	if ( pblock() == NULL ) return;

	map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
		}
	}
	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		SendMessage( hWnd, WM_COMMAND, message, (LPARAM)0);
	}
}


void PFTestGoToRotation::UpdatePViewUI(int updateID) const
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		map->Invalidate(updateID);
		Interval currentTimeInterval;
		currentTimeInterval.SetInstant(GetCOREInterface()->GetTime());
		map->Validity() = currentTimeInterval;
	}
}


} // end of namespace PFActions