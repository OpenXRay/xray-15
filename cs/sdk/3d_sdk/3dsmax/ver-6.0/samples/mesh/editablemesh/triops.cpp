 /**********************************************************************  
 *<
	FILE: triops.cpp

	DESCRIPTION: Editable Triangle Mesh Object

	CREATED BY: Rolf Berteig

	HISTORY: created 4 March 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "pack1.h"
#include "triobjed.h"
#include "macrorec.h"
#include "decomp.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

void EditTriObject::CloneSelSubComponents(TimeValue t) {
	if (selLevel == SL_OBJECT) return;
	if (!ip) return;

	theHold.Begin();

	MeshDelta tmd(GetMesh());
	switch (selLevel) {
	case SL_VERTEX: tmd.CloneVerts(GetMesh(), GetMesh().vertSel); break;
	case SL_EDGE: tmd.ExtrudeEdges (GetMesh(), GetMesh().edgeSel); break;
	default: tmd.CloneFaces (GetMesh(), GetMesh().faceSel); break;
	}
	ApplyMeshDelta (tmd, this, t);

	theHold.Accept(GetString(IDS_RB_CLONE));	
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::AcceptCloneSelSubComponents(TimeValue t) {
	if ((selLevel==SL_OBJECT) || (selLevel==SL_EDGE)) return;
	TSTR name;
	if (!GetCloneObjectName(ip, name)) return;
	Detach (name, (selLevel != SL_VERTEX), TRUE, FALSE);
}

void EditTriObject::Transform (TimeValue t, Matrix3& partm, Matrix3 tmAxis, 
		BOOL localOrigin, Matrix3 xfrm, int type) {
	if (!ip) return;

	if (sliceMode) {
		// Special case -- just transform slicing plane.
		theHold.Put (new TransformPlaneRestore (this));
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

	// Otherwise, moving subobject selections.

	// Get node transform
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	Matrix3 nodeTm = nodes[0]->GetObjectTM(t);

	// Get axis type:
	int numAxis = ip->GetNumAxis();

	// Special case for vertices: Only individual axis when moving in local space
	if ((selLevel==SL_VERTEX) && (numAxis==NUMAXIS_INDIVIDUAL)) {
		if (ip->GetRefCoordSys()!=COORDS_LOCAL || 
			ip->GetCommandMode()->ID()!=CID_SUBOBJMOVE) {
			numAxis = NUMAXIS_ALL;
		}
	}

	// Selected vertices - either directly or indirectly through selected faces or edges.
	BitArray sel = GetMesh().VertexTempSel();
	if (!sel.NumberSet()) {
		nodes.DisposeTemporary ();
		return;
	}
	MeshDelta tmd (GetMesh());

	int i, nv = GetMesh().numVerts;

	// Compute the transforms
	if (numAxis==NUMAXIS_INDIVIDUAL && selLevel != SL_VERTEX) {
		// Do each cluster one at a time
		DWORD count;
		Tab<DWORD> *vclust = NULL;
		if (selLevel == SL_EDGE) {
			count = TempData()->EdgeClusters()->count;
			vclust = TempData()->VertexClusters(MESH_EDGE);
		} else {
			count = TempData()->FaceClusters()->count;
			vclust = TempData()->VertexClusters(MESH_FACE);
		}
		// If we have soft selections from multiple clusters,
		// we need to add up the vectors and divide by the total soft selection,
		// to get the right direction for movement,
		// but we also need to add up the squares of the soft selections and divide by the total soft selection,
		// essentially getting a weighted sum of the selection weights themselves,
		// to get the right scale of movement.

		// (Note that this works out to ordinary soft selections in the case of a single cluster.)

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
			tmAxis = ip->GetTransformAxis (nodes[0], j);
			tm  = partm * Inverse(tmAxis);
			itm = Inverse(tm);
			tm *= xfrm;
			if (affectRegion) clustDist = TempData()->ClusterDist(meshSelLevel[selLevel], j, useEdgeDist, edgeIts)->Addr(0);
			for (i=0; i<nv; i++) {
				if (sel[i]) {
					if ((*vclust)[i]!=j) continue;
					Point3 & old = GetMesh().verts[i];
					tmd.Move (i, (tm*old)*itm - old);
				} else {
					if (!affectRegion) continue;
					if (clustDist[i] < 0) continue;
					if (clustDist[i] > falloff) continue;
					float af = AffectRegionFunction (clustDist[i], falloff, pinch, bubble);
					sss[i] += fabsf(af);
					ssss[i] += af*af;
					Point3 & old = GetMesh().verts[i];
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
		Matrix3 tm  = partm * Inverse(tmAxis);
		Matrix3 itm = Inverse(tm);
		tm *= xfrm;
		Point3 *vn = (numAxis == NUMAXIS_INDIVIDUAL) ? TempData()->VertexNormals()->Addr(0) : 0;
		float *vsw = affectRegion ? TempData()->VSWeight(useEdgeDist, edgeIts, arIgBack,
				falloff, pinch, bubble)->Addr(0) : NULL;
		for (i=0; i<nv; i++) {
			if (!sel[i]) {
				if (!vsw || !vsw[i]) continue;
			}
			Point3 & old = GetMesh().verts[i];
			Point3 delta;
			if (numAxis == NUMAXIS_INDIVIDUAL) {
				MatrixFromNormal (vn[i], tm);
				tm  = partm * Inverse(tm*nodeTm);
				itm = Inverse(tm);
				delta = itm*(xfrm*(tm*old)) - old;
			} else {
				delta = itm*(tm*old)-old;
			}
			if (sel[i]) tmd.Move (i, delta);
			else tmd.Move (i, delta * vsw[i]);
		}
	}

	nodes.DisposeTemporary ();
	DragMove (tmd, this);
}

void EditTriObject::Move (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) {
	Transform(t, partm, tmAxis, localOrigin, TransMatrix(val), 0);
	macroRecorder->FunctionCall(_T("move"), 2, 0, mr_meshsel, selLevel, this, mr_point3, &val);  // JBW : macrorecorder
}

void EditTriObject::Rotate (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Quat& val, BOOL localOrigin) {
	Matrix3 mat;
	val.MakeMatrix(mat);
	Transform(t, partm, tmAxis, localOrigin, mat, 1);
	macroRecorder->FunctionCall(_T("rotate"), 2, 0, mr_meshsel, selLevel, this, mr_quat, &val);  // JBW : macrorecorder
	return;
}

void EditTriObject::Scale (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) {
	Transform (t, partm, tmAxis, localOrigin, ScaleMatrix(val), 2);	
	macroRecorder->FunctionCall(_T("scale"), 2, 0, mr_meshsel, selLevel, this, mr_point3, &val);  // JBW : macrorecorder
	return;
}

void EditTriObject::TransformStart(TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(TRUE);
	if (sliceMode) return;
	DragMoveInit(false);
}

void EditTriObject::TransformHoldingFinish (TimeValue t) {
	if (!ip || sliceMode) return;
	DragMoveAccept (t);
}

void EditTriObject::TransformFinish(TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(FALSE);
}

void EditTriObject::TransformCancel (TimeValue t) {
	DragMoveClear ();
	if (!ip) return;
	ip->LockAxisTripods(FALSE);
	if (sliceMode) return;
}

// Selection dialog ops

void EditTriObject::HideSelectedVerts() {
	theHold.Begin();
	if (theHold.Holding())
		theHold.Put(new MeshVertHideRestore(this));
	GetMesh().vertHide |= GetMesh().vertSel;
	GetMesh().InvalidateGeomCache ();
	BitArray emptyVertSel;
	emptyVertSel.SetSize (GetMesh().numVerts);
	SetVertSel (emptyVertSel, this, ip ? ip->GetTime () : 0);
	theHold.Accept(GetString(IDS_RB_HIDEVERT));

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	LocalDataChanged ();
}

void EditTriObject::UnhideAllVerts() {
	theHold.Begin();
	if (theHold.Holding())
		theHold.Put(new MeshVertHideRestore(this));
	GetMesh().vertHide.ClearAll();
	GetMesh().InvalidateGeomCache ();
	theHold.Accept(GetString(IDS_RB_UNHIDEALLFACES));
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::HideSelectedFaces() {
	theHold.Begin();
	theHold.Put(new MeshFaceHideRestore(this));
	theHold.Put(new MeshVertHideRestore(this));
	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		if (GetMesh().faceSel[i]) GetMesh().faces[i].Hide();
	}
	HiddenFacesToVerts (GetMesh(), GetMesh().vertHide);
	BitArray emptyFaceSel;
	emptyFaceSel.SetSize (GetMesh().numFaces);
	SetFaceSel (emptyFaceSel, this, ip ? ip->GetTime() : 0);
	GetMesh().InvalidateGeomCache ();
	GetMesh().InvalidateTopologyCache();
	theHold.Accept(GetString(IDS_RB_HIDEFACE));

	NotifyDependents (FOREVER, PART_DISPLAY | PART_TOPO, REFMSG_CHANGE);
	LocalDataChanged ();
}

void EditTriObject::UnhideAllFaces() {
	theHold.Begin();
	theHold.Put(new MeshFaceHideRestore(this));
	theHold.Put(new MeshVertHideRestore(this));
	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		if (!GetMesh().faces[i].Hidden ()) continue;
		GetMesh().faces[i].Show();
		for (int j=0; j<3; j++) GetMesh().vertHide.Clear (GetMesh().faces[i].v[j]);
	}
	GetMesh().InvalidateGeomCache ();
	GetMesh().InvalidateTopologyCache();
	theHold.Accept(GetString(IDS_RB_UNHIDEALLFACES));
	NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// Edit Geometry ops

DWORD EditTriObject::CreateVertex(Point3 pt) {
	if (!ip) return UNDEFINED;
	// Put the point in object space:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	pt = pt * Inverse(nodes[0]->GetObjectTM(ip->GetTime()));
	nodes.DisposeTemporary();

	MeshDelta tmd(GetMesh());
	DWORD ret = tmd.VCreate (&pt);
	tmd.vsel.Set (ret);

	theHold.Begin();
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_ADDVERTS));

	ip->RedrawViews(ip->GetTime());
	return ret;
}

bool EditTriObject::CreateFace(int *v, int deg) {
	int tnum = deg-2;
	MeshDelta tmd(GetMesh());
	if (tmd.CreatePolygon (GetMesh (), deg, v) == UNDEFINED) return FALSE;
	theHold.Begin();
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_BUILDFACE));
	ip->RedrawViews(ip->GetTime());
	return TRUE;
}

void EditTriObject::DeleteSelected() {
	if (selLevel == SL_OBJECT) return;

	int i, j;
	BitArray fset, iso;
	DWORD stringID, of;
	AdjFaceList *af;

	theHold.Begin();
	MeshDelta tmd(GetMesh());

	switch (selLevel) {
	case SL_VERTEX:
		tmd.DeleteVertSet(GetMesh(), GetMesh().vertSel);
		stringID = IDS_RB_DELETEVERT;
		break;

	case SL_EDGE:
		af = TempData()->AdjFList();
		fset.SetSize (GetMesh().getNumFaces());

		for (i=0; i<GetMesh().getNumFaces(); i++) {
			for (j=0; j<3; j++) if (GetMesh().edgeSel[i*3+j]) break;
			if (j==3) continue;
			fset.Set(i);
			// Mark other sides of edges:
			for (; j<3; j++) {
				if (!GetMesh().edgeSel[i*3+j]) continue;
				if ((of=(*af)[i].f[j]) != UNDEFINED) fset.Set(of);
			}
		}

		GetMesh().FindVertsUsedOnlyByFaces (fset, iso);
		tmd.DeleteFaceSet (GetMesh(), fset);

		if (ip && iso.NumberSet()) {
			TSTR str1 = GetString(IDS_RB_DELETEISOLATED);
			TSTR str2 = GetString(IDS_RB_DELETEEDGE);
			if (IDYES==MessageBox (ip->GetMAXHWnd(), str1,str2, MB_ICONQUESTION|MB_YESNO)) {
				tmd.VDelete (iso);
			}
		}

		stringID = IDS_RB_DELETEEDGE;
		break;

	default:
		GetMesh().FindVertsUsedOnlyByFaces (GetMesh().faceSel, iso);
		tmd.DeleteFaceSet (GetMesh(), GetMesh().faceSel);

		if (ip && iso.NumberSet()) {
			TSTR str1 = GetString(IDS_RB_DELETEISOLATED);
			TSTR str2 = GetString(IDS_RB_DELETEFACE);
			if (IDYES==MessageBox (ip->GetMAXHWnd(), str1,str2, MB_ICONQUESTION|MB_YESNO)) {
				tmd.VDelete (iso);
			}
		}
		stringID = IDS_RB_DELETEFACE;
		break;
	}

	if (!ip) return;	// Can happen if we leave EMesh modify panel during MessageBox.

	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept (GetString (stringID));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::Attach (INode *node, bool & canUndo) {
	// First get the object
	BOOL del = FALSE;
	TriObject *obj = NULL;
	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(triObjectClassID)) obj = (TriObject *) os.obj;
	else {
		if (!os.obj->CanConvertToType(triObjectClassID)) return;
		obj = (TriObject*)os.obj->ConvertToType(ip->GetTime(),triObjectClassID);
		if (obj!=os.obj) del = TRUE;
	}

	// Get our node:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	theHold.Begin();

	// Combine the materials of the two nodes.
	int mat2Offset=0;
	Mtl *m1 = nodes[0]->GetMtl();
	Mtl *m2 = node->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 && (m1 != m2)) {
		if (attachMat==ATTACHMAT_IDTOMAT) {
			FitMeshIDsToMaterial (GetMesh(), m1);
			FitMeshIDsToMaterial (obj->GetMesh(), m2);
			if (condenseMat) condenseMe = TRUE;
		}

		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		theHold.Suspend ();
		if (attachMat==ATTACHMAT_MATTOID) {
			m1 = FitMaterialToMeshIDs (GetMesh(), m1);
			m2 = FitMaterialToMeshIDs (obj->GetMesh(), m2);
		}
		Mtl *multi = CombineMaterials(m1, m2, mat2Offset);
		if (attachMat == ATTACHMAT_NEITHER) mat2Offset = 0;
		theHold.Resume ();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = selLevel;
		if (oldSL>SL_EDGE) selLevel = SL_OBJECT;
		nodes[0]->SetMtl(multi);
		if (oldSL>SL_EDGE) selLevel = oldSL;
		m1 = multi;
		canUndo = TRUE;	// DS 10/13/00 -- Undo should work now.
	}
	if (!m1 && m2) {
		// This material operation seems safe.
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = selLevel;
		if (oldSL>SL_EDGE) selLevel = SL_OBJECT;
		nodes[0]->SetMtl (m2);
		if (oldSL>SL_EDGE) selLevel = oldSL;
		m1 = m2;
	}

	// Construct a transformation that takes a vertex out of the space of
	// the source node and puts it into the space of the destination node.
	Matrix3 tm = node->GetObjectTM(ip->GetTime()) *
		Inverse(nodes[0]->GetObjectTM(ip->GetTime()));

	MeshDelta nmd;
	nmd.AttachMesh (GetMesh(), obj->GetMesh(), tm, mat2Offset);
	ApplyMeshDelta (nmd, this, ip->GetTime());

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

	/*// 020326  --prs.
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

	ip->DeleteNode (node);

	// 020326  --prs.
	//if (par != NULL)
		//ip->DeleteNode(par);

	theHold.Accept (GetString (IDS_RB_ATTACHOBJECT));

	if (m1 && condenseMe) {
		// Following clears undo stack.
		m1 = CondenseMatAssignments (GetMesh(), m1);
	}

	nodes.DisposeTemporary ();
	if (del) delete obj;
}

