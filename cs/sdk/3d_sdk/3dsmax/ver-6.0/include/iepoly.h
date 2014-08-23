/**********************************************************************
 *<
	FILE: iEPoly.h

	DESCRIPTION:   Editable Polygon Mesh Object SDK Interface

	CREATED BY: Steve Anderson

	HISTORY: created March 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __POLYOBJED_INTERFACE_
#define __POLYOBJED_INTERFACE_

#include "iFnPub.h"

// Editable Poly Selection Levels
enum ePolySelLevel { EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, 
	EP_SL_BORDER, EP_SL_FACE, EP_SL_ELEMENT, EP_SL_CURRENT };

// Editable Poly Parameters:
// (Use with EpactionGetParamBlock)
enum epolyParameters { ep_by_vertex, ep_ignore_backfacing, ep_show_normals,
	ep_normal_size, ep_ss_use, ep_ss_retro, ep_ss_edist_use, ep_ss_edist,
	ep_ss_affect_back, ep_ss_falloff, ep_ss_pinch, ep_ss_bubble,
	ep_extrusion_type, ep_split, ep_refine_ends, ep_weld_threshold,
	ep_weld_pixels, ep_ms_smoothness, ep_ms_smooth, ep_ms_sep_smooth,
	ep_ms_sep_mat, ep_tess_type, ep_tess_tension, ep_surf_subdivide,
	ep_surf_ig_sel, ep_surf_iter, ep_surf_thresh, ep_surf_riter,
	ep_surf_rthresh, ep_surf_use_riter, ep_surf_use_rthresh,
	ep_surf_sep_smooth, ep_surf_sep_mat, ep_surf_update,
	ep_vert_sel_color, ep_vert_selc_r, ep_vert_selc_g, ep_vert_selc_b,
	ep_face_smooth_thresh, ep_vert_color_selby,
	ep_sd_use, ep_sd_split_mesh, ep_sd_method, ep_sd_tess_steps,
	ep_sd_tess_edge, ep_sd_tess_distance, ep_sd_tess_angle, ep_sd_view_dependent,
	ep_asd_style, ep_asd_min_iters, ep_asd_max_iters, ep_asd_max_tris,
	ep_surf_subdiv_smooth, ep_face_extrude_height, ep_bevel_outline,
	ep_vertex_chamfer, ep_cut_start_level, ep_cut_start_index,
	ep_cut_start_coords, ep_cut_end_coords, ep_cut_normal,
	ep_interactive_full, ep_vertex_extrude_width, ep_lift_angle, ep_lift_edge, 
	ep_lift_segments, ep_connect_edge_segments, ep_slide_edge_direction,
	ep_slide_edge_start, ep_extrude_spline_node, ep_extrude_spline_segments,
	ep_extrude_spline_taper, ep_extrude_spline_taper_curve, ep_extrude_spline_twist,
	ep_extrude_spline_rotation, ep_inset, ep_inset_type, ep_constrain_type,
	ep_extrude_spline_align, ep_show_cage, ep_bevel_height, ep_vertex_extrude_height,
	ep_edge_extrude_height, ep_edge_extrude_width, ep_outline, ep_edge_chamfer,
	ep_bevel_type, ep_edge_weld_threshold, ep_surf_isoline_display
};

// Editable Poly Button operations:
// (Use with EpActionButtonOp)
enum epolyButtonOp { epop_hide, epop_unhide, epop_ns_copy, epop_ns_paste,
	epop_cap, epop_delete, epop_detach, epop_attach_list, epop_split,
	epop_break, epop_collapse, epop_reset_plane, epop_slice, epop_weld_sel,
	epop_create_shape, epop_make_planar, epop_align_grid, epop_align_view,
	epop_remove_iso_verts,
	epop_meshsmooth, epop_tessellate, epop_update, epop_selby_vc,
	epop_retriangulate, epop_flip_normals, epop_selby_matid, epop_selby_smg,
	epop_autosmooth, epop_clear_smg, epop_bevel, epop_chamfer, epop_cut,
	epop_null, epop_sel_grow, epop_sel_shrink, epop_inset, epop_extrude,
	epop_extrude_along_spline, epop_connect_edges, epop_connect_vertices,
	epop_lift_from_edge, epop_select_ring, epop_select_loop,
	epop_remove_iso_map_verts, epop_remove, epop_outline,
	epop_toggle_shaded_faces, epop_hide_unsel
};

// Editable Poly Command modes:
// (Use with EpActionToggleCommandMode.)
enum epolyCommandMode { epmode_create_vertex, epmode_create_edge,
	epmode_create_face, epmode_divide_edge, epmode_divide_face,
	epmode_extrude_vertex, epmode_extrude_edge,
	epmode_extrude_face, epmode_chamfer_vertex, epmode_chamfer_edge,
	epmode_bevel, epmode_sliceplane, epmode_cut_vertex, epmode_cut_edge,
	epmode_cut_face, epmode_weld, epmode_edit_tri, epmode_inset_face,
	epmode_quickslice, epmode_lift_from_edge, epmode_pick_lift_edge,
	epmode_outline
};

// Pick modes:
enum epolyPickMode { epmode_attach, epmode_pick_shape };

// Enum our enums, for use in the function publishing interface description
enum epolyEnumList { epolyEnumButtonOps, epolyEnumCommandModes, epolyEnumPickModes,
	ePolySelLevel, PMeshSelLevel };

#define EPOLY_INTERFACE Interface_ID(0x092779, 0x634020)
#define GetEPolyInterface(cd) (EPoly *)(cd)->GetInterface(EPOLY_INTERFACE)

// "Mixin" Interface methods:
enum ePolyMethods { epfn_hide, epfn_unhide_all, epfn_named_selection_copy,
	epfn_named_selection_paste, epfn_create_vertex, epfn_create_edge,
	epfn_create_face, epfn_cap_holes, epfn_delete, epfn_attach, epfn_multi_attach,
	epfn_detach_to_element, epfn_detach_to_object, epfn_split_edges,
	epfn_break_verts, epfn_divide_face, epfn_divide_edge,
	epfn_collapse, epfn_extrude_faces, epfn_bevel_faces,
	epfn_chamfer_vertices, epfn_chamfer_edges, epfn_slice, epfn_cut_vertex,
	epfn_cut_edge, epfn_cut_face, epfn_weld_verts, epfn_weld_edges,
	epfn_weld_flagged_verts, epfn_weld_flagged_edges, epfn_create_shape,
	epfn_make_planar, epfn_move_to_plane, epfn_align_to_grid, epfn_align_to_view,
	epfn_delete_iso_verts, epfn_meshsmooth,
	epfn_tessellate, epfn_set_diagonal, epfn_retriangulate,
	epfn_flip_normals, epfn_select_by_mat, epfn_select_by_smooth_group,
	epfn_autosmooth, epfn_in_slice_plane_mode, epfn_force_subdivision,
	epfn_button_op, epfn_toggle_command_mode, epfn_enter_pick_mode,
	epfn_exit_command_modes,
	// Flag Accessors:
	epfn_get_vertices_by_flag, epfn_get_edges_by_flag, epfn_get_faces_by_flag,
	epfn_set_vertex_flags, epfn_set_edge_flags, epfn_set_face_flags,
	// Data accessors:
	epfn_reset_slice_plane, epfn_set_slice_plane, epfn_get_slice_plane,
	epfn_get_vertex_color, epfn_set_vertex_color, epfn_get_face_color,
	epfn_set_face_color, epfn_get_vertex_data, epfn_get_edge_data,
	epfn_set_vertex_data, epfn_set_edge_data, epfn_reset_vertex_data,
	epfn_reset_edge_data, epfn_begin_modify_perdata, 
	epfn_in_modify_perdata, epfn_end_modify_perdata,
	epfn_begin_modify_vertex_color, epfn_in_modify_vertex_color,
	epfn_end_modify_vertex_color,
	epfn_get_mat_index, epfn_set_mat_index,
	epfn_get_smoothing_groups, epfn_set_smoothing_groups,
	// other things...
	epfn_collapse_dead_structs, /*epfn_apply_delta,*/ epfn_propagate_component_flags,

	// New for Luna:
	// Preview mode access:
	epfn_preview_clear, epfn_preview_begin, epfn_preview_cancel, epfn_preview_accept,
	epfn_preview_invalidate, epfn_preview_on, epfn_preview_set_dragging, epfn_preview_get_dragging,

	// Popup dialog access:
	epfn_popup_dialog, epfn_close_popup_dialog,

	// Other new stuff:
	epfn_repeat_last, epfn_grow_selection, epfn_shrink_selection, epfn_convert_selection,
	epfn_select_border, epfn_select_element, epfn_select_edge_loop, epfn_select_edge_ring,
	epfn_remove, epfn_delete_iso_map_verts, epfn_outline, epfn_connect_edges,
	epfn_connect_vertices, epfn_extrude_along_spline, epfn_lift_from_edge,
	epfn_toggle_shaded_faces,

	// Access to selections:
	epfn_get_epoly_sel_level, epfn_get_mn_sel_level, epfn_get_selection, epfn_set_selection,

	// Access to information about mesh:
	epfn_get_num_vertices, epfn_get_vertex, epfn_get_vertex_face_count, epfn_get_vertex_face,
	epfn_get_vertex_edge_count, epfn_get_vertex_edge, epfn_get_num_edges, epfn_get_edge_vertex,
	epfn_get_edge_face, epfn_get_num_faces, epfn_get_face_degree, epfn_get_face_vertex,
	epfn_get_face_edge, epfn_get_face_material, epfn_get_face_smoothing_group,
	epfn_get_num_map_channels, epfn_get_map_channel_active,
	epfn_get_num_map_vertices, epfn_get_map_vertex, epfn_get_map_face_vertex,

	// All right all ready
	epfn_collapse_dead_structs_spelled_right, epfn_get_map_face_vert
};

