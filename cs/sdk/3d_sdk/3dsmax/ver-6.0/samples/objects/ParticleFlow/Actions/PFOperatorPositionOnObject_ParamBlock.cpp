/**********************************************************************
 *<
	FILE:			PFOperatorPositionOnObject_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for PositionOnObject Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorPositionOnObject_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorPositionOnObjectDesc		ThePFOperatorPositionOnObjectDesc;
// Dialog Proc
PFOperatorPositionOnObjectDlgProc	ThePFOperatorPositionOnObjectDlgProc;

// PositionOnObject Operator param block
ParamBlockDesc2 positionOnObject_paramBlockDesc 
(
	// required block spec
		kPositionOnObject_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorPositionOnObjectDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kPositionOnObject_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_POSITIONONOBJECT_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorPositionOnObjectDlgProc,
	// required param specs
		// locked on the emitter
			kPositionOnObject_lock,	_T("Lock_On_Emitter"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_LOCK,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_LOCK,
			end,
		// inherit the emitter speed/movement
			kPositionOnObject_inherit, _T("Inherit_Emitter_Movement"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_INHERIT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_INHERIT,
			end,
		// percentage of speed inherited
			kPositionOnObject_inheritAmount,	_T("Multiplier"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_MULTIPLIER,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-999999999.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MULTIPLIER, IDC_MULTIPLIERSPIN, 1.0f,
			end,
		// var of percentage of speed inherited
			kPositionOnObject_inheritAmountVar,	_T("Variation"),
									TYPE_PCNT_FRAC,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_VARIATION2, IDC_VARIATIONSPIN, 1.0f,
			end,
		// emitter objects
			kPositionOnObject_emitters, _T(""),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE,	0,
						p_ui,	TYPE_NODELISTBOX, IDC_OBJECTLIST,IDC_PICKNODE,0,IDC_REMOVENODE,
//						p_accessor,	&lag_accessor,
			end,
		// emitter objects for maxscript manipulation
			kPositionOnObject_emittersMaxscript, _T("Emitter_Objects"),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE|P_NO_REF,	IDS_OPERATOR_EMITTER_NODES,
			end,
		// animated shape
			kPositionOnObject_animated,	_T("Animated_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ANIMATEDSHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANIMATED,
			end,
		// subframe sampling
			kPositionOnObject_subframe,	_T("Subframe_Sampling"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLING,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SUBFRAME,
			end,
		// location type
			kPositionOnObject_type,	_T("Location"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_LOCATION,
			// optional tagged param specs
					p_default,		kPositionOnObject_locationType_surface,
					p_ui,			TYPE_INTLISTBOX,	IDC_TYPE,	kPositionOnObject_locationType_num,
												IDS_LOCATIONTYPE_PIVOT, IDS_LOCATIONTYPE_VERTEX,
												IDS_LOCATIONTYPE_EDGE, IDS_LOCATIONTYPE_SURFACE,
												IDS_LOCATIONTYPE_VOLUME, IDS_LOCATIONTYPE_SELVERTEX,
												IDS_LOCATIONTYPE_SELEDGE, IDS_LOCATIONTYPE_SELFACES,
					p_vals,			kPositionOnObject_locationType_pivot,
									kPositionOnObject_locationType_vertex,
									kPositionOnObject_locationType_edge,
									kPositionOnObject_locationType_surface,
									kPositionOnObject_locationType_volume,
									kPositionOnObject_locationType_selVertex,
									kPositionOnObject_locationType_selEdge,
									kPositionOnObject_locationType_selFaces,
			end,
		// use surface offset
			kPositionOnObject_useOffset,	_T("Use_Surface_Offset"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESURFACEOFFSET,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SURFACEOFFSET,
			end,
		// surface offset minumum
			kPositionOnObject_offsetMin, _T("Surface_Offset_Minimum"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SURFACEOFFSETMINIMUM,
			// optional tagged param specs
					p_default,		-1.0f,
					p_range,		-10000.0f, 10000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_OFFSETMIN, IDC_OFFSETMINSPIN, SPIN_AUTOSCALE,
			end,
		// surface offset maximum
			kPositionOnObject_offsetMax, _T("Surface_Offset_Maximum"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SURFACEOFFSETMAXIMUM,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		-10000.0f, 10000.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_OFFSETMAX, IDC_OFFSETMAXSPIN, SPIN_AUTOSCALE,
			end,
		// use non-uniform density
			kPositionOnObject_useNonUniform,	_T("Density_By_Emitter_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USENONUNIFORMDENSITY,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_NONUNIFORM,
			end,
		// density type
			kPositionOnObject_densityType,	_T("Density_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_DENSITYTYPE,
			// optional tagged param specs
					p_default,		kPositionOnObject_densityType_grayscale,
					p_ui,			TYPE_INTLISTBOX,	IDC_DENSITYTYPE,	kPositionOnObject_densityType_num,
												IDS_DENSITYTYPE_GRAYSCALE, IDS_DENSITYTYPE_OPACITY,
												IDS_DENSITYTYPE_GRAYSCALEOPACITY, IDS_DENSITYTYPE_RED, 
												IDS_DENSITYTYPE_GREEN, IDS_DENSITYTYPE_BLUE,
					p_vals,			kPositionOnObject_densityType_grayscale,
									kPositionOnObject_densityType_opacity,
									kPositionOnObject_densityType_grayscaleOpacity,
									kPositionOnObject_densityType_red,
									kPositionOnObject_densityType_green,
									kPositionOnObject_densityType_blue,
			end,
		// use sub-material for density
			kPositionOnObject_useSubMtl,	_T("Use_Sub_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESUBMTL,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESUBMAT,
			end,
		// Sub-Material ID
			kPositionOnObject_mtlID, _T("Material_ID"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MATERIALID,
			// optional tagged param specs
					p_default,		1,
					p_range,		1, 32000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_MATID,	IDC_MATIDSPIN,	1.0f,
			end,
		// apart placement
			kPositionOnObject_apartPlacement,	_T("Apart_Placement"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_APARTPLACEMENT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX, IDC_APART,
			end,
		// apart distance
			kPositionOnObject_distance,	_T("Apart_Distance"),
									TYPE_WORLD,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_APARTDISTANCE,
			// optional tagged param specs
					p_default,		1.f,
					p_range,		0.f, 999999999.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_UNIVERSE,
									IDC_DISTANCE, IDC_DISTANCESPIN, SPIN_AUTOSCALE,
			end,
		// distinct points only
			kPositionOnObject_distinctOnly,	_T("Distinct_Points_Only"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_DISTINCTPOINTSONLY,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX, IDC_DISTINCT,
			end,
		// total number of of distinct points
			kPositionOnObject_distinctTotal, _T("Total_Distinct_Points"),
									TYPE_INT,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_TOTALDISTINCT,
			// optional tagged param specs
					p_default,		10,
					p_range,		1, 10000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_TOTALDISTINCT,	IDC_TOTALDISTINCTSPIN,	1.0f,
			end,
		// remove particles with invalid location
			kPositionOnObject_delete,	_T("Delete"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_DELETE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX, IDC_DELETEPARTICLES,
			end,
		// random seed
			kPositionOnObject_randomSeed,	_T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345, // initial random seed
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,
		// attempts maximum
			kPositionOnObject_attempts,	_T("Maximum_Number_of_Attempts"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_ATTEMPTSMAX,
			// optional tagged param specs
					p_default,		100, 
					p_range,		1, 1000000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_ATTEMPTSMAX,	IDC_ATTEMPTSMAXSPIN,	1.0f,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc positionOnObject_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorPositionOnObjectDesc, FP_MIXIN, 
				
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

FPInterfaceDesc positionOnObject_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorPositionOnObjectDesc, FP_MIXIN,

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

FPInterfaceDesc positionOnObject_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorPositionOnObjectDesc, FP_MIXIN,

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

BOOL PFOperatorPositionOnObjectDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
												WPARAM wParam, LPARAM lParam )
{
	IParamBlock2* pblock;
	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kPositionOnObject_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		pblock = (map != NULL) ? map->GetParamBlock() : NULL;
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			if (pblock) pblock->NotifyDependents( FOREVER, PART_OBJ, kPositionOnObject_RefMsg_NewRand );
			break;
		case IDC_PICKNODE:
			if (pblock) pblock->NotifyDependents( FOREVER, 0, kPositionOnObject_RefMsg_ResetValidatorAction );
			break;
		case IDC_BYLIST:
			if (pblock) pblock->NotifyDependents( FOREVER, 0, kPositionOnObject_RefMsg_ListSelect );
			break;
		case kPositionOnObject_message_lock:
			if (pblock)	UpdateInheritSpeedDlg( hWnd, pblock->GetInt(kPositionOnObject_lock, t),
													pblock->GetInt(kPositionOnObject_inherit, t) );
			break;
		case kPositionOnObject_message_type:
			UpdateTypeDlg( hWnd, pblock );
			break;
		case kPositionOnObject_message_objects:
			if (pblock) UpdateObjectsNumDlg( hWnd, pblock->Count(kPositionOnObject_emitters) );
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorPositionOnObjectDlgProc::UpdateInheritSpeedDlg( HWND hWnd, BOOL lock, BOOL inherit )
{
	EnableWindow( GetDlgItem( hWnd, IDC_INHERIT ), !lock );
	bool useInherit = ((lock == 0) && (inherit != 0));
	EnableWindow( GetDlgItem( hWnd, IDC_MULTIPLIERTEXT ), useInherit);
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_MULTIPLIERSPIN ) );
	spin->Enable(useInherit);
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_VARIATIONTEXT ), useInherit);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_VARIATIONSPIN ) );
	spin->Enable(useInherit);
	ReleaseISpinner( spin );
}

void PFOperatorPositionOnObjectDlgProc::UpdateTypeDlg( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;

	int type = pblock->GetInt(kPositionOnObject_type, 0);
	int useOffset = pblock->GetInt(kPositionOnObject_useOffset, 0);
	int useNonUniform = pblock->GetInt(kPositionOnObject_useNonUniform, 0);
	int useSubMtl = pblock->GetInt(kPositionOnObject_useSubMtl, 0);
	int useApart = pblock->GetInt(kPositionOnObject_apartPlacement, 0);
	int useDistinct = pblock->GetInt(kPositionOnObject_distinctOnly, 0);

	bool enableOffset = ((type != kPositionOnObject_locationType_pivot) 
						&& (type != kPositionOnObject_locationType_volume));
	bool enableMinMax = enableOffset && (useOffset != 0);
	EnableWindow( GetDlgItem( hWnd, IDC_SURFACEOFFSET), enableOffset );
	EnableWindow( GetDlgItem( hWnd, IDC_OFFSETMINTEXT), enableMinMax );
	EnableWindow( GetDlgItem( hWnd, IDC_OFFSETMAXTEXT), enableMinMax );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_OFFSETMINSPIN ) );
	spin->Enable(enableMinMax);
	ReleaseISpinner(spin);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_OFFSETMAXSPIN ) );
	spin->Enable(enableMinMax);
	ReleaseISpinner(spin);

	bool enableDensity = (type != kPositionOnObject_locationType_pivot);
	EnableWindow( GetDlgItem( hWnd, IDC_NONUNIFORM), enableDensity );
	EnableWindow( GetDlgItem( hWnd, IDC_DENSITYTYPE), enableDensity && useNonUniform );
	EnableWindow( GetDlgItem( hWnd, IDC_USESUBMAT), enableDensity && useNonUniform );
	EnableWindow( GetDlgItem( hWnd, IDC_MATIDTEXT), enableDensity && useNonUniform && useSubMtl );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_MATIDSPIN ) );
	spin->Enable( enableDensity && useNonUniform && useSubMtl );
	ReleaseISpinner(spin);
	EnableWindow( GetDlgItem( hWnd, IDC_APART), enableDensity );
	EnableWindow( GetDlgItem( hWnd, IDC_DISTANCETEXT), enableDensity && useApart );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DISTANCESPIN ) );
	spin->Enable(enableDensity && useApart);
	ReleaseISpinner(spin);
	EnableWindow( GetDlgItem( hWnd, IDC_DISTINCT), enableDensity );
	EnableWindow( GetDlgItem( hWnd, IDC_TOTALTEXT), enableDensity && useDistinct );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_TOTALDISTINCTSPIN ) );
	spin->Enable(enableDensity && useDistinct);
	ReleaseISpinner(spin);
}

void PFOperatorPositionOnObjectDlgProc::UpdateObjectsNumDlg( HWND hWnd, int num )
{
	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_REMOVENODE ) );
	button->Enable( num > 0 );
	ReleaseICustButton( button );
}



} // end of namespace PFActions