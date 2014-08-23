/**********************************************************************
 *<
	FILE: edmrest.cpp

	DESCRIPTION:  Edit Mesh OSM	Restore objects

	CREATED BY: Rolf Berteig

	HISTORY: created 1 September, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "MeshDLib.h"
#include "editmesh.h"

VertexEditRestore::VertexEditRestore (EditMeshData* md, EditMeshMod* mod) {
	meshData  = md;
	this->mod = mod;

	int num;
	if (num=md->mdelta.vMove.Count()) {
		oMove.SetCount (num);
		memcpy (oMove.Addr(0), md->mdelta.vMove.Addr(0), num*sizeof(VertMove));
	}
	if (num=md->mdelta.vClone.Count()) {
		oClone.SetCount (md->mdelta.vClone.Count());
		memcpy (oClone.Addr(0), md->mdelta.vClone.Addr(0), num*sizeof(VertMove));
	}
}

void VertexEditRestore::Restore(int isUndo) {
	int num;
	nMove.SetCount (num=meshData->mdelta.vMove.Count());
	if (num) memcpy (nMove.Addr(0), meshData->mdelta.vMove.Addr(0), num*sizeof(VertMove));
	nClone.SetCount (num=meshData->mdelta.vClone.Count());
	if (num) memcpy (nClone.Addr(0), meshData->mdelta.vClone.Addr(0), num*sizeof(VertMove));

	meshData->mdelta.vMove.SetCount (num=oMove.Count());
	if (num) memcpy (meshData->mdelta.vMove.Addr(0), oMove.Addr(0), num*sizeof(VertMove));
	meshData->mdelta.vClone.SetCount (num=oClone.Count());
	if (num) memcpy (meshData->mdelta.vClone.Addr(0), oClone.Addr(0), num*sizeof(VertMove));

	meshData->Invalidate(PART_GEOM,FALSE);
	if (isUndo) mod->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

void VertexEditRestore::Redo() {
	int num;
	meshData->mdelta.vMove.SetCount (num=nMove.Count());
	if (num) memcpy (meshData->mdelta.vMove.Addr(0), nMove.Addr(0), num*sizeof(VertMove));
	meshData->mdelta.vClone.SetCount (num=nClone.Count());
	if (num) memcpy (meshData->mdelta.vClone.Addr(0), nClone.Addr(0), num*sizeof(VertMove));

	meshData->Invalidate (PART_GEOM,FALSE);
	mod->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

/*-------------------------------------------------------------------*/

MapVertexEditRestore::MapVertexEditRestore (EditMeshData *md, EditMeshMod *mod, int mapChan) {
	meshData  = md;
	this->mod = mod;
	mapChannel = mapChan;
	if (!md->mdelta.getMapSupport(mapChannel)) return;

	int num;
	num=md->mdelta.Map(mapChannel).vSet.Count();
	oSet.SetCount (num);
	if (num) memcpy (oSet.Addr(0), md->mdelta.Map(mapChannel).vSet.Addr(0), num*sizeof(UVVertSet));
	num=md->mdelta.Map(mapChannel).vCreate.Count();
	oCreate.SetCount (num);
	if (num) memcpy (oCreate.Addr(0), md->mdelta.Map(mapChannel).vCreate.Addr(0), num*sizeof(UVVert));
}

// Returns true only if there's an actual change.
void MapVertexEditRestore::After () {
	if (!meshData->mdelta.getMapSupport (mapChannel)) return;
	int num;
	num=meshData->mdelta.Map(mapChannel).vSet.Count();
	nSet.SetCount (num);
	if (num) memcpy (nSet.Addr(0), meshData->mdelta.Map(mapChannel).vSet.Addr(0), num*sizeof(UVVertSet));
	num=meshData->mdelta.Map(mapChannel).vCreate.Count();
	nCreate.SetCount (num);
	if (num) memcpy (nCreate.Addr(0), meshData->mdelta.Map(mapChannel).vCreate.Addr(0), num*sizeof(UVVert));
}