class EPoly : public FPMixinInterface {
public:
	BEGIN_FUNCTION_MAP
		FN_2(epfn_hide, TYPE_bool, EpfnHide, TYPE_INT, TYPE_DWORD);
		FN_1(epfn_unhide_all, TYPE_bool, EpfnUnhideAll, TYPE_INT);
		VFN_1(epfn_named_selection_copy, EpfnNamedSelectionCopy, TYPE_STRING);
		VFN_1(epfn_named_selection_paste, EpfnNamedSelectionPaste, TYPE_bool);
		FN_3(epfn_create_vertex, TYPE_INDEX, EpfnCreateVertex, TYPE_POINT3, TYPE_bool, TYPE_bool);
		FN_3(epfn_create_edge, TYPE_INDEX, EpfnCreateEdge, TYPE_INDEX, TYPE_INDEX, TYPE_bool);
		FN_2(epfn_create_face, TYPE_INDEX, EpfnCreateFace2, TYPE_INDEX_TAB, TYPE_bool);	// switching in createFace2, because CreateFace wasn't working here.
		FN_2(epfn_cap_holes, TYPE_bool, EpfnCapHoles, TYPE_INT, TYPE_DWORD);
		FN_3(epfn_delete, TYPE_bool, EpfnDelete, TYPE_INT, TYPE_DWORD, TYPE_bool);
		VFNT_2(epfn_attach, EpfnAttach, TYPE_INODE, TYPE_INODE);
		//VFNT_2(epfn_multi_attach, EpfnMultiAttach, TYPE_INODE_TAB, TYPE_INODE);
		FN_3(epfn_detach_to_element, TYPE_bool, EpfnDetachToElement, TYPE_INT, TYPE_DWORD, TYPE_bool);
		//FNT_5(epfn_detach_to_object, TYPE_bool, EpfnDetachToObject, TYPE_STRING, TYPE_INT, TYPE_DWORD, TYPE_bool, TYPE_INODE_BP);
		FN_1(epfn_split_edges, TYPE_bool, EpfnSplitEdges, TYPE_DWORD);
		FN_1(epfn_break_verts, TYPE_bool, EpfnBreakVerts, TYPE_DWORD);
		FN_3(epfn_divide_face, TYPE_INDEX, EpfnDivideFace, TYPE_INDEX, TYPE_FLOAT_TAB_BR, TYPE_bool);
		FN_3(epfn_divide_edge, TYPE_INDEX, EpfnDivideEdge, TYPE_INDEX, TYPE_FLOAT, TYPE_bool);
		FN_2(epfn_collapse, TYPE_bool, EpfnCollapse, TYPE_INT, TYPE_DWORD);
		VFNT_2(epfn_extrude_faces, EpfnExtrudeFaces, TYPE_FLOAT, TYPE_DWORD);
		VFNT_3(epfn_bevel_faces, EpfnBevelFaces, TYPE_FLOAT, TYPE_FLOAT, TYPE_DWORD);
		VFNT_1(epfn_chamfer_vertices, EpfnChamferVertices, TYPE_FLOAT);
		VFNT_1(epfn_chamfer_edges, EpfnChamferEdges, TYPE_FLOAT);
		FN_4(epfn_slice, TYPE_bool, EpfnSlice, TYPE_POINT3, TYPE_POINT3, TYPE_bool, TYPE_DWORD);
		FN_0(epfn_in_slice_plane_mode, TYPE_bool, EpfnInSlicePlaneMode);
		FN_3(epfn_cut_vertex, TYPE_INDEX, EpfnCutVertex, TYPE_INDEX, TYPE_POINT3, TYPE_POINT3);
		FN_5(epfn_cut_edge, TYPE_INDEX, EpfnCutEdge, TYPE_INDEX, TYPE_FLOAT, TYPE_INDEX, TYPE_FLOAT, TYPE_POINT3);
		FN_4(epfn_cut_face, TYPE_INDEX, EpfnCutFace, TYPE_INDEX, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3);
		FN_3(epfn_weld_verts, TYPE_bool, EpfnWeldVerts, TYPE_INDEX, TYPE_INDEX, TYPE_POINT3);
		FN_2(epfn_weld_edges, TYPE_bool, EpfnWeldEdges, TYPE_INDEX, TYPE_INDEX);
		FN_1(epfn_weld_flagged_verts, TYPE_bool, EpfnWeldFlaggedVerts, TYPE_DWORD);
		FN_1(epfn_weld_flagged_edges, TYPE_bool, EpfnWeldFlaggedEdges, TYPE_DWORD);
		FN_4(epfn_create_shape, TYPE_bool, EpfnCreateShape, TYPE_STRING, TYPE_bool, TYPE_INODE, TYPE_DWORD);
		FNT_2(epfn_make_planar, TYPE_bool, EpfnMakePlanar, TYPE_INT, TYPE_DWORD);
		FNT_4(epfn_move_to_plane, TYPE_bool, EpfnMoveToPlane, TYPE_POINT3, TYPE_FLOAT, TYPE_INT, TYPE_DWORD);
		FN_2(epfn_align_to_grid, TYPE_bool, EpfnAlignToGrid, TYPE_INT, TYPE_DWORD);
		FN_2(epfn_align_to_view, TYPE_bool, EpfnAlignToView, TYPE_INT, TYPE_DWORD);
		FN_0(epfn_delete_iso_verts, TYPE_bool, EpfnDeleteIsoVerts);
		FN_2(epfn_meshsmooth, TYPE_bool, EpfnMeshSmooth, TYPE_INT, TYPE_DWORD );
		FN_2(epfn_tessellate, TYPE_bool, EpfnTessellate, TYPE_INT, TYPE_DWORD );
		VFN_0(epfn_force_subdivision, EpfnForceSubdivision);
		VFN_3(epfn_set_diagonal, EpfnSetDiagonal, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX);
		FN_1(epfn_retriangulate, TYPE_bool, EpfnRetriangulate, TYPE_DWORD);
		FN_1(epfn_flip_normals, TYPE_bool, EpfnFlipNormals, TYPE_DWORD);
		VFNT_2(epfn_select_by_mat, EpfnSelectByMat, TYPE_INDEX, TYPE_bool);
		VFNT_2(epfn_select_by_smooth_group, EpfnSelectBySmoothGroup, TYPE_DWORD, TYPE_bool);
		VFNT_0(epfn_autosmooth, EpfnAutoSmooth);

