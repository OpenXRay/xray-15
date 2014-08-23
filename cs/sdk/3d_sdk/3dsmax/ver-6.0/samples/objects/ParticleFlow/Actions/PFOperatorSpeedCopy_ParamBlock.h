/**********************************************************************
 *<
	FILE:			PFOperatorSpeedCopy_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SpeedCopy Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSPEEDCOPY_PARAMBLOCK_H_
#define  _PFOPERATORSPEEDCOPY_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSpeedCopyDesc.h"

namespace PFActions {

// block IDs
enum { kSpeedCopy_mainPBlockIndex };


// param IDs
enum {	kSpeedCopy_accelLimit,
		kSpeedCopy_influence,
		kSpeedCopy_useSpeedVariation,
		kSpeedCopy_speedMin,
		kSpeedCopy_speedMax,
		kSpeedCopy_useOrient,
		kSpeedCopy_steer,
		kSpeedCopy_distance,
		kSpeedCopy_syncByParams,
		kSpeedCopy_syncByIcon,
		kSpeedCopy_iconSize,
		kSpeedCopy_seed,
		kSpeedCopy_colorType
};

enum {	kSpeedCopy_syncBy_time,
		kSpeedCopy_syncBy_age,
		kSpeedCopy_syncBy_event,
		kSpeedCopy_syncBy_num = 3
};

// color type
enum {  kSpeedCopy_colorType_test,
		kSpeedCopy_colorType_coordinated,
		kSpeedCopy_colorType_num = 2
};

// User Defined Reference Messages from PB
enum {	kSpeedCopy_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSpeedCopy_RefMsg_NewRand
};

// dialog messages
enum {	kSpeedCopy_message_update = 100	// settings change
};

class PFOperatorSpeedCopyDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	void UpdateDlg( HWND hWnd, IParamBlock2* pblock );
};

extern PFOperatorSpeedCopyDesc ThePFOperatorSpeedCopyDesc;
extern ParamBlockDesc2 speedCopy_paramBlockDesc;
extern FPInterfaceDesc speedCopy_action_FPInterfaceDesc;
extern FPInterfaceDesc speedCopy_operator_FPInterfaceDesc;
extern FPInterfaceDesc speedCopy_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSPEEDCOPY_PARAMBLOCK_H_