/**********************************************************************
 *<
	FILE: editmops.cpp

	DESCRIPTION:  Edit Mesh OSM operations

	CREATED BY: Rolf Berteig

	HISTORY: created 30 March, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "MeshDLib.h"
#include "editmesh.h"
#include "imtl.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

void Matrix3DebugPrint (Matrix3 & tm) {
	if (tm.GetIdentFlags() & POS_IDENT) DebugPrint ("   No translation\n");
	if (tm.GetIdentFlags() & ROT_IDENT) DebugPrint ("   No rotation\n");
	if (tm.GetIdentFlags() & SCL_IDENT) DebugPrint ("   No scaling\n");
	for (int i=0; i<4; i++) {
		Point3 r = tm.GetRow(i);
		DebugPrint ("  %7.3f  %7.3f  %7.3f\n", r[0], r[1], r[2]);
	}
}

void EditMeshMod::ClearMeshDataFlag(ModContextList& mcList,DWORD f) {
	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		meshData->SetFlag(f,FALSE);
	}
}

void EditMeshMod::CloneSelSubComponents(TimeValue t) {
	if (selLevel == SL_OBJECT) return;
	if (!ip) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);
	
	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(t);

		MeshDelta tmd(*mesh);
		switch (selLevel) {
		case SL_VERTEX: tmd.CloneVerts (*mesh, mesh->vertSel); break;
		case SL_EDGE: tmd.ExtrudeEdges (*mesh, mesh->edgeSel); break;
		default: tmd.CloneFaces (*mesh, mesh->faceSel); break;
		}
		meshData->ApplyMeshDelta (tmd, this, t);

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_CLONE));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::AcceptCloneSelSubComponents(TimeValue t) {
	if (!ip) return;
	if ((selLevel==SL_OBJECT) || (selLevel==SL_EDGE)) return;
	TSTR name;
	if (!GetCloneObjectName (ip, name)) return;
	if (!ip) return;
	if (!HasActiveSelection()) return;
	Detach (name, (selLevel != SL_VERTEX), TRUE, FALSE);
}

void EditMeshMod::Transform (TimeValue t, Matrix3& partm, Matrix3 tmAxis, 
		BOOL localOrigin, Matrix3 xfrm, int type) {
	if (!ip) return;

	if (sliceMode) {
		// Special case -- just transform slicing plane.
		theHold.Put (new TransformPlaneRestore(this));
		Matrix3 tm  = partm * Inverse(tmAxis);
		Matrix3 itm = Inverse(tm);
		Matrix3 myxfm = tm * xfrm * itm;
		Point3 myTrans, myScale;
		Quat myRot;
		DecomposeMatrix (myxfm, myTrans, myRot, myScale);
		float factor;
		switch (type) {
		case 0: sliceCenter += myTrans; break;
		case 1: sliceRot *= myRot; break;
		case 2:
			factor = (float) exp(log(myScale[0]*myScale[1]*myScale[2])/3.0);
			sliceSize *= factor;
			break;
		}
		NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		ip->RedrawViews(ip->GetTime());
		return;
	}

	// Definitely transforming subobject geometry:
	DragMoveRestore ();

	// Get modcontexts
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	// Get axis type:
	int numAxis = ip->GetNumAxis();
	BOOL normMoveVerts = FALSE;

	// Special case for vertices: Only individual axis when moving in local space
	if ((selLevel==SL_VERTEX) && (numAxis==NUMAXIS_INDIVIDUAL)) {
		if (ip->GetRefCoordSys()!=COORDS_LOCAL || 
			ip->GetCommandMode()->ID()!=CID_SUBOBJMOVE) {
			numAxis = NUMAXIS_ALL;
		} else {
			normMoveVerts = TRUE;
		}
	}

	ClearMeshDataFlag(mcList,EMD_BEENDONE);
	for (int nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *meshData = (EditMeshData*)mcList[nd]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;

		// If the mesh isn't yet cache, this will cause it to get cached.
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *td = meshData->TempData(t);

		// partm is useless -- need this matrix which includes object-offset:
		Matrix3 objTM = nodes[nd]->GetObjectTM(t);

		// Selected vertices - either directly or indirectly through selected faces or edges.
		BitArray sel = mesh->VertexTempSel();
		if (!sel.NumberSet()) continue;
		MeshDelta tmd(*mesh);

		// Setup some of the affect region stuff
		Tab<Point3> *vertNorms=NULL;
		if (normMoveVerts) vertNorms = td->VertexNormals ();
		int i, nv=mesh->numVerts;

		// Compute the transforms
		if ((numAxis==NUMAXIS_INDIVIDUAL) && (selLevel != SL_VERTEX)) {
			// Do each cluster one at a time

			// If we have soft selections from multiple clusters,
			// we need to add up the vectors and divide by the total soft selection,
			// to get the right direction for movement,
			// but we also need to add up the squares of the soft selections and divide by the total soft selection,
			// essentially getting a weighted sum of the selection weights themselves,
			// to get the right scale of movement.

			// (Note that this works out to ordinary soft selections in the case of a single cluster.)

			DWORD count = (selLevel == SL_EDGE) ? td->EdgeClusters()->count : td->FaceClusters()->count;
			Tab<DWORD> *vclust = td->VertexClusters(meshLevel[selLevel]);
			float *clustDist=NULL, *sss=NULL, *ssss=NULL;
			Tab<float> softSelSum, softSelSquareSum;
			Matrix3 tm, itm;
			if (affectRegion) {
				softSelSum.SetCount(nv);
				sss = softSelSum.Addr(0);
				softSelSquareSum.SetCount (nv);
				ssss = softSelSquareSum.Addr(0);
				for (i=0; i<nv; i++) {
					sss[i] = 0.0f;
					ssss[i] = 0.0f;
				}
			}
			for (DWORD j=0; j<count; j++) {
				tmAxis = ip->GetTransformAxis (nodes[nd], j);
				tm  = objTM * Inverse(tmAxis);
				itm = Inverse(tm);
				tm *= xfrm;
				if (affectRegion) clustDist = td->ClusterDist(meshLevel[selLevel], j, useEdgeDist, edgeIts)->Addr(0);
				for (i=0; i<nv; i++) {
					if (sel[i]) {
						if ((*vclust)[i]!=j) continue;
						Point3 & old = mesh->verts[i];
						tmd.Move (i, (tm*old)*itm - old);
					} else {
						if (!affectRegion) continue;
						if (clustDist[i] < 0) continue;
						if (clustDist[i] > falloff) continue;
						float af = AffectRegionFunction (clustDist[i], falloff, pinch, bubble);
						sss[i] += fabsf(af);
						ssss[i] += af*af;
						Point3 & old = mesh->verts[i];
						tmd.Move (i, ((tm*old)*itm - old)*af);
					}
				}
			}
			if (affectRegion) {
				for (i=0; i<nv; i++) {
					if (sel[i]) continue;
					if (sss[i] == 0) continue;
					j = tmd.MoveID(i);
					if (j==(DWORD)tmd.vMove.Count()) continue;	// shouldn't happen
					if (tmd.vMove[j].vid != (DWORD)i) continue;	// shouldn't happen
					tmd.vMove[j].dv *= (ssss[i] / (sss[i]*sss[i]));
				}
			}
		} else {
			Matrix3 tm  = objTM * Inverse(tmAxis);
			Matrix3 itm = Inverse(tm);
			tm *= xfrm;
			Matrix3 ntm;
			if (numAxis == NUMAXIS_INDIVIDUAL) ntm = nodes[nd]->GetObjectTM(t);
			float *ws=NULL;
			if (affectRegion) ws = td->VSWeight(useEdgeDist, edgeIts, arIgBack, falloff, pinch, bubble)->Addr(0);
			for (i=0; i<nv; i++) {
				if (!sel[i] && (!ws || !ws[i])) continue;
				Point3 & old = mesh->verts[i];
				Point3 delta;
				if (numAxis == NUMAXIS_INDIVIDUAL) {
					MatrixFromNormal ((*vertNorms)[i], tm);
					tm  = objTM * Inverse(tm*ntm);
					itm = Inverse(tm);
					delta = itm*(xfrm*(tm*old)) - old;
				} else {
					delta = itm*(tm*old)-old;
				}
				if (sel[i]) tmd.Move (i, delta);
				else tmd.Move (i, delta * ws[i]);
			}
		}

		DragMove (tmd, meshData);
		meshData->SetFlag(EMD_BEENDONE,TRUE);		
	}

	nodes.DisposeTemporary();
	ClearMeshDataFlag(mcList,EMD_BEENDONE);
}

void EditMeshMod::Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin ) {
	Transform (t, partm, tmAxis, localOrigin, TransMatrix(val), 0);	
}

void EditMeshMod::Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin ) {
	Matrix3 mat;
	val.MakeMatrix(mat);
	Transform(t, partm, tmAxis, localOrigin, mat, 1);
}

void EditMeshMod::Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin ) {
	Transform (t, partm, tmAxis, localOrigin, ScaleMatrix(val), 2);	
}

void EditMeshMod::TransformStart (TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(TRUE);
	if (sliceMode) return;
	DragMoveInit (t, false);
}

void EditMeshMod::TransformHoldingFinish (TimeValue t) {
	if (!ip) return;
	if (sliceMode) return;
	DragMoveAccept ();
}

void EditMeshMod::TransformFinish (TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(FALSE);
}

void EditMeshMod::TransformCancel (TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(FALSE);
	DragMoveRestore ();
}

// Selection dialog ops

void EditMeshMod::HideSelectedVerts() {
	if (!ip) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());

		// Start a restore object...
		meshData->AddVertHide (mesh->vertSel, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
 	theHold.Accept(GetString(IDS_RB_HIDEVERT));
	nodes.DisposeTemporary();

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::UnhideAllVerts() {
	ModContextList mcList;
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->ClearVertHide (this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);		
	}
	theHold.Accept(GetString(IDS_RB_UNHIDEALLFACES));	// just says unhide all.
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::HideSelectedFaces() {
	ModContextList mcList;
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->BeginEdit(ip->GetTime());
		Mesh *mesh = meshData->GetMesh(ip->GetTime());

		theHold.Put (new MeshEditRestore (meshData, this, MDELTA_FCHANGE | MDELTA_FCREATE));

		for (int j=0; j<mesh->getNumFaces(); j++) {
			if (mesh->faceSel[j]) {
				mesh->faces[j].Hide();
				meshData->mdelta.FChange ((DWORD)j, ATTRIB_HIDE_FACE, ATTRIB_HIDE_FACE);
			}
		}
		mesh->InvalidateGeomCache ();
		
		BitArray emptyFaceSel;
		emptyFaceSel.SetSize (mesh->numFaces);
		meshData->SetFaceSel (emptyFaceSel, this, ip->GetTime());

		// Reset vertex hide flags
		theHold.Put (new VertexHideRestore (meshData, this));
		HiddenFacesToVerts (*mesh,meshData->mdelta.vhide);
		meshData->mdelta.vhide = mesh->vertHide;
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
		
	theHold.Accept(GetString(IDS_RB_HIDEFACE));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO|PART_DISPLAY, REFMSG_CHANGE);
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::UnhideAllFaces() {
	ModContextList mcList;
	INodeTab nodes;

	if (!ip) return;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->BeginEdit(ip->GetTime());
		Mesh *mesh = meshData->GetMesh(ip->GetTime());		

		theHold.Put(new FaceChangeRestore(meshData,this));

		for (int j=0; j<mesh->getNumFaces(); j++) {
			if (!mesh->faces[j].Hidden()) continue;
			mesh->faces[j].Show();
			meshData->mdelta.FChange ((DWORD)j, ATTRIB_HIDE_FACE, 0);
		}
		mesh->InvalidateGeomCache ();

		// Reset vertex hide flags
		HiddenFacesToVerts (*mesh, meshData->mdelta.vhide);
		meshData->SetFlag(EMD_BEENDONE,TRUE);		
	}
		
	theHold.Accept(GetString(IDS_RB_UNHIDEALLFACES));
	
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// Edit Geometry ops

DWORD EditMeshMod::CreateVertex(Point3 pt, EditMeshData *meshData, INode *nref) {
	if (!ip) return UNDEFINED;
	// Put the point in object space:
	INodeTab nodes;
	bool dispTemp=FALSE;
	if (meshData == NULL) {
		ModContextList mcList;
		ip->GetModContexts(mcList,nodes);	
		meshData = (EditMeshData*)mcList[0]->localData;
		if (!meshData) {
			nodes.DisposeTemporary();
			return UNDEFINED;
		}
		nref = nodes[0];
		dispTemp=TRUE;
	}
	meshData->BeginEdit(ip->GetTime());		
	pt = pt * Inverse(nref->GetObjectTM(ip->GetTime()));
	if (dispTemp) nodes.DisposeTemporary();

	Mesh *mesh = meshData->GetMesh(ip->GetTime());
	MeshDelta tmd(*mesh);
	DWORD ret = tmd.VCreate (&pt);
	tmd.vsel.Set (ret);

	theHold.Begin();
	meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_ADDVERTS));

	ip->RedrawViews(ip->GetTime());
	return ret;
}

bool EditMeshMod::CreateFace (EditMeshData *meshData, int *v, int deg) {
	Mesh *mesh = meshData->GetMesh(ip->GetTime());
	MeshDelta tmd(*mesh);
	if (tmd.CreatePolygon (*mesh, deg, v) == UNDEFINED) return FALSE;
	theHold.Begin ();
	meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept (GetString (IDS_RB_BUILDFACE));
	ip->RedrawViews(ip->GetTime());
	return TRUE;
}

void EditMeshMod::DeleteSelected() {
	if (selLevel == SL_OBJECT) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	int i, j, delIso = -1;
	BitArray fset, iso;
	DWORD stringID, of;
	AdjFaceList *af;

	theHold.Begin();

	for (int nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *meshData = (EditMeshData*)mcList[nd]->localData;
		if ( !meshData ) continue;
		if ( meshData->GetFlag(EMD_BEENDONE)) continue;

		Mesh *mesh=meshData->GetMesh (ip->GetTime());
		MeshDelta tmd(*mesh);

		switch (selLevel) {
		case SL_VERTEX:
			tmd.DeleteVertSet (*mesh, mesh->vertSel);
			stringID = IDS_RB_DELETEVERT;
			break;

		case SL_EDGE:
			af = meshData->TempData(ip->GetTime())->AdjFList();
			fset.SetSize(mesh->getNumFaces());

			for (i=0; i<mesh->getNumFaces(); i++) {
				for (j=0; j<3; j++) if (mesh->edgeSel[i*3+j]) break;
				if (j==3) continue;
				fset.Set(i);
				// Mark other sides of edges:
				for (; j<3; j++) {
					if (!mesh->edgeSel[i*3+j]) continue;
					if ((of=(*af)[i].f[j]) != UNDEFINED) fset.Set(of);
				}
			}

			mesh->FindVertsUsedOnlyByFaces (fset, iso);
			tmd.DeleteFaceSet (*mesh, fset);

			if (iso.NumberSet()) {
				if (delIso==-1) {
					TSTR str1 = GetString(IDS_RB_DELETEISOLATED);
					TSTR str2 = GetString(IDS_RB_DELETEFACE);
					delIso = (IDYES==MessageBox (ip->GetMAXHWnd(), str1, str2, MB_ICONQUESTION|MB_YESNO)) ? 1 : 0;
				}
				if (delIso) tmd.VDelete (iso);
			}

			stringID = IDS_RB_DELETEEDGE;
			break;

		default:
			mesh->FindVertsUsedOnlyByFaces (mesh->faceSel, iso);
			tmd.DeleteFaceSet (*mesh, meshData->mdelta.fsel);

			if (iso.NumberSet()) {
				if (delIso==-1) {
					TSTR str1 = GetString(IDS_RB_DELETEISOLATED);
					TSTR str2 = GetString(IDS_RB_DELETEFACE);
					delIso = (IDYES==MessageBox (ip->GetMAXHWnd(), str1, str2, MB_ICONQUESTION|MB_YESNO)) ? 1 : 0;
				}
				if (delIso) tmd.VDelete (iso);
			}
			stringID = IDS_RB_DELETEFACE;
			break;

		}
		if (!ip) break;
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	if (!ip) return;

	theHold.Accept(GetString(stringID));
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::Attach (INode *node, bool & canUndo) {
	// Get the attach object
	BOOL del = FALSE;
	TriObject *obj = NULL;
	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(triObjectClassID)) obj = (TriObject *) os.obj;
	else {
		if (!os.obj->CanConvertToType(triObjectClassID)) return;
		obj = (TriObject*)os.obj->ConvertToType(ip->GetTime(),triObjectClassID);
		if (obj!=os.obj) del = TRUE;
	}

	// Get our node: (We always attach to node 0.)
	ModContextList mcList;
	INodeTab nodes;	
	ip->GetModContexts(mcList,nodes);	

	// Get local data:
	EditMeshData *meshData = (EditMeshData*)mcList[0]->localData;
	if (!meshData) {
		nodes.DisposeTemporary();
		return;
	}
	Mesh *mesh = meshData->GetMesh(ip->GetTime());

	theHold.Begin();

	// Combine the materials of the two nodes.
	int mat2Offset=0;
	Mtl *m1 = nodes[0]->GetMtl();
	Mtl *m2 = node->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 && (m1 != m2)) {
		if (attachMat==ATTACHMAT_IDTOMAT) {
			int ct=1;
			if (m1->IsMultiMtl()) ct = m1->NumSubMtls();
			MeshDelta tmd(*mesh);
			tmd.RestrictMatIDs (*mesh, ct);
			meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
			FitMeshIDsToMaterial (obj->GetMesh(), m2);
			if (condenseMat) condenseMe = TRUE;
		}

		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		theHold.Suspend ();
		if (attachMat==ATTACHMAT_MATTOID) {
			m1 = FitMaterialToMeshIDs (*mesh, m1);
			m2 = FitMaterialToMeshIDs (obj->GetMesh(), m2);
		}
		Mtl *multi = CombineMaterials (m1, m2, mat2Offset);
		if (attachMat == ATTACHMAT_NEITHER) mat2Offset = 0;
		theHold.Resume ();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = selLevel;
		if (oldSL>SL_EDGE) selLevel = SL_OBJECT;
		nodes[0]->SetMtl(multi);
		if (oldSL>SL_EDGE) selLevel = oldSL;
		m1 = multi;
		canUndo = TRUE; // DS 10/14/00 -- this should work now
	}
	if (!m1 && m2) {
		// This material operation seems safe.
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = selLevel;
		if (oldSL>SL_EDGE) selLevel = SL_OBJECT;
		nodes[0]->SetMtl(m2);
		if (oldSL>SL_EDGE) selLevel = oldSL;
		m1 = m2;
	}

	// Construct a transformation that takes a vertex out of the space of
	// the source node and puts it into the space of the destination node.
	Matrix3 tm = node->GetObjectTM(ip->GetTime()) *
		Inverse(nodes[0]->GetObjectTM(ip->GetTime()));

	MeshDelta nmd;
	nmd.AttachMesh (*mesh, obj->GetMesh(), tm, mat2Offset);
	meshData->ApplyMeshDelta (nmd, this, ip->GetTime());

	// Steve Anderson 4/25/02
	// Commenting out Pete's additions below due to crash bugs resulting.
	// Adam Felt makes the following observations:
	// Pete is the owner for groups.  I think what he is doing here is saying:
	// "If you attach an object that belongs to a group, and after deleting the node
	// for the attached object you are left with an empty group, delete the group head".  
	// I would think that this behavior belongs in ip->DeleteNode().  If ever a node is
	// deleted where it would leave an empty group the group head should be deleted.
	// Notice the logic below fails with nested groups.  If the parent node is the only
	// member of another group ip->DeleteNode(par); will leave an empty group in the scene.
	//  If the group head was deleted in DeleteNode() this behavior would be nested.

	/*/ 020326  --prs.
	INode *par = node->GetParentNode();
	if (par != NULL && par->IsGroupHead()) {
		// if all of par's group children are being deleted, then delete it.
		int j;
		for (j = par->NumberOfChildren() - 1; j >= 0; j--) {
			INode *nd = par->GetChildNode(j);
			if (/* nd->IsGroupMember() && * / nd != node)
				break;
			}
		if (j >= 0)
			par = NULL;
		}*/

	ip->DeleteNode(node);

	// 020326  --prs.
	//if (par != NULL)
		//ip->DeleteNode(par);

	theHold.Accept (GetString (IDS_RB_ATTACHOBJECT));

	if (m1 && condenseMe) {
		// Have to clear undo stack.
		if (!m1->IsMultiMtl()) GetSystemSetting(SYSSET_CLEAR_UNDO);
		mesh = meshData->GetMesh(ip->GetTime());
		m1 = CondenseMatAssignments (*mesh, m1);
	}

	nodes.DisposeTemporary();
	if (del) delete obj;
}

void EditMeshMod::MultiAttach (INodeTab &nodeTab) {
	bool canUndo = TRUE;
	if (nodeTab.Count() > 1) theHold.SuperBegin ();
	for (int i=0; i<nodeTab.Count(); i++) Attach (nodeTab[i], canUndo);
	if (nodeTab.Count() > 1) theHold.SuperAccept (GetString (IDS_EM_ATTACH_LIST));
	if (!canUndo) GetSystemSetting (SYSSET_CLEAR_UNDO);
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::Detach (TSTR &name,BOOL doFaces,BOOL del, BOOL elem) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();
	if (!elem) {	// Animation confuses things.
		SuspendAnimate();
		AnimateOff();
	}

	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (doFaces) {	// defect 261134
			if (mesh->faceSel.NumberSet() == 0) continue;
		} else {
			if (mesh->vertSel.NumberSet() == 0) continue;
		}

		TriObject *newObj;
		if (!elem) newObj = CreateNewTriObject ();

		MeshDelta tmd;
		tmd.Detach (*mesh, elem ? NULL : &(newObj->GetMesh()),
			doFaces ? mesh->faceSel : mesh->vertSel, doFaces, del, elem);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		if (!elem) {
			// Add the object to the scene. Give it the given name
			// and set its transform to ours.
			// Also, give it our material.
			INode *node = ip->CreateObjectNode(newObj);
			Matrix3 ntm = nodes[i]->GetNodeTM(ip->GetTime());
			INode *nodeByName = ip->GetINodeByName (name);
			if (nodeByName != node) {	// Can happen, eg for "Object01".
				TSTR uname = name;
				if (nodeByName) ip->MakeNameUnique(uname);
				node->SetName (uname);
			}
			node->CopyProperties (nodes[i]);
			node->SetNodeTM(ip->GetTime(),ntm);
			node->FlagForeground(ip->GetTime(),FALSE);
			node->SetMtl(nodes[i]->GetMtl());
			node->SetObjOffsetPos (nodes[i]->GetObjOffsetPos());
			node->SetObjOffsetRot (nodes[i]->GetObjOffsetRot());
			node->SetObjOffsetScale (nodes[i]->GetObjOffsetScale());
		}

		meshData->SetFlag (EMD_BEENDONE, TRUE);
	}

	if (!elem) ResumeAnimate ();
	theHold.Accept (GetString (doFaces ? IDS_EM_DETACHFACES : IDS_EM_DETACHVERTS));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::BreakVerts () {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	ClearMeshDataFlag (mcList, EMD_BEENDONE);

	theHold.Begin ();
	for (int nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		if (emd->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = emd->GetMesh (ip->GetTime());
		if (!mesh->vertSel.NumberSet()) continue;

		MeshDelta tmd;
		tmd.BreakVerts (*mesh, mesh->vertSel);
		emd->ApplyMeshDelta (tmd, this, ip->GetTime());
		emd->SetFlag(EMD_BEENDONE,TRUE);
	}
	theHold.Accept (GetString (IDS_EM_VERT_BREAK));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::DoExtrusion() {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	Tab<Point3> edir;
	TimeValue t = ip->GetTime();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		if ( meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *tempd = meshData->TempData(t);
		
		MeshDelta tmd(*mesh);
		if (selLevel == SL_EDGE) {
			tmd.ExtrudeEdges (*mesh, mesh->edgeSel, &edir);
			meshData->ApplyMeshDelta (tmd, this, t);
			meshData->TempData(t)->EdgeExtDir (&edir, extType);
		} else {
			AdjEdgeList *ae = tempd->AdjEList();
			tmd.ExtrudeFaces (*mesh, mesh->faceSel, ae);
			meshData->ApplyMeshDelta (tmd, this, t);
			meshData->TempData(t)->FaceExtDir (extType);
		}
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_EXTRUDE));
	nodes.DisposeTemporary();
}

void EditMeshMod::BeginExtrude (TimeValue t) {
	if (inExtrude) return;
	inExtrude = TRUE;
	theHold.SuperBegin();
	DoExtrusion();
	DragMoveInit (t, false);
}

void EditMeshMod::EndExtrude (TimeValue t, BOOL accept) {	
	if (!inExtrude) return;
	if (!ip) return;
	inExtrude = FALSE;

	theHold.Begin ();
	DragMoveAccept ();
	theHold.Accept(GetString(IDS_DS_MOVE));
	if (accept) theHold.SuperAccept(IDS_RB_EXTRUDE);
	else theHold.SuperCancel();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		MeshTempData *tempd = meshData->TempData(t);
		tempd->freeBevelInfo();
	}
	nodes.DisposeTemporary();

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hGeom,IDC_EM_EXTRUDESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
}

void EditMeshMod::Extrude( TimeValue t, float amount ) {	
	if (!inExtrude) return;
	DragMoveRestore ();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *tempd = meshData->TempData(t);

		BitArray sel = mesh->VertexTempSel();
		Tab<Point3> *extDir = tempd->CurrentExtDir();
		if ((extDir==NULL) || (amount==0)) continue;
		Tab<DWORD> *clust = tempd->VertexClusters(meshLevel[selLevel]);

		if (!mesh->numVerts) continue;
		MeshDelta tmd(*mesh);
		tmd.Bevel (*mesh, sel, 0, NULL, amount, extDir);
		DragMove (tmd, meshData);
	}

	nodes.DisposeTemporary();
}

static int ExtDone=FALSE;

void EditMeshMod::BeginBevel (TimeValue t, BOOL doExtrude) {
	if (inBevel) return;
	inBevel = TRUE;
	theHold.SuperBegin();
	if (ExtDone=doExtrude) DoExtrusion();
	DragMoveInit (t, false);
}

void EditMeshMod::EndBevel (TimeValue t, BOOL accept) {	
	if (!inBevel) return;
	if (!ip) return;
	inBevel = FALSE;

	theHold.Begin ();
	DragMoveAccept ();
	theHold.Accept(GetString (IDS_DS_MOVE));
	if (accept)
		theHold.SuperAccept(GetString (ExtDone ? IDS_EM_BEVEL : IDS_EM_OUTLINE));
	else theHold.SuperCancel();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		MeshTempData *tempd = meshData->TempData(t);
		tempd->freeBevelInfo();
	}

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hGeom,IDC_EM_EXTRUDESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
	spin = GetISpinner(GetDlgItem(hGeom,IDC_EM_OUTLINESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
}

void EditMeshMod::Bevel (TimeValue t, float outline, float height) {	
	if (!inBevel) return;
	DragMoveRestore();

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *tempd = meshData->TempData(t);

		BitArray sel = mesh->VertexTempSel();
		MeshDelta tmd(*mesh);
		tmd.Bevel (*mesh, sel, outline, tempd->OutlineDir(extType), height, tempd->CurrentExtDir());
		DragMove (tmd, meshData);
	}

	nodes.DisposeTemporary();
}

void EditMeshMod::DoChamfer (TimeValue t) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		if ( meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *tempd = meshData->TempData(t);
		AdjEdgeList *ae = tempd->AdjEList();
		MeshChamferData *mcd = tempd->ChamferData();
		
		MeshDelta tmd(*mesh);
		if (selLevel == SL_EDGE) tmd.ChamferEdges (*mesh, mesh->edgeSel, *mcd, ae);
		else tmd.ChamferVertices (*mesh, mesh->vertSel, *mcd, ae);
		meshData->ApplyMeshDelta (tmd, this, t);
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept (GetString (IDS_EM_CHAMFER));
	nodes.DisposeTemporary();
}

void EditMeshMod::BeginChamfer (TimeValue t) {
	if (inChamfer) return;
	inChamfer = TRUE;
	theHold.SuperBegin();
	DoChamfer (t);
	DragMoveInit (t, true);
}

void EditMeshMod::EndChamfer (TimeValue t, BOOL accept) {		
	if (!ip) return;
	if (!inChamfer) return;
	inChamfer = FALSE;

	// Eliminate all the chamfer data:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		MeshTempData *tempd = meshData->TempData(t);
		tempd->freeChamferData();
	}
	nodes.DisposeTemporary();

	theHold.Begin ();
	DragMoveAccept ();
	theHold.Accept(GetString(IDS_DS_MOVE));
	if (accept) theHold.SuperAccept(GetString(IDS_EM_CHAMFER));
	else theHold.SuperCancel();

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hGeom,IDC_EM_OUTLINESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
}

