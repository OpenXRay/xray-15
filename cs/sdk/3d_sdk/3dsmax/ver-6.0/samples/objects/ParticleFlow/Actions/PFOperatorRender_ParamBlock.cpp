/**********************************************************************
 *<
	FILE:			PFOperatorRender_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Render Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorRender_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorRenderDesc		ThePFOperatorRenderDesc;
// Dialog Proc
PFOperatorRenderDlgProc	ThePFOperatorRenderDlgProc;


// Render Operator param block
ParamBlockDesc2 render_paramBlockDesc 
(
	// required block spec
		kRender_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorRenderDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kRender_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_RENDER_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorRenderDlgProc,
	// required param specs
		// render type
			kRender_type,_T("Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TYPE,
			// optional tagged param specs
					p_default,		kRender_type_geometry,
					p_ui,			TYPE_INTLISTBOX,	IDC_DISPLAYRENDER,	kRender_type_num,
												IDS_DISPLAYTYPE_NONE, IDS_DISPLAYTYPE_BBOXES, 
												IDS_DISPLAYTYPE_GEOM, IDS_DISPLAYTYPE_PHANTOM,
					p_vals,			kRender_type_none,
									kRender_type_boundingBoxes,
									kRender_type_geometry,
									kRender_type_phantom,
			end,
		// visible % at render time
			kRender_visible, _T("Visible"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_VISIBLE,
			// optional tagged param specs
					p_default,		1.0f,
					p_ms_default,	1.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_VISIBLERENDER, IDC_VISIBLERENDERSPIN, 1.0f,
			end,
		// split type
			kRender_splitType,	_T("Split_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SPLITTYPE,
			// optional tagged param specs
					p_default,		kRender_splitType_single,
					p_range,		0, kRender_splitType_num-1,
					p_ui,			TYPE_RADIO, kRender_splitType_num, IDC_SINGLE, IDC_MULTIPLE, IDC_PARTICLE,
			end,
		// mesh count
			kRender_meshCount, _T("Mesh_Count"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MESHCOUNT,
			// optional tagged param specs
					p_default,		10000,
					p_range,		1, 10000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_MESHCOUNT,	IDC_MESHCOUNTSPIN,	SPIN_AUTOSCALE,
			end,
		// particles per mesh
			kRender_perMeshCount, _T("Particles_Per_Mesh"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_PARTICLESPERMESH,
			// optional tagged param specs
					p_default,		1000,
					p_range,		1, 999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_PERMESHCOUNT,	IDC_PERMESHCOUNTSPIN,	SPIN_AUTOSCALE,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc render_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorRenderDesc, FP_MIXIN, 
				
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

FPInterfaceDesc render_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorRenderDesc, FP_MIXIN,

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

FPInterfaceDesc render_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorRenderDesc, FP_MIXIN,

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

BOOL PFOperatorRenderDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kRender_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case kRender_message_type:
			pblock = map->GetParamBlock();
			if (pblock != NULL)
				UpdateTypeDlg( hWnd, pblock->GetInt(kRender_type, t),
										pblock->GetInt(kRender_splitType, t));
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorRenderDlgProc::UpdateTypeDlg( HWND hWnd, int type, int splitType)
{
	ISpinnerControl *spin;
	bool enable = (type != kRender_type_none) && (type != kRender_type_phantom);
	
	EnableWindow( GetDlgItem( hWnd, IDC_VISIBLE), enable);
	spin = GetISpinner(GetDlgItem(hWnd, IDC_VISIBLERENDERSPIN));
	spin->Enable( enable );
	ReleaseISpinner( spin );
	
	EnableWindow( GetDlgItem( hWnd, IDC_RENDERFRAME), enable);
	EnableWindow( GetDlgItem( hWnd, IDC_SINGLE), enable);
	EnableWindow( GetDlgItem( hWnd, IDC_MULTIPLE), enable);
	EnableWindow( GetDlgItem( hWnd, IDC_PARTICLE), enable);

	bool enableMultiple = (enable && (splitType == kRender_splitType_multiple));
	EnableWindow( GetDlgItem( hWnd, IDC_MESHCOUNTTEXT), enableMultiple);
	EnableWindow( GetDlgItem( hWnd, IDC_PERMESHCOUNTTEXT), enableMultiple);
	spin = GetISpinner(GetDlgItem(hWnd, IDC_MESHCOUNTSPIN));
	spin->Enable( enableMultiple );
	ReleaseISpinner( spin );
	spin = GetISpinner(GetDlgItem(hWnd, IDC_PERMESHCOUNTSPIN));
	spin->Enable( enableMultiple );
	ReleaseISpinner( spin );
}


} // end of namespace PFActions