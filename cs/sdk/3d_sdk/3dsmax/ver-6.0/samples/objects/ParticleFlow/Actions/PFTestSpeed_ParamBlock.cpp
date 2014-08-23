/**********************************************************************
 *<
	FILE:			PFTestSpeed_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Speed Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestSpeed_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestSpeedDesc ThePFTestSpeedDesc;

// custom update dialog process
TestSpeedDlgProc theTestSpeedDlgProc;

// Speed Test param block
ParamBlockDesc2 speed_paramBlockDesc 
(
	// required block spec
		kSpeedTest_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestSpeedDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpeedTest_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_TESTSPEED_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestSpeedDlgProc,
	// required param specs
			// test type
			kSpeedTest_testType,	_T("Test_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TESTTYPE,
			// optional tagged param specs
					p_default,		kSpeedTest_testType_speed,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kSpeedTest_testType_num,
												IDS_SPEEDTYPE_SPEEDMAG, 
												IDS_SPEEDTYPE_SPEEDX,
												IDS_SPEEDTYPE_SPEEDY,
												IDS_SPEEDTYPE_SPEEDZ,
												IDS_SPEEDTYPE_ACCELMAG,
												IDS_SPEEDTYPE_ACCELX,
												IDS_SPEEDTYPE_ACCELY,
												IDS_SPEEDTYPE_ACCELZ,
												IDS_SPEEDTYPE_STEERINGRATE,
												IDS_SPEEDTYPE_WHENACCELS,
												IDS_SPEEDTYPE_WHENSLOWS,
					p_vals,			kSpeedTest_testType_speed,
									kSpeedTest_testType_speedX,
									kSpeedTest_testType_speedY,
									kSpeedTest_testType_speedZ,
									kSpeedTest_testType_accel,
									kSpeedTest_testType_accelX,
									kSpeedTest_testType_accelY,
									kSpeedTest_testType_accelZ,
									kSpeedTest_testType_steering,
									kSpeedTest_testType_whenAccels,
									kSpeedTest_testType_whenSlows,
			end,
		// condition type: less or greater
			kSpeedTest_conditionType, _T("Condition_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_CONDITIONTYPE,
			// optional tagged param specs
					p_default,		kSpeedTest_conditionType_greater,
					p_range,		0, 1,
					p_ui,			TYPE_RADIO, 2, IDC_TRUELESS, IDC_TRUEGREATER,
			end,
		// unit value
			kSpeedTest_unitValue,	_T("Unit_Value"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_UNITVALUE,
			// optional tagged param specs
					p_default,		300.0f,
					p_range,		-999999999.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE, IDC_UNITVALUE, IDC_UNITSPIN, SPIN_AUTOSCALE,
			end,
		// unit variation
			kSpeedTest_unitVariation,	_T("Unit_Variation"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_UNITVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE, IDC_UNITVAR, IDC_UNITVARSPIN, SPIN_AUTOSCALE,
			end,

		// angle value
			kSpeedTest_angleValue,	_T("Angle_Value"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ANGLEVALUE,
			// optional tagged param specs
					p_default,		6.2831853f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_ANGLEVALUE, IDC_ANGLESPIN, 1.0f,
			end,
		// angle variation
			kSpeedTest_angleVariation,	_T("Angle_Variation"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ANGLEVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT, IDC_ANGLEVAR, IDC_ANGLEVARSPIN, 1.0f,
			end,
		// sync type
			kSpeedTest_sync,		_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpeedTest_sync_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSpeedTest_sync_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedTest_sync_time,
									kSpeedTest_sync_age,
									kSpeedTest_sync_event,
			end,
		// random seed
			kSpeedTest_randomSeed,	_T("Random_Seed"),
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
FPInterfaceDesc speedTest_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestSpeedDesc, FP_MIXIN, 
				
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

FPInterfaceDesc speedTest_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestSpeedDesc, FP_MIXIN,

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

FPInterfaceDesc speedTest_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestSpeedDesc, FP_MIXIN,

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
//|					custom UI update for Speed Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestSpeedDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;
	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSpeedTest_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = map->GetParamBlock();
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			if (pblock)
				pblock->NotifyDependents( FOREVER, PART_OBJ, kSpeedTest_RefMsg_NewRand );
			break;
		case kSpeedTest_message_update:
			if (pblock)
				UpdateDlg( hWnd, pblock->GetInt(kSpeedTest_testType, t),
								pblock->GetFloat(kSpeedTest_unitVariation, t),
								pblock->GetFloat(kSpeedTest_angleVariation, t) );
			break;
		}
	}
	return FALSE;
}

void TestSpeedDlgProc::UpdateDlg( HWND hWnd, int type, float sizeVar, float speedVar )
{
	ISpinnerControl* spin;

	bool needParams = (type < kSpeedTest_testType_whenAccels);
	
	EnableWindow( GetDlgItem( hWnd, IDC_TESTFRAME ), needParams );
	EnableWindow( GetDlgItem( hWnd, IDC_TRUELESS ), needParams );
	EnableWindow( GetDlgItem( hWnd, IDC_TRUEGREATER ), needParams );

	EnableWindow( GetDlgItem( hWnd, IDC_TESTVALUETEXT ), needParams );
	EnableWindow( GetDlgItem( hWnd, IDC_VARIATIONTEXT ), needParams );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCBYTEXT ), needParams );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNC ), needParams );

	bool useUnit = (type != kSpeedTest_testType_steering);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_UNITSPIN ) );
	spin->Enable( useUnit && needParams);
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_UNITVARSPIN ) );
	spin->Enable( useUnit && needParams);
	ReleaseISpinner( spin );
	int showUnit = useUnit ? SW_SHOW : SW_HIDE;
	ShowWindow( GetDlgItem( hWnd, IDC_UNITVALUE), showUnit);
	ShowWindow( GetDlgItem( hWnd, IDC_UNITSPIN), showUnit);
	ShowWindow( GetDlgItem( hWnd, IDC_UNITVAR), showUnit);
	ShowWindow( GetDlgItem( hWnd, IDC_UNITVARSPIN), showUnit);

	bool useAngle = (type == kSpeedTest_testType_steering);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_ANGLESPIN ) );
	spin->Enable( useAngle && needParams);
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_ANGLEVARSPIN ) );
	spin->Enable( useAngle && needParams);
	ReleaseISpinner( spin );
	int showAngle = useAngle ? SW_SHOW : SW_HIDE;
	ShowWindow( GetDlgItem( hWnd, IDC_ANGLEVALUE), showAngle);
	ShowWindow( GetDlgItem( hWnd, IDC_ANGLESPIN), showAngle);
	ShowWindow( GetDlgItem( hWnd, IDC_ANGLEVAR), showAngle);
	ShowWindow( GetDlgItem( hWnd, IDC_ANGLEVARSPIN), showAngle);

	bool useRandom = (type == kSpeedTest_testType_speed) ? 
							(speedVar > 0.0f) : (sizeVar > 0.0f);
	useRandom = useRandom && needParams;
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
