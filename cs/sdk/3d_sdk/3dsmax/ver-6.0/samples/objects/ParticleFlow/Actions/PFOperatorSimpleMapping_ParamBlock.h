/**********************************************************************
 *<
	FILE:			PFOperatorSimpleMapping_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SimpleMapping Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORMARKSHAPE_PARAMBLOCK_H_
#define  _PFOPERATORMARKSHAPE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSimpleMappingDesc.h"

namespace PFActions {

// block IDs
enum { kSimpleMapping_mainPBlockIndex };


// param IDs
enum {	kSimpleMapping_U,
		kSimpleMapping_V,
		kSimpleMapping_W,
		kSimpleMapping_syncBy,
		kSimpleMapping_channelType,
		kSimpleMapping_mapChannel,
		kSimpleMapping_showInViewport };

enum {	kSimpleMapping_syncBy_time,
		kSimpleMapping_syncBy_age,
		kSimpleMapping_syncBy_event,
		kSimpleMapping_syncBy_num=3 };

enum {	kSimpleMapping_channelType_map,
		kSimpleMapping_channelType_vertexColor,
		kSimpleMapping_channelType_num=2 }; 


// User Defined Reference Messages from PB
enum {	kSimpleMapping_RefMsg_InitDlg = REFMSG_USER + 13279 };

// dialog messages
enum {	kSimpleMapping_message_channelType = 100 };	// change of channel type: map or vertex color


class PFOperatorSimpleMappingDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateChannelTypeDlg( HWND hWnd, int type );
};

class PFOperatorSimpleMappingSyncTypeValidator: public PBValidator
{
	BOOL Validate(PB2Value& v);
};

class PFOperatorSimpleMappingChannelTypeValidator: public PBValidator
{
	BOOL Validate(PB2Value& v);
};

class PFOperatorSimpleMappingMapChannelValidator: public PBValidator
{
	BOOL Validate(PB2Value& v);
};

extern PFOperatorSimpleMappingDesc ThePFOperatorSimpleMappingDesc;
extern ParamBlockDesc2 simpleMapping_paramBlockDesc;
extern FPInterfaceDesc simpleMapping_action_FPInterfaceDesc;
extern FPInterfaceDesc simpleMapping_operator_FPInterfaceDesc;
extern FPInterfaceDesc simpleMapping_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORMARKSHAPE_PARAMBLOCK_H_