/**********************************************************************
 *<
	FILE: ffdmod.cpp

	DESCRIPTION: DllMain is in here

	CREATED BY: Ravi Karra

	HISTORY: created 1/11/99

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/


#include "ffdmod.h"
#include "ffdui.h"
#include "istdplug.h"
#include "Maxscrpt.h"
#include "3DMath.h"
#include "definsfn.h"

// Maxscript stuff
def_visible_primitive (conform,			"conformToShape" );
def_visible_primitive (getDimensions,	"getDimensions" );
def_visible_primitive (setDimensions,	"setDimensions" );
def_visible_primitive (animateAll,		"animateAll" );
def_visible_primitive (resetLattice,	"resetLattice" );

#define get_ffd_mod()																\
	Modifier *mod = arg_list[0]->to_modifier();										\
	Class_ID id = mod->ClassID();													\
	if ( id != FFDNMOSSQUARE_CLASS_ID && id != FFDNMOSCYL_CLASS_ID &&				\
			id != FFD44_CLASS_ID && id != FFD33_CLASS_ID && id !=FFD22_CLASS_ID )	\
		throw RuntimeError(GetString(IDS_RK_NOT_FFD_ERROR), arg_list[0]);			\
	IFFDMod<Modifier>* ffd = (IFFDMod<Modifier>*)mod;			


Value*
conform_cf(Value** arg_list, int count)
{
	check_arg_count(conform, 1, count);
	get_ffd_mod();
	ffd->Conform();
	MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
	return &ok;
}

Value*
getDimensions_cf(Value** arg_list, int count)
{
	check_arg_count(setDimensions, 1, count);
	get_ffd_mod();
	IPoint3 p = ffd->GetGridDim();
	return new Point3Value(Point3(p.x, p.y, p.z));	
}


Value*
setDimensions_cf(Value** arg_list, int count)
{
	check_arg_count(setDimensions, 2, count);
	get_ffd_mod();
	Point3 p = arg_list[1]->to_point3();
	ffd->SetGridDim(IPoint3((int)p.x, (int)p.y, (int)p.z));
	MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
	return &ok;
}

Value*
animateAll_cf(Value** arg_list, int count)
{
	check_arg_count(animateAll, 1, count);
	get_ffd_mod();
	ffd->AnimateAll();	
	MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
	return &ok;
}

Value*
resetLattice_cf(Value** arg_list, int count)
{
	check_arg_count(resetLattice, 1, count);
	get_ffd_mod();
	ffd->SetGridDim(ffd->GetGridDim());	
	mod->NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	MAXScript_interface->RedrawViews(MAXScript_interface->GetTime());
	return &ok;
}

static ActionDescription spActions[] = {
	ID_SUBOBJ_TOP,
    IDS_SWITCH_TOP,
    IDS_SWITCH_TOP,
    IDS_RB_FFDGEN,

	ID_SUBOBJ_CP,
    IDS_SWITCH_CP,
    IDS_SWITCH_CP,
    IDS_RB_FFDGEN,

    ID_SUBOBJ_LATTICE,
    IDS_SWITCH_LATTICE,
    IDS_SWITCH_LATTICE,
    IDS_RB_FFDGEN,

    ID_SUBOBJ_SETVOLUME,
    IDS_SWITCH_SETVOLUME,			
    IDS_SWITCH_SETVOLUME,
    IDS_RB_FFDGEN,

	};

ActionTable* GetActions()
{
    TSTR name = GetString(IDS_RB_FFDGEN);
    HACCEL hAccel = LoadAccelerators(hInstance,
                                     MAKEINTRESOURCE(IDR_FFD_SHORTCUTS));
    int numOps = NumElements(spActions);
    ActionTable* pTab;
    pTab = new ActionTable(kFFDActions, kFFDContext, name, hAccel, numOps,
                             spActions, hInstance);        
    GetCOREInterface()->GetActionManager()->RegisterActionContext(kFFDContext, name.data());
	return pTab;
}


int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
			{
            Nodes.Append(1, (INode **)&rmaker);                 
			}
     return 0;              
	}

