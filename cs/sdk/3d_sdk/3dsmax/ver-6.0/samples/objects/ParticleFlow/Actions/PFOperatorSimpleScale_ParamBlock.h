/**********************************************************************
 *<
	FILE:			PFOperatorSimpleScale_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleScale Operator (declaration)
											 
	CREATED BY:		David C. Thompson

	HISTORY:		created 07-22-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLESCALE_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLESCALE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleScaleDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleScale_mainPBlockIndex };


// param IDs
enum {	kSimpleScale_Type, 
		kSimpleScale_XScaleFactor, kSimpleScale_YScaleFactor, kSimpleScale_ZScaleFactor,
		kSimpleScale_sfConstrain,
		kSimpleScale_XScaleVariation, kSimpleScale_YScaleVariation, kSimpleScale_ZScaleVariation,
		kSimpleScale_svConstrain, kSimpleScale_svBias,
		kSimpleScale_Sync,
		kSimpleScale_Seed };

// types
enum {	kSS_OverwriteOnce,
		kSS_InheritOnce,
		kSS_Absolute,
		kSS_RelativeFirst,
		kSS_RelativeSuccessive = 4 };

// bias
enum {	kSS_None,
		kSS_Centered,
		kSS_TowardsMin,
		kSS_TowardsMax = 3 };

// sync
enum {	kSS_AbsoluteTime,
		kSS_ParticleAge,
		kSS_EventDuration = 2 };

// user messages
enum {	kSimpleScale_RefMsg_NewRand = REFMSG_USER + 13279,
		kSimpleScale_RefMsg_InitDialog,
		kSimpleScale_RefMsg_DestroyDialog };

class PFOperatorSimpleScaleDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};

extern PFOperatorSimpleScaleDesc ThePFOperatorSimpleScaleDesc;
extern ParamBlockDesc2 simpleScale_paramBlockDesc;
extern FPInterfaceDesc simpleScale_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleScale_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleScale_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESCALE_PARAMBLOCK_H_