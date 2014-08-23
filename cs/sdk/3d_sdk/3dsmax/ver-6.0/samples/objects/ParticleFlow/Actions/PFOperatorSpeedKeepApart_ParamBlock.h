/**********************************************************************
 *<
	FILE:			PFOperatorSpeedKeepApart_ParamBlock.h

	DESCRIPTION:	ParamBlocks2 for SpeedKeepApart Operator (declaration)
											 
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-15-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#ifndef  _PFOPERATORSPEEDKEEPAPART_PARAMBLOCK_H_
#define  _PFOPERATORSPEEDKEEPAPART_PARAMBLOCK_H_

#include "max.h"
#include "iparamm2.h"

#include "PFOperatorSpeedKeepApartDesc.h"

namespace PFActions {

// block IDs
enum {	kSpeedKeepApart_mainPBlockIndex,
		kSpeedKeepApart_scriptPBlockIndex };

// param IDs
enum {	kSpeedKeepApart_force,
		kSpeedKeepApart_useAccelLimit,
		kSpeedKeepApart_accelLimit,
		kSpeedKeepApart_useSpeedLimit,
		kSpeedKeepApart_speedLimit,
		kSpeedKeepApart_rangeType,
		kSpeedKeepApart_coreSize,
		kSpeedKeepApart_falloffSize,
		kSpeedKeepApart_coreRatio,
		kSpeedKeepApart_falloffRatio,
		kSpeedKeepApart_variation,
		kSpeedKeepApart_scopeType,
		kSpeedKeepApart_groups,
		kSpeedKeepApart_systems,
		kSpeedKeepApart_seed
};

// rangeType
enum {	kSpeedKeepApart_rangeType_absolute,
		kSpeedKeepApart_rangeType_relative,
		kSpeedKeepApart_rangeType_num = 2
};

// scopeType
enum {	kSpeedKeepApart_scopeType_currentGroup,
		kSpeedKeepApart_scopeType_currentSystem,
		kSpeedKeepApart_scopeType_selectedGroups,
		kSpeedKeepApart_scopeType_selectedSystems,
		kSpeedKeepApart_scopeType_num = 4
};

// use script wiring param IDs
enum {	kSpeedKeepApart_useScriptWiring,
		kSpeedKeepApart_useFloat,
		kSpeedKeepApart_useInt, // for future use
		kSpeedKeepApart_useVector,
		kSpeedKeepApart_useMatrix // for future use
};

// float wiring
enum {	kSpeedKeepApart_useFloat_none,
		kSpeedKeepApart_useFloat_force,
		kSpeedKeepApart_useFloat_num = 2
};

// vector wiring
enum {	kSpeedKeepApart_useVector_none,
		kSpeedKeepApart_useVector_absRange,
		kSpeedKeepApart_useVector_relRange,
		kSpeedKeepApart_useVector_num = 3
};

// user messages
enum {	kSpeedKeepApart_RefMsg_NewRand = REFMSG_USER + 13279,
		kSpeedKeepApart_RefMsg_InitDlg,
		kSpeedKeepApart_RefMsg_MacrorecGroups,
		kSpeedKeepApart_RefMsg_MacrorecSystems };

// dialog messages
enum {	kSpeedKeepApart_message_update = 100,	// settings change
		kSpeedKeepApart_message_enableForce,
		kSpeedKeepApart_message_disableForce,
		kSpeedKeepApart_message_enableAbsRange,
		kSpeedKeepApart_message_disableAbsRange,
		kSpeedKeepApart_message_enableRelRange,
		kSpeedKeepApart_message_disableRelRange,
		kSpeedKeepApart_message_enableVariation,
		kSpeedKeepApart_message_disableVariation
};

class PFOperatorSpeedKeepApartDlgProc : public ParamMap2UserDlgProc
{
public:
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() {}

	int NumMaps();
	IParamMap2* GetMap(int index);
	void UpdateActionListDlg( IParamMap2* map );
	void UpdatePSystemDlg( IParamMap2* map );

private:
	void UpdateDlg( HWND hWnd, IParamBlock2* pblock );
	void AddMap(IParamMap2* map);
	void RemoveMap(IParamMap2* map);
	void SelectActionLists( HWND hWnd, IParamBlock2* pblock );
	void SelectPSystems( HWND hWnd, IParamBlock2* pblock );
	void UpdateForceDlg( HWND hWnd, bool enable);
	void UpdateAbsRangeDlg( HWND hWnd, bool enable);
	void UpdateRelRangeDlg( HWND hWnd, bool enable);
	void UpdateVariationDlg( HWND hWnd, bool enable);

	Tab<IParamMap2*> m_activeMaps; // list of currently open maps
};

bool SKAIsSelectedActionList(INode* node, IParamBlock2* pblock);
bool SKAIsSelectedPSystem(INode* node, IParamBlock2* pblock);

extern PFOperatorSpeedKeepApartDesc ThePFOperatorSpeedKeepApartDesc;
extern ParamBlockDesc2 speedKeepApart_paramBlockDesc;
extern ParamBlockDesc2 speedKeepApart_scriptBlockDesc;
extern FPInterfaceDesc speedKeepApart_action_FPInterfaceDesc;
extern FPInterfaceDesc speedKeepApart_operator_FPInterfaceDesc;
extern FPInterfaceDesc speedKeepApart_PViewItem_FPInterfaceDesc;


} // end of namespace PFActions

#endif // _PFOPERATORSPEEDKEEPAPART_PARAMBLOCK_H_