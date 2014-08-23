/**********************************************************************
 *<
	FILE:			PFOperatorSimpleBirth_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleBirth Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleBirth_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class.
PFOperatorSimpleBirthDesc		ThePFOperatorSimpleBirthDesc;
// Dialog Proc
PFOperatorSimpleBirthDlgProc	ThePFOperatorSimpleBirthDlgProc;

// SampleBirth Operator param block
ParamBlockDesc2 simpleBirth_paramBlockDesc 
(
	// required block spec
		kSimpleBirth_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleBirthDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleBirth_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLEBIRTH_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleBirthDlgProc,
	// required param specs
		// start emission at frame
//			kSimpleBirth_start,		_T("Emit Start"),
//									TYPE_TIMEVALUE,
//									P_RESET_DEFAULT,
//									IDS_EMITSTART,
			// optional tagged param specs
//					p_default,		0.0f,
//					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_START,	IDC_STARTSPIN,	0.2f,

			kSimpleBirth_start,		_T("Emit_Start"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_EMITSTART,
			// optional tagged param specs
					p_default,		0,
					p_ms_default,	0,
					p_range,		-0x7FFF000, 0x7FFF000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_START,	IDC_STARTSPIN,	80.0f,
					p_validator,	NULL, // defined on the fly
			end,
		// stop emission at frame
			kSimpleBirth_finish,	_T("Emit_Stop"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_EMITSTOP,
			// optional tagged param specs
					p_default,		4800, // 1 sec
					p_ms_default,	4800, // 1 sec
					p_range,		-0x7FFF000, 0x7FFF000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_FINISH,	IDC_FINISHSPIN,	80.0f,
					p_validator,	NULL, // defined on the fly
			end,
		// generation type: amount or rate
			kSimpleBirth_type,		_T("Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_BIRTHTYPE,
			// optional tagged param specs
					p_default,		kSimpleBirth_type_amount,
					p_range,		kSimpleBirth_type_amount, kSimpleBirth_type_rate,
					p_ui,			TYPE_RADIO, 2, IDC_ISAMOUNT, IDC_ISRATE,
			end,
		// total amount of particles to emit
			kSimpleBirth_amount,	_T("Amount"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_AMOUNT,
			// optional tagged param specs
					p_default,		200,
					p_ms_default,	200,
					p_range,		0, kSimpleBirth_amountMax,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_AMOUNT,	IDC_AMOUNTSPIN,	1.0f,
			end,
		// rate of emission (particles per frame)
			kSimpleBirth_oldRate,	_T(""), //_T("<obsolete>"),  --010803  az: hide from MXS exposure
									TYPE_INT,
									P_RESET_DEFAULT,
									0,
			// optional tagged param specs
					p_default,		10,
					p_range,		0, 999999999,
//					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_RATE,	IDC_RATESPIN,	1.0f,
			end,
		// Subframe sampling
			kSimpleBirth_subframe,	_T("Subframe_Sampling"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLING,
			// optional tagged param specs
					p_default,		TRUE,
					p_ms_default,	TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SUBFRAME,
			end,
		// floatrate of emission (particles per frame)
			kSimpleBirth_rate,		_T("Rate"),
									TYPE_FLOAT,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_RATE,
			// optional tagged param specs
					p_default,		60.f,
					p_range,		0.f, 999999999.f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	IDC_RATE,	IDC_RATESPIN,	1.0f,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc simpleBirth_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleBirthDesc, FP_MIXIN, 
				
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

FPInterfaceDesc simpleBirth_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleBirthDesc, FP_MIXIN,

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

FPInterfaceDesc simpleBirth_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleBirthDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleBirthDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											 WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSimpleBirth_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case kSimpleBirth_message_type:
			UpdateBirthTypeDlg( hWnd, (int)lParam );
			break;
		case kSimpleBirth_message_amount:
			UpdateTotalAmountDlg( hWnd, (int)lParam );
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorSimpleBirthDlgProc::UpdateBirthTypeDlg( HWND hWnd, int birthType )
{
	bool birthByAmount = false, birthByRate = false;
	switch( birthType )
	{
	case kSimpleBirth_type_amount:
		birthByAmount = true;
		birthByRate = false;
		break;
	case kSimpleBirth_type_rate:
		birthByAmount = false;
		birthByRate = true;
		break;
	}

	ISpinnerControl *spin;
	EnableWindow( GetDlgItem( hWnd, IDC_AMOUNT ), birthByAmount );
	EnableWindow( GetDlgItem( hWnd, IDC_AMOUNTTEXT ), birthByAmount );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_AMOUNTSPIN ) );
	spin->Enable( birthByAmount );
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_RATE ), birthByRate );
	EnableWindow( GetDlgItem( hWnd, IDC_RATETEXT ), birthByRate );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_RATESPIN ) );
	spin->Enable( birthByRate );
	ReleaseISpinner( spin );
}

void PFOperatorSimpleBirthDlgProc::UpdateTotalAmountDlg( HWND hWnd, int amount )
{
	TSTR buf;

	buf.printf( "%d", amount );
	SetDlgItemText( hWnd, IDC_TOTAL, buf );
}


} // end of namespace PFActions