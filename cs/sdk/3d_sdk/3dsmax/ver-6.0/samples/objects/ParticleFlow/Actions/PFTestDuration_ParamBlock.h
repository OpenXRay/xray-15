/**********************************************************************
 *<
	FILE:			PFTestDuration_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Duration Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTDURATION_PARAMBLOCK_H_
#define  _PFTESTDURATION_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestDurationDesc.h"

namespace PFActions {

// block IDs
enum { kDuration_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kDuration_testType,
		kDuration_conditionType,
		kDuration_testValue,
		kDuration_variation,
		kDuration_subframeSampling,
		kDuration_disparity,
		kDuration_testFirst,
		kDuration_testLast,
		kDuration_lastIndex,
		kDuration_randomSeed
};

enum {	kDuration_testType_time,
		kDuration_testType_age,
		kDuration_testType_event,
		kDuration_testType_num = 3
};

enum {	kDuration_conditionType_less,
		kDuration_conditionType_greater
};

// User Defined Reference Messages from PB
enum {	kDuration_RefMsg_InitDlg = REFMSG_USER + 13279,
		kDuration_RefMsg_NewRand
};

// dialog messages
enum {	kDuration_message_variation = 100,	// variation change message
		kDuration_message_disparity		// disparity change message
};

extern PFTestDurationDesc ThePFTestDurationDesc;
extern ParamBlockDesc2 duration_paramBlockDesc;
extern FPInterfaceDesc duration_action_FPInterfaceDesc;
extern FPInterfaceDesc duration_test_FPInterfaceDesc;
extern FPInterfaceDesc duration_PViewItem_FPInterfaceDesc;

class TestDurationDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateVariationDlg( HWND hWnd, TimeValue variation );
	void UpdateDisparityDlg( HWND hWnd, int disparity );
};


} // end of namespace PFActions

#endif // _PFTESTDURATION_PARAMBLOCK_H_