		VFN_1(epfn_button_op, EpActionButtonOp, TYPE_ENUM);
		VFN_1(epfn_toggle_command_mode, EpActionToggleCommandMode, TYPE_ENUM);
		VFN_1(epfn_enter_pick_mode, EpActionEnterPickMode, TYPE_ENUM);
		VFN_0(epfn_exit_command_modes, EpActionExitCommandModes);

		// Flag accessors:
		FN_3(epfn_get_vertices_by_flag, TYPE_bool, EpGetVerticesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD);
		FN_3(epfn_get_edges_by_flag, TYPE_bool, EpGetEdgesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD);
		FN_3(epfn_get_faces_by_flag, TYPE_bool, EpGetFacesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD);

		VFN_4(epfn_set_vertex_flags, EpSetVertexFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool);
		VFN_4(epfn_set_edge_flags, EpSetEdgeFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool);
		VFN_4(epfn_set_face_flags, EpSetFaceFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool);

		// Data accessors:
		VFN_0(epfn_reset_slice_plane, EpResetSlicePlane);
		VFN_3(epfn_get_slice_plane, EpGetSlicePlane, TYPE_POINT3_BR, TYPE_POINT3_BR, TYPE_FLOAT_BP);
		VFN_3(epfn_set_slice_plane, EpSetSlicePlane, TYPE_POINT3_BR, TYPE_POINT3_BR, TYPE_FLOAT);
		FNT_4(epfn_get_vertex_data, TYPE_FLOAT, GetVertexDataValue, TYPE_INT, TYPE_INT_BP, TYPE_bool_BP, TYPE_DWORD);
		FNT_4(epfn_get_edge_data, TYPE_FLOAT, GetEdgeDataValue, TYPE_INT, TYPE_INT_BP, TYPE_bool_BP, TYPE_DWORD);
		VFNT_3(epfn_set_vertex_data, SetVertexDataValue, TYPE_INT, TYPE_FLOAT, TYPE_DWORD);
		VFNT_3(epfn_set_edge_data, SetEdgeDataValue, TYPE_INT, TYPE_FLOAT, TYPE_DWORD);
		VFN_1(epfn_reset_vertex_data, ResetVertexData, TYPE_INT);
		VFN_1(epfn_reset_edge_data, ResetEdgeData, TYPE_INT);
		VFN_2(epfn_begin_modify_perdata, BeginPerDataModify, TYPE_INT, TYPE_INT);
		FN_0(epfn_in_modify_perdata, TYPE_bool, InPerDataModify);
		VFN_1(epfn_end_modify_perdata, EndPerDataModify, TYPE_bool);
		//FN_3(epfn_get_vertex_color, TYPE_COLOR, GetVertexColor, TYPE_bool_BP, TYPE_INT_BP, TYPE_INT);
		//VFN_2(epfn_set_vertex_color, SetVertexColor, TYPE_COLOR, TYPE_INT);
		//FN_3(epfn_get_face_color, TYPE_COLOR, GetFaceColor, TYPE_bool_BP, TYPE_INT_BP, TYPE_INT);
		//VFN_2(epfn_set_face_color, SetFaceColor, TYPE_COLOR, TYPE_INT);
		//VFN_1(epfn_begin_modify_vertex_color, BeginVertexColorModify, TYPE_INT);
		//FN_0(epfn_in_modify_vertex_color, TYPE_bool, InVertexColorModify);
		//VFN_1(epfn_end_modify_vertex_color, EndVertexColorModify, TYPE_bool);
		FN_1(epfn_get_mat_index, TYPE_INDEX, GetMatIndex, TYPE_bool_BP);
		VFN_2(epfn_set_mat_index, SetMatIndex, TYPE_INDEX, TYPE_DWORD);
		VFN_3(epfn_get_smoothing_groups, GetSmoothingGroups, TYPE_DWORD, TYPE_DWORD_BP, TYPE_DWORD_BP);
		VFN_3(epfn_set_smoothing_groups, SetSmoothBits, TYPE_DWORD, TYPE_DWORD, TYPE_DWORD);

