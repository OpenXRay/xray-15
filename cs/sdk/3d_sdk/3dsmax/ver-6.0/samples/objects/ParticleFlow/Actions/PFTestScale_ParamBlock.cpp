/**********************************************************************
 *<
	FILE:			PFTestScale_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Scale Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestScale_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestScaleDesc ThePFTestScaleDesc;

// custom update dialog process
TestScaleDlgProc theTestScaleDlgProc;

// Scale Test param block
ParamBlockDesc2 scale_paramBlockDesc 
(
	// required block spec
		kScaleTest_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestScaleDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kScaleTest_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_TESTSCALE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestScaleDlgProc,
	// required param specs
			// test type
			kScaleTest_testType,	_T("Test_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TESTTYPE,
			// optional tagged param specs
					p_default,		kScaleTest_testType_scale,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kScaleTest_testType_num,
												IDS_SCALETYPE_PRESIZE, 
												IDS_SCALETYPE_POSTSIZE,
												IDS_SCALETYPE_SCALE,
					p_vals,			kScaleTest_testType_preSize,
									kScaleTest_testType_postSize,
									kScaleTest_testType_scale,
			end,
			// axis type
			kScaleTest_axisType,	_T("Axis_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_AXISTYPE,
			// optional tagged param specs
					p_default,		kScaleTest_axisType_average,
					p_ui,			TYPE_INTLISTBOX,	IDC_AXISTYPE,	kScaleTest_axisType_num,
												IDS_AXISTYPE_AVERAGE, 
												IDS_AXISTYPE_MINIMUM, 
												IDS_AXISTYPE_MEDIAN, 
												IDS_AXISTYPE_MAXIMUM, 
												IDS_AXISTYPE_X, 
												IDS_AXISTYPE_Y, 
												IDS_AXISTYPE_Z, 
					p_vals,			kScaleTest_axisType_average,
									kScaleTest_axisType_minimum,
									kScaleTest_axisType_median,
									kScaleTest_axisType_maximum,
									kScaleTest_axisType_x,
									kScaleTest_axisType_y,
									kScaleTest_axisType_z,
			end,
		// condition type: less or greater
			kScaleTest_conditionType, _T("Condition_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_CONDITIONTYPE,
			// optional tagged param specs
					p_default,		kScaleTest_conditionType_greater,
					p_range,		0, 1,
					p_ui,			TYPE_RADIO, 2, IDC_TRUELESS, IDC_TRUEGREATER,
			end,
		// size value
			kScaleTest_sizeValue,	_T("Size_Value"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SIZEVALUE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE, IDC_SIZEVALUE, IDC_SIZESPIN, SPIN_AUTOSCALE,
			end,
		// size variation
			kScaleTest_sizeVariation,	_T("Size_Variation"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SIZEVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE, IDC_SIZEVAR, IDC_SIZEVARSPIN, SPIN_AUTOSCALE,
			end,

		// scale value
			kScaleTest_scaleValue,	_T("Scale_Value"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SCALEVALUE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_SCALEVALUE, IDC_SCALESPIN, 0.5f,
			end,
		// size variation
			kScaleTest_scaleVariation,	_T("Scale_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SCALEVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_SCALEVAR, IDC_SCALEVARSPIN, 0.5f,
			end,
		// sync type
			kScaleTest_sync,		_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kScaleTest_sync_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kScaleTest_sync_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kScaleTest_sync_time,
									kScaleTest_sync_age,
									kScaleTest_sync_event,
			end,
		// random seed
			kScaleTest_randomSeed,	_T("Random_Seed"),
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
FPInterfaceDesc scaleTest_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestScaleDesc, FP_MIXIN, 
				
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

FPInterfaceDesc scaleTest_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestScaleDesc, FP_MIXIN,

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

FPInterfaceDesc scaleTest_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestScaleDesc, FP_MIXIN,

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
//|					custom UI update for Scale Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestScaleDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;
	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kScaleTest_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = map->GetParamBlock();
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			if (pblock)
				pblock->NotifyDependents( FOREVER, PART_OBJ, kScaleTest_RefMsg_NewRand );
			break;
		case kScaleTest_message_update:
			if (pblock)
				UpdateDlg( hWnd, pblock->GetInt(kScaleTest_testType, t),
								pblock->GetFloat(kScaleTest_sizeVariation, t),
								pblock->GetFloat(kScaleTest_scaleVariation, t) );
			break;
		}
	}
	return FALSE;
}

void TestScaleDlgProc::UpdateDlg( HWND hWnd, int type, float sizeVar, float scaleVar )
{
	ISpinnerControl* spin;

	bool useSize = (type != kScaleTest_testType_scale);
	EnableWindow( GetDlgItem( hWnd, IDC_SIZEFRAME ), useSize );
	EnableWindow( GetDlgItem( hWnd, IDC_SIZEVALUETEXT ), useSize );
	EnableWindow( GetDlgItem( hWnd, IDC_SIZEVARTEXT ), useSize );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SIZESPIN ) );
	spin->Enable( useSize );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SIZEVARSPIN ) );
	spin->Enable( useSize );
	ReleaseISpinner( spin );

	bool useScale = (type == kScaleTest_testType_scale);
	EnableWindow( GetDlgItem( hWnd, IDC_SCALEFRAME ), useScale );
	EnableWindow( GetDlgItem( hWnd, IDC_SCALEVALUETEXT ), useScale );
	EnableWindow( GetDlgItem( hWnd, IDC_SCALEVARTEXT ), useScale );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SCALESPIN ) );
	spin->Enable( useScale );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SCALEVARSPIN ) );
	spin->Enable( useScale );
	ReleaseISpinner( spin );

	bool useRandom = (type == kScaleTest_testType_scale) ? 
							(scaleVar > 0.0f) : (sizeVar > 0.0f);
	EnableWindow( GetDlgItem( hWnd, IDC_UNIQUENESSBOX ), useRandom );
	EnableWindow( GetDlgItem( hWnd, IDC_SEEDTEXT ), useRandom );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable( useRandom );
	ReleaseISpinner( spin );
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
	button->Enable( useRandom );
	ReleaseICustButton( button );
}


} // end of namespace PFActions
