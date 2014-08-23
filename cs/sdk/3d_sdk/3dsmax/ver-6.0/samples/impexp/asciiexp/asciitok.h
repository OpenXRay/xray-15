//************************************************************************** 
//* Asciitok.h	- Ascii File Exporter
//* 
//* By Christer Janson
//* Kinetix Development
//*
//* January 20, 1997 CCJ Initial coding
//*
//* File format tokens
//*
//* Copyright (c) 1997, All Rights Reserved. 
//***************************************************************************

#ifndef __ASCIITOK__H
#define __ASCIITOK__H

/**********************************************************************
 This is a list of all output tokens

 Note to translators:
 The strings in this module defines the file format and should not be
 translated.
***********************************************************************/

// Top level category ID's
#define ID_SCENE				_T("*SCENE")
#define ID_GEOMETRY				_T("*GEOMOBJECT") 
#define ID_SHAPE				_T("*SHAPEOBJECT") 
#define ID_CAMERA				_T("*CAMERAOBJECT") 
#define ID_LIGHT				_T("*LIGHTOBJECT") 
#define ID_HELPER				_T("*HELPEROBJECT")
#define ID_MATERIAL_LIST		_T("*MATERIAL_LIST")

// Hierarchy
#define ID_GROUP				_T("*GROUP")

// Node related ID's
#define ID_NODE_TM				_T("*NODE_TM")
#define ID_NODE_NAME			_T("*NODE_NAME") 
#define ID_NODE_PARENT			_T("*NODE_PARENT")

// Object (node) properties
#define ID_PROP_MOTIONBLUR		_T("*PROP_MOTIONBLUR")
#define ID_PROP_CASTSHADOW		_T("*PROP_CASTSHADOW")
#define ID_PROP_RECVSHADOW		_T("*PROP_RECVSHADOW")

// Mesh related ID's
#define ID_MESH					_T("*MESH")
#define ID_MESH_NORMALS			_T("*MESH_NORMALS") 
#define ID_MESH_NUMVERTEX		_T("*MESH_NUMVERTEX") 
#define ID_MESH_NUMFACES		_T("*MESH_NUMFACES") 
#define ID_MESH_VERTEX_LIST		_T("*MESH_VERTEX_LIST") 
#define ID_MESH_VERTEX			_T("*MESH_VERTEX")
#define ID_MESH_FACE_LIST		_T("*MESH_FACE_LIST") 
#define ID_MESH_FACE			_T("*MESH_FACE")
#define ID_MESH_SMOOTHING		_T("*MESH_SMOOTHING") 
#define ID_MESH_MTLID			_T("*MESH_MTLID")

#define ID_MESH_NUMTVERTEX		_T("*MESH_NUMTVERTEX") 
#define ID_MESH_NUMTVFACES		_T("*MESH_NUMTVFACES") 
#define ID_MESH_TVERTLIST		_T("*MESH_TVERTLIST") 
#define ID_MESH_TVERT			_T("*MESH_TVERT") 
#define ID_MESH_TFACELIST		_T("*MESH_TFACELIST") 
#define ID_MESH_TFACE			_T("*MESH_TFACE")

#define ID_MESH_NUMCVERTEX		_T("*MESH_NUMCVERTEX")
#define ID_MESH_NUMCVFACES		_T("*MESH_NUMCVFACES") 
#define ID_MESH_CVERTLIST		_T("*MESH_CVERTLIST")
#define ID_MESH_VERTCOL			_T("*MESH_VERTCOL")
#define ID_MESH_CFACELIST		_T("*MESH_CFACELIST")
#define ID_MESH_CFACE			_T("*MESH_CFACE")

// New for R3 - indicates a block with new map channel information
#define ID_MESH_MAPPINGCHANNEL	_T("*MESH_MAPPINGCHANNEL")
// End new

#define ID_MESH_FACEMAPLIST		_T("*MESH_FACEMAPLIST") 
#define ID_MESH_FACEMAP			_T("*MESH_FACEMAP") 
#define ID_MESH_FACEVERT		_T("*MESH_FACEMAPVERT")

#define ID_MESH_FACENORMAL		_T("*MESH_FACENORMAL") 
#define ID_MESH_VERTEXNORMAL	_T("*MESH_VERTEXNORMAL")

#define ID_MESH_ANIMATION		_T("*MESH_ANIMATION")

