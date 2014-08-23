/**********************************************************************
 *<
	FILE: PFTestSpawn.h

	DESCRIPTION: Duration Test implementation
				 The tests generates new particles from the existing
				 ones and then sends new particles (and may also the
				 old ones) to the next action list

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

#include "PFTestSpawn.h"

#include "PFTestSpawn_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IParticleObjectExt.h"
#include "IPViewManager.h"
#include "PFSimpleActionState.h"

namespace PFActions {

// Spawn Test creates a particle channel to store if a particle is able to spawn
// The value is calculated only once when a particle enters the event
#define PARTICLECHANNELSPAWNABLER_INTERFACE Interface_ID(0x1bc01a26, 0x1eb34500) 
#define PARTICLECHANNELSPAWNABLEW_INTERFACE Interface_ID(0x1bc01a26, 0x1eb34501) 
#define GetParticleChannelSpawnAbleRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELSPAWNABLER_INTERFACE)) 
#define GetParticleChannelSpawnAbleWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELSPAWNABLEW_INTERFACE)) 

// Spawn Test creates a particle channel to distinguish parents and children
#define PARTICLECHANNELISSPAWNR_INTERFACE Interface_ID(0x1bc01a25, 0x1eb34500) 
#define PARTICLECHANNELISSPAWNW_INTERFACE Interface_ID(0x1bc01a25, 0x1eb34501) 
#define GetParticleChannelIsSpawnRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELISSPAWNR_INTERFACE)) 
#define GetParticleChannelIsSpawnWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELISSPAWNW_INTERFACE)) 

// Spawn Test creates a particle channel to store data that are used to calculate next time for spawn
#define PARTICLECHANNELCALCLASTTIMER_INTERFACE Interface_ID(0x1bc01a27, 0x1eb34500) 
#define PARTICLECHANNELCALCLASTTIMEW_INTERFACE Interface_ID(0x1bc01a27, 0x1eb34501) 
#define GetParticleChannelCalcLastTimeRInterface(obj) ((IParticleChannelPTVR*)obj->GetInterface(PARTICLECHANNELCALCLASTTIMER_INTERFACE)) 
#define GetParticleChannelCalcLastTimeWInterface(obj) ((IParticleChannelPTVW*)obj->GetInterface(PARTICLECHANNELCALCLASTTIMEW_INTERFACE)) 
#define PARTICLECHANNELCALCACCUMDEFR_INTERFACE Interface_ID(0x1bc01a28, 0x1eb34500) 
#define PARTICLECHANNELCALCACCUMDEFW_INTERFACE Interface_ID(0x1bc01a28, 0x1eb34501) 
#define GetParticleChannelCalcAccumDefRInterface(obj) ((IParticleChannelFloatR*)obj->GetInterface(PARTICLECHANNELCALCACCUMDEFR_INTERFACE)) 
#define GetParticleChannelCalcAccumDefWInterface(obj) ((IParticleChannelFloatW*)obj->GetInterface(PARTICLECHANNELCALCACCUMDEFW_INTERFACE)) 


// static members
Object*		PFTestSpawn::m_editOb				= NULL;
IObjParam*	PFTestSpawn::m_ip					= NULL;

// constructors/destructors
PFTestSpawn::PFTestSpawn()
				:IPFTest()
{ 
	RegisterParticleFlowNotification();
	_pblock() = NULL;
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _trueIcon() = _falseIcon() = NULL;
}

PFTestSpawn::~PFTestSpawn()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFTestSpawn::GetInterface(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return (IPFAction*)this;
	if (id == PFTEST_INTERFACE) return (IPFTest*)this;
	if (id == PVIEWITEM_INTERFACE) return (IPViewItem*)this;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestSpawn::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &spawn_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &spawn_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &spawn_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestSpawn::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SPAWN_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSpawn::ClassID()
{
	return PFTestSpawn_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFTestSpawn::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFTestSpawn::SubAnimName(int i)
{
	return PFActions::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFTestSpawn::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestSpawn::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestSpawn::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFTestSpawn::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PFTestSpawn::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSpawn_speedType:
					RefreshUI(kSpawn_message_speedType);
					break;
				case kSpawn_spawnType:
					RefreshUI(kSpawn_message_spawnType);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				case kSpawn_numVariation:
				case kSpawn_speedVariation:
				case kSpawn_speedDivergence:
					RefreshUI(kSpawn_message_useRandom);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kSpawn_RefMsg_InitDlg:
			RefreshUI(kSpawn_message_speedType, (IParamMap2*)partID);
			RefreshUI(kSpawn_message_spawnType, (IParamMap2*)partID);
			RefreshUI(kSpawn_message_useRandom, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSpawn_RefMsg_NewRand:
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
RefTargetHandle PFTestSpawn::Clone(RemapDir &remap)
{
	PFTestSpawn* newTest = new PFTestSpawn();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PFTestSpawn::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestSpawn::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestSpawn::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestSpawn::GetObjectName()
{
	return GetString(IDS_TEST_SPAWN_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSpawn::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	return _randLinker().Init( pCont, GetRand() );
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestSpawn::Release(IObject* pCont)
{
	_randLinker().Release( pCont );
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestSpawn::GetRand()
{
	return pblock()->GetInt(kSpawn_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::SetRand(int seed)
{
	pblock()->SetValue(kSpawn_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFTestSpawn::GetCurrentState(IObject* pContainer)
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
void PFTestSpawn::SetCurrentState(IObject* aSt, IObject* pContainer)
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
const ParticleChannelMask& PFTestSpawn::ChannelsUsed(const Interval& time) const
{
									// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_ID|PCU_Time|PCU_BirthTime
									|PCU_EventStart|PCU_Position|PCU_Speed|PCU_Scale
									|PCU_Orientation|PCU_Spin, 
									// write channels
									PCU_Amount|PCU_New|PCU_ID|PCU_Time|PCU_BirthTime
									|PCU_EventStart|PCU_Position|PCU_Speed|PCU_Scale
									|PCU_Orientation|PCU_Spin);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		// able to spawn channel
		mask.AddChannel(PARTICLECHANNELSPAWNABLER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELSPAWNABLEW_INTERFACE); // write channel
		// parent or spawn child
		mask.AddChannel(PARTICLECHANNELISSPAWNR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELISSPAWNW_INTERFACE); // write channel
		// time of the last measurement of accumulated defficiency
		mask.AddChannel(PARTICLECHANNELCALCLASTTIMER_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELCALCLASTTIMEW_INTERFACE); // write channel
		// accumulated deficiency: range: from 0.0 to 1.0
		// when value reaches 1.0 it means that it's time to spawn particles
		mask.AddChannel(PARTICLECHANNELCALCACCUMDEFR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELCALCACCUMDEFW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpawn::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPAWN_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpawn::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPAWN_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpawn::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPAWN_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestSpawn::HasDynamicName(TSTR& nameSuffix)
{
	int type = pblock()->GetInt(kSpawn_spawnType, 0);
	switch(type) {
	case kSpawn_spawnType_once:
		nameSuffix = GetString(IDS_ONCE);
		break;
	case kSpawn_spawnType_perFrame:
		nameSuffix = GetString(IDS_BYRATE);
		break;
	case kSpawn_spawnType_byDistance:
		nameSuffix = GetString(IDS_BYTRAVEL);
		break;
	}
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSpawn::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	// check if the test is allowed to produce more particles
	IPFSystem* iSystem = GetPFSystemInterface(pSystem);
	if (iSystem == NULL) return false; // given particle system doesn't have PFSystem interface
	int numLimit = iSystem->GetBornAllowance();
	IParticleObjectExt* iPObj = GetParticleObjectExtInterface(GetPFObject(pSystem));
	if (iPObj == NULL) return false;
	int numTotal = iPObj->NumParticles();
	if (numTotal >= numLimit) return false; // particle amount limit was reached

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmountR = GetParticleChannelAmountRInterface(pCont);
	if (chAmountR == NULL) return false; // can't find number of particles in the container
	IParticleChannelAmountW* chAmountW = GetParticleChannelAmountWInterface(pCont);
	if (chAmountW == NULL) return false; // can't modify number of particles in the container
	IParticleChannelPTVR* chTimeR = GetParticleChannelTimeRInterface(pCont);
	if (chTimeR == NULL) return false; // can't read timing for a particle
	IParticleChannelPTVW* chTimeW = GetParticleChannelTimeWInterface(pCont);
	if (chTimeW == NULL) return false; // can't modify timing for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	IParticleChannelNewW* chNewW = GetParticleChannelNewWInterface(pCont);
	if (chNewW == NULL) return false; // can't modify 'new' status of the spawned particles
	IParticleChannelIDR* chIDR = GetParticleChannelIDRInterface(pCont);
	if (chIDR == NULL) return false; // can't read IDs 
	IParticleChannelIDW* chIDW = GetParticleChannelIDWInterface(pCont);
	if (chIDW == NULL) return false; // can't set IDs for new particles

	// position, orientation and spin channels may not be present
	IParticleChannelPoint3R* chPosR = GetParticleChannelPositionRInterface(pCont);
	IParticleChannelPoint3W* chPosW = GetParticleChannelPositionWInterface(pCont);
	IParticleChannelQuatR* chOrientR = GetParticleChannelOrientationRInterface(pCont);
	IParticleChannelQuatW* chOrientW = GetParticleChannelOrientationWInterface(pCont);
	IParticleChannelAngAxisR* chSpinR = GetParticleChannelSpinRInterface(pCont);
	IParticleChannelAngAxisW* chSpinW = GetParticleChannelSpinWInterface(pCont);
	// if the read is present then the write should be present; otherwise it is unexpected situation
	if (chPosR != NULL) if (chPosW == NULL) return false;
	if (chOrientR != NULL) if (chOrientW == NULL) return false;
	if (chSpinR != NULL) if (chSpinW == NULL) return false;

	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// spawn test requires presence of Speed and Scale channels
	bool initSpeed = false;
	IParticleChannelPoint3R* chSpeedR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSPEEDR_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELSPEEDR_INTERFACE,
																	PARTICLECHANNELSPEEDW_INTERFACE, true,
																	actionNode, NULL, &initSpeed);
	if (chSpeedR == NULL) return false; // can't read/create speed channel
	IParticleChannelPoint3W* chSpeedW = GetParticleChannelSpeedWInterface(pCont);
	if (chSpeedW == NULL) return false; // can't modify speed channel

	bool initScale = false;
	IParticleChannelPoint3R* chScaleR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSCALER_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELSCALER_INTERFACE,
																	PARTICLECHANNELSCALEW_INTERFACE, true, 
																	actionNode, NULL, &initScale);
	if (chScaleR == NULL) return false; // can't read/create scale channel
	IParticleChannelPoint3W* chScaleW = GetParticleChannelScaleWInterface(pCont);
	if (chScaleW == NULL) return false; // can't modify scale channel
	IParticleChannelBoolW* chSelect = NULL;
	if (iSystem->NumParticlesSelected() > 0)
		chSelect = (IParticleChannelBoolW*)chCont->EnsureInterface(PARTICLECHANNELSELECTIONW_INTERFACE,
																	ParticleChannelBool_Class_ID,
																	true, PARTICLECHANNELSELECTIONR_INTERFACE,
																	PARTICLECHANNELSELECTIONW_INTERFACE);

	int i, oldCount = chAmountR->Count(); // number of particles in the container before spawn
	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);

	int spawnType = pblock()->GetInt(kSpawn_spawnType, 0);
	int syncType = pblock()->GetInt(kSpawn_syncBy, 0);
	int speedType = pblock()->GetInt(kSpawn_speedType, 0);

	// define if the test needs EventStart & BirthTime channels
	int restartAge = pblock()->GetInt(kSpawn_restartAge, 0);
	int paramAnim = IsControlAnimated(pblock()->GetController(kSpawn_numOffsprings));
	paramAnim = paramAnim || IsControlAnimated(pblock()->GetController(kSpawn_numVariation));
	int paramFrameAnim = IsControlAnimated(pblock()->GetController(kSpawn_spawnRate));
	int paramDistAnim = IsControlAnimated(pblock()->GetController(kSpawn_stepSize));
	if (spawnType == kSpawn_spawnType_perFrame)
		paramAnim = paramAnim || paramFrameAnim;
	if (spawnType == kSpawn_spawnType_byDistance)
		paramAnim = paramAnim || paramDistAnim;

	// acquire BirthTime channel
	IParticleChannelPTVR* chBirthTimeR = GetParticleChannelBirthTimeRInterface(pCont);
	if (chBirthTimeR == NULL) return false; // can't read particle age
	IParticleChannelPTVW* chBirthTimeW = GetParticleChannelBirthTimeWInterface(pCont);
	if (chBirthTimeW == NULL) return false; // can't modify particle age for new spawn particles

	// acquire EventStart channel
	bool initEventStart = false;
	IParticleChannelPTVR* chEventStartR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
													ParticleChannelPTV_Class_ID,
													true,	PARTICLECHANNELEVENTSTARTR_INTERFACE,
															PARTICLECHANNELEVENTSTARTW_INTERFACE,
													false,  // no transfer & public
													actionNode, NULL, &initEventStart);
	if (chEventStartR == NULL) return false;
	IParticleChannelPTVW* chEventStartW = GetParticleChannelEventStartWInterface(pCont);
	if (chEventStartW == NULL) return false;

	// acquire SpawnAble particle channel; if not present then create it		
	IParticleChannelBoolW* chAbleW = NULL;
	// first try to get the private channel.
	// If it's not possible then create a new one
	bool initSpawnAble = false;
	IParticleChannelBoolR* chAbleR = (IParticleChannelBoolR*)chCont->EnsureInterface(PARTICLECHANNELSPAWNABLER_INTERFACE,
													ParticleChannelBool_Class_ID,
													true,	PARTICLECHANNELSPAWNABLER_INTERFACE,
															PARTICLECHANNELSPAWNABLEW_INTERFACE,
													true, actionNode, (Object*)this, // transfer & private
													&initSpawnAble);
	chAbleW = (IParticleChannelBoolW*)chCont->GetPrivateInterface(PARTICLECHANNELSPAWNABLEW_INTERFACE, (Object*)this);
	if ((chAbleR == NULL) || (chAbleW == NULL)) return false; // can't find/create SpawnAble channel

	// acquire IsShawn particle channel; if not present then create it		
	IParticleChannelBoolW* chIsSpawnW = NULL;
	// first try to get the private channel.
	// If it's not possible then create a new one
	bool initIsSpawn = false;
	IParticleChannelBoolR* chIsSpawnR = (IParticleChannelBoolR*)chCont->EnsureInterface(PARTICLECHANNELISSPAWNR_INTERFACE,
													ParticleChannelBool_Class_ID,
													true,	PARTICLECHANNELISSPAWNR_INTERFACE,
															PARTICLECHANNELISSPAWNW_INTERFACE,
													false, actionNode, (Object*)this, // non-transferable & private
													&initIsSpawn);
	chIsSpawnW = (IParticleChannelBoolW*)chCont->GetPrivateInterface(PARTICLECHANNELISSPAWNW_INTERFACE, (Object*)this);
	if ((chIsSpawnR == NULL) || (chIsSpawnW == NULL)) return false; // can't find/create IsSpawn channel

	// define if the test needs CalcLastTime & CalcAccumDef channels
	bool initLastTime = false;
	bool initAccumDef = false;
	IParticleChannelPTVR* chLastTimeR = NULL;
	IParticleChannelPTVW* chLastTimeW = NULL;
	IParticleChannelFloatR* chAccumDefR = NULL;
	IParticleChannelFloatW* chAccumDefW = NULL;
	if ((spawnType == kSpawn_spawnType_perFrame) || (spawnType == kSpawn_spawnType_byDistance))
	{ 
		chLastTimeR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELCALCLASTTIMER_INTERFACE,
																ParticleChannelPTV_Class_ID,
																true,	PARTICLECHANNELCALCLASTTIMER_INTERFACE,
																		PARTICLECHANNELCALCLASTTIMEW_INTERFACE,
																true, actionNode, (Object*)this, &initLastTime ); // transfer & private
		chAccumDefR = (IParticleChannelFloatR*)chCont->EnsureInterface(PARTICLECHANNELCALCACCUMDEFR_INTERFACE,
																ParticleChannelFloat_Class_ID,
																true,	PARTICLECHANNELCALCACCUMDEFR_INTERFACE,
																		PARTICLECHANNELCALCACCUMDEFW_INTERFACE,
																true, actionNode, (Object*)this, &initAccumDef ); // transfer & private
		if ((chLastTimeR == NULL) || (chAccumDefR == NULL)) return false; // can't find/create CalcLastTime channel

		chLastTimeW = (IParticleChannelPTVW*)chCont->GetPrivateInterface(PARTICLECHANNELCALCLASTTIMEW_INTERFACE, (Object*)this);
		chAccumDefW = (IParticleChannelFloatW*)chCont->GetPrivateInterface(PARTICLECHANNELCALCACCUMDEFW_INTERFACE, (Object*)this);
		if ((chLastTimeW == NULL) || (chAccumDefW == NULL)) return false; // can't find CalcLastTimeW channel
	}

	// define speed for each new particle if the Speed channel was created
	// define scale for each new particle if the Scale channel was created
	// define spawn ability for each new particle if the AbleSpawn channel was created
	// setup EventStart value for each new particle if the EventStart channel was created
	// init calcLastTime value for each new particle if the channel was created
	// init calcAccumDef value for each new particle if the channel was created
	int isSpawnAbleAnimated = IsControlAnimated(pblock()->GetController(kSpawn_spawnAble));
	if ((!chNew->IsAllOld()) 
			&& (initSpeed || initScale || initSpawnAble || initEventStart || initLastTime || initAccumDef || initIsSpawn))
	{
		float spawnRatio;
		if (!isSpawnAbleAnimated)
			spawnRatio = GetPFFloat(pblock(), kSpawn_spawnAble, 0);
		for(i=0; i<oldCount; i++)
			if (chNew->IsNew(i)) {
				if (initSpeed)
					chSpeedW->SetValue(i, Point3::Origin);
				if (initScale)
					chScaleW->SetValue(i, Point3(1.0f, 1.0f, 1.0f));
				if (initEventStart)
					chEventStartW->SetValue(i, chTimeR->GetValue(i));
				if (initSpawnAble)
				{
					if (isSpawnAbleAnimated)
						spawnRatio = GetPFFloat(pblock(), kSpawn_spawnAble, chTimeR->GetTick(i));
					if (spawnRatio <= 0.0f)
						chAbleW->SetValue(i,false);
					else if (spawnRatio >= 1.0f)
						chAbleW->SetValue(i,true);
					else
						chAbleW->SetValue(i, (randGen->Rand01() < spawnRatio) );
				}
				if (initIsSpawn)
					chIsSpawnW->SetValue(i, false);
				if (initLastTime)
					chLastTimeW->SetValue(i, chTimeR->GetValue(i));
				if (initAccumDef)
					chAccumDefW->SetValue(i, 0.0f);
			}
	}

	// create storage to keep data for new spawn particles
	Tab<int> spawnFactor;
	Tab<TabSpawnParticleData*> spawnData; 
	spawnData.SetCount(oldCount);
	spawnFactor.SetCount(oldCount);
	for(i=0; i<oldCount; i++) {
		if (chAbleR->GetValue(i)) spawnData[i] = new TabSpawnParticleData();
		else spawnData[i] = NULL;
		spawnFactor[i] = 1;
	}
		

	TimeValue paramTime = 0;
	TimeValue syncParamTime = 0;
	bool firstParticleToSpawn = true;
	float spawnRate = 1.0f/TIME_TICKSPERSEC;
	if (!paramFrameAnim)
		spawnRate = getSpawnRate(0);
	float spawnStepSize = 1.0f;
	if (!paramDistAnim)
		spawnStepSize = getStepSize(0);
	int numChildren = GetPFInt(pblock(), kSpawn_numOffsprings, 0);
	float varChildren = GetPFFloat(pblock(), kSpawn_numVariation, 0);

	for(i=0; i<oldCount; i++) 
	{ // proceed particles one-by-one
		if (numTotal >= numLimit) break; // particle amount limit was reached
		if (spawnData[i] == NULL) continue; // no spawn

		// calculate next spawn time
		PreciseTimeValue spawnTime;
		switch (spawnType) {
		case kSpawn_spawnType_once:
			// only new particles spawn
			// spawn right now, the current particle time
			spawnTime = (chNew->IsNew(i)) ? chTimeR->GetValue(i) : TIME_PosInfinity;
			break;
		case kSpawn_spawnType_perFrame: 
			spawnTime = calculateNextSpawnTimeForFrames(i, syncType, timeEnd, paramFrameAnim, spawnRate,
												chLastTimeR, chLastTimeW,
												chAccumDefR, chAccumDefW,
												chBirthTimeR, chEventStartR);
			break;
		case kSpawn_spawnType_byDistance:
			spawnTime = calculateNextSpawnTimeForDistance(i, syncType, timeEnd, paramDistAnim, spawnStepSize,
												chSpeedR,
												chLastTimeR, chLastTimeW,
												chAccumDefR, chAccumDefW,
												chBirthTimeR, chEventStartR);
			break;
		}

		if (spawnTime > timeEnd) continue; // not ready to spawn yet

		while (spawnTime <= timeEnd) {

			if (spawnType != kSpawn_spawnType_once) {
				// advance particle to its spawn time
				if (firstParticleToSpawn) {
					integrator->Proceed(pCont, spawnTime, i); // to initialize channels in the integrator
					firstParticleToSpawn = false;
				} else {
					integrator->Proceed(NULL, spawnTime, i); // without the initialization
				}
			}

			PreciseTimeValue syncSpawnTime = spawnTime;
			if (paramAnim) {
				switch (syncType) {
				case kSpawn_syncBy_age:
					syncSpawnTime -= chBirthTimeR->GetValue(i);
					break;
				case kSpawn_syncBy_event:
					syncSpawnTime -= chEventStartR->GetValue(i);
					break;
				}
				numChildren = GetPFInt(pblock(), kSpawn_numOffsprings, syncSpawnTime.TimeValue());
				varChildren = GetPFFloat(pblock(), kSpawn_numVariation, syncSpawnTime.TimeValue());
			}

			int spawnFactor = (int)floor(numChildren*(1 + randGen->Rand11()*varChildren) + 0.5f);

			if (numTotal + spawnFactor > numLimit)
				spawnFactor = numLimit - numTotal;
			numTotal += spawnFactor;

			if (spawnFactor > 0) 
			{
				SpawnParticleData newP;
				newP.time = chTimeR->GetValue(i);
				if (restartAge) newP.birthTime = chTimeR->GetValue(i);
				else newP.birthTime = chBirthTimeR->GetValue(i);
				if (chPosR) newP.position = chPosR->GetValue(i);
				if (chOrientR) newP.orientation = chOrientR->GetValue(i);
				if (chSpinR) newP.spin = chSpinR->GetValue(i);
				Point3 parentSpeed = chSpeedR->GetValue(i);
				Point3 parentScale = chScaleR->GetValue(i);
				PreciseTimeValue parentTime = chTimeR->GetValue(i);
				TimeValue paramTime = parentTime.TimeValue();
				Point3 childSpeed;
				if (speedType == kSpawn_speedType_units) {
					childSpeed = GetPFFloat(pblock(), kSpawn_speedUnits, paramTime)*
									parentSpeed.Normalize()/TIME_TICKSPERSEC;
				} else {
					childSpeed = GetPFFloat(pblock(), kSpawn_speedInherited, paramTime)*parentSpeed;
				}
				float speedVar = GetPFFloat(pblock(), kSpawn_speedVariation, paramTime);
				float speedDiv = GetPFFloat(pblock(), kSpawn_speedDivergence,paramTime);
				Point3 childScale = GetPFFloat(pblock(), kSpawn_scale, paramTime)*parentScale;
				float scaleVar = GetPFFloat(pblock(), kSpawn_scaleVariation, paramTime);
				for(int j=0; j<spawnFactor; j++) {
					newP.speed = childSpeed;
					if (speedVar > 0.0f)
						newP.speed *= (1.0f + speedVar*randGen->Rand11());
					if (LengthSquared(newP.speed))
						newP.speed = DivergeVectorRandom(newP.speed, randGen, speedDiv);
					newP.scale = childScale;
					if (scaleVar > 0.0)
						newP.scale *= (1.0f + scaleVar*randGen->Rand11());
					spawnData[i]->Append(1, &newP);
				}
				if (initLastTime)
					chLastTimeW->SetValue(i, chTimeR->GetValue(i));
				if (initAccumDef)
					chAccumDefW->SetValue(i, 0.0f);
			}

			if (numTotal >= numLimit) break;

			switch (spawnType) {
			case kSpawn_spawnType_once:
				spawnTime = timeEnd+1; // "once" particles spawn only once
				break;
			case kSpawn_spawnType_perFrame: 
				spawnTime = calculateNextSpawnTimeForFrames(i, syncType, timeEnd, paramFrameAnim, spawnRate,
													chLastTimeR, chLastTimeW,
													chAccumDefR, chAccumDefW,
													chBirthTimeR, chEventStartR);
				break;
			case kSpawn_spawnType_byDistance:
				spawnTime = calculateNextSpawnTimeForDistance(i, syncType, timeEnd, paramDistAnim, spawnStepSize,
													chSpeedR,
													chLastTimeR, chLastTimeW,
													chAccumDefR, chAccumDefW,
													chBirthTimeR, chEventStartR);
				break;
			}
		} 

	}

	for(i=0; i<oldCount; i++) {
		if (spawnData[i] != NULL)
			spawnFactor[i] += spawnData[i]->Count();
	}
	int newCount = 0;
	for(i=0; i<oldCount; i++)
		newCount += spawnFactor[i];

	int bornIndex;
	bool removeParents;
	int index = 0;
	bool retVal = false;
	if (!chAmountW->Spawn(spawnFactor)) goto end;


	bornIndex = iPObj->NumParticlesGenerated();
	// set proper values in spawn particles
	for(i=0; i<oldCount; i++) {
		index++;
		if (spawnFactor[i] > 1) {
			for(int j=0; j<spawnData[i]->Count(); j++) 
			{
				SpawnParticleData newP = (*(spawnData[i]))[j];
				ParticleID id = chIDR->GetID(i);
				id.born = bornIndex++;
				chIDW->SetID(index, id);
				chNewW->SetNew(index);
				chTimeW->SetValue(index, newP.time);
				chBirthTimeW->SetValue(index, newP.birthTime);
				if (chSelect) chSelect->SetValue(index, iSystem->IsParticleSelected(id.born));
				if (chPosR) chPosW->SetValue(index, newP.position);
				chSpeedW->SetValue(index, newP.speed);
				chScaleW->SetValue(index, newP.scale);
				if (chOrientR) chOrientW->SetValue(index, newP.orientation);
				if (chSpinR) chSpinW->SetValue(index, newP.spin);
				chIsSpawnW->SetValue(index, true);
				index++;
			}
		}
	}

	// remove old particles if required
	removeParents = false;
	if (spawnType == kSpawn_spawnType_once)
		removeParents = (pblock()->GetInt(kSpawn_deleteParent,0) != 0);
	if (removeParents) {
		BitArray deleteP;
		deleteP.SetSize(newCount);
		deleteP.ClearAll();
		index = 0;
		for(i=0; i<oldCount; i++) {
			if (spawnFactor[i] > 1) deleteP.Set(index);
			index += spawnFactor[i];
		}
		chAmountW->Delete(deleteP);
	}

	// mark new spawn particles as the particles that satisfy the test
	newCount = chAmountR->Count();
	testResult.SetSize(newCount);
	testResult.ClearAll();
	testTime.SetCount(newCount);
	for(i=0; i<newCount; i++) {
		if (chIsSpawnR->GetValue(i) == false) continue;
		testResult.Set(i);
		testTime[i] = float(chTimeR->GetValue(i) - timeStart);
	}

	retVal = true;
end:
	for(i=0; i<oldCount; i++)
		if (spawnData[i] != NULL) delete spawnData[i];

	return retVal;
}

//+--------------------------------------------------------------------------+
//|							From IPFTest									 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::ProceedStep1(IObject* pCont, 
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
bool PFTestSpawn::ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime)
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

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From PFTestSpawn								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
float PFTestSpawn::getSpawnRate(TimeValue time)
{
	float spawnRate = GetPFFloat(pblock(), kSpawn_spawnRate, time)/TIME_TICKSPERSEC;
	return spawnRate;
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
float PFTestSpawn::getSpawnRate(PreciseTimeValue time)
{
	return getSpawnRate(time.TimeValue());
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
float PFTestSpawn::getStepSize(TimeValue time)
{
	float stepSize = pblock()->GetFloat(kSpawn_stepSize, time);
	if (stepSize < kSpawn_stepSize_min) stepSize = kSpawn_stepSize_min;
	if (stepSize > kSpawn_stepSize_max) stepSize = kSpawn_stepSize_max;
	return stepSize;
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
float PFTestSpawn::getStepSize(PreciseTimeValue time)
{
	return getStepSize(time.TimeValue());
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
PreciseTimeValue PFTestSpawn::calculateNextSpawnTimeForFrames(int index, // particle index
														int syncType,
														PreciseTimeValue timeEnd,
														int paramAnim,
														float spawnRate, // frame rate if constant
														IParticleChannelPTVR* chLastTimeR,
														IParticleChannelPTVW* chLastTimeW,
														IParticleChannelFloatR* chAccumDefR,
														IParticleChannelFloatW* chAccumDefW,
														IParticleChannelPTVR* chBirthTime,
														IParticleChannelPTVR* chEventStart )
{
	PreciseTimeValue spawnTime;
	int tpf = GetTicksPerFrame();
	PreciseTimeValue lastTime = chLastTimeR->GetValue(index);
	if (lastTime < timeEnd) 
	{
		float accumDef = chAccumDefR->GetValue(index);
		if (paramAnim) {
			PreciseTimeValue syncLastTime = lastTime;
			switch (syncType) {
			case kSpawn_syncBy_age:
				syncLastTime -= chBirthTime->GetValue(index);
				break;
			case kSpawn_syncBy_event:
				syncLastTime -= chEventStart->GetValue(index);
				break;
			}
			float lastRate = getSpawnRate(syncLastTime);
			if (lastRate > 0.0f) {
calcAccumDef:
				while (lastTime < timeEnd) {
					if (accumDef + lastRate >= 1.0f) {
						spawnTime = lastTime + (1.0f-accumDef)/lastRate;
						lastTime = spawnTime;
						accumDef = 1.0f;
					} else {
						accumDef += lastRate;
						syncLastTime += 1;
						lastTime += 1;
						lastRate = getSpawnRate(syncLastTime);
						spawnTime = lastTime + 1;
					}
				}
				chLastTimeW->SetValue(index, lastTime);
				chAccumDefW->SetValue(index, accumDef);
			} else {
				float nextRate = getSpawnRate(timeEnd);
				if (nextRate > 0.0f) goto calcAccumDef;
				chLastTimeW->SetValue(index, timeEnd);
				spawnTime = timeEnd + 1;
			}
		} else {
			if (spawnRate > 0.0f) {
				spawnTime = lastTime + (1.0f-accumDef)/spawnRate;
				if (spawnTime > timeEnd) {
					chLastTimeW->SetValue(index, timeEnd);
					float addDef = float(timeEnd - lastTime)*spawnRate;
					chAccumDefW->SetValue(index, accumDef + addDef);
				} else {
					chLastTimeW->SetValue(index, spawnTime);
					chAccumDefW->SetValue(index, 1.0f);
				}
			} else {
				spawnTime = timeEnd + 1;
			}
		}
	} else {
		spawnTime = timeEnd + 1;
	}
	return spawnTime;
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
PreciseTimeValue PFTestSpawn::calculateNextSpawnTimeForDistance(int index, // particle index
														int syncType,
														PreciseTimeValue timeEnd,
														int paramAnim,
														float stepSize, // step size if constant
														IParticleChannelPoint3R* chSpeed,
														IParticleChannelPTVR* chLastTimeR,
														IParticleChannelPTVW* chLastTimeW,
														IParticleChannelFloatR* chAccumDefR,
														IParticleChannelFloatW* chAccumDefW,
														IParticleChannelPTVR* chBirthTime,
														IParticleChannelPTVR* chEventStart )
{
	PreciseTimeValue spawnTime;
//	int tpf = GetTicksPerFrame();
	float speed = Length(chSpeed->GetValue(index));
	if (speed <= 0.0f) return (timeEnd + 1);

	PreciseTimeValue lastTime = chLastTimeR->GetValue(index);
	if (lastTime < timeEnd) 
	{
		float accumDef = chAccumDefR->GetValue(index);
		if (paramAnim) {
			PreciseTimeValue syncLastTime = lastTime;
			switch (syncType) {
			case kSpawn_syncBy_age:
				syncLastTime -= chBirthTime->GetValue(index);
				break;
			case kSpawn_syncBy_event:
				syncLastTime -= chEventStart->GetValue(index);
				break;
			}
			stepSize = getStepSize(syncLastTime);
		}
		//spawnTime = lastTime + (1.0f-accumDef)*stepSize/speed; // may create critical numerical error
		//if (spawnTime > timeEnd) {
		if (float(timeEnd-lastTime)*speed < (1.0f-accumDef)*stepSize) {
			chLastTimeW->SetValue(index, timeEnd);
			float addDef = float(timeEnd - lastTime)*speed/stepSize;
			chAccumDefW->SetValue(index, accumDef + addDef);
			spawnTime = timeEnd + 1;
		} else {
			spawnTime = lastTime + (1.0f-accumDef)*stepSize/speed;
			chLastTimeW->SetValue(index, spawnTime);
			chAccumDefW->SetValue(index, 1.0f);
		}
	} else {
		spawnTime = timeEnd + 1;
	}
	return spawnTime;
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::RefreshUI(WPARAM message, IParamMap2* map) const
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

//+--------------------------------------------------------------------------+
//|							From PFTestSpawn								 |
//+--------------------------------------------------------------------------+
void PFTestSpawn::UpdatePViewUI(int updateID) const
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		map->Invalidate(updateID);
		Interval currentTimeInterval;
		currentTimeInterval.SetInstant(GetCOREInterface()->GetTime());
		map->Validity() = currentTimeInterval;
	}
}


ClassDesc* PFTestSpawn::GetClassDesc() const
{
	return GetPFTestSpawnDesc();
}





} // end of namespace PFActions