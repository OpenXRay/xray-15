/**********************************************************************
 *<
	FILE:			PFTestSpeedGoToTarget_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SpeedGoToTarget Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY: created 07-16-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPEEDGOTOTARGET_PARAMBLOCK_H_
#define  _PFTESTSPEEDGOTOTARGET_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSpeedGoToTargetDesc.h"

namespace PFActions {

// block IDs
enum { kSpeedGoToTarget_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kSpeedGoToTarget_type,
		kSpeedGoToTarget_useCruiseSpeed,
		kSpeedGoToTarget_cruiseSpeed,
		kSpeedGoToTarget_cruiseSpeedVariation,
		kSpeedGoToTarget_accelLimit,
		kSpeedGoToTarget_syncBy,
		kSpeedGoToTarget_distance,
		kSpeedGoToTarget_timingType,
		kSpeedGoToTarget_timeValue,
		kSpeedGoToTarget_timeVariation,
		kSpeedGoToTarget_subframe,
		kSpeedGoToTarget_useDockingSpeed,
		kSpeedGoToTarget_dockingSpeed,
		kSpeedGoToTarget_dockingSpeedVariation,
		kSpeedGoToTarget_targetType,
		kSpeedGoToTarget_targets,
		kSpeedGoToTarget_animated,
		kSpeedGoToTarget_follow,
		kSpeedGoToTarget_lock,
		kSpeedGoToTarget_pointType,
		kSpeedGoToTarget_assignment,
		kSpeedGoToTarget_dockingType,
		kSpeedGoToTarget_iconSize,
		kSpeedGoToTarget_randomSeed,
		kSpeedGoToTarget_testType,
		kSpeedGoToTarget_easeIn,
		kSpeedGoToTarget_targetSync,
		kSpeedGoToTarget_dockingDistance,
		kSpeedGoToTarget_colorType,
		kSpeedGoToTarget_targetsMaxscript
};

// control type: speed or time
enum {	kSpeedGoToTarget_type_speed,
		kSpeedGoToTarget_type_time,
		kSpeedGoToTarget_type_none,
		kSpeedGoToTarget_type_num = 3
};

// test type
enum {  kSpeedGoToTarget_testType_pivot,
		kSpeedGoToTarget_testType_point,
		kSpeedGoToTarget_testType_num = 2
};

// sync type
enum {	kSpeedGoToTarget_syncBy_time,
		kSpeedGoToTarget_syncBy_age,
		kSpeedGoToTarget_syncBy_event,
		kSpeedGoToTarget_syncBy_num=3 
};

// timing type
enum {	kSpeedGoToTarget_timingType_time,
		kSpeedGoToTarget_timingType_age,
		kSpeedGoToTarget_timingType_event,
		kSpeedGoToTarget_timingType_num=3 
};

// target type
enum {  kSpeedGoToTarget_targetType_icon,
		kSpeedGoToTarget_targetType_objects,
		kSpeedGoToTarget_targetType_num = 2
};

// point type
enum {  kSpeedGoToTarget_pointType_random,
		kSpeedGoToTarget_pointType_surface,
		kSpeedGoToTarget_pointType_vector,
		kSpeedGoToTarget_pointType_num = 3
};

// assignment type
enum {	kSpeedGoToTarget_assignment_random,
		kSpeedGoToTarget_assignment_distance,
		kSpeedGoToTarget_assignment_surface,
		kSpeedGoToTarget_assignment_deviation,
		kSpeedGoToTarget_assignment_integer,
		kSpeedGoToTarget_assignment_num = 5
};

// docking type
enum {	kSpeedGoToTarget_dockingType_none,
		kSpeedGoToTarget_dockingType_parallel,
		kSpeedGoToTarget_dockingType_spherical,
		kSpeedGoToTarget_dockingType_cylindrical,
		kSpeedGoToTarget_dockingType_normal,
		kSpeedGoToTarget_dockingType_num = 5
};

// color type
enum {  kSpeedGoToTarget_colorType_test,
		kSpeedGoToTarget_colorType_coordinated,
		kSpeedGoToTarget_colorType_num = 2
};

// User Defined Reference Messages from PB
enum {	kSpeedGoToTarget_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSpeedGoToTarget_RefMsg_NewRand,
		kSpeedGoToTarget_RefMsg_ListSelect, // add more nodes by select dialog
		kSpeedGoToTarget_RefMsg_ResetValidatorAction
};

// dialog messages
enum {	kSpeedGoToTarget_message_update = 100	// settings change
};

extern PFTestSpeedGoToTargetDesc ThePFTestSpeedGoToTargetDesc;
extern ParamBlockDesc2 speedGoToTarget_paramBlockDesc;
extern FPInterfaceDesc speedGoToTarget_action_FPInterfaceDesc;
extern FPInterfaceDesc speedGoToTarget_test_FPInterfaceDesc;
extern FPInterfaceDesc speedGoToTarget_PViewItem_FPInterfaceDesc;

class TestSpeedGoToTargetDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateDlg( HWND hWnd, IParamBlock2* pblock, TimeValue time );
};


} // end of namespace PFActions

#endif // _PFTESTSPEEDGOTOTARGET_PARAMBLOCK_H_