void EditTriObject::MultiAttach (INodeTab &nodeTab) {
	bool canUndo = TRUE;
	if (nodeTab.Count() > 1) theHold.SuperBegin ();
	for (int i=0; i<nodeTab.Count(); i++) Attach (nodeTab[i], canUndo);
	if (nodeTab.Count() > 1) theHold.SuperAccept (GetString (IDS_ATTACH_LIST));
	if (!canUndo) GetSystemSetting (SYSSET_CLEAR_UNDO);
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::Detach (TSTR &name,BOOL doFaces,BOOL del,BOOL elem) {
	if (doFaces) {	// defect 261134
		if (mesh.faceSel.NumberSet() == 0) return;
	} else {
		if (mesh.vertSel.NumberSet() == 0) return;
	}
	theHold.Begin ();
	if (!elem) {	// Animation confuses things.
		SuspendAnimate();
		AnimateOff();
	}

	TriObject *newObj;
	if (!elem) newObj = CreateNewTriObject();

	MeshDelta tmd;
	tmd.Detach (GetMesh(), elem ? NULL : &(newObj->GetMesh()),
		doFaces ? GetMesh().faceSel : GetMesh().vertSel, doFaces, del, elem);
	ApplyMeshDelta (tmd, this, ip->GetTime());

	if (!elem) {
		// We need our node (for our transform).
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);

		// Add the object to the scene. Give it the given name
		// and set its transform to ours.
		// Also, give it our material.
		INode *node = ip->CreateObjectNode(newObj);
		Matrix3 ntm = nodes[0]->GetNodeTM(ip->GetTime());
		if (ip->GetINodeByName(name) != node) {	// Just in case name = "Object01" or some such default.
			TSTR uname = name;
			if (ip->GetINodeByName (uname)) ip->MakeNameUnique(uname);
			node->SetName(uname);
		}
		node->CopyProperties (nodes[0]);
		node->SetNodeTM(ip->GetTime(),ntm);
		node->FlagForeground(ip->GetTime(),FALSE);
		node->SetMtl(nodes[0]->GetMtl());
		node->SetObjOffsetPos(   nodes[0]->GetObjOffsetPos());
		node->SetObjOffsetRot(	 nodes[0]->GetObjOffsetRot());
		node->SetObjOffsetScale( nodes[0]->GetObjOffsetScale());

		nodes.DisposeTemporary();
	}

	if (!elem) ResumeAnimate();
	theHold.Accept(GetString(doFaces?IDS_RB_DETACHFACES:IDS_RB_DETACHVERT));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::BreakVerts() {
	theHold.Begin();
	MeshDelta tmd;
	tmd.BreakVerts (GetMesh(), GetMesh().vertSel);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_BREAKVERTS));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::DoExtrusion() {
	theHold.Begin();
	MeshDelta tmd;
	Tab<Point3> edir;
	if (selLevel == SL_EDGE) tmd.ExtrudeEdges (GetMesh(), GetMesh().edgeSel, &edir);
	else tmd.ExtrudeFaces (GetMesh(), GetMesh().faceSel, TempData()->AdjEList ());
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept (GetString (IDS_RB_EXTRUDE));

	if (selLevel == SL_EDGE) {
		// Edge normals messed up, so compute directions based on extrusion feedback.
		TempData()->EdgeExtDir (&edir, extType);
	} else {
		TempData()->FaceExtDir (extType);
	}
}

