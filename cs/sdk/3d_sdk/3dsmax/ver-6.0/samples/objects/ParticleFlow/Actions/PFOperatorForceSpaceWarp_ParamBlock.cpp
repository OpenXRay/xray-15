/**********************************************************************
 *<
	FILE:			PFOperatorForceSpaceWarp_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Force Space Warp Operator (definition)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 02-06-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorForceSpaceWarp_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorForceSpaceWarpDesc ThePFOperatorForceSpaceWarpDesc;

// Dialog Proc
PFOperatorForceSpaceWarpDlgProc ThePFOperatorForceSpaceWarpDlgProc;

// ForceSpaceWarp Operator param block
ParamBlockDesc2 forceSpaceWarp_paramBlockDesc 
(
	// required block spec
		kForceSpaceWarp_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorForceSpaceWarpDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kForceSpaceWarp_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_FORCESPACEWARP_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorForceSpaceWarpDlgProc,
	// required param specs
		// force node
			// not used now (o.bayboro 09-12-2002)
			kForceSpaceWarp_ForceNode,	_T(""), 	//_T("Force_Node"),
						TYPE_INODE,			
						0,	IDS_NOTHING,  //IDS_PW_FORCENODE,
//						p_ui,			TYPE_PICKNODEBUTTON, IDC_PICK,
						end,


		// variation
			kForceSpaceWarp_Influence,	_T("Influence"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_PW_INFLUENCE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_INFLUENCE, IDC_INFLUENCESPIN, 1.0f,
			end,

		// direction
			kForceSpaceWarp_Sync,	_T("Sync"),
									TYPE_INT,
									0,
									IDS_PW_SYNC,
			// optional tagged param specs
					p_default,		kAbsoluteTime,
					p_ui,			TYPE_INTLISTBOX, IDC_SYNC, kEventDuration + 1,
										IDS_PW_ABSOLUTE, IDS_PW_PARTICLEAGE, IDS_PW_EVENTDURATION,
					p_vals,			kAbsoluteTime, kParticleAge, kEventDuration,
			end,

			// force space warps
			kForceSpaceWarp_ForceNodeList,		_T(""),
						TYPE_INODE_TAB,		0,	P_VARIABLE_SIZE,	0,//string name,			
						p_ui,	TYPE_NODELISTBOX, IDC_FORCESWLIST,IDC_FORCESW_PICKNODE,0,IDC_REMOVE_FORCESW,
						end,

			// forces pointers for maxscript manipulation
			kForceSpaceWarp_ForcesMaxscript,		_T("Force_Space_Warps"),
						TYPE_INODE_TAB,		0,	P_VARIABLE_SIZE|P_NO_REF,	IDS_FORCESPACEWARP_NODES,//string name,			
						end,

			// force overlapping: additive or maximum
			kForceSpaceWarp_Overlapping,		_T("Force_Overlapping"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_FORCEOVERLAPPING,
			// optional tagged param specs
					p_default,		kForceSpaceWarp_Overlapping_additive,
					p_range,		0, kForceSpaceWarp_Overlapping_num-1,
					p_ui,			TYPE_RADIO, kForceSpaceWarp_Overlapping_num, IDC_ADDITIVE, IDC_MAXIMUM,
			end,

			//future parameters

			kForceSpaceWarp_UseAccel,		_T(""),
						TYPE_INT,		0,		0,//string name,			
						end,
/*
		// reverse
			kSimpleSpeed_reverse,	_T("Reverse"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_REVERSE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_REVERSE,
			end,
		// divergence
			kSimpleSpeed_divergence,_T("Divergence"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 180.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_DIVERGENCE, IDC_DIVERGENCESPIN, 0.5f,
			end,
		// seed
			kSimpleSpeed_seed,		_T("Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT, IDC_SEED, IDC_SEEDSPIN, 1.f,
			end,
*/
		// new?
		end 
);

