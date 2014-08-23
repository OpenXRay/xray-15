/**********************************************************************
 *<
	FILE:			PFOperatorExit_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Exit Operator (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATOREXIT_PARAMBLOCK_H_
#define  _PFOPERATOREXIT_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorExitDesc.h"

namespace PFActions {

// block IDs
enum { kExit_mainPBlockIndex };


// param IDs
enum {	kExit_type,				// exit type all/selected/by age
		kExit_lifeSpan,			// life span of particles if exit by age
		kExit_variation,		// variation of life span
		kExit_randomSeed		// random seed
};

enum {	kExit_type_all,
		kExit_type_selected,
		kExit_type_byAge,
		kExit_type_num=3
};


// User Defined Reference Messages from PB
enum {	kExit_RefMsg_InitDlg = REFMSG_USER + 13279,
		kExit_RefMsg_NewRand
};

// dialog messages
enum {	kExit_message_type = 100	// exit type change message
};


class PFOperatorExitDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	void UpdateLifeSpanDlg( HWND hWnd, int exitType );
};


extern PFOperatorExitDesc ThePFOperatorExitDesc;
extern ParamBlockDesc2 exit_paramBlockDesc;
extern FPInterfaceDesc exit_action_FPInterfaceDesc;
extern FPInterfaceDesc exit_operator_FPInterfaceDesc;
extern FPInterfaceDesc exit_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATOREXIT_PARAMBLOCK_H_