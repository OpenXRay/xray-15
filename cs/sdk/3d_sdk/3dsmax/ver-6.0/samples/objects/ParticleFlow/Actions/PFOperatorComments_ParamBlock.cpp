/**********************************************************************
 *<
	FILE:			PFOperatorComments_ParamBlock.cpp

	DESCRIPTION:	ParamBlocks2 for Comments Operator (definition)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 08-27-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#include "PFOperatorComments_ParamBlock.h"

#include "PFActions_GlobalVariables.h"
#include "PFActions_GlobalFunctions.h"
#include "IPFAction.h"
#include "IPFOperator.h"
#include "IPViewItem.h"

#include "resource.h"

namespace PFActions {

// Each plug-in should have one instance of the ClassDesc class
PFOperatorCommentsDesc ThePFOperatorCommentsDesc;

// Dialog Proc
PFOperatorCommentsDlgProc ThePFOperatorCommentsDlgProc;

// Comments Operator param block
ParamBlockDesc2 comments_paramBlockDesc 
(
	// required block spec
		kComments_mainPBlockIndex, 
		_T("Parameters"),
		IDS_PARAMETERS,
		&ThePFOperatorCommentsDesc,
		P_AUTO_CONSTRUCT + P_AUTO_UI,
	// auto constuct block refno : none
		kComments_mainPBlockIndex,
	// auto ui parammap specs : none
		IDD_COMMENTS_PARAMETERS, 
		IDS_PARAMETERS,
		0,
		0, // open/closed
		&ThePFOperatorCommentsDlgProc,
	// required param specs
		// comments
			kComments_comments,		_T("Comments"),
									TYPE_STRING,
									P_RESET_DEFAULT,
									IDS_COMMENTS,
			// optional tagged param specs
					p_default,		GetString(IDS_TYPEYOURCOMMENTSHERE),
//					p_ui,			TYPE_EDITBOX, IDC_COMMENTS,
			end,
		end 
);

//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
//|							Function Publishing System						 |
//+>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>+
FPInterfaceDesc comments_action_FPInterfaceDesc(
			PFACTION_INTERFACE, 
			_T("action"), 0, 
			&ThePFOperatorCommentsDesc, FP_MIXIN, 
				
			IPFAction::kInit,	_T("init"),		0,	TYPE_bool, 0, 5,
				_T("container"), 0, TYPE_IOBJECT,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actions"), 0, TYPE_OBJECT_TAB_BR,
				_T("actionNodes"), 0, TYPE_INODE_TAB_BR, 
			IPFAction::kRelease, _T("release"), 0, TYPE_bool, 0, 1,
				_T("container"), 0, TYPE_IOBJECT,
			// reserved for future use
			//IPFAction::kChannelsUsed, _T("channelsUsed"), 0, TYPE_VOID, 0, 2,
			//	_T("interval"), 0, TYPE_INTERVAL_BR,
			//	_T("channels"), 0, TYPE_FPVALUE,
			IPFAction::kActivityInterval, _T("activityInterval"), 0, TYPE_INTERVAL_BV, 0, 0,
			IPFAction::kIsFertile, _T("isFertile"), 0, TYPE_bool, 0, 0,
			IPFAction::kIsNonExecutable, _T("isNonExecutable"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportRand, _T("supportRand"), 0, TYPE_bool, 0, 0,
			IPFAction::kIsMaterialHolder, _T("isMaterialHolder"), 0, TYPE_bool, 0, 0,
			IPFAction::kSupportScriptWiring, _T("supportScriptWiring"), 0, TYPE_bool, 0, 0,

		end
);

FPInterfaceDesc comments_operator_FPInterfaceDesc(
			PFOPERATOR_INTERFACE,
			_T("operator"), 0,
			&ThePFOperatorCommentsDesc, FP_MIXIN,

			IPFOperator::kProceed, _T("proceed"), 0, TYPE_bool, 0, 7,
				_T("container"), 0, TYPE_IOBJECT,
				_T("timeStart"), 0, TYPE_TIMEVALUE,
				_T("timeEnd"), 0, TYPE_TIMEVALUE_BR,
				_T("particleSystem"), 0, TYPE_OBJECT,
				_T("particleSystemNode"), 0, TYPE_INODE,
				_T("actionNode"), 0, TYPE_INODE,
				_T("integrator"), 0, TYPE_INTERFACE,

		end
);

FPInterfaceDesc comments_PViewItem_FPInterfaceDesc(
			PVIEWITEM_INTERFACE,
			_T("PViewItem"), 0,
			&ThePFOperatorCommentsDesc, FP_MIXIN,

			IPViewItem::kNumPViewParamBlocks, _T("numPViewParamBlocks"), 0, TYPE_INT, 0, 0,
			IPViewItem::kGetPViewParamBlock, _T("getPViewParamBlock"), 0, TYPE_REFTARG, 0, 1,
				_T("index"), 0, TYPE_INDEX,
			IPViewItem::kHasComments, _T("hasComments"), 0, TYPE_bool, 0, 1,
				_T("actionNode"), 0, TYPE_INODE,
			IPViewItem::kGetComments, _T("getComments"), 0, TYPE_STRING, 0, 1,
				_T("actionNode"), 0, TYPE_INODE,
			IPViewItem::kSetComments, _T("setComments"), 0, TYPE_VOID, 0, 2,
				_T("actionNode"), 0, TYPE_INODE,
				_T("comments"), 0, TYPE_STRING,

		end
);

BOOL PFOperatorCommentsDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hWnd,
											UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
			if (map != NULL) {
				IParamBlock2* pblock = map->GetParamBlock();
				if (pblock != NULL) {
					CStr val = pblock->GetStr(kComments_comments, 0);
					SetWindowText( GetDlgItem( hWnd, IDC_COMMENTS), val);
				}
			}
			break;
		case WM_DESTROY:
/*			if (map != NULL) {
				IParamBlock2* pblock = map->GetParamBlock();
				if (pblock != NULL) {
					TCHAR val[2048];
					GetWindowText( GetDlgItem( hWnd, IDC_COMMENTS), val, 2047);
					pblock->SetValue(kComments_comments, 0, val);
				}
			}
*/			break;

		case WM_COMMAND: 
			switch( LOWORD(wParam) ) {
				case IDC_COMMENTS:
					switch(HIWORD(wParam)) {
						case EN_SETFOCUS:
							DisableAccelerators();					
							break;
						case EN_KILLFOCUS:
							EnableAccelerators();
							break;
						case EN_CHANGE: {
							int len = SendDlgItemMessage(hWnd, IDC_COMMENTS, WM_GETTEXTLENGTH, 0, 0);
							TSTR temp;
							temp.Resize(len+1);
							SendDlgItemMessage(hWnd, IDC_COMMENTS, WM_GETTEXT, len+1, (LPARAM)temp.data());
							if (map != NULL) {
								IParamBlock2* pblock = map->GetParamBlock();
								if (pblock != NULL)
									pblock->SetValue(kComments_comments, 0, temp);
							}
										}
							break;
					}
					break;
			}
			break;
	}

	return FALSE;
}

} // end of namespace PFActions