void EditTriObject::BeginExtrude(TimeValue t) {	
	if (inExtrude) return;
	inExtrude = TRUE;
	theHold.SuperBegin();
	DoExtrusion();
	BitArray sel = GetMesh().VertexTempSel();
	PlugControllersSel(t,sel);
	DragMoveInit(false);
}

void EditTriObject::EndExtrude (TimeValue t, BOOL accept) {		
	if (!ip) return;
	if (!inExtrude) return;
	inExtrude = FALSE;
	TempData()->freeBevelInfo();

	theHold.Begin ();
	DragMoveAccept (t);
	theHold.Accept(GetString(IDS_DS_MOVE));
	if (accept) theHold.SuperAccept(GetString(IDS_RB_EXTRUDE));
	else theHold.SuperCancel();

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hGeom,IDC_EXTRUDESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
}

void EditTriObject::Extrude( TimeValue t, float amount ) {
	if (!inExtrude) return;
	if (!GetMesh().numVerts) return;
	DragMoveRestore ();
	MeshDelta tmd(GetMesh());
	BitArray sel = GetMesh().VertexTempSel();
	Tab<Point3> *extDir = TempData()->CurrentExtDir();
	if ((extDir==NULL) || (amount==0)) return;
	tmd.Bevel (GetMesh(), sel, 0, NULL, amount, extDir);
	DragMove (tmd, this);
}

static int ExtDone=FALSE;

void EditTriObject::BeginBevel (TimeValue t, BOOL doExtrude) {	
	if (inBevel) return;
	inBevel = TRUE;
	theHold.SuperBegin();
	if (doExtrude) DoExtrusion ();
	ExtDone = doExtrude;
	BitArray sel = GetMesh().VertexTempSel();
	PlugControllersSel(t,sel);
	//theHold.Begin();
	DragMoveInit (false);
}

