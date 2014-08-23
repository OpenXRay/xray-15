/**********************************************************************
 *<
	FILE:			PFTestSplitBySource_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for SplitBySource Test (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#include "PFTestSplitBySource_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFTest.h"
#include "IPViewItem.h"
#include "IPFSystemPool.h"

#include "resource.h"
#include "notify.h"
#include "macrorec.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFTestSplitBySourceDesc ThePFTestSplitBySourceDesc;

// custom update dialog process
TestSplitBySourceDlgProc theTestSplitBySourceDlgProc;

// SplitBySource Test param block
ParamBlockDesc2 splitBySource_paramBlockDesc 
(
	// required block spec
		kSplitBySource_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFTestSplitBySourceDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kSplitBySource_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_SPLITBYSOURCE_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&theTestSplitBySourceDlgProc,
	// required param specs
		// condition type: selected or not selected
			kSplitBySource_conditionType, _T("Condition_Type"),
									TYPE_RADIOBTN_INDEX,
									P_RESET_DEFAULT,
									IDS_CONDITIONTYPE,
			// optional tagged param specs
					p_default,		kSplitBySource_conditionType_selected,
					p_range,		0, 1,
					p_ui,			TYPE_RADIO, 2, IDC_TRUESOURCE, IDC_TRUENOTSOURCE,
			end,
		// selected particle systems - by node handles
			kSplitBySource_sources, _T("Selected_Sources"),
									TYPE_INT_TAB,
									0, P_VARIABLE_SIZE,
									IDS_SELECTEDSOURCES,
			end,

		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc splitBySource_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFTestSplitBySourceDesc, FP_MIXIN, 
				
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

FPInterfaceDesc splitBySource_test_FPInterfaceDesc(
			PFTEST_INTERFACE,
			_T("test"), 0,
			&ThePFTestSplitBySourceDesc, FP_MIXIN,

			IPFTest::kProceedStep1, _T("proceedStep1"), 0, TYPE_VOID, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,
			IPFTest::kProceedStep2, _T("proceedStep2"), 0, TYPE_bool, 0, 6,
				_T("timeStartTick"), 0, TYPE_TIMEVALUE,
				_T("timeStartFraction"), 0, TYPE_FLOAT,
				_T("timeEndTick"), 0, TYPE_TIMEVALUE_BR,
				_T("timeEndFraction"), 0, TYPE_FLOAT_BR,
				_T("testResult"), 0, TYPE_BITARRAY_BR,
				_T("testTime"), 0, TYPE_FLOAT_TAB_BR,
			IPFTest::kGetNextActionList, _T("getNextActionList"), 0, TYPE_INODE, 0, 2,
				_T("testNode"), 0, TYPE_INODE,
				_T("linkActive"), 0, TYPE_bool_BP,
			IPFTest::kSetNextActionList, _T("setNextActionList"), 0, TYPE_bool, 0, 2,
				_T("actionList"), 0, TYPE_INODE,
				_T("testNode"), 0, TYPE_INODE,
			IPFTest::kSetLinkActive, _T("setLinkActive"), 0, TYPE_bool, 0, 2,
				_T("linkActive"), 0, TYPE_bool,
				_T("testNode"), 0, TYPE_INODE,
			IPFTest::kClearNextActionList, _T("clearNextActionList"), 0, TYPE_bool, 0, 1,
				_T("testNode"), 0, TYPE_INODE,
		end
);

FPInterfaceDesc splitBySource_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFTestSplitBySourceDesc, FP_MIXIN,

			IPViewItem::kNumPViewParamBlocks, _T("numPViewParamBlocks"), 0, TYPE_INT, 0, 0,
			IPViewItem::kGetPViewParamBlock, _T("getPViewParamBlock"), 0, TYPE_REFTARG, 0, 1,
				_T("index"), 0, TYPE_INDEX,
			IPViewItem::kHasComments, _T("hasComments"), 0, TYPE_bool, 0, 1,
				_T("action"), 0, TYPE_INODE,
			IPViewItem::kGetComments, _T("getComments"), 0, TYPE_STRING, 0, 1,
				_T("action"), 0, TYPE_INODE,
			IPViewItem::kSetComments, _T("setComments"), 0, TYPE_VOID, 0, 2,
				_T("action"), 0, TYPE_INODE,
				_T("comments"), 0, TYPE_STRING,
			IPViewItem::kGetWireExtension, _T("getWireExtension"), 0, TYPE_INT, 0, 2,
				_T("action"), 0, TYPE_INODE,
				_T("wireHeight"), 0, TYPE_INT_BR,
			IPViewItem::kSetWireExtension, _T("setWireExtension"), 0, TYPE_VOID, 0, 3,
				_T("action"), 0, TYPE_INODE,
				_T("wireWidth"), 0, TYPE_INT,
				_T("wireHeight"), 0, TYPE_INT,
		end
);


//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|					custom UI update for SplitBySource Test  					 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
static void UpdateSources(void *param, NotifyInfo *info) 
{
	TestSplitBySourceDlgProc* proc = (TestSplitBySourceDlgProc*)param;
	for(int i=0; i<proc->NumMaps(); i++)
		proc->UpdateSourceDlg(proc->GetMap(i));
}

BOOL TestSplitBySourceDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2* pblock = NULL;

	switch(msg) {
		case WM_INITDIALOG:
			if (NumMaps() == 0) { // first map to open
				GetPFSystemPool()->RegisterNotification(UpdateSources, this);
			}
			AddMap(map);
			if (map != NULL) pblock = map->GetParamBlock();
			if (pblock != NULL)	pblock->NotifyDependents( FOREVER, (PartID)map, kSplitBySource_RefMsg_InitDlg );
			break;
		case WM_DESTROY:
			RemoveMap(map);
			if (NumMaps() == 0) { // last map to close
				GetPFSystemPool()->UnRegisterNotification(UpdateSources, this);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_SOURCELIST:
					SelectSources(hWnd, map->GetParamBlock());
					break;
				case kSplitBySource_message_update:
					UpdateSourceDlg(map);
					break;
			}
			break;
	}

	return FALSE;
}

void TestSplitBySourceDlgProc::UpdateSourceDlg( IParamMap2* map )
{
	if (map == NULL) return;
	IParamBlock2* pblock = map->GetParamBlock();
	if (pblock == NULL) return;
	HWND hWnd = map->GetHWnd();

	HWND hWndSystem = GetDlgItem(hWnd, IDC_SOURCELIST);
	SendMessage(hWndSystem, LB_RESETCONTENT, 0, 0);
	IPFSystemPool* pool = GetPFSystemPool();
	for(int i=0; i<pool->NumPFSystems(); i++) {
		INode* pSystem = pool->GetPFSystem(i);
		SendMessage(hWndSystem, LB_ADDSTRING, 0, (LPARAM)(TCHAR*)(pSystem->GetName()) );
		if (SBSIsSelectedSource(pSystem, pblock))
			SendMessage(hWndSystem, LB_SETSEL, TRUE, i);
	}
}

void TestSplitBySourceDlgProc::AddMap(IParamMap2* map)
{
	m_activeMaps.Append(1, &map);
}

void TestSplitBySourceDlgProc::RemoveMap(IParamMap2* map)
{
	int count = m_activeMaps.Count();
	for(int i=count-1; i>=0; i--)
		if (m_activeMaps[i] == map)
			m_activeMaps.Delete(i, 1);
}

int TestSplitBySourceDlgProc::NumMaps()
{
	return m_activeMaps.Count();
}

IParamMap2* TestSplitBySourceDlgProc::GetMap(int index)
{		
	if ((index >= 0) && (index < NumMaps()))
		return m_activeMaps[index];
	return NULL;
}

static selectionUpdateInProgress = false;

void TestSplitBySourceDlgProc::SelectSources( HWND hWnd, IParamBlock2* pblock )
{
	if (pblock == NULL) return;
	if (selectionUpdateInProgress) return;

	int allCount = GetPFSystemPool()->NumPFSystems();
	HWND hWndSystem = GetDlgItem(hWnd, IDC_SOURCELIST);
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

	bool resetSelected = ( selNodes.Count() != pblock->Count(kSplitBySource_sources) );
	if (!resetSelected) {
		for(int i=0; i<selNodes.Count(); i++)
			if (!SBSIsSelectedSource(selNodes[i], pblock))
			{ resetSelected = true; break; }
	}
	
	if (resetSelected) {
		macroRec->Disable();
		theHold.Begin();
		pblock->ZeroCount(kSplitBySource_sources);
		if (selNodes.Count() > 0) {
			for(int i=0; i<selNodes.Count(); i++) {
				int nodeHandle = (int)selNodes[i]->GetHandle();
				pblock->Append(kSplitBySource_sources, 1, &nodeHandle);
			}
		}
		theHold.Accept(GetString(IDS_PARAMETERCHANGE));
		macroRec->Enable();
		pblock->NotifyDependents( FOREVER, 0, kSplitBySource_RefMsg_MacrorecSources );
	}

	selectionUpdateInProgress = false;
}

bool SBSIsSelectedSource(INode* node, IParamBlock2* pblock)
{
	int nodeHandle = (int)node->GetHandle();
	for(int i=0; i<pblock->Count(kSplitBySource_sources); i++)
		if (nodeHandle == pblock->GetInt(kSplitBySource_sources, 0, i))
			return true;
	return false;
}



} // end of namespace PFActions
