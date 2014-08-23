
/**********************************************************************
 
	FILE: undo.cpp

	DESCRIPTION:  This contains all the undo records

	CREATED BY: Peter Watje

	HISTORY: 2/18/03




 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#include "bonesdef.h"


PasteRestore::PasteRestore(BonesDefMod *c) 
	{
	mod = c;
	ubone = mod->BoneData[mod->ModeBoneIndex];
	uRefTable = mod->RefTable;
	urefHandleList = mod->refHandleList;
	}   		

void PasteRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		rbone = mod->BoneData[mod->ModeBoneIndex];
		rRefTable = mod->RefTable;
		rrefHandleList = mod->refHandleList;
		}
	mod->BoneData[mod->ModeBoneIndex] = ubone;
	mod->RefTable = uRefTable ;
	mod->refHandleList = urefHandleList ;
//watje 9-7-99  198721 
	mod->Reevaluate(TRUE);

	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void PasteRestore::Redo()
	{
//watje 9-7-99  198721 
	mod->BoneData[mod->ModeBoneIndex] = rbone ;
	mod->RefTable = rRefTable ;
	mod->refHandleList = rrefHandleList ;

	mod->Reevaluate(TRUE);

	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);

	}		
void PasteRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR PasteRestore::Description() { return TSTR(_T(GetString(IDS_PW_PASTE))); }






PasteToAllRestore::PasteToAllRestore(BonesDefMod *c) 
	{
	mod = c;
	uRefTable = mod->RefTable;
	urefHandleList = mod->refHandleList;
	ubuffer.SetCount(mod->BoneData.Count());
	for (int k = 0; k < ubuffer.Count();k ++)
		ubuffer[k] = NULL;

	for (k = 0; k < ubuffer.Count();k ++)
		{
		if (mod->BoneData[k].Node)
			{
			BoneDataClass *t = new BoneDataClass();
			ubuffer[k] = t;
			*ubuffer[k]  = mod->BoneData[k];
			}
		}

	}   		

PasteToAllRestore::~PasteToAllRestore() 
	{
	for (int k = 0; k < rbuffer.Count();k ++)
		{
		if (rbuffer[k]) delete rbuffer[k];
			rbuffer[k] = NULL;
		}
	for (k = 0; k < ubuffer.Count();k ++)
		{
		if (ubuffer[k]) delete ubuffer[k];
			ubuffer[k] = NULL;
		}
	}

void PasteToAllRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		rbuffer.SetCount(mod->BoneData.Count());
		rRefTable = mod->RefTable;
		rrefHandleList = mod->refHandleList;;
	
		for (int k = 0; k < rbuffer.Count();k ++)
			rbuffer[k] = NULL;

		for (k = 0; k < rbuffer.Count();k ++)
			{
			if (mod->BoneData[k].Node)
				{
				BoneDataClass *t = new BoneDataClass();
				rbuffer[k] = t;

				*rbuffer[k] = mod->BoneData[k];
				}
			}
		}

	mod->RefTable = uRefTable;
	mod->refHandleList=	urefHandleList;

	for (int k = 0; k < ubuffer.Count(); k++)
		{
		if (ubuffer[k])
			{
			mod->BoneData[k] = *ubuffer[k];

			}
		}
	mod->Reevaluate(TRUE);

	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void PasteToAllRestore::Redo()
	{
	mod->RefTable = rRefTable;
	mod->refHandleList=	rrefHandleList;

	for (int k = 0; k < rbuffer.Count(); k++)
		{
		if (rbuffer[k])
			{
			mod->BoneData[k] = *rbuffer[k];
			}
		}
//watje 9-7-99  198721 
	mod->Reevaluate(TRUE);

	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void PasteToAllRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR PasteToAllRestore::Description() { return TSTR(_T(GetString(IDS_PW_PASTE))); }







SelectionRestore::SelectionRestore(BonesDefMod *c, BoneModData *md) 
	{
	mod = c;
	bmd = md;
	uModeBoneIndex = mod->ModeBoneIndex;
	uModeBoneEnvelopeIndex  = mod->ModeBoneEnvelopeIndex;
	uModeBoneEndPoint  = mod->ModeBoneEndPoint;
	uModeBoneEnvelopeIndex = mod->ModeBoneEnvelopeIndex;
	uModeBoneEnvelopeSubType = mod->ModeBoneEnvelopeSubType;
	uVertSel.SetSize(bmd->selected.GetSize());
	uVertSel = bmd->selected;
	}   		
void SelectionRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		rModeBoneIndex = mod->ModeBoneIndex;
		rModeBoneEnvelopeIndex  = mod->ModeBoneEnvelopeIndex;
		rModeBoneEndPoint  = mod->ModeBoneEndPoint;
		rModeBoneEnvelopeIndex = mod->ModeBoneEnvelopeIndex;
		rModeBoneEnvelopeSubType = mod->ModeBoneEnvelopeSubType;
		rVertSel.SetSize(bmd->selected.GetSize());
		rVertSel = bmd->selected;
		}
	mod->ModeBoneIndex = uModeBoneIndex;
	mod->ModeBoneEnvelopeIndex = uModeBoneEnvelopeIndex ;
	mod->ModeBoneEndPoint = uModeBoneEndPoint  ;
	mod->ModeBoneEnvelopeIndex = uModeBoneEnvelopeIndex ;
	mod->ModeBoneEnvelopeSubType = uModeBoneEnvelopeSubType ;
	bmd->selected = uVertSel;
	mod->UpdatePropInterface();
	if (bmd->selected.NumberSet() >0)
		mod->EnableEffect(TRUE);
	else mod->EnableEffect(FALSE);
		mod->UpdateEffectSpinner(bmd);

	mod->SyncSelections();
			

	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void SelectionRestore::Redo()
	{
	mod->ModeBoneIndex = rModeBoneIndex;
	mod->ModeBoneEnvelopeIndex = rModeBoneEnvelopeIndex ;
	mod->ModeBoneEndPoint = rModeBoneEndPoint  ;
	mod->ModeBoneEnvelopeIndex = rModeBoneEnvelopeIndex ;
	mod->ModeBoneEnvelopeSubType = rModeBoneEnvelopeSubType ;
	bmd->selected = rVertSel;
	mod->UpdatePropInterface();
	if (bmd->selected.NumberSet() >0)
		mod->EnableEffect(TRUE);
	else mod->EnableEffect(FALSE);
		mod->UpdateEffectSpinner(bmd);

	mod->SyncSelections();
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void SelectionRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR SelectionRestore::Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }







DeleteBoneRestore::DeleteBoneRestore(BonesDefMod *c, int whichBone) 
	{
	mod = c;
 	undo = mod->BoneData[whichBone];
	undoID = whichBone;
	undoRefTable = mod->RefTable;
	urefHandleList = mod->refHandleList;
	}   		
void DeleteBoneRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		redoID = mod->ModeBoneIndex;
		redo = mod->BoneData[undoID];
		redoRefTable = mod->RefTable;
		rrefHandleList = mod->refHandleList;
		}

	mod->BoneData[undoID] = undo;
	mod->RefTable = undoRefTable;

	mod->refHandleList = urefHandleList;

	if (mod->ip)
		{
		mod->RefillListBox();
	
		int selID = mod->ConvertSelectedBoneToListID(undoID);
		SendMessage(GetDlgItem(mod->hParam,IDC_LIST1),
					LB_SETCURSEL ,selID,0);
		mod->UpdatePropInterface();
		}
	mod->ModeBoneIndex = undoID;

//watje 9-7-99  198721 
	mod->Reevaluate(TRUE);
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	}
void DeleteBoneRestore::Redo()
	{
	mod->BoneData[undoID] = redo;
	mod->RefTable = redoRefTable;

	mod->refHandleList = rrefHandleList;


	if (mod->ip)
		{
		mod->RefillListBox();

		int selID = mod->ConvertSelectedBoneToListID(redoID);
		SendMessage(GetDlgItem(mod->hParam,IDC_LIST1),
					LB_SETCURSEL ,selID,0);
		mod->UpdatePropInterface();
		}

	mod->ModeBoneIndex = redoID;

//watje 9-7-99  198721 
	mod->Reevaluate(TRUE);
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews(mod->ip->GetTime());
	}		
void DeleteBoneRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR DeleteBoneRestore::Description() { return TSTR(_T(GetString(IDS_PW_DELETEBONE))); }




AddBoneRestore::AddBoneRestore(BonesDefMod *c) 
	{
	mod = c;
	for (int i = 0; i < mod->BoneData.Count(); i++)
		{
 		undo.Append(mod->BoneData[i]);
		}
	undoRefTable = mod->RefTable;
	undoID = mod->ModeBoneIndex;
	urefHandleList = mod->refHandleList;
	}   		
void AddBoneRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		redo.New();
		for (int i = 0; i < mod->BoneData.Count(); i++)
			{
 			redo.Append(mod->BoneData[i]);
			}

		
		redoRefTable = mod->RefTable;
		redoID = mod->ModeBoneIndex;
		rrefHandleList = mod->refHandleList;
		}

	mod->BoneData.New();
	for (int i = 0; i < undo.Count(); i++)
		{
		mod->BoneData.Append(undo[i]);
		}
	mod->RefTable = undoRefTable;
	mod->ModeBoneIndex = undoID;
	mod->refHandleList = urefHandleList; 
	mod->updateListBox = TRUE;
			

//watje 9-7-99  198721 
	mod->Reevaluate(TRUE);
	mod->cacheValid = FALSE;

	}
void AddBoneRestore::Redo()
	{
	mod->BoneData.New();
	for (int i = 0; i < redo.Count(); i++)
		{
		mod->BoneData.Append(redo[i]);
		}

	mod->RefTable = redoRefTable;

	mod->ModeBoneIndex = redoID;
	mod->refHandleList = rrefHandleList; 
	mod->updateListBox = TRUE;



//watje 9-7-99  198721 
	mod->Reevaluate(TRUE);
	mod->cacheValid = FALSE;
	}		
void AddBoneRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR AddBoneRestore::Description() { return TSTR(_T(GetString(IDS_PW_ADDBONE))); }




WeightRestore::WeightRestore(BonesDefMod *bmod, BoneModData *md, BOOL updateView) 
	{
	this->updateView = updateView;
	int c = md->VertexData.Count();
	undoVertexData.SetCount(md->VertexData.Count());
	for (int i=0; i<c; i++) 
		{
		VertexListClass *vc;
		vc = new VertexListClass;
		undoVertexData[i] = vc;
		undoVertexData[i]->Modified (md->VertexData[i]->IsModified());
		undoVertexData[i]->UnNormalized (md->VertexData[i]->IsUnNormalized());
		undoVertexData[i]->Rigid (md->VertexData[i]->IsRigid());
		undoVertexData[i]->RigidHandle (md->VertexData[i]->IsRigidHandle());
				
		undoVertexData[i]->d = md->VertexData[i]->d;
		}

	mod = bmod;
	bmd = md;
	}   	
WeightRestore::~WeightRestore()
	{
	int c = undoVertexData.Count();
	for (int i=0; i<c; i++) 
		delete undoVertexData[i];
	c = redoVertexData.Count();
	for (i=0; i<c; i++) 
		delete redoVertexData[i];
	}
void WeightRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		int c = bmd->VertexData.Count();
		redoVertexData.SetCount(bmd->VertexData.Count());
		for (int i=0; i<c; i++) 
			{
			VertexListClass *vc;
			vc = new VertexListClass;
			redoVertexData[i] = vc;
			redoVertexData[i]->Modified (bmd->VertexData[i]->IsModified());
			redoVertexData[i]->UnNormalized (bmd->VertexData[i]->IsUnNormalized());
			redoVertexData[i]->Rigid (bmd->VertexData[i]->IsRigid());
			redoVertexData[i]->RigidHandle (bmd->VertexData[i]->IsRigidHandle());
			redoVertexData[i]->d = bmd->VertexData[i]->d;
			}

		}

	int c = undoVertexData.Count();
	if (c != bmd->VertexData.Count())
		{
		for (int i=0; i<bmd->VertexData.Count(); i++) 
			if (bmd->VertexData[i]) delete bmd->VertexData[i];
		bmd->VertexData.SetCount(c);
		for (i=0; i<c; i++) 
			bmd->VertexData[i] = new VertexListClass;
		mod->Reevaluate(TRUE);
		}

	for (int i=0; i<c; i++) 
		{
		bmd->VertexData[i]->Modified (undoVertexData[i]->IsModified());
		bmd->VertexData[i]->UnNormalized (undoVertexData[i]->IsUnNormalized());
		bmd->VertexData[i]->Rigid (undoVertexData[i]->IsRigid());
		bmd->VertexData[i]->RigidHandle (undoVertexData[i]->IsRigidHandle());
		bmd->VertexData[i]->d = undoVertexData[i]->d;
		}

	float ef;
	mod->pblock_param->GetValue(skin_effect,0,ef,FOREVER);
	bmd->effect = ef;

	mod->SyncSelections();

	if (updateView)
		mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void WeightRestore::Redo()
	{
	int c = redoVertexData.Count();

	if (c != bmd->VertexData.Count())
		{
		for (int i=0; i<bmd->VertexData.Count(); i++) 
			if (bmd->VertexData[i]) delete bmd->VertexData[i];
		bmd->VertexData.SetCount(c);
		for (i=0; i<c; i++) 
			bmd->VertexData[i] = new VertexListClass;
		mod->Reevaluate(TRUE);
		}

	for (int i=0; i<c; i++) 
		{
		bmd->VertexData[i]->Modified (redoVertexData[i]->IsModified());
		bmd->VertexData[i]->UnNormalized (redoVertexData[i]->IsUnNormalized());
		bmd->VertexData[i]->Rigid (redoVertexData[i]->IsRigid());
		bmd->VertexData[i]->RigidHandle (redoVertexData[i]->IsRigidHandle());

		bmd->VertexData[i]->d = redoVertexData[i]->d;
		}

	float ef;
	mod->pblock_param->GetValue(skin_effect,0,ef,FOREVER);
	bmd->effect = ef;
	mod->SyncSelections();


	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void WeightRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	updateView =TRUE;
	}
TSTR WeightRestore::Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }






ExclusionListRestore::ExclusionListRestore(BonesDefMod *bmod, BoneModData *md,int boneID) 
	{
	whichBone = boneID;
	uAffectedVerts.ZeroCount();
	mod = bmod;
	bmd = md;
	if ((whichBone < bmd->exclusionList.Count()) && (bmd->exclusionList[whichBone]))
		{
		int ct = bmd->exclusionList[whichBone]->Count();
		uAffectedVerts.SetCount(ct);
		for (int i = 0; i < ct; i++)
			uAffectedVerts[i] = bmd->exclusionList[whichBone]->Vertex(i);
		}
			
	}   	
void ExclusionListRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		rAffectedVerts.ZeroCount();
		if ((whichBone < bmd->exclusionList.Count()) && (bmd->exclusionList[whichBone]))
			{
			int ct = bmd->exclusionList[whichBone]->Count();
			rAffectedVerts.SetCount(ct);
			for (int i = 0; i < ct; i++)
				rAffectedVerts[i] = bmd->exclusionList[whichBone]->Vertex(i);
			}

		}
	if ((whichBone < bmd->exclusionList.Count()) && (bmd->exclusionList[whichBone]))
		bmd->exclusionList[whichBone]->SetExclusionVerts(uAffectedVerts);
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void ExclusionListRestore::Redo()
	{
	if ((whichBone < bmd->exclusionList.Count()) && (bmd->exclusionList[whichBone]))
		bmd->exclusionList[whichBone]->SetExclusionVerts(rAffectedVerts);
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void ExclusionListRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR ExclusionListRestore::Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }





GizmoPasteRestore::GizmoPasteRestore(GizmoClass *c) 
	{
	giz = c;
	uBuffer = NULL;
	rBuffer = NULL;
	uBuffer = giz->CopyToBuffer();
	}   
GizmoPasteRestore::~GizmoPasteRestore()
	{
	delete uBuffer;
	delete rBuffer;
	}
void GizmoPasteRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		rBuffer = giz->CopyToBuffer();
		}

	giz->PasteFromBuffer(uBuffer);

	giz->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void GizmoPasteRestore::Redo()
	{
	giz->PasteFromBuffer(rBuffer);
	giz->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		

void GizmoPasteRestore::EndHold() 
	{ 
	giz->ClearAFlag(A_HELD);
	}
TSTR GizmoPasteRestore::Description() { return TSTR(_T(GetString(IDS_PW_PASTE))); }



UpdateUIRestore::UpdateUIRestore(BonesDefMod *bmod) 
	{
	mod = bmod;
	}  
void UpdateUIRestore::Restore(int isUndo) 
	{
	if (mod->ip) mod->UpdateGizmoList();
	}
void UpdateUIRestore::Redo()
	{
	if (mod->ip) mod->UpdateGizmoList();
	}		
void UpdateUIRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR UpdateUIRestore::Description() { return TSTR(_T(GetString(IDS_PW_ADDGIZMO))); }




UpdatePRestore::UpdatePRestore(BonesDefMod *bmod) 
	{
	mod = bmod;
	}  
void UpdatePRestore::Restore(int isUndo) 			
	{
	mod->updateP = TRUE;
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void UpdatePRestore::Redo()
	{
	mod->updateP = TRUE;
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void UpdatePRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR UpdatePRestore::Description() { return TSTR(_T(GetString(IDS_PW_EDIT_ENVELOPE))); }




UpdateSquashUIRestore::UpdateSquashUIRestore(BonesDefMod *bmod, float val,  int id) 
	{
	mod = bmod;
	index = id;
	uval = val;
	}  
void UpdateSquashUIRestore::Restore(int isUndo) 			
	{
	if (isUndo)
		{
		mod->pblock_param->GetValue(skin_local_squash,mod->ip->GetTime(),rval,FOREVER,index);
		}
	mod->pblock_param->SetValue(skin_local_squash,mod->ip->GetTime(),uval,index);
	mod->UpdatePropInterface();
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void UpdateSquashUIRestore::Redo()
	{
	mod->pblock_param->SetValue(skin_local_squash,mod->ip->GetTime(),rval,index);

	mod->UpdatePropInterface();
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void UpdateSquashUIRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR UpdateSquashUIRestore::Description() { return TSTR(_T(GetString(IDS_PW_EDIT_ENVELOPE))); }




AddGizmoRestore::AddGizmoRestore(BonesDefMod *bmod,GizmoClass *gizmo) 
	{
	mod = bmod;
	rGizmo = gizmo;
	}  

void AddGizmoRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		int last = mod->pblock_gizmos->Count(skin_gizmos_list)-1;
		}
	if (mod->ip)
		{			
		rGizmo->EndEditParams(mod->ip, END_EDIT_REMOVEUI,NULL);
		}

	mod->UpdateGizmoList();
	}
void AddGizmoRestore::Redo()
	{
	if (mod->ip)
		{
		mod->UpdateGizmoList();
		}

	}		
void AddGizmoRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR AddGizmoRestore::Description() { return TSTR(_T(GetString(IDS_PW_ADDGIZMO))); }





AddGizmoLocalDataRestore::AddGizmoLocalDataRestore(BonesDefMod *bmod, BoneModData *md) 
	{
	mod = bmod;
	bmd = md;
	whichGizmo = bmd->gizmoData.Count()-1;
	}   	
AddGizmoLocalDataRestore::~AddGizmoLocalDataRestore() 
	{		
	}
void AddGizmoLocalDataRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		int last =  bmd->gizmoData.Count()-1;
		rData = new LocalGizmoData();
		*rData = *bmd->gizmoData[last];
		}
			
	bmd->gizmoData.Delete(whichGizmo,1);
	}
void AddGizmoLocalDataRestore::Redo()
	{
	bmd->gizmoData.Append(1,&rData);
	}		
void AddGizmoLocalDataRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR AddGizmoLocalDataRestore::Description() { return TSTR(_T(GetString(IDS_PW_ADDGIZMO))); }




RefTargetHandle RemoveGizmoRestore::GetReference(int i)
	{
	return (RefTargetHandle)uGizmo;
	}

void RemoveGizmoRestore::SetReference(int i, RefTargetHandle rtarg)
	{
	uGizmo = (GizmoClass*)rtarg;
	}
RefResult RemoveGizmoRestore::NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
			   PartID& partID, RefMessage message)	
	{
	return REF_SUCCEED;
	}
void RemoveGizmoRestore::DeleteThis() { delete this; }
RemoveGizmoRestore::RemoveGizmoRestore(BonesDefMod *bmod,GizmoClass *g, int id) 
	{
	uGizmo = NULL;
	mod = bmod;
	whichGizmo = id;
	theHold.Suspend();
	ReplaceReference(0,g);
	theHold.Resume();
	}   	
RemoveGizmoRestore::~RemoveGizmoRestore()
	{
	theHold.Suspend();
	DeleteAllRefsFromMe();
	theHold.Resume();
	}
void RemoveGizmoRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		}

//put back gizmo
	theHold.Suspend();
	ReferenceTarget *ref = (ReferenceTarget *) uGizmo;
//insert gizmo to the list at spot
	mod->pblock_gizmos->Insert(skin_gizmos_list,whichGizmo,1,&ref);
			
//remove the previous rollout
	if (mod->currentSelectedGizmo >=0)
		{
		}

	mod->currentSelectedGizmo = whichGizmo;
	if (mod->ip)
		{
		mod->UpdateGizmoList();
		}
	theHold.Resume();
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void RemoveGizmoRestore::Redo()
	{
	theHold.Suspend();
	ReferenceTarget *ref = NULL;
//take away gizmo
	int id = whichGizmo;
	if ( id < 0) return;
	ref = mod->pblock_gizmos->GetReferenceTarget(skin_gizmos_list,0,id);
	GizmoClass *gizmo = (GizmoClass *)ref;
	if (mod->ip)
		{
		gizmo->EndEditParams(mod->ip, END_EDIT_REMOVEUI,NULL);
		}
	mod->pblock_gizmos->Delete(skin_gizmos_list,id,1);
	mod->UpdateGizmoList();

	if (	mod->currentSelectedGizmo != -1)

		{
		}
	theHold.Resume();
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}		
void RemoveGizmoRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR RemoveGizmoRestore::Description() { return TSTR(_T(GetString(IDS_PW_REMOVEGIZMO))); }




RemoveGizmoLocalDataRestore::RemoveGizmoLocalDataRestore(BonesDefMod *bmod, BoneModData *md,int gid) 
	{
	mod = bmod;
	bmd = md;
	whichGizmo = gid;
	if (gid < bmd->gizmoData.Count())
		{
		uData = new LocalGizmoData();
		*uData = *bmd->gizmoData[gid];
		}
	else uData = NULL;
	}   	
RemoveGizmoLocalDataRestore::~RemoveGizmoLocalDataRestore()
	{
	}
void RemoveGizmoLocalDataRestore::Restore(int isUndo) 
	{
	if (uData)
		{
		bmd->gizmoData.Insert(whichGizmo,1,&uData);
		for (int i = whichGizmo+1; i <bmd->gizmoData.Count(); i++)
			bmd->gizmoData[i]->whichGizmo++;
		}
	}
void RemoveGizmoLocalDataRestore::Redo()
	{
	if (uData)
		{
		bmd->gizmoData.Delete(whichGizmo,1);
		for (int i = whichGizmo; i <bmd->gizmoData.Count(); i++)
			bmd->gizmoData[i]->whichGizmo--;
		}
	}				
void RemoveGizmoLocalDataRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR RemoveGizmoLocalDataRestore::Description() { return TSTR(_T(GetString(IDS_PW_REMOVEGIZMO))); }






SelectGizmoRestore::SelectGizmoRestore(BonesDefMod *bmod) 
	{
	mod = bmod;
	uWhichGizmo = mod->currentSelectedGizmo;
	}   	
void SelectGizmoRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		rWhichGizmo = mod->currentSelectedGizmo;
		}
			
	mod->SelectGizmo(uWhichGizmo);

	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
void SelectGizmoRestore::Redo()
	{
	mod->SelectGizmo(rWhichGizmo);
	mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}		
void SelectGizmoRestore::EndHold() 
	{ 
	mod->ClearAFlag(A_HELD);
	}
TSTR SelectGizmoRestore::Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }
