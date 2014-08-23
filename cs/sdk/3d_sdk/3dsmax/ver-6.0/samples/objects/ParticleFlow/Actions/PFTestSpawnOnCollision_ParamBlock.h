/**********************************************************************
 *<
	FILE:			PFTestSpawnOnCollision_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Duration Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPAWNONCOLLISION_PARAMBLOCK_H_
#define  _PFTESTSPAWNONCOLLISION_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSpawnOnCollisionDesc.h"

namespace PFActions {

// block IDs
enum { kSpawnCollision_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kSpawnCollision_sendParentOut,
		kSpawnCollision_sendSpawnOut,
		kSpawnCollision_deflectors,
		kSpawnCollision_spawnType, // once, or multiple
		kSpawnCollision_deleteParent,
		kSpawnCollision_untilNum,
		kSpawnCollision_spawnAble,
		kSpawnCollision_numOffsprings,
		kSpawnCollision_numVariation,
		kSpawnCollision_syncBy,
		kSpawnCollision_restartAge,
		kSpawnCollision_speedParent,
		kSpawnCollision_speedOffspring,
		kSpawnCollision_speedType, // in units, or inherited
		kSpawnCollision_speedUnits,
		kSpawnCollision_speedInherited,
		kSpawnCollision_speedVariation,
		kSpawnCollision_speedDivergence,
		kSpawnCollision_scale,
		kSpawnCollision_scaleVariation,
		kSpawnCollision_randomSeed,
		kSpawnCollision_deflectorsMaxscript // deflectors list for maxscript manipulation
};

enum {	kSpawnCollision_spawnType_once,
		kSpawnCollision_spawnType_multiple,
		kSpawnCollision_spawnType_num = 2
};

enum {	kSpawnCollision_syncBy_time,
		kSpawnCollision_syncBy_age,
		kSpawnCollision_syncBy_event,
		kSpawnCollision_syncBy_num = 3
};

enum {	kSpawnCollision_speedType_units,
		kSpawnCollision_speedType_inherited,
		kSpawnCollision_speedType_num = 2
};

enum {	kSpawnCollision_speedType_bounce,
		kSpawnCollision_speedType_continue,
		kSpawnCollision_speedParent_num = 2,
		kSpawnCollision_speedOffspring_num = 2
};

extern PFTestSpawnOnCollisionDesc ThePFTestSpawnOnCollisionDesc;
extern ParamBlockDesc2 spawnOnCollision_paramBlockDesc;
extern FPInterfaceDesc spawnOnCollision_action_FPInterfaceDesc;
extern FPInterfaceDesc spawnOnCollision_test_FPInterfaceDesc;
extern FPInterfaceDesc spawnOnCollision_PViewItem_FPInterfaceDesc;
extern FPInterfaceDesc spawnOnCollision_integrator_FPInterfaceDesc;

// User Defined Reference Messages from PB
enum {	kSpawnCollision_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSpawnCollision_RefMsg_NewRand,
		kSpawnCollision_RefMsg_ListSelect, // add more nodes by select dialog
		kSpawnCollision_RefMsg_ResetValidatorAction 
};

// dialog messages
enum {	kSpawnCollision_message_type = 100,	// type change & deleteParent message
		kSpawnCollision_message_speed,
		kSpawnCollision_message_random,		// change in chaotic parameter
		kSpawnCollision_message_deflectors		// message sent when number of deflectors is changed
};

class TestSpawnOnCollisionDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateSpawnTypeDlg( HWND hWnd, int spawnType, int deleteParent );
	void UpdateSpeedTypeDlg( HWND hWnd, int speedType );
	void UpdateRandomSeedDlg( HWND hWnd, float affects, float multVar, float speedVar, float divergence, float scaleVar );
	void UpdateDeflectorsDlg( HWND hWnd, int numDeflectors );
};



} // end of namespace PFActions

#endif // _PFTESTSPAWNONCOLLISION_PARAMBLOCK_H_