// Shape ID's
#define ID_SHAPE_LINECOUNT		_T("*SHAPE_LINECOUNT") 
#define ID_SHAPE_LINE			_T("*SHAPE_LINE")
#define ID_SHAPE_VERTEX_KNOT	_T("*SHAPE_VERTEX_KNOT") 
#define ID_SHAPE_VERTEX_INTERP	_T("*SHAPE_VERTEX_INTERP") 
#define ID_SHAPE_VERTEXCOUNT	_T("*SHAPE_VERTEXCOUNT") 
#define ID_SHAPE_CLOSED			_T("*SHAPE_CLOSED")

// Light ID's
#define ID_LIGHT_SETTINGS		_T("*LIGHT_SETTINGS") 
#define ID_LIGHT_TYPE			_T("*LIGHT_TYPE") 
#define ID_LIGHT_COLOR			_T("*LIGHT_COLOR") 
#define ID_LIGHT_INTENS			_T("*LIGHT_INTENS") 
#define ID_LIGHT_HOTSPOT		_T("*LIGHT_HOTSPOT") 
#define ID_LIGHT_FALLOFF		_T("*LIGHT_FALLOFF") 
#define ID_LIGHT_ATTNSTART		_T("*LIGHT_ATTNSTART") 
#define ID_LIGHT_ATTNEND		_T("*LIGHT_ATTNEND")
#define ID_LIGHT_ASPECT			_T("*LIGHT_ASPECT")
#define ID_LIGHT_SHADOWS		_T("*LIGHT_SHADOWS")
#define ID_LIGHT_USELIGHT		_T("*LIGHT_USELIGHT")
#define ID_LIGHT_SPOTSHAPE		_T("*LIGHT_SPOTSHAPE")
#define ID_LIGHT_TDIST			_T("*LIGHT_TDIST")
#define ID_LIGHT_MAPBIAS		_T("*LIGHT_MAPBIAS")
#define ID_LIGHT_MAPRANGE		_T("*LIGHT_MAPRANGE")
#define ID_LIGHT_MAPSIZE		_T("*LIGHT_MAPSIZE")
#define ID_LIGHT_RAYBIAS		_T("*LIGHT_RAYBIAS")
#define ID_LIGHT_USEGLOBAL		_T("*LIGHT_USEGLOBAL")
#define ID_LIGHT_ABSMAPBIAS		_T("*LIGHT_ABSMAPBIAS")
#define ID_LIGHT_OVERSHOOT		_T("*LIGHT_OVERSHOOT")
#define ID_LIGHT_EXCLUSIONLIST	_T("*LIGHT_EXCLUDELIST")
#define ID_LIGHT_NUMEXCLUDED	_T("*LIGHT_NUMEXCLUDED")
#define ID_LIGHT_EXCLUDED		_T("*LIGHT_EXCLUDED")
#define ID_LIGHT_EXCLINCLUDE	_T("*LIGHT_EXCLUDED_INCLUDE")
#define ID_LIGHT_EXCL_AFFECT_ILLUM	_T("*LIGHT_EXCLUDED_AFFECT_ILLUM")
#define ID_LIGHT_EXCL_AFFECT_SHAD	_T("*LIGHT_EXCLUDED_AFFECT_SHADOW")
#define ID_LIGHT_ANIMATION		_T("*LIGHT_ANIMATION")

#define ID_LIGHT_TYPE_OMNI		_T("Omni")
#define ID_LIGHT_TYPE_TARG		_T("Target")
#define ID_LIGHT_TYPE_DIR		_T("Directional")
#define ID_LIGHT_TYPE_FREE		_T("Free")

#define ID_LIGHT_SHAD_OFF		_T("Off")
#define ID_LIGHT_SHAD_MAP		_T("Mapped")
#define ID_LIGHT_SHAD_RAY		_T("Raytraced")

#define ID_LIGHT_SHAPE_CIRC		_T("Circle")
#define ID_LIGHT_SHAPE_RECT		_T("Rect")

// Camera ID's
#define ID_CAMERA_SETTINGS		_T("*CAMERA_SETTINGS")
#define ID_CAMERA_HITHER		_T("*CAMERA_HITHER") 
#define ID_CAMERA_YON			_T("*CAMERA_YON") 
#define ID_CAMERA_NEAR			_T("*CAMERA_NEAR") 
#define ID_CAMERA_FAR			_T("*CAMERA_FAR") 
#define ID_CAMERA_FOV			_T("*CAMERA_FOV") 
#define ID_CAMERA_TDIST			_T("*CAMERA_TDIST")
#define ID_CAMERA_ANIMATION		_T("*CAMERA_ANIMATION")

#define ID_CAMERA_TYPE			_T("*CAMERA_TYPE")
#define ID_CAMERATYPE_TARGET	_T("Target")
#define ID_CAMERATYPE_FREE		_T("Free")

