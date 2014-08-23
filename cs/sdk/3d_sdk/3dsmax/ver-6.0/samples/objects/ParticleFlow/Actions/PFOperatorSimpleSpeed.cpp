/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpeed.cpp

	DESCRIPTION:	SimpleSpeed Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		David C. Thompson

	HISTORY:		created 01-15-02
					modified 07-23-02 [o.bayborodin]

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleSpeed.h"

#include "PFOperatorSimpleSpeed_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

PFOperatorSimpleSpeed::PFOperatorSimpleSpeed()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSimpleSpeed::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleSpeed_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleSpeed_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleSpeed_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleSpeed::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLESPEED_CLASS_NAME);
}

Class_ID PFOperatorSimpleSpeed::ClassID()
{
	return PFOperatorSimpleSpeed_Class_ID;
}

void PFOperatorSimpleSpeed::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleSpeed::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleSpeed::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kSimpleSpeed_direction)
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case kSimpleSpeed_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleSpeed::Clone(RemapDir &remap)
{
	PFOperatorSimpleSpeed* newOp = new PFOperatorSimpleSpeed();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleSpeed::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLESPEED_OBJECT_NAME);
}

const ParticleChannelMask& PFOperatorSimpleSpeed::ChannelsUsed(const Interval& time) const
{
								//  read						 &	write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time|PCU_Position|PCU_Amount, PCU_Speed);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleSpeed::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESPEED_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleSpeed::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLESPEED_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleSpeed::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kSimpleSpeed_direction, 0);
	int ids;
	switch(type) {
	case kSS_Along_Icon_Arrow:	ids = IDS_ALONG_ICON_ARROW;	break;
	case kSS_Icon_Center_Out:	ids = IDS_ICON_CENTER_0UT;	break;
	case kSS_Icon_Arrow_Out:	ids = IDS_ICON_ARROW_OUT;	break;
	case kSS_Rand_3D:			ids = IDS_RAND_3D;			break;
	case kSS_Rand_Horiz:		ids = IDS_RAND_HORIZ;		break;
	case kSS_Inherit_Prev:		ids = IDS_INHERIT_PREV;		break;
	}
	nameSuffix = GetString(ids);
	return true;
}

bool PFOperatorSimpleSpeed::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if(chNew == NULL) return false;
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;
	// the position channel may not be present. For some option configurations it is okay
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	int iDir = _pblock()->GetInt(kSimpleSpeed_direction, timeStart);
	if ((chPos == NULL) && ((iDir == kSS_Icon_Center_Out) || (iDir == kSS_Icon_Arrow_Out)))
		return false;

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// the channel of interest
	bool initSpeed = false;
	IParticleChannelPoint3W* chSpeed = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELSPEEDR_INTERFACE,
																			PARTICLECHANNELSPEEDW_INTERFACE, true,
																			actionNode, (Object*)NULL, &initSpeed);
	IParticleChannelPoint3R* chSpeedR = GetParticleChannelSpeedRInterface(pCont);
	if ((chSpeed == NULL) || (chSpeedR == NULL)) return false;

	// there are no new particles
	if (chNew->IsAllOld()) return true;

	float fUPFScale = 1.0f/TIME_TICKSPERSEC; // conversion units per seconds to units per tick
	Point3 pt3SpeedVec;
	RandGenerator* prg = randLinker().GetRandGenerator(pCont);
	int iQuant = chAmount->Count();
	bool wasIgnoringEmitterTMChange = IsIgnoringEmitterTMChange();
	if (!wasIgnoringEmitterTMChange) SetIgnoreEmitterTMChange();
	for(int i = 0; i < iQuant; i++) {
		if(chNew->IsNew(i)) { // apply only to new particles
			TimeValue tv = chTime->GetValue(i).TimeValue();
			Matrix3 nodeTM = pNode->GetObjectTM(tv);
			float fSpeedParam = fUPFScale * GetPFFloat(pblock(), kSimpleSpeed_speed, tv);
			// change speed in user selected direction
			switch(iDir) {
				case kSS_Along_Icon_Arrow: {
						// icon arrow appears to be in the negative z direction
						pt3SpeedVec = -Normalize(nodeTM.GetRow(2));
					}
					break;
				case kSS_Icon_Center_Out: {
						Point3 pt3IconCenter = nodeTM.GetTrans();
						Point3 pt3PartPos = chPos->GetValue(i);
						pt3SpeedVec = Normalize(pt3PartPos - pt3IconCenter);
					}
					break;
				case kSS_Icon_Arrow_Out: {
						Point3 pt3PartPos = chPos->GetValue(i);
						Point3 pt3ArrowVec = nodeTM.GetRow(2);
						Point3 pt3Tmp = CrossProd(pt3PartPos - nodeTM.GetTrans(), pt3ArrowVec);
						pt3SpeedVec = Normalize(CrossProd(pt3ArrowVec, pt3Tmp));
					}
					break;
				case kSS_Rand_3D: {
						pt3SpeedVec = RandSphereSurface(prg);
					}
					break;
				case kSS_Rand_Horiz: {
						float fAng = TWOPI * prg->Rand01();
						// establish x, y coordinates of random angle, z component zero
						float x = cos(fAng); float y = sin(fAng); float z = 0.0f;
						pt3SpeedVec = Point3(x, y, z);
					}
					break;
				case kSS_Inherit_Prev: {
						if (initSpeed) 
							pt3SpeedVec = Point3::Origin;
						else
							pt3SpeedVec = Normalize(chSpeedR->GetValue(i));
					}
					break;
			}
			// account for reverse check box
			int iRev = _pblock()->GetInt(kSimpleSpeed_reverse, 0);
			float fDirMult = iRev > 0 ? -1.f : 1.f;
			// calculate variation
			float fVar = fUPFScale * GetPFFloat(pblock(), kSimpleSpeed_variation, tv);
			if(fVar > 0.f)
				fSpeedParam = fSpeedParam + fVar * prg->Rand11();
			pt3SpeedVec = fDirMult * fSpeedParam * pt3SpeedVec;
			// calculate divergence
			float fDiv = GetPFFloat(pblock(), kSimpleSpeed_divergence, tv);
			pt3SpeedVec = DivergeVectorRandom(pt3SpeedVec, prg, fDiv);

			chSpeed->SetValue(i, pt3SpeedVec);
		}
	}
	if (!wasIgnoringEmitterTMChange) ClearIgnoreEmitterTMChange();

	return true;
}

ClassDesc* PFOperatorSimpleSpeed::GetClassDesc() const
{
	return GetPFOperatorSimpleSpeedDesc();
}

int PFOperatorSimpleSpeed::GetRand()
{
	return pblock()->GetInt(kSimpleSpeed_seed);
}

void PFOperatorSimpleSpeed::SetRand(int seed)
{
	_pblock()->SetValue(kSimpleSpeed_seed, 0, seed);
}

} // end of namespace PFActions