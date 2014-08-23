/**********************************************************************
 *<
	FILE:			PFOperatorForceSpaceWarp_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Force Space Warp Operator (declaration)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 02-06-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORFORCESPACEWARP_PARAMBLOCK_H_
#define  _PFOPERATORFORCESPACEWARP_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorForceSpaceWarpDesc.h"

namespace PFActions {

// block IDs
enum {	kForceSpaceWarp_mainPBlockIndex,
		kForceSpaceWarp_scriptPBlockIndex };

// param IDs
enum {	kForceSpaceWarp_ForceNode, 
		kForceSpaceWarp_Influence, 
		kForceSpaceWarp_Sync,
		kForceSpaceWarp_ForceNodeList,
		kForceSpaceWarp_Overlapping,
		kForceSpaceWarp_UseAccel,
		kForceSpaceWarp_ForcesMaxscript // forces list for maxscript manipulation
	};

// directions

enum {	kAbsoluteTime,
		kParticleAge,
		kEventDuration = 2
 };

enum {	kForceSpaceWarp_Overlapping_additive,
		kForceSpaceWarp_Overlapping_maximum,
		kForceSpaceWarp_Overlapping_num = 2
};

// use script wiring param IDs
enum {	kForceSpaceWarp_useScriptWiring,
		kForceSpaceWarp_useFloat,
		// for future use
		kForceSpaceWarp_useInt,
		kForceSpaceWarp_useVector,
		kForceSpaceWarp_useMatrix
};

// float wiring
enum {	kForceSpaceWarp_useFloat_none,
		kForceSpaceWarp_useFloat_influence,
		kForceSpaceWarp_useFloat_num = 2
};

// User Defined Reference Messages from PB
enum {	kForceSpaceWarp_RefMsg_InitDlg = REFMSG_USER + 13279,
		kForceSpaceWarp_RefMsg_ListSelect, // add more nodes by select dialog
		kForceSpaceWarp_RefMsg_ResetValidatorAction
};

// dialog messages
enum {	kForceSpaceWarp_message_nodes = 100, // nodes list change message
		kForceSpaceWarp_message_enableInfluence,
		kForceSpaceWarp_message_disableInfluence
};

class PFOperatorForceSpaceWarpDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	void UpdateOverlappingDlg( HWND hWnd, IParamBlock2* pblock);
	void UpdateInfluenceDlg( HWND hWnd, bool enable );
};

extern PFOperatorForceSpaceWarpDesc ThePFOperatorForceSpaceWarpDesc;
extern ParamBlockDesc2 forceSpaceWarp_paramBlockDesc;
extern ParamBlockDesc2 forceSpaceWarp_scriptBlockDesc;
extern FPInterfaceDesc forceSpaceWarp_action_FPInterfaceDesc;
extern FPInterfaceDesc forceSpaceWarp_operator_FPInterfaceDesc;
extern FPInterfaceDesc forceSpaceWarp_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESPEED_PARAMBLOCK_H_