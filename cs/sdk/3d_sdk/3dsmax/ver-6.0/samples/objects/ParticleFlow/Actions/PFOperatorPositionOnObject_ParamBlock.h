/**********************************************************************
 *<
	FILE:			PFOperatorPositionOnObject_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for PositionOnObject Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORPOSITIONONOBJECT_PARAMBLOCK_H_
#define  _PFOPERATORPOSITIONONOBJECT_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorPositionOnObjectDesc.h"

namespace PFActions {

// block IDs
enum { kPositionOnObject_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// param IDs
enum {	kPositionOnObject_lock, // particles are constantly locked on the emitter
		kPositionOnObject_inherit, // particles inherit the emitter speed/movement
		kPositionOnObject_inheritAmount, // percentable of the speed inherited
		kPositionOnObject_inheritAmountVar, // variation of percentable of the speed inherited
		kPositionOnObject_emitters, // list of emitter objects
		kPositionOnObject_animated, // indicates if an emitter has an animated shape
		kPositionOnObject_subframe, // make subframe sampling for more precise placement on moving emitters
		kPositionOnObject_type, // type of particle location (see below)
		kPositionOnObject_useOffset, // offset location out of surface
		kPositionOnObject_offsetMin, // minimum distance for offset
		kPositionOnObject_offsetMax, // maximum distance for offset
		kPositionOnObject_useNonUniform, // make non-uniform density by emitter material
		kPositionOnObject_densityType, // component of material to use for density evaluation
		kPositionOnObject_useSubMtl, // use sub-material for non-uniform density
		kPositionOnObject_mtlID, // sub-material ID
		kPositionOnObject_apartPlacement, // place particles apart from each other
		kPositionOnObject_distance, // desirable distance between particles (at least)
		kPositionOnObject_distinctOnly, // place particles in distinct points only
		kPositionOnObject_distinctTotal, // total number of of distinct points
		kPositionOnObject_randomSeed, // random seed
		kPositionOnObject_attempts, // maximum number of attempts to generate a location
		kPositionOnObject_delete, // delete particles with invalid placement
		kPositionOnObject_emittersMaxscript // emitters list for maxscript manipulations
};

enum {	kPositionOnObject_locationType_pivot, // At Emitter Pivot
		kPositionOnObject_locationType_vertex, // At Emitter Vertices
		kPositionOnObject_locationType_edge, // At Emitter Edges
		kPositionOnObject_locationType_surface, // Over Entire Surface
		kPositionOnObject_locationType_volume, // Over Entire Volume
		kPositionOnObject_locationType_selVertex, // At Selected Vertices Only
		kPositionOnObject_locationType_selEdge, // At Selected Edges Only
		kPositionOnObject_locationType_selFaces, // At Selected Faces Only
		kPositionOnObject_locationType_num=8 };

enum {	kPositionOnObject_densityType_grayscale,
		kPositionOnObject_densityType_opacity,
		kPositionOnObject_densityType_grayscaleOpacity,
		kPositionOnObject_densityType_red,
		kPositionOnObject_densityType_green,
		kPositionOnObject_densityType_blue,
		kPositionOnObject_densityType_num=6 };

// User Defined Reference Messages from PB
enum {	kPositionOnObject_RefMsg_InitDlg = REFMSG_USER + 13279,
		kPositionOnObject_RefMsg_NewRand,
		kPositionOnObject_RefMsg_ListSelect, // add more nodes by select dialog
		kPositionOnObject_RefMsg_ResetValidatorAction
};

// dialog messages
enum {	kPositionOnObject_message_lock = 100,	// lock on emitter change message
		kPositionOnObject_message_type,		// location type change message
		kPositionOnObject_message_objects	// change in number of objects
};

class PFOperatorPositionOnObjectDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	void UpdateInheritSpeedDlg( HWND hWnd, BOOL lock, BOOL inherit );
	void UpdateTypeDlg( HWND hWnd, IParamBlock2* pblock);
	void UpdateObjectsNumDlg( HWND hWnd, int num );
};


extern PFOperatorPositionOnObjectDesc ThePFOperatorPositionOnObjectDesc;
extern ParamBlockDesc2 positionOnObject_paramBlockDesc;
extern FPInterfaceDesc positionOnObject_action_FPInterfaceDesc;
extern FPInterfaceDesc positionOnObject_operator_FPInterfaceDesc;
extern FPInterfaceDesc positionOnObject_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORPOSITIONONOBJECT_PARAMBLOCK_H_