void MapVertexEditRestore::Restore (int isUndo) {
	if (isUndo && (nSet.Count() == 0) && (nCreate.Count() == 0)) After ();

	int num;
	meshData->mdelta.Map(mapChannel).vSet.SetCount (num=oSet.Count());
	if (num) memcpy (meshData->mdelta.Map(mapChannel).vSet.Addr(0), oSet.Addr(0), num*sizeof(UVVertSet));
	meshData->mdelta.Map(mapChannel).vCreate.SetCount (num=oCreate.Count());
	if (num) memcpy (meshData->mdelta.Map(mapChannel).vCreate.Addr(0), oCreate.Addr(0), num*sizeof(UVVert));

	meshData->Invalidate(MapChannelID(mapChannel),FALSE);
	if (isUndo) mod->NotifyDependents(FOREVER, MapChannelID(mapChannel), REFMSG_CHANGE);
}

void MapVertexEditRestore::Redo () {
	int num;
	meshData->mdelta.Map(mapChannel).vSet.SetCount (num=nSet.Count());
	if (num) memcpy (meshData->mdelta.Map(mapChannel).vSet.Addr(0), nSet.Addr(0), num*sizeof(UVVertSet));
	meshData->mdelta.Map(mapChannel).vCreate.SetCount (num=nCreate.Count());
	if (num) memcpy (meshData->mdelta.Map(mapChannel).vCreate.Addr(0), nCreate.Addr(0), num*sizeof(UVVert));

	meshData->Invalidate(MapChannelID(mapChannel),FALSE);
	mod->NotifyDependents(FOREVER, MapChannelID(mapChannel), REFMSG_CHANGE);
}

/*-------------------------------------------------------------------*/

MeshEditRestore::MeshEditRestore (EditMeshData* md, EditMeshMod *mod, MeshDelta & changer) {
	meshData = md;
	this->mod = mod;
	mapChanges.SetCount (NUM_HIDDENMAPS + changer.GetMapNum());
	changeFlags = changer.ChangeFlags (&mapChanges);
	updateMD = md->updateMD;
	if (updateMD) changeFlags = MDELTA_ALL;
	omdelta.CopyMDChannels (meshData->mdelta, changeFlags, &mapChanges);
}

MeshEditRestore::MeshEditRestore (EditMeshData* md, EditMeshMod *mod, DWORD cF) {
	meshData = md;
	this->mod = mod;
	changeFlags = cF;
	mapChanges.SetCount (md->mdelta.GetMapNum());
	for (int i=0; i<mapChanges.Count(); i++) mapChanges[i] = 0;
	updateMD = md->updateMD;
	omdelta.CopyMDChannels (meshData->mdelta, changeFlags, &mapChanges);
}

void MeshEditRestore::Restore(int isUndo) {
	nmdelta.CopyMDChannels (meshData->mdelta, changeFlags, &mapChanges);
	meshData->mdelta.CopyMDChannels (omdelta, changeFlags, &mapChanges);
	meshData->Invalidate (PART_TOPO|PART_GEOM|PART_SELECT, FALSE);	
	meshData->updateMD = updateMD;
	mod->NotifyDependents (FOREVER, PART_TOPO|PART_GEOM|PART_SELECT, REFMSG_CHANGE);
}

void MeshEditRestore::Redo () {
	meshData->mdelta.CopyMDChannels (nmdelta, changeFlags, &mapChanges);
	meshData->Invalidate (PART_TOPO|PART_GEOM|PART_SELECT, FALSE);	
	meshData->updateMD = FALSE;
	mod->NotifyDependents (FOREVER, PART_TOPO|PART_GEOM|PART_SELECT, REFMSG_CHANGE);
}

/*-------------------------------------------------------------------*/

MeshSelectRestore::MeshSelectRestore (EditMeshData * md, EditMeshMod * mm) {
	meshData = md;
	mod = mm;
	t = GetCOREInterface()->GetTime(); // LAM - defect 289808
	selLevel = mm->selLevel;
	switch (selLevel) {
	case SL_VERTEX: undo = md->mdelta.vsel; break;
	case SL_EDGE: undo = md->mdelta.esel; break;
	default: undo = md->mdelta.fsel; break;
	}
}

MeshSelectRestore::MeshSelectRestore (EditMeshData * md, EditMeshMod * mm, DWORD selLev) {
	meshData = md;
	mod = mm;
	t = GetCOREInterface()->GetTime(); // LAM - defect 289808
	selLevel = selLev;
	switch (selLev) {
	case SL_VERTEX: undo = md->mdelta.vsel; break;
	case SL_EDGE: undo = md->mdelta.esel; break;
	default: undo = md->mdelta.fsel; break;
	}
}

