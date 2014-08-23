/**********************************************************************
 *<
	FILE:			PFTestGoToRotation_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for GoToRotation Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-14-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTGOTOROTATION_PARAMBLOCK_H_
#define  _PFTESTGOTOROTATION_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestGoToRotationDesc.h"

namespace PFActions {

// block IDs
enum { kGoToRotation_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kGoToRotation_sendOut,
		kGoToRotation_syncBy,
		kGoToRotation_time,
		kGoToRotation_variation, // once, or multiple
		kGoToRotation_targetType,
		kGoToRotation_matchSpin,
		kGoToRotation_spin,
		kGoToRotation_spinVariation,
		kGoToRotation_easeIn,
		kGoToRotation_stopSpin,
		kGoToRotation_randomSeed
};

enum {	kGoToRotation_syncBy_time,
		kGoToRotation_syncBy_age,
		kGoToRotation_syncBy_event,
		kGoToRotation_syncBy_num = 3
};

enum {	kGoToRotation_targetType_constant,
		kGoToRotation_targetType_changing,
		kGoToRotation_targetType_num = 2
};

extern PFTestGoToRotationDesc ThePFTestGoToRotationDesc;
extern ParamBlockDesc2 goToRotation_paramBlockDesc;
extern FPInterfaceDesc goToRotation_action_FPInterfaceDesc;
extern FPInterfaceDesc goToRotation_operator_FPInterfaceDesc;
extern FPInterfaceDesc goToRotation_test_FPInterfaceDesc;
extern FPInterfaceDesc goToRotation_PViewItem_FPInterfaceDesc;

// User Defined Reference Messages from PB
enum {	kGoToRotation_RefMsg_InitDlg = REFMSG_USER + 13279,
		kGoToRotation_RefMsg_NewRand
};

// dialog messages
enum {	kGoToRotation_message_sync = 100,	// sync type change message
		kGoToRotation_message_variation,	// time or spin variation change message
		kGoToRotation_message_match			// change in match initial spin
};

class TestGoToRotationDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateSyncDlg( HWND hWnd, int syncBy);
	void UpdateVariationDlg( HWND hWnd, TimeValue timeVar, float spinVar );
	void UpdateMatchDlg( HWND hWnd, int matchSpin);
};



} // end of namespace PFActions

#endif // _PFTESTGOTOROTATION_PARAMBLOCK_H_