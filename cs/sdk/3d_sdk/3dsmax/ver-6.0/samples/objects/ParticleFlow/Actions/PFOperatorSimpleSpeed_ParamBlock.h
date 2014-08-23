/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpeed_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpeed Operator (declaration)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 01-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLESPEED_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLESPEED_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleSpeedDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleSpeed_mainPBlockIndex };


// param IDs
enum {	kSimpleSpeed_speed, kSimpleSpeed_variation, kSimpleSpeed_direction,
		kSimpleSpeed_reverse, kSimpleSpeed_divergence, kSimpleSpeed_seed };

// directions
enum {	kSS_Along_Icon_Arrow,
		kSS_Icon_Center_Out,
		kSS_Icon_Arrow_Out,
		kSS_Rand_3D,
		kSS_Rand_Horiz,
		kSS_Inherit_Prev = 5 };

// user messages
enum { kSimpleSpeed_RefMsg_NewRand = REFMSG_USER + 13279 };

class PFOperatorSimpleSpeedDlgProc : public ParamMap2UserDlgProc
{
	void SetCtlByDir(IParamMap2* map, HWND hWnd);
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};

extern PFOperatorSimpleSpeedDesc ThePFOperatorSimpleSpeedDesc;
extern ParamBlockDesc2 simpleSpeed_paramBlockDesc;
extern FPInterfaceDesc simpleSpeed_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleSpeed_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleSpeed_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESPEED_PARAMBLOCK_H_