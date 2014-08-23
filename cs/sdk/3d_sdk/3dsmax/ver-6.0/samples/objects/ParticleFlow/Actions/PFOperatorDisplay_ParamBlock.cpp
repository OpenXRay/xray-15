/**********************************************************************
 *<
	FILE:			PFOperatorDisplay_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Display Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorDisplay_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorDisplayDesc		ThePFOperatorDisplayDesc;
// Dialog Proc
PFOperatorDisplayDlgProc	ThePFOperatorDisplayDlgProc;


// Display Operator param block
ParamBlockDesc2 display_paramBlockDesc 
(
	// required block spec
		kDisplay_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorDisplayDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kDisplay_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_DISPLAY_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorDisplayDlgProc,
	// required param specs
		// viewport type
			kDisplay_type,_T("Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TYPE,
			// optional tagged param specs
					p_default,		kDisplay_type_ticks,
					p_ui,			TYPE_INTLISTBOX,	IDC_DISPLAYVIEWPORT,	kDisplay_type_num,
												IDS_DISPLAYTYPE_NONE, IDS_DISPLAYTYPE_DOTS,
												IDS_DISPLAYTYPE_TICKS, IDS_DISPLAYTYPE_CIRCLES, IDS_DISPLAYTYPE_LINES,
												IDS_DISPLAYTYPE_BBOXES, IDS_DISPLAYTYPE_GEOM,
												IDS_DISPLAYTYPE_DIAMONDS, IDS_DISPLAYTYPE_BOXES,
												IDS_DISPLAYTYPE_ASTERISKS, IDS_DISPLAYTYPE_TRIANGLES,
					p_vals,			kDisplay_type_none,
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
			end,
		// visible % in viewport
			kDisplay_visible, _T("Visible"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_VISIBLE,
			// optional tagged param specs
					p_default,		1.0f,
					p_ms_default,	1.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_VISIBLEVIEWPORT, IDC_VISIBLEVIEWPORTSPIN, 1.0f,
			end,
		// viewport color
			kDisplay_color, _T("Color"),
									TYPE_RGBA,
									P_RESET_DEFAULT,
									IDS_COLOR,
			// optional tagget param specs
					p_default,		Color(0, 0, 1),
					p_ui,			TYPE_COLORSWATCH, IDC_VIEWPORTCOLOR,
			end,
		// show numbering
			kDisplay_showNumbering, _T("Show_Numbering"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWNUMBERING,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_NUMBER,
			end,
		// selected viewport type
			kDisplay_selectedType,_T("Selected_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SELECTEDTYPE,
			// optional tagged param specs
					p_default,		kDisplay_type_ticks,
					p_ui,			TYPE_INTLISTBOX,	IDC_SELECTEDVIEWPORT,	kDisplay_type_num,
												IDS_DISPLAYTYPE_NONE, IDS_DISPLAYTYPE_DOTS,
												IDS_DISPLAYTYPE_TICKS, IDS_DISPLAYTYPE_CIRCLES, IDS_DISPLAYTYPE_LINES,
												IDS_DISPLAYTYPE_BBOXES, IDS_DISPLAYTYPE_GEOM, // last two options aren't working yet (Bayboro 11-07-01)
												IDS_DISPLAYTYPE_DIAMONDS, IDS_DISPLAYTYPE_BOXES,
												IDS_DISPLAYTYPE_ASTERISKS, IDS_DISPLAYTYPE_TRIANGLES,
					p_vals,			kDisplay_type_none,
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
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc display_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorDisplayDesc, FP_MIXIN, 
				
			IPFAction::kInit,	_T("init"),		0,	TYPE_bool, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actions"), 0, TYPE_OBJECT_TAB_BR,
				_T("actionNodes"), 0, TYPE_INODE_TAB_BR, 
			IPFAction::kRelease, _T("release"), 0, TYPE_bool, 0, 1,
				_T("container"), 0, TYPE_IOBJECT,
			// reserved for future use
			//IPFAction::kChannelsUsed, _T("channelsUsed"), 0, TYPE_VOID, 0, 2,
			//	_T("interval"), 0, TYPE_INTERVAL_BR,
			//	_T("channels"), 0, TYPE_FPVALUE,
			IPFAction::kActivityInterval, _T("activityInterval"), 0, TYPE_INTERVAL_BV, 0, 0,
			IPFAction::kIsFertile, _T("isFertile"), 0, TYPE_bool, 0, 0,
			IPFAction::kIsNonExecutable, _T("isNonExecutable"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportRand, _T("supportRand"), 0, TYPE_bool, 0, 0,
			IPFAction::kIsMaterialHolder, _T("isMaterialHolder"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc display_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorDisplayDesc, FP_MIXIN,

			IPFOperator::kProceed, _T("proceed"), 0, TYPE_bool, 0, 7,
				_T("container"), 0, TYPE_IOBJECT,
				_T("timeStart"), 0, TYPE_TIMEVALUE,
				_T("timeEnd"), 0, TYPE_TIMEVALUE_BR,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,

		end
);

FPInterfaceDesc display_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorDisplayDesc, FP_MIXIN,

			IPViewItem::kNumPViewParamBlocks, _T("numPViewParamBlocks"), 0, TYPE_INT, 0, 0,
			IPViewItem::kGetPViewParamBlock, _T("getPViewParamBlock"), 0, TYPE_REFTARG, 0, 1,
				_T("index"), 0, TYPE_INDEX,
			IPViewItem::kHasComments, _T("hasComments"), 0, TYPE_bool, 0, 1,
				_T("actionNode"), 0, TYPE_INODE,
			IPViewItem::kGetComments, _T("getComments"), 0, TYPE_STRING, 0, 1,
				_T("actionNode"), 0, TYPE_INODE,
			IPViewItem::kSetComments, _T("setComments"), 0, TYPE_VOID, 0, 2,
				_T("actionNode"), 0, TYPE_INODE,
				_T("comments"), 0, TYPE_STRING,

		end
);


//+--------------------------------------------------------------------------+
//|							Dialog Process									 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorDisplayDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam)
{
//	ICustButton *iBut;

	switch ( msg )
	{
	case WM_INITDIALOG:
		;
		break;
	case WM_DESTROY:
		;
		break;
	}
	return FALSE;
}


} // end of namespace PFActions