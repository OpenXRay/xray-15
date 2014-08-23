/**********************************************************************
 *<
	FILE:			PFOperatorDisplay_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Display Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORDISPLAY_PARAMBLOCK_H_
#define  _PFOPERATORDISPLAY_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorDisplayDesc.h"

namespace PFActions {

// block IDs
enum { kDisplay_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// param IDs
enum {	kDisplay_type, 
		kDisplay_visible,
		kDisplay_color,
		kDisplay_showNumbering,
		kDisplay_selectedType,
};

enum {	kDisplay_type_none,
		kDisplay_type_dots,
		kDisplay_type_ticks,
		kDisplay_type_circles,
		kDisplay_type_lines,
		kDisplay_type_boundingBoxes,
		kDisplay_type_geometry,
		kDisplay_type_diamonds,
		kDisplay_type_boxes,
		kDisplay_type_asterisks,
		kDisplay_type_triangles,
		kDisplay_type_Xs,			// not in UI, for empty geometry or boundingBoxes.
		kDisplay_type_num=11 };

class PFOperatorDisplayDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};


extern PFOperatorDisplayDesc ThePFOperatorDisplayDesc;
extern ParamBlockDesc2 display_paramBlockDesc;
extern FPInterfaceDesc display_action_FPInterfaceDesc;
extern FPInterfaceDesc display_operator_FPInterfaceDesc;
extern FPInterfaceDesc display_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORDISPLAY_PARAMBLOCK_H_