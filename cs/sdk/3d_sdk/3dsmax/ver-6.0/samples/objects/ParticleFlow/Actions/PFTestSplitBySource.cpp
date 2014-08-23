/**********************************************************************
 *<
	FILE: PFTestSplitBySource.h

	DESCRIPTION: SplitBySource Test implementation
				 The Test checks particles by their origin, what
				 emitter has produced these particles

	CREATED BY: Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "resource.h"
#include "max.h"
#include "iparamm2.h"
#include "macrorec.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFTestSplitBySource.h"

#include "PFTestSplitBySource_ParamBlock.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPFArrow.h"
#include "PFMessages.h"
#include "IPViewManager.h"

namespace PFActions {

// constructors/destructors
PFTestSplitBySource::PFTestSplitBySource()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From FPMixinInterface							 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc* PFTestSplitBySource::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &splitBySource_action_FPInterfaceDesc;
	if (id == PFTEST_INTERFACE) return &splitBySource_test_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &splitBySource_PViewItem_FPInterfaceDesc;
	return NULL;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From Animatable									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
void PFTestSplitBySource::GetClassName(TSTR& s)
{
	s = GetString(IDS_TEST_SPLITBYSOURCE_CLASS_NAME);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
Class_ID PFTestSplitBySource::ClassID()
{
	return PFTestSplitBySource_Class_ID;
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSplitBySource::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

//+--------------------------------------------------------------------------+
//|							From Animatable									 |
//+--------------------------------------------------------------------------+
void PFTestSplitBySource::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From ReferenceMaker								 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
RefResult PFTestSplitBySource::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				switch ( lastUpdateID )
				{
				case kSplitBySource_sources:
					RefreshUI(kSplitBySource_message_update);
					break;
				}
				UpdatePViewUI(lastUpdateID);
			}
			break;

		case REFMSG_NODE_HANDLE_CHANGED:
			if (hTarget == NULL) // direct notification
				updateNodeHandles((IMergeManager*)partID);
			return REF_STOP;

		// Initialization of Dialog
		case kSplitBySource_RefMsg_InitDlg:
			RefreshUI(kSplitBySource_message_update, (IParamMap2*)partID);
			return REF_STOP;
		// New Random Seed
		case kSplitBySource_RefMsg_NewRand:
			theHold.Begin();
			NewRand();
			theHold.Accept(GetString(IDS_NEWRANDOMSEED));
			return REF_STOP;
		case kSplitBySource_RefMsg_MacrorecSources:
			macrorecSources();
			return REF_STOP;
	}	
	return REF_SUCCEED;
}

//+--------------------------------------------------------------------------+
//|							From ReferenceMaker								 |
//+--------------------------------------------------------------------------+
RefTargetHandle PFTestSplitBySource::Clone(RemapDir &remap)
{
	PFTestSplitBySource* newTest = new PFTestSplitBySource();
	newTest->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newTest, remap);
	return newTest;
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From BaseObject									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
TCHAR* PFTestSplitBySource::GetObjectName()
{
	return GetString(IDS_TEST_SPLITBYSOURCE_OBJECT_NAME);
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFAction									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
const ParticleChannelMask& PFTestSplitBySource::ChannelsUsed(const Interval& time) const
{
								//  read	 &	write channels
	static ParticleChannelMask mask(PCU_Amount, 0);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSplitBySource::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPLITBYSOURCE_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSplitBySource::GetTruePViewIcon()
{
	if (trueIcon() == NULL)
		_trueIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPLITBYSOURCE_TRUEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return trueIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFTestSplitBySource::GetFalsePViewIcon()
{
	if (falseIcon() == NULL)
		_falseIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_SPLITBYSOURCE_FALSEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return falseIcon();
}

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							From IPFTest									 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
bool PFTestSplitBySource::Proceed(IObject* pCont, 
							PreciseTimeValue timeStart, 
							PreciseTimeValue& timeEnd, 
							Object* pSystem, 
							INode* pNode, 
							INode* actionNode, 
							IPFIntegrator* integrator, 
							BitArray& testResult, 
							Tab<float>& testTime)
{
	if (pNode == NULL) return false;

	bool exactStep = IsExactIntegrationStep(timeEnd, pSystem);
	int conditionType	= pblock()->GetInt(kSplitBySource_conditionType, timeStart);

	// acquire absolutely necessary particle channels
	IParticleChannelAmountR* chAmount = GetParticleChannelAmountRInterface(pCont);
	if (chAmount == NULL) return false; // can't find number of particles in the container
	int count = chAmount->Count();

	bool isSelectedSource = false;
	int i, systemHandle = pNode->GetHandle();
	for(i=0; i<pblock()->Count(kSplitBySource_sources); i++) {
		if (systemHandle == pblock()->GetInt(kSplitBySource_sources, 0, i)) {
			isSelectedSource = true; break;
		}
	}
				
	// test all particles
	testResult.SetSize(count);
	testResult.ClearAll();
	testTime.SetCount(count);
	if (exactStep) {
		if ((isSelectedSource && (conditionType == kSplitBySource_conditionType_selected)) 
			|| (!isSelectedSource && (conditionType == kSplitBySource_conditionType_notSelected))) {
			testResult.SetAll();
			for(i=0; i<count; i++) testTime[i] = 0.0f;
		}
	}

	return true;
}

ClassDesc* PFTestSplitBySource::GetClassDesc() const
{
	return GetPFTestSplitBySourceDesc();
}

void PFTestSplitBySource::updateNodeHandles(IMergeManager* pMM)
{
	if (pMM == NULL) return;
	if (pblock() == NULL) return;
	int wasHolding = theHold.Holding();
	if (wasHolding) theHold.Suspend();

	int i, systemCount = pblock()->Count(kSplitBySource_sources);
	for(i=0; i<systemCount; i++) {
		ULONG systemHandle = pblock()->GetInt(kSplitBySource_sources, 0, i);
		pblock()->SetValue(kSplitBySource_sources, 0, int(pMM->GetNewHandle(systemHandle)), i);
	}

	if (wasHolding) theHold.Resume();
}

void PFTestSplitBySource::macrorecSources()
{
	if (pblock() == NULL) return;
	TSTR selItemNames = _T("(");
	int numItems = pblock()->Count(kSplitBySource_sources);
	bool nonFirst = false;
	Interface* ip = GetCOREInterface();
	for(int i=0; i<numItems; i++) {
		INode* selNode = ip->GetINodeByHandle(ULONG(pblock()->GetInt(kSplitBySource_sources, 0, i)));
		if (selNode == NULL) continue;
		if (nonFirst) selItemNames = selItemNames + _T(", ");
		selItemNames = selItemNames + _T("$'");
		selItemNames = selItemNames + selNode->GetName();
		selItemNames = selItemNames + _T("'.handle");
		nonFirst = true;
	}
	selItemNames = selItemNames + _T(")");
	TCHAR selNames[3000];
	sprintf(selNames,"%s", selItemNames);
	macroRec->EmitScript();
	macroRec->SetProperty(this, _T("Selected_Sources"), mr_name, selNames);
}




} // end of namespace PFActions