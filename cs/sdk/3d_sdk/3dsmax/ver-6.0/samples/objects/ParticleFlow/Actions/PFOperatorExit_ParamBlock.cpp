/**********************************************************************
 *<
	FILE:			PFOperatorExit_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Exit Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorExit_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorExitDesc		ThePFOperatorExitDesc;
// Dialog Proc
PFOperatorExitDlgProc	ThePFOperatorExitDlgProc;

// Exit Operator param block
ParamBlockDesc2 exit_paramBlockDesc 
(
	// required block spec
		kExit_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorExitDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kExit_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_EXIT_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorExitDlgProc,
	// required param specs
		// exit type: all/selected/by age
			kExit_type,				_T("Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_EXITTYPE,
			// optional tagged param specs
					p_default,		kExit_type_all,
					p_range,		kExit_type_all, kExit_type_byAge,
					p_ui,			TYPE_RADIO, kExit_type_num, IDC_ALL, IDC_SELECTED, IDC_BYAGE,
			end,
		// life span of particles if exit by age
			kExit_lifeSpan,			_T("Life_Span"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_LIFESPAN,
			// optional tagged param specs
					p_default,		9600,
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_AGE,	IDC_AGESPIN,	80.0f,
			end,
		// variation of life span
			kExit_variation,		_T("Variation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		1600,
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_VARIATION,	IDC_VARIATIONSPIN,	80.0f,
			end,
		// random seed
			kExit_randomSeed,		_T("Random_Seed"),
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
FPInterfaceDesc exit_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorExitDesc, FP_MIXIN, 
				
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

FPInterfaceDesc exit_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorExitDesc, FP_MIXIN,

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

FPInterfaceDesc exit_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorExitDesc, FP_MIXIN,

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


BOOL PFOperatorExitDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
												WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kExit_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kExit_RefMsg_NewRand );
			break;
		case kExit_message_type:
			UpdateLifeSpanDlg( hWnd, map->GetParamBlock()->GetInt(kExit_type, t) );
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorExitDlgProc::UpdateLifeSpanDlg( HWND hWnd, int exitType )
{
	BOOL enable = (exitType == kExit_type_byAge);

	ISpinnerControl *spin;

	// enable/disable life span spinner
	EnableWindow( GetDlgItem( hWnd, IDC_LIFESPANTEXT ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_AGE ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_AGESPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
	// enable/disable variation spinner
	EnableWindow( GetDlgItem( hWnd, IDC_VARIATIONTEXT ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_VARIATION ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_VARIATIONSPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
}


} // end of namespace PFActions