void EditMeshMod::Chamfer (TimeValue t, float amount) {
	if (!inChamfer) return;
	DragMoveRestore();
	if (amount<=0) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if ( !meshData ) continue;
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *tempd = meshData->TempData(t);
		MeshChamferData *mcd = tempd->ChamferData();
		AdjEdgeList *ae = tempd->AdjEList ();

		MeshDelta tmd(*mesh);
		tmd.ChamferMove (*mesh, *mcd, amount, ae);
		tmd.MyDebugPrint (FALSE, TRUE);
		DragMove (tmd, meshData);
	}
	nodes.DisposeTemporary();
}

void EditMeshMod::AlignTo (int alignType) {
	// We'll need the viewport or construction plane transform:
	Matrix3 atm;
	ViewExp *vpt = ip->GetActiveViewport();
	if (alignType == ALIGN_CONST) {
		vpt->GetConstructionTM(atm);
	} else {
		vpt->GetAffineTM(atm);
		atm = Inverse(atm);
	}
	ip->ReleaseViewport (vpt);

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	ClearMeshDataFlag (mcList, EMD_BEENDONE);

	theHold.Begin ();
	for (int nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		if (emd->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = emd->GetMesh (ip->GetTime());
		BitArray sel = mesh->VertexTempSel ();
		if (!sel.NumberSet()) continue;

		// Need to find plane equation of alignment plane in object space.
		// if in view align, the exact z-offset of the plane is determined by the points selected.
		Matrix3 otm, res;
		otm = nodes[nd]->GetObjectTM(ip->GetTime());
		res = atm*Inverse(otm);
		Point3 ZNorm(0,0,1), zero(0,0,0);
		zero = res * zero;
		ZNorm = Normalize (VectorTransform (res, ZNorm));
		float zoff;
		if (alignType == ALIGN_VIEW) {
			zoff = 0.0f;
			for (int i=0; i<mesh->numVerts; i++) {
				if (!sel[i]) continue;
				zoff += DotProd (ZNorm, mesh->verts[i]);
			}
			zoff /= (float) sel.NumberSet();
		} else {
			zoff = DotProd (ZNorm, res.GetTrans());
		}

		float *vsw = NULL;
		if (affectRegion) vsw = emd->TempData(ip->GetTime())->VSWeight (useEdgeDist, edgeIts, arIgBack, falloff, pinch, bubble)->Addr(0);

		MeshDelta tmd(*mesh);
		tmd.MoveVertsToPlane (*mesh, sel, ZNorm, zoff);
		if (vsw) {
			// MeshDelta method doesn't cover soft selections - do that here.
			for (int i=0; i<mesh->numVerts; i++) {
				if (sel[i]) continue;	// covered.
				if (!vsw[i]) continue;
				Point3 delta = (zoff - DotProd(mesh->verts[i], ZNorm)) * ZNorm * vsw[i];
				tmd.Move (i, delta);
			}
		}
		emd->ApplyMeshDelta (tmd, this, ip->GetTime());
		emd->SetFlag(EMD_BEENDONE,TRUE);
	}
	theHold.Accept (GetString (IDS_EM_ALIGN));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

// NOTE: Following should go in class MeshDelta, method MakeSelVertsPlanar.  Going
// in here and in Editable Mesh instead so we don't break the SDK.
void MakeSelVertsPlanar (MeshDelta & md, Mesh & m, BitArray & sel, float *vsw) {
	int i, numf = m.getNumFaces();
	int numv = m.getNumVerts();

	// Find average point normals of selected vertices.  (Ignore soft selections in this part.)
	Tab<Point3> pnorm;
	pnorm.SetCount (numv);
	Tab<int> pdeg;
	pdeg.SetCount (numv);
	for (i=0; i<numv; i++) {
		if (!sel[i]) continue;
		pdeg[i] = 0;
		pnorm[i] = Point3(0,0,0);
	}

	// Calculate the vertex normals:
	for (i=0; i<numf; i++) {
		DWORD *vv = m.faces[i].v;
		for (int j=0; j<3; j++) if (sel[vv[j]]) break;
		if (j==3) continue;

		Point3 v1 = m.verts[vv[1]] - m.verts[vv[0]];
		Point3 v2 = m.verts[vv[2]] - m.verts[vv[1]];
		Point3 fnorm = Normalize(v1^v2);
		for (; j<3; j++) {
			if (!sel[vv[j]]) continue;
			pnorm[vv[j]] += fnorm;
			pdeg[vv[j]]++;
		}
	}

	// Find the cumulative center and normal:
	int nsel = 0;
	Point3 norm(0,0,0);
	Point3 cent(0,0,0);
	for (i=0; i<numv; i++) {
		if (!sel[i]) continue;
		cent += m.verts[i];
		if (pdeg[i]) norm += pnorm[i]/(float)pdeg[i];
		nsel++;
	}
	if (!nsel) return;
	cent /= float(nsel);
	norm = Normalize(norm);
	float offset = DotProd (norm, cent);
	md.MoveVertsToPlane (m, sel, norm, offset);
	if (vsw) {
		// MeshDelta method doesn't cover soft selections - do that here.
		for (i=0; i<numv; i++) {
			if (sel[i]) continue;	// covered.
			if (!vsw[i]) continue;
			Point3 delta = (offset - DotProd(m.verts[i], norm)) * norm * vsw[i];
			md.Move (i, delta);
		}
	}
}

// NOTE: Following should go in class MeshDelta, method MakeSelFacesPlanar.  Going
// in here and in Editable Mesh instead so we don't break the SDK.
void MakeSelFacesPlanar (MeshDelta & md, Mesh & m, BitArray & fsel, float *vsw) {
	int numf = m.getNumFaces();
	int numv = m.getNumVerts();
	Point3 norm(0,0,0);
	Point3 cent(0,0,0);
	int sel = 0;
	BitArray set;
	set.SetSize(numv);

	// Calculate the center of all faces and the average normal	
	for (int i=0; i<numf; i++) {
		if (!m.faceSel[i]) continue;
		Face *f = &m.faces[i];
		Point3 v1 = m.verts[f->v[1]] - m.verts[f->v[0]];
		Point3 v2 = m.verts[f->v[2]] - m.verts[f->v[1]];
		norm += v1^v2;
		cent += m.verts[f->v[0]];
		cent += m.verts[f->v[1]];
		cent += m.verts[f->v[2]];
		set.Set(f->v[0]);
		set.Set(f->v[1]);
		set.Set(f->v[2]);
		sel++;
	}
	if (!sel) return;
	cent /= float(sel*3);
	norm = Normalize(norm);
	float offset = DotProd (norm, cent);
	md.MoveVertsToPlane (m, set, norm, offset);
	if (vsw) {
		// MeshDelta method doesn't cover soft selections - do that here.
		for (i=0; i<numv; i++) {
			if (set[i]) continue;	// covered.
			if (!vsw[i]) continue;
			Point3 delta = (offset - DotProd(m.verts[i], norm)) * norm * vsw[i];
			md.Move (i, delta);
		}
	}
}

void EditMeshMod::MakePlanar() {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		float *vsw = NULL;
		if (affectRegion) vsw = meshData->TempData(ip->GetTime())->VSWeight (useEdgeDist, edgeIts, arIgBack, falloff, pinch, bubble)->Addr(0);

		MeshDelta tmd(*mesh);
		BitArray tempSel;
		int j, k;
		switch (selLevel) {
		case SL_VERTEX:
			MakeSelVertsPlanar (tmd, *mesh, mesh->vertSel, vsw);
			break;

		case SL_EDGE:
			tempSel.SetSize (mesh->numVerts);
			tempSel.ClearAll ();
			for (j=0; j<mesh->numFaces; j++) {
				for (k=0; k<3; k++) if (mesh->edgeSel[j*3+k]) break;
				if (k==3) continue;
				tempSel.Set (mesh->faces[j].v[k]);
				tempSel.Set (mesh->faces[j].v[(k+1)%3]);
				if ((k<2) && (mesh->edgeSel[j*3+(k+1)%3] || mesh->edgeSel[j*3+(k+2)%3]))
					tempSel.Set (mesh->faces[j].v[(k+2)%3]);
			}
			MakeSelVertsPlanar (tmd, *mesh, tempSel, vsw);
			break;

		default:
			MakeSelFacesPlanar (tmd, *mesh, mesh->faceSel, vsw);
			break;
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_MAKEPLANAR));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::Collapse () {
	if (selLevel == SL_OBJECT) return;
	DWORD sid;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin ();
	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());

		MeshDelta tmd(*mesh);
		switch (selLevel) {
		case SL_EDGE:
			sid = IDS_RB_COLLAPSEEDGE;
			tmd.CollapseEdges(*mesh, mesh->edgeSel, meshData->TempData(ip->GetTime())->AdjEList());
			break;
		case SL_VERTEX:
			sid = IDS_RB_COLLAPSE;
			tmd.WeldVertSet (*mesh, mesh->vertSel);
			break;
		default:
			sid = IDS_RB_FACECOLLAPSE;
			tmd.WeldVertSet (*mesh, mesh->VertexTempSel());
			break;
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
	theHold.Accept(GetString(sid));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::Tessellate (float tens,BOOL edge) {	
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);
	TimeValue t = ip->GetTime();

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(t);
		MeshTempData *tempd = meshData->TempData(t);
		AdjEdgeList *ae = tempd->AdjEList();
		AdjFaceList *af = tempd->AdjFList();

		MeshDelta tmd;
		if (edge) tmd.EdgeTessellate (*mesh, mesh->faceSel, tens, ae, af);
		else tmd.DivideFaces (*mesh, mesh->faceSel);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
	
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept (GetString(IDS_RB_TESSELLATE));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::Explode (float thresh,BOOL objs,TSTR &name) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	TimeValue t = ip->GetTime();
	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(t);
		AdjFaceList *af = meshData->TempData(t)->AdjFList ();

		MeshDelta tmd;
		if (!objs) tmd.ExplodeFaces (*mesh, thresh, (selLevel>=SL_FACE), af);
		else {
			theHold.Put (new MeshSelectRestore (meshData, this, SL_FACE));
			ExplodeToObjects (mesh, thresh, nodes[i], name, ip, &tmd, af, selLevel>=SL_FACE);
		}
		meshData->ApplyMeshDelta (tmd, this, t);
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_EXPLODE));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::Slice () {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	ClearMeshDataFlag (mcList, EMD_BEENDONE);

	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);

	theHold.Begin ();
	for (int nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		if (emd->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = emd->GetMesh (ip->GetTime());

		Matrix3 mcInv = Inverse(*mcList[nd]->tm);
		Point3 N = Normalize (VectorTransform (mcInv, Point3(0.0f,0.0f,1.0f) * rotMatrix));
		float offset = DotProd (N, mcInv*sliceCenter);

		MeshDelta tmd;
		if (selLevel >= SL_FACE) tmd.Slice (*mesh, N, offset, sliceSplit, FALSE, &(mesh->faceSel));
		else tmd.Slice (*mesh, N, offset, sliceSplit);
		emd->ApplyMeshDelta (tmd, this, ip->GetTime());
		emd->SetFlag(EMD_BEENDONE,TRUE);
	}
	theHold.Accept (GetString (IDS_EM_SLICE));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}


BOOL EditMeshMod::WeldVerts (float thresh) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	BOOL found = FALSE;
	TimeValue t = ip->GetTime();
	theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh (t);
		if (mesh->vertSel.NumberSet() < 1) continue;

		MeshDelta tmd(*mesh);
		if (tmd.WeldByThreshold (*mesh, mesh->vertSel, thresh)) found = TRUE;
		meshData->ApplyMeshDelta (tmd, this, t);

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_WELDVERTS));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
	return found;
}

