/**********************************************************************
 *<
	FILE:			PFTestGoToRotation_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for GoToRotation Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-14-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestGoToRotation_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestGoToRotationDesc ThePFTestGoToRotationDesc;

// custom update dialog process
TestGoToRotationDlgProc theTestGoToRotationDlgProc;

// Duration Test param block
ParamBlockDesc2 goToRotation_paramBlockDesc 
(
	// required block spec
		kGoToRotation_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestGoToRotationDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kGoToRotation_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_GOTOROTATION_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestGoToRotationDlgProc,
	// required param specs
		// test true for particles when transition period ends
			kGoToRotation_sendOut,	_T("Send_Out"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SENDOUT,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_HASTEST,
			end,
		// sync by
			kGoToRotation_syncBy,	_T("Transition_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TRANSITIONTYPE,
			// optional tagged param specs
					p_default,		kGoToRotation_syncBy_event,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kGoToRotation_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, 
												IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kGoToRotation_syncBy_time,
									kGoToRotation_syncBy_age,
									kGoToRotation_syncBy_event,
			end,
		// test value
			kGoToRotation_time,		_T("Time"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TIME,
			// optional tagged param specs
					p_default,		4800, // 1 sec
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_VALUE,	IDC_VALUESPIN,	80.0f,
			end,
		// variation
			kGoToRotation_variation,_T("Variation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0,
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_VARIATION,	IDC_VARIATIONSPIN,	80.0f,
			end,
		// test type: once, or per frame, or by distance
			kGoToRotation_targetType,	_T("Target_Rotation"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_TARGETROTATION,
			// optional tagged param specs
					p_default,		kGoToRotation_targetType_constant,
					p_range,		0, kGoToRotation_targetType_num-1,
					p_ui,			TYPE_RADIO, kGoToRotation_targetType_num, IDC_TARGETCONSTANT, IDC_TARGETCHANGING,
			end,
		// match initial spin
			kGoToRotation_matchSpin,	_T("Match_Initial_Spin"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_MATCHINITIALSPIN,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_MATCHSPIN,
			end,
		// spin rate
			kGoToRotation_spin,		_T("SpinRate"),
									TYPE_ANGLE,
									P_RESET_DEFAULT,
									IDS_SPINRATE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_SPINRATE, IDC_SPINRATESPIN, SPIN_AUTOSCALE,
			end,
		// spin rate variation
			kGoToRotation_spinVariation, _T("Spin_Rate_Variation"),
									TYPE_ANGLE,
									P_RESET_DEFAULT,
									IDS_SPINVARIATION,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 9999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_SPINVARIATION, IDC_SPINVARIATIONSPIN, SPIN_AUTOSCALE,
			end,
		// easy in percentage
			kGoToRotation_easeIn, _T("Ease_In"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_EASEIN,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_EASEIN2, IDC_EASEINSPIN, 0.2f,
			end,
		// stop spinning particles when they reach the target rotation
			kGoToRotation_stopSpin,	_T("Stop_Spinning"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_STOPSPINNING,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_STOPSPIN,
			end,
		// random seed
			kGoToRotation_randomSeed,		_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc goToRotation_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestGoToRotationDesc, FP_MIXIN, 
				
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

FPInterfaceDesc goToRotation_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFTestGoToRotationDesc, FP_MIXIN,

			IPFOperator::kProceed, _T("proceed"), 0, TYPE_bool, 0, 7,
				_T("container"), 0, TYPE_IOBJECT,
				_T("timeStart"), 0, TYPE_TIMEVALUE,
				_T("timeEnd"), 0, TYPE_TIMEVALUE,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,

		end
);

FPInterfaceDesc goToRotation_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestGoToRotationDesc, FP_MIXIN,

			IPFTest::kProceedStep1, _T("proceedStep1"), 0, TYPE_VOID, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,
			IPFTest::kProceedStep2, _T("proceedStep2"), 0, TYPE_bool, 0, 6,
				_T("timeStartTick"), 0, TYPE_TIMEVALUE,
				_T("timeStartFraction"), 0, TYPE_FLOAT,
				_T("timeEndTick"), 0, TYPE_TIMEVALUE_BR,
				_T("timeEndFraction"), 0, TYPE_FLOAT_BR,
				_T("testResult"), 0, TYPE_BITARRAY_BR,
				_T("testTime"), 0, TYPE_FLOAT_TAB_BR,
			IPFTest::kGetNextActionList, _T("getNextActionList"), 0, TYPE_INODE, 0, 2,
				_T("testNode"), 0, TYPE_INODE,
				_T("linkActive"), 0, TYPE_bool_BP,
			IPFTest::kSetNextActionList, _T("setNextActionList"), 0, TYPE_bool, 0, 2,
				_T("actionList"), 0, TYPE_INODE,
				_T("testNode"), 0, TYPE_INODE,
			IPFTest::kSetLinkActive, _T("setLinkActive"), 0, TYPE_bool, 0, 2,
				_T("linkActive"), 0, TYPE_bool,
				_T("testNode"), 0, TYPE_INODE,
			IPFTest::kClearNextActionList, _T("clearNextActionList"), 0, TYPE_bool, 0, 1,
				_T("testNode"), 0, TYPE_INODE,

		end
);

FPInterfaceDesc goToRotation_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestGoToRotationDesc, FP_MIXIN,

			IPViewItem::kNumPViewParamBlocks, _T("numPViewParamBlocks"), 0, TYPE_INT, 0, 0,
			IPViewItem::kGetPViewParamBlock, _T("getPViewParamBlock"), 0, TYPE_REFTARG, 0, 1,
				_T("index"), 0, TYPE_INDEX,
			IPViewItem::kHasComments, _T("hasComments"), 0, TYPE_bool, 0, 1,
				_T("action"), 0, TYPE_INODE,
			IPViewItem::kGetComments, _T("getComments"), 0, TYPE_STRING, 0, 1,
				_T("action"), 0, TYPE_INODE,
			IPViewItem::kSetComments, _T("setComments"), 0, TYPE_VOID, 0, 2,
				_T("action"), 0, TYPE_INODE,
				_T("comments"), 0, TYPE_STRING,
			IPViewItem::kGetWireExtension, _T("getWireExtension"), 0, TYPE_INT, 0, 2,
				_T("action"), 0, TYPE_INODE,
				_T("wireHeight"), 0, TYPE_INT_BR,
			IPViewItem::kSetWireExtension, _T("setWireExtension"), 0, TYPE_VOID, 0, 3,
				_T("action"), 0, TYPE_INODE,
				_T("wireWidth"), 0, TYPE_INT,
				_T("wireHeight"), 0, TYPE_INT,
		end
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|					custom UI update for Duration Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestGoToRotationDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	IParamBlock2* pblock;
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kGoToRotation_RefMsg_InitDlg );
		break;
	case WM_COMMAND:
		pblock = map->GetParamBlock();
		switch (LOWORD(wParam))
		{
		case IDC_NEW:
			if (pblock)
				pblock->NotifyDependents(FOREVER, PART_OBJ, kGoToRotation_RefMsg_NewRand);
			return TRUE;
		case kGoToRotation_message_sync:
			if (pblock)
				UpdateSyncDlg( hWnd, pblock->GetInt(kGoToRotation_syncBy,t) );
			break;
		case kGoToRotation_message_variation:
			if (pblock)
				UpdateVariationDlg( hWnd, pblock->GetTimeValue(kGoToRotation_variation,t),
											pblock->GetFloat(kGoToRotation_spinVariation,t) );
			break;
		case kGoToRotation_message_match:
			if (pblock)
				UpdateMatchDlg( hWnd, pblock->GetInt(kGoToRotation_matchSpin,t) );
			break;
		}
		break;
	}
	return FALSE;
}

void TestGoToRotationDlgProc::UpdateSyncDlg( HWND hWnd, int syncBy )
{
	int ids = 0;
	switch(syncBy) {
	case kGoToRotation_syncBy_time: ids = IDS_TIME; break;
	case kGoToRotation_syncBy_age: ids = IDS_AGE; break;
	case kGoToRotation_syncBy_event: ids = IDS_DURATION; break;
	}
	TCHAR label[32];
	sprintf(label,"%s:",GetString(ids));
	SetWindowText( GetDlgItem(hWnd, IDC_VALUETEXT), label);
}

void TestGoToRotationDlgProc::UpdateVariationDlg( HWND hWnd, TimeValue timeVar, float spinVar )
{
	bool enable = ((timeVar != 0) || (spinVar != 0.0f));
	EnableWindow( GetDlgItem(hWnd, IDC_UNIQUENESSBOX), enable );
	EnableWindow( GetDlgItem(hWnd, IDC_SEEDTEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner(spin);
	ICustButton* but = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
	but->Enable(enable);
	ReleaseICustButton(but);
}

void TestGoToRotationDlgProc::UpdateMatchDlg( HWND hWnd, int matchSpin )
{
	bool enable = (matchSpin == 0);
	EnableWindow( GetDlgItem(hWnd, IDC_SPINTEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SPINRATESPIN ) );
	spin->Enable(enable);
	ReleaseISpinner(spin);
}





} // end of namespace PFActions
