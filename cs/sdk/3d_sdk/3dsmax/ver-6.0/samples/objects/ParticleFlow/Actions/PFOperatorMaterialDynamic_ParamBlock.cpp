/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamic_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for MaterialDynamic Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorMaterialDynamic_ParamBlock.h"

#include "PFOperatorMaterialDynamic.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorMaterialDynamicDesc		ThePFOperatorMaterialDynamicDesc;
// Dialog Proc
PFOperatorMaterialDynamicDlgProc		ThePFOperatorMaterialDynamicDlgProc;

// MaterialDynamic Operator param block
ParamBlockDesc2 materialDynamic_paramBlockDesc 
(
	// required block spec
		kMaterialDynamic_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorMaterialDynamicDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kMaterialDynamic_reference_pblock,
	// auto ui parammap specs : none
		IDD_MATERIALDYNAMIC_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorMaterialDynamicDlgProc,
	// required param specs
		// assign materialDynamic
			kMaterialDynamic_assignMaterial, _T("Assign_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNMATERIAL,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNMATERIAL,
			end,
		// materialDynamic
			kMaterialDynamic_material,	_T("Assigned_Material"),
									TYPE_MTL,
									P_RESET_DEFAULT|P_SHORT_LABELS|P_NO_REF,
									IDS_ASSIGNEDMATERIAL,
			// optional tagged param specs
					p_ui,			TYPE_MTLBUTTON,	IDC_MATERIAL,
			end,
		// assign materialDynamic ID
			kMaterialDynamic_assignID, _T("Assign_Material_ID"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNMATERIALID,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNID,
			end,
		// show mtl IDs in viewport
			kMaterialDynamic_showInViewport, _T("Show_In_Viewport"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWINVIEWPORT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SHOWINVIEWPORT,
			end,
		// materialDynamic ID animation type
			kMaterialDynamic_type,	_T("Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_TYPE,
			// optional tagged param specs
					p_default,		kMaterialDynamic_type_particleID,
					p_range,		0, kMaterialDynamic_type_num-1,
					p_ui,			TYPE_RADIO, kMaterialDynamic_type_num, IDC_TYPE_PARTID, IDC_TYPE_MTLID, IDC_TYPE_CYCLE, IDC_TYPE_RANDOM,
			end,
		// reset particle age
			kMaterialDynamic_resetAge, _T("Reset_Particle_Age"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_RESETPARTICLEAGE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_RESETAGE,
			end,
		// randomize particle age
			kMaterialDynamic_randomizeAge, _T("Randomize_Age"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_RANDOMIZEAGE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_RANDOMIZEAGE,
			end,
		// maximum age offset
			kMaterialDynamic_maxAgeOffset, _T("Max_Age_Offset"),
									TYPE_TIMEVALUE,
									P_RESET_DEFAULT,
									IDS_MAXAGEOFFSET,
			// optional tagged param specs
					p_default,		4800, // 1 sec
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_MAXOFFSET,	IDC_MAXOFFSETSPIN,	1.0f,
			end,
		// material ID
			kMaterialDynamic_materialID, _T("Material_ID"),
									TYPE_INT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MATERIALID,
			// optional tagged param specs
					p_default,		1,
					p_range,		1, 65535,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_MATERIALID,	IDC_MATERIALIDSPIN,	1.0f,
			end,
		// number of sub-materialDynamics
			kMaterialDynamic_numSubMtls, _T("Number_of_Sub_Materials"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_NUMSUBMTLS,
			// optional tagged param specs
					p_default,		0,
					p_range,		0, 65535,
					p_ui,			TYPE_SPINNER,	EDITTYPE_INT,	IDC_NUMSUBS,	IDC_NUMSUBSSPIN,	1.0f,
			end,
		// number of sub-materialDynamics per frame
			kMaterialDynamic_rate, _T("Sub_Materials_Rate"),
									TYPE_FLOAT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_SUBMTLRATE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		kMaterialDynamic_minRate, kMaterialDynamic_maxRate,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_RATE, IDC_RATESPIN, SPIN_AUTOSCALE,
			end,
		// loop frames
			kMaterialDynamic_loop,		_T("Loop"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_LOOP,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_LOOP,
			end,
		// sync type
			kMaterialDynamic_sync, _T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kMaterialDynamic_sync_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kMaterialDynamic_sync_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kMaterialDynamic_sync_time,
									kMaterialDynamic_sync_age,
									kMaterialDynamic_sync_event,
			end,
		// add random offset to the synchronization
			kMaterialDynamic_randomizeRotoscoping, _T("Add_Random_Offset"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ADDRANDOMOFFSET,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNCRANDOM,
			end,
		// random offset
			kMaterialDynamic_maxRandOffset, _T("Random_Offset"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMOFFSET,
			// optional tagged param specs
					p_default,		4800, // 1 sec
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_SYNCOFFSET,	IDC_SYNCOFFSETSPIN,	80.0f,
			end,
		// random seed
			kMaterialDynamic_randomSeed, _T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345, // initial random seed
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc materialDynamic_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorMaterialDynamicDesc, FP_MIXIN, 
				
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
			IPFAction::kGetMaterial, _T("getMaterial"), 0, TYPE_MTL, 0, 0,
			IPFAction::kSetMaterial, _T("setMaterial"), 0, TYPE_bool, 0, 1,
				_T("material"), 0, TYPE_MTL,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc materialDynamic_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorMaterialDynamicDesc, FP_MIXIN,

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

FPInterfaceDesc materialDynamic_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorMaterialDynamicDesc, FP_MIXIN,

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


//+--------------------------------------------------------------------------+
//|							Dialog Process									 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorMaterialDynamicDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
//	BOOL alignTo, angleDistortion;
	IParamBlock2* pblock;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kMaterialDynamic_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kMaterialDynamic_RefMsg_NewRand );
			break;
		case kMaterialDynamic_message_assignMaterial:
			pblock = map->GetParamBlock();
			if (pblock != NULL)
				UpdateAssignMaterialDlg( hWnd, pblock->GetInt(kMaterialDynamic_assignMaterial, t));
			break;
		case kMaterialDynamic_message_assignID:
			pblock = map->GetParamBlock();
			if (pblock != NULL)
				UpdateAssignIDDlg( hWnd, pblock->GetInt(kMaterialDynamic_assignID, t),
										pblock->GetInt(kMaterialDynamic_type, t),
										pblock->GetInt(kMaterialDynamic_randomizeAge, t),
										pblock->GetInt(kMaterialDynamic_randomizeRotoscoping, t) );
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorMaterialDynamicDlgProc::UpdateAssignMaterialDlg( HWND hWnd, int assign )
{
	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_MATERIAL));
	but->Enable( assign != 0);
	ReleaseICustButton(but);
}

void PFOperatorMaterialDynamicDlgProc::UpdateAssignIDDlg( HWND hWnd, int assign, int type, int randAge, int randOffset)
{
	ISpinnerControl *spin;
	bool enableAssign = (assign != 0);
	
	EnableWindow( GetDlgItem( hWnd, IDC_SHOWINVIEWPORT), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_ANIMATEDFRAME), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_TYPE_PARTID), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_SUBMTLSFRAME), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_TYPE_MTLID), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_TYPE_CYCLE), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_TYPE_RANDOM), enableAssign);

	bool enableAnimated = (enableAssign && (type == kMaterialDynamic_type_particleID));
	bool enableMaxOffset = (enableAnimated && (randAge != 0));
	EnableWindow( GetDlgItem( hWnd, IDC_RESETAGE), enableAnimated);
	EnableWindow( GetDlgItem( hWnd, IDC_RANDOMIZEAGE), enableAnimated);
	EnableWindow( GetDlgItem( hWnd, IDC_MAXOFFSETTEXT), enableMaxOffset);
	spin = GetISpinner(GetDlgItem(hWnd, IDC_MAXOFFSETSPIN));
	spin->Enable( enableMaxOffset );
	ReleaseISpinner( spin );

	bool enableMtlID = (enableAssign && (type == kMaterialDynamic_type_materialID));
	spin = GetISpinner(GetDlgItem(hWnd, IDC_MATERIALIDSPIN));
	spin->Enable( enableMtlID );
	ReleaseISpinner( spin );
	
	bool enableRoto = (enableAssign && (type >= kMaterialDynamic_type_cycle));
	EnableWindow( GetDlgItem( hWnd, IDC_NUMSUBSTEXT), enableRoto);
	EnableWindow( GetDlgItem( hWnd, IDC_RATETEXT), enableRoto);
	spin = GetISpinner(GetDlgItem(hWnd, IDC_NUMSUBSSPIN));
	spin->Enable( enableRoto );
	ReleaseISpinner( spin );
	spin = GetISpinner(GetDlgItem(hWnd, IDC_RATESPIN));
	spin->Enable( enableRoto );
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_LOOP), enableAssign && (type == kMaterialDynamic_type_cycle));

	bool enableSync = (enableAssign && ((type == kMaterialDynamic_type_materialID)
									|| ( type == kMaterialDynamic_type_cycle)) );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCBYTEXT), enableSync);
	EnableWindow( GetDlgItem( hWnd, IDC_SYNC), enableSync);
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCRANDOM), enableSync);
	spin = GetISpinner(GetDlgItem(hWnd, IDC_SYNCOFFSETSPIN));
	spin->Enable( enableSync && (randOffset != 0) );
	ReleaseISpinner( spin );
}

} // end of namespace PFActions