/*===========================================================================*\
 | 
 |  FILE:	MorpherMXS.cpp
 |			A new MAX Script Plugin that adds Morpher modifier access
 |			3D Studio MAX R3.0
 | 
 |  AUTH:   Harry Denholm
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 5-4-99
 | 
\*===========================================================================*/

#include "MorpherMXS.h"




/*===========================================================================*\
 |	Define our new functions
\*===========================================================================*/

// To check whether a modifier is a valid Morpher mod
def_visible_primitive( is_morpher,				"IsValidMorpherMod");

// UI access
def_visible_primitive( wm3_refeshchannelUI,		"WM3_RefreshChannelListUI");
def_visible_primitive( wm3_refeshmcpUI,			"WM3_RefreshChannelParamsUI");
def_visible_primitive( wm3_setscrollpos,		"WM3_SetChannelPos");
def_visible_primitive( wm3_setscrollsel,		"WM3_SetChannelSel");

// Morph Channel access
def_visible_primitive( wm3_mc_bfn,				"WM3_MC_BuildFromNode");
def_visible_primitive( wm3_mc_rebuild,			"WM3_MC_Rebuild");
def_visible_primitive( wm3_mc_delete,			"WM3_MC_Delete");

def_visible_primitive( wm3_mc_getname,			"WM3_MC_GetName");
def_visible_primitive( wm3_mc_setname,			"WM3_MC_SetName");

def_visible_primitive( wm3_mc_getamount,		"WM3_MC_GetValue");
def_visible_primitive( wm3_mc_setamount,		"WM3_MC_SetValue");

def_visible_primitive( wm3_mc_hastarget,		"WM3_MC_HasTarget");
def_visible_primitive( wm3_mc_gettarget,		"WM3_MC_GetTarget");

def_visible_primitive( wm3_mc_isvalid,			"WM3_MC_IsValid");
def_visible_primitive( wm3_mc_hasdata,			"WM3_MC_HasData");

def_visible_primitive( wm3_mc_isactive,			"WM3_MC_IsActive");
def_visible_primitive( wm3_mc_setactive,		"WM3_MC_SetActive");

def_visible_primitive( wm3_mc_getuselimits,		"WM3_MC_GetUseLimits");
def_visible_primitive( wm3_mc_setuselimits,		"WM3_MC_SetUseLimits");

def_visible_primitive( wm3_mc_getlimitmax,		"WM3_MC_GetLimitMAX");
def_visible_primitive( wm3_mc_getlimitmin,		"WM3_MC_GetLimitMIN");
def_visible_primitive( wm3_mc_setlimitmax,		"WM3_MC_SetLimitMAX");
def_visible_primitive( wm3_mc_setlimitmin,		"WM3_MC_SetLimitMIN");

def_visible_primitive( wm3_mc_getvertsel,		"WM3_MC_GetUseVertexSel");
def_visible_primitive( wm3_mc_setvertsel,		"WM3_MC_SetUseVertexSel");

def_visible_primitive( wm3_mc_getmemuse,		"WM3_MC_GetMemUse");

// Lowlevel morph info access
def_visible_primitive( wm3_mc_getnumpts,		"WM3_MC_NumPts");
def_visible_primitive( wm3_mc_getnummpts,		"WM3_MC_NumMPts");
def_visible_primitive( wm3_mc_getmorphpoint,	"WM3_MC_GetMorphPoint");
def_visible_primitive( wm3_mc_getmorphweight,	"WM3_MC_GetMorphWeight");

// Cache access
def_visible_primitive( wm3_rebuildIMC,			"WM3_RebuildInternalCache");



/*===========================================================================*\
 |	Check to see if modifier is valid as a Morpher Mod
\*===========================================================================*/

BOOL check_ValidMorpher(ReferenceTarget* obj,Value** arg_list){
	if(obj->ClassID()!=Class_ID(0x17bb6854, 0xa5cba2a3)) return FALSE;
	else return TRUE;
}


/*===========================================================================*\
 |	MAXScript Plugin Initialization
\*===========================================================================*/

__declspec( dllexport ) void
LibInit() { 
}

