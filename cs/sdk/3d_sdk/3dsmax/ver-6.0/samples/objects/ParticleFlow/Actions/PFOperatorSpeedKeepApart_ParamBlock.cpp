/**********************************************************************
 *<
	FILE:			PFOperatorSpeedKeepApart_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SpeedKeepApart Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorSpeedKeepApart_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"
#include "IPFActionListPool.h"
#include "IPFSystemPool.h"

#include "resource.h"
#include "notify.h"
#include "macrorec.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorSpeedKeepApartDesc ThePFOperatorSpeedKeepApartDesc;

// Dialog Proc
PFOperatorSpeedKeepApartDlgProc ThePFOperatorSpeedKeepApartDlgProc;

// SpeedKeepApart Operator param block
ParamBlockDesc2 speedKeepApart_paramBlockDesc 
(
	// required block spec
		kSpeedKeepApart_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorSpeedKeepApartDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpeedKeepApart_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPEEDKEEPAPART_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorSpeedKeepApartDlgProc,
	// required param specs
		// force value
			kSpeedKeepApart_force, _T("Force"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_FORCE,
			// optional tagged param specs
					p_default,		100.0f,
					p_range,		-999999999.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_FORCE, IDC_FORCESPIN, SPIN_AUTOSCALE,
			end,
		// use accel limit
			kSpeedKeepApart_useAccelLimit,	_T("Use_Accel_Limit"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USEACCELLIMIT,
			// optional tagged param specs
					p_default,		TRUE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USEACCELLIMIT,
			end,
		// accel limit value
			kSpeedKeepApart_accelLimit, _T("Accel_Limit"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_ACCELLIMIT,
			// optional tagged param specs
					p_default,		1000.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ACCELLIMIT, IDC_ACCELLIMITSPIN, SPIN_AUTOSCALE,
			end,
		// use speed limit
			kSpeedKeepApart_useSpeedLimit,	_T("Use_Speed_Limit"),
									TYPE_BOOL,
									P_RESET_DEFAULT,
									IDS_USESPEEDLIMIT,
			// optional tagged param specs
					p_default,		FALSE,
					p_ui,			TYPE_SINGLECHEKBOX,	IDC_USESPEEDLIMIT,
			end,
		// speed limit value
			kSpeedKeepApart_speedLimit, _T("Speed_Limit"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_SPEEDLIMIT,
			// optional tagged param specs
					p_default,		600.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_SPEEDLIMIT, IDC_SPEEDLIMITSPIN, SPIN_AUTOSCALE,
			end,
		// range type: absolute or relative
			kSpeedKeepApart_rangeType, _T("Range_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_RANGETYPE,
			// optional tagged param specs
					p_default,		kSpeedKeepApart_rangeType_absolute,
					p_range,		kSpeedKeepApart_rangeType_absolute, kSpeedKeepApart_rangeType_relative,
					p_ui,			TYPE_RADIO, kSpeedKeepApart_rangeType_num, IDC_ABSRANGE, IDC_RELRANGE,
			end,
		// core size
			kSpeedKeepApart_coreSize, _T("Core_Size"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_CORESIZE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ABSCORE, IDC_ABSCORESPIN, SPIN_AUTOSCALE,
			end,
		// falloff size
			kSpeedKeepApart_falloffSize, _T("Falloff_Size"),
									TYPE_WORLD,
									P_ANIMATABLE|P_RESET_DEFAULT,
									IDS_FALLOFFSIZE,
			// optional tagged param specs
					p_default,		10.0f,
					p_range,		0.0f, 999999999.0f,
					p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ABSFALLOFF, IDC_ABSFALLOFFSPIN, SPIN_AUTOSCALE,
			end,
		// core scale
			kSpeedKeepApart_coreRatio,	_T("Core_Percentage"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_CORERATIO,
			// optional tagged param specs
					p_default,		2.0f,
					p_range,		0.0f, 1000000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_RELCORE, IDC_RELCORESPIN, 1.0f,
			end,
		// falloff scale
			kSpeedKeepApart_falloffRatio,	_T("Falloff_Percentage"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_FALLOFFRATIO,
			// optional tagged param specs
					p_default,		1.0f,
					p_range,		0.0f, 1000000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_RELFALLOFF, IDC_RELFALLOFFSPIN, 1.0f,
			end,
		// range variation
			kSpeedKeepApart_variation,	_T("Variation"),
									TYPE_PCNT_FRAC,
									P_RESET_DEFAULT,
									IDS_VARIATION,
			// optional tagged param specs
					p_default,		0.0f,
					p_range,		0.0f, 1000000.0f,
					p_ui,			TYPE_SPINNER,	EDITTYPE_FLOAT,	IDC_VARIATION, IDC_VARIATIONSPIN, 1.0f,
			end,
		// scope type
			kSpeedKeepApart_scopeType, _T("Scope_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_SCOPETYPE,
			// optional tagged param specs
					p_default,		kSpeedKeepApart_scopeType_currentGroup,
					p_range,		kSpeedKeepApart_scopeType_currentGroup, kSpeedKeepApart_scopeType_selectedSystems,
					p_ui,			TYPE_RADIO, kSpeedKeepApart_scopeType_num, IDC_GROUP, IDC_SYSTEM, IDC_SELGROUPS, IDC_SELSYSTEMS,
			end,
		// selected groups - by node handles
			kSpeedKeepApart_groups,	_T("Selected_Events"),
									TYPE_INT_TAB,
									0, P_VARIABLE_SIZE,
									IDS_SELECTEDGROUPS,
			end,
		// selected particle systems - by node handles
			kSpeedKeepApart_systems, _T("Selected_Systems"),
									TYPE_INT_TAB,
									0, P_VARIABLE_SIZE,
									IDS_SELECTEDSYSTEMS,
			end,
		// seed
			kSpeedKeepApart_seed,		_T("Random_Seed"),
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

// SpeedKeepApart Operator param block for script wiring
ParamBlockDesc2 speedKeepApart_scriptBlockDesc 
(
	// required block spec
		kSpeedKeepApart_scriptPBlockIndex, 
		_T("Script Wiring"),
		IDS_SCRIPTWIRING,
		&ThePFOperatorSpeedKeepApartDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSpeedKeepApart_scriptPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPEEDKEEPAPART_SCRIPTWIRING, 
		IDS_SCRIPTWIRING,
		0,
		0, // open/closed
		NULL,
	// required param specs
		// use script wiring
		kSpeedKeepApart_useScriptWiring, _T(""),  //_T("Use Script Wiring") --010803  az: for future use
									TYPE_INT,
									0,
									IDS_USESCRIPTWIRING,
			// optional tagged param specs
					p_default,		FALSE,
			end,
		// float wiring
			kSpeedKeepApart_useFloat,	_T("Use_Script_Float"),
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTFLOAT,
			// optional tagged param specs
					p_default,		kSpeedKeepApart_useFloat_none,
					p_range,		0, kSpeedKeepApart_useFloat_num-1,
					p_ui,			TYPE_RADIO, kSpeedKeepApart_useFloat_num, IDC_FLOATNOTUSED, IDC_FORCE,
			end,

		// for future use
			kSpeedKeepApart_useInt,	_T(""),  //_T("Use Script Integer") --010803  az: for future use
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTINTEGER,
			// optional tagged param specs
					p_default,		0,
			end,
			kSpeedKeepApart_useVector,	_T("Use_Script_Vector"),  
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTVECTOR,
			// optional tagged param specs
					p_default,		kSpeedKeepApart_useVector_none,
					p_range,		0, kSpeedKeepApart_useVector_num-1,
					p_ui,			TYPE_RADIO, kSpeedKeepApart_useVector_num, IDC_VECTORNOTUSED, IDC_ABSOLUTERANGE, IDC_RELATIVERANGE,
			end,
			kSpeedKeepApart_useMatrix,	_T(""), //_T("Use Script Matrix") --010803  az: for future use
									TYPE_INT,
									P_RESET_DEFAULT,
									IDS_USESCRIPTMATRIX,
			// optional tagged param specs
					p_default,		0,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc speedKeepApart_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorSpeedKeepApartDesc, FP_MIXIN, 
				
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
			IPFAction::kGetUseScriptWiring, _T("getUseScriptWiring"), 0, TYPE_bool, 0, 0,
			IPFAction::kSetUseScriptWiring, _T("setUseScriptWiring"), 0, TYPE_VOID, 0, 1,
				_T("useState"), 0, TYPE_bool,

		end
);

FPInterfaceDesc speedKeepApart_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorSpeedKeepApartDesc, FP_MIXIN,

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

FPInterfaceDesc speedKeepApart_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorSpeedKeepApartDesc, FP_MIXIN,

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

static void UpdateActionLists(void *param, NotifyInfo *info) 
{
	PFOperatorSpeedKeepApartDlgProc* proc = (PFOperatorSpeedKeepApartDlgProc*)param;
	for(int i=0; i<proc->NumMaps(); i++)
		proc->UpdateActionListDlg(proc->GetMap(i));
}

static void UpdatePFSystems(void *param, NotifyInfo *info) 
{
	PFOperatorSpeedKeepApartDlgProc* proc = (PFOperatorSpeedKeepApartDlgProc*)param;
	for(int i=0; i<proc->NumMaps(); i++)
		proc->UpdatePSystemDlg(proc->GetMap(i));
}

BOOL PFOperatorSpeedKeepApartDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock = NULL;

	switch(msg) {
		case WM_INITDIALOG:
			if (NumMaps() == 0) { // first map to open
				GetPFActionListPool()->RegisterNotification(UpdateActionLists, this);
				GetPFSystemPool()->RegisterNotification(UpdatePFSystems, this);
			}
			AddMap(map);
			if (map != NULL) pblock = map->GetParamBlock();
			if (pblock != NULL)	pblock->NotifyDependents( FOREVER, (PartID)map, kSpeedKeepApart_RefMsg_InitDlg );

//			UpdateDlg(hWnd, map->GetParamBlock());
//			UpdateActionListDlg(map);
//			UpdatePSystemDlg(map);

//			SetCtlByDir(map, hWnd);
			break;
		case WM_DESTROY:
			RemoveMap(map);
			if (NumMaps() == 0) { // last map to close
				GetPFActionListPool()->UnRegisterNotification(UpdateActionLists, this);
				GetPFSystemPool()->UnRegisterNotification(UpdatePFSystems, this);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_GROUPLIST:
					SelectActionLists(hWnd, map->GetParamBlock());
					break;
				case IDC_SYSTEMLIST:
					SelectPSystems(hWnd, map->GetParamBlock());
					break;
				case kSpeedKeepApart_message_update:
					UpdateDlg(hWnd, map->GetParamBlock());
					UpdateActionListDlg(map);
					UpdatePSystemDlg(map);
					break;
				case kSpeedKeepApart_message_enableForce:
					UpdateForceDlg( hWnd, true);
					break;
				case kSpeedKeepApart_message_disableForce:
					UpdateForceDlg( hWnd, false);
					break;
				case kSpeedKeepApart_message_enableAbsRange:
					UpdateAbsRangeDlg( hWnd, true);
					break;
				case kSpeedKeepApart_message_disableAbsRange:
					UpdateAbsRangeDlg( hWnd, false);
					break;
				case kSpeedKeepApart_message_enableRelRange:
					UpdateRelRangeDlg( hWnd, true);
					break;
				case kSpeedKeepApart_message_disableRelRange:
					UpdateRelRangeDlg( hWnd, false);
					break;
				case kSpeedKeepApart_message_enableVariation:
					UpdateVariationDlg( hWnd, true);
					break;
				case kSpeedKeepApart_message_disableVariation:
					UpdateVariationDlg( hWnd, false);
					break;
				case IDC_NEW:
					map->GetParamBlock()->NotifyDependents(FOREVER, PART_OBJ, kSpeedKeepApart_RefMsg_NewRand);
					break;
			}
			break;
	}

	return FALSE;
}

void PFOperatorSpeedKeepApartDlgProc::UpdateDlg( HWND hWnd, IParamBlock2* pblock)
{
	if (pblock == NULL) return;

	int scopeType = pblock->GetInt(kSpeedKeepApart_scopeType, 0);

	bool useAccelLimit = (pblock->GetInt(kSpeedKeepApart_useAccelLimit, 0) != 0);
	bool useSpeedLimit = (pblock->GetInt(kSpeedKeepApart_useSpeedLimit, 0) != 0);
	bool useGroups = (scopeType == kSpeedKeepApart_scopeType_selectedGroups);
	bool useSystems = (scopeType == kSpeedKeepApart_scopeType_selectedSystems);

	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_ACCELLIMITSPIN ) );
	spin->Enable( useAccelLimit );
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_SPEEDLIMITSPIN ) );
	spin->Enable( useSpeedLimit );
	ReleaseISpinner( spin );

	EnableWindow( GetDlgItem( hWnd, IDC_GROUPLIST ), useGroups);
	EnableWindow( GetDlgItem( hWnd, IDC_SYSTEMLIST ), useSystems);
}

void PFOperatorSpeedKeepApartDlgProc::UpdateActionListDlg( IParamMap2* map )
{
	if (map == NULL) return;
	IParamBlock2* pblock = map->GetParamBlock();
	if (pblock == NULL) return;
	HWND hWnd = map->GetHWnd();

	HWND hWndAList = GetDlgItem(hWnd, IDC_GROUPLIST);
	SendMessage(hWndAList, LB_RESETCONTENT, 0, 0);
	IPFActionListPool* pool = GetPFActionListPool();
	for(int i=0; i<pool->NumActionLists(); i++) {
		INode* aList = pool->GetActionList(i); DbgAssert(aList);
		SendMessage(hWndAList, LB_ADDSTRING, 0, (LPARAM)(TCHAR*)(aList->GetName()) );
		if (SKAIsSelectedActionList(aList, pblock))
			SendMessage(hWndAList, LB_SETSEL, TRUE, i);
	}
}

void PFOperatorSpeedKeepApartDlgProc::UpdatePSystemDlg( IParamMap2* map )
{
	if (map == NULL) return;
	IParamBlock2* pblock = map->GetParamBlock();
	if (pblock == NULL) return;
	HWND hWnd = map->GetHWnd();

	HWND hWndSystem = GetDlgItem(hWnd, IDC_SYSTEMLIST);
	SendMessage(hWndSystem, LB_RESETCONTENT, 0, 0);
	IPFSystemPool* pool = GetPFSystemPool();
	for(int i=0; i<pool->NumPFSystems(); i++) {
		INode* pSystem = pool->GetPFSystem(i);
		SendMessage(hWndSystem, LB_ADDSTRING, 0, (LPARAM)(TCHAR*)(pSystem->GetName()) );
		if (SKAIsSelectedPSystem(pSystem, pblock))
			SendMessage(hWndSystem, LB_SETSEL, TRUE, i);
	}
}

void PFOperatorSpeedKeepApartDlgProc::UpdateForceDlg( HWND hWnd, bool enable )
{
	EnableWindow( GetDlgItem( hWnd, IDC_FORCETEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_FORCESPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
}

void PFOperatorSpeedKeepApartDlgProc::UpdateAbsRangeDlg( HWND hWnd, bool enable )
{
	EnableWindow( GetDlgItem( hWnd, IDC_ABSCORETEXT), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_ABSFALLOFFTEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_ABSCORESPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_ABSFALLOFFSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
}

void PFOperatorSpeedKeepApartDlgProc::UpdateRelRangeDlg( HWND hWnd, bool enable )
{
	EnableWindow( GetDlgItem( hWnd, IDC_RELCORETEXT), enable );
	EnableWindow( GetDlgItem( hWnd, IDC_RELFALLOFFTEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_RELCORESPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
	spin = GetISpinner( GetDlgItem( hWnd, IDC_RELFALLOFFSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
}

void PFOperatorSpeedKeepApartDlgProc::UpdateVariationDlg( HWND hWnd, bool enable )
{
	EnableWindow( GetDlgItem( hWnd, IDC_VARIATIONTEXT), enable );
	ISpinnerControl* spin = GetISpinner( GetDlgItem( hWnd, IDC_VARIATIONSPIN ) );
	spin->Enable(enable);
	ReleaseISpinner( spin );
}

void PFOperatorSpeedKeepApartDlgProc::AddMap(IParamMap2* map)
{
	m_activeMaps.Append(1, &map);
}

void PFOperatorSpeedKeepApartDlgProc::RemoveMap(IParamMap2* map)
{
	int count = m_activeMaps.Count();
	for(int i=count-1; i>=0; i--)
		if (m_activeMaps[i] == map)
			m_activeMaps.Delete(i, 1);
}

int PFOperatorSpeedKeepApartDlgProc::NumMaps()
{
	return m_activeMaps.Count();
}

IParamMap2* PFOperatorSpeedKeepApartDlgProc::GetMap(int index)
{		
	if ((index >= 0) && (index < NumMaps()))
		return m_activeMaps[index];
	return NULL;
}

static selectionUpdateInProgress = false;

void PFOperatorSpeedKeepApartDlgProc::SelectActionLists( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;
	if (selectionUpdateInProgress) return;

	int allCount = GetPFActionListPool()->NumActionLists();
	HWND hWndAList = GetDlgItem(hWnd, IDC_GROUPLIST);
	int listCount = SendMessage(hWndAList, LB_GETCOUNT, 0, 0);
	if (listCount == LB_ERR) return; // dialog error
	if (allCount != listCount) return; // async

	selectionUpdateInProgress = true;
	
	INodeTab selNodes;
	for(int i=0; i<listCount; i++) {
		int selected = SendMessage(hWndAList, LB_GETSEL, i, 0);
		if (selected > 0) {
			INode* selNode = GetPFActionListPool()->GetActionList(i);
			if (selNode != NULL)
				selNodes.Append(1, &selNode);
		}
	}

	bool resetSelected = ( selNodes.Count() != pblock->Count(kSpeedKeepApart_groups) );
	if (!resetSelected) {
		for(int i=0; i<selNodes.Count(); i++)
			if (!SKAIsSelectedActionList(selNodes[i], pblock))
			{ resetSelected = true; break; }
	}
	
	if (resetSelected) {
		macroRec->Disable();
		theHold.Begin();
		pblock->ZeroCount(kSpeedKeepApart_groups);
		if (selNodes.Count() > 0) {
			for(int i=0; i<selNodes.Count(); i++) {
				int nodeHandle = (int)selNodes[i]->GetHandle();
				pblock->Append(kSpeedKeepApart_groups, 1, &nodeHandle);
			}
		}
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
		macroRec->Enable();
		pblock->NotifyDependents( FOREVER, 0, kSpeedKeepApart_RefMsg_MacrorecGroups );
	}

	selectionUpdateInProgress = false;
}

void PFOperatorSpeedKeepApartDlgProc::SelectPSystems( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;
	if (selectionUpdateInProgress) return;

	int allCount = GetPFSystemPool()->NumPFSystems();
	HWND hWndSystem = GetDlgItem(hWnd, IDC_SYSTEMLIST);
	int listCount = SendMessage(hWndSystem, LB_GETCOUNT, 0, 0);
	if (listCount == LB_ERR) return; // dialog error
	if (allCount != listCount) return; // async

	selectionUpdateInProgress = true;
	
	INodeTab selNodes;
	for(int i=0; i<listCount; i++) {
		int selected = SendMessage(hWndSystem, LB_GETSEL, i, 0);
		if (selected > 0) {
			INode* selNode = GetPFSystemPool()->GetPFSystem(i);
			if (selNode != NULL)
				selNodes.Append(1, &selNode);
		}
	}

	bool resetSelected = ( selNodes.Count() != pblock->Count(kSpeedKeepApart_systems) );
	if (!resetSelected) {
		for(int i=0; i<selNodes.Count(); i++)
			if (!SKAIsSelectedPSystem(selNodes[i], pblock))
			{ resetSelected = true; break; }
	}
	
	if (resetSelected) {
		macroRec->Disable();
		theHold.Begin();
		pblock->ZeroCount(kSpeedKeepApart_systems);
		if (selNodes.Count() > 0) {
			for(int i=0; i<selNodes.Count(); i++) {
				int nodeHandle = (int)selNodes[i]->GetHandle();
				pblock->Append(kSpeedKeepApart_systems, 1, &nodeHandle);
			}
		}
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
		macroRec->Enable();
		pblock->NotifyDependents( FOREVER, 0, kSpeedKeepApart_RefMsg_MacrorecSystems );
	}

	selectionUpdateInProgress = false;
}

bool SKAIsSelectedActionList(INode* node, IParamBlock2* pblock)
{
	if (node == NULL) return false;
	int nodeHandle = (int)node->GetHandle();
	for(int i=0; i<pblock->Count(kSpeedKeepApart_groups); i++)
		if (nodeHandle == pblock->GetInt(kSpeedKeepApart_groups, 0, i))
			return true;
	return false;
}

bool SKAIsSelectedPSystem(INode* node, IParamBlock2* pblock)
{
	if (node == NULL) return false;
	int nodeHandle = (int)node->GetHandle();
	for(int i=0; i<pblock->Count(kSpeedKeepApart_systems); i++)
		if (nodeHandle == pblock->GetInt(kSpeedKeepApart_systems, 0, i))
			return true;
	return false;
}

} // end of namespace PFActions