void EditMeshMod::WeldVerts (Point3 weldPoint) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);
	TimeValue t = ip->GetTime();

	//theHold.Begin();

	for ( int i = 0; i < mcList.Count(); i++ ) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh (t);
		if (mesh->vertSel.NumberSet() < 1) continue;

		MeshDelta tmd(*mesh);
		tmd.WeldVertSet (*mesh, mesh->vertSel, &weldPoint);
		meshData->ApplyMeshDelta (tmd, this, t);

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	//theHold.Accept(GetString(IDS_RB_WELDVERTS));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::RemoveIsoVerts () {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts (mcList, nodes);
	ClearMeshDataFlag (mcList,EMD_BEENDONE);

	theHold.Begin ();
	for (int nd=0; nd<mcList.Count(); nd++) {
		EditMeshData *emd = (EditMeshData *) mcList[nd]->localData;
		if (!emd) continue;
		if (emd->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = emd->GetMesh (ip->GetTime());

		MeshDelta tmd(*mesh);
		tmd.DeleteIsoVerts (*mesh);
		emd->ApplyMeshDelta (tmd, this, ip->GetTime());

		emd->SetFlag(EMD_BEENDONE,TRUE);
	}
	theHold.Accept (GetString (IDS_EM_DELETE_ISOVERTS));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::SelectOpenEdges () {
	ModContextList list;
	INodeTab nodes;	
	ip->GetModContexts(list,nodes);
	ClearMeshDataFlag (list,EMD_BEENDONE);

	theHold.Begin();
	BitArray nesel;
	for (int i=0; i<list.Count(); i++) {
		EditMeshData *d = (EditMeshData*)list[i]->localData;		
		if (!d) continue;
		if (d->GetFlag (EMD_BEENDONE)) continue;
		Mesh *mesh = d->GetMesh(ip->GetTime());
		mesh->FindOpenEdges (nesel);
		d->SetEdgeSel (nesel, this, ip->GetTime());
	}
	nodes.DisposeTemporary();
	LocalDataChanged ();
	theHold.Accept(GetString (IDS_EM_SELECT_OPEN));
	ip->RedrawViews(ip->GetTime());
}

// Vertex surface operations:

float EditMeshMod::GetWeight (TimeValue t, int *numSel) {
	if (selLevel != SL_VERTEX) return 1.0f;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	float weight = 1.0f;
	int i, j, num = 0;
	for (i=0; i<mcList.Count(); i++) {
		EditMeshData *emd = (EditMeshData*) mcList[i]->localData;
		if (!emd) continue;
		Mesh *mesh = emd->GetMesh (t);
		if (emd->mdelta.vsel.NumberSet () == 0) continue;
		float *vw = mesh->getVertexWeights ();
		if (!vw) {
			if (num && (weight != 1.0f)) weight = -1.0f;
			num += emd->mdelta.vsel.NumberSet();
			continue;
		}
		for (j=0; j<emd->mdelta.vsel.GetSize(); j++) {
			if (!emd->mdelta.vsel[j]) continue;
			if (!num) weight = vw[j];
			else if (weight != vw[j]) weight = -1.0f;
			num++;
		}
	}
	if (numSel) *numSel = num;
	nodes.DisposeTemporary ();
	return weight;
}

void EditMeshMod::SetWeight (TimeValue t, float weight) {
	if (selLevel != SL_VERTEX) return;
	if (weight < MIN_WEIGHT) weight = MIN_WEIGHT;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *emd = (EditMeshData*) mcList[i]->localData;
		if (!emd) continue;
		if (emd->mdelta.vsel.NumberSet() == 0) continue;
		Mesh *mesh = emd->GetMesh (t);

		MeshDelta tmd (*mesh);
		tmd.SetVertWeights (*mesh, emd->mdelta.vsel, weight);
		emd->ApplyMeshDelta (tmd, this, t);
	}
	nodes.DisposeTemporary ();

	NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews (t);
}

