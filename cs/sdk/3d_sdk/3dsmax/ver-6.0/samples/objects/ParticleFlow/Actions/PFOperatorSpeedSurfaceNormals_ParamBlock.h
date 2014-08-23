/**********************************************************************
 *<
	FILE:			PFOperatorSpeedSurfaceNormals_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SpeedSurfaceNormals Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSPEEDSURFACENORMALS_PARAMBLOCK_H_
#define  _PFOPERATORSPEEDSURFACENORMALS_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSpeedSurfaceNormalsDesc.h"

namespace PFActions {

// block IDs
enum { kSpeedSurfaceNormals_mainPBlockIndex };


// param IDs
enum {	kSpeedBySurface_type, // instant or continuous operator
		kSpeedBySurface_setSpeed, // set speed magnitude
		kSpeedBySurface_speed, // speed value
		kSpeedBySurface_speedVariation, // range of variation for speed value
		kSpeedBySurface_objects, // list of geometry objects to get surface from
		kSpeedBySurface_animated, // indicates if the geometry is surface animated
		kSpeedBySurface_subframe, // use subframe sampling for precision
		kSpeedBySurface_speedByMtl, // use material values to set speed
		kSpeedBySurface_materialType, // defines what property of material to use for speed settings
		kSpeedBySurface_useSubMtl, // use sub-material for setting
		kSpeedBySurface_mtlID, // sub-material ID
		kSpeedBySurface_directionType, // type of direction to set for speed
		kSpeedBySurface_divergence, // range of variation for speed direction
		kSpeedBySurface_accelLimit, // acceleration maximum acceptable
		kSpeedBySurface_unlimitedRange, // unlimited range of influence
		kSpeedBySurface_range, // range value
		kSpeedBySurface_falloff, // influence value (%) on the boundary of the range
		kSpeedBySurface_syncBy, // sync for animated parameters
		kSpeedBySurface_seed, // random seed
		kSpeedBySurface_objectsMaxscript // objects list for maxscript manipulation

};

// instant or continuous
enum {	kSpeedBySurface_type_once, // speed is set only once when a particle enters the event
		kSpeedBySurface_type_continuous, // speed is under continuous control
		kSpeedBySurface_type_num = 2
};

// types of material usage
enum {	kSpeedBySurface_materialType_lumCenter, // use grayscale value as a speed value multiplier, with black as -1.0, and white as 1.0, and gray as 0.0
		kSpeedBySurface_materialType_worldRGB, // use RGB value as a world XYZ multiplier for speed value
		kSpeedBySurface_materialType_localRGB, // use RGB value as a local XYZ multiplier for speed value in the object coordinates
		kSpeedBySurface_materialType_grayscale, // use grayscale value as a speed value multiplier
		kSpeedBySurface_materialType_num = 4
};

// direction types
enum {	kSpeedBySurface_directionType_normals, // as surface normals
		kSpeedBySurface_directionType_outOfSurface, // out of the surface (bi-directional depending on the position: above or below the surface)
		kSpeedBySurface_directionType_parallel, // parallel to the surface
		kSpeedBySurface_directionType_num = 3
};

// sync type
enum {	kSpeedBySurface_syncBy_time,
		kSpeedBySurface_syncBy_age,
		kSpeedBySurface_syncBy_event,
		kSpeedBySurface_syncBy_num=3 };

// user messages
enum {	kSpeedSurfaceNormals_RefMsg_NewRand = REFMSG_USER + 13279,
		kSpeedSurfaceNormals_RefMsg_InitDlg,
		kSpeedSurfaceNormals_RefMsg_ListSelect, // add more nodes by select dialog
		kSpeedSurfaceNormals_RefMsg_ResetValidatorAction
};

// dialog messages
enum {	kSpeedSurfaceNormals_message_update = 100	// settings change
};

class PFOperatorSpeedSurfaceNormalsDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	void UpdateDlg( HWND hWnd, IParamBlock2* pblock );
};

extern PFOperatorSpeedSurfaceNormalsDesc ThePFOperatorSpeedSurfaceNormalsDesc;
extern ParamBlockDesc2 speedSurfaceNormals_paramBlockDesc;
extern FPInterfaceDesc speedSurfaceNormals_action_FPInterfaceDesc;
extern FPInterfaceDesc speedSurfaceNormals_operator_FPInterfaceDesc;
extern FPInterfaceDesc speedSurfaceNormals_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSPEEDSURFACENORMALS_PARAMBLOCK_H_