void EditTriObject::EndBevel (TimeValue t, BOOL accept) {		
	if (!ip) return;
	if (!inBevel) return;
	inBevel = FALSE;
	TempData()->freeBevelInfo();

	theHold.Begin ();
	DragMoveAccept (t);
	theHold.Accept(GetString(IDS_DS_MOVE));
	if (accept)
		theHold.SuperAccept(GetString(ExtDone ? IDS_BEVEL : IDS_OUTLINE));
	else theHold.SuperCancel();

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hGeom,IDC_OUTLINESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
	spin = GetISpinner(GetDlgItem(hGeom,IDC_EXTRUDESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
}

void EditTriObject::Bevel (TimeValue t, float amount, float height) {
	if (!inBevel) return;
	DragMoveRestore ();

	MeshDelta tmd(GetMesh());
	tmd.Bevel (GetMesh(), GetMesh().VertexTempSel (), amount, TempData()->OutlineDir(extType),
		height, TempData()->FaceExtDir (extType));
	DragMove (tmd, this);
}

void EditTriObject::DoChamfer () {
	theHold.Begin();
	MeshDelta tmd;
	MeshChamferData *mcd = TempData()->ChamferData();
	AdjEdgeList *ae = TempData()->AdjEList();
	if (selLevel == SL_EDGE) tmd.ChamferEdges (GetMesh(), GetMesh().edgeSel, *mcd, ae);
	else tmd.ChamferVertices (GetMesh(), GetMesh().vertSel, *mcd, ae);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept (GetString (IDS_CHAMFER));
}

void EditTriObject::BeginChamfer (TimeValue t) {
	if (inChamfer) return;
	inChamfer = TRUE;
	theHold.SuperBegin();
	DoChamfer ();
	if (GetMesh().numVerts) {
		BitArray sel;
		sel.SetSize (GetMesh().numVerts);
		sel.ClearAll ();
		float *vmp = TempData()->ChamferData()->vmax.Addr(0);
		for (int i=0; i<GetMesh().numVerts; i++) if (vmp[i]) sel.Set(i);
		PlugControllersSel(t,sel);
	}
	//theHold.Begin();
	DragMoveInit(true);
}

void EditTriObject::EndChamfer (TimeValue t, BOOL accept) {		
	if (!ip) return;
	if (!inChamfer) return;
	inChamfer = FALSE;
	TempData()->freeChamferData();

	theHold.Begin ();
	DragMoveAccept (t);
	theHold.Accept(GetString(IDS_DS_MOVE));
	if (accept) theHold.SuperAccept(GetString(IDS_CHAMFER));
	else theHold.SuperCancel();

	ISpinnerControl *spin = GetISpinner(GetDlgItem(hGeom,IDC_OUTLINESPINNER));
	if (spin) {
		spin->SetValue(0,FALSE);
		ReleaseISpinner(spin);
	}
}

void EditTriObject::Chamfer (TimeValue t, float amount) {
	if (!inChamfer) return;
	DragMoveRestore ();
	if (amount<=0) return;

	MeshDelta tmd(GetMesh());
	tmd.ChamferMove (GetMesh(), *TempData()->ChamferData(), amount, TempData()->AdjEList());
	DragMove (tmd, this);
}

void EditTriObject::AlignTo(int alignType) {
	// We'll need the viewport or construction plane transform:
	Matrix3 atm, otm, res;
	ViewExp *vpt = ip->GetActiveViewport();
	float zoff;
	if (alignType == ALIGN_CONST) vpt->GetConstructionTM(atm);
	else {
		vpt->GetAffineTM(atm);
		atm = Inverse(atm);
	}
	ip->ReleaseViewport (vpt);

	// We'll also need our own transform:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	otm = nodes[0]->GetObjectTM(ip->GetTime());
	nodes.DisposeTemporary();
	res = atm*Inverse (otm);	// screenspace-to-objectspace.

	BitArray sel = GetMesh().VertexTempSel();
	// For ZNorm, we want the object-space unit vector pointing into the screen:
	Point3 ZNorm (0,0,-1);
	ZNorm = Normalize (VectorTransform (res, ZNorm));

	if (alignType != ALIGN_CONST) {
		// Find the average z-depth for the current selection.
		zoff = 0.0f;
		for (int i=0; i<GetMesh().numVerts; i++) {
			if (!sel[i]) continue;
			zoff += DotProd (ZNorm, GetMesh().verts[i]);
		}
		zoff /= (float) sel.NumberSet(); 
	} else {
		// Find the z-depth of the construction plane, in object space:
		zoff = DotProd (ZNorm, res.GetTrans());
	}

	float *vsw = affectRegion ? TempData()->VSWeight(useEdgeDist, edgeIts, arIgBack,
			falloff, pinch, bubble)->Addr(0) : NULL;

	theHold.Begin();
	MeshDelta tmd(GetMesh());
	tmd.MoveVertsToPlane (GetMesh(), sel, ZNorm, zoff);
	if (vsw) {
		// MeshDelta method doesn't cover soft selections - do that here.
		for (int i=0; i<mesh.numVerts; i++) {
			if (sel[i]) continue;	// covered.
			if (!vsw[i]) continue;
			Point3 delta = (zoff - DotProd(mesh.verts[i], ZNorm)) * ZNorm * vsw[i];
			tmd.Move (i, delta);
		}
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(alignType==ALIGN_CONST?IDS_RB_ALIGNTOCONST:IDS_RB_ALIGNTOVIEW));
	ip->RedrawViews(ip->GetTime());
}

// NOTE: Following should go in class MeshDelta, method MakeSelVertsPlanar.  Going
// in here and in Edit Mesh instead so we don't break the SDK.
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
// in here and in Edit Mesh instead so we don't break the SDK.
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

void EditTriObject::MakePlanar () {
	float *vsw = affectRegion ? TempData()->VSWeight(useEdgeDist, edgeIts, arIgBack,
			falloff, pinch, bubble)->Addr(0) : NULL;
	theHold.Begin ();
	MeshDelta tmd (GetMesh());
	BitArray tempSel;
	int j, k;
	switch (selLevel) {
	case SL_VERTEX:
		MakeSelVertsPlanar (tmd, GetMesh(), GetMesh().vertSel, vsw);
		break;

	case SL_EDGE:
		tempSel.SetSize (GetMesh().numVerts);
		tempSel.ClearAll ();
		for (j=0; j<GetMesh().numFaces; j++) {
			for (k=0; k<3; k++) if (GetMesh().edgeSel[j*3+k]) break;
			if (k==3) continue;
			tempSel.Set (GetMesh().faces[j].v[k]);
			tempSel.Set (GetMesh().faces[j].v[(k+1)%3]);
			if ((k<2) && (GetMesh().edgeSel[j*3+k+1] || GetMesh().edgeSel[j*3+(k+2)%3]))
				tempSel.Set (GetMesh().faces[j].v[(k+2)%3]);
		}
		MakeSelVertsPlanar (tmd, GetMesh(), tempSel, vsw);
		break;

	default:
		MakeSelFacesPlanar (tmd, GetMesh(), GetMesh().faceSel, vsw);
		break;
	}

	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_MAKEPLANAR));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::Collapse () {
	if (selLevel == SL_OBJECT) return;
	theHold.Begin();

	MeshDelta tmd (GetMesh());
	switch (selLevel) {
	case SL_EDGE:
		tmd.CollapseEdges (GetMesh(), GetMesh().edgeSel, TempData()->AdjEList());
		break;
	default:
		tmd.WeldVertSet (GetMesh(), GetMesh().VertexTempSel());
		break;
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());

	theHold.Accept(GetString(IDS_RB_COLLAPSE));	
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::Tessellate (float tens,BOOL edge) {
	if (selLevel < SL_FACE) return;
	theHold.Begin();

	MeshDelta tmd;
	if (edge) tmd.EdgeTessellate (GetMesh(), GetMesh().faceSel, tens, TempData()->AdjEList(), TempData()->AdjFList());
	else tmd.DivideFaces (GetMesh(), GetMesh().faceSel);
	ApplyMeshDelta (tmd, this, ip->GetTime());

	theHold.Accept(GetString(IDS_RB_TESSELLATE));	
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::Explode (float thresh, BOOL objs, TSTR &name) {
	INodeTab nodes;
	ModContextList mcList;
	ip->GetModContexts(mcList,nodes);
	theHold.Begin();

	AdjFaceList *af = TempData()->AdjFList();
	MeshDelta tmd;
	if (!objs) tmd.ExplodeFaces (GetMesh(), thresh, (selLevel>=SL_FACE), af);
	else {
		theHold.Put (new MeshSelRestore (this));
		ExplodeToObjects (&GetMesh(), thresh, nodes[0], name, ip, &tmd, af, selLevel>=SL_FACE);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());

	theHold.Accept(GetString(IDS_RB_EXPLODE));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::Slice () {
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	theHold.Begin ();

	Point3 N = Point3(0.0f,0.0f,1.0f) * rotMatrix;
	float offset = DotProd (N, sliceCenter);

	MeshDelta tmd;
	if (selLevel >= SL_FACE) tmd.Slice (GetMesh(), N, offset, sliceSplit, FALSE, &(GetMesh().faceSel));
	else tmd.Slice (GetMesh(), N, offset, sliceSplit, FALSE);
	ApplyMeshDelta (tmd, this, ip->GetTime());

	theHold.Accept (GetString (IDS_SCA_SLICE));
	ip->RedrawViews(ip->GetTime());	
}

BOOL EditTriObject::WeldVerts (float thresh) {
	if (GetMesh().vertSel.NumberSet()<1) return FALSE;
	theHold.Begin();
	MeshDelta tmd(GetMesh());
	BOOL found = tmd.WeldByThreshold (GetMesh(), GetMesh().vertSel, thresh);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_WELDVERTS));	
	ip->RedrawViews(ip->GetTime());
	return found;
}

void EditTriObject::WeldVerts (Point3 pt) {
	theHold.Begin();
	MeshDelta tmd(GetMesh());
	tmd.WeldVertSet (GetMesh(), GetMesh().vertSel, &pt);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_WELDVERTS));	
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::RemoveIsoVerts() {
	theHold.Begin();
	MeshDelta tmd(GetMesh());
	tmd.DeleteIsoVerts (GetMesh());
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_DELETEISO));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::SelectOpenEdges() {
	theHold.Begin();
	BitArray nesel;
	GetMesh().FindOpenEdges (nesel);
	SetEdgeSel (nesel, this, ip->GetTime());
	LocalDataChanged ();
	theHold.Accept(GetString(IDS_RB_SELOPENEDGES));
	ip->RedrawViews(ip->GetTime());	
}

// Vertex surface operations:

float EditTriObject::GetWeight (TimeValue t, int *numSel) {
	if (numSel) *numSel = GetMesh().vertSel.NumberSet();
	if (selLevel != SL_VERTEX) return 1.0f;
	float *vw = GetMesh().getVertexWeights ();
	if (!vw) return 1.0f;
	float weight=1.0f;
	int found=0;
	for (int j=0; j< GetMesh().numVerts; j++) {
		if (!GetMesh().vertSel[j]) continue;
		if (!found) {
			weight = vw[j];
			found++;
		}
		else if (weight != vw[j]) weight = -1.0f;
	}
	return weight;
}

void EditTriObject::SetWeight (TimeValue t, float weight) {
	if (selLevel != SL_VERTEX) return;
	if (weight < MIN_WEIGHT) weight = MIN_WEIGHT;
	if (!GetMesh().vertSel.NumberSet()) return;

	MeshDelta tmd (GetMesh());
	tmd.SetVertWeights (GetMesh(), GetMesh().vertSel, weight);
	tmd.Apply (GetMesh());	// NOTE: not using ApplyMeshDelta; no undo support.
	NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews (t);
}

void EditTriObject::ResetWeights (TimeValue t) {
	if (selLevel != SL_VERTEX) return;
	MeshDelta tmd (GetMesh());
	tmd.ResetVertWeights (GetMesh());
	ApplyMeshDelta (tmd, this, t);
	NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews (t);
}

Color EditTriObject::GetVertColor (int mp) {
	static Color white(1,1,1), black(0,0,0);

	Color col=white;
	BOOL init=FALSE;

	TVFace *cf = GetMesh().mapFaces(mp);
	UVVert *cv = GetMesh().mapVerts(mp);
	if (!cf || !cv) return white;

	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		DWORD *tt = cf[i].t;
		DWORD *vv = GetMesh().faces[i].v;
		for (int j=0; j<3; j++) {
			if (!GetMesh().vertSel[vv[j]]) continue;
			if (!init) {
				col = cv[tt[j]];
				init = TRUE;
			} else {
				Color ac = cv[tt[j]];
				if (ac!=col) return black;
			}
		}
	}
	return col;
}

void EditTriObject::SetVertColor (Color clr, int mp) {
	if (GetMesh().vertSel.NumberSet()==0) return;
	MeshDelta tmd(GetMesh());
	tmd.SetVertColors (GetMesh(), GetMesh().vertSel, clr, mp);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::SelectVertByColor (VertColor clr, int deltaR, int deltaG, int deltaB, BOOL add, BOOL sub, int mp) {
	float dr = float(deltaR)/255.0f;
	float dg = float(deltaG)/255.0f;
	float db = float(deltaB)/255.0f;

	TVFace *cf = GetMesh().mapFaces(mp);
	UVVert *cv = GetMesh().mapVerts(mp);
	if (!cf || !cv) return;
	theHold.Begin();

	BitArray nvs;
	if (add || sub) {
		nvs = GetMesh().vertSel;
		nvs.SetSize (GetMesh().numVerts, TRUE);
	} else {
		nvs.SetSize (GetMesh().numVerts);
		nvs.ClearAll();
	}

	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		for (int j=0; j<3; j++) {
			Point3 col = cv[cf[i].t[j]];
			if ((float)fabs(col.x-clr.x) > dr) continue;
			if ((float)fabs(col.y-clr.y) > dg) continue;
			if ((float)fabs(col.z-clr.z) > db) continue;
			if (sub) nvs.Clear(GetMesh().faces[i].v[j]);
			else nvs.Set(GetMesh().faces[i].v[j]);
		}
	}
	SetVertSel (nvs, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_SELBYCOLOR));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