void EditMeshMod::ResetWeights (TimeValue t) {
	if (selLevel != SL_VERTEX) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *emd = (EditMeshData*) mcList[i]->localData;
		if (!emd) continue;
		Mesh *mesh = emd->GetMesh (t);

		MeshDelta tmd (*mesh);
		tmd.ResetVertWeights (*mesh);
		emd->ApplyMeshDelta (tmd, this, t);
	}
	nodes.DisposeTemporary ();

	NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews (t);
}

Color EditMeshMod::GetVertColor (int mp) {
	static Color white(1,1,1), black(0,0,0);
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	Color col(white);
	BOOL init=FALSE;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (!mesh->vertSel.NumberSet()) continue;

		TVFace *cf = mesh->mapFaces(mp);
		UVVert *cv = mesh->mapVerts(mp);
		if (!cf) {
			if (init) {
				if (col != white) return black;
			} else {
				col = white;
				init = TRUE;
			}
			meshData->SetFlag (EMD_BEENDONE, TRUE);
			continue;
		}

		for (int i=0; i<mesh->getNumFaces(); i++) {
			DWORD *tt = cf[i].t;
			DWORD *vv = mesh->faces[i].v;
			for (int j=0; j<3; j++) {
				if (!mesh->vertSel[vv[j]]) continue;
				if (!init) {
					col = cv[tt[j]];
					init = TRUE;
				} else {
					Color ac = cv[tt[j]];
					if (ac!=col) return black;
				}
			}
		}
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	return col;
}

void EditMeshMod::SetVertColor (Color clr, int mp) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (!mesh->vertSel.NumberSet()) continue;

		MeshDelta tmd(*mesh);
		tmd.SetVertColors (*mesh, mesh->vertSel, clr, mp);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

float EditMeshMod::GetAlpha (int mp, int *num, bool *differs) {
	float alpha=1.0f;
	BOOL init=FALSE;
	if (num) *num=0;
	if (differs) *differs = false;
	bool vsel=false, fsel=false;
	switch (selLevel) {
	case SL_VERTEX:
		vsel=true;
		break;
	case SL_OBJECT:
	case SL_EDGE:
		return 1.0f;
	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		fsel = true;
		break;
	}

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (vsel && !mesh->vertSel.NumberSet()) continue;
		if (fsel && !mesh->faceSel.NumberSet()) continue;

		TVFace *cf = mesh->mapFaces(mp);
		UVVert *cv = mesh->mapVerts(mp);
		if (!cf) {
			if (num) (*num)++;
			if (init) {
				if (alpha != 1.0f) {
					if (differs) *differs = true;
					return alpha;
				}
			} else {
				alpha = 1.0f;
				init = TRUE;
			}
			meshData->SetFlag (EMD_BEENDONE, TRUE);
			continue;
		}

		for (int i=0; i<mesh->getNumFaces(); i++) {
			if (fsel && !mesh->faceSel[i]) continue;
			DWORD *tt = cf[i].t;
			DWORD *vv = mesh->faces[i].v;
			for (int j=0; j<3; j++) {
				if (vsel && !mesh->vertSel[vv[j]]) continue;
				if (num) (*num)++;
				if (!init) {
					alpha = cv[tt[j]].x;
					init = TRUE;
				} else {
					float ac = cv[tt[j]].x;
					if (ac!=alpha) {
						if (differs) *differs = true;
						return alpha;
					}
				}
			}
		}
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	return alpha;
}

