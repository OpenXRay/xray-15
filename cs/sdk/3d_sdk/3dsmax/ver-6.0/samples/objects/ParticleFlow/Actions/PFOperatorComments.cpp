/**********************************************************************
 *<
	FILE:			PFOperatorComments.cpp

	DESCRIPTION:	Comments Operator implementation
					Operator to effect speed unto particles

	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 08-27-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#include "resource.h"

#include "PFActions_SysUtil.h"
#include "PFActions_GlobalFunctions.h"
#include "PFActions_GlobalVariables.h"

#include "PFOperatorComments.h"

#include "PFOperatorComments_ParamBlock.h"
#include "PFClassIDs.h"
#include "IPFSystem.h"
#include "IParticleObjectExt.h"
#include "IParticleChannels.h"
#include "IChannelContainer.h"
#include "IPViewManager.h"

namespace PFActions {

PFOperatorComments::PFOperatorComments()
{ 
	GetClassDesc()->MakeAutoParamBlocks(this); 
}

FPInterfaceDesc* PFOperatorComments::GetDescByID(Interface_ID id)
{
	if (id == PFACTION_INTERFACE) return &comments_action_FPInterfaceDesc;
	if (id == PFOPERATOR_INTERFACE) return &comments_operator_FPInterfaceDesc;
	if (id == PVIEWITEM_INTERFACE) return &comments_PViewItem_FPInterfaceDesc;
	return NULL;
}

void PFOperatorComments::GetClassName(TSTR& s)
{
	s = GetString(IDS_OPERATOR_COMMENTS_CLASS_NAME);
}

Class_ID PFOperatorComments::ClassID()
{
	return PFOperatorComments_Class_ID;
}

void PFOperatorComments::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	GetClassDesc()->BeginEditParams(ip, this, flags, prev);
}

void PFOperatorComments::EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next)
{
	GetClassDesc()->EndEditParams(ip, this, flags, next );
}

RefResult PFOperatorComments::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
														PartID& partID, RefMessage message)
{
	switch(message) {
		case REFMSG_CHANGE:
			if ( hTarget == pblock() ) {
				int lastUpdateID = pblock()->LastNotifyParamID();
				UpdatePViewUI(lastUpdateID);
			}
			return REF_STOP;
	}

	return REF_SUCCEED;
}

RefTargetHandle PFOperatorComments::Clone(RemapDir &remap)
{
	PFOperatorComments* newOp = new PFOperatorComments();
	newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	BaseClone(this, newOp, remap);
	return newOp;
}

TCHAR* PFOperatorComments::GetObjectName()
{
	return GetString(IDS_OPERATOR_COMMENTS_OBJECT_NAME);
}

const ParticleChannelMask& PFOperatorComments::ChannelsUsed(const Interval& time) const
{
								//  read & write channels
	static ParticleChannelMask mask(0, 0);
	return mask;
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorComments::GetActivePViewIcon()
{
	if (activeIcon() == NULL)
		_activeIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_COMMENTS_ACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return activeIcon();
}

//+--------------------------------------------------------------------------+
//|							From IPViewItem									 |
//+--------------------------------------------------------------------------+
HBITMAP PFOperatorComments::GetInactivePViewIcon()
{
	if (inactiveIcon() == NULL)
		_inactiveIcon() = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_COMMENTS_INACTIVEICON),IMAGE_BITMAP,
									kActionImageWidth, kActionImageHeight, LR_SHARED);
	return inactiveIcon();
}

bool PFOperatorComments::Proceed(IObject* pCont, 
									 PreciseTimeValue timeStart, 
									 PreciseTimeValue& timeEnd,
									 Object* pSystem,
									 INode* pNode,
									 INode* actionNode,
									 IPFIntegrator* integrator)
{
	return true;
}

ClassDesc* PFOperatorComments::GetClassDesc() const
{
	return GetPFOperatorCommentsDesc();
}

} // end of namespace PFActions