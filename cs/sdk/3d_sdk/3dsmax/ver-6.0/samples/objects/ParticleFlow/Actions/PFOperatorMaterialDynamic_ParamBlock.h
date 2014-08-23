/**********************************************************************
 *<
	FILE:			PFOperatorMaterialDynamic_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for MaterialDynamic Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORMATERIALDYNAMIC_PARAMBLOCK_H_
#define  _PFOPERATORMATERIALDYNAMIC_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorMaterialDynamicDesc.h"

namespace PFActions {

const int	kMaterialDynamic_minNumMtls = 0;
const float kMaterialDynamic_minRate = 0.00001f;
const float kMaterialDynamic_maxRate = 1000000.0f;


// block IDs
enum { kMaterialDynamic_mainPBlockIndex };


// param IDs
enum {	kMaterialDynamic_assignMaterial,
		kMaterialDynamic_material,
		kMaterialDynamic_assignID,
		kMaterialDynamic_showInViewport,
		kMaterialDynamic_type,
		kMaterialDynamic_resetAge,
		kMaterialDynamic_randomizeAge,
		kMaterialDynamic_maxAgeOffset,
		kMaterialDynamic_materialID,
		kMaterialDynamic_numSubMtls,
		kMaterialDynamic_rate,
		kMaterialDynamic_loop,
		kMaterialDynamic_sync,
		kMaterialDynamic_randomizeRotoscoping,
		kMaterialDynamic_maxRandOffset,
		kMaterialDynamic_randomSeed };

enum {	kMaterialDynamic_type_particleID,
		kMaterialDynamic_type_materialID,
		kMaterialDynamic_type_cycle,
		kMaterialDynamic_type_random,
		kMaterialDynamic_type_num=4 };

enum {	kMaterialDynamic_sync_time,
		kMaterialDynamic_sync_age,
		kMaterialDynamic_sync_event,
		kMaterialDynamic_sync_num=3 };

// User Defined Reference Messages from PB
enum {	kMaterialDynamic_RefMsg_InitDlg = REFMSG_USER + 13279,
		kMaterialDynamic_RefMsg_NewRand };

// dialog messages
enum {	kMaterialDynamic_message_assignMaterial = 100,	// assign material
		kMaterialDynamic_message_assignID };			// assign material ID


class PFOperatorMaterialDynamicDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateAssignMaterialDlg( HWND hWnd, int assign );
	static void UpdateAssignIDDlg( HWND hWnd, int assign, int type, int randAge, int randOffset);
};

extern PFOperatorMaterialDynamicDesc ThePFOperatorMaterialDynamicDesc;
extern ParamBlockDesc2 materialDynamic_paramBlockDesc;
extern FPInterfaceDesc materialDynamic_action_FPInterfaceDesc;
extern FPInterfaceDesc materialDynamic_operator_FPInterfaceDesc;
extern FPInterfaceDesc materialDynamic_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORMATERIALDYNAMIC_PARAMBLOCK_H_