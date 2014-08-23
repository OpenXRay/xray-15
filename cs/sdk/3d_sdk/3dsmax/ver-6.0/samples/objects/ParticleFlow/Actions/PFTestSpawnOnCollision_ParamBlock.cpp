/**********************************************************************
 *<
	FILE:			PFTestSpawnOnCollision_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Duration Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestSpawnOnCollision_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestSpawnOnCollisionDesc ThePFTestSpawnOnCollisionDesc;

// custom update dialog process
TestSpawnOnCollisionDlgProc theTestSpawnOnCollisionDlgProc;

// Duration Test param block
ParamBlockDesc2 spawnOnCollision_paramBlockDesc 
(
	// required block spec
		kSpawnCollision_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestSpawnOnCollisionDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpawnCollision_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPAWNCOLLISION_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestSpawnOnCollisionDlgProc,
	// required param specs
		// test true for particles that collided
			kSpawnCollision_sendParentOut,	_T("True_for_Particles_Collided"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_TRUEFORPARTICLESCOLLIDED,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_PARTICLESCOLLIDED,
			end,
		// test true for new spawn particles
			kSpawnCollision_sendSpawnOut,		_T("True_for_Spawn_Particles"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_TRUEFORSPAWNPARTICLES,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SPAWNPARTICLES,
			end,
		// deflectors
			kSpawnCollision_deflectors, _T(""),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE,	0,
						p_ui,	TYPE_NODELISTBOX, IDC_DEFLECTORLIST,IDC_COLLISION_PICKNODE,0,IDC_REMOVE_COLLISION,
			end,
		// deflectors for maxscript manipulation
			kSpawnCollision_deflectorsMaxscript, _T("Collision_Nodes"),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE|P_NO_REF,	IDS_TEST_COLLISIONSPACEWARP_NODES,
			end,
		// test type: once, or per frame, or by distance
			kSpawnCollision_spawnType,		_T("Spawn_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SPAWNTYPE,
			// optional tagged param specs
					p_default,		kSpawnCollision_spawnType_once,
					p_range,		0, kSpawnCollision_spawnType_num-1,
					p_ui,			TYPE_RADIO, kSpawnCollision_spawnType_num, IDC_ONCE, IDC_EACH,
			end,
		// delete parent
			kSpawnCollision_deleteParent,	_T("Delete_Parent"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_DELETEPARENT,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_DELETEPARENT,
			end,
		// index of the last particle
			kSpawnCollision_untilNum, _T("Number_of_Collisions"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_NUMCOLLISIONS,
			// optional tagged param specs
					p_default,		3,
					p_range,		0, 1000000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_UNTIL,	IDC_UNTILSPIN,	0.5f,
			end,
		// percentage of spawn-able particles
			kSpawnCollision_spawnAble,		_T("Spawn_Able"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPAWNABLE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_AFFECTS, IDC_AFFECTSSPIN, 1.0f,
			end,
		// number of particles spawn
			kSpawnCollision_numOffsprings,	_T("Number_of_Offsprings"),
									TYPE_INT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_OFFSPRINGSNUM,
			// optional tagged param specs
					p_default,		1,
					p_range,		0, 1000000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_MULTIPLIER,	IDC_MULTIPLIERSPIN,	0.25f,
			end,
		// variation for number of particles spawn
			kSpawnCollision_numVariation,	_T("Offsprings_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_AMOUNTVARIATION, IDC_AMOUNTVARIATIONSPIN, 1.0f,
			end,
		// sync by
			kSpawnCollision_syncBy,			_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpawnCollision_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSpawnCollision_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, 
												IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpawnCollision_syncBy_time,
									kSpawnCollision_syncBy_age,
									kSpawnCollision_syncBy_event,
			end,
		// restart particle age
			kSpawnCollision_restartAge,		_T("Restart_Particle_Age"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_RESTARTAGE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_RESTARTAGE,
			end,
		// parent speed type
			kSpawnCollision_speedParent, _T("Parent_Speed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_PARENTSPEED,
			// optional tagged param specs
					p_default,		kSpawnCollision_speedType_bounce,
					p_ui,			TYPE_INTLISTBOX,	IDC_PARENTSPEED,	kSpawnCollision_speedParent_num,
												IDS_SPEEDTYPE_BOUNCE, 
												IDS_SPEEDTYPE_CONTINUE,
					p_vals,			kSpawnCollision_speedType_bounce,
									kSpawnCollision_speedType_continue,
			end,
		// offspring speed type
			kSpawnCollision_speedOffspring, _T("Offspring_Speed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_OFFSPRINGSPEED,
			// optional tagged param specs
					p_default,		kSpawnCollision_speedType_bounce,
					p_ui,			TYPE_INTLISTBOX,	IDC_CHILDSPEED,	kSpawnCollision_speedOffspring_num,
												IDS_SPEEDTYPE_BOUNCE, 
												IDS_SPEEDTYPE_CONTINUE, 
					p_vals,			kSpawnCollision_speedType_bounce,
									kSpawnCollision_speedType_continue,
			end,
		// speed type: in units, or inherited
			kSpawnCollision_speedType,		_T("Speed_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SPEEDTYPE,
			// optional tagged param specs
					p_default,		kSpawnCollision_speedType_inherited,
					p_range,		0, kSpawnCollision_speedType_num-1,
					p_ui,			TYPE_RADIO, kSpawnCollision_speedType_num, IDC_SPEEDUNITS, IDC_SPEEDINHERITED,
			end,
		// speed units
			kSpawnCollision_speedUnits,		_T("Speed"),
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
			kSpawnCollision_speedInherited,	_T("Speed_Inherited"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPEEDINHERITED,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-1000000.0f, 1000000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_INHERITED, IDC_INHERITEDSPIN, 1.0f,
			end,
		// variation for speed
			kSpawnCollision_speedVariation,	_T("Speed_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_VARIATION, IDC_VARIATIONSPIN, 1.0f,
			end,
		// speed divergence
			kSpawnCollision_speedDivergence,	_T("Divergence"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCE,
			// optional tagged param specs
					p_default,		0.2094395067f,
					p_range,		0.0f, 180.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_DIVERGENCE, IDC_DIVERGENCESPIN, 0.5f,
			end,
		// scale factor
			kSpawnCollision_scale,		_T("Scale_Factor"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SCALE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 1000000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_SCALE, IDC_SCALESPIN, 1.0f,
			end,
		// variation for scale
			kSpawnCollision_scaleVariation,	_T("Scale_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_SCALEVARIATION, IDC_SCALEVARIATIONSPIN, 1.0f,
			end,
		// random seed
			kSpawnCollision_randomSeed,		_T("Random_Seed"),
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
FPInterfaceDesc spawnOnCollision_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestSpawnOnCollisionDesc, FP_MIXIN, 
				
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

FPInterfaceDesc spawnOnCollision_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestSpawnOnCollisionDesc, FP_MIXIN,

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

FPInterfaceDesc spawnOnCollision_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestSpawnOnCollisionDesc, FP_MIXIN,

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

FPInterfaceDesc spawnOnCollision_integrator_FPInterfaceDesc(
			PFINTEGRATOR_INTERFACE, 
			_T("integrator"), 0, 
			&ThePFTestSpawnOnCollisionDesc, FP_MIXIN, 
			
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
//|					custom UI update for Duration Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
BOOL TestSpawnOnCollisionDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	IParamBlock2* pblock;
	case WM_INITDIALOG:
		pblock = map->GetParamBlock();
		UpdateSpawnTypeDlg( hWnd, pblock->GetInt(kSpawnCollision_spawnType,t),
								  pblock->GetInt(kSpawnCollision_deleteParent,t) );
		UpdateSpeedTypeDlg( hWnd, pblock->GetInt(kSpawnCollision_speedType,t) );
		UpdateRandomSeedDlg( hWnd, pblock->GetFloat(kSpawnCollision_spawnAble,t),
								   pblock->GetFloat(kSpawnCollision_numVariation,t),
								   pblock->GetFloat(kSpawnCollision_speedVariation,t),
								   pblock->GetFloat(kSpawnCollision_speedDivergence,t),
								   pblock->GetFloat(kSpawnCollision_scaleVariation,t) );						   
		break;
	case WM_COMMAND:
		pblock = map->GetParamBlock();
		switch (LOWORD(wParam))
		{
		case IDC_NEW:
			if (pblock) pblock->NotifyDependents(FOREVER, PART_OBJ, kSpawnCollision_RefMsg_NewRand);
			return TRUE;
		case IDC_COLLISION_PICKNODE:
			if (pblock) pblock->NotifyDependents( FOREVER, 0, kSpawnCollision_RefMsg_ResetValidatorAction );
			break;
		case IDC_BYLIST:
			if (pblock) pblock->NotifyDependents( FOREVER, 0, kSpawnCollision_RefMsg_ListSelect );
			break;
		case kSpawnCollision_message_type:
			pblock = map->GetParamBlock();
			UpdateSpawnTypeDlg( hWnd, pblock->GetInt(kSpawnCollision_spawnType,t),
									  pblock->GetInt(kSpawnCollision_deleteParent,t) );
			break;
		case kSpawnCollision_message_speed:
			pblock = map->GetParamBlock();
			UpdateSpeedTypeDlg( hWnd, pblock->GetInt(kSpawnCollision_speedType,t) );
			break;
		case kSpawnCollision_message_random:
			pblock = map->GetParamBlock();
			UpdateRandomSeedDlg( hWnd, pblock->GetFloat(kSpawnCollision_spawnAble,t),
									   pblock->GetFloat(kSpawnCollision_numVariation,t),
									   pblock->GetFloat(kSpawnCollision_speedVariation,t),
									   pblock->GetFloat(kSpawnCollision_speedDivergence,t),
									   pblock->GetFloat(kSpawnCollision_scaleVariation,t) );						   
			break;
		case kSpawnCollision_message_deflectors:
			pblock = map->GetParamBlock();
			if (pblock)
				UpdateDeflectorsDlg( hWnd, pblock->Count(kSpawnCollision_deflectors) );
			break;
		}
		break;
	}
	return FALSE;
}

void TestSpawnOnCollisionDlgProc::UpdateSpawnTypeDlg( HWND hWnd, int spawnType, int deleteParent )
{
	bool multType = (spawnType == kSpawnCollision_spawnType_multiple);
	bool sentParent = (multType || (deleteParent == 0));
	EnableWindow( GetDlgItem(hWnd, IDC_PARTICLESCOLLIDED), sentParent );
	EnableWindow( GetDlgItem(hWnd, IDC_DELETEPARENT), spawnType == kSpawnCollision_spawnType_once );
	EnableWindow( GetDlgItem(hWnd, IDC_UNTILTEXT), multType);
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_UNTILSPIN ) );
	spin->Enable(multType);
	ReleaseISpinner(spin);
}

void TestSpawnOnCollisionDlgProc::UpdateSpeedTypeDlg( HWND hWnd, int speedType )
{
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SPEEDSPIN ) );
	spin->Enable(speedType == kSpawnCollision_speedType_units);
	ReleaseISpinner(spin);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_INHERITEDSPIN ) );
	spin->Enable(speedType == kSpawnCollision_speedType_inherited);
	ReleaseISpinner(spin);
}

void TestSpawnOnCollisionDlgProc::UpdateRandomSeedDlg( HWND hWnd, float affects, float multVar, float speedVar, float divergence, float scaleVar )
{
	bool hasChaos = !(((affects==0.0f) || (affects==1.0f)) && (multVar==0.0f) && (speedVar==0.0f) && (divergence==0.0f) && (scaleVar==0.0f));
	EnableWindow( GetDlgItem(hWnd, IDC_UNIQUENESSBOX), hasChaos );
	EnableWindow( GetDlgItem(hWnd, IDC_SEEDTEXT), hasChaos );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
	spin->Enable(hasChaos);
	ReleaseISpinner(spin);
	ICustButton* but = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
	but->Enable(hasChaos);
	ReleaseICustButton(but);
}

void TestSpawnOnCollisionDlgProc::UpdateDeflectorsDlg( HWND hWnd, int numDeflectors )
{
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_REMOVE_COLLISION ) );
	button->Enable( numDeflectors > 0 );
	ReleaseICustButton( button );
}





} // end of namespace PFActions
