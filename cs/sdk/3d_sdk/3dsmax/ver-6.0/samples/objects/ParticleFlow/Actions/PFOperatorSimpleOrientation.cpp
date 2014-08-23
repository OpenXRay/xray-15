/**********************************************************************
 *<
	FILE:			PFOperatorSimpleOrientation.cpp

	DESCRIPTION:	SimpleOrientation Operator implementation
					Operator to set initial orientation of particles

	CREATED BY:		David C. Thompson

	HISTORY:		created 01-24-02
					modified 07-23-02 [o.bayborodin]

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleOrientation.h"

#include "PFOperatorSimpleOrientation_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

PFOperatorSimpleOrientation::PFOperatorSimpleOrientation()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorSimpleOrientation::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleOrientation_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleOrientation_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleOrientation_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleOrientation::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLEORIENTATION_CLASS_NAME);
}

Class_ID PFOperatorSimpleOrientation::ClassID()
{
	return PFOperatorSimpleOrientation_Class_ID;
}

void PFOperatorSimpleOrientation::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleOrientation::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleOrientation::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	IParamMap2* map = NULL;

	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch(lastUpdateID) {
				case kSimpleOrientation_direction:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					// break is omitted on purpose (bayboro 6/18/2003)
				case kSimpleOrientation_restrictToAxis:
					RefreshUI(kSimpleOrientation_message_update);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		case kSimpleOrientation_RefMsg_Init:
			map = (IParamMap2*)partID;
			RefreshUI(kSimpleOrientation_message_update, map);
			return REF_STOP;
		case kSimpleOrientation_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}
	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleOrientation::Clone(RemapDir &remap)
{
	PFOperatorSimpleOrientation* newOp = new PFOperatorSimpleOrientation();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleOrientation::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLEORIENTATION_OBJECT_NAME);
}

const ParticleChannelMask& PFOperatorSimpleOrientation::ChannelsUsed(const Interval& time) const
{
								//  read			&	write channels
	static ParticleChannelMask mask(PCU_New|PCU_Time, PCU_Orientation);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleOrientation::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEORIENTATION_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleOrientation::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEORIENTATION_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleOrientation::HasDynamicName(TSTR& nameSuffix)
{
	int type = pblock()->GetInt(kSimpleOrientation_direction, 0);
	int ids;
	switch(type) {
	case kSO_Rand_3D:		ids = IDS_RAND_3D;		break;
	case kSO_Rand_Horiz:	ids = IDS_RAND_HORIZ;	break;
	case kSO_World:			ids = IDS_WORLD_SPACE;	break;
	case kSO_Speed:			ids = IDS_SPEED_SPACE;	break;
	case kSO_SpeedFollow:	ids = IDS_SPEED_FOLLOW;	break;
	}
	nameSuffix = GetString(ids);
	return true;
}

bool PFOperatorSimpleOrientation::Proceed(IObject* pCont, 
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

	// there are no new particles
//	if (chNew->IsAllOld()) return true;

	// we may need speed channel for "Speed Space" and "Speed Space Follow" types
	IParticleChannelPoint3R* chSpeed;
	int iDir = pblock()->GetInt(kSimpleOrientation_direction, 0);
	if ((iDir == kSO_Speed) || (iDir == kSO_SpeedFollow))
		chSpeed = GetParticleChannelSpeedRInterface(pCont);
	bool bRestrictToAxis = (pblock()->GetInt(kSimpleOrientation_restrictToAxis, 0) != 0);

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	// the channel of interest
	IParticleChannelQuatW* chOrient = (IParticleChannelQuatW*)chCont->EnsureInterface(PARTICLECHANNELORIENTATIONW_INTERFACE,
																			ParticleChannelQuat_Class_ID,
																			true, PARTICLECHANNELORIENTATIONR_INTERFACE,
																			PARTICLECHANNELORIENTATIONW_INTERFACE, true );
	if (chOrient == NULL) return false;

	RandGenerator* prg = randLinker().GetRandGenerator(pCont);
	Matrix3 m3Orient;
	float fEulerAng[3];
	int iQuant = chAmount->Count();
	for(int i = 0; i < iQuant; i++) {
		if((chNew->IsNew(i)) || (iDir == kSO_SpeedFollow)) { // apply only to new particles or to all for "follow" type
			TimeValue tv = chTime->GetValue(i).TimeValue();
			// set particle direction in user selected direction
			switch(iDir) {
				case kSO_Rand_3D: {
						Point3 p3x = RandSphereSurface(prg);
						Point3 p3y = RandSphereSurface(prg);
						while(p3x == p3y)
							p3y = RandSphereSurface(prg);
						p3y = Normalize(p3y - p3x * DotProd(p3x, p3y));
						Point3 p3z = p3x ^ p3y;
						m3Orient = Matrix3(p3x, p3y, p3z, Point3::Origin);
					}
					break;
				case kSO_Rand_Horiz: {
						fEulerAng[0] = fEulerAng[1] = 0.0f;
						fEulerAng[2] = TWOPI * prg->Rand01();
						EulerToMatrix(fEulerAng, m3Orient, EULERTYPE_XYZ);
					}
					break;
				case kSO_World: {
						fEulerAng[0] = GetPFFloat(pblock(), kSimpleOrientation_x, tv);
						fEulerAng[1] = GetPFFloat(pblock(), kSimpleOrientation_y, tv);
						fEulerAng[2] = GetPFFloat(pblock(), kSimpleOrientation_z, tv);
						EulerToMatrix(fEulerAng, m3Orient, EULERTYPE_XYZ);
					}
					break;
				case kSO_Speed:
				case kSO_SpeedFollow: {
						fEulerAng[0] = GetPFFloat(pblock(), kSimpleOrientation_x, tv);
						fEulerAng[1] = GetPFFloat(pblock(), kSimpleOrientation_y, tv);
						fEulerAng[2] = GetPFFloat(pblock(), kSimpleOrientation_z, tv);
						if (chSpeed != NULL)
							m3Orient = SpeedSpaceMatrix(chSpeed->GetValue(i));
						else
							m3Orient = Matrix3(Point3::XAxis, Point3::YAxis, Point3::ZAxis, Point3::Origin);
//						m3Orient.SetRotate(Quat(m3Orient));
						Matrix3 eulerRot;
						EulerToMatrix(fEulerAng, eulerRot, EULERTYPE_XYZ);
//						m3Orient = m3Orient * eulerRot;
						m3Orient = eulerRot * m3Orient;
					}
					break;
			}
			// account for divergence parameter
			if ((iDir != kSO_SpeedFollow) && (iDir != kSO_Rand_3D)) {
				float fDiv = GetPFFloat(pblock(), kSimpleOrientation_divergence, tv);
				Point3 p3RotAxis = RandSphereSurface(prg);
				if(fDiv > 0.f) {
					if (bRestrictToAxis) {
						p3RotAxis.x = GetPFFloat(pblock(), kSimpleOrientation_axisX, tv);
						p3RotAxis.y = GetPFFloat(pblock(), kSimpleOrientation_axisY, tv);
						p3RotAxis.z = GetPFFloat(pblock(), kSimpleOrientation_axisZ, tv);
						if (LengthSquared(p3RotAxis) > 0.0f) {
							p3RotAxis = Normalize(p3RotAxis);
						} else {
							p3RotAxis = Point3::XAxis;
							fDiv = 0.0f;
						}
					}
					float fRandDiv = fDiv * prg->Rand11();
					m3Orient = m3Orient * RotAngleAxisMatrix(p3RotAxis, fRandDiv);
				} else { // perform operations that change randomness state
					prg->Rand11();
				}
			}
			chOrient->SetValue(i, Quat(m3Orient));
		}
	}

	return true;
}

ClassDesc* PFOperatorSimpleOrientation::GetClassDesc() const
{
	return GetPFOperatorSimpleOrientationDesc();
}

int PFOperatorSimpleOrientation::GetRand()
{
	return pblock()->GetInt(kSimpleOrientation_seed);
}

void PFOperatorSimpleOrientation::SetRand(int seed)
{
	_pblock()->SetValue(kSimpleOrientation_seed, 0, seed);
}

} // end of namespace PFActions