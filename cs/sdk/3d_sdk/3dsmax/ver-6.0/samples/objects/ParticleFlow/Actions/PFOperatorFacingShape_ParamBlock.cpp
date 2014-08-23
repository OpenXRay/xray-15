/**********************************************************************
 *<
	FILE:			PFOperatorFacingShape_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for FacingShape Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorFacingShape_ParamBlock.h"

#include "PFOperatorFacingShape.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorFacingShapeDesc		ThePFOperatorFacingShapeDesc;
// Dialog Proc
PFOperatorFacingShapeDlgProc	ThePFOperatorFacingShapeDlgProc;


// FacingShape Operator param block
ParamBlockDesc2 facingShape_paramBlockDesc 
(
	// required block spec
		kFacingShape_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorFacingShapeDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kFacingShape_reference_pblock,
	// auto ui parammap specs : none
		IDD_FACINGSHAPE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorFacingShapeDlgProc,
	// required param specs
		// look at object for maxscript manipulation
			kFacingShape_objectMaxscript,	_T("Look_At_Object"),
									TYPE_INODE,
									0,
									IDS_LOOKATOBJECT,
			end,
		// use parallel direction
			kFacingShape_parallel,	_T("Parallel"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_PARALLEL,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_PARALLEL,
			end,
		// size space (world/screen)
			kFacingShape_sizeSpace, _T("Size_Space"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SIZESPACE,
			// optional tagged param specs
					p_default,		kFacingShape_sizeSpace_world,
					p_range,		kFacingShape_sizeSpace_world, kFacingShape_sizeSpace_screen,
					p_ui,			TYPE_RADIO, kFacingShape_sizeSpace_num, IDC_WORLD, IDC_LOCAL, IDC_SCREEN,
			end,
		// unit size in world space
			kFacingShape_units,		_T("Units"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_UNITS,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		kFacingShape_minUnits, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_UNITS, IDC_UNITSSPIN, 1.0f,
			end,
		// inheritance of size in local space
			kFacingShape_inheritedScale, _T("Inherited_Scale"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_INHERITEDSCALE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 10000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_INHERITED, IDC_INHERITEDSPIN, 1.0f,
			end,
		// proportion of size in screen space
			kFacingShape_proportion, _T("Proportion"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_PROPORTION,
			// optional tagged param specs
					p_default,		0.01f,
					p_range,		kFacingShape_minProportion, kFacingShape_maxProportion,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_PROPORTION, IDC_PROPORTIONSPIN, 1.0f,
			end,
		// size variation
			kFacingShape_variation, _T("Variation"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_SIZEVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,	IDC_VARIATION, IDC_VARIATIONSPIN, 1.0f,
			end,
		// pivot at
			kFacingShape_pivotAt,	_T("Pivot_At"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_PIVOTAT,
			// optional tagged param specs
					p_default,		kFacingShape_pivotAt_center,
					p_ui,			TYPE_INTLISTBOX,	IDC_PIVOT,	kFacingShape_pivotAt_num,
												IDS_PIVOTAT_TOP, IDS_PIVOTAT_CENTER,
												IDS_PIVOTAT_BOTTOM,
					p_vals,			kFacingShape_pivotAt_top,
									kFacingShape_pivotAt_center,
									kFacingShape_pivotAt_bottom,
			end,
		// width/height ratio
			kFacingShape_WHRatio,	_T("WH_Ratio"),
									TYPE_FLOAT,
									P_RESET_DEFAULT,
									IDS_WHRATIO,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		kFacingShape_minWHRatio, kFacingShape_maxWHRatio,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_RATIO, IDC_RATIOSPIN, SPIN_AUTOSCALE,
			end,
		// use material
			kFacingShape_useMaterial_obsolete, _T(""), // _T("Use_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_USEMATERIAL,
			// optional tagged param specs
					p_default,		TRUE,
			end,
		// material
			kFacingShape_material_obsolete,	_T(""), // _T("Material"),
									TYPE_MTL,
									P_OWNERS_REF,
									IDS_NOTHING, // IDS_MATERIAL,
			// optional tagged param specs
					p_refno,		kFacingShape_reference_material,
			end,
		// number of sub-materials
			kFacingShape_numSubMtls_obsolete, _T(""), // _T("Num_SubMtls"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_NUMFSUBMTLS,
			// optional tagged param specs
					p_default,		kFacingShape_minNumMtls,
					p_range,		kFacingShape_minNumMtls, 999999999,
			end,
		// number of sub-materials per frame
			kFacingShape_subsPerFrame_obsolete, _T(""), // _T("Subs_Per_Frame"),
									TYPE_FLOAT,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_SUBSPERFRAME,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		kFacingShape_minRate, kFacingShape_maxRate,
			end,
		// loop frames
			kFacingShape_loop_obsolete,		_T(""), // _T("Loop"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_LOOP,
			// optional tagged param specs
					p_default,		TRUE,
			end,
		// sync type
			kFacingShape_syncType_obsolete, _T(""), // _T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kFacingShape_syncBy_absoluteTime,
			end,
		// add random offset to the synchronization
			kFacingShape_syncRandom_obsolete, _T(""), // _T("Add_Random_Offset"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_ADDRANDOMOFFSET,
			// optional tagged param specs
					p_default,		FALSE,
			end,
		// random offset
			kFacingShape_randomOffset_obsolete, _T(""), // _T("Random_Offset"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_RANDOMOFFSET,
			// optional tagged param specs
					p_default,		30,
					p_range,		1, 999999999,
			end,
		// random seed
			kFacingShape_randomSeed, _T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345, // initial random seed
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,
		// orientation
			kFacingShape_orientation,	_T("Orientation"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_ORIENTATION,
			// optional tagged param specs
					p_default,		kFacingShape_orientation_horizon,
					p_ui,			TYPE_INTLISTBOX,	IDC_ORIENTATION,	kFacingShape_orientation_num,
												IDS_ALIGNTOHORIZON, 
												IDS_ALIGNTOSPEEDFOLLOW,
												IDS_RANDOM,
												IDS_ALLOWSPIN,
					p_vals,			kFacingShape_orientation_horizon,
									kFacingShape_orientation_speed,
									kFacingShape_orientation_random,
									kFacingShape_orientation_spin,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc facingShape_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorFacingShapeDesc, FP_MIXIN, 
				
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

FPInterfaceDesc facingShape_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorFacingShapeDesc, FP_MIXIN,

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

FPInterfaceDesc facingShape_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorFacingShapeDesc, FP_MIXIN,

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

BOOL PFOperatorFacingShapeDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
	ICustButton *iBut;
	PFOperatorFacingShape *op;
	INode* obj;
	TSTR name;
	int sizeSpace;
	BOOL isCameraObj, parallelDir;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Set ICustButton properties for Pick button
		iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
		iBut->SetType(CBT_CHECK);
		iBut->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(iBut);
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kFacingShape_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_PICK:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kFacingShape_RefMsg_PickObj );
			break;
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kFacingShape_RefMsg_NewRand );
			break;
		case kFacingShape_message_name:
			op = reinterpret_cast <PFOperatorFacingShape *> (lParam);
			obj = (op) ? const_cast <INode*> (op->object()) : NULL;
			name = (obj) ? obj->GetName() : GetString(IDS_NONE);
			UpdateObjectNameDlg(hWnd, name);
			break;
		case kFacingShape_message_enterPick:
			iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
			iBut->SetCheck(TRUE);
			ReleaseICustButton(iBut);
			break;
		case kFacingShape_message_exitPick:
			iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
			iBut->SetCheck(FALSE);
			ReleaseICustButton(iBut);
			break;
		case kFacingShape_message_size:
			op = reinterpret_cast <PFOperatorFacingShape *> (lParam);
			isCameraObj = (op) ? PFOperatorFacingShape::IsCameraObject(op->_object()) : FALSE;
			parallelDir = map->GetParamBlock()->GetInt(kFacingShape_parallel, t);
			sizeSpace = map->GetParamBlock()->GetInt(kFacingShape_sizeSpace, t);
			UpdateWidthSizeDlg( hWnd, isCameraObj, parallelDir, sizeSpace );
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorFacingShapeDlgProc::UpdateObjectNameDlg( HWND hWnd, TSTR& objectName )
{
	ICustButton* but;
	but = GetICustButton( GetDlgItem( hWnd, IDC_PICK ) );
	but->SetText(objectName);
	ReleaseICustButton(but);
}

void PFOperatorFacingShapeDlgProc::UpdateWidthSizeDlg( HWND hWnd, BOOL isCameraObj, BOOL parallelDir, int sizeSpace )
{
	ISpinnerControl *spin;
	bool okScreen = isCameraObj && !parallelDir;
	bool inWorld = (sizeSpace == kFacingShape_sizeSpace_world);
	bool inLocal = (sizeSpace == kFacingShape_sizeSpace_local);
	bool inScreen = (sizeSpace == kFacingShape_sizeSpace_screen);

	if (!okScreen && inScreen) {
		inScreen = false;
		inWorld = true;
	}
	
	// enable/disable in screen radio button
	EnableWindow( GetDlgItem( hWnd, IDC_SCREEN ), okScreen );
	// enable/disable units size spinner
	EnableWindow( GetDlgItem( hWnd, IDC_UNITSTEXT ), inWorld );
	EnableWindow( GetDlgItem( hWnd, IDC_UNITS ), inWorld );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_UNITSSPIN ) );
	spin->Enable( inWorld );
	ReleaseISpinner( spin );
	// enable/disable inheritance size spinner
	EnableWindow( GetDlgItem( hWnd, IDC_INHERITEDTEXT ), inLocal );
	EnableWindow( GetDlgItem( hWnd, IDC_INHERITED ), inLocal );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_INHERITEDSPIN ) );
	spin->Enable( inLocal );
	ReleaseISpinner( spin );
	// enable/disable proportion size spinner
	EnableWindow( GetDlgItem( hWnd, IDC_PROPORTIONTEXT ), inScreen );
	EnableWindow( GetDlgItem( hWnd, IDC_PROPORTION ), inScreen );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_PROPORTIONSPIN ) );
	spin->Enable( inScreen );
	ReleaseISpinner( spin );
}

void PFOperatorFacingShapeDlgProc::UpdateMaterialDlg( HWND hWnd, BOOL useMaterial, BOOL addRandomOffset )
{
	ISpinnerControl *spin;

	// enable/disable number of sub-materials spinner
	EnableWindow( GetDlgItem( hWnd, IDC_NUMSUBSTEXT ), useMaterial );
	EnableWindow( GetDlgItem( hWnd, IDC_NUMSUBS ), useMaterial );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_NUMSUBSSPIN ) );
	spin->Enable( useMaterial );
	ReleaseISpinner( spin );
	// enable/disable sub-materials per frame spinner
	EnableWindow( GetDlgItem( hWnd, IDC_RATETEXT ), useMaterial );
	EnableWindow( GetDlgItem( hWnd, IDC_RATE ), useMaterial );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_RATESPIN ) );
	spin->Enable( useMaterial );
	ReleaseISpinner( spin );
	// enable/disable loop checkbox
	EnableWindow( GetDlgItem( hWnd, IDC_LOOP ), useMaterial );
	// enable/disable sync by drop list
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCBYTEXT ), useMaterial );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNC ), useMaterial );
	// enable/disable random offset checkbox
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCRANDOM ), useMaterial );

	UpdateRandomOffsetDlg( hWnd, useMaterial && addRandomOffset );
}

void PFOperatorFacingShapeDlgProc::UpdateRandomOffsetDlg( HWND hWnd, BOOL enableRandomOffset )
{
	ISpinnerControl *spin;
	
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCFRAMETEXT ), enableRandomOffset );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCOFFSET ), enableRandomOffset );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SYNCOFFSETSPIN ) );
	spin->Enable( enableRandomOffset );
	ReleaseISpinner( spin );
}

} // end of namespace PFActions