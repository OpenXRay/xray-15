
/*****************************************************************

  This is just a header that contains all our constants and enums

******************************************************************/

#ifndef __BONESDEF_CONSTANTS__H
#define __BONESDEF_CONSTANTS__H

//Joint Gizmo Class ID
#define GIZMOJOINT_CLASSID Class_ID(9815854,999622)


// Reference IDs for param blocks and the start of our dynamic references
#define PBLOCK_PARAM_REF		0		//basic param block ref
#define POINT1_REF				1		//ref to point3 controller that is used to handle viewport input
#define PBLOCK_DISPLAY_REF		2		//display param block
#define PBLOCK_ADVANCE_REF		3		//advance param block
#define PBLOCK_GIZMOS_REF		4		//gizmo param block

//WEIGHTTABLE
#define PBLOCK_WEIGHTTABLE_REF	5		//weight table param block	

//MIRROR
#define PBLOCK_MIRROR_REF		6		//mirror param pblock

#define BONES_REF				10		//the start of our dynamic entries for bone references


//These are all our save data chunks
//for the modifier and local data
#define BASE_TM_CHUNK           0x10		
#define BONE_COUNT_CHUNK        0x20		
#define BONE_DATATM_CHUNK       0x25
#define BONE_DATA_CHUNK         0x30
#define VERTEX_COUNT_CHUNK      0x40
#define VERTEX_DATA_CHUNK       0x50
#define BONE_NAME_CHUNK			0x160
#define BONE_BIND_CHUNK			0x200
#define BONE_SPLINE_CHUNK		0x210

#define EXCLUSION_CHUNK			0x220
#define VER_CHUNK				0x230
#define GIZMOCOUNT_CHUNK		0x240
#define GIZMODATA_CHUNK			0x250

#define GIZMOTM_CHUNK			0x260
#define USENEWROTATION_CHUNK	0x270

#define DELTA_COUNT_CHUNK		0x400
#define DELTA_DATA_CHUNK		0x410

#define NAMEDSEL_STRING_CHUNK		0x420		//Named selection stuff
#define NAMEDSEL_STRINGCOUNT_CHUNK  0x430
#define NAMEDSEL_STRINGID_CHUNK     0x440
#define NAMEDSEL_BITS_CHUNK			0x450
#define NAMEDSEL_BITSCOUNT_CHUNK    0x460
#define NAMEDSEL_BITSID_CHUNK       0x470

#define WEIGHTTABLE_CHUNK			0x490

#define BONE_INITDATATM_CHUNK		0x480

#define VERTEXV5_DATA_CHUNK         0x490
#define BASENODE_TM_CHUNK           0x500
//5.1.02
#define MESHNODEBACKPATCH_CHUNK		0x510
//5.1.03
#define BONE_INITSTRETCHTM_CHUNK	0x520

//These are our fall off flags for the envelopes
#define BONE_FALLOFF_X3_FLAG    0
#define BONE_FALLOFF_X2_FLAG    1
#define BONE_FALLOFF_X_FLAG     2
#define BONE_FALLOFF_SINE_FLAG  3
#define BONE_FALLOFF_2X_FLAG    4
#define BONE_FALLOFF_3X_FLAG    5

//Command IDs for the insert cross section and paint mode
#define CID_CREATECROSS CID_USER + 203
#define CID_CREATEPAINT CID_USER + 204

//These are IDs for the bone property flyout in the UI
#define ID_LOCK				0x0100
#define ID_ABSOLUTE			0x0110
#define ID_DRAW_ENVELOPE	0x0120
#define ID_FALLOFF			0x0130
#define ID_COPY				0x0140
#define ID_PASTE			0x0150

//these are vertex flag attributes
#define VERTEXFLAG_MODIFIED		2	//whether the vertex has been hand weighted
#define VERTEXFLAG_UNNORMALIZED	4	//whether the vertex is normalized
#define VERTEXFLAG_RIGID		8	//whether the vertex is rigid,if it is rigid only one bone will be affect the vertex
#define VERTEXFLAG_RIGIDHANDLE	16	//only applies to patches, when set if it is a handle it will use the weights of the knot that owns the handle
#define VERTEXFLAG_TEMPSELECTED	32  //used internally to hold temporary selections for cut and paste


#define SELMOD 26

//--- Parameter map/block descriptors -------------------------------
enum { skin_params, skin_display, skin_advance, skin_gizmos,
		skin_weighttable,
//MIRROR
		skin_mirror};

