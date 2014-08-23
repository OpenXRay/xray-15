/**********************************************************************
 *<
	FILE:			PFTestCollisionSpaceWarp_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for CollisionSpaceWarp Test (declaration)
											 
	CREATED BY:		Peter Watje

	HISTORY:		created 2-07-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTCOLLISIONSPACEWARP_PARAMBLOCK_H_
#define  _PFTESTCOLLISIONSPACEWARP_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestCollisionSpaceWarpDesc.h"

namespace PFActions {

// block IDs
enum { kCollisionSpaceWarp_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {
		kCollisionSpaceWarp_CollisionNode,
		kCollisionSpaceWarp_CollisionNodelist,

		kCollisionSpaceWarp_CollisionTestType,
		kCollisionSpaceWarp_CollisionSpeedOption,

		kCollisionSpaceWarp_CollisionMinSpeed,

		kCollisionSpaceWarp_randomSeed,
		kCollisionSpaceWarp_CollisionMaxSpeed,
		kCollisionSpaceWarp_CollisionWillCollideFrame,
		kCollisionSpaceWarp_CollisionCount,
		kCollisionSpaceWarp_CollisionCountOptions,
		kCollisionSpaceWarp_CollisionsMaxscript // deflectors list for maxscript manipulation
};

enum {	
		kCollisionSpaceWarp_SpeedBounce,
		kCollisionSpaceWarp_SpeedContinue,
		kCollisionSpaceWarp_SpeedStop,
		kCollisionSpaceWarp_SpeedRandom = 3,
};

enum {	
	/* kCollisionSpaceWarp_conditionType_less,
		kCollisionSpaceWarp_conditionType_greater
		*/
};

// User Defined Reference Messages from PB
enum {	kCollisionSpaceWarp_RefMsg_InitDlg = REFMSG_USER + 13279,
		kCollisionSpaceWarp_RefMsg_NewRand,
		kCollisionSpaceWarp_RefMsg_ListSelect, // add more nodes by select dialog
		kCollisionSpaceWarp_RefMsg_ResetValidatorAction 
};

// dialog messages
enum {	kCollisionSpaceWarp_message_type = 100,	// type change 
		kCollisionSpaceWarp_message_deflectors // change in number of deflectors
};


extern PFTestCollisionSpaceWarpDesc ThePFTestCollisionSpaceWarpDesc;
extern ParamBlockDesc2 collision_paramBlockDesc;
extern FPInterfaceDesc collision_action_FPInterfaceDesc;
extern FPInterfaceDesc collision_test_FPInterfaceDesc;
extern FPInterfaceDesc collision_PViewItem_FPInterfaceDesc;
extern FPInterfaceDesc collision_integrator_FPInterfaceDesc;

class TestCollisionSpaceWarpDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateTypeDlg( HWND hWnd, int type, int singleType, int multipleType );
	void UpdateDeflectorNumDlg( HWND hWnd, int num );
};


} // end of namespace PFActions

#endif // _PFTESTCOLLISIONSPACEWARP_PARAMBLOCK_H_