/**********************************************************************
 *<
	FILE:			PFOperatorSimpleSpin_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleSpin Operator (declaration)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 02-01-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLESPIN_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLESPIN_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleSpinDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleSpin_mainPBlockIndex };


// param IDs
enum {	kSimpleSpin_spinrate, kSimpleSpin_variation, kSimpleSpin_direction,
		kSimpleSpin_X, kSimpleSpin_Y, kSimpleSpin_Z, kSimpleSpin_divergence, kSimpleSpin_seed };

// directions
enum {	kSS_Rand_3D,
		kSS_World_Space,
		kSS_Particle_Space,
		kSS_Speed_Space,
		kSS_Speed_Space_Follow = 4 };

// user messages
enum { kSimpleSpin_RefMsg_NewRand = REFMSG_USER + 13279 };

// dialog messages
enum {	kSimpleSpin_message_update = 100 };		// change in parameters

class PFOperatorSimpleSpinDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateDlg(IParamMap2* map, HWND hWnd);
};

class PFOperatorSimpleSpinValidator: public PBValidator
{
	BOOL Validate(PB2Value& v);
};

extern PFOperatorSimpleSpinDesc ThePFOperatorSimpleSpinDesc;
extern ParamBlockDesc2 simpleSpin_paramBlockDesc;
extern FPInterfaceDesc simpleSpin_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleSpin_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleSpin_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESPIN_PARAMBLOCK_H_