/**********************************************************************
 *<
	FILE: reactorui.cpp

	DESCRIPTION: MAXScript support

	CREATED BY: Adam Felt

	HISTORY: created 2/20/99

 *>	Copyright (c) 1999 Adam Felt, All Rights Reserved.
 **********************************************************************/
#include "MAXScrpt.h"
#include "Numbers.h"
#include "3DMath.h"
#include "Name.h"
#include "strings.h"
#include "MAXobj.h"
#include "ReactAPI.h"
#include "reactor.h"

// Maxscript stuff
def_visible_primitive (createReaction,			"createReaction" );
def_visible_primitive (deleteReaction,			"deleteReaction" );
def_visible_primitive (selectReaction,			"selectReaction" );
def_visible_primitive (getSelectedReaction,		"getSelectedReactionNum" );

def_visible_primitive (setReactionState,		"setReactionState" );
def_visible_primitive (setReactionValue,		"setReactionValue" );
def_visible_primitive (setReactionInfluence,	"setReactionInfluence" );
def_visible_primitive (setReactionStrength,		"setReactionStrength" );
def_visible_primitive (setReactionFalloff,		"setReactionFalloff" );
def_visible_primitive (setReactionName,			"setReactionName" );

def_visible_primitive (getReactionCount,		"getReactionCount" );
def_visible_primitive (getReactionInfluence,	"getReactionInfluence" );
def_visible_primitive (getReactionStrength,		"getReactionStrength" );
def_visible_primitive (getReactionFalloff,		"getReactionFalloff" );
def_visible_primitive (getReactionName,			"getReactionName" );
def_visible_primitive (getReactionState,		"getReactionState" );
def_visible_primitive (getReactionValue,		"getReactionValue" );
def_visible_primitive (reactTo,					"reactTo" );


#define get_reactor_cont()															\
	Control *cont = arg_list[0]->to_controller();									\
	Class_ID id = cont->ClassID();													\
	if ( id != REACTORFLOAT_CLASS_ID && id != REACTORPOS_CLASS_ID &&				\
			id != REACTORP3_CLASS_ID && id != REACTORROT_CLASS_ID &&				\
			id !=REACTORSCALE_CLASS_ID )											\
		throw RuntimeError(GetString(IDS_NOT_A_REACTOR), arg_list[0]);				\
	IReactor* react = (IReactor*)cont;			


Value*
setReactionState_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionState, 3, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	Point3 p;
	float f;
	Quat q;

	if (sel>0 && sel <= react->getReactionCount())
	{
		switch (react->getType())
		{
			case REACTORFLOAT: 
				if (is_float(arg_list[2]))
				{
					f = arg_list[2]->to_float();
					react->setState(sel-1, f);
					break;
				}else throw RuntimeError(GetString(IDS_NOT_A_VALID_STATE), arg_list[2]);
			case REACTORROT: 
				if (is_quat(arg_list[2]))
				{
					q = arg_list[2]->to_quat();
					react->setState(sel -1, q);
					break;
				}else throw RuntimeError(GetString(IDS_NOT_A_VALID_STATE), arg_list[2]);
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				if (is_point3(arg_list[2]))
				{
					p = arg_list[2]->to_point3();
					react->setState(sel-1, p);
					break;
				}else throw RuntimeError(GetString(IDS_NOT_A_VALID_STATE), arg_list[2]);
			default: break;
		}
	}
	return &ok;
}


Value*
getReactionState_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionState, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);

	if (sel>0 && sel <= react->getReactionCount())
	{
		switch (react->getType())
		{
			case REACTORFLOAT: 
				return Float::intern(*((float*)react->getState(sel-1))); 
			case REACTORROT: 
				return new QuatValue(*((Quat*)react->getState(sel-1))); 
			case REACTORP3: 
			case REACTORSCALE: 
			case REACTORPOS: 
				return new Point3Value(*((Point3*)react->getState(sel-1))); 
			default: 
				return &false_value; 
		}
	}
	return Name::intern(GetString(IDS_NOT_AN_INT));
}

