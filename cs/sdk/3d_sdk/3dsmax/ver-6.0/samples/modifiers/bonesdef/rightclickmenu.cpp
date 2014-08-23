 /**********************************************************************
 
	FILE: RightClickMenu.cpp

	DESCRIPTION:  Right Click menu stuff

	CREATED BY: Peter Watje

	HISTORY: 8/5/98




 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"

// This uses the linked-list class templates
#include "linklist.h"
#include "bonesdef.h"


void BonesRightMenu::Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m) {
		
		int flags1,flags2,flags3,flags4,flags5,flags6,flags7,flags8,flags9,flags10;
		int flags11, flags12;

		flags1 = flags2 = flags3 = flags4 = flags5 = flags6 = flags7 = MF_STRING | MF_UNCHECKED;
		flags8 = flags1;
		flags9 = flags1;
		flags10 = flags1;
		flags11 = flags1;
		flags12 = flags1;

		if (ep->FilterVertices  == 0)
			flags2 |= MF_CHECKED;
		if (ep->FilterBones  == 0)
			flags3 |= MF_CHECKED;
		if (ep->FilterEnvelopes  == 0)
			flags4 |= MF_CHECKED;
		if (ep->DrawEnvelopes  == 1)
			flags5 |= MF_CHECKED;
		if (ep->DrawVertices  == 1)
			flags7 |= MF_CHECKED;
		if (ep->ip->GetSubObjectLevel() == 1)
			flags10 |= MF_CHECKED;

		
		manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
		manager->AddMenu(this, flags10, 9, GetString(IDS_PW_EDIT_ENVELOPE));
		manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
		manager->AddMenu(this, flags2, 1, GetString(IDS_PW_FILTERVERTICES));
		manager->AddMenu(this, flags3, 2, GetString(IDS_PW_FILTERBONES));
		manager->AddMenu(this, flags4, 3, GetString(IDS_PW_FILTERENVELOPES));
		manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
		manager->AddMenu(this, flags5, 4, GetString(IDS_PW_DRAWALLENVELOPES));
		manager->AddMenu(this, flags7, 6, GetString(IDS_PW_COLORVERTS));
		manager->AddMenu(this, MF_SEPARATOR, 0, NULL);

		}
	

void BonesRightMenu::Selected(UINT id) {
//  Add Cross Section
	if (id ==  0)
		{
		ep->StartCrossSectionMode(0);
		}
//  Remove Cross Section
	else if (id ==  5)
		{
		ep->RemoveCrossSection();
		ep->ip->RedrawViews(ep->ip->GetTime());
		}
	else if (id ==  1)
		{
//		ep->FilterVertices  = !ep->FilterVertices;
		ep->pblock_param->SetValue(skin_filter_vertices,0,ep->FilterVertices);
		ep->FilterVertices = !ep->FilterVertices;

//		ep->ClearVertexSelections();
		}
	else if (id ==  2)
		{
//		ep->FilterBones  = !ep->FilterBones;
		ep->pblock_param->SetValue(skin_filter_bones,0,ep->FilterBones);
		ep->FilterBones  = !ep->FilterBones;

		ep->ClearBoneEndPointSelections();

		}
	else if (id ==  3)
		{
//		ep->FilterEnvelopes  = !ep->FilterEnvelopes;
		ep->pblock_param->SetValue(skin_filter_envelopes,0,ep->FilterEnvelopes);
		ep->FilterEnvelopes  = !ep->FilterEnvelopes;
		ep->ClearEnvelopeSelections();

		}	
	else if (id ==  4)
		{
		ep->DrawEnvelopes  = !ep->DrawEnvelopes;
		ep->pblock_param->SetValue(skin_draw_all_envelopes,0,ep->DrawEnvelopes);
		ep->ip->RedrawViews(ep->ip->GetTime());
		}
	else if (id ==  6)
		{
		ep->DrawVertices  = !ep->DrawVertices;
		ep->pblock_param->SetValue(skin_draw_vertices,0,ep->DrawVertices);
		ep->ip->RedrawViews(ep->ip->GetTime());

		}
	else if (id ==  7)
		{
		ep->ip->DoHitByNameDialog(new DumpHitDialog(ep));
		}	
	else if (id == 9)
		{
		if (ep->ip->GetSubObjectLevel() == 1)
			ep->ip->SetSubObjectLevel(0);
		else ep->ip->SetSubObjectLevel(1);	


		}
	}

