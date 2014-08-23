/**********************************************************************
 *<
	FILE:			PFOperatorSimpleShape_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SimpleShape Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSimpleShape_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSimpleShapeDesc ThePFOperatorSimpleShapeDesc;
// Dialog Proc
PFOperatorSimpleShapeDlgProc	ThePFOperatorSimpleShapeDlgProc;

// SimpleShape Operator param block
ParamBlockDesc2 simpleShape_paramBlockDesc 
(
	// required block spec
		kSimpleShape_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSimpleShapeDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSimpleShape_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SIMPLESHAPE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSimpleShapeDlgProc,
	// required param specs
		// shape type
			kSimpleShape_shape,		_T("Shape"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SHAPETYPE,
			// optional tagged param specs
					p_default,		kSimpleShape_shape_pyramid,
					p_ui,			TYPE_INTLISTBOX,	IDC_SHAPETYPE,	kSimpleShape_shape_num,
												IDS_SHAPETYPE_VERTEX, IDS_SHAPETYPE_PYRAMID, 
												IDS_SHAPETYPE_CUBE, IDS_SHAPETYPE_SPHERE,
					p_vals,			kSimpleShape_shape_vertex,
									kSimpleShape_shape_pyramid,
									kSimpleShape_shape_cube,
									kSimpleShape_shape_sphere,
			end,
		// shape size
			kSimpleShape_size,		_T("Size"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_MESHSIZE,
			// optional tagged param specs
					p_default,		10.0f, // initial size
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_MESHSIZE, IDC_MESHSIZESPIN, SPIN_AUTOSCALE,
			end,
		// use scale
			kSimpleShape_useScale,	_T("Use_Scale"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_MESHUSESCALE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESCALE,
			end,
		// shape scale
			kSimpleShape_scale, _T("Scale_Value"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_MESHSCALE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 100000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_MESHSCALE, IDC_MESHSCALESPIN, 1.0f,
			end,



		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc simpleShape_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSimpleShapeDesc, FP_MIXIN, 
				
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

FPInterfaceDesc simpleShape_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSimpleShapeDesc, FP_MIXIN,

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

FPInterfaceDesc simpleShape_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSimpleShapeDesc, FP_MIXIN,

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

BOOL PFOperatorSimpleShapeDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
	IParamBlock2* pblock;
	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		pblock = map->GetParamBlock();
		if (pblock)
			pblock->NotifyDependents( FOREVER, (PartID)map, kSimpleShape_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case kSimpleShape_message_useScale:
			pblock = map->GetParamBlock();
			if (pblock)
				UpdateScaleDlg(hWnd, pblock->GetInt(kSimpleShape_useScale, 0) );
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorSimpleShapeDlgProc::UpdateScaleDlg( HWND hWnd, int useScale )
{
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_MESHSCALESPIN ) );
	spin->Enable( useScale != 0 );
	ReleaseISpinner( spin );
}







} // end of namespace PFActions