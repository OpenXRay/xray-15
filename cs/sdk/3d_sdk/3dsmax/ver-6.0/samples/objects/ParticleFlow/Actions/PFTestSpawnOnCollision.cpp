/**********************************************************************
 *<
	FILE: PFTestSpawnOnCollision.h

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

#include "PFTestSpawnOnCollision.h"

#include "PFTestSpawnOnCollision_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "IParticleObjectExt.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "PFSimpleActionState.h"

namespace PFActions {

// the test creates two local particle channels to store some data per particle
// first channel stores number of hits the particle has experienced
#define PARTICLECHANNELNUMHITSR_INTERFACE Interface_ID(0x61324c5c, 0x1eb34500) 
#define PARTICLECHANNELNUMHITSW_INTERFACE Interface_ID(0x61324c5c, 0x1eb34501) 
#define GetParticleChannelNumHitsRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELNUMHITSR_INTERFACE)) 
#define GetParticleChannelNumHitsWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELNUMHITSW_INTERFACE)) 
// second channel stores a flag if a particle is a spawn particle
// value false means that this is an original particle
// value true means that the particle is a spawn particle
#define PARTICLECHANNELISSPAWNR_INTERFACE Interface_ID(0x61324c5d, 0x1eb34500) 
#define PARTICLECHANNELISSPAWNW_INTERFACE Interface_ID(0x61324c5d, 0x1eb34501) 
#define GetParticleChannelIsSpawnRInterface(obj) ((IParticleChannelBoolR*)obj->GetInterface(PARTICLECHANNELISSPAWNR_INTERFACE)) 
#define GetParticleChannelIsSpawnWInterface(obj) ((IParticleChannelBoolW*)obj->GetInterface(PARTICLECHANNELISSPAWNW_INTERFACE)) 


// static members
Object*		PFTestSpawnOnCollision::m_editOb				= NULL;
IObjParam*	PFTestSpawnOnCollision::m_ip					= NULL;

CollisionSpaceWarpValidatorClass PFTestSpawnOnCollision::validator;
bool PFTestSpawnOnCollision::validatorInitiated = false;

// constructors/destructors
PFTestSpawnOnCollision::PFTestSpawnOnCollision()
				:IPFTest()
{ 
	RegisterParticleFlowNotification();
	_pblock() = NULL;
	_numDeflectors() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kSpawnCollision_deflectors;
		spawnOnCollision_paramBlockDesc.ParamOption(kSpawnCollision_deflectors,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _trueIcon() = _falseIcon() = NULL;
}

PFTestSpawnOnCollision::~PFTestSpawnOnCollision()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFTestSpawnOnCollision::GetInterface(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return (IPFAction*)this;
	if (id == PFTEST_INTERFACE) return (IPFTest*)this;
	if (id == PVIEWITEM_INTERFACE) return (IPViewItem*)this;
	if (id == PFINTEGRATOR_INTERFACE) return (IPFIntegrator*)this;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestSpawnOnCollision::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &spawnOnCollision_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &spawnOnCollision_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &spawnOnCollision_PViewItem_FPInterfaceDesc;
	if (id == PFINTEGRATOR_INTERFACE) return &spawnOnCollision_integrator_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestSpawnOnCollision::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SPAWNONCOLLISION_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSpawnOnCollision::ClassID()
{
	return PFTestSpawnCollisionSW_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFTestSpawnOnCollision::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFTestSpawnOnCollision::SubAnimName(int i)
{
	return PFActions::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFTestSpawnOnCollision::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestSpawnOnCollision::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestSpawnOnCollision::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFTestSpawnOnCollision::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PFTestSpawnOnCollision::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) 
			{
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kSpawnCollision_deflectorsMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSDeflectors();
					return REF_STOP;
				}
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch ( lastUpdateID )
				{
				case kSpawnCollision_spawnType:
				case kSpawnCollision_deleteParent:
					RefreshUI(kSpawnCollision_message_type);
					break;
				case kSpawnCollision_speedType:
					RefreshUI(kSpawnCollision_message_speed);
					break;
				case kSpawnCollision_spawnAble:
				case kSpawnCollision_numVariation:
				case kSpawnCollision_speedVariation:
				case kSpawnCollision_speedDivergence:
				case kSpawnCollision_scaleVariation:
					RefreshUI(kSpawnCollision_message_random);
					break;
				case kSpawnCollision_deflectors:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealDeflectors();
					if (updateObjectNames(kSpawnCollision_deflectors)) {
						RefreshUI(kSpawnCollision_message_random);
						RefreshUI(kSpawnCollision_message_deflectors);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			UpdatePViewUI(kSpawnCollision_deflectors);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealDeflectors();
			break;
		case kSpawnCollision_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kSpawnCollision_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kSpawnCollision_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFTestSpawnOnCollision::Clone(RemapDir &remap)
{
	PFTestSpawnOnCollision* newTest = new PFTestSpawnOnCollision();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PFTestSpawnOnCollision::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestSpawnOnCollision::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestSpawnOnCollision::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestSpawnOnCollision::GetObjectName()
{
	return GetString(IDS_TEST_SPAWNONCOLLISION_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSpawnOnCollision::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_givenIntegrator(pCont) = NULL;
	return _randLinker().Init( pCont, GetRand() );
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Release(IObject* pCont)
{
	_givenIntegrator(pCont) = NULL;
	_randLinker().Release( pCont );
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestSpawnOnCollision::GetRand()
{
	return pblock()->GetInt(kSpawnCollision_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::SetRand(int seed)
{
	pblock()->SetValue(kSpawnCollision_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFTestSpawnOnCollision::GetCurrentState(IObject* pContainer)
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
void PFTestSpawnOnCollision::SetCurrentState(IObject* aSt, IObject* pContainer)
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
const ParticleChannelMask& PFTestSpawnOnCollision::ChannelsUsed(const Interval& time) const
{
	static ParticleChannelMask mask(PCU_New|PCU_ID|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_Position|PCU_Speed, // read 
									PCU_ID|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_Position|PCU_Speed); // write
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELNUMHITSR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELNUMHITSW_INTERFACE); // write channel
		mask.AddChannel(PARTICLECHANNELISSPAWNR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELISSPAWNW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpawnOnCollision::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPAWNONCOLLISION_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpawnOnCollision::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPAWNONCOLLISION_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSpawnOnCollision::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPAWNONCOLLISION_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::HasDynamicName(TSTR& nameSuffix)
{
	int num = pblock()->Count(kSpawnCollision_deflectors);
	int count=0, firstIndex=-1;
	for(int i=0; i<num; i++) {
		if (pblock()->GetINode(kSpawnCollision_deflectors, 0, i) != NULL) {
			count++;
			if (firstIndex < 0) firstIndex = i;
		}
	}
	if (count > 0) {
		INode* force = pblock()->GetINode(kSpawnCollision_deflectors, 0, firstIndex);
		nameSuffix = force->GetName();
		if (count > 1) {
			TCHAR buf[32];
			sprintf(buf, " +%d", count-1);
			nameSuffix += buf;
		}
	} else {
		nameSuffix = GetString(IDS_NONE);
	}
	return true;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
class SpawnOnCollisionNonAnimData {
public:
	void Init(IParamBlock2* pblock, PreciseTimeValue time);

	int sendParentOut, sendSpawnOut, spawnType, deleteParent, untilNum;
	int restartAge, parentSpeedType, spawnSpeedType, speedType;
};

void SpawnOnCollisionNonAnimData::Init(IParamBlock2* pblock, PreciseTimeValue time)
{
	sendParentOut	= pblock->GetInt(kSpawnCollision_sendParentOut, time);
	sendSpawnOut	= pblock->GetInt(kSpawnCollision_sendSpawnOut, time);
	spawnType		= pblock->GetInt(kSpawnCollision_spawnType, time);
	deleteParent	= pblock->GetInt(kSpawnCollision_deleteParent, time);
	untilNum		= pblock->GetInt(kSpawnCollision_untilNum, time);
	restartAge		= pblock->GetInt(kSpawnCollision_restartAge, time);
	parentSpeedType	= pblock->GetInt(kSpawnCollision_speedParent, time);
	spawnSpeedType	= pblock->GetInt(kSpawnCollision_speedOffspring, time);
	speedType		= pblock->GetInt(kSpawnCollision_speedType, time);
}

 // returns number of new spawn particles
int DoSpawn( RandGenerator* randGen, // random number generator
			 PreciseTimeValue collisionTime, // time of the collision
			 PreciseTimeValue syncTime, // relative time to acquire animated parameters from pblock
			 const Point3& parentPos, // parent particle position at collision moment
			 const Point3& oldSpeed, // parent particle speed before collision
			 const Point3& newSpeed, // parent particle speed after collision
			 const Point3& parentScale, // parent particle scale factor
			 const Quat& parentOrient, // parent particle orientation at collision moment
			 const AngAxis& parentSpin, // parent particle spin at collision moment
			 const SpawnOnCollisionNonAnimData& nad, // non-animatable parameters
			 IParamBlock2* pblock, // other parameters
			 int& numLimit, // maximum allowed amount of particles to be born; reduce by amount of new particles
			 Tab<PreciseTimeValue>& spawnTime, // spawn/birth time
			 Tab<Point3>& position, // position at birth
			 Tab<Point3>& speed, // speed at birth
			 Tab<Point3>& scale, // scale at birth
			 Tab<Quat>& orient, // orientation at birth
			 Tab<AngAxis>& spin) // spin at birth
{
	
	float spawnAble = GetPFFloat(pblock, kSpawnCollision_spawnAble, syncTime);
	if (spawnAble <= 0.0f) return 0;
	if (spawnAble < 1.0f)
		if (randGen->Rand01() > spawnAble) return 0;

	int numSpawn = GetPFInt(pblock, kSpawnCollision_numOffsprings, syncTime);
	float numVar = GetPFFloat(pblock, kSpawnCollision_numVariation, syncTime);
	if (numVar > 0.0f)
		numSpawn = (int)(numSpawn * (1.0f + numVar*randGen->Rand11()));
	if (numSpawn <= 0) return 0;		

	if (numSpawn > numLimit) numSpawn = numLimit;
	numLimit -= numSpawn;

	float fUPFScale = 1.0f/TIME_TICKSPERSEC; // conversion units per seconds to units per tick

	int oldCount = spawnTime.Count();
	int newCount = oldCount + numSpawn;
	spawnTime.SetCount(newCount);
	position.SetCount(newCount);
	speed.SetCount(newCount);
	scale.SetCount(newCount);
	orient.SetCount(newCount);
	spin.SetCount(newCount);
	for(int i=oldCount; i<newCount; i++) {
		spawnTime[i] = collisionTime;
		position[i] = parentPos;
		orient[i] = parentOrient;
		spin[i] = parentSpin;
		Point3 spawnSpeed = newSpeed;
		if (nad.spawnSpeedType == kSpawnCollision_speedType_continue) spawnSpeed = oldSpeed;
		float speedLen = Length(spawnSpeed);
		if (speedLen > 0.0f) {
			float divergence = pblock->GetFloat(kSpawnCollision_speedDivergence, collisionTime);
			if (divergence > 0.0f)
				spawnSpeed = DivergeVectorRandom(spawnSpeed, randGen, divergence);
			else
				DivergeVectorRandom(spawnSpeed, randGen, 10.0f); // to keep rand generator in sync
			if (nad.speedType == kSpawnCollision_speedType_units) {
				float unitLen = GetPFFloat(pblock, kSpawnCollision_speedUnits, collisionTime);
				spawnSpeed *= fUPFScale*unitLen/speedLen;
			} else {
				spawnSpeed *= GetPFFloat(pblock, kSpawnCollision_speedInherited, collisionTime);
			}
			float speedVar = GetPFFloat(pblock, kSpawnCollision_speedVariation, collisionTime);		
			if (speedVar > 0.0f)
				spawnSpeed *= 1 + speedVar*randGen->Rand11();
			else
				randGen->Rand11(); // to keep rand generator in sync
		}
		speed[i] = spawnSpeed;
		float scaleFactor = GetPFFloat(pblock, kSpawnCollision_scale, collisionTime);
		float scaleVar = GetPFFloat(pblock, kSpawnCollision_scaleVariation, collisionTime);
		if (scaleVar > 0.0f)
			scaleFactor *= 1 + scaleVar*randGen->Rand11();
		else
			randGen->Rand11(); // to keep rand generator in sync
		scale[i] = scaleFactor*parentScale;
	}

	return numSpawn;
}

//+--------------------------------------------------------------------------+
//|				From IPFTest: the Proceed method							 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	// grab integrator
	if (integrator == NULL) return false;
	if ((integrator != (IPFIntegrator*)this) && (!integrator->HasEncapsulatedIntegrator((IPFIntegrator*)this)))
	_givenIntegrator(pCont) = integrator;

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;
	IPFSystem* iSystem = GetPFSystemInterface(pSystem);
	if (iSystem == NULL) return false; // given particle system doesn't have PFSystem interface

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmountR = GetParticleChannelAmountRInterface(pCont);
	if (chAmountR == NULL) return false; // can't find number of particles in the container
	IParticleChannelAmountW* chAmountW = GetParticleChannelAmountWInterface(pCont);
	if (chAmountW == NULL) return false; // can't modify number of particles in the container
	IParticleChannelIDR* chIDR = GetParticleChannelIDRInterface(pCont);
	if (chIDR == NULL) return false; // can't read ID info for a particle
	IParticleChannelIDW* chIDW = GetParticleChannelIDWInterface(pCont);
	if (chIDW == NULL) return false; // can't modify ID info for a particle
	IParticleChannelPTVR* chTimeR = GetParticleChannelTimeRInterface(pCont);
	if (chTimeR == NULL) return false; // can't read timing info for a particle
	IParticleChannelPTVW* chTimeW = GetParticleChannelTimeWInterface(pCont);
	if (chTimeW == NULL) return false; // can't modify timing info for a particle
	IParticleChannelPTVR* chBirthTimeR = GetParticleChannelBirthTimeRInterface(pCont);
	if (chBirthTimeR == NULL) return false; // can't read timing info for a particle
	IParticleChannelPTVW* chBirthTimeW = GetParticleChannelBirthTimeWInterface(pCont);
	if (chBirthTimeW == NULL) return false; // can't modify timing info for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	IParticleChannelNewW* chNewW = GetParticleChannelNewWInterface(pCont);
	if (chNewW == NULL) return false; // can't find modify 'new' status of the spawned particles

	// the channel of interest: position
	IParticleChannelPoint3R* chPosR = GetParticleChannelPositionRInterface(pCont);
	IParticleChannelPoint3W* chPosW = GetParticleChannelPositionWInterface(pCont);
	if ((chPosR == NULL) || (chPosW == NULL)) return false; // particle position is underfined

	// the channel of interest: speed
	bool initSpeed = false;
	IParticleChannelPoint3R* chSpeedR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELSPEEDR_INTERFACE,
																	ParticleChannelPoint3_Class_ID,
																	true, PARTICLECHANNELSPEEDR_INTERFACE,
																	PARTICLECHANNELSPEEDW_INTERFACE, true,
																	actionNode, NULL, &initSpeed);
	if (chSpeedR == NULL) return false; // can't read/create speed channel
	IParticleChannelPoint3W* chSpeedW = GetParticleChannelSpeedWInterface(pCont);
	if (chSpeedW == NULL) return false; // can't modify speed channel
	IParticleChannelBoolW* chSelect = NULL;
	if (iSystem->NumParticlesSelected() > 0)
		chSelect = (IParticleChannelBoolW*)chCont->EnsureInterface(PARTICLECHANNELSELECTIONW_INTERFACE,
																	ParticleChannelBool_Class_ID,
																	true, PARTICLECHANNELSELECTIONR_INTERFACE,
																	PARTICLECHANNELSELECTIONW_INTERFACE);
	int i, count = chAmountR->Count();
	if (initSpeed) {
		for(i=0; i<count; i++)
			if (chNew->IsNew(i))
				chSpeedW->SetValue(i, Point3::Origin);
	}

	// the channel of interest: acceleration
	// may not be present
	IParticleChannelPoint3R* chAccel	= GetParticleChannelAccelerationRInterface(pCont);

	// the channel of interest: orientation
	// may not be present
	IParticleChannelQuatR* chOrientR	= GetParticleChannelOrientationRInterface(pCont);
	IParticleChannelQuatW* chOrientW	= GetParticleChannelOrientationWInterface(pCont);

	// the channel of interest: spin
	// may not be present
	IParticleChannelAngAxisR* chSpinR	= GetParticleChannelSpinRInterface(pCont);
	IParticleChannelAngAxisW* chSpinW	= GetParticleChannelSpinWInterface(pCont);

	// a couple of accounting channels (first one)
	// need to create a collision hits channel if we have colision hit count test on
	// the data are transferred for instances of the test to use
	// therefore if there are two instances of the same test then those hits are
	// accumulated by both instances
	bool initNumHits = false;
	IParticleChannelIntR* chNumHitsR = (IParticleChannelIntR*)chCont->EnsureInterface(PARTICLECHANNELNUMHITSR_INTERFACE,
																		ParticleChannelInt_Class_ID,
																		true, PARTICLECHANNELNUMHITSR_INTERFACE,
																		PARTICLECHANNELNUMHITSW_INTERFACE, true,
																		actionNode, (Object*)this, &initNumHits);
	IParticleChannelIntW* chNumHitsW = (IParticleChannelIntW*)chCont->GetPrivateInterface(PARTICLECHANNELNUMHITSW_INTERFACE, (Object*)this);
	if ((chNumHitsR == NULL) || (chNumHitsW == NULL)) return false; // can't set test value for newly entered particles

	// a couple of accounting channels (second one)
	// to store a flag to identify new spawn particles
	bool initIsSpawn = false;
	IParticleChannelBoolR* chIsSpawnR = (IParticleChannelBoolR*)chCont->EnsureInterface(PARTICLECHANNELISSPAWNR_INTERFACE,
																		ParticleChannelBool_Class_ID,
																		true, PARTICLECHANNELISSPAWNR_INTERFACE,
																		PARTICLECHANNELISSPAWNW_INTERFACE, true,
																		actionNode, (Object*)this, &initIsSpawn);
	IParticleChannelBoolW* chIsSpawnW = (IParticleChannelBoolW*)chCont->GetPrivateInterface(PARTICLECHANNELISSPAWNW_INTERFACE, (Object*)this);
	if ((chIsSpawnR == NULL) || (chIsSpawnW == NULL)) return false; // can't set test value for newly entered particles

	// we may need EventStart channel if syncBy is set to Event Duration time
	int syncBy = pblock()->GetInt(kSpawnCollision_syncBy, timeEnd);
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (syncBy == kSpawnCollision_syncBy_event) {
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

	// we may need Scale channel if there is a change in scale
	// the scale parameter may be animated so let's reserve the channel
	bool initScale = false;
	IParticleChannelPoint3W* chScaleW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSCALEW_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELSCALER_INTERFACE,
																PARTICLECHANNELSCALEW_INTERFACE, true,
																actionNode, NULL, &initScale);
	if (chScaleW == NULL) return false;	// can't modify Scale channel in the container
	IParticleChannelPoint3R* chScaleR = GetParticleChannelScaleRInterface(pCont);
	if (chScaleR == NULL) return false; // can't read scale channel in the container

	RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
	DbgAssert(randGen);
	if (randGen == NULL) return false;

	if ((!chNew->IsAllOld()) && (initNumHits || initIsSpawn || initEventStart || initScale))
	{ // initialize some channels
		for(i=0; i<count; i++) {
			if (!chNew->IsNew(i)) continue;
			if (initNumHits)
				chNumHitsW->SetValue(i, 0);
			PreciseTimeValue curTime = chTimeR->GetValue(i);
			if (initIsSpawn) 
				chIsSpawnW->SetValue(i, false);
			if (initEventStart)
				chEventStartW->SetValue(i, curTime);
			if (initScale)
				chScaleW->SetValue(i, Point3(1.0f,1.0f,1.0f));
		}
	}

	// the real calculations for spawn particles start here

	BitArray doDelete; // fixed size arrays
	doDelete.SetSize(count);
	doDelete.ClearAll();
	Tab<int> spawnFactor;
	spawnFactor.SetCount(count);
	for(i=0; i<count; i++) spawnFactor[i] = 0;

	// data for spawn particles
	Tab<PreciseTimeValue> spawnTime;
	Tab<Point3> position, speed, scale;
	Tab<Quat> orient;
	Tab<AngAxis> spin;

	// get non-animatable parameters
	SpawnOnCollisionNonAnimData nad;
	nad.Init(pblock(), timeEnd);
	int numHitUntil = 1;
	if (nad.spawnType == kSpawnCollision_spawnType_multiple)
		numHitUntil = nad.untilNum;

	// check if the test is allowed to produce more particles
	int numLimit = iSystem->GetBornAllowance();
	IParticleObjectExt* iPObj = GetParticleObjectExtInterface(GetPFObject(pSystem));
	if (iPObj == NULL) return false;
	int numTotal = iPObj->NumParticles();
	numLimit -= iPObj->NumParticles(); // how many particles are allowed to be born now
	if (numLimit < 0) numLimit = 0;

	// check if we have valid deflectors
	Tab<CollisionObject*> collisionObjs;
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend(); // to cloak creation of collision objects
	for(i=0; i<pblock()->Count(kSpawnCollision_deflectors); i++) {
		INode* node = pblock()->GetINode(kSpawnCollision_deflectors, timeEnd, i);
		if (node == NULL) continue;
		Object* obj = GetPFObject(node->GetObjectRef());
		if (obj == NULL) continue;
		int id = (obj?obj->SuperClassID():SClass_ID(0));
		if (id != WSM_OBJECT_CLASS_ID) continue;
		WSMObject* obref = (WSMObject*)obj;
		CollisionObject* colObj = obref->GetCollisionObject(node);
		if (colObj != NULL) {
			colObj->SetRandSeed(randGen->Rand0X(RAND_MAX-1));
			collisionObjs.Append(1, &colObj);
		}
	}
	if (wasHolding) theHold.Resume();
	if (collisionObjs.Count() == 0) return true; // no deflectors to hit on
	if (wasHolding) theHold.Suspend(); // collision object can be creation during CheckCollision call below
	CollisionCollection cc;
	cc.Init(collisionObjs);

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	// bayboro August 1 2002
	// this is a work-around to avoid "under-initialized" collisionObject
	// if the first "dt" value that is passed to collisionObject is low
	// then the collisionObject is initialized for the interval [timeEnd-dt, timeEnd]
	// that creates a problem when a later particle is passed with a higher "dt" value
	// therefore it is necessary to perform a 'fake' collision check for the whole time interval
	float fakeHitTime;
	Point3 fakePosition = Point3::Origin;
	Point3 fakeSpeed = Point3::Origin;
	cc.CheckCollision((int)timeEnd, fakePosition, fakeSpeed, (int)timeEnd-(int)timeStart, 0, &fakeHitTime, FALSE);

	for(i=0; i<count; i++) {
		if (nad.sendSpawnOut)
			if (chIsSpawnR->GetValue(i)) continue; // don't proceed spawn particles if they diverted out

		PreciseTimeValue curTime = chTimeR->GetValue(i);
		if (curTime >= timeEnd) continue; // particle has been proceeded beyond 
										  // the testing interval [timeStart,timeEnd)
		float timeSlice = float(timeEnd - curTime);

		Point3 curPos = chPosR->GetValue(i);
		Point3 curSpeed = chSpeedR->GetValue(i);
		Point3 curAccel = Point3::Origin;
		if (chAccel != NULL) curAccel = chAccel->GetValue(i);

		curSpeed += (0.5f*timeSlice)*curAccel;
		
		float hitTime, remTime = timeSlice;
		Point3 newPos = curPos;
		Point3 newSpeed = curSpeed;
		bool collide=false, maybeStuck=false;
		PreciseTimeValue birthTime, eventStart;
		birthTime = chBirthTimeR->GetValue(i);
		if (syncBy == kSpawnCollision_syncBy_event)
			eventStart = chEventStartR->GetValue(i);
		int hasNumHits = chNumHitsR->GetValue(i);
		int bornID = chIDR->GetParticleBorn(i);
		for(int j=0; j<cc.MAX_COLLISIONS_PER_STEP; j++)
		{
			if (nad.sendParentOut)
				if (hasNumHits >= numHitUntil) break; // particle has reached it's hit maximum

			if (cc.CheckCollision(timeEnd.TimeValue(), newPos, newSpeed, remTime, bornID, &hitTime, FALSE))
			{
				remTime -= hitTime;
				PreciseTimeValue collisionTime = timeEnd - remTime;
				integrator->Proceed(pCont, collisionTime, i);
				chPosW->SetValue(i, newPos);
				if (nad.parentSpeedType == kSpawnCollision_speedType_bounce)
					chSpeedW->SetValue(i, newSpeed);
				else
					chSpeedW->SetValue(i, curSpeed);
				collide = true;
				
				// check if we can produce new particles
				if ((hasNumHits < numHitUntil) && (numLimit>0)) 
				{
					PreciseTimeValue syncTime = collisionTime;
					if (syncBy == kSpawnCollision_syncBy_age)
						syncTime -= birthTime;
					else if (syncBy == kSpawnCollision_syncBy_event)
						syncTime -= eventStart;

					Quat parentOrient;
					if (chOrientR != NULL) parentOrient = chOrientR->GetValue(i);
					AngAxis parentSpin;
					if (chSpinR != NULL) parentSpin = chSpinR->GetValue(i);

					spawnFactor[i] += DoSpawn(randGen, collisionTime, syncTime, 
											newPos, curSpeed, newSpeed,
											chScaleR->GetValue(i), parentOrient, parentSpin,
											nad, pblock(), numLimit,
											spawnTime, position, 
											speed, scale, orient, spin);
				}
				hasNumHits++;
				if (remTime <= 0.0f) break; // time limit for the current integration step
			}
			else break;
			if (i==cc.MAX_COLLISIONS_PER_STEP-1) maybeStuck = true;
		}
		if (collide) {
			chNumHitsW->SetValue(i, hasNumHits);
			// mark the parent to be deleted if...
			if (nad.spawnType == kSpawnCollision_spawnType_once)
				if (nad.deleteParent) {
					doDelete.Set(i, 1);
					numLimit++;
				}
		}

		if (maybeStuck) { // if particle stuck we can't risk to propagate particle movement for the current frame
			integrator->Proceed(pCont, timeEnd, i);
			chPosW->SetValue(i, newPos);
		}
	}

	// release deflectors
	for(i=0; i<collisionObjs.Count(); i++)
	{
		if (collisionObjs[i] != NULL)
			collisionObjs[i]->DeleteThis();
		collisionObjs[i] = NULL;
	}
	if (wasHolding) theHold.Resume();

	// check consistency for the number of spawn particles
	int numSpawn = 0;
	for(i=0; i<count; i++) numSpawn += spawnFactor[i];
	assert(numSpawn == spawnTime.Count());
	assert(numSpawn == position.Count());
	assert(numSpawn == speed.Count());
	assert(numSpawn == scale.Count());
	assert(numSpawn == orient.Count());
	assert(numSpawn == spin.Count());

	// assign bornIDs
	int bornIndex = iPObj->NumParticlesGenerated();

	// add 1 to each entry in spawnFactor for the parent particle itself
	for(i=0; i<count; i++) spawnFactor[i]++;
	// make formal spawning
	if (!chAmountW->Spawn(spawnFactor)) {
		if (!wasIgnoring) ClearIgnoreRefNodeChange();
		return false;
	}

	// set proper values in spawn particles
	// mark parents for deletion
	BitArray toRemove;
	toRemove.SetSize(count + numSpawn);
	toRemove.ClearAll();
	int index = 0, parentIndex = 0, spawnIndex = 0;
	for(i=0; i<count; i++) 
	{
		index++;
		for(int j=0; j<(spawnFactor[i]-1); j++) 
		{
			ParticleID id = chIDR->GetID(parentIndex);
			id.born = bornIndex++;
			chIDW->SetID(index, id);
			chNewW->SetNew(index);
			chTimeW->SetValue(index, spawnTime[spawnIndex]);
			if (nad.restartAge)
				chBirthTimeW->SetValue(index, spawnTime[spawnIndex]);
			if (chSelect != NULL)
				chSelect->SetValue(index, iSystem->IsParticleSelected(id.born));
			chPosW->SetValue(index, position[spawnIndex]);
			chSpeedW->SetValue(index, speed[spawnIndex]);
			chScaleW->SetValue(index, scale[spawnIndex]);
			if (chOrientW != NULL)
				chOrientW->SetValue(index, orient[spawnIndex]);
			if (chSpinW != NULL)
				chSpinW->SetValue(index, spin[spawnIndex]);
			if (nad.sendSpawnOut)
				chNumHitsW->SetValue(index, 0);
			else
				chNumHitsW->SetValue(index, -1); // for additional processing
			chIsSpawnW->SetValue(index, true);
			index++; spawnIndex++;
		}
		if (doDelete[i])
			toRemove.Set(parentIndex, 1);
		parentIndex = index;
	}

	// remove parents if required
	if ((nad.spawnType == kSpawnCollision_spawnType_once) && (nad.deleteParent)) 
		if (!chAmountW->Delete(toRemove)) {
			if (!wasIgnoring) ClearIgnoreRefNodeChange();
			return false;
		}

	// mark particles tested true
	count = chAmountR->Count();
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	for(i=0; i<count; i++) {
		if (((nad.sendParentOut) && (chNumHitsR->GetValue(i) >= numHitUntil)) ||
			((nad.sendSpawnOut) && (chIsSpawnR->GetValue(i) == true)))
		{
			testResult.Set(i);
			testTime[i] = float(chTimeR->GetValue(i) - timeStart);
		}
	}

	if (!nad.sendSpawnOut) {
		if (!wasIgnoring) ClearIgnoreRefNodeChange();
		return true;
	}
	// if spawn particles aren't sent out then they require additional proceeding
	for(i=0; i<count; i++) 
	{
		if (chNumHitsR->GetValue(i) >= 0) continue; // for fresh spawn particles only

		PreciseTimeValue curTime = chTimeR->GetValue(i);
		if (curTime >= timeEnd) continue; // particle has been proceeded beyond 
										  // the testing interval [timeStart,timeEnd)
		float timeSlice = float(timeEnd - curTime);

		Point3 curPos = chPosR->GetValue(i);
		Point3 curSpeed = chSpeedR->GetValue(i);
		
		float hitTime, remTime = timeSlice;
		Point3 newPos = curPos;
		Point3 newSpeed = curSpeed;
		bool maybeStuck=false;
		int bornID = chIDR->GetParticleBorn(i);
		for(int j=0; j<cc.MAX_COLLISIONS_PER_STEP; j++)
		{
			if (cc.CheckCollision(timeEnd.TimeValue(), newPos, newSpeed, remTime, bornID, &hitTime, FALSE))
			{
				remTime -= hitTime;
				PreciseTimeValue collisionTime = timeEnd - remTime;
				integrator->Proceed(pCont, collisionTime, i);
				chPosW->SetValue(i, newPos);
				chSpeedW->SetValue(i, newSpeed);
				if (remTime <= 0.0f) break; // time limit for the current integration step
			}
			else break;
			if (i==cc.MAX_COLLISIONS_PER_STEP-1) maybeStuck = true;
		}

		if (!maybeStuck) { // if particle stuck we can't risk to propagate particle movement for the current frame
			integrator->Proceed(pCont, timeEnd, i);
			chPosW->SetValue(i, newPos);
		}
		chNumHitsW->SetValue(i, 0);
	}

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFTest									 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::ProceedStep1(IObject* pCont, 
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
bool PFTestSpawnOnCollision::ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime)
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

//+--------------------------------------------------------------------------+
//|							From IPFIntegrator								 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Proceed(IObject* pCont, PreciseTimeValue time, int index)
{
	if (pCont != NULL) _prevContainer() = pCont;
	else pCont = prevContainer();
	if (pCont == NULL) return false;

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// check index range
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if ((index < 0) || (index >= count)) return false; // out of range index

	// acquire absolutely necessary particle channels
	IParticleChannelPTVR* chTimeR = GetParticleChannelTimeRInterface(pCont);
	IParticleChannelPTVW* chTimeW = GetParticleChannelTimeWInterface(pCont);
	if ((chTimeR == NULL) || (chTimeW == NULL)) return false; // can't read/modify timing info for a particle
	IParticleChannelPoint3R* chPosR		= GetParticleChannelPositionRInterface(pCont);
	IParticleChannelPoint3W* chPosW		= GetParticleChannelPositionWInterface(pCont);
	IParticleChannelPoint3R* chSpeedR	= GetParticleChannelSpeedRInterface(pCont);
	IParticleChannelPoint3W* chSpeedW	= GetParticleChannelSpeedWInterface(pCont);
	IParticleChannelPoint3R* chAccel	= GetParticleChannelAccelerationRInterface(pCont);
	
	// check timing
	PreciseTimeValue curTime = chTimeR->GetValue(index);
	if (curTime >= time) {
		if (_givenIntegrator(pCont) != NULL)
			return _givenIntegrator(pCont)->Proceed(pCont, time, index);
	}

	// get data
	Point3 pos, speed, accel;
	pos = speed = accel = Point3::Origin;
	if (chPosR != NULL) pos = chPosR->GetValue(index);
	if (chSpeedR != NULL) speed = chSpeedR->GetValue(index);
	if (chAccel != NULL) accel = chAccel->GetValue(index);
	
	float timeSlice = float(time - curTime);

	// first, let the given integration to do the job of updating values
	bool res = true;
	if (_givenIntegrator(pCont) != NULL)
		res = _givenIntegrator(pCont)->Proceed(pCont, time, index);

	// second, set integration values of interest
	speed += 0.5f*accel*timeSlice;
	pos += speed*timeSlice;
	chTimeW->SetValue(index, time);
	if (chPosW != NULL)
		chPosW->SetValue(index, pos);
	if (chSpeedW != NULL)
		chSpeedW->SetValue(index, speed);

	return res;
}

//+--------------------------------------------------------------------------+
//|							From IPFIntegrator								 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Proceed(IObject* pCont, PreciseTimeValue time)
{
	if (pCont != NULL) _prevContainer() = pCont;
	else pCont = prevContainer();
	if (pCont == NULL) return false;

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// get count info
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int i, count = chAmount->Count();

	// acquire absolutely necessary particle channels
	IParticleChannelPTVR* chTimeR = GetParticleChannelTimeRInterface(pCont);
	IParticleChannelPTVW* chTimeW = GetParticleChannelTimeWInterface(pCont);
	if ((chTimeR == NULL) || (chTimeW == NULL)) return false; // can't read/modify timing info for a particle
	IParticleChannelPoint3R* chPosR		= GetParticleChannelPositionRInterface(pCont);
	IParticleChannelPoint3W* chPosW		= GetParticleChannelPositionWInterface(pCont);
	IParticleChannelPoint3R* chSpeedR	= GetParticleChannelSpeedRInterface(pCont);
	IParticleChannelPoint3W* chSpeedW	= GetParticleChannelSpeedWInterface(pCont);
	IParticleChannelPoint3R* chAccel	= GetParticleChannelAccelerationRInterface(pCont);

	// get timing
	Tab<PreciseTimeValue> curTime;
	curTime.SetCount(count);
	for(i=0; i<count; i++)
		curTime[i] = chTimeR->GetValue(i);

	// get data
	Tab<Point3> pos, speed, accel;
	pos.SetCount(count);
	speed.SetCount(count);
	accel.SetCount(count);
	if (chPosR != NULL) {
		for(i=0; i<count; i++) pos[i] = chPosR->GetValue(i);
	} else {
		for(i=0; i<count; i++) pos[i] = Point3::Origin;
	}
	if (chSpeedR != NULL) {
		for(i=0; i<count; i++) speed[i] = chSpeedR->GetValue(i);
	} else {
		for(i=0; i<count; i++) speed[i] = Point3::Origin;
	}
	if (chAccel != NULL) {
		for(i=0; i<count; i++) accel[i] = chAccel->GetValue(i);
	} else {
		for(i=0; i<count; i++) accel[i] = Point3::Origin;
	}

	// first, let the given integration to do the job of updating values
	bool res = true;
	if (_givenIntegrator(pCont) != NULL)
		res = _givenIntegrator(pCont)->Proceed(pCont, time);
	
	// second, set integration values of interest
	for(i=0; i<count; i++) {
		float timeSlice = float(time - curTime[i]);
		speed[i] += 0.5f*accel[i]*timeSlice;
		pos[i] += speed[i]*timeSlice;
	}
	if (chPosW != NULL) {
		for(i=0; i<count; i++)
			chPosW->SetValue(i, pos[i]);
	}
	if (chSpeedW != NULL) {
		for(i=0; i<count; i++)
			chSpeedW->SetValue(i, speed[i]);
	}
	chTimeW->SetValue(time);

	return res;
}

//+--------------------------------------------------------------------------+
//|							From IPFIntegrator								 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Proceed(IObject* pCont, Tab<PreciseTimeValue>& times)
{	// fast non-optimized implementation

	if (pCont != NULL) _prevContainer() = pCont;
	else pCont = prevContainer();
	if (pCont == NULL) return false;

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// get/check count info
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count != times.Count()) return false;

	bool res = true;
	for(int i=0; i<count; i++)
		res = res && Proceed(pCont, times[i], i);

	return res;
}

//+--------------------------------------------------------------------------+
//|							From IPFIntegrator								 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Proceed(IObject* pCont, PreciseTimeValue time, BitArray& selected)
{	// fast non-optimized implementation

	if (pCont != NULL) _prevContainer() = pCont;
	else pCont = prevContainer();
	if (pCont == NULL) return false;

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// get/check count info
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count != selected.GetSize()) return false;

	bool res = true;
	for(int i=0; i<count; i++)
		if (selected[i])
			res = res && Proceed(pCont, time, i);

	return res;
}

//+--------------------------------------------------------------------------+
//|							From IPFIntegrator								 |
//+--------------------------------------------------------------------------+
bool PFTestSpawnOnCollision::Proceed(IObject* pCont, Tab<PreciseTimeValue>& times, BitArray& selected)
{	// fast non-optimized implementation

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// get/check count info
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if ((count != times.Count()) || (count != selected.GetSize())) return false;

	bool res = true;
	for(int i=0; i<count; i++)
		if (selected[i])
			res = res && Proceed(pCont, times[i], i);

	return res;
}

int PFTestSpawnOnCollision::getNumDeflectors()
{
	int num = pblock()->Count(kSpawnCollision_deflectors);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kSpawnCollision_deflectors, 0, i) != NULL)
			count++;
	return count;	
}

bool gUpdateSDeflectorsInProgress = false;
bool PFTestSpawnOnCollision::updateFromRealDeflectors()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kSpawnCollision_deflectors, 
				kSpawnCollision_deflectorsMaxscript, gUpdateSDeflectorsInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kSpawnCollision_deflectors, _T("Collision_Nodes"));
	return res;
}

bool PFTestSpawnOnCollision::updateFromMXSDeflectors()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kSpawnCollision_deflectors, 
				kSpawnCollision_deflectorsMaxscript, gUpdateSDeflectorsInProgress, this);
}

bool PFTestSpawnOnCollision::updateObjectNames(int pblockID)
{
	if (pblock() == NULL) return false;
	bool needUpdate = false;
	int count = pblock()->Count(pblockID);
	if (count != objectsToShow().Count()) needUpdate = true;
	if (!needUpdate) {
		for(int i=0; i<count; i++)
			if (pblock()->GetINode(pblockID, 0, i) != objectToShow(i))
			{	needUpdate = true; 	break;	}
	}
	if (!needUpdate) return false;
	_objectsToShow().SetCount(count);
	for(int i=0; i<count; i++)
		_objectToShow(i) = pblock()->GetINode(pblockID, 0, i);
	return true;
}

//+--------------------------------------------------------------------------+
//|							From HitByNameDlgCallback						 |
//+--------------------------------------------------------------------------+
int PFTestSpawnOnCollision::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PFTestSpawnOnCollision::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kSpawnCollision_deflectors, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}

ClassDesc* PFTestSpawnOnCollision::GetClassDesc() const
{
	return GetPFTestSpawnOnCollisionDesc();
}

//+--------------------------------------------------------------------------+
//|							From PFTestSpawnOnCollision					 |
//+--------------------------------------------------------------------------+
void PFTestSpawnOnCollision::RefreshUI(int updateID) const
{
	if ( pblock() == NULL ) return;

	IParamMap2* map = pblock()->GetMap();
	if ( (map == NULL) && (NumPViewParamMaps() == 0) ) return;

	HWND hWnd;
	if (map != NULL) {
		hWnd = map->GetHWnd();
		if ( hWnd ) {
			SendMessage( hWnd, WM_COMMAND, updateID, NULL );
		}
	}

	for(int i=0; i<NumPViewParamMaps(); i++) {
		hWnd = GetPViewParamMap(i)->GetHWnd();
		if (hWnd)
			SendMessage( hWnd, WM_COMMAND, updateID, NULL );
	}
}

void PFTestSpawnOnCollision::UpdatePViewUI(int updateID) const
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