/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpeed_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpeed Operator (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 01-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleSpeed_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimpleSpeedDesc ThePFOperatorSimpleSpeedDesc;

// Dialog Proc
PFOperatorSimpleSpeedDlgProc ThePFOperatorSimpleSpeedDlgProc;

// SimpleSpeed Operator param block
ParamBlockDesc2 simpleSpeed_paramBlockDesc 
(
	// required block spec
		kSimpleSpeed_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleSpeedDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleSpeed_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLESPEED_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleSpeedDlgProc,
	// required param specs
		// speed
			kSimpleSpeed_speed,		_T("Speed"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPEED,
			// optional tagged param specs
					p_default,		300.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_SPEED, IDC_SPEEDSPIN, SPIN_AUTOSCALE,
			end,
		// variation
			kSimpleSpeed_variation,	_T("Variation"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_VARIATION, IDC_VARIATIONSPIN, SPIN_AUTOSCALE,
			end,
		// direction
			kSimpleSpeed_direction,	_T("Direction"),
									TYPE_INT,
									0,
									IDS_DIRECTION,
			// optional tagged param specs
					p_default,		kSS_Along_Icon_Arrow,
					p_ui,			TYPE_INTLISTBOX, IDC_DIRECTION, kSS_Inherit_Prev + 1,
										IDS_ALONG_ICON_ARROW, IDS_ICON_CENTER_0UT, IDS_ICON_ARROW_OUT,
										IDS_RAND_3D, IDS_RAND_HORIZ, IDS_INHERIT_PREV,
					p_vals,			kSS_Along_Icon_Arrow, kSS_Icon_Center_Out, kSS_Icon_Arrow_Out,
									kSS_Rand_3D, kSS_Rand_Horiz, kSS_Inherit_Prev,
			end,
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
			kSimpleSpeed_seed,		_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT, IDC_SEED, IDC_SEEDSPIN, 1.f,
			end,
		// new?
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc simpleSpeed_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleSpeedDesc, FP_MIXIN, 
				
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

FPInterfaceDesc simpleSpeed_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleSpeedDesc, FP_MIXIN,

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

FPInterfaceDesc simpleSpeed_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleSpeedDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleSpeedDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			SetCtlByDir(map, hWnd);
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_DIRECTION:
					switch(HIWORD(wParam)) {
						case CBN_SELCHANGE:
							SetCtlByDir(map, hWnd);
							break;
					}
					break;
				case IDC_NEW:
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSimpleSpeed_RefMsg_NewRand);
					break;
			}
			break;
	}

	return FALSE;
}

void PFOperatorSimpleSpeedDlgProc::SetCtlByDir(IParamMap2* map, HWND hWnd)
{
	BOOL bReverse, bDivergence;
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int iDir = ppb->GetInt(kSimpleSpeed_direction, 0);
			if(iDir == kSS_Rand_3D)
				bReverse = bDivergence = FALSE;
			else if(iDir == kSS_Rand_Horiz) {
				bReverse = FALSE;
				bDivergence = TRUE;
			}
			else
				bReverse = bDivergence = TRUE;
			EnableWindow(GetDlgItem(hWnd, IDC_REVERSE), bReverse);
			ISpinnerControl* psc = GetISpinner(GetDlgItem(hWnd, IDC_DIVERGENCE));
			psc->Enable(bDivergence);
			ReleaseISpinner(psc);
		}
	}
}

} // end of namespace PFActions