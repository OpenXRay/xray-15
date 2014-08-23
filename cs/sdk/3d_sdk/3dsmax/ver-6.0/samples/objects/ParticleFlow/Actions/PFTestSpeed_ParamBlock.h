/**********************************************************************
 *<
	FILE:			PFTestSpeed_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Speed Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 11-25-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPEED_PARAMBLOCK_H_
#define  _PFTESTSPEED_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSpeedDesc.h"

namespace PFActions {

// block IDs
enum { kSpeedTest_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// paramIDs
enum {	kSpeedTest_testType,
		kSpeedTest_conditionType,
		kSpeedTest_unitValue,
		kSpeedTest_unitVariation,
		kSpeedTest_angleValue,
		kSpeedTest_angleVariation,
		kSpeedTest_sync,
		kSpeedTest_randomSeed
};

enum {	kSpeedTest_testType_speed,
		kSpeedTest_testType_speedX,
		kSpeedTest_testType_speedY,
		kSpeedTest_testType_speedZ,
		kSpeedTest_testType_accel,
		kSpeedTest_testType_accelX,
		kSpeedTest_testType_accelY,
		kSpeedTest_testType_accelZ,
		kSpeedTest_testType_steering,
		kSpeedTest_testType_whenAccels,
		kSpeedTest_testType_whenSlows,
		kSpeedTest_testType_num = 11
};

enum {	kSpeedTest_conditionType_less,
		kSpeedTest_conditionType_greater
};

enum {	kSpeedTest_sync_time,
		kSpeedTest_sync_age,
		kSpeedTest_sync_event,
		kSpeedTest_sync_num=3 
};

// User Defined Reference Messages from PB
enum {	kSpeedTest_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSpeedTest_RefMsg_NewRand
};

// dialog messages
enum {	kSpeedTest_message_update = 100 	// variation and type change message
};

extern PFTestSpeedDesc ThePFTestSpeedDesc;
extern ParamBlockDesc2 speedTest_paramBlockDesc;
extern FPInterfaceDesc speedTest_action_FPInterfaceDesc;
extern FPInterfaceDesc speedTest_test_FPInterfaceDesc;
extern FPInterfaceDesc speedTest_PViewItem_FPInterfaceDesc;

class TestSpeedDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateVariationDlg( HWND hWnd, TimeValue variation );
	void UpdateDlg( HWND hWnd, int type, float unitVar, float angleVar );
};


} // end of namespace PFActions

#endif // _PFTESTSPEED_PARAMBLOCK_H_