		VFN_0(epfn_collapse_dead_structs, CollapseDeadStructs);
		VFN_0(epfn_collapse_dead_structs_spelled_right, CollapseDeadStructs);
//		VFNT_2(epfn_apply_delta, ApplyDelta, TYPE_POINT3_TAB_BR, TYPE_IOBJECT); // no type corresponding to Tab<Point3>
		FN_7(epfn_propagate_component_flags, TYPE_INT, EpfnPropagateComponentFlags, TYPE_INT, TYPE_DWORD, TYPE_INT, TYPE_DWORD, TYPE_bool, TYPE_bool, TYPE_bool);

		VFN_1 (epfn_preview_begin, EpPreviewBegin, TYPE_ENUM);
		VFN_0 (epfn_preview_cancel, EpPreviewCancel);
		VFN_0 (epfn_preview_accept, EpPreviewAccept);
		VFN_0 (epfn_preview_invalidate, EpPreviewInvalidate);
		FN_0 (epfn_preview_on, TYPE_bool, EpPreviewOn);
		VFN_1 (epfn_preview_set_dragging, EpPreviewSetDragging, TYPE_bool);
		FN_0 (epfn_preview_get_dragging, TYPE_bool, EpPreviewGetDragging);

		FN_1 (epfn_popup_dialog, TYPE_bool, EpfnPopupDialog, TYPE_ENUM);
		VFN_0 (epfn_close_popup_dialog, EpfnClosePopupDialog);