Value*
setReactionValue_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionState, 3, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);

	Point3 p;
	float f;
	Quat q;
	
	if (sel>0 && sel <= react->getReactionCount())
	{
		switch (react->getReactionType())
		{
			case FLOAT_VAR: 
				if (is_float(arg_list[2]))
				{
					f = arg_list[2]->to_float();
					react->setReactionValue(sel-1, f);
					break;
				}else throw RuntimeError(GetString(IDS_NOT_A_VALID_VALUE), arg_list[2]);
			case QUAT_VAR: 
				if (is_quat(arg_list[2]))
				{
					q = arg_list[2]->to_quat();
					react->setReactionValue(sel-1, q);
					break;
				}else throw RuntimeError(GetString(IDS_NOT_A_VALID_VALUE), arg_list[2]);
			case VECTOR_VAR: 
			case SCALE_VAR: 
				if (is_point3(arg_list[2]))
				{
					p = arg_list[2]->to_point3();
					react->setReactionValue(sel-1, p);
					break;
				}else throw RuntimeError(GetString(IDS_NOT_A_VALID_VALUE), arg_list[2]);
			default: 
				break;
		}
	}
	return &ok;
}


Value*
getReactionValue_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionState, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);

	if (sel>0 && sel <= react->getReactionCount())
	{
		switch (react->getReactionType())
		{
			case FLOAT_VAR: 
				return Float::intern(*((float*)react->getReactionValue(sel-1))); 
			case QUAT_VAR: 
				return new QuatValue(*((Quat*)react->getReactionValue(sel-1))); 
			case VECTOR_VAR: 
			case SCALE_VAR: 
				return new Point3Value(*((Point3*)react->getReactionValue(sel-1))); 
			default: 
				return &false_value; 
		}
	}
	return Name::intern(GetString(IDS_NOT_AN_INT));
}


Value*
reactTo_cf(Value** arg_list, int count)
{
	check_arg_count(reactTo, 2, count);
	get_reactor_cont();

//	Animatable* anim = ((MAXWrapper*)arg_list[1])->get_max_object();
	
	if ( is_node(arg_list[1]) )
	{
		INode* c = arg_list[1]->to_node();
		react->reactTo(((ReferenceTarget*)c));
		return &ok;
	}
	else if ( is_controller(arg_list[1]) )
	{
		Control* c = arg_list[1]->to_controller();
		react->reactTo(((ReferenceTarget*)c));
		return &ok;
	}
		else throw RuntimeError("Not a Controller or a Node: ", arg_list[1]);
	return NULL;
				// Win64 Cleanup: Shuler
				// Compiler wants a return value. How about NULL. 
				// Should never happen, given the logic.
}


Value*
createReaction_cf(Value** arg_list, int count)
{
	check_arg_count(createReaction, 1, count);
	get_reactor_cont();
	react->CreateReaction(NULL, MAXScript_time());
	MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
	return &ok;
}

