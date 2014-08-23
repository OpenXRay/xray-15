/**********************************************************************
 *<
	FILE:			PFOperatorFacingShape_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for FacingShape Operator (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-21-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORFACINGSHAPE_PARAMBLOCK_H_
#define  _PFOPERATORFACINGSHAPE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorFacingShapeDesc.h"

namespace PFActions {

const float kFacingShape_minUnits = 0.0000001f;
const float kFacingShape_minProportion = 0.001f;
const float kFacingShape_maxProportion = 100.0f;
const float kFacingShape_minWHRatio = 0.001f;
const float kFacingShape_maxWHRatio = 1000.0f;
const int	kFacingShape_minNumMtls = 1;
const float kFacingShape_minRate = 0.01f;
const float kFacingShape_maxRate = 100.0f;


// block IDs
enum { kFacingShape_mainPBlockIndex };


// param IDs
enum {	kFacingShape_parallel,
		kFacingShape_sizeSpace,
		kFacingShape_units,
		kFacingShape_inheritedScale,
		kFacingShape_proportion,
		kFacingShape_variation,
		kFacingShape_pivotAt,
		kFacingShape_WHRatio,
		kFacingShape_useMaterial_obsolete,
		kFacingShape_material_obsolete,
		kFacingShape_numSubMtls_obsolete,
		kFacingShape_subsPerFrame_obsolete,
		kFacingShape_loop_obsolete,
		kFacingShape_syncType_obsolete,
		kFacingShape_syncRandom_obsolete,
		kFacingShape_randomOffset_obsolete,
		kFacingShape_randomSeed,
		kFacingShape_orientation,
		kFacingShape_objectMaxscript };

enum {	kFacingShape_sizeSpace_world,
		kFacingShape_sizeSpace_local,
		kFacingShape_sizeSpace_screen,
		kFacingShape_sizeSpace_num=3 };

enum {	kFacingShape_pivotAt_top,
		kFacingShape_pivotAt_center,
		kFacingShape_pivotAt_bottom,
		kFacingShape_pivotAt_num=3 };

enum {	kFacingShape_syncBy_absoluteTime,
		kFacingShape_syncBy_particleAge,
		kFacingShape_syncBy_eventStart,
		kFacingShape_syncBy_num=3 };

enum {	kFacingShape_orientation_horizon,
		kFacingShape_orientation_speed,
		kFacingShape_orientation_random,
		kFacingShape_orientation_spin,
		kFacingShape_orientation_num=4 };


// User Defined Reference Messages from PB
enum {	kFacingShape_RefMsg_InitDlg = REFMSG_USER + 13279,
		kFacingShape_RefMsg_PickObj,
		kFacingShape_RefMsg_NewRand };

// dialog messages
enum {	kFacingShape_message_name = 100,	// object name changed message
		kFacingShape_message_enterPick,		// enter pick mode
		kFacingShape_message_exitPick,		// exit pick mode
		kFacingShape_message_size };		// size/width parameters

class PFOperatorFacingShapeDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateObjectNameDlg( HWND hWnd, TSTR& objectName );
	static void UpdateWidthSizeDlg( HWND hWnd, BOOL isCameraObj, BOOL parallelDir, int sizeSpace );
	static void UpdateMaterialDlg( HWND hWnd, BOOL useMaterial, BOOL addRandomOffset );
	static void UpdateRandomOffsetDlg( HWND hWnd, BOOL enableRandomOffset );
};


extern PFOperatorFacingShapeDesc ThePFOperatorFacingShapeDesc;
extern ParamBlockDesc2 facingShape_paramBlockDesc;
extern FPInterfaceDesc facingShape_action_FPInterfaceDesc;
extern FPInterfaceDesc facingShape_operator_FPInterfaceDesc;
extern FPInterfaceDesc facingShape_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORFACINGSHAPE_PARAMBLOCK_H_