/*===========================================================================*\
 | 
 |  FILE:	MCFunc.cpp
 |			Implimentations of the UI/toplevel functions of MorpherMXS
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
 |	Given a modifier, return TRUE if Morpher, FALSE if not
\*===========================================================================*/

Value*
is_morpher_cf(Value** arg_list, int count)
{
	check_arg_count(is_morpher, 1, count);
	type_check(arg_list[0], MAXModifier, "IsValidMorpherMod [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	return &true_value;
}


/*===========================================================================*\
 |	Refresh the channel list UI
\*===========================================================================*/

Value*
wm3_refeshchannelUI_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_refeshchannelUI, 1, count);
	type_check(arg_list[0], MAXModifier, "WM3_RefreshChannelListUI [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->Update_channelFULL();

	return &true_value;
}


/*===========================================================================*\
 |	Refresh the channel parameters' UI
\*===========================================================================*/

Value*
wm3_refeshmcpUI_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_refeshmcpUI, 1, count);
	type_check(arg_list[0], MAXModifier, "WM3_RefreshChannelParamsUI [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->Update_channelParams();

	return &true_value;
}


/*===========================================================================*\
 |	Set the position of the scroll bar (auto-clamped by VScroll fn) (1-91)
\*===========================================================================*/

Value*
wm3_setscrollpos_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_setscrollpos, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_SetChannelPos [Morpher Modifier] [NewPos]");
	type_check(arg_list[1], Integer, "WM3_SetChannelPos [Morpher Modifier] [NewPos]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->VScroll(SB_THUMBPOSITION,(arg_list[1]->to_int()-1));

	return &true_value;
}


/*===========================================================================*\
 |	Set which button is selected on the channel list (1-10)
\*===========================================================================*/

Value*
wm3_setscrollsel_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_setscrollsel, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_SetChannelSel [Morpher Modifier] [NewSel]");
	type_check(arg_list[1], Integer, "WM3_SetChannelSel [Morpher Modifier] [NewSel]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>9) sel = 9;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanSel = sel;
	mp->Update_channelFULL();

	return &true_value;
}