void EditMeshMod::SetAlpha (float alpha, int mp) {
	bool vsel=false, fsel=false;
	switch (selLevel) {
	case SL_VERTEX:
		vsel=true;
		break;
	case SL_OBJECT:
	case SL_EDGE:
		return;
	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		fsel = true;
		break;
	}

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (vsel && !mesh->vertSel.NumberSet()) continue;
		if (fsel && !mesh->faceSel.NumberSet()) continue;

		MeshDelta tmd(*mesh);
		if (vsel) tmd.SetVertAlpha (*mesh, mesh->vertSel, alpha, mp);
		else tmd.SetFaceAlpha (*mesh, mesh->faceSel, alpha, mp);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::SelectVertByColor (VertColor clr, int deltaR, 
									 int deltaG, int deltaB, BOOL add, BOOL sub, int mp) {
	float dr = float(deltaR)/255.0f;
	float dg = float(deltaG)/255.0f;
	float db = float(deltaB)/255.0f;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();
	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		TVFace *cf = mesh->mapFaces(mp);
		UVVert *cv = mesh->mapVerts(mp);
		if ((!cf) || (!cv)) continue;

		BitArray nvs;
		if (add || sub) {
			nvs = mesh->vertSel;
			nvs.SetSize (mesh->numVerts, TRUE);
		} else {
			nvs.SetSize (mesh->numVerts);
			nvs.ClearAll ();
		}

		for (int i=0; i<mesh->getNumFaces(); i++) {
			for (int j=0; j<3; j++) {
				Point3 col = cv[cf[i].t[j]];
				if ((float)fabs(col.x-clr.x) > dr) continue;
				if ((float)fabs(col.y-clr.y) > dg) continue;
				if ((float)fabs(col.z-clr.z) > db) continue;
				if (sub) nvs.Clear (mesh->faces[i].v[j]);
				else nvs.Set (mesh->faces[i].v[j]);
			}
		}
		meshData->SetVertSel (nvs, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
	nodes.DisposeTemporary ();
	theHold.Accept(GetString(IDS_EM_SELBYCOLOR));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

// Edge surface operations:

void EditMeshMod::SetEdgeVis(BOOL vis) {
	ModContextList mcList;	
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		AdjFaceList *af = meshData->TempData(ip->GetTime())->AdjFList();

		MeshDelta tmd(*mesh);
		int maxedge = mesh->getNumFaces()*3;
		for (int j=0; j<maxedge; j++) {
			if (mesh->edgeSel[j]) tmd.SetSingleEdgeVis (*mesh, j, vis, af);										
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	if (vis) theHold.Accept (GetString(IDS_RB_EDGEVISIBLE));
	else theHold.Accept (GetString(IDS_RB_EDGEINVISIBLE));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::AutoEdge(float thresh, int type) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());

		AdjEdgeList *ae = meshData->TempData(ip->GetTime())->AdjEList();
		AdjFaceList *af = meshData->TempData(ip->GetTime())->AdjFList();
		Tab<MEdge> &edges = ae->edges;

		MeshDelta tmd(*mesh);
		for (int j=0; j<edges.Count(); j++) {						
			if (!edges[j].Selected(mesh->faces,meshData->mdelta.esel)) continue;
			BOOL vis = (thresh==0.0f) || (mesh->AngleBetweenFaces(edges[j].f[0],edges[j].f[1]) > thresh);
			if ((type == 1) && !vis) continue;
			if ((type == 2) && vis) continue;
			if (edges[j].f[0]!=UNDEFINED) {
				int e = mesh->faces[edges[j].f[0]].GetEdgeIndex (edges[j].v[0],edges[j].v[1]);
				tmd.SetSingleEdgeVis (*mesh, edges[j].f[0]*3+e, vis, af);
				continue;
			}
			MaxAssert (edges[j].f[1]!=UNDEFINED);
			int e = mesh->faces[edges[j].f[1]].GetEdgeIndex(edges[j].v[0],edges[j].v[1]);
			tmd.SetSingleEdgeVis (*mesh, edges[j].f[1]*3+e, vis, af);
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
	theHold.Accept(GetString(IDS_RB_AUTOEDGE));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

// Face surface operations:

void EditMeshMod::ShowNormals(DWORD vpFlags) {
	DWORD normalFlags = 0;
	switch (selLevel) {
	case SL_FACE:
	case SL_POLY:
	case SL_ELEMENT:
		if (showFNormals) normalFlags = MESH_DISP_FACE_NORMALS;
		break;
	case SL_VERTEX:
		if (showVNormals) normalFlags = MESH_DISP_VERTEX_NORMALS;
		break;
	}

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		mesh->displayNormals (normalFlags, normScale);
		mesh->buildBoundingBox ();
	}
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews (ip->GetTime(), vpFlags);
}

void EditMeshMod::FlipNormals() {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->BeginEdit(ip->GetTime());
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (mesh->faceSel.NumberSet() < 1) continue;

		MeshDelta tmd(*mesh);
		for (int j=0; j<mesh->getNumFaces(); j++) {
			if (!mesh->faceSel[j]) continue;
			tmd.FlipNormal (*mesh, j);
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_FLIPNORMALS));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::UnifyNormals() {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);	

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		AdjFaceList *af = meshData->TempData(ip->GetTime())->AdjFList();

		MeshDelta tmd(*mesh);
		tmd.UnifyNormals (*mesh, mesh->faceSel, af);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);		
	}
	
	theHold.Accept(GetString(IDS_RB_UNIFYNORMALS));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

DWORD EditMeshMod::GetMatIndex() {
	ModContextList mcList;	
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	DWORD mat=UNDEFINED;
	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		
		for (int j=0; j<mesh->getNumFaces(); j++) {
			if (!mesh->faceSel[j]) continue;
			if (mat==UNDEFINED) mat = mesh->getFaceMtlIndex(j);
			else if (mesh->getFaceMtlIndex(j) != mat) {
				nodes.DisposeTemporary ();
				return UNDEFINED;
			}
		}
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	return mat;
}

// Note: good reasons for handling theHold.Begin/Accept at higher level.
void EditMeshMod::SetMatIndex(DWORD index) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);	

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());		

		MeshDelta tmd(*mesh);
		for (int j=0; j<mesh->getNumFaces(); j++) {			
			if (mesh->faceSel[j]) tmd.SetMatID (j, MtlID(index));
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime ());

		meshData->SetFlag(EMD_BEENDONE,TRUE);		
	}
	
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::SelectByMat (DWORD index,BOOL clear) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);
	
	theHold.Begin();
	BitArray ns;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());		

		if (clear) {
			ns.SetSize (mesh->numFaces);
			ns.ClearAll ();
		} else {
			ns = mesh->faceSel;
			if (ns.GetSize() != mesh->numFaces) ns.SetSize (mesh->numFaces, TRUE);
		}
		for (int j=0; j<mesh->getNumFaces(); j++) {			
			if (mesh->getFaceMtlIndex(j)==index) ns.Set(j);
		}
		meshData->SetFaceSel (ns, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
		
	theHold.Accept(GetString(IDS_RB_SELECTBYMATID));
	nodes.DisposeTemporary();
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

DWORD EditMeshMod::GetUsedSmoothBits () {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	DWORD bits = 0;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->BeginEdit(ip->GetTime());
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		for (int j=0; j<mesh->getNumFaces(); j++)
			bits |= mesh->faces[j].smGroup;
	}
	nodes.DisposeTemporary();
	return bits;
}

// Those bits used by ANY selected faces are set in "some".
// Those bits used by ALL selected faces are set in the return value.
DWORD EditMeshMod::GetSelSmoothBits(DWORD &some) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	DWORD all = ~0;
	some = 0;
	bool someFaceSel=FALSE;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->BeginEdit(ip->GetTime());
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (!mesh->faceSel.NumberSet()) continue;
		someFaceSel = TRUE;
		for (int j=0; j<mesh->getNumFaces(); j++) {
			if (!mesh->faceSel[j]) continue;
			some |= mesh->faces[j].smGroup;
			all &= mesh->faces[j].smGroup;
		}
	}
	nodes.DisposeTemporary();
	if (!someFaceSel) return 0;
	return all;
}

void EditMeshMod::SetSelSmoothBits(DWORD bits,DWORD mask) {
	ModContextList mcList;	
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	theHold.Begin();
	bits &= mask;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (mesh->faceSel.NumberSet() < 1) continue;

		MeshDelta tmd (*mesh);
		for (int j=0; j<mesh->getNumFaces(); j++) {
			if (!mesh->faceSel[j]) continue;
			tmd.FSmooth (j, mask, bits);
		}
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_SETSMOOTHGROUP));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::SelectBySmoothGroup (DWORD bits,BOOL clear) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);
	
	theHold.Begin();
	BitArray nfs;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		meshData->BeginEdit(ip->GetTime());
		Mesh *mesh = meshData->GetMesh(ip->GetTime());		
		
		if (clear) {
			nfs.SetSize (mesh->getNumFaces());
			nfs.ClearAll();
		} else {
			nfs = mesh->faceSel;
			nfs.SetSize (mesh->getNumFaces(), TRUE);
		}
		for (int j=0; j<mesh->getNumFaces(); j++) {			
			if (mesh->faces[j].smGroup & bits) nfs.Set(j);
		}

		meshData->SetFaceSel (nfs, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
		
	theHold.Accept(GetString(IDS_RB_SELECTBYSMOOTH));
	nodes.DisposeTemporary();
	LocalDataChanged();
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::AutoSmooth(float thresh) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);	

	theHold.Begin();

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (mesh->faceSel.NumberSet() < 1) continue;

		AdjFaceList *af = meshData->TempData(ip->GetTime())->AdjFList();
		AdjEdgeList *ae = meshData->TempData(ip->GetTime())->AdjEList();

		MeshDelta tmd(*mesh);
		tmd.AutoSmooth (*mesh, mesh->faceSel, thresh, af, ae);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());

		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	theHold.Accept(GetString(IDS_RB_AUTOSMOOTH));
	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

Color EditMeshMod::GetFaceColor (int mp) {
	static Color white(1,1,1), black(0,0,0);
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	Color col = white;
	BOOL init=FALSE;

	for (int i = 0; i < mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh(ip->GetTime());
		if (!mesh->faceSel.NumberSet()) continue;

		TVFace *cf=mesh->mapFaces(mp);
		UVVert *cv=mesh->mapVerts(mp);
		if (!cf) {
			if (init) {
				if (col != white) break;
			} else {
				col = white;
				init = TRUE;
			}
			meshData->SetFlag (EMD_BEENDONE, TRUE);
			continue;
		}

		for (int k=0; k<mesh->getNumFaces(); k++) {
			if (!mesh->faceSel[k]) continue;
			DWORD *tt = cf[k].t;
			int j;
			for (j=0; j<3; j++) {
				if (!init) {
					col = cv[tt[j]];
					init = TRUE;
				} else {
					Color ac = cv[tt[j]];
					if (ac!=col) break;
				}
			}
			if (j<3) break;
		}
		if (k<mesh->numFaces) break;
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}
	nodes.DisposeTemporary();
	return (i<mcList.Count()) ? black : col;
}

void EditMeshMod::SetFaceColor(Color clr, int mp) {
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	ClearMeshDataFlag(mcList,EMD_BEENDONE);

	for (int i=0; i<mcList.Count(); i++) {
		EditMeshData *meshData = (EditMeshData*)mcList[i]->localData;
		if (!meshData) continue;
		if (meshData->GetFlag(EMD_BEENDONE)) continue;
		Mesh *mesh = meshData->GetMesh (ip->GetTime());
		if (!mesh->faceSel.NumberSet()) continue;

		MeshDelta tmd(*mesh);
		tmd.SetFaceColors (*mesh, mesh->faceSel, clr, mp);
		meshData->ApplyMeshDelta (tmd, this, ip->GetTime());
		meshData->SetFlag(EMD_BEENDONE,TRUE);
	}

	nodes.DisposeTemporary();
	ip->RedrawViews(ip->GetTime());
}

//----Globals----------------------------------------------
// Move to class Interface or someplace someday?

void ExplodeToObjects (Mesh *mesh, float thresh, INode *node, TSTR &name,
					   IObjParam *ip, MeshDelta *tmd, AdjFaceList *af, BOOL selOnly) {
	FaceClusterList fc(mesh, *af, thresh, selOnly);
	if (fc.count == 0) return;
	if (!selOnly || (mesh->faceSel.NumberSet() == mesh->numFaces)) {
		if (fc.count==1) return;	// nothing to do -- all in one cluster.
		if (!selOnly) mesh->faceSel.SetAll();
		for (int i=0; i<mesh->numFaces; i++) if (fc[i] == fc.count-1) mesh->faceSel.Clear (i);
		fc.count--;
	}

	// For speed, make face-lists for each cluster:
	int j,k, mp;
	DWORD i;
	DWORDTab *flist = new DWORDTab[fc.count];
	for (i=0; i<(DWORD)mesh->numFaces; i++) {
		if (!mesh->faceSel[i]) continue;
		flist[fc[i]].Append (1, &i, 10);
	}

	// Make used, lookup arrays big enough to hold verts or any mapping vert set.
	int vsize = mesh->numVerts;
	for (mp=0; mp<mesh->getNumMaps(); mp++) {
		if (!mesh->mapSupport(mp)) continue;
		if (mesh->getNumMapVerts (mp) > vsize) vsize = mesh->getNumMapVerts(mp);
	}
	BitArray usedVerts;
	usedVerts.SetSize (vsize);
	DWORD *vlut = new DWORD[vsize];

	// Transfer all clusters to new meshes.
	for (i=0; i<fc.count; i++) {
		if (flist[i].Count() < 1) continue;	// shouldn't happen.

		usedVerts.ClearAll();
		for (j=0; j<flist[i].Count(); j++) {
			DWORD *vv = mesh->faces[flist[i][j]].v;
			usedVerts.Set (vv[0]);
			usedVerts.Set (vv[1]);
			usedVerts.Set (vv[2]);
		}

		TriObject *newObj = CreateNewTriObject();
		Mesh *newMesh = &newObj->GetMesh();
		newMesh->setNumVerts(usedVerts.NumberSet());
		newMesh->setNumFaces(flist[i].Count());
		for (j=0,k=0; j<mesh->numVerts; j++) {
			if (!usedVerts[j]) continue;
			newMesh->verts[k] = mesh->verts[j];
			vlut[j] = k++;
		}
		for (j=0; j<flist[i].Count(); j++) {
			Face & f = mesh->faces[flist[i][j]];
			newMesh->faces[j] = f;
			for (k=0; k<3; k++) newMesh->faces[j].v[k] = vlut[f.v[k]];
		}

		// Handle mapping.
		newMesh->setNumMaps (mesh->getNumMaps ());
		for (mp=0; mp<mesh->getNumMaps(); mp++) {
			if (!mesh->mapSupport(mp)) {
				newMesh->setMapSupport (mp, FALSE);
				continue;
			}
			newMesh->setMapSupport (mp, TRUE);

			TVFace *mapf = mesh->mapFaces(mp);
			UVVert *mapv = mesh->mapVerts(mp);
			TVFace *nmapf = newMesh->mapFaces(mp);
			usedVerts.ClearAll ();
			for (j=0; j<flist[i].Count(); j++) {
				DWORD *vv = mapf[flist[i][j]].t;
				usedVerts.Set (vv[0]);
				usedVerts.Set (vv[1]);
				usedVerts.Set (vv[2]);
			}

			newMesh->setNumMapVerts (mp, usedVerts.NumberSet());
			for (j=0,k=0; j<mesh->getNumMapVerts(mp); j++) {
				if (!usedVerts[j]) continue;
				newMesh->setMapVert (mp, k, mapv[j]);
				vlut[j] = k++;
			}
			for (j=0; j<flist[i].Count(); j++) {
				TVFace & f = mapf[flist[i][j]];
				nmapf[j] = f;
				for (k=0; k<3; k++) nmapf[j].t[k] = vlut[f.t[k]];
			}
		}

		INode *nd = ip->CreateObjectNode(newObj);
		Matrix3 ntm = node->GetNodeTM(ip->GetTime());
		TSTR uname = name;
		ip->MakeNameUnique(uname);
		nd->SetName(uname);
		nd->CopyProperties (node);
		nd->SetNodeTM (ip->GetTime(), ntm);
		nd->FlagForeground (ip->GetTime(), FALSE);
		nd->SetMtl (node->GetMtl());
		nd->SetObjOffsetPos (node->GetObjOffsetPos());
		nd->SetObjOffsetRot (node->GetObjOffsetRot());
		nd->SetObjOffsetScale (node->GetObjOffsetScale());
	}
	delete [] vlut;
	delete [] flist;

	// Finally, set up meshdelta to delete components from this mesh.
	tmd->ClearAllOps ();
	tmd->InitToMesh (*mesh);
	BitArray vdel;
	mesh->FindVertsUsedOnlyByFaces (mesh->faceSel, vdel);
	tmd->FDelete (mesh->faceSel);
	tmd->VDelete (vdel);
}

