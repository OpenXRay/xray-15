/**********************************************************************
 *<
	FILE:			PFOperatorSpeedCopy_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SpeedCopy Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSpeedCopy_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSpeedCopyDesc ThePFOperatorSpeedCopyDesc;

// Dialog Proc
PFOperatorSpeedCopyDlgProc ThePFOperatorSpeedCopyDlgProc;

// SpeedCopy Operator param block
ParamBlockDesc2 speedCopy_paramBlockDesc 
(
	// required block spec
		kSpeedCopy_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSpeedCopyDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpeedCopy_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPEEDCOPY_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSpeedCopyDlgProc,
	// required param specs
		// accel limit value
			kSpeedCopy_accelLimit, _T("Accel_Limit"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_ACCELLIMIT,
			// optional tagged param specs
					p_default,		100.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ACCELLIMIT, IDC_ACCELLIMITSPIN, SPIN_AUTOSCALE,
			end,
		// influence
			kSpeedCopy_influence,	_T("Influence"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_INFLUENCE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_INFLUENCE, IDC_INFLUENCESPIN, 1.0f,
			end,
		// use speed variation
			kSpeedCopy_useSpeedVariation,	_T("Use_Speed_Variation"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESPEEDVARIATION,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SPEEDVARIATION,
			end,
		// speed minimum
			kSpeedCopy_speedMin,	_T("Speed_Minimum"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SPEEDMIN,
			// optional tagged param specs
					p_default,		0.5f,
					p_range,		0.0f, 1000000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_MINSPEED, IDC_MINSPEEDSPIN, 1.0f,
			end,
		// speed maximum
			kSpeedCopy_speedMax,	_T("Speed_Maximum"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SPEEDMAX,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 1000000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_MAXSPEED, IDC_MAXSPEEDSPIN, 1.0f,
			end,
		// use icon orientation
			kSpeedCopy_useOrient,	_T("Use_Icon_Orientation"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USEORIENT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USEORIENT,
			end,
		// steer towards trajectory
			kSpeedCopy_steer,	_T("Steer_Towards_Trajectory"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_STEERTOWARDSTRAJECTORY,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_STEER,
			end,
		// distance towards trajectory
			kSpeedCopy_distance, _T("Distance"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_DISTANCE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DISTANCE, IDC_DISTANCESPIN, SPIN_AUTOSCALE,
			end,
		// sync by
			kSpeedCopy_syncByParams,_T("Sync_Type_Param_Animation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpeedCopy_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_PARAMSYNC,	kSpeedCopy_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, 
												IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedCopy_syncBy_time,
									kSpeedCopy_syncBy_age,
									kSpeedCopy_syncBy_event,
			end,
		// icon sync by
			kSpeedCopy_syncByIcon,	_T("Sync_Type_Icon_Animation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpeedCopy_syncBy_event,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSpeedCopy_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, 
												IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedCopy_syncBy_time,
									kSpeedCopy_syncBy_age,
									kSpeedCopy_syncBy_event,
			end,
		// icon size
			kSpeedCopy_iconSize, _T("Icon_Size"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_ICONSIZE,
			// optional tagged param specs
					p_default,		30.0f,
					p_ms_default,	30.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ICONSIZE, IDC_ICONSIZESPIN, SPIN_AUTOSCALE,
			end,
		// color scheme
			kSpeedCopy_colorType, _T("Color_Type"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_COLORTYPE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_COLORCOORDINATED,
			end,
		// seed
			kSpeedCopy_seed,		_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT, IDC_SEED, IDC_SEEDSPIN, 1.f,
			end,
		// new?
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc speedCopy_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSpeedCopyDesc, FP_MIXIN, 
				
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

FPInterfaceDesc speedCopy_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSpeedCopyDesc, FP_MIXIN,

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

FPInterfaceDesc speedCopy_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSpeedCopyDesc, FP_MIXIN,

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

BOOL PFOperatorSpeedCopyDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock = NULL;
	switch(msg) {

		case WM_INITDIALOG:
			// Send the message to notify the initialization of dialog
			if (map != NULL) pblock = map->GetParamBlock();
			if (pblock != NULL)	pblock->NotifyDependents( FOREVER, (PartID)map, kSpeedCopy_RefMsg_InitDlg );
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			if (map != NULL) pblock = map->GetParamBlock();
			switch(LOWORD(wParam)) {
				case IDC_NEW:
					if (pblock)
						pblock->NotifyDependents(FOREVER, PART_OBJ, kSpeedCopy_RefMsg_NewRand);
					break;
				case kSpeedCopy_message_update:
					UpdateDlg( hWnd, pblock);
					break;
			}
			break;
	}

	return FALSE;
}

void PFOperatorSpeedCopyDlgProc::UpdateDlg( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;

	bool useSpeedVar = ( pblock->GetInt(kSpeedCopy_useSpeedVariation, 0) != 0);
	bool useSteer = ( pblock->GetInt(kSpeedCopy_steer, 0) != 0);

	EnableWindow( GetDlgItem( hWnd, IDC_MINTEXT ), useSpeedVar);
	EnableWindow( GetDlgItem( hWnd, IDC_MAXTEXT ), useSpeedVar);
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_MINSPEEDSPIN ) );
	spin->Enable( useSpeedVar );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_MAXSPEEDSPIN ) );
	spin->Enable( useSpeedVar );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_DISTANCETEXT ), useSteer);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DISTANCESPIN ) );
	spin->Enable( useSteer );
	ReleaseISpinner( spin );
}

} // end of namespace PFActions