enum {	skin_mirrorplane,		//the mirror plane axis X,Y,Z
		skin_mirroroffset,		//the offset along the axis
		skin_mirrorinitialtm,	//NOT USED ANYMOR
		skin_mirrorenabled,		//whether or not the mirror mode is on
		skin_mirrorthreshold,	//the threshold used to detect matiching vertices and bones
		skin_mirrorprojection,	//this is a display flag
								//	0 the mirror vertices are displayed normally, matched vertices are either
								//		displayed in red or green depending on which side of the plane they are on
								//	1 positive projection, the vertices on the positive side of the plane are prjected
								//		across to the negative side to see where they land
								//	2 negative projection just the flip of 1
		skin_mirrormanualupdate,		//this is a toggle that lets you update the mirror data by hand

		skin_mirrorfast
		};

//Skin Param Block Basic Parameters
enum { skin_effect,					//
	   skin_filter_vertices,		// filters vertex hit testing
	   skin_filter_bones,			// filters cross section hit testing
	   skin_filter_envelopes,		// filters bone hit testing
	   skin_draw_all_envelopes,		// displays all envelopes in the viewpport
	   skin_draw_vertices,			// whether to display vertex ticks
	   skin_ref_frame,				// the reference frame for skin, the reference frame
									// is used to get the initial bone node tms when the user
									// resets skin

	   skin_paint_radius,			// no longer used
	   skin_paint_feather,			// no longer used

	   skin_cross_radius,			// this holds the current selected cross section radius, this is 
									// is just a stub for the UI the cross section radii is stored in the bone data

	   skin_always_deform,			// this toggles whether the skin deformation is on or off
									// when toggled from off to on, this will cause the all the initial bone
									// matrices will reset


	   skin_paint_str,				// no longer used

	   skin_local_squash,			// a table of the local squash values which the user can set to  
									// adjust the squash amount
	   skin_initial_squash,			// this is a table of the initial squash values

		// these are a bunch of variables to control the initial position of envelopes
		// when you add a bone
	   skin_initial_staticenvelope,			
	   skin_initial_envelope_innerpercent,
	   skin_initial_envelope_outerpercent,
	   skin_initial_envelope_inner,
	   skin_initial_envelope_outer,
//5.1.03
		skin_paintblendmode


};

//Skin Param Block Advance Parameters
enum{
	skin_advance_always_deform,		// this is tied to skin_always_deform, they are linked
	skin_advance_ref_frame,			// this is tied to skin_always_deform, they are linked

	skin_advance_rigid_handles,		// this is the global rigid handle toggle, a rigid handle
									// uses the same weight info as the knot that owns it
									// this only applies to patch objects
	skin_advance_rigid_verts,		// this toggles the global rigid property, when this is checked
									// only one bone influences a vertex

	skin_advance_fast_update,		// this forces a faster but less accurate algorythm to display the 
									// deformation, basically it uses a rigid display and turns off all gizmos
	skin_advance_no_update,			// this turns off/on the skin deformer
	skin_advance_updateonmouseup,	// this forces updates only on mouse up when interacting with the envelopes
	skin_advance_bonelimit,			// this is the maximum number of bones that can influence a vertex
	skin_advance_backtransform,		// this turns on the back transform which removes the double transform when the
									// skin mesh is linked to the skeleton

	skin_advance_shortennames,		// this shortens the bone names in the bone list box in the UI

	skin_advance_fastsubanims,		//debuggin tool turns on/off the optimization for fast sub anim look ups
	skin_advance_fasttmcache,		//debuggin tool turns on/off the optimization for the modified tm caching
	skin_advance_fastvertexweighting,   //debuggin tool turns on/off the optimization for the modified vertex weighting
	skin_advance_fastgizmo,			//debuggin tool turns on/off the optimization for the gizmos

//5.1.03
	skin_advance_ignorebonescale	//this toggles whether the bone scaling tm will be used in the deformation
	};

//Skin Param Block Display parameters
enum{
	
	skin_display_draw_all_envelopes,	// tied to skin_draw_all_envelopes
	skin_display_draw_vertices,			// tied to skin_draw_vertices
	skin_display_all_gizmos,			// turns on/off gizmo displays
	skin_display_all_vertices,			// this is for patches and when on draws all the handles and handle connections

