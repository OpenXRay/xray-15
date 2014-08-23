/**********************************************************************
 *<
	FILE:			PFOperatorMarkShape_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for MarkShape Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-29-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorMarkShape_ParamBlock.h"

#include "PFOperatorMarkShape.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorMarkShapeDesc		ThePFOperatorMarkShapeDesc;
// Dialog Proc
PFOperatorMarkShapeDlgProc		ThePFOperatorMarkShapeDlgProc;
// PB2Value Validator
PFOperatorMarkShapePBValidator	ThePFOperatorMarkShapePBValidator;
// PB2Value Accessor
PFOperatorMarkShapePBAccessor	ThePFOperatorMarkShapePBAccessor;


// MarkShape Operator param block
ParamBlockDesc2 markShape_paramBlockDesc 
(
	// required block spec
		kMarkShape_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorMarkShapeDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kMarkShape_reference_pblock,
	// auto ui parammap specs : none
		IDD_MARKSHAPE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorMarkShapeDlgProc,
	// required param specs
		// contact object 
			kMarkShape_contactObj,	_T("Contact_Object"),
									TYPE_INODE,
									0,
									IDS_CONTACTOBJECT,
			// optional tagged param specs
					p_ui,			TYPE_PICKNODEBUTTON, IDC_PICK,
					p_validator,	&ThePFOperatorMarkShapePBValidator,
					p_accessor,		&ThePFOperatorMarkShapePBAccessor,
					p_prompt,		IDS_PICKCONTACTOBJECT,
			end,
		// deform mark shape to surface animation
			kMarkShape_surfAnimate,	_T("Surface_Animation"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SURFACEAMINATION,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_VERTEXANIMATED,
			end,
		// align mark shape according to direction
			kMarkShape_alignTo,		_T("Align_To"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_ALIGNTO,
			// optional tagged param specs
					p_default,		kMarkShape_alignTo_speed,
					p_ui,			TYPE_INTLISTBOX,	IDC_DIRECTION,	kMarkShape_alignTo_num,
												IDS_ALIGNTO_SPEED, IDS_ALIGNTO_PARTICLEX,
												IDS_ALIGNTO_PARTICLEY, IDS_ALIGNTO_PARTICLEZ,
												IDS_ALIGNTO_RANDOM,
					p_vals,			kMarkShape_alignTo_speed,
									kMarkShape_alignTo_particleX,
									kMarkShape_alignTo_particleY,
									kMarkShape_alignTo_particleZ,
									kMarkShape_alignTo_random,
			end,
		// divergence of alignment direction
			kMarkShape_divergence,	_T("Divergence"),
									TYPE_FLOAT,
									P_RESET_DEFAULT|P_ANIMATABLE,
									IDS_DIVERGENCE,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 180.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_DIVERGENCE, IDC_DIVERGENCESPIN, 1.0f,
			end,
		// size space (world/local)
			kMarkShape_sizeSpace,	_T("Size_Space"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SIZESPACE,
			// optional tagged param specs
					p_default,		kMarkShape_sizeSpace_world,
					p_range,		kMarkShape_sizeSpace_world, kMarkShape_sizeSpace_local,
					p_ui,			TYPE_RADIO, kMarkShape_sizeSpace_num, IDC_WORLD, IDC_LOCAL,
			end,
		// width in world space
			kMarkShape_width,		_T("Width"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_WIDTH,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_WIDTH, IDC_WIDTHSPIN, 1.0f,
			end,
		// length in world space
			kMarkShape_length,		_T("Length"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_LENGTH,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_LENGTH, IDC_LENGTHSPIN, 1.0f,
			end,
		// inheritance of size in local space
			kMarkShape_inheritedScale, _T("Inherited_Scale"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_INHERITEDSCALE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_POS_FLOAT, IDC_INHERITED, IDC_INHERITEDSPIN, 1.0f,
			end,
		// size variation
			kMarkShape_variation, _T("Variation"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_SIZEVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,	IDC_VARIATION, IDC_VARIATIONSPIN, 1.0f,
			end,
		// impact angle distortion
			kMarkShape_angleDistortion,	_T("Angle_Distortion"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ANGLEDISTORTION,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_IMPACTDISTORTION,
			end,
		// maximum distortion percentage
			kMarkShape_maxDistortion, _T("Max_Distortion"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_MAXDISTORTION,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		100.0f, 1000000.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_POS_FLOAT,	IDC_DISTORMAX, IDC_DISTORMAXSPIN, SPIN_AUTOSCALE,
			end,
		// mark shape type
			kMarkShape_markType,	_T("Mark_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_MARKTYPE,
			// optional tagged param specs
					p_default,		kMarkShape_markType_rectangle,
					p_range,		kMarkShape_markType_rectangle, kMarkShape_markType_boxIntersect,
					p_ui,			TYPE_RADIO, kMarkShape_markType_num, IDC_RECTANGLE, IDC_BOX,
			end,
		// box height
			kMarkShape_height,		_T("Height"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_HEIGHT,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_HEIGHT, IDC_HEIGHTSPIN, 1.0f,
			end,
		// allow multiple elements for a particle
			kMarkShape_multiElements, _T("Multi_Elements"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_MULTIELEMENTS,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_MULTISHAPE,
			end,
		// pivot offset percentage
			kMarkShape_pivotOffset, _T("Pivot_Offset"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_PIVOTOFFSET,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		-50.0f, 50.0f,
					p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT,	IDC_PIVOTLENGTH, IDC_PIVOTLENGTHSPIN, 1.0f,
			end,
		// surface offset
			kMarkShape_surfaceOffset, _T("Surface_Offset"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_SURFACEOFFSET,
			// optional tagged param specs
					p_default,		0.001f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_CLEARANCE, IDC_CLEARANCESPIN, SPIN_AUTOSCALE,
			end,
		// surface offset variation
			kMarkShape_surfaceOffsetVar, _T("Surface_Offset_Variation"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_SURFACEOFFSETVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_OFFSETVAR, IDC_OFFSETVARSPIN, SPIN_AUTOSCALE,
			end,
		// surface offset
			kMarkShape_vertexNoise, _T("Vertex_Noise"),
									TYPE_WORLD,
									P_RESET_DEFAULT,
									IDS_VERTEXNOISE,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_VERTEXNOISE, IDC_VERTEXNOISESPIN, 0.001f,
			end,
		// constant update for box intersection cut-out
			kMarkShape_constantUpdate, _T("Continuous_Update"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_CONTINUOUSUPDATE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_CONTUPDATE,
			end,
		// material
			kMarkShape_materialObsolete,	_T(""),   //_T("Material"), --010803  az: hide from MXS exposure
									TYPE_MTL,
									0,
									IDS_NOTHING, // IDS_MATERIAL,
			end,
		// number of sub-materials
			kMarkShape_numSubMtlsObsolete, _T(""), //_T("Num_SubMtls_Obsolete"),   --010803  az: hide from MXS exposure
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_NUMFSUBMTLS,
			// optional tagged param specs
					p_default,		kMarkShape_minNumMtls,
					p_range,		kMarkShape_minNumMtls, 999999999,
			end,
		// number of sub-materials per frame
			kMarkShape_subsPerFrameObsolete, _T(""), //_T("Subs_Per_Frame_Obsolote") --010803  az: hide from MXS exposure
									TYPE_FLOAT,
									P_RESET_DEFAULT,
									IDS_NOTHING, // IDS_SUBSPERFRAME,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		kMarkShape_minRate, kMarkShape_maxRate,
			end,
		// generate mapping coordinates
			kMarkShape_generateMapping,		_T("Generate_Mapping_Coords"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_GENERATEMAPPINGCOORDS,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_GENERATEMAPPING,
			end,
		// random seed
			kMarkShape_randomSeed, _T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345, // initial random seed
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc markShape_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorMarkShapeDesc, FP_MIXIN, 
				
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

FPInterfaceDesc markShape_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorMarkShapeDesc, FP_MIXIN,

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

FPInterfaceDesc markShape_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorMarkShapeDesc, FP_MIXIN,

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

BOOL PFOperatorMarkShapeDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
	BOOL alignTo, angleDistortion;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kMarkShape_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kMarkShape_RefMsg_NewRand );
			break;
		case kMarkShape_message_alignTo:
			alignTo = map->GetParamBlock()->GetInt(kMarkShape_alignTo, t);
			angleDistortion = map->GetParamBlock()->GetInt(kMarkShape_angleDistortion, t);
			UpdateDivergenceDlg( hWnd, alignTo );
			UpdateAngleDistortionDlg( hWnd, alignTo, angleDistortion );
			break;
		case kMarkShape_message_sizeSpace:
			UpdateSizeDlg( hWnd, map->GetParamBlock()->GetInt(kMarkShape_sizeSpace, t) );
			break;
		case kMarkShape_message_angleDistort:
			alignTo = map->GetParamBlock()->GetInt(kMarkShape_alignTo, t);
			angleDistortion = map->GetParamBlock()->GetInt(kMarkShape_angleDistortion, t);
			UpdateAngleDistortionDlg( hWnd, alignTo, angleDistortion );
			break;
		case kMarkShape_message_markType:
			UpdateBoxIntersectionDlg( hWnd, map->GetParamBlock()->GetInt(kMarkShape_markType, t),
											map->GetParamBlock()->GetInt(kMarkShape_constantUpdate, t));
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorMarkShapeDlgProc::UpdateDivergenceDlg( HWND hWnd, int alignTo )
{
	ISpinnerControl *spin;
	bool enable = (alignTo != kMarkShape_alignTo_random);

	EnableWindow( GetDlgItem( hWnd, IDC_DIVERGENCETEXT ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_DIVERGENCE ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DIVERGENCESPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
}

void PFOperatorMarkShapeDlgProc::UpdateSizeDlg( HWND hWnd, int sizeSpace )
{
	ISpinnerControl *spin;
	bool inWorld = (sizeSpace == kMarkShape_sizeSpace_world);
	bool inLocal = (sizeSpace == kMarkShape_sizeSpace_local);
	
	// enable/disable width & length spinners
	EnableWindow( GetDlgItem( hWnd, IDC_WIDTHTEXT ), inWorld );
	EnableWindow( GetDlgItem( hWnd, IDC_WIDTH ), inWorld );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_WIDTHSPIN ) );
	spin->Enable( inWorld );
	ReleaseISpinner( spin );
	EnableWindow( GetDlgItem( hWnd, IDC_LENGTHTEXT ), inWorld );
	EnableWindow( GetDlgItem( hWnd, IDC_LENGTH ), inWorld );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_LENGTHSPIN ) );
	spin->Enable( inWorld );
	ReleaseISpinner( spin );
	// enable/disable inheritance size spinner
	EnableWindow( GetDlgItem( hWnd, IDC_INHERITEDTEXT ), inLocal );
	EnableWindow( GetDlgItem( hWnd, IDC_INHERITED ), inLocal );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_INHERITEDSPIN ) );
	spin->Enable( inLocal );
	ReleaseISpinner( spin );
}

void PFOperatorMarkShapeDlgProc::UpdateAngleDistortionDlg( HWND hWnd, int alignTo, BOOL angleDistortion )
{
	ISpinnerControl *spin;
	bool okDistortion = (alignTo == kMarkShape_alignTo_speed);
	bool maxDistortion = (okDistortion && angleDistortion);

	// enable/disable angle distortion checkbox
	EnableWindow( GetDlgItem( hWnd, IDC_IMPACTDISTORTION ), okDistortion );
	// enable/disable max distortion spinner
	EnableWindow( GetDlgItem( hWnd, IDC_DISTORMAXTEXT ), maxDistortion );
	EnableWindow( GetDlgItem( hWnd, IDC_DISTORMAX ), maxDistortion );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_DISTORMAXSPIN ) );
	spin->Enable( maxDistortion );
	ReleaseISpinner( spin );
}

void PFOperatorMarkShapeDlgProc::UpdateBoxIntersectionDlg( HWND hWnd, int markType, int constantUpdate )
{
	ISpinnerControl *spin;
	bool enable = (markType == kMarkShape_markType_boxIntersect);

	// enable/disable box height spinner
	EnableWindow( GetDlgItem( hWnd, IDC_HEIGHTTEXT ), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_HEIGHT ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_HEIGHTSPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
	// enable/disable 'Allow Multiple Elements' checkbox
	EnableWindow( GetDlgItem( hWnd, IDC_MULTISHAPE ), enable );
	// enable/disable 'Continuous Update' checkbox
	EnableWindow( GetDlgItem( hWnd, IDC_CONTUPDATE ), enable );
	// enable/disable 'Vertex Noise' parameter
	enable = enable && (constantUpdate == 0);
	EnableWindow( GetDlgItem( hWnd, IDC_VERTEXNOISETEXT ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_VERTEXNOISESPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
}

//+--------------------------------------------------------------------------+
//|							PB2Value Validator								 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorMarkShapePBValidator::Validate(PB2Value& v)
{
	INode* iNode = (INode*)v.r;
	if (iNode == NULL) return NULL;
	TimeValue t = GetCOREInterface()->GetTime();
	Tab<INode*> stack;

	if (iNode->IsGroupMember())		// get highest closed group node
		iNode = GetHighestClosedGroupNode(iNode);

	stack.Append(1, &iNode, 10);
	while (stack.Count())
	{
		INode *node = stack[stack.Count()-1];
		stack.Delete(stack.Count()-1, 1);

		Object *obj = node->EvalWorldState(t).obj;
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
			return TRUE;

		// add children to the stack
		for (int i = 0; i < node->NumberOfChildren(); i++) {
			INode *childNode = node->GetChildNode(i);
			if (childNode)	stack.Append(1, &childNode, 10);
		}
	}
	return FALSE;
}

INode* PFOperatorMarkShapePBValidator::GetHighestClosedGroupNode(INode *iNode)
{
	if (iNode == NULL) return NULL;

	INode *node = iNode;
	while (node->IsGroupMember()) {
		node = node->GetParentNode();
		while (!(node->IsGroupHead())) node = node->GetParentNode();
		if (!(node->IsOpenGroupHead())) iNode = node;
	}
	return iNode;
}


//+--------------------------------------------------------------------------+
//|							PB2Value Accessor								 |
//+--------------------------------------------------------------------------+

void PFOperatorMarkShapePBAccessor::Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
{
	switch (id)
	{
	case kMarkShape_contactObj:
		if (v.r) {
			INode *iNode = (INode*)v.r;
			if (iNode->IsGroupMember())		// get highest closed group node
				v.r = PFOperatorMarkShapePBValidator::GetHighestClosedGroupNode(iNode);
		}
		break;
	}
}


} // end of namespace PFActions