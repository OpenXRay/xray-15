/**********************************************************************
 *<
	FILE:			PFTestScale_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Scale Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSCALE_PARAMBLOCK_H_
#define  _PFTESTSCALE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestScaleDesc.h"

namespace PFActions {

// block IDs
enum { kScaleTest_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kScaleTest_testType,
		kScaleTest_axisType,
		kScaleTest_conditionType,
		kScaleTest_sizeValue,
		kScaleTest_sizeVariation,
		kScaleTest_scaleValue,
		kScaleTest_scaleVariation,
		kScaleTest_sync,
		kScaleTest_randomSeed
};

enum {	kScaleTest_testType_preSize,
		kScaleTest_testType_postSize,
		kScaleTest_testType_scale,
		kScaleTest_testType_num = 3
};

enum {	kScaleTest_axisType_average,
		kScaleTest_axisType_minimum,
		kScaleTest_axisType_median,
		kScaleTest_axisType_maximum,
		kScaleTest_axisType_x,
		kScaleTest_axisType_y,
		kScaleTest_axisType_z,
		kScaleTest_axisType_num = 7
};

enum {	kScaleTest_conditionType_less,
		kScaleTest_conditionType_greater
};

enum {	kScaleTest_sync_time,
		kScaleTest_sync_age,
		kScaleTest_sync_event,
		kScaleTest_sync_num=3 
};

// User Defined Reference Messages from PB
enum {	kScaleTest_RefMsg_InitDlg = REFMSG_USER + 13279,
		kScaleTest_RefMsg_NewRand
};

// dialog messages
enum {	kScaleTest_message_update = 100	// variation or type change message
};

extern PFTestScaleDesc ThePFTestScaleDesc;
extern ParamBlockDesc2 scaleTest_paramBlockDesc;
extern FPInterfaceDesc scaleTest_action_FPInterfaceDesc;
extern FPInterfaceDesc scaleTest_test_FPInterfaceDesc;
extern FPInterfaceDesc scaleTest_PViewItem_FPInterfaceDesc;

class TestScaleDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateDlg( HWND hWnd, int type, float sizeVar, float scaleVar );
};


} // end of namespace PFActions

#endif // _PFTESTSCALE_PARAMBLOCK_H_