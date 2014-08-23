/**********************************************************************
 *<
	FILE:			PFOperatorSpeedSurfaceNormals_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SpeedSurfaceNormals Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSpeedSurfaceNormals_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSpeedSurfaceNormalsDesc ThePFOperatorSpeedSurfaceNormalsDesc;

// Dialog Proc
PFOperatorSpeedSurfaceNormalsDlgProc ThePFOperatorSpeedSurfaceNormalsDlgProc;

// SpeedSurfaceNormals Operator param block
ParamBlockDesc2 speedSurfaceNormals_paramBlockDesc 
(
	// required block spec
		kSpeedSurfaceNormals_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSpeedSurfaceNormalsDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpeedSurfaceNormals_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPEEDSURFACENORMALS_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSpeedSurfaceNormalsDlgProc,
	// required param specs
		// duration type: instant or continuous
			kSpeedBySurface_type,	_T("Speed_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SPEEDTYPE,
			// optional tagged param specs
					p_default,		kSpeedBySurface_type_once,
					p_ui,			TYPE_INTLISTBOX,	IDC_SPEEDTYPE,	kSpeedBySurface_type_num,
												IDS_SPEEDTYPE_ONCE, IDS_SPEEDTYPE_CONTINUOUS,
					p_vals,			kSpeedBySurface_type_once,
									kSpeedBySurface_type_continuous,
			end,
		// set speed value
			kSpeedBySurface_setSpeed,	_T("Set_Speed_Magnitude"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SETSPEEDMAGNITUDE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESPEEDVALUE,
			end,
		// speed value
			kSpeedBySurface_speed, _T("Speed_Value"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SPEEDVALUE,
			// optional tagged param specs
					p_default,		300.0f,
					p_range,		-99999999.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_SPEED, IDC_SPEEDSPIN, SPIN_AUTOSCALE,
			end,
		// speed variation
			kSpeedBySurface_speedVariation, _T("Speed_Variation"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SPEEDVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_VARIATION, IDC_VARIATIONSPIN, SPIN_AUTOSCALE,
			end,
		// surface objects
			kSpeedBySurface_objects, _T(""),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE,	0,
						p_ui,	TYPE_NODELISTBOX, IDC_OBJECTLIST,IDC_PICKNODE,0,IDC_REMOVENODE,
			end,
		// surface objects for maxscript manipulation
			kSpeedBySurface_objectsMaxscript, _T("Surface_Objects"),
						TYPE_INODE_TAB, 0,			
						P_VARIABLE_SIZE|P_NO_REF,	IDS_SURFACEOBJECTS,
			end,
		// animated shape
			kSpeedBySurface_animated,	_T("Animated_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ANIMATEDSHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANIMATED,
			end,
		// subframe sampling
			kSpeedBySurface_subframe,	_T("Subframe_Sampling"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SUBFRAMESAMPLING,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SUBFRAME,
			end,
		// use material multiplier/set
			kSpeedBySurface_speedByMtl,	_T("Set_Speed_By_Surface_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SETSPEEDBYSURFACEMTL,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_BYMATERIAL,
			end,
		// speedMaterial type
			kSpeedBySurface_materialType,	_T("Material_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MATERIALTYPE,
			// optional tagged param specs
					p_default,		kSpeedBySurface_materialType_grayscale,
					p_ui,			TYPE_INTLISTBOX,	IDC_MATERIALTYPE,	kSpeedBySurface_materialType_num,
												IDS_MATERIALTYPE_GRAYSCALE, IDS_MATERIALTYPE_LUMCENTER,
												IDS_MATERIALTYPE_RGBASWORLD, IDS_MATERIALTYPE_RGBASLOCAL,
					p_vals,			kSpeedBySurface_materialType_grayscale,
									kSpeedBySurface_materialType_lumCenter,
									kSpeedBySurface_materialType_worldRGB,
									kSpeedBySurface_materialType_localRGB,
			end,
		// use sub-material for speed material
			kSpeedBySurface_useSubMtl,	_T("Use_Sub_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESUBMTL,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESUBMAT,
			end,
		// Sub-Material ID
			kSpeedBySurface_mtlID, _T("Material_ID"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MATERIALID,
			// optional tagged param specs
					p_default,		1,
					p_range,		1, 32000,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_MATID,	IDC_MATIDSPIN,	1.0f,
			end,
		// direction type
			kSpeedBySurface_directionType,	_T("Direction_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_DIRECTIONTYPE,
			// optional tagged param specs
					p_default,		kSpeedBySurface_directionType_normals,
					p_ui,			TYPE_INTLISTBOX,	IDC_DIRECTION,	kSpeedBySurface_directionType_num,
												IDS_DIRECTIONTYPE_NORMALS, IDS_DIRECTIONTYPE_OUTOFSURFACE,
												IDS_DIRECTIONTYPE_PARALLEL,
					p_vals,			kSpeedBySurface_directionType_normals,
									kSpeedBySurface_directionType_outOfSurface,
									kSpeedBySurface_directionType_parallel,
			end,
		// divergence
			kSpeedBySurface_divergence,_T("Divergence"),
									TYPE_ANGLE,
									P_ANIMATABLE | P_RESET_DEFAULT,
									IDS_DIVERGENCE,
			// optional tagged param specs
					p_default,		0.f,
					p_range,		0.f, 180.f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,
									IDC_DIVERGENCE, IDC_DIVERGENCESPIN, 0.5f,
			end,
		// acceleration maximum
			kSpeedBySurface_accelLimit, _T("Acceleration_Limit"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_ACCELERATIONLIMIT,
			// optional tagged param specs
					p_default,		100.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ACCELLIMIT, IDC_ACCELLIMITSPIN, SPIN_AUTOSCALE,
			end,
		// unlimited range of speed control
			kSpeedBySurface_unlimitedRange,	_T("Unlimited_Range"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_UNLIMITEDRANGE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_UNLIMITEDRANGE,
			end,
		// range
			kSpeedBySurface_range, _T("Range"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_RANGE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_RANGE, IDC_RANGESPIN, SPIN_AUTOSCALE,
			end,
		// falloff zone size
			kSpeedBySurface_falloff, _T("Falloff"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_FALLOFF,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_FALLOFF, IDC_FALLOFFSPIN, SPIN_AUTOSCALE,
			end,
		// sync type
			kSpeedBySurface_syncBy, _T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kSpeedBySurface_syncBy_time,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kSpeedBySurface_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kSpeedBySurface_syncBy_time,
									kSpeedBySurface_syncBy_age,
									kSpeedBySurface_syncBy_event,
			end,
		// seed
			kSpeedBySurface_seed,		_T("Random_Seed"),
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
FPInterfaceDesc speedSurfaceNormals_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSpeedSurfaceNormalsDesc, FP_MIXIN, 
				
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

FPInterfaceDesc speedSurfaceNormals_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSpeedSurfaceNormalsDesc, FP_MIXIN,

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

FPInterfaceDesc speedSurfaceNormals_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSpeedSurfaceNormalsDesc, FP_MIXIN,

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

BOOL PFOperatorSpeedSurfaceNormalsDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock = NULL;

	switch(msg) {
		case WM_INITDIALOG:
			// Send the message to notify the initialization of dialog
			if (map != NULL) pblock = map->GetParamBlock();
			if (pblock != NULL)	pblock->NotifyDependents( FOREVER, (PartID)map, kSpeedSurfaceNormals_RefMsg_InitDlg );
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			if (map != NULL) pblock = map->GetParamBlock();
			switch(LOWORD(wParam)) {
				case IDC_NEW:
					if (pblock)
						pblock->NotifyDependents(FOREVER, PART_OBJ, kSpeedSurfaceNormals_RefMsg_NewRand);
					break;
				case IDC_PICKNODE:
					if (pblock)
						pblock->NotifyDependents( FOREVER, 0, kSpeedSurfaceNormals_RefMsg_ResetValidatorAction );
					break;
				case IDC_BYLIST:
					if (pblock)
						pblock->NotifyDependents( FOREVER, 0, kSpeedSurfaceNormals_RefMsg_ListSelect );
					break;
				case kSpeedSurfaceNormals_message_update:
					UpdateDlg( hWnd, pblock);
					break;
			}
			break;
	}

	return FALSE;
}

void PFOperatorSpeedSurfaceNormalsDlgProc::UpdateDlg( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;

	int type = pblock->GetInt(kSpeedBySurface_type, 0);
	int setSpeed = pblock->GetInt(kSpeedBySurface_setSpeed, 0);
	int speedByMtl = pblock->GetInt(kSpeedBySurface_speedByMtl, 0);
	int mtlType = pblock->GetInt(kSpeedBySurface_materialType, 0);
	int useSubMtl = pblock->GetInt(kSpeedBySurface_useSubMtl, 0);
	int direction = pblock->GetInt(kSpeedBySurface_directionType, 0);
	int unlimitedRange = pblock->GetInt(kSpeedBySurface_unlimitedRange, 0);
	int numObjects = pblock->Count(kSpeedBySurface_objects);

	EnableWindow( GetDlgItem( hWnd, IDC_USESPEEDVALUE ), type == kSpeedBySurface_type_continuous);
	int useSpeed = setSpeed || (type == kSpeedBySurface_type_once);
	EnableWindow( GetDlgItem( hWnd, IDC_SPEEDTEXT ), useSpeed);
	EnableWindow( GetDlgItem( hWnd, IDC_VARIATIONTEXT ), useSpeed);
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_SPEEDSPIN ) );
	spin->Enable( useSpeed );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_VARIATIONSPIN ) );
	spin->Enable( useSpeed );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_MATERIALTYPE ), speedByMtl );
	EnableWindow( GetDlgItem( hWnd, IDC_USESUBMAT ), speedByMtl );
	bool enableSubMtl = (speedByMtl != 0) && (useSubMtl != 0);
	EnableWindow( GetDlgItem( hWnd, IDC_MATIDTEXT ), enableSubMtl);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_MATIDSPIN ) );
	spin->Enable( enableSubMtl );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_DIRECTION ), (speedByMtl == 0) || (mtlType == kSpeedBySurface_materialType_grayscale) );
	bool useDiv = (type == kSpeedBySurface_type_once);
	EnableWindow( GetDlgItem( hWnd, IDC_DIVERGENCETEXT ), useDiv);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DIVERGENCESPIN ) );
	spin->Enable( useDiv );
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_SUBFRAME ), useDiv );

	bool useCont = (type == kSpeedBySurface_type_continuous);
	EnableWindow( GetDlgItem( hWnd, IDC_CONTINUOUSFRAME ), useCont);
	EnableWindow( GetDlgItem( hWnd, IDC_ACCELLIMITTEXT ), useCont);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_ACCELLIMITSPIN ) );
	spin->Enable( useCont );
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_UNLIMITEDRANGE ), useCont);
	bool useRange = useCont && (unlimitedRange == 0);
	EnableWindow( GetDlgItem( hWnd, IDC_RANGETEXT ), useRange);
	EnableWindow( GetDlgItem( hWnd, IDC_FALLOFFTEXT ), useRange);
	spin = GetISpinner( GetDlgItem( hWnd, IDC_RANGESPIN ) );
	spin->Enable( useRange );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_FALLOFFSPIN ) );
	spin->Enable( useRange );
	ReleaseISpinner( spin );

	ICustButton* button = GetICustButton( GetDlgItem( hWnd, IDC_REMOVENODE ) );
	button->Enable( numObjects > 0 );
	ReleaseICustButton( button );
}



} // end of namespace PFActions