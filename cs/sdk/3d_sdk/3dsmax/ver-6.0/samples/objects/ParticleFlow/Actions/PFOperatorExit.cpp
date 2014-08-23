/**********************************************************************
 *<
	FILE:			PFOperatorExit.cpp

	DESCRIPTION:	Exit Operator implementation
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorExit.h"

#include "PFOperatorExit_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

PFOperatorExit::PFOperatorExit()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorExit::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &exit_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &exit_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &exit_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorExit::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_EXIT_CLASS_NAME);
}

Class_ID PFOperatorExit::ClassID()
{
	return PFOperatorExit_Class_ID;
}

void PFOperatorExit::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorExit::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorExit::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch (message)
	{
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kExit_type:
					RefreshUI(kExit_message_type);
					// break; // the break commented intentionally (bayboro 11-19-02)
				case kExit_lifeSpan:
				case kExit_variation:
					NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;
		// Initialization of Dialog
		case kExit_RefMsg_InitDlg:
			RefreshUI(kExit_message_type, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kExit_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
	}
	return REF_SUCCEED;
}

RefTargetHandle PFOperatorExit::Clone(RemapDir &remap)
{
	PFOperatorExit* newOp = new PFOperatorExit();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorExit::GetObjectName()
{
	return GetString(IDS_OPERATOR_EXIT_OBJECT_NAME);
}

const ParticleChannelMask& PFOperatorExit::ChannelsUsed(const Interval& time) const
{
								// read channels
	static ParticleChannelMask mask(PCU_Amount|PCU_New|PCU_Time|PCU_BirthTime|PCU_Lifespan|PCU_Selection,
								// write channels
									PCU_Amount);
	return mask;
}

int	PFOperatorExit::GetRand()
{
	return pblock()->GetInt(kExit_randomSeed);
}

void PFOperatorExit::SetRand(int seed)
{
	pblock()->SetValue(kExit_randomSeed, 0, seed);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorExit::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_EXIT_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorExit::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_EXIT_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorExit::HasDynamicName(TSTR& nameSuffix)
{
	int type	= pblock()->GetInt(kExit_type, 0);
	switch(type) {
	case kExit_type_all:		
		nameSuffix = GetString(IDS_ALL);		
		break;
	case kExit_type_selected:	
		nameSuffix = GetString(IDS_SELECTED);		
		break;
	case kExit_type_byAge:
		nameSuffix = GetString(IDS_BYAGE);		
		nameSuffix += TSTR(" ");
		{
			int tpf = GetTicksPerFrame();
			TimeValue lifespan = pblock()->GetTimeValue(kExit_lifeSpan, 0);
			TCHAR buf[32];
			sprintf(buf, "%d", lifespan/tpf);
			nameSuffix += buf;
			TimeValue variation = pblock()->GetTimeValue(kExit_variation, 0)/tpf;
			if (variation > 0) {
				buf[0] = BYTE(177);
				buf[1] = 0;
				nameSuffix += buf;
				sprintf(buf, "%d", variation);
				nameSuffix += buf;
			}
		}
		break;
	}
	return true;
}

bool PFOperatorExit::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	int exitType = pblock()->GetInt(kExit_type);

	// acquire all necessary channels, create additional if needed
	IParticleChannelAmountR* chAmountR = GetParticleChannelAmountRInterface(pCont);
	if (chAmountR == NULL) return false; // can't read number of particles in the container
	IParticleChannelAmountW* chAmountW = GetParticleChannelAmountWInterface(pCont);
	if (chAmountW == NULL) return false; // can't write number of particles in the container

	IParticleChannelBoolR* chSelect;
	if (exitType == kExit_type_selected) {
		chSelect = GetParticleChannelSelectionRInterface(pCont);
		if (chSelect == NULL) return false; // can't read particle selection
	}

	IParticleChannelNewR* chNew = NULL;
	IParticleChannelPTVR* chTime = NULL;
	IParticleChannelPTVR* chBirthTime = NULL;
	IParticleChannelPTVR* chLifespanR = NULL;
	IParticleChannelPTVW* chLifespanW = NULL;
	bool initLifespan = false;
	if (exitType == kExit_type_byAge) {
		chNew = GetParticleChannelNewRInterface(pCont);
		if (chNew == NULL) return false; // can't find "new" property of particles in the container
		chTime = GetParticleChannelTimeRInterface(pCont);
		if (chTime == NULL) return false; // can't find particle times in the container
		chBirthTime = GetParticleChannelBirthTimeRInterface(pCont);
		if (chBirthTime == NULL) return false; // can't read particle age

		IChannelContainer* chCont;
		chCont = GetChannelContainerInterface(pCont);
		if (chCont == NULL) return false; // can't get access to ChannelContainer interface

		chLifespanR = (IParticleChannelPTVR*)chCont->EnsureInterface(PARTICLECHANNELLIFESPANR_INTERFACE,
															ParticleChannelPTV_Class_ID,
															true, PARTICLECHANNELLIFESPANR_INTERFACE,
															PARTICLECHANNELLIFESPANW_INTERFACE, true,
															actionNode, NULL, &initLifespan);
		if (chLifespanR == NULL) return false;
		chLifespanW = GetParticleChannelLifespanWInterface(pCont);
		if (chLifespanW == NULL) return false;
		// it may happen that the lifespan was created by another Delete operator
		// in this case the channel has to be re-initialized
		IObject* lifespanChannel = chCont->GetChannel(PARTICLECHANNELLIFESPANR_INTERFACE);
		if (lifespanChannel != NULL) {
			IParticleChannel* iChannel = GetParticleChannelInterface(lifespanChannel);
			if (iChannel != NULL) {
				INode* creatorNode = iChannel->GetCreatorAction();
				if (creatorNode != actionNode) {
					if (GetPFObject(creatorNode->GetObjectRef()) != (Object*)this) {
						// the creator of the channel is another operator
						// hence the data need to be re-initialized
						iChannel->SetCreatorAction(actionNode);
						initLifespan = true;
					}
				}
			}
		}
	}

	// modify amount of particles in the container
	int amount = chAmountR->Count();

	switch (exitType)
	{
	case kExit_type_all:
		if (amount > 0) chAmountW->ZeroCount();
		break;
	case kExit_type_selected:
	case kExit_type_byAge:
		{
		BitArray toRemove;
		toRemove.SetSize(amount);
		switch (exitType)
		{
		case kExit_type_selected:
			{
			for (int i = 0; i < amount; i++)
				toRemove.Set(i, chSelect->GetValue(i));
			}
			break;
		case kExit_type_byAge:
			{
			TimeValue lifeSpan = GetPFTimeValue(pblock(), kExit_lifeSpan);
			TimeValue variation = GetPFTimeValue(pblock(), kExit_variation);
			RandGenerator* randGen = randLinker().GetRandGenerator(pCont);
			TimeValue life;
			for (int i = 0; i < amount; i++)
			{
				// compute particle age by the end of this Proceed
				PreciseTimeValue time = chTime->GetValue(i);
				if (time < timeEnd) time = timeEnd;
				PreciseTimeValue deathTime;
				// compute life span
				if ((chNew->IsNew(i)) && (initLifespan)) {
					life = lifeSpan;
					if (variation > 0)
						life += randGen->Rand0X(variation);
					chLifespanW->SetValue(i, life);
					deathTime = chBirthTime->GetValue(i) + life;
				} else {
					deathTime = chBirthTime->GetValue(i) + chLifespanR->GetValue(i);
				}
				// set remove bit
				toRemove.Set(i, time >= deathTime);
			}
			}
			break;
		}
		chAmountW->Delete(toRemove);
		}
		break;
	}

	return true;
}

ClassDesc* PFOperatorExit::GetClassDesc() const
{
	return GetPFOperatorExitDesc();
}



} // end of namespace PFActions