/**********************************************************************
 *<
	FILE:			PFOperatorSimpleOrientation_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleOrientation Operator (declaration)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 01-24-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLEORIENTATION_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLEORIENTATION_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleOrientationDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleOrientation_mainPBlockIndex };


// param IDs
enum {	kSimpleOrientation_direction, 
		kSimpleOrientation_x, 
		kSimpleOrientation_y,
		kSimpleOrientation_z, 
		kSimpleOrientation_divergence, 
		kSimpleOrientation_seed,
		kSimpleOrientation_restrictToAxis,
		kSimpleOrientation_axisX,
		kSimpleOrientation_axisY,
		kSimpleOrientation_axisZ };

// directions
enum {	kSO_Rand_3D,
		kSO_Rand_Horiz,
		kSO_World,
		kSO_Speed,
		kSO_SpeedFollow,
		kSO_type_num=5 };

// user messages
enum {	kSimpleOrientation_RefMsg_Init = REFMSG_USER + 13279,
		kSimpleOrientation_RefMsg_NewRand };

// dialog messages
enum {	kSimpleOrientation_message_update = 100	};	// update parameters availability

class PFOperatorSimpleOrientationDlgProc : public ParamMap2UserDlgProc
{
	void SetCtlByDir(IParamMap2* map, HWND hWnd);
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};

extern PFOperatorSimpleOrientationDesc ThePFOperatorSimpleOrientationDesc;
extern ParamBlockDesc2 simpleOrientation_paramBlockDesc;
extern FPInterfaceDesc simpleOrientation_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleOrientation_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleOrientation_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEORIENTATION_PARAMBLOCK_H_