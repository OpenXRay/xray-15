/**********************************************************************
 *<
	FILE:			PFOperatorInstanceShape_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for InstanceShape Operator (definition)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-04-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorInstanceShape_ParamBlock.h"

#include "PFOperatorInstanceShape.h"
#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorInstanceShapeDesc ThePFOperatorInstanceShapeDesc;
// Dialog Proc
PFOperatorInstanceShapeDlgProc	ThePFOperatorInstanceShapeDlgProc;
// PB2Value Validator
PFOperatorInstanceShapeMXSValidator	ThePFOperatorInstanceShapeMSXValidator;

// InstanceShape Operator param block
ParamBlockDesc2 instanceShape_paramBlockDesc 
(
	// required block spec
		kInstanceShape_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorInstanceShapeDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kInstanceShape_reference_pblock,
	// auto ui parammap specs : none
		IDD_INSTANCESHAPE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorInstanceShapeDlgProc,
	// required param specs
		// shape object for maxscript manipulation
			kInstanceShape_objectMaxscript,	_T("Shape_Object"),
									TYPE_INODE,
									0,
									IDS_SHAPEOBJECT,
			// optional tagged param specs
					p_validator,	&ThePFOperatorInstanceShapeMSXValidator,
			end,
		// separate group members
			kInstanceShape_sepGroup, _T("Group_Members"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_GROUPMEMBERS,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_GROUPMEMBERS,
			end,
		// separate object and children
			kInstanceShape_sepHierarchy, _T("Object_And_Children"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_OBJECTANDCHILDREN,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_CHILDREN,
			end,
		// separate object elements
			kInstanceShape_sepElements, _T("Object_Elements"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_OBJECTELEMENTS,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ELEMENTS,
			end,
		// shape scale
			kInstanceShape_scale,	_T("Scale_Value"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_SHAPESCALE,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 100000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_PARTICLESCALE, IDC_PARTICLESCALESPIN, 1.0f,
			end,
		// shape scale variation
			kInstanceShape_variation, _T("Variation"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_SHAPEVARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 100.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_VARIATION, IDC_VARIATIONSPIN, 1.0f,
			end,
		// acquire mapping data
			kInstanceShape_mapping,	_T("Acquire_Mapping"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_MAPPING,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_MAPPING,
			end,
		// acquire material data
			kInstanceShape_material, _T("Acquire_Material"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_MATERIAL,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_MATERIAL,
			end,
		// multi-shape random order
			kInstanceShape_randomShape, _T("Random_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_RANDOMSHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ORDER,
			end,
		// animated shape
			kInstanceShape_animatedShape, _T("Animated_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ANIMATEDSHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ANIMATED,
			end,
		// acquire current shape
			kInstanceShape_acquireShape, _T("Acquire_Shape"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ACQUIRESHAPE,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_ACQUIRE,
			end,
		// sync type
			kInstanceShape_syncType, _T("Sync_Type"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_SYNCTYPE,
			// optional tagged param specs
					p_default,		kInstanceShape_syncBy_absoluteTime,
					p_ui,			TYPE_INTLISTBOX,	IDC_SYNC,	kInstanceShape_syncBy_num,
												IDS_SYNCBY_ABSOLUTE, IDS_SYNCBY_PARTICLE,
												IDS_SYNCBY_EVENT,
					p_vals,			kInstanceShape_syncBy_absoluteTime,
									kInstanceShape_syncBy_particleAge,
									kInstanceShape_syncBy_eventStart,
			end,
		// add random offset to the synchronization
			kInstanceShape_syncRandom, _T("Add_Random_Offset"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_ADDRANDOMOFFSET,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SYNCRANDOM,
			end,
		// random offset
			kInstanceShape_randomOffset, _T("Random_Offset"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMOFFSET,
			// optional tagged param specs
					p_default,		4800,
					p_range,		0, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_TIME,	IDC_SYNCOFFSET,	IDC_SYNCOFFSETSPIN,	80.0f,
			end,
		// random seed
			kInstanceShape_randomSeed, _T("Random_Seed"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_RANDOMSEED,
			// optional tagged param specs
					p_default,		12345, // initial random seed
					p_range,		1, 999999999,
					p_ui,			TYPE_SPINNER,	EDITTYPE_POS_INT,	IDC_SEED,	IDC_SEEDSPIN,	1.0f,
			end,
		// set scale channel
			kInstanceShape_setScale, _T("Set_Scale"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_SETSCALE,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_SETSCALE,
			end,
		// list of all reference objects for shape instance
			kInstanceShape_objectList,		_T(""),
					TYPE_INODE_TAB,		0,	P_VARIABLE_SIZE,	0,//string name,			
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc instanceShape_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorInstanceShapeDesc, FP_MIXIN, 
				
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
			IPFAction::kGetMaterial, _T("getMaterial"), 0, TYPE_MTL, 0, 0,
			IPFAction::kSetMaterial, _T("setMaterial"), 0, TYPE_bool, 0, 1,
				_T("material"), 0, TYPE_MTL,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc instanceShape_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorInstanceShapeDesc, FP_MIXIN,

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

FPInterfaceDesc instanceShape_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorInstanceShapeDesc, FP_MIXIN,

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

BOOL PFOperatorInstanceShapeDlgProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg,
											   WPARAM wParam, LPARAM lParam )
{
	ICustButton *iBut;
	PFOperatorInstanceShape *op;
	INode* obj;
	TSTR name;
	BOOL separateAny, animatedShape, addRandomOffset, setScale;

	switch ( msg )
	{
	case WM_INITDIALOG:
		// Set ICustButton properties
		iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
		iBut->SetType(CBT_CHECK);
		iBut->SetHighlightColor(GREEN_WASH);
		ReleaseICustButton(iBut);
		// Send the message to notify the initialization of dialog
		map->GetParamBlock()->NotifyDependents( FOREVER, (PartID)map, kInstanceShape_RefMsg_InitDlg );
		break;
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDC_PICK:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kInstanceShape_RefMsg_PickObj );
			break;
		case IDC_UPDATESHAPE:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kInstanceShape_RefMsg_UpdateShape );
			break;
		case IDC_NEW:
			map->GetParamBlock()->NotifyDependents( FOREVER, PART_OBJ, kInstanceShape_RefMsg_NewRand );
			break;
		case kInstanceShape_message_name:
			op = reinterpret_cast <PFOperatorInstanceShape *> (lParam);
			obj = (op) ? const_cast <INode*> (op->object()) : NULL;
			name = (obj) ? obj->GetName() : GetString(IDS_NONE);
			UpdateObjectNameDlg(hWnd, name);
			break;
		case kInstanceShape_message_type:
			op = reinterpret_cast <PFOperatorInstanceShape *> (lParam);
			separateAny = map->GetParamBlock()->GetInt(kInstanceShape_sepGroup, t) ||
							map->GetParamBlock()->GetInt(kInstanceShape_sepHierarchy, t) ||
							map->GetParamBlock()->GetInt(kInstanceShape_sepElements, t);
			UpdateParticleShapeDlg(hWnd, op->meshData());
			UpdateRandomShapeDlg(hWnd, separateAny);
			break;
		case kInstanceShape_message_enterPick:
			iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
			iBut->SetCheck(TRUE);
			ReleaseICustButton(iBut);
			break;
		case kInstanceShape_message_exitPick:
			iBut = GetICustButton(GetDlgItem(hWnd, IDC_PICK));
			iBut->SetCheck(FALSE);
			ReleaseICustButton(iBut);
			break;
		case kInstanceShape_message_animate:
			animatedShape = map->GetParamBlock()->GetInt(kInstanceShape_animatedShape, t);
			addRandomOffset = map->GetParamBlock()->GetInt(kInstanceShape_syncRandom, t);
			UpdateAnimatedSahpeDlg( hWnd, animatedShape, addRandomOffset );
			break;
		case kInstanceShape_message_setScale:
			setScale = map->GetParamBlock()->GetInt(kInstanceShape_setScale, t);
			UpdateScaleDlg( hWnd, setScale);
			break;
		}
		break;
	}
	return FALSE;
}

void PFOperatorInstanceShapeDlgProc::UpdateObjectNameDlg( HWND hWnd, TSTR& objectName )
{
	ICustButton* but;
	but = GetICustButton( GetDlgItem( hWnd, IDC_PICK ) );
	but->SetText(objectName);
	ReleaseICustButton(but);
}

void PFOperatorInstanceShapeDlgProc::UpdateParticleShapeDlg( HWND hWnd, const Tab<Mesh*>& meshList )
{
	int numVerts, numFaces, numShapes;

	numShapes = meshList.Count();
	numVerts = numFaces = 0;
	if (numShapes > 0) {
		for (int i = 0; i < numShapes; i++) {
			numVerts += meshList[i]->getNumVerts();
			numFaces += meshList[i]->getNumFaces();
		}
		numVerts /= numShapes;
		numFaces /= numShapes;
	}
	
	TSTR buf;
	buf.printf("%d", numVerts);
	SetDlgItemText( hWnd, IDC_VERTICES, buf );
	buf.printf("%d", numFaces);
	SetDlgItemText( hWnd, IDC_FACES, buf );
	buf.printf("%d", numShapes);
	SetDlgItemText( hWnd, IDC_SHAPES, buf );
}

void PFOperatorInstanceShapeDlgProc::UpdateRandomShapeDlg( HWND hWnd, BOOL separateAny )
{
	EnableWindow( GetDlgItem( hWnd, IDC_ORDER ), separateAny );
}

void PFOperatorInstanceShapeDlgProc::UpdateAnimatedSahpeDlg( HWND hWnd, BOOL animatedShape, BOOL addRandomOffset )
{
	ISpinnerControl *spin;
	BOOL enableOffset = animatedShape && addRandomOffset;

	EnableWindow( GetDlgItem( hWnd, IDC_ACQUIRE ), !animatedShape );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCFRAME ), animatedShape );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCBYTEXT ), animatedShape );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNC ), animatedShape );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCRANDOM ), animatedShape );
	EnableWindow( GetDlgItem( hWnd, IDC_SYNCOFFSET ), enableOffset );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SYNCOFFSETSPIN ) );
	spin->Enable( enableOffset );
	ReleaseISpinner( spin );
}

void PFOperatorInstanceShapeDlgProc::UpdateScaleDlg( HWND hWnd, BOOL setScale)
{
	ISpinnerControl *spin;
	bool enable = (setScale != 0);

	EnableWindow( GetDlgItem( hWnd, IDC_VARIATIONTEXT ), enable );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_PARTICLESCALESPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_VARIATIONSPIN ) );
	spin->Enable( enable );
	ReleaseISpinner( spin );
}

//+--------------------------------------------------------------------------+
//|							PB2Value Validator								 |
//+--------------------------------------------------------------------------+

BOOL PFOperatorInstanceShapeMXSValidator::Validate(PB2Value& v)
{
	INode* iNode = (INode*)v.r;
	if (iNode == NULL) return NULL;
	TimeValue t = GetCOREInterface()->GetTime();
	Tab<INode*> stack;

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


} // end of namespace PFActions