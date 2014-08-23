/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpin.cpp

	DESCRIPTION:	SimpleSpin Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		David C. Thompson

	HISTORY:		created 02-01-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleSpin.h"

#include "PFOperatorSimpleSpin_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

// Creates a transferable particle channel to store an initial vector for each particle
// for speed follow option
#define PARTICLECHANNELINITVECR_INTERFACE Interface_ID(0x16f021f2, 0x1eb34500) 
#define PARTICLECHANNELINITVECW_INTERFACE Interface_ID(0x16f021f2, 0x1eb34501) 
#define GetParticleChannelInitVecRInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELINITVECR_INTERFACE)) 
#define GetParticleChannelInitVecWInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELINITVECW_INTERFACE)) 

PFOperatorSimpleSpin::PFOperatorSimpleSpin()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSimpleSpin::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleSpin_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleSpin_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleSpin_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleSpin::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLESPIN_CLASS_NAME);
}

Class_ID PFOperatorSimpleSpin::ClassID()
{
	return PFOperatorSimpleSpin_Class_ID;
}

void PFOperatorSimpleSpin::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleSpin::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleSpin::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kSimpleSpin_direction:
				case kSimpleSpin_spinrate:
				case kSimpleSpin_variation:
					RefreshUI(kSimpleSpin_message_update);
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;

		case kSimpleSpin_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleSpin::Clone(RemapDir &remap)
{
	PFOperatorSimpleSpin* newOp = new PFOperatorSimpleSpin();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleSpin::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLESPIN_OBJECT_NAME);
}

const ParticleChannelMask& PFOperatorSimpleSpin::ChannelsUsed(const Interval& time) const
{
								//  read						&write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Speed|PCU_Amount, PCG_AngAxis|PCU_Spin);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleSpin::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESPIN_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleSpin::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESPIN_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleSpin::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kSimpleSpin_direction, 0);
	int ids;
	switch(type) {
	case kSS_Rand_3D:			ids = IDS_RAND_3D;			break;
	case kSS_World_Space:		ids = IDS_WORLD_SPACE;		break;
	case kSS_Particle_Space:	ids = IDS_PARTICLE_SPACE;	break;
	case kSS_Speed_Space:		ids = IDS_SPEED_SPACE;		break;
	case kSS_Speed_Space_Follow:ids = IDS_SPEED_FOLLOW;		break;
	}
	nameSuffix = GetString(ids);
	Control* ctrl = pblock()->GetController(kSimpleSpin_spinrate);
	bool isRateAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	ctrl = pblock()->GetController(kSimpleSpin_variation);
	bool isVarAnimated = (ctrl) ? (ctrl->IsAnimated()!=0) : false;
	if ((!isRateAnimated) && (!isVarAnimated)) {
		TCHAR buf[32];
		bool addedSpace = false;
		int rate = int(pblock()->GetFloat(kSimpleSpin_spinrate, 0)*RAD_TO_DEG);
		if (rate != 0) {
			sprintf(buf," %d",rate);
			nameSuffix += buf;
			addedSpace = true;
		}
		int var = int(pblock()->GetFloat(kSimpleSpin_variation, 0)*RAD_TO_DEG);
		if (var != 0) {
			if (!addedSpace) nameSuffix += TSTR(" ");
			buf[0] = BYTE(177);
			buf[1] = 0;
			nameSuffix += buf;
			sprintf(buf,"%d",var);
			nameSuffix += buf;
		}
	}
	return true;
}

