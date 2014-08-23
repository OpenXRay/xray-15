/**********************************************************************
 *<
	FILE:			PFOperatorSimpleOrientation_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleOrientation Operator (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleOrientation_ParamBlock.h"
#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimpleOrientationDesc ThePFOperatorSimpleOrientationDesc;

// Dialog Proc
PFOperatorSimpleOrientationDlgProc ThePFOperatorSimpleOrientationDlgProc;

// SimpleOrientation Operator param block
ParamBlockDesc2 simpleOrientation_paramBlockDesc 
(
	// required block spec
		kSimpleOrientation_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleOrientationDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleOrientation_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLEORIENTATION_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleOrientationDlgProc,
	// required param specs
		// direction
			kSimpleOrientation_direction,	_T("Direction"),
									TYPE_INT,
									0,
									IDS_DIRECTION,
			// optional tagged param specs
					p_default,		kSO_Rand_3D,
					p_ui,			TYPE_INTLISTBOX, IDC_DIRECTION, kSO_type_num,
										IDS_RAND_3D, IDS_RAND_HORIZ, IDS_WORLD_SPACE, IDS_SPEED_SPACE, IDS_SPEED_SPACE_FOLLOW,
					p_vals,			kSO_Rand_3D, kSO_Rand_Horiz, kSO_World, kSO_Speed, kSO_SpeedFollow,
			end,
		// Euler x
			kSimpleOrientation_x,	_T("Euler_X"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_EULER_X,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-9999999.f, 9999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_X, IDC_XSPIN, 0.5f,
			end,
		// Euler y
			kSimpleOrientation_y,	_T("Euler_Y"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_EULER_Y,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-9999999.f, 9999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_Y, IDC_YSPIN, 0.5f,
			end,
		// Euler z
			kSimpleOrientation_z,	_T("Euler_Z"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_EULER_Z,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-9999999.f, 9999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_Z, IDC_ZSPIN, 0.5f,
			end,
		// divergence
			kSimpleOrientation_divergence,_T("Divergence"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 180.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,
									IDC_DIVERGENCE, IDC_DIVERGENCESPIN, 0.5f,
			end,
		// locked on the emitter
			kSimpleOrientation_restrictToAxis,	_T("Restrict_Divergence_To_Axis"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_RESTRICTTOAXIS,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_RESTRICTTOAXIS,
			end,
		// Div. Axis X
			kSimpleOrientation_axisX,	_T("Divergence_Axis_X"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCEAXIS_X,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		-1.f, 1.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AXISX, IDC_AXISXSPIN, SPIN_AUTOSCALE,
			end,
		// Div. Axis Y
			kSimpleOrientation_axisY,	_T("Divergence_Axis_Y"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCEAXIS_Y,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-1.f, 1.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AXISY, IDC_AXISYSPIN, SPIN_AUTOSCALE,
			end,
		// Div. Axis Y
			kSimpleOrientation_axisZ,	_T("Divergence_Axis_Z"),
									TYPE_FLOAT,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCEAXIS_Z,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		-1.f, 1.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_AXISZ, IDC_AXISZSPIN, SPIN_AUTOSCALE,
			end,
		// seed
			kSimpleOrientation_seed,		_T("Random_Seed"),
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
FPInterfaceDesc simpleOrientation_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleOrientationDesc, FP_MIXIN, 
				
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

FPInterfaceDesc simpleOrientation_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleOrientationDesc, FP_MIXIN,

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

FPInterfaceDesc simpleOrientation_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleOrientationDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleOrientationDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			// Send the message to notify the initialization of dialog
			map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSimpleOrientation_RefMsg_Init );
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_NEW:
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSimpleOrientation_RefMsg_NewRand);
					break;
				case kSimpleOrientation_message_update:
					SetCtlByDir(map, hWnd);
					break;
			}
			break;
	}

	return FALSE;
}

void PFOperatorSimpleOrientationDlgProc::SetCtlByDir(IParamMap2* map, HWND hWnd)
{
	BOOL bEulerSpinOn = false;
	if(map && hWnd) {
		IParamBlock2* ppb = map->GetParamBlock();
		if(ppb) {
			int iDir = ppb->GetInt(kSimpleOrientation_direction, 0);
			if((iDir == kSO_World) || (iDir == kSO_Speed) || (iDir == kSO_SpeedFollow))
				bEulerSpinOn = true;

			EnableWindow(GetDlgItem(hWnd, IDC_XTEXT), bEulerSpinOn);
			ISpinnerControl* psc = GetISpinner(GetDlgItem(hWnd, IDC_XSPIN));
			psc->Enable(bEulerSpinOn);
			ReleaseISpinner(psc);
			EnableWindow(GetDlgItem(hWnd, IDC_YTEXT), bEulerSpinOn);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_YSPIN));
			psc->Enable(bEulerSpinOn);
			ReleaseISpinner(psc);
			EnableWindow(GetDlgItem(hWnd, IDC_ZTEXT), bEulerSpinOn);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_ZSPIN));
			psc->Enable(bEulerSpinOn);
			ReleaseISpinner(psc);
			bool bDivergenceAvailable = ((iDir != kSO_Rand_3D) && (iDir != kSO_SpeedFollow));
			EnableWindow(GetDlgItem(hWnd, IDC_DIVERGENCETEXT), bDivergenceAvailable);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_DIVERGENCESPIN));
			psc->Enable(bDivergenceAvailable);
			ReleaseISpinner(psc);
			
			EnableWindow(GetDlgItem(hWnd, IDC_RESTRICTTOAXIS), bDivergenceAvailable);
			bool bRestrictToAxis = (ppb->GetInt(kSimpleOrientation_restrictToAxis, 0) != 0) && bDivergenceAvailable;
			EnableWindow(GetDlgItem(hWnd, IDC_DIVERGENCEAXISBOX), bRestrictToAxis);
			EnableWindow(GetDlgItem(hWnd, IDC_AXISXTEXT), bRestrictToAxis);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_AXISXSPIN));
			psc->Enable(bRestrictToAxis);
			ReleaseISpinner(psc);
			EnableWindow(GetDlgItem(hWnd, IDC_AXISYTEXT), bRestrictToAxis);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_AXISYSPIN));
			psc->Enable(bRestrictToAxis);
			ReleaseISpinner(psc);
			EnableWindow(GetDlgItem(hWnd, IDC_AXISZTEXT), bRestrictToAxis);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_AXISZSPIN));
			psc->Enable(bRestrictToAxis);
			ReleaseISpinner(psc);

			bool bChaosAvailable = (iDir != kSO_SpeedFollow);
			EnableWindow(GetDlgItem(hWnd, IDC_UNIQUENESSBOX), bChaosAvailable);
			EnableWindow(GetDlgItem(hWnd, IDC_SEEDTEXT), bChaosAvailable);
			psc = GetISpinner(GetDlgItem(hWnd, IDC_SEEDSPIN));
			psc->Enable(bChaosAvailable);
			ReleaseISpinner(psc);
			ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_NEW));
			but->Enable(bChaosAvailable);
			ReleaseICustButton(but);
		}
	}
}

} // end of namespace PFActions