// Helper objects
#define ID_HELPER_CLASS			_T("*HELPER_CLASS")

// Controller ID's
#define ID_CONTROL_POINT3_TCB		_T("*CONTROL_POINT3_TCB") 
#define ID_CONTROL_POINT3_BEZIER	_T("*CONTROL_POINT3_BEZIER") 
#define ID_CONTROL_COLOR_BEZIER		_T("*CONTROL_COLOR_BEZIER") 
#define ID_CONTROL_POINT3_SAMPLE	_T("*CONTROL_POINT3_SAMPLE")

#define ID_CONTROL_FLOAT_TCB	_T("*CONTROL_FLOAT_TCB") 
#define ID_CONTROL_FLOAT_BEZIER	_T("*CONTROL_FLOAT_BEZIER") 
#define ID_CONTROL_FLOAT_LINEAR	_T("*CONTROL_FLOAT_LINEAR") 
#define ID_CONTROL_FLOAT_SAMPLE	_T("*CONTROL_FLOAT_SAMPLE")

// "Track" is the identification of a sampled controller
#define ID_POS_TRACK			_T("*CONTROL_POS_TRACK")
#define ID_ROT_TRACK			_T("*CONTROL_ROT_TRACK") 
#define ID_SCALE_TRACK			_T("*CONTROL_SCALE_TRACK")

// Sampled keys
#define ID_POS_SAMPLE			_T("*CONTROL_POS_SAMPLE")
#define ID_ROT_SAMPLE			_T("*CONTROL_ROT_SAMPLE") 
#define ID_SCALE_SAMPLE			_T("*CONTROL_SCALE_SAMPLE")

// Specific controller keys
#define ID_POS_KEY				_T("*CONTROL_POS_KEY")
#define ID_ROT_KEY				_T("*CONTROL_ROT_KEY") 
#define ID_SCALE_KEY			_T("*CONTROL_SCALE_KEY")
#define ID_POINT3_KEY			_T("*CONTROL_POINT3_KEY")
#define ID_FLOAT_KEY			_T("*CONTROL_FLOAT_KEY")

// TCB Keys have Tens, cont, bias, easeIn, easeOut
#define ID_TCB_POINT3_KEY		_T("*CONTROL_TCB_POINT3_KEY")
#define ID_TCB_FLOAT_KEY		_T("*CONTROL_TCB_FLOAT_KEY")
#define ID_TCB_POS_KEY			_T("*CONTROL_TCB_POS_KEY")
#define ID_TCB_ROT_KEY			_T("*CONTROL_TCB_ROT_KEY") 
#define ID_TCB_SCALE_KEY		_T("*CONTROL_TCB_SCALE_KEY")

// Bezier keys have inTan, outTan
#define ID_BEZIER_FLOAT_KEY		_T("*CONTROL_BEZIER_FLOAT_KEY")
#define ID_BEZIER_POINT3_KEY	_T("*CONTROL_BEZIER_POINT3_KEY")
#define ID_BEZIER_POS_KEY		_T("*CONTROL_BEZIER_POS_KEY")
#define ID_BEZIER_SCALE_KEY		_T("*CONTROL_BEZIER_SCALE_KEY")


#define ID_CONTROL_POS_LINEAR	_T("*CONTROL_POS_LINEAR")
#define ID_CONTROL_POS_TCB		_T("*CONTROL_POS_TCB")
#define ID_CONTROL_POS_BEZIER	_T("*CONTROL_POS_BEZIER")
#define ID_CONTROL_ROT_LINEAR	_T("*CONTROL_ROT_LINEAR")
#define ID_CONTROL_ROT_TCB		_T("*CONTROL_ROT_TCB")
#define ID_CONTROL_ROT_BEZIER	_T("*CONTROL_ROT_BEZIER")
#define ID_CONTROL_SCALE_LINEAR _T("*CONTROL_SCALE_LINEAR")
#define ID_CONTROL_SCALE_TCB	_T("*CONTROL_SCALE_TCB")
#define ID_CONTROL_SCALE_BEZIER	_T("*CONTROL_SCALE_BEZIER")


// IK Node Info
#define ID_IKTERMINATOR			_T("*IK_TERMINATOR")
#define ID_IKROT_PINNED			_T("*IK_ROT_PINNED")
#define ID_IKPOS_PINNED			_T("*IK_POS_PINNED")

