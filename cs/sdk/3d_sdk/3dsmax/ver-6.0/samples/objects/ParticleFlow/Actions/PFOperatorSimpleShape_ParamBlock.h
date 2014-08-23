/**********************************************************************
 *<
	FILE:			PFOperatorSimpleShape_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleShape Operator (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 12-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSIMPLESHAPE_PARAMBLOCK_H_
#define  _PFOPERATORSIMPLESHAPE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleShapeDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleShape_mainPBlockIndex };

// param IDs
enum {	kSimpleShape_shape, 
		kSimpleShape_size,
		kSimpleShape_useScale,
		kSimpleShape_scale };

enum {	kSimpleShape_shape_pyramid, 
		kSimpleShape_shape_cube,
		kSimpleShape_shape_sphere,
		kSimpleShape_shape_vertex,
		kSimpleShape_shape_num=4 };

// User Defined Reference Messages from PB
enum {	kSimpleShape_RefMsg_InitDlg = REFMSG_USER + 13279 };

// dialog messages
enum {	kSimpleShape_message_useScale = 100 };	// useScale changed message

class PFOperatorSimpleShapeDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateScaleDlg( HWND hWnd, int useScale );
};

extern PFOperatorSimpleShapeDesc ThePFOperatorSimpleShapeDesc;
extern ParamBlockDesc2 simpleShape_paramBlockDesc;
extern FPInterfaceDesc simpleShape_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleShape_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleShape_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSIMPLESHAPE_PARAMBLOCK_H_