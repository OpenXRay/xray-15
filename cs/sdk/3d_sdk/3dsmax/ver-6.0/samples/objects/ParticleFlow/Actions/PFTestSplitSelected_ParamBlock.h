/**********************************************************************
 *<
	FILE:			PFTestSplitSelected_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SplitSelected Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 09-12-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPLITSELECTED_PARAMBLOCK_H_
#define  _PFTESTSPLITSELECTED_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSplitSelectedDesc.h"

namespace PFActions {

// block IDs
enum { kSplitSelected_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kSplitSelected_conditionType
};

enum {	kSplitSelected_conditionType_selected,
		kSplitSelected_conditionType_notSelected
};

extern PFTestSplitSelectedDesc ThePFTestSplitSelectedDesc;
extern ParamBlockDesc2 splitSelected_paramBlockDesc;
extern FPInterfaceDesc splitSelected_action_FPInterfaceDesc;
extern FPInterfaceDesc splitSelected_test_FPInterfaceDesc;
extern FPInterfaceDesc splitSelected_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFTESTSPLITSELECTED_PARAMBLOCK_H_