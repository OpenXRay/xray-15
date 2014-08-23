/**********************************************************************
 *<
	FILE: PFTestCollisionSpaceWarp.h

	DESCRIPTION: CollisionSpaceWarp Test implementation
				 
	CREATED BY: Peter Watje

	HISTORY: created 2-07-02

  hook up random UI


 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFTestCollisionSpaceWarp.h"

#include "PFTestCollisionSpaceWarp_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"
#include "PFSimpleActionState.h"

namespace PFActions {

// CollisionSpaceWarp Test creates a particle channel to store a test value for each particle
// This way the value is calculated only once when a particle enters the event
#define PARTICLECHANNELTESTHITCOUNTR_INTERFACE Interface_ID(0x61324c5c, 0x1eba5600) 
#define PARTICLECHANNELTESTHITCOUNTW_INTERFACE Interface_ID(0x61324c5c, 0x1eba5601) 

#define GetParticleChannelTestHitCountRInterface(obj) ((IParticleChannelIntR*)obj->GetInterface(PARTICLECHANNELTESTHITCOUNTR_INTERFACE)) 
#define GetParticleChannelTestHitCountWInterface(obj) ((IParticleChannelIntW*)obj->GetInterface(PARTICLECHANNELTESTHITCOUNTW_INTERFACE)) 

// static members
Object*		PFTestCollisionSpaceWarp::m_editOb				= NULL;
IObjParam*	PFTestCollisionSpaceWarp::m_ip					= NULL;

CollisionSpaceWarpValidatorClass PFTestCollisionSpaceWarp::validator;
bool PFTestCollisionSpaceWarp::validatorInitiated = false;

// constructors/destructors
PFTestCollisionSpaceWarp::PFTestCollisionSpaceWarp()
				:IPFTest()
{ 
	RegisterParticleFlowNotification();
	_pblock() = NULL;
	_numDeflectors() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kCollisionSpaceWarp_CollisionNodelist;
		collision_paramBlockDesc.ParamOption(kCollisionSpaceWarp_CollisionNodelist,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this); 
	_activeIcon() = _trueIcon() = _falseIcon() = NULL;
}

PFTestCollisionSpaceWarp::~PFTestCollisionSpaceWarp()
{
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();
	DeleteAllRefsFromMe();
	if (wasHolding) theHold.Resume();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From InterfaceServer									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BaseInterface* PFTestCollisionSpaceWarp::GetInterface(Interface_ID id)
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
FPInterfaceDesc* PFTestCollisionSpaceWarp::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &collision_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &collision_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &collision_PViewItem_FPInterfaceDesc;
	if (id == PFINTEGRATOR_INTERFACE) return &collision_integrator_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestCollisionSpaceWarp::DeleteThis()
{
	delete this;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_COLLISIONSPACEWARP_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestCollisionSpaceWarp::ClassID()
{
	return PFTestCollisionSpaceWarp_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	_ip() = ip; _editOb() = this;
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	_ip() = NULL; _editOb() = NULL;
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFTestCollisionSpaceWarp::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFTestCollisionSpaceWarp::SubAnimName(int i)
{
	return PFActions::GetString(IDS_PARAMETERS);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFTestCollisionSpaceWarp::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestCollisionSpaceWarp::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFTestCollisionSpaceWarp::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFTestCollisionSpaceWarp::GetReference(int i)
{
	if (i==0) return (RefTargetHandle)pblock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==0) _pblock() = (IParamBlock2*)rtarg;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefResult PFTestCollisionSpaceWarp::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if (pblock() == hTarget) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kCollisionSpaceWarp_CollisionsMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSDeflectors();
					return REF_STOP;
				}
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch ( lastUpdateID )
				{
				case kCollisionSpaceWarp_CollisionTestType:
				case kCollisionSpaceWarp_CollisionSpeedOption:
				case kCollisionSpaceWarp_CollisionCountOptions:
					RefreshUI(kCollisionSpaceWarp_message_type);
					break;
				case kCollisionSpaceWarp_CollisionNodelist:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealDeflectors();
					if (updateObjectNames(kCollisionSpaceWarp_CollisionNodelist)) {
						RefreshUI(kCollisionSpaceWarp_message_deflectors);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				}
				UpdatePViewUI(pblock()->LastNotifyParamID());
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			UpdatePViewUI(kCollisionSpaceWarp_CollisionNodelist);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealDeflectors();
			break;
		// Initialization of Dialog
		case kCollisionSpaceWarp_RefMsg_InitDlg:
			RefreshUI(kCollisionSpaceWarp_message_type, (IParamMap2*)partID);
			RefreshUI(kCollisionSpaceWarp_message_deflectors, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kCollisionSpaceWarp_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kCollisionSpaceWarp_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kCollisionSpaceWarp_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFTestCollisionSpaceWarp::Clone(RemapDir &remap)
{
	PFTestCollisionSpaceWarp* newTest = new PFTestCollisionSpaceWarp();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
int PFTestCollisionSpaceWarp::RemapRefOnLoad(int iref)
{
	return iref;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestCollisionSpaceWarp::Save(ISave *isave)
{
	return IO_OK;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
IOResult PFTestCollisionSpaceWarp::Load(ILoad *iload)
{
	return IO_OK;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestCollisionSpaceWarp::GetObjectName()
{
	return GetString(IDS_TEST_COLLISIONSPACEWARP_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestCollisionSpaceWarp::Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes)
{
	_givenIntegrator(pCont) = NULL;
	return _randLinker().Init( pCont, GetRand() );
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFTestCollisionSpaceWarp::Release(IObject* pCont)
{
	_givenIntegrator(pCont) = NULL;
	_randLinker().Release( pCont );
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
int	PFTestCollisionSpaceWarp::GetRand()
{
	return pblock()->GetInt(kCollisionSpaceWarp_randomSeed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::SetRand(int seed)
{
	pblock()->SetValue(kCollisionSpaceWarp_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
IObject* PFTestCollisionSpaceWarp::GetCurrentState(IObject* pContainer)
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
void PFTestCollisionSpaceWarp::SetCurrentState(IObject* aSt, IObject* pContainer)
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
const ParticleChannelMask& PFTestCollisionSpaceWarp::ChannelsUsed(const Interval& time) const
{
								//  read									  &	write channels
	static ParticleChannelMask mask(PCU_ID|PCU_Time|PCU_Position|PCU_Amount|PCU_Speed|PCU_BirthTime|PCU_New|PCU_Acceleration, 
									PCU_Speed|PCU_Position);
	static bool maskSet(false);
	if (!maskSet)
	{
		maskSet = true;
		mask.AddChannel(PARTICLECHANNELTESTHITCOUNTR_INTERFACE); // read channel
		mask.AddChannel(PARTICLECHANNELTESTHITCOUNTW_INTERFACE); // write channel
	}
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestCollisionSpaceWarp::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_COLLISIONSPACEWARP_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestCollisionSpaceWarp::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_COLLISIONSPACEWARP_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestCollisionSpaceWarp::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_COLLISIONSPACEWARP_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFTestCollisionSpaceWarp::HasDynamicName(TSTR& nameSuffix)
{
	int num = pblock()->Count(kCollisionSpaceWarp_CollisionNodelist);
	int count=0, firstIndex=-1;
	for(int i=0; i<num; i++) {
		if (pblock()->GetINode(kCollisionSpaceWarp_CollisionNodelist, 0, i) != NULL) {
			count++;
			if (firstIndex < 0) firstIndex = i;
		}
	}
	if (count > 0) {
		INode* force = pblock()->GetINode(kCollisionSpaceWarp_CollisionNodelist, 0, firstIndex);
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
bool PFTestCollisionSpaceWarp::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
//	DebugPrint("Collision Calc Start start = %d   end = %d \n", TimeValue(timeStart), TimeValue(timeEnd));
	// grab integrator
	if (integrator == NULL) return false;
	if ((integrator != (IPFIntegrator*)this) && (!integrator->HasEncapsulatedIntegrator((IPFIntegrator*)this)))
		_givenIntegrator(pCont) = integrator;

	// get channel container interface
	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
	if (chID == NULL) return false; // can't find particle id
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing info for a particle
	IParticleChannelPTVW* chTimeW = GetParticleChannelTimeWInterface(pCont);
	if (chTimeW == NULL) return false; // can't modify timing info for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	IParticleChannelPTVR* chAge = GetParticleChannelBirthTimeRInterface(pCont);
	if(chAge == NULL) return false;

	// the channel of interest: position
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	IParticleChannelPoint3W* chPosW = GetParticleChannelPositionWInterface(pCont);
	if ((chPos == NULL) || (chPosW == NULL)) return false; // particle position is underfined

	// the channel of interest: speed
	bool initSpeed = false;
	//channel does not exist so make it and note that we have to fill it out
	IParticleChannelPoint3W* chSpeedW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELSPEEDR_INTERFACE,
																			PARTICLECHANNELSPEEDW_INTERFACE, true,
																			actionNode, NULL, &initSpeed);
	IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
	if ((chSpeed == NULL) || (chSpeedW == NULL)) return false; // particle speed is underfined

	int i, iQuant = chAmount->Count();
	if (iQuant <= 0) return true; // no particles here

	bool hasNextEvent = (testTime.Count() > 0);
	testTime.SetCount(iQuant);
	testResult.SetSize(iQuant);
	testResult.ClearAll();

	// set particle speed to zero for particles without any speed info
	if (initSpeed) {
		for(i=0; i<iQuant; i++)
			if (chNew->IsNew(i))
				chSpeedW->SetValue(i, Point3::Origin);
	}

	IParticleChannelPoint3R* chAccel = GetParticleChannelAccelerationRInterface(pCont);
	RandGenerator* prg = randLinker().GetRandGenerator(pCont);

	// check if we have valid deflectors
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend(); // to cloak creation of collision objects
	Tab<CollisionObject*> collisionObjs;
	for(i=0; i<pblock()->Count(kCollisionSpaceWarp_CollisionNodelist); i++) {
		INode* node = pblock()->GetINode(kCollisionSpaceWarp_CollisionNodelist, timeEnd, i);
		if (node == NULL) continue;
		Object* obj = GetPFObject(node->GetObjectRef());
		if (obj == NULL) continue;
		int id = (obj?obj->SuperClassID():SClass_ID(0));
		if (id != WSM_OBJECT_CLASS_ID) continue;
		WSMObject* obref = (WSMObject*)obj;
		CollisionObject* colObj = obref->GetCollisionObject(node);
		if (colObj != NULL) {
			colObj->SetRandSeed(prg->Rand0X(RAND_MAX-1));
			collisionObjs.Append(1, &colObj);
		}
	}
	if (wasHolding) theHold.Resume();
	if (collisionObjs.Count() == 0) return true; // no deflectors to hit on
	if (wasHolding) theHold.Suspend(); // the collision object can be created in the CheckCollision call
	CollisionCollection cc;
	cc.Init(collisionObjs);

	int testType;
	_pblock()->GetValue(kCollisionSpaceWarp_CollisionTestType,0,testType,FOREVER);
	int willCollideFrame = _pblock()->GetTimeValue(kCollisionSpaceWarp_CollisionWillCollideFrame,0);
	// bayboro August 1 2002
	// this is a work-around to avoid "under-initialized" collisionObject
	// if the first "dt" value that is passed to collisionObject is low
	// then the collisionObject is initialized for the interval [timeEnd-dt, timeEnd]
	// that creates a problem when a later particle is passed with a higher "dt" value
	// therefore it is necessary to perform a 'fake' collision check for the whole time interval
	float fakeHitTime;
	Point3 fakePosition = Point3::Origin;
	Point3 fakeSpeed = Point3::Origin;
	if (testType == 4) { // will collide
		cc.CheckCollision((int)timeEnd+willCollideFrame, fakePosition, fakeSpeed, 
							(int)timeEnd-(int)timeStart+1+willCollideFrame, 0, &fakeHitTime, FALSE);
	} else {
		cc.CheckCollision((int)timeEnd, fakePosition, fakeSpeed, (int)timeEnd-(int)timeStart, 0, &fakeHitTime, FALSE);
	}

	int speedOption;
	_pblock()->GetValue(kCollisionSpaceWarp_CollisionSpeedOption,0,speedOption,FOREVER);

	float minSpeed = pblock()->GetFloat(kCollisionSpaceWarp_CollisionMinSpeed,0)/TIME_TICKSPERSEC;
	float minSpeedSquared = minSpeed*minSpeed;
	float maxSpeed = pblock()->GetFloat(kCollisionSpaceWarp_CollisionMaxSpeed,0)/TIME_TICKSPERSEC;
	float maxSpeedSquared = maxSpeed*maxSpeed;

	int hitCount = _pblock()->GetInt(kCollisionSpaceWarp_CollisionCount,0);
	int hitCountOption = _pblock()->GetInt(kCollisionSpaceWarp_CollisionCountOptions,0);

	// acquire TestDuration particle channel; if not present then create it		
	IParticleChannelIntR* chHitCountR = NULL;
	IParticleChannelIntW* chHitCountW = NULL;
	bool initHitCount = false;
	// need to create a collision hits channel if we have colision hit count test on
	// the data are transferred for instances of the test to use
	// therefore if there are two instances of the same test then those hits are
	// accumulated by both instances
	if (testType == 3) {
		chHitCountW = (IParticleChannelIntW*)chCont->EnsureInterface(PARTICLECHANNELTESTHITCOUNTW_INTERFACE,
												ParticleChannelInt_Class_ID,
												true, PARTICLECHANNELTESTHITCOUNTR_INTERFACE,
												PARTICLECHANNELTESTHITCOUNTW_INTERFACE, true,
												actionNode, (Object*)this, &initHitCount);
		chHitCountR = (IParticleChannelIntR*)chCont->GetPrivateInterface(PARTICLECHANNELTESTHITCOUNTR_INTERFACE, (Object*)this);
		if ((chHitCountR == NULL) || (chHitCountW == NULL)) return false; // can't set test value for newly entered particles
	}			

	Tab<Point3> hitList;		//List of position of points after collision used to put 
								//particles in there final position after the last collision
	Tab<Point3> velList;		//List of particle velocities atfter collision,used
								// restore their last velocity
	float influence = 0.0f;
					
	PreciseTimeValue curParticleTime;
	hitList.SetCount(iQuant);
	velList.SetCount(iQuant);
										//this will cause leaks with moving deflectors

	int MAXCOLLISIONS = cc.MAX_COLLISIONS_PER_STEP;	//this is the maximu, number of collisions per time slice
													//this really should be a UI element 									

	if (testType == 4)					//On these 2 type of test types the collision pos and velocity are ignored
		MAXCOLLISIONS = 1;				//so we only need to do one pass
	if ((testType == 0) && (speedOption == kCollisionSpaceWarp_SpeedContinue))
		MAXCOLLISIONS = 1;

	BitArray particlesThatHit;			//this is the a list of all particles that collided so we know which ones
	particlesThatHit.SetSize(iQuant);	//to put back there proper positions and velocity
	particlesThatHit.ClearAll();

	Tab<PreciseTimeValue> timeList;	
	timeList.SetCount(iQuant);

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	for(i = 0; i < iQuant; i++) //load up our initial pos/vel/ and time values
	{
		if (chNew->IsNew(i) && initHitCount)
			chHitCountW->SetValue(i, 0);

		hitList[i] = chPos->GetValue(i);
		velList[i] = chSpeed->GetValue(i);
		timeList[i] = chTime->GetValue(i);
	}


	PreciseTimeValue holdEndTime;
	if (testType == 4)  //will collide condition we need to fiddle with the dt and times
	{
		holdEndTime = timeEnd;
		timeEnd += willCollideFrame;  //need to expand our time slice
		if (timeEnd < holdEndTime) timeEnd = holdEndTime;
	}

//	DebugPrint("Collision Calc Start EndTime = %d   Count = %d \n", (int)timeEnd, iQuant);

	for(i = 0; i < iQuant; i++) 
	{
		curParticleTime = chTime->GetValue(i);
		Point3 p = hitList[i];
		Point3 v = velList[i];

		float timeSlice = float(timeEnd - curParticleTime);
		if (timeSlice <= 0.0f) continue; // particle doesn't have any time to travel

		if (chAccel)  //this is a fudge old collision engines only used velocity, so if an acceleration channel is present we use it + the velocity channel 
					 //to get an average velocity across the time slice
		{
			Point3 accel = chAccel->GetValue(i);
			Point3 pDelta = (v * timeSlice) + (0.5f * accel * timeSlice * timeSlice);
			v = pDelta/timeSlice;
		}

		float dt = timeSlice;
		float hitTime  = 0.0f;
		int bornID = chID->GetParticleBorn(i);
		for (int j=0; j<MAXCOLLISIONS; j++)
		{
			if (dt <= 0.0f) break;
			Point3 speedBefore = v;
			Point3 posBefore = p;

			BOOL hit = cc.CheckCollision(timeEnd, p, v, dt, bornID, &hitTime, FALSE);
			if (hit) {
				particlesThatHit.Set(i, TRUE); //mark this particle as hitting the deflector
			} else {
				break;
			}

			dt -= hitTime;
			particlesThatHit.Set(i,TRUE);  //mark this particle as hitting the deflector
			hitList[i] = p;
			velList[i] = v;
			timeList[i] += hitTime;
		
			if (testType == 0) { // straight up collision
				switch(speedOption) {
				case kCollisionSpaceWarp_SpeedContinue:
					velList[i] = speedBefore;
					break;
				case kCollisionSpaceWarp_SpeedStop:
					velList[i] = Point3::Origin;
					break;
				case kCollisionSpaceWarp_SpeedRandom:
					velList[i] = Length(v) * RandSphereSurface(prg);
					break;
				}
				testResult.Set(i, TRUE);
				if (hasNextEvent) break;
			}

			if (testType == 1) { // min speed test
				if (LengthSquared(v) < minSpeedSquared) {
					testResult.Set(i, TRUE);
					if (hasNextEvent) break;
				}
			}

			if (testType == 2) { // max speed test
				if (LengthSquared(v) > maxSpeedSquared) {
					testResult.Set(i, TRUE);
					if (hasNextEvent) break;
				}
			}

			if (testType == 3) { // multiple collisions
				int totalHitCount = chHitCountR->GetValue(i);
				totalHitCount++;
				chHitCountW->SetValue(i, totalHitCount);
				if (totalHitCount >= hitCount)
				{
					testResult.Set(i,TRUE);
					switch(hitCountOption) {
					case kCollisionSpaceWarp_SpeedContinue:
						velList[i] = speedBefore;
						break;
					case kCollisionSpaceWarp_SpeedStop:
						velList[i] = Point3::Origin;
						break;
					case kCollisionSpaceWarp_SpeedRandom:
						velList[i] = Length(v) * RandSphereSurface(prg);
						break;
					}
					testResult.Set(i, TRUE);
					if (hasNextEvent) break;
				}
			}

			if (testType == 4) { // will collide
				timeList[i] -= hitTime;
				if (hitTime <= willCollideFrame) {
					hitTime = 0.0f;
				} else {
					hitTime -= willCollideFrame;
				}
				timeList[i] += hitTime;
				hitList[i] = posBefore + hitTime*speedBefore;
				velList[i] = speedBefore;
				testResult.Set(i, TRUE);
			}
		}
	}
/*
	if (hasNextEvent) {
		if (integrator != NULL)  //this integrates all our particles that collided to their end times
		{
			Tab<PreciseTimeValue> timeToAdvance;
			timeToAdvance.SetCount(iQuant);
			for(i=0; i<iQuant; i++)
			{
				if (testResult[i]) {
					timeToAdvance[i] = timeList[i];//testTime[i];  this is another fudge since the deflector integrator and the
											//pf intergrator compute particle differently.   The pf integrator may
											// put the particle in a spot different than the deflection engine which
											//may invalidate our hit point.  The solution is to integrate the particle
											//to the end of the time slice using the PF, then put the particle back
											//where the deflection engine wants it,
											//Draw back to this is that any operators downstream will be ignored for this 
											//time slice since the particle has already been intergrated forward
				} else {
					timeToAdvance[i] = timeEnd;
				}
			}
			integrator->Proceed(pCont, timeToAdvance);
		}
		for(i=0; i<iQuant; i++) //we need to also update the velocity direction of those particles that collided
		{
			if (testResult[i]) {
				chPosW->SetValue(i, hitList[i]); //this puts back the particles to where the deflection engine thinks they should be
				chSpeedW->SetValue(i, velList[i]);
				testTime[i] = float(timeList[i] - timeStart);
			} else {
				chPosW->SetValue(i, hitList[i] + velList[i]*float(timeEnd - timeList[i]));
				chSpeedW->SetValue(i, velList[i]);
			}
		}
	} else {
		if (integrator != NULL)  //this integrates all our particles that collided to their end times
			integrator->Proceed(pCont, timeEnd);
		for(i=0; i<iQuant; i++) {
			chPosW->SetValue(i, hitList[i] + velList[i]*float(timeEnd - timeList[i]));
			chSpeedW->SetValue(i, velList[i]);
			if (testResult[i]) testTime[i] = float(timeList[i] - timeStart);
		}
	}
*/

	if (integrator != NULL)
	{
		Tab<PreciseTimeValue> timeToAdvance;
		timeToAdvance.SetCount(iQuant);
		for(i=0; i<iQuant; i++)
		{
			if (particlesThatHit[i]) {
				timeToAdvance[i] = timeList[i];//testTime[i];  this is another fudge since the deflector integrator and the
											//pf intergrator compute particle differently.   The pf integrator may
											// put the particle in a spot different than the deflection engine which
											//may invalidate our hit point.  The solution is to integrate the particle
											//to the end of the time slice using the PF, then put the particle back
											//where the deflection engine wants it,
											//Draw back to this is that any operators downstream will be ignored for this 
											//time slice since the particle has already been intergrated forward
			}
		}
		integrator->Proceed(pCont, timeToAdvance, particlesThatHit);
	}
	for(i=0; i<iQuant; i++) //we need to also update the velocity direction of those particles that collided
	{
		if (particlesThatHit[i]) {
			chPosW->SetValue(i, hitList[i]); //this puts back the particles to where the deflection engine thinks they should be
			chSpeedW->SetValue(i, velList[i]);
			testTime[i] = float(timeList[i] - timeStart);
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

	if (testType == 4)  //will collide condition we need to restore the end time
		timeEnd = holdEndTime;

//	DebugPrint("Collision Calc End\n");

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

//+--------------------------------------------------------------------------+
//|							From IPFTest									 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::ProceedStep1(IObject* pCont, 
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
bool PFTestCollisionSpaceWarp::ProceedStep2(TimeValue timeStartTick, float timeStartFraction, TimeValue& timeEndTick, float& timeEndFraction, BitArray& testResult, Tab<float>& testTime)
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
//|							From PFTestCollisionSpaceWarp								 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::RefreshUI(WPARAM message, IParamMap2* map) const
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
//|							From PFTestCollisionSpaceWarp					 |
//+--------------------------------------------------------------------------+
void PFTestCollisionSpaceWarp::UpdatePViewUI(int updateID) const
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		map->Invalidate(updateID);
		Interval currentTimeInterval;
		currentTimeInterval.SetInstant(GetCOREInterface()->GetTime());
		map->Validity() = currentTimeInterval;
	}
}

