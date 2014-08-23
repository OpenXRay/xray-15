/**********************************************************************
 *<
	FILE:			PFOperatorSimpleBirth_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleBirth Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLEBIRTH_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLEBIRTH_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleBirthDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleBirth_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// param IDs
enum {	kSimpleBirth_start, // start of emission
		kSimpleBirth_finish, // end of emission
		kSimpleBirth_type, // amount or rate
		kSimpleBirth_amount, // amount of particles emitted
		kSimpleBirth_oldRate, // old rate of emission (particles per frame, int)
		kSimpleBirth_subframe, // exact_frame or even_distribution
		kSimpleBirth_rate // rate of emission (particles per second, float)
};

// birth types
enum {	kSimpleBirth_type_amount,	// generate by amount
		kSimpleBirth_type_rate		// generate by rate
};

enum { kSimpleBirth_amountMax = 10000000 };

// User Defined Reference Messages from PB
enum { kSimpleBirth_RefMsg_InitDlg = REFMSG_USER + 13279
};

// dialog messages
enum {	kSimpleBirth_message_type = 100,	// type change message
		kSimpleBirth_message_amount			// amount change message
};

class PFOperatorSimpleBirthDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	void UpdateBirthTypeDlg( HWND hWnd, int birthType );
	void UpdateTotalAmountDlg( HWND hWnd, int amount );
};


extern PFOperatorSimpleBirthDesc ThePFOperatorSimpleBirthDesc;
extern ParamBlockDesc2 simpleBirth_paramBlockDesc;
extern FPInterfaceDesc simpleBirth_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleBirth_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleBirth_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLEBIRTH_PARAMBLOCK_H_