BOOL CreateCurveFromMeshEdges (Mesh & mesh, INode *onode, Interface *ip, AdjEdgeList *ae,
							   TSTR & name, BOOL curved, BOOL ignoreHiddenEdges) {
	if (!mesh.edgeSel.NumberSet()) return FALSE;

	SuspendAnimate();
	AnimateOff();	

	SplineShape *shape = (SplineShape*)GetSplineShapeDescriptor()->Create(0);	

	BitArray done;
	done.SetSize(ae->edges.Count());

	// Mark hidden edges as done to ignore them
	if (ignoreHiddenEdges) {
		for (int i=0; i<ae->edges.Count(); i++) {
			if (!ae->edges[i].Visible(mesh.faces)) done.Set(i);
		}
	}

	for (int i=0; i<ae->edges.Count(); i++) {
		if (done[i]) continue;
		if (!ae->edges[i].Selected (mesh.faces,mesh.edgeSel)) continue;
			
		// The array of points for the spline
		Tab<Point3> pts;
		
		// Add the first two points.
		pts.Append(1,&mesh.verts[ae->edges[i].v[0]],10);
		pts.Append(1,&mesh.verts[ae->edges[i].v[1]],10);
		int nextv = ae->edges[i].v[1], start = ae->edges[i].v[0];
		
		// Mark this edge as done
		done.Set(i);

		// Trace along selected edges
		while (1) {
			DWORDTab &ve = ae->list[nextv];
			for (int j=0; j<ve.Count(); j++) {
				if (done[ve[j]]) continue;
				if (ae->edges[ve[j]].Selected(mesh.faces,mesh.edgeSel)) break;
			}
			if (j==ve.Count()) break;
			if (ae->edges[ve[j]].v[0]==(DWORD)nextv) nextv = (int)ae->edges[ve[j]].v[1];
			else nextv = ae->edges[ve[j]].v[0];

			// Mark this edge as done
			done.Set(ve[j]);

			// Add this vertex to the list
			pts.Append(1,&mesh.verts[nextv],10);
		}
		int lastV = nextv;

		// Now trace backwards
		nextv = start;
		while (1) {
			DWORDTab &ve = ae->list[nextv];
			for (int j=0; j<ve.Count(); j++) {
				if (done[ve[j]]) continue;
				if (ae->edges[ve[j]].Selected(mesh.faces,mesh.edgeSel)) break;
			}
			if (j==ve.Count()) break;
			if (ae->edges[ve[j]].v[0]==(DWORD)nextv) nextv = (int)ae->edges[ve[j]].v[1];
			else nextv = ae->edges[ve[j]].v[0];

			// Mark this edge as done
			done.Set(ve[j]);

			// Add this vertex to the list
			pts.Insert(0,1,&mesh.verts[nextv]);
		}
		int firstV = nextv;

		// Now weve got all th points. Create the spline and add points
		Spline3D *spline = new Spline3D(KTYPE_AUTO,KTYPE_BEZIER);
		int max = pts.Count();
		if (firstV == lastV) {
			max--;
			spline->SetClosed ();
		}
		if (curved) {
			for (int j=0; j<max; j++) {
				int prvv = j ? j-1 : ((firstV==lastV) ? max-1 : 0);
				int nxtv = (max-1-j) ? j+1 : ((firstV==lastV) ? 0 : max-1);
				float prev_length = Length(pts[j] - pts[prvv])/3.0f;
				float next_length = Length(pts[j] - pts[nxtv])/3.0f;
				Point3 tangent = Normalize (pts[nxtv] - pts[prvv]);
				SplineKnot sn (KTYPE_BEZIER, LTYPE_CURVE, pts[j],
						pts[j] - prev_length*tangent, pts[j] + next_length*tangent);
				spline->AddKnot(sn);
			}
		} else {
			for (int j=0; j<max; j++) {
				SplineKnot sn(KTYPE_CORNER, LTYPE_LINE, pts[j],pts[j],pts[j]);
				spline->AddKnot(sn);
			}
			spline->ComputeBezPoints();
		}
		shape->shape.AddSpline(spline);
	}

	shape->shape.InvalidateGeomCache();
	shape->shape.UpdateSels();

	INode *node = ip->CreateObjectNode (shape);
	INode *nodeByName = ip->GetINodeByName (name);
	if (nodeByName != node) {
		if (nodeByName) ip->MakeNameUnique(name);
		node->SetName (name);
	}
	Matrix3 ntm = onode->GetNodeTM(ip->GetTime());
	node->CopyProperties (onode);
	node->SetNodeTM (ip->GetTime(),ntm);
	node->FlagForeground (ip->GetTime(),FALSE);
	node->SetMtl (onode->GetMtl());
	node->SetObjOffsetPos (onode->GetObjOffsetPos());
	node->SetObjOffsetRot (onode->GetObjOffsetRot());
	node->SetObjOffsetScale (onode->GetObjOffsetScale());	
	ResumeAnimate();
	ip->RedrawViews(ip->GetTime());
	return TRUE;
}

/*------------Command modes & Mouse procs----------------------*/

HitRecord *PickEdgeMouseProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, float *prop, Point3 *snapPoint) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (!hr) return hr;
	if (!prop) return hr;

	// Find where along this edge we hit
	// Strategy:
	// Get Mouse click, plus viewport z-direction at mouse click, in object space.
	// Then find the direction of the edge in a plane orthogonal to z, and see how far
	// along that edge we are.

	EditMeshData *meshData = (EditMeshData*)hr->modContext->localData;
	DWORD ee = hr->hitInfo;

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	Point3 Zdir = Normalize (r.dir);
	if (!snapPoint) snapPoint = &(r.p);

	Mesh *mm = meshData->GetMesh (ip->GetTime());
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());
	Point3 A = obj2world * mm->verts[mm->faces[ee/3].v[ee%3]];
	Point3 B = obj2world * mm->verts[mm->faces[ee/3].v[(ee+1)%3]];
	Point3 Xdir = B-A;
	Xdir -= DotProd(Xdir, Zdir)*Zdir;
	*prop = DotProd (Xdir, *snapPoint-A) / LengthSquared (Xdir);
	if (*prop<.0001f) *prop=0;
	if (*prop>.9999f) *prop=1;
	return hr;
}

int PickEdgeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt;
	HitRecord *hr;
	float prop;
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint(m,m,NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestEdges (m, vpt, &prop, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
		if (hr) {
			EditMeshData *meshData = (EditMeshData*)hr->modContext->localData;
			EdgePick (meshData, hr->hitInfo, prop);
		}
		break;
		
	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);//|SNAP_SEL_OBJS_ONLY);
		if (HitTestEdges(m,vpt,NULL,NULL)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;	
}

// --------------------------------------------------------

HitRecord *PickFaceMouseProc::HitTestFaces (IPoint2 &m, ViewExp *vpt, float *bary, Point3 *snapPoint) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (!hr) return hr;
	if (!bary) return hr;

	// Find barycentric coordinates of hit in face.
	// Strategy:
	// Get Mouse click, plus viewport z-direction at mouse click, in object space.
	// Then find face coords in a plane orthogonal to z, and compare hit to points.

	EditMeshData *meshData = (EditMeshData*)hr->modContext->localData;
	DWORD ff = hr->hitInfo;
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	Point3 Zdir = Normalize (r.dir);
	if (!snapPoint) snapPoint = &(r.p);

	Mesh *mm = meshData->GetMesh (ip->GetTime());
	Point3 A = obj2world * mm->verts[mm->faces[ff].v[0]];
	Point3 B = obj2world * mm->verts[mm->faces[ff].v[1]];
	Point3 C = obj2world * mm->verts[mm->faces[ff].v[2]];
	Point3 Xdir = B-A;
	Xdir = Normalize (Xdir - DotProd(Xdir, Zdir)*Zdir);
	Point3 Ydir = Zdir^Xdir;	// orthonormal basis
	bary[2] = DotProd (*snapPoint-A, Ydir) / DotProd (C-A, Ydir);
	if (bary[2] < .0001f) bary[2] = 0.f;
	if (bary[2] > .9999f) {
		bary[2] = 1.0f;
		bary[0] = bary[1] = 0.0f;
		return hr;
	}
	A += bary[2] * (C-A);
	B += bary[2] * (C-B);
	bary[1] = DotProd (Xdir, *snapPoint-A) / DotProd (Xdir, B-A);
	bary[1] *= (1.0f-bary[2]);
	if (bary[1] < .0001f) bary[1] = 0.f;
	if (bary[1] > .9999f) bary[1] = 1.f;
	if (bary[1] + bary[2] > .9999f) {
		float prop = 1.0f / (bary[1] + bary[2]);
		bary[1] *= prop;
		bary[2] *= prop;
		bary[0] = 0.0f;
		return hr;
	}
	bary[0] = 1.0f - bary[1] - bary[2];
	return hr;
}

int PickFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt = ip->GetViewport(hwnd);
	HitRecord *hr;
	float bary[3];
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint(m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestFaces(m,vpt,bary, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
		if (hr) {
			EditMeshData *meshData = (EditMeshData *) hr->modContext->localData;
			FacePick (meshData, hr->hitInfo, bary);
		}
		break;
	
	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);//|SNAP_SEL_OBJS_ONLY);
		if (HitTestFaces(m,vpt,NULL,NULL)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;
}

// -------------------------------------------------------

void CreateVertCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void CreateVertCMode::ExitMode() {
	if ( !em->hGeom ) return;
	em->EndExtrude(em->ip->GetTime(),TRUE);
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// following cursor used both here and in CreateFaceMouseProc
static HCURSOR hCurCreateVerts = NULL;
int CreateVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	if (!hCurCreateVerts) hCurCreateVerts = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR));

	ViewExp *vpt;

	Matrix3 ctm;
	Point3 pt;
	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		vpt->GetConstructionTM(ctm);
		pt = vpt->SnapPoint(m,m,&ctm);
		if (vpt) ip->ReleaseViewport(vpt);
		pt = pt * ctm;
		em->CreateVertex(pt);
		break;
	
	case MOUSE_FREEMOVE:
		SetCursor(hCurCreateVerts);
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview(m,m,NULL, SNAP_FORCE_3D_RESULT);
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;
}

//----------------------------------------------------------

void CreateFaceCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	em->inBuildFace = TRUE;
	em->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	em->ip->RedrawViews(em->ip->GetTime());
}

void CreateFaceCMode::ExitMode() {
	if (!em->hGeom ) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	em->inBuildFace = FALSE;
	em->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	em->ip->RedrawViews(em->ip->GetTime());
}

CreateFaceMouseProc::CreateFaceMouseProc(EditMeshMod* mod, IObjParam *i) {
	em = mod;
	ip = i;
	meshData = NULL;
	pt = 0;
}

