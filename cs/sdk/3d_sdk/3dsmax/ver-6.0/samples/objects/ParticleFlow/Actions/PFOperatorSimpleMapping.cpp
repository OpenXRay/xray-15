/**********************************************************************
 *<
	FILE:			PFOperatorSimpleMapping.cpp

	DESCRIPTION:	SimpleMapping Operator implementation
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include <limits>

#include "max.h"
#include "meshdlib.h"

#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorSimpleMapping.h"

#include "PFOperatorSimpleMapping_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

PFOperatorSimpleMapping::PFOperatorSimpleMapping()
{
	GetClassDesc()->MakeAutoParamBlocks(this);
}

PFOperatorSimpleMapping::~PFOperatorSimpleMapping()
{
	;
}

FPInterfaceDesc* PFOperatorSimpleMapping::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &simpleMapping_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &simpleMapping_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &simpleMapping_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorSimpleMapping::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_SIMPLEMAPPING_CLASS_NAME);
}

Class_ID PFOperatorSimpleMapping::ClassID()
{
	return PFOperatorSimpleMapping_Class_ID;
}

void PFOperatorSimpleMapping::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorSimpleMapping::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorSimpleMapping::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
													 PartID& partID, RefMessage message)
{
//	IParamMap2* map;

	if ( hTarget == pblock() ) {
		int lastUpdateID;
		switch (message)
		{
		case REFMSG_CHANGE:
			lastUpdateID = pblock()->LastNotifyParamID();
			switch ( lastUpdateID )
			{
			case kSimpleMapping_syncBy:
				NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
				break;
			case kSimpleMapping_channelType:
				RefreshUI(kSimpleMapping_message_channelType);
				break;
			}
			UpdatePViewUI(lastUpdateID);
			break;
		// Initialization of Dialog
		case kSimpleMapping_RefMsg_InitDlg:
			// Refresh UI
			RefreshUI(kSimpleMapping_message_channelType);
			return REF_STOP;
		}
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorSimpleMapping::Clone(RemapDir &remap)
{
	PFOperatorSimpleMapping* newOp = new PFOperatorSimpleMapping();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorSimpleMapping::GetObjectName()
{
	return GetString(IDS_OPERATOR_SIMPLEMAPPING_OBJECT_NAME);
}

const ParticleChannelMask& PFOperatorSimpleMapping::ChannelsUsed(const Interval& time) const
{
								// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime|PCU_EventStart|PCU_ShapeTexture,
								// write channels
									PCU_EventStart|PCU_ShapeTexture);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleMapping::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEMAPPING_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorSimpleMapping::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SIMPLEMAPPING_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorSimpleMapping::HasDynamicName(TSTR& nameSuffix)
{
	int sync = pblock()->GetInt(kSimpleMapping_syncBy, 0);
	switch(sync) {
	case kSimpleMapping_syncBy_time:
		nameSuffix = GetString(IDS_TIME);
		break;
	case kSimpleMapping_syncBy_age:
		nameSuffix = GetString(IDS_AGE);
		break;
	case kSimpleMapping_syncBy_event:
		nameSuffix = GetString(IDS_EVENT);
		break;
	}
	return true;
}

bool PFOperatorSimpleMapping::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	if (pblock() == NULL) return false;

	int showInViewport = pblock()->GetInt(kSimpleMapping_showInViewport, timeEnd);
	if (!showInViewport) { // check if the system is in render; if not then return
		IPFSystem* iSystem = GetPFSystemInterface(pSystem);
		if (iSystem == NULL) return false;
		if (!iSystem->IsRenderState()) return true; // nothing to show in viewport
	}

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();
	if (count <= 0) return true; // no particles to modify
	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if (chTime == NULL) return false; // can't read timing for a particle
	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; // can't find newly entered particles for duration calculation
	
	IChannelContainer* chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	int syncType = pblock()->GetInt(kSimpleMapping_syncBy);
	bool initEventStart = false;
	IParticleChannelPTVR* chBirth = NULL;
	IParticleChannelPTVR* chEventStartR = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	if (syncType == kSimpleMapping_syncBy_age) {
		chBirth = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirth == NULL) return false; // can't read particle age
	}
	if (syncType == kSimpleMapping_syncBy_event) {
		chEventStartR = (IParticleChannelPTVR*) chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTR_INTERFACE,
																	ParticleChannelPTV_Class_ID,
																	true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
																	PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
																	actionNode, NULL, &initEventStart);
		if (chEventStartR == NULL) return false; // can't read/create EventStart channel
		if (initEventStart) {
			chEventStartW = GetParticleChannelEventStartWInterface(pCont);
			if (chEventStartW == NULL) return false; // can't init EventStart channel
		}
	}

	IParticleChannelMeshMapW* chMap = (IParticleChannelMeshMapW*)chCont->EnsureInterface(PARTICLECHANNELSHAPETEXTUREW_INTERFACE,
																ParticleChannelMeshMap_Class_ID,
																true, PARTICLECHANNELSHAPETEXTURER_INTERFACE,
																PARTICLECHANNELSHAPETEXTUREW_INTERFACE, false,
																actionNode, NULL);
	if (chMap == NULL) return false;

	int channelNum = 0;
	if (pblock()->GetInt(kSimpleMapping_channelType, timeEnd) == kSimpleMapping_channelType_map)
		channelNum = pblock()->GetInt(kSimpleMapping_mapChannel, timeEnd);
	
	chMap->SetMapSupport(channelNum, true);
	IParticleChannelMapW* map = chMap->GetMapChannel(channelNum); DbgAssert(map);
	if (map == NULL) return false;
	map->SetTVFace(NULL); // no tvFace data for all particles

	for(int i=0; i<count; i++) {
		PreciseTimeValue syncTime = chTime->GetValue(i);
		if (initEventStart && chNew->IsNew(i))
			chEventStartW->SetValue(i, syncTime);
		if (syncType == kSimpleMapping_syncBy_age) {
			syncTime -= chBirth->GetValue(i);
		} else if (syncType == kSimpleMapping_syncBy_event) {
			syncTime -= chEventStartR->GetValue(i);
		}
		
		UVVert mapValue = Point3::Origin;
		mapValue.x = GetPFFloat(pblock(), kSimpleMapping_U, syncTime);
		mapValue.y = GetPFFloat(pblock(), kSimpleMapping_V, syncTime);
		mapValue.z = GetPFFloat(pblock(), kSimpleMapping_W, syncTime);
		map->SetUVVert(i, mapValue);
	}

	return true;
}

ClassDesc* PFOperatorSimpleMapping::GetClassDesc() const
{
	return GetPFOperatorSimpleMappingDesc();
}


} // end of namespace PFActions