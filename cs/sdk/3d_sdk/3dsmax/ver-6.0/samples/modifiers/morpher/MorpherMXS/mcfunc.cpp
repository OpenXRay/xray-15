/*===========================================================================*\
 | 
 |  FILE:	MCFunc.cpp
 |			Implimentations of the moprhChannel functions of MorpherMXS
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
 |	Given the <target object> from the scene, this function initializes the 
 |  channel with all necessary data
\*===========================================================================*/

Value*
wm3_mc_bfn_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_bfn, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_BuildFromNode [Morpher Modifier] [Channel Index] [Target]");
	type_check(arg_list[1], Integer, "WM3_MC_BuildFromNode [Morpher Modifier] [Channel Index] [Target]");
	type_check(arg_list[2], MAXNode, "WM3_MC_BuildFromNode [Morpher Modifier] [Channel Index] [Target]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	INode *node = arg_list[2]->to_node();
	mp->ReplaceReference(101+sel,node);
	mp->chanBank[sel].buildFromNode( node );

	return &true_value;
}


/*===========================================================================*\
 |	Rebuilds optimization and morph data in this channel
 |  Use this after changing the channel's target
\*===========================================================================*/

Value*
wm3_mc_rebuild_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_rebuild, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_Rebuild [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_Rebuild [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanBank[sel].rebuildChannel();

	return &true_value;
}


/*===========================================================================*\
 |	Deletes the channel
\*===========================================================================*/

Value*
wm3_mc_delete_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_delete, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_Delete [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_Delete [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mConnection) 
		mp->DeleteReference(101+sel);

	// Ask channel to reset itself
	mp->chanBank[sel].ResetMe();

	// Reassign paramblock info
	ParamBlockDescID *channelParams = new ParamBlockDescID[1];

	ParamBlockDescID add;
	add.type=TYPE_FLOAT;
	add.user=NULL;
	add.animatable=TRUE;
	add.id=1;
	channelParams[0] = add;

	mp->MakeRefByID(FOREVER, 1+sel, CreateParameterBlock(channelParams,1,1));	
	assert(mp->chanBank[sel].cblock);

	Control *c = (Control*)CreateInstance(CTRL_FLOAT_CLASS_ID,GetDefaultController(CTRL_FLOAT_CLASS_ID)->ClassID());

	mp->chanBank[sel].cblock->SetValue(0,0,0.0f);
	mp->chanBank[sel].cblock->SetController(0,c);

	delete channelParams;

	// Refresh system
	mp->Update_channelFULL();
	mp->Update_channelParams();	
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}


/*===========================================================================*\
 |	Retrieves the name of the morpher channel
\*===========================================================================*/

Value*
wm3_mc_getname_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getname, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetName [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetName [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return new String(mp->chanBank[sel].mName);
}


/*===========================================================================*\
 |	Sets the name of the channel to be <string name>
\*===========================================================================*/

Value*
wm3_mc_setname_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setname, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetName [Morpher Modifier] [Channel Index] [Name String]");
	type_check(arg_list[1], Integer, "WM3_MC_SetName [Morpher Modifier] [Channel Index] [Name String]");
	type_check(arg_list[2], String, "WM3_MC_SetName [Morpher Modifier] [Channel Index] [Name String]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanBank[sel].mName = arg_list[2]->to_string();
	return &true_value;
}


/*===========================================================================*\
 |	Returns the weighting value of the channel
\*===========================================================================*/

Value*
wm3_mc_getamount_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getamount, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetValue [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetValue [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	// The value of the channel - ie, its weighted percentage
	float tmpVal;
	mp->chanBank[sel].cblock->GetValue(0, MAXScript_interface->GetTime(), tmpVal, FOREVER);

	return Float::intern(tmpVal);
}


/*===========================================================================*\
 |	Sets the weighted value of the channel
\*===========================================================================*/

Value*
wm3_mc_setamount_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setamount, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetValue [Morpher Modifier] [Channel Index] [Value]");
	type_check(arg_list[1], Integer, "WM3_MC_SetValue [Morpher Modifier] [Channel Index] [Value]");
	type_check(arg_list[2], Float, "WM3_MC_SetValue [Morpher Modifier] [Channel Index] [Value]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->chanBank[sel].cblock->SetValue(0, MAXScript_interface->GetTime(), arg_list[2]->to_float());
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel has an active connection to a scene object
\*===========================================================================*/

Value*
wm3_mc_hastarget_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_hastarget, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_HasTarget [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_HasTarget [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mConnection!=NULL) return &true_value;
	else return &false_value;
}


/*===========================================================================*\
 |	Returns a pointer to the object in the scene the channel is connected to
\*===========================================================================*/

Value*
wm3_mc_gettarget_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_gettarget, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetTarget [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetTarget [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mConnection!=NULL) return new MAXNode(mp->chanBank[sel].mConnection);
	else return &false_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel has not been marked as an invalid channel
\*===========================================================================*/

Value*
wm3_mc_isvalid_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_isvalid, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_IsValid [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_IsValid [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(!mp->chanBank[sel].mInvalid) return &true_value;
	else return &false_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel has some morpher data in it (Indicator: BLUE)
\*===========================================================================*/

Value*
wm3_mc_hasdata_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_hasdata, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_HasData [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_HasData [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mActive) return &true_value;
	else return &false_value;
}


/*===========================================================================*\
 |	Returns TRUE if the channel is turned on and used in the Morph
\*===========================================================================*/

Value*
wm3_mc_isactive_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_isactive, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_IsActive [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_IsActive [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mActiveOverride) return &true_value;
	return &false_value;
}



/*===========================================================================*\
 |	Sets wether or not the channel is used in the morph results or not
\*===========================================================================*/

Value*
wm3_mc_setactive_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setactive, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetActive [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[1], Integer, "WM3_MC_SetActive [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[2], Boolean, "WM3_MC_SetActive [Morpher Modifier] [Channel Index] [true/false]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mActiveOverride = arg_list[2]->to_bool();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}



/*===========================================================================*\
 |	Returns TRUE if the 'Use Limits' checkbox is on
\*===========================================================================*/

Value*
wm3_mc_getuselimits_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getuselimits, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetUseLimits [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetUseLimits [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mUseLimit) return &true_value;
	return &false_value;
}


/*===========================================================================*\
 |	Turns on and off the 'Use Limits' checkbox
\*===========================================================================*/

Value*
wm3_mc_setuselimits_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setuselimits, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetUseLimits [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[1], Integer, "WM3_MC_SetUseLimits [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[2], Boolean, "WM3_MC_SetUseLimits [Morpher Modifier] [Channel Index] [true/false]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mUseLimit = arg_list[2]->to_bool();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}


/*===========================================================================*\
 |	Returns the upper limit for the channel values (only used if 'Use Limits' is on)
\*===========================================================================*/

Value*
wm3_mc_getlimitmax_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getlimitmax, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetLimitMAX [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetLimitMAX [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Float::intern(mp->chanBank[sel].mSpinmax);
}


/*===========================================================================*\
 |	Returns the lower limit for the channel values (only used if 'Use Limits' is on)
\*===========================================================================*/

Value*
wm3_mc_getlimitmin_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getlimitmin, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetLimitMIN [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetLimitMIN [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Float::intern(mp->chanBank[sel].mSpinmin);
}


/*===========================================================================*\
 |	Sets the high limit for the channel's value
\*===========================================================================*/

Value*
wm3_mc_setlimitmax_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setlimitmax, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetLimitMAX [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[1], Integer, "WM3_MC_SetLimitMAX [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[2], Float, "WM3_MC_SetLimitMAX [Morpher Modifier] [Channel Index] [Float Value]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mSpinmax = arg_list[2]->to_float();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}


/*===========================================================================*\
 |	Sets the lower limit for the channel's value
\*===========================================================================*/

Value*
wm3_mc_setlimitmin_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setlimitmin, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetLimitMIN [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[1], Integer, "WM3_MC_SetLimitMIN [Morpher Modifier] [Channel Index] [Float Value]");
	type_check(arg_list[2], Float, "WM3_MC_SetLimitMIN [Morpher Modifier] [Channel Index] [Float Value]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mSpinmin = arg_list[2]->to_float();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}


/*===========================================================================*\
 |	Returns TRUE if the 'Use Vertex Selection' button is on
\*===========================================================================*/

Value*
wm3_mc_getvertsel_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getvertsel, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetUseVertexSel [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetUseVertexSel [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	if(mp->chanBank[sel].mUseSel) return &true_value;
	return &false_value;
}


/*===========================================================================*\
 |	Sets whether or not to use vertex selection in this channel
\*===========================================================================*/

Value*
wm3_mc_setvertsel_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_setvertsel, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_SetUseVertexSel [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[1], Integer, "WM3_MC_SetUseVertexSel [Morpher Modifier] [Channel Index] [true/false]");
	type_check(arg_list[2], Boolean, "WM3_MC_SetUseVertexSel [Morpher Modifier] [Channel Index] [true/false]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	mp->chanBank[sel].mUseSel = arg_list[2]->to_bool();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}


/*===========================================================================*\
 |	Returns an estimation of how many bytes this channel takes up in memory
\*===========================================================================*/

Value*
wm3_mc_getmemuse_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getmemuse, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetMemUse [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetMemUse [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	float tmSize = 0.0f;
	tmSize += mp->chanBank[sel].getMemSize();

	return Float::intern(tmSize);
}


/*===========================================================================*\
 |	The actual number of points in this channel
\*===========================================================================*/

Value*
wm3_mc_getnumpts_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getnumpts, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_NumPts [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_NumPts [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Integer::intern(mp->chanBank[sel].mNumPoints);
}


/*===========================================================================*\
 |  The number of 'morphable points' in this channel
 |  'morphable points' are those that are different to the original object
\*===========================================================================*/

Value*
wm3_mc_getnummpts_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getnummpts, 2, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_NumMPts [Morpher Modifier] [Channel Index]");
	type_check(arg_list[1], Integer, "WM3_MC_NumMPts [Morpher Modifier] [Channel Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Integer::intern(mp->chanBank[sel].mPoints.size());
}


/*===========================================================================*\
 |	Gets a Point3 value of the <index> point in the channel
\*===========================================================================*/

Value*
wm3_mc_getmorphpoint_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getmorphpoint, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetMorphPoint [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetMorphPoint [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[2], Integer, "WM3_MC_GetMorphPoint [Morpher Modifier] [Channel Index] [Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return new Point3Value(mp->chanBank[sel].mPoints[arg_list[2]->to_int()]);
}


/*===========================================================================*\
 |	Gets a floating point value of the <index> weight in the channel
\*===========================================================================*/

Value*
wm3_mc_getmorphweight_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_mc_getmorphweight, 3, count);
	type_check(arg_list[0], MAXModifier, "WM3_MC_GetMorphWeight [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[1], Integer, "WM3_MC_GetMorphWeight [Morpher Modifier] [Channel Index] [Index]");
	type_check(arg_list[2], Integer, "WM3_MC_GetMorphWeight [Morpher Modifier] [Channel Index] [Index]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	int sel = arg_list[1]->to_int(); sel -= 1;
	if(sel>100) sel = 99;
	if(sel<0) sel = 0;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();

	return Float::intern((float)mp->chanBank[sel].mWeights[arg_list[2]->to_int()]);
}


/*===========================================================================*\
 |	Resets the internal object cache, forcing a complete rebuild.
\*===========================================================================*/

Value*
wm3_rebuildIMC_cf(Value** arg_list, int count)
{
	check_arg_count(wm3_rebuildIMC, 1, count);
	type_check(arg_list[0], MAXModifier, "WM3_RebuildInternalCache [Morpher Modifier]");

	ReferenceTarget* obj = arg_list[0]->to_modifier();	
	if( !check_ValidMorpher(obj,arg_list) ) return &false_value;

	MorphR3 *mp = (MorphR3*)arg_list[0]->to_modifier();
	mp->MC_Local.NukeCache();
	mp->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);

	Interface *ip = MAXScript_interface;
	ip->RedrawViews(ip->GetTime());

	return &true_value;
}