float EditTriObject::GetAlpha (int mp, int *num, bool *differs) {
	float alpha=1.0f;
	bool init=false;
	if (num) *num=0;
	if (differs) *differs=false;

	bool vlev = (selLevel == SL_VERTEX) ? true : false;
	bool flev = ((selLevel == SL_FACE) || (selLevel==SL_POLY) || (selLevel == SL_ELEMENT)) ? true : false;

	TVFace *cf = GetMesh().mapFaces(mp);
	UVVert *cv = GetMesh().mapVerts(mp);
	if (!cf || !cv) {
		if (num) *num=1;
		return alpha;
	}

	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		if (flev && !GetMesh().faceSel[i]) continue;
		DWORD *tt = cf[i].t;
		DWORD *vv = GetMesh().faces[i].v;
		for (int j=0; j<3; j++) {
			if (vlev && !GetMesh().vertSel[vv[j]]) continue;
			if (num) (*num)++;
			if (!init) {
				alpha = cv[tt[j]].x;
				init = true;
			} else {
				float ac = cv[tt[j]].x;
				if (ac!=alpha) {
					if (differs) *differs=true;
					return alpha;
				}
			}
		}
	}
	return alpha;
}

void EditTriObject::SetAlpha (float alpha, int mp) {
	MeshDelta tmd(GetMesh());
	if (selLevel == SL_VERTEX) {
		if (GetMesh().vertSel.NumberSet()==0) return;
		tmd.SetVertAlpha (GetMesh(), GetMesh().vertSel, alpha, mp);
	} else {
		if (GetMesh().faceSel.NumberSet() == 0) return;
		tmd.SetFaceAlpha (GetMesh(), GetMesh().faceSel, alpha, mp);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());
	ip->RedrawViews(ip->GetTime());
}

// Edge surface operations:

void EditTriObject::SetEdgeVis(BOOL vis) {
	theHold.Begin();
	AdjFaceList *af = TempData()->AdjFList ();
	MeshDelta tmd(GetMesh());
	int maxedge = GetMesh().numFaces*3;
	for (int j=0; j<maxedge; j++) {
		if (GetMesh().edgeSel[j]) tmd.SetSingleEdgeVis (GetMesh(), j, vis, af);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());
	if (vis) theHold.Accept(GetString(IDS_RB_EDGEVISIBLE));
	else theHold.Accept(GetString(IDS_RB_EDGEINVISIBLE));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::AutoEdge (float thresh, int type) {
	theHold.Begin();
	
	AdjEdgeList *ae = TempData()->AdjEList();
	AdjFaceList *af = TempData()->AdjFList();
	Tab<MEdge> &edges = ae->edges;

	MeshDelta tmd(GetMesh());
	for (int j=0; j<edges.Count(); j++) {
		if (!edges[j].Selected(GetMesh().faces,GetMesh().edgeSel)) continue;
		BOOL vis = (thresh==0.0f) || GetMesh().AngleBetweenFaces(edges[j].f[0], edges[j].f[1]) > thresh;
		if ((type == 1) && !vis) continue;
		if ((type == 2) && vis) continue;
		if (edges[j].f[0]!=UNDEFINED) {
			int e = GetMesh().faces[edges[j].f[0]].GetEdgeIndex(edges[j].v[0],edges[j].v[1]);
			tmd.SetSingleEdgeVis (GetMesh(), edges[j].f[0]*3+e, vis, af);
			continue;
		}
		assert (edges[j].f[1]!=UNDEFINED);
		int e = GetMesh().faces[edges[j].f[1]].GetEdgeIndex(edges[j].v[0],edges[j].v[1]);
		tmd.SetSingleEdgeVis (GetMesh(), edges[j].f[1]*3+e, vis, af);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());

	theHold.Accept(GetString(IDS_RB_AUTOEDGE));
	ip->RedrawViews(ip->GetTime());
}

// Face surface operations:

void EditTriObject::ShowNormals(DWORD vpFlags) {
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

	GetMesh().displayNormals (normalFlags, normScale);
	GetMesh().buildBoundingBox ();
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews (ip->GetTime(), vpFlags);
}

void EditTriObject::FlipNormals() {
	theHold.Begin();
	MeshDelta tmd(GetMesh());
	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		if (GetMesh().faceSel[i]) tmd.FlipNormal (GetMesh(), i);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_FLIPNORMALS));	
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::UnifyNormals() {
	theHold.Begin();
	AdjFaceList *af = TempData()->AdjFList ();
	MeshDelta tmd (GetMesh());
	tmd.UnifyNormals (GetMesh(), GetMesh().faceSel, af);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept (GetString(IDS_RB_UNIFYNORMALS));
	ip->RedrawViews (ip->GetTime());
}

DWORD EditTriObject::GetMatIndex() {
	DWORD mat = UNDEFINED;
	for (int j=0; j<GetMesh().getNumFaces(); j++) {
		if (!GetMesh().faceSel[j]) continue;
		if (mat==UNDEFINED) mat = GetMesh().getFaceMtlIndex(j);
		else if (GetMesh().getFaceMtlIndex(j) != mat) return UNDEFINED;
	}
	return mat;
}

// Note: good reasons for handling theHold.Begin/Accept at higher level.
void EditTriObject::SetMatIndex (DWORD index) {
	MeshDelta tmd(GetMesh());
	for (int j=0; j<GetMesh().getNumFaces(); j++) {
		if (GetMesh().faceSel[j]) tmd.SetMatID (j,(MtlID)index);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::SelectByMat (DWORD index,BOOL clear) {
	theHold.Begin();
	BitArray ns;
	if (clear) {
		ns.SetSize (GetMesh().numFaces);
		ns.ClearAll ();
	} else {
		ns = GetMesh().faceSel;
		if (ns.GetSize() != GetMesh().numFaces) ns.SetSize (GetMesh().numFaces, TRUE);
	}
	for (int j=0; j<GetMesh().getNumFaces(); j++) {
		if (GetMesh().getFaceMtlIndex(j)==index) ns.Set(j);
	}
	SetFaceSel (ns, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_SELECTBYMATID));
	LocalDataChanged ();
	ip->RedrawViews(ip->GetTime());
}

DWORD EditTriObject::GetUsedSmoothBits() {
	DWORD bits = 0;
	for (int j=0; j<GetMesh().getNumFaces(); j++)
		bits |= GetMesh().faces[j].smGroup;
	return bits;
}

// Those bits used by ANY selected faces are set in "some".
// Those bits used by ALL selected faces are set in the return value.
DWORD EditTriObject::GetSelSmoothBits(DWORD &some) {
	DWORD all = ~0;
	some = 0;
	if (!GetMesh().faceSel.NumberSet()) return 0;
	for (int j=0; j<GetMesh().getNumFaces(); j++) {
		if (!GetMesh().faceSel[j]) continue;
		some |= GetMesh().faces[j].smGroup;
		all &= GetMesh().faces[j].smGroup;
	}
	return all;
}

void EditTriObject::SetSelSmoothBits (DWORD bits, DWORD mask) {
	theHold.Begin();
	bits &= mask;
	MeshDelta tmd(GetMesh());
	for (int j=0; j<GetMesh().getNumFaces(); j++) {			
		if (!GetMesh().faceSel[j]) continue;
		tmd.FSmooth (j, mask, bits);
	}
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_SETSMOOTHGROUP));
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::SelectBySmoothGroup(DWORD bits,BOOL clear) {
	theHold.Begin();
	BitArray nfs;
	if (clear) {
		nfs.SetSize (GetMesh().getNumFaces());
		nfs.ClearAll ();
	} else {
		nfs = GetMesh().faceSel;
		nfs.SetSize (GetMesh().getNumFaces(), TRUE);
	}
	for (int j=0; j<GetMesh().getNumFaces(); j++) {
		if (GetMesh().faces[j].smGroup & bits) nfs.Set(j);
	}
	SetFaceSel (nfs, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_SELECTBYSMOOTH));
	LocalDataChanged ();
	ip->RedrawViews (ip->GetTime());
}