		VFN_0 (epfn_repeat_last, EpfnRepeatLastOperation);
		VFN_1 (epfn_grow_selection, EpfnGrowSelection, TYPE_ENUM);
		VFN_1 (epfn_shrink_selection, EpfnShrinkSelection, TYPE_ENUM);
		FN_3 (epfn_convert_selection, TYPE_INT, EpfnConvertSelection, TYPE_ENUM, TYPE_ENUM, TYPE_bool);
		VFN_0 (epfn_select_border, EpfnSelectBorder);
		VFN_0 (epfn_select_element, EpfnSelectElement);
		VFN_0 (epfn_select_edge_loop, EpfnSelectEdgeLoop);
		VFN_0 (epfn_select_edge_ring, EpfnSelectEdgeRing);
		FN_2 (epfn_remove, TYPE_bool, EpfnRemove, TYPE_ENUM, TYPE_DWORD);
		FN_0 (epfn_delete_iso_map_verts, TYPE_bool, EpfnDeleteIsoMapVerts);
		FN_1 (epfn_outline, TYPE_bool, EpfnOutline, TYPE_DWORD);
		FN_1 (epfn_connect_edges, TYPE_bool, EpfnConnectEdges, TYPE_DWORD);
		FN_1 (epfn_connect_vertices, TYPE_bool, EpfnConnectVertices, TYPE_DWORD);
		FN_1 (epfn_extrude_along_spline, TYPE_bool, EpfnExtrudeAlongSpline, TYPE_DWORD);
		FN_1 (epfn_lift_from_edge, TYPE_bool, EpfnLiftFromEdge, TYPE_DWORD);
		VFN_0 (epfn_toggle_shaded_faces, EpfnToggleShadedFaces);

		FN_0 (epfn_get_epoly_sel_level, TYPE_ENUM, GetEPolySelLevel);
		FN_0 (epfn_get_mn_sel_level, TYPE_ENUM, GetMNSelLevel);

		FN_1 (epfn_get_selection, TYPE_BITARRAY, EpfnGetSelection, TYPE_ENUM);
		VFN_2 (epfn_set_selection, EpfnSetSelection, TYPE_ENUM, TYPE_BITARRAY);

		FN_0 (epfn_get_num_vertices, TYPE_INT, EpfnGetNumVertices);
		FN_1 (epfn_get_vertex, TYPE_POINT3_BV, EpfnGetVertex, TYPE_INDEX);
		FN_1 (epfn_get_vertex_face_count, TYPE_INT, EpfnGetVertexFaceCount, TYPE_INDEX);
		FN_2 (epfn_get_vertex_face, TYPE_INDEX, EpfnGetVertexFace, TYPE_INDEX, TYPE_INDEX);
		FN_1 (epfn_get_vertex_edge_count, TYPE_INT, EpfnGetVertexEdgeCount, TYPE_INDEX);
		FN_2 (epfn_get_vertex_edge, TYPE_INDEX, EpfnGetVertexEdge, TYPE_INDEX, TYPE_INDEX);

		FN_0 (epfn_get_num_edges, TYPE_INT, EpfnGetNumEdges);
		FN_2 (epfn_get_edge_vertex, TYPE_INDEX, EpfnGetEdgeVertex, TYPE_INDEX, TYPE_INDEX);
		FN_2 (epfn_get_edge_face, TYPE_INDEX, EpfnGetEdgeFace, TYPE_INDEX, TYPE_INDEX);

		FN_0 (epfn_get_num_faces, TYPE_INT, EpfnGetNumFaces);
		FN_1 (epfn_get_face_degree, TYPE_INT, EpfnGetFaceDegree, TYPE_INDEX);
		FN_2 (epfn_get_face_vertex, TYPE_INDEX, EpfnGetFaceVertex, TYPE_INDEX, TYPE_INDEX);
		FN_2 (epfn_get_face_edge, TYPE_INDEX, EpfnGetFaceEdge, TYPE_INDEX, TYPE_INDEX);
		FN_1 (epfn_get_face_material, TYPE_INDEX, EpfnGetFaceMaterial, TYPE_INDEX);
		FN_1 (epfn_get_face_smoothing_group, TYPE_DWORD, EpfnGetFaceSmoothingGroup, TYPE_INDEX);