bool PFOperatorSimpleSpin::Proceed(IObject* pCont,
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	TimeValue tv = timeEnd.TimeValue();
	int iDir = _pblock()->GetInt(kSimpleSpin_direction, tv);
	bool proceedOld = (iDir == kSS_Speed_Space_Follow);
	bool useSpeed = ((iDir == kSS_Speed_Space) || (iDir == kSS_Speed_Space_Follow));
	// verify that iDir parameter is in range
	switch (iDir) {
		case kSS_Rand_3D: case kSS_World_Space: case kSS_Particle_Space:
		case kSS_Speed_Space: case kSS_Speed_Space_Follow:
			break;
		default: return false;
	}

	// acquire all necessary channels, create additional if needed
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if(chNew == NULL) return false;
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;
	IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
	if((chSpeed == NULL) && useSpeed) return false;
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;

	// there are no new particles
	// if not SpeedFollow then no need to modify anything
	if ((chNew->IsAllOld()) && !proceedOld) return true;

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// ensure presence of the Orientation channel
	bool initOrientation = false;
	IParticleChannelQuatR* chOrientR = (IParticleChannelQuatR*)chCont->EnsureInterface(PARTICLECHANNELORIENTATIONR_INTERFACE,
																ParticleChannelQuat_Class_ID,
																true, PARTICLECHANNELORIENTATIONR_INTERFACE,
																PARTICLECHANNELORIENTATIONW_INTERFACE, true,
																actionNode, NULL, &initOrientation);
	if (chOrientR == NULL) return false; // can't read/create Orientation channel
	IParticleChannelQuatW* chOrientW = GetParticleChannelOrientationWInterface(pCont);
	if (chOrientW == NULL) return false; // can't modify Orientation channel

	// ensure presence of the Rotation Spin channel
	bool initSpin = false;
	IParticleChannelAngAxisR* chSpinR = (IParticleChannelAngAxisR*)chCont->EnsureInterface(PARTICLECHANNELSPINR_INTERFACE,
																ParticleChannelAngAxis_Class_ID,
																true, PARTICLECHANNELSPINR_INTERFACE,
																PARTICLECHANNELSPINW_INTERFACE, true,
																actionNode, NULL, &initSpin);
	if (chSpinR == NULL) return false; // can't read/create Spin channel
	IParticleChannelAngAxisW* chSpinW = GetParticleChannelSpinWInterface(pCont);
	if (chSpinW == NULL) return false; // can't modify Spin channel

	bool initInitVec = false;
	IParticleChannelPoint3R* chInitVecR = NULL;
	IParticleChannelPoint3W* chInitVecW = NULL;
	if (iDir == kSS_Speed_Space_Follow) {
		chInitVecR = (IParticleChannelPoint3R*)chCont->EnsureInterface(PARTICLECHANNELINITVECR_INTERFACE,
																ParticleChannelPoint3_Class_ID,
																true, PARTICLECHANNELINITVECR_INTERFACE,
																PARTICLECHANNELINITVECW_INTERFACE, true,
																actionNode, (Object*)this, &initInitVec);
		if (chInitVecR == NULL) return false; // can't read/create initial vector for speed follow
		chInitVecW = (IParticleChannelPoint3W*)chCont->GetPrivateInterface(PARTICLECHANNELINITVECW_INTERFACE, (Object*)this);
		if (chInitVecW == NULL) return false; // can't modify initial vector for speed follow
	}

	RandGenerator* prg = randLinker().GetRandGenerator(pCont);
	float fRPTScale = 1.0f/TIME_TICKSPERSEC; // conversion units per seconds to units per tick
	float fSpinRate = fRPTScale * GetPFFloat(pblock(), kSimpleSpin_spinrate, tv);
	Point3 pt3RefAxis, pt3SpinAxis, pt3InputAxis;
	pt3InputAxis.x = GetPFFloat(pblock(), kSimpleSpin_X, tv);
	pt3InputAxis.y = GetPFFloat(pblock(), kSimpleSpin_Y, tv);
	pt3InputAxis.z = GetPFFloat(pblock(), kSimpleSpin_Z, tv);
	float fVar = fRPTScale * GetPFFloat(pblock(), kSimpleSpin_variation, tv);
	float fDiv = GetPFFloat(pblock(), kSimpleSpin_divergence, tv);
	bool axisXAnimated = false;
	bool axisYAnimated = false;
	bool axisZAnimated = false;
	bool varAnimated = false;
	bool divAnimated = false;
	bool spinRateAnimated = false;
	Control* ctrl = pblock()->GetController(kSimpleSpin_X);
	if (ctrl != NULL) axisXAnimated = (ctrl->IsAnimated() != 0);
	ctrl = pblock()->GetController(kSimpleSpin_Y);
	if (ctrl != NULL) axisYAnimated = (ctrl->IsAnimated() != 0);
	ctrl = pblock()->GetController(kSimpleSpin_Z);
	if (ctrl != NULL) axisZAnimated = (ctrl->IsAnimated() != 0);
	ctrl = pblock()->GetController(kSimpleSpin_variation);
	if (ctrl != NULL) varAnimated = (ctrl->IsAnimated() != 0);
	ctrl = pblock()->GetController(kSimpleSpin_divergence);
	if (ctrl != NULL) divAnimated = (ctrl->IsAnimated() != 0);
	ctrl = pblock()->GetController(kSimpleSpin_spinrate);
	if (ctrl != NULL) spinRateAnimated = (ctrl->IsAnimated() != 0);
	bool isAnyAnimated = (axisXAnimated || axisYAnimated || axisZAnimated 
							|| varAnimated || divAnimated || spinRateAnimated);

	AngAxis defaultSpin(Point3::ZAxis, 0.0f);
	Quat defaultOrientation(defaultSpin);
	TimeValue curTime = tv;
	Matrix3 tm(TRUE);
	float spinRate;
	AngAxis curSpin;

	int iQuant = chAmount->Count();
	for(int i = 0; i < iQuant; i++) {
		bool isNew = chNew->IsNew(i);
		if (!proceedOld && !isNew) continue;
		if (isNew && initOrientation)
			chOrientW->SetValue(i, defaultOrientation);

		if (isAnyAnimated) curTime = (chTime->GetValue(i)).TimeValue();
		switch(iDir) {
		case kSS_Rand_3D:
			pt3SpinAxis = RandSphereSurface(prg);
			break;
		case kSS_World_Space:
			if (axisXAnimated) pt3InputAxis.x = GetPFFloat(pblock(), kSimpleSpin_X, curTime);
			if (axisYAnimated) pt3InputAxis.y = GetPFFloat(pblock(), kSimpleSpin_Y, curTime);
			if (axisZAnimated) pt3InputAxis.z = GetPFFloat(pblock(), kSimpleSpin_Z, curTime);
			pt3SpinAxis = pt3InputAxis;
			break;
		case kSS_Particle_Space:
			if (axisXAnimated) pt3InputAxis.x = GetPFFloat(pblock(), kSimpleSpin_X, curTime);
			if (axisYAnimated) pt3InputAxis.y = GetPFFloat(pblock(), kSimpleSpin_Y, curTime);
			if (axisZAnimated) pt3InputAxis.z = GetPFFloat(pblock(), kSimpleSpin_Z, curTime);
			tm.SetRotate(chOrientR->GetValue(i));
			pt3SpinAxis = tm.VectorTransform(pt3InputAxis);
			break;
		case kSS_Speed_Space:
			if (axisXAnimated) pt3InputAxis.x = GetPFFloat(pblock(), kSimpleSpin_X, curTime);
			if (axisYAnimated) pt3InputAxis.y = GetPFFloat(pblock(), kSimpleSpin_Y, curTime);
			if (axisZAnimated) pt3InputAxis.z = GetPFFloat(pblock(), kSimpleSpin_Z, curTime);
			tm = SpeedSpaceMatrix(chSpeed->GetValue(i));
			pt3SpinAxis = tm.VectorTransform(pt3InputAxis);
			break;
		case kSS_Speed_Space_Follow:
			if (isNew && initInitVec) {
				if (axisXAnimated) pt3InputAxis.x = GetPFFloat(pblock(), kSimpleSpin_X, curTime);
				if (axisYAnimated) pt3InputAxis.y = GetPFFloat(pblock(), kSimpleSpin_Y, curTime);
				if (axisZAnimated) pt3InputAxis.z = GetPFFloat(pblock(), kSimpleSpin_Z, curTime);
				chInitVecW->SetValue(i, pt3InputAxis);
			} else {
				pt3InputAxis = chInitVecR->GetValue(i);
			}
			tm = SpeedSpaceMatrix(chSpeed->GetValue(i));
			pt3SpinAxis = tm.VectorTransform(pt3InputAxis);
			break;
		}
		
		if (isNew) {
			if (varAnimated) fVar = fRPTScale * GetPFFloat(pblock(), kSimpleSpin_variation, curTime);
			if (spinRateAnimated) fSpinRate = fRPTScale * GetPFFloat(pblock(), kSimpleSpin_spinrate, curTime);
			if(fVar > 0.0f)
				spinRate = fSpinRate + fVar * prg->Rand11();
			else
				spinRate = fSpinRate;
		} else { 
			curSpin = chSpinR->GetValue(i);
			spinRate = curSpin.angle;
		}
		if (iDir != kSS_Speed_Space_Follow) {
			if (divAnimated) fDiv = GetPFFloat(pblock(), kSimpleSpin_divergence, curTime);
			if (fDiv > 0.0f) pt3SpinAxis = DivergeVectorRandom(pt3SpinAxis, prg, fDiv);
		}
		if (LengthSquared(pt3SpinAxis) <= 0.0f) {
			curSpin.axis = Point3::ZAxis;
			curSpin.angle = 0.0f;
		} else {
			curSpin.axis = Normalize(pt3SpinAxis);
			curSpin.angle = spinRate;
		}
		chSpinW->SetValue(i, curSpin);
	}

	return true;
}

ClassDesc* PFOperatorSimpleSpin::GetClassDesc() const
{
	return GetPFOperatorSimpleSpinDesc();
}

int PFOperatorSimpleSpin::GetRand()
{
	return pblock()->GetInt(kSimpleSpin_seed);
}

void PFOperatorSimpleSpin::SetRand(int seed)
{
	_pblock()->SetValue(kSimpleSpin_seed, 0, seed);
}


} // end of namespace PFActions