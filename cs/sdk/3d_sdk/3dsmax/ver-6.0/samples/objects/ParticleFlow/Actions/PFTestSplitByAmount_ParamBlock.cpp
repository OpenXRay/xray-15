/**********************************************************************
 *<
	FILE:			PFTestSplitByAmount_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SplitByAmount Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestSplitByAmount_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestSplitByAmountDesc ThePFTestSplitByAmountDesc;

// custom update dialog process
TestSplitByAmountDlgProc theTestSplitByAmountDlgProc;

// SplitByAmount Test param block
ParamBlockDesc2 splitByAmount_paramBlockDesc 
(
	// required block spec
		kSplitByAmount_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestSplitByAmountDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSplitByAmount_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPLITBYAMOUNT_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestSplitByAmountDlgProc,
	// required param specs
		// test type
			kSplitByAmount_testType,_T("Test_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TESTTYPE,
			// optional tagged param specs
					p_default,		kSplitByAmount_testType_fraction,
					p_range,		0, kSplitByAmount_testType_num-1,
					p_ui,			TYPE_RADIO, kSplitByAmount_testType_num, 
									IDC_TRUEFRACTION, IDC_TRUENTH, IDC_TRUEFIRSTN, IDC_TRUEAFTERN,
			end,
		// fraction
			kSplitByAmount_fraction, _T("Ratio"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_RATIO,
			// optional tagged param specs
					p_default,		0.5f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_RATIO, IDC_RATIO2SPIN, 1.0f,
			end,
		// every n-th particle
			kSplitByAmount_everyN,	_T("Every_Nth"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_EVERYNTH,
			// optional tagged param specs
					p_default,		3,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_EVERYN,	IDC_EVERYNSPIN,	1.0f,
			end,
		// first n particle
			kSplitByAmount_firstN,	_T("First_N"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_FIRSTN,
			// optional tagged param specs
					p_default,		100,
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_FIRSTN,	IDC_FIRSTNSPIN,	1.0f,
			end,
		// per emission source
			kSplitByAmount_perSource,	 _T("Per_Emission_Source"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_PEREMISSIONSOURCE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_PERSOURCE,
			end,
		// random seed
			kSplitByAmount_randomSeed,	_T("Random_Seed"),
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
FPInterfaceDesc splitByAmount_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestSplitByAmountDesc, FP_MIXIN, 
				
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

FPInterfaceDesc splitByAmount_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestSplitByAmountDesc, FP_MIXIN,

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

FPInterfaceDesc splitByAmount_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestSplitByAmountDesc, FP_MIXIN,

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
//|					custom UI update for SplitByAmount Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestSplitByAmountDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;
	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		pblock = (map != NULL) ? map->GetParamBlock() : NULL;
		if (pblock) pblock->NotifyDependents( FOREVER, (PartID)map, kSplitByAmount_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = (map != NULL) ? map->GetParamBlock() : NULL;
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			if (pblock) pblock->NotifyDependents( FOREVER, PART_OBJ, kSplitByAmount_RefMsg_NewRand );
			break;
		case kSplitByAmount_message_type:
			if (pblock) UpdateTypeDlg( hWnd, pblock->GetTimeValue(kSplitByAmount_testType, t) );
			break;
		}
	}
	return FALSE;
}

void TestSplitByAmountDlgProc::UpdateTypeDlg( HWND hWnd, int type )
{
	bool enable = (type == kSplitByAmount_testType_fraction);
	bool bRandom = enable;
	EnableWindow( GetDlgItem( hWnd, IDC_RATIOTEXT ), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_RATIO2SPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );

	enable = (type == kSplitByAmount_testType_everyN);
	EnableWindow( GetDlgItem( hWnd, IDC_EVERYNTEXT ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_EVERYNSPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );

	enable = (type >= kSplitByAmount_testType_firstN);
	EnableWindow( GetDlgItem( hWnd, IDC_FIRSTNTEXT ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_PERSOURCE ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_FIRSTNSPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_UNIQUENESSBOX ), bRandom );
	EnableWindow( GetDlgItem( hWnd, IDC_SEEDTEXT ), bRandom );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable( bRandom );
	ReleaseISpinner( spin );
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
	button->Enable( bRandom );
	ReleaseICustButton( button );
}


} // end of namespace PFActions
