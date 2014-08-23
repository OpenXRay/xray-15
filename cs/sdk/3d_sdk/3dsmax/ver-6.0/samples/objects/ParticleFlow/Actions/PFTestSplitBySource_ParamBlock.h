/**********************************************************************
 *<
	FILE:			PFTestSplitBySource_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SplitBySource Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPLITBYSOURCE_PARAMBLOCK_H_
#define  _PFTESTSPLITBYSOURCE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSplitBySourceDesc.h"

namespace PFActions {

// block IDs
enum { kSplitBySource_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kSplitBySource_conditionType,
		kSplitBySource_sources
};

enum {	kSplitBySource_conditionType_selected,
		kSplitBySource_conditionType_notSelected
};

// User Defined Reference Messages from PB
enum {	kSplitBySource_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSplitBySource_RefMsg_NewRand,
		kSplitBySource_RefMsg_MacrorecSources
};

// dialog messages
enum {	kSplitBySource_message_update = 100	// settings change
};

extern PFTestSplitBySourceDesc ThePFTestSplitBySourceDesc;
extern ParamBlockDesc2 splitBySource_paramBlockDesc;
extern FPInterfaceDesc splitBySource_action_FPInterfaceDesc;
extern FPInterfaceDesc splitBySource_test_FPInterfaceDesc;
extern FPInterfaceDesc splitBySource_PViewItem_FPInterfaceDesc;

class TestSplitBySourceDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }

	int NumMaps();
	IParamMap2* GetMap(int index);
	void UpdateSourceDlg( IParamMap2* map );

private:
	void AddMap(IParamMap2* map);
	void RemoveMap(IParamMap2* map);
	void SelectSources( HWND hWnd, IParamBlock2* pblock );

	Tab<IParamMap2*> m_activeMaps; // list of currently open maps
};

bool SBSIsSelectedSource(INode* node, IParamBlock2* pblock);


} // end of namespace PFActions

#endif // _PFTESTSPLITBYSOURCE_PARAMBLOCK_H_