//+--------------------------------------------------------------------------+
//|							From IPFIntegrator								 |
//+--------------------------------------------------------------------------+
bool PFTestCollisionSpaceWarp::Proceed(IObject* pCont, PreciseTimeValue time, int index)
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
// 	speed += 0.5f*accel*timeSlice; // collision integrator is strictly linear
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
bool PFTestCollisionSpaceWarp::Proceed(IObject* pCont, PreciseTimeValue time)
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
//		speed[i] += 0.5f*accel[i]*timeSlice; // collision integrator is strictly linear
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
bool PFTestCollisionSpaceWarp::Proceed(IObject* pCont, Tab<PreciseTimeValue>& times)
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
bool PFTestCollisionSpaceWarp::Proceed(IObject* pCont, PreciseTimeValue time, BitArray& selected)
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
bool PFTestCollisionSpaceWarp::Proceed(IObject* pCont, Tab<PreciseTimeValue>& times, BitArray& selected)
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
	if ((count != times.Count()) || (count != selected.GetSize())) return false;

	bool res = true;
	for(int i=0; i<count; i++)
		if (selected[i])
			res = res && Proceed(pCont, times[i], i);

	return res;
}

int PFTestCollisionSpaceWarp::getNumDeflectors()
{
	int num = pblock()->Count(kCollisionSpaceWarp_CollisionNodelist);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kCollisionSpaceWarp_CollisionNodelist, 0, i) != NULL)
			count++;
	return count;	
}

bool gUpdateDeflectorsInProgress = false;
bool PFTestCollisionSpaceWarp::updateFromRealDeflectors()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kCollisionSpaceWarp_CollisionNodelist, 
				kCollisionSpaceWarp_CollisionsMaxscript, gUpdateDeflectorsInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kCollisionSpaceWarp_CollisionNodelist, _T("Collision_Nodes"));
	return res;
}

bool PFTestCollisionSpaceWarp::updateFromMXSDeflectors()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kCollisionSpaceWarp_CollisionNodelist, 
				kCollisionSpaceWarp_CollisionsMaxscript, gUpdateDeflectorsInProgress, this);
}

bool PFTestCollisionSpaceWarp::updateObjectNames(int pblockID)
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
int PFTestCollisionSpaceWarp::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PFTestCollisionSpaceWarp::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kCollisionSpaceWarp_CollisionNodelist, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}


ClassDesc* PFTestCollisionSpaceWarp::GetClassDesc() const
{
	return GetPFTestCollisionSpaceWarpDesc();
}



} // end of namespace PFActions