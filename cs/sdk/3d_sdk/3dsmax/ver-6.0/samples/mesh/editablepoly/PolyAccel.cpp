/**********************************************************************
 *<
	FILE: PolyAccel.cpp

	DESCRIPTION: Editable Polygon Mesh Object Accelerator table (keyboard shortcuts)

	CREATED BY: Steve Anderson

	HISTORY: created January 26, 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "EPoly.h"
#include "PolyEdit.h"

BOOL EPolyActionCB::ExecuteAction (int id) {
	int checkboxValue;	// for toggles.
	int constrainType;

	switch (id) {

	// Selection dialog shortcuts:

	case ID_EP_SEL_LEV_OBJ:
		mpObj->SetEPolySelLevel (EP_SL_OBJECT);
		break;

	case ID_EP_SEL_LEV_VERTEX:
		mpObj->SetEPolySelLevel (EP_SL_VERTEX);
		break;

	case ID_EP_SEL_LEV_EDGE:
		mpObj->SetEPolySelLevel (EP_SL_EDGE);
		break;

	case ID_EP_SEL_LEV_BORDER:
		mpObj->SetEPolySelLevel (EP_SL_BORDER);
		break;

	case ID_EP_SEL_LEV_FACE:
		mpObj->SetEPolySelLevel (EP_SL_FACE);
		break;

	case ID_EP_SEL_LEV_ELEMENT:
		mpObj->SetEPolySelLevel (EP_SL_ELEMENT);
		break;

	case ID_EP_SEL_BYVERT:
		theHold.Begin ();
		mpObj->getParamBlock()->GetValue (ep_by_vertex, TimeValue(0), checkboxValue, FOREVER);
		mpObj->getParamBlock()->SetValue (ep_by_vertex, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SEL_IGBACK:
		theHold.Begin ();
		mpObj->getParamBlock()->GetValue (ep_ignore_backfacing, TimeValue(0), checkboxValue, FOREVER);
		mpObj->getParamBlock()->SetValue (ep_ignore_backfacing, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SEL_GROW:
		mpObj->EpActionButtonOp (epop_sel_grow);
		break;

	case ID_EP_SEL_SHRINK:
		mpObj->EpActionButtonOp (epop_sel_shrink);
		break;

	case ID_EP_SEL_RING:
		if (mpObj->GetMNSelLevel() == MNM_SL_EDGE)
			mpObj->EpActionButtonOp (epop_select_ring);
		break;

	case ID_EP_SEL_LOOP:
		if (mpObj->GetMNSelLevel() == MNM_SL_EDGE)
			mpObj->EpActionButtonOp (epop_select_loop);
		break;

	case ID_EP_SEL_HIDE:
		mpObj->EpActionButtonOp (epop_hide);
		break;

	case ID_EP_UNSEL_HIDE:
		mpObj->EpActionButtonOp (epop_hide_unsel);
		break;

	case ID_EP_SEL_UNHIDE:
		mpObj->EpActionButtonOp (epop_unhide);
		break;

	// Soft Selection dialog shortcuts:

	case ID_EP_SS_ON:
		theHold.Begin ();
		mpObj->getParamBlock()->GetValue (ep_ss_use, TimeValue(0), checkboxValue, FOREVER);
		mpObj->getParamBlock()->SetValue (ep_ss_use, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SS_BACKFACE:
		theHold.Begin ();
		mpObj->getParamBlock()->GetValue (ep_ss_affect_back, TimeValue(0), checkboxValue, FOREVER);
		mpObj->getParamBlock()->SetValue (ep_ss_affect_back, TimeValue(0), !checkboxValue);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_SS_SHADEDFACES:
		mpObj->EpActionButtonOp (epop_toggle_shaded_faces);
		break;

	// Edit Geometry dialog shortcuts:

	case ID_EP_EDIT_CREATE:
		switch (mpObj->GetMNSelLevel()) {
		case MNM_SL_VERTEX:
			mpObj->EpActionToggleCommandMode (epmode_create_vertex);
			break;
		case MNM_SL_EDGE:
			mpObj->EpActionToggleCommandMode (epmode_create_edge);
			break;
		default:
			mpObj->EpActionToggleCommandMode (epmode_create_face);
			break;
		}
		break;

	case ID_EP_EDIT_ATTACH:
		mpObj->EpActionEnterPickMode (epmode_attach);
		break;

	case ID_EP_EDIT_DETACH:
		if (mpObj->GetSelLevel() != EP_SL_OBJECT)
			mpObj->EpActionButtonOp (epop_detach);
		break;

	case ID_EP_EDIT_ATTACHLIST:
		mpObj->EpActionButtonOp (epop_attach_list);
		break;

	case ID_EP_EDIT_COLLAPSE:
		if (mpObj->GetSelLevel() != EP_SL_OBJECT)
			mpObj->EpActionButtonOp (epop_collapse);
		break;

	case ID_EP_EDIT_CUT:
		mpObj->EpActionToggleCommandMode (epmode_cut_vertex);
		break;

	case ID_EP_EDIT_QUICKSLICE:
		mpObj->EpActionToggleCommandMode (epmode_quickslice);
		break;

	case ID_EP_EDIT_SLICEPLANE:
		mpObj->EpActionToggleCommandMode (epmode_sliceplane);
		break;

	case ID_EP_EDIT_SLICE:
			// Luna task 748A - preview mode
			if (mpObj->EpPreviewOn()) {
				mpObj->EpPreviewAccept ();
				mpObj->EpPreviewBegin (epop_slice);
			}
			else mpObj->EpActionButtonOp (epop_slice);
			break;
		break;

	case ID_EP_EDIT_RESETPLANE:
		mpObj->EpActionButtonOp (epop_reset_plane);
		break;

	case ID_EP_EDIT_MAKEPLANAR:
		mpObj->EpActionButtonOp (epop_make_planar);
		break;

	case ID_EP_EDIT_ALIGN_VIEW:
		mpObj->EpActionButtonOp (epop_align_view);
		break;

	case ID_EP_EDIT_ALIGN_GRID:
		mpObj->EpActionButtonOp (epop_align_grid);
		break;

	case ID_EP_EDIT_MESHSMOOTH:
		mpObj->EpActionButtonOp (epop_meshsmooth);
		break;

	case ID_EP_EDIT_MESHSMOOTH_SETTINGS:
		mpObj->EpfnPopupDialog (epop_meshsmooth);
		break;

	case ID_EP_EDIT_TESSELLATE:
		mpObj->EpActionButtonOp (epop_tessellate);
		break;

	case ID_EP_EDIT_TESSELLATE_SETTINGS:
		mpObj->EpfnPopupDialog (epop_tessellate);
		break;

	case ID_EP_EDIT_REPEAT:
		mpObj->EpfnRepeatLastOperation ();
		break;

	// Subobject-level editing dialog shortcuts:
	case ID_EP_SUB_REMOVE:
		mpObj->EpActionButtonOp (epop_remove);
		break;

	case ID_EP_SUB_BREAK:
		mpObj->EpActionButtonOp (epop_break);
		break;

	case ID_EP_SUB_SPLIT:
		mpObj->EpActionButtonOp (epop_split);
		break;

	case ID_EP_SUB_INSERTVERT:
		switch (mpObj->GetMNSelLevel()) {
		case MNM_SL_EDGE:
			mpObj->EpActionToggleCommandMode (epmode_divide_edge);
			break;
		case MNM_SL_FACE:
			mpObj->EpActionToggleCommandMode (epmode_divide_face);
			break;
		}
		break;

	case ID_EP_SUB_CAP:
		mpObj->EpActionButtonOp (epop_cap);
		break;

	case ID_EP_SUB_EXTRUDE:
		switch (mpObj->GetEPolySelLevel()) {
		case EP_SL_VERTEX:
			mpObj->EpActionToggleCommandMode (epmode_extrude_vertex);
			break;
		case EP_SL_EDGE:
		case EP_SL_BORDER:
			mpObj->EpActionToggleCommandMode (epmode_extrude_edge);
			break;
		case EP_SL_FACE:
			mpObj->EpActionToggleCommandMode (epmode_extrude_face);
			break;
		}
		break;

	case ID_EP_SUB_EXTRUDE_SETTINGS:
		switch (mpObj->GetEPolySelLevel()) {
		case EP_SL_VERTEX:
		case EP_SL_EDGE:
		case EP_SL_BORDER:
		case EP_SL_FACE:
			mpObj->EpfnPopupDialog (epop_extrude);
			break;
		}
		break;

	case ID_EP_SUB_BEVEL:
		if (mpObj->GetEPolySelLevel() != EP_SL_FACE) break;
		mpObj->EpActionToggleCommandMode (epmode_bevel);
		break;

	case ID_EP_SUB_BEVEL_SETTINGS:
		if (mpObj->GetEPolySelLevel() != EP_SL_FACE) break;
		mpObj->EpfnPopupDialog (epop_bevel);
		break;

	case ID_EP_SUB_OUTLINE:
		if (mpObj->GetEPolySelLevel() != EP_SL_FACE) break;
		mpObj->EpActionToggleCommandMode (epmode_outline);
		break;

	case ID_EP_SUB_OUTLINE_SETTINGS:
		if (mpObj->GetEPolySelLevel() != EP_SL_FACE) break;
		mpObj->EpfnPopupDialog (epop_outline);
		break;

	case ID_EP_SUB_INSET:
		if (mpObj->GetEPolySelLevel() != EP_SL_FACE) break;
		mpObj->EpActionToggleCommandMode (epmode_inset_face);
		break;

	case ID_EP_SUB_INSET_SETTINGS:
		if (mpObj->GetEPolySelLevel() != EP_SL_FACE) break;
		mpObj->EpfnPopupDialog (epop_inset);
		break;

	case ID_EP_SUB_CHAMFER:
		switch (mpObj->GetMNSelLevel ()) {
		case MNM_SL_VERTEX:
			mpObj->EpActionToggleCommandMode (epmode_chamfer_vertex);
			break;
		case MNM_SL_EDGE:
			mpObj->EpActionToggleCommandMode (epmode_chamfer_edge);
			break;
		}
		break;

	case ID_EP_SUB_CHAMFER_SETTINGS:
		if (mpObj->GetMNSelLevel() > MNM_SL_EDGE) break;
		mpObj->EpfnPopupDialog (epop_chamfer);
		break;

	case ID_EP_SUB_WELD:
		if (mpObj->GetEPolySelLevel() > EP_SL_EDGE) break;
		mpObj->EpActionToggleCommandMode (epmode_weld);
		break;

	case ID_EP_SUB_WELD_SETTINGS:
		if (mpObj->GetEPolySelLevel() > EP_SL_EDGE) break;
		mpObj->EpfnPopupDialog (epop_weld_sel);
		break;

	case ID_EP_SUB_REMOVE_ISO:
		mpObj->EpActionButtonOp (epop_remove_iso_verts);
		break;

	case ID_EP_SUB_REMOVE_ISOMAP:
		mpObj->EpActionButtonOp (epop_remove_iso_map_verts);
		break;

	case ID_EP_SUB_MAKESHAPE:
		if (mpObj->GetMNSelLevel() != MNM_SL_EDGE) break;
		mpObj->EpActionButtonOp (epop_create_shape);
		break;

	case ID_EP_SUB_CONNECT:
		switch (mpObj->GetMNSelLevel ()) {
		case MNM_SL_VERTEX:
			mpObj->EpActionButtonOp (epop_connect_vertices);
			break;
		case MNM_SL_EDGE:
			mpObj->EpActionButtonOp (epop_connect_edges);
			break;
		}
		break;

	case ID_EP_SUB_CONNECT_SETTINGS:
		if (mpObj->GetMNSelLevel() != MNM_SL_EDGE) break;
		mpObj->EpfnPopupDialog (epop_connect_edges);
		break;

	case ID_EP_SUB_LIFT:
		if (mpObj->GetEPolySelLevel () != EP_SL_FACE) break;
		mpObj->EpActionToggleCommandMode (epmode_lift_from_edge);
		break;

	case ID_EP_SUB_LIFT_SETTINGS:
		if (mpObj->GetEPolySelLevel () != EP_SL_FACE) break;
		mpObj->EpfnPopupDialog (epop_lift_from_edge);
		break;

	case ID_EP_SUB_EXSPLINE:
		if (mpObj->GetEPolySelLevel () != EP_SL_FACE) break;
		mpObj->EpActionEnterPickMode (epmode_pick_shape);
		break;

	case ID_EP_SUB_EXSPLINE_SETTINGS:
		if (mpObj->GetEPolySelLevel () != EP_SL_FACE) break;
		mpObj->EpfnPopupDialog (epop_extrude_along_spline);
		break;

	// Surface dialog control shortcuts:

	case ID_EP_SURF_EDITTRI:
		if (mpObj->GetEPolySelLevel () < EP_SL_EDGE) break;
		mpObj->EpActionToggleCommandMode (epmode_edit_tri);
		break;

	case ID_EP_SURF_RETRI:
		mpObj->EpActionButtonOp (epop_retriangulate);
		break;

	case ID_EP_SURF_FLIPNORM:
		mpObj->EpActionButtonOp (epop_flip_normals);
		break;

	case ID_EP_SURF_AUTOSMOOTH:
		mpObj->EpActionButtonOp (epop_autosmooth);
		break;

	case ID_EP_EDGE_CONSTRAINT:
		theHold.Begin ();
		mpObj->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
		if (constrainType == 1) constrainType = 0;
		else constrainType = 1;
		mpObj->getParamBlock()->SetValue (ep_constrain_type, TimeValue(0), constrainType);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	case ID_EP_FACE_CONSTRAINT:
		theHold.Begin ();
		mpObj->getParamBlock()->GetValue (ep_constrain_type, TimeValue(0), constrainType, FOREVER);
		if (constrainType == 2) constrainType = 0;
		else constrainType = 2;
		mpObj->getParamBlock()->SetValue (ep_constrain_type, TimeValue(0), constrainType);
		theHold.Accept (GetString (IDS_PARAMETERS));
		break;

	default:
		return false;
	}
	return true;
}

#ifndef RENDER_VER
const int kNumEPolyActions = 68;
#else
const int kNumEPolyActions = 0;
#endif

static ActionDescription epolyActions[] = {
#ifndef RENDER_VER
	ID_EP_SEL_LEV_OBJ,
		IDS_OBJECT_LEVEL,
		IDS_OBJECT_LEVEL,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_LEV_VERTEX,
		IDS_VERTEX_LEVEL,
		IDS_VERTEX_LEVEL,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_LEV_EDGE,
		IDS_EDGE_LEVEL,
		IDS_EDGE_LEVEL,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_LEV_BORDER,
		IDS_BORDER_LEVEL,
		IDS_BORDER_LEVEL,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_LEV_FACE,
		IDS_FACE_LEVEL,
		IDS_FACE_LEVEL,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_LEV_ELEMENT,
		IDS_ELEMENT_LEVEL,
		IDS_ELEMENT_LEVEL,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_BYVERT,
		IDS_SEL_BY_VERTEX,
		IDS_SEL_BY_VERTEX,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_IGBACK,
		IDS_IGNORE_BACKFACING,
		IDS_IGNORE_BACKFACING,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_GROW,
		IDS_SELECTION_GROW,
		IDS_SELECTION_GROW,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_SHRINK,
		IDS_SELECTION_SHRINK,
		IDS_SELECTION_SHRINK,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_LOOP,
		IDS_SELECT_EDGE_LOOP,
		IDS_SELECT_EDGE_LOOP,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_RING,
		IDS_SELECT_EDGE_RING,
		IDS_SELECT_EDGE_RING,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_HIDE,
		IDS_HIDE,
		IDS_HIDE,
		IDS_EDITABLE_POLY,

	ID_EP_UNSEL_HIDE,
		IDS_HIDE_UNSELECTED,
		IDS_HIDE_UNSELECTED,
		IDS_EDITABLE_POLY,

	ID_EP_SEL_UNHIDE,
		IDS_UNHIDE_ALL,
		IDS_UNHIDE_ALL,
		IDS_EDITABLE_POLY,

	ID_EP_SS_BACKFACE,
		IDS_AFFECT_BACKFACING,
		IDS_AFFECT_BACKFACING,
		IDS_EDITABLE_POLY,

	ID_EP_SS_ON,
		IDS_USE_SOFTSEL,
		IDS_USE_SOFTSEL,
		IDS_EDITABLE_POLY,

	ID_EP_SS_SHADEDFACES,
		IDS_SHADED_FACE_TOGGLE,
		IDS_SHADED_FACE_TOGGLE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_ALIGN_GRID,
		IDS_ALIGN_TO_GRID,
		IDS_ALIGN_TO_GRID,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_ALIGN_VIEW,
		IDS_ALIGN_TO_VIEW,
		IDS_ALIGN_TO_VIEW,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_ATTACH,
		IDS_ATTACH,
		IDS_ATTACH,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_ATTACHLIST,
		IDS_ATTACH_LIST,
		IDS_ATTACH_LIST,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_COLLAPSE,
		IDS_COLLAPSE,
		IDS_COLLAPSE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_CREATE,
		IDS_CREATE,
		IDS_CREATE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_CUT,
		IDS_CUT,
		IDS_CUT,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_DETACH,
		IDS_DETACH,
		IDS_DETACH,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_MAKEPLANAR,
		IDS_MAKE_PLANAR,
		IDS_MAKE_PLANAR,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_MESHSMOOTH,
		IDS_MESHSMOOTH,
		IDS_MESHSMOOTH,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_MESHSMOOTH_SETTINGS,
		IDS_MESHSMOOTH_SETTINGS,
		IDS_MESHSMOOTH_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_QUICKSLICE,
		IDS_QUICKSLICE_MODE,
		IDS_QUICKSLICE_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_REPEAT,
		IDS_REPEAT_LAST_OPERATION,
		IDS_REPEAT_LAST_OPERATION,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_RESETPLANE,
		IDS_RESET_SLICE_PLANE,
		IDS_RESET_SLICE_PLANE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_SLICE,
		IDS_SLICE,
		IDS_SLICE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_SLICEPLANE,
		IDS_SLICE_PLANE_MODE,
		IDS_SLICE_PLANE_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_TESSELLATE,
		IDS_TESSELLATE,
		IDS_TESSELLATE,
		IDS_EDITABLE_POLY,

	ID_EP_EDIT_TESSELLATE_SETTINGS,
		IDS_TESSELLATE_SETTINGS,
		IDS_TESSELLATE_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_BEVEL,
		IDS_BEVEL_MODE,
		IDS_BEVEL_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_BEVEL_SETTINGS,
		IDS_BEVEL_SETTINGS,
		IDS_BEVEL_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_BREAK,
		IDS_BREAK,
		IDS_BREAK,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_CAP,
		IDS_CAP,
		IDS_CAP,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_CHAMFER,
		IDS_CHAMFER_MODE,
		IDS_CHAMFER_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_CHAMFER_SETTINGS,
		IDS_CHAMFER_SETTINGS,
		IDS_CHAMFER_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_CONNECT,
		IDS_CONNECT,
		IDS_CONNECT,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_CONNECT_SETTINGS,
		IDS_CONNECT_EDGE_SETTINGS,
		IDS_CONNECT_EDGE_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_EXSPLINE,
		IDS_EXTRUDE_ALONG_SPLINE_MODE,
		IDS_EXTRUDE_ALONG_SPLINE_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_EXSPLINE_SETTINGS,
		IDS_EXTRUDE_ALONG_SPLINE_SETTINGS,
		IDS_EXTRUDE_ALONG_SPLINE_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_EXTRUDE,
		IDS_EXTRUDE_MODE,
		IDS_EXTRUDE_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_EXTRUDE_SETTINGS,
		IDS_EXTRUDE_SETTINGS,
		IDS_EXTRUDE_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_INSERTVERT,
		IDS_INSERT_VERTEX_MODE,
		IDS_INSERT_VERTEX_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_INSET,
		IDS_INSET_MODE,
		IDS_INSET_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_INSET_SETTINGS,
		IDS_INSET_SETTINGS,
		IDS_INSET_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_LIFT,
		IDS_LIFT_FROM_EDGE_MODE,
		IDS_LIFT_FROM_EDGE_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_LIFT_SETTINGS,
		IDS_LIFT_FROM_EDGE_SETTINGS,
		IDS_LIFT_FROM_EDGE_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_MAKESHAPE,
		IDS_CREATE_SHAPE_FROM_EDGES,
		IDS_CREATE_SHAPE_FROM_EDGES,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_OUTLINE,
		IDS_OUTLINE_MODE,
		IDS_OUTLINE_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_OUTLINE_SETTINGS,
		IDS_OUTLINE_SETTINGS,
		IDS_OUTLINE_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_REMOVE,
		IDS_REMOVE,
		IDS_REMOVE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_REMOVE_ISO,
		IDS_REMOVE_ISOLATED_VERTICES,
		IDS_REMOVE_ISOLATED_VERTICES,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_REMOVE_ISOMAP,
		IDS_REMOVE_UNUSED_MAP_VERTICES,
		IDS_REMOVE_UNUSED_MAP_VERTICES,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_SPLIT,
		IDS_SPLIT_EDGES,
		IDS_SPLIT_EDGES,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_WELD,
		IDS_WELD_MODE,
		IDS_WELD_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SUB_WELD_SETTINGS,
		IDS_WELD_SETTINGS,
		IDS_WELD_SETTINGS,
		IDS_EDITABLE_POLY,

	ID_EP_SURF_AUTOSMOOTH,
		IDS_AUTOSMOOTH,
		IDS_AUTOSMOOTH,
		IDS_EDITABLE_POLY,

	ID_EP_SURF_EDITTRI,
		IDS_EDIT_TRIANGULATION_MODE,
		IDS_EDIT_TRIANGULATION_MODE,
		IDS_EDITABLE_POLY,

	ID_EP_SURF_FLIPNORM,
		IDS_FLIP_NORMALS,
		IDS_FLIP_NORMALS,
		IDS_EDITABLE_POLY,

	ID_EP_SURF_RETRI,
		IDS_RETRIANGULATE,
		IDS_RETRIANGULATE,
		IDS_EDITABLE_POLY,

	ID_EP_EDGE_CONSTRAINT,
		IDS_CONSTRAIN_TO_EDGES,
		IDS_CONSTRAIN_TO_EDGES,
		IDS_EDITABLE_POLY,

	ID_EP_FACE_CONSTRAINT,
		IDS_CONSTRAIN_TO_FACES,
		IDS_CONSTRAIN_TO_FACES,
		IDS_EDITABLE_POLY,
#else  // RENDER_VER
	0 // empty initializers are not allowed
#endif // RENDER_VER
};

ActionTable* GetEPolyActions() {
	TSTR name = GetString (IDS_EDITABLE_POLY);
	HACCEL hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_ACCEL_EPOLY));
	ActionTable* pTab;
	pTab = new ActionTable (kEPolyActionID, kEPolyActionID,
		name, hAccel, kNumEPolyActions, epolyActions, hInstance);
    GetCOREInterface()->GetActionManager()->RegisterActionContext
		(kEPolyActionID, name.data());

	return pTab;
}

ActionTable* EditablePolyObjectClassDesc::GetActionTable(int i) {
	return GetEPolyActions ();
}

