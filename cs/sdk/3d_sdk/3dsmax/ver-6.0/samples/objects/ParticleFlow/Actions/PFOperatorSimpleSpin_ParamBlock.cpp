/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpin_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 02-01-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleSpin_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimpleSpinDesc ThePFOperatorSimpleSpinDesc;

// Dialog Proc
PFOperatorSimpleSpinDlgProc ThePFOperatorSimpleSpinDlgProc;

// validator for the direction parameter
PFOperatorSimpleSpinValidator thePFOperatorSimpleSpinValidator;

// SimpleSpin Operator param block
ParamBlockDesc2 simpleSpin_paramBlockDesc 
(
	// required block spec
		kSimpleSpin_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleSpinDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleSpin_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLESPIN_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleSpinDlgProc,
	// required param specs
		// spin rate
			kSimpleSpin_spinrate,	_T("SpinRate"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPINRATE,
			// optional tagged param specs
					p_default,		6.2831853f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_SPINRATE, IDC_SPINRATESPIN, SPIN_AUTOSCALE,
			end,
		// spin rate variation
			kSimpleSpin_variation,	_T("Variation"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 9999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_VARIATION, IDC_VARIATIONSPIN, SPIN_AUTOSCALE,
			end,
		// direction
			kSimpleSpin_direction,	_T("Direction"),
									TYPE_INT,
									0,
									IDS_DIRECTION,
			// optional tagged param specs
					p_default,		kSS_Rand_3D,
					p_ui,			TYPE_INTLISTBOX, IDC_DIRECTION, kSS_Speed_Space_Follow + 1,
										IDS_RAND_3D, IDS_WORLD_SPACE,
										IDS_PARTICLE_SPACE, IDS_SPEED_SPACE, IDS_SPEED_SPACE_FOLLOW,
					p_vals,			kSS_Rand_3D, kSS_World_Space,
									kSS_Particle_Space, kSS_Speed_Space, kSS_Speed_Space_Follow,
					p_validator,	&thePFOperatorSimpleSpinValidator,
			end,
		// spin axis x
			kSimpleSpin_X,			_T("Spin_X_Axis"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPIN_X,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-1.f, 1.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_X, IDC_XSPIN, SPIN_AUTOSCALE,
			end,
		// spin axis y
			kSimpleSpin_Y,			_T("Spin_Y_Axis"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPIN_Y,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-1.f, 1.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_Y, IDC_YSPIN, SPIN_AUTOSCALE,
			end,
		// spin axis z
			kSimpleSpin_Z,			_T("Spin_Z_Axis"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_SPIN_Z,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-1.f, 1.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_Z, IDC_ZSPIN, SPIN_AUTOSCALE,
			end,
		// divergence
			kSimpleSpin_divergence,	_T("Divergence"),
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
			kSimpleSpin_seed,		_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345,
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_INT, IDC_SEED, IDC_SEEDSPIN, 1.f,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc simpleSpin_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleSpinDesc, FP_MIXIN, 
				
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

FPInterfaceDesc simpleSpin_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleSpinDesc, FP_MIXIN,

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

FPInterfaceDesc simpleSpin_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleSpinDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleSpinDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			UpdateDlg(map, hWnd);
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_NEW:
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSimpleSpin_RefMsg_NewRand);
					break;
				case kSimpleSpin_message_update:
					UpdateDlg(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PFOperatorSimpleSpinDlgProc::UpdateDlg(IParamMap2* map, HWND hWnd)
{
	BOOL bAxes, bDivergence, bRandom;
	bool varAnimated, divAnimated;
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int iDir = ppb->GetInt(kSimpleSpin_direction, 0);
			switch(iDir) {
			case kSS_Rand_3D:
				bAxes = bDivergence = FALSE;
				break;
			case kSS_Speed_Space_Follow:
				bAxes = TRUE;
				bDivergence = FALSE;
				break;
			default:
				bAxes = bDivergence = TRUE;
			}
			EnableWindow( GetDlgItem( hWnd, IDC_XTEXT ), bAxes );
			ISpinnerControl* psc = GetISpinner(GetDlgItem(hWnd, IDC_XSPIN));
			psc->Enable(bAxes);
			ReleaseISpinner(psc);
			EnableWindow( GetDlgItem( hWnd, IDC_YTEXT ), bAxes );
			psc = GetISpinner(GetDlgItem(hWnd, IDC_YSPIN));
			psc->Enable(bAxes);
			ReleaseISpinner(psc);
			EnableWindow( GetDlgItem( hWnd, IDC_ZTEXT ), bAxes );
			psc = GetISpinner(GetDlgItem(hWnd, IDC_ZSPIN));
			psc->Enable(bAxes);
			ReleaseISpinner(psc);
			EnableWindow( GetDlgItem( hWnd, IDC_DIVERGENCETEXT ), bDivergence );
			psc = GetISpinner(GetDlgItem(hWnd, IDC_DIVERGENCESPIN));
			psc->Enable(bDivergence);
			ReleaseISpinner(psc);
			bRandom = (ppb->GetFloat(kSimpleSpin_variation, 0) == 0.0f) ? FALSE : TRUE;
			if (!bRandom) { // maybe variation is animated
				varAnimated = false;
				Control* ctrl = ppb->GetController(kSimpleSpin_variation);
				if (ctrl != NULL)
					varAnimated = (ctrl->IsAnimated() != 0);
				if (varAnimated) bRandom = TRUE;
			}
			if (!bRandom) { // maybe divergence is non-zero
				if ((iDir != kSS_Rand_3D) && (iDir != kSS_Speed_Space_Follow)) {
					bRandom = (ppb->GetFloat(kSimpleSpin_divergence, 0) == 0.0f) ? FALSE : TRUE;
					if (!bRandom) { // maybe divergence is animated
						divAnimated = false;
						Control* ctrl = ppb->GetController(kSimpleSpin_divergence);
						if (ctrl != NULL)
							divAnimated = (ctrl->IsAnimated() != 0);
						if (divAnimated) bRandom = TRUE;
					}
				}
			}
			EnableWindow( GetDlgItem( hWnd, IDC_UNIQUENESSBOX ), bRandom );
			EnableWindow( GetDlgItem( hWnd, IDC_SEEDTEXT ), bRandom );
			psc = GetISpinner( GetDlgItem( hWnd, IDC_SEEDSPIN ) );
			psc->Enable( bRandom );
			ReleaseISpinner( psc );
			ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_NEW ) );
			button->Enable( bRandom );
			ReleaseICustButton( button );
		}
	}
}

BOOL PFOperatorSimpleSpinValidator::Validate(PB2Value& v)
{
	switch(v.i) {
		case kSS_Rand_3D: case kSS_World_Space: case kSS_Particle_Space:
		case kSS_Speed_Space: case kSS_Speed_Space_Follow:
			return TRUE;
	}
	return FALSE;
}

} // end of namespace PFActions