/*===========================================================================*\
 | 
 |  FILE:	wM3_pick.cpp
 |			Weighted Morpher for MAX R3
 |			Pick mode methods
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 27-9-98
 | 
\*===========================================================================*/

#include "wM3.h"


BOOL  GetMorphNode::Filter(INode *node)
{
	Interval valid; 
	
	ObjectState os = node->GetObjectRef()->Eval(mp->ip->GetTime());

	if( os.obj->IsDeformable() == FALSE ) return FALSE;

	// Check for same-num-of-verts-count
	if( os.obj->NumPoints()!=mp->MC_Local.Count) return FALSE;

	node->BeginDependencyTest();
	mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {		
		return FALSE;
	}

	// check to make sure that the max number of progressive targets will not be exceeded
	//
	morphChannel &bank = mp->CurrentChannel();
	if(bank.mConnection ) {
		if ( bank.mNumProgressiveTargs >= MAX_TARGS )
			return FALSE;
	}
	
	return TRUE;
}


BOOL  GetMorphNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
}

BOOL  GetMorphNode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	
	INode *node = vpt->GetClosestHit();
	if (node) {

		node->BeginDependencyTest();
		mp->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest()) return FALSE;		

		if( mp->CheckMaterialDependency() ) return FALSE;
		// Make the node reference, and then ask the channel to load itself

		UI_MAKEBUSY

		if (!theHold.Holding()) theHold.Begin();
		theHold.Put(new Restore_FullChannel(mp, mp->chanSel+mp->chanNum));

		morphChannel &bank = mp->CurrentChannel();
		if(bank.mConnection ) {
			int refnum = (mp->CurrentChannelIndex()*MAX_TARGS) + 201 + bank.mNumProgressiveTargs;
            mp->ReplaceReference(refnum,node);
			bank.InitTargetCache(bank.mNumProgressiveTargs,node);
			bank.mNumProgressiveTargs++;
			assert(bank.mNumProgressiveTargs<=MAX_TARGS);
			bank.ReNormalize();
			mp->Update_channelParams();
		}
		else {
			mp->ReplaceReference(101+mp->CurrentChannelIndex(),node);
			bank.buildFromNode(node, TRUE, 0, TRUE);
			bank.mNumProgressiveTargs=0;
			bank.ReNormalize();
		}
		theHold.Accept(GetString(IDS_MENUNAME));

		mp->DisplayMemoryUsage();

		UI_MAKEFREE
	}
	
	return TRUE;
}


void  GetMorphNode::EnterMode(IObjParam *ip)
{
	// FIX: select the currently active viewport so that
	// the user can use the H shortcut
	ViewExp *ve = mp->ip->GetActiveViewport();
	SetFocus(ve->GetHWnd());
	mp->ip->ReleaseViewport(ve);

	// flag that we are infact picking
	isPicking=TRUE;

	ICustButton *iBut;

	iBut = GetICustButton(GetDlgItem(mp->hwChannelParams,IDC_PICK));
		if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);

	for( int i=IDC_P1;i<IDC_P10+1;i++){
	HWND button = GetDlgItem(mp->hwChannelList,i);
	iBut = GetICustButton(button);
		if (iBut) 
		{
			iBut->SetHighlightColor(GREEN_WASH);
			InvalidateRect(button, NULL, FALSE);
		}
	ReleaseICustButton(iBut);
	}
}

void  GetMorphNode::ExitMode(IObjParam *ip)
{
	isPicking=FALSE;

	ICustButton *iBut;

	iBut = GetICustButton(GetDlgItem(mp->hwChannelParams,IDC_PICK));
		if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);

	for( int i=IDC_P1;i<IDC_P10+1;i++){
	HWND button = GetDlgItem(mp->hwChannelList,i);
	iBut = GetICustButton(button);
		if (iBut) 
		{
#ifndef GAME_VER	// russom - 08/20/01
			iBut->SetHighlightColor(RGB(210,210,210));
#else
			iBut->SetHighlightColor(GetCustSysColor( COLOR_BTNHIGHLIGHT )); 
#endif
			InvalidateRect(button, NULL, FALSE);
		}
	ReleaseICustButton(iBut);
	}
}