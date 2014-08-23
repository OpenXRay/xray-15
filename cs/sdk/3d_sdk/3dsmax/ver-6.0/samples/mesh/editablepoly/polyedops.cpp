 /**********************************************************************  
 *<
	FILE: PolyEdOps.cpp

	DESCRIPTION: Editable Polygon Mesh Object

	CREATED BY: Steve Anderson

	HISTORY: created Nov. 9, 1999

 *>	Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"
#include "MeshDLib.h"
#include "macrorec.h"
#include "decomp.h"
#include "spline3d.h"
#include "splshape.h"
#include "shape.h"

//---------------------------------------------------------
// EditPolyObject implementations of EPoly methods:

void EditPolyObject::LocalDataChanged (DWORD parts) {
	bool sel = (parts & PART_SELECT) ? TRUE : FALSE;
	bool topo = (parts & PART_TOPO) ? TRUE : FALSE;
	bool geom = (parts & PART_GEOM) ? TRUE : FALSE;
	bool vertCol = (parts & PART_VERTCOLOR) ? true : false;
	InvalidateTempData (parts);
	if (topo||sel||vertCol) InvalidateSurfaceUI ();
	if (topo) {
		SynchContArray(mm.numv);
		// We need to make sure our named selection sets' sizes match our sizes:
		selSet[0].SetSize (mm.numv);
		selSet[1].SetSize (mm.nume);
		selSet[2].SetSize (mm.numf);
	}
	if (geom||topo) {
		mm.InvalidateGeomCache ();
		mm.freeRVerts ();
	} else if (vertCol) mm.InvalidateHardwareMesh ();

	if (sel) {
		InvalidateNumberSelected ();
		UpdateNamedSelDropDown ();
	}
	subdivValid.SetEmpty ();
	if (killRefmsg.DistributeRefmsg())
	NotifyDependents(FOREVER, parts, REFMSG_CHANGE);
}

void EditPolyObject::EpfnForceSubdivision () {
	SetFlag (EPOLY_FORCE);
	subdivValid.SetEmpty ();

	macroRecorder->FunctionCall(_T("$.EditablePoly.forceSubdivision"), 0, 0);
	macroRecorder->EmitScript ();

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void EditPolyObject::RefreshScreen () {
#ifdef _DEBUG
	if (!mm.CheckAllData ()) {
		mm.MNDebugPrint ();
		DbgAssert (0);
	}
#elif defined _SHOW_DEBUG_ASSERTIONS_IN_RELEASE
	if (!mm.CheckAllData ()) {
		DbgAssert (0);
	}
#endif
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));

	if (ip) ip->RedrawViews (ip->GetTime ());
}

int EditPolyObject::EpfnPropagateComponentFlags (int slTo, DWORD flTo, int slFrom, DWORD flFrom, bool ampersand, bool set, bool undoable) {
	if (undoable && theHold.Holding())
		theHold.Put (new ComponentFlagRestore(this, slTo));
	int res = mm.PropegateComponentFlags (slTo, flTo, slFrom, flFrom, ampersand, set);
	DWORD flag =0;
	if (flTo & MN_SEL) flag |= PART_SELECT;
	if (flTo & (MN_HIDDEN | MN_EDGE_INVIS)) flag |= PART_DISPLAY;
	if (flTo & MN_HIDDEN) flag |= PART_GEOM;	// Bounding box change -sca
	if (res && flag) LocalDataChanged (flag);
	return res;
}

// Luna task 748A - necessary for preview mode.
void EditPolyObject::ApplyMeshOp (MNMesh & mesh, MNTempData *temp, int operation) {
	int msl = meshSelLevel[selLevel];
	switch (operation) {
	case epop_hide:
		EpfnHideMesh (mesh, msl, MN_SEL);
		break;
	case epop_unhide:
		EpfnUnhideAllMesh (mesh, msl);
		break;
	case epop_meshsmooth:
		EpfnMeshSmoothMesh (mesh, msl, MN_SEL);
		break;
	case epop_tessellate:
		EpfnTessellateMesh (mesh, msl, MN_SEL);
		break;
	case epop_extrude:
		EpfnExtrudeMesh (mesh, temp, msl, MN_SEL);
		break;
	case epop_bevel:
		EpfnBevelMesh (mesh, temp, msl, MN_SEL);
		break;
	case epop_inset:
		EpfnInsetMesh (mesh, temp, msl, MN_SEL);
		break;
	case epop_outline:
		if (msl != MNM_SL_FACE) break;
		EpfnOutlineMesh (mesh, temp, MN_SEL);
		break;
	case epop_chamfer:
		EpfnChamferMesh (mesh, temp, msl, MN_SEL);
		break;

	case epop_slice:
		EpfnSliceMesh (mesh, msl, MN_SEL);
		break;

	case epop_weld_sel:
		switch (selLevel) {
		case EP_SL_VERTEX:
			EpfnWeldMeshVerts (mesh, MN_SEL);
			break;
		case EP_SL_EDGE:
			EpfnWeldMeshEdges (mesh, MN_SEL);
			break;
		}
		break;

	case epop_cut:
		EpfnCutMesh (mesh);
		break;

	case epop_connect_edges:	// Luna task 748Q
		if (msl != MNM_SL_EDGE) break;
		EpfnConnectMeshEdges (mesh, MN_SEL);
		break;

	case epop_connect_vertices:
		if (msl != MNM_SL_VERTEX) break;
		EpfnConnectMeshVertices (mesh, MN_SEL);
		break;

	case epop_extrude_along_spline:	// Luna task 748T
		EpfnExtrudeAlongSplineMesh (mesh, temp, MN_SEL);
		break;

	case epop_lift_from_edge:	// Luna task 748P
		EpfnLiftFromEdgeMesh (mesh, temp, MN_SEL);
		break;
	}
}

// No macrorecording in following --
// All the little Epfn methods do macrorecording themselves.
void EditPolyObject::EpActionButtonOp (int opcode) {
	TSTR name;
	int i, index;
	bool delIsoVerts, ret;
	bool createCurveSmooth;
	Point3 planeNormal, planeCenter;
	int msl = meshSelLevel[selLevel];
	TimeValue t = ip ? ip->GetTime() : TimeValue (0);

	switch (opcode) {
	case epop_hide:
		theHold.Begin ();
		if (EpfnHide (msl, MN_SEL)) RefreshScreen ();
		theHold.Accept (GetString (IDS_HIDE_SELECTED));
		break;

	case epop_hide_unsel:
		theHold.Begin ();
		EpfnPropagateComponentFlags (msl, MN_USER, MNM_SL_OBJECT, MN_MESH_FILLED_IN);
		EpfnPropagateComponentFlags (msl, MN_USER, msl, MN_SEL, false, false);
		if (EpfnHide (msl, MN_USER)) RefreshScreen ();
		theHold.Accept (GetString (IDS_HIDE_UNSELECTED));
		break;

	case epop_unhide:
		theHold.Begin ();
		if (EpfnUnhideAll (msl)) RefreshScreen ();
		theHold.Accept (GetString (IDS_UNHIDE_ALL));
		break;

	case epop_ns_copy:
		index = SelectNamedSet();
		NamedSelectionCopy (index);
		break;

	case epop_ns_paste:
		EpfnNamedSelectionPaste (true);
		break;

	case epop_toggle_shaded_faces:
		theHold.Begin ();
		EpfnToggleShadedFaces ();
		theHold.Accept (GetString (IDS_TOGGLE_SHADED_FACES));
		RefreshScreen ();
		break;

	case epop_cap:
		if (selLevel != EP_SL_BORDER) break;
		theHold.Begin ();
		if (EpfnCapHoles ()) RefreshScreen ();
		theHold.Accept (GetString (IDS_CAP));
		break;

	case epop_remove:
		theHold.Begin ();
		if (EpfnRemove (msl, MN_SEL)) {
			RefreshScreen ();
		}
		switch (msl) {
		case MNM_SL_VERTEX:
			theHold.Accept (GetString (IDS_REMOVE_VERTICES));
			break;
		default:
			theHold.Accept (GetString (IDS_REMOVE_EDGES));
			break;
		}
		break;

	case epop_delete:
		if (msl == MNM_SL_OBJECT) break;
		if (msl == MNM_SL_FACE) {
			delIsoVerts = mm.PropegateComponentFlags (MNM_SL_VERTEX, 0, MNM_SL_FACE, MN_SEL, TRUE) ? true : false;
			if (ip && delIsoVerts) {
				TSTR str1 = GetString(IDS_DELETE_ISOLATED);
				TSTR str2 = GetString(IDS_DELETE_FACES);
				if (IDYES!=MessageBox (ip->GetMAXHWnd(), str1,str2, MB_ICONQUESTION|MB_YESNO)) {
					delIsoVerts = FALSE;
				}
			}
		}
		theHold.Begin ();
		if (EpfnDelete (msl, MN_SEL, delIsoVerts)) RefreshScreen ();

		switch (msl) {
		case MNM_SL_VERTEX:
			theHold.Accept (GetString (IDS_DELETE_VERTICES));
			break;
		case MNM_SL_EDGE:
			theHold.Accept (GetString (IDS_DELETE_EDGES));
			break;
		case MNM_SL_FACE:
			theHold.Accept (GetString (IDS_DELETE_FACES));
			break;
		}
		break;

	case epop_attach_list:
		if (ip) {
			AttachHitByName proc(this);
			ip->DoHitByNameDialog(&proc);
		}
		break;

	case epop_detach:
		if (!editObj || !ip) break;
		if (mm.VertexTempSel().NumberSet() == 0) break;	// defect 261157 (4)
		bool elem, asClone;
		if (GetDetachObjectName (ip, name, elem, asClone)) {
			if (elem) EpfnDetachToElement (msl, MN_SEL, asClone);
			else {
				ModContextList mcList;
				INodeTab nodes;
				ip->GetModContexts(mcList,nodes);
				EpfnDetachToObject (name, msl, MN_SEL, asClone, nodes[0], ip->GetTime());
				nodes.DisposeTemporary ();
			}
			RefreshScreen ();
		}
		break;

	case epop_split:
		if (selLevel != EP_SL_EDGE) break;
		theHold.Begin ();
		if (EpfnSplitEdges (MN_SEL)) RefreshScreen ();
		theHold.Accept (GetString (IDS_SPLIT_EDGES));
		break;

	case epop_break:
		if (selLevel != EP_SL_VERTEX) break;
		theHold.Begin ();
		if (EpfnBreakVerts ()) {
			RefreshScreen ();
		}
		theHold.Accept (GetString (IDS_BREAK_VERTS));
		break;

	case epop_collapse:
		if ((selLevel > EP_SL_OBJECT) && (selLevel<EP_SL_ELEMENT)) {
			theHold.Begin();
			if (EpfnCollapse (msl, MN_SEL)) {
				RefreshScreen ();
			}
			theHold.Accept(GetString(IDS_COLLAPSE));
		}
		break;

	case epop_reset_plane:
		if (sliceMode && selLevel) {
			theHold.Begin ();
			EpResetSlicePlane ();
			theHold.Accept (GetString (IDS_RESET_SLICE_PLANE));
		}
		break;

	case epop_slice:
		// Luna task 748J
		// No need to restrict Slice ability to slice plane "mode".
		// (This allows for QuickSlice to work.)
		theHold.Begin ();
		EpGetSlicePlane (planeNormal, planeCenter);
		if (EpfnSlice (planeNormal, planeCenter, msl==MNM_SL_FACE, MN_SEL)) {
			RefreshScreen ();
		}
		theHold.Accept (GetString (IDS_SLICE));
		break;

	case epop_weld_sel:
		int actionID;
		ret = false;
		theHold.Begin ();
		switch (meshSelLevel[selLevel]) {
		case MNM_SL_VERTEX:
			ret = EpfnWeldFlaggedVerts (MN_SEL)?true:false;
			actionID = IDS_WELD_VERTS;
			break;
		case MNM_SL_EDGE:
			ret = EpfnWeldFlaggedEdges (MN_SEL)?true:false;
			actionID = IDS_WELD_EDGES;
			break;
		}
		if (ret) RefreshScreen ();
		theHold.Accept (GetString (actionID));
		break;

	case epop_create_shape:
		if (meshSelLevel[selLevel] != MNM_SL_EDGE) break;
		for (i=0; i<mm.nume; i++) if (mm.e[i].GetFlag (MN_SEL)) break;
		if (i>= mm.nume) {
			TSTR buf1 = GetString(IDS_CREATE_SHAPE_FROM_EDGES);
			TSTR buf2 = GetString(IDS_NOEDGESSELECTED);
			MessageBox (ip->GetMAXHWnd(),buf2,buf1,MB_ICONEXCLAMATION|MB_OK);
			break;
		}
		if (GetCreateShapeName (ip, name, createCurveSmooth)) {
			theHold.Begin ();
			ModContextList mcList;
			INodeTab nodes;
			ip->GetModContexts(mcList,nodes);
			if (EpfnCreateShape (name, createCurveSmooth, nodes[0], MN_SEL)) {
				RefreshScreen ();
			}
			theHold.Accept (GetString(IDS_CREATE_SHAPE_FROM_EDGES));
			nodes.DisposeTemporary ();
		}
		break;

	case epop_make_planar:
		theHold.Begin ();
		EpfnMakePlanar (msl, MN_SEL, t);
		theHold.Accept(GetString(IDS_MAKE_PLANAR));
		break;

	case epop_align_view:
		theHold.Begin ();
		EpfnAlignToView (msl, MN_SEL);
		theHold.Accept(GetString(IDS_ALIGN_TO_VIEW));
		break;

	case epop_align_grid:
		theHold.Begin ();
		EpfnAlignToGrid (msl, MN_SEL);
		theHold.Accept(GetString(IDS_ALIGN_TO_GRID));
		break;

	case epop_remove_iso_verts:
		theHold.Begin ();
		if (EpfnDeleteIsoVerts ()) RefreshScreen ();
		theHold.Accept (GetString (IDS_REMOVE_ISOLATED_VERTICES));
		break;

	case epop_remove_iso_map_verts:
		theHold.Begin ();
		if (EpfnDeleteIsoMapVerts ()) RefreshScreen ();
		theHold.Accept (GetString (IDS_REMOVE_UNUSED_MAP_VERTICES));
		break;

	case epop_meshsmooth:
		theHold.Begin();
		if (EpfnMeshSmooth (msl, MN_SEL)) {
			RefreshScreen ();
		}
		theHold.Accept(GetString(IDS_MESHSMOOTH));
		break;

	case epop_tessellate:
		theHold.Begin ();
		if (EpfnTessellate (msl, MN_SEL)) {
			RefreshScreen ();
		}
		theHold.Accept(GetString(IDS_TESSELLATE));
		break;

	case epop_update:
		EpfnForceSubdivision ();
		break;

	case epop_selby_vc:
		BOOL add, sub;
		add = GetKeyState(VK_CONTROL)<0;
		sub = GetKeyState(VK_MENU)<0;
		int selByIllum;
		pblock->GetValue (ep_vert_color_selby, t, selByIllum, FOREVER);
		EpfnSelectVertByColor (add, sub, selByIllum ? -1 : 0, t);
		break;

	case epop_retriangulate:
		if (meshSelLevel[selLevel] != MNM_SL_FACE) break;
		theHold.Begin ();
		if (EpfnRetriangulate (MN_SEL)) {
			RefreshScreen ();
		}
		theHold.Accept (GetString (IDS_RETRIANGULATE));
		break;

	case epop_flip_normals:
		// Luna task 748I - flip normals to work on faces as well as elements
		theHold.Begin ();
		if (EpfnFlipNormals (MN_SEL)) {
			RefreshScreen ();
		}
		theHold.Accept (GetString (IDS_FLIP_NORMALS));
		break;

	case epop_selby_matid:
		MtlID material;
		bool clear;
		if (GetSelectByMaterialParams (this, material, clear)) {
			theHold.Begin();
			EpfnSelectByMat (material-1, clear, t);
			theHold.Accept(GetString(IDS_SELECT_BY_MATID));
			RefreshScreen ();
		}
		break;

	case epop_selby_smg:
		DWORD smBits, usedBits;
		GetSmoothingGroups (0, &usedBits);
		if (GetSelectBySmoothParams (ip, usedBits, smBits, clear)) {
			theHold.Begin ();
			EpfnSelectBySmoothGroup (smBits, clear, t);
			theHold.Accept(GetString(IDS_SEL_BY_SMGROUP));
			RefreshScreen ();
		}
		break;

	case epop_autosmooth:
		theHold.Begin ();
		EpfnAutoSmooth (t);
		theHold.Accept (GetString (IDS_AUTOSMOOTH));
		RefreshScreen ();
		break;

	case epop_clear_smg:
		DWORD bitmask;
		bitmask=0;
		bitmask = ~bitmask;
		SetSmoothBits (0, bitmask, MN_SEL);
		break;

	// Luna task 748A - new "op" for preview
	case epop_extrude:
		theHold.Begin ();
		TopoChangeRestore *tchange;
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
		if (EpfnExtrudeMesh (mm, TempData(), msl, MN_SEL)) {
			tchange->After();
			theHold.Put (tchange);
			macroRecorder->FunctionCall(_T("$.EditablePoly.buttonOp"), 1, 0,
				mr_name, _T("Extrude"));
			macroRecorder->EmitScript ();
			LocalDataChanged (PART_ALL);
			RefreshScreen ();
		} else {
			delete tchange;
		}
		theHold.Accept (GetString (IDS_EXTRUDE));
		break;

	// Luna task 748A - new "op" for preview
	case epop_bevel:
		theHold.Begin ();
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
		if (EpfnBevelMesh (mm, TempData(), msl, MN_SEL)) {
			tchange->After();
			theHold.Put (tchange);
			macroRecorder->FunctionCall(_T("$.EditablePoly.buttonOp"), 1, 0,
				mr_name, _T("Bevel"));
			macroRecorder->EmitScript ();
			LocalDataChanged (PART_ALL);
			RefreshScreen ();
		} else {
			delete tchange;
		}
		theHold.Accept (GetString (IDS_BEVEL));
		break;

	// Luna task 748R
	case epop_inset:
		theHold.Begin ();
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
		if (EpfnInsetMesh (mm, TempData(), msl, MN_SEL)) {
			tchange->After();
			theHold.Put (tchange);
			macroRecorder->FunctionCall(_T("$.EditablePoly.buttonOp"), 1, 0,
				mr_name, _T("Inset"));
			macroRecorder->EmitScript ();
			LocalDataChanged (PART_ALL);
			RefreshScreen ();
		} else {
			delete tchange;
		}
		theHold.Accept (GetString (IDS_INSET));
		break;

	case epop_outline:
		if (msl != MNM_SL_FACE) break;
		theHold.Begin ();
		if (EpfnOutline (MN_SEL)) RefreshScreen ();
		theHold.Accept (GetString (IDS_OUTLINE));
		break;

	// Luna task 748A - new "op" for preview
	case epop_chamfer:
		theHold.Begin ();
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
		if (EpfnChamferMesh (mm, TempData(), msl, MN_SEL)) {
			tchange->After();
			theHold.Put (tchange);
			macroRecorder->FunctionCall(_T("$.EditablePoly.buttonOp"), 1, 0,
				mr_name, _T("Chamfer"));
			macroRecorder->EmitScript ();
			LocalDataChanged (PART_ALL);
			RefreshScreen ();
		} else {
			delete tchange;
		}
		theHold.Accept (GetString (IDS_CHAMFER));
		break;

	// Luna task 748D - new cut "op"
	case epop_cut:
		theHold.Begin ();
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
		i = EpfnCutMesh (mm);
		if (i > -1) {
			// Note: following are covered by undo operation
			pblock->SetValue (ep_cut_start_index, TimeValue(0), i);
			pblock->SetValue (ep_cut_start_level, TimeValue(0), MNM_SL_VERTEX);
			pblock->SetValue (ep_cut_start_coords, TimeValue(0), mm.P(i));
		}
		tchange->After();
		theHold.Put (tchange);
		theHold.Accept (GetString (IDS_CUT));
		LocalDataChanged (PART_ALL);
		RefreshScreen ();
		break;

	// Luna task 748P 
	case epop_lift_from_edge:
		theHold.Begin ();
		if (EpfnLiftFromEdge (MN_SEL)) RefreshScreen ();
		theHold.Accept (GetString (IDS_LIFT_FROM_EDGE));
		break;

	// Luna task 748T
	case epop_extrude_along_spline:
		theHold.Begin ();
		if (EpfnExtrudeAlongSpline (MN_SEL)) RefreshScreen();
		theHold.Accept (GetString (IDS_EXTRUDE_ALONG_SPLINE));
		break;

	// Luna task 748Q
	case epop_connect_edges:
		if (msl != MNM_SL_EDGE) break;
		theHold.Begin ();
		if (EpfnConnectEdges (MN_SEL)) RefreshScreen();
		theHold.Accept (GetString (IDS_CONNECT_EDGES));
		break;

	case epop_connect_vertices:
		if (msl != MNM_SL_VERTEX) break;
		theHold.Begin ();
		if (EpfnConnectVertices (MN_SEL)) RefreshScreen();
		theHold.Accept (GetString (IDS_CONNECT_VERTICES));
		break;

	// Luna task 748V
	case epop_sel_grow:
		theHold.Begin ();
		EpfnGrowSelection (msl);
		theHold.Accept (GetString (IDS_SELECT));
		RefreshScreen ();
		break;

	// Luna task 748V
	case epop_sel_shrink:
		theHold.Begin ();
		EpfnShrinkSelection (msl);
		theHold.Accept (GetString (IDS_SELECT));
		RefreshScreen ();
		break;

	// Luna task 748U
	case epop_select_ring:
		theHold.Begin ();
		EpfnSelectEdgeRing ();
		theHold.Accept (GetString (IDS_SELECT));
		RefreshScreen ();
		break;

	// Luna task 748U
	case epop_select_loop:
		theHold.Begin ();
		EpfnSelectEdgeLoop ();
		theHold.Accept (GetString (IDS_SELECT));
		LocalDataChanged ();
		RefreshScreen ();
		break;
	}

	// Luna task 748BB
	EpSetLastOperation (opcode);
}

// Luna task 748BB
void EditPolyObject::EpfnRepeatLastOperation () {
	EpActionButtonOp (mLastOperation);
}

void EditPolyObject::MoveSelection(int msl, TimeValue t, Matrix3& partm,
								   Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	Transform (msl, t, partm, tmAxis, localOrigin, TransMatrix(val), 0);
}

void EditPolyObject::Move (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) {
	Transform(meshSelLevel[selLevel], t, partm, tmAxis, localOrigin, TransMatrix(val), 0);
	macroRecorder->FunctionCall(_T("move"), 2, 0, mr_meshsel, meshSelLevel[selLevel], this, mr_point3, &val);  // JBW : macrorecorder
}

void EditPolyObject::RotateSelection(int msl, TimeValue t, Matrix3& partm,
									 Matrix3& tmAxis, Quat& val, BOOL localOrigin) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	Matrix3 mat;
	val.MakeMatrix(mat);
	Transform(msl, t, partm, tmAxis, localOrigin, mat, 1);
}

void EditPolyObject::Rotate (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Quat& val, BOOL localOrigin) {
	Matrix3 mat;
	val.MakeMatrix(mat);
	Transform(meshSelLevel[selLevel], t, partm, tmAxis, localOrigin, mat, 1);
	// SCA: disabled following because there is no rotate command on subobject levels.
	//macroRecorder->FunctionCall(_T("rotate"), 2, 0, mr_meshsel, meshSelLevel[selLevel], this, mr_quat, &val);  // JBW : macrorecorder
}

void EditPolyObject::ScaleSelection(int msl, TimeValue t, Matrix3& partm, 
									Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	Transform (msl, t, partm, tmAxis, localOrigin, ScaleMatrix(val), 2);	
}

void EditPolyObject::Scale (TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) {
	Transform (meshSelLevel[selLevel], t, partm, tmAxis, localOrigin, ScaleMatrix(val), 2);	
	// SCA: disabled following because there is no scale command on subobject levels.
	//macroRecorder->FunctionCall(_T("scale"), 2, 0, mr_meshsel, meshSelLevel[selLevel], this, mr_point3, &val);  // JBW : macrorecorder
}

void EditPolyObject::Transform (int sl, TimeValue t, Matrix3& partm, Matrix3 tmAxis, 
		BOOL localOrigin, Matrix3 xfrm, int type) {
	if (!ip) return;
	if (sl == MNM_SL_OBJECT) return;

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

	// Get our node transform
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	// nodeTm is no longer needed due to changes below (-sca, 1/29/01)
	//Matrix3 nodeTm = nodes[0]->GetObjectTM(t);

	// Get axis type:
	int numAxis = ip->GetNumAxis();

	// Get soft selection parameters:
	int softSel, edgeIts=1, useEdgeDist=FALSE, affectBack=FALSE;
	float falloff, pinch, bubble;
	Interval frvr = FOREVER;
	pblock->GetValue (ep_ss_use, t, softSel, frvr);
	if (softSel) {
		pblock->GetValue (ep_ss_edist_use, t, useEdgeDist, frvr);
		if (useEdgeDist) pblock->GetValue (ep_ss_edist, t, edgeIts, frvr);
		pblock->GetValue (ep_ss_affect_back, t, affectBack, frvr);
		pblock->GetValue (ep_ss_falloff, t, falloff, frvr);
		pblock->GetValue (ep_ss_pinch, t, pinch, frvr);
		pblock->GetValue (ep_ss_bubble, t, bubble, frvr);
	}

	// Special case for vertices: Only individual axis when moving in local space
	if ((sl==MNM_SL_VERTEX) && (numAxis==NUMAXIS_INDIVIDUAL)) {
		if (ip->GetRefCoordSys()!=COORDS_LOCAL || 
			ip->GetCommandMode()->ID()!=CID_SUBOBJMOVE) {
			numAxis = NUMAXIS_ALL;
		}
	}

	// Selected vertices - either directly or indirectly through selected faces or edges.
	BitArray sel = GetMesh().VertexTempSel ();
	if (!sel.NumberSet()) {
		nodes.DisposeTemporary ();
		return;
	}

	int i, nv = GetMesh().numv;
	if (!nv) return;
	Tab<Point3> delta;
	delta.SetCount (nv);
	for (i=0; i<nv; i++) delta[i] = Point3(0,0,0);

	// Compute the transforms
	if (numAxis==NUMAXIS_INDIVIDUAL && sl != MNM_SL_VERTEX) {
		// Do each cluster one at a time

		// If we have soft selections from multiple clusters,
		// we need to add up the vectors and divide by the total soft selection,
		// to get the right direction for movement,
		// but we also need to add up the squares of the soft selections and divide by the total soft selection,
		// essentially getting a weighted sum of the selection weights themselves,
		// to get the right "scale" of movement.

		// (Note that this works out to ordinary soft selections in the case of a single cluster.)

		int count;
		Tab<int> *vclust = NULL;
		if (sl == MNM_SL_EDGE) count = TempData()->EdgeClusters()->count;
		else count = TempData()->FaceClusters()->count;
		vclust = TempData()->VertexClusters(sl);
		float *clustDist=NULL, *sss=NULL, *ssss=NULL;
		Tab<float> softSelSum, softSelSquareSum;
		Matrix3 tm, itm;
		if (softSel) {
			softSelSum.SetCount(nv);
			sss = softSelSum.Addr(0);
			softSelSquareSum.SetCount (nv);
			ssss = softSelSquareSum.Addr(0);
			for (i=0; i<nv; i++) {
				sss[i] = 0.0f;
				ssss[i] = 0.0f;
			}
		}
		for (int j=0; j<count; j++) {
			tmAxis = ip->GetTransformAxis (nodes[0], j);
			tm  = partm * Inverse(tmAxis);
			itm = Inverse(tm);
			tm *= xfrm;
			if (softSel) clustDist = TempData()->ClusterDist(sl, MN_SEL, j, useEdgeDist, edgeIts)->Addr(0);
			for (i=0; i<nv; i++) {
				if (sel[i]) {
					if ((*vclust)[i]!=j) continue;
					Point3 & old = GetMesh().v[i].p;
					delta[i] = (tm*old)*itm - old;
				} else {
					if (!softSel) continue;
					if (clustDist[i] < 0) continue;
					if (clustDist[i] > falloff) continue;
					float af = AffectRegionFunction (clustDist[i], falloff, pinch, bubble);
					sss[i] += fabsf(af);
					ssss[i] += af*af;
					Point3 & old = GetMesh().v[i].p;
					delta[i] = ((tm*old)*itm - old) * af;
				}
			}
		}
		if (softSel) {
			for (i=0; i<nv; i++) {
				if (sel[i]) continue;
				if (sss[i] == 0) continue;
				delta[i] *= (ssss[i] / (sss[i]*sss[i]));
			}
		}
	} else {
		Matrix3 tm  = partm * Inverse(tmAxis);
		Matrix3 itm = Inverse(tm);
		tm *= xfrm;
		// Normals are no longer needed; we use the MNMesh::GetVertexSpace method now.  -sca, 1/29/01
		//Point3 *vn = (numAxis == NUMAXIS_INDIVIDUAL) ? TempData()->VertexNormals()->Addr(0) : 0;
		float *vsw = softSel ? TempData()->VSWeight(useEdgeDist, edgeIts, !affectBack,
				falloff, pinch, bubble)->Addr(0) : NULL;
		for (i=0; i<nv; i++) {
			if (!sel[i]) {
				if (!vsw || !vsw[i]) continue;
			}
			Point3 & old = mm.v[i].p;
			if (numAxis == NUMAXIS_INDIVIDUAL) {
				// Changed for 4.patch by Steve A, 1/25/00,
				// Previous code was obselete, dates back to before proper
				// SubObjectTransforms were established.
				//MatrixFromNormal (vn[i], tm);
				//tm  = partm * Inverse(tm*nodeTm);
				Matrix3 axis2obj;
				mm.GetVertexSpace (i, axis2obj);
				axis2obj.SetTrans(old);
				Matrix3 obj2axis = Inverse (axis2obj);
				delta[i] = ((old*obj2axis)*xfrm)*axis2obj - old;
			} else {
				delta[i] = itm*(tm*old)-old;
			}
			if (!sel[i]) delta[i] *= vsw[i];
		}
	}

	nodes.DisposeTemporary ();
	DragMove (delta, this, t);
}

void EditPolyObject::TransformStart(TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(TRUE);
	DragMoveInit();
	EpPreviewSetDragging (true);
}

void EditPolyObject::TransformHoldingFinish (TimeValue t) {
	if (!ip) return;
	DragMoveAccept (t);
}

void EditPolyObject::TransformFinish(TimeValue t) {
	if (!ip) return;
	ip->LockAxisTripods(FALSE);
	EpPreviewSetDragging (false);
	EpPreviewInvalidate ();	// Luna task 748A
}

void EditPolyObject::TransformCancel (TimeValue t) {
	EpPreviewInvalidate ();	// Luna task 748A
	DragMoveClear ();
	if (!ip) return;
	ip->LockAxisTripods(FALSE);
}

// Luna task 748A - Hide preview - not actually used as preview, just
// separated out from EpfnHide.
bool EditPolyObject::EpfnHideMesh (MNMesh & mesh, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	int i;
	bool ret = false;
	switch (msl) {
	case MNM_SL_VERTEX:
		ret = mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_HIDDEN, MNM_SL_VERTEX, flag) ? true : false;
		if (ret) mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_HIDDEN, false, false);
		break;

	case MNM_SL_FACE:
		// Hide the verts on all-hidable-faces first:
		if (mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_HIDDEN, MNM_SL_FACE, flag, TRUE)) ret = true;
		if (ret) mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_SEL, MNM_SL_VERTEX, MN_HIDDEN, false, false);
		for (i=0; i<mesh.numf; i++) {
			if (mesh.f[i].GetFlag (MN_DEAD)) continue;
			if (mesh.f[i].GetFlag (flag)) {
				mesh.f[i].SetFlag (MN_HIDDEN);
				mesh.f[i].ClearFlag (MN_SEL);
				ret = true;
			}
		}
		break;
	}
	return ret;
}

bool EditPolyObject::EpfnHide (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	ComponentFlagRestore *vertFlags=NULL, *faceFlags=NULL;
	switch (msl) {
	case MNM_SL_VERTEX:
		if (theHold.Holding()) vertFlags = new ComponentFlagRestore(this, MNM_SL_VERTEX);
		break;
	case MNM_SL_FACE:
		if (theHold.Holding()) {
			vertFlags = new ComponentFlagRestore(this, MNM_SL_VERTEX);
			faceFlags = new ComponentFlagRestore(this, MNM_SL_FACE);
		}
		break;
	}
	bool ret = EpfnHideMesh (mm, msl, flag);
	if (ret) {
		if (vertFlags) theHold.Put (vertFlags);
		if (faceFlags) theHold.Put (faceFlags);
		macroRecorder->FunctionCall(_T("$.EditablePoly.Hide"), 1, 0,
			mr_int, msl);
		macroRecorder->EmitScript ();
		LocalDataChanged (PART_DISPLAY|PART_SELECT|PART_GEOM);	// sca GEOM added because of bounding box change.
	} else {
		if (vertFlags) delete vertFlags;
		if (faceFlags) delete faceFlags;
	}

	return ret;
}

// Luna task 748A - Hide preview - not actually used as preview, just
// separated out from EpfnHide.
bool EditPolyObject::EpfnUnhideAllMesh (MNMesh & mesh, int msl) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	int i;
	bool ret = false;
	switch (msl) {
	case MNM_SL_VERTEX:
		for (i=0; i<mesh.numv; i++) {
			if (mesh.v[i].GetFlag (MN_DEAD)) continue;
			if (mesh.v[i].GetFlag (MN_HIDDEN)) {
				mesh.v[i].ClearFlag (MN_HIDDEN);
				ret = true;
			}
		}
		break;

	case MNM_SL_FACE:
		// We don't clear vertex hide flags on most vertices here, only those on previously hidden faces.
		// (to be consistent with EMesh behavior).
		for (i=0; i<mesh.numf; i++) {
			if (mesh.f[i].GetFlag (MN_DEAD)) continue;
			if (mesh.f[i].GetFlag (MN_HIDDEN)) {
				for (int j=0; j<mesh.f[i].deg; j++) mesh.v[mesh.f[i].vtx[j]].ClearFlag (MN_HIDDEN);
				ret = true;
			}
			mesh.f[i].ClearFlag (MN_HIDDEN);
		}
		break;
	}
	return ret;
}

bool EditPolyObject::EpfnUnhideAll (int msl) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	ComponentFlagRestore *vertFlags=NULL, *faceFlags=NULL;
	switch (msl) {
	case MNM_SL_VERTEX:
		if (theHold.Holding()) vertFlags = new ComponentFlagRestore (this, MNM_SL_VERTEX);
		break;
	case MNM_SL_FACE:
		if (theHold.Holding())  {
			faceFlags = new ComponentFlagRestore (this, MNM_SL_FACE);
			vertFlags = new ComponentFlagRestore (this, MNM_SL_VERTEX);
		}
		break;
	}
	bool ret = EpfnUnhideAllMesh (mm, msl);
	if (ret) {
		if (faceFlags) theHold.Put (faceFlags);
		if (vertFlags) theHold.Put (vertFlags);
		macroRecorder->FunctionCall(_T("$.EditablePoly.unhideAll"), 1, 0, mr_int, msl);
		macroRecorder->EmitScript ();
		LocalDataChanged (PART_DISPLAY|PART_GEOM);	// sca GEOM added because of bbox change.
	} else {
		if (faceFlags) delete faceFlags;
		if (vertFlags) delete vertFlags;
	}
	return ret;
}

bool EditPolyObject::EpfnDelete (int msl, DWORD flag, bool delIsoVerts) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	bool ret=false;
	switch (msl) {
	case MNM_SL_VERTEX:
		ret = DeleteVerts (flag);
		if (ret) CollapseDeadStructs();
		break;
	case MNM_SL_EDGE:
		// Delete all faces using flagged edges, as well as vertices isolated by the action:
		mm.ClearFFlags (MN_USER);
		if (mm.PropegateComponentFlags (MNM_SL_FACE, MN_USER, MNM_SL_EDGE, flag))
			ret = DeleteFaces (MN_USER, true);
		if (ret) CollapseDeadStructs ();
		break;
	case MNM_SL_FACE:
		ret = DeleteFaces (flag, delIsoVerts);
		if (ret) CollapseDeadStructs ();
		break;
	}
	if (ret) {
		if ((msl == MNM_SL_FACE) && !delIsoVerts) {
			macroRecorder->FunctionCall(_T("$.EditablePoly.delete"), 1, 1,
				mr_int, msl, _T("deleteIsoVerts"), mr_bool, false);
		} else {
			macroRecorder->FunctionCall(_T("$.EditablePoly.delete"), 1, 0,
				mr_int, msl);
		}
		macroRecorder->EmitScript ();
	}
	return ret;
}

// Actually joins polygons together.
bool EditPolyObject::EpfnRemove (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	bool ret=false;
	switch (msl) {
	case MNM_SL_VERTEX:
		ret = RemoveVerts (flag);
		break;
	case MNM_SL_EDGE:
		ret = RemoveEdges (flag);
		break;
	}
	if (ret) {
		CollapseDeadStructs();
		macroRecorder->FunctionCall(_T("$.EditablePoly.Remove"), 0, 0);
		macroRecorder->EmitScript ();
	}
	LocalDataChanged (PART_ALL);
	return ret;
}

void EditPolyObject::CloneSelSubComponents(TimeValue t) {
	if (selLevel == EP_SL_OBJECT) return;
	if (!ip) return;

	theHold.Begin();
	theHold.Put (new ComponentFlagRestore (this, meshSelLevel[selLevel]));

	TopoChangeRestore *tChange;

	switch (selLevel) {
	case EP_SL_VERTEX:
		theHold.Put (new CreateOnlyRestore (this));
		mm.CloneVerts ();
		break;
	case EP_SL_EDGE:
	case EP_SL_BORDER:
		tChange = new TopoChangeRestore (this);
		tChange->Before ();
		if (EpfnExtrudeOpenEdges (MN_SEL)) {
			tChange->After ();
			theHold.Put (tChange);
		} else {
			delete tChange;
		}
		break;
	case EP_SL_FACE:
	case EP_SL_ELEMENT:
		theHold.Put (new CreateOnlyRestore (this));
		mm.CloneFaces ();
		break;
	}
	theHold.Accept (IDS_CLONE);

	LocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT);
	RefreshScreen ();
}

void EditPolyObject::AcceptCloneSelSubComponents(TimeValue t) {
	switch (selLevel) {
	case EP_SL_OBJECT:
	case EP_SL_EDGE:
	case EP_SL_BORDER:
		return;
	}
	TSTR name;
	if (!GetCloneObjectName(ip, name)) return;
	if (!ip) return;

	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	EpfnDetachToObject (name, meshSelLevel[selLevel], MN_SEL, false, nodes[0], t);
	nodes.DisposeTemporary ();
	RefreshScreen ();
}

// Edit Geometry ops

int EditPolyObject::EpfnCreateVertex(Point3 pt, bool pt_local, bool select) {
	if (!pt_local) {
		if (!ip) return -1;
		// Put the point in object space:
		ModContextList mcList;
		INodeTab nodes;
		ip->GetModContexts(mcList,nodes);
		pt = pt * Inverse(nodes[0]->GetObjectTM(ip->GetTime()));
		nodes.DisposeTemporary();
	}

	if (theHold.Holding())
		theHold.Put (new CreateOnlyRestore (this));
	int ret = mm.NewVert (pt);
	if (select) {
		mm.v[ret].SetFlag (MN_SEL);
		LocalDataChanged (PART_GEOM|PART_SELECT);
	}
	else 
		LocalDataChanged (PART_GEOM);

	if (select) macroRecorder->FunctionCall(_T("$.EditablePoly.createVertex"), 1, 1,
		mr_point3, &pt, _T("select"), mr_bool, true);
	else macroRecorder->FunctionCall(_T("$.EditablePoly.createVertex"), 1, 0, mr_point3, &pt);
	macroRecorder->EmitScript ();

	return ret;
}

int EditPolyObject::EpfnCreateEdge (int v1, int v2, bool select) {
	DbgAssert (v1>=0);
	DbgAssert (v2>=0);
	Tab<int> v1fac = mm.vfac[v1];
	int i, j, ff, v1pos, v2pos=-1;
	for (i=0; i<v1fac.Count(); i++) {
		MNFace & mf = mm.f[v1fac[i]];
		for (j=0; j<mf.deg; j++) {
			if (mf.vtx[j] == v2) v2pos = j;
			if (mf.vtx[j] == v1) v1pos = j;
		}
		if (v2pos<0) continue;
		ff = v1fac[i];
		break;
	}

	if (v2pos<0) return -1;

	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int nf, ne;
	mm.SeparateFace (ff, v1pos, v2pos, nf, ne, TRUE, select);

	if (nf<0) {	// error condition!
		if (tchange) delete tchange;
		return -1;
	}

	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	if (select) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.createEdge"), 2, 1,
			mr_int, v1+1, mr_int, v2+1, _T("select"), mr_bool, true);
		LocalDataChanged (PART_TOPO|PART_SELECT);
	} else {
		macroRecorder->FunctionCall(_T("$.EditablePoly.createEdge"), 2, 0,
			mr_int, v1+1, mr_int, v2+1);
		LocalDataChanged (PART_TOPO);
	}
	macroRecorder->EmitScript ();
	return ne;
}

int EditPolyObject::EpfnCreateFace(int *v, int deg, bool select) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}

	int ret = mm.CreateFace (deg, v);
	// Luna task 748B - more robust Face Create:
	if (ret<0) {	// Try flipping the face over.
		int *w = new int[deg];
		for (int i=0; i<deg; i++) w[i] = v[deg-1-i];
		ret = mm.CreateFace (deg, w);
	}
	if (ret<0) {	// Really can't make this face.
		if (tchange) delete tchange;
		return -1;
	}
	mm.RetriangulateFace (ret);
	if (select) mm.f[ret].SetFlag (MN_SEL);
	mm.ClearFlag (MN_MESH_NO_BAD_VERTS);

	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	if (macroRecorder->Enabled()) {
		// since we don't have macrorecorder support for arrays,
		// we need to throw together a string and pass it as an mr_name.
		char vertexList[500];
		char vertexID[40];
		int stringPos=0;
		vertexList[stringPos++] = '(';
		for (int i=0; i<deg; i++) {
			sprintf (vertexID, "%d", v[i]+1);
			memcpy (vertexList + stringPos, vertexID, strlen(vertexID));
			stringPos += strlen(vertexID);
			if (i+1<deg) vertexList[stringPos++] = ',';
		}
		vertexList[stringPos++] = ')';
		vertexList[stringPos++] = '\0';

		if (select) {
			macroRecorder->FunctionCall(_T("$.EditablePoly.createFace"), 1, 1,
				mr_name, _T(vertexList), _T("select"), mr_bool, true);
		} else {
			macroRecorder->FunctionCall(_T("$.EditablePoly.createFace"), 1, 0,
				mr_name, _T(vertexList));
		}
		macroRecorder->EmitScript ();
	}

	if (select) {
		LocalDataChanged (PART_TOPO|PART_SELECT);
	} else {
		LocalDataChanged (PART_TOPO);
	}
	return ret;
}

int EditPolyObject::EpfnCreateFace2 (Tab<int> *vertices, bool select) {
	int deg = vertices->Count();
	if (deg<3) return false;
	int *v = vertices->Addr(0);
	return EpfnCreateFace (v, deg, select);
}

bool EditPolyObject::EpfnCapHoles (int msl, DWORD targetFlags) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	MNMeshBorder brdr;
	mm.GetBorder (brdr, msl, targetFlags);
	for (int i=0; i<brdr.Num(); i++) if (brdr.LoopTarg (i)) break;
	if (i>= brdr.Num()) return false;	// Nothing to do.

	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	mm.FillInBorders (&brdr);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.capHoles"), 1, 0, mr_int, msl);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_TOPO|PART_SELECT);
	return true;
}

void EditPolyObject::CollapseDeadVerts () {
	int i, max;

	// Check that we have at least one dead vertex.
	max = mm.numv;
	for (i=0; i<max; i++) if (mm.v[i].GetFlag (MN_DEAD)) break;
	if (i>=max) return;

	if (theHold.Holding ()) theHold.Put (new CollapseDeadVertsRestore (this));

	// Have to delete vertices in the MNMesh, in the Master point controller, and in the actual controller array.

	// Master point controller:
	BitArray pointsToDelete;
	pointsToDelete.SetSize (max);
	pointsToDelete.ClearAll();
	for (; i<max; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) pointsToDelete.Set (i);
	}
	DeletePointConts (pointsToDelete);

	// MNMesh itself:
	mm.CollapseDeadVerts ();

	LocalDataChanged (PART_TOPO|PART_GEOM);
}

void EditPolyObject::CollapseDeadEdges () {
	if (theHold.Holding ()) theHold.Put (new CollapseDeadEdgesRestore (this));
	mm.CollapseDeadEdges ();
	LocalDataChanged (PART_TOPO);
}

void EditPolyObject::CollapseDeadFaces () {
	if (theHold.Holding ()) theHold.Put (new CollapseDeadFacesRestore (this));
	mm.CollapseDeadFaces ();
	LocalDataChanged (PART_TOPO);
}

void EditPolyObject::CollapseDeadStructs () {
	CollapseDeadFaces ();
	CollapseDeadEdges ();
	CollapseDeadVerts ();
	RefreshScreen ();
}

bool EditPolyObject::DeleteVerts (DWORD flag) {
	if (mm.numv<1) return false;
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	DbgAssert (mm.vedg);
	DbgAssert (mm.vfac);

	// First, find out if any of these vertices are present and/or used.
	int i,j;
	bool present=FALSE, used=FALSE;
	for (i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (!mm.v[i].GetFlag (flag)) continue;
		present = TRUE;
		if (mm.vfac[i].Count()) {
			used = TRUE;
			break;
		}
	}
	if (!present) return false;	// Nothing to do.

	if (used) {
		// Delete the relevant faces and edges first.
		mm.ClearFFlags (MN_USER);	// Luna task 748Z: code cleanup - don't use WHATEVER.
		for (; i<mm.numv; i++) {
			if (mm.v[i].GetFlag (MN_DEAD)) continue;
			if (!mm.v[i].GetFlag (flag)) continue;
			int ct;
			if ((ct=mm.vfac[i].Count()) == 0) continue;
			for (j=0; j<ct; j++) mm.f[mm.vfac[i][j]].SetFlag (MN_USER);
		}
		DeleteFaces (MN_USER, FALSE);
	}
	if (theHold.Holding ()) theHold.Put (new ComponentFlagRestore (this, MNM_SL_VERTEX));
	for (i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (!mm.v[i].GetFlag (flag)) continue;
		mm.v[i].SetFlag (MN_DEAD);
	}

	LocalDataChanged (PART_GEOM|PART_TOPO|PART_SELECT);
	return true;
}

bool EditPolyObject::EpfnDeleteIsoVerts () {
	if (mm.numv<1) return false;
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	DbgAssert (mm.vfac);

	DWORD partsChanged = 0x0;
	ComponentFlagRestore *vertFlags = NULL;
	if (theHold.Holding ()) vertFlags = new ComponentFlagRestore (this, MNM_SL_VERTEX);
	for (int i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (mm.vfac[i].Count()) continue;
		mm.v[i].SetFlag (MN_DEAD);
		partsChanged = PART_GEOM|PART_SELECT;
	}

	if (partsChanged) {
		if (vertFlags) theHold.Put (vertFlags);
		macroRecorder->FunctionCall(_T("$.EditablePoly.deleteIsoVerts"), 0, 0);
		macroRecorder->EmitScript ();
		LocalDataChanged (partsChanged);
	} else {
		if (vertFlags) delete vertFlags;
	}
	if (partsChanged&PART_GEOM) CollapseDeadVerts ();
	return partsChanged ? true : false;
}

// Strip out unused map vertices, in all map channels.
bool EditPolyObject::EpfnDeleteIsoMapVerts () {
	DWORD partsChanged=0x0;
	for (int mapChannel = -NUM_HIDDENMAPS; mapChannel < mm.numm; mapChannel++) {
		if (mm.M(mapChannel)->GetFlag(MN_DEAD)) continue;
		int prevNumMapVerts = mm.M(mapChannel)->numv;
		MapChangeRestore *pMCR = NULL;
		if (theHold.Holding()) pMCR = new MapChangeRestore (this, mapChannel);
		mm.EliminateIsoMapVerts (mapChannel);
		if (pMCR) {
			if (pMCR->After()) theHold.Put (pMCR);
			else delete pMCR;
		}
		if (prevNumMapVerts > mm.M(mapChannel)->numv)
			partsChanged |= (mapChannel<1) ? PART_VERTCOLOR : PART_TEXMAP;
	}
	if (!partsChanged) return false;
	
	macroRecorder->FunctionCall(_T("$.EditablePoly.DeleteIsoMapVerts"), 0, 0);
	macroRecorder->EmitScript ();

	LocalDataChanged (partsChanged);
	return true;
}

bool EditPolyObject::RemoveVerts (DWORD flag) {
	if (mm.numv<1) return false;
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	DbgAssert (mm.vedg);
	DbgAssert (mm.vfac);
	for (int i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (mm.v[i].GetFlag (flag)) break;
	}
	if (i>= mm.numv) return false;
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}

	bool retVal = false;
	for (; i<mm.numv; i++) {
		if (!mm.v[i].GetFlag (flag)) continue;
		if (mm.RemoveVertex (i)) retVal=true;
	}

	if (!retVal) {
		if (tchange) delete tchange;
		return false;
	}

	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	return true;
}

bool EditPolyObject::RemoveEdges (DWORD flag) {
	if (mm.numv<1) return false;
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	DbgAssert (mm.vedg);
	DbgAssert (mm.vfac);
	for (int i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (mm.e[i].f2<0) continue;
		if (mm.e[i].f1 == mm.e[i].f2) continue;
		if (mm.e[i].GetFlag (flag)) break;
	}
	if (i>= mm.nume) return false;
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}

	for (; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (mm.e[i].f2<0) continue;
		if (mm.e[i].f1 == mm.e[i].f2) continue;
		if (!mm.e[i].GetFlag (flag)) continue;

		mm.RemoveEdge (i);
	}

	if (theHold.Holding()) {
		tchange->After ();
		theHold.Put (tchange);
	}
	return true;
}

bool EditPolyObject::DeleteFaces (DWORD flag, bool delIsoVerts) {
	if (mm.numv<1) return false;
	DbgAssert (mm.GetFlag (MN_MESH_FILLED_IN));
	DbgAssert (mm.vedg);
	DbgAssert (mm.vfac);
	for (int i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		if (mm.f[i].GetFlag (flag)) break;
	}
	if (i>= mm.numf) return false;
	TopoChangeRestore *tchange=NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before();
	}
	DWORD invalid = PART_TOPO|PART_SELECT;
	flag |= MN_DEAD;	// so we also remove connections to dead faces, if any creep in...
	for (i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if ((mm.e[i].f2>-1) && mm.f[mm.e[i].f2].GetFlag(flag))
			mm.e[i].f2 = -1;
		if (!mm.f[mm.e[i].f1].GetFlag (flag)) continue;
		mm.e[i].f1 = -1;
		if (mm.e[i].f2 > -1) mm.e[i].Invert ();
		else mm.e[i].SetFlag (MN_DEAD);
	}
	for (i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		Tab<int> & vf = mm.vfac[i];
		Tab<int> & ve = mm.vedg[i];
		int j;
		if (j=vf.Count ()) {
			for (j--; j>=0; j--) {
				if (mm.f[vf[j]].GetFlag (flag)) vf.Delete (j,1);
			}
			if (!vf.Count() && delIsoVerts) {
				mm.v[i].SetFlag (MN_DEAD);
				invalid |= PART_GEOM;
			}
		}
		for (j=ve.Count()-1; j>=0; j--) {
			if (mm.e[ve[j]].GetFlag (MN_DEAD)) ve.Delete (j, 1);
		}
	}
	for (i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (flag)) mm.f[i].SetFlag (MN_DEAD);
	}
	// This operation can create 2-boundary vertices:
	mm.ClearFlag (MN_MESH_NO_BAD_VERTS);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	BOOL useMacros = macroRecorder->Enabled ();
	macroRecorder->Disable ();
	EpfnDeleteIsoMapVerts ();
	if (useMacros) macroRecorder->Enable ();

	LocalDataChanged (invalid);
	return true;
}

// Old version preserved for backward compatibility.
void EditPolyObject::EpfnAttach (INode *node, bool & canUndo, INode *myNode, TimeValue t) {
	EpfnAttach (node, myNode, t);
}

void EditPolyObject::EpfnAttach (INode *node, INode *myNode, TimeValue t) {
	// First get the object
	BOOL del = FALSE;
	PolyObject *obj = NULL;
	ObjectState os = node->GetObjectRef()->Eval(t);
	if (os.obj->IsSubClassOf(polyObjectClassID)) obj = (PolyObject *) os.obj;
	else {
		if (!os.obj->CanConvertToType(polyObjectClassID)) return;
		obj = (PolyObject*)os.obj->ConvertToType (t, polyObjectClassID);
		if (obj!=os.obj) del = TRUE;
	}

	EditPolyObject *pobj = NULL;
	if (os.obj->ClassID() == EPOLYOBJ_CLASS_ID) pobj = (EditPolyObject *)obj;

	macroRecorder->FunctionCall(_T("$.EditablePoly.attach"), 2, 0,
		mr_reftarg, node, mr_reftarg, myNode);
	macroRecorder->EmitScript ();

	theHold.Begin();

	// Combine the materials of the two nodes.
	int mat2Offset=0;
	Mtl *m1 = myNode->GetMtl();
	Mtl *m2 = node->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 && (m1 != m2)) {
		if (attachMat==ATTACHMAT_IDTOMAT) {
			FitPolyMeshIDsToMaterial (mm, m1);
			FitPolyMeshIDsToMaterial (obj->mm, m2);
			if (condenseMat) condenseMe = TRUE;
		}

		theHold.Suspend ();
		if (attachMat==ATTACHMAT_MATTOID) {
			m1 = FitMaterialToPolyMeshIDs (GetMesh(), m1);
			m2 = FitMaterialToPolyMeshIDs (obj->GetMesh(), m2);
		}
		Mtl *multi = CombineMaterials(m1, m2, mat2Offset);
		if (attachMat == ATTACHMAT_NEITHER) mat2Offset = 0;
		theHold.Resume ();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = selLevel;
		selLevel = EP_SL_OBJECT;
		myNode->SetMtl(multi);
		selLevel = oldSL;
		m1 = multi;
	}
	if (!m1 && m2) {
		// This material operation seems safe.
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = selLevel;
		if (oldSL>EP_SL_EDGE) selLevel = EP_SL_OBJECT;
		myNode->SetMtl (m2);
		if (oldSL>EP_SL_EDGE) selLevel = oldSL;
		m1 = m2;
	}

	// Construct a transformation that takes a vertex out of the space of
	// the source node and puts it into the space of the destination node.
	Matrix3 relativeTransform = node->GetObjectTM(t) *
		Inverse(myNode->GetObjectTM(t));

	theHold.Put (new CreateOnlyRestore (this));
	int ovnum = mm.numv;
	int oenum = mm.nume;
	int ofnum = mm.numf;
	mm += obj->mm;
	// Note that following code only modifies created elements,
	// so it's still ok to use the CreateOnlyRestore.
	for (int i=ovnum; i<mm.numv; i++) mm.v[i].p = relativeTransform * mm.v[i].p;
	if (relativeTransform.Parity()) {
		mm.ClearFFlags (MN_USER);	// Luna task 748Z: code cleanup - don't use WHATEVER.
		for (i=ofnum; i<mm.numf; i++) {
			if (!mm.f[i].GetFlag (MN_DEAD)) mm.f[i].SetFlag (MN_USER);
		}
		mm.FlipElementNormals (MN_USER);
	}
	if (mat2Offset) {
		for (i=ofnum; i<mm.numf; i++) mm.f[i].material += mat2Offset;
	}
	GetCOREInterface()->DeleteNode (node);

	if (pobj != NULL)
	{
		// Attach the named selections!
		int ourSize[3], ourOldSize[3], theirSize[3];
		ourSize[0] = mm.numv;
		ourSize[1] = mm.nume;
		ourSize[2] = mm.numf;
		ourOldSize[0] = ovnum;
		ourOldSize[1] = oenum;
		ourOldSize[2] = ofnum;
		theirSize[0] = pobj->mm.numv;
		theirSize[1] = pobj->mm.nume;
		theirSize[2] = pobj->mm.numf;

		for (int level=0; level<3; level++)
		{
			if (pobj->selSet[level].Count () < 1) continue;

			for (i=0; i<pobj->selSet[level].Count(); i++)
			{
				// Prepare a new bitarray with the re-indexed bits set:
				BitArray & theirSet = *(pobj->selSet[level].sets[i]);
				BitArray newSet;
				newSet.SetSize (ourSize[level]);
				int k, maxK = theirSet.GetSize();
				if (maxK > theirSize[level]) maxK = theirSize[level];
				for (k=0; k<maxK; k++) newSet.Set (k+ourOldSize[level], theirSet[k]);

				// Figure out if we've got a name that matches:
				for (int j=0; j<selSet[level].Count(); j++)
				{
					if (*(selSet[level].names[j]) == *(pobj->selSet[level].names[i])) break;
				}

				// Append or merge sets as appropriate.
				if (j==selSet[level].Count()) {
					theHold.Put(new AppendSetRestore(&(selSet[level]),this));
					selSet[level].AppendSet (newSet, 0, *pobj->selSet[level].names[i]);
				}
				else
				{
					BitArray & ourSet = *GetNamedSelSet (level, j);
					maxK = ourSet.GetSize();
					if (maxK > ourOldSize[level]) maxK = ourOldSize[level];
					for (k=0; k<maxK; k++) newSet.Set (k, ourSet[k]);
					SetNamedSelSet (level, j, &newSet);
				}
			}
		}
	}
	theHold.Accept (GetString (IDS_ATTACH_OBJECT));

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);

	if (m1 && condenseMe) {
		// Following clears undo stack.
		m1 = CondenseMatAssignments (GetMesh(), m1);
	}
	if (del) delete obj;
}

void EditPolyObject::EpfnMultiAttach (INodeTab &nodeTab, INode *myNode, TimeValue t) {
	bool canUndo = TRUE;
	if (nodeTab.Count() > 1) theHold.SuperBegin ();
	for (int i=0; i<nodeTab.Count(); i++) EpfnAttach (nodeTab[i], myNode, t);
	if (nodeTab.Count() > 1) theHold.SuperAccept (GetString (IDS_ATTACH_LIST));
	RefreshScreen ();
}

bool EditPolyObject::EpfnDetachToElement (int msl, DWORD flags, bool keepOriginal) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	// Verify that we have some components that may be detached.
	int i;
	switch (msl) {
	case MNM_SL_FACE:
		for (i=0; i<mm.numf; i++) {
			if (mm.f[i].GetFlag (MN_DEAD)) continue;
			if (mm.f[i].GetFlag (flags)) break;
		}
		if (i==mm.numf) return false;
		break;
	case MNM_SL_VERTEX:
	case MNM_SL_EDGE:
		mm.ClearFFlags (MN_USER);	// Luna task 748Z: code cleanup - don't use WHATEVER.
		if (mm.PropegateComponentFlags (MNM_SL_FACE, MN_USER, msl, flags) == 0)
			return false;
		flags = MN_USER;
		break;
	default:
		return false;
	}

	bool localHold = !theHold.Holding();
	if (localHold) theHold.Begin ();
	TopoChangeRestore *tchange = new TopoChangeRestore (this);
	tchange->Before ();
	if (keepOriginal) mm.CloneFaces (flags, true);
	else mm.DetachFaces (flags);
	tchange->After ();
	theHold.Put (tchange);
	if (localHold) theHold.Accept (GetString (IDS_DETACH));

	macroRecorder->FunctionCall(_T("$.EditablePoly.detachToElement"), 1, 1,
		mr_int, msl, _T("keepOriginal"), mr_bool, keepOriginal);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return true;
}

// NOTE: Still depends on ip for node creation...
// now getting via GetCOREInterface()
bool EditPolyObject::EpfnDetachToObject (TSTR name, int msl, DWORD flags, bool keepOriginal, INode *myNode, TimeValue t) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	theHold.Begin ();

	int isoVertCount = 0;
	int i;
	if (msl == MNM_SL_VERTEX) {
		// Check for isolated vertices:
		for (i=0; i<mm.numv; i++) {
			if (mm.v[i].GetFlag (MN_DEAD)) continue;
			if (!mm.v[i].GetFlag (flags)) continue;
			if (mm.vedg[i].Count() == 0) isoVertCount++;
		}
	}

	// First, make the detachables into their own element:
	if (!EpfnDetachToElement (msl, flags, keepOriginal) && !isoVertCount) {
		theHold.Accept (GetString (IDS_DETACH));
		return FALSE;
	}

	// we want to deal with the faces detached by DetachToElement.
	// If we're not in that SO level, such faces are indicated by MN_USER.
	// Luna task 748Z: code cleanup - don't use WHATEVER.
	DWORD elemFlags = (msl==MNM_SL_FACE) ? flags : MN_USER;

	// Animation confuses things.
	SuspendAnimate();
	AnimateOff();

	PolyObject *newObj = CreateEditablePolyObject();
	// Copy our parameters (such as soft selection, subdivision) to new object.
	newObj->ReplaceReference(EPOLY_PBLOCK,pblock->Clone());

	// Put detached portion into newObj
	TopoChangeRestore *tchange = new TopoChangeRestore (this);
	tchange->Before ();
	mm.DetachElementToObject (newObj->mm, elemFlags, true);

	if (isoVertCount) {
		// Move the isolated vertices last.
		int oldNumV = newObj->mm.numv;
		newObj->mm.setNumVerts (oldNumV + isoVertCount);
		isoVertCount = oldNumV;
		for (i=0; i<mm.numv; i++) {
			if (mm.v[i].GetFlag (MN_DEAD)) continue;
			if (!mm.v[i].GetFlag (flags)) continue;
			if (mm.vedg[i].Count()) continue;	// (Shouldn't happen after above detach.)
			newObj->mm.v[isoVertCount++] = mm.v[i];
			mm.v[i].SetFlag (MN_DEAD);
		}
	}

	// SCA Fix 1/29/01: the newObj mesh was always being created with the NONTRI flag empty,
	// leading to problems in rendering and a couple other areas.  Instead we copy the
	// NONTRI flag from our current mesh.  Note that this may result in the NONTRI flag
	// being set on a fully triangulated detached region, but that's acceptable; leaving out
	// the NONTRI flag when it is non-triangulated is much more serious.
	newObj->mm.CopyFlags (mm, MN_MESH_NONTRI);
	tchange->After ();
	theHold.Put (tchange);

	CollapseDeadStructs ();

	// Add the object to the scene. Give it the given name
	// and set its transform to ours.
	// Also, give it our material.
	INode *node = GetCOREInterface()->CreateObjectNode(newObj);
	Matrix3 ntm = myNode->GetNodeTM(t);
	if (GetCOREInterface()->GetINodeByName(name) != node) {	// Just in case name = "Object01" or some such default.
		TSTR uname = name;
		if (GetCOREInterface()->GetINodeByName (uname)) GetCOREInterface()->MakeNameUnique(uname);
		node->SetName(uname);
	}
	node->SetNodeTM(t,ntm);
	node->CopyProperties (myNode);
	node->FlagForeground(t,FALSE);
	node->SetMtl(myNode->GetMtl());
	node->SetObjOffsetPos (myNode->GetObjOffsetPos());
	node->SetObjOffsetRot (myNode->GetObjOffsetRot());
	node->SetObjOffsetScale (myNode->GetObjOffsetScale());

	theHold.Accept (GetString (IDS_DETACH));
	ResumeAnimate();
	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return true;
}

bool EditPolyObject::EpfnSplitEdges (DWORD flag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!mm.SplitFlaggedEdges (flag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.splitEdges"), 0, 0);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT);
	return true;
}

bool EditPolyObject::EpfnBreakVerts (DWORD flag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!mm.SplitFlaggedVertices (flag)) {
		if (tchange) {
			delete tchange;
		}
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.breakVerts"), 0,0);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT);
	return true;
}

int EditPolyObject::EpfnDivideFace (int face, Tab<float> &bary, bool select) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int nv = mm.DivideFace (face, bary);
	if (select) mm.v[nv].SetFlag (MN_SEL);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	if (macroRecorder->Enabled()) {
		// since we don't have macrorecorder support for arrays,
		// we need to throw together a string and pass it as an mr_name.
		char baryCoordString[500];
		char baryValue[40];
		int stringPos=0;
		baryCoordString[stringPos++] = '(';
		for (int i=0; i<bary.Count(); i++) {
			sprintf (baryValue, "%f", bary[i]);
			memcpy (baryCoordString + stringPos, baryValue, strlen(baryValue));
			stringPos += strlen(baryValue);
			if (i+1<bary.Count()) baryCoordString[stringPos++] = ',';
		}
		baryCoordString[stringPos++] = ')';
		baryCoordString[stringPos++] = '\0';

		if (select) {
			macroRecorder->FunctionCall(_T("$.EditablePoly.divideFace"), 2, 1,
				mr_int, face+1, mr_name, _T(baryCoordString), _T("select"), mr_bool, true);
		} else {
			macroRecorder->FunctionCall(_T("$.EditablePoly.divideFace"), 2, 0,
				mr_int, face+1, mr_name, _T(baryCoordString));
		}
		macroRecorder->EmitScript ();
	}

	if (select) {
		LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
	} else {
		LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE-PART_SELECT);
	}

	return nv;
}

int EditPolyObject::EpfnDivideEdge (int edge, float prop, bool select) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int nv = mm.SplitEdge (edge, prop);
	if (select) mm.v[nv].SetFlag (MN_SEL);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	if (select) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.divideEdge"), 2, 1,
			mr_int, edge+1, mr_float, prop, _T("select"), mr_bool, true);
		LocalDataChanged (PART_TOPO|PART_GEOM|PART_SELECT);
	} else {
		macroRecorder->FunctionCall(_T("$.EditablePoly.divideEdge"), 2, 0,
			mr_int, edge+1, mr_float, prop);	// +1 for index!
		LocalDataChanged (PART_TOPO|PART_GEOM);
	}
	macroRecorder->EmitScript ();
	return nv;
}

// Luna task 748A - "epop" version of Extrude
bool EditPolyObject::EpfnExtrudeMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	float height, width;
	Tab<Point3> tUpDir, tDelta;
	Tab<UVVert> tMapDelta;
	MNChamferData *pChamData = NULL;
	int i, mapChannel;


	switch (msl) {
	case MNM_SL_VERTEX:
	case MNM_SL_EDGE:
		pChamData = temp->ChamferData();
		if (pChamData == NULL) return false;

		// Do the topology change:
		if (msl == MNM_SL_EDGE) {
			if (!mesh.ExtrudeEdges (flag, pChamData, tUpDir)) return false;
			pblock->GetValue (ep_edge_extrude_height, TimeValue(0), height, FOREVER);
			pblock->GetValue (ep_edge_extrude_width, TimeValue(0), width, FOREVER);
		} else {
			if (!mesh.ExtrudeVertices (flag, pChamData, tUpDir)) return false;
			pblock->GetValue (ep_vertex_extrude_height, TimeValue(0), height, FOREVER);
			pblock->GetValue (ep_vertex_extrude_width, TimeValue(0), width, FOREVER);
		}

		// Apply map changes based on base width:
		for (mapChannel=-NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
			if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
			pChamData->GetMapDelta (mesh, mapChannel, width, tMapDelta);
			UVVert *pMapVerts = mesh.M(mapChannel)->v;
			if (!pMapVerts) continue;
			for (i=0; i<mesh.M(mapChannel)->numv; i++) pMapVerts[i] += tMapDelta[i];
		}

		// Apply geom changes based on base width:
		pChamData->GetDelta (width, tDelta);
		for (i=0; i<mesh.numv; i++) mesh.v[i].p += tDelta[i];

		// Move the points up:
		for (i=0; i<tUpDir.Count(); i++) mesh.v[i].p += tUpDir[i]*height;

		return true;

	case MNM_SL_FACE:
		// Do the topology change:
		if (!DoExtrusionMesh (mesh, temp, msl, flag, false)) return false;

		pChamData = temp->ChamferData();
		if (pChamData == NULL) return false;

		// Apply height-based geometric changes:
		pblock->GetValue (ep_face_extrude_height, TimeValue(0), height, FOREVER);
		pChamData->GetDelta (height, tUpDir);
		for (i=0; i<mesh.numv; i++) {
			if (tUpDir[i] == Point3(0,0,0)) continue;
			mesh.v[i].p += tUpDir[i];
		}
		return true;
	}

	return false;
}

// Luna task 748A - "epop" version of Bevel
bool EditPolyObject::EpfnBevelMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (msl != MNM_SL_FACE) return false;

	if (!DoExtrusionMesh (mesh, temp, msl, flag, true)) return false;

	float extHeight, outline;
	pblock->GetValue (ep_bevel_height, TimeValue(0), extHeight, FOREVER);
	pblock->GetValue (ep_bevel_outline, TimeValue(0), outline, FOREVER);

	MNChamferData *chamData = temp->ChamferData();
	if (chamData == NULL) return false;
	Tab<Point3> delta;
	delta.SetCount (mesh.numv);
	chamData->GetDelta (extHeight, delta);

	int i;
	if (outline) {
		int extType;
		pblock->GetValue (ep_bevel_type, TimeValue(0), extType, FOREVER);
		Tab<Point3> *outDir;
		if (extType == 0) outDir = temp->OutlineDir (MESH_EXTRUDE_CLUSTER);
		else outDir = temp->OutlineDir (MESH_EXTRUDE_LOCAL);
		Point3 *od = outDir->Addr(0);
		for (i=0; i<mesh.numv; i++) delta[i] += od[i]*outline;
	}

	for (i=0; i<mesh.numv; i++) mesh.v[i].p += delta[i];
	return true;
}

// Luna task 748R
bool EditPolyObject::EpfnInsetMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (msl != MNM_SL_FACE) return false;

	if (!DoInsetMesh (mesh, temp, msl, flag)) return false;

	float inset;
	pblock->GetValue (ep_inset, TimeValue(0), inset, FOREVER);

	int i;
	if (inset) {
		Tab<Point3> *outDir = temp->OutlineDir (MESH_EXTRUDE_CLUSTER);
		Point3 *od = outDir->Addr(0);
		for (i=0; i<mesh.numv; i++) mesh.v[i].p -= od[i]*inset;
	}

	return true;
}

bool EditPolyObject::EpfnOutline (DWORD flag) {
	float outline;
	pblock->GetValue (ep_outline, TimeValue(0), outline, FOREVER);
	if (!outline) return false;

	int i;
	Tab<Point3> tDelta;
	tDelta.SetCount (mm.numv);
	bool ret = false;
	if (outline) {
		Tab<Point3> *outDir = TempData()->OutlineDir (MESH_EXTRUDE_CLUSTER, flag);
		Point3 *od = outDir->Addr(0);
		Point3 *pDelta = tDelta.Addr(0);
		for (i=0; i<mm.numv; i++) {
			pDelta[i] = od[i]*outline;
			if (!ret && (od[i] != Point3(0,0,0))) ret = true;
		}
	}

	if (ret) {
		ApplyDelta (tDelta, this, ip->GetTime());
		macroRecorder->FunctionCall(_T("$.EditablePoly.Outline"), 0, 0);
		macroRecorder->EmitScript ();
	}
	return ret;
}

bool EditPolyObject::EpfnOutlineMesh (MNMesh & mesh, MNTempData *temp, DWORD flag) {
	float outline;
	pblock->GetValue (ep_outline, TimeValue(0), outline, FOREVER);
	if (!outline) return false;

	int i;
	Tab<Point3> tDelta;
	tDelta.SetCount (mesh.numv);
	if (outline) {
		Tab<Point3> *outDir = temp->OutlineDir (MESH_EXTRUDE_CLUSTER, flag);
		Point3 *od = outDir->Addr(0);
		for (i=0; i<mesh.numv; i++) mesh.v[i].p += od[i]*outline;
	}

	return true;
}

// Luna task 748A - "epop" version of Chamfer
bool EditPolyObject::EpfnChamferMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (msl == MNM_SL_FACE) return false;
	if (msl == MNM_SL_OBJECT) return false;

	if (!DoChamferMesh (mesh, temp, msl, flag)) return false;
	MNChamferData *chamData = temp->ChamferData();
	if (chamData == NULL) return false;

	float chamfer;
	switch (msl) {
	case MNM_SL_VERTEX:
		pblock->GetValue (ep_vertex_chamfer, TimeValue(0), chamfer, FOREVER);
		break;
	case MNM_SL_EDGE:
		pblock->GetValue (ep_edge_chamfer, TimeValue(0), chamfer, FOREVER);
		break;
	}
	if (chamfer==0.0f) return true;	// (true for topological change.)

	// Do DragMap's first, because they don't call RefreshScreen.
	Tab<UVVert> mapDelta;
	int i, mapChannel;
	for (mapChannel=-NUM_HIDDENMAPS; mapChannel<mesh.numm; mapChannel++) {
		if (mesh.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		chamData->GetMapDelta (mesh, mapChannel, chamfer, mapDelta);
		UVVert *mv = mesh.M(mapChannel)->v;
		if (!mv) continue;
		for (i=0; i<mesh.M(mapChannel)->numv; i++) mv[i] += mapDelta[i];
	}

	Tab<Point3> delta;
	chamData->GetDelta (chamfer, delta);
	for (i=0; i<mesh.numv; i++) {
		if (delta[i] == Point3(0,0,0)) continue;
		mesh.v[i].p += delta[i];
	}
	return true;
}

void EditPolyObject::EpfnExtrudeFaces (float amount, DWORD flag, TimeValue t) {
	pblock->SetValue (ep_face_extrude_height, t, amount);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (EpfnExtrudeMesh (mm, TempData(), MNM_SL_FACE, flag)) {
		if (tchange) {
			tchange->After();
			theHold.Put (tchange);
		}
		macroRecorder->FunctionCall(_T("$.EditablePoly.extrudeFaces"), 1, 0, mr_float, amount);
		macroRecorder->EmitScript ();
	} else {
		if (tchange) delete tchange;
	}
}

void EditPolyObject::EpfnBevelFaces (float height, float outline, DWORD flag, TimeValue t) {
	pblock->SetValue (ep_bevel_height, t, height);
	pblock->SetValue (ep_bevel_outline, t, outline);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (EpfnBevelMesh (mm, TempData(), MNM_SL_FACE, flag)) {
		if (tchange) {
			tchange->After();
			theHold.Put (tchange);
		}
		macroRecorder->FunctionCall(_T("$.EditablePoly.bevelFaces"), 2, 0, mr_float, height, mr_float, outline);
		macroRecorder->EmitScript ();
	} else {
		if (tchange) delete tchange;
	}
}

void EditPolyObject::EpfnChamferVertices (float amount, TimeValue t) {
	pblock->SetValue (ep_vertex_chamfer, t, amount);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (EpfnChamferMesh (mm, TempData(), MNM_SL_VERTEX, MN_SEL)) {
		if (tchange) {
			tchange->After();
			theHold.Put (tchange);
		}
		macroRecorder->FunctionCall(_T("$.EditablePoly.chamferVertices"), 1, 0, mr_float, amount);
		macroRecorder->EmitScript ();
	} else {
		if (tchange) delete tchange;
	}
}

void EditPolyObject::EpfnChamferEdges (float amount, TimeValue t) {
	pblock->SetValue (ep_edge_chamfer, t, amount);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (EpfnChamferMesh (mm, TempData(), MNM_SL_EDGE, MN_SEL)) {
		if (tchange) {
			tchange->After();
			theHold.Put (tchange);
		}
		macroRecorder->FunctionCall(_T("$.EditablePoly.chamferEdges"), 1, 0, mr_float, amount);
		macroRecorder->EmitScript ();
	} else {
		if (tchange) delete tchange;
	}
}

bool EditPolyObject::DoExtrusionMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag, bool isBevel) {
	bool ret=false, topoChange=false;
	if (msl != MNM_SL_FACE) return false;

	int extType;
	if (isBevel)
		pblock->GetValue (ep_bevel_type, TimeValue(0), extType, FOREVER);
	else
		pblock->GetValue (ep_extrusion_type, TimeValue(0), extType, FOREVER);

	if (flag != MN_SEL) {
		// will need to rebuild face clusters.
		temp->Invalidate (PART_TOPO);
	}
	MNFaceClusters *fclust;
	int i;
	switch (extType) {
	case 0:	// Group normals
		fclust = temp->FaceClusters(flag);
		if (fclust->count > 0) ret = true;
		topoChange = mesh.ExtrudeFaceClusters (*fclust);
		if (topoChange) {
			temp->Invalidate (PART_TOPO);	// Forces reevaluation of face clusters.
			temp->freeBevelInfo ();
		}
		if (ret) {
			mesh.GetExtrudeDirection (temp->ChamferData(),
				temp->FaceClusters(flag),
				temp->ClusterNormals (MNM_SL_FACE, flag)->Addr(0));
		}
		break;

	case 1:	// Local normals
		fclust = temp->FaceClusters(flag);
		if (fclust->count > 0) ret = true;
		topoChange = mesh.ExtrudeFaceClusters (*fclust);
		if (topoChange) {
			temp->Invalidate (PART_TOPO);	// Forces reevaluation of face clusters.
			temp->freeBevelInfo ();
		}
		if (ret) mesh.GetExtrudeDirection (temp->ChamferData(), flag);
		break;

	case 2:	// poly-by-poly extrusion.
		for (i=0; i<mesh.numf; i++) if (mesh.f[i].GetFlag (flag)) break;
		if (i<mesh.numf) ret = true;
		topoChange = mesh.ExtrudeFaces (flag);
		if (topoChange) {
			temp->Invalidate (PART_TOPO);
			temp->freeBevelInfo ();
		}
		if (ret) mesh.GetExtrudeDirection (temp->ChamferData(), flag);
		break;
	}
	return ret;
}

bool EditPolyObject::DoExtrusion (int msl, DWORD flag, bool isBevel) {
	theHold.Begin();
	TopoChangeRestore *tchange = new TopoChangeRestore (this);
	tchange->Before ();
	bool ret = DoExtrusionMesh (mm, TempData(), msl, flag, isBevel);

	if (ret) {
		tchange->After ();
		theHold.Put (tchange);
		LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
		RefreshScreen ();
	} else {
		delete tchange;
	}
	theHold.Accept (GetString (IDS_EXTRUDE));	// (doesn't do anything if no Put() call.)
	return ret;
}

void EditPolyObject::EpfnBeginExtrude (int msl, DWORD flag, TimeValue t) {	
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (inExtrude) return;
	inExtrude = TRUE;
	theHold.SuperBegin();
	if (!DoExtrusion(msl, flag, false)) {
		theHold.SuperCancel ();
		inExtrude = FALSE;
		return;
	}
	BitArray vset;
	if ((msl != MNM_SL_VERTEX) || !(flag & MN_USER)) {	// Luna task 748Z: code cleanup - don't use WHATEVER.
		mm.ClearVFlags (MN_USER);
		// NOTE: following won't work for msl==MNM_SL_OBJECT
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, msl, flag);
	}
	mm.getVerticesByFlag (vset, (msl == MNM_SL_VERTEX) ? flag : MN_USER);
	PlugControllersSel (t, vset);
	DragMoveInit();
}

void EditPolyObject::EpfnEndExtrude (bool accept, TimeValue t) {
	if (!inExtrude) return;
		inExtrude = FALSE;
		TempData()->freeChamferData ();

		theHold.Begin ();
		DragMoveAccept (t);
		theHold.Accept(GetString(IDS_MOVE));
	if (accept) {
		theHold.SuperAccept(GetString(IDS_EXTRUDE));
		mLastOperation = epop_extrude;
		macroRecorder->FunctionCall (_T("$.EditablePoly.buttonOp"), 1, 0, mr_name, _T("extrude"));
		macroRecorder->EmitScript ();
	} else {
		theHold.SuperCancel();
		InvalidateTempData ();
	}
}

void EditPolyObject::EpfnDragExtrude (float amount, TimeValue t) {
	if (!inExtrude) return;
	DragMoveRestore ();

	MNChamferData *chamData = TempData()->ChamferData();
	if (chamData == NULL) return;
	Tab<Point3> delta;
	delta.SetCount (mm.numv);
	chamData->GetDelta (amount, delta);
	DragMove (delta, this, t);
}

static bool ExtDone=false;
static DWORD lastBevelFlag=0x0;

void EditPolyObject::EpfnBeginBevel (int msl, DWORD flag, bool doExtrude, TimeValue t) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (inBevel) return;
	inBevel = TRUE;
	theHold.SuperBegin();
	if (doExtrude && !DoExtrusion (MNM_SL_FACE, flag, true)) {
		theHold.SuperCancel ();
		inBevel = FALSE;
		return;
	}
	ExtDone = doExtrude;
	BitArray vset;
	if ((msl != MNM_SL_VERTEX) || !(flag & MN_USER)) {	// Luna task 748Z: code cleanup - don't use WHATEVER.
		mm.ClearVFlags (MN_USER);
		// NOTE: following won't work for msl==MNM_SL_OBJECT
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, msl, flag);
	}
	mm.getVerticesByFlag (vset, (msl == MNM_SL_VERTEX) ? flag : MN_USER);
	PlugControllersSel(t,vset);
	DragMoveInit ();
	lastBevelFlag = flag;
}

void EditPolyObject::EpfnEndBevel (bool accept, TimeValue t) {
	if (!inBevel) return;
	inBevel = FALSE;
	TempData()->freeBevelInfo();
	TempData()->freeChamferData ();

	theHold.Begin ();
	DragMoveAccept (t);
	theHold.Accept(GetString(IDS_BEVEL));
	if (accept) {
		theHold.SuperAccept(GetString(ExtDone ? IDS_BEVEL : IDS_OUTLINE));
		EpSetLastOperation (ExtDone ? epop_bevel : epop_null);
		macroRecorder->FunctionCall (_T("$.EditablePoly.buttonOp"), 1, 0, mr_name, _T("bevel"));
		macroRecorder->EmitScript ();
	} else {
		theHold.SuperCancel();
		InvalidateTempData ();
	}
	ExtDone = false;
}

void EditPolyObject::EpfnDragBevel (float amount, float height, TimeValue t) {
	if (!inBevel) return;
	DragMoveRestore ();

	MNChamferData *chamData = TempData()->ChamferData();
	if (chamData == NULL) return;

	int i;
	Tab<Point3> delta;
	delta.SetCount (mm.numv);
	if (height) chamData->GetDelta (height, delta);
	else {
		for (i=0; i<mm.numv; i++) delta[i] = Point3(0,0,0);
	}

	if (amount) {
		if (!lastBevelFlag) lastBevelFlag = MN_SEL;	// shouldn't happen
		int extType;
		pblock->GetValue (ep_bevel_type, TimeValue(0), extType, FOREVER);
		Tab<Point3> *outDir;
		if (extType == 0) outDir = TempData()->OutlineDir (MESH_EXTRUDE_CLUSTER, lastBevelFlag);
		else outDir = TempData()->OutlineDir (MESH_EXTRUDE_LOCAL, lastBevelFlag);
		Point3 *od = outDir->Addr(0);
		for (i=0; i<mm.numv; i++) delta[i] += od[i]*amount;
	}

	DragMove (delta, this, t);
}

void EditPolyObject::EpfnBeginInset (int msl, DWORD flag, TimeValue t) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (inInset) return;
	inInset = TRUE;
	theHold.SuperBegin();
	if (!DoInset (MNM_SL_FACE, flag)) {
		theHold.SuperCancel ();
		inInset = false;
		return;
	}
	BitArray vset;
	if ((msl != MNM_SL_VERTEX) || !(flag & MN_USER)) {	// Luna task 748Z: code cleanup - don't use WHATEVER.
		mm.ClearVFlags (MN_USER);
		// NOTE: following won't work for msl==MNM_SL_OBJECT
		mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, msl, flag);
	}
	mm.getVerticesByFlag (vset, (msl == MNM_SL_VERTEX) ? flag : MN_USER);
	PlugControllersSel(t,vset);
	DragMoveInit ();
}

void EditPolyObject::EpfnEndInset (bool accept, TimeValue t) {
	if (!inInset) return;
	inInset = FALSE;
	TempData()->freeBevelInfo();
	TempData()->freeChamferData ();

	theHold.Begin ();
	DragMoveAccept (t);
	theHold.Accept(GetString(IDS_BEVEL));
	if (accept) {
		theHold.SuperAccept(GetString(IDS_INSET));
		EpSetLastOperation (epop_inset);
		macroRecorder->FunctionCall (_T("$.EditablePoly.buttonOp"), 1, 0, mr_name, _T("inset"));
		macroRecorder->EmitScript ();
	} else {
		theHold.SuperCancel();
		InvalidateTempData ();
	}
}

void EditPolyObject::EpfnDragInset (float amount, TimeValue t) {
	if (!inInset) return;
	DragMoveRestore ();
	if (!amount) return;

	Tab<Point3> *outDir;
	outDir = TempData()->OutlineDir (MESH_EXTRUDE_LOCAL);
	Point3 *od = outDir->Addr(0);

	Tab<Point3> delta;
	delta.SetCount (mm.numv);
	for (int i=0; i<mm.numv; i++) delta[i] = -od[i]*amount;

	DragMove (delta, this, t);
}

// Luna task 748A - MNMesh-argument version of DoChamfer.
bool EditPolyObject::DoChamferMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag) {
	MNChamferData *mcd = temp->ChamferData();
	bool ret = false;
	switch (msl) {
	case MNM_SL_VERTEX:
		ret = mesh.ChamferVertices (MN_SEL, mcd);
		break;
	case MNM_SL_EDGE:
		ret = mesh.ChamferEdges (MN_SEL, mcd);
		break;
	}
	return ret;
}

bool EditPolyObject::DoChamfer (int msl, DWORD flag) {
	theHold.Begin();
	TopoChangeRestore *tchange = new TopoChangeRestore (this);
	tchange->Before ();
	bool ret = DoChamferMesh (mm, TempData(), msl, flag);
	if (ret) {
		tchange->After ();
		theHold.Put (tchange);
		LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
		RefreshScreen ();
	} else {
		delete tchange;
	}
	theHold.Accept (GetString (IDS_CHAMFER));
	return ret;
}

void EditPolyObject::EpfnBeginChamfer (int msl, TimeValue t) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (inChamfer) return;
	inChamfer = TRUE;
	theHold.SuperBegin();
	if (!DoChamfer (msl, MN_SEL) || !TempData()->ChamferData()->vmax.Count()) {
		theHold.SuperCancel ();
		return;
	}
	BitArray sel;
	sel.SetSize (mm.numv);
	sel.ClearAll ();
	float *vmp = TempData()->ChamferData()->vmax.Addr(0);
	for (int i=0; i<mm.numv; i++) if (vmp[i]) sel.Set(i);
	PlugControllersSel (t,sel);
	DragMoveInit();
}

void EditPolyObject::EpfnEndChamfer (bool accept, TimeValue t) {
	if (!inChamfer) return;
	inChamfer = FALSE;
	TempData()->freeChamferData();

	theHold.Begin ();
	DragMoveAccept (t);
	theHold.Accept(GetString(IDS_MOVE));
	if (accept) {	// Luna task 748BB
		theHold.SuperAccept(GetString(IDS_CHAMFER));
		mLastOperation = epop_chamfer;
		macroRecorder->FunctionCall (_T("$.EditablePoly.buttonOp"), 1, 0, mr_name, _T("chamfer"));
		macroRecorder->EmitScript ();
	} else {
		theHold.SuperCancel();
		InvalidateTempData ();
	}
}

void EditPolyObject::EpfnDragChamfer (float amount, TimeValue t) {
	if (!inChamfer) return;
	DragMoveRestore ();
	if (amount<=0) return;
	if (!TempData()->ChamferData()) return;

	// Do DragMap's first, because they don't call RefreshScreen.
	Tab<UVVert> mapDelta;
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
		if (mm.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		TempData()->ChamferData()->GetMapDelta (mm, mapChannel, amount, mapDelta);
		DragMap (mapChannel, mapDelta, this, t);
	}

	Tab<Point3> delta;
	TempData()->ChamferData()->GetDelta (amount, delta);
	DragMove (delta, this, t);
}

// Luna task 748R - Inset mode
bool EditPolyObject::DoInsetMesh (MNMesh & mesh, MNTempData *temp, int msl, DWORD flag) {
	if (msl != MNM_SL_FACE) return false;

	int insetType;
	pblock->GetValue (ep_inset_type, TimeValue(0), insetType, FOREVER);

	if (flag != MN_SEL) {
		// will need to rebuild face clusters.
		temp->Invalidate (PART_TOPO);
	}
	bool ret;
	switch (insetType) {
	case 0:	// do whole cluster together
		ret = mesh.ExtrudeFaceClusters (*(temp->FaceClusters(flag)));
		break;

	case 1:	// poly-by-poly
		ret = mesh.ExtrudeFaces (flag);
		break;
	}

	if (ret) {
		temp->Invalidate (PART_TOPO);	// Forces reevaluation of face clusters.
		temp->freeBevelInfo ();
		mesh.GetExtrudeDirection (temp->ChamferData(), flag);
	}

	return ret;
}

bool EditPolyObject::DoInset (int msl, DWORD flag) {
	if (msl != MNM_SL_FACE) return false;

	theHold.Begin();
	TopoChangeRestore *tchange = new TopoChangeRestore (this);
	tchange->Before ();

	bool ret = DoInsetMesh (mm, TempData(), msl, flag);

	if (ret) {
		tchange->After ();
		theHold.Put (tchange);
		LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
		RefreshScreen ();
	} else {
		delete tchange;
	}
	theHold.Accept (GetString (IDS_INSET));
	return ret;
}

bool EditPolyObject::EpfnCreateShape (TSTR name, bool createCurveSmooth, INode *myNode, DWORD edgeFlag) {
	bool ret = CreateCurveFromMeshEdges (GetMesh(), myNode, GetCOREInterface(), name, createCurveSmooth, edgeFlag);

	if (ret) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.createShape"), 3, 0,
			mr_string, name, mr_bool, createCurveSmooth, mr_reftarg, myNode);
		macroRecorder->EmitScript ();
	}

	return ret;
}

bool EditPolyObject::EpfnMakePlanar (int msl, DWORD flag, TimeValue t) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (!mm.numv) return false;

	Tab<Point3> delta;
	delta.SetCount (mm.numv);

	if (!mm.MakeFlaggedPlanar (msl, flag, delta.Addr(0))) {
		// Nothing was moved.
		return false;
	}

	ApplyDelta (delta, this, t);

	macroRecorder->FunctionCall(_T("$.EditablePoly.makePlanar"), 1, 0, mr_int, msl);
	macroRecorder->EmitScript ();

	return true;
}

bool EditPolyObject::EpfnAlignToGrid (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	// We'll need the viewport or construction plane transform:
	if (!ip) return false;
	Matrix3 atm, otm, res;
	ViewExp *vpt = ip->GetActiveViewport();
	float zoff;
	vpt->GetConstructionTM(atm);
	ip->ReleaseViewport (vpt);

	// We'll also need our own transform:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	otm = nodes[0]->GetObjectTM(ip->GetTime());
	nodes.DisposeTemporary();
	res = atm*Inverse (otm);	// screenspace-to-objectspace.

	// For ZNorm, we want the object-space unit vector pointing into the screen:
	Point3 ZNorm (0,0,-1);
	ZNorm = Normalize (VectorTransform (res, ZNorm));

	// Find the z-depth of the construction plane, in object space:
	zoff = DotProd (ZNorm, res.GetTrans());

	EpfnMoveToPlane (ZNorm, zoff, msl, flag);

	macroRecorder->FunctionCall(_T("$.EditablePoly.alignToGrid"), 1, 0, mr_int, msl);
	macroRecorder->EmitScript ();

	return true;
}

bool EditPolyObject::EpfnMoveToPlane (Point3 ZNorm, float zoff, int msl, DWORD flag, TimeValue t) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (!mm.numv) return false;

	// Target appropriate vertices with flags:
	if ((msl != MNM_SL_VERTEX) || ((flag&MN_USER) == 0))	// Luna task 748Z: Don't use WHATEVER.
		mm.ClearVFlags (MN_USER);
	if (msl == MNM_SL_OBJECT) {
		for (int i=0; i<mm.numv; i++) mm.v[i].SetFlag (MN_USER, !mm.v[i].GetFlag (MN_DEAD));
	} else mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, msl, flag);

	Tab<Point3> delta;
	delta.SetCount (mm.numv);
	if (!mm.MoveVertsToPlane (ZNorm, zoff, MN_USER, delta.Addr(0))) return false;	// nothing happened.
	ApplyDelta (delta, this, t);
	return true;
}

bool EditPolyObject::EpfnAlignToView (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (!mm.numv) return false;
	// We'll need the viewport or construction plane transform:
	if (!ip) return false;
	Matrix3 atm, otm, res;
	ViewExp *vpt = ip->GetActiveViewport();
	float zoff;
	vpt->GetAffineTM(atm);
	atm = Inverse(atm);
	ip->ReleaseViewport (vpt);

	// We'll also need our own transform:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);
	otm = nodes[0]->GetObjectTM(ip->GetTime());
	nodes.DisposeTemporary();
	res = atm*Inverse (otm);	// screenspace-to-objectspace.

	// Target appropriate vertices with flags:
	if ((msl != MNM_SL_VERTEX) || ((flag&MN_USER) == 0))
		mm.ClearVFlags (MN_USER);
	if (msl == MNM_SL_OBJECT) {
		for (int i=0; i<mm.numv; i++) mm.v[i].SetFlag (MN_USER, !mm.v[i].GetFlag (MN_DEAD));
	} else mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, msl, flag);

	BitArray sel = GetMesh().VertexTempSel();
	// For ZNorm, we want the object-space unit vector pointing into the screen:
	Point3 ZNorm (0,0,-1);
	ZNorm = Normalize (VectorTransform (res, ZNorm));

	// Find the average z-depth for the current selection.
	zoff = 0.0f;
	int ct=0;
	for (int i=0; i<mm.numv; i++) {
		if (!mm.v[i].GetFlag (MN_USER)) continue;
		zoff += DotProd (ZNorm, mm.v[i].p);
		ct++;
	}
	zoff /= float(ct);

	Tab<Point3> delta;
	delta.SetCount (mm.numv);
	if (!mm.MoveVertsToPlane (ZNorm, zoff, MN_USER, delta.Addr(0))) return false;	// nothing happened.
	ApplyDelta (delta, this, ip->GetTime());

	macroRecorder->FunctionCall(_T("$.EditablePoly.alignToView"), 1, 0, mr_int, msl);
	macroRecorder->EmitScript ();

	return true;
}

// This method needs to be moved to class MNMesh when the SDK opens up again.
bool MNMeshCollapseEdges (MNMesh & mm, DWORD edgeFlag) {
	// Collect average locations ourselves
	// We do this by collecting information about which vertex
	// or map vertex each eliminated point was welded to - its destination.
	// Then we average things out at the end.
	Tab<int> pointDest;
	pointDest.SetCount (mm.numv);
	for (int i=0; i<mm.numv; i++) pointDest[i] = -1;

	Tab<Tab<int> *> mapPointDest;
	mapPointDest.SetCount (mm.numm + NUM_HIDDENMAPS);
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
		int nhm = mapChannel + NUM_HIDDENMAPS;
		if (mm.M(mapChannel)->GetFlag (MN_DEAD)) mapPointDest[nhm] = NULL;
		else {
			int nv = mm.M(mapChannel)->numv;
			if (!nv) mapPointDest[nhm] = NULL;
			else {
				mapPointDest[nhm] = new Tab<int>;
				mapPointDest[nhm]->SetCount (nv);
				int *md = mapPointDest[nhm]->Addr(0);
				for (i=0; i<nv; i++) md[i] = -1;
			}
		}
	}

	// Perform topological welding, edge by edge
	bool ret=false;
	for (i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!mm.e[i].GetFlag (MN_USER)) continue;
		int v1 = mm.e[i].v1;
		int v2 = mm.e[i].v2;
		int f1 = mm.e[i].f1;
		int f2 = mm.e[i].f2;
		int eid1 = mm.f[f1].EdgeIndex (i);
		int eid2 = (f2>-1) ? mm.f[f2].EdgeIndex (i) : -1;
		Tab<int> mv1, mv2;
		mv1.SetCount ((mm.numm+NUM_HIDDENMAPS)*2);
		mv2.SetCount ((mm.numm+NUM_HIDDENMAPS)*2);
		for (int mapChannel = -NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
			MNMap *map = mm.M(mapChannel);
			int nhm = (NUM_HIDDENMAPS + mapChannel)*2;
			if (map->GetFlag (MN_DEAD)) {
				mv1[nhm] = -1;
				continue;
			}
			mv1[nhm] = map->f[f1].tv[eid1];
			mv1[nhm+1] = (f2>-1) ? map->f[f2].tv[(eid2+1)%mm.f[f2].deg] : mv1[nhm];
			mv2[nhm] = map->f[f1].tv[(eid1+1)%mm.f[f1].deg];
			mv2[nhm+1] = (f2>-1) ? map->f[f2].tv[eid2] : mv2[nhm];
		}
		if (mm.WeldEdge (i)) {
			pointDest[v2] = v1;
			for (int nhm=0; nhm<mapPointDest.Count(); nhm++) {
				if (mapPointDest[nhm] == NULL) continue;
				if (!mapPointDest[nhm]->Count()) continue;
				if (mv1[nhm*2]<0) continue;
				int *mpd = mapPointDest[nhm]->Addr(0);
				if (mv2[nhm*2] != mv1[nhm*2]) mpd[mv2[nhm*2]] = mv1[nhm*2];
				if ((mv2[nhm*2+1] != mv2[nhm*2]) && (mv2[nhm*2+1] != mv1[nhm*2+1]))
					mpd[mv2[nhm*2+1]] = mv1[nhm*2+1];
			}
			ret = true;
		}
	}

	// Then set all the welded vertices to the correct averaged locations
	if (ret) {
		for (mapChannel = -NUM_HIDDENMAPS-1; mapChannel<mm.numm; mapChannel++) {
			// note - -NUM_HIDDENMAPS-1 is not a valid map channel,
			// We're using it for a convenient extra loop for the actual geometry.
			int nhm = mapChannel+NUM_HIDDENMAPS;
			if ((nhm>-1) && mapPointDest[nhm] == NULL) continue;
			Tab<int> & destinations = (nhm<0) ? pointDest : *(mapPointDest[nhm]);

			for (i=0; i<destinations.Count(); i++) {
				if (destinations[i] < 0) continue;
				if (destinations[destinations[i]] > -1) {
					// We have nested destinations - straighten them out.
					// Form a stack of all the intermediate destinations:
					Tab<int> intermediate;
					for (int last=i; destinations[last]>-1; last=destinations[last]) {
						intermediate.Append (1, &last, 2);
						if (intermediate.Count() > destinations.Count()) {
							DbgAssert (0);
							break;
						}
					}
					// Now destinations[current] = -1, which means it's the final destination of all of these.
					// Correct whole chain.
					for (int k=0; k<intermediate.Count()-1; k++) destinations[intermediate[k]] = last;
				}

				if (nhm<0) mm.v[destinations[i]].p += mm.v[i].p;
				else mm.M(mapChannel)->v[destinations[i]] += mm.M(mapChannel)->v[i];

				destinations[destinations[i]]--;
			}

			for (i=0; i<destinations.Count(); i++) {
				if (destinations[i] > -2) continue;
				if (nhm<0) mm.v[i].p /= float(-destinations[i]);
				else mm.M(mapChannel)->v[i] /= float(-destinations[i]);
			}

			// Free memory
			if (nhm>-1) {
				delete mapPointDest[nhm];
				mapPointDest[nhm] = NULL;
			}
		}
	}

	return ret;
}

bool EditPolyObject::EpfnCollapse (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	if (msl == MNM_SL_OBJECT) return false;

	TopoChangeRestore *tchange=NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}

	// In order to collapse other SO levels, we turn them into edge selections.
	// In order to make use of edge clusters (to make this easy), we need to backup the current edge
	// selection, replace it with the new selection, and replace the original edge selection later.

	if (msl != MNM_SL_EDGE || flag != MN_USER)
		mm.ClearEFlags (MN_USER);
	// at vertex level, we require that edges have both ends selected to be targeted.
	// at face level, any edge with at least one selected face is targeted.
	if (msl == MNM_SL_VERTEX) 
		mm.PropegateComponentFlags (MNM_SL_EDGE, MN_USER, msl, flag, true);
	else if (msl != MNM_SL_EDGE || flag != MN_USER)
		mm.PropegateComponentFlags (MNM_SL_EDGE, MN_USER, msl, flag, false);

	bool ret = MNMeshCollapseEdges (mm, MN_USER);

	// If no collapsing occurred, but we're in vertex level, try a weld.
	if (!ret && (msl == MNM_SL_VERTEX)) {
		ret = mm.WeldBorderVerts (999999.0f, flag);
	}

	if (ret) {
		if (tchange) {
			tchange->After ();
			theHold.Put (tchange);
		}
		CollapseDeadStructs ();

		macroRecorder->FunctionCall(_T("$.EditablePoly.collapse"), 1, 0, mr_int, msl);
		macroRecorder->EmitScript ();

		LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
		return true;
	} else {
		if (tchange) delete tchange;
		return false;
	}
}

// Luna task 748A - needed for preview.
bool EditPolyObject::EpfnMeshSmoothMesh (MNMesh & mesh, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	float smoothness;
	int sepSmooth, sepMat;
	int i;
	pblock->GetValue (ep_ms_smoothness, TimeValue(0), smoothness, FOREVER);
	pblock->GetValue (ep_ms_sep_smooth, TimeValue(0), sepSmooth, FOREVER);
	pblock->GetValue (ep_ms_sep_mat, TimeValue(0), sepMat, FOREVER);

	switch (msl) {
	case MNM_SL_OBJECT:
		mesh.ClearVFlags (MN_TARG);
		for (i=0; i<mesh.numv; i++) if (!mesh.v[i].GetFlag (MN_DEAD)) mesh.v[i].SetFlag (MN_TARG);
		break;
	case MNM_SL_VERTEX:
		if ((flag & MN_TARG) == 0) mesh.ClearVFlags (MN_TARG);
		for (i=0; i<mesh.numv; i++) if (mesh.v[i].GetFlag (flag)) break;
		if (i==mesh.numv) return false;
		if (flag != MN_TARG) {
			for (; i<mesh.numv; i++) if (mesh.v[i].GetFlag (flag)) mesh.v[i].SetFlag (MN_TARG);
		}
		break;
	default:
		mesh.ClearVFlags (MN_TARG);
		int num = mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_TARG, msl, flag);
		if (!num) return false;
	}
	if (smoothness < 1.0f) {
		mesh.DetargetVertsBySharpness (1.0f - smoothness);
	}
	mesh.ClearEFlags (MN_EDGE_NOCROSS|MN_EDGE_MAP_SEAM);
	mesh.SetMapSeamFlags ();
	mesh.FenceOneSidedEdges ();
	if (sepMat) mesh.FenceMaterials ();
	if (sepSmooth) mesh.FenceSmGroups ();
	mesh.SplitFacesUsingBothSidesOfEdge (0);
	mesh.CubicNURMS (NULL, NULL, MN_SUBDIV_NEWMAP);
	mesh.ClearEFlags (MN_EDGE_NOCROSS|MN_EDGE_MAP_SEAM);
	return true;
}

bool EditPolyObject::EpfnMeshSmooth (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore(this);
		tchange->Before ();
	}
	if (!EpfnMeshSmoothMesh (mm, msl, flag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	CollapseDeadFaces ();

	macroRecorder->FunctionCall(_T("$.EditablePoly.meshSmooth"), 1, 0, mr_int, msl);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
	return true;
}

// Luna task 748A - needed for preview.
bool EditPolyObject::EpfnTessellateMesh (MNMesh & mesh, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	float tens;
	int type;
	pblock->GetValue (ep_tess_tension, TimeValue(0), tens, FOREVER);
	pblock->GetValue (ep_tess_type, TimeValue(0), type, FOREVER);
	switch (msl) {
		int i, num;
	case MNM_SL_OBJECT:
		mesh.ClearFFlags (MN_TARG);
		for (i=0; i<mesh.numf; i++) if (!mesh.f[i].GetFlag (MN_DEAD)) mesh.f[i].SetFlag (MN_TARG);
		break;
	case MNM_SL_FACE:
		if ((flag & MN_TARG) == 0) mesh.ClearFFlags (MN_TARG);
		for (i=0; i<mesh.numf; i++) if (mesh.f[i].GetFlag (flag)) break;
		if (i==mesh.numf) return false;
		if (flag != MN_TARG) {
			for (; i<mesh.numf; i++) if (mesh.f[i].GetFlag (flag)) mesh.f[i].SetFlag (MN_TARG);
		}
		break;
	default:
		mesh.ClearFFlags (MN_TARG);
		num = mesh.PropegateComponentFlags (MNM_SL_FACE, MN_TARG, msl, flag);
		if (!num) return false;
	}
	if (type) mesh.TessellateByCenters ();
	else mesh.TessellateByEdges (tens/400.0f);
	return true;
}

bool EditPolyObject::EpfnTessellate (int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore(this);
		tchange->Before ();
	}
	if (!EpfnTessellateMesh (mm, msl, flag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	CollapseDeadFaces ();

	macroRecorder->FunctionCall(_T("$.EditablePoly.tessellate"), 1, 0, mr_int, msl);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
	return true;
}

void EditPolyObject::EpResetSlicePlane () {
	sliceInitialized = true;
	if (theHold.Holding ()) theHold.Put (new TransformPlaneRestore (this));
	Box3 box = mm.getBoundingBox ();
	sliceCenter = (box.pmax + box.pmin)*.5f;
	sliceRot.Identity();
	box.pmax -= box.pmin;
	sliceSize = (box.pmax.x > box.pmax.y) ? box.pmax.x : box.pmax.y;
	if (box.pmax.z > sliceSize) sliceSize = box.pmax.z;
	sliceSize *= .52f;
	if (sliceSize < 1) sliceSize = 1.0f;
	if (mPreviewOn && (mPreviewOperation==epop_slice)) EpPreviewInvalidate ();	// Luna task 748A

	macroRecorder->FunctionCall(_T("$.EditablePoly.resetSlicePlane"), 0, 0);
	macroRecorder->EmitScript ();

	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	RefreshScreen ();
}

void EditPolyObject::EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float *planeSize) {
	Matrix3 rotMatrix;
	sliceRot.MakeMatrix (rotMatrix);
	planeNormal = Point3(0.0f,0.0f,1.0f) * rotMatrix;
	planeCenter = sliceCenter;
	if (planeSize) *planeSize = sliceSize;
}

void EditPolyObject::EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float planeSize) {
	sliceInitialized = true;
	if (theHold.Holding ()) theHold.Put (new TransformPlaneRestore (this));
	sliceCenter = planeCenter;
	sliceSize = planeSize;
	float len = Length (planeNormal);
	Point3 pnorm = planeNormal/len;
	Point3 axis = Normalize(Point3(0,0,1)^pnorm);	// Must normalize to construct quaternion.
	float angle = acosf (pnorm.z);	// pnorm.z is the dot product with (0,0,1).
	sliceRot.Set (AngAxis (axis, -angle));	// Sign change on angle for left-hand-rule (groan)
	if (mPreviewOn && (mPreviewOperation==epop_slice)) EpPreviewInvalidate ();	// Luna task 748A

	macroRecorder->FunctionCall(_T("$.EditablePoly.setSlicePlane"), 3, 0,
		mr_point3, &planeNormal, mr_point3, &planeCenter, mr_float, planeSize);
}

bool EditPolyObject::EpfnSlice (Point3 planeNormal, Point3 planeCenter,
								bool flaggedFacesOnly, DWORD faceFlags) {
	if (flaggedFacesOnly) {
		for (int i=0; i<mm.numf; i++) {
			if (mm.f[i].GetFlag (MN_DEAD)) continue;
			if (mm.f[i].GetFlag (faceFlags)) break;
		}
		if (i==mm.numf) return false;
	}
	float offset = DotProd (planeNormal, planeCenter);
	BOOL sliceSplit;
	pblock->GetValue (ep_split, TimeValue(0), sliceSplit, FOREVER);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!mm.Slice (planeNormal, offset, MNEPS,
		sliceSplit?true:false, FALSE, flaggedFacesOnly, faceFlags)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	if (flaggedFacesOnly) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.slice"), 2, 1,
			mr_point3, &planeNormal, mr_point3, &planeCenter,
			_T("flaggedFacesOnly"), mr_bool, true);
	} else {
		macroRecorder->FunctionCall(_T("$.EditablePoly.slice"), 2, 0,
			mr_point3, &planeNormal, mr_point3, &planeCenter);
	}
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return true;
}

// Luna task 748A - needed for preview.
bool EditPolyObject::EpfnSliceMesh (MNMesh & mesh, int msl, DWORD flag) {
	if (msl == MNM_SL_CURRENT) msl = meshSelLevel[selLevel];
	int i;
	if (msl == MNM_SL_FACE) {	// do flagged faces only.
		for (i=0; i<mesh.numf; i++) {
			if (mesh.f[i].GetFlag (MN_DEAD)) continue;
			if (mesh.f[i].GetFlag (flag)) break;
		}
		if (i==mesh.numf) return false;
	}
	Point3 planeNormal, planeCenter;
	EpGetSlicePlane (planeNormal, planeCenter);
	float offset = DotProd (planeNormal, planeCenter);
	BOOL sliceSplit;
	pblock->GetValue (ep_split, TimeValue(0), sliceSplit, FOREVER);
	bool ret = mesh.Slice (planeNormal, offset, MNEPS,
		sliceSplit?true:false, false, (msl==MNM_SL_FACE)?true:false, flag);
	return ret;
}

// NOTE: copied over from MNMesh::Cut code.
const float kCUT_EPS = .001f;

// Luna task 748A - needed for preview.
int EditPolyObject::EpfnCutMesh (MNMesh & mesh) {
	int cutSplit, cutStartLevel, cutStartIndex;
	Point3 cutStart, cutEnd, cutNormal;
	pblock->GetValue (ep_cut_start_level, TimeValue(0), cutStartLevel, FOREVER);
	pblock->GetValue (ep_cut_start_index, TimeValue(0), cutStartIndex, FOREVER);
	pblock->GetValue (ep_cut_start_coords, TimeValue(0), cutStart, FOREVER);
	pblock->GetValue (ep_cut_end_coords, TimeValue(0), cutEnd, FOREVER);
	pblock->GetValue (ep_cut_normal, TimeValue(0), cutNormal, FOREVER);
	pblock->GetValue (ep_split, TimeValue(0), cutSplit, FOREVER);

	int endVertex = -1;
	Point3 edgeDirection;
	Point3 startDirection;

	switch (cutStartLevel) {
	case MNM_SL_OBJECT:
		break;

	case MNM_SL_VERTEX:
		endVertex = mesh.Cut (cutStartIndex, cutEnd, cutNormal, cutSplit?true:false);
		break;

	case MNM_SL_EDGE:
		// Find proportion along edge for starting point:
		float prop;
		edgeDirection = mesh.v[mesh.e[cutStartIndex].v2].p - mesh.v[mesh.e[cutStartIndex].v1].p;
		float edgeLen2;
		edgeLen2 = LengthSquared (edgeDirection);
		startDirection = cutStart - mesh.v[mesh.e[cutStartIndex].v1].p;
		if (edgeLen2) prop = DotProd (startDirection, edgeDirection) / edgeLen2;
		else prop = 0.0f;
		int nv;
		if (prop < kCUT_EPS) nv = mesh.e[cutStartIndex].v1;
		else {
			if (prop > 1-kCUT_EPS) nv = mesh.e[cutStartIndex].v2;
			else {
				// If we need to cut the initial edge, do so before starting main cut routine:
				nv = mesh.SplitEdge (cutStartIndex, prop);
			}
		}
		endVertex = mesh.Cut (nv, cutEnd, cutNormal, cutSplit?true:false);
		break;

	case MNM_SL_FACE:
		endVertex = mesh.CutFace (cutStartIndex, cutStart, cutEnd, cutNormal, cutSplit?true:false);
		break;
	}

	return endVertex;
}

int EditPolyObject::EpfnCutVertex (int startv, Point3 destination, Point3 projDir) {
	int splitme;
	pblock->GetValue (ep_split, TimeValue(0), splitme, FOREVER);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int ret = mm.Cut (startv, destination, projDir, splitme?true:false);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.cutVertices"), 3, 0,
		mr_int, startv+1, mr_point3, &destination, mr_point3, &projDir);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return ret;
}

int EditPolyObject::EpfnCutEdge (int e1, float prop1, int e2, float prop2, Point3 projDir) {
	int splitme;
	pblock->GetValue (ep_split, TimeValue(0), splitme, FOREVER);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int vret;
	vret = mm.CutEdge (e1, prop1, e2, prop2, projDir, splitme?true:false);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.cutEdges"), 5, 0,
		mr_int, e1+1, mr_float, prop1, mr_int, e2+1, mr_float, prop2, mr_point3, &projDir);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return vret;
}

int EditPolyObject::EpfnCutFace (int f1, Point3 start, Point3 dest, Point3 projDir) {
	int splitme;
	pblock->GetValue (ep_split, TimeValue(0), splitme, FOREVER);
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	f1 = mm.CutFace (f1, start, dest, projDir, splitme?true:false);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.cutFaces"), 4, 0,
		mr_int, f1+1, mr_point3, &start, mr_point3, &dest, mr_point3, &projDir);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return f1;
}

bool EditPolyObject::EpfnWeldVerts (int v1, int v2, Point3 destination) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore(this);
		tchange->Before();
	}
	// If vertices v1 and v2 share an edge, then take a collapse type approach;
	// Otherwise, weld them if they're suitable (open verts, etc.)
	int i;
	bool ret=false;
	for (i=0; i<mm.vedg[v1].Count(); i++) {
		if (mm.e[mm.vedg[v1][i]].OtherVert(v1) == v2) break;
	}
	if (i<mm.vedg[v1].Count()) {
		ret = mm.WeldEdge (mm.vedg[v1][i]);
		if (mm.v[v1].GetFlag (MN_DEAD)) mm.v[v2].p = destination;
		else mm.v[v1].p = destination;
	} else {
		ret = mm.WeldBorderVerts (v1, v2, &destination);
	}
	if (!ret) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After();
		theHold.Put (tchange);
	}
	CollapseDeadStructs ();

	macroRecorder->FunctionCall(_T("$.EditablePoly.weldVerts"), 3, 0,
		mr_int, v1+1, mr_int, v2+1, mr_point3, &destination);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return true;
}

bool EditPolyObject::EpfnWeldEdges (int e1, int e2) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore(this);
		tchange->Before();
	}
	bool ret = mm.WeldBorderEdges (e1, e2);
	if (!ret) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After();
		theHold.Put (tchange);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.weldEdges"), 2, 0,
		mr_int, e1+1, mr_int, e2+1);
	macroRecorder->EmitScript ();

	CollapseDeadStructs ();
	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return true;
}

// Luna task 748A - needed for preview.
bool EditPolyObject::EpfnWeldMeshVerts (MNMesh & mesh, DWORD flag) {
	bool returnValue = false;
	int i;
	float thresh;
	pblock->GetValue (ep_weld_threshold, TimeValue(0), thresh, FOREVER);
	if (mesh.WeldBorderVerts (thresh, flag)) returnValue = true;

	// Check for collapsable vertices:
	// (This code was copied from EpfnCollapse.)
	// In order to collapse vertices, we find all edges shorter than the weld threshold.
	mesh.ClearEFlags (MN_USER);
	float threshSq = thresh*thresh;
	for (i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.v[mesh.e[i].v1].GetFlag (flag)) continue;
		if (!mesh.v[mesh.e[i].v2].GetFlag (flag)) continue;
		if (LengthSquared (mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2)) > threshSq) continue;
		mesh.e[i].SetFlag (MN_USER);
	}

	returnValue |= MNMeshCollapseEdges (mesh, MN_USER);
	return returnValue;
}

bool EditPolyObject::EpfnWeldFlaggedVerts (DWORD flag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!EpfnWeldMeshVerts (mm, flag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	CollapseDeadStructs ();

	macroRecorder->FunctionCall(_T("$.EditablePoly.weldFlaggedVertices"), 0, 0);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL-PART_SUBSEL_TYPE);
	return true;
}

// Luna task 748A - needed for preview.
bool EditPolyObject::EpfnWeldMeshEdges (MNMesh & mesh, DWORD flag) {
	float thresh;
	pblock->GetValue (ep_edge_weld_threshold, TimeValue(0), thresh, FOREVER);

	// Mark the corresponding vertices:
	mesh.ClearVFlags (MN_USER);
	mesh.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_EDGE, flag);

	// Weld border edges together via border vertices:
	mesh.ClearVFlags (MN_VERT_WELDED);
	bool ret = mesh.WeldBorderVerts (thresh, MN_USER);
	ret |= mesh.WeldOpposingEdges (flag);
	if (!ret) return false;

	// Check for edges for which one end has been welded but not the other.
	// This is made possible by the MN_VERT_WELDED flag which is set in WeldBorderVerts.
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.e[i].GetFlag (flag)) continue;
		int v1 = mesh.e[i].v1;
		int v2 = mesh.e[i].v2;

		// If both were welded, or both were not welded, continue.
		if (mesh.v[v1].GetFlag (MN_VERT_WELDED) == mesh.v[v2].GetFlag (MN_VERT_WELDED)) continue;

		// Ok, one's been welded but the other hasn't.  See if there's another weld we can do to seal it up.
		int weldEnd = mesh.v[v1].GetFlag (MN_VERT_WELDED) ? 0 : 1;
		int unweldEnd = 1-weldEnd;
		int va = mesh.e[i][weldEnd];
		int vb = mesh.e[i][unweldEnd];

		// First of all, if the welded vert only has 2 flagged, open edges, it's clear they should be welded together.
		// Use this routine to find all flagged, open edges touching va that *aren't* i:
		Tab<int> elist;
		for (int j=0; j<mesh.vedg[va].Count(); j++) {
			int eid = mesh.vedg[va][j];
			if (eid == i) continue;
			if (mesh.e[eid].f2 > -1) continue;
			if (!mesh.e[eid].GetFlag (flag)) continue;
			if (mesh.e[eid][unweldEnd] != va) continue;	// should have va at the opposite end from edge i.
			elist.Append (1, &eid);
		}
		if (elist.Count() != 1) {
			// Give up for now.  (Perhaps make better solution later.)
			continue;
		}

		// Otherwise, there's exactly one, and we'll want to weld it to edge i:
		int vc = mesh.e[elist[0]].OtherVert (va);
		if (mesh.v[vc].GetFlag (MN_VERT_WELDED)) continue;	// might happen if vc already welded.

		// Ok, now we know which vertices to weld.
		Point3 dest = (mesh.v[vb].p + mesh.v[vc].p)*.5f;
		if (mesh.WeldBorderVerts (vb, vc, &dest)) mesh.v[vb].SetFlag (MN_VERT_WELDED);
	}

	return true;
}

bool EditPolyObject::EpfnWeldFlaggedEdges (DWORD flag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!EpfnWeldMeshEdges (mm, flag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	CollapseDeadStructs ();

	macroRecorder->FunctionCall(_T("$.EditablePoly.weldFlaggedEdges"), 0, 0);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_ALL - PART_SUBSEL_TYPE);
	return true;
}

// Luna task 748Q
bool EditPolyObject::EpfnConnectMeshEdges (MNMesh & mesh, DWORD edgeFlag) {
	int connectSegs;
	pblock->GetValue (ep_connect_edge_segments, TimeValue(0), connectSegs, FOREVER);
	return mesh.ConnectEdges (edgeFlag, connectSegs);
}

bool EditPolyObject::EpfnConnectEdges (DWORD edgeFlag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int connectSegs;
	pblock->GetValue (ep_connect_edge_segments, TimeValue(0), connectSegs, FOREVER);
	if (!mm.ConnectEdges (edgeFlag, connectSegs)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	LocalDataChanged (PART_ALL);
	macroRecorder->FunctionCall(_T("$.EditablePoly.ConnectEdges"), 0, 0);
	macroRecorder->EmitScript ();
	return true;
}

bool EditPolyObject::EpfnConnectMeshVertices (MNMesh & mesh, DWORD vertexFlag) {
	return mesh.ConnectVertices (vertexFlag);
}

bool EditPolyObject::EpfnConnectVertices (DWORD vertexFlag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!mm.ConnectVertices (vertexFlag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	LocalDataChanged (PART_ALL);
	macroRecorder->FunctionCall(_T("$.EditablePoly.ConnectVertices"), 0, 0);
	macroRecorder->EmitScript ();
	return true;
}

// Luna task 748T
bool EditPolyObject::EpfnExtrudeAlongSplineMesh (MNMesh & mesh, MNTempData *pTempData, DWORD faceFlag) {
	if (!ip) return false;
	// First get the Spline.
	TimeValue t = ip->GetTime();
	INode *pSplineNode = NULL;
	pblock->GetValue (ep_extrude_spline_node, t, pSplineNode, FOREVER);
	if (!pSplineNode) return false;
	BOOL del = FALSE;
	SplineShape *pSplineShape = NULL;
	ObjectState os = pSplineNode->GetObjectRef()->Eval(t);
	if (os.obj->IsSubClassOf(splineShapeClassID)) pSplineShape = (SplineShape *) os.obj;
	else {
		if (!os.obj->CanConvertToType(splineShapeClassID)) return false;
		pSplineShape = (SplineShape*)os.obj->ConvertToType (t, splineShapeClassID);
		if (pSplineShape!=os.obj) del = TRUE;
	}
	BezierShape & bezShape = pSplineShape->GetShape();

	// Find the transform that maps the spline's object space to ours:
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);	// nodes[0] is this EPoly's node.
	Matrix3 relativeTransform = pSplineNode->GetObjectTM(ip->GetTime()) *
		Inverse(nodes[0]->GetObjectTM(ip->GetTime()));
	nodes.DisposeTemporary ();

	// Get other parameters:
	int segments, align;
	float taper, taperCurve, twist, rotation;
	pblock->GetValue (ep_extrude_spline_segments, TimeValue (0), segments, FOREVER);
	pblock->GetValue (ep_extrude_spline_taper, TimeValue (0), taper, FOREVER);
	pblock->GetValue (ep_extrude_spline_taper_curve, TimeValue(0), taperCurve, FOREVER);
	pblock->GetValue (ep_extrude_spline_twist, TimeValue(0), twist, FOREVER);
	pblock->GetValue (ep_extrude_spline_rotation, TimeValue(0), rotation, FOREVER);
	pblock->GetValue (ep_extrude_spline_align, TimeValue(0), align, FOREVER);

	Tab<Matrix3> tTransforms;
	ConvertPathToFrenets (bezShape.GetSpline(0), relativeTransform, tTransforms, segments, align?true:false, rotation);

	if (del) delete pSplineShape;

	// Apply taper and twist to transforms:
	float denom = float(tTransforms.Count()-1);
	for (int i=1; i<tTransforms.Count(); i++) {
		float amount = float(i)/denom;
		// This equation taken from Taper modifier:
		float taperAmount =	1.0f + amount*taper + taperCurve*amount*(1.0f-amount);
		if (taperAmount != 1.0f) {
			// Pre-scale matrix by taperAmount.
			tTransforms[i].PreScale (Point3(taperAmount, taperAmount, taperAmount));
		}
		if (twist != 0.0f) {
			float twistAmount = twist * amount;
			tTransforms[i].PreRotateZ (twistAmount);
		}
	}

	// Note:
	// If there are multiple face clusters, the first call to ExtrudeFaceClusterAlongPath
	// will bring mesh.numf and fclust->clust.Count() out of synch - fclust isn't updated.
	// So we fix that here.
	bool ret=false;
	MNFaceClusters *fclust = pTempData->FaceClusters (faceFlag);
	for (i=0; i<fclust->count; i++) {
		if (mesh.ExtrudeFaceClusterAlongPath (tTransforms, *fclust, i, align?true:false)) {
			ret = true;
			if (i+1<fclust->count) {
				// New faces not in any cluster.
				int oldnumf = fclust->clust.Count();
				fclust->clust.SetCount (mesh.numf);
				for (int j=oldnumf; j<mesh.numf; j++) fclust->clust[j] = -1;
			}
		}
	}

	if (ret) pTempData->Invalidate (PART_ALL);

	return ret;
}

bool EditPolyObject::EpfnExtrudeAlongSpline (DWORD faceFlag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!EpfnExtrudeAlongSplineMesh (mm, TempData(), faceFlag)) {
		if (tchange) delete tchange;
		return false;
	}

	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.ExtrudeAlongSpline"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_ALL);
	return true;
}

// Luna task 748P 
bool EditPolyObject::EpfnLiftFromEdgeMesh (MNMesh & mesh, MNTempData *pTemp, DWORD faceFlag) {
	int liftEdge, segments;
	float liftAngle;
	pblock->GetValue (ep_lift_edge, TimeValue(0), liftEdge, FOREVER);
	pblock->GetValue (ep_lift_angle, TimeValue(0), liftAngle, FOREVER);
	pblock->GetValue (ep_lift_segments, TimeValue(0), segments, FOREVER);

	if ((liftEdge<0) || (liftEdge>=mesh.nume)) return false;	// value of -1: "Undefined".
	if (mesh.e[liftEdge].GetFlag(MN_DEAD)) return false;

	// Find cluster of faces that touch this edge:
	MNFaceClusters *pClusters = pTemp->FaceClusters (faceFlag);
	bool ret = false;
	for (int i=0; i<pClusters->count; i++) {
		if (mesh.LiftFaceClusterFromEdge (liftEdge, liftAngle, segments, *pClusters, i)) {
			ret = true;
			if (i+1<pClusters->count) {
				// New faces not in any cluster.
				int oldnumf = pClusters->clust.Count();
				pClusters->clust.SetCount (mesh.numf);
				for (int j=oldnumf; j<mesh.numf; j++) pClusters->clust[j] = -1;
			}
	}
	}

	if (ret) pTemp->Invalidate (PART_ALL);

	return ret;
}

bool EditPolyObject::EpfnLiftFromEdge (DWORD faceFlag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	if (!EpfnLiftFromEdgeMesh (mm, TempData(), faceFlag)) {
		if (tchange) delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}
	macroRecorder->FunctionCall(_T("$.EditablePoly.HingeFromEdge"), 0, 0);
	macroRecorder->EmitScript ();
	LocalDataChanged (PART_ALL);
	return true;
}

void EditPolyObject::EpfnAutoSmooth (TimeValue t) {
	for (int i=0; i<mm.numf; i++) if (mm.f[i].FlagMatch (MN_DEAD|MN_SEL, MN_SEL)) break;
	if (i==mm.numf) return;
	float angle;
	pblock->GetValue (ep_face_smooth_thresh, t, angle, FOREVER);
	SmGroupRestore *smr = NULL;
	if (theHold.Holding ()) smr = new SmGroupRestore (this);
	mm.AutoSmooth (angle, true, false);
	if (smr) {
		smr->After ();
		theHold.Put (smr);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.autosmooth"), 0, 0);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO);
}

class ToggleShadedRestore : public RestoreObj {
	INode *mpNode;
	bool mOldShowVertCol, mOldShadedVertCol, mNewShowVertCol;
	int mOldVertexColorType;

public:
	ToggleShadedRestore (INode *pNode, bool newShow);
	void Restore(int isUndo);
	void Redo();
	int Size() { return sizeof (void *) + 3*sizeof(bool) + sizeof(int); }
	TSTR Description() { return TSTR(_T("ToggleShadedRestore")); }
};

ToggleShadedRestore::ToggleShadedRestore (INode *pNode, bool newShow) : mpNode(pNode), mNewShowVertCol(newShow) {
	mOldShowVertCol = mpNode->GetCVertMode() ? true : false;
	mOldShadedVertCol = mpNode->GetShadeCVerts() ? true : false;
	mOldVertexColorType = mpNode->GetVertexColorType ();
}

void ToggleShadedRestore::Restore (int isUndo) {
	mpNode->SetCVertMode (mOldShowVertCol);
	mpNode->SetShadeCVerts (mOldShadedVertCol);
	mpNode->SetVertexColorType (mOldVertexColorType);
	mpNode->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void ToggleShadedRestore::Redo () {
	mpNode->SetCVertMode (mNewShowVertCol);
	mpNode->SetShadeCVerts (true);
	mpNode->SetVertexColorType (nvct_soft_select);
	mpNode->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void EditPolyObject::EpfnToggleShadedFaces () {
	if (!ip) return;
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList,nodes);	// nodes[0] is this EPoly's node.

	int oldShow, oldShaded, oldType;
	oldShow = nodes[0]->GetCVertMode ();
	oldShaded = nodes[0]->GetShadeCVerts ();
	oldType = nodes[0]->GetVertexColorType ();

	// If we have anything other than our perfect (true, true, nvct_soft_select) combo, set to that combo.
	// Otherwise, turn off shading.
	if (oldShow && oldShaded && (oldType == nvct_soft_select)) {
		if (theHold.Holding ()) theHold.Put (new ToggleShadedRestore(nodes[0], false));
		nodes[0]->SetCVertMode (false);
	} else {
		if (theHold.Holding ()) theHold.Put (new ToggleShadedRestore(nodes[0], true));
		nodes[0]->SetCVertMode (true);
		nodes[0]->SetShadeCVerts (true);
		nodes[0]->SetVertexColorType (nvct_soft_select);
	}
	nodes[0]->NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);

	macroRecorder->FunctionCall(_T("$.EditablePoly.ToggleShadedFaces"), 0, 0);
	macroRecorder->EmitScript ();

	// KLUGE: above notify dependents call only seems to set a limited refresh region.  Result looks bad.
	// So we do this too:
	NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

void EditPolyObject::EpfnSetDiagonal (int face, int corner1, int corner2) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	mm.SetDiagonal (face, corner1, corner2);
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.setDiagonal"), 3, 0,
		mr_int, face+1, mr_int, corner1+1, mr_int, corner2+1);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO|PART_SELECT);
}

bool EditPolyObject::EpfnRetriangulate (DWORD flag) {
//	if (meshSelLevel[selLevel] != MNM_SL_FACE) return false;
	bool ret = false;
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding ()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}
	int i;
	for (i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		if (!mm.f[i].GetFlag (flag)) continue;
		ret = true;
		mm.RetriangulateFace (i);
	}
	if (!ret) {
		delete tchange;
		return false;
	}
	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.retriangulate"), 1, 0, mr_int, flag);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO);
	return true;
}

bool EditPolyObject::EpfnFlipNormals (DWORD flag) {
	TopoChangeRestore *tchange = NULL;
	if (theHold.Holding()) {
		tchange = new TopoChangeRestore (this);
		tchange->Before ();
	}

	bool ret=false;
	switch (selLevel) {
	// Luna task 748I: can now flip non-element sets of faces.
	case EP_SL_FACE:
		ret = mm.FlipFaceNormals (flag);
		break;
	case EP_SL_ELEMENT:
		ret = mm.FlipElementNormals (flag);
		break;
	}

	if (!ret) {
		if (tchange) delete tchange;
		return false;
	}

	if (tchange) {
		tchange->After ();
		theHold.Put (tchange);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.flipNormals"), 1, 0, mr_int, flag);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO);
	return true;
}

// Vertex / Edge data operations:

float EditPolyObject::GetVertexDataValue (int channel, int *numSel, bool *uniform, DWORD flags, TimeValue t) {
	if (numSel) *numSel = 0;
	if (uniform) *uniform = TRUE;
	float ret=1.0f;
	float *vd=NULL;
	int i, found=0;
	memcpy (&ret, VertexDataDefault(channel), sizeof(float));
	vd = mm.vertexFloat (channel);
	for (i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (!mm.v[i].GetFlag (flags)) continue;
		if (!found && vd) ret = vd[i];
		found++;
		if (vd && (ret != vd[i])) break;
	}
	if (uniform && (i<mm.numv)) *uniform = FALSE;
	if (numSel) *numSel = found;
	return ret;
}

float EditPolyObject::GetEdgeDataValue (int channel, int *numSel, bool *uniform, DWORD flags, TimeValue t) {
	if (numSel) *numSel = 0;
	if (uniform) *uniform = TRUE;
	float ret=1.0f;
	float *vd=NULL;
	int i, found=0;
	memcpy (&ret, EdgeDataDefault(channel), sizeof(float));
	vd = mm.edgeFloat (channel);
	for (i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!mm.e[i].GetFlag (flags)) continue;
		if (!found && vd) ret = vd[i];
		found++;
		if (vd && (ret != vd[i])) break;
	}
	if (uniform && (i<mm.nume)) *uniform = FALSE;
	if (numSel) *numSel = found;
	return ret;
}

bool HasMinimum (int msl, int channel, float *minim) {
	switch (msl) {
	case MNM_SL_VERTEX:
		switch (channel) {
		case VDATA_WEIGHT:
			*minim = MIN_WEIGHT;
			return TRUE;
		}
		break;
	case MNM_SL_EDGE:
		switch (channel) {
		case EDATA_KNOT:
			*minim = MIN_WEIGHT;
			return TRUE;
		case EDATA_CREASE:
			*minim = 0.0f;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

bool HasMaximum (int msl, int channel, float *maxim) {
	switch (msl) {
	case MNM_SL_VERTEX:
		switch (channel) {
		case VDATA_WEIGHT:
			*maxim = MAX_WEIGHT;
			return TRUE;
		}
		break;
	case MNM_SL_EDGE:
		switch (channel) {
		case EDATA_KNOT:
			*maxim = MAX_WEIGHT;
			return TRUE;
		case EDATA_CREASE:
			*maxim = 1.0f;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static PerDataRestore *perDataRestore = NULL;
void EditPolyObject::BeginPerDataModify (int mnSelLevel, int channel) {
	if (mnSelLevel == MNM_SL_CURRENT) mnSelLevel = meshSelLevel[selLevel];
	if (perDataRestore) delete perDataRestore;
	perDataRestore = new PerDataRestore (this, mnSelLevel, channel);
}

bool EditPolyObject::InPerDataModify () {
	return (perDataRestore) ? true : false;
}

void EditPolyObject::EndPerDataModify (bool success) {
	if (!perDataRestore) return;
	if (success) {
		if (theHold.Holding()) theHold.Put (perDataRestore);
		else delete perDataRestore;
	} else {
		perDataRestore->Restore (FALSE);
		delete perDataRestore;
	}
	perDataRestore = NULL;
}

static MapChangeRestore *mchange = NULL;
void EditPolyObject::BeginVertexColorModify (int mapChannel) {
	if ((mm.numm <= mapChannel) || mm.M(mapChannel)->GetFlag (MN_DEAD)) InitVertColors (mapChannel);
	if (mchange) delete mchange;
	mchange = new MapChangeRestore (this, mapChannel);
}

bool EditPolyObject::InVertexColorModify () {
	return mchange ? true : false;
}

void EditPolyObject::EndVertexColorModify (bool success) {
	if (!mchange) return;
	if (success) {
		if (theHold.Holding()) {
			if (mchange->After ()) theHold.Put (mchange);
			else delete mchange;
		}
		else delete mchange;
	} else {
		mchange->Restore (FALSE);
		delete mchange;
	}
	mchange = NULL;
}

void EditPolyObject::SetVertexDataValue (int channel, float value, DWORD flags, TimeValue t) {
	if (!perDataRestore) BeginPerDataModify (MNM_SL_VERTEX, channel);
	float minim=0.0f, maxim=1.0f;
	if (HasMinimum (MNM_SL_VERTEX, channel, &minim)) {
		if (value < minim) value = minim;
	}
	if (HasMaximum (MNM_SL_VERTEX, channel, &maxim)) {
		if (value > maxim) value = maxim;
	}

	int i;
	bool change=false;
	float *defaultValue = (float *) VertexDataDefault (channel);
	bool setToDefault = (value == *defaultValue);
	float *vd = mm.vertexFloat (channel);
	if (!vd) {
		if (setToDefault) return;
		mm.setVDataSupport (channel, TRUE);
		vd = mm.vertexFloat (channel);
	}
	bool allFlagged = true;
	for (i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (!mm.v[i].GetFlag (flags)) {
			allFlagged = false;
			continue;
		}
		vd[i] = value;
		change=true;
	}
	if (setToDefault && allFlagged) mm.setVDataSupport (channel, false);
	if (change) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.setVertexData"), 2, 0,
			mr_int, channel, mr_float, value);
		LocalDataChanged (PART_GEOM);
		RefreshScreen ();
	}
}

void EditPolyObject::SetEdgeDataValue (int channel, float value, DWORD flags, TimeValue t) {
	if (!perDataRestore) BeginPerDataModify (MNM_SL_EDGE, channel);
	float minim=0.0f, maxim=1.0f;
	if (HasMinimum (MNM_SL_EDGE, channel, &minim)) {
		if (value < minim) value = minim;
	}
	if (HasMaximum (MNM_SL_EDGE, channel, &maxim)) {
		if (value > maxim) value = maxim;
	}

	int i;
	bool change=false;
	float *defaultValue = (float *) EdgeDataDefault (channel);
	bool setToDefault = (value == *defaultValue);
	float *vd = mm.edgeFloat (channel);
	if (!vd) {
		if (setToDefault) return;
		mm.setEDataSupport (channel, TRUE);
		vd = mm.edgeFloat (channel);
	}
	bool allFlagged = true;
	for (i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!mm.e[i].GetFlag (flags)) {
			allFlagged = false;
			continue;
		}
		vd[i] = value;
		change=true;
	}
	if (setToDefault && allFlagged) mm.setEDataSupport (channel, false);
	if (change) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.setEdgeData"), 2, 0,
			mr_int, channel, mr_float, value);
		LocalDataChanged (PART_TOPO);
		RefreshScreen ();
	}
}

void EditPolyObject::ResetVertexData (int channel) {
	mm.freeVData (channel);
	LocalDataChanged (PART_GEOM);
	macroRecorder->FunctionCall(_T("$.EditablePoly.resetVertexData"), 1, 0,
		mr_int, channel);
	macroRecorder->EmitScript ();
}

void EditPolyObject::ResetEdgeData (int channel) {
	mm.freeEData (channel);
	LocalDataChanged (PART_TOPO);
	macroRecorder->FunctionCall(_T("$.EditablePoly.resetEdgeData"), 1, 0,
		mr_int, channel);
	macroRecorder->EmitScript ();
}

void EditPolyObject::InitVertColors (int mapChannel) {
	if (theHold.Holding ()) theHold.Put (new InitVertColorRestore(this, mapChannel));
	if (mapChannel>=mm.numm) mm.SetMapNum (mapChannel+1);
	mm.M(mapChannel)->ClearFlag (MN_DEAD);
	mm.M(mapChannel)->setNumFaces (mm.numf);
	mm.M(mapChannel)->setNumVerts (mm.numv);
	for (int i=0; i<mm.numv; i++) mm.M(mapChannel)->v[i] = UVVert(1,1,1);
	for (i=0; i<mm.numf; i++) mm.M(mapChannel)->f[i] = mm.f[i];
	LocalDataChanged (PART_VERTCOLOR);
}

Color EditPolyObject::GetVertexColor (bool *uniform, int *num, int mapChannel, DWORD flags, TimeValue t) {
	static Color white(1,1,1), black(0,0,0);
	int i;
	Color col=white;
	bool init=false;
	if (uniform) *uniform = true;
	if (num) *num = 0;

	if ((mm.numm <= mapChannel) || (mm.M(mapChannel)->GetFlag (MN_DEAD))) {
		if (num) {
			for (i=0; i<mm.numv; i++) {
				if (mm.v[i].GetFlag (MN_DEAD)) continue;
				if (mm.v[i].GetFlag (flags)) (*num)++;
			}
		}
		return white;
	}
	MNMapFace *cf = mm.M(mapChannel)->f;
	UVVert *cv = mm.M(mapChannel)->v;
	if (!cf || !cv) {
		if (num) {
			for (i=0; i<mm.numv; i++) {
				if (mm.v[i].GetFlag (MN_DEAD)) continue;
				if (mm.v[i].GetFlag (flags)) (*num)++;
			}
		}
		return white;
	}

	for (i=0; i<mm.numf; i++) {
		int *tt = cf[i].tv;
		int *vv = mm.f[i].vtx;
		for (int j=0; j<mm.f[i].deg; j++) {
			if (!mm.v[vv[j]].GetFlag (flags)) continue;
			if (num) (*num)++;
			if (!init) {
				col = cv[tt[j]];
				init = TRUE;
			} else {
				Color ac = cv[tt[j]];
				if (ac == col) continue;
				// Otherwise, check for floating-pt errors:
				Color diff = ac - col;
				if (fabsf(diff.r) + fabsf(diff.g) + fabsf(diff.b) < 2.0e-07f) continue;
				// Ok, we'll call these different colors:
				if (uniform) *uniform = false;
				return black;
			}
		}
	}
	return col;
}

void EditPolyObject::SetVertexColor (Color clr, int mapChannel, DWORD flags, TimeValue t) {
	if (!mchange) BeginVertexColorModify (mapChannel);
	for (int i=0; i<mm.numv; i++) {
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (mm.v[i].GetFlag (flags)) break;
	}
	if (i>=mm.numv) return;
	if ((mm.numm <= mapChannel) || mm.M(mapChannel)->GetFlag (MN_DEAD)) InitVertColors (mapChannel);
	UVVert uvColor(clr.r, clr.g, clr.b);
	mm.SetVertColor (uvColor, mapChannel, flags);
	LocalDataChanged ((mapChannel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	RefreshScreen ();
}

void EditPolyObject::EpfnSelectVertByColor (BOOL add, BOOL sub, int mapChannel, TimeValue t) {
	MNMapFace *cf;
	UVVert *cv;
	if ((mapChannel>=mm.numm) || (mapChannel<-NUM_HIDDENMAPS) || mm.M(mapChannel)->GetFlag (MN_DEAD)) {
		cf = NULL;
		cv = NULL;
	} else {
		cf = mm.M(mapChannel)->f;
		cv = mm.M(mapChannel)->v;
	}

	int deltaR, deltaG, deltaB;
	Color selColor;
	pblock->GetValue (ep_vert_sel_color, t, selColor, FOREVER);
	pblock->GetValue (ep_vert_selc_r, t, deltaR, FOREVER);
	pblock->GetValue (ep_vert_selc_g, t, deltaG, FOREVER);
	pblock->GetValue (ep_vert_selc_b, t, deltaB, FOREVER);

	UVVert clr(selColor.r, selColor.g, selColor.b);
	float dr = float(deltaR)/255.0f;
	float dg = float(deltaG)/255.0f;
	float db = float(deltaB)/255.0f;

	theHold.Begin();

	BitArray nvs;
	if (add || sub) {
		nvs = GetVertSel ();
		nvs.SetSize (mm.numv, TRUE);
	} else {
		nvs.SetSize (mm.numv);
		nvs.ClearAll();
	}

	Point3 col(1,1,1);
	for (int i=0; i<mm.numf; i++) {
		for (int j=0; j<mm.f[i].deg; j++) {
			if (cv && cf) col = cv[cf[i].tv[j]];
			if ((float)fabs(col.x-clr.x) > dr) continue;
			if ((float)fabs(col.y-clr.y) > dg) continue;
			if ((float)fabs(col.z-clr.z) > db) continue;
			if (sub) nvs.Clear(mm.f[i].vtx[j]);
			else nvs.Set(mm.f[i].vtx[j]);
		}
	}
	SetVertSel (nvs, this, t);
	theHold.Accept (GetString (IDS_SEL_BY_COLOR));
	LocalDataChanged ();
	RefreshScreen ();
}

Color EditPolyObject::GetFaceColor (bool *uniform, int *num, int mapChannel, DWORD flags, TimeValue t) {
	static Color white(1,1,1), black(0,0,0);
	if (uniform) *uniform = true;
	if (num) *num = 0;
	int i;
	if ((mapChannel>=mm.numm) || (mapChannel<-NUM_HIDDENMAPS) || (mm.M(mapChannel)->GetFlag (MN_DEAD))) {
		if (num) {
			for (i=0; i<mm.numf; i++) {
				if (!mm.f[i].GetFlag (MN_DEAD) && mm.f[i].GetFlag (flags)) (*num)++;
			}
		}
		return white;
	}
	BOOL init=FALSE;
	Color col=white;

	MNMapFace *cf = mm.M(mapChannel)->f;
	UVVert *cv = mm.M(mapChannel)->v;

	for (i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		if (!mm.f[i].GetFlag (flags)) continue;
		if (num) (*num)++;
		int *tt = cf[i].tv;
		for (int j=0; j<cf[i].deg; j++) {
			if (!init) {
				col = cv[tt[j]];
				init = TRUE;
			} else {
				Color ac = cv[tt[j]];
				if (ac == col) continue;
				// Otherwise, check for floating-pt errors:
				Color diff = ac - col;
				if (fabsf(diff.r) + fabsf(diff.g) + fabsf(diff.b) < 2.0e-07f) continue;
				// Ok, we'll call these different colors:
				if (uniform) *uniform = false;
				return black;
			}
		}
	}
	return col;
}

void EditPolyObject::SetFaceColor (Color clr, int mapChannel, DWORD flags, TimeValue t) {
	if (!mchange) BeginVertexColorModify (mapChannel);
	for (int i=0; i<mm.numf; i++) {
		if (!mm.f[i].GetFlag (MN_DEAD) && mm.f[i].GetFlag (flags)) break;
	}
	if (i>=mm.numf) return;
	if ((mm.numm <= mapChannel) || mm.M(mapChannel)->GetFlag (MN_DEAD)) InitVertColors (mapChannel);
	UVVert uvColor (clr.r, clr.g, clr.b);
	mm.SetFaceColor (uvColor, mapChannel, flags);
	LocalDataChanged ((mapChannel<1) ? PART_VERTCOLOR : PART_TEXMAP);
	RefreshScreen ();
}

// Face surface operations:
int EditPolyObject::GetMatIndex(bool * determined) {
	MtlID mat = 0;
	*determined = false;
	for (int j=0; j<mm.numf; j++) {
		if (!mm.f[j].FlagMatch (MN_SEL|MN_DEAD, MN_SEL)) continue;
		if (!(*determined)) {
			mat = mm.f[j].material;
			*determined = true;
		} else if (mm.f[j].material != mat) {
			*determined = false;
			return mat;
		}
	}
	return mat;
}

// Note: good reasons for handling theHold.Begin/Accept at higher level.
void EditPolyObject::SetMatIndex (int index, DWORD flags) {
	if (theHold.Holding ()) {
		theHold.Put (new MtlIDRestore (this));
	}
	for (int j=0; j<mm.numf; j++) {
		if (!mm.f[j].FlagMatch (flags|MN_DEAD, flags)) continue;
		mm.f[j].material = MtlID(index);
	}

	macroRecorder->FunctionCall(_T("$.EditablePoly.setMaterialIndex"), 1, 0,
		mr_int, index+1, mr_int, flags);

	LocalDataChanged (PART_TOPO);
	RefreshScreen ();
}

void EditPolyObject::EpfnSelectByMat (int index, bool clear, TimeValue t) {
	MtlID matIndex = MtlID(index);
	BitArray ns;
	if (clear) {
		ns.SetSize (mm.numf);
		ns.ClearAll ();
	} else {
		ns = GetFaceSel ();
	}
	for (int j=0; j<mm.numf; j++) {
		if (mm.f[j].GetFlag (MN_DEAD)) continue;
		if (mm.f[j].material == matIndex) ns.Set(j);
	}
	SetFaceSel (ns, this, t);

	if (!clear) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.selectByMaterial"), 1, 1,
			mr_int, index+1, _T("clearCurrentSelection"), mr_bool, false);
	} else {
		macroRecorder->FunctionCall(_T("$.EditablePoly.selectByMaterial"), 1, 0,
			mr_int, index+1);
	}
	macroRecorder->EmitScript ();

	LocalDataChanged ();
}

void EditPolyObject::GetSmoothingGroups (DWORD flag, DWORD *anyFaces, DWORD *allFaces) {
	DWORD localAny;
	if (allFaces) {
		*allFaces = ~0;
		if (!anyFaces) anyFaces = &localAny;
	}
	if (!anyFaces) return;
	*anyFaces = 0;
	for (int j=0; j<mm.numf; j++) {
		if (mm.f[j].GetFlag (MN_DEAD)) continue;
		if (flag && !mm.f[j].GetFlag (flag)) continue;
		*anyFaces |= mm.f[j].smGroup;
		if (allFaces) *allFaces &= mm.f[j].smGroup;
	}
	if (allFaces) *allFaces &= *anyFaces;
}

bool EditPolyObject::LocalSetSmoothBits (DWORD bits, DWORD bitmask, DWORD flags) {
	for (int i=0; i<mm.numf; i++) {
		if (mm.f[i].GetFlag (MN_DEAD)) continue;
		if (mm.f[i].GetFlag (flags)) break;
	}
	if (i==mm.numf) return false;

	theHold.Begin();
	SmGroupRestore *smr = new SmGroupRestore (this);
	bits &= bitmask;
	for (int j=0; j<mm.numf; j++) {
		if (mm.f[j].GetFlag (MN_DEAD)) continue;
		if (!mm.f[j].GetFlag (flags)) continue;
		mm.f[j].smGroup &= ~bitmask;
		mm.f[j].smGroup |= bits;
	}
	smr->After ();
	theHold.Put (smr);
	theHold.Accept(GetString(IDS_ASSIGN_SMGROUP));

	macroRecorder->FunctionCall(_T("$.EditablePoly.setSmoothingGroups"), 1, 0,
		mr_int, bits, mr_int, bitmask, mr_int, flags);
	macroRecorder->EmitScript ();

	LocalDataChanged (PART_TOPO);
	RefreshScreen ();
	return true;
}

void EditPolyObject::EpfnSelectBySmoothGroup(DWORD bits,BOOL clear, TimeValue t) {
	BitArray nfs;
	if (clear) {
		nfs.SetSize (mm.numf);
		nfs.ClearAll ();
	} else {
		nfs = GetFaceSel ();
	}
	for (int j=0; j<mm.numf; j++) {
		if (mm.f[j].smGroup & bits) nfs.Set(j);
	}
	SetFaceSel (nfs, this, t);

	if (!clear) {
		macroRecorder->FunctionCall(_T("$.EditablePoly.selectBySmoothGroup"), 1, 1,
			mr_int, bits, _T("clearCurrentSelection"), mr_bool, false);
	} else {
		macroRecorder->FunctionCall(_T("$.EditablePoly.selectBySmoothGroup"), 1, 0,
			mr_int, bits);
	}
	macroRecorder->EmitScript ();
	LocalDataChanged ();
}

//----Globals----------------------------------------------
// Move to class Interface or someplace someday?
bool CreateCurveFromMeshEdges (MNMesh & mesh, INode *onode, Interface *ip, TSTR & name, bool curved, DWORD flag) {
	SuspendAnimate();
	AnimateOff();

	SplineShape *shape = (SplineShape*)GetSplineShapeDescriptor()->Create(0);	
	BitArray done;
	done.SetSize (mesh.nume);

	for (int i=0; i<mesh.nume; i++) {
		if (done[i]) continue;
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.e[i].GetFlag (flag)) continue;

		// The array of points for the spline
		Tab<Point3> pts;

		// Add the first two points.
		pts.Append(1,&mesh.v[mesh.e[i].v1].p,10);
		pts.Append(1,&mesh.v[mesh.e[i].v2].p,10);
		int nextv = mesh.e[i].v2, start = mesh.e[i].v1;

		// Mark this edge as done
		done.Set(i);

		// Trace along selected edges
		// Use loopcount/maxLoop just to avoid a while(1) loop.
		int loopCount, maxLoop=mesh.nume;
		for (loopCount=0; loopCount<maxLoop; loopCount++) {
			Tab<int> & ve = mesh.vedg[nextv];
			for (int j=0; j<ve.Count(); j++) {
				if (done[ve[j]]) continue;
				if (mesh.e[ve[j]].GetFlag (flag)) break;
			}
			if (j==ve.Count()) break;
			if (mesh.e[ve[j]].v1 == nextv) nextv = mesh.e[ve[j]].v2;
			else nextv = mesh.e[ve[j]].v1;

			// Mark this edge as done
			done.Set(ve[j]);

			// Add this vertex to the list
			pts.Append(1,&mesh.v[nextv].p,10);
		}
		int lastV = nextv;

		// Now trace backwards
		nextv = start;
		for (loopCount=0; loopCount<maxLoop; loopCount++) {
			Tab<int> & ve = mesh.vedg[nextv];
			for (int j=0; j<ve.Count(); j++) {
				if (done[ve[j]]) continue;
				if (mesh.e[ve[j]].GetFlag (flag)) break;
			}
			if (j==ve.Count()) break;
			if (mesh.e[ve[j]].v1 == nextv) nextv = mesh.e[ve[j]].v2;
			else nextv = mesh.e[ve[j]].v1;

			// Mark this edge as done
			done.Set(ve[j]);

			// Add this vertex to the list
			pts.Insert(0,1,&mesh.v[nextv].p);
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
	node->SetNodeTM (ip->GetTime(),ntm);
	node->FlagForeground (ip->GetTime(),FALSE);
	node->SetMtl (onode->GetMtl());
	node->SetObjOffsetPos (onode->GetObjOffsetPos());
	node->SetObjOffsetRot (onode->GetObjOffsetRot());
	node->SetObjOffsetScale (onode->GetObjOffsetScale());	
	ResumeAnimate();
	return true;
}

// For shift-clone type extrusion, where we only deal with open edges:
bool EditPolyObject::EpfnExtrudeOpenEdges (DWORD edgeFlag) {
	int firstFlagged=-1, lastFlagged, numFlagged=0;
	for (int i=0; i<mm.nume; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!mm.e[i].GetFlag(edgeFlag)) continue;
		if (mm.e[i].f2 > -1) {
			mm.e[i].ClearFlag (edgeFlag);
			continue;
		}
		if (firstFlagged<0) firstFlagged = i;
		lastFlagged = i;
		numFlagged++;
	}

	if (!numFlagged) return false;

	// For each flagged edge, we'll need one new face and one new edge.
	// For each vertex used by at least one flagged edge, we'll need one new vertex and one new edge.
	mm.ClearVFlags (MN_USER);
	int numVFlagged = mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_EDGE, edgeFlag);

	// We'll need to make sure that such vertices are on at most 2 open edges:
	if (mm.EliminateBadVerts (MN_USER)) {
		// repropogate flags:
		mm.ClearVFlags (MN_USER);
		numVFlagged = mm.PropegateComponentFlags (MNM_SL_VERTEX, MN_USER, MNM_SL_EDGE, edgeFlag);
	}

	int orignumf = mm.numf;
	int orignumv = mm.numv;
	int orignume = mm.nume;

	mm.setNumFaces (mm.numf + numFlagged);
	mm.setNumVerts (mm.numv + numVFlagged);
	mm.setNumEdges (mm.nume + numFlagged + numVFlagged);

	// Mapping - we'll create the map vertices as needed, but preallocate for now.
	bool hasMap=false;
	for (int mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
		if (mm.M(mapChannel)->GetFlag (MN_DEAD)) continue;
		mm.M(mapChannel)->setNumFaces (mm.numf);
		mm.M(mapChannel)->VAlloc (mm.M(mapChannel)->numv + numVFlagged*2);
		hasMap = true;
	}

	// Ok, time to get started cloning.  Do the vertices first.
	Tab<int> vClone;
	vClone.SetCount (orignumv);
	int numVClones = 0;
	for (i=0; i<orignumv; i++) {
		vClone[i] = 0;
		if (mm.v[i].GetFlag (MN_DEAD)) continue;
		if (!mm.v[i].GetFlag (MN_USER)) continue;

		// Create vertex clone:
		int newVert = orignumv + numVClones;
		mm.v[newVert].p = mm.v[i].p;

		// Create "vertical" edge clone:
		int newEdge = orignume + numVClones;
		mm.e[newEdge].v1 = i;
		mm.e[newEdge].v2 = newVert;

		// Make explicit that we don't yet have the faces for this edge:
		mm.e[newEdge].f1 = -1;
		mm.e[newEdge].f2 = -1;

		// Add vertical edge to vertices:
		mm.vedg[i].Append (1, &newEdge);
		mm.vedg[newVert].Append (1, &newEdge, 2);

		// Record clone ID and increment number of clones:
		vClone[i] = numVClones;
		numVClones++;
	}
	DbgAssert (numVClones+orignumv == mm.numv);

	// Now do the rest, edge-by-edge.
	int newEdge=orignume+numVClones, newFace=orignumf;
	for (i=firstFlagged; i<=lastFlagged; i++) {
		if (mm.e[i].GetFlag (MN_DEAD)) continue;
		if (!mm.e[i].GetFlag (edgeFlag)) continue;

		// Create new edge, with all links:
		mm.e[newEdge].v1 = vClone[mm.e[i].v1] + orignumv;
		mm.e[newEdge].v2 = vClone[mm.e[i].v2] + orignumv;
		mm.e[newEdge].f1 = newFace;
		mm.e[newEdge].f2 = -1;

		// Add new edge to its vertices:
		for (int j=0; j<2; j++) {
			mm.vedg[mm.e[newEdge][j]].Append (1, &newEdge);
		}

		// Create new face, with all links:
		mm.f[newFace].SetDeg (4);
		mm.f[newFace].vtx[0] = mm.e[i].v2;
		mm.f[newFace].vtx[1] = mm.e[i].v1;
		mm.f[newFace].vtx[2] = mm.e[newEdge].v1;
		mm.f[newFace].vtx[3] = mm.e[newEdge].v2;
		mm.f[newFace].edg[0] = i;
		mm.f[newFace].edg[1] = vClone[mm.e[i].v1] + orignume;
		mm.f[newFace].edg[2] = newEdge;
		mm.f[newFace].edg[3] = vClone[mm.e[i].v2] + orignume;

		// Add new face to old edge:
		mm.e[i].f2 = newFace;

		// Add new face to side edges:
		mm.e[mm.f[newFace].edg[1]].f1 = newFace;
		mm.e[mm.f[newFace].edg[3]].f2 = newFace;

		// Add new face to all 4 vertices:
		for (j=0; j<4; j++) {
			mm.vfac[mm.f[newFace].vtx[j]].Append (1, &newFace, 1);
		}

		// Create relevant map faces:
		if (hasMap) {
			int edgeIndex = mm.f[mm.e[i].f1].EdgeIndex (i, mm.e[i].v1);
			int otherEdge1=-1, otherEdge2=-1;
			Tab<int> & ve1 = mm.vedg[mm.e[i].v1];
			Tab<int> & ve2 = mm.vedg[mm.e[i].v2];
			for (j=0; j<ve1.Count(); j++) {
				if (ve1[j] >= orignume) continue;
				if (ve1[j] == i) continue;
				if (!mm.e[ve1[j]].GetFlag (edgeFlag)) continue;
				otherEdge1 = ve1[j];
				break;
			}
			for (j=0; j<ve2.Count(); j++) {
				if (ve2[j] >= orignume) continue;
				if (ve2[j] == i) continue;
				if (!mm.e[ve2[j]].GetFlag (edgeFlag)) continue;
				otherEdge2 = ve2[j];
				break;
			}
			for (mapChannel=-NUM_HIDDENMAPS; mapChannel<mm.numm; mapChannel++) {
				if (mm.M(mapChannel)->GetFlag (MN_DEAD)) continue;
				MNMapFace *baseMapFace = mm.M(mapChannel)->F(mm.e[i].f1);
				MNMapFace *newMapFace = mm.M(mapChannel)->F(newFace);
				newMapFace->SetSize (4);
				newMapFace->tv[0] = baseMapFace->tv[(edgeIndex+1)%baseMapFace->deg];
				newMapFace->tv[1] = baseMapFace->tv[edgeIndex];

				// Map vertex cloning.
				UVVert *pMV = mm.M(mapChannel)->v;
				bool useOldClone = false;
				int oldClone;
				if ((otherEdge1>-1) && (otherEdge1<i)) {
					MNMapFace *otherMapFace = mm.M(mapChannel)->F(mm.e[otherEdge1].f2);	// get recently created face.
					if (otherMapFace->tv[0] == newMapFace->tv[1]) {
						useOldClone = true;
						oldClone = otherMapFace->tv[3];
					}
				}
				newMapFace->tv[2] = useOldClone ? oldClone : mm.M(mapChannel)->NewVert (pMV[newMapFace->tv[1]]);

				useOldClone = false;
				if ((otherEdge2>-1) && (otherEdge2<i)) {
					MNMapFace *otherMapFace = mm.M(mapChannel)->F(mm.e[otherEdge2].f2);	// get recently created face.
					if (otherMapFace->tv[1] == newMapFace->tv[0]) {
						useOldClone = true;
						oldClone = otherMapFace->tv[2];
					}
				}
				newMapFace->tv[3] = useOldClone ? oldClone : mm.M(mapChannel)->NewVert (pMV[newMapFace->tv[0]]);
			}
		}

		// Transfer flag to new edge from old:
		mm.e[newEdge].SetFlag (edgeFlag);

		newEdge++;
		newFace++;
	}

	for (i=firstFlagged; i<=lastFlagged; i++) mm.e[i].ClearFlag (edgeFlag);

	// Scan the vertical edges for any that need flipping:
	for (i=orignume; i<orignume+numVClones; i++) {
		if (mm.e[i].f1 < 0) mm.e[i].Invert ();
	}

	// In case this was cleared - it shouldn't be:
	mm.SetFlag (MN_MESH_FILLED_IN);

	return true;
}