Value*
deleteReaction_cf(Value** arg_list, int count)
{
	check_arg_count(deleteReaction, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	if (sel>0 && sel <= react->getReactionCount()) react->DeleteReaction(sel-1);
	return &ok;
}

Value*
getSelectedReaction_cf(Value** arg_list, int count)
{
	check_arg_count(getSelectedReaction, 1, count);
	get_reactor_cont();
	return Integer::intern(react->getSelected()+1);
}

Value*
selectReaction_cf(Value** arg_list, int count)
{
	check_arg_count(selectReaction, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	if (sel>0 && sel <= react->getReactionCount()) react->setSelected(sel-1);
	return &ok;
}


Value*
setReactionInfluence_cf(Value** arg_list, int count)
{
	check_arg_count(setReactionInfluence, 3, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	float inf;
	if (is_float(arg_list[2])||is_integer(arg_list[2])) inf = arg_list[2]->to_float();
		else throw RuntimeError(GetString(IDS_NOT_A_FLOAT), arg_list[2]);
	if (sel>0 && sel <= react->getReactionCount()) react->setInfluence(sel-1, inf);
	return &ok;
}

Value*
setReactionStrength_cf(Value** arg_list, int count)
{
	check_arg_count(setReactionStrength, 3, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	float str;
	if (is_float(arg_list[2])||is_integer(arg_list[2])) str = arg_list[2]->to_float();
		else throw RuntimeError(GetString(IDS_NOT_A_FLOAT), arg_list[2]);
	if (sel>0 && sel <= react->getReactionCount()) react->setStrength(sel-1, str);
	return &ok;
}

Value*
setReactionFalloff_cf(Value** arg_list, int count)
{
	check_arg_count(setReactionFalloff, 3, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	float fall;
	if (is_float(arg_list[2])||is_integer(arg_list[2])) fall = arg_list[2]->to_float();
		else throw RuntimeError(GetString(IDS_NOT_A_FLOAT), arg_list[2]);
	if (sel>0 && sel <= react->getReactionCount()) react->setFalloff(sel-1, fall);
	return &ok;
}

Value*
setReactionName_cf(Value** arg_list, int count)
{
	check_arg_count(setReactionName, 3, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	TCHAR *name;
	if (is_string(arg_list[2])) name = arg_list[2]->to_string();
		else throw RuntimeError(GetString(IDS_NOT_A_NAME), arg_list[2]);
	if (sel>0 && sel <= react->getReactionCount()) react->setReactionName(sel-1, name);
	react->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	return &ok;
}

Value*
getReactionCount_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionCount, 1, count);
	get_reactor_cont();
	return Integer::intern(react->getReactionCount());
}

Value*
getReactionInfluence_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionInfluence, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	if (sel>0 && sel <= react->getReactionCount()) return Float::intern(react->getInfluence(sel-1));
	else return &false_value; 
}

Value*
getReactionStrength_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionStrength, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	if (sel>0 && sel <= react->getReactionCount()) return Float::intern(react->getStrength(sel-1));
	else return &false_value; 
}

Value*
getReactionFalloff_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionFalloff, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	if (sel>0 && sel <= react->getReactionCount()) return Float::intern(react->getFalloff(sel-1));
	else return &false_value; 
}

Value*
getReactionName_cf(Value** arg_list, int count)
{
	check_arg_count(getReactionName, 2, count);
	get_reactor_cont();
	int sel;
	if (is_integer(arg_list[1])) sel = arg_list[1]->to_int();
		else throw RuntimeError(GetString(IDS_NOT_AN_INT), arg_list[1]);
	if (sel>0 && sel <= react->getReactionCount()) return Name::intern(react->getReactionName(sel-1));
	else return &false_value; 
}


// action table
static ActionDescription spActions[] = {

	ID_MIN_INFLUENCE,
    IDS_MIN_INFLUENCE,
    IDS_MIN_INFLUENCE,
    IDS_AF_REACTOR,

    ID_MAX_INFLUENCE,
    IDS_MAX_INFLUENCE,
    IDS_MAX_INFLUENCE,
    IDS_AF_REACTOR,

    ID_CREATE_REACTION,
    IDS_CREATE_REACTION,
    IDS_CREATE_REACTION,
    IDS_AF_REACTOR,

    ID_DELETE_REACTION,
    IDS_DELETE_REACTION,
    IDS_DELETE_REACTION,
    IDS_AF_REACTOR,

    ID_SET_VALUE,
    IDS_SET_VALUE,
    IDS_SET_VALUE,
    IDS_AF_REACTOR,

    ID_EDIT_STATE,
    IDS_EDIT_STATE,
    IDS_EDIT_STATE,
    IDS_AF_REACTOR,

};

ActionTable* GetActions()
{
    TSTR name = GetString(IDS_AF_REACTOR);
    HACCEL hAccel = LoadAccelerators(hInstance,
                                     MAKEINTRESOURCE(IDR_REACTOR_SHORTCUTS));
    int numOps = NumElements(spActions);
    ActionTable* pTab;
    pTab = new ActionTable(kReactorActions, kReactorContext, name, hAccel, numOps,
                             spActions, hInstance);
    GetCOREInterface()->GetActionManager()->RegisterActionContext(kReactorContext, name.data());

    return pTab;
}


