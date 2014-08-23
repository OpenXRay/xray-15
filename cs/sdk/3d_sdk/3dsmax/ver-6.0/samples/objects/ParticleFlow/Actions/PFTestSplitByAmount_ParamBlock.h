/**********************************************************************
 *<
	FILE:			PFTestSplitByAmount_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SplitByAmount Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPLITBYAMOUNT_PARAMBLOCK_H_
#define  _PFTESTSPLITBYAMOUNT_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSplitByAmountDesc.h"

namespace PFActions {

// block IDs
enum { kSplitByAmount_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kSplitByAmount_testType,
		kSplitByAmount_fraction,
		kSplitByAmount_everyN,
		kSplitByAmount_firstN,
		kSplitByAmount_perSource,
		kSplitByAmount_randomSeed
};

enum {	kSplitByAmount_testType_fraction,
		kSplitByAmount_testType_everyN,
		kSplitByAmount_testType_firstN,
		kSplitByAmount_testType_afterFirstN,
		kSplitByAmount_testType_num = 4
};

// User Defined Reference Messages from PB
enum {	kSplitByAmount_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSplitByAmount_RefMsg_NewRand
};

// dialog messages
enum {	kSplitByAmount_message_type = 100	// type change message
};

extern PFTestSplitByAmountDesc ThePFTestSplitByAmountDesc;
extern ParamBlockDesc2 splitByAmount_paramBlockDesc;
extern FPInterfaceDesc splitByAmount_action_FPInterfaceDesc;
extern FPInterfaceDesc splitByAmount_test_FPInterfaceDesc;
extern FPInterfaceDesc splitByAmount_PViewItem_FPInterfaceDesc;

class TestSplitByAmountDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateTypeDlg( HWND hWnd, int type );
};


} // end of namespace PFActions

#endif // _PFTESTSPLITBYAMOUNT_PARAMBLOCK_H_