void MeshSelectRestore::Restore(int isUndo) {
	switch (selLevel) {
	case SL_VERTEX:
		redo = meshData->mdelta.vsel; 
		meshData->mdelta.vsel = undo;
		break;
	case SL_EDGE:
		redo = meshData->mdelta.esel;
		meshData->mdelta.esel = undo;
		break;
	default:
		redo = meshData->mdelta.fsel; 
		meshData->mdelta.fsel = undo;
		break;
	}
	if (meshData->MeshCached(t)) {
		Mesh *mesh = meshData->GetMesh(t);
		switch (selLevel) {
		case SL_VERTEX: mesh->vertSel = undo; break;
		case SL_EDGE: mesh->edgeSel = undo; break;
		default: mesh->faceSel = undo; break;
		}
		meshData->Invalidate(PART_SELECT);
	}
	mod->LocalDataChanged();
}

void MeshSelectRestore::Redo() {
	switch (selLevel) {
	case SL_VERTEX:
		meshData->mdelta.vsel = redo;
		break;
	case SL_EDGE:   			
		meshData->mdelta.esel = redo;
		break;
	default:
		meshData->mdelta.fsel = redo;
		break;
	}
	if (meshData->MeshCached(t)) {
		Mesh *mesh = meshData->GetMesh(t);
		switch (selLevel) {
		case SL_VERTEX: mesh->vertSel = redo; break;
		case SL_EDGE: mesh->edgeSel = redo; break;
		default: mesh->faceSel = redo; break;
		}
		meshData->Invalidate(PART_SELECT);
	}
	mod->LocalDataChanged();
}

/*-------------------------------------------------------------------*/

VertexHideRestore::VertexHideRestore (EditMeshData* md, EditMeshMod* mod) {
	hide = md->mdelta.vhide;
	meshData = md;
	this->mod = mod;
	t = GetCOREInterface()->GetTime(); // LAM - defect 289808
}

void VertexHideRestore::Restore(int isUndo) {
	rhide = meshData->mdelta.vhide;
	meshData->mdelta.vhide = hide;
	if (meshData->MeshCached(t)) {
		Mesh *mesh = meshData->GetMesh(t);
		mesh->vertHide = hide;		
		mesh->InvalidateGeomCache ();
	}
	mod->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews (mod->ip->GetTime());
}

void VertexHideRestore::Redo() {
	meshData->mdelta.vhide = rhide;
	if (meshData->MeshCached(t)) {
		Mesh *mesh = meshData->GetMesh(t);
		mesh->vertHide = rhide;
		mesh->InvalidateGeomCache ();
	}
	mod->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	if (mod->ip) mod->ip->RedrawViews (mod->ip->GetTime());
}

/*-------------------------------------------------------------------*/

FaceChangeRestore::FaceChangeRestore(EditMeshData* md, EditMeshMod* mod) {
	attribs = md->mdelta.fChange;
	meshData = md;
	this->mod = mod;
}

void FaceChangeRestore::Restore(int isUndo) {
	if (isUndo) rattribs = meshData->mdelta.fChange;

	meshData->mdelta.fChange = attribs;
	meshData->Invalidate(PART_TOPO,FALSE);

	mod->InvalidateSurfaceUI();
	mod->NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
}

void FaceChangeRestore::Redo() {
	meshData->mdelta.fChange = rattribs;
	meshData->Invalidate(PART_TOPO,FALSE);
	mod->InvalidateSurfaceUI();
	mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

TransformPlaneRestore::TransformPlaneRestore (EditMeshMod *emm) {
	em = emm;
	oldSliceCenter = em->sliceCenter;
	oldSliceRot = em->sliceRot;
	oldSliceSize = em->sliceSize;
}

void TransformPlaneRestore::Restore (int isUndo) {
	newSliceCenter = em->sliceCenter;
	newSliceRot = em->sliceRot;
	newSliceSize = em->sliceSize;
	em->sliceCenter = oldSliceCenter;
	em->sliceRot = oldSliceRot;
	em->sliceSize = oldSliceSize;
	em->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void TransformPlaneRestore::Redo () {
	em->sliceCenter = newSliceCenter;
	em->sliceRot = newSliceRot;
	em->sliceSize = newSliceSize;
	em->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

