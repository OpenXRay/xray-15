/**********************************************************************
 *<
	FILE:			PFTestCollisionSpaceWarp_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for CollisionSpaceWarp Test (definition)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 2-07-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestCollisionSpaceWarp_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"
#include "PFActions_GlobalFunctions.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestCollisionSpaceWarpDesc ThePFTestCollisionSpaceWarpDesc;

// custom update dialog process
TestCollisionSpaceWarpDlgProc theTestCollisionSpaceWarpDlgProc;

// CollisionSpaceWarp Test param block
ParamBlockDesc2 collision_paramBlockDesc 
(
	// required block spec
		kCollisionSpaceWarp_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestCollisionSpaceWarpDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kCollisionSpaceWarp_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_COLLISIONSPACEWARP_PARAMETERS,

		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestCollisionSpaceWarpDlgProc,



		// deflectors
		kCollisionSpaceWarp_CollisionNodelist, _T(""),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE,	0,
						p_ui,	TYPE_NODELISTBOX, IDC_DEFLECTORLIST,IDC_COLLISION_PICKNODE,0,IDC_REMOVE_COLLISION,
			end,
		// deflectors for maxscript manipulation
		kCollisionSpaceWarp_CollisionsMaxscript, _T("Collision_Nodes"),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE|P_NO_REF,	IDS_TEST_COLLISIONSPACEWARP_NODES,
			end,

		kCollisionSpaceWarp_CollisionTestType,  _T("Test_Type"),
						TYPE_INT, P_RESET_DEFAULT, 	IDS_PW_COLLISIONTESTTYPE, 
						p_default, 		0,	
						p_range, 		0, 4, 
						p_ui, 			TYPE_RADIO, 5,IDC_ONCE,IDC_SLOW,IDC_FAST,IDC_MULTIPLE,IDC_WILL,
						end, 

		kCollisionSpaceWarp_CollisionSpeedOption,  _T("Speed_Option"),
						TYPE_INT, 	
						P_RESET_DEFAULT, 	IDS_PW_COLLISIONSPEEDOPTION, 
			// optional tagged param specs
						p_default,		kCollisionSpaceWarp_SpeedBounce,
						p_ui,			TYPE_INTLISTBOX,	IDC_ONCESPEED,	4,
												IDS_SPEEDTYPE_BOUNCE, 
												IDS_SPEEDTYPE_CONTINUE, 
												IDS_SPEEDTYPE_STOP, 
												IDS_SPEEDTYPE_RANDOM, 
						p_vals,		kCollisionSpaceWarp_SpeedBounce,
									kCollisionSpaceWarp_SpeedContinue,
									kCollisionSpaceWarp_SpeedStop,
									kCollisionSpaceWarp_SpeedRandom,
						end,

		kCollisionSpaceWarp_CollisionMinSpeed,	_T("Min_Speed"),
						TYPE_WORLD,	P_RESET_DEFAULT, IDS_PW_MINSPEED,
			// optional tagged param specs
						p_default,		1.0f, 
						p_ms_default,	1.0f, // 1 sec
						p_range,		0.0f, 999999999.0f,
						p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE,	IDC_SLOWSPEED,	IDC_SLOWSPEEDSPIN,	0.1f,
						end,

		kCollisionSpaceWarp_CollisionMaxSpeed,	_T("Max_Speed"),
						TYPE_WORLD,	P_RESET_DEFAULT, IDS_PW_MAXSPEED,
			// optional tagged param specs
						p_default,		1000.0f, 
						p_ms_default,	1000.0f, 
						p_range,		0.0f, 999999999.0f,
						p_ui,			TYPE_SPINNER,	EDITTYPE_UNIVERSE,	IDC_FASTSPEED,	IDC_FASTSPEEDSPIN,	0.1f,
						end,



		// random seed
			kCollisionSpaceWarp_randomSeed,	_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_ms_default,	12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED2,	IDC_SEEDSPIN2,	1.0f,
			end,
			kCollisionSpaceWarp_CollisionWillCollideFrame,	_T("Will_Collide_Frame"),
						TYPE_INT,	P_RESET_DEFAULT, IDS_PW_WILLCOLLIDEFRAME,
			// optional tagged param specs
						p_default,		800, 
						p_ms_default,	800, 
						p_range,		0, 999999999,
						p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_NUMFRAMES,	IDC_NUMFRAMESSPIN,	80.0f,
						end,

		kCollisionSpaceWarp_CollisionCount,	_T("Collision_Count"),
						TYPE_INT,	P_RESET_DEFAULT, IDS_PW_COLLISIONCOUNT,
			// optional tagged param specs
						p_default,		5, 
						p_ms_default,	5, 
						p_range,		0, 999999999,
						p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_NUMTIMES,	IDC_NUMTIMESSPIN,	1.0f,
						end,

		kCollisionSpaceWarp_CollisionCountOptions,  _T("Collision_Count_Options"),
						TYPE_INT, 	
						P_RESET_DEFAULT, 	IDS_PW_COLLISIONCOUNTOPTIONS, 
			// optional tagged param specs
						p_default,		kCollisionSpaceWarp_SpeedBounce,
						p_ui,			TYPE_INTLISTBOX,	IDC_MULTISPEED,	4,
												IDS_SPEEDTYPE_BOUNCE, 
												IDS_SPEEDTYPE_CONTINUE, 
												IDS_SPEEDTYPE_STOP, 
												IDS_SPEEDTYPE_RANDOM, 
						p_vals,		kCollisionSpaceWarp_SpeedBounce,
									kCollisionSpaceWarp_SpeedContinue,
									kCollisionSpaceWarp_SpeedStop,
									kCollisionSpaceWarp_SpeedRandom,
						end,


		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc collision_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestCollisionSpaceWarpDesc, FP_MIXIN, 
				
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
			IPFAction::kActivityInterval, _T("activityInterval"), 0, TYPE_INTERVAL, 0, 0,
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

FPInterfaceDesc collision_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestCollisionSpaceWarpDesc, FP_MIXIN,

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

FPInterfaceDesc collision_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestCollisionSpaceWarpDesc, FP_MIXIN,

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

FPInterfaceDesc collision_integrator_FPInterfaceDesc(
			PFINTEGRATOR_INTERFACE, 
			_T("integrator"), 0, 
			&ThePFTestCollisionSpaceWarpDesc, FP_MIXIN, 
			
			kPFIntegrator_proceedSync, _T("proceedSync"), 0, TYPE_bool, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("timeTick"), 0, TYPE_TIMEVALUE,
				_T("tickFraction"), 0, TYPE_FLOAT,
				_T("selectedOnly"), 0, TYPE_bool,
				_T("selected"), 0, TYPE_BITARRAY,
			kPFIntegrator_proceedASync, _T("proceedASync"), 0, TYPE_bool, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("timeTick"), 0, TYPE_TIMEVALUE_TAB,
				_T("tickFraction"), 0, TYPE_FLOAT_TAB,
				_T("selectedOnly"), 0, TYPE_bool,
				_T("selected"), 0, TYPE_BITARRAY,

		end
); 

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|					custom UI update for CollisionSpaceWarp Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestCollisionSpaceWarpDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;

	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		pblock = map->GetParamBlock();
		if (pblock)
			pblock->NotifyDependents( FOREVER, (PartID)map, kCollisionSpaceWarp_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = map->GetParamBlock();
		switch (LOWORD(wParam))
		{
		case IDC_NEW2:
			if (pblock) pblock->NotifyDependents( FOREVER, PART_OBJ, kCollisionSpaceWarp_RefMsg_NewRand );
			return TRUE;
		case IDC_COLLISION_PICKNODE:
			if (pblock) pblock->NotifyDependents( FOREVER, 0, kCollisionSpaceWarp_RefMsg_ResetValidatorAction );
			break;
		case IDC_BYLIST:
			if (pblock) pblock->NotifyDependents( FOREVER, PART_OBJ, kCollisionSpaceWarp_RefMsg_ListSelect );
			break;
		case kCollisionSpaceWarp_message_type:
			if (pblock)
				UpdateTypeDlg( hWnd, pblock->GetInt(kCollisionSpaceWarp_CollisionTestType,t),
											pblock->GetInt(kCollisionSpaceWarp_CollisionSpeedOption,t),
											pblock->GetInt(kCollisionSpaceWarp_CollisionCountOptions,t) );
			break;
		case kCollisionSpaceWarp_message_deflectors:
			if (pblock)
				UpdateDeflectorNumDlg( hWnd, pblock->Count(kCollisionSpaceWarp_CollisionNodelist) );
			break;
		}
		break;
	}
	return FALSE;
}

void TestCollisionSpaceWarpDlgProc::UpdateTypeDlg( HWND hWnd, int type, int singleType, int multipleType )
{
	ISpinnerControl* spin;

	bool enable = (type == 0); // single collide
	EnableWindow( GetDlgItem(hWnd, IDC_ONCESPEEDTEXT), enable );
	EnableWindow( GetDlgItem(hWnd, IDC_ONCESPEED), enable );

	enable = (type == 1); // min speed
	EnableWindow( GetDlgItem(hWnd, IDC_SLOWSPEEDTEXT), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SLOWSPEEDSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner(spin);

	enable = (type == 2); // max speed
	EnableWindow( GetDlgItem(hWnd, IDC_FASTSPEEDTEXT), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_FASTSPEEDSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner(spin);

	enable = (type == 3); // multiple collide
	EnableWindow( GetDlgItem(hWnd, IDC_NUMTIMESTEXT), enable );
	EnableWindow( GetDlgItem(hWnd, IDC_MULTISPEEDTEXT), enable );
	EnableWindow( GetDlgItem(hWnd, IDC_MULTISPEED), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_NUMTIMESSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner(spin);

	enable = (type == 4); // will collide
	EnableWindow( GetDlgItem(hWnd, IDC_NUMFRAMESTEXT), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_NUMFRAMESSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner(spin);

	bool useRandom = (((type == 0) && (singleType == kCollisionSpaceWarp_SpeedRandom)) 
						|| ((type == 3) && (multipleType == kCollisionSpaceWarp_SpeedRandom)) );
	EnableWindow( GetDlgItem( hWnd, IDC_UNIQUENESSBOX2 ), useRandom );
	EnableWindow( GetDlgItem( hWnd, IDC_SEEDTEXT2 ), useRandom );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable( useRandom );
	ReleaseISpinner( spin );
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_NEW2 ) );
	button->Enable( useRandom );
	ReleaseICustButton( button );
}

void TestCollisionSpaceWarpDlgProc::UpdateDeflectorNumDlg( HWND hWnd, int num )
{
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_REMOVE_COLLISION ) );
	button->Enable( num > 0 );
	ReleaseICustButton( button );
}



} // end of namespace PFActions
