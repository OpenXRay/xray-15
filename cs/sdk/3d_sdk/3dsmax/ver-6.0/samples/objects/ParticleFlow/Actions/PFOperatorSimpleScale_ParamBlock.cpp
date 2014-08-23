/**********************************************************************
 *<
	FILE:			PFOperatorSimpleScale_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleScale Operator (definition)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 07-22-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleScale_ParamBlock.h"
#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimpleScaleDesc ThePFOperatorSimpleScaleDesc;

// Dialog Proc
PFOperatorSimpleScaleDlgProc ThePFOperatorSimpleScaleDlgProc;

// SimpleScale Operator param block
ParamBlockDesc2 simpleScale_paramBlockDesc 
(
	// required block spec
		kSimpleScale_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleScaleDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleScale_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLESCALE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleScaleDlgProc,
	// required param specs
		// type
			kSimpleScale_Type,		_T("Type"),
									TYPE_INT,
									0,
									IDS_TYPE,
			// optional tagged param specs
					p_default,		kSS_OverwriteOnce,
					p_ui,			TYPE_INTLISTBOX, IDC_TYPE, kSS_RelativeSuccessive + 1,
										IDS_OVERWRITE_ONCE, IDS_INHERIT_ONCE, IDS_ABSOLUTE,
										IDS_RELATIVE_FIRST, IDS_RELATIVE_SUCCESSIVE,

					p_vals,			kSS_OverwriteOnce, kSS_InheritOnce, kSS_Absolute,
									kSS_RelativeFirst, kSS_RelativeSuccessive,
			end,
		// x scale factor
			kSimpleScale_XScaleFactor,	_T("X_Scale_Factor"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_XSCALE_FACTOR,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_XSCALE, IDC_XSCALESPIN, 1.0f,
			end,
		// y scale factor
			kSimpleScale_YScaleFactor,	_T("Y_Scale_Factor"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_YSCALE_FACTOR,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_YSCALE, IDC_YSCALESPIN, 1.0f,
			end,
		// z scale factor
			kSimpleScale_ZScaleFactor,	_T("Z_Scale_Factor"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ZSCALE_FACTOR,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_ZSCALE, IDC_ZSCALESPIN, 1.0f,
			end,
		// constrain scale factor
			kSimpleScale_sfConstrain,	_T("Constrain_Scale"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_CONSTRAINSCALE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_CONSTRAINSCALE,
			end,
		// x scale variation
			kSimpleScale_XScaleVariation,	_T("X_Scale_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_XSCALE_VARIATION,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_XVAR, IDC_XVARSPIN, 1.0f,
			end,
		// y scale variation
			kSimpleScale_YScaleVariation,	_T("Y_Scale_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_YSCALE_VARIATION,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_YVAR, IDC_YVARSPIN, 1.0f,
			end,
		// z scale variation
			kSimpleScale_ZScaleVariation,	_T("Z_Scale_Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_ZSCALE_VARIATION,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 10000000.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,
									IDC_ZVAR, IDC_ZVARSPIN, 1.0f,
			end,
		// constrain scale variation
			kSimpleScale_svConstrain,	_T("Constrain_Scale_Variation"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_CONSTRAINVARIATION,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_CONSTRAINVARIATION,
			end,
		// bias
			kSimpleScale_svBias,	_T("Bias"),
									TYPE_INT,
									0,
									IDS_BIAS,
			// optional tagged param specs
					p_default,		kSS_None,
					p_ui,			TYPE_INTLISTBOX, IDC_BIAS, kSS_TowardsMax + 1,
										IDS_NONE, IDS_CENTERED, IDS_TOWARDSMIN, IDS_TOWARDSMAX,
					p_vals,			kSS_None, kSS_Centered, kSS_TowardsMin, kSS_TowardsMax,
			end,
		// sync
			kSimpleScale_Sync,		_T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSS_AbsoluteTime,
					p_ui,			TYPE_INTLISTBOX, IDC_SYNC, kSS_EventDuration + 1,
												IDS_SYNCBY_ABSOLUTE, 
												IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSS_AbsoluteTime, kSS_ParticleAge, kSS_EventDuration,
			end,
		// seed
			kSimpleScale_Seed,		_T("Random_Seed"),
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
FPInterfaceDesc simpleScale_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleScaleDesc, FP_MIXIN, 
				
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

FPInterfaceDesc simpleScale_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleScaleDesc, FP_MIXIN,

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

FPInterfaceDesc simpleScale_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleScaleDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleScaleDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			if (map != NULL)
				if (map->GetParamBlock() != NULL)
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSimpleScale_RefMsg_InitDialog);
			break;

		case WM_DESTROY:
			if (map != NULL)
				if (map->GetParamBlock() != NULL)
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSimpleScale_RefMsg_DestroyDialog);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_NEW:
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSimpleScale_RefMsg_NewRand);
					break;
			}
			break;
	}

	return FALSE;
}


} // end of namespace PFActions