#include "unwrap.h"

//*********************************************************
// Undo record for TV posiitons only
//*********************************************************

TVertRestore::TVertRestore(UnwrapMod *m, BOOL update) 
	{
	mod = m;
	undo = mod->TVMaps.v;
	uvsel   = mod->vsel;
	uesel   = mod->esel;
	ufsel   = mod->fsel;
	updateView = update;
	}

void TVertRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		redo = mod->TVMaps.v;
		rvsel   = mod->vsel;
		resel   = mod->esel;
		rfsel   = mod->fsel;
		}
	mod->TVMaps.v = undo;
	mod->vsel = uvsel;
	mod->esel = uesel;
	mod->fsel = ufsel;

	if (updateView)
		{

		if (mod->fnGetSyncSelectionMode()) 
			{
			theHold.Suspend();
			mod->fnSyncGeomSelection();
			theHold.Resume();
			}

		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
		}
	}

void TVertRestore::Redo() 
	{
	mod->TVMaps.v = redo;
	mod->vsel = rvsel;
	mod->esel = resel;
	mod->fsel = rfsel;

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (updateView)
		{
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
		}
	}
void TVertRestore::EndHold() 
	{
	updateView = TRUE;
	mod->ClearAFlag(A_HELD);
	}
TSTR TVertRestore::Description() 
	{
	return TSTR(_T(GetString(IDS_PW_UVW_VERT_EDIT)));
	}		


//*********************************************************
// Undo record for TV posiitons and face topology
//*********************************************************


TVertAndTFaceRestore::TVertAndTFaceRestore(UnwrapMod *m) 
	{
	mod = m;
	undo = mod->TVMaps.v;

	mod->TVMaps.CloneFaces(fundo);

	uvsel   = mod->vsel;
	ufsel   = mod->fsel;
	uesel   = mod->esel;
	update = FALSE;
	}
TVertAndTFaceRestore::~TVertAndTFaceRestore() 
	{
	int ct = fundo.Count();
	for (int i =0; i < ct; i++)
		{
		if (fundo[i]->vecs) delete fundo[i]->vecs;
		fundo[i]->vecs = NULL;

		if (fundo[i]) delete fundo[i];
		fundo[i] = NULL;
		}
			
	ct = fredo.Count();
	for (i =0; i < ct; i++)
		{
		if (fredo[i]->vecs) delete fredo[i]->vecs;
		fredo[i]->vecs = NULL;

		if (fredo[i]) delete fredo[i];
		fredo[i] = NULL;
		}


	}	

void TVertAndTFaceRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		redo = mod->TVMaps.v;
		mod->TVMaps.CloneFaces(fredo);
		rvsel   = mod->vsel;
		rfsel   = mod->fsel;
		resel   = mod->esel;
		}

	mod->TVMaps.v = undo;
	for (int i=0; i < mod->TVMaps.v.Count(); i++)
		{
		if (mod->TVMaps.cont[i]) 
			mod->TVMaps.cont[i]->SetValue(GetCOREInterface()->GetTime(),&mod->TVMaps.v[i].p);
		}


	mod->TVMaps.AssignFaces(fundo);
	mod->vsel = uvsel;
	mod->fsel = ufsel;

	mod->RebuildEdges();

	mod->esel = uesel;

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (update)
		{
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
		}
	}
void TVertAndTFaceRestore::Redo() 
	{
	mod->TVMaps.v = redo;

	if ( mod->TVMaps.cont.Count() !=  mod->TVMaps.v.Count())
	{
		int start = mod->TVMaps.cont.Count();
		mod->TVMaps.cont.SetCount(mod->TVMaps.v.Count());
		for (int i=start; i < mod->TVMaps.v.Count(); i++)
		{
		
			mod->SetReference(i+11,NewDefaultPoint3Controller());			
		}
		
	}

	for (int i=0; i < mod->TVMaps.v.Count(); i++)
		{
		if ((i < mod->TVMaps.cont.Count()) && (mod->TVMaps.cont[i]) )
			mod->TVMaps.cont[i]->SetValue(GetCOREInterface()->GetTime(),&mod->TVMaps.v[i].p);
		}


	mod->TVMaps.AssignFaces(fredo);
	mod->vsel = rvsel;
	mod->fsel = rfsel;

	mod->RebuildEdges();

	mod->esel = resel;

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (update)
		{
		mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
		if (mod->editMod==mod && mod->hView) mod->InvalidateView();
		}
	}
void TVertAndTFaceRestore::EndHold() 
	{
	update = TRUE;
	mod->ClearAFlag(A_HELD);
	}
TSTR TVertAndTFaceRestore::Description() {return TSTR(_T(GetString(IDS_PW_UVW_EDIT)));}




//*********************************************************
// Undo record for selection of point in the dialog window
//*********************************************************

TSelRestore::TSelRestore(UnwrapMod *m) 
	{
	mod = m;
	undo = mod->vsel;
	fundo = mod->fsel;
	eundo = mod->esel;
	}
void TSelRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		redo = mod->vsel;
		fredo = mod->fsel;
		eredo = mod->esel;
		}
	mod->vsel = undo;
	mod->fsel = fundo;
	mod->esel = eundo;
	mod->RebuildDistCache();

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
void TSelRestore::Redo() 
	{
	mod->vsel = redo;
	mod->fsel = fredo;
	mod->esel = eredo;
	mod->RebuildDistCache();

	if (mod->fnGetSyncSelectionMode()) 
		{
		theHold.Suspend();
		mod->fnSyncGeomSelection();
		theHold.Resume();
		}

	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
void TSelRestore::EndHold() {mod->ClearAFlag(A_HELD);}
TSTR TSelRestore::Description() {return TSTR(_T(GetString(IDS_PW_SELECT_UVW)));}


//*********************************************************
// Undo record for a reset operation
//*********************************************************


ResetRestore::ResetRestore(UnwrapMod *m) 
	{
			
	mod = m;
//			undo = mod->tvert;
	undo = mod->TVMaps.v;

	mod->TVMaps.CloneFaces(fundo);

	uvsel   = mod->vsel;
	uesel   = mod->esel;
	ufsel   = mod->fsel;
	uchan   = mod->channel;
		
	ucont = mod->TVMaps.cont;

	}
ResetRestore::~ResetRestore() 
	{
	int ct = fundo.Count();
	for (int i =0; i < ct; i++)
		{
		if (fundo[i]->vecs) delete fundo[i]->vecs;
		fundo[i]->vecs = NULL;

		if (fundo[i]) delete fundo[i];
		fundo[i] = NULL;
		}
			
	ct = fredo.Count();
	for (i =0; i < ct; i++)
		{
		if (fredo[i]->vecs) delete fredo[i]->vecs;
		fredo[i]->vecs = NULL;

		if (fredo[i]) delete fredo[i];
		fredo[i] = NULL;
		}

	}	

void ResetRestore::Restore(int isUndo) 
	{
	if (isUndo) 
		{
		redo = mod->TVMaps.v;
		mod->TVMaps.CloneFaces(fredo);
		rvsel   = mod->vsel;
		resel   = mod->esel;
		rfsel   = mod->fsel;
		rchan   = mod->channel;

		rcont = mod->TVMaps.cont;


		}
	mod->TVMaps.v = undo;
	mod->TVMaps.AssignFaces(fundo);
	mod->vsel = uvsel;
	mod->esel = uesel;
	mod->fsel = ufsel;
	mod->channel = uchan;
	mod->TVMaps.cont = ucont;
	mod->RebuildEdges();
	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}
void ResetRestore::Redo() 
	{
	mod->TVMaps.v = redo;
	mod->TVMaps.AssignFaces(fredo);
	mod->vsel = rvsel;
	mod->esel = resel;
	mod->fsel = rfsel;
	mod->channel = rchan;
	mod->TVMaps.cont = rcont;
	mod->RebuildEdges();
	mod->NotifyDependents(FOREVER,TEXMAP_CHANNEL,REFMSG_CHANGE);
	if (mod->editMod==mod && mod->hView) mod->InvalidateView();
	}

void ResetRestore::EndHold() {mod->ClearAFlag(A_HELD);}
TSTR ResetRestore::Description() {return TSTR(_T(GetString(IDS_PW_RESET_UNWRAP)));}



//*********************************************************
// Undo record selection for the mesh object
//*********************************************************


UnwrapSelRestore::UnwrapSelRestore(UnwrapMod *m, MeshTopoData *data) {
	mod     = m;
	d       = data;
//	d->held = TRUE;
//	usel = d->faceSel;
	usel.SetSize(mod->TVMaps.f.Count());
	usel.ClearAll();
	for (int i =0; i < usel.GetSize();i++)
		{
		if (mod->TVMaps.f[i]->flags & FLAG_SELECTED)
			usel.Set(i);
		}

	
}

UnwrapSelRestore::UnwrapSelRestore(UnwrapMod *m, MeshTopoData *data, int sLevel) 
{
	mod     = m;
//	level   = sLevel;
	d       = data;
//	d->held = TRUE;
	usel = d->faceSel;
	
}

void UnwrapSelRestore::Restore(int isUndo) {
	if (isUndo) {
		rsel = d->faceSel;
		}
	
	d->faceSel = usel; 
	mod->UpdateFaceSelection(d->faceSel);
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
//	mod->SetNumSelLabel();
}

void UnwrapSelRestore::Redo() {
	d->faceSel = rsel;
	mod->UpdateFaceSelection(d->faceSel);
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
	}




UnwrapPivotRestore::UnwrapPivotRestore(UnwrapMod *m) 
{
	mod     = m;
	upivot = mod->freeFormPivotOffset;
	
}

void UnwrapPivotRestore::Restore(int isUndo) {
	if (isUndo) {
		rpivot = mod->freeFormPivotOffset;
		}
	
	mod->freeFormPivotOffset = upivot; 
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
//	mod->SetNumSelLabel();
}

void UnwrapPivotRestore::Redo() {
	mod->freeFormPivotOffset = rpivot; 
	
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	mod->InvalidateView();
	}