		FN_0 (epfn_get_num_map_channels, TYPE_INT, EpfnGetNumMapChannels);
		FN_1 (epfn_get_map_channel_active, TYPE_bool, EpfnGetMapChannelActive, TYPE_INT);
		FN_1 (epfn_get_num_map_vertices, TYPE_INT, EpfnGetNumMapVertices, TYPE_INT);
		FN_2 (epfn_get_map_vertex, TYPE_POINT3_BV, EpfnGetMapVertex, TYPE_INT, TYPE_INDEX);
		FN_3 (epfn_get_map_face_vertex, TYPE_INT, EpfnGetMapFaceVertex, TYPE_INT, TYPE_INDEX, TYPE_INDEX);
		FN_3 (epfn_get_map_face_vert, TYPE_INDEX, EpfnGetMapFaceVertex, TYPE_INT, TYPE_INDEX, TYPE_INDEX);
	END_FUNCTION_MAP

	FPInterfaceDesc *GetDesc ();

	// General interface tools for accessing polymesh, updating object.
	virtual MNMesh *GetMeshPtr () { return NULL; }
	virtual IParamBlock2 *getParamBlock () { return NULL; }
	virtual void LocalDataChanged (DWORD parts) { }
	// Invalidates soft selection values:
	virtual void InvalidateSoftSelectionCache () { }
	// Invalidates precomputed distances on which soft selection values are based
	// (as well as soft selection values) - note this is automatically done when
	// you call LocalDataChanged (PART_GEOM).  Should also be done if the soft
	// selection parameters are changed.
	virtual void InvalidateDistanceCache () { }
	// Simple call to ip->RedrawViewports.
	virtual void RefreshScreen () { }
	// returns TRUE if between BeginEditParams, EndEditParams
	virtual bool Editing () { return FALSE; }

	// Selection level accessors:
	virtual int GetEPolySelLevel() { return EP_SL_OBJECT; }
	virtual int GetMNSelLevel () { return MNM_SL_OBJECT; }
	virtual void SetEPolySelLevel(int level) { }

	// Component flag accessors:
	virtual bool EpGetVerticesByFlag (BitArray & vset, DWORD flags, DWORD fmask=0x0) { return false; }
	virtual bool EpGetEdgesByFlag (BitArray & eset, DWORD flags, DWORD fmask=0x0) { return false; }
	virtual bool EpGetFacesByFlag (BitArray & fset, DWORD flags, DWORD fmask=0x0) { return false; }
	virtual void EpSetVertexFlags (BitArray &vset, DWORD flags, DWORD fmask=0x0, bool undoable=true) { return; }
	virtual void EpSetEdgeFlags (BitArray &eset, DWORD flags, DWORD fmask = 0x0, bool undoable=true) { return; }
	virtual void EpSetFaceFlags (BitArray &fset, DWORD flags, DWORD fmask = 0x0, bool undoable=true) { return; }

	// Action interface stuff:
	virtual void EpActionButtonOp (int opcode) { }
	virtual void EpActionToggleCommandMode (int mode) { }
	virtual void EpActionEnterPickMode (int mode) { }
	virtual void EpActionExitCommandModes () { }

