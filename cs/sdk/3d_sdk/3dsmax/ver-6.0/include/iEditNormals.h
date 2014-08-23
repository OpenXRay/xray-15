/**********************************************************************
 *<
	FILE: iEditNormals.h

	DESCRIPTION:   Edit Normals Modifier SDK Interface

	CREATED BY: Steve Anderson

	HISTORY: created January 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef __EDITNORMALSMOD_INTERFACE_
#define __EDITNORMALSMOD_INTERFACE_

#include "iFnPub.h"

// Edit Normals Selection Levels - selection is always some set of normals
// (vertex/face selLevels are just rules for hit testing - select normals by vertex, or by face.)
enum editNormalSelLevel { EN_SL_OBJECT, EN_SL_NORMAL, EN_SL_VERTEX, EN_SL_EDGE, EN_SL_FACE };

// Edit Normals Parameters:
enum editNormalParameters { en_display_length, en_ignore_backfacing, en_select_by, en_show_handles };

#define EDIT_NORMALS_MOD_INTERFACE Interface_ID(0x2b572ad6,0x7cf86ae2)

// "Mixin" Interface methods:
enum editNormalsModMethods { enfn_get_sel_level, enfn_set_sel_level, enfn_move,
	enfn_rotate, enfn_break, enfn_unify, enfn_reset, enfn_specify,
	enfn_make_explicit, enfn_copy, enfn_paste,
	enfn_select, enfn_get_selection, enfn_set_selection,
	enfn_convert_vertex_selection, enfn_convert_edge_selection,
	enfn_convert_face_selection, enfn_get_num_normals,
	enfn_get_num_faces, enfn_get_normal, enfn_set_normal,
	enfn_get_normal_explicit, enfn_set_normal_explicit,
	enfn_get_face_degree, enfn_get_normal_id, enfn_set_normal_id,
	enfn_get_num_vertices, enfn_get_vertex_id, enfn_get_vertex,
	enfn_get_num_edges, enfn_get_edge_id, enfn_get_edge_vertex,
	enfn_get_face_edge_side, enfn_get_edge_face, enfn_get_edge_normal,
	enfn_get_face_normal_specified, enfn_set_face_normal_specified,
	enfn_rebuild_normals, enfn_recompute_normals, enfn_average,
	enfn_average_global, enfn_average_two };

// Enum of enums, for methods which accept enum parameters
enum editNormalsModEnums { enprop_sel_level };

class IEditNormalsMod : public FPMixinInterface {
public:
	BEGIN_FUNCTION_MAP
		// Selection Level Access
		PROP_FNS(enfn_get_sel_level, EnfnGetSelLevel, enfn_set_sel_level, EnfnSetSelLevel, TYPE_INT);

		// Transforms
		FNT_1(enfn_move, TYPE_bool, EnfnMove, TYPE_POINT3_BR);
		FNT_1(enfn_rotate, TYPE_bool, EnfnRotate, TYPE_QUAT_BR);

		// Operations
		FN_3(enfn_break, TYPE_bool, EnfnBreakNormals, TYPE_BITARRAY, TYPE_INODE, TYPE_bool);
		FN_3(enfn_unify, TYPE_bool, EnfnUnifyNormals, TYPE_BITARRAY, TYPE_INODE, TYPE_bool);
		FN_2(enfn_reset, TYPE_bool, EnfnResetNormals, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_specify, TYPE_bool, EnfnSpecifyNormals, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_make_explicit, TYPE_bool, EnfnMakeNormalsExplicit, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_copy, TYPE_bool, EnfnCopyNormal, TYPE_INDEX, TYPE_INODE);
		FN_2(enfn_paste, TYPE_bool, EnfnPasteNormal, TYPE_BITARRAY, TYPE_INODE);
		FN_4(enfn_average, TYPE_bool, EnfnAverageNormals, TYPE_bool, TYPE_FLOAT, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_average_global, TYPE_bool, EnfnAverageGlobalNormals, TYPE_bool, TYPE_FLOAT);
		FN_4(enfn_average_two, TYPE_bool, EnfnAverageTwoNormals, TYPE_INODE, TYPE_INDEX, TYPE_INODE, TYPE_INDEX);

		// Selection set access
		FN_1(enfn_get_selection, TYPE_BITARRAY, EnfnGetSelection, TYPE_INODE);
		FN_2(enfn_set_selection, TYPE_bool, EnfnSetSelection, TYPE_BITARRAY_BR, TYPE_INODE);
		FN_4(enfn_select, TYPE_bool, EnfnSelect, TYPE_BITARRAY_BR, TYPE_bool, TYPE_bool, TYPE_INODE);
		VFN_3(enfn_convert_vertex_selection, EnfnConvertVertexSelection, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE);
		VFN_3(enfn_convert_edge_selection, EnfnConvertEdgeSelection, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE);
		VFN_3(enfn_convert_face_selection, EnfnConvertFaceSelection, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE);

		// Normal access
		FN_1(enfn_get_num_normals, TYPE_INT, EnfnGetNumNormals, TYPE_INODE);
		FNT_2(enfn_get_normal, TYPE_POINT3, EnfnGetNormal, TYPE_INDEX, TYPE_INODE);
		VFNT_3(enfn_set_normal, EnfnSetNormal, TYPE_INDEX, TYPE_POINT3_BR, TYPE_INODE);
		FN_2(enfn_get_normal_explicit, TYPE_bool, EnfnGetNormalExplicit, TYPE_INDEX, TYPE_INODE);
		VFN_3(enfn_set_normal_explicit, EnfnSetNormalExplicit, TYPE_INDEX, TYPE_bool, TYPE_INODE);

		// Normal face access
		FN_1(enfn_get_num_faces, TYPE_INT, EnfnGetNumFaces, TYPE_INODE);
		FN_2(enfn_get_face_degree, TYPE_INT, EnfnGetFaceDegree, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_normal_id, TYPE_INDEX, EnfnGetNormalID, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_4(enfn_set_normal_id, EnfnSetNormalID, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_face_normal_specified, TYPE_bool, EnfnGetFaceNormalSpecified, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_4(enfn_set_face_normal_specified, EnfnSetFaceNormalSpecified, TYPE_INDEX, TYPE_INDEX, TYPE_bool, TYPE_INODE);

		// Vertex access
		FN_1(enfn_get_num_vertices, TYPE_INT, EnfnGetNumVertices, TYPE_INODE);
		FN_3(enfn_get_vertex_id, TYPE_INDEX, EnfnGetVertexID, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FNT_2(enfn_get_vertex, TYPE_POINT3_BV, EnfnGetVertex, TYPE_INDEX, TYPE_INODE);

		// Edge access
		FN_1(enfn_get_num_edges, TYPE_INT, EnfnGetNumEdges, TYPE_INODE);
		FN_3(enfn_get_edge_id, TYPE_INDEX, EnfnGetEdgeID, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_face_edge_side, TYPE_INDEX, EnfnGetFaceEdgeSide, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_edge_vertex, TYPE_INDEX, EnfnGetEdgeVertex, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_edge_face, TYPE_INDEX, EnfnGetEdgeFace, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_4(enfn_get_edge_normal, TYPE_INDEX, EnfnGetEdgeNormal, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		// Cache rebuilders - shouldn't normally be necessary - above methods do this work as needed.
		VFN_1(enfn_rebuild_normals, EnfnRebuildNormals, TYPE_INODE);
		VFN_1(enfn_recompute_normals, EnfnRecomputeNormals, TYPE_INODE);
	END_FUNCTION_MAP

	FPInterfaceDesc *GetDesc ();

	// Selection level accessors:
	virtual int EnfnGetSelLevel () { return EN_SL_OBJECT; }
	virtual void EnfnSetSelLevel (int selLevel) { }

	// Transform commands:
	virtual bool EnfnMove (Point3& offset, TimeValue t) { return false; }
	virtual bool EnfnRotate (Quat & rotation, TimeValue t) { return false; }

	// Operations:
	virtual bool EnfnBreakNormals (BitArray *normalSelection=NULL, INode *pNode=NULL, bool toAverage=false) { return false; }
	virtual bool EnfnUnifyNormals (BitArray *normalSelection=NULL, INode *pNode=NULL, bool toAverage=false) { return false; }
	virtual bool EnfnResetNormals (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	virtual bool EnfnSpecifyNormals (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	virtual bool EnfnMakeNormalsExplicit (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	virtual bool EnfnCopyNormal (int normalID, INode *pNode=NULL) { return false; }
	virtual bool EnfnPasteNormal (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	virtual bool EnfnAverageNormals (bool useThresh=false, float threshold=0.0f, BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	virtual bool EnfnAverageGlobalNormals (bool useThresh=false, float threshold=0.0f) { return false; }
	virtual bool EnfnAverageTwoNormals (INode *pNode1, int normID1, INode *pNode2, int normID2) { return false; }

	// Selection accessors
	virtual BitArray *EnfnGetSelection (INode *pNode=NULL) { return NULL; }
	virtual bool EnfnSetSelection (BitArray & selection, INode *pNode=NULL) { return false; }
	virtual bool EnfnSelect (BitArray & selection, bool invert=false, bool select=true, INode *pNode=NULL) { return false; }
	virtual void EnfnConvertVertexSelection (BitArray & vertexSelection, BitArray & normalSelection, INode *pNode=NULL) { }
	virtual void EnfnConvertEdgeSelection (BitArray & edgeSelection, BitArray & normalSelection, INode *pNode=NULL) { }
	virtual void EnfnConvertFaceSelection (BitArray & faceSelection, BitArray & normalSelection, INode *pNode=NULL) { }

	// Accessors for the normals:
	virtual int EnfnGetNumNormals (INode *pNode=NULL) { return 0; }
	// Direct access to the normals themselves:
	virtual Point3 *EnfnGetNormal (int normalID, INode *pNode=NULL, TimeValue t=0) { return NULL; }
	virtual void EnfnSetNormal (int normalID, Point3 &direction, INode *pNode=NULL, TimeValue t=0) { }
	// Control whether a given normal is built from smoothing groups or set to an explicit value
	virtual bool EnfnGetNormalExplicit (int normID, INode *pNode=NULL) { return false; }
	// (Also makes the normal specified for all faces using this normal.)
	virtual void EnfnSetNormalExplicit (int normID, bool value, INode *pNode=NULL) { }

	// Normals can be used by multiple faces, even at different vertices.
	// So we require access to both face and normal information.
	// Access to the normals that are assigned to each face:
	virtual int EnfnGetNumFaces (INode *pNode=NULL) { return 0; }
	virtual int EnfnGetFaceDegree (int face, INode *pNode=NULL) { return 0; }
	virtual int EnfnGetNormalID (int face, int corner, INode *pNode=NULL) { return 0; }
	virtual void EnfnSetNormalID (int face, int corner, int normalID, INode *pNode=NULL) { }

	// Control whether a corner of a face uses an specified normal ID, or builds normals based on smoothing groups.
	virtual bool EnfnGetFaceNormalSpecified (int face, int corner, INode *pNode=NULL) { return false; }
	virtual void EnfnSetFaceNormalSpecified (int face, int corner, bool specified, INode *pNode=NULL) { }

	// Access to vertices - often important for proper normal handling to have access to the vertex the normal is based on.
	virtual int EnfnGetNumVertices (INode *pNode=NULL) {return 0; }
	virtual int EnfnGetVertexID (int face, int corner, INode *pNode=NULL) { return 0; }
	virtual Point3 EnfnGetVertex (int vertexID, INode *pNode=NULL, TimeValue t=0) { return Point3(0,0,0); }

	// Access to edges
	virtual int EnfnGetNumEdges (INode *pNode=NULL) { return 0; }
	virtual int EnfnGetEdgeID (int faceIndex, int sideIndex, INode *pNode=NULL) { return 0; }
	virtual int EnfnGetFaceEdgeSide (int faceIndex, int edgeIndex, INode *pNode=NULL) { return 0; }
	virtual int EnfnGetEdgeVertex (int edgeIndex, int end, INode *pNode=NULL) { return 0; }
	virtual int EnfnGetEdgeFace (int edgeIndex, int side, INode *pNode=NULL) { return 0; }
	virtual int EnfnGetEdgeNormal (int edgeIndex, int end, int side, INode *pNode=NULL) { return 0; }

	// Rebuild all non-specified normals from smoothing groups
	// Note that this can change the number of normals in some cases, and often changes their order.
	virtual void EnfnRebuildNormals (INode *pNode=NULL) { }
	// Recompute - computes nonexplicit normals from face normals.
	virtual void EnfnRecomputeNormals (INode *pNode=NULL) { }

	// Direct accessors to MNMesh and MNNormalSpec classes - unpublished for now.
	virtual MNMesh *EnfnGetMesh (INode *pNode=NULL, TimeValue t=0) { return NULL; }
	virtual MNNormalSpec *EnfnGetNormals (INode *pNode=NULL, TimeValue t=0) { return NULL; }
};

#endif //__EDITNORMALSMOD_INTERFACE_
