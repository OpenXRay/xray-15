/**********************************************************************
 *<
	FILE:			PFOperatorMaterialFrequency_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for MaterialFrequency Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorMaterialFrequency_ParamBlock.h"

#include "PFOperatorMaterialFrequency.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorMaterialFrequencyDesc		ThePFOperatorMaterialFrequencyDesc;
// Dialog Proc
PFOperatorMaterialFrequencyDlgProc		ThePFOperatorMaterialFrequencyDlgProc;

// MaterialFrequency Operator param block
ParamBlockDesc2 materialFrequency_paramBlockDesc 
(
	// required block spec
		kMaterialFrequency_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorMaterialFrequencyDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kMaterialFrequency_reference_pblock,
	// auto ui parammap specs : none
		IDD_MATERIALFREQUENCY_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorMaterialFrequencyDlgProc,
	// required param specs
		// assign materialFrequency
			kMaterialFrequency_assignMaterial, _T("Assign_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNMATERIAL,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNMATERIAL,
			end,
		// materialFrequency
			kMaterialFrequency_material,	_T("Assigned_Material"),
									TYPE_MTL,
									P_RESET_DEFAULT|P_SHORT_LABELS|P_NO_REF,
									IDS_ASSIGNEDMATERIAL,
			// optional tagged param specs
					p_ui,			TYPE_MTLBUTTON,	IDC_MATERIAL,
			end,
		// assign materialFrequency ID
			kMaterialFrequency_assignID, _T("Assign_Material_ID"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ASSIGNMATERIALID,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ASSIGNID,
			end,
		// show mtl IDs in viewport
			kMaterialFrequency_showInViewport, _T("Show_In_Viewport"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWINVIEWPORT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SHOWINVIEWPORT,
			end,
		// materialFrequency ID #1 frequency
			kMaterialFrequency_id1, _T("Mtl_ID_1"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID1FREQ,
			// optional tagged param specs
					p_default,		0.5f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID1, IDC_ID1SPIN, 1.0f,
			end,
		// materialFrequency ID #2 frequency
			kMaterialFrequency_id2, _T("Mtl_ID_2"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID2FREQ,
			// optional tagged param specs
					p_default,		0.5f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID2, IDC_ID2SPIN, 1.0f,
			end,
		// materialFrequency ID #3 frequency
			kMaterialFrequency_id3, _T("Mtl_ID_3"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID3FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID3, IDC_ID3SPIN, 1.0f,
			end,
		// materialFrequency ID #4 frequency
			kMaterialFrequency_id4, _T("Mtl_ID_4"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID4FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID4, IDC_ID4SPIN, 1.0f,
			end,
		// materialFrequency ID #5 frequency
			kMaterialFrequency_id5, _T("Mtl_ID_5"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID5FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID5, IDC_ID5SPIN, 1.0f,
			end,
		// materialFrequency ID #6 frequency
			kMaterialFrequency_id6, _T("Mtl_ID_6"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID6FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID6, IDC_ID6SPIN, 1.0f,
			end,
		// materialFrequency ID #7 frequency
			kMaterialFrequency_id7, _T("Mtl_ID_7"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID7FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID7, IDC_ID7SPIN, 1.0f,
			end,
		// materialFrequency ID #8 frequency
			kMaterialFrequency_id8, _T("Mtl_ID_8"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID8FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID8, IDC_ID8SPIN, 1.0f,
			end,
		// materialFrequency ID #9 frequency
			kMaterialFrequency_id9, _T("Mtl_ID_9"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID9FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID9, IDC_ID9SPIN, 1.0f,
			end,
		// materialFrequency ID #8 frequency
			kMaterialFrequency_id10, _T("Mtl_ID_10"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MTLID10FREQ,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_ID10, IDC_ID10SPIN, 1.0f,
			end,
		// random seed
			kMaterialFrequency_randomSeed, _T("Random_Seed"),
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
FPInterfaceDesc materialFrequency_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorMaterialFrequencyDesc, FP_MIXIN, 
				
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

FPInterfaceDesc materialFrequency_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorMaterialFrequencyDesc, FP_MIXIN,

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

FPInterfaceDesc materialFrequency_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorMaterialFrequencyDesc, FP_MIXIN,

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

BOOL PFOperatorMaterialFrequencyDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
//	BOOL alignTo, angleDistortion;
	IParamBlock2* pblock;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kMaterialFrequency_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kMaterialFrequency_RefMsg_NewRand );
			break;
		case kMaterialFrequency_message_assignMaterial:
			pblock = map->GetParamBlock();
			if (pblock != NULL)
				UpdateAssignMaterialDlg( hWnd, pblock->GetInt(kMaterialFrequency_assignMaterial, t),
												pblock->GetMtl(kMaterialFrequency_material, t) );
			break;
		case kMaterialFrequency_message_numSubMtls:
			pblock = map->GetParamBlock();
			if (pblock != NULL)
				UpdateNumSubMtlsDlg( hWnd, pblock->GetMtl(kMaterialFrequency_material, t) );
			break;
		case kMaterialFrequency_message_assignID:
			pblock = map->GetParamBlock();
			if (pblock != NULL)
				UpdateAssignIDDlg( hWnd, pblock->GetInt(kMaterialFrequency_assignID, t));
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorMaterialFrequencyDlgProc::UpdateAssignMaterialDlg( HWND hWnd, int assign, Mtl *mtl )
{
	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_MATERIAL));
	but->Enable( assign != 0);
	ReleaseICustButton(but);
	
	int numSubs = (mtl != NULL) ? mtl->NumSubMtls() : 0;
	TCHAR buf[32];
	sprintf(buf, "%d", numSubs);
	SetWindowText( GetDlgItem( hWnd, IDC_NUMSUBS ), buf );
}

void PFOperatorMaterialFrequencyDlgProc::UpdateNumSubMtlsDlg( HWND hWnd, Mtl *mtl )
{
	int numSubs = (mtl != NULL) ? mtl->NumSubMtls() : 0;
	TCHAR buf[32];
	sprintf(buf, "%d", numSubs);
	SetWindowText( GetDlgItem( hWnd, IDC_NUMSUBS ), buf );
}

void PFOperatorMaterialFrequencyDlgProc::UpdateAssignIDDlg( HWND hWnd, int assign)
{
	ISpinnerControl *spin;
	bool enableAssign = (assign != 0);
	
	EnableWindow( GetDlgItem( hWnd, IDC_SHOWINVIEWPORT), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_NUMSUBSTEXT), enableAssign);
	EnableWindow( GetDlgItem( hWnd, IDC_NUMSUBS), enableAssign);
	int textIDC[] = { IDC_ID1TEXT, IDC_ID2TEXT, IDC_ID3TEXT, IDC_ID4TEXT, IDC_ID5TEXT, IDC_ID6TEXT, IDC_ID7TEXT, IDC_ID8TEXT, IDC_ID9TEXT, IDC_ID10TEXT };
	int spinIDC[] = { IDC_ID1SPIN, IDC_ID2SPIN, IDC_ID3SPIN, IDC_ID4SPIN, IDC_ID5SPIN, IDC_ID6SPIN, IDC_ID7SPIN, IDC_ID8SPIN, IDC_ID9SPIN, IDC_ID10SPIN };
	for(int i=0; i<10; i++) {
		EnableWindow( GetDlgItem( hWnd, textIDC[i]), enableAssign);
		spin = GetISpinner( GetDlgItem(hWnd, spinIDC[i]) );
		spin->Enable( enableAssign );
		ReleaseISpinner( spin );
	}
}



} // end of namespace PFActions