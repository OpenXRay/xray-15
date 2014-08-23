/**********************************************************************
 *<
	FILE:			PFTestGoToNextEvent_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for GoToNextEvent Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 08-27-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTGOTONEXTEVENT_PARAMBLOCK_H_
#define  _PFTESTGOTONEXTEVENT_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestGoToNextEventDesc.h"

namespace PFActions {

// block IDs
enum { kGoToNextEvent_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kGoToNextEvent_conditionType
};

enum {	kGoToNextEvent_conditionType_all,
		kGoToNextEvent_conditionType_none
};

extern PFTestGoToNextEventDesc ThePFTestGoToNextEventDesc;
extern ParamBlockDesc2 goToNextEvent_paramBlockDesc;
extern FPInterfaceDesc goToNextEvent_action_FPInterfaceDesc;
extern FPInterfaceDesc goToNextEvent_test_FPInterfaceDesc;
extern FPInterfaceDesc goToNextEvent_PViewItem_FPInterfaceDesc;

class TestGoToNextEventDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
};


} // end of namespace PFActions

#endif // _PFTESTGOTONEXTEVENT_PARAMBLOCK_H_