void EditTriObject::AutoSmooth(float thresh) {
	theHold.Begin();
	MeshDelta tmd(GetMesh());
	AdjEdgeList *ae = TempData()->AdjEList();
	AdjFaceList *af = TempData()->AdjFList();
	tmd.AutoSmooth (GetMesh(), GetMesh().faceSel, thresh, af, ae);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_AUTOSMOOTH));
	ip->RedrawViews(ip->GetTime());
}

Color EditTriObject::GetFaceColor (int mp) {
	static Color white(1,1,1), black(0,0,0);

	BOOL init=FALSE;
	Color col=white;

	TVFace *cf = GetMesh().mapFaces(mp);
	UVVert *cv = GetMesh().mapVerts(mp);
	if (!cf || !cv) return col;

	for (int i=0; i<GetMesh().getNumFaces(); i++) {
		if (!GetMesh().faceSel[i]) continue;
		DWORD *tt = cf[i].t;
		for (int j=0; j<3; j++) {
			if (!init) {
				col = cv[tt[j]];
				init = TRUE;
			} else {
				Color ac = cv[tt[j]];
				if (ac!=col) return black;
			}
		}
	}
	return col;
}

void EditTriObject::SetFaceColor(Color c, int mp) {
	if (GetMesh().faceSel.NumberSet() == 0) return;
	MeshDelta tmd(GetMesh());
	tmd.SetFaceColors (GetMesh(), GetMesh().faceSel, c, mp);
	ApplyMeshDelta (tmd, this, ip->GetTime());
	ip->RedrawViews(ip->GetTime());
}

//----Globals----------------------------------------------
// Move to class Interface or someplace someday?

void ExplodeToObjects (Mesh *mesh, float thresh, INode *node, TSTR &name,
					   Interface *ip, MeshDelta *tmd, AdjFaceList *af, BOOL selOnly) {  
	FaceClusterList fc(mesh, *af, thresh, selOnly);
	if (fc.count == 0) return;
	if (!selOnly || (mesh->faceSel.NumberSet() == mesh->numFaces)) {
		if (fc.count==1) return;	// nothing to do -- all in one cluster.
		if (!selOnly) mesh->faceSel.SetAll ();
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

		INode *newNode = ip->CreateObjectNode(newObj);
		Matrix3 ntm = node->GetNodeTM(ip->GetTime());
		TSTR uname = name;
		ip->MakeNameUnique(uname);
		newNode->SetName(uname);
		newNode->CopyProperties (node);
		newNode->SetNodeTM (ip->GetTime(), ntm);
		newNode->FlagForeground (ip->GetTime(), FALSE);
		newNode->SetMtl (node->GetMtl());
		newNode->SetObjOffsetPos (node->GetObjOffsetPos());
		newNode->SetObjOffsetRot (node->GetObjOffsetRot());
		newNode->SetObjOffsetScale (node->GetObjOffsetScale());
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

	DWORD ee = hr->hitInfo;
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	if (!snapPoint) snapPoint = &(r.p);
	Point3 Zdir = Normalize (r.dir);

	Mesh *mm = &(et->GetMesh());
	Point3 A = obj2world * mm->verts[mm->faces[ee/3].v[ee%3]];
	Point3 B = obj2world * mm->verts[mm->faces[ee/3].v[(ee+1)%3]];
	Point3 Xdir = B-A;
	Xdir -= DotProd(Xdir, Zdir)*Zdir;
	*prop = DotProd (Xdir, *snapPoint-A) / LengthSquared (Xdir);
	if (*prop<.0001f) *prop=0;
	if (*prop>.9999f) *prop=1;
	return hr;
}

int PickEdgeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
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
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestEdges (m, vpt, &prop, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
		if (hr) EdgePick(hr->hitInfo, prop);
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

HitRecord *PickFaceMouseProc::HitTestFaces (IPoint2 &m, ViewExp *vpt, float *bary, Point3 *snapPoint=NULL) {
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

	DWORD ff = hr->hitInfo;
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	if (!snapPoint) snapPoint = &(r.p);
	Point3 Zdir = Normalize (r.dir);

	Mesh *mm = &(et->GetMesh());
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
	ViewExp *vpt;
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
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestFaces (m, vpt, bary, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
		if (hr) FacePick (hr->hitInfo, bary);
		break;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:			
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);//|SNAP_SEL_OBJS_ONLY);
		if (HitTestFaces(m,vpt,NULL)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));		
		if (vpt) ip->ReleaseViewport(vpt);
		break;			
	}

	return TRUE;	
}

// -------------------------------------------------------

static HCURSOR hCurCreateVert = NULL;

void CreateVertCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void CreateVertCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

int CreateVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR)); 

	ViewExp *vpt = ip->GetViewport (hwnd);
	Matrix3 ctm;
	Point3 pt;
	IPoint2 m2;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt->GetConstructionTM(ctm);
		pt = vpt->SnapPoint (m, m2, &ctm);
		pt = pt * ctm;
		et->CreateVertex(pt);
		break;

	case MOUSE_FREEMOVE:
		SetCursor(hCurCreateVert);
		vpt->SnapPreview(m, m, NULL, SNAP_FORCE_3D_RESULT);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

//----------------------------------------------------------

void CreateFaceCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	et->inBuildFace = TRUE;
	if (!et->ip->GetShowEndResult()) {
		et->GetMesh().dispFlags |= (DISP_VERTTICKS|DISP_SELVERTS);
	}
	et->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	et->ip->RedrawViews(et->ip->GetTime());
}

void CreateFaceCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_CREATE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	et->inBuildFace = FALSE;
	et->GetMesh().dispFlags &= ~(DISP_VERTTICKS|DISP_SELVERTS);
	et->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	et->ip->RedrawViews(et->ip->GetTime());
}

CreateFaceMouseProc::CreateFaceMouseProc (EditTriObject* e, IObjParam *i) {
	et = e;
	ip = i;	
	pt = 0;
}

// We assume the transform, color, render style, etc, has been set up in advance.
void CreateFaceMouseProc::DrawEstablishedFace (GraphicsWindow *gw) {
	if (pt<2) return;
	Tab<Point3> rverts;
	rverts.SetCount (pt+1);
	for (int j=0; j<pt; j++) {
		rverts[j] = et->GetMesh().verts[vts[j]];
	}
	gw->polyline (pt, rverts.Addr(0), NULL, NULL, FALSE, NULL);
}

void CreateFaceMouseProc::DrawCreatingFace (HWND hWnd, const IPoint2 & m) {
	if (pt<1) return;

	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));

	MoveToEx (hdc, mfirst.x, mfirst.y, NULL);
	LineTo (hdc, m.x, m.y);
	if (pt>1) LineTo (hdc, mlast.x, mlast.y);

	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

BOOL CreateFaceMouseProc::HitTestVerts (IPoint2 m, ViewExp *vpt, int &v) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return FALSE;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	assert(hr);
	if (et->selLevel != SL_POLY) {
		for (int i=0; i<pt; i++) if (vts[i]==(int)hr->hitInfo) return FALSE;
	}
	v = hr->hitInfo;
	return TRUE;
}

int CreateFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {	
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR));

	ViewExp *vpt = ip->GetViewport(hwnd);
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

		if (HitTestVerts(m, vpt, nv)) {
			HitLog& hitLog = vpt->GetSubObjHitList();
			HitRecord *hr = hitLog.ClosestHit();
			MaxAssert (hr);
			for (int j=0; j<pt; j++) if (vts[j] == nv) break;
			if (j<pt) done=TRUE;
			else {
				vts.Append (1, &nv, 20);
				if (pt==0) mfirst = m;
				else mlast = m;
				pt++;
				et->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				ip->RedrawViews (ip->GetTime());
				oldm = m;
				DrawCreatingFace(hwnd, m);
			}
		} else if (flags & MOUSE_SHIFT) {
			Matrix3 ctm;
			vpt->GetConstructionTM(ctm);
			Point3 newpt = vpt->SnapPoint(m,m,&ctm, SNAP_FORCE_3D_RESULT|SNAP_SEL_SUBOBJ_ONLY);
			newpt = newpt * ctm;
			nv = et->GetMesh().numVerts;

			vts.Append (1, &nv, 20);
			if (pt==0) mfirst = m;
			else mlast = m;
			pt++;
			et->CreateVertex(newpt);
			oldm = m;
			DrawCreatingFace(hwnd, m);
		} else {
			return false;
		}

		if ((et->selLevel != SL_POLY) && (pt==3)) done = TRUE;

		if (done) {
			// We're done collecting verts - build a face
			lpt = pt;
			pt = 0;	// so the redraw gets that we're done.
			if ((lpt>2) && (!et->CreateFace(vts.Addr(0), lpt))) {
				InvalidateRect(vpt->getGW()->getHWnd(),NULL,FALSE);
				TSTR buf1 = GetString(IDS_RB_DUPFACEWARNING);
				TSTR buf2 = GetString(IDS_SCA_BASE_MESH);
				MessageBox(et->ip->GetMAXHWnd(),buf1,buf2,MB_OK|MB_ICONINFORMATION);						
				et->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
				ip->RedrawViews (ip->GetTime());
			}
			vts.SetCount(0);
			ip->ReleaseViewport (vpt);
			return FALSE;
		}
		break;

	case MOUSE_MOVE:
		if (pt) DrawCreatingFace (hwnd, oldm);	// Erase old outline
		if (HitTestVerts(m,vpt,dummyVert))
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else {
			if (flags & MOUSE_SHIFT) {
				SetCursor(hCurCreateVert);
				vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT|SNAP_SEL_SUBOBJ_ONLY);
			} else SetCursor (LoadCursor (NULL, IDC_ARROW));
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
			if (flags & MOUSE_SHIFT) SetCursor(hCurCreateVert);
			else SetCursor (LoadCursor (NULL, IDC_ARROW));
 			vpt->SnapPreview (m, m, NULL, SNAP_SEL_SUBOBJ_ONLY|SNAP_FORCE_3D_RESULT);
		}
		break;

	case MOUSE_ABORT:
		pt = 0;
		vts.SetCount(0);
		et->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		ip->RedrawViews (ip->GetTime());
		break;
	}

	ip->ReleaseViewport(vpt);
	return TRUE;
}

