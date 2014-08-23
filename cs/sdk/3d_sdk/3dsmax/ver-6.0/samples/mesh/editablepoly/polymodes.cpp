 /**********************************************************************  
 *<
	FILE: PolyModes.cpp

	DESCRIPTION: Editable Polygon Mesh Object - Command modes

	CREATED BY: Steve Anderson

	HISTORY: created April 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "macrorec.h"
#include "decomp.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

#define EPSILON .0001f

CommandMode * EditPolyObject::getCommandMode (int mode) {
	switch (mode) {
	case epmode_create_vertex: return createVertMode;
	case epmode_create_edge: return createEdgeMode;
	case epmode_create_face: return createFaceMode;
	case epmode_divide_edge: return divideEdgeMode;
	case epmode_divide_face: return divideFaceMode;

	case epmode_extrude_vertex:
	case epmode_extrude_edge:
		return extrudeVEMode;
	case epmode_extrude_face: return extrudeMode;
	case epmode_chamfer_vertex:
	case epmode_chamfer_edge:
		return chamferMode;
	case epmode_bevel: return bevelMode;
	case epmode_inset_face: return insetMode;
	case epmode_outline: return outlineMode;
	case epmode_cut_vertex: return cutMode;
	case epmode_cut_edge: return cutMode;
	case epmode_cut_face: return cutMode;
	case epmode_quickslice: return quickSliceMode;
	case epmode_weld: return weldMode;
	case epmode_lift_from_edge: return liftFromEdgeMode;
	case epmode_pick_lift_edge: return pickLiftEdgeMode;
	case epmode_edit_tri: return editTriMode;
	}
	return NULL;
}

void EditPolyObject::EpActionToggleCommandMode (int mode) {
	if (!ip) return;
	if ((mode == epmode_sliceplane) && (selLevel == EP_SL_OBJECT)) return;

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
	macroRecorder->FunctionCall(_T("$.EditablePoly.toggleCommandMode"), 1, 0, mr_int, mode);
	macroRecorder->EmitScript ();
#endif

	if (mode == epmode_sliceplane) {
		// Special case.
		ip->ClearPickMode();
		if (sliceMode) ExitSliceMode();
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have slice mode, settings at same time.
			EnterSliceMode();
			// If we're already in a SO move or rotate mode, stay in it;
			// Otherwise, enter SO move.
			if ((ip->GetCommandMode() != moveMode) && (ip->GetCommandMode() != rotMode)) {
				ip->PushCommandMode (moveMode);
			}
		}
		return;
	}

	// Otherwise, make sure we're not in Slice mode:
	if (sliceMode) ExitSliceMode ();

	CommandMode *cmd = getCommandMode (mode);
	if (cmd==NULL) return;
	CommandMode *currentMode = ip->GetCommandMode ();

	switch (mode) {
	case epmode_extrude_vertex:
	case epmode_chamfer_vertex:
		// Special case - only use DeleteMode to exit mode if in correct SO level,
		// Otherwise use EnterCommandMode to switch SO levels and enter mode again.
		if ((currentMode==cmd) && (selLevel==EP_SL_VERTEX)) ip->DeleteMode (cmd);
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have command mode, settings at same time.
			EnterCommandMode (mode);
		}
		break;
	case epmode_extrude_edge:
	case epmode_chamfer_edge:
		// Special case - only use DeleteMode to exit mode if in correct SO level,
		// Otherwise use EnterCommandMode to switch SO levels and enter mode again.
		if ((currentMode==cmd) && (meshSelLevel[selLevel]==MNM_SL_EDGE)) ip->DeleteMode (cmd);
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have command mode, settings at same time.
			EnterCommandMode (mode);
		}
		break;
	case epmode_pick_lift_edge:
		// Special case - we do not want to end our preview or close the dialog,
		// since this command mode is controlled from the dialog.
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else EnterCommandMode (mode);
		break;
	default:
		if (currentMode == cmd) ip->DeleteMode (cmd);
		else {
			EpPreviewCancel ();
			EpfnClosePopupDialog ();	// Cannot have command mode, settings at same time.
			EnterCommandMode (mode);
		}
		break;
	}
}

void EditPolyObject::EnterCommandMode(int mode) {
	if (!ip) return;

	// First of all, we don't want to pile up our command modes:
	ExitAllCommandModes (false, false);

	switch (mode) {
	case epmode_create_vertex:
		if (selLevel != EP_SL_VERTEX) ip->SetSubObjectLevel (EP_SL_VERTEX);
		ip->PushCommandMode(createVertMode);
		break;

	case epmode_create_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (createEdgeMode);
		break;

	case epmode_create_face:
	
		if ((selLevel < EP_SL_FACE) && (selLevel != EP_SL_OBJECT)) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (createFaceMode);
		break;

	case epmode_divide_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (divideEdgeMode);
		break;

	case epmode_divide_face:
		if (selLevel < EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (divideFaceMode);
		break;

	case epmode_extrude_vertex:
		if (selLevel != EP_SL_VERTEX) ip->SetSubObjectLevel (EP_SL_VERTEX);
	
		ip->PushCommandMode (extrudeVEMode);
		break;

	case epmode_extrude_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
	
		ip->PushCommandMode (extrudeVEMode);
		break;

	case epmode_extrude_face:
		if (selLevel < EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (extrudeMode);
		break;

	case epmode_chamfer_vertex:
		if (selLevel != EP_SL_VERTEX) ip->SetSubObjectLevel (EP_SL_VERTEX);
		ip->PushCommandMode (chamferMode);
		break;

	case epmode_chamfer_edge:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (chamferMode);
		break;

	case epmode_bevel:
		if (selLevel < EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (bevelMode);
		break;


	case epmode_inset_face:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (insetMode);
		break;

	case epmode_outline:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (outlineMode);
		break;


	//(Should never have been 3 of these.)
	case epmode_cut_vertex:
	case epmode_cut_edge:
	case epmode_cut_face:
		ip->PushCommandMode (cutMode);
		break;


	case epmode_quickslice:
		ip->PushCommandMode (quickSliceMode);
		break;

	case epmode_weld:
		if (selLevel > EP_SL_BORDER) ip->SetSubObjectLevel (EP_SL_VERTEX);
		if (selLevel == EP_SL_BORDER) ip->SetSubObjectLevel (EP_SL_EDGE);
		ip->PushCommandMode (weldMode);
		break;


	case epmode_lift_from_edge:
		if (selLevel != EP_SL_FACE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (liftFromEdgeMode);
		break;


	case epmode_pick_lift_edge:
		ip->PushCommandMode (pickLiftEdgeMode);
		break;

	case epmode_edit_tri:
		if (selLevel < EP_SL_EDGE) ip->SetSubObjectLevel (EP_SL_FACE);
		ip->PushCommandMode (editTriMode);
		break;
	}
}

void EditPolyObject::EpActionEnterPickMode (int mode) {
	if (!ip) return;

	// Make sure we're not in Slice mode:
	if (sliceMode) ExitSliceMode ();

#ifdef EPOLY_MACRO_RECORD_MODES_AND_DIALOGS
	macroRecorder->FunctionCall(_T("$.EditablePoly.enterPickMode"), 1, 0, mr_int, mode);
	macroRecorder->EmitScript ();
#endif

	switch (mode) {
	case epmode_attach:
		ip->SetPickMode (attachPickMode);
		break;
	case epmode_pick_shape:
		if (GetMNSelLevel () != MNM_SL_FACE) return;
	
		ip->SetPickMode (shapePickMode);
		break;
	}
}

//------------Command modes & Mouse procs----------------------

HitRecord *PickEdgeMouseProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, float *prop, 
											Point3 *snapPoint) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	if (pEditPoly->GetEPolySelLevel() != EP_SL_BORDER)
		pEditPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	if (pEditPoly->GetEPolySelLevel() != EP_SL_BORDER)
		pEditPoly->ClearHitLevelOverride ();
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

	MNMesh & mm = *(mpEPoly->GetMeshPtr());
	Point3 A = obj2world * mm.v[mm.e[ee].v1].p;
	Point3 B = obj2world * mm.v[mm.e[ee].v2].p;
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
	
		if (!hr) break;
		if (!EdgePick (hr->hitInfo, prop)) {
			// False return value indicates exit mode.
			ip->PopCommandMode ();
			return false;
		}
		return true;

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

HitRecord *PickFaceMouseProc::HitTestFaces (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNFACES);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	return hr;
}

void PickFaceMouseProc::ProjectHitToFace (IPoint2 &m, ViewExp *vpt,
										  HitRecord *hr, Point3 *snapPoint) {
	if (!hr) return ;

	// Find subtriangle, barycentric coordinates of hit in face.
	face = hr->hitInfo;
	Matrix3 obj2world = hr->nodeRef->GetObjectTM (ip->GetTime ());

	Ray r;
	vpt->MapScreenToWorldRay ((float)m.x, (float)m.y, r);
	if (!snapPoint) snapPoint = &(r.p);
	Point3 Zdir = Normalize (r.dir);

	// Find an approximate location for the point on the surface we hit:
	// Get the average normal for the face, thus the plane, and intersect.
	Point3 intersect;
	MNMesh & mm = *(mpEPoly->GetMeshPtr());
	Point3 planeNormal = mm.GetFaceNormal (face, TRUE);
	planeNormal = Normalize (obj2world.VectorTransform (planeNormal));
	float planeOffset=0.0f;
	for (int i=0; i<mm.f[face].deg; i++)
		planeOffset += DotProd (planeNormal, obj2world*mm.v[mm.f[face].vtx[i]].p);
	planeOffset = planeOffset/float(mm.f[face].deg);

	// Now we intersect the snapPoint + r.dir*t line with the
	// DotProd (planeNormal, X) = planeOffset plane.
	float rayPlane = DotProd (r.dir, planeNormal);
	float firstPointOffset = planeOffset - DotProd (planeNormal, *snapPoint);
	if (fabsf(rayPlane) > EPSILON) {
		float amount = firstPointOffset / rayPlane;
		intersect = *snapPoint + amount*r.dir;
	} else {
		intersect = *snapPoint;
	}

	Matrix3 world2obj = Inverse (obj2world);
	intersect = world2obj * intersect;

	mm.FacePointBary (face, intersect, bary);
}

int PickFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt;
	HitRecord *hr;
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
		hr = HitTestFaces (m, vpt);
		ProjectHitToFace (m, vpt, hr, &snapPoint);
		if (vpt) ip->ReleaseViewport(vpt);
		if (hr) FacePick ();
		break;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		if (HitTestFaces(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		break;
	}

	return TRUE;
}

// -------------------------------------------------------

HitRecord *ConnectVertsMouseProc::HitTestVertices (IPoint2 & m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNVERTS);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (v1<0) {
		// can't accept vertices on no faces.
		if (mpEPoly->GetMeshPtr()->vfac[hr->hitInfo].Count() == 0) return NULL;
		return hr;
	}

	// Otherwise, we're looking for a vertex on v1's faces - these are listed in neighbors.
	for (int i=0; i<neighbors.Count(); i++) if (neighbors[i] == hr->hitInfo) break;
	if (i>=neighbors.Count()) return NULL;
	return hr;
}

void ConnectVertsMouseProc::DrawDiag (HWND hWnd, const IPoint2 & m) {
	if (v1<0) return;

	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));

	MoveToEx (hdc, m1.x, m1.y, NULL);
	LineTo (hdc, m.x, m.y);

	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

void ConnectVertsMouseProc::SetV1 (int vv) {
	v1 = vv;
	neighbors.ZeroCount();
	Tab<int> & vf = mpEPoly->GetMeshPtr()->vfac[vv];
	Tab<int> & ve = mpEPoly->GetMeshPtr()->vedg[vv];
	// Add to neighbors all the vertices that share faces (but no edges) with this one:
	int i,j,k;
	for (i=0; i<vf.Count(); i++) {
		MNFace & mf = mpEPoly->GetMeshPtr()->f[vf[i]];
		for (j=0; j<mf.deg; j++) {
			// Do not consider v1 a neighbor:
			if (mf.vtx[j] == v1) continue;

			// Filter out those that share an edge with v1:
			for (k=0; k<ve.Count(); k++) {
				if (mpEPoly->GetMeshPtr()->e[ve[k]].OtherVert (vv) == mf.vtx[j]) break;
			}
			if (k<ve.Count()) continue;

			// Add to neighbor list.
			neighbors.Append (1, &(mf.vtx[j]), 4);
		}
	}
}

int ConnectVertsMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt;
	HitRecord *hr;
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_INIT:
		v1 = v2 = -1;
		neighbors.ZeroCount ();
		break;

	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		ip->SetActiveViewport(hwnd);
		vpt = ip->GetViewport(hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		hr = HitTestVertices (m, vpt);
		ip->ReleaseViewport(vpt);
		if (!hr) break;
		if (v1<0) {
			SetV1 (hr->hitInfo);
			m1 = m;
			lastm = m;
			DrawDiag (hwnd, m);
			break;
		}
		// Otherwise, we've found a connection.
		DrawDiag (hwnd, lastm);	// erase last dotted line.
		v2 = hr->hitInfo;
		VertConnect ();
		v1 = -1;
		return FALSE;	// Done with this connection.

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		if (hr=HitTestVertices(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		if (vpt) ip->ReleaseViewport(vpt);
		DrawDiag (hwnd, lastm);
		DrawDiag (hwnd, m);
		lastm = m;
		break;
	}

	return TRUE;
}

// -------------------------------------------------------

static HCURSOR hCurCreateVert = NULL;

void CreateVertCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void CreateVertCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
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
		theHold.Begin ();
		if (mpEPoly->EpfnCreateVertex(pt)<0) {
			theHold.Cancel ();
		} else {
			theHold.Accept (GetString (IDS_CREATE_VERTEX));
			mpEPoly->RefreshScreen ();
		}
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

void CreateEdgeCMode::EnterMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateEdgeCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateEdgeMouseProc::VertConnect () {
	theHold.Begin();
	if (mpEPoly->EpfnCreateEdge (v1, v2) < 0) {
		theHold.Cancel ();
		return;
	}
	theHold.Accept (GetString (IDS_CREATE_EDGE));
	mpEPoly->RefreshScreen ();
}

//----------------------------------------------------------

void CreateFaceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateFaceCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CREATE));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	proc.pt = 0;
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void CreateFaceCMode::Display (GraphicsWindow *gw) {
	if (proc.pt < 2) return;
	Tab<Point3> rverts;
	rverts.SetCount (proc.pt+1);
	for (int j=0; j<proc.pt; j++) rverts[j] = mpEditPoly->GetMeshPtr()->P(proc.vts[j]);
	gw->setColor(LINE_COLOR,GetUIColor(COLOR_SUBSELECTION));
	gw->polyline (proc.pt, rverts.Addr(0), NULL, NULL, FALSE, NULL);
}

void CreateFaceMouseProc::DrawCreatingFace (HWND hWnd) {
	if (pt<1) return;

	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));

	MoveToEx (hdc, mpts[0].x, mpts[0].y, NULL);
	LineTo (hdc, mMousePosition.x, mMousePosition.y);
	if (pt>1) LineTo (hdc, mpts[pt-1].x, mpts[pt-1].y);

	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

BOOL CreateFaceMouseProc::HitTestVerts (IPoint2 m, ViewExp *vpt, int &v) {
	vpt->ClearSubObjHitList();

	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNVERTS|SUBHIT_OPENONLY);
	ip->SubObHitTest(ip->GetTime(),HITTYPE_POINT,0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return FALSE;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	assert(hr);
	v = hr->hitInfo;
	return TRUE;
}

int CreateFaceMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	if (!hCurCreateVert) hCurCreateVert = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ADDVERTCUR));

	ViewExp *vpt = ip->GetViewport(hwnd);
	int dummyVert;
	int nv, lpt;
	Point3 snapPoint;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		vts.ZeroCount ();
		mpts.ZeroCount ();
		break;

	case MOUSE_POINT:
		if (point==1) break;	// Don't want to react to release from first click.
		ip->SetActiveViewport(hwnd);
		bool done;
		done=FALSE;

		if (!point) {
			// We need to get node of this polyobject, and obtain world->obj transform.
			ModContextList mcList;
			INodeTab nodes;
			ip->GetModContexts(mcList,nodes);
			Matrix3 otm = nodes[0]->GetObjectTM(ip->GetTime());
			nodes.DisposeTemporary();
			mWorld2obj = Inverse (otm);
		}
		if (HitTestVerts(m, vpt, nv)) {
			HitLog& hitLog = vpt->GetSubObjHitList();
			HitRecord *hr = hitLog.ClosestHit();
			MaxAssert (hr);
			for (int j=0; j<pt; j++) if (vts[j] == nv) break;
			if (j<pt) done=TRUE;
			else {
				vts.Append (1, &nv, 20);
				mpts.Append (1, &m, 20);
				pt++;
				mpEPoly->LocalDataChanged (PART_DISPLAY);
				ip->RedrawViews (ip->GetTime());
				mMousePosition = m;
				DrawCreatingFace (hwnd);
			}
		} else if (flags & MOUSE_SHIFT) {
			Matrix3 ctm;
			vpt->GetConstructionTM(ctm);
			Point3 newpt = vpt->SnapPoint(m,m,&ctm, SNAP_FORCE_3D_RESULT|SNAP_SEL_SUBOBJ_ONLY);
			newpt = newpt * ctm;
			nv = mpEPoly->GetMeshPtr()->numv;

			vts.Append (1, &nv, 20);
			mpts.Append (1, &m, 20);
			pt++;
			theHold.Begin ();
			if (mpEPoly->EpfnCreateVertex(newpt) < 0) {
				theHold.Cancel ();
			} else {
				theHold.Accept (GetString (IDS_CREATE_VERTEX));
				ip->RedrawViews (ip->GetTime());
			}
			mMousePosition = m;
			DrawCreatingFace (hwnd);
		} else {
			if (!pt) return false;
		}

		if (done) {
			// We're done collecting verts - build a face
			lpt = pt;
			pt = 0;	// so the redraw gets that we're done.
			theHold.Begin ();
			if ((lpt>2) && (mpEPoly->EpfnCreateFace(vts.Addr(0), lpt) < 0)) {
				theHold.Cancel ();
				InvalidateRect(vpt->getGW()->getHWnd(),NULL,FALSE);
				TSTR buf1 = GetString(IDS_ILLEGAL_NEW_FACE);
				TSTR buf2 = GetString(IDS_EDITABLE_POLY);
				MessageBox(ip->GetMAXHWnd(),buf1,buf2,MB_OK|MB_ICONINFORMATION);						
				mpEPoly->LocalDataChanged (PART_DISPLAY);
			} else {
				theHold.Accept (GetString (IDS_CREATE_FACE));
			}
			ip->RedrawViews (ip->GetTime());
			vts.ZeroCount();
			mpts.ZeroCount();
			ip->ReleaseViewport (vpt);
			return FALSE;
		}
		break;

	case MOUSE_MOVE:
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		ip->RedrawViews (ip->GetTime(), REDRAW_INTERACTIVE);

		if (pt) DrawCreatingFace (hwnd);
		if (HitTestVerts(m,vpt,dummyVert)) {
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		} else {
			if (flags & MOUSE_SHIFT) SetCursor(hCurCreateVert);
			else SetCursor (LoadCursor (NULL, IDC_ARROW));
		}

		mMousePosition = m;
		DrawCreatingFace (hwnd);
		break;

	case MOUSE_FREEMOVE:
		if (HitTestVerts(m,vpt,dummyVert)) {
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		} else {
			if (flags & MOUSE_SHIFT) {
				vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
				SetCursor(hCurCreateVert);
			} else SetCursor (LoadCursor (NULL, IDC_ARROW));
		}
		break;

	case MOUSE_ABORT:
		pt = 0;
		vts.ZeroCount ();
		mpts.ZeroCount ();
		mpEPoly->LocalDataChanged (PART_DISPLAY);
		ip->RedrawViews (ip->GetTime());
		break;
	}

	ip->ReleaseViewport(vpt);
	return TRUE;
}

void CreateFaceMouseProc::Backspace () {
	pt--;
	mpts.Delete (pt-1, 1);
	vts.Delete (pt-1, 1);
}

//-----------------------------------------------------------------------/

BOOL AttachPickMode::Filter(INode *node) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	if (!node) return FALSE;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(polyObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(polyObjectClassID)) return TRUE;
	return FALSE;
}

BOOL AttachPickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL AttachPickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpEditPoly) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return FALSE;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);

	BOOL ret = TRUE;
	if (nodes[0]->GetMtl() && node->GetMtl() && (nodes[0]->GetMtl()!=node->GetMtl())) {
		ret = DoAttachMatOptionDialog (ip, mpEditPoly);
	}
	if (!ret) {
		nodes.DisposeTemporary ();
		return FALSE;
	}

	bool canUndo = TRUE;
	mpEditPoly->EpfnAttach (node, canUndo, nodes[0], ip->GetTime());
	if (!canUndo) GetSystemSetting (SYSSET_CLEAR_UNDO);
	mpEditPoly->RefreshScreen ();
	nodes.DisposeTemporary ();
	return FALSE;
}

void AttachPickMode::EnterMode(IObjParam *ip) {
	if (!mpEditPoly) return;
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void AttachPickMode::ExitMode(IObjParam *ip) {
	if (!mpEditPoly) return;
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_ATTACH));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// -----------------------------

int AttachHitByName::filter(INode *node) {
	if (!node) return FALSE;

	// Make sure the node does not depend on this modifier.
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(mpEditPoly->ip->GetTime());
	if (os.obj->IsSubClassOf(polyObjectClassID)) return TRUE;
	if (os.obj->CanConvertToType(polyObjectClassID)) return TRUE;
	return FALSE;
}

void AttachHitByName::proc(INodeTab &nodeTab) {
	if (inProc) return;
	if (!mpEditPoly->ip) return;
	inProc = TRUE;
	ModContextList mcList;
	INodeTab nodes;
	mpEditPoly->ip->GetModContexts (mcList, nodes);
	BOOL ret = TRUE;
	if (nodes[0]->GetMtl()) {
		for (int i=0; i<nodeTab.Count(); i++) {
			if (nodeTab[i]->GetMtl() && (nodes[0]->GetMtl()!=nodeTab[i]->GetMtl())) break;
		}
		if (i<nodeTab.Count()) ret = DoAttachMatOptionDialog ((IObjParam *)mpEditPoly->ip, mpEditPoly);
		if (!mpEditPoly->ip) ret = FALSE;
	}
	inProc = FALSE;
	if (ret) mpEditPoly->EpfnMultiAttach (nodeTab, nodes[0], mpEditPoly->ip->GetTime ());
	nodes.DisposeTemporary ();
}

//-----------------------------------------------------------------------/

BOOL ShapePickMode::Filter(INode *node) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	if (!node) return FALSE;

	// Make sure the node does not depend on us
	node->BeginDependencyTest();
	mpEditPoly->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
	if (os.obj->IsSubClassOf(splineShapeClassID)) return TRUE;
	if (os.obj->CanConvertToType(splineShapeClassID)) return TRUE;
	return FALSE;
}

BOOL ShapePickMode::HitTest(IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m,int flags) {
	if (!mpEditPoly) return false;
	if (!ip) return false;
	return ip->PickNode(hWnd,m,this) ? TRUE : FALSE;
}

BOOL ShapePickMode::Pick(IObjParam *ip,ViewExp *vpt) {
	if (!mpEditPoly) return false;
	INode *node = vpt->GetClosestHit();
	if (!Filter(node)) return FALSE;
	mpEditPoly->getParamBlock()->SetValue (ep_extrude_spline_node, ip->GetTime(), node);
	if (!mpEditPoly->EpPreviewOn()) {
		// We're not in preview mode - do the extrusion.
		mpEditPoly->EpActionButtonOp (epop_extrude_along_spline);
	} else {
	mpEditPoly->RefreshScreen ();
	}
	return TRUE;
}

void ShapePickMode::EnterMode(IObjParam *ip) {
	HWND hOp = mpEditPoly->GetDlgHandle (ep_settings);
	ICustButton *but = NULL;
	if (hOp) but = GetICustButton (GetDlgItem (hOp, IDC_EXTRUDE_PICK_SPLINE));
	if (but) {
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		but = NULL;
	}
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hGeom) but = GetICustButton (GetDlgItem (hGeom, IDC_EXTRUDE_ALONG_SPLINE));
	if (but) {
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		but = NULL;
	}
}

void ShapePickMode::ExitMode(IObjParam *ip) {
	HWND hOp = mpEditPoly->GetDlgHandle (ep_settings);
	ICustButton *but = NULL;
	if (hOp) but = GetICustButton (GetDlgItem (hOp, IDC_EXTRUDE_PICK_SPLINE));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hGeom) but = GetICustButton (GetDlgItem (hGeom, IDC_EXTRUDE_ALONG_SPLINE));
	if (but) {
		but->SetCheck(false);
		ReleaseICustButton(but);
		but = NULL;
	}
}

//----------------------------------------------------------

// Divide edge modifies two faces; creates a new vertex and a new edge.
bool DivideEdgeProc::EdgePick (int edge, float prop) {
	theHold.Begin ();
	mpEPoly->EpfnDivideEdge (edge, prop);
	theHold.Accept (GetString (IDS_INSERT_VERTEX));
	mpEPoly->RefreshScreen ();
	return true;	// false = exit mode when done; true = stay in mode.
}

void DivideEdgeCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_edge);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void DivideEdgeCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_edge);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

//----------------------------------------------------------

void DivideFaceProc::FacePick () {
	theHold.Begin ();
	mpEPoly->EpfnDivideFace (face, bary);
	theHold.Accept (GetString (IDS_INSERT_VERTEX));
	mpEPoly->RefreshScreen ();
}

void DivideFaceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);		
	mpEditPoly->SetDisplayLevelOverride (MNDISP_VERTTICKS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void DivideFaceCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSERT_VERTEX));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
	mpEditPoly->ClearDisplayLevelOverride ();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

// ------------------------------------------------------

int ExtrudeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m2;
	float height;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			mpEditPoly->EpfnBeginExtrude(mpEditPoly->GetMNSelLevel(), MN_SEL, ip->GetTime());
			om = m;
		} else {
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			mpEditPoly->EpfnEndExtrude (true, ip->GetTime());
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView (om,float(-200));
		m2.x = om.x;
		m2.y = m.y;
		p1 = vpt->MapScreenToView (m2,float(-200));
		height = Length (p1-p0);
		if (m.y > om.y) height *= -1.0f;
		mpEditPoly->EpfnDragExtrude (height, ip->GetTime());
		mpEditPoly->getParamBlock()->SetValue (ep_face_extrude_height, ip->GetTime(), height);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndExtrude (false, ip->GetTime ());
		ip->RedrawViews (ip->GetTime(), REDRAW_END);
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
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	if (but) {
		but->SetCheck (true);
		ReleaseICustButton(but);
	} else {
		DebugPrint ("Editable Poly: we're entering Extrude mode, but we can't find the extrude button!\n");
		DbgAssert (0);
	}
}

void ExtrudeCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	if (but) {
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	} else {
		DebugPrint ("Editable Poly: we're exiting Extrude mode, but we can't find the extrude button!\n");
		DbgAssert (0);
	}
}

// ------------------------------------------------------

int ExtrudeVEMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1, p2;
	IPoint2 m1, m2;
	float width, height;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_height, TimeValue(0), 0.0f);
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_width, TimeValue(0), 0.0f);
				break;
			case MNM_SL_EDGE:
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_height, TimeValue(0), 0.0f);
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_width, TimeValue(0), 0.0f);
				break;
			}
			mpEditPoly->EpPreviewBegin (epop_extrude);
			break;
		case 1:
			p0 = vpt->MapScreenToView (m0, float(-200));
			m1.x = m.x;
			m1.y = m0.y;
			p1 = vpt->MapScreenToView (m1, float(-200));
			m2.x = m0.x;
			m2.y = m.y;
			p2 = vpt->MapScreenToView (m2, float(-200));
			width = Length (p0 - p1);
			height = Length (p0 - p2);
			if (m.y > m0.y) height *= -1.0f;
			switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_width, ip->GetTime(), width);
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_height, ip->GetTime(), height);
				break;
			case MNM_SL_EDGE:
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_width, ip->GetTime(), width);
				mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_height, ip->GetTime(), height);
				break;
			}
			mpEditPoly->EpPreviewAccept ();
			ip->RedrawViews(ip->GetTime());
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
		p0 = vpt->MapScreenToView (m0, float(-200));
		m1.x = m.x;
		m1.y = m0.y;
		p1 = vpt->MapScreenToView (m1, float(-200));
		m2.x = m0.x;
		m2.y = m.y;
		p2 = vpt->MapScreenToView (m2, float(-200));
		width = Length (p0 - p1);
		height = Length (p0 - p2);
		if (m.y > m0.y) height *= -1.0f;
		switch (mpEditPoly->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_width, ip->GetTime(), width);
			mpEditPoly->getParamBlock()->SetValue (ep_vertex_extrude_height, ip->GetTime(), height);
			break;
		case MNM_SL_EDGE:
			mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_width, ip->GetTime(), width);
			mpEditPoly->getParamBlock()->SetValue (ep_edge_extrude_height, ip->GetTime(), height);
			break;
		}
		mpEditPoly->EpPreviewSetDragging (true);
		mpEditPoly->EpPreviewInvalidate ();
		mpEditPoly->EpPreviewSetDragging (false);

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpPreviewCancel ();
		ip->RedrawViews (ip->GetTime(), REDRAW_END);
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ExtrudeVESelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if (!hCur) {
		if (mpEPoly->GetMNSelLevel() == MNM_SL_EDGE) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDE_EDGE_CUR));
		else hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_EXTRUDE_VERTEX_CUR));
	}
	return hCur; 
}

void ExtrudeVECMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ExtrudeVECMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_EXTRUDE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int BevelMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
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
			mpEditPoly->EpfnBeginBevel (mpEditPoly->GetMNSelLevel(), MN_SEL, true, ip->GetTime());
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
			mpEditPoly->EpfnDragBevel (0.0f, height, ip->GetTime());
			mpEditPoly->getParamBlock()->SetValue (ep_bevel_height, ip->GetTime(), height);
			break;
		case 2:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			mpEditPoly->EpfnEndBevel (true, ip->GetTime());
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
			height = Length (p1-p0);
			if (m.y > m0.y) height *= -1.0f;
			mpEditPoly->EpfnDragBevel (0.0f, height, ip->GetTime());
			mpEditPoly->getParamBlock()->SetValue (ep_bevel_height, ip->GetTime(), height);
		} else {
			p0 = vpt->MapScreenToView (m1, float(-200));
			m2.x = m1.x;
			p1 = vpt->MapScreenToView (m2, float(-200));
			float outline = Length (p1-p0);
			if (m.y > m1.y) outline *= -1.0f;
			mpEditPoly->EpfnDragBevel (outline, height, ip->GetTime());
			mpEditPoly->getParamBlock()->SetValue (ep_bevel_outline, ip->GetTime(), outline);
		}

		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndBevel (false, ip->GetTime());
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
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BEVEL));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void BevelCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_BEVEL));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int InsetMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	ISpinnerControl *spin=NULL;
	float inset;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			mpEditPoly->EpfnBeginInset (mpEditPoly->GetMNSelLevel(), MN_SEL, ip->GetTime());
			break;
		case 1:
			ip->RedrawViews(ip->GetTime(),REDRAW_END);
			mpEditPoly->EpfnEndInset (true, ip->GetTime());
			m0set = FALSE;
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;
		p0 = vpt->MapScreenToView (m0, float(-200));
		p1 = vpt->MapScreenToView (m, float(-200));
		inset = Length (p1-p0);
		mpEditPoly->EpfnDragInset (inset, ip->GetTime());
		mpEditPoly->getParamBlock()->SetValue (ep_inset, ip->GetTime(), inset);
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndInset (false, ip->GetTime());
		ip->RedrawViews(ip->GetTime(),REDRAW_END);
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR InsetSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_INSETCUR));
	return hCur;
}

void InsetCMode::EnterMode() {
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSET));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void InsetCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_INSET));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int OutlineMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	IPoint2 m1;
	ISpinnerControl *spin=NULL;
	float outline;

	switch (msg) {
	case MOUSE_PROPCLICK:
		if (mpEditPoly->EpPreviewOn()) mpEditPoly->EpPreviewCancel ();
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		switch (point) {
		case 0:
			m0 = m;
			m0set = TRUE;
			// Prevent flash of last outline value:
			mpEditPoly->getParamBlock()->SetValue (ep_outline, ip->GetTime(), 0.0f);
			mpEditPoly->EpPreviewBegin (epop_outline);
			break;
		case 1:
			mpEditPoly->EpPreviewAccept();
			m0set = FALSE;
			break;
		}
		break;

	case MOUSE_MOVE:
		if (!m0set) break;

		// Get signed right/left distance from original point:
		p0 = vpt->MapScreenToView (m0, float(-200));
		m1.y = m.y;
		m1.x = m0.x;
		p1 = vpt->MapScreenToView (m1, float(-200));
		outline = Length (p1-p0);
		if (m.y > m0.y) outline *= -1.0f;

		mpEditPoly->EpPreviewSetDragging (true);
		mpEditPoly->getParamBlock()->SetValue (ep_outline, ip->GetTime(), outline);
		mpEditPoly->EpPreviewSetDragging (false);
		mpEditPoly->RefreshScreen ();
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpPreviewCancel ();
		m0set = FALSE;
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR OutlineSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hCur = NULL;
	if ( !hCur ) hCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_INSETCUR));	// STEVE: need new cursor?
	return hCur;
}

void OutlineCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_OUTLINE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void OutlineCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_OUTLINE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// ------------------------------------------------------

int ChamferMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {	
	ViewExp *vpt=ip->GetViewport (hwnd);
	Point3 p0, p1;
	static float chamfer;

	switch (msg) {
	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		break;

	case MOUSE_POINT:
		if (!point) {
			mpEditPoly->EpfnBeginChamfer (mpEditPoly->GetMNSelLevel(), ip->GetTime());
			chamfer = 0;
			om = m;
		} else {
			ip->RedrawViews (ip->GetTime(),REDRAW_END);
			switch (mpEditPoly->GetMNSelLevel()) {
			case MNM_SL_VERTEX:
				mpEditPoly->getParamBlock()->SetValue (ep_vertex_chamfer, ip->GetTime(), chamfer);
				break;
			case MNM_SL_EDGE:
				mpEditPoly->getParamBlock()->SetValue (ep_edge_chamfer, ip->GetTime(), chamfer);
				break;
			}
			mpEditPoly->EpfnEndChamfer (true, ip->GetTime());
		}
		break;

	case MOUSE_MOVE:
		p0 = vpt->MapScreenToView(om,float(-200));
		p1 = vpt->MapScreenToView(m,float(-200));
		chamfer = Length(p1-p0);
		mpEditPoly->EpfnDragChamfer (chamfer, ip->GetTime());
		switch (mpEditPoly->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpEditPoly->getParamBlock()->SetValue (ep_vertex_chamfer, ip->GetTime(), chamfer);
			break;
		case MNM_SL_EDGE:
			mpEditPoly->getParamBlock()->SetValue (ep_edge_chamfer, ip->GetTime(), chamfer);
			break;
		}
		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		break;

	case MOUSE_ABORT:
		mpEditPoly->EpfnEndChamfer (false, ip->GetTime());			
		ip->RedrawViews (ip->GetTime(),REDRAW_END);
		break;
	}

	if (vpt) ip->ReleaseViewport(vpt);
	return TRUE;
}

HCURSOR ChamferSelectionProcessor::GetTransformCursor() { 
	static HCURSOR hECur = NULL;
	static HCURSOR hVCur = NULL;
	if (mpEditPoly->GetSelLevel() == EP_SL_VERTEX) {
		if ( !hVCur ) hVCur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_VCHAMFERCUR));
		return hVCur;
	}
	if ( !hECur ) hECur = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_ECHAMFERCUR));
	return hECur;
}

void ChamferCMode::EnterMode() {
	mpEditPoly->SuspendContraints (true);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(hGeom,IDC_CHAMFER));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void ChamferCMode::ExitMode() {
	mpEditPoly->SuspendContraints (false);
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton(GetDlgItem(hGeom,IDC_CHAMFER));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

// --- Slice: not really a command mode, just looks like it.--------- //

void EditPolyObject::EnterSliceMode () {
	sliceMode = TRUE;
	HWND hGeom = GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_SLICE));
		but->Enable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_RESET_PLANE));
		but->Enable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
		but->SetCheck (TRUE);
		ReleaseICustButton (but);
	}

	if (!sliceInitialized) EpResetSlicePlane ();

	EpPreviewBegin (epop_slice);

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::ExitSliceMode () {
	sliceMode = FALSE;

	if (EpPreviewOn()) EpPreviewCancel ();
	HWND hGeom = GetDlgHandle (ep_geom);
	if (hGeom) {
		ICustButton *but = GetICustButton (GetDlgItem(hGeom,IDC_SLICE));
		but->Disable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_RESET_PLANE));
		but->Disable ();
		ReleaseICustButton (but);
		but = GetICustButton (GetDlgItem (hGeom, IDC_SLICEPLANE));
		but->SetCheck (FALSE);
		ReleaseICustButton (but);
	}
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::GetSlicePlaneBoundingBox (Box3 & box, Matrix3 *tm) {
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	rotMatrix.SetTrans (sliceCenter);
	if (tm) rotMatrix *= (*tm);
	box += Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
	box += Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
	box += Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
}

void EditPolyObject::DisplaySlicePlane (GraphicsWindow *gw) {
	// Draw rectangle representing slice plane.
	Point3 rp[5];
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	rotMatrix.SetTrans (sliceCenter);
	rp[0] = Point3(-sliceSize,-sliceSize,0.0f)*rotMatrix;
	rp[1] = Point3(-sliceSize,sliceSize,0.0f)*rotMatrix;
	rp[2] = Point3(sliceSize,sliceSize,0.0f)*rotMatrix;
	rp[3] = Point3(sliceSize,-sliceSize,0.0f)*rotMatrix;
	gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	gw->polyline (4, rp, NULL, NULL, TRUE, NULL);
}

// -- Cut proc/mode -------------------------------------

HitRecord *CutProc::HitTestVerts (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEditPoly->SetHitLevelOverride (SUBHIT_MNVERTS);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// Ok, if we haven't yet started a cut, and we don't want to know the hit location,
	// we're done:
	if ((startIndex<0) && !completeAnalysis) return hitLog.First();

	// Ok, now we need to find the closest eligible point:
	// First we grab our node's transform from the first hit:
	mObj2world = hitLog.First()->nodeRef->GetObjectTM(ip->GetTime());

	int bestHitIndex = -1;
	int bestHitDistance;
	Point3 bestHitPoint;
	HitRecord *ret = NULL;

	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if ((bestHitIndex>-1) && (bestHitDistance < hitRec->distance)) continue;

		if (startIndex >= 0)
		{
			// Check that our hit doesn't touch starting component:
			switch (startLevel) {
			case MNM_SL_VERTEX:
				if (startIndex == int(hitRec->hitInfo)) continue;
				break;
			case MNM_SL_EDGE:
				if (mpEPoly->GetMeshPtr()->e[startIndex].v1 == hitRec->hitInfo) continue;
				if (mpEPoly->GetMeshPtr()->e[startIndex].v2 == hitRec->hitInfo) continue;
				break;
			case MNM_SL_FACE:
				// Any face is suitable.
				break;
			}
		}

		Point3 & p = mpEPoly->GetMeshPtr()->P(hitRec->hitInfo);

		if (bestHitIndex>-1)
		{
			Point3 diff = p - bestHitPoint;
			diff = mObj2world.VectorTransform(diff);
			if (DotProd (diff, mLastHitDirection) > 0) continue;	// this vertex clearly further away.
		}

		bestHitIndex = hitRec->hitInfo;
		bestHitDistance = hitRec->distance;
		bestHitPoint = p;
		ret = hitRec;
	}

	if (bestHitIndex>-1)
	{
		mLastHitLevel = MNM_SL_VERTEX;
		mLastHitIndex = bestHitIndex;
		mLastHitPoint = bestHitPoint;
	}

	return ret;
}

HitRecord *CutProc::HitTestEdges (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEditPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	int numHits = vpt->NumSubObjHits();
	if (numHits == 0) return NULL;

	HitLog& hitLog = vpt->GetSubObjHitList();

	// Ok, if we haven't yet started a cut, and we don't want to know the actual hit location,
	// we're done:
	if ((startIndex<0) && !completeAnalysis) return hitLog.First ();

	// Ok, now we want the edge with the closest hit point.
	// So we form a list of all the edges and their hit points.

	// First we grab our node's transform from the first hit:
	mObj2world = hitLog.First()->nodeRef->GetObjectTM(ip->GetTime());

	int bestHitIndex = -1;
	int bestHitDistance;
	Point3 bestHitPoint;
	HitRecord *ret = NULL;

	for (HitRecord *hitRec=hitLog.First(); hitRec != NULL; hitRec=hitRec->Next()) {
		// Always want the closest hit pixelwise:
		if ((bestHitIndex>-1) && (bestHitDistance < hitRec->distance)) continue;

		if (startIndex>-1) 	{
			// Check that component doesn't touch starting component:
			switch (startLevel) {
			case MNM_SL_VERTEX:
				if (mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v1 == startIndex) continue;
				if (mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v2 == startIndex) continue;
				break;
			case MNM_SL_EDGE:
				if (startIndex == int(hitRec->hitInfo)) continue;
				break;
				// (any face is acceptable)
			}
		}

		// Get endpoints in world space:
		Point3 Aobj = mpEPoly->GetMeshPtr()->P(mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v1);
		Point3 Bobj = mpEPoly->GetMeshPtr()->P(mpEPoly->GetMeshPtr()->e[hitRec->hitInfo].v2);
		Point3 A = mObj2world * Aobj;
		Point3 B = mObj2world * Bobj;

		// Find proportion of our nominal hit point along this edge:
		Point3 Xdir = B-A;
		Xdir -= DotProd(Xdir, mLastHitDirection)*mLastHitDirection;	// make orthogonal to mLastHitDirection.
		float prop = DotProd (Xdir, mLastHitPoint-A) / LengthSquared (Xdir);
		if (prop<.0001f) prop=0;
		if (prop>.9999f) prop=1;

		// Find hit point in object space:
		Point3 p = Aobj*(1.0f-prop) + Bobj*prop;

		if (bestHitIndex>-1) {
			Point3 diff = p - bestHitPoint;
			diff = mObj2world.VectorTransform(diff);
			if (DotProd (diff, mLastHitDirection)>0) continue; // this edge clearly further away.
		}
		bestHitIndex = hitRec->hitInfo;
		bestHitDistance = hitRec->distance;
		bestHitPoint = p;
		ret = hitRec;
	}

	if (bestHitIndex>-1)
	{
		mLastHitLevel = MNM_SL_EDGE;
		mLastHitIndex = bestHitIndex;
		mLastHitPoint = bestHitPoint;
	}

	return ret;
}

HitRecord *CutProc::HitTestFaces (IPoint2 &m, ViewExp *vpt, bool completeAnalysis) {
	vpt->ClearSubObjHitList();

	mpEditPoly->SetHitLevelOverride (SUBHIT_MNFACES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();

	// We don't bother to choose the view-direction-closest face,
	// since generally that's the only one we expect to get.
	HitRecord *ret = hitLog.ClosestHit();
	if (!completeAnalysis) return ret;

	mLastHitIndex = ret->hitInfo;
	mObj2world = ret->nodeRef->GetObjectTM (ip->GetTime ());
	
	// Get the average normal for the face, thus the plane, and intersect.
	MNMesh & mm = *(mpEPoly->GetMeshPtr());
	Point3 planeNormal = mm.GetFaceNormal (mLastHitIndex, TRUE);
	planeNormal = Normalize (mObj2world.VectorTransform (planeNormal));
	float planeOffset=0.0f;
	for (int i=0; i<mm.f[mLastHitIndex].deg; i++)
		planeOffset += DotProd (planeNormal, mObj2world*mm.v[mm.f[mLastHitIndex].vtx[i]].p);
	planeOffset = planeOffset/float(mm.f[mLastHitIndex].deg);

	// Now we intersect the mLastHitPoint + mLastHitDirection*t line with the
	// DotProd (planeNormal, X) = planeOffset plane.
	float rayPlane = DotProd (mLastHitDirection, planeNormal);
	float firstPointOffset = planeOffset - DotProd (planeNormal, mLastHitPoint);
	if (fabsf(rayPlane) > EPSILON) {
		float amount = firstPointOffset / rayPlane;
		mLastHitPoint += amount*mLastHitDirection;
	}

	// Put hitPoint in object space:
	Matrix3 world2obj = Inverse (mObj2world);
	mLastHitPoint = world2obj * mLastHitPoint;
	mLastHitLevel = MNM_SL_FACE;

	return ret;
}

HitRecord *CutProc::HitTestAll (IPoint2 & m, ViewExp *vpt, int flags, bool completeAnalysis) {
	HitRecord *hr = NULL;
	mLastHitLevel = MNM_SL_OBJECT;	// no hit.

	mpEditPoly->ForceIgnoreBackfacing (true);
	if (!(flags & (MOUSE_SHIFT|MOUSE_CTRL))) {
		hr = HitTestVerts (m,vpt, completeAnalysis);
		if (hr) mLastHitLevel = MNM_SL_VERTEX;
	}
	if (!hr && !(flags & MOUSE_SHIFT)) {
		hr = HitTestEdges (m, vpt, completeAnalysis);
		if (hr) mLastHitLevel = MNM_SL_EDGE;
	}
	if (!hr) {
		hr = HitTestFaces (m, vpt, completeAnalysis);
		if (hr) mLastHitLevel = MNM_SL_FACE;
	}
	if (!hr)
	{
		// We still need to transform the hit point into object space.
		Matrix3 world2obj = Inverse (mObj2world);
		mLastHitPoint = world2obj * mLastHitPoint;
	}
	mpEditPoly->ForceIgnoreBackfacing (false);

	return hr;
}

void CutProc::DrawCutter (HWND hWnd) {
	if (startIndex<0) return;
	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);	
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));
	MoveToEx(hdc,mMouse1.x,mMouse1.y,NULL);
	LineTo(hdc,mMouse2.x,mMouse2.y);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

int CutProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt;
	IPoint2 betterM;
	Matrix3 world2obj;
	Ray r;
	static HCURSOR hCutVertCur = NULL, hCutEdgeCur=NULL, hCutFaceCur = NULL;

	if (!hCutVertCur) {
		hCutVertCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_VERT_CUR));
		hCutEdgeCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_EDGE_CUR));
		hCutFaceCur = LoadCursor (hInstance, MAKEINTRESOURCE(IDC_CUT_FACE_CUR));
	}

	switch (msg) {
	case MOUSE_ABORT:
		startIndex = -1;
		// Turn off Cut preview:
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend ()) {
			mpEditPoly->EpPreviewSetSuspend (false);
			ip->PopPrompt ();
		}
		return FALSE;

	case MOUSE_PROPCLICK:
		ip->PopCommandMode ();
		// Turn off Cut preview:
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend ()) {
			mpEditPoly->EpPreviewSetSuspend (false);
			ip->PopPrompt ();
		}
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		if (point==1) break;		// Want click-move-click behavior.
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);

		// Get a worldspace hit point and view direction:
		mLastHitPoint = vpt->SnapPoint (m, betterM, NULL);
		mLastHitPoint = vpt->MapCPToWorld (mLastHitPoint);
		vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
		mLastHitDirection = Normalize (r.dir);	// in world space

		// Hit test all subobject levels:
		HitTestAll (m, vpt, flags, true);
		ip->ReleaseViewport (vpt);

		if (mLastHitLevel == MNM_SL_OBJECT) return true;

		// Get hit directon in object space:
		world2obj = Inverse (mObj2world);
		mLastHitDirection = Normalize (VectorTransform (world2obj, mLastHitDirection));

		if (startIndex<0) {
			startIndex = mLastHitIndex;
			startLevel = mLastHitLevel;
			mpEPoly->getParamBlock()->SetValue (ep_cut_start_level, TimeValue(0), startLevel);
			mpEPoly->getParamBlock()->SetValue (ep_cut_start_index, TimeValue(0), startIndex);
			mpEPoly->getParamBlock()->SetValue (ep_cut_start_coords, TimeValue(0), mLastHitPoint);
			mpEPoly->getParamBlock()->SetValue (ep_cut_end_coords, TimeValue(0), mLastHitPoint);
			mpEPoly->getParamBlock()->SetValue (ep_cut_normal, TimeValue(0), -mLastHitDirection);
			if (ip->GetSnapState()) {
				// Must suspend "fully interactive" while snapping in Cut mode.
				mpEditPoly->EpPreviewSetSuspend (true);
				ip->PushPrompt (GetString (IDS_CUT_SNAP_PREVIEW_WARNING));
			}
			mpEditPoly->EpPreviewBegin (epop_cut);
			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawCutter (hwnd);
			break;
		}

		// Erase last cutter line:
		DrawCutter (hwnd);

		// Do the cut:
		mpEPoly->getParamBlock()->SetValue (ep_cut_end_coords, TimeValue(0), mLastHitPoint);
		mpEPoly->getParamBlock()->SetValue (ep_cut_normal, TimeValue(0), -mLastHitDirection);
		mpEditPoly->EpPreviewAccept ();

		mpEPoly->getParamBlock()->GetValue (ep_cut_start_index, TimeValue(0), mLastHitIndex, FOREVER);
		mpEPoly->getParamBlock()->GetValue (ep_cut_start_level, TimeValue(0), mLastHitLevel, FOREVER);
		if ((startLevel == mLastHitLevel) && (startIndex == mLastHitIndex)) {
			// no change - cut was unable to finish.
			startIndex = -1;
			if (mpEditPoly->EpPreviewGetSuspend ()) {
				mpEditPoly->EpPreviewSetSuspend (false);
				ip->PopPrompt ();
			}
			return false;	// end cut mode.
		}

		// Otherwise, start on next cut.
		startIndex = mLastHitIndex;
		startLevel = MNM_SL_VERTEX;
		mpEPoly->getParamBlock()->SetValue (ep_cut_start_coords, TimeValue(0),
			mpEPoly->GetMeshPtr()->P(startIndex));
		mpEditPoly->EpPreviewBegin (epop_cut);
		mMouse1 = betterM;
		mMouse2 = betterM;
		DrawCutter (hwnd);
		return true;

	case MOUSE_MOVE:
		vpt = ip->GetViewport (hwnd);

		// Show snap preview:
		vpt->SnapPreview (m, m, NULL, SNAP_FORCE_3D_RESULT);
		ip->RedrawViews (ip->GetTime(), REDRAW_INTERACTIVE);	// hey - why are we doing this?

		// Find 3D point in object space:
		mLastHitPoint = vpt->SnapPoint (m, betterM, NULL);	// Snap point goes in "betterM".
		mLastHitPoint = vpt->MapCPToWorld (mLastHitPoint);
		vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
		mLastHitDirection = Normalize (r.dir);	// in world space

		HitTestAll (betterM, vpt, flags, true);

		if (startIndex>-1) {
			// Erase last dotted line
			DrawCutter (hwnd);

			// Set the cut preview to use the point we just hit on:
			mpEditPoly->EpPreviewSetDragging (true);
			mpEPoly->getParamBlock()->SetValue (ep_cut_end_coords, TimeValue(0), mLastHitPoint);
			mpEditPoly->EpPreviewSetDragging (false);

			// Draw new dotted line
			mMouse2 = betterM;
			DrawCutter (hwnd);
		}

		// Set cursor based on best subobject match:
		switch (mLastHitLevel) {
		case MNM_SL_VERTEX: SetCursor (hCutVertCur); break;
		case MNM_SL_EDGE: SetCursor (hCutEdgeCur); break;
		case MNM_SL_FACE: SetCursor (hCutFaceCur); break;
		default: SetCursor (LoadCursor (NULL, IDC_ARROW));
		}

		ip->ReleaseViewport (vpt);
		// STEVE: why does this need preview protection when the same call in QuickSliceProc doesn't
		int fullyInteractive;
		mpEPoly->getParamBlock()->GetValue (ep_interactive_full, TimeValue(0), fullyInteractive, FOREVER);
		if (fullyInteractive) ip->RedrawViews (ip->GetTime());
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		vpt->SnapPreview(m,m,NULL);
		HitTestAll (m, vpt, flags, false);
		ip->ReleaseViewport (vpt);

		// Set cursor based on best subobject match:
		switch (mLastHitLevel) {
		case MNM_SL_VERTEX: SetCursor (hCutVertCur); break;
		case MNM_SL_EDGE: SetCursor (hCutEdgeCur); break;
		case MNM_SL_FACE: SetCursor (hCutFaceCur); break;
		default: SetCursor (LoadCursor (NULL, IDC_ARROW));
		}
		break;
	}

	return TRUE;
}

void CutCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CUT));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.startIndex = -1;
}

void CutCMode::ExitMode() {
	// Lets make double-extra-sure that Cut's suspension of the preview system doesn't leak out...
	// (This line is actually necessary in the case where the user uses a shortcut key to jump-start
	// another command mode.)
	if (mpEditPoly->EpPreviewGetSuspend ()) {
		mpEditPoly->EpPreviewSetSuspend (false);
		if (mpEditPoly->ip) mpEditPoly->ip->PopPrompt ();
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_CUT));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//------- QuickSlice proc/mode------------------------

void QuickSliceProc::DrawSlicer (HWND hWnd) {
	if (!mSlicing) return;
	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);	
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));
	MoveToEx(hdc,mMouse1.x,mMouse1.y,NULL);
	LineTo(hdc,mMouse2.x,mMouse2.y);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

// Given two points and a view direction (in obj space), find slice plane:
void QuickSliceProc::ComputeSliceParams (bool center) {
	Point3 Xdir = mLocal2 - mLocal1;
	Xdir = Xdir - mZDir * DotProd (Xdir, mZDir);	// make orthogonal to view
	float len = Length(Xdir);
	Point3 planeNormal;
	if (len<.001f) {
		// Xdir is insignificant, but mZDir is valid.
		// Choose an arbitrary Xdir that'll work:
		Xdir = Point3(1,0,0);
		Xdir = Xdir - mZDir * DotProd (Xdir, mZDir);
		len = Length(Xdir);
	}

	if (len<.001f) {
		// straight X-direction didn't work; therefore straight Y-direction should:
		Xdir = Point3(0,1,0);
		Xdir = Xdir - mZDir * DotProd (Xdir, mZDir);
		len = Length (Xdir);
	}

	// Now guaranteed to have some valid Xdir:
	Xdir = Xdir/len;
	planeNormal = mZDir^Xdir;
	float size;
	if (!mpEditPoly->EpGetSliceInitialized()) mpEPoly->EpResetSlicePlane ();	// initializes size if needed.
	Point3 foo1, foo2;
	mpEPoly->EpGetSlicePlane (foo1, foo2, &size);	// Don't want to modify size.
	if (center) {
		Box3 bbox;
		mpEPoly->GetMeshPtr()->BBox(bbox, false);
		Point3 planeCtr = bbox.Center();
		planeCtr = planeCtr - DotProd (planeNormal, planeCtr) * planeNormal;
		planeCtr += DotProd (planeNormal, mLocal1) * planeNormal;
		mpEPoly->EpSetSlicePlane (planeNormal, planeCtr, size);
	} else {
		mpEPoly->EpSetSlicePlane (planeNormal, (mLocal1+mLocal2)*.5f, size);
	}
}

static HCURSOR hCurQuickSlice = NULL;

int QuickSliceProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	if (!hCurQuickSlice)
		hCurQuickSlice = LoadCursor(hInstance,MAKEINTRESOURCE(IDC_QUICKSLICE_CURSOR));

	ViewExp *vpt;
	IPoint2 betterM;
	Point3 snapPoint;
	Ray r;

	switch (msg) {
	case MOUSE_ABORT:
		if (mSlicing) DrawSlicer (hwnd);	// Erase last slice line.
		mSlicing = false;
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend()) {
			mpInterface->PopPrompt();
			mpEditPoly->EpPreviewSetSuspend (false);
		}
		return FALSE;

	case MOUSE_PROPCLICK:
		if (mSlicing) DrawSlicer (hwnd);	// Erase last slice line.
		mSlicing = false;
		mpEditPoly->EpPreviewCancel ();
		if (mpEditPoly->EpPreviewGetSuspend()) {
			mpInterface->PopPrompt();
			mpEditPoly->EpPreviewSetSuspend (false);
		}
		mpInterface->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		if (point==1) break;	// don't want to get a notification on first mouse-click release.
		mpInterface->SetActiveViewport (hwnd);

		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = mpInterface->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, betterM, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		if (!mSlicing) {
			// We need to get node of this polyobject, and obtain world->obj transform.
			ModContextList mcList;
			INodeTab nodes;
			mpInterface->GetModContexts(mcList,nodes);
			Matrix3 otm = nodes[0]->GetObjectTM(mpInterface->GetTime());
			nodes.DisposeTemporary();
			mWorld2obj = Inverse (otm);

			// first point:
			mLocal1 = mWorld2obj * snapPoint;
			mLocal2 = mLocal1;

			// We'll also need to find our Z direction:
			vpt->MapScreenToWorldRay ((float)betterM.x, (float)betterM.y, r);
			mZDir = Normalize (mWorld2obj.VectorTransform (r.dir));

			// Set the slice plane position:
			ComputeSliceParams (false);

			if (mpInterface->GetSnapState()) {
				// Must suspend "fully interactive" while snapping in Quickslice mode.
				mpEditPoly->EpPreviewSetSuspend (true);
				mpInterface->PushPrompt (GetString (IDS_QUICKSLICE_SNAP_PREVIEW_WARNING));
			}

			// Start the slice preview mode:
			mpEditPoly->EpPreviewBegin (epop_slice);

			mSlicing = true;
			mMouse1 = betterM;
			mMouse2 = betterM;
			DrawSlicer (hwnd);
		} else {
			DrawSlicer (hwnd);	// Erase last slice line.

			// second point:
			mLocal2 = mWorld2obj * snapPoint;
			ComputeSliceParams (true);
			mSlicing = false;	// do before PreviewAccept to make sure display is correct.
			mpEditPoly->EpPreviewAccept ();
			if (mpEditPoly->EpPreviewGetSuspend()) {
				mpInterface->PopPrompt();
				mpEditPoly->EpPreviewSetSuspend (false);
			}
		}
		mpInterface->ReleaseViewport (vpt);
		return true;

	case MOUSE_MOVE:
		if (!mSlicing) break;	// Nothing to do if we haven't clicked first point yet

		SetCursor (hCurQuickSlice);
		DrawSlicer (hwnd);	// Erase last slice line.

		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = mpInterface->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, mMouse2, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		mpInterface->ReleaseViewport (vpt);

		// Find object-space equivalent for mLocal2:
		mLocal2 = mWorld2obj * snapPoint;

		mpEditPoly->EpPreviewSetDragging (true);
		ComputeSliceParams ();
		mpEditPoly->EpPreviewSetDragging (false);

		mpInterface->RedrawViews (mpInterface->GetTime());

		DrawSlicer (hwnd);	// Draw this slice line.
		break;

	case MOUSE_FREEMOVE:
		SetCursor (hCurQuickSlice);
		vpt = mpInterface->GetViewport (hwnd);
		// Show any snapping:
		vpt->SnapPreview(m,m,NULL);//, SNAP_SEL_OBJS_ONLY);
		mpInterface->ReleaseViewport (vpt);
		break;
	}

	return TRUE;
}

void QuickSliceCMode::EnterMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_QUICKSLICE));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
	proc.mSlicing = false;
}

void QuickSliceCMode::ExitMode() {
	// Lets make double-extra-sure that QuickSlice's suspension of the preview system doesn't leak out...
	// (This line is actually necessary in the case where the user uses a shortcut key to jump-start
	// another command mode.)
	if (mpEditPoly->EpPreviewGetSuspend()) {
		if (mpEditPoly->ip) mpEditPoly->ip->PopPrompt();
		mpEditPoly->EpPreviewSetSuspend (false);
	}

	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_QUICKSLICE));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//------- LiftFromEdge proc/mode------------------------

HitRecord *LiftFromEdgeProc::HitTestEdges (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	EditPolyObject *pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: make go away someday.
	pEditPoly->SetHitLevelOverride (SUBHIT_MNEDGES);
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	pEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	return hitLog.ClosestHit();	// (may be NULL.)
}

// Mouse interaction:
// - user clicks on hinge edge
// - user drags angle.

int LiftFromEdgeProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m ) {
	ViewExp *vpt;
	HitRecord *hr;
	Point3 snapPoint;
	IPoint2 diff;
	float angle;
	EditPolyObject *pEditPoly;

	switch (msg) {
	case MOUSE_ABORT:
		pEditPoly = (EditPolyObject *) mpEPoly;
		pEditPoly->EpPreviewCancel ();
		return FALSE;

	case MOUSE_PROPCLICK:
		pEditPoly = (EditPolyObject *) mpEPoly;
		pEditPoly->EpPreviewCancel ();
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);
		if (!edgeFound) {
			hr = HitTestEdges (m, vpt);
			if (!hr) return true;
			pEditPoly = (EditPolyObject *) mpEPoly;
			edgeFound = true;
			mpEPoly->getParamBlock()->SetValue (ep_lift_edge, ip->GetTime(), int(hr->hitInfo));
			mpEPoly->getParamBlock()->SetValue (ep_lift_angle, ip->GetTime(), 0.0f);	// prevent "flash"
			pEditPoly->EpPreviewBegin (epop_lift_from_edge);
			firstClick = m;
		} else {
			IPoint2 diff = m - firstClick;
			// (this arbirtrarily scales each pixel to one degree.)
			float angle = diff.y * PI / 180.0f;
			mpEPoly->getParamBlock()->SetValue (ep_lift_angle, ip->GetTime(), angle);
			pEditPoly = (EditPolyObject *) mpEPoly;
			pEditPoly->EpPreviewAccept ();
			ip->RedrawViews (ip->GetTime());
			edgeFound = false;
			return false;
		}
		if (vpt) ip->ReleaseViewport (vpt);
		return true;

	case MOUSE_MOVE:
		// Find where we hit, in world space, on the construction plane or snap location:
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		if (!edgeFound) {
			// Just set cursor depending on presence of edge:
			if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
			else SetCursor(LoadCursor(NULL,IDC_ARROW));
			if (vpt) ip->ReleaseViewport(vpt);
			break;
		}
		ip->ReleaseViewport (vpt);
		diff = m - firstClick;
		// (this arbirtrarily scales each pixel to one degree.)
		angle = diff.y * PI / 180.0f;

		pEditPoly = (EditPolyObject *) mpEPoly;	// STEVE: Temporarily necessary.
		pEditPoly->EpPreviewSetDragging (true);
		mpEPoly->getParamBlock()->SetValue (ep_lift_angle, ip->GetTime(), angle);
		pEditPoly->EpPreviewSetDragging (false);

		// Even if we're not fully interactive, we need to update the dotted line display:
		pEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
		ip->RedrawViews (ip->GetTime());
		break;

	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport (hwnd);
		snapPoint = vpt->SnapPoint (m, m, NULL);
		snapPoint = vpt->MapCPToWorld (snapPoint);

		if (!edgeFound) {
			// Just set cursor depending on presence of edge:
			if (HitTestEdges(m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		ip->ReleaseViewport (vpt);
		break;
	}

	return TRUE;
}

void LiftFromEdgeCMode::EnterMode() {
	proc.Reset();
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_LIFT_FROM_EDG));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void LiftFromEdgeCMode::ExitMode() {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom,IDC_LIFT_FROM_EDG));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

//-------------------------------------------------------

bool PickLiftEdgeProc::EdgePick (int edge, float prop) {
	mpEPoly->getParamBlock()->SetValue (ep_lift_edge, ip->GetTime(), edge);
	ip->RedrawViews (ip->GetTime());
	return false;	// false = exit mode when done; true = stay in mode.
}

void PickLiftEdgeCMode::EnterMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_LIFT_PICK_EDGE));
	but->SetCheck(true);
	ReleaseICustButton(but);
}

void PickLiftEdgeCMode::ExitMode () {
	HWND hSettings = mpEditPoly->GetDlgHandle (ep_settings);
	if (!hSettings) return;
	ICustButton *but = GetICustButton (GetDlgItem (hSettings,IDC_LIFT_PICK_EDGE));
	but->SetCheck(false);
	ReleaseICustButton(but);
}

//-------------------------------------------------------

void WeldCMode::EnterMode () {
	mproc.Reset();
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	but->SetCheck(TRUE);
	ReleaseICustButton(but);
}

void WeldCMode::ExitMode () {
	HWND hGeom = mpEditPoly->GetDlgHandle (ep_geom_vertex);
	if (!hGeom) return;
	ICustButton *but = GetICustButton (GetDlgItem (hGeom, IDC_TARGET_WELD));
	but->SetCheck(FALSE);
	ReleaseICustButton(but);
}

void WeldMouseProc::DrawWeldLine (HWND hWnd,IPoint2 &m) {
	if (targ1<0) return;
	HDC hdc = GetDC(hWnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);	
	SelectObject(hdc,CreatePen(PS_DOT, 0, ComputeViewportXORDrawColor()));
	MoveToEx(hdc,m1.x,m1.y,NULL);
	LineTo(hdc,m.x,m.y);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));
	ReleaseDC(hWnd, hdc);
}

bool WeldMouseProc::CanWeldVertices (int v1, int v2) {
	MNMesh & mm = mpEditPoly->GetMesh ();
	// If vertices v1 and v2 share an edge, then take a collapse type approach;
	// Otherwise, weld them if they're suitable (open verts, etc.)
	int i;
	for (i=0; i<mm.vedg[v1].Count(); i++) {
		if (mm.e[mm.vedg[v1][i]].OtherVert(v1) == v2) break;
	}
	if (i<mm.vedg[v1].Count()) {
		int ee = mm.vedg[v1][i];
		if (mm.e[ee].f1 == mm.e[ee].f2) return false;
		// there are other conditions, but they're complex....
	} else {
		if (mm.vedg[v1].Count() && (mm.vedg[v1].Count() <= mm.vfac[v1].Count())) return false;
		for (i=0; i<mm.vedg[v1].Count(); i++) {
			for (int j=0; j<mm.vedg[v2].Count(); j++) {
				int e1 = mm.vedg[v1][i];
				int e2 = mm.vedg[v2][j];
				int ov = mm.e[e1].OtherVert (v1);
				if (ov != mm.e[e2].OtherVert (v2)) continue;
				// Edges from these vertices connect to the same other vert.
				// That means we have additional conditions:
				if (((mm.e[e1].v1 == ov) && (mm.e[e2].v1 == ov)) ||
					((mm.e[e1].v2 == ov) && (mm.e[e2].v2 == ov))) return false;	// edges trace same direction, so cannot be merged.
				if (mm.e[e1].f2 > -1) return false;
				if (mm.e[e2].f2 > -1) return false;
				if (mm.vedg[ov].Count() <= mm.vfac[ov].Count()) return false;
			}
		}
	}
	return true;
}

bool WeldMouseProc::CanWeldEdges (int e1, int e2) {
	MNMesh & mm = mpEditPoly->GetMesh ();
	if (mm.e[e1].f2 > -1) return false;
	if (mm.e[e2].f2 > -1) return false;
	if (mm.e[e1].f1 == mm.e[e2].f1) return false;
	if (mm.e[e1].v1 == mm.e[e2].v1) return false;
	if (mm.e[e1].v2 == mm.e[e2].v2) return false;
	return true;
}

HitRecord *WeldMouseProc::HitTest (IPoint2 &m, ViewExp *vpt) {
	vpt->ClearSubObjHitList();
	// Use the default pixel value - no one wanted the old weld_pixels spinner.
	if (mpEditPoly->GetMNSelLevel()==MNM_SL_EDGE) {
		mpEditPoly->SetHitLevelOverride (SUBHIT_EDGES|SUBHIT_OPENONLY);
	}
	ip->SubObHitTest (ip->GetTime(), HITTYPE_POINT, 0, 0, &m, vpt);
	mpEditPoly->ClearHitLevelOverride ();
	if (!vpt->NumSubObjHits()) return NULL;
	HitLog& hitLog = vpt->GetSubObjHitList();
	HitRecord *hr = hitLog.ClosestHit();
	if (targ1>-1) {
		if (targ1 == hr->hitInfo) return NULL;
		if (mpEditPoly->GetMNSelLevel() == MNM_SL_EDGE) {
			if (!CanWeldEdges (targ1, hr->hitInfo)) return NULL;
		} else {
			if (!CanWeldVertices (targ1, hr->hitInfo)) return NULL;
		}
	}
	return hr;
}

int WeldMouseProc::proc (HWND hwnd, int msg, int point, int flags, IPoint2 m) {
	ViewExp *vpt = NULL;
	int res = TRUE;
	HitRecord *hr;

	switch (msg) {
	case MOUSE_ABORT:
		// Erase last weld line:
		if (targ1>-1) DrawWeldLine (hwnd, oldm2);
		targ1 = -1;
		return FALSE;

	case MOUSE_PROPCLICK:
		// Erase last weld line:
		if (targ1>-1) DrawWeldLine (hwnd, oldm2);
		ip->PopCommandMode ();
		return FALSE;

	case MOUSE_DBLCLICK:
		return false;

	case MOUSE_POINT:
		ip->SetActiveViewport (hwnd);
		vpt = ip->GetViewport (hwnd);
		hr = HitTest (m, vpt);
		if (!hr) break;
		if (targ1 < 0) {
			targ1 = hr->hitInfo;
			m1 = m;
			DrawWeldLine (hwnd, m);
			oldm2 = m;
			break;
		}

		// Otherwise, we're completing the weld.
		// Erase the last weld-line:
		DrawWeldLine (hwnd, oldm2);

		// Do the weld:
		theHold.Begin();
		bool ret;
		ret = false;
		int stringID;
		if (mpEditPoly->GetSelLevel() == EP_SL_VERTEX) {
			Point3 destination = mpEditPoly->mm.P(hr->hitInfo);
			ret = mpEditPoly->EpfnWeldVerts (targ1, hr->hitInfo, destination)?true:false;
			stringID = IDS_WELD_VERTS;
		} else if (mpEditPoly->GetMNSelLevel() == MNM_SL_EDGE) {
			ret = mpEditPoly->EpfnWeldEdges (targ1, hr->hitInfo)?true:false;
			stringID = IDS_WELD_EDGES;
		}
		if (ret) {
			theHold.Accept (GetString(stringID));
			mpEditPoly->RefreshScreen ();
		} else {
			theHold.Cancel ();
		}
		targ1 = -1;
		ip->ReleaseViewport(vpt);
		return false;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		vpt = ip->GetViewport(hwnd);
		if (targ1 > -1) {
			DrawWeldLine (hwnd, oldm2);
			oldm2 = m;
		}
		if (HitTest (m,vpt)) SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else SetCursor (LoadCursor (NULL, IDC_ARROW));
		ip->RedrawViews (ip->GetTime());
		if (targ1 > -1) DrawWeldLine (hwnd, m);
		break;
	}
	
	if (vpt) ip->ReleaseViewport(vpt);
	return true;	
}

//-------------------------------------------------------

void EditTriCMode::EnterMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_FS_EDIT_TRI));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
	mpEditPoly->SetDisplayLevelOverride (MNDISP_DIAGONALS);
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void EditTriCMode::ExitMode() {
	HWND hSurf = mpEditPoly->GetDlgHandle (ep_geom_face);
	if (hSurf) {
		ICustButton *but = GetICustButton (GetDlgItem (hSurf, IDC_FS_EDIT_TRI));
		but->SetCheck (FALSE);
		ReleaseICustButton(but);
	}
	mpEditPoly->ClearDisplayLevelOverride();
	mpEditPoly->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	mpEditPoly->ip->RedrawViews(mpEditPoly->ip->GetTime());
}

void EditTriProc::VertConnect () {
	DbgAssert (v1>=0);
	DbgAssert (v2>=0);
	Tab<int> v1fac = mpEPoly->GetMeshPtr()->vfac[v1];
	int i, j, ff, v1pos, v2pos=-1;
	for (i=0; i<v1fac.Count(); i++) {
		MNFace & mf = mpEPoly->GetMeshPtr()->f[v1fac[i]];
		for (j=0; j<mf.deg; j++) {
			if (mf.vtx[j] == v2) v2pos = j;
			if (mf.vtx[j] == v1) v1pos = j;
		}
		if (v2pos<0) continue;
		ff = v1fac[i];
		break;
	}

	if (v2pos<0) return;

	theHold.Begin();
	mpEPoly->EpfnSetDiagonal (ff, v1pos, v2pos);
	theHold.Accept (GetString (IDS_EDIT_TRIANGULATION));
	mpEPoly->RefreshScreen ();
}
