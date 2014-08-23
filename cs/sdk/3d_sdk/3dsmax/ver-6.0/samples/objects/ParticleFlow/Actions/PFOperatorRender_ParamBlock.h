/**********************************************************************
 *<
	FILE:			PFOperatorRender_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Render Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 11-06-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORRENDER_PARAMBLOCK_H_
#define  _PFOPERATORRENDER_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorRenderDesc.h"

namespace PFActions {

// block IDs
enum { kRender_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};


// param IDs
enum {	kRender_type,
		kRender_visible,
		kRender_splitType,
		kRender_meshCount,
		kRender_perMeshCount
};

enum {	kRender_type_none,
		kRender_type_boundingBoxes,
		kRender_type_geometry,
		kRender_type_phantom,
		kRender_type_num=4 };

enum {	kRender_splitType_single,
		kRender_splitType_multiple,
		kRender_splitType_particle,
		kRender_splitType_num=3 
};

enum {	kRender_vertCount_limit = 5000000, kRender_faceCount_limit = 5000000 };

// User Defined Reference Messages from PB
enum {	kRender_RefMsg_InitDlg = REFMSG_USER + 13279,
};

// dialog messages
enum {	kRender_message_type = 100		// change in split type
};

class PFOperatorRenderDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}
private:
	static void UpdateTypeDlg( HWND hWnd, int type, int splitType);
};

extern PFOperatorRenderDesc ThePFOperatorRenderDesc;
extern ParamBlockDesc2 render_paramBlockDesc;
extern FPInterfaceDesc render_action_FPInterfaceDesc;
extern FPInterfaceDesc render_operator_FPInterfaceDesc;
extern FPInterfaceDesc render_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORRENDER_PARAMBLOCK_H_