/**********************************************************************
 *<
	FILE:			PFOperatorComments_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Comments Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 08-27-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORCOMMENTS_PARAMBLOCK_H_
#define  _PFOPERATORCOMMENTS_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorCommentsDesc.h"

namespace PFActions {

// block IDs
enum { kComments_mainPBlockIndex };


// param IDs
enum {	kComments_comments };

class PFOperatorCommentsDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
};

extern PFOperatorCommentsDesc ThePFOperatorCommentsDesc;
extern ParamBlockDesc2 comments_paramBlockDesc;
extern FPInterfaceDesc comments_action_FPInterfaceDesc;
extern FPInterfaceDesc comments_operator_FPInterfaceDesc;
extern FPInterfaceDesc comments_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORCOMMENTS_PARAMBLOCK_H_