// IK Joints
#define ID_IKJOINT				_T("*IK_JOINT")
#define ID_IKTYPE				_T("*IK_TYPE")
#define ID_IKDOF				_T("*IK_DOF")
#define ID_IKXACTIVE			_T("*IK_XACTIVE")
#define ID_IKYACTIVE			_T("*IK_YACTIVE")
#define ID_IKZACTIVE			_T("*IK_ZACTIVE")
#define ID_IKXLIMITED			_T("*IK_XLIMITED")
#define ID_IKYLIMITED			_T("*IK_YLIMITED")
#define ID_IKZLIMITED			_T("*IK_ZLIMITED")
#define ID_IKXEASE				_T("*IK_XEASE")
#define ID_IKYEASE				_T("*IK_YEASE")
#define ID_IKZEASE				_T("*IK_ZEASE")
#define ID_IKLIMITEXACT			_T("*IK_LIMITEXACT")
#define ID_IKJOINTINFO			_T("*IK_JOINTINFO")
#define ID_IKTYPEPOS			_T("Position")
#define ID_IKTYPEROT			_T("Rotation")

// Material / Texture related ID's
#define ID_WIRECOLOR			_T("*WIREFRAME_COLOR") 
#define ID_MATERIAL				_T("*MATERIAL") 
#define ID_MATERIAL_COUNT		_T("*MATERIAL_COUNT") 
#define ID_MATERIAL_REF			_T("*MATERIAL_REF")
#define ID_NUMSUBMTLS			_T("*NUMSUBMTLS") 
#define ID_SUBMATERIAL			_T("*SUBMATERIAL") 
#define ID_MATNAME				_T("*MATERIAL_NAME") 
#define ID_MATCLASS				_T("*MATERIAL_CLASS")

#define ID_MAT_SHADE_CONST		_T("Constant")
#define ID_MAT_SHADE_PHONG		_T("Phong")
#define ID_MAT_SHADE_METAL		_T("Metal")
#define ID_MAT_SHADE_BLINN		_T("Blinn")
#define ID_MAT_SHADE_OTHER		_T("Other")

#define ID_MAP_XPTYPE_FLT		_T("Filter")
#define ID_MAP_XPTYPE_SUB		_T("Subtractive")
#define ID_MAP_XPTYPE_ADD		_T("Additive")
#define ID_MAP_XPTYPE_OTH		_T("Other")

#define ID_BMP_FILT_PYR			_T("Pyramidal")
#define ID_BMP_FILT_SAT			_T("SAT")
#define ID_BMP_FILT_NONE		_T("None")

#define ID_FALLOFF_OUT			_T("Out")
#define ID_FALLOFF_IN			_T("In")
								
#define ID_MAPTYPE_EXP			_T("Explicit")
#define ID_MAPTYPE_SPH			_T("Spherical")
#define ID_MAPTYPE_CYL			_T("Cylindrical")
#define ID_MAPTYPE_SHR			_T("Shrinkwrap")
#define ID_MAPTYPE_SCR			_T("Screen")
								
#define ID_AMBIENT				_T("*MATERIAL_AMBIENT") 
#define ID_DIFFUSE				_T("*MATERIAL_DIFFUSE") 
#define ID_SPECULAR				_T("*MATERIAL_SPECULAR") 
#define ID_SHINE				_T("*MATERIAL_SHINE")
#define ID_SHINE_STRENGTH		_T("*MATERIAL_SHINESTRENGTH") 
#define ID_TRANSPARENCY			_T("*MATERIAL_TRANSPARENCY") 
#define ID_WIRESIZE				_T("*MATERIAL_WIRESIZE")
								
#define ID_SHADING				_T("*MATERIAL_SHADING") 
#define ID_XP_FALLOFF			_T("*MATERIAL_XP_FALLOFF") 
#define ID_SELFILLUM			_T("*MATERIAL_SELFILLUM") 
#define ID_TWOSIDED				_T("*MATERIAL_TWOSIDED") 
#define ID_WIRE					_T("*MATERIAL_WIRE")
#define ID_WIREUNITS			_T("*MATERIAL_WIREUNITS") 
#define ID_FALLOFF				_T("*MATERIAL_FALLOFF") 
#define ID_FACEMAP				_T("*MATERIAL_FACEMAP") 
#define ID_SOFTEN				_T("*MATERIAL_SOFTEN") 
#define ID_XP_TYPE				_T("*MATERIAL_XP_TYPE")
								
#define ID_TEXNAME				_T("*MAP_NAME") 
#define ID_TEXCLASS				_T("*MAP_CLASS") 
#define ID_TEXSUBNO				_T("*MAP_SUBNO") 
#define ID_TEXAMOUNT			_T("*MAP_AMOUNT")
								
#define ID_BITMAP				_T("*BITMAP")
#define ID_TEX_INVERT			_T("*BITMAP_INVERT") 
#define ID_BMP_FILTER			_T("*BITMAP_FILTER")
								
