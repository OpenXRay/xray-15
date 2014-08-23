/**********************************************************************
 *<
	FILE:			PFTestDuration_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Duration Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestDuration_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestDurationDesc ThePFTestDurationDesc;

// custom update dialog process
TestDurationDlgProc theTestDurationDlgProc;

// Duration Test param block
ParamBlockDesc2 duration_paramBlockDesc 
(
	// required block spec
		kDuration_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestDurationDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kDuration_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_TESTDURATION_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestDurationDlgProc,
	// required param specs
		// test type
			kDuration_testType,_T("Test_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TESTTYPE,
			// optional tagged param specs
					p_default,		kDuration_testType_age,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kDuration_testType_num,
												IDS_DURATIONTYPE_TIME, 
												IDS_DURATIONTYPE_AGE,
												IDS_DURATIONTYPE_EVENT,
					p_vals,			kDuration_testType_time,
									kDuration_testType_age,
									kDuration_testType_event,
			end,
		// condition type: less or greater
			kDuration_conditionType, _T("Condition_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_CONDITIONTYPE,
			// optional tagged param specs
					p_default,		kDuration_conditionType_greater,
					p_range,		0, 1,
					p_ui,			TYPE_RADIO, 2, IDC_TRUELESS, IDC_TRUEGREATER,
			end,
		// test value
			kDuration_testValue,	_T("Test_Value"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TESTVALUE,
			// optional tagged param specs
					p_default,		4800, // 1 sec
					p_ms_default,	4800, // 1 sec
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_TEST,	IDC_TESTSPIN,	80.0f,
			end,
		// variation
			kDuration_variation,	_T("Variation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		800, // 1/6 sec
					p_ms_default,	800, // 1/6 sec
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_VARIATION,	IDC_VARIATIONSPIN,	80.0f,
			end,
		// subframe sampling
			kDuration_subframeSampling,	 _T("Subframe_Sampling"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLING,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SUBFRAME,
			end,
		// first/last disparity: test value differs for first particles from the last ones
			kDuration_disparity,	 _T(""), //_T("First_Last_Disparity"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									0, // IDS_FIRSTLASTDISPARITY,
			// optional tagged param specs
					p_default,		FALSE,
//					p_ui,			TYPE_SINGLECHEKBOX,	IDC_FIRSTTOLAST,
			end,
		// test for first particles
			kDuration_testFirst,	_T(""), //_T("Test_First"),
									TYPE_TIMEVALUE,
									P_RESET_DEFAULT,
									0, // IDS_TESTFIRST,
			// optional tagged param specs
					p_default,		4800, // 1 sec
					p_ms_default,	4800, // 1 sec
					p_range,		-999999999, 999999999,
//					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_TESTFIRST,	IDC_TESTFIRSTSPIN,	0.2f,
			end,
		// test for last particles
			kDuration_testLast,		_T(""), // _T("Test_Last"),
									TYPE_TIMEVALUE,
									P_RESET_DEFAULT,
									0, // IDS_TESTLAST,
			// optional tagged param specs
					p_default,		9600, // 2 sec
					p_ms_default,	9600, // 2 sec
					p_range,		-999999999, 999999999,
//					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_TESTLAST,	IDC_TESTLASTSPIN,	0.2f,
			end,
		// index of the last particle
			kDuration_lastIndex,	_T(""), // _T("Last_Index"),
									TYPE_INT,
									P_RESET_DEFAULT,
									0, // IDS_LASTINDEX,
			// optional tagged param specs
					p_default,		1000,
					p_ms_default,	1000,
					p_range,		0, 999999999,
//					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_LASTINDEX,	IDC_LASTINDEXSPIN,	1.0f,
			end,
		// random seed
			kDuration_randomSeed,	_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_ms_default,	12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc duration_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestDurationDesc, FP_MIXIN, 
				
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

FPInterfaceDesc duration_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestDurationDesc, FP_MIXIN,

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

FPInterfaceDesc duration_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestDurationDesc, FP_MIXIN,

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
BOOL TestDurationDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kDuration_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kDuration_RefMsg_NewRand );
			break;
		case kDuration_message_variation:
			UpdateVariationDlg( hWnd, map->GetParamBlock()->GetTimeValue(kDuration_variation, t) );
			break;
		case kDuration_message_disparity:
			UpdateDisparityDlg( hWnd, map->GetParamBlock()->GetInt(kDuration_disparity, t) );
			break;
		}
	}
	return FALSE;
}

void TestDurationDlgProc::UpdateVariationDlg( HWND hWnd, TimeValue variation )
{
	bool useRandom = (variation != 0);
	ISpinnerControl* spin;

	EnableWindow( GetDlgItem( hWnd, IDC_UNIQUENESSBOX ), useRandom );
	EnableWindow( GetDlgItem( hWnd, IDC_SEEDTEXT ), useRandom );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable( useRandom );
	ReleaseISpinner( spin );
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
	button->Enable( useRandom );
	ReleaseICustButton( button );
}

void TestDurationDlgProc::UpdateDisparityDlg( HWND hWnd, int disparity )
{
	bool useDisparity = (disparity != 0);
/*	ISpinnerControl* spin;

	EnableWindow( GetDlgItem( hWnd, IDC_TESTVALUETEXT ), !useDisparity);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_TESTSPIN ) );
	spin->Enable( !useDisparity );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_TESTFIRSTTEXT ), useDisparity );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_TESTFIRSTSPIN ) );
	spin->Enable( useDisparity );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_TESTLASTTEXT ), useDisparity );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_TESTLASTSPIN ) );
	spin->Enable( useDisparity );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_LASTINDEXTEXT ), useDisparity );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_LASTINDEXSPIN ) );
	spin->Enable( useDisparity );
	ReleaseISpinner( spin );
*/
}



} // end of namespace PFActions
