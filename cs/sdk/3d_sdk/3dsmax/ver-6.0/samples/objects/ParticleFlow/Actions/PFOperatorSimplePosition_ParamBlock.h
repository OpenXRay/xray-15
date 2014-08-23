/**********************************************************************
 *<
	FILE:			PFOperatorSimplePosition_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimplePosition Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLEPOSITION_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLEPOSITION_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimplePositionDesc.h"

namespace PFActions {

// block IDs
enum { kSimplePosition_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// param IDs
enum {	kSimplePosition_lock, // particles are constantly locked on the emitter
		kSimplePosition_inherit, // particles inherit the emitter speed/movement
		kSimplePosition_inheritAmount, // percentable of the speed inherited
		kSimplePosition_type, // type of particle location (see below)
		kSimplePosition_distinctOnly, // place particles at distinct points only
		kSimplePosition_distinctTotal, // total number of of distinct points
		kSimplePosition_randomSeed, // random seed
		kSimplePosition_subframe, // make subframe sampling for more precise placement on moving emitters
};

enum {	kSimplePosition_locationType_pivot, // At Emitter Pivot
		kSimplePosition_locationType_vertex, // At Emitter Vertices
		kSimplePosition_locationType_edge, // At Emitter Edges
		kSimplePosition_locationType_surface, // Over Entire Surface
		kSimplePosition_locationType_volume, // Over Entire Volume
		kSimplePosition_locationType_num=5 };


// User Defined Reference Messages from PB
enum {	kSimplePosition_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSimplePosition_RefMsg_NewRand
};

// dialog messages
enum {	kSimplePosition_message_lock = 100,	// lock on emitter change message
		kSimplePosition_message_type		// location type change message
};

class PFOperatorSimplePositionDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	void UpdateInheritSpeedDlg( HWND hWnd, BOOL lock, BOOL inherit);
	void UpdateDistinctPointsDlg( HWND hWnd, int locationType, int useDistinct );
};


extern PFOperatorSimplePositionDesc ThePFOperatorSimplePositionDesc;
extern ParamBlockDesc2 simplePosition_paramBlockDesc;
extern FPInterfaceDesc simplePosition_action_FPInterfaceDesc;
extern FPInterfaceDesc simplePosition_operator_FPInterfaceDesc;
extern FPInterfaceDesc simplePosition_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEPOSITION_PARAMBLOCK_H_