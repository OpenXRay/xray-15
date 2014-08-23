/**********************************************************************
 *<
	FILE:			PFOperatorSimplePosition_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimplePosition Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimplePosition_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimplePositionDesc		ThePFOperatorSimplePositionDesc;
// Dialog Proc
PFOperatorSimplePositionDlgProc	ThePFOperatorSimplePositionDlgProc;

// SimplePosition Operator param block
ParamBlockDesc2 simplePosition_paramBlockDesc 
(
	// required block spec
		kSimplePosition_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimplePositionDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimplePosition_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLEPOSITION_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimplePositionDlgProc,
	// required param specs
		// locked on the emitter
			kSimplePosition_lock,	_T("Lock_On_Emitter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_LOCK,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_LOCK,
			end,
		// inherit the emitter speed/movement
			kSimplePosition_inherit, _T("Inherit_Emitter_Movement"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_INHERIT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_INHERIT,
			end,
		// percentage of speed inherited
			kSimplePosition_inheritAmount,	_T("Multiplier"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_MULTIPLIER,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MULTIPLIER, IDC_MULTIPLIERSPIN, 1.0f,
			end,
		// location type
			kSimplePosition_type,	_T("Location"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_LOCATION,
			// optional tagged param specs
					p_default,		kSimplePosition_locationType_volume,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kSimplePosition_locationType_num,
												IDS_LOCATIONTYPE_PIVOT, IDS_LOCATIONTYPE_VERTEX,
												IDS_LOCATIONTYPE_EDGE, IDS_LOCATIONTYPE_SURFACE,
												IDS_LOCATIONTYPE_VOLUME,
					p_vals,			kSimplePosition_locationType_pivot,
									kSimplePosition_locationType_vertex,
									kSimplePosition_locationType_edge,
									kSimplePosition_locationType_surface,
									kSimplePosition_locationType_volume,
			end,
		// place at distinct points only
			kSimplePosition_distinctOnly, _T("Distinct_Points_Only"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_DISTINCT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_DISTINCT,
			end,
		// total number of of distinct points
			kSimplePosition_distinctTotal, _T("Total_Distinct_Points"),
									TYPE_INT,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_TOTALDISTINCT,
			// optional tagged param specs
					p_default,		10,
					p_range,		1, 10000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_TOTALDISTINCT,	IDC_TOTALDISTINCTSPIN,	1.0f,
			end,
		// subframe sampling
			kSimplePosition_subframe,	_T("Subframe_Sampling"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLING,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SUBFRAME,
			end,
		// random seed
			kSimplePosition_randomSeed,	_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345, // initial random seed
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc simplePosition_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimplePositionDesc, FP_MIXIN, 
				
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
			IPFAction::kGetRand, _T("getRand"), 0, TYPE_INT, 0, 0,
			IPFAction::kSetRand, _T("setRand"), 0, TYPE_VOID, 0, 1,
				_T("randomSeed"), 0, TYPE_INT,
			IPFAction::kNewRand, _T("newRand"), 0, TYPE_INT, 0, 0,
			IPFAction::kIsMaterialHolder, _T("isMaterialHolder"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc simplePosition_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimplePositionDesc, FP_MIXIN,

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

FPInterfaceDesc simplePosition_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimplePositionDesc, FP_MIXIN,

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

BOOL PFOperatorSimplePositionDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
												WPARAM wParam, LPARAM lParam )
{
	IParamBlock2* pblock;
	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSimplePosition_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = (map != NULL) ? map->GetParamBlock() : NULL;
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			if (pblock) pblock->NotifyDependents( FOREVER, PART_OBJ, kSimplePosition_RefMsg_NewRand );
			break;
		case kSimplePosition_message_lock:
			if (pblock) UpdateInheritSpeedDlg( hWnd, pblock->GetInt(kSimplePosition_lock, t),
													pblock->GetInt(kSimplePosition_inherit, t));
			break;
		case kSimplePosition_message_type:
			if (pblock)	
				UpdateDistinctPointsDlg( hWnd, pblock->GetInt(kSimplePosition_type, t),
												pblock->GetInt(kSimplePosition_distinctOnly, t));
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorSimplePositionDlgProc::UpdateInheritSpeedDlg( HWND hWnd, BOOL lock, BOOL inherit )
{
	EnableWindow( GetDlgItem( hWnd, IDC_INHERIT ), !lock );
	bool useInherit = ((lock == 0) && (inherit != 0));
	EnableWindow( GetDlgItem( hWnd, IDC_MULTIPLIERTEXT ), useInherit);
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_MULTIPLIERSPIN ) );
	spin->Enable(useInherit);
	ReleaseISpinner( spin );
}

void PFOperatorSimplePositionDlgProc::UpdateDistinctPointsDlg( HWND hWnd, int locationType, int useDistinct )
{
	BOOL enable = (locationType != kSimplePosition_locationType_pivot);

	EnableWindow( GetDlgItem( hWnd, IDC_DISTINCT ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_TOTALTEXT ), enable && (useDistinct != 0));
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_TOTALDISTINCTSPIN ) );
	spin->Enable( enable && (useDistinct != 0));
	ReleaseISpinner( spin );
}


} // end of namespace PFActions