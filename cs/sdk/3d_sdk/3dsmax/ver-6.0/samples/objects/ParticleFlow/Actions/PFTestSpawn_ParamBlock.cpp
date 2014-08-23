/**********************************************************************
 *<
	FILE:			PFTestSpawn_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Duration Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestSpawn_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestSpawnDesc ThePFTestSpawnDesc;

// custom update dialog process
TestSpawnDlgProc theTestSpawnDlgProc;

// Spawn Test param block
ParamBlockDesc2 spawn_paramBlockDesc 
(
	// required block spec
		kSpawn_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestSpawnDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpawn_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPAWN_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestSpawnDlgProc,
	// required param specs
		// test type: once, or per frame, or by distance
			kSpawn_spawnType,		_T("Spawn_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SPAWNTYPE,
			// optional tagged param specs
					p_default,		kSpawn_spawnType_once,
					p_range,		0, kSpawn_spawnType_num-1,
					p_ui,			TYPE_RADIO, kSpawn_spawnType_num, IDC_ONCE, IDC_PERFRAME, IDC_BYTRAVEL,
			end,
		// delete parent
			kSpawn_deleteParent,	_T("Delete_Parent"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_DELETEPARENT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_DELETEPARENT,
			end,
		// index of the last particle
			kSpawn_spawnRate,		_T("Spawn_Rate"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPAWNRATE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		kSpawn_frameRate_min, kSpawn_frameRate_max,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	IDC_PERFRAMENUM,	IDC_PERFRAMENUMSPIN,	0.25f,
			end,
		// step size
			kSpawn_stepSize,		_T("Spawn_Step_Size"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPAWNSTEPSIZE,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		kSpawn_stepSize_min, kSpawn_stepSize_max,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_STEPSIZE, IDC_STEPSIZESPIN, SPIN_AUTOSCALE,
			end,
		// percentage of spawn-able particles
			kSpawn_spawnAble,		_T("Spawn_Able"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPAWNABLE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_AFFECTS, IDC_AFFECTSSPIN, 1.0f,
			end,
		// number of particles spawn
			kSpawn_numOffsprings,	_T("Number_of_Offsprings"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_OFFSPRINGSNUM,
			// optional tagged param specs
					p_default,		1,
					p_range,		1, 1000000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_MULTIPLIER,	IDC_MULTIPLIERSPIN,	0.25f,
			end,
		// variation for number of particles spawn
			kSpawn_numVariation,	_T("Offsprings_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_AMOUNTVARIATION, IDC_AMOUNTVARIATIONSPIN, 1.0f,
			end,
		// sync by
			kSpawn_syncBy,			_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpawn_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSpawn_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, 
												IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpawn_syncBy_time,
									kSpawn_syncBy_age,
									kSpawn_syncBy_event,
			end,
		// restart particle age
			kSpawn_restartAge,		_T("Restart_Particle_Age"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_RESTARTAGE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_RESTARTAGE,
			end,
		// speed type: in units, or inherited
			kSpawn_speedType,		_T("Speed_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SPEEDTYPE,
			// optional tagged param specs
					p_default,		kSpawn_speedType_inherited,
					p_range,		0, kSpawn_speedType_num-1,
					p_ui,			TYPE_RADIO, kSpawn_speedType_num, IDC_SPEEDUNITS, IDC_SPEEDINHERITED,
			end,
		// speed units
			kSpawn_speedUnits,		_T("Speed"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPEED,
			// optional tagged param specs
					p_default,		100.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_SPEED, IDC_SPEEDSPIN, SPIN_AUTOSCALE,
			end,
		// speed inherited
			kSpawn_speedInherited,	_T("Speed_Inherited"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPEEDINHERITED,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-1000000.0f, 1000000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_INHERITED, IDC_INHERITEDSPIN, 1.0f,
			end,
		// variation for speed
			kSpawn_speedVariation,	_T("Speed_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_VARIATION, IDC_VARIATIONSPIN, 1.0f,
			end,
		// speed divergence
			kSpawn_speedDivergence,	_T("Divergence"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCE,
			// optional tagged param specs
					p_default,		0.34906585f,
					p_range,		0.0f, 180.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,
									IDC_DIVERGENCE, IDC_DIVERGENCESPIN, 0.5f,
			end,
		// scale factor
			kSpawn_scale,		_T("Scale_Factor"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SCALE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 1000000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_SCALE, IDC_SCALESPIN, 1.0f,
			end,
		// variation for scale
			kSpawn_scaleVariation,	_T("Scale_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_SCALEVARIATION, IDC_SCALEVARIATIONSPIN, 1.0f,
			end,
		// random seed
			kSpawn_randomSeed,		_T("Random_Seed"),
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
FPInterfaceDesc spawn_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestSpawnDesc, FP_MIXIN, 
				
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

FPInterfaceDesc spawn_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestSpawnDesc, FP_MIXIN,

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

FPInterfaceDesc spawn_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestSpawnDesc, FP_MIXIN,

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
BOOL TestSpawnDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSpawn_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kSpawn_RefMsg_NewRand );
			break;
		case kSpawn_message_speedType:
			UpdateSpeedTypeDlg( hWnd, map->GetParamBlock()->GetInt(kSpawn_speedType, t) );
			break;
		case kSpawn_message_spawnType:
			UpdateSpawnTypeDlg( hWnd, map->GetParamBlock()->GetInt(kSpawn_spawnType, t) );
			break;
		case kSpawn_message_useRandom: {
			IParamBlock2* pblock = map->GetParamBlock(); DbgAssert(pblock);
			UpdateUseRandomDlg( hWnd, (pblock->GetFloat(kSpawn_numVariation, t) != 0.0f)
					|| (pblock->GetFloat(kSpawn_speedVariation, t) != 0.0f)
					|| (pblock->GetFloat(kSpawn_speedDivergence, t) != 0.0f) ); }
			break;
		}
	}
	return FALSE;
}

void TestSpawnDlgProc::UpdateSpeedTypeDlg( HWND hWnd, int speedType )
{
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SPEEDSPIN ) );
	spin->Enable( speedType == kSpawn_speedType_units );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_INHERITEDSPIN ) );
	spin->Enable( speedType == kSpawn_speedType_inherited );
	ReleaseISpinner( spin );
}

void TestSpawnDlgProc::UpdateSpawnTypeDlg( HWND hWnd, int spawnType )
{
	EnableWindow( GetDlgItem( hWnd, IDC_DELETEPARENT ), spawnType == kSpawn_spawnType_once );
	EnableWindow( GetDlgItem( hWnd, IDC_PERFRAMETEXT ), spawnType == kSpawn_spawnType_perFrame );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_PERFRAMENUMSPIN ) );
	spin->Enable( spawnType == kSpawn_spawnType_perFrame );
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_STEPSIZETEXT ), spawnType == kSpawn_spawnType_byDistance );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_STEPSIZESPIN ) );
	spin->Enable( spawnType == kSpawn_spawnType_byDistance );
	ReleaseISpinner( spin );
}

void TestSpawnDlgProc::UpdateUseRandomDlg( HWND hWnd, int useRandom )
{
	EnableWindow( GetDlgItem( hWnd, IDC_UNIQUENESSBOX ), useRandom );
	EnableWindow( GetDlgItem( hWnd, IDC_SEEDTEXT ), useRandom );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable( useRandom );
	ReleaseISpinner( spin );
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
	button->Enable( useRandom );
	ReleaseICustButton( button );
}






} // end of namespace PFActions