BOOL CreateFaceMouseProc::HitTestVerts(IPoint2 m, ViewExp *vpt, int &v) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return FALSE;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	MaxAssert(hr);
	if ((meshData != hr->modContext->localData) && (meshData != NULL)) return FALSE;
	if (em->selLevel != SL_POLY) {
		for (int i=0; i<pt; i++) if (vts[i]==(int)hr->hitInfo) return FALSE;
	}
	v = hr->hitInfo;
	return TRUE;
}
		
// We assume the transform, color, render style, etc, has been set up in advance.
void CreateFaceMouseProc::DrawEstablishedFace (GraphicsWindow *gw) {
	if (pt<2) return;
	if (!meshData) return;
	Mesh *mesh = meshData->GetMesh (ip->GetTime());
	Tab<Point3> rverts;
	rverts.SetCount (pt+1);
	for (int j=0; j<pt; j++) {
		rverts[j] = mesh->verts[vts[j]];
	}
	gw->polyline (pt, rverts.Addr(0), NULL, NULL, FALSE, NULL);
}

void CreateFaceMouseProc::DrawCreatingFace (HWND hWnd, const IPoint2 & m) {
	if (!pt) return;
	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));

	MoveToEx (hdc, mfirst.x, mfirst.y, NULL);
	LineTo (hdc, m.x, m.y);
	if (pt>1) LineTo(hdc, mlast.x, mlast.y);

	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

int CreateFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	if (!hCurCreateVerts) hCurCreateVerts = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR));

	ViewExp *vpt = ip->GetViewport(hwnd);
	ModContextList mcList;	
	INodeTab nodes;
	int dummyVert;
	int nv, lpt;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (point==1) break;
		ip->SetActiveViewport(hwnd);
		bool done;
		done=FALSE;

		if (HitTestVerts (m, vpt, nv)) {
			HitLog& hitLog = vpt->GetSubObjHitList();
			HitRecord *hr = hitLog.ClosestHit();
			MaxAssert(hr);
			for (int j=0; j<pt; j++) if (vts[j] == nv) break;
			if (!meshData) {
				// The first point
				meshData = (EditMeshData*)hr->modContext->localData;
				nref = hr->nodeRef;
			}
			if (j<pt) done = TRUE;
			else {
				vts.Append (1, &nv, 20);
				if (pt==0) mfirst = m;
				else mlast = m;
				pt++;
				em->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				ip->RedrawViews (ip->GetTime());
				oldm = m;
				DrawCreatingFace(hwnd, m);
			}
		} else if (flags & MOUSE_SHIFT) {
			Matrix3 ctm;
			vpt->GetConstructionTM (ctm);
			Point3 newpt = vpt->SnapPoint (m, m, &ctm, SNAP_FORCE_3D_RESULT|SNAP_SEL_SUBOBJ_ONLY);
			newpt = newpt * ctm;
			if (!meshData) {
				// The first point
				ip->GetModContexts (mcList, nodes);
				meshData = (EditMeshData *) mcList[0]->localData;
				nref = nodes[0];
			}
			nv = meshData->GetMesh(ip->GetTime())->numVerts;
			vts.Append (1, &nv, 20);
			if (pt==0) mfirst = m;
			else mlast = m;
			pt++;
			em->CreateVertex (newpt, meshData, nref);
			oldm = m;
			DrawCreatingFace(hwnd, m);
		} else {
			if (!pt) return false;
		}

		if ((em->selLevel != SL_POLY) && (pt==3)) done = TRUE;

		if (done) {
			// We're done collecting verts - build a face
			lpt = pt;
			pt = 0;	// so the redraw gets that we're done.
			if ((lpt>2) && !em->CreateFace (meshData, vts.Addr(0), lpt)) {
				InvalidateRect(vpt->getGW()->getHWnd(),NULL,FALSE);
				TSTR buf1 = GetString(IDS_RB_DUPFACEWARNING);
				TSTR buf2 = GetString(IDS_RB_EDITMESHTITLE);
				MessageBox (ip->GetMAXHWnd(), buf1, buf2, MB_OK|MB_ICONINFORMATION);
				em->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				ip->RedrawViews (ip->GetTime());
			}
			vts.SetCount(0);
			meshData = NULL;
			ip->ReleaseViewport (vpt);
			return FALSE;
		}
		break;

	case MOUSE_MOVE:
		if (pt) DrawCreatingFace (hwnd, oldm);	// Erase old outline
		if (HitTestVerts(m,vpt,dummyVert))
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else {
			if (flags & MOUSE_SHIFT) SetCursor (hCurCreateVerts);
			else SetCursor (LoadCursor (NULL, IDC_ARROW));
			vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
			ip->RedrawViews (ip->GetTime());
		}
		if (pt) {
			oldm = m;
			DrawCreatingFace (hwnd, oldm);
		}
		break;

	case MOUSE_FREEMOVE:
		if (HitTestVerts(m,vpt,dummyVert))
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else {
			if (flags & MOUSE_SHIFT) SetCursor (hCurCreateVerts);
			else SetCursor (LoadCursor (NULL, IDC_ARROW));
			vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		}
		break;

	case MOUSE_ABORT:
		pt = 0;
		vts.SetCount(0);
		meshData = NULL;
		em->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		ip->RedrawViews (ip->GetTime());
		break;
	}

	ip->ReleaseViewport(vpt);
	return TRUE;
}

/*-----------------------------------------------------------------------*/

BOOL AttachPickMode::Filter(INode *node) {
	if (!em) return false;
	if (!ip) return false;
	if (!node) return FALSE;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	em->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(triObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(triObjectClassID)) return TRUE;
	return FALSE;
}

BOOL AttachPickMode::HitTest (IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags) {
	if (!em) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd, m, this) ? TRUE : FALSE;
}

BOOL AttachPickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!em) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return FALSE;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	// (We always attach to node 0.)

	BOOL ret = TRUE;
	if (nodes[0]->GetMtl() && node->GetMtl() && (nodes[0]->GetMtl()!=node->GetMtl())) {
		ret = DoAttachMatOptionDialog (ip, em);
	}
	nodes.DisposeTemporary ();
	if (!ret) return FALSE;
	if (!em->ip) return FALSE;

	bool canUndo = TRUE;
	em->Attach (node, canUndo);
	if (!canUndo) GetSystemSetting (SYSSET_CLEAR_UNDO);
	ip->RedrawViews(ip->GetTime());
	return FALSE;
}

void AttachPickMode::EnterMode(IObjParam *ip) {
	if (!em) return;
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_OBJ_ATTACH));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void AttachPickMode::ExitMode(IObjParam *ip) {
	if (!em) return;
	if ( !em->hGeom ) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_OBJ_ATTACH));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// -----------------------------

int MAttachHitByName::filter(INode *node) {
	if (!node) return FALSE;

	// Make sure the node does not depend on this modifier.
	node->BeginDependencyTest();
	em->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(em->ip->GetTime());
	if (os.obj->IsSubClassOf(triObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(triObjectClassID)) return TRUE;
	return FALSE;
}

void MAttachHitByName::proc(INodeTab &nodeTab) {
	if (inProc) return;
	inProc = TRUE;
	ModContextList mcList;
	INodeTab nodes;
	em->ip->GetModContexts (mcList, nodes);
	BOOL ret = TRUE;
	if (nodes[0]->GetMtl()) {
		for (int i=0; i<nodeTab.Count(); i++) {
			if (nodeTab[i]->GetMtl() && (nodes[0]->GetMtl()!=nodeTab[i]->GetMtl())) break;
		}
		if (i<nodeTab.Count()) ret = DoAttachMatOptionDialog (em->ip, em);
	}
	nodes.DisposeTemporary ();
	inProc = FALSE;
	if (!ret) return;
	if (!em->ip) return;
	em->MultiAttach (nodeTab);
}

//----------------------------------------------------------

void DivideEdgeProc::EdgePick(EditMeshData *meshData, DWORD edge, float prop) {
	Mesh *mesh = meshData->GetMesh(ip->GetTime());
	theHold.Begin ();
	MeshDelta tmd;
	tmd.DivideEdge (*mesh, edge, prop, meshData->TempData(ip->GetTime())->AdjEList());
	meshData->ApplyMeshDelta (tmd, em, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_EDGEDIVIDE));

	ip->RedrawViews(ip->GetTime());	
}

void DivideEdgeCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_DIVIDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void DivideEdgeCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_DIVIDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//----------------------------------------------------------

void DivideFaceProc::FacePick (EditMeshData *meshData, DWORD ff, float *bary) {
	Mesh *mesh = meshData->GetMesh(ip->GetTime());
	theHold.Begin ();
	MeshDelta tmd;
	tmd.DivideFace (*mesh, ff, bary);
	meshData->ApplyMeshDelta (tmd, em, ip->GetTime());
	theHold.Accept(GetString(IDS_EM_FACE_DIVIDE));
	ip->RedrawViews(ip->GetTime());	
}

void DivideFaceCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom, IDC_EM_DIVIDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void DivideFaceCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom, IDC_EM_DIVIDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------------------

void TurnEdgeProc::EdgePick(EditMeshData *meshData,DWORD edge, float prop) {
	Mesh *mesh = meshData->GetMesh(ip->GetTime());

	theHold.Begin ();
	MeshDelta tmd(*mesh);
	tmd.TurnEdge (*mesh, edge);
	meshData->ApplyMeshDelta (tmd, em, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_EDGETURN));

	ip->RedrawViews(ip->GetTime());
}

void TurnEdgeCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_TURN));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void TurnEdgeCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_TURN));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int ExtrudeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {	
	ViewExp *vpt = ip->GetViewport(hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin;
	float amount;
	IPoint2 m2;

	switch ( msg ) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			em->BeginExtrude(ip->GetTime());
			om = m;
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			em->EndExtrude(ip->GetTime(),TRUE);
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om,float(-200));
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView (m2, float(-200));
		amount = Length (p1-p0);
		if (m.y > om.y) amount *= -1.0f;
		em->Extrude (ip->GetTime(), amount);

		spin = GetISpinner(GetDlgItem(em->hGeom,IDC_EM_EXTRUDESPINNER));
		if (spin) {
			spin->SetValue (amount, FALSE);
			ReleaseISpinner(spin);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		em->EndExtrude(ip->GetTime(),FALSE);			
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		break;		
	}
	
	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ExtrudeSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDECUR));
	return hCur;
}

void ExtrudeCMode::EnterMode() {
	if (!em->hGeom ) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ExtrudeCMode::ExitMode() {
	if (!em->hGeom ) return;
	em->EndExtrude(em->ip->GetTime(),TRUE);
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_EXTRUDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int BevelMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin;
	float amount;
	IPoint2 m2;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			em->BeginBevel (ip->GetTime(), TRUE);
			break;
		case 1:
			m1 = m;
			m1set = TRUE;
			p0 = vpt->MapScreenToView (m0, float(-200));
			m2.x = m0.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView (m2, float(-200));
			height = Length (p0-p1);
			if (m.y > m0.y) height *= -1.0f;
			em->Bevel (ip->GetTime(), 0, height);
			spin = GetISpinner(GetDlgItem(em->hGeom,IDC_EM_EXTRUDESPINNER));
			if (spin) {
				spin->SetValue (height, FALSE);
				ReleaseISpinner (spin);
			}
			break;
		case 2:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			em->EndBevel(ip->GetTime(),TRUE);
			m1set = m0set = FALSE;
			if (vpt) ip->ReleaseViewport(vpt);
			return FALSE;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
		m2.y = m.y;
		if (!m1set) {
			p0 = vpt->MapScreenToView (m0, float(-200));
			m2.x = m0.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			amount = Length (p1-p0);
			if (m.y > m0.y) amount *= -1.0f;
			em->Bevel (ip->GetTime(), 0, amount);
			spin = GetISpinner(GetDlgItem(em->hGeom,IDC_EM_EXTRUDESPINNER));
		} else {
			p0 = vpt->MapScreenToView (m1, float(-200));
			m2.x = m1.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			amount = Length (p1-p0);
			if (m.y > m1.y) amount *= -1.0f;
			em->Bevel (ip->GetTime(), amount, height);
			spin = GetISpinner(GetDlgItem(em->hGeom,IDC_EM_OUTLINESPINNER));
		}
		if (spin) {
			spin->SetValue (amount, FALSE);
			ReleaseISpinner(spin);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		em->EndBevel(ip->GetTime(),FALSE);		
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m1set = m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR BevelSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_BEVEL));
	return hCur;
}

void BevelCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void BevelCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int ChamferMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			em->BeginChamfer (ip->GetTime());		
			om = m;
		} else {
			ip->RedrawViews (ip->GetTime(),REDRAW_END);
			em->EndChamfer (ip->GetTime(),TRUE);
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om,float(-200));
		p1 = vpt->MapScreenToView(m,float(-200));
		em->Chamfer (ip->GetTime(), Length(p1-p0));

		spin = GetISpinner(GetDlgItem(em->hGeom,IDC_EM_OUTLINESPINNER));
		if (spin) {
			spin->SetValue(Length(p1-p0),FALSE);
			ReleaseISpinner(spin);
		}

		ip->RedrawViews (ip->GetTime(), REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		em->EndChamfer (ip->GetTime(),FALSE);			
		ip->RedrawViews (ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ChamferSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hECur = NULL;
	static HCURSOR hVCur = NULL;
	if (em->selLevel == SL_VERTEX) {
		if ( !hVCur ) hVCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_VCHAMFERCUR));
		return hVCur;
	}
	if ( !hECur ) hECur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ECHAMFERCUR));
	return hECur;
}

void ChamferCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ChamferCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// --- Slice: not really a command mode, just looks like it.--------- //

// Each time we enter the slice mode, we reset the slice plane.
// It should encompass all nodes.
void EditMeshMod::EnterSliceMode () {
	sliceMode = TRUE;
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_EM_SLICE));
		but->Enable ();
		ReleaseICustButton (but);
		EnableWindow (GetDlgItem (hGeom, IDC_EM_REFINE), FALSE);
		but = GetICustButton (GetDlgItem (hGeom, IDC_EM_SLICEPLANE));
		but->SetCheck (TRUE);
		ReleaseICustButton (but);
	}

	ModContextList mcList;
	INodeTab inodes;
	ip->GetModContexts (mcList, inodes);
	Box3 *bbox = mcList[0]->box;
	sliceCenter = (bbox->pmin + bbox->pmax) * .5f;
	sliceRot.Identity();
	Point3 boxDiff = bbox->pmax - bbox->pmin;
	sliceSize = (boxDiff.x > boxDiff.y) ? boxDiff.x : boxDiff.y;
	sliceSize *= .52f;
	if (sliceSize<1.0f) {
		EditMeshData *pData = (EditMeshData *) mcList[0]->localData;
		Box3 nodeBox = pData->GetMesh(ip->GetTime())->getBoundingBox();
		boxDiff = nodeBox.pmax - nodeBox.pmin;
		sliceSize = (boxDiff.x > boxDiff.y) ? boxDiff.x : boxDiff.y;
		sliceSize *= .52f;

		if (sliceSize<1.0f) sliceSize = 1.0f;
	}

	if (ip->GetCommandMode()->ID() >= CID_USER)
		ip->PopCommandMode ();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void EditMeshMod::ExitSliceMode () {
	sliceMode = FALSE;
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_EM_SLICE));
		but->Disable ();
		ReleaseICustButton (but);
		if (selLevel >= SL_EDGE) EnableWindow (GetDlgItem (hGeom, IDC_EM_REFINE), TRUE);
		but = GetICustButton (GetDlgItem (hGeom, IDC_EM_SLICEPLANE));
		but->SetCheck (FALSE);
		ReleaseICustButton (but);
	}
	ip->RedrawViews(ip->GetTime());
}

// -- Cut Edge proc/mode -------------------------------------

class CutAbandon : public RestoreObj {
public:
	EditMeshMod *mod;

	CutAbandon (EditMeshMod *m) { mod = m; }
	void Restore(int isUndo) { if (mod->cutEdgeMode) mod->cutEdgeMode->AbandonCut(); }
	void Redo() { if (mod->cutEdgeMode) mod->cutEdgeMode->AbandonCut (); }
	int Size() { return sizeof(void *); }
	TSTR Description() { return TSTR(_T("Edit Mesh Cut Abandon")); }
};

HitRecord *CutEdgeProc::HitTestEdges (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *ret;

	if (!e1set) {
		ret = hitLog.ClosestHit();
		mc = ret->modContext;
		return ret;
	}

	for (ret=hitLog.First(); ret != NULL; ret=ret->Next())
		if (ret->modContext == mc) break;
	if (ret==NULL) return NULL;
	DWORD best = ret->distance;
	HitRecord *iter;
	for (iter=ret; iter!=NULL; iter=iter->Next()) {
		if (iter->modContext != mc) continue;
		if (iter->distance < best) {
			ret = iter;
			best = ret->distance;
		}
	}
	if (e1 == (int)ret->hitInfo) return NULL;
	return ret;
}

void CutEdgeProc::DrawCutter (HWND hWnd,IPoint2 &m) {
	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);	
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));
	MoveToEx(hdc,m1.x,m1.y,NULL);
	LineTo(hdc,m.x,m.y);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

int CutEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt;
	HitRecord *hr;
	AdjEdgeList *ae;
	MeshDelta tmd;
	IPoint2 betterM;
	Point3 A, B, snapPoint, Zdir, Xdir;
	Matrix3 obj2world, world2obj;
	float prop;
	Ray r;

	EditMeshData *meshData;
	Mesh *mesh;
	TimeValue t = ip->GetTime();

	switch (msg) {
	case MOUSE_ABORT:
		// Erase last cutter line:
		if (e1set) DrawCutter (hwnd, oldm2);
		e1set = FALSE;
		return FALSE;

	case MOUSE_PROPCLICK:
		// Erase last cutter line:
		if (e1set) DrawCutter (hwnd, oldm2);
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		if (!e1set) break;
		// Erase last cutter line:
		DrawCutter (hwnd, oldm2);

		theHold.Begin ();
		meshData = (EditMeshData *) mc->localData;
		mesh = meshData->GetMesh(t);
		ae = meshData->TempData(t)->AdjEList();
		tmd.DivideEdge (*mesh, e1, prop1, ae, FALSE, em->cutRefine,
			FALSE, em->sliceSplit);
		meshData->ApplyMeshDelta (tmd, em, t);
		theHold.Accept (GetString (IDS_EM_CUT));
		ip->RedrawViews(ip->GetTime());
		e1set = FALSE;
		return FALSE;

	case MOUSE_POINT:
		if (point==1) break;
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport (hwnd);
		hr = HitTestEdges(m,vpt);
		if (!hr) {
			ip->ReleaseViewport(vpt);
			break;
		}

		DWORD ee;
		ee = hr->hitInfo;

		// Find where along this edge we hit
		// Strategy:
		// Get Mouse click, plus viewport z-direction at mouse click, in object space.
		// Then find the direction of the edge in a plane orthogonal to z, and see how far
		// along that edge we are.
		snapPoint = vpt->SnapPoint(m,betterM,NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		obj2world = hr->nodeRef->GetObjectTM (t);

		t = ip->GetTime();
		meshData = (EditMeshData *) mc->localData;
		mesh = meshData->GetMesh(t);

		vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
		Zdir = Normalize (r.dir);
		A = obj2world * mesh->verts[mesh->faces[ee/3].v[ee%3]];
		B = obj2world * mesh->verts[mesh->faces[ee/3].v[(ee+1)%3]];
		Xdir = B-A;
		Xdir -= DotProd(Xdir, Zdir)*Zdir;
		prop = DotProd (Xdir, snapPoint-A) / LengthSquared (Xdir);
		if (prop<.0001f) prop=0;
		if (prop>.9999f) prop=1;

		ip->ReleaseViewport(vpt);

		if (!e1set) {
			e1 = hr->hitInfo;
			prop1 = prop;
			e1set = TRUE;
			m1 = betterM;
			DrawCutter (hwnd, m);
			oldm2=m;
			break;
		}

		// Erase last cutter line:
		DrawCutter (hwnd, oldm2);

		// Do the cut:
		theHold.Begin ();
		world2obj = Inverse (obj2world);
		Zdir = Normalize (VectorTransform (world2obj, Zdir));
		e1 = tmd.Cut (*mesh, e1, prop1, ee, prop, -Zdir, em->cutRefine, em->sliceSplit);
		meshData->ApplyMeshDelta (tmd, em, t);
		theHold.Put (new CutAbandon(em));
		theHold.Accept (GetString (IDS_EM_CUT));
		ip->RedrawViews(ip->GetTime());

		if (e1==UNDEFINED) {
			e1set = FALSE;
			return FALSE;
		} else {
			prop1=0.0f;
			m1 = betterM;
			DrawCutter (hwnd, m);
			oldm2 = m;
		}
		break;

	case MOUSE_MOVE:
		vpt = ip->GetViewport (hwnd);
		vpt->SnapPoint(m,m,NULL);
		if (e1set) {
			DrawCutter (hwnd,oldm2);
			oldm2 = m;
		}
		if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		ip->ReleaseViewport(vpt);
		ip->RedrawViews(ip->GetTime());
		if (e1set) DrawCutter (hwnd, oldm2);
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		vpt->SnapPreview(m,m,NULL);//, SNAP_SEL_OBJS_ONLY);
		if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;	
}

void CutEdgeCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_CUT));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.e1set = FALSE;
	em->inCutEdge = TRUE;
}

void CutEdgeCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_CUT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	em->inCutEdge = FALSE;
}

void CutEdgeCMode::AbandonCut () {
	proc.e1set = FALSE;
}

/*-------------------------------------------------------------------*/

void WeldVertCMode::EnterMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_WELDTOVERT));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void WeldVertCMode::ExitMode() {
	if (!em->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hGeom,IDC_EM_WELDTOVERT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

BOOL WeldVertMouseProc::HitTestVerts (IPoint2 &m, ViewExp *vpt, int &v) {
	vpt->ClearSubObjHitList();
	em->pickBoxSize = em->weldBoxSize;
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, HIT_UNSELONLY, &m, vpt);
	em->pickBoxSize = DEF_PICKBOX_SIZE;
	if (!vpt->NumSubObjHits()) return FALSE;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	MaxAssert(hr);
	v = hr->hitInfo;
	return TRUE;
}

void WeldVertMouseProc::PostTransformHolding () {
	TransformModBox::PostTransformHolding();
	if (targetVert < 0) return;
	Mesh *mesh = emd->GetMesh(ip->GetTime());
	Point3 pt = mesh->verts[targetVert];

	// Select the point that was hit
	BitArray vsel = emd->GetVertSel ();
	vsel.Set (targetVert);
	emd->SetVertSel (vsel, em, ip->GetTime());

	// Do the weld
	em->WeldVerts (pt);
}

int WeldVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt = ip->GetViewport(hwnd);
	int res = TRUE;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (point==1) {
			if (!HitTestVerts(m,vpt,targetVert)) {
				targetVert = -1;
				break;
			}
			// Get the hit record
			HitLog& hitLog = vpt->GetSubObjHitList();
			HitRecord *hr = hitLog.ClosestHit();
			MaxAssert(hr);
			// Find the point on the mesh that was hit
			emd = (EditMeshData*)hr->modContext->localData;
		}
		break;

	case MOUSE_MOVE:
		int vert;
		if (HitTestVerts(m,vpt,vert)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(ip->GetSysCursor(SYSCUR_MOVE));
		break;
	}
	
	if (vpt) ip->ReleaseViewport(vpt);
	return MoveModBox::proc(hwnd,msg,point,flags,m);	
}

HCURSOR WeldVertSelectionProcessor::GetTransformCursor() { 	
	return ip->GetSysCursor(SYSCUR_MOVE); 
}

//----------------------------------------------------------

void FlipNormProc::FacePick (EditMeshData *meshData, DWORD face, float *bary) {
	Mesh *mesh = meshData->GetMesh(ip->GetTime());
	theHold.Begin ();
	MeshDelta tmd(*mesh);
	if (em->selLevel <= SL_FACE) {
		tmd.FlipNormal (*mesh, face);
	} else {
		BitArray flip;
		flip.SetSize (tmd.fnum);
		flip.ClearAll ();
		AdjFaceList *af = meshData->TempData(ip->GetTime())->AdjFList();
		if (em->selLevel == SL_POLY) {
			mesh->PolyFromFace (face, flip, em->GetPolyFaceThresh(), em->ignoreVisEdge, af);
		} else {
			mesh->ElementFromFace (face, flip, af);
		}
		for (DWORD i=0; i<tmd.fnum; i++) if (flip[i]) tmd.FlipNormal (*mesh, i);
	}
	meshData->ApplyMeshDelta (tmd, em, ip->GetTime());
	theHold.Accept(GetString(IDS_EM_FLIP_NORMAL));
	ip->RedrawViews(ip->GetTime());	
}

void FlipNormCMode::EnterMode() {
	if (!em->hSurf) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hSurf, IDC_EM_NORMAL_FLIPMODE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);	
}

void FlipNormCMode::ExitMode() {
	if (!em->hSurf) return;
	ICustButton *but = GetICustButton(GetDlgItem(em->hSurf, IDC_EM_NORMAL_FLIPMODE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);		
}
