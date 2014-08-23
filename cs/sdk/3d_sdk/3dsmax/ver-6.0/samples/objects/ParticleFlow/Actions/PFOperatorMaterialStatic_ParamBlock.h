/**********************************************************************
 *<
	FILE:			PFOperatorMaterialStatic_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for MaterialStatic Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 06-14-2002

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORMATERIALSTATIC_PARAMBLOCK_H_
#define  _PFOPERATORMATERIALSTATIC_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorMaterialStaticDesc.h"

namespace PFActions {

const int	kMaterialStatic_minNumMtls = 0;
const float kMaterialStatic_minRate = 0.0001f;
const float kMaterialStatic_maxRate = 1000000.0f;


// block IDs
enum { kMaterialStatic_mainPBlockIndex };


// param IDs
enum {	kMaterialStatic_assignMaterial,
		kMaterialStatic_material,
		kMaterialStatic_assignID,
		kMaterialStatic_showInViewport,
		kMaterialStatic_type,
		kMaterialStatic_materialID,
		kMaterialStatic_numSubMtls,
		kMaterialStatic_rateType,
		kMaterialStatic_ratePerSecond,
		kMaterialStatic_ratePerParticle,
		kMaterialStatic_loop,
		kMaterialStatic_randomSeed };

enum {	kMaterialStatic_type_id,
		kMaterialStatic_type_cycle,
		kMaterialStatic_type_random,
		kMaterialStatic_type_num=3 };

enum {	kMaterialStatic_rateType_second,
		kMaterialStatic_rateType_particle,
		kMaterialStatic_rateType_num=2 };

// User Defined Reference Messages from PB
enum {	kMaterialStatic_RefMsg_InitDlg = REFMSG_USER + 13279,
		kMaterialStatic_RefMsg_NewRand };

// dialog messages
enum {	kMaterialStatic_message_assignMaterial = 100,		// assign material
		kMaterialStatic_message_assignID,					// assign material ID
		kMaterialStatic_message_type,						// change in type: material ID, cycle or random
		kMaterialStatic_message_rateType };					// change in rate type: second or particle

class PFOperatorMaterialStaticDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateAssignMaterialDlg( HWND hWnd, int assign);
	static void UpdateAssignIDDlg( HWND hWnd, int assign, int type, int rateType);
};

extern PFOperatorMaterialStaticDesc ThePFOperatorMaterialStaticDesc;
extern ParamBlockDesc2 materialStatic_paramBlockDesc;
extern FPInterfaceDesc materialStatic_action_FPInterfaceDesc;
extern FPInterfaceDesc materialStatic_operator_FPInterfaceDesc;
extern FPInterfaceDesc materialStatic_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORMATERIALSTATIC_PARAMBLOCK_H_