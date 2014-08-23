/**********************************************************************
 *<
	FILE:			PFTestSpeedGoToTarget_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SpeedGoToTarget Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 07-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestSpeedGoToTarget_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestSpeedGoToTargetDesc ThePFTestSpeedGoToTargetDesc;

// custom update dialog process
TestSpeedGoToTargetDlgProc theTestSpeedGoToTargetDlgProc;

// SpeedGoToTarget Test param block
ParamBlockDesc2 speedGoToTarget_paramBlockDesc 
(
	// required block spec
		kSpeedGoToTarget_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestSpeedGoToTargetDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpeedGoToTarget_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPEEDGOTOTARGET_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestSpeedGoToTargetDlgProc,
	// required param specs
		// control type: by speed or by time
			kSpeedGoToTarget_type,	_T("Speed_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SPEEDTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_type_speed,
					p_ui,			TYPE_INTLISTBOX,	IDC_SPEEDTYPE,	kSpeedGoToTarget_type_num,
												IDS_SPEEDTYPE_BYSPEED, IDS_SPEEDTYPE_BYTIME,
												IDS_SPEEDTYPE_NOCONTROL,
					p_vals,			kSpeedGoToTarget_type_speed,
									kSpeedGoToTarget_type_time,
									kSpeedGoToTarget_type_none,
			end,
		// use cruise speed
			kSpeedGoToTarget_useCruiseSpeed, _T("Use_Cruise_Speed"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USECRUISESPEED,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USECRUISESPEED,
			end,
		// speed value
			kSpeedGoToTarget_cruiseSpeed, _T("Cruise_Speed"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_CRUISESPEED,
			// optional tagged param specs
					p_default,		300.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_CRUISESPEED, IDC_CRUISESPEEDSPIN, SPIN_AUTOSCALE,
			end,
		// speed variation
			kSpeedGoToTarget_cruiseSpeedVariation, _T("Cruise_Speed_Variation"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_CRUISESPEEDVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_CRUISEVARIATION, IDC_CRUISEVARIATIONSPIN, SPIN_AUTOSCALE,
			end,
		// acceleration maximum
			kSpeedGoToTarget_accelLimit, _T("Acceleration_Limit"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_ACCELERATIONLIMIT,
			// optional tagged param specs
					p_default,		1000.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ACCELLIMIT, IDC_ACCELLIMITSPIN, SPIN_AUTOSCALE,
			end,
		// sync type
			kSpeedGoToTarget_syncBy, _T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSpeedGoToTarget_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedGoToTarget_syncBy_time,
									kSpeedGoToTarget_syncBy_age,
									kSpeedGoToTarget_syncBy_event,
			end,
		// test distance
			kSpeedGoToTarget_distance, _T("Test_Distance"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_TESTDISTANCE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DISTANCE, IDC_DISTANCESPIN, SPIN_AUTOSCALE,
			end,
		// timing type
			kSpeedGoToTarget_timingType, _T("Timing_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TIMINGTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_timingType_event,
					p_ui,			TYPE_INTLISTBOX,	IDC_TIMING,	kSpeedGoToTarget_timingType_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedGoToTarget_timingType_time,
									kSpeedGoToTarget_timingType_age,
									kSpeedGoToTarget_timingType_event,
			end,
		// time value
			kSpeedGoToTarget_timeValue,	_T("Time"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TIME,
			// optional tagged param specs
					p_default,		9600, // 1 sec
					p_ms_default,	9600, // 1 sec
					p_range,		-999999999, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_TIMEVALUE,	IDC_TIMEVALUESPIN,	80.0f,
			end,
		// time variation
			kSpeedGoToTarget_timeVariation,	_T("Time_Variation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TIMEVARIATION,
			// optional tagged param specs
					p_default,		800, // 1/6 sec
					p_ms_default,	800, // 1/6 sec
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_TIMEVARIATION,	IDC_TIMEVARIATIONSPIN,	80.0f,
			end,
		// subframe sampling
			kSpeedGoToTarget_subframe,	 _T("Subframe_Sampling"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLING,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SUBFRAME,
			end,
		// use docking speed
			kSpeedGoToTarget_useDockingSpeed,	 _T("Use_Docking_Speed"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USEDOCKINGSPEED,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USEDOCKINGSPEED,
			end,
		// docking speed
			kSpeedGoToTarget_dockingSpeed, _T("Docking_Speed"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_DOCKINGSPEED,
			// optional tagged param specs
					p_default,		100.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DOCKINGSPEED, IDC_DOCKINGSPEEDSPIN, SPIN_AUTOSCALE,
			end,
		// docking easy in
			kSpeedGoToTarget_dockingSpeedVariation, _T("Docking_Speed_Variation"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DOCKINGSPEEDVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DOCKINGVARIATION, IDC_DOCKINGVARIATIONSPIN, SPIN_AUTOSCALE,
			end,
		// target type: icon or mesh object(s)
			kSpeedGoToTarget_targetType, _T("Target_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_TARGETTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_targetType_icon,
					p_range,		kSpeedGoToTarget_targetType_icon, kSpeedGoToTarget_targetType_objects,
					p_ui,			TYPE_RADIO, kSpeedGoToTarget_targetType_num, IDC_TARGETICON, IDC_TARGETOBJECTS,
			end,
		// target objects
			kSpeedGoToTarget_targets, _T(""),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE,	0,
						p_ui,	TYPE_NODELISTBOX, IDC_OBJECTLIST,IDC_PICKNODE,0,IDC_REMOVENODE,
			end,
		// target objects for maxscript manipulation
			kSpeedGoToTarget_targetsMaxscript, _T("Target_Objects"),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE|P_NO_REF,	IDS_TARGETOBJECTS,
			end,
		// animated shape
			kSpeedGoToTarget_animated,	 _T("Animated_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ANIMATEDSHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANIMATED,
			end,
		// follow target animation
			kSpeedGoToTarget_follow,	 _T("Follow_Target"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_FOLLOWTARGET,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_FOLLOWTARGET,
			end,
		// follow target animation
			kSpeedGoToTarget_lock,	 _T("Lock_On_Target"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_LOCKONTARGET,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_LOCKTARGET,
			end,
		// point type
			kSpeedGoToTarget_pointType, _T("Aim_Point_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_AIMPOINTTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_pointType_random,
					p_ui,			TYPE_INTLISTBOX,	IDC_POINT,	kSpeedGoToTarget_pointType_num,
												IDS_AIMPOINT_RANDOM, IDS_AIMPOINT_SURFACE,
												IDS_AIMPOINT_VECTOR,
					p_vals,			kSpeedGoToTarget_pointType_random,
									kSpeedGoToTarget_pointType_surface,
									kSpeedGoToTarget_pointType_vector,
			end,
		// assignment type
			kSpeedGoToTarget_assignment, _T("Assignment_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_ASSIGNMENTTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_assignment_random,
					p_ui,			TYPE_INTLISTBOX,	IDC_ASSIGNMENT,	kSpeedGoToTarget_assignment_num,
												IDS_ASSIGNMENT_RANDOM, IDS_ASSIGNMENT_DISTANCE,
												IDS_ASSIGNMENT_SURFACE, IDS_ASSIGMENT_DEVIATION,
												IDS_ASSIGNMENT_INTEGER,
					p_vals,			kSpeedGoToTarget_assignment_random,
									kSpeedGoToTarget_assignment_distance,
									kSpeedGoToTarget_assignment_surface,
									kSpeedGoToTarget_assignment_deviation,
									kSpeedGoToTarget_assignment_integer,
			end,
		// docking type
			kSpeedGoToTarget_dockingType, _T("Docking_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_DOCKINGTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_dockingType_none,
					p_ui,			TYPE_INTLISTBOX,	IDC_DIRECTION,	kSpeedGoToTarget_dockingType_num,
												IDS_DOCKINGTYPE_NONE,
												IDS_DOCKINGTYPE_PARALLEL, IDS_DOCKINGTYPE_SPHERICAL,
												IDS_DOCKINGTYPE_CYLINDRICAL, IDS_DOCKINGTYPE_NORMAL,
					p_vals,			kSpeedGoToTarget_dockingType_none,
									kSpeedGoToTarget_dockingType_parallel,
									kSpeedGoToTarget_dockingType_spherical,
									kSpeedGoToTarget_dockingType_cylindrical,
									kSpeedGoToTarget_dockingType_normal,
			end,
		// icon size
			kSpeedGoToTarget_iconSize, _T("Icon_Size"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_ICONSIZE,
			// optional tagged param specs
					p_default,		30.0f,
					p_ms_default,	30.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ICONSIZE, IDC_ICONSIZESPIN, SPIN_AUTOSCALE,
			end,
		// random seed
			kSpeedGoToTarget_randomSeed,	_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_ms_default,	12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,
		// test type
			kSpeedGoToTarget_testType,	_T("Test_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TESTTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_testType_point,
					p_ui,			TYPE_INTLISTBOX,	IDC_TESTTYPE,	kSpeedGoToTarget_testType_num,
												IDS_TESTTYPE_TARGETPIVOT, IDS_TESTTYPE_TARGETPOINT,
					p_vals,			kSpeedGoToTarget_testType_pivot,
									kSpeedGoToTarget_testType_point,
			end,
		// ease in
			kSpeedGoToTarget_easeIn, _T("Ease_In"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_EASEIN,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,	IDC_EASEIN, IDC_EASEINSPIN2, 1.0f,
			end,
		// target sync type
			kSpeedGoToTarget_targetSync, _T("Target_Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TARGETSYNCTYPE,
			// optional tagged param specs
					p_default,		kSpeedGoToTarget_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_TARGETSYNC,	kSpeedGoToTarget_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedGoToTarget_syncBy_time,
									kSpeedGoToTarget_syncBy_age,
									kSpeedGoToTarget_syncBy_event,
			end,
		// docking distance
			kSpeedGoToTarget_dockingDistance, _T("Docking_Distance"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_DOCKINGDISTANCE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_DOCKINGDISTANCE, IDC_DOCKINGDISTANCESPIN, SPIN_AUTOSCALE,
			end,
		// color scheme
			kSpeedGoToTarget_colorType, _T("Color_Type"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_COLORTYPE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_COLORCOORDINATED,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc speedGoToTarget_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestSpeedGoToTargetDesc, FP_MIXIN, 
				
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

FPInterfaceDesc speedGoToTarget_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestSpeedGoToTargetDesc, FP_MIXIN,

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

FPInterfaceDesc speedGoToTarget_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestSpeedGoToTargetDesc, FP_MIXIN,

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
			IPViewItem::kSetWireExtension, _T("setWireExtension"), 0, TYPE_VOID, 0, 2,
				_T("wireWidth"), 0, TYPE_INT,
				_T("wireHeight"), 0, TYPE_INT,
		end
);


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|					custom UI update for SpeedGoToTarget Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestSpeedGoToTargetDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSpeedGoToTarget_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kSpeedGoToTarget_RefMsg_NewRand );
			break;
		case IDC_PICKNODE:
			map->GetParamBlock()->NotifyDependents( FOREVER, 0, kSpeedGoToTarget_RefMsg_ResetValidatorAction );
			break;
		case IDC_BYLIST:
			map->GetParamBlock()->NotifyDependents( FOREVER, 0, kSpeedGoToTarget_RefMsg_ListSelect );
			break;
		case kSpeedGoToTarget_message_update:
			UpdateDlg( hWnd, map->GetParamBlock(), t );
			break;
		}
	}
	return FALSE;
}

void TestSpeedGoToTargetDlgProc::UpdateDlg( HWND hWnd, IParamBlock2* pblock, TimeValue time )
{
	if (pblock == NULL) return;

	int type = pblock->GetInt(kSpeedGoToTarget_type, time);
	bool useCruiseSpeed = (pblock->GetInt(kSpeedGoToTarget_useCruiseSpeed, time) != 0);
	bool useDockingSpeed = (pblock->GetInt(kSpeedGoToTarget_useDockingSpeed, time) != 0);
	int targetType = pblock->GetInt(kSpeedGoToTarget_targetType, time);
	int dockingType = pblock->GetInt(kSpeedGoToTarget_dockingType, type);
	int num = pblock->Count(kSpeedGoToTarget_targets);
	int numTargets = 0;
	for(int i=0; i<num; i++)
		if (pblock->GetINode(kSpeedGoToTarget_targets, time, i) != NULL)
			numTargets++;
	bool followTarget = (pblock->GetInt(kSpeedGoToTarget_follow, time) != 0);

	ISpinnerControl* spin;
	bool useBySpeed = (type == kSpeedGoToTarget_type_speed);
	EnableWindow( GetDlgItem( hWnd, IDC_SPEEDFRAME ), useBySpeed);
	EnableWindow( GetDlgItem( hWnd, IDC_USECRUISESPEED ), useBySpeed);
	EnableWindow( GetDlgItem( hWnd, IDC_CRUISESPEEDTEXT ), useBySpeed && useCruiseSpeed );
	EnableWindow( GetDlgItem( hWnd, IDC_CRUISEVARIATIONTEXT ), useBySpeed && useCruiseSpeed );
	EnableWindow( GetDlgItem( hWnd, IDC_ACCELLIMITEXT ), useBySpeed );
	EnableWindow( GetDlgItem( hWnd, IDC_EASEINTEXT2 ), useBySpeed );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCBYTEXT ), useBySpeed );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNC ), useBySpeed );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_CRUISESPEEDSPIN ) );
	spin->Enable( useBySpeed  && useCruiseSpeed);
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_CRUISEVARIATIONSPIN ) );
	spin->Enable( useBySpeed  && useCruiseSpeed);
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_ACCELLIMITSPIN ) );
	spin->Enable( useBySpeed );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_EASEINSPIN2 ) );
	spin->Enable( useBySpeed );
	ReleaseISpinner( spin );
	int showState = (type != kSpeedGoToTarget_type_time) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem( hWnd, IDC_SPEEDFRAME ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_USECRUISESPEED ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_CRUISESPEEDTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_CRUISEVARIATIONTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_ACCELLIMITEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_EASEINTEXT2 ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_SYNCBYTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_SYNC ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_CRUISESPEED ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_CRUISESPEEDSPIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_CRUISEVARIATION ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_CRUISEVARIATIONSPIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_ACCELLIMIT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_ACCELLIMITSPIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_EASEIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_EASEINSPIN2 ), showState);

	bool useByTime = (type == kSpeedGoToTarget_type_time);
	EnableWindow( GetDlgItem( hWnd, IDC_TIMEFRAME ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_TIMINGTEXT ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_TIMING ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_TIMEVALUETEXT ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_TIMEVARIATIONTEXT ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_SUBFRAME ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_USEDOCKINGSPEED ), useByTime );
	EnableWindow( GetDlgItem( hWnd, IDC_DOCKINGSPEEDTEXT ), useByTime && useDockingSpeed );
	EnableWindow( GetDlgItem( hWnd, IDC_DOCKINGVARIATIONTEXT ), useByTime && useDockingSpeed );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_TIMEVALUESPIN ) );
	spin->Enable( useByTime );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_TIMEVARIATIONSPIN ) );
	spin->Enable( useByTime );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DOCKINGSPEEDSPIN ) );
	spin->Enable( useByTime && useDockingSpeed );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DOCKINGVARIATIONSPIN ) );
	spin->Enable( useByTime && useDockingSpeed );
	ReleaseISpinner( spin );
	showState = (type == kSpeedGoToTarget_type_time) ? SW_SHOW : SW_HIDE;
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEFRAME ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMINGTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMING ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEVALUETEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEVARIATIONTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_SUBFRAME ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_USEDOCKINGSPEED ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_DOCKINGSPEEDTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_DOCKINGVARIATIONTEXT ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEVALUE ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEVALUESPIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEVARIATION ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_TIMEVARIATIONSPIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_DOCKINGSPEED ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_DOCKINGSPEEDSPIN ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_DOCKINGVARIATION ), showState);
	ShowWindow(GetDlgItem( hWnd, IDC_DOCKINGVARIATIONSPIN ), showState);

	ICustButton* but;
	bool useObjects = (targetType == kSpeedGoToTarget_targetType_objects);
	EnableWindow( GetDlgItem( hWnd, IDC_OBJECTLIST ), useObjects );
	EnableWindow( GetDlgItem( hWnd, IDC_BYLIST ), useObjects );
	but = GetICustButton( GetDlgItem( hWnd, IDC_PICKNODE ) );
	if (!useObjects)
		if (but->IsChecked()) GetCOREInterface()->PopCommandMode();
	but->Enable(useObjects);
	ReleaseICustButton(but);
	but = GetICustButton( GetDlgItem( hWnd, IDC_REMOVENODE ) );
	but->Enable(useObjects && (num > 0));
	ReleaseICustButton(but);
	EnableWindow( GetDlgItem( hWnd, IDC_ANIMATED ), targetType == kSpeedGoToTarget_targetType_objects );
	EnableWindow( GetDlgItem( hWnd, IDC_LOCKTARGET ), useObjects && (numTargets > 1) );
	EnableWindow( GetDlgItem( hWnd, IDC_OBJECTTEXT ), useObjects && (numTargets > 1) );
	EnableWindow( GetDlgItem( hWnd, IDC_ASSIGNMENT ), useObjects && (numTargets > 1) );

	bool useDocking = (type != kSpeedGoToTarget_type_none);
	bool useDockingDistance = useDocking && (dockingType != kSpeedGoToTarget_dockingType_none);
	EnableWindow( GetDlgItem( hWnd, IDC_DOCKINGFRAME ), useDocking );
	EnableWindow( GetDlgItem( hWnd, IDC_DOCKINGTYPETEXT ), useDocking );
	EnableWindow( GetDlgItem( hWnd, IDC_DIRECTION ), useDocking );
	EnableWindow( GetDlgItem( hWnd, IDC_DISTANCETEXT2 ), useDockingDistance );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DOCKINGDISTANCESPIN ) );
	spin->Enable( useDockingDistance );
	ReleaseISpinner( spin );

	bool useRandom = false;
	if (useBySpeed  && useCruiseSpeed) {
		float speedVar = pblock->GetFloat(kSpeedGoToTarget_cruiseSpeedVariation, time);
		if (speedVar == 0.0f) {
			Control* ctrl = pblock->GetController(kSpeedGoToTarget_cruiseSpeedVariation);
			if (ctrl != NULL) useRandom = (ctrl->IsAnimated() != 0);
		} else {
			useRandom = true;
		}
	}
	if (!useRandom && useByTime) {
		TimeValue timeVar = pblock->GetTimeValue(kSpeedGoToTarget_timeVariation, time);
		if (timeVar == 0) {
			Control* ctrl = pblock->GetController(kSpeedGoToTarget_timeVariation);
			if (ctrl != NULL) useRandom = (ctrl->IsAnimated() != 0);
		} else {
			useRandom = true;
		}
		if (!useRandom && useDockingSpeed) {
			float speedVar = pblock->GetFloat(kSpeedGoToTarget_dockingSpeedVariation, time);
			if (speedVar == 0.0f) {
				Control* ctrl = pblock->GetController(kSpeedGoToTarget_dockingSpeedVariation);
				if (ctrl != NULL) useRandom = (ctrl->IsAnimated() != 0);
			} else {
				useRandom = true;
			}
		}
	}
	if (!useRandom)
		useRandom = (pblock->GetInt(kSpeedGoToTarget_pointType, time) == kSpeedGoToTarget_pointType_random);
	if (!useRandom && (targetType == kSpeedGoToTarget_targetType_objects))
		useRandom = ((numTargets > 1) && (pblock->GetInt(kSpeedGoToTarget_assignment, time) == kSpeedGoToTarget_assignment_random));
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
