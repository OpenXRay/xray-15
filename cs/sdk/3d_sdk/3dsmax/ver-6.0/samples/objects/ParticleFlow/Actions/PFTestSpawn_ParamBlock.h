/**********************************************************************
 *<
	FILE:			PFTestSpawn_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for Duration Test (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 12-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFTESTSPAWN_PARAMBLOCK_H_
#define  _PFTESTSPAWN_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFTestSpawnDesc.h"

namespace PFActions {

// block IDs
enum { kSpawn_mainPBlockIndex };

//	block ref no
//enum { pm_setup_ref_no, pm_display_ref_no, pm_memman_ref_no};

// paramIDs
enum {	kSpawn_spawnType, // once, or per frame, or by distance
		kSpawn_deleteParent,
		kSpawn_spawnRate,
		kSpawn_stepSize,
		kSpawn_spawnAble,
		kSpawn_numOffsprings,
		kSpawn_numVariation,
		kSpawn_syncBy,
		kSpawn_restartAge,
		kSpawn_speedType, // in units, or inherited
		kSpawn_speedUnits,
		kSpawn_speedInherited,
		kSpawn_speedVariation,
		kSpawn_speedDivergence,
		kSpawn_scale,
		kSpawn_scaleVariation,
		kSpawn_randomSeed
};

enum {	kSpawn_spawnType_once,
		kSpawn_spawnType_perFrame,
		kSpawn_spawnType_byDistance,
		kSpawn_spawnType_num = 3
};

enum {	kSpawn_syncBy_time,
		kSpawn_syncBy_age,
		kSpawn_syncBy_event,
		kSpawn_syncBy_num = 3
};

enum {	kSpawn_speedType_units,
		kSpawn_speedType_inherited,
		kSpawn_speedType_num = 2
};

// User Defined Reference Messages from PB
enum {	kSpawn_RefMsg_InitDlg = REFMSG_USER + 13279,
		kSpawn_RefMsg_NewRand
};

// dialog messages
enum {	kSpawn_message_speedType = 100,	// speed type change message
		kSpawn_message_spawnType,		// spawn type change message
		kSpawn_message_useRandom		// message sent when parameters changed that influence usage of randomness
};


#define kSpawn_frameRate_min 0.00001f
#define kSpawn_frameRate_max 9999999.0f
#define kSpawn_stepSize_min 0.00001f
#define kSpawn_stepSize_max 9999999.0f

extern PFTestSpawnDesc ThePFTestSpawnDesc;
extern ParamBlockDesc2 spawn_paramBlockDesc;
extern FPInterfaceDesc spawn_action_FPInterfaceDesc;
extern FPInterfaceDesc spawn_test_FPInterfaceDesc;
extern FPInterfaceDesc spawn_PViewItem_FPInterfaceDesc;

class TestSpawnDlgProc : public ParamMap2UserDlgProc 
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }
private:
	void UpdateSpeedTypeDlg( HWND hWnd, int speedType );
	void UpdateSpawnTypeDlg( HWND hWnd, int spawnType );
	void UpdateUseRandomDlg( HWND hWnd, int useRandom );
};


} // end of namespace PFActions

#endif // _PFTESTSPAWN_PARAMBLOCK_H_