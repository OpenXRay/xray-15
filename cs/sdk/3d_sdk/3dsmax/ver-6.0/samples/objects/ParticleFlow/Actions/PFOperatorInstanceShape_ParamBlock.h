/**********************************************************************
 *<
	FILE:			PFOperatorInstanceShape_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for InstanceShape Operator (declaration)
											 
	CREATED BY:		Chung-An Lin

	HISTORY:		created 01-04-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORINSTANCESHAPE_PARAMBLOCK_H_
#define  _PFOPERATORINSTANCESHAPE_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorInstanceShapeDesc.h"

namespace PFActions {

// block IDs
enum { kInstanceShape_mainPBlockIndex };


// param IDs
enum {	kInstanceShape_sepGroup,
		kInstanceShape_sepHierarchy,
		kInstanceShape_sepElements,
		kInstanceShape_scale,
		kInstanceShape_variation,
		kInstanceShape_mapping,
		kInstanceShape_material,
		kInstanceShape_randomShape,
		kInstanceShape_animatedShape,
		kInstanceShape_acquireShape,
		kInstanceShape_syncType,
		kInstanceShape_syncRandom,
		kInstanceShape_randomOffset,
		kInstanceShape_randomSeed,
		kInstanceShape_setScale,
		kInstanceShape_objectMaxscript,
		kInstanceShape_objectList };

enum {	kInstanceShape_syncBy_absoluteTime,
		kInstanceShape_syncBy_particleAge,
		kInstanceShape_syncBy_eventStart,
		kInstanceShape_syncBy_num=3 };


// User Defined Reference Messages from PB
enum {	kInstanceShape_RefMsg_InitDlg = REFMSG_USER + 13279,
		kInstanceShape_RefMsg_PickObj,
		kInstanceShape_RefMsg_UpdateShape,
		kInstanceShape_RefMsg_NewRand
};

// dialog messages
enum {	kInstanceShape_message_name = 100,	// object name changed message
		kInstanceShape_message_type,		// instance type changed message
		kInstanceShape_message_enterPick,	// enter pick mode
		kInstanceShape_message_exitPick,	// exit pick mode
		kInstanceShape_message_animate,		// animated shape changed message
		kInstanceShape_message_setScale		// set scale setting is changed
};

class PFOperatorInstanceShapeDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	void DeleteThis() {}
private:
	static void UpdateObjectNameDlg( HWND hWnd, TSTR& objectName );
	static void UpdateParticleShapeDlg( HWND hWnd, const Tab<Mesh*>& meshList );
	static void UpdateRandomShapeDlg( HWND hWnd, BOOL separateAny );
	static void UpdateAnimatedSahpeDlg( HWND hWnd, BOOL animatedShape, BOOL addRandomOffset );
	static void UpdateScaleDlg( HWND hWnd, BOOL setScale);
};

class PFOperatorInstanceShapeMXSValidator : public PBValidator
{
public:
	BOOL Validate(PB2Value& v);
};

extern PFOperatorInstanceShapeDesc ThePFOperatorInstanceShapeDesc;
extern ParamBlockDesc2 instanceShape_paramBlockDesc;
extern FPInterfaceDesc instanceShape_action_FPInterfaceDesc;
extern FPInterfaceDesc instanceShape_operator_FPInterfaceDesc;
extern FPInterfaceDesc instanceShape_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORINSTANCESHAPE_PARAMBLOCK_H_