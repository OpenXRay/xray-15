/**********************************************************************
 *<
	FILE:			PFOperatorForceSpaceWarp.cpp

	DESCRIPTION:	Force Space Warp Operator implementation
					Operator to let you use the older max particle forces

	CREATED BY:		Peter Watje

	HISTORY:		created 02-06-02

  add the influence

  hook up the 3 time types




 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorForceSpaceWarp.h"

#include "PFOperatorForceSpaceWarp_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"
#include "PFMessages.h"

namespace PFActions {

ForceSpaceWarpValidatorClass PFOperatorForceSpaceWarp::validator;
bool PFOperatorForceSpaceWarp::validatorInitiated = false;

PFOperatorForceSpaceWarp::PFOperatorForceSpaceWarp()
{ 
	_scriptPBlock() = NULL;
	_numForces() = 0;
	if (!validatorInitiated) {
		validator.action = NULL;
		validator.paramID = kForceSpaceWarp_ForceNodeList;
		forceSpaceWarp_paramBlockDesc.ParamOption(kForceSpaceWarp_ForceNodeList,p_validator,&validator);
		validatorInitiated = true;
	}
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorForceSpaceWarp::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &forceSpaceWarp_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &forceSpaceWarp_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &forceSpaceWarp_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFOperatorForceSpaceWarp::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_FORCESPACEWARP_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFOperatorForceSpaceWarp::ClassID()
{
	return PFOperatorForceSpaceWarp_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorForceSpaceWarp::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFOperatorForceSpaceWarp::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Animatable* PFOperatorForceSpaceWarp::SubAnim(int i)
{
	switch(i) {
	case 0: return _pblock();
	case 1: return _scriptPBlock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
TSTR PFOperatorForceSpaceWarp::SubAnimName(int i)
{
	if (i == 0) return GetString(IDS_PARAMETERS);
//	if (i == 1) return GetString(IDS_SCRIPTWIRING);
	return GetString(IDS_UNDEFINED);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
ParamDimension* PFOperatorForceSpaceWarp::GetParamDimension(int i)
{
	return _pblock()->GetParamDimension(i);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorForceSpaceWarp::GetParamBlock(int i)
{
	if (i==0) return _pblock();
	if (i==1) return _scriptPBlock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorForceSpaceWarp::GetParamBlockByID(short id)
{
	if (id == 0) return _pblock();
	if (id == 1) return _scriptPBlock();
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefTargetHandle PFOperatorForceSpaceWarp::GetReference(int i)
{
	switch (i) {
	case kForceSpaceWarp_mainPBlockIndex:
		return (RefTargetHandle)pblock();
	case kForceSpaceWarp_scriptPBlockIndex:
		return (RefTargetHandle)scriptPBlock();
	}
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorForceSpaceWarp::SetReference(int i, RefTargetHandle rtarg)
{
	if (i==kForceSpaceWarp_mainPBlockIndex) {
		_pblock() = (IParamBlock2*)rtarg;
	}
	if (i==kForceSpaceWarp_scriptPBlockIndex) {
		_scriptPBlock() = (IParamBlock2*)rtarg;
	}
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
void PFOperatorForceSpaceWarp::UpdatePViewUI(IParamBlock2* pblock, int updateID)
{
	for(int i=0; i<NumPViewParamMaps(); i++) {
		IParamMap2* map = GetPViewParamMap(i);
		if (pblock != map->GetParamBlock()) continue;
		map->Invalidate(updateID);
		Interval currentTimeInterval;
		currentTimeInterval.SetInstant(GetCOREInterface()->GetTime());
		map->Validity() = currentTimeInterval;
	}
}

RefResult PFOperatorForceSpaceWarp::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch (message)
	{
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				if (lastUpdateID == kForceSpaceWarp_ForcesMaxscript) {
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromMXSForces();
					return REF_STOP;
				}
				if (!(theHold.Holding() || theHold.RestoreOrRedoing())) return REF_STOP;
				switch ( lastUpdateID )
				{
				case kForceSpaceWarp_ForceNodeList:
					if (IsIgnoringRefNodeChange()) return REF_STOP;
					updateFromRealForces();
					if (updateObjectNames(kForceSpaceWarp_ForceNodeList)) {
						RefreshUI(kForceSpaceWarp_message_nodes);
						NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
						UpdatePViewUI(pblock(), lastUpdateID);
					}
					return REF_SUCCEED; // to avoid unnecessary UI update
				}
				UpdatePViewUI(pblock(), lastUpdateID);
			} else if (hTarget == scriptPBlock() ) {
				int lastUpdateID = scriptPBlock()->LastNotifyParamID();
				bool disable = ((scriptPBlock()->GetInt(kForceSpaceWarp_useScriptWiring, 0) != 0)
							&& (scriptPBlock()->GetInt(kForceSpaceWarp_useFloat, 0) == kForceSpaceWarp_useFloat_influence));
				if (disable) 
					RefreshUI(kForceSpaceWarp_message_disableInfluence);
				else
					RefreshUI(kForceSpaceWarp_message_enableInfluence);
				UpdatePViewUI(scriptPBlock(), lastUpdateID);
			}
			break;
		case REFMSG_NODE_NAMECHANGE:
			NotifyDependents(FOREVER, 0, kPFMSG_DynamicNameChange, NOTIFY_ALL, TRUE);
			UpdatePViewUI(pblock(), kForceSpaceWarp_ForceNodeList);
			break;
		case REFMSG_NODE_WSCACHE_UPDATED:
			updateFromRealForces();
			break;
		// Initialization of Dialog
		case kForceSpaceWarp_RefMsg_InitDlg:
			{
				if (scriptPBlock() != NULL) {
					bool disable = ((scriptPBlock()->GetInt(kForceSpaceWarp_useScriptWiring, 0) != 0)
								&& (scriptPBlock()->GetInt(kForceSpaceWarp_useFloat, 0) == kForceSpaceWarp_useFloat_influence));
					if (disable) 
						RefreshUI(kForceSpaceWarp_message_disableInfluence, (IParamMap2*)partID);
					else
						RefreshUI(kForceSpaceWarp_message_enableInfluence, (IParamMap2*)partID);
				}
				RefreshUI(kForceSpaceWarp_message_nodes, (IParamMap2*)partID);
			}
			return REF_STOP;
		case kForceSpaceWarp_RefMsg_ListSelect:
			validator.action = this;
			GetCOREInterface()->DoHitByNameDialog(this);
			return REF_STOP;
		case kForceSpaceWarp_RefMsg_ResetValidatorAction:
			validator.action = this;
			return REF_STOP;
	}
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFOperatorForceSpaceWarp::Clone(RemapDir &remap)
{
	PFOperatorForceSpaceWarp* newOp = new PFOperatorForceSpaceWarp();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	newOp->ReplaceReference(1, remap.CloneRef(scriptPBlock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

//+--------------------------------------------------------------------------+
//|							From BaseObject								 |
//+--------------------------------------------------------------------------+
TCHAR* PFOperatorForceSpaceWarp::GetObjectName()
{
	return GetString(IDS_OPERATOR_FORCESPACEWARP_OBJECT_NAME);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
const ParticleChannelMask& PFOperatorForceSpaceWarp::ChannelsUsed(const Interval& time) const
{
								//  read						 &	write channels
	static ParticleChannelMask mask(PCU_ID|PCU_Time|PCU_Position|PCU_Amount|PCU_Speed|PCU_BirthTime|PCU_EventStart|PCU_New, 
									PCU_Speed|PCU_Position);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
bool PFOperatorForceSpaceWarp::GetUseScriptWiring() const
{
	if (scriptPBlock() == NULL) return false;
	return (scriptPBlock()->GetInt(kForceSpaceWarp_useScriptWiring, 0) != 0);
}

//+--------------------------------------------------------------------------+
//|							From IPFAction									 |
//+--------------------------------------------------------------------------+
void PFOperatorForceSpaceWarp::SetUseScriptWiring(bool useScriptWiring)
{
	if (scriptPBlock() == NULL) return;
	scriptPBlock()->SetValue(kForceSpaceWarp_useScriptWiring, 0, (int)useScriptWiring);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
int PFOperatorForceSpaceWarp::NumPViewParamBlocks() const
{
	return (GetUseScriptWiring() ? 2 : 1);
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
IParamBlock2* PFOperatorForceSpaceWarp::GetPViewParamBlock(int i) const
{
	if (i==0) return pblock();
	if (i==1) return scriptPBlock();
	return NULL;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorForceSpaceWarp::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_FORCESPACEWARP_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorForceSpaceWarp::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_FORCESPACEWARP_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
bool PFOperatorForceSpaceWarp::HasDynamicName(TSTR& nameSuffix)
{
	int num = pblock()->Count(kForceSpaceWarp_ForceNodeList);
	int count=0, firstIndex=-1;
	for(int i=0; i<num; i++) {
		if (pblock()->GetINode(kForceSpaceWarp_ForceNodeList, 0, i) != NULL) {
			count++;
			if (firstIndex < 0) firstIndex = i;
		}
	}
	if (count > 0) {
		INode* force = pblock()->GetINode(kForceSpaceWarp_ForceNodeList, 0, firstIndex);
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

bool PFOperatorForceSpaceWarp::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	// acquire all necessary channels, create additional if needed

	IChannelContainer* chCont;
	chCont = GetChannelContainerInterface(pCont);
	if (chCont == NULL) return false;

	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if(chAmount == NULL) return false;
	int iQuant = chAmount->Count();
	if (iQuant < 1) return true; // no particles to proceed

	IParticleChannelNewR* chNew = GetParticleChannelNewRInterface(pCont);
	if (chNew == NULL) return false; 

	IParticleChannelIDR* chID = GetParticleChannelIDRInterface(pCont);
	if (chID == NULL) return false;

	IParticleChannelPTVR* chTime = GetParticleChannelTimeRInterface(pCont);
	if(chTime == NULL) return false;

	IParticleChannelPTVR* chAge = GetParticleChannelBirthTimeRInterface(pCont);
	if(chAge == NULL) return false;

// the channel of interest speed
	bool initSpeed = false;
//channel does not exist so make it and note that we have to fill it out
	IParticleChannelPoint3W* chSpeedW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELSPEEDW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELSPEEDR_INTERFACE,
																			PARTICLECHANNELSPEEDW_INTERFACE, true,
																			actionNode, NULL, &initSpeed);
	IParticleChannelPoint3R* chSpeed = GetParticleChannelSpeedRInterface(pCont);
	if ((chSpeedW == NULL) || (chSpeed == NULL)) return false;

	bool initPosition = false;
	IParticleChannelPoint3W* chPosW = (IParticleChannelPoint3W*)chCont->EnsureInterface(PARTICLECHANNELPOSITIONW_INTERFACE,
																			ParticleChannelPoint3_Class_ID,
																			true, PARTICLECHANNELPOSITIONR_INTERFACE,
																			PARTICLECHANNELPOSITIONW_INTERFACE, true,
																			actionNode, NULL, &initPosition);
	IParticleChannelPoint3R* chPos = GetParticleChannelPositionRInterface(pCont);
	if ((chPosW == NULL) || (chPos == NULL)) return false;

	bool useScript = ((scriptPBlock()->GetInt(kForceSpaceWarp_useScriptWiring, 0) != 0)
						&& (scriptPBlock()->GetInt(kForceSpaceWarp_useFloat, 0) == kForceSpaceWarp_useFloat_influence));
	IParticleChannelFloatR* chFloat = NULL;
	if (useScript) {
		chFloat = GetParticleChannelMXSFloatRInterface(pCont);
		if (chFloat == NULL) return false;
	}

	int timeType = kAbsoluteTime;
	_pblock()->GetValue(kForceSpaceWarp_Sync,0, timeType, FOREVER);

	IParticleChannelPTVR* chEventStart = NULL;
	IParticleChannelPTVW* chEventStartW = NULL;
	bool initEventStart = false;
	if (timeType == kEventDuration)
	{
		chEventStartW = (IParticleChannelPTVW*) chCont->EnsureInterface(PARTICLECHANNELEVENTSTARTW_INTERFACE,
								  ParticleChannelPTV_Class_ID,
								  true, PARTICLECHANNELEVENTSTARTR_INTERFACE,
								  PARTICLECHANNELEVENTSTARTW_INTERFACE, false,
								  actionNode, NULL, &initEventStart);

		chEventStart = GetParticleChannelEventStartRInterface(pCont);
		if ((chEventStart == NULL) || (chEventStartW == NULL)) return false;
	}

	int overlapping = pblock()->GetInt(kForceSpaceWarp_Overlapping, 0);
	// collecting force fields
	Tab<ForceField*> ff;
	ForceField* curFF;
	int i, j;
	for(i=0; i<pblock()->Count(kForceSpaceWarp_ForceNodeList); i++) {
		INode* node = pblock()->GetINode(kForceSpaceWarp_ForceNodeList, 0, i);
		if (node == NULL) continue;
		Object* ob = GetPFObject(node->GetObjectRef());
		if (ob == NULL) continue;
		if (ob->SuperClassID() == WSM_OBJECT_CLASS_ID) {
			WSMObject* obref = (WSMObject*)ob;
			curFF = obref->GetForceField(node);
			if (curFF != NULL) {
				if (ob->ClassID() == CS_VFIELDOBJECT_CLASS_ID) {
					// CS VectorField SW doesn't init properly partobj on GetForceField
					// this is a quick fix for that (bayboro 3/6/2003)
					CS_VectorField* vf = (CS_VectorField*)curFF;
					vf->partobj = GetParticleInterface(pSystem);
				}
				ff.Append(1, &curFF);
			}
		}
	}
	if (ff.Count() == 0) return true; // no force fields

	// some calls for a reference node TM may initiate REFMSG_CHANGE notification
	// we have to ignore that while processing the particles
	bool wasIgnoring = IsIgnoringRefNodeChange();
	if (!wasIgnoring) SetIgnoreRefNodeChange();

	float influence = 0.0f;
	for(i = 0; i < iQuant; i++) 
	{
		TimeValue t = 0;
		if (timeType == kAbsoluteTime)
			t = chTime->GetValue(i).TimeValue();
		else if (timeType == kParticleAge)
			t = chTime->GetValue(i).TimeValue() - chAge->GetValue(i).TimeValue();
		else 
		{
			if (initEventStart && chNew->IsNew(i))
				chEventStartW->SetValue(i, chTime->GetValue(i));
			t = chTime->GetValue(i).TimeValue() - chEventStart->GetValue(i).TimeValue();
		}

		if (useScript) {
			influence = chFloat->GetValue(i);
		} else {
			influence = GetPFFloat(pblock(), kForceSpaceWarp_Influence, t);
		}

		Point3 v(0.0f,0.0f,0.0f);
		if (!initSpeed || !chNew->IsNew(i)) //if we created a speed channel the channel incoming is bogus so just use 0,0,0 ad default
			v = chSpeed->GetValue(i);

		Point3 p(0.0f,0.0f,0.0f);
		if (!initPosition || !chNew->IsNew(i)) //if we created a pos channel the channel incoming is bogus so just use 0,0,0 ad default
			p = chPos->GetValue(i);

		Point3 force = Point3::Origin;
		for(j=0; j<ff.Count(); j++) {
			// buffer vectors to guard true position and speed from malicious force
			Point3 pp = p;
			Point3 vv = v;
			Point3 nextForce = ff[j]->Force(t,pp,vv,chID->GetParticleBorn(i)) * influence;
			float lenSq = LengthSquared(nextForce);
			if (lenSq <= 0.0f) continue; // not a valid force
			if (overlapping == kForceSpaceWarp_Overlapping_additive) {
				force += nextForce;
			} else {
				if (lenSq > LengthSquared(force))
					force = nextForce;
			}
//			p = pp;
//			v = vv;
		}

		v += force * float(timeEnd - chTime->GetValue(i));
		chPosW->SetValue(i, p);
		chSpeedW->SetValue(i, v);
	}

	for(i=0; i<ff.Count(); i++)
		if (ff[i] != NULL) ff[i]->DeleteThis();

	if (!wasIgnoring) ClearIgnoreRefNodeChange();
	return true;
}

int PFOperatorForceSpaceWarp::getNumForces()
{
	int num = pblock()->Count(kForceSpaceWarp_ForceNodeList);
	int count=0;
	for(int i=0; i<num; i++)
		if (pblock()->GetINode(kForceSpaceWarp_ForceNodeList, 0, i) != NULL)
			count++;
	return count;	
}

bool gUpdateForcesInProgress = false;
bool PFOperatorForceSpaceWarp::updateFromRealForces()
{
	if (theHold.RestoreOrRedoing()) return false;
	bool res = UpdateFromRealRefObjects(pblock(), kForceSpaceWarp_ForceNodeList, 
				kForceSpaceWarp_ForcesMaxscript, gUpdateForcesInProgress);
	if (res && theHold.Holding())
		MacrorecObjects(this, pblock(), kForceSpaceWarp_ForceNodeList, _T("Force_Space_Warps"));
	return res;
}

bool PFOperatorForceSpaceWarp::updateFromMXSForces()
{
	if (theHold.RestoreOrRedoing()) return false;
	return UpdateFromMXSRefObjects(pblock(), kForceSpaceWarp_ForceNodeList, 
				kForceSpaceWarp_ForcesMaxscript, gUpdateForcesInProgress, this);
}

bool PFOperatorForceSpaceWarp::updateObjectNames(int pblockID)
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
int PFOperatorForceSpaceWarp::filter(INode *node)
{
	PB2Value v;
	v.r = node;
	validator.action = this;
	return validator.Validate(v);
}

void PFOperatorForceSpaceWarp::proc(INodeTab &nodeTab)
{
	if (nodeTab.Count() == 0) return;
	theHold.Begin();
	pblock()->Append(kForceSpaceWarp_ForceNodeList, nodeTab.Count(), nodeTab.Addr(0));
	theHold.Accept(GetString(IDS_PARAMETERCHANGE));
}

ClassDesc* PFOperatorForceSpaceWarp::GetClassDesc() const
{
	return GetPFOperatorForceSpaceWarpDesc();
}


} // end of namespace PFActions