#define ID_MAPTYPE				_T("*MAP_TYPE") 
#define ID_U_OFFSET				_T("*UVW_U_OFFSET") 
#define ID_V_OFFSET				_T("*UVW_V_OFFSET") 
#define ID_U_TILING				_T("*UVW_U_TILING") 
#define ID_V_TILING				_T("*UVW_V_TILING") 
#define ID_ANGLE				_T("*UVW_ANGLE") 
#define ID_BLUR					_T("*UVW_BLUR")
#define ID_BLUR_OFFSET			_T("*UVW_BLUR_OFFSET") 
#define ID_NOISE_AMT			_T("*UVW_NOUSE_AMT") 
#define ID_NOISE_SIZE			_T("*UVW_NOISE_SIZE") 
#define ID_NOISE_LEVEL			_T("*UVW_NOISE_LEVEL") 
#define ID_NOISE_PHASE			_T("*UVW_NOISE_PHASE")
								
// Sub texture types			
#define ID_MAP_GENERIC			_T("*MAP_GENERIC") 
#define ID_MAP_AMBIENT			_T("*MAP_AMBIENT") 
#define ID_MAP_DIFFUSE			_T("*MAP_DIFFUSE") 
#define ID_MAP_SPECULAR			_T("*MAP_SPECULAR") 
#define ID_MAP_SHINE			_T("*MAP_SHINE")
#define ID_MAP_SHINESTRENGTH	_T("*MAP_SHINESTRENGTH") 
#define ID_MAP_SELFILLUM		_T("*MAP_SELFILLUM") 
#define ID_MAP_OPACITY			_T("*MAP_OPACITY")
#define ID_MAP_FILTERCOLOR		_T("*MAP_FILTERCOLOR") 
#define ID_MAP_BUMP				_T("*MAP_BUMP")
#define ID_MAP_REFLECT			_T("*MAP_REFLECT") 
#define ID_MAP_REFRACT			_T("*MAP_REFRACT")
								
// TM related ID's				
#define ID_TM_ROW0				_T("*TM_ROW0") 
#define ID_TM_ROW1				_T("*TM_ROW1") 
#define ID_TM_ROW2				_T("*TM_ROW2") 
#define ID_TM_ROW3				_T("*TM_ROW3") 
#define ID_TM_POS				_T("*TM_POS")
#define ID_TM_ROTAXIS			_T("*TM_ROTAXIS") 
#define ID_TM_ROTANGLE			_T("*TM_ROTANGLE") 
#define ID_TM_SCALE				_T("*TM_SCALE") 
#define ID_TM_SCALEAXIS			_T("*TM_SCALEAXIS")
#define ID_TM_SCALEAXISANG		_T("*TM_SCALEAXISANG") 
#define ID_TM_ANIMATION			_T("*TM_ANIMATION")

// TM Inheritance flags
#define ID_INHERIT_POS			_T("*INHERIT_POS")
#define ID_INHERIT_ROT			_T("*INHERIT_ROT")
#define ID_INHERIT_SCL			_T("*INHERIT_SCL")

// Scene related ID's			
#define ID_FILENAME				_T("*SCENE_FILENAME") 
#define ID_FIRSTFRAME			_T("*SCENE_FIRSTFRAME") 
#define ID_LASTFRAME			_T("*SCENE_LASTFRAME") 
#define ID_FRAMESPEED			_T("*SCENE_FRAMESPEED") 
#define ID_TICKSPERFRAME		_T("*SCENE_TICKSPERFRAME") 
#define ID_ENVMAP				_T("*SCENE_ENVMAP")
#define ID_STATICBGCOLOR		_T("*SCENE_BACKGROUND_STATIC") 
#define ID_ANIMBGCOLOR			_T("*SCENE_BACKGROUND_ANIM") 
#define ID_STATICAMBIENT		_T("*SCENE_AMBIENT_STATIC") 
#define ID_ANIMAMBIENT			_T("*SCENE_AMBIENT_ANIM")

#define ID_VISIBILITY_TRACK		_T("*NODE_VISIBILITY_TRACK")

// Generic ID's that can show up here and there 
#define ID_TIMEVALUE			_T("*TIMEVALUE")
#define ID_COMMENT				_T("*COMMENT")
#define ID_FILEID				_T("*3DSMAX_ASCIIEXPORT")
#define ID_BOUNDINGBOX_MIN		_T("*BOUNDINGBOX_MIN")
#define ID_BOUNDINGBOX_MAX		_T("*BOUNDINGBOX_MAX")

#endif 