	skin_display_shadeweights,			// this turns on/off the shaded vertex weights
	skin_display_envelopesalwaysontop,  // this turns on/off whether the envelopes on drawn on top or not
	skin_display_crosssectionsalwaysontop,	// this turns on/off whether the cross sections on drawn on top or not

	skin_display_shownoenvelopes,		//this turns off all envelope display
	};

//Skin Gizmo Param Block Data
enum{
	
	skin_gizmos_list				// this is a ref targ list of all the gizmos
	};

//Skin Weight Param Block Data
enum{
	
	skin_wt_affectselected,			// this toggles whether individual rows or the whole selection is changed when a cell is changed

	skin_wt_showaffectbones,		// this only showes bones thta affact the current vertices
	skin_wt_updateonmouseup,		// this updates skin only on mouse up
	skin_wt_flipui,					// this toggles whether the bones go across or up and down
	skin_wt_showattrib,				// this toggles whether the attribute column will display
	skin_wt_showglobal,				// this showes the global edit row
	skin_wt_shortenlabel,			// this shortens the vertex name list
	skin_wt_showexclusion,			// this toggles the exclusion check boxes
	skin_wt_showlock,				// not used yet



	skin_wt_precision,				// this the precision of the drag 
	skin_wt_fontsize,				// this is the font sice

	skin_wt_xpos,skin_wt_ypos,skin_wt_width,skin_wt_height,  // this is where the window position is stored
	
	skin_wt_showoptionui,			// this showes the options UI
	skin_wt_showsetui,				// this showes the sets UI
	skin_wt_showcopypasteui,		// this showes the copy/paste UI
	skin_wt_showmenu,				// this showes the menu bar
	skin_wt_tabley,					// this is an offset to move the whole table down in the UI

	skin_wt_dragleftright,			// this determines whether you drag left/right or up/down to adjust a cell

	skin_wt_currentvertexset,		// this is the current active vertex set the first 2 are static
									// 0 - is the all vertex set 
									// 1 - is the selected vertex set
									// 2 - is all the vertices affected by the current bone
									// 3+ are custom vertex sets

	// just the 3d display options of the tagged vertices in the viewport
	skin_wt_showmarker,
	skin_wt_markertype,
	skin_wt_markercolor,


	skin_wt_jbuimethod,				// this similiar to affect selected, if a rows are selected it affects all rows
									// other wise it only affects the one cell

	skin_wt_debugmode,				// is debug mode when on listener will be spammed with debug info

	skin_wt_attriblabelheight,		// this is the height of the attribute lables

	//5.1.01 adds left/right justification
	skin_wt_rightjustify			// this is whether the bone names are right or left justified

	};



//Enum for the Joint Gizmo
enum { skin_gizmoparam};

//Joint Gizmo Param Block Data
enum { skin_gizmoparam_name,


	   skin_gizmoparam_joint_lower_compression_inner,   //no longer used
	   skin_gizmoparam_joint_lower_compression_outer,   //no longer used
	   skin_gizmoparam_joint_upper_compression_inner,   //no longer used
	   skin_gizmoparam_joint_upper_compression_outer,
	   skin_gizmoparam_joint_bias_inner,   //no longer used
	   skin_gizmoparam_joint_bias_outer,   //no longer used
	   skin_gizmoparam_joint_bulge_inner,   //no longer used
	   skin_gizmoparam_joint_bulge_outer,   //no longer used

	   skin_gizmoparam_joint_twist,

	   skin_gizmoparam_joint_parent_id,
	   skin_gizmoparam_joint_child_id,
	   skin_gizmoparam_joint_graph,   //no longer used

   	   skin_gizmoparam_joint_points,
 	   skin_gizmoparam_joint_deformed_points, //no longer used all this data is now stored in deformedPoints for speed reasons
   	   skin_gizmoparam_joint_weights,
   	   skin_gizmoparam_joint_initial_angle,
   	   skin_gizmoparam_joint_use_graph,   //no longer used
	   skin_gizmoparam_joint_use_volume,
	   skin_gizmoparam_joint_enable,


	   skin_gizmoparam_joint_orientation,

	   skin_gizmoparam_joint_deformed_points_offsets,     //no longer used


	   skin_gizmoparam_joint_deformed_points_realoffsets,    //no longer used
	                                                      

	   skin_gizmoparam_joint_selection,
	   skin_gizmoparam_joint_editing,
	   skin_gizmoparam_joint_keygraph,

	};

 
#endif