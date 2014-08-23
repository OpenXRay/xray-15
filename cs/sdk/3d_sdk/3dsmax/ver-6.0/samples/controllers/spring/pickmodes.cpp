/**********************************************************************
 *<
	FILE: PickModes.cpp

	DESCRIPTION:	A procedural Mass/Spring Position controller 
					Pick Mode functionality

	CREATED BY:		Adam Felt

	HISTORY: 

 *>	Copyright (c) 1999-2000, All Rights Reserved.
 **********************************************************************/

#include "jiggle.h"

//--- PickLinkMode ------------------------------------------------

BOOL PickForceMode::Filter(INode *node)
{
	if (node->TestForLoop(FOREVER,(ReferenceMaker *) dlg->cont)!=REF_SUCCEED) return FALSE;
	
	Object* obj = node->GetObjectRef();
	if (obj && (obj = obj->FindBaseObject())) 
	{	
		if (obj->SuperClassID()==WSM_OBJECT_CLASS_ID)
		{
			ForceField *ff = ((WSMObject*)obj)->GetForceField(node);
			if (ff)
			{
				ff->DeleteThis();
				return TRUE;
			}
			//return (BOOL)((WSMObject*)obj)->SupportsDynamics();
		}
	}
	return FALSE;	
}

BOOL PickForceMode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{
	INode *node = dlg->ip->PickNode(hWnd,m);	
	if (!node) return false;
	else return Filter(node);
	}

BOOL PickForceMode::Pick(IObjParam *ip,ViewExp *vpt)
	{
	INode *node = vpt->GetClosestHit();
	if (node) {
//		theHold.Begin();
		dlg->cont->force_pb->Append(jig_force_node, 1, &node);		
		dlg->UpdateForceList();

//		theHold.Accept(GetString(IDS_AF_ADDLINK));
		IParamMap2* pmap;
		pmap = dlg->cont->force_pb->GetMap();
		if (pmap) pmap->Invalidate(jig_force_node);

		dlg->ip->RedrawViews(dlg->ip->GetTime());
		}
	return FALSE;
	}

void PickForceMode::EnterMode(IObjParam *ip)
	{dlg->iPickForce->SetCheck(TRUE);}

void PickForceMode::ExitMode(IObjParam *ip)
	{dlg->iPickForce->SetCheck(FALSE);}


//--- PickNodeMode ------------------------------------------------

BOOL PickNodeMode::Filter(INode *node)
{
	INode* nodeInList;
	BOOL inList = false;
	if (node->TestForLoop(FOREVER,(ReferenceMaker *) cont)!=REF_SUCCEED) return FALSE;
	else
	{
		for (int i=0;i<cont->dyn_pb->Count(jig_control_node); i++)
		{
			cont->dyn_pb->GetValue(jig_control_node, 0, nodeInList, FOREVER, i);
			if (nodeInList == node ) return FALSE;
		}
	}
	
	if (node->IsRootNode())
		return FALSE;

	return true;	
}

BOOL PickNodeMode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{
	INode *node = ip->PickNode(hWnd,m);	
	if (!node) return false;
	else return Filter(node);
}

BOOL PickNodeMode::Pick(IObjParam *ip,ViewExp *vpt)
{
	INode *node = vpt->GetClosestHit();
	cont->AddSpring(node);
	return FALSE;
}

BOOL PickNodeMode::PickAnimatable(Animatable* anim)
{
	if (anim->SuperClassID() == BASENODE_CLASS_ID)
	{
		INode *node = ((INode*)anim);
		if (Filter(node))
		{
			cont->AddSpring(node);
			return TRUE;
		}
	}
	return FALSE;
}


void PickNodeMode::EnterMode(IObjParam *ip)
{
	if (cont->dlg) cont->dlg->iAddBone->SetCheck(TRUE);
	if (cont->hParams1 && map) map->iAddBone->SetCheck(TRUE);
}

void PickNodeMode::ExitMode(IObjParam *ip)
{
	if (cont->dlg) cont->dlg->iAddBone->SetCheck(FALSE); 
	if (cont->hParams1 && map) map->iAddBone->SetCheck(FALSE);
}