// ForceSpaceWarp Operator param block for script wiring
ParamBlockDesc2 forceSpaceWarp_scriptBlockDesc 
(
	// required block spec
		kForceSpaceWarp_scriptPBlockIndex, 
		_T("Script Wiring"),
		IDS_SCRIPTWIRING,
		&ThePFOperatorForceSpaceWarpDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kForceSpaceWarp_scriptPBlockIndex,
	// auto ui parammap specs : none
		IDD_FORCESPACEWARP_SCRIPTWIRING, 
		IDS_SCRIPTWIRING,
		0,
		0, // open/closed
		NULL,
	// required param specs
		// use script wiring
			kForceSpaceWarp_useScriptWiring, _T(""), //_T("Use_Script_Wiring"),     --010803  az: for future use 
									TYPE_INT,
									0,
									IDS_USESCRIPTWIRING,
			// optional tagged param specs
					p_default,		FALSE,
			end,
		// float wiring
			kForceSpaceWarp_useFloat,	_T("Use_Script_Float"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTFLOAT,
			// optional tagged param specs
					p_default,		kForceSpaceWarp_useFloat_none,
					p_range,		0, kForceSpaceWarp_useFloat_num-1,
					p_ui,			TYPE_RADIO, kForceSpaceWarp_useFloat_num, IDC_NOTUSED, IDC_INFLUENCE,
			end,

		// for future use
			kForceSpaceWarp_useInt,	_T(""),  //_T("Use_Script_Integer"), --010803  az: for future use 
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTINTEGER,
			// optional tagged param specs
					p_default,		0,
			end,
			kForceSpaceWarp_useVector,	_T(""),  //_T("Use_Script_Vector"), --010803  az: for future use 
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTVECTOR,
			// optional tagged param specs
					p_default,		0,
			end,
			kForceSpaceWarp_useMatrix,	_T(""),  //_T("Use_Script_Matrix") --010803  az: for future use 
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTMATRIX,
			// optional tagged param specs
					p_default,		0,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc forceSpaceWarp_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorForceSpaceWarpDesc, FP_MIXIN, 
				
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
			IPFAction::kIsMaterialHolder, _T("isMaterialHolder"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,
			IPFAction::kGetUseScriptWiring, _T("getUseScriptWiring"), 0, TYPE_bool, 0, 0,
			IPFAction::kSetUseScriptWiring, _T("setUseScriptWiring"), 0, TYPE_VOID, 0, 1,
				_T("useState"), 0, TYPE_bool,

		end
);

FPInterfaceDesc forceSpaceWarp_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorForceSpaceWarpDesc, FP_MIXIN,

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

FPInterfaceDesc forceSpaceWarp_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorForceSpaceWarpDesc, FP_MIXIN,

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

BOOL PFOperatorForceSpaceWarpDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock;
	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kForceSpaceWarp_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = (map != NULL) ? map->GetParamBlock() : NULL;
		switch ( LOWORD( wParam ) )
		{
		case IDC_FORCESW_PICKNODE:
			if (pblock)
				pblock->NotifyDependents( FOREVER, 0, kForceSpaceWarp_RefMsg_ResetValidatorAction );
			break;
		case IDC_BYLIST:
			if (pblock)
				pblock->NotifyDependents( FOREVER, 0, kForceSpaceWarp_RefMsg_ListSelect );
			break;
		case kForceSpaceWarp_message_nodes:
			UpdateOverlappingDlg( hWnd, pblock );
			break;
		case kForceSpaceWarp_message_enableInfluence:
			UpdateInfluenceDlg( hWnd, true);
			break;
		case kForceSpaceWarp_message_disableInfluence:
			UpdateInfluenceDlg( hWnd, false);
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorForceSpaceWarpDlgProc::UpdateOverlappingDlg( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;

	int numNodes = pblock->Count(kForceSpaceWarp_ForceNodeList);
	EnableWindow( GetDlgItem( hWnd, IDC_OVERLAPPINGFRAME), numNodes > 1 );
	EnableWindow( GetDlgItem( hWnd, IDC_ADDITIVE), numNodes > 1 );
	EnableWindow( GetDlgItem( hWnd, IDC_MAXIMUM), numNodes > 1 );

	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_REMOVE_FORCESW ) );
	button->Enable( numNodes > 0 );
	ReleaseICustButton( button );
}

void PFOperatorForceSpaceWarpDlgProc::UpdateInfluenceDlg( HWND hWnd, bool enable )
{
	EnableWindow( GetDlgItem( hWnd, IDC_INFLUENCETEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_INFLUENCESPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
}


} // end of namespace PFActions