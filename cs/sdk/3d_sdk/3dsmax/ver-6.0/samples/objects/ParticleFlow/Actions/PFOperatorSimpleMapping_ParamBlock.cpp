/**********************************************************************
 *<
	FILE:			PFOperatorSimpleMapping_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleMapping Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleMapping_ParamBlock.h"

#include "PFOperatorSimpleMapping.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimpleMappingDesc		ThePFOperatorSimpleMappingDesc;
// Dialog Proc
PFOperatorSimpleMappingDlgProc		ThePFOperatorSimpleMappingDlgProc;
// validators
PFOperatorSimpleMappingSyncTypeValidator thePFOperatorSimpleMappingSyncTypeValidator;
PFOperatorSimpleMappingChannelTypeValidator thePFOperatorSimpleMappingChannelTypeValidator;
PFOperatorSimpleMappingMapChannelValidator thePFOperatorSimpleMappingMapChannelValidator;

// SimpleMapping Operator param block
ParamBlockDesc2 simpleMapping_paramBlockDesc 
(
	// required block spec
		kSimpleMapping_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleMappingDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleMapping_reference_pblock,
	// auto ui parammap specs : none
		IDD_SIMPLEMAPPING_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleMappingDlgProc,
	// required param specs
		// U coord. mapping
			kSimpleMapping_U,	_T("U_Map"),
									TYPE_FLOAT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_UMAP,
			// optional tagged param specs
					p_default,		0.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_U, IDC_USPIN, 0.01f,
			end,
		// V coord. mapping
			kSimpleMapping_V,	_T("V_Map"),
									TYPE_FLOAT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_VMAP,
			// optional tagged param specs
					p_default,		0.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_V, IDC_VSPIN, 0.01f,
			end,
		// W coord. mapping
			kSimpleMapping_W,	_T("W_Map"),
									TYPE_FLOAT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_WMAP,
			// optional tagged param specs
					p_default,		0.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_W, IDC_WSPIN, 0.01f,
			end,
		// sync type
			kSimpleMapping_syncBy, _T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSimpleMapping_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSimpleMapping_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSimpleMapping_syncBy_time,
									kSimpleMapping_syncBy_age,
									kSimpleMapping_syncBy_event,
					p_validator,	&thePFOperatorSimpleMappingSyncTypeValidator,
			end,
		// channel type
			kSimpleMapping_channelType,	_T("Channel_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_CHANNELTYPE,
			// optional tagged param specs
					p_default,		kSimpleMapping_channelType_map,
					p_range,		kSimpleMapping_channelType_map, kSimpleMapping_channelType_vertexColor,
					p_ui,			TYPE_RADIO, kSimpleMapping_channelType_num, IDC_MAPCHANNEL, IDC_VERTEXCOLORCHANNEL,
					p_validator,	&thePFOperatorSimpleMappingChannelTypeValidator,
			end,
		// map channel number
			kSimpleMapping_mapChannel, _T("Map_Channel"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MAPCHANNEL,
			// optional tagged param specs
					p_default,		1,
					p_range,		1, MAX_MESHMAPS-1,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_CHANNELID,	IDC_CHANNELIDSPIN,	1.0f,
					p_validator,	&thePFOperatorSimpleMappingMapChannelValidator,
			end,
		// show map in viewport
			kSimpleMapping_showInViewport, _T("Show_In_Viewport"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SHOWINVIEWPORT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SHOWINVIEWPORT,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc simpleMapping_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleMappingDesc, FP_MIXIN, 
				
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

		end
);

FPInterfaceDesc simpleMapping_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleMappingDesc, FP_MIXIN,

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

FPInterfaceDesc simpleMapping_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleMappingDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleMappingDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
	int channelType;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kSimpleMapping_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case kSimpleMapping_message_channelType:
			if (map->GetParamBlock()) {
				channelType = map->GetParamBlock()->GetInt(kSimpleMapping_channelType, t);
				UpdateChannelTypeDlg(hWnd, channelType);
			}
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorSimpleMappingDlgProc::UpdateChannelTypeDlg( HWND hWnd, int channelType )
{
	ISpinnerControl *spin = GetISpinner( GetDlgItem(hWnd, IDC_CHANNELIDSPIN) );
	spin->Enable(channelType == kSimpleMapping_channelType_map);
	ReleaseISpinner( spin );
}

BOOL PFOperatorSimpleMappingSyncTypeValidator::Validate(PB2Value& v)
{
	switch(v.i) {
	case kSimpleMapping_syncBy_time: case kSimpleMapping_syncBy_age: case kSimpleMapping_syncBy_event:
		return TRUE;
	}
	return FALSE;
}

BOOL PFOperatorSimpleMappingChannelTypeValidator::Validate(PB2Value& v)
{
	if ((v.i >= kSimpleMapping_channelType_map) && (v.i <= kSimpleMapping_channelType_vertexColor))
		return TRUE;
	return FALSE;
}

BOOL PFOperatorSimpleMappingMapChannelValidator::Validate(PB2Value& v)
{
	if ((v.i >= 1) && (v.i <= MAX_MESHMAPS-1))
			return TRUE;
	return FALSE;
}


} // end of namespace PFActions