/*-----------------------------------------------------------------------*/

BOOL AttachPickMode::Filter(INode *node) {
	if (!node) return FALSE;
	if (!eo) return false;
	if (!ip) return false;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	eo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(triObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(triObjectClassID)) return TRUE;
	return FALSE;
}

BOOL AttachPickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!eo) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL AttachPickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!eo) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return FALSE;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	BOOL ret = TRUE;
	if (nodes[0]->GetMtl() && node->GetMtl() && (nodes[0]->GetMtl()!=node->GetMtl())) {
		ret = DoAttachMatOptionDialog (ip, eo);
	}
	nodes.DisposeTemporary ();
	if (!ret) return FALSE;
	if (!eo->ip) return FALSE;

	bool canUndo = TRUE;
	eo->Attach (node, canUndo);
	if (!canUndo) GetSystemSetting (SYSSET_CLEAR_UNDO);
	ip->RedrawViews(ip->GetTime());
	return FALSE;
}

void AttachPickMode::EnterMode(IObjParam *ip) {
	if (!eo) return;

	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (eo->hGeom,IDC_OBJ_ATTACH));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void AttachPickMode::ExitMode(IObjParam *ip) {
	if (!eo) return;

	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_OBJ_ATTACH));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// -----------------------------

int AttachHitByName::filter(INode *node) {
	if (!node) return FALSE;

	// Make sure the node does not depend on this modifier.
	node->BeginDependencyTest();
	eo->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(eo->ip->GetTime());
	if (os.obj->IsSubClassOf(triObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(triObjectClassID)) return TRUE;
	return FALSE;
}

void AttachHitByName::proc(INodeTab &nodeTab) {
	if (inProc) return;
	inProc = TRUE;
	ModContextList mcList;
	INodeTab nodes;
	eo->ip->GetModContexts (mcList, nodes);
	BOOL ret = TRUE;
	if (nodes[0]->GetMtl()) {
		for (int i=0; i<nodeTab.Count(); i++) {
			if (nodeTab[i]->GetMtl() && (nodes[0]->GetMtl()!=nodeTab[i]->GetMtl())) break;
		}
		if (i<nodeTab.Count()) ret = DoAttachMatOptionDialog ((IObjParam *)eo->ip, eo);
		if (!eo->ip) ret = FALSE;
	}
	nodes.DisposeTemporary ();
	inProc = FALSE;
	if (!ret) return;
	eo->MultiAttach (nodeTab);
}

//----------------------------------------------------------

void DivideEdgeProc::EdgePick(DWORD edge, float prop) {			
	theHold.Begin();
	MeshDelta tmd;
	tmd.DivideEdge (et->GetMesh(), edge, prop, et->TempData()->AdjEList());
	et->ApplyMeshDelta (tmd, et, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_EDGEDIVIDE));

	ip->RedrawViews(ip->GetTime());	
}

void DivideEdgeCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_DIVIDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);		
}

void DivideEdgeCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_DIVIDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);		
}

//----------------------------------------------------------

void DivideFaceProc::FacePick (DWORD ff, float *bary) {
	theHold.Begin();
	MeshDelta tmd;
	tmd.DivideFace (et->GetMesh(), ff, bary);
	et->ApplyMeshDelta (tmd, et, ip->GetTime());
	theHold.Accept(GetString(IDS_FACE_DIVIDE));
	ip->RedrawViews(ip->GetTime());	
}

void DivideFaceCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom, IDC_DIVIDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);		
}

void DivideFaceCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom, IDC_DIVIDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

void TurnEdgeProc::EdgePick(DWORD edge, float prop) {
	theHold.Begin();
	MeshDelta tmd(et->GetMesh());
	tmd.TurnEdge (et->GetMesh(), edge);
	et->ApplyMeshDelta (tmd, et, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_EDGETURN));

	ip->RedrawViews(ip->GetTime());	
}

void TurnEdgeCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_EDGE_TURN));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);		
}

void TurnEdgeCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_EDGE_TURN));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);		
}

// ------------------------------------------------------

int ExtrudeMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
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
		if (!point) {
			eo->BeginExtrude(ip->GetTime());		
			om = m;
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			eo->EndExtrude(ip->GetTime(),TRUE);
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView (om,float(-200));
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView (m2,float(-200));
		amount = Length (p1-p0);
		if (m.y > om.y) amount *= -1.0f;
		eo->Extrude (ip->GetTime(), amount);

		spin = GetISpinner (GetDlgItem (eo->hGeom,IDC_EXTRUDESPINNER));
		if (spin) {
			spin->SetValue (amount, FALSE);
			ReleaseISpinner(spin);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		eo->EndExtrude(ip->GetTime(),FALSE);			
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
	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ExtrudeCMode::ExitMode() {
	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_EXTRUDE));
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
			eo->BeginBevel (ip->GetTime(), TRUE);
			break;
		case 1:
			m1 = m;
			m1set = TRUE;
			p0 = vpt->MapScreenToView (m0, float(-200));
			m2.x = m0.x;
			m2.y = m.y;
			p1 = vpt->MapScreenToView (m2, float(-200));
			height = Length (p0-p1);
			if (m1.y > m0.y) height *= -1.0f;
			eo->Bevel (ip->GetTime(), 0, height);
			spin = GetISpinner(GetDlgItem(eo->hGeom,IDC_EXTRUDESPINNER));
			if (spin) {
				spin->SetValue(height,FALSE);
				ReleaseISpinner(spin);
			}
			break;
		case 2:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			eo->EndBevel(ip->GetTime(),TRUE);
			m1set = m0set = FALSE;
			break;
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
			eo->Bevel (ip->GetTime(), 0, amount);
			spin = GetISpinner(GetDlgItem(eo->hGeom,IDC_EXTRUDESPINNER));
		} else {
			p0 = vpt->MapScreenToView (m1, float(-200));
			m2.x = m1.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			amount = Length (p1-p0);
			if (m.y > m1.y) amount *= -1.0f;
			eo->Bevel (ip->GetTime(), amount, height);
			spin = GetISpinner(GetDlgItem(eo->hGeom,IDC_OUTLINESPINNER));
		}
		if (spin) {
			spin->SetValue(amount,FALSE);
			ReleaseISpinner(spin);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		eo->EndBevel(ip->GetTime(),FALSE);
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m1set = m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR BevelSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_BEVELCUR));
	return hCur;
}

void BevelCMode::EnterMode() {
	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void BevelCMode::ExitMode() {
	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_BEVEL));
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
			eo->BeginChamfer (ip->GetTime());		
			om = m;
		} else {
			ip->RedrawViews (ip->GetTime(),REDRAW_END);
			eo->EndChamfer (ip->GetTime(),TRUE);
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om,float(-200));
		p1 = vpt->MapScreenToView(m,float(-200));
		eo->Chamfer (ip->GetTime(), Length(p1-p0));

		spin = GetISpinner(GetDlgItem(eo->hGeom,IDC_OUTLINESPINNER));
		if (spin) {
			spin->SetValue(Length(p1-p0),FALSE);
			ReleaseISpinner(spin);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		eo->EndChamfer (ip->GetTime(),FALSE);			
		ip->RedrawViews (ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ChamferSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hECur = NULL;
	static HCURSOR hVCur = NULL;
	if (eto->selLevel == SL_VERTEX) {
		if ( !hVCur ) hVCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_VCHAMFERCUR));
		return hVCur;
	}
	if ( !hECur ) hECur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ECHAMFERCUR));
	return hECur;
}