	// Transform commands:
	virtual void MoveSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) { }
	virtual void RotateSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin) { }
	virtual void ScaleSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) { }
	virtual void ApplyDelta (Tab<Point3> & delta, EPoly *epol, TimeValue t) { }

	// Slice plane accessors:
	virtual void EpResetSlicePlane () { }
	virtual void EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float *planeSize=NULL) { }
	virtual void EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float planeSize) { }

	// Component data access methods:
	virtual Color GetVertexColor (bool *uniform=NULL, int *num=NULL, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { return Color(1,1,1); }
	virtual void SetVertexColor (Color clr, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { }
	virtual Color GetFaceColor (bool *uniform=NULL, int *num=NULL, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { return Color(1,1,1); }
	virtual void SetFaceColor (Color clr, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { }
	virtual float GetVertexDataValue (int channel, int *numSel, bool *uniform, DWORD vertexFlags, TimeValue t) { return 1.0f; }
	virtual float GetEdgeDataValue (int channel, int *numSel, bool *uniform, DWORD edgeFlags, TimeValue t) { return 1.0f; }
	virtual void SetVertexDataValue (int channel, float w, DWORD vertexFlags, TimeValue t) { }
	virtual void SetEdgeDataValue (int channel, float w, DWORD edgeFlags, TimeValue t) { }
	virtual void ResetVertexData (int channel) { }
	virtual void ResetEdgeData (int channel) { }
	virtual void BeginPerDataModify (int mnSelLevel, int channel) { }
	virtual bool InPerDataModify () { return false; }
	virtual void EndPerDataModify (bool success) { }
	virtual void BeginVertexColorModify (int mp=0) { }
	virtual bool InVertexColorModify () { return false; }
	virtual void EndVertexColorModify (bool success) { }

	// Face level characteristics:
	virtual int GetMatIndex (bool *determined) { determined=false; return 0; }
	virtual void SetMatIndex (int index, DWORD flag=MN_SEL) { }
	virtual void GetSmoothingGroups (DWORD faceFlag, DWORD *anyFaces, DWORD *allFaces=NULL) { if (anyFaces) *anyFaces = 0; if (allFaces) *allFaces = 0; }
	virtual void SetSmoothBits (DWORD bits, DWORD bitmask, DWORD flag) { }

	// Displacement approximation methods:
	// Copy displacement parameters from pblock to polyobject:
	virtual void SetDisplacementParams () { }
	// Copy displacement parameters from polyobject to pblock:
	virtual void UpdateDisplacementParams () { }
	// Engage a displacement approximation preset:
	virtual void UseDisplacementPreset (int presetNumber) { }

	// Drag operations:
	virtual void EpfnBeginExtrude (int msl, DWORD flag, TimeValue t) { }
	virtual void EpfnEndExtrude (bool accept, TimeValue t) { }
	virtual void EpfnDragExtrude (float amount, TimeValue t) { }
	virtual void EpfnBeginBevel (int msl, DWORD flag, bool doExtrude, TimeValue t) { }
	virtual void EpfnEndBevel (bool accept, TimeValue t) { }
	virtual void EpfnDragBevel (float outline, float height, TimeValue t) { }
	virtual void EpfnBeginChamfer (int msl, TimeValue t) { }
	virtual void EpfnEndChamfer (bool accept, TimeValue t) { }
	virtual void EpfnDragChamfer (float amount, TimeValue t) { }

	// Operations:
	// Select panel ops:
	virtual bool EpfnHide (int msl, DWORD flags) { return false; }
	virtual bool EpfnUnhideAll (int msl) { return false; }
	virtual void EpfnNamedSelectionCopy (TSTR setName) {}
	virtual void EpfnNamedSelectionPaste (bool useDlgToRename) {}

	// Topological & Geometric ops from the Edit Geometry panel:
	// Create button:
	virtual int EpfnCreateVertex(Point3 pt, bool pt_local=false, bool select=true) { return -1; }
	virtual int EpfnCreateEdge (int v1, int v2, bool select=true) { return -1; }
	virtual int EpfnCreateFace(int *v, int deg, bool select=true) { return -1; }
	virtual bool EpfnCapHoles (int msl=MNM_SL_EDGE, DWORD targetFlags=MN_SEL) { return false; }
	// Delete button:
	virtual bool EpfnDelete (int msl, DWORD delFlag=MN_SEL, bool delIsoVerts=false) { return false; }
	// Attach button:
	virtual void EpfnAttach (INode *node, bool & canUndo, INode *myNode, TimeValue t) {}
	// Detach button:
	virtual void EpfnMultiAttach (INodeTab &nodeTab, INode *myNode, TimeValue t) {}
	virtual bool EpfnDetachToElement (int msl, DWORD flag, bool keepOriginal) { return false; }
	virtual bool EpfnDetachToObject (TSTR name, int msl, DWORD flag, bool keepOriginal, INode *myNode, TimeValue t) { return false; }
	virtual bool EpfnSplitEdges (DWORD flag=MN_SEL) { return false; }
	// Break/divide button:
	virtual bool EpfnBreakVerts (DWORD flag=MN_SEL) { return false; }
	virtual int EpfnDivideFace (int face, Tab<float> &bary, bool select=true) { return -1; }
	virtual int EpfnDivideEdge (int edge, float prop, bool select=true) { return -1; }
	// Collapse button:
	virtual bool EpfnCollapse (int msl, DWORD flag) { return false; }

	// Extrude/Bevel/Chamfer section:
	virtual void EpfnExtrudeFaces (float amount, DWORD flag, TimeValue t) {}
	virtual void EpfnBevelFaces (float height, float outline, DWORD flag, TimeValue t) {}
	virtual void EpfnChamferVertices (float amount, TimeValue t) {}
	virtual void EpfnChamferEdges (float amount, TimeValue t) {}

	// Cut 'n' Slice
	virtual bool EpfnSlice (Point3 planeNormal, Point3 planeCenter, bool flaggedFacesOnly=false, DWORD faceFlags=MN_SEL) { return false; }
	virtual bool EpfnInSlicePlaneMode () { return false; }
	virtual int EpfnCutVertex (int startv, Point3 destination, Point3 projDir) { return -1; }
	virtual int EpfnCutEdge (int e1, float prop1, int e2, float prop2, Point3 projDir) { return -1; }
	virtual int EpfnCutFace (int f1, Point3 p1, Point3 p2, Point3 projDir) { return -1; }

	// Weld section:
	virtual bool EpfnWeldVerts (int vert1, int vert2, Point3 destination) { return false; }
	virtual bool EpfnWeldEdges (int edge1, int edge2) { return false; }
	virtual bool EpfnWeldFlaggedVerts (DWORD flag) { return false; }
	virtual bool EpfnWeldFlaggedEdges (DWORD flag) { return false; }

	// Rest of Geom Panel
	virtual bool EpfnCreateShape (TSTR name, bool smooth, INode *myNode, DWORD edgeFlag=MN_SEL) { return false; }
	virtual bool EpfnMakePlanar (int msl, DWORD flag=MN_SEL, TimeValue t=0) { return false; }
	virtual bool EpfnMoveToPlane (Point3 planeNormal, float planeOffset, int msl, DWORD flag=MN_SEL, TimeValue t=0) { return false; }
	virtual bool EpfnAlignToGrid (int msl, DWORD flag=MN_SEL) { return false; }
	virtual bool EpfnAlignToView (int msl, DWORD flag=MN_SEL) { return false; }
	virtual bool EpfnDeleteIsoVerts () { return false; }

	// Subdivision panel
	virtual bool EpfnMeshSmooth (int msl, DWORD flag=MN_SEL)  { return false; }
	virtual bool EpfnTessellate (int msl, DWORD flag=MN_SEL) { return false; }

	// Surface panels
	virtual void EpfnForceSubdivision () { }
	virtual void EpfnSelectVertByColor (BOOL add, BOOL sub, int mp=0, TimeValue t=0) { }

	virtual void EpfnSetDiagonal (int face, int corner1, int corner2) { }
	virtual bool EpfnRetriangulate (DWORD flag=MN_SEL) { return false; }
	virtual bool EpfnFlipNormals (DWORD flag=MN_SEL) { return false; }
	virtual void EpfnSelectByMat (int index, bool clear, TimeValue t) { }
	virtual void EpfnSelectBySmoothGroup (DWORD bits, BOOL clear, TimeValue t) { }
	virtual void EpfnAutoSmooth (TimeValue t) { }

	// other things...
	virtual void CollapseDeadStructs () { }
	virtual int EpfnPropagateComponentFlags (int slTo, DWORD flTo, int slFrom, DWORD flFrom, bool ampersand=FALSE, bool set=TRUE, bool undoable=FALSE) { return 0; }

	// New stuff we'd like to add, but have to keep private for SDK compatibility
	// (We add as private so we can function-publish.)
private:
	// Access to new Preview mode:
	virtual void EpPreviewBegin (int previewOperation) { }
	virtual void EpPreviewCancel () { }
	virtual void EpPreviewAccept () { }
	virtual void EpPreviewInvalidate () { }
	virtual bool EpPreviewOn () { return false; }
	virtual void EpPreviewSetDragging (bool drag) { }
	virtual bool EpPreviewGetDragging () { return false; }

	virtual bool EpfnPopupDialog (int popupOperation) { return false; }
	virtual void EpfnClosePopupDialog () { }

	virtual void EpfnRepeatLastOperation () { }

	virtual void EpfnGrowSelection (int meshSelLevel) { }
	virtual void EpfnShrinkSelection (int meshSelLevel) { }
	virtual int EpfnConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll=false) { return false; }
	virtual void EpfnSelectBorder () { }
	virtual void EpfnSelectElement () { }
	virtual void EpfnSelectEdgeLoop () { }
	virtual void EpfnSelectEdgeRing () { }
	virtual bool EpfnRemove (int msl, DWORD delFlag=MN_SEL) { return false; }
	virtual bool EpfnDeleteIsoMapVerts () { return false; }
	virtual bool EpfnOutline (DWORD flag) { return false; }
	virtual bool EpfnConnectEdges (DWORD edgeFlag) { return false; }
	virtual bool EpfnConnectVertices (DWORD vertexFlag) { return false; }
	virtual bool EpfnExtrudeAlongSpline (DWORD faceFlag) { return false; }
	virtual bool EpfnLiftFromEdge (DWORD faceFlag) { return false; }
	virtual void EpfnToggleShadedFaces () { }
	virtual int EpfnCreateFace2 (Tab<int> *vertices, bool select=true) { return -1; }
	virtual void EpfnAttach (INode *node, INode *myNode, TimeValue t) { }

	virtual BitArray *EpfnGetSelection (int msl) { return NULL; }
	virtual void EpfnSetSelection (int msl, BitArray *newSel) { }

	virtual int EpfnGetNumVertices () { return 0; }
	virtual Point3 EpfnGetVertex (int vertIndex) { return Point3(0,0,0); }
	virtual int EpfnGetVertexFaceCount (int vertIndex) { return 0; }
	virtual int EpfnGetVertexFace (int vertIndex, int whichFace) { return 0; }
	virtual int EpfnGetVertexEdgeCount (int vertIndex) { return 0; }
	virtual int EpfnGetVertexEdge (int vertIndex, int whichEdge) { return 0; }

	virtual int EpfnGetNumEdges () { return 0; }
	virtual int EpfnGetEdgeVertex (int edgeIndex, int end) { return 0; }
	virtual int EpfnGetEdgeFace (int edgeIndex, int side) { return 0; }

	virtual int EpfnGetNumFaces() { return 0; }
	virtual int EpfnGetFaceDegree (int faceIndex) { return 0; }
	virtual int EpfnGetFaceVertex (int faceIndex, int corner) { return 0; }
	virtual int EpfnGetFaceEdge (int faceIndex, int side) { return 0; }
	virtual int EpfnGetFaceMaterial (int faceIndex) { return 0; }
	virtual DWORD EpfnGetFaceSmoothingGroup (int faceIndex) { return 0; }

	virtual int EpfnGetNumMapChannels () { return 0; }
	virtual bool EpfnGetMapChannelActive (int mapChannel) { return false; }
	virtual int EpfnGetNumMapVertices (int mapChannel) { return 0; }
	virtual UVVert EpfnGetMapVertex (int mapChannel, int vertIndex) { return UVVert(0,0,0); }
	virtual int EpfnGetMapFaceVertex (int mapChannel, int faceIndex, int corner) { return 0; }
};

#endif //__POLYOBJED_INTERFACE_