void ChamferCMode::EnterMode() {
	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ChamferCMode::ExitMode() {
	if (!eo->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(eo->hGeom,IDC_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// --- Slice: not really a command mode, just looks like it.--------- //

// Each time we enter the slice mode, we reset the slice plane.

void EditTriObject::EnterSliceMode () {
	sliceMode = TRUE;
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_SLICE));
		but->Enable ();
		ReleaseICustButton (but);
		EnableWindow (GetDlgItem (hGeom, IDC_REFINE), FALSE);
		but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
		but->SetCheck (TRUE);
		ReleaseICustButton (but);
	}

	Box3 box = GetMesh().getBoundingBox();
	sliceCenter = (box.pmax + box.pmin)*.5f;
	sliceRot.Identity();
	box.pmax -= box.pmin;
	sliceSize = (box.pmax.x > box.pmax.y) ? box.pmax.x : box.pmax.y;
	if (box.pmax.z > sliceSize) sliceSize = box.pmax.z;
	sliceSize *= .52f;
	if (sliceSize < 1) sliceSize = 1.0f;

	if (ip->GetCommandMode()->ID() >= CID_USER) ip->SetStdCommandMode (CID_OBJMOVE);
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

void EditTriObject::ExitSliceMode () {
	sliceMode = FALSE;
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_SLICE));
		but->Disable ();
		ReleaseICustButton (but);
		if (selLevel >= SL_EDGE) EnableWindow (GetDlgItem (hGeom, IDC_REFINE), TRUE);
		but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
		but->SetCheck (FALSE);
		ReleaseICustButton (but);
	}
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime());
}

// -- Cut Edge proc/mode -------------------------------------

class CutAbandon : public RestoreObj {
public:
	EditTriObject *obj;

	CutAbandon (EditTriObject *m) { obj = m; }
	void Restore(int isUndo) { if (obj->cutEdgeMode) obj->cutEdgeMode->AbandonCut(); }
	void Redo() { if (obj->cutEdgeMode) obj->cutEdgeMode->AbandonCut (); }
	int Size() { return sizeof(void *); }
	TSTR Description() { return TSTR(_T("Editable Mesh Cut Abandon")); }
};

HitRecord *CutEdgeProc::HitTestEdges (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *ret = hitLog.ClosestHit();

	if (e1set && (e1 == (int)ret->hitInfo)) return NULL;
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

int CutEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt;
	HitRecord *hr;
	AdjEdgeList *ae;
	AdjFaceList *af;
	MeshDelta tmd;
	IPoint2 betterM;
	Point3 A, B, snapPoint, Xdir, Zdir;
	Matrix3 obj2world, world2obj;
	float prop;
	Ray r;

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
		ae = et->TempData()->AdjEList();
		tmd.DivideEdge (et->GetMesh(), e1, prop1, ae, FALSE, et->cutRefine,
			FALSE, et->sliceSplit);
		et->ApplyMeshDelta (tmd, et, ip->GetTime());
		theHold.Accept (GetString (IDS_SCA_EDGECUT));
		ip->RedrawViews(ip->GetTime());
		e1set = FALSE;
		return FALSE;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		hr = HitTestEdges(m,vpt);
		if (!hr) {
			ip->ReleaseViewport (vpt);
			break;
		}

		DWORD ee;
		ee = hr->hitInfo;

		// Find where along this edge we hit
		// Strategy:
		// Get Mouse click, plus viewport z-direction at mouse click, in object space.
		// Then find the direction of the edge in a plane orthogonal to z, and see how far
		// along that edge we are.
		snapPoint = vpt->SnapPoint (m, betterM, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
		Zdir = Normalize (r.dir);
		obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());
		A = obj2world * et->GetMesh().verts[et->GetMesh().faces[ee/3].v[ee%3]];
		B = obj2world * et->GetMesh().verts[et->GetMesh().faces[ee/3].v[(ee+1)%3]];
		Xdir = B-A;
		Xdir -= DotProd(Xdir, Zdir)*Zdir;
		prop = DotProd (Xdir, snapPoint-A) / LengthSquared (Xdir);
		if (prop<.0001f) prop=0;
		if (prop>.9999f) prop=1;

		ip->ReleaseViewport (vpt);

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
		af = et->TempData()->AdjFList();
		world2obj = Inverse (obj2world);
		Zdir = Normalize (VectorTransform (world2obj, Zdir));
		theHold.Begin ();
		e1 = tmd.Cut (et->GetMesh(), e1, prop1, ee, prop, -Zdir, et->cutRefine, et->sliceSplit);
		et->ApplyMeshDelta (tmd, et, ip->GetTime());
		theHold.Put (new CutAbandon(et));
		theHold.Accept (GetString (IDS_SCA_EDGECUT));
		ip->RedrawViews (ip->GetTime());

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
		ip->ReleaseViewport (vpt);
		ip->RedrawViews (ip->GetTime());
		if (e1set) DrawCutter (hwnd,oldm2);
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		vpt->SnapPreview(m,m,NULL);//, SNAP_SEL_OBJS_ONLY);
		if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		ip->ReleaseViewport (vpt);
		break;
	}

	return TRUE;	
}

void CutEdgeCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_CUT));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.e1set = FALSE;
	et->inCutEdge = TRUE;
}

void CutEdgeCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_CUT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	et->inCutEdge = FALSE;
}

void CutEdgeCMode::AbandonCut () {
	proc.e1set = FALSE;
}

/*-------------------------------------------------------------------*/

void WeldVertCMode::EnterMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_WELDTOVERT));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void WeldVertCMode::ExitMode() {
	if (!et->hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hGeom,IDC_WELDTOVERT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

BOOL WeldVertMouseProc::HitTestVerts (IPoint2 &m, ViewExp *vpt, int &v) {
	vpt->ClearSubObjHitList();	
	et->pickBoxSize = et->weldBoxSize;
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, HIT_UNSELONLY, &m, vpt);
	et->pickBoxSize = DEF_PICKBOX_SIZE;
	if (!vpt->NumSubObjHits()) return FALSE;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	MaxAssert(hr);
	v = hr->hitInfo;
	return TRUE;
}

void WeldVertMouseProc::PostTransformHolding () {
	TransformModBox::PostTransformHolding();	// so our object gets the "TranformHoldingFinish" call.
	if (targetVert < 0) return;
	Mesh & mesh = et->GetMesh();
	Point3 pt = mesh.verts[targetVert];

	// Select the point that was hit
	BitArray vsel = et->GetVertSel ();
	vsel.Set (targetVert);
	et->SetVertSel (vsel, et, ip->GetTime());

	// Do the weld
	et->WeldVerts (pt);
}

int WeldVertMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
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
			}
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

void FlipNormProc::FacePick (DWORD face, float *bary) {
	theHold.Begin();
	MeshDelta tmd (et->GetMesh());
	if (et->selLevel <= SL_FACE) {
		tmd.FlipNormal (et->GetMesh(), face);
	} else {
		BitArray flip;
		flip.SetSize (tmd.fnum);
		if (et->selLevel == SL_POLY) {
			et->GetMesh().PolyFromFace (face, flip, et->GetPolyFaceThresh(), et->ignoreVisEdge, et->TempData()->AdjFList());
		} else {
			et->GetMesh().ElementFromFace (face, flip, et->TempData()->AdjFList());
		}
		for (DWORD i=0; i<tmd.fnum; i++) if (flip[i]) tmd.FlipNormal (et->GetMesh(), i);
	}
	et->ApplyMeshDelta (tmd, et, ip->GetTime());
	theHold.Accept(GetString(IDS_RB_FLIPNORMALS));
	ip->RedrawViews(ip->GetTime());
}

void FlipNormCMode::EnterMode() {
	if (!et->hSurf) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hSurf, IDC_NORMAL_FLIPMODE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);		
}

void FlipNormCMode::ExitMode() {
	if (!et->hSurf) return;
	ICustButton *but = GetICustButton(GetDlgItem(et->hSurf, IDC_NORMAL_FLIPMODE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//----------------------------------------------------------
// MeshDeltaUserData methods  (see also triedui.cpp for ui-related MeshDeltaUser stuff

void EditTriObject::MoveSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)
{
}

void EditTriObject::RotateSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin)
{
}

void EditTriObject::ScaleSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)
{
}

void EditTriObject::ExtrudeSelection(int level, BitArray* sel, float amount, float bevel, BOOL groupNormal, Point3* direction)
{
}

