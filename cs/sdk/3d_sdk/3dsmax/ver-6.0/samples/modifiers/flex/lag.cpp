/**********************************************************************
 *<
	FILE: lag.cpp

	DESCRIPTION:  Lag modifier

	CREATED BY: Peter Watje
				Now fully supports param2 except the paint mode
	HISTORY: 8/13/98

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/


#include "lag.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"
#include "meshadj.h"

static GenSubObjType SOT_Points(15);
static GenSubObjType SOT_Edges(30);
static GenSubObjType SOT_Weights(31);


IParamMap       *LagMod::pmapParam = NULL;
IObjParam       *LagMod::ip        = NULL;
LagMod          *LagMod::editMod   = NULL;
MoveModBoxCMode *LagMod::moveMode  = NULL;
HWND			LagMod::hParams  = NULL;
HWND			LagMod::hPaint  = NULL;
HWND			LagMod::hAdvance  = NULL;
HWND			LagMod::hAdvanceSprings = NULL;
HWND			LagMod::hCreateSimple = NULL;

CreatePaintMode* LagMod::PaintMode   = NULL;

extern ClassDesc* GetLagModDesc() {return &lagDesc;}

//watje
INT_PTR CALLBACK VertexInfluenceDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.
// JBW: since the old path controller kept all parameters as instance data members, this setter callback
// is implemented to to reduce changes to existing code 
class LagPBAccessor : public PBAccessor
{ 
public:
	void TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count);
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		LagMod* p = (LagMod*)owner;
		switch (id)
		{
			case lag_solver:
			{
				IParamMap2* pmap = p->pblock2->GetMap();
				if (pmap)
					{
					SendMessage(GetDlgItem(pmap->GetHWnd(), IDC_QUALITY_COMBO), CB_SETCURSEL, v.i, (LPARAM)(0) );
					}
					
				break;

			}

			case lag_force_node:
				DebugPrint("setting forcee\n");
				break;
			case lag_springcolors:
			case lag_customspringdisplay:
				if (p->ip)
					{
					p->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
					}
				break;

		}
	}
};


static LagPBAccessor lag_accessor;


//--- Parameter map/block descriptors -------------------------------
//new parm blocks
//
//
// Parameters

// JBW: Here follows the new parameter block descriptors.  There are now 3, 
//      two new STATIC ones to hold the old class var parameters, one for the main
//		per-instance parameters.  Apart from all the extra 
//      metadata you see in the definitions, one important new idea is the
//      folding of ParamMap description info into the parameter descriptor and
//      providing a semi-automatic rollout desipaly mechanism.
//      

// Parameter Block definitions

// JBW: First come the position and version independent IDs for each
//      of the blocks and the parameters in each block.  These IDs are used
//	    in subsequent Get/SetValue() parameter access, etc. and for version-independent
//      load and save



// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

enum { lag_map_params,lag_map_simplesoft, lag_map_weights, lag_map_forces,lag_map_advanceparams, lag_map_advancesprings };     	// enum IDs for the 2 parammaps

// per instance geosphere block
static ParamBlockDesc2 lag_param_blk ( lag_params, _T("FlexParameters"),  0, &lagDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, PBLOCK_REF, 
	//rollout
	6,
	lag_map_params,  IDD_LAGPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
	lag_map_simplesoft, IDD_LAGSIMPLESOFT, IDS_LAG_SIMPLESOFT,0,0,NULL,
	lag_map_weights, IDD_LAGWEIGHTS, IDS_LAG_WEIGHTSPAINTS,0,0,NULL,
	lag_map_forces,  IDD_LAGFORCES, IDS_LAG_FORCES_COLLISIONS,0,0,NULL,
	lag_map_advanceparams,IDD_LAGADVANCEPARAMS, IDS_LAG_ADVANCE_PARAMETERS, 0, 0, NULL,
    lag_map_advancesprings,  IDD_LAGADVANCESPRINGS, IDS_LAG_ADVANCESPRINGSG, 0, 0, NULL,
//	IDD_LAGPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
	// params
	lag_flex,  _T("flex"), 			TYPE_FLOAT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_FALLOFF, 
		p_default, 		1.0,	
		p_range, 		0.0f, 1000.0f, 
		p_ui, 			lag_map_params,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_FALLOFF,IDC_LAG_FALLOFFSPIN, .1f, 
		end, 
	lag_strength, 	_T("strength"), 	TYPE_FLOAT, 	P_ANIMATABLE  | P_RESET_DEFAULT, 	IDS_LAG_STRENGTH, 
		p_default, 		3.0f, 
		p_range, 		0.0f, 100.0f, 
		p_ui, 			lag_map_params,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_SETTLE, IDC_LAG_SETTLE_SPIN, 0.01f,  
		end, 

	lag_sway, 	_T("sway"), 	TYPE_FLOAT, 	P_ANIMATABLE  | P_RESET_DEFAULT, 	IDS_LAG_SWAY, 
		p_default, 		7.0f, 
		p_range, 		0.0f, 100.0f, 
		p_ui, 			lag_map_params,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_WAVES, IDC_LAG_WAVES_SPIN, 0.01f,  
		end, 

	lag_referenceframe, 	_T("referenceFrame"), 	TYPE_INT, 	 P_RESET_DEFAULT, 	IDS_LAG_REFERENCEFRAME, 
		p_default, 		0, 
		p_range, 		-99999, 99999, 
		p_ui, 			lag_map_advanceparams,TYPE_SPINNER, EDITTYPE_INT, IDC_REFERENCEFRAME, IDC_REFERENCEFRAME_SPIN, SPIN_AUTOSCALE,  
		end, 

	lag_paint_strength,  _T("paintStrength"),		TYPE_FLOAT, 	 P_RESET_DEFAULT, 	IDS_LAG_STRENGTH, 
		p_default, 		0.1,	
		p_range, 		-1.0f, 1.0f, 
		p_ui, 			lag_map_weights,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PAINT_STRENGTH,IDC_PAINTSTRENGTHSPIN, .1f, 
		end, 

	lag_paint_radius,  _T("paintRadius"),		TYPE_FLOAT, 	 P_RESET_DEFAULT, 	IDS_LAG_STRENGTH, 
		p_default, 		36.0f,	
		p_range, 		0.001f, 99999.0f, 
		p_ui, 			lag_map_weights,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_RADIUS,IDC_RADIUSSPIN, 1.0f, 
		end, 

	lag_paint_feather,  _T("paintFeather"),	TYPE_FLOAT, 	 P_RESET_DEFAULT, 	IDS_LAG_STRENGTH, 
		p_default, 		0.70f,	
		p_range, 		0.001f, 1.0f, 
		p_ui, 			lag_map_weights,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FEATHER,IDC_FEATHERSPIN, SPIN_AUTOSCALE, 
		end, 

	lag_paint_backface, _T("paintBackface"),	TYPE_BOOL, 		 P_RESET_DEFAULT,	IDS_LAG_STRENGTH,
		p_default, 		TRUE, 
		p_ui, 			lag_map_params,TYPE_SINGLECHEKBOX, 	IDC_PROJECT_THROUGH_CHECK, 
		end, 
	lag_force_node,    _T("forceNode"),  TYPE_INODE_TAB,		0,	P_AUTO_UI|P_VARIABLE_SIZE,	IDS_LAG_FORCE_NODES,
		p_ui,			lag_map_forces,TYPE_NODELISTBOX, IDC_LIST1,IDC_GRAVITY_PICKNODE,0,IDC_REMOVE_GRAV,
		p_accessor,	&lag_accessor,
		end,
	lag_absolute, _T("absolute"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_ABSOLUTE,
		p_default, 		FALSE, 
		p_ui, 			lag_map_weights,TYPE_SINGLECHEKBOX, 	IDC_ABSOLUTE, 
		end, 

	lag_solver, 	_T("solver"), 	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_LAG_SOLVER, 
		p_default, 		0, 
		p_range, 		0, 2, 
		p_accessor,	&lag_accessor,
		end, 
	lag_samples, 	_T("samples"), 	TYPE_INT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_SAMPLES, 
		p_default, 		5, 
		p_range, 		1, 50, 
		p_ui, 			lag_map_params,TYPE_SPINNER, EDITTYPE_INT, IDC_SAMPLES, IDC_SAMPLES_SPIN, SPIN_AUTOSCALE,  
		end, 

	lag_chase, _T("chase"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_CHASE,
		p_default, 		TRUE, 
		p_ui, 			lag_map_params,TYPE_SINGLECHEKBOX, 	IDC_CHASE, 
		p_enable_ctrls,		2,lag_strength,lag_sway,
		end, 

	lag_center, _T("center"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_CENTER,
		p_default, 		TRUE, 
		p_ui, 			lag_map_params,TYPE_SINGLECHEKBOX, 	IDC_CENTER, 
		p_enable_ctrls,	3, lag_paint_feather,lag_paint_radius,lag_paint_strength,
		end, 

	lag_endframe, 	_T("endFrame"), 	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_LAG_ENDFRAME, 
		p_default, 		100, 
		p_range, 		-99999, 99999, 
		p_ui, 			lag_map_advanceparams,TYPE_SPINNER, EDITTYPE_INT, IDC_ENDFRAME, IDC_ENDFRAME_SPIN, SPIN_AUTOSCALE,  
		end, 

	lag_endframeon, _T("enableEndFrame"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_ENABLEENDFRAME,
		p_default, 		FALSE, 
		p_ui, 			lag_map_advanceparams,TYPE_SINGLECHEKBOX, 	IDC_ENDFRAMEON, 
		p_enable_ctrls,	1, lag_endframe,
		end, 

	lag_collider_node,    _T("colliderNode"),  TYPE_INODE_TAB,		0,	P_AUTO_UI|P_VARIABLE_SIZE,	IDS_LAG_COLLIDER_NODES,
		p_ui,			lag_map_forces,TYPE_NODELISTBOX, IDC_LIST2,IDC_COLLISION_PICKNODE,0,IDC_REMOVE_COLLISION,
		p_accessor,	&lag_accessor,
		end,

	lag_stretch_str, 	_T("stretchStrength"), 	TYPE_FLOAT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_STRETCHSTRENGTH, 
		p_default, 		.2f, 
		p_range, 		0.0f, 100.0f, 
		p_ui, 			lag_map_advancesprings,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_STRETCHSTR, IDC_LAG_STRETCHSTR_SPIN, 0.01f,  
		end, 

	lag_stretch_sway, 	_T("stretchSway"), 	TYPE_FLOAT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_STRETCHSWAY, 
		p_default, 		.2f, 
		p_range, 		0.0f, 100.0f, 
		p_ui, 			lag_map_advancesprings,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_STRETCHSWAY, IDC_LAG_STRETCHSWAY_SPIN, 0.01f,  
		end, 


	lag_torque_str, 	_T("torqueStrength"), 	TYPE_FLOAT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_TORQUESTRENGTH, 
		p_default, 		.2f, 
		p_range, 		0.0f, 100.0f, 
		p_ui, 			lag_map_advancesprings,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_TORQUESTR, IDC_LAG_TORQUESTR_SPIN, 0.01f,  
		end, 

	lag_torque_sway, 	_T("torqueSway"), 	TYPE_FLOAT, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_TORQUESWAY, 
		p_default, 		.2f, 
		p_range, 		0.0f, 100.0f, 
		p_ui, 			lag_map_advancesprings,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_TORQUESWAY, IDC_LAG_TORQUESWAY_SPIN, 0.01f,  
		end, 


	lag_extra_str, 	_T("extraStrength"), 	TYPE_FLOAT_TAB,10, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_EXTRASTRENGTH, 
		p_default, 		.2f, 
		p_range, 		0.0f, 100.0f, 
		end, 

	lag_extra_sway, 	_T("extraSway"), 	TYPE_FLOAT_TAB,10, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_EXTRASWAY, 
		p_default, 		.2f, 
		p_range, 		0.0f, 100.0f, 
		end, 

	lag_hold_radius, 	_T("holdRadius"), 	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_LAG_HOLDRADIUS, 
		p_default, 		50.0f, 
		p_range, 		0.0f, 10000.0f, 
		end, 
	lag_add_mode, 	_T("addMode"), 	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_LAG_ADDMODE, 
		p_default, 		0, 
		p_range, 		0, 4, 
		end, 
	lag_displaysprings, _T("displaySprings"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_DISPLAYSPRINGS,
		p_default, 		FALSE, 
		p_ui, 			lag_map_advancesprings,TYPE_SINGLECHEKBOX, 	IDC_SHOWSPRINGS, 
		end, 

	lag_holdlength, _T("holdLength"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_HOLDLENGTH,
		p_default, 		FALSE, 
		p_ui, 			lag_map_advancesprings,TYPE_SINGLECHEKBOX, 	IDC_HOLDLENGTH, 
		p_enable_ctrls,		1,lag_holdlengthpercent,

		end, 
	lag_holdlengthpercent,_T("holdLengthPercent"),	TYPE_FLOAT, P_RESET_DEFAULT,	IDS_LAG_HOLDLENGTHPERCENT,
		p_default, 		25.0, 
		p_ui, 			lag_map_advancesprings,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_HOLDLENGTHPER, IDC_LAG_HOLDLENGTHPER_SPIN, 0.01f,  
		end, 
	lag_lazyeval, _T("lazyEval"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_LAZYEVAL,
		p_default, 		FALSE, 
		end, 

	lag_stretch,_T("stretch"),	TYPE_FLOAT, P_ANIMATABLE | P_RESET_DEFAULT,	IDS_LAG_STRETCH,
		p_default, 		5.0, 
		p_range, 		0.0f, 50.f, 
		p_ui, 			lag_map_simplesoft,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_SSTRETCH, IDC_LAG_SSTRETCH_SPIN, .10f,  
		end, 


	lag_stiffness,_T("stiffness"),	TYPE_FLOAT, P_ANIMATABLE | P_RESET_DEFAULT,	IDS_LAG_STIFFNESS,
		p_default, 		0.1, 
		p_range, 		0.0f, 50.f, 
		p_ui, 			lag_map_simplesoft,TYPE_SPINNER, EDITTYPE_FLOAT, IDC_LAG_STIFFNESS, IDC_LAG_STIFFNESS_SPIN, .10f,  
		end, 

	lag_enable_advance_springs,_T("enableAdvanceSprings"),	TYPE_BOOL, P_RESET_DEFAULT,	IDS_LAG_ENABLESPRINGS,
		p_default, 		FALSE, 
		p_ui, 			lag_map_advancesprings,TYPE_SINGLECHEKBOX, 	IDC_ENABLE, 
		p_enable_ctrls,	4, lag_stretch_str, lag_stretch_sway, lag_torque_str, lag_torque_sway,
		end, 

	lag_springcolors, 	_T("springColors"), 	TYPE_POINT3_TAB,12, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_SPRINGCOLOR, 
		p_accessor,	&lag_accessor,
		end, 
	lag_customspringdisplay, 	_T("customSpringDisplay"), 	TYPE_BOOL_TAB,12, 	P_ANIMATABLE | P_RESET_DEFAULT, 	IDS_LAG_DISPLAYSPRINGS, 
		p_accessor,	&lag_accessor,
		end, 
	
	lag_affectall, _T("affectAll"),	TYPE_BOOL, 		P_RESET_DEFAULT,	IDS_LAG_AFFECTALL,
		p_default, 		FALSE, 
		p_ui, 			lag_map_advanceparams,TYPE_SINGLECHEKBOX, 	IDC_AFFECTALL, 
		end, 

	lag_createspringdepth, 	_T("createSpringDepth"), 	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_LAG_CREATESPRINGDEPTH, 
		p_default, 		2, 
		end, 

	lag_createspringmult, 	_T("createSpringMult"), 	TYPE_FLOAT, 	P_RESET_DEFAULT, 	IDS_LAG_CREATESPRINGMULT, 
		p_default, 		2.0f, 
		end, 


	end
	);


void LagPBAccessor::TabChanged(tab_changes changeCode, Tab<PB2Value>* tab, ReferenceMaker* owner, ParamID id, int tabIndex, int count)
		{

		if (id == lag_force_node)
			{
			if ( (changeCode == tab_append) || (changeCode == tab_insert) )
				{
				LagMod* p = (LagMod*)owner;
				BOOL updateUI=FALSE;
				for (int i =0; i < tab->Count(); i++)
					{
					PB2Value v = (*tab)[i];
					if (!p->validator.Validate(v))
						{
						tab->Delete(i,1);
						i--;
						updateUI=TRUE;
						}
					}
				if (updateUI)
					lag_param_blk.InvalidateUI(id);

				}

			}
		else if (id == lag_collider_node)
			{
			if ( (changeCode == tab_append) || (changeCode == tab_insert) )
				{
				LagMod* p = (LagMod*)owner;
				BOOL updateUI=FALSE;
				for (int i =0; i < tab->Count(); i++)
					{
					PB2Value v = (*tab)[i];
					if (!p->colValidator.Validate(v))
						{
						tab->Delete(i,1);
						i--;
						updateUI=TRUE;
						}
					}
				if (updateUI)
					lag_param_blk.InvalidateUI(id);


				}


			}

		}



BOOL LagModEnumProc::proc (ModContext *mc) {
	if (mc->localData == NULL) return TRUE;

	LagModData *lmd = (LagModData *) mc->localData;

	BOOL found = FALSE;
	for (int i = 0; i < lm->lmdData.Count(); i++)
		{
		if (lmd == lm->lmdData[i])
			{
			found = TRUE;
			i = lm->lmdData.Count();
			}
		}
	if (!found)
		lm->lmdData.Append(1,&lmd,1);

	return TRUE;
}





class LagWeightRestore : public RestoreObj {
	public:
		LagMod *mod;
		LagModData *bmd;
		Tab<float> uInflu;
		Tab<float> rInflu;

		LagWeightRestore(LagMod *c, LagModData *md) 
			{
			mod = c;
			bmd = md;
			uInflu.SetCount(md->SpringList.Count());
			for (int i = 0; i < uInflu.Count(); i++)
				{
				uInflu[i] = md->SpringList[i].InheritVel;
				}

			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rInflu.SetCount(bmd->SpringList.Count());
				for (int i = 0; i < uInflu.Count(); i++)
					{
					rInflu[i] = bmd->SpringList[i].InheritVel;
					}

				}

			for (int i = 0; i < uInflu.Count(); i++)
				{
				bmd->SpringList[i].InheritVel = uInflu[i];
				}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			for (int i = 0; i < uInflu.Count(); i++)
				{
				bmd->SpringList[i].InheritVel = rInflu[i];
				}

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_CHANGE_WEIGHT))); }
	};


void LagMod::HoldWeights(LagModData *lmd)
{
//if (theHold.Holding()) 
	theHold.Put (new LagWeightRestore (this, lmd));

}



class LagSpringRestore : public RestoreObj {
	public:
		LagMod *mod;
		LagModData *bmd;
		Tab<EdgeBondage> uSpringList;
		Tab<EdgeBondage> rSpringList;

		LagSpringRestore(LagMod *c, LagModData *md) 
			{
			mod = c;
			bmd = md;
			uSpringList = md->edgeSprings;
			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rSpringList = bmd->edgeSprings;
				}
			bmd->edgeSprings = uSpringList;

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			bmd->edgeSprings = rSpringList;
			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_LAG_CREATESPRING))); }
	};


/*-------------------------------------------------------------------*/
/*																	*/
/*				Paint Command Mode									*/
/*																	*/
/*-------------------------------------------------------------------*/

class CreatePaintMouseProc : public MouseCallBack {
        private:
                LagMod *mod;
                IObjParam *iObjParams;
				IPoint2 lastPoint;
                IPoint2 om;
                Tab<float> FalloffList;
				Tab<IPoint2> mouseHitList;
        
        protected:
                BOOL HitTest(LagModData *lmd, ViewExp *vpt, IPoint2 *p, int type, int flags, Object *obj );
                BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }           
				BOOL BuildFalloffList( ViewExp *vpt, Tab<IPoint2> plist, 
											 LagModData *lmd, Object *obj);


        public:
                CreatePaintMouseProc(LagMod* bmod, IObjParam *i) { mod=bmod; iObjParams=i; }
                int proc( 
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m );
        };



class CreatePaintMode : public CommandMode {
        private:
                ChangeFGObject fgProc;
                CreatePaintMouseProc eproc;
                LagMod* mod;

        public:
                CreatePaintMode(LagMod* bmod, IObjParam *i) :
                        fgProc(bmod), eproc(bmod,i) {mod=bmod;}

                int Class() { return MODIFY_COMMAND; }
                int ID() { return CID_CREATEPAINT; }
                MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
                ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
                BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
                void EnterMode();
                void ExitMode();
        };





float LineToPoint(Point3 p1, Ray r, float &u)

{
Point3 VectorA,VectorB,VectorC;
float Angle;
float dist = 0.0f;

VectorA = r.dir;
VectorB = p1-r.p;
Angle = (float)acos(DotProd(Normalize(VectorA),Normalize(VectorB)));

float hyp;
hyp = Length(VectorB);
dist = (float) sin(Angle) * hyp;

u = (float) cos(Angle) * hyp;

return dist;

}



BOOL CreatePaintMouseProc::HitTest(LagModData *lmd, 
                ViewExp *vpt, IPoint2 *p, int type, int flags, Object *obj )

        {

		if (lmd == NULL ) return FALSE;

        Ray                     ray;
        BOOL res = FALSE;

        vpt->MapScreenToWorldRay((float)p->x, (float)p->y, ray);

//now map it to our local space
		float feather, ifeather;

        feather = 1.0f -  mod->Feather;
	    ifeather = mod->Feather;

		if (1)
			{
			ray.p   = lmd->InverseTM * ray.p;
			ray.dir = VectorTransform(lmd->InverseTM, ray.dir);	

			float at;
			Point3 norm;

		    for (int i = 0; i<lmd->psel.Count(); i++)
			        lmd->psel[i] = 0;


			if (obj->IntersectRay(mod->ip->GetTime(),ray, at, norm)) 
				{
				lmd->hitPoint = ray.p + ray.dir * at;
				float rad = mod->Radius * mod->Radius;
				Box3 boundingBox;
				boundingBox.Init();
				boundingBox += lmd->hitPoint;					
				boundingBox.EnlargeBy(rad);

		        if ((lmd->isMesh)&& (obj->NumPoints() < VERTLIMIT)) FalloffList.ZeroCount();
				else if ((lmd->isPatch)&& (obj->NumPoints() < PATCHLIMIT)) FalloffList.ZeroCount();

				for (i = 0; i<lmd->SpringList.Count(); i++)
		            {
					if (boundingBox.Contains(lmd->SpringList[i].LocalPt))
						{
					    float dist = LengthSquared(lmd->SpringList[i].LocalPt-lmd->hitPoint);
						if (dist <= rad)
			                {
				            lmd->psel[i] = 1;
							if( ( (lmd->isMesh) && (obj->NumPoints() < VERTLIMIT) ) ||
							    ( (lmd->isPatch) && (obj->NumPoints() < PATCHLIMIT) )
							  )
								{
                                float f;
								dist = (float) sqrt(dist);
                                if (dist < mod->Radius*feather)
									f = 1.0f;
                                else 
                                    {
                                    f = 1.0f - (dist-(mod->Radius*feather))/(mod->Radius*ifeather);
                                    }
								f = f * mod->PaintStrength;
                                FalloffList.Append(1,&f,1);

								}



							}
						}
					}


				return TRUE;
				}

		
			}

        return res;


        }


float LagLineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u)
{
Point3 VectorA,VectorB,VectorC;
double Angle;
double dist = 0.0f;
VectorA = l2-l1;
VectorB = p1-l1;
Angle =  acos(DotProd(Normalize(VectorA),Normalize(VectorB)));
if (Angle > (3.14/2.0))
	{
	dist = Length(p1-l1);
	u = 0.0f;
	}
else
	{
	VectorA = l1-l2;
	VectorB = p1-l2;
	Angle = acos(DotProd(Normalize(VectorA),Normalize(VectorB)));
	if (Angle > (3.14/2.0))
		{
		dist = Length(p1-l2);
		u = 1.0f;
		}
		else
		{
		double hyp;
		hyp = Length(VectorB);
		dist =  sin(Angle) * hyp;
		double du =  (cos(Angle) * hyp);
		double a = Length(VectorA);
		if ( a== 0.0f)
			return 0.0f;
		else u = (float)((a-du) / a);

		}

	}

return (float) dist;

}


BOOL CreatePaintMouseProc::BuildFalloffList( ViewExp *vpt, Tab<IPoint2> plist, 
											 LagModData *lmd, Object *obj)

{
Box3 boundingBox;

if (lmd == NULL ) return FALSE;

Tab<Point3> hitList;

boundingBox.Init();

for (int pct = 0; pct < plist.Count(); pct++)
		{
        Ray ray;

		IPoint2 p = plist[pct];
        vpt->MapScreenToWorldRay((float)p.x, (float)p.y, ray);
		ray.p   = lmd->InverseTM * ray.p;
		ray.dir = VectorTransform(lmd->InverseTM, ray.dir);	

//now map it to our local space
		if (1)
			{

			float at;
			Point3 norm;

			if (obj->IntersectRay(mod->ip->GetTime(),ray, at, norm)) 
				{
				Point3 hitPoint;
				hitPoint = ray.p + ray.dir * at;
				hitList.Append(1,&hitPoint,1);
				boundingBox += hitPoint;					
				}
			}

		}
boundingBox.EnlargeBy(mod->Radius);
float feather, ifeather;
feather = 1.0f -  mod->Feather;
ifeather = mod->Feather;


if (hitList.Count() ==1 )
	{	
	for (int i = 0; i< lmd->SpringList.Count(); i++)
		{
		if (boundingBox.Contains(lmd->SpringList[i].LocalPt))
			{
			float dist = Length(lmd->SpringList[i].LocalPt-hitList[0]);
			if (dist < mod->Radius)
				{
				float f;
	            if (dist < mod->Radius*feather)
					f = 1.0f;
                else 
                   {
		           f = 1.0f - (dist-(mod->Radius*feather))/(mod->Radius*ifeather);
                   }
				f = f * mod->PaintStrength;
				if (f > FalloffList[i]) 
					FalloffList[i] = f;
				}
			}
		}

	}
else
	{
	for (pct = 1; pct < hitList.Count(); pct++)
		{
		for (int i = 0; i< lmd->SpringList.Count(); i++)
			{
			if (boundingBox.Contains(lmd->SpringList[i].LocalPt))
				{
				Point3 l1,l2;
				l1 = hitList[pct-1];
				l2 = hitList[pct];
				float u;
				float dist = LagLineToPoint(lmd->SpringList[i].LocalPt,l1, l2,u);

				if (dist < mod->Radius)
					{
					float f;
			        if (dist < (mod->Radius*feather))
			           f = 1.0f;
				    else 
					   {
						f = 1.0f - (dist-(mod->Radius*feather))/(mod->Radius*ifeather);
						}	
					f = f * mod->PaintStrength;
					if (f > FalloffList[i]) 
						FalloffList[i] = f;

					}
				}
			}
		}

	}
return 1;
}

static void LagXORDottedLine( HWND hwnd, IPoint2 p0, IPoint2 p1 )
	{
	HDC hdc;
	hdc = GetDC( hwnd );
	SetROP2( hdc, R2_XORPEN );
	SetBkMode( hdc, TRANSPARENT );
	SelectObject( hdc, CreatePen( PS_DOT, 0, ComputeViewportXORDrawColor() ) );
	MoveToEx( hdc, p0.x, p0.y, NULL );
	LineTo( hdc, p1.x, p1.y );		
	DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
	ReleaseDC( hwnd, hdc );
	}

int CreatePaintMouseProc::proc(
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m )
        {
        ViewExp *vpt = iObjParams->GetViewport(hwnd);   
        int res = TRUE;
        float st = 1.0f;
        int flip = 0;
		int i;

		ModContextList mcList;		
		INodeTab nodes;

		if (!mod->ip) return 0;

		mod->ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for ( int k = 0; k < objects; k++ ) 
			{

			LagModData *lmd = (LagModData*)mcList[k]->localData;

			ObjectState os = nodes[k]->EvalWorldState(mod->ip->GetTime());
			Object *obj = os.obj;

			if (lmd)
				{

		        switch ( msg ) {
		            case MOUSE_PROPCLICK:
			                iObjParams->SetStdCommandMode(CID_OBJMOVE);
							if (mod->painting)
								theHold.Cancel();
							mod->painting = FALSE;
				            break;

					case MOUSE_POINT:
						if (point == 0)
							{
							mouseHitList.ZeroCount();
							mod->painting = TRUE;

							theHold.Begin();
							mod->HoldWeights(lmd);

							}
						else if (mouseHitList.Count() != 0)
							{
//compute hit list
							mod->painting = FALSE;

		                    st = 1.0f;
			                flip = 0;
				            if (flags & MOUSE_ALT) 
					            {
						        st = -1.0f;
							    flip = 1;
								}
		                    mod->pblock2->GetValue(lag_paint_strength,0,mod->PaintStrength,FOREVER);
			                mod->pblock2->GetValue(lag_paint_radius,0,mod->Radius,FOREVER);
				            mod->pblock2->GetValue(lag_paint_feather,0,mod->Feather,FOREVER);
							FalloffList.SetCount(lmd->SpringList.Count());
						
							for (i = 0; i < lmd->SpringList.Count(); i++)
								FalloffList[i] = -10.0f;
							BuildFalloffList( vpt, mouseHitList,lmd,obj);
						    for (i=0;i<lmd->SpringList.Count();i++)
								{
								if (FalloffList[i] > 0.0f )
									{
									float currentv = FalloffList[i];
									if (st == 1.0f)
										lmd->SpringList[i].InheritVel -= currentv;
									else lmd->SpringList[i].InheritVel += currentv;
	
									lmd->SpringList[i].modified = TRUE;
									if (lmd->SpringList[i].InheritVel <0.0f) lmd->SpringList[i].InheritVel = 0.0f;
									if (lmd->SpringList[i].InheritVel >1.0f) lmd->SpringList[i].InheritVel = 1.0f;
									}
								}


							for (i = 1; i < mouseHitList.Count(); i++)
								LagXORDottedLine(hwnd, mouseHitList[i-1], mouseHitList[i]);	// Draw it!


			                mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
				            mod->ip->RedrawViews(mod->ip->GetTime());
							}
						
						theHold.Accept(GetString(IDS_PW_CHANGE_WEIGHT));

						break;



					case MOUSE_MOVE:
						if ( ((lmd->isMesh) && (obj->NumPoints() < VERTLIMIT)) ||
							  ((lmd->isPatch) && (obj->NumPoints() < PATCHLIMIT)) 
							  )
							{
	                        st = 1.0f;
		                    flip = 0;
			                if (flags & MOUSE_ALT) 
				                    {
					                st = -1.0f;
						            flip = 1;
							        }
	                        mod->pblock2->GetValue(lag_paint_strength,0,mod->PaintStrength,FOREVER);
		                    mod->pblock2->GetValue(lag_paint_radius,0,mod->Radius,FOREVER);
			                mod->pblock2->GetValue(lag_paint_feather,0,mod->Feather,FOREVER);
//change this to a radius type hit
				            if(HitTest(lmd, vpt,&m,HITTYPE_CIRCLE,0, obj) ) {

//transfrom mouse point to world
								int ct = 0;
							    for (i=0;i<lmd->SpringList.Count();i++)
									{
									if (lmd->psel[i] == 1 )
										{
										float currentv = FalloffList[ct++];
										if (st == 1.0f)
											lmd->SpringList[i].InheritVel -= currentv;
										else lmd->SpringList[i].InheritVel += currentv;

										lmd->SpringList[i].modified = TRUE;
										if (lmd->SpringList[i].InheritVel <0.0f) lmd->SpringList[i].InheritVel = 0.0f;
										if (lmd->SpringList[i].InheritVel >1.0f) lmd->SpringList[i].InheritVel = 1.0f;
										}
									}
								mod->painting = FALSE;

	                            mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
		                        mod->ip->RedrawViews(mod->ip->GetTime());

			                    }
							}
						else
							{
							if (mouseHitList.Count() > 1)
								{
								if (mouseHitList[mouseHitList.Count()-1] != m)
									{
									if (mouseHitList.Count() > 1)
										{
										LagXORDottedLine(hwnd, lastPoint, m);	// Draw it!
										}

		
									lastPoint = m;

							        if ( HitTest(lmd,vpt,&m,HITTYPE_CIRCLE,HIT_ABORTONHIT,obj) ) 
										{

				                        mod->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
										lmd->isHit = TRUE;
							            mod->ip->RedrawViews(mod->ip->GetTime());
	
										}
									else
										{
										if (lmd->isHit)
											{
											mod->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
											lmd->isHit = FALSE;
								            mod->ip->RedrawViews(mod->ip->GetTime());
											}
										else lmd->isHit = FALSE;

										}	

									mouseHitList.Append(1,&m,1);
									}

								}
							else {
								mouseHitList.Append(1,&m,1);
								lastPoint = m;
								}

							}
					       break;
                
	                case MOUSE_FREEMOVE:
                        if ( HitTest(lmd,vpt,&m,HITTYPE_CIRCLE,HIT_ABORTONHIT,obj) ) {
                                SetCursor(LoadCursor(NULL,IDC_CROSS ));
							lmd->isHit = TRUE;
							mod->painting = FALSE;

                             mod->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
                             mod->ip->RedrawViews(mod->ip->GetTime());

                             }
                        else {
                              SetCursor(LoadCursor(NULL,IDC_ARROW));

							  if (lmd->isHit)
								{
								  lmd->isHit = FALSE;
		                         mod->NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	                             mod->ip->RedrawViews(mod->ip->GetTime());
								}
	
							  else lmd->isHit = FALSE;

								mod->painting = FALSE;
                              }
                        break;
					}
                        
                }
			}

        if ( vpt ) iObjParams->ReleaseViewport(vpt);

        return res;
        }



/*-------------------------------------------------------------------*/

void CreatePaintMode::EnterMode()
        {
        mod->iPaintButton->SetCheck(TRUE);
		mod->painting = FALSE;
		mod->inPaint = TRUE;

		BOOL en = ((Modifier*) mod)->IsEnabled();
		BOOL env = mod->IsEnabledInViews();

		if ((!en) || (!env))
			{	
			ModContextList mcList;		
			INodeTab nodes;
			
			if (mod->ip)
				{


				mod->ip->GetModContexts(mcList,nodes);
				int objects = mcList.Count();
/*				BOOL en = TRUE;
				if (!mod->IsEnabled())
					en = FALSE;
*/

				for ( int k = 0; k < objects; k++ ) 
					{
					if (en)
						mod->EnableModInViews();	
					else mod->EnableMod();	
					ObjectState os = nodes[k]->EvalWorldState(mod->ip->GetTime());
					if (en)
						mod->DisableModInViews();	
					else mod->DisableMod();	
					}	
				}
			}

        }

void CreatePaintMode::ExitMode()
        {
        mod->iPaintButton->SetCheck(FALSE);
		mod->painting = FALSE;
		mod->inPaint = FALSE;

//nuke mesh
        }

//dialog stuff to get the Set Ref button
class MapDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		LagMod *mod;		
		MapDlgProc(LagMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};

class PaintDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		LagMod *mod;		
		PaintDlgProc(LagMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};

//dialog stuff to get the Set Ref button
class AdvanceDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		LagMod *mod;		
		AdvanceDlgProc(LagMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};

//dialog stuff to get the Set Ref button
class AdvanceSpringDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		LagMod *mod;		
		AdvanceSpringDlgProc(LagMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};

//dialog stuff to get the Set Ref button
class SimpleSoftDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		LagMod *mod;		
		SimpleSoftDlgProc(LagMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};

int LagMod::ApplyDelta() 

{
ModContextList mcList;		
INodeTab nodes;

if (!ip) return 0;

ip->GetModContexts(mcList,nodes);
int objects = mcList.Count();

theHold.Begin();
for ( int k = 0; k < objects; k++ ) 
	{

	LagModData *lmd = (LagModData*)mcList[k]->localData;
	if (lmd)
		{
		HoldWeights(lmd);

		for (int i = 0; i < lmd->wsel.Count(); i++)
			{
			if (lmd->wsel[i] == 1)
				{
				if (absoluteWeight) 
					lmd->SpringList[i].InheritVel = 1.0f- weightDelta;
				else lmd->SpringList[i].InheritVel -= weightDelta ;
				if (lmd->SpringList[i].InheritVel < 0.0f) lmd->SpringList[i].InheritVel = 0.0f;
				if (lmd->SpringList[i].InheritVel > 1.0f) lmd->SpringList[i].InheritVel = 1.0f;
				lmd->SpringList[i].modified = TRUE;
				}

			}
		}
	}
theHold.Accept(GetString(IDS_PW_CHANGE_WEIGHT));

return 1;
}

BOOL MapDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			mod->hParams = hWnd;


			TSTR blank("<blank>");
			int iret = SendMessage(GetDlgItem(hWnd,IDC_QUALITY_COMBO),
				LB_ADDSTRING,0,(LPARAM)(TCHAR*)blank.data());

			iret = SendMessage(GetDlgItem(hWnd,IDC_QUALITY_COMBO), CB_ADDSTRING, 0, (LPARAM)(GetString(IDS_LAG_EULER)) );
			iret = SendMessage(GetDlgItem(hWnd,IDC_QUALITY_COMBO), CB_ADDSTRING, 0, (LPARAM)(GetString(IDS_LAG_MID)) );
			iret = SendMessage(GetDlgItem(hWnd,IDC_QUALITY_COMBO), CB_ADDSTRING, 0, (LPARAM)(GetString(IDS_LAG_R4)) );
			SendMessage(GetDlgItem(hWnd,IDC_QUALITY_COMBO), CB_SETCURSEL, mod->solver, (LPARAM)(0) );

			
			break;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
					
				case IDC_QUALITY_COMBO:
					{
					if (HIWORD(wParam)==CBN_SELCHANGE) 
						{
						int fsel;
						fsel = SendMessage(
							GetDlgItem(hWnd,IDC_QUALITY_COMBO),
							CB_GETCURSEL,0,0);	
						mod->pblock2->SetValue(lag_solver,0,fsel);
//						SetProperty(ReferenceTarget* targ, TCHAR* prop_name, BYTE type, ...)
//						macroRecorder->FunctionCall(_T("$.modifiers[#flex].quality"), 0, 0);

						}
					break;
					}


				}
			break;


		}
	return FALSE;
	}

BOOL PaintDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			mod->hPaint = hWnd;

			mod->iPaintButton = GetICustButton(GetDlgItem(hWnd,IDC_PAINT));
			mod->iPaintButton->SetType(CBT_CHECK);
			mod->iPaintButton->SetHighlightColor(GREEN_WASH);
			mod->iPaintButton->Enable(FALSE);

			mod->iVWeight = GetISpinner(GetDlgItem(hWnd,IDC_VWEIGHT_SPIN));
			mod->iVWeight->LinkToEdit(GetDlgItem(hWnd,IDC_VWEIGHT),EDITTYPE_FLOAT);
			mod->iVWeight->SetLimits(-100.0f, 100.0f, FALSE);

			
			break;
			}

		case CC_SPINNER_BUTTONDOWN:
			if (!mod->absoluteWeight) 
				{
				mod->iVWeight->SetValue(0.0f,TRUE);
				mod->weightDelta = 0.0f;
				}
			else mod->weightDelta = mod->iVWeight->GetFVal() * 0.01f;
			mod->spinnerDown = TRUE;
			break;
		case CC_SPINNER_CHANGE:
			mod->weightDelta = mod->iVWeight->GetFVal() * 0.01f;
			mod->spinnerDown = TRUE;
            mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			mod->spinnerDown = FALSE;
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				{
				//apply

				mod->ApplyDelta();
				}
			else
				{
				mod->weightDelta = 0.0f;
				}
			if (!mod->absoluteWeight) 
				mod->iVWeight->SetValue(0.0f,TRUE);
            mod->ip->RedrawViews(mod->ip->GetTime());
            mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
			mod->spinnerDown = FALSE;

			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{

				case IDC_ABSOLUTE:
					{
					Interval iv;
					mod->pblock2->GetValue(lag_absolute,0,mod->absoluteWeight,iv);

					if (!mod->absoluteWeight) 
						{
						mod->iVWeight->SetValue(0.0f,TRUE);
						mod->weightDelta = 0.0f;
						}
					break;

					}
				case IDC_PAINT:
					{
					mod->StartPaintMode();
					macroRecorder->FunctionCall(_T("$.modifiers[#flex].flexOps.paint"), 0, 0);

					break;
					}

				}
			break;


		}
	return FALSE;
	}

BOOL AdvanceDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			mod->hAdvance = hWnd;			
			break;
			}


		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_SETREFERENCE_BUTTON:
					{
//set time to ref time
					Interface *ip = GetCOREInterface();
					TimeValue CurrentTime;
					Interval CurrentSeg,ReferenceSeg;
				
					CurrentTime = ip->GetTime();
					CurrentSeg = ip->GetAnimRange();
					ReferenceSeg = CurrentSeg;
//set new range to include reference frame
					ReferenceSeg.Set(mod->ReferenceFrame,mod->ReferenceFrame+GetTicksPerFrame());
					ip->SetAnimRange(ReferenceSeg);
					ip->SetTime(mod->ReferenceFrame,FALSE);

//set back old rangle
					ip->SetAnimRange(CurrentSeg);
//set back to original time
					ip->SetTime(CurrentTime,FALSE);
					macroRecorder->FunctionCall(_T("$.modifiers[#flex].flexOps.setReference"), 0, 0);

					break;
					}
				case IDC_RESET_BUTTON:
					{
					mod->ResetSprings();
//set time to ref time
					Interface *ip = GetCOREInterface();
					TimeValue CurrentTime;
					Interval CurrentSeg,ReferenceSeg;
				
					CurrentTime = ip->GetTime();
					CurrentSeg = ip->GetAnimRange();
					ReferenceSeg = CurrentSeg;
//set new range to include reference frame
					ReferenceSeg.Set(mod->ReferenceFrame,mod->ReferenceFrame+GetTicksPerFrame());
					ip->SetAnimRange(ReferenceSeg);
					ip->SetTime(mod->ReferenceFrame,FALSE);

//set back old rangle
					ip->SetAnimRange(CurrentSeg);
//set back to original time
					ip->SetTime(CurrentTime,FALSE);
                    mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);

                    ip->RedrawViews(ip->GetTime());

					macroRecorder->FunctionCall(_T("$.modifiers[#flex].flexOps.reset"), 0, 0);

					break;
					}

				}
			break;


		}
	return FALSE;
	}


BOOL AdvanceSpringDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			mod->hAdvanceSprings = hWnd;

			
			break;
			}



		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_ADD_SPRING:
					{
					mod->fnAddSpringButton();
					macroRecorder->FunctionCall(_T("$.modifiers[#flex].AddSpringButton"), 0, 0);

					theHold.Begin();
					for (int i =0; i < mod->lmdData.Count(); i++)
						theHold.Put (new LagSpringRestore (mod, mod->lmdData[i]));
					theHold.Accept(GetString(IDS_LAG_CREATESPRING));

/*					LagModEnumProc lmdproc(mod);
					mod->EnumModContexts(&lmdproc);

					for (int i =0; i < mod->lmdData.Count(); i++)
						mod->lmdData[i]->addSprings = TRUE;
                    mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
*/
					break;
					}
				case IDC_REMOVE_SPRING:
					{
					mod->fnRemoveSpringButton();
					macroRecorder->FunctionCall(_T("$.modifiers[#flex].RemoveSpringButton"), 0, 0);

					theHold.Begin();
					for (int i =0; i < mod->lmdData.Count(); i++)
						theHold.Put (new LagSpringRestore (mod, mod->lmdData[i]));
					theHold.Accept(GetString(IDS_LAG_REMOVESPRING));

/*
					LagModEnumProc lmdproc(mod);
					mod->EnumModContexts(&lmdproc);

					for (int i =0; i < mod->lmdData.Count(); i++)
						mod->lmdData[i]->removeSprings = TRUE;
                    mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					*/


					break;
					}
				case IDC_OPTIONS_SPRING:
					{
					mod->fnOptionButton();
					macroRecorder->FunctionCall(_T("$.modifiers[#flex].OptionsButton"), 0, 0);

/*
					LagModEnumProc lmdproc(mod);
					mod->EnumModContexts(&lmdproc);

					for (int i =0; i < mod->lmdData.Count(); i++)
						mod->lmdData[i]->computeEdges = TRUE;
					
					int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_SPRINGOPTION_DIALOG),
						hWnd,OptionDlgProc,(LPARAM)mod);	
*/
					break;
					}
					
				}
			break;


		}
	return FALSE;
	}

BOOL SimpleSoftDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG: {
			mod->hCreateSimple = hWnd;
			
			break;
			}

		case CC_SPINNER_BUTTONDOWN:
			mod->spinnerDown = TRUE;
			break;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			mod->spinnerDown = FALSE;
			break;



		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_CREATE_SOFT:
					{

					mod->fnSimpleSoftButton();
					macroRecorder->FunctionCall(_T("$.modifiers[#flex].CreateSimpleSoftButton"), 0, 0);

					theHold.Begin();
					for (int i =0; i < mod->lmdData.Count(); i++)
						theHold.Put (new LagSpringRestore (mod, mod->lmdData[i]));
					theHold.Accept(GetString(IDS_LAG_CREATESPRING));

/*
					LagModEnumProc lmdproc(mod);
					mod->EnumModContexts(&lmdproc);

					for (int i =0; i < mod->lmdData.Count(); i++)
						mod->lmdData[i]->simpleSoft = TRUE;
                    mod->NotifyDependents(FOREVER, GEOM_CHANNEL, REFMSG_CHANGE);
					*/

					break;
					}
				}
			break;


		}
	return FALSE;
	}


	class MyEnumProc : public DependentEnumProc 
	{
      public :
      virtual int proc(ReferenceMaker *rmaker); 
      INodeTab Nodes;              
	  BOOL nukeME;
	  BOOL xRefFound;
	  int count;
	};

int MyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
	if (rmaker->ClassID()==Class_ID(0x5c2417bd, 0x44050964))    
		{
		IParamBlock *pb;
		pb = (IParamBlock *) rmaker->GetReference(4);
		Interval ivalid;
		int anioff;
		pb->GetValue(32,0,anioff,ivalid);
		if (anioff != 0)
			nukeME = TRUE;
		}
//7-1-99
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
		Nodes.Append(1, (INode **)&rmaker);  
		count++;
		}
	if (rmaker->ClassID()==Class_ID(XREFOBJ_CLASS_ID,0))    
		{
		xRefFound = TRUE;
		}


	return 0;
	}	



// Declare the callback function
static void RenderFrameStart(void *param, NotifyInfo *info) {
	// Handle the units changing...
	LagMod *mod =  (LagMod*) param;
	TimeValue *t = (TimeValue*) info->callParam;

	Modifier *m = (Modifier *) mod;
	if (m->IsEnabled() && m->IsEnabledInRender())
		mod->CheckCache(*t);

}


//--- Lag mod methods -------------------------------

LagMod::LagMod() 
	{

	suggested = -1.0f;

	flexVersion = 400;
	spinnerDown = FALSE;

	occuppiedList = NULL;
	disableUI = FALSE;
	edgeLength = -1.0f;
	maxEdgeLength = -1.0f;
	minEdgeLength = -1.0f;
	subSelection = FALSE;
	solver = 0;
	GetLagModDesc()->MakeAutoParamBlocks(this);
	p1 = NULL;
	ff.ZeroCount();
	MakeRefByID(FOREVER,POINT1_REF,NewDefaultPoint3Controller()); 
	sel[0] = 1;

	Point3 p(0.0f,0.0f,0.0f);
	p1->SetValue(
			0,p,
			TRUE,CTRL_ABSOLUTE);

	iPaintButton = NULL;
	iVWeight = NULL;
	ReferenceFrame = 0;
	absoluteWeight = 0;
	weightDelta = 0.0f;
	container = NULL;

	notify = new LagNodeNotify(this);

	validator.mod = this;
	colValidator.mod = this;
	painting = FALSE;
	inPaint = FALSE;
	updating = FALSE;
	nukeCache = FALSE;
	updateWeights = FALSE;
	aboutToRender = FALSE;
// Register the callback
	RegisterNotification(RenderFrameStart,this,	NOTIFY_RENDER_PREEVAL);

	for (int i =0; i < 12; i ++)
		{
		Point3 color;
		
		color.x = 1.0f - (float) (i)/11.f;
		color.y = .0f;
		color.z = .0f;
		if (i == 0) color = Point3(0.0f,0.0f,1.0f);

		pblock2->SetValue(lag_springcolors,0,color,i);
		BOOL on = TRUE;
		pblock2->SetValue(lag_customspringdisplay,0,on,i);
		}
	}


LagMod::~LagMod() 
{
	DeleteAllRefsFromMe();
	for (int i = 0; i< ff.Count(); i++)
	{
		if (ff[i]) 
			ff[i]->DeleteThis();
	}

	for (i = 0; i< colliderObjects.Count(); i++)
	{
		if (colliderObjects[i]) 
			colliderObjects[i]->DeleteThis();
	}

// mjm - begin - 5.10.99
	if (container && notify)
		{
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(LAGCONTAINERMASTER_TVNODE_CLASS_ID);
		if (tvroot) 
			{
			int ct = tvroot->NumItems();
			for (int i = 0; i <ct; i++)
				{
				ITrackViewNode *n = tvroot->GetNode(i);
				if (container == n)
					container->UnRegisterTVNodeNotify(notify);
				}
			}
		else
			{
//			ITrackViewNode *tvroot = tvr->GetNode(LAGCONTAINER_TVNODE_CLASS_ID);
			int ct = tvr->NumItems();
			for (int i = 0; i <ct; i++)
				{
				ITrackViewNode *n = tvr->GetNode(i);
				if (container == n)
					container->UnRegisterTVNodeNotify(notify);
				}


			}	
		}
//		container->UnRegisterTVNodeNotify(notify);
	if (notify)
		delete notify;
// mjm - end


	if (iPaintButton)
		ReleaseICustButton(iPaintButton);
	if (iVWeight)
		ReleaseISpinner(iVWeight);
// When done, unregister the callback
	UnRegisterNotification(RenderFrameStart,this,	NOTIFY_RENDER_PREEVAL);

}



void LagMod::BeginEditParams(
		IObjParam  *ip, ULONG flags,Animatable *prev)
	{
	this->ip = ip;
	editMod  = this;

	// Add our sub object type
	// 	TSTR type1(GetString(IDS_RB_LAGPOINTS));
	// TSTR type2(GetString(IDS_RB_LAG_EDGE));
	// TSTR type3(GetString(IDS_LAG_WEIGHTS));
	// const TCHAR *ptype[] = {type1,type2,type3};
	// This call is obsolete. Please see BaseObject::NumSubObjTypes() and BaseObject::GetSubObjType()
	// ip->RegisterSubObjectTypes(ptype, 3);

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	PaintMode = new CreatePaintMode(this,ip);


	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	lagDesc.BeginEditParams(ip, this, flags, prev);
	lag_param_blk.SetUserDlgProc(lag_map_params,new MapDlgProc(this));
	lag_param_blk.SetUserDlgProc(lag_map_weights,new PaintDlgProc(this));
	lag_param_blk.SetUserDlgProc(lag_map_advanceparams,new AdvanceDlgProc(this));
	lag_param_blk.SetUserDlgProc( lag_map_advancesprings,new AdvanceSpringDlgProc(this));

	lag_param_blk.SetUserDlgProc( lag_map_simplesoft,new SimpleSoftDlgProc(this));

	lag_param_blk.ParamOption(lag_force_node,p_validator,&validator);
	lag_param_blk.ParamOption(lag_collider_node,p_validator,&colValidator);

	float f;
	Interval iv;
	pblock2->GetValue(lag_paint_strength,ip->GetTime(),f,iv);
	pblock2->SetValue(lag_paint_strength,ip->GetTime(),f);

	suggested = -1.0f;

	}

void LagMod::EndEditParams(
		IObjParam *ip,ULONG flags,Animatable *next)
	{
	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);

	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);	
	if (moveMode) delete moveMode;
	moveMode = NULL;	

	ip->DeleteMode(PaintMode);	
	if (PaintMode) delete PaintMode;
	PaintMode = NULL;
	
	ip->ClearPickMode();

	lagDesc.EndEditParams(ip, this, flags, next);
	}

RefTargetHandle LagMod::Clone(RemapDir& remap)
	{
	LagMod *mod = new LagMod();
	mod->ReplaceReference(PBLOCK_REF,pblock2->Clone(remap));
	mod->ReplaceReference(POINT1_REF,p1->Clone(remap));

	BaseClone(this, mod, remap);
	return mod;
	}

BOOL LagMod::AssignController(Animatable *control, int subAnim)
{
	if (control->SuperClassID()!=CTRL_POINT3_CLASS_ID || subAnim != POINT1_REF) return FALSE;
	ReplaceReference(POINT1_REF,(ReferenceTarget*)control);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
}

int LagMod::CheckCache(TimeValue t)
{
//back step now if needed

	int tps = GetTicksPerFrame();
    MyEnumProc dep;              
	dep.nukeME = FALSE;
	dep.xRefFound = FALSE;
	dep.count = 0;
	EnumDependents(&dep);

	int instanced = dep.count;


	LagModEnumProc lmdproc(this);
	EnumModContexts(&lmdproc);

	for (int li = 0; li < lmdData.Count(); li++)
		{
		LagModData *lmd = lmdData[li];

		BOOL rebuildSystem = FALSE;
		
		if ((TestAFlag(A_RENDER)) && (aboutToRender))
			{
			lmd->lastFrame = 999999999;
			aboutToRender = FALSE;

			}

		if (t>=ReferenceFrame)
			{

			int fract ;
			int frames ;
			TimeValue StartFrame;
			if ( (t<lmd->lastFrame) )
//				||
//				 ( (t==lmd->lastFrame) && (rebuildSystem) )
//				)
				{

				fract = (t) % tps;
				frames =  t / tps;
				StartFrame = ReferenceFrame;
				int nv = lmd->SpringList.Count();
				for (int i=0;i<nv;i++)
					{
					Point3 v(0.0f,0.0f,0.0f);
					lmd->SpringList[i].pos = lmd->SpringList[i].init_pos;
					lmd->SpringList[i].vel = v;
					}
				}
			else
				{
				fract = t %tps;
				frames =  t / tps;
				StartFrame = lmd->lastFrame+tps;
				int nv = lmd->SpringList.Count();
				for (int i=0;i<nv;i++)
					{
					lmd->SpringList[i].pos = lmd->WholeFrameCache[i].pos;
					lmd->SpringList[i].vel = lmd->WholeFrameCache[i].vel;
					}

				}
//	DebugPrint("time t %d start frame %d\n",t,StartFrame);
			for (int i = StartFrame; i < (frames*tps); i+=tps)
				{
//				Matrix3 tm = lmd->SelfNode->GetObjectTM(i);



				Point3 pp,p,force;

				ObjectState tos;
				int nv = lmd->SpringList.Count();
				int tnv = nv;
				if (i != t)
					{
					if (TestAFlag(A_RENDER)) 
						{
						if (instanced==1)
							EnableModsAbove(lmd,FALSE);
						tos = lmd->SelfNode->EvalWorldState(i);
						if (instanced==1)
							EnableModsAbove(lmd,TRUE);
						}
					}
				}
			}
		}
	return 1;

}




int LagMod::RenderBegin(TimeValue t, ULONG flags)
	{
	aboutToRender = TRUE;

	SetAFlag(A_RENDER);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	nukeCache = TRUE;


/*
//back step now if needed

	int tps = GetTicksPerFrame();
    MyEnumProc dep;              
	dep.nukeME = FALSE;
	dep.xRefFound = FALSE;
	dep.count = 0;
	EnumDependents(&dep);

	int instanced = dep.count;


	LagModEnumProc lmdproc(this);
	EnumModContexts(&lmdproc);

	for (int li = 0; li < lmdData.Count(); li++)
		{
		LagModData *lmd = lmdData[li];

		BOOL rebuildSystem = FALSE;
		
//		if ((TestAFlag(A_RENDER)) && (aboutToRender))
//			{
			lmd->lastFrame = 999999999;
//			}

		if (t>=ReferenceFrame)
			{

			int fract ;
			int frames ;
			TimeValue StartFrame;
			if ( (t<lmd->lastFrame) )
//				||
//				 ( (t==lmd->lastFrame) && (rebuildSystem) )
//				)
				{

				fract = (t) % tps;
				frames =  t / tps;
				StartFrame = ReferenceFrame;
				int nv = lmd->SpringList.Count();
				for (int i=0;i<nv;i++)
					{
					Point3 v(0.0f,0.0f,0.0f);
					lmd->SpringList[i].pos = lmd->SpringList[i].init_pos;
					lmd->SpringList[i].vel = v;
					}
				}
			else
				{
				fract = t %tps;
				frames =  t / tps;
				StartFrame = lmd->lastFrame+tps;
				int nv = lmd->SpringList.Count();
				for (int i=0;i<nv;i++)
					{
					lmd->SpringList[i].pos = lmd->WholeFrameCache[i].pos;
					lmd->SpringList[i].vel = lmd->WholeFrameCache[i].vel;
					}

				}

			for (int i = StartFrame; i < (frames*tps); i+=tps)
				{
				Matrix3 tm = lmd->SelfNode->GetObjectTM(i);



				Point3 pp,p,force;

				ObjectState tos;
				int nv = lmd->SpringList.Count();
				int tnv = nv;
				if (i != t)
					{
					if (TestAFlag(A_RENDER)) 
						{
						if (instanced==1)
							EnableModsAbove(lmd,FALSE);
						tos = lmd->SelfNode->EvalWorldState(i);
						if (instanced==1)
							EnableModsAbove(lmd,TRUE);
						}
					}
				}
			}
		}
*/
	return 0;
	}

int LagMod::RenderEnd(TimeValue t)
	{
	ClearAFlag(A_RENDER);
	NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
	nukeCache = FALSE;

	return 0;
	}




Interval LagMod::LocalValidity(TimeValue t)
	{
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
	}

//aszabo|feb.06.02 - When LocalValidity is called by ModifyObject,
// it returns NEVER and thus the object channels are marked non valid
// As a result, the mod stack enters and infinite evaluation of the modifier
// ModifyObject now calls GetValidity and CORE calls LocalValidity to
// allow for building a cache on the input of this modifier when it's 
// being edited 
Interval LagMod::GetValidity(TimeValue t)
{
	Interval iv = NEVER;
	iv.Set(t,t);
	return iv;
}

Matrix3 LagMod::CompMatrix(TimeValue t,INode *inode,ModContext *mc)
	{
	Interval iv;
	Matrix3 tm(1);	
	if (inode) 
		tm = tm * inode->GetObjTMBeforeWSM(t,&iv);
	return tm;
	}

int LagMod::HitTest(
		TimeValue t, INode* inode, 
		int type, int crossing, int flags, 
		IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{

	
	LagModData *lmd = (LagModData *) mc->localData;

	if (lmd == NULL) return 0;


	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits, res = 0;
	Matrix3 tm = CompMatrix(t,inode,mc);

	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	
	gw->setTransform(tm);

	// Hit test start point
	if (ip && ip->GetSubObjectLevel()==1) {

		if ((flags&HIT_SELONLY   &&  sel[0]) ||
			(flags&HIT_UNSELONLY && !sel[0]) ||
			!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) {
	
			gw->clearHitCode();
			p1->GetValue(t,&pt,FOREVER,CTRL_ABSOLUTE);
			gw->marker(&pt,HOLLOW_BOX_MRKR);
			if (gw->checkHitCode()) {
				vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
				res = 1;
				}
			}
		}
	else if ((ip && ip->GetSubObjectLevel()==2) )
		{


//		ObjectState os;

//		os = inode->EvalWorldState(t);
//loop through points checking for selection and then marking for points
//		int nv = os.obj->NumPoints(); 
		int nv = lmd->SpringList.Count();;
		for (int i=0;i<nv;i++)
			{
			if ((flags&HIT_SELONLY   &&  lmd->esel[i]) ||
				(flags&HIT_UNSELONLY && !lmd->esel[i]) ||
				!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
				{

				Point3 pt;
				gw->clearHitCode();
//				pt = os.obj->GetPoint(i);
				pt = lmd->SpringList[i].LocalPt;
				gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
				gw->marker(&pt,POINT_MRKR);
				if (gw->checkHitCode()) {
					vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL); 
					res = 1;
					}
				}
			}

		}
	else if ( (ip && ip->GetSubObjectLevel()==3) )
		{


//		ObjectState os;

//		os = inode->EvalWorldState(t);
//loop through points checking for selection and then marking for points
//		int nv = os.obj->NumPoints(); 
		int nv = lmd->SpringList.Count();;

		for (int i=0;i<nv;i++)
			{
			if ((flags&HIT_SELONLY   &&  lmd->wsel[i]) ||
				(flags&HIT_UNSELONLY && !lmd->wsel[i]) ||
				!(flags&(HIT_UNSELONLY|HIT_SELONLY)) ) 
				{

				Point3 pt;
				gw->clearHitCode();
				pt = lmd->SpringList[i].LocalPt;
//				pt = os.obj->GetPoint(i);
				gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
				gw->marker(&pt,POINT_MRKR);
				if (gw->checkHitCode()) {
					vpt->LogHit(inode, mc, gw->getHitDistance(), i, NULL); 
					res = 1;
					}
				}
			}

		}


	gw->setRndLimits(savedLimits);
	
	return res;

		return 0;
	}


void LagMod::DrawCrossSectionNoMarkers(Point3 a, Point3 align, float length, GraphicsWindow *gw)

{

#define NNUM_SEGS	8

Point3 plist[NNUM_SEGS+1];
Point3 mka,mkb,mkc,mkd;

align = Normalize(align);
	{
	int ct = 0;
	float angle = TWOPI/float(NNUM_SEGS) ;
	Matrix3 rtm = RotAngleAxisMatrix(align, angle);
	Point3 p(0.0f,0.0f,0.0f);
	if (align.x == 1.0f)
		{
		p.z = length;
		}
	else if (align.y == 1.0f)
		{
		p.x = length;
		}
	else if (align.z == 1.0f)
		{
		p.y = length;
		}
	else if (align.x == -1.0f)
		{
		p.z = -length;
		}
	else if (align.y == -1.0f)
		{
		p.x = -length;
		}
	else if (align.z == -1.0f)
		{
		p.y = -length;
		}
	else 
		{
		p = Normalize(align^Point3(1.0f,0.0f,0.0f))*length;
		}

	for (int i=0; i<NNUM_SEGS; i++) {
		p = p * rtm;
		plist[ct++] = p;
		}

	p = p * rtm;
	plist[ct++] = p;


	for (i=0; i<NNUM_SEGS+1; i++) 
		{
		plist[i].x += a.x;
		plist[i].y += a.y;
		plist[i].z += a.z;
		}
	}
mka = plist[15];
mkb = plist[3];
mkc = plist[7];
mkd = plist[11];

gw->polyline(NNUM_SEGS+1, plist, NULL, NULL, 0);

}



int LagMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flagst, ModContext *mc)
	{

	if (inode)
		{
		if (ip)
			{
			int nodeCount = ip->GetSelNodeCount();
			BOOL found = FALSE;
			for (int nct =0; nct < nodeCount; nct++)
				{
				if (inode == ip->GetSelNode(nct))
					{
					found = TRUE;
					nct = nodeCount;
					}
				}
			if (!found) return 0;
			}

		}
	GraphicsWindow *gw = vpt->getGW();
	Point3 pt[4];
	Matrix3 tm = CompMatrix(t,inode,mc);
	int savedLimits;

	LagModData *lmd = (LagModData *) mc->localData;

	if (lmd == NULL) return 0;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);
	
	// Draw start point
	if (ip && ip->GetSubObjectLevel() == 1) {
		if (sel[0]) 
			 gw->setColor(LINE_COLOR, (float)1.0, (float)0.0, (float)0.0);			 
		 else 
			gw->setColor(LINE_COLOR,(float)0.0, (float)0.0, (float)1.0);
			
	
	
		p1->GetValue(t,&pt[0],FOREVER,CTRL_ABSOLUTE);
		gw->marker(&pt[0],HOLLOW_BOX_MRKR);
		}
	else if (ip && ip->GetSubObjectLevel() == 2) {
//		ObjectState os;

//		os = inode->EvalWorldState(t);
		int nv = lmd->SpringList.Count();;
		for (int i = 0; i<nv;i++)
			{
			if (lmd->esel[i] == 1)
				{
				Point3 pt;
				pt = lmd->SpringList[i].LocalPt;
//				pt = os.obj->GetPoint(i);
				gw->setColor(LINE_COLOR, 1.0f,0.0f,1.0f);
				gw->marker(&pt,HOLLOW_BOX_MRKR);

				}
			}

		}
	else if (ip && ip->GetSubObjectLevel() == 3) {
//		ObjectState os;

//		os = inode->EvalWorldState(t);
//		int nv = os.obj->NumPoints();
		int nv = lmd->SpringList.Count();;
		for (int i = 0; i<nv;i++)
			{
			if (lmd->wsel[i] == 1)
				{
				Point3 pt;
				pt = lmd->SpringList[i].LocalPt;
//				pt = os.obj->GetPoint(i);
				gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
				gw->marker(&pt,HOLLOW_BOX_MRKR);

				}
			}

		}

	if (ip && (ip->GetSubObjectLevel() != 0) ) 
		{

		if (inPaint)
			{

			if (lmd->isHit)
				{
//draw 3d cursor
				Point3 x(1.0f,0.0f,0.0f),y(0.0f,1.0f,0.0f),z(0.0f,0.0f,1.0f);
				gw->setColor(LINE_COLOR, 1.0f,1.0f,0.0f);

				DrawCrossSectionNoMarkers(lmd->hitPoint, x, Radius, gw); 
				DrawCrossSectionNoMarkers(lmd->hitPoint, y, Radius, gw); 
				DrawCrossSectionNoMarkers(lmd->hitPoint, z, Radius, gw); 

				}

			}
		}

	if ((ip && (ip->GetSubObjectLevel() != 0) ) && (((Modifier *)this)->IsEnabled()) && (this->IsEnabledInViews()) 
		 )
		{


		ObjectState os;

		os = inode->EvalWorldState(t);
//loop through points checking for selection and then marking
		float r,g,b;
		Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
		Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
		Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);
		Point3 subSelSoftColor = GetUIColor(COLOR_SUBSELECTION_SOFT);
		Point3 subSelColor = GetUIColor(COLOR_SUBSELECTION);
		if (useCenter)
			{
			gw->startMarkers();
			for (int i=0;i<lmd->SpringList.Count();i++)
				{
				Point3 pt;
//			pt = os.obj->GetPoint(i);
			
				pt = lmd->SpringList[i].LocalPt;
//gte red is strongest,green, blue is weakest based on influence
				float infl;
				if (lmd->wsel[i]==1)
					{
					if (absoluteWeight)
						infl = weightDelta;
						else infl = 1.0f - lmd->SpringList[i].InheritVel + weightDelta;
					}
				else infl = 1.0f - lmd->SpringList[i].InheritVel;
				Point3 selColor(0.0f,0.0f,0.0f);


				if (infl <= 0.0f) selColor = subSelSoftColor;
				else if ( (infl<0.5) && (infl >= 0.0f))
					{
					selColor = selSoft + ( (selMedium-selSoft) * (infl/0.5f));
					}
				else if (infl<=1.0)
					{
					selColor = selMedium + ( (selHard-selMedium) * ((infl-0.5f)/0.5f));
					}
				else 
					{
					selColor = subSelColor;
					}
				r = selColor.x;
				g = selColor.y;
				b = selColor.z;

				gw->setColor(LINE_COLOR, r,g,b);
				gw->marker(&pt,SM_DOT_MRKR);
	
				if (inPaint)
					{
					if (lmd->psel[i] == 1)
						{
						gw->setColor(LINE_COLOR, 1.0f,1.0f,1.0f);
						gw->marker(&pt,HOLLOW_BOX_MRKR);
						}

					}



				}
			gw->endMarkers();
			}
		BOOL displaySprings;
		pblock2->GetValue(lag_displaysprings,t,displaySprings,FOREVER);
		if (displaySprings)
			{
			Point3 colorList[12];
			BOOL onList[12];
			for (int j = 0; j < 12; j++)
				{
				pblock2->GetValue(lag_springcolors,t,colorList[j],FOREVER,j);
				pblock2->GetValue(lag_customspringdisplay,t,onList[j],FOREVER,j);
				}

			for (j=0; j<lmd->edgeSprings.Count();j++)
				{
//				ObjectState os;

//				os = inode->EvalWorldState(t);
				int a = lmd->edgeSprings[j].v1;	
				int b = lmd->edgeSprings[j].v2;	
			

				Point3 ptA,ptB;
			
				ptA = os.obj->GetPoint(a);
				ptB = os.obj->GetPoint(b);
				Point3 l[3];
				l[0] = ptA;
				l[1] = ptB;
				Point3 selColor(0.0f,0.0f,0.0f);
				int id = lmd->edgeSprings[j].flags;
				if ((id <12) && (id >=0))
					{
					selColor = colorList[id];
					r = selColor.x;
					g = selColor.y;
					b = selColor.z;
					
					if (onList[id])
						{
						gw->setColor(LINE_COLOR, r,g,b);
						gw->polyline(2, l, NULL, NULL, 0, NULL);
						}

					}
/*				float infl = (float)lmd->edgeSprings[j].flags;
				infl = infl/12.0f;

				Point3 selSoft = GetUIColor(COLOR_SUBSELECTION_SOFT);
				Point3 selMedium = GetUIColor(COLOR_SUBSELECTION_MEDIUM);
				Point3 selHard = GetUIColor(COLOR_SUBSELECTION_HARD);

				if (infl <= 0.0f) selColor = GetUIColor(COLOR_SUBSELECTION_SOFT);
				else if ( (infl<0.5) && (infl >= 0.0f))
					{
					selColor = selSoft + ( (selMedium-selSoft) * (infl/0.5f));
					}
				else if (infl<=1.0)
					{
					selColor = selMedium + ( (selHard-selMedium) * ((infl-0.5f)/0.5f));
					}
				else 
					{
					selColor = GetUIColor(COLOR_SUBSELECTION);
					}
*/
				}
			}

		}

//	gw->marker(&pt[1],HOLLOW_BOX_MRKR);

	gw->setRndLimits(savedLimits);

	return 0;

	}

void LagMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{
	
	if (inPaint)
		{
		Matrix3 tm = CompMatrix(t,inode,mc);
		Point3 pt1, pt2;
		box.Init();
		LagModData *lmd = (LagModData *) mc->localData;
		if (lmd == NULL) return;

		pt1 = lmd->hitPoint;
		box += pt1 * tm;
		box.EnlargeBy(Radius*2.5f);
		}

	
	}

void LagMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin)
	{
	if (ip && ip->GetSubObjectLevel() == 1) {
		if (sel[0]) {
			p1->SetValue(
				ReferenceFrame,VectorTransform(tmAxis*Inverse(partm),val),
				TRUE,CTRL_RELATIVE);
			updateWeights = TRUE;
			}
		}
	else if (ip && ip->GetSubObjectLevel() == 2) {
		}
	else if (ip && ip->GetSubObjectLevel() == 3) {
		}

	}

void LagMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{

	Matrix3 tm = CompMatrix(t,node,mc);
	Point3 pt(0,0,0), p;
	int c=0;
	if (sel[0]) {
		p1->GetValue(t,&p,FOREVER,CTRL_ABSOLUTE);
		pt += p;
		c++;
		}
	if (c) pt /= float(c);
	tm.PreTranslate(pt);
	cb->Center(tm.GetTrans(),0);

	}

void LagMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,
		INode *node,ModContext *mc)
	{
	Matrix3 tm = CompMatrix(t,node,mc);
	cb->TM(tm,0);
	
	}


void LagMod::SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
	{

	LagModData *lmd = (LagModData*)hitRec->modContext->localData;

	if (lmd == NULL) return;

	if (ip && ip->GetSubObjectLevel() == 1) {
	
		while (hitRec) {
			assert(hitRec->hitInfo<=1);
			BOOL state = selected;
			if (invert) state = !sel[hitRec->hitInfo];
			if (state) sel[hitRec->hitInfo] = 1;
			else       sel[hitRec->hitInfo] = 0;
			if (!all) break;
			hitRec = hitRec->Next();
			}	
		}
	else if ( (ip && ip->GetSubObjectLevel() == 2) || (ip && ip->GetSubObjectLevel() == 3)){
		BOOL add = GetKeyState(VK_CONTROL)<0;
		BOOL sub = GetKeyState(VK_MENU)<0;
		updateWeights = TRUE;

		if (!add && !sub) 
			{
			if (ip && ip->GetSubObjectLevel() == 2)
				{
				for (int i =0;i<lmd->esel.Count();i++)
					lmd->esel[i] = FALSE;
				}
			else
				{
				for (int i =0;i<lmd->wsel.Count();i++)
					lmd->wsel[i] = FALSE;
				}
			}

		int Count = 0;
		BOOL state = selected;
		while (hitRec) {
			state = hitRec->hitInfo;
			if (sub)
				{
				if (ip && ip->GetSubObjectLevel() == 2)
					lmd->esel[state] = FALSE;
				else lmd->wsel[state] = FALSE;
				}
			else 
				{
				if (ip && ip->GetSubObjectLevel() == 2)
					lmd->esel[state] = TRUE;
				else lmd->wsel[state] = TRUE;
				}

		
			hitRec = hitRec->Next();
			Count++;
			}	
		if (ip && ip->GetSubObjectLevel() == 3)
			{
			tempBitArray.SetSize(lmd->wsel.Count());
			for (int i = 0; i < lmd->wsel.Count(); i++)
				tempBitArray.Set(i,lmd->wsel[i]);
			macroRecorder->FunctionCall(_T("$.modifiers[#flex].flexOps.selectVertices"), 1, 0, 
												mr_bitarray,&(tempBitArray));

			}
		else if (ip && ip->GetSubObjectLevel() == 2)
			{
			tempBitArray.SetSize(lmd->esel.Count());
			for (int i = 0; i < lmd->wsel.Count(); i++)
				tempBitArray.Set(i,lmd->esel[i]);
			macroRecorder->FunctionCall(_T("$.modifiers[#flex].flexOps.setEdgeList"), 1, 0, 
												mr_bitarray,&(tempBitArray));

			}
		}


	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	
	}

void LagMod::ClearSelection(int selLevel)
	{
	ModContextList mcList;		
	INodeTab nodes;

	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for ( int i = 0; i < objects; i++ ) 
			{
			LagModData *lmd = (LagModData*)mcList[i]->localData;
			if (lmd)
				{
				if (ip->GetSubObjectLevel() == 2)
					{
					for (int i =0 ; i < lmd->esel.Count(); i++)
						lmd->esel[i] = 0;
					}
				else if (ip->GetSubObjectLevel() == 3)
					{
					for (int i =0 ; i < lmd->wsel.Count(); i++)
						lmd->wsel[i] = 0;
					}
				}	
			}
		}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void LagMod::SelectAll(int selLevel)
	{

	ModContextList mcList;		
	INodeTab nodes;

	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for ( int i = 0; i < objects; i++ ) 
			{
			LagModData *lmd = (LagModData*)mcList[i]->localData;
			if (lmd)
				{
				if (ip->GetSubObjectLevel() == 2)
					{
					for (int i =0 ; i < lmd->esel.Count(); i++)
						lmd->esel[i] = 1;
					}
				else if (ip->GetSubObjectLevel() == 3)
					{
					for (int i =0 ; i < lmd->wsel.Count(); i++)
						lmd->wsel[i] = 1;
					}
				}
			}
		}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void LagMod::InvertSelection(int selLevel)
	{
	ModContextList mcList;		
	INodeTab nodes;

	if (ip)
		{
		ip->GetModContexts(mcList,nodes);
		int objects = mcList.Count();

		for ( int i = 0; i < objects; i++ ) 
			{
			LagModData *lmd = (LagModData*)mcList[i]->localData;
			if (lmd)
				{
				if (ip->GetSubObjectLevel() == 2)
					{
					for (int i =0 ; i < lmd->esel.Count(); i++)
						lmd->esel[i] = !lmd->esel[i];
					}
				else if (ip->GetSubObjectLevel() == 3)
					{
					for (int i =0 ; i < lmd->wsel.Count(); i++)
						lmd->wsel[i] = !lmd->wsel[i];
					}
				}
			}
		}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	}

void LagMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	switch (level) {
		case 0:
			if (ip->GetCommandMode() == PaintMode) {
				ip->SetStdCommandMode(CID_OBJMOVE);
				return;
				}
			iPaintButton->Enable(FALSE);

			break;
		case 1: // Center Point
			iPaintButton->Enable(TRUE);
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
		case 2: // Edge
			iPaintButton->Enable(TRUE);
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
		case 3: // weigths
			iPaintButton->Enable(TRUE);
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;		
		}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}


void LagMod::StartPaintMode()
{
if ( !ip ) return;
if (ip && ( (ip->GetSubObjectLevel()==1) || (ip->GetSubObjectLevel()==2) || (ip->GetSubObjectLevel()==3) ) )
	{

//get mesh
	if (ip->GetCommandMode() == PaintMode) {
		ip->SetStdCommandMode(CID_OBJMOVE);
		return;
		}

    ip->SetCommandMode(PaintMode);

    }
}


RefTargetHandle LagMod::GetReference(int i)
	{
	switch (i) {
		case PBLOCK_REF: return (RefTargetHandle)pblock2;

		case POINT1_REF: return (RefTargetHandle)p1;

		default : return NULL;
		}
	return NULL;
	}

void LagMod::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case PBLOCK_REF: pblock2 = (IParamBlock2*)rtarg; break;
		case POINT1_REF: p1     = (Control*)rtarg; break;
		}
	}


Animatable* LagMod::SubAnim(int i)
	{
	switch (i) {
		case PBLOCK_REF: 		return pblock2;
		case POINT1_REF: 		return p1;
		default: 			return NULL;   
		}
	}

TSTR LagMod::SubAnimName(int i)
	{
	if (i==POINT1_REF) return GetString(IDS_RB_LAGPOINTS);
	return _T(""); 

	}

RefResult LagMod::NotifyRefChanged(
		Interval changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message)
	{
	switch (message) {
		case REFMSG_CHANGE:	
			{ 
			if (hTarget == pblock2)
				{
				if (!disableUI)
					{
					ParamID changing_param = pblock2->LastNotifyParamID();
				
					lag_param_blk.InvalidateUI(changing_param);
					}
				}

			break;
			}


		}
	return REF_SUCCEED;
	}


class TempStorageClass
{
public:
	Point3 pos;
	float IVel;
	BYTE sel;
	BOOL modified;
};


#define BACKPATCH_CHUNK		0x2120
#define VERSION_CHUNK		0x2130



IOResult LagMod::Save(ISave *isave)
	{
	Modifier::Save(isave);
	ULONG nb;
	flexVersion = 400;

	ULONG id = isave->GetRefID(container);

	isave->BeginChunk(BACKPATCH_CHUNK);
	isave->Write(&id,sizeof(ULONG),&nb);
	isave->EndChunk();


	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&flexVersion,sizeof(int),&nb);
	isave->EndChunk();


	return IO_OK;
	}



class LagModPostLoad : public PostLoadCallback {
	public:
		LagMod *n;
		LagModPostLoad(LagMod *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->container != NULL)
				{
				n->container->RegisterTVNodeNotify(n->notify);
				n->container->HideChildren(TRUE);
				}
			if (n->flexVersion < 400)
				{
				n->pblock2->SetValue(lag_samples,0,1);
				}
			delete this; 


			} 
	};


IOResult LagMod::Load(ILoad *iload)
	{

	IOResult res = IO_OK;
	int NodeID = 0;
	ULONG nb;



	Modifier::Load(iload);

	TempStorageClass tsc;


	int ct = 0;
	flexVersion = 0;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VERSION_CHUNK:
				iload->Read(&flexVersion,sizeof(int), &nb);
				break;
			case BACKPATCH_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&container);
					}

				break;
			case POS_WEIGHT_CHUNK:
				Point3 Zero(0.0f,0.0f,0.0f);
			
//read in data
				int ct;
				iload->Read(&ct, sizeof(ct), &nb);
				oldSpringList.SetCount(ct);
				oldesel.SetCount(ct);
				for (int i=0;i<ct;i++)
					{
					iload->Read(&tsc, sizeof(tsc), &nb);

					oldSpringList[i].pos = tsc.pos;
					oldSpringList[i].init_pos = tsc.pos;
					oldSpringList[i].vel = Zero;
					oldSpringList[i].InheritVel = tsc.IVel;
					oldSpringList[i].modified = tsc.modified;
					oldesel[i] = tsc.sel;
				
					}

				break;

			}


		iload->CloseChunk();
		if (res!=IO_OK)
			return res;
		

		}

	iload->RegisterPostLoadCallback(new LagModPostLoad(this));

	return IO_OK;
}

Point3 LagDeformer::Map(int i, Point3 p)
	{

	if (lmd)
		{
		if (i < lmd->SpringList.Count())
			{
			float IVel = 0.0f;
		
			if ((mod->useCenter) /*&& (!mod->constrainSpline)*/)
				{
				if (lmd->wsel[i] == 1)
					{
					if (mod->absoluteWeight) 
						IVel = (1.0f-mod->weightDelta) * mod->falloff;
					else IVel = (lmd->SpringList[i].InheritVel-mod->weightDelta) * mod->falloff;
					}
				else IVel = lmd->SpringList[i].InheritVel * mod->falloff;
				p = (lmd->SpringList[i].pos * lmd->InverseTM * IVel)+(p *(1.0f-IVel));
				lmd->SpringList[i].LocalPt = p;
				}
			else
				{
				IVel = mod->falloff;
				p = (lmd->SpringList[i].pos * lmd->InverseTM * IVel)+(p *(1.0f-IVel));
//				p = (lmd->SpringList[i].pos * lmd->InverseTM);
				lmd->SpringList[i].LocalPt = p;
				}
			}
		}
	return p;


	}

Point3 LastFrameLagDeformer::Map(int i, Point3 p)
	{

	if (lmd)
		{
		if (i < lmd->SpringList.Count())
			{
			float IVel = 0.0f;
		
			if ((mod->useCenter) /*&& (!mod->constrainSpline)*/)
				{
				if (lmd->wsel[i] == 1)
					{
					if (mod->absoluteWeight) 
						IVel = (1.0f-mod->weightDelta) * mod->falloff;
					else IVel = (lmd->SpringList[i].InheritVel-mod->weightDelta) * mod->falloff;
					}
				else IVel = lmd->SpringList[i].InheritVel * mod->falloff;
				p = (lmd->WholeFrameCache[i].pos * lmd->InverseTM * IVel)+(p *(1.0f-IVel));
				lmd->SpringList[i].LocalPt = p;
				}
			else
				{
				p = (lmd->WholeFrameCache[i].pos * lmd->InverseTM);
				lmd->SpringList[i].LocalPt = p;
				}
			}
		}
	return p;


	}


void LagMod::ResetSprings()

{
ModContextList mcList;		
INodeTab nodes;

if (!ip) return;

ip->GetModContexts(mcList,nodes);
int objects = mcList.Count();

for ( int k = 0; k < objects; k++ ) 
	{

	LagModData *lmd = (LagModData*)mcList[k]->localData;
	
	if (lmd)
		{

		for (int i = 0; i < lmd->SpringList.Count(); i++)
			lmd->SpringList[i].modified = FALSE;
		}
	}
}


void LagMod::AddForceField()

{
int ct = pblock2->Count(lag_force_node);
ff.SetCount(ct);
for (int i = 0; i < ct; i++)
	{
	INode *node = NULL;
	Interval iv;
	pblock2->GetValue(lag_force_node,0,node,iv, i);
	if (node != NULL)
		{
		WSMObject *obref=(WSMObject*)node->GetObjectRef();

		// to accommodate XRef, 020321 --prs.
		if (obref->ClassID() == Class_ID(XREFOBJ_CLASS_ID, 0))
			obref = (WSMObject *)((IXRefObject *)obref)->GetReference(0);
		ff[i] = (obref != NULL) ? obref->GetForceField(node) : NULL;

		}
	else ff[i]=NULL;
	}
}

void LagMod::AddCollider()

{
int ct = pblock2->Count(lag_collider_node);
colliderObjects.SetCount(ct);
for (int i = 0; i < ct; i++)
	{
	INode *node = NULL;
	Interval iv;
	pblock2->GetValue(lag_collider_node,GetCOREInterface()->GetTime(),node,iv, i);
	if (node != NULL)
		{
		WSMObject *obref=(WSMObject*)node->GetObjectRef();
		colliderObjects[i]=obref->GetCollisionObject(node);
		}
	else colliderObjects[i]=NULL;
	}
}

int LagMod::GetCollision(TimeValue t, Point3 &p, Point3 &v, TimeValue &dt)
{

Point3 f(0.0f,0.0f,0.0f);
	
int ct = pblock2->Count(lag_collider_node);
BOOL collision = FALSE;
float initialDt = dt;
v = v/(float) initialDt;
for (int i = 0; i < ct; i++)
	{
	INode *node = NULL;
	Interval iv;
	pblock2->GetValue(lag_collider_node,0,node,iv, i);
	if ((node != NULL) && (colliderObjects[i] != NULL))
		{
		float dtf = (float) dt;
		float finalDT;
		collision = colliderObjects[i]->CheckCollision(t,p, v, (float)dtf,0, &finalDT,FALSE);
		if (collision)
			{
			dt = (TimeValue) finalDT;
			i = ct;
			}
//		f += 16000.0f * ff[i]->Force(t,p,v,0);
		}
	}
if (collision) 
	v = (v*(float) initialDt);// *(float) dt/160.f;
else v *= (float) initialDt;

if (collision)
	p += v;



return collision;
}


Point3 LagMod::GetForce(TimeValue t, Point3 p, Point3 v, Matrix3 tm)
{

Point3 f(0.0f,0.0f,0.0f);
	


int ct = pblock2->Count(lag_force_node);
for (int i = 0; i < ct; i++)
	{
	INode *node = NULL;
	Interval iv;
	pblock2->GetValue(lag_force_node,0,node,iv, i);
	if ((node != NULL) && (ff[i] != NULL))
		f += 16000.0f * ff[i]->Force(t,p,v,0);
	}

return f;
}


void LagMod::ComputeCenterFalloff(LagModData *lmd, TimeValue t, ObjectState *os)
{
	Point3 Center;
	Interval iv;
	float Dist =0.0f;

	p1->GetValue(t,&Center,iv,CTRL_ABSOLUTE);
	int nv = os->obj->NumPoints(); 

//build a temp edge list here 
	Tab<Point3> EdgeList;
	EdgeList.Append(1,&Center,1);

	Tab<int> EdgeIndex;
	EdgeIndex.SetCount(nv);

	for (int i = 0; i< lmd->esel.Count();i++)
		{
		if (lmd->esel[i]==1)
			{
			Point3 ep = os->obj->GetPoint(i);
			EdgeList.Append(1,&ep,1);
			}

		}


	for (i=0;i<nv;i++)
		{
//loop through all edges find closest
		float temp = Length(EdgeList[0] - os->obj->GetPoint(i));
		EdgeIndex[i] = 0;
		for (int j = 1; j < EdgeList.Count(); j++)
			{
			float etemp;
			etemp = Length(EdgeList[j] - os->obj->GetPoint(i));
			if (etemp < temp) 
				{
				temp = etemp;
				EdgeIndex[i] = j;
				}
			}
//				temp = Length(Center - os->obj->GetPoint(i));
		if (temp>Dist)
			Dist = temp;
		}
	BOOL nuke = FALSE;

	for (i=0;i<nv;i++)
		{
		float temp;
		temp = Length(EdgeList[EdgeIndex[i]] - os->obj->GetPoint(i))/Dist;
		temp = (temp*temp);
		if (!lmd->SpringList[i].modified) 
			{
			lmd->SpringList[i].InheritVel = temp;
			}
					
		}
	EdgeList.ZeroCount();

}



void LagMod::EnableModsAbove(LagModData *lmd, BOOL enable)
	{

	int				i;
	SClass_ID		sc;
	IDerivedObject* dobj;

// return the indexed modifier in the mod chain
	INode *node = lmd->SelfNode;
	BOOL found = FALSE;

// then osm stack
	Object* obj = node->GetObjectRef();
	int ct = 0;

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;
		if (!enable) 
			{
			enableStates.ZeroCount();
			enableViewStates.ZeroCount();
			}

		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (i = 0; i < dobj->NumModifiers(); i++)
				{
				TSTR name;
				Modifier *m = dobj->GetModifier(i);
				m->GetClassName(name);
				if (this == dobj->GetModifier(i))
					found = TRUE;


				BOOL en = m->IsEnabled();
				BOOL env = m->IsEnabledInViews();
//				DebugPrint("%s enable state %d view %d\n",name,en,env);
				if (!enable)
					{
					if (!found)
						{
						enableStates.Append(1,&en,1);
						enableViewStates.Append(1,&env,1);
						m->DisableMod();
						m->DisableModInViews();
						}
					}
				else 
					{
					if (!found)
						{
						if (!enableStates[ct])
							m->DisableMod();
						else m->EnableMod();
						if (!enableViewStates[ct++])
							m->DisableModInViews();
						else m->EnableModInViews();
						}
					}

				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			sc = dobj->SuperClassID();
			}
		}

	if ((dobj = node->GetWSMDerivedObject()) != NULL)
		for (i = 0; i < dobj->NumModifiers(); i++)
			{
			Modifier *m = dobj->GetModifier(i);
			BOOL en = m->IsEnabled();
			BOOL env = m->IsEnabledInViews();
			if (!enable)
				{
				enableStates.Append(1,&en,1);
				enableViewStates.Append(1,&env,1);
				}
			else
				{
				if (!enableStates[ct])
					m->DisableMod();
				else m->EnableMod();
				if (!enableViewStates[ct++])
					m->DisableModInViews();
				else m->EnableModInViews();

				}

			}


	}
void LagMod::Solve(int level, LagModData *lmd, TimeValue i, TimeValue t, 
				 int nv, ObjectState *os, int samples)

{
	Matrix3 tm = lmd->SelfNode->GetObjectTM(i);

	for (int k=0;k<ff.Count();k++)
		{
		if (ff[k]) 
			{
			ff[k]->DeleteThis();
			}

		ff[k] = NULL;
		}

	for (k=0;k<colliderObjects.Count();k++)
		{
		if (colliderObjects[k]) 
			{
			colliderObjects[k]->DeleteThis();
			}

		colliderObjects[k] = NULL;
		}

//compute gravity
	AddForceField();
	AddCollider();

	Point3 pp,p,force;

	ObjectState tos;
	int tnv = nv;

	float time = 1.0f;//( 1.0f/(float)samples);  put back with a multiplier in 4+
	float timeSquared = time*time;
	nv = lmd->SpringList.Count();
	for (int j=0;j<nv;j++)
		{
		if ( (!(subSelection) || (affectAll) || (selectedList[j])  ) &&
			 ((j<lmd->esel.Count()) && (lmd->esel[j] ==0)) )
//		if (1) 
			{
	
			Point3 g(0.0f,0.0f,0.0f);
			g = GetForce(i, lmd->SpringList[j].pos, lmd->SpringList[j].vel, tm)*time;
			
			if (i != t)
				{
				if (TestAFlag(A_RENDER)) 
					{
					if (j < lmd->pointCache.Count()) p = lmd->pointCache[j];
					}
				else 
					{
					if (j < nv) p = os->obj->GetPoint(j);
					}
				}
			else p = os->obj->GetPoint(j);

			lmd->SpringList[j].tempVel[level].x = 0.0f;
			lmd->SpringList[j].tempVel[level].y = 0.0f;
			lmd->SpringList[j].tempVel[level].z = 0.0f;

			lmd->SpringList[j].tempVel[level] += g ;
			if (useChase)
				{
				force = (lmd->SpringList[j].pos-(p * tm));
				lmd->SpringList[j].tempVel[level] += ((-strength*force)-(dampening*lmd->SpringList[j].vel))*time;
				}
			}

		}
//do springs


//need to put a bitarray check here for any verts that are not used

	for (j=0; j<lmd->edgeSprings.Count();j++)
		{
		int a = lmd->edgeSprings[j].v1;	
		int b = lmd->edgeSprings[j].v2;	
		if ((a>=0) && (a<nv) && (b>=0) && (b<nv))
			{
			Point3 p1,p2;
			Point3 v1(0.0f,0.0f,0.0f),v2(0.0f,0.0f,0.0f);
			BOOL lockedEnd=FALSE;
			if ( ( !(subSelection) ||  (affectAll) || (selectedList[a])) &&
				 ((a<lmd->esel.Count()) && (lmd->esel[a] ==0)) )
				{
				p1=lmd->SpringList[a].pos;
				v1 = lmd->SpringList[lmd->edgeSprings[j].v1].vel;
				}
			else
				{
				p1 = os->obj->GetPoint(a)*tm;
				}
			if ( (!(subSelection)  || (affectAll) || (selectedList[b]) ) &&
				 ((b<lmd->esel.Count()) && (lmd->esel[b] ==0)) )
				{
				p2=lmd->SpringList[b].pos;
				v2 = lmd->SpringList[lmd->edgeSprings[j].v2].vel;
				}
			else
				{ 
				p2 = os->obj->GetPoint(b)*tm;
				}
			int flag = lmd->edgeSprings[j].flags;	
			float strength = extraStrAndSway[flag].strength;
			float dampening = extraStrAndSway[flag].sway;
//if (flag !=0) DebugPrint("got here\n");
			Point3 l = p1-p2;
			float len = Length(p2-p1);
			float restLen = lmd->edgeSprings[j].dist;
			Point3 dvel = v1-v2;
			if (len < 0.0001f)
				{
//				DebugPrint("Illegal Spring Length %d\n",j);
				}
			else 
				{
				Point3 v = (strength*(len-restLen)+dampening*((DotProd(dvel,l))/len)) * l/len;
				if (lockedEnd) v *= 2.0f;
				v *= time;
				lmd->SpringList[lmd->edgeSprings[j].v1].tempVel[level] -= v;
				lmd->SpringList[lmd->edgeSprings[j].v2].tempVel[level] += v;
				}
			}
		}



/*
		for (j=0;j<nv;j++)
			{
			lmd->SpringList[j].tempVel[level] *= (1.0f-friction);
			}
				
*/
		

}

void LagMod::Evaluate(LagModData *lmd, TimeValue i, TimeValue t, 
				 int nv, ObjectState *os, int samples, float per)
{

	Solve(0, lmd, i, t,  nv, os,samples);
	int tps = GetTicksPerFrame();
	float time = 1.0f;//( 1.0f/(float)samples); put back with a multiplier in 4+


	TimeValue tempT;
	if (solver >= 1)
		{
		for (int j=0;j<nv;j++)
			{
			lmd->SpringList[j].tempPos[0] =  lmd->SpringList[j].pos;
			lmd->SpringList[j].pos = lmd->SpringList[j].pos + lmd->SpringList[j].tempVel[0] * (0.5f*time);
			}
		tempT = i + tps/2;
		Solve(1, lmd, tempT, t,  nv, os,samples);
		}

	if (solver > 1)
		{
		for (int j=0;j<nv;j++)
			{
			lmd->SpringList[j].pos = lmd->SpringList[j].tempPos[0] + lmd->SpringList[j].tempVel[1] * 0.5f*time;

			}
		Solve(2, lmd, tempT, t,  nv, os,samples);

		for (j=0;j<nv;j++)
			{
			lmd->SpringList[j].pos = lmd->SpringList[j].tempPos[0] + lmd->SpringList[j].tempVel[2] *0.5f*time;
			}
		Solve(3, lmd, tempT, t,  nv, os,samples);


		for (j=0;j<nv;j++)
			{
			lmd->SpringList[j].pos = lmd->SpringList[j].tempPos[0] + lmd->SpringList[j].tempVel[3]*time;
			}
		tempT += tps/2;
		Solve(4, lmd, tempT, t,  nv, os,samples);
		}


//apply vels
	Matrix3 tm = lmd->SelfNode->GetObjectTM(t);
	for (int j=0;j<nv;j++)
		{
		if ( ( !(subSelection)  || (affectAll) || (selectedList[j])  )  &&
			 ((j<lmd->esel.Count()) && (lmd->esel[j] ==0)) )
			{
						
			TimeValue dt = tps/(float)samples;	
			if (solver ==0)
				lmd->SpringList[j].vel += lmd->SpringList[j].tempVel[0];
			if (solver ==1)
				{
				lmd->SpringList[j].pos = lmd->SpringList[j].tempPos[0];
				lmd->SpringList[j].vel += lmd->SpringList[j].tempVel[1];
				}
			if (solver ==2)
				{
				lmd->SpringList[j].pos = lmd->SpringList[j].tempPos[0];
				lmd->SpringList[j].vel += lmd->SpringList[j].tempVel[0]/6.0f + 
					lmd->SpringList[j].tempVel[1]/3.0f + 
					lmd->SpringList[j].tempVel[2]/3.0f + 
					lmd->SpringList[j].tempVel[3]/6.0f ;
				}
			}
		else
			{
			lmd->SpringList[j].pos = lmd->SpringList[j].LocalPt * tm;
			lmd->SpringList[j].vel = Point3(0.0f,0.0f,0.0f);
			}
		}
//now do second filter here
	if (holdLength)
		FilterPass(lmd,1, holdLengthPercent, per);

//apply collisions
	for (j=0;j<nv;j++)
		{
		if ( ( !(subSelection)  || (affectAll) || (selectedList[j])  )  &&
			 ((j<lmd->esel.Count()) && (lmd->esel[j] ==0)) )
			{
						
			TimeValue dt = tps/(float)samples;	
			Point3 p,v;
			p = lmd->SpringList[j].pos;
			v = lmd->SpringList[j].vel*per;
			BOOL col = FALSE;

			col = GetCollision(i, p, v, dt);
			if (!col) 
				lmd->SpringList[j].pos += lmd->SpringList[j].vel*per*time;
			else
	
				{
				Point3 offset = p - lmd->SpringList[j].pos;
				lmd->SpringList[j].pos = p+v;
				lmd->SpringList[j].vel = v;
				}

			}
		}


}

BOOL RecursePipeAndMatch(ModContext *smd, Object *obj)
	{
	SClass_ID		sc;
	IDerivedObject* dobj;
	Object *currentObject = obj;

	if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
		{
		dobj = (IDerivedObject*)obj;
		while (sc == GEN_DERIVOB_CLASS_ID)
			{
			for (int j = 0; j < dobj->NumModifiers(); j++)
				{
				ModContext *mc = dobj->GetModContext(j);
				if (mc == smd)
					{
					return TRUE;
					}

				}
			dobj = (IDerivedObject*)dobj->GetObjRef();
			currentObject = (Object*) dobj;
			sc = dobj->SuperClassID();
			}
		}

	int bct = currentObject->NumPipeBranches(FALSE);
	if (bct > 0)
		{
		for (int bi = 0; bi < bct; bi++)
			{
			Object* bobj = currentObject->GetPipeBranch(bi,FALSE);
			if (RecursePipeAndMatch(smd, bobj)) return TRUE;
			}

		}

	return FALSE;
}

INode* LagMod::GetNodeFromModContext(ModContext *smd, int &which)
	{

	int	i;

    MyEnumProc dep;              
	EnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
			{
			Object* obj = node->GetObjectRef();
	
			if ( RecursePipeAndMatch(smd,obj) )
				{
				which = i;
				return node;
				}
			}
		}
	return NULL;
	}
/*
INode* LagMod::GetNodeFromModContext(ModContext *smd, int &which)
	{

	int				i;
	SClass_ID		sc;
	IDerivedObject* dobj;

    MyEnumProc dep;              
	EnumDependents(&dep);
	for ( i = 0; i < dep.Nodes.Count(); i++)
		{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		Object* obj = node->GetObjectRef();
		int ct = 0;

		if ((sc = obj->SuperClassID()) == GEN_DERIVOB_CLASS_ID)
			{
			dobj = (IDerivedObject*)obj;
			while (sc == GEN_DERIVOB_CLASS_ID)
				{
				for (int j = 0; j < dobj->NumModifiers(); j++)
					{
					ModContext *mc = dobj->GetModContext(j);
					if (mc == smd)
						{
						return node;
						which = i;
						}

					}
				dobj = (IDerivedObject*)dobj->GetObjRef();
				sc = dobj->SuperClassID();

				}

			}
		}
	return NULL;
	}

*/

void LagMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{	


//this is a hack to handle if scatter is in my dependancy
//scatter and lag do not get along so when scatter using its animationn offset i turn my self off
//to prevent an infintie loop of lag and scatter trying to update themselves 
    MyEnumProc dep;              
	dep.nukeME = FALSE;
	dep.xRefFound = FALSE;
	dep.count = 0;
	EnumDependents(&dep);
	if (dep.nukeME) return;

	int instanced = dep.count;

	if (os->obj->IsWorldSpaceObject()) return;

	int nv = os->obj->NumPoints();

	int tps = GetTicksPerFrame();

//7-1-99
	BOOL isXRefObj = dep.xRefFound;

	if (mc.localData == NULL)
		{


//add a new inode to trackview
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(LAGCONTAINERMASTER_TVNODE_CLASS_ID);
		if (!tvroot) 
			{
			ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
			global->AddNode(mcontainer,GetString(IDS_PW_FLEXDATA),LAGCONTAINERMASTER_TVNODE_CLASS_ID);
			tvroot = mcontainer;
			}

//add a new a container
		if (container == NULL)
			{
			container =  CreateITrackViewNode(TRUE);
			container->HideChildren(TRUE);
			tvroot->AddNode(container,GetString(IDS_PW_FLEXDATA),LAGCONTAINER_TVNODE_CLASS_ID);
			container->RegisterTVNodeNotify(notify);
			}

		INode *localnode;
		int id;
		localnode = GetNodeFromModContext(&mc,id);

		container->AddController(localnode->GetTMController(),localnode->GetName(),LAGNODE_TVNODE_CLASS_ID);
		LagModData *d  = new LagModData(id,localnode);
		mc.localData = d;
			
//load up old pre beta files
		if (oldSpringList.Count() != 0)
			{
			LagModData *d = (LagModData *) mc.localData;

			d->SpringList = oldSpringList;
			d->esel = oldesel;
			d->psel.SetCount(oldesel.Count());
			d->wsel.SetCount(oldesel.Count());
			for (int esel_index = 0; esel_index < nv; esel_index++)
				{
				d->psel[esel_index] = 0;
				d->wsel[esel_index] = 0;
				}
			}
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate

		return;
				
		}
	else
		{
		LagModData *d = (LagModData *) mc.localData;
		if (d->SelfNode == NULL)
			{

//add a new inode to trackview
			ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
			ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
			ITrackViewNode *tvroot = global->GetNode(LAGCONTAINERMASTER_TVNODE_CLASS_ID);
			if (!tvroot) 
				{
				ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
				global->AddNode(mcontainer,GetString(IDS_PW_FLEXDATA),LAGCONTAINERMASTER_TVNODE_CLASS_ID);
				tvroot = mcontainer;
				}
//add a new a container
			if (container == NULL)
				{
				container = CreateITrackViewNode(TRUE);
				container->HideChildren(TRUE);
				tvroot->AddNode(container,GetString(IDS_PW_FLEXDATA),LAGCONTAINER_TVNODE_CLASS_ID);
				container->RegisterTVNodeNotify(notify);
				}

			INode *localnode;
			int id;
			localnode = GetNodeFromModContext(&mc,id);

			container->AddController(localnode->GetTMController(),localnode->GetName(),LAGNODE_TVNODE_CLASS_ID);
			d->SelfNode = localnode;

			NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
			Interval valid;
			valid.SetEmpty();
			os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
			return;
			}


				
		}


	LagModData *lmd = (LagModData *) mc.localData;

//7-1-99 special acase code to handle xrefs since xrefs don't save thus the backpointer is wrong
	if (isXRefObj)
		{
		lmd->SelfNode = NULL;
		if (dep.Nodes.Count() > 0)
			lmd->SelfNode = dep.Nodes[0];
		}
//this is here to handle a merge you will have a lmd nad self node but no container which must be recreated
	else if ((container == NULL) && (lmd) && (lmd->SelfNode))
		{
//add a new inode to trackview
		Interface *tip = ip;
		if (!tip) tip = GetCOREInterface();
		ITrackViewNode *tvr = tip->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(Class_ID(0xb27e9f2a, 0x73fad370));
		ITrackViewNode *tvroot = global->GetNode(LAGCONTAINERMASTER_TVNODE_CLASS_ID);
		if (!tvroot) 
			{
			ITrackViewNode *mcontainer =  CreateITrackViewNode(TRUE);
			global->AddNode(mcontainer,GetString(IDS_PW_FLEXDATA),LAGCONTAINERMASTER_TVNODE_CLASS_ID);
			tvroot = mcontainer;
			}
//add a new a container
		container = CreateITrackViewNode(TRUE);
		container->HideChildren(TRUE);
		tvroot->AddNode(container,GetString(IDS_PW_FLEXDATA),LAGCONTAINER_TVNODE_CLASS_ID);
		container->AddController(lmd->SelfNode->GetTMController(),lmd->SelfNode->GetName(),LAGNODE_TVNODE_CLASS_ID);
		container->RegisterTVNodeNotify(notify);
		}
	else 	if (container == NULL)
		{
		return;
		}


	if ((ip && editMod == this) && (lmd) && (lmd->SelfNode == NULL))
		{
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
		return;
		}


	Point3 Center;
	Interval iv = FOREVER;
	pblock2->GetValue(lag_absolute,t,absoluteWeight,iv);
	pblock2->GetValue(lag_flex,t,falloff,iv);
	
	pblock2->GetValue(lag_strength,t,strength,iv);

	pblock2->GetValue(lag_sway,t,dampening,iv);

	pblock2->GetValue(lag_paint_strength,t,PaintStrength,iv);
	pblock2->GetValue(lag_paint_radius,t,Radius,iv);
	pblock2->GetValue(lag_paint_feather,t,Feather,iv);

	pblock2->GetValue(lag_solver,t,solver,iv);

	int samples;
	pblock2->GetValue(lag_samples,t,samples,iv);

	pblock2->GetValue(lag_chase,t,useChase,iv);
	pblock2->GetValue(lag_center,t,useCenter,iv);

	BOOL endFrameOn;
	int endFrame;
	pblock2->GetValue(lag_endframeon,t,endFrameOn,iv);
	pblock2->GetValue(lag_endframe,t,endFrame,iv);


	pblock2->GetValue(lag_hold_radius,t,holdRadius,iv);
	pblock2->GetValue(lag_add_mode,t,addMode,iv);

	pblock2->GetValue(lag_holdlength,t,holdLength,iv);
	pblock2->GetValue(lag_holdlengthpercent,t,holdLengthPercent,iv);
	holdLengthPercent *= 0.01f;

	pblock2->GetValue(lag_lazyeval,t,lazyEval,iv);


	pblock2->GetValue(lag_enable_advance_springs,t,enableAdvanceSprings,iv);
	pblock2->GetValue(lag_stretch,t,stretch,iv);
	pblock2->GetValue(lag_stiffness,t,stiffness,iv);
	stretch = stretch * 0.1f;	
	stiffness = stiffness *0.1f;


	pblock2->GetValue(lag_affectall,t,affectAll,iv);


	if (endFrameOn)
		{
		if (t > (endFrame * GetTicksPerFrame()))
			{
			os->obj->UpdateValidity(GEOM_CHAN_NUM, GetValidity(t));

			return;
			}
		}


	strength = strength/100.0f;
	dampening = dampening/100.0f;

	pblock2->GetValue(lag_referenceframe,t,ReferenceFrame,iv);
	ReferenceFrame = ReferenceFrame * GetTicksPerFrame();


//check if SpringList is zero or does not match current vertex count nuke are and rebuild

	if (lmd != NULL)
		{
		}


	if ((lmd != NULL) && (lmd->SelfNode != NULL))
		{

		if (TestAFlag(A_RENDER)) 
			{
			if (lmd->nukeRenderCache)
				{
				lmd->lastFrame = 999999999;
				lmd->nukeRenderCache = FALSE;
				}
			}
		else
			{
			lmd->nukeRenderCache = TRUE;
			}

		if (lmd->addSprings)
			{
			AddSprings(lmd,os->obj);
			lmd->addSprings = FALSE;
			suggested = -1.0f;
			}

		if (lmd->removeSprings)
			{
			RemoveSprings(lmd);
			lmd->removeSprings = FALSE;
			suggested = -1.0f;
			}
		if (lmd->computeEdges)
			{
			ComputeEdgeLength(lmd,os->obj);
			lmd->computeEdges = FALSE;
			suggested = -1.0f;
			}
		if (lmd->simpleSoft)
			{
			CreateSimpleSoft(lmd,os->obj);
			lmd->simpleSoft = FALSE;
			suggested = -1.0f;
			}

		
		if ( (lmd->edgeSprings.Count() > 0) )
			{
			for (int j=0; j<lmd->edgeSprings.Count();j++)
				{
				int a = lmd->edgeSprings[j].v1;	
				int b = lmd->edgeSprings[j].v2;	
				if ((a >= nv ) || (b >=nv) || (a<0) || (b<0))
					{
					lmd->edgeSprings.Delete(j,1);
					j--;
					}
				}

			if (suggested == -1.0f)
				{
		 		Tab<int> springsPerVert;
				springsPerVert.SetCount(nv);
				for (int j=0;j<nv;j++)
					springsPerVert[j] = 0;


				for (j=0; j<lmd->edgeSprings.Count();j++)
					{
					int a = lmd->edgeSprings[j].v1;	
					int b = lmd->edgeSprings[j].v2;	
					springsPerVert[a] += 1;
					springsPerVert[b] += 1;
					}
				int largest = 0;
				for (j=0; j<nv;j++)
					{
					if (springsPerVert[j] > largest) largest = springsPerVert[j];
					}
				TSTR maxStr;
				float f = 0.0f;
				if (largest > 0)
					{
					f = 1.0f/(float)largest;
					maxStr.printf("%s %0.3f",GetString(IDS_SUGGESTED),f);
					}
				if (ip)
					{
					SendMessage(GetDlgItem(hAdvanceSprings, IDC_SUGEST), WM_SETTEXT, 0, (LPARAM)(char *)maxStr);
					maxStr.printf("%s %d(%d)",GetString(IDS_SPRINGCOUNT),lmd->edgeSprings.Count(),largest);
					SendMessage(GetDlgItem(hAdvanceSprings, IDC_INFO), WM_SETTEXT, 0, (LPARAM)(char *)maxStr);
					}
				suggested = f;

				}
			}

		if (!enableAdvanceSprings)
			{
			float f = suggested;
			stretch = f * stretch;
			stiffness = f * stiffness ;
//now copy to the other params
			extraStrAndSway[0].strength = stretch;
			extraStrAndSway[0].sway = stretch;

			extraStrAndSway[1].strength = stiffness;
			extraStrAndSway[1].sway = stiffness;
			stiffness = stiffness * stiffness;
			extraStrAndSway[2].strength = stiffness;
			extraStrAndSway[2].sway = stiffness;
			stiffness = stiffness * stiffness;
			extraStrAndSway[3].strength = stiffness;
			extraStrAndSway[3].sway = stiffness;

			stiffness = stiffness * stiffness;
			extraStrAndSway[4].strength = stiffness;
			extraStrAndSway[4].sway = stiffness;

			stiffness = stiffness * stiffness;
			extraStrAndSway[5].strength = stiffness;
			extraStrAndSway[5].sway = stiffness;
//disable ui and macro otherwise get nasty flashing in the UI
			macroRecorder->Disable();

			Interval iv;
			float val;
			if (spinnerDown)
				{
				pblock2->GetValue(lag_stretch_str,t,val,iv);
				if (val != extraStrAndSway[0].strength)
					pblock2->SetValue(lag_stretch_str,t,extraStrAndSway[0].strength);
				pblock2->GetValue(lag_stretch_sway,t,val,iv);
				if (val != extraStrAndSway[0].sway)
					pblock2->SetValue(lag_stretch_sway,t,extraStrAndSway[0].sway);

				pblock2->GetValue(lag_torque_str,t,val,iv);
				if (val != extraStrAndSway[1].strength)
					pblock2->SetValue(lag_torque_str,t,extraStrAndSway[1].strength);
				pblock2->GetValue(lag_torque_sway,t,val,iv);
				if (val != extraStrAndSway[1].sway)
					pblock2->SetValue(lag_torque_sway,t,extraStrAndSway[1].sway);

				pblock2->GetValue(lag_extra_str,t,val,iv,0);
				if (val != extraStrAndSway[2].strength)
					pblock2->SetValue(lag_extra_str,t,extraStrAndSway[2].strength,0);
				pblock2->GetValue(lag_extra_sway,t,val,iv,0);
				if (val != extraStrAndSway[2].sway)
					pblock2->SetValue(lag_extra_sway,t,extraStrAndSway[2].sway,0);

				pblock2->GetValue(lag_extra_str,t,val,iv,1);
				if (val != extraStrAndSway[3].strength)
					pblock2->SetValue(lag_extra_str,t,extraStrAndSway[3].strength,1);
				pblock2->GetValue(lag_extra_sway,t,val,iv,1);
				if (val != extraStrAndSway[3].sway)
					pblock2->SetValue(lag_extra_sway,t,extraStrAndSway[3].sway,1);

				pblock2->GetValue(lag_extra_str,t,val,iv,2);
				if (val != extraStrAndSway[4].strength)
					pblock2->SetValue(lag_extra_str,t,extraStrAndSway[4].strength,2);
				pblock2->GetValue(lag_extra_sway,t,val,iv,2);
				if (val != extraStrAndSway[4].sway)
					pblock2->SetValue(lag_extra_sway,t,extraStrAndSway[4].sway,2);

				pblock2->GetValue(lag_extra_str,t,val,iv,3);
				if (val != extraStrAndSway[5].strength)
					pblock2->SetValue(lag_extra_str,t,extraStrAndSway[5].strength,3);
				pblock2->GetValue(lag_extra_sway,t,val,iv,3);
				if (val != extraStrAndSway[5].sway)
					pblock2->SetValue(lag_extra_sway,t,extraStrAndSway[5].sway,3);
				}

			macroRecorder->Enable();
			
			}
		else
			{
			pblock2->GetValue(lag_stretch_str,t,extraStrAndSway[0].strength,iv);
			pblock2->GetValue(lag_stretch_sway,t,extraStrAndSway[0].sway,iv);

			pblock2->GetValue(lag_torque_str,t,extraStrAndSway[1].strength,iv);
			pblock2->GetValue(lag_torque_sway,t,extraStrAndSway[1].sway,iv);

			for (int i=0; i < 10;i++)
				{
				pblock2->GetValue(lag_extra_str,t,extraStrAndSway[i+2].strength,iv,i);
				pblock2->GetValue(lag_extra_sway,t,extraStrAndSway[i+2].sway,iv,i);
				}
			}





		if (lmd->esel.Count() != nv)
			{
			lmd->esel.ZeroCount();
			lmd->psel.ZeroCount();
			lmd->wsel.ZeroCount();
			lmd->esel.SetCount(nv);
			lmd->psel.SetCount(nv);
			lmd->wsel.SetCount(nv);

			for (int esel_index = 0; esel_index < nv; esel_index++)
				{
				lmd->esel[esel_index] = 0;
				lmd->psel[esel_index] = 0;
				lmd->wsel[esel_index] = 0;

				}
			}

		Point3 v(0.0f,0.0f,0.0f);
		lmd->InverseTM = Inverse(lmd->SelfNode->GetObjectTM(t));
		float Dist =0.0f;

		int ect = 0;
		for (int i = 0; i< lmd->esel.Count();i++)
			{
			if (lmd->esel[i]==1)
				ect++;
			}


		if ((nv == ect) || (updating)) 
			{
			if (updating)
				{
				if (lmd->pointCache.Count() != os->obj->NumPoints())
					lmd->pointCache.SetCount(os->obj->NumPoints());
				for (int i = 0; i < lmd->pointCache.Count(); i++)
					lmd->pointCache[i] = os->obj->GetPoint(i);
				}
			os->obj->UpdateValidity(GEOM_CHAN_NUM, GetValidity(t));
//			os->obj->UpdateValidity(TOPO_CHAN_NUM, LocalValidity(t)); // Have to do this to get it to evaluate
			return;
			}

//		if ((TestAFlag(A_RENDER)) && (aboutToRender))
//			{
//			lmd->lastFrame = 999999999;
//			}



		subSelection = FALSE;
		if (os->obj->GetSubselState() != 0)
			subSelection = TRUE;

		BOOL rebuildSystem = FALSE;
		
		if (lmd->pointCache.Count() != os->obj->NumPoints())
			rebuildSystem = TRUE;
	
		if ((!rebuildSystem) && (t==lmd->lastFrame))
			{
			for (int i = 0; i < lmd->pointCache.Count(); i++)
				{
				if (lmd->pointCache[i] != os->obj->GetPoint(i))
					{
					rebuildSystem = TRUE;
					i = lmd->pointCache.Count();
					}
				}
			}

		Point3 tCenter;
		p1->GetValue(ReferenceFrame,&tCenter,iv,CTRL_ABSOLUTE);
		if (tCenter != lagCenter)
			updateWeights = TRUE;


		if ( (t==ReferenceFrame) || (lmd->SpringList.Count() == 0) || (lmd->SpringList.Count() != nv) || updateWeights)
			{    

			Matrix3 tm = lmd->SelfNode->GetObjectTM(ReferenceFrame);
			p1->GetValue(ReferenceFrame,&Center,iv,CTRL_ABSOLUTE);
			lagCenter = Center;

			updateWeights = FALSE;

//build a temp edge list here 
			Tab<Point3> EdgeList;
			EdgeList.Append(1,&Center,1);

			Tab<int> EdgeIndex;
			EdgeIndex.SetCount(nv);

			for (int i = 0; i< lmd->esel.Count();i++)
				{
				if (lmd->esel[i]==1)
					{
					Point3 ep = os->obj->GetPoint(i);
					EdgeList.Append(1,&ep,1);

					}

				}


			for (i=0;i<nv;i++)
				{
//loop through all edges find closest
				float temp = Length(EdgeList[0] - os->obj->GetPoint(i));
				EdgeIndex[i] = 0;
				for (int j = 1; j < EdgeList.Count(); j++)
					{
					float etemp;
					etemp = Length(EdgeList[j] - os->obj->GetPoint(i));
					if (etemp < temp) 
						{
						temp = etemp;
						EdgeIndex[i] = j;
						}
					}
//				temp = Length(Center - os->obj->GetPoint(i));
				if (temp>Dist)
					Dist = temp;
				}
			BOOL nuke = FALSE;
			if (lmd->SpringList.Count() != nv)
				{
				lmd->SpringList.ZeroCount();
				lmd->SpringList.SetCount(nv);
				lmd->WholeFrameCache.SetCount(nv);
				nuke = TRUE;
				}

 
// fix for bug 207988 9/8/99
 			if (lmd->pointCache.Count() != nv)
				lmd->pointCache.SetCount(nv);


			for (i=0;i<nv;i++)
				{
				float temp;
				temp = Length(EdgeList[EdgeIndex[i]] - os->obj->GetPoint(i))/Dist;
				temp = (temp*temp);
				SpringClass sc;
				sc.pos = os->obj->GetPoint(i) * tm;
				sc.init_pos = sc.pos;
				sc.vel = v;
				if ( (nuke) || (!lmd->SpringList[i].modified) )
					{
					sc.InheritVel = temp;// * falloff;
					sc.modified = FALSE;
					}
				else 
					{
					if (lmd->wsel[i] == 1)
						{
						if (absoluteWeight)
							sc.InheritVel = 1.0f-weightDelta;
						else sc.InheritVel = lmd->SpringList[i].InheritVel -weightDelta;
						}
					else sc.InheritVel = lmd->SpringList[i].InheritVel;// * falloff;
					sc.modified = lmd->SpringList[i].modified;
					
					}
				lmd->SpringList[i]=sc;
				lmd->WholeFrameCache[i].pos = sc.pos;
				lmd->WholeFrameCache[i].vel = sc.vel;
// fix for bug 207988 9/8/99
				lmd->pointCache[i] = os->obj->GetPoint(i);
				}
			EdgeList.ZeroCount();
			lmd->lastFrame = ReferenceFrame;
			}

		for (i=0;i < nv; i++)
			{
			lmd->SpringList[i].LocalPt = os->obj->GetPoint(i);

			}
//		for (i=0;i < nv; i++)
//			{
//			DebugPrint(" Lp %f %f %f %d  \n",lmd->SpringList[i].LocalPt.x,lmd->SpringList[i].LocalPt.y,lmd->SpringList[i].LocalPt.z,i);
//
//			}

//		else ComputeCenterFalloff(lmd,t,os);



//for now we will do no cacheing and recompute the system from the ref, frame everytime
//		char ts[200];
//		wsprintf(ts,"nv %d sp %d cach %d pcache %d\n",nv,lmd->SpringList.Count(),lmd->WholeFrameCache.Count(),lmd->pointCache.Count());
//		if ((TestAFlag(A_RENDER)) && (t==ReferenceFrame))
//			MessageBox(NULL,ts,"Error",MB_OK);


		if ( (subSelection) && (!affectAll) )
			{
			if (selectedList.GetSize() != os->obj->NumPoints())
				selectedList.SetSize(os->obj->NumPoints());
			selectedList.ClearAll();
			for (i = 0; i < os->obj->NumPoints(); i++)
				{
				if (os->obj->PointSelection(i)>0.0f)
					selectedList.Set(i);
				}
			}

		if (t>=ReferenceFrame)
			{

			int fract ;
			int frames;
			TimeValue StartFrame;
//			if ( (t<lmd->lastFrame) )

			if ( (t < lmd->lastFrame) && (!TestAFlag(A_RENDER)) && (lazyEval))
				{
//this is a cheat for viewport display, since we have the cache for the previous frame if the user 
//steps back one frame use that for the display instead of recomputing.
//this also helps with systems that evaluate a frame back to get interpolated motion such as some particles
				LastFrameLagDeformer deformer(this,lmd);
				if (affectAll)
					{
					for (i = 0; i < os->obj->NumPoints(); i++)
						{
						Point3 p = os->obj->GetPoint(i);
						p = deformer.Map(i,p);
						os->obj->SetPoint(i,p);
						}
					}
				else os->obj->Deform(&deformer, TRUE);

				os->obj->UpdateValidity(GEOM_CHAN_NUM, GetValidity(t));
				return;
				}
			else if ( (t<lmd->lastFrame) ||
				 ( (t==lmd->lastFrame) && (rebuildSystem) )
				)
				{

				fract = (t) % tps;
				frames = t /tps;
				StartFrame = ReferenceFrame;
				for (int i=0;i<nv;i++)
					{
					Point3 v(0.0f,0.0f,0.0f);
					lmd->SpringList[i].pos = lmd->SpringList[i].init_pos;
					lmd->SpringList[i].vel = v;
					}
				}
			else
				{
				fract = t %tps;
				StartFrame = lmd->lastFrame;
				frames = t /tps;
				for (int i=0;i<nv;i++)
					{
					lmd->SpringList[i].pos = lmd->WholeFrameCache[i].pos;
					lmd->SpringList[i].vel = lmd->WholeFrameCache[i].vel;
					}

				}
			if (samples < 1) samples = 1;
			int sampleInc = tps/samples;

				

			for (i = StartFrame; i < (frames*tps); i+=tps)
				{
				SHORT iret = GetAsyncKeyState (VK_ESCAPE);
				if (iret==-32767)
					{
					i = (frames*tps);
					}

				TimeValue sampleTime = i;
				for (int k =0; k < samples; k++)
					{
					sampleTime += sampleInc;
					if (sampleTime <= t)
						{
						Evaluate(lmd, sampleTime,t, nv, os,samples,1.0f);
						}
					}

				}
//copy that last whole frame to cache
			for (i=0;i<nv;i++)
				{
				lmd->WholeFrameCache[i].pos = lmd->SpringList[i].pos;
				lmd->WholeFrameCache[i].vel = lmd->SpringList[i].vel;
				}
			lmd->lastFrame = frames*tps;
			
//compute fraction incase of motion blur, fields, or realtime playback
			if (lmd->lastFrame != t)
				{
				TimeValue sampleTime = lmd->lastFrame;
				for (int k =0; k < samples; k++)
					{
					sampleTime += sampleInc;
					if (sampleTime <= t)
						{
						Evaluate(lmd, sampleTime,t, nv, os,samples,1.0f);

						if (sampleTime == t)
							{
							k = samples;
							}
						}
//we need to compute the fraction of time
					else 
						{
						sampleTime -= sampleInc;
						float per = (float)(t-sampleTime)/sampleInc;
						Evaluate(lmd, sampleTime,t, nv, os,samples,per);

						k = samples;
						}
					}

				}

			}

		if (lmd->pointCache.Count() != os->obj->NumPoints())
			lmd->pointCache.SetCount(os->obj->NumPoints());
		for (i = 0; i < lmd->pointCache.Count(); i++)
			lmd->pointCache[i] = os->obj->GetPoint(i);
  
		if (t>=ReferenceFrame)
			{
			LagDeformer deformer(this,lmd);
			if (affectAll)

				{
				for (i = 0; i < os->obj->NumPoints(); i++)
					{
					Point3 p = os->obj->GetPoint(i);
					p = deformer.Map(i,p);
					os->obj->SetPoint(i,p);
					}
				
				}
			else 
				{
				os->obj->Deform(&deformer, TRUE);
				}
			}
		

		if ((inPaint) && (!painting))
			{
			if (os->obj->IsSubClassOf(triObjectClassID))
				{
				lmd->isMesh = TRUE;
				lmd->isPatch = FALSE;
				}
#ifndef NO_PATCHES
			else if (os->obj->IsSubClassOf(patchObjectClassID))
				{
				lmd->isMesh = FALSE;
				lmd->isPatch = TRUE;
				}
#endif

			else 

				{
//ask if can convert to mesh
				if (os->obj->CanConvertToType(triObjectClassID))
					{
					lmd->isMesh = TRUE;
					lmd->isPatch = FALSE;
					}
				else
					{
					lmd->isMesh = FALSE;
					lmd->isPatch = FALSE;
					}

				}
			}
		else if (!inPaint)
			{
			}
		}

	os->obj->UpdateValidity(GEOM_CHAN_NUM, GetValidity(t));
#ifndef NO_PATCHES
	if (os->obj->IsSubClassOf(patchObjectClassID))
		{
		PatchObject *pobj = (PatchObject*)os->obj;
		pobj->patch.computeInteriors();
		}
#endif // NO_PATCHES
//	os->obj->UpdateValidity(TOPO_CHAN_NUM, LocalValidity(t)); // Have to do this to get it to evaluate


	}



#define ID_CHUNK 0x1000
#define NODE_CHUNK 0x1010
#define EDGESPRING_COUNT_CHUNK 0x1020
#define EDGESPRING_DATA_CHUNK 0x1030

IOResult LagMod::SaveLocalData(ISave *isave, LocalModData *pld)
{
LagModData *p;
IOResult	res;
ULONG		nb;

p = (LagModData*)pld;

isave->BeginChunk(ID_CHUNK);
res = isave->Write(&p->id, sizeof(int), &nb);
isave->EndChunk();

ULONG id = isave->GetRefID(p->SelfNode);

isave->BeginChunk(NODE_CHUNK);
isave->Write(&id,sizeof(ULONG),&nb);
isave->EndChunk();



TempStorageClass tsc;
int ct = p->SpringList.Count();

isave->BeginChunk(POS_WEIGHT_CHUNK);
isave->Write(&ct, sizeof(ct), &nb);
for (int i =0;i<ct;i++)
		{
		tsc.pos = p->SpringList[i].init_pos;
		tsc.IVel = p->SpringList[i].InheritVel;
		tsc.sel = p->esel[i];
		tsc.modified = p->SpringList[i].modified;
		isave->Write(&tsc, sizeof(tsc), &nb);
		}

isave->EndChunk();

ct = p->edgeSprings.Count();
if (ct>0)
	{
	isave->BeginChunk(EDGESPRING_COUNT_CHUNK);
	isave->Write(&ct,sizeof(ct),&nb);
	isave->EndChunk();

	isave->BeginChunk(EDGESPRING_DATA_CHUNK);
	isave->Write(p->edgeSprings.Addr(0),sizeof(EdgeBondage)*ct,&nb);
	isave->EndChunk();
}


return IO_OK;
}

IOResult LagMod::LoadLocalData(ILoad *iload, LocalModData **pld)

{
	IOResult	res;
	ULONG		nb;
	LagModData *p= new LagModData();
	*pld = p;

//	lmdData.Append(1,&p,1);

	int id;
	int ct =0;
//	INode *n;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {

			case EDGESPRING_COUNT_CHUNK:
				iload->Read(&id,sizeof(int), &nb);
				p->edgeSprings.SetCount(id);
				break;
			case EDGESPRING_DATA_CHUNK:
				ct = p->edgeSprings.Count();
				iload->Read(p->edgeSprings.Addr(0),sizeof(EdgeBondage)*ct, &nb);
				
				break;


			case ID_CHUNK:
				iload->Read(&id,sizeof(int), &nb);
				p->id = id;
				break;
			case NODE_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&p->SelfNode);
					}
				break;

			case POS_WEIGHT_CHUNK:
				{
				Point3 Zero(0.0f,0.0f,0.0f);
			
//read in data
				int ct;
				iload->Read(&ct, sizeof(ct), &nb);
				p->SpringList.SetCount(ct);
				p->psel.ZeroCount();
				p->WholeFrameCache.SetCount(ct);
				p->esel.SetCount(ct);
				p->psel.SetCount(ct);
				p->wsel.SetCount(ct);
				for (int i=0;i<ct;i++)
					{
					TempStorageClass tsc;

					iload->Read(&tsc, sizeof(tsc), &nb);

					p->SpringList[i].pos = tsc.pos;
					p->SpringList[i].init_pos = tsc.pos;
					p->SpringList[i].vel = Zero;
					p->SpringList[i].InheritVel = tsc.IVel;
					p->SpringList[i].modified = tsc.modified;
					p->esel[i] = tsc.sel;
					p->psel[i] = 0;
					p->wsel[i] = 0;
				
					}
				break;

				}


			}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
		}



return IO_OK;

}

int LagMod::NumSubObjTypes() 
{ 
	return 3;
}

ISubObjType *LagMod::GetSubObjType(int i) 
{

	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Points.SetName(GetString(IDS_RB_LAGPOINTS));
		SOT_Edges.SetName(GetString(IDS_RB_LAG_EDGE));
		SOT_Weights.SetName(GetString(IDS_LAG_WEIGHTS));
	}

	switch(i)
	{
	case 0:
		return &SOT_Points;
	case 1:
		return &SOT_Edges;
	case 2:
		return &SOT_Weights;
	}
	return NULL;
}






static INT_PTR CALLBACK VertexInfluenceDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	static ISpinnerControl *spinRadius;

	LagMod *mod = (LagMod*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			{
		    mod = (LagMod *)lParam;
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			
			spinRadius = SetupFloatSpinner(
				hWnd,IDC_LAG_VERTEXINLFUENCE_SPIN,IDC_LAG_VERTEXINLFUENCE,
				0.0f,10000.0f,mod->tholdRadius);		



			break;

			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDOK:
					{
//get radius and addmode


					mod->tholdRadius = spinRadius->GetFVal();
					ReleaseISpinner(spinRadius);

					EndDialog(hWnd,1);
					break;
					}
				}
			break;

		
		case WM_CLOSE:
			EndDialog(hWnd, 0);
			break;

	
		default:
			return FALSE;
		}
	return TRUE;
	}


void LagMod::ComputeEdgeLength(LagModData *lmd,Object *obj)
{
	if (obj->IsSubClassOf(triObjectClassID))
		{
//if it is a mesh
		TriObject *tobj = (TriObject*)obj;
		Mesh &mesh = tobj->GetMesh();
		int ct = 0;
		edgeLength = 0;
		for (int i =0; i < mesh.numFaces; i++)
			{
			Face f = mesh.faces[i];
			for (int j=0; j < 3; j++)
				{
				int a = f.v[j];
				int b ;
				if (j == 2)
					b = f.v[0];
				else b = f.v[j+1];
				if (f.getEdgeVis(j))
					{
					if ( (a>=0) && (a<lmd->SpringList.Count()) &&
						(b>=0) && (b<lmd->SpringList.Count()) )
						{
						Point3 pa = mesh.verts[a];
						Point3 pb = mesh.verts[b];
						float dist = Length(pa -pb);
						edgeLength += dist;
						if (maxEdgeLength < 0.0f)
							{
							maxEdgeLength = dist;
							}
						if (minEdgeLength < 0.0f)
							{
							minEdgeLength = dist;
							}
						ct++;
						if (dist > maxEdgeLength )
							{
							maxEdgeLength = dist;
							}
							if (dist < minEdgeLength )
							{
							minEdgeLength = dist;
							}
						}

					}

				}
			}
		if (ct >0) edgeLength = edgeLength/(float)ct;

		}
#ifndef NO_PATCHES
	else if (obj->IsSubClassOf(patchObjectClassID))
		{
		PatchObject *pobj = (PatchObject*)obj;
		PatchMesh &mesh = pobj->patch;
		int ct = 0;

		for (int i =0; i < mesh.numEdges; i++)
			{
			PatchEdge edge = mesh.edges[i];
			int a,b,c,d;
			a = edge.v1;
			b = edge.vec12;
			c = edge.vec21;
			d = edge.v2;
			Point3 pa = mesh.verts[a].p;
			Point3 pb = mesh.vecs[a].p;
			Point3 pc = mesh.vecs[a].p;
			Point3 pd = mesh.verts[a].p;
			float dist = 0.0f;
			if (maxEdgeLength < 0.0f)
				{
				maxEdgeLength = dist;
				}
			if (minEdgeLength < 0.0f)
				{
				minEdgeLength = dist;
				}
			dist = Length(pa-pb);
			edgeLength += dist;

			ct++;
			if (dist > maxEdgeLength )
				{
				maxEdgeLength = dist;
				}
			if (dist < minEdgeLength )
				{
				minEdgeLength = dist;
				}
			dist = Length(pb-pc);
			edgeLength += dist;

			ct++;
			if (dist > maxEdgeLength )
				{
				maxEdgeLength = dist;
				}
			if (dist < minEdgeLength )
				{
				minEdgeLength = dist;
				}
			dist = Length(pd-pc);
			edgeLength += dist;

			ct++;
			if (dist > maxEdgeLength )
				{
				maxEdgeLength = dist;
				}
			if (dist < minEdgeLength )
				{
				minEdgeLength = dist;
				}


			}
		if (ct >0) edgeLength = edgeLength/(float)ct;


		}
#endif // NO_PATCHES
	else if (obj->SuperClassID() == SHAPE_CLASS_ID)
		{
//loop through parts
		int nv = obj->NumPoints();
		int ct = 0;
		edgeLength = 0;
		int prev = -1;
		for (int i=0; i < nv; i++)
			{
			if (prev != -1)
				{
				Point3 pa = obj->GetPoint(prev);
				Point3 pb = obj->GetPoint(i);
				float dist = Length(pa -pb);
				edgeLength += dist;
				if (maxEdgeLength < 0.0f)
					{
					maxEdgeLength = dist;
					}
				if (minEdgeLength < 0.0f)
					{
					minEdgeLength = dist;
					}
				ct++;
				if (dist > maxEdgeLength )
					{
					maxEdgeLength = dist;
					}
				if (dist < minEdgeLength )
					{
					minEdgeLength = dist;
					}
				}

			prev = i;
			}
		if (ct >0) edgeLength = edgeLength/(float)ct;
		}

}

void LagMod::AddSprings(LagModData *lmd, Object *obj)
{
if (addMode ==0)
	{
	AddSingleSpringFromSelection(lmd, 0,FALSE);

	}
else if ((addMode ==1) || (addMode ==2))
	{
//add based on edge lengths
	if (obj->IsSubClassOf(triObjectClassID))
		{
//if it is a mesh
		TriObject *tobj = (TriObject*)obj;
		Mesh &mesh = tobj->GetMesh();
		for (int i =0; i < mesh.numFaces; i++)
			{
			Face f = mesh.faces[i];
			for (int j=0; j < 3; j++)
				{
				int a = f.v[j];
				int b ;
				if (j == 2)
					b = f.v[0];
				else b = f.v[j+1];
				if (f.getEdgeVis(j))
					{
					if (addMode ==2)
						{
						if ( (lmd->wsel[a]==1) && (lmd->wsel[b]==1))
							AddSpring(lmd, a, b, 0,FALSE);
						}
					else
						{
						if ( (lmd->wsel[a]==1) || (lmd->wsel[b]==1))
							AddSpring(lmd, a, b, 0,FALSE);

						}
					}

				}
			SHORT iret = GetAsyncKeyState (VK_ESCAPE);
			if (iret==-32767)
				{
				i = mesh.numFaces;
				}

			}
		}
#ifndef NO_PATCHES
	else if (obj->IsSubClassOf(patchObjectClassID))
		{
		PatchObject *pobj = (PatchObject*)obj;
		PatchMesh &mesh = pobj->patch;
		for (int i =0; i < mesh.numEdges; i++)
			{
			PatchEdge edge = mesh.edges[i];
			int a,b,c,d;
			a = edge.v1;
			b = mesh.numVerts+edge.vec12;
			c = mesh.numVerts+edge.vec21;
			d = edge.v2;
			if (addMode ==2)
				{
				if ( (lmd->wsel[a]==1) && (lmd->wsel[b]==1))
					{
					AddSpring(lmd, a, b, 0,FALSE);
					}
				if ( (lmd->wsel[b]==1) && (lmd->wsel[c]==1))
					{
					AddSpring(lmd, b, c, 0,FALSE);
					}
				if ( (lmd->wsel[c]==1) && (lmd->wsel[d]==1))
					{
					AddSpring(lmd, c, d, 0,FALSE);
					}
				}
			else
				{
				if ( (lmd->wsel[a]==1) || (lmd->wsel[b]==1))
					{
					AddSpring(lmd, a, b, 0,FALSE);
					}
				if ( (lmd->wsel[b]==1) || (lmd->wsel[c]==1))
					{
					AddSpring(lmd, b, c, 0,FALSE);
					}
				if ( (lmd->wsel[c]==1) || (lmd->wsel[d]==1))
					{
					AddSpring(lmd, c, d, 0,FALSE);
					}
				}
			SHORT iret = GetAsyncKeyState (VK_ESCAPE);
			if (iret==-32767)
				{
				i = mesh.numEdges;
				}


			}

		}	
#endif // NO_PATCHES
	else if (obj->SuperClassID() == SHAPE_CLASS_ID)
		{

//loop through parts
		int prev = -1;
		for (int i =0; i < lmd->wsel.Count(); i++)
			{
			if (lmd->wsel[i]==1)
				{
				if (prev != -1)
					{
					AddSpring(lmd, prev, i, 0,FALSE);
					}
				prev = i;
				}	
			}

		}

	}
else if ((addMode ==3) || (addMode ==4))
	{
//add based on closeness
	for (int i =0; i < lmd->wsel.Count(); i++)
		{
		if (lmd->wsel[i]==1)
			{
			Point3 center = lmd->SpringList[i].LocalPt;
			for (int j =0; j < lmd->SpringList.Count(); j++)
				{


				if ((lmd->wsel[j]==1) || (addMode ==3))
					{
					Point3 a = lmd->SpringList[j].LocalPt;
					float dist = Length(center -a);
					if (dist <= holdRadius)
						{
#ifndef NO_PATCHES
						if 	(obj->IsSubClassOf(patchObjectClassID))
							{
							PatchObject *pobj = (PatchObject*)obj;
							PatchMesh &mesh = pobj->patch;
							if ((i < mesh.numVerts) && (j < mesh.numVerts))
								AddSpring(lmd, i, j, 1,FALSE);
							
							}
						else
#endif // NO_PATCHES
							AddSpring(lmd, i, j, 1,FALSE);
						}
					}
				SHORT iret = GetAsyncKeyState (VK_ESCAPE);
				if (iret==-32767)
					{
					i = lmd->SpringList.Count();
					j = lmd->wsel.Count();
					}

				}

			}
		}
	}

}

void LagMod::RemoveSprings(LagModData *lmd)
{

for (int i =0; i < lmd->edgeSprings.Count(); i++)
		{
		int a,b;
		a = lmd->edgeSprings[i].v1;
		b = lmd->edgeSprings[i].v2;
		if ( (lmd->wsel[a]==1) &&  (lmd->wsel[b]==1))
			{
			lmd->edgeSprings.Delete(i,1);
			i--;
			}
		}


}

void LagMod::FilterPass(LagModData *lmd,int numberPasses, float stretchPercentage, float per)
{
//gather up springs that have only one edge selected
BitArray visitedList;

visitedList.SetSize(lmd->SpringList.Count());
visitedList.ClearAll();

BitArray visitedSpringList;
visitedSpringList.SetSize(lmd->edgeSprings.Count());
visitedSpringList.ClearAll();

//maybe use edge sel
BitArray edgeSel;
edgeSel.SetSize(lmd->SpringList.Count());
edgeSel.ClearAll();
for (int i = 0; i < lmd->esel.Count(); i++)
	{	
	if (lmd->esel[i]) edgeSel.Set(i);
	}

Tab<int> startList;
for (i = 0; i < lmd->edgeSprings.Count(); i++)
	{
	int a = lmd->edgeSprings[i].v1;
	int b = lmd->edgeSprings[i].v2;
	if ( (lmd->esel[a] && !lmd->esel[b]) ||
		 (lmd->esel[b] && !lmd->esel[a]) )
		{
		startList.Append(1,&i);
		}
	}
while (startList.Count() != 0)
	{
	for (i = 0; i < startList.Count(); i++)
		{
		int dir;
		int a = lmd->edgeSprings[startList[i]].v1;
		int b = lmd->edgeSprings[startList[i]].v2;
		if (edgeSel[a])
			dir =0;
		else if (edgeSel[b])
			dir =1;
		if (dir==1)
			{
			int temp = a;
			a = b;
			b = temp;
			}

		Point3 pa = lmd->SpringList[a].pos +  lmd->SpringList[a].vel*per;
		Point3 pb = lmd->SpringList[b].pos +  lmd->SpringList[b].vel*per;
		float vecDist = Length(pa-pb);
		float dist = vecDist - lmd->edgeSprings[startList[i]].dist;
		float initialDist = lmd->edgeSprings[startList[i]].dist;

		float changePer = fabs(dist/initialDist);
		if (changePer > stretchPercentage)
			{
//DebugPrint("initial length %f newLength %f\n", initialDist, vecDist);

//apply a negative force to 
			Point3 force = Normalize(pa-pb);
//			float amount = (dist/lmd->edgeSprings[startList[i]].dist);
//			float newDist;
//			if (amount > 0.0f) newDist= dist + dist*stretchPercentage;
//			else newDist= dist - dist*stretchPercentage;
//			newDist = newDist-pb;
//			lmd->SpringList[b].vel += force*newDist;
			float newDist = 0.f;
			if (dist< 0.0f)
				{
				float tdist = initialDist * stretchPercentage;
				newDist = -(fabs(dist)-tdist);
				}
			else
				{
				{
				float tdist = initialDist * stretchPercentage;
				newDist = (fabs(dist)-tdist);
				}
				}
			lmd->SpringList[b].vel += (force*newDist)/per;

	
			Point3 ta = lmd->SpringList[a].pos +  lmd->SpringList[a].vel*per;
			Point3 tb = lmd->SpringList[b].pos +  lmd->SpringList[b].vel*per;
			float tvecDist = Length(ta-tb);

			}
		visitedList.Set(a);
		visitedList.Set(b);
		visitedSpringList.Set(startList[i]);
		}
	for (i = 0; i < startList.Count(); i++)
		{
		int a = lmd->edgeSprings[startList[i]].v1;
		int b = lmd->edgeSprings[startList[i]].v2;
		edgeSel.Set(a);
		edgeSel.Set(b);

		}

//now rebuild startlist
	Tab<int> tempList;
	tempList = startList;
	startList.ZeroCount();
	for (i = 0; i < lmd->edgeSprings.Count(); i++)
		{
		int a = lmd->edgeSprings[i].v1;
		int b = lmd->edgeSprings[i].v2;
		if ( (visitedList[a]==FALSE)  || (visitedList[b]==FALSE)) 
			{
			for (int j=0; j < tempList.Count(); j++)
				{
				if ( (a==lmd->edgeSprings[tempList[j]].v1) || (a==lmd->edgeSprings[tempList[j]].v2) ||
					 (b==lmd->edgeSprings[tempList[j]].v1) || (b==lmd->edgeSprings[tempList[j]].v2) )
					{
					startList.Append(1,&i);
					j = tempList.Count();
					}
				}
			}
		}
	}



}



DWORD WINAPI dummy(LPVOID arg) {
	return(0);
}


void LagMod::CreateSimpleSoft(LagModData *lmd, Object *obj)
{

SetCursor(LoadCursor(NULL,IDC_WAIT));

lmd->edgeSprings.ZeroCount();
//add edge lengths in first
//add based on edge lengths
Tab<float> dists;
dists.SetCount(obj->NumPoints());
Tab<int> count;
count.SetCount(obj->NumPoints());

occuppiedList = new BitArray[obj->NumPoints()];

int oct = obj->NumPoints();
for (int k = 0; k < obj->NumPoints(); k++)
	{
	dists[k]=0.0f;
	count[k]=0;
	occuppiedList[k].SetSize(oct+1);
	occuppiedList[k].ClearAll();
	oct--;
	}
BOOL userDefined = FALSE;
if (obj->IsSubClassOf(triObjectClassID))
	{
//if it is a mesh
	TriObject *tobj = (TriObject*)obj;
	Mesh &mesh = tobj->GetMesh();
	AdjEdgeList eList(mesh);

	for (int i =0; i < eList.edges.Count(); i++)
		{
		int a = eList.edges[i].v[0];
		int b = eList.edges[i].v[1];
//check if sub selection one vert needs to be selected
		if (subSelection)
			{
//			if ((obj->PointSelection(a)>0.0f) || (obj->PointSelection(b)>0.0f))
			if (selectedList[a] || selectedList[b])
				AddSpring(lmd, a, b, 0,FALSE);
			}
		else AddSpring(lmd, a, b, 0,FALSE);
			
		if (!eList.edges[i].Visible(mesh.faces))
			{
			int faceA, faceB;
			faceA = eList.edges[i].f[0];
			faceB = eList.edges[i].f[1];
//find opposing edge
			int aSel = eList.edges[i].EdgeIndex(mesh.faces,0);
			int bSel = eList.edges[i].EdgeIndex(mesh.faces,1);
			int aEdges[2];
			int bEdges[2];
			int aOpposing, bOpposing;
			if (aSel == 0)
				{
				aOpposing = 2;
				aEdges[0] = 1;
				aEdges[1] = 2;
				}
			else if (aSel == 1)
				{
				aOpposing = 0;
				aEdges[0] = 2;
				aEdges[1] = 0;
				}
			else
				{
				aOpposing = 1;
				aEdges[0] = 0;
				aEdges[1] = 1;
				}
			if (bSel == 0)
				{
				bOpposing = 2;
				bEdges[0] = 1;
				bEdges[1] = 2;
				}
			else if (bSel == 1)
				{
				bOpposing = 0;
				bEdges[0] = 2;
				bEdges[1] = 0;
				}
			else
				{
				bOpposing = 1;
				bEdges[0] = 0;
				bEdges[1] = 1;
				}
			if ( (mesh.faces[faceA].getEdgeVis(aEdges[0])) && (mesh.faces[faceA].getEdgeVis(aEdges[1])) &&
				 (mesh.faces[faceB].getEdgeVis(bEdges[0])) && (mesh.faces[faceB].getEdgeVis(bEdges[1])) )
				{
				int a,b;
				a = mesh.faces[faceA].v[aOpposing];
				b = mesh.faces[faceB].v[bOpposing];
				AddSpring(lmd, a, b, 0,FALSE);
				}
			}

		}
/*
	for (int i =0; i < mesh.numFaces; i++)
		{
		Face f = mesh.faces[i];
		for (int j=0; j < 3; j++)
			{
			int a = f.v[j];
			int b ;
			if (j == 2)
				b = f.v[0];
			else b = f.v[j+1];
			if (f.getEdgeVis(j))
				{
//check if sub selection one vert needs to be selected
				if (subSelection)
					{
					if ((obj->PointSelection(a)>0.0f) || (obj->PointSelection(b)>0.0f))
						AddSpring(lmd, a, b, 0,FALSE);
					}
				else AddSpring(lmd, a, b, 0,FALSE);
				}
			if (!f.getEdgeVis(j))
				{
//lok for crossing face
				}


			}
		SHORT iret = GetAsyncKeyState (VK_ESCAPE);
		if (iret==-32767)
			{
			i = mesh.numFaces;
			}

		}
*/
	}
#ifndef NO_PATCHES
else if (obj->IsSubClassOf(patchObjectClassID))
	{
	PatchObject *pobj = (PatchObject*)obj;
	PatchMesh &mesh = pobj->patch;
	for (int i =0; i < mesh.numEdges; i++)
		{
		PatchEdge edge = mesh.edges[i];
		int a,b,c,d;
		a = edge.v1;
		b = mesh.numVerts+edge.vec12;
		c = mesh.numVerts+edge.vec21;
		d = edge.v2;
//check if sub selection one vert needs to be selected
		if (subSelection)
			{
			if ((obj->PointSelection(a)>0.0f) || (obj->PointSelection(d)>0.0f))
				{
				AddSpring(lmd, a, b, 0,FALSE);
				AddSpring(lmd, b, c, 0,FALSE);
				AddSpring(lmd, c, d, 0,FALSE);
				}
			}
		else 
			{
			AddSpring(lmd, a, b, 0,FALSE);
			AddSpring(lmd, b, c, 0,FALSE);
			AddSpring(lmd, c, d, 0,FALSE);

			}
		SHORT iret = GetAsyncKeyState (VK_ESCAPE);
		if (iret==-32767)
			{
			i = mesh.numEdges;
			}
		}
	}	
#endif // NO_PATCHES
	else if (obj->SuperClassID() == SHAPE_CLASS_ID)
		{

//loop through parts
		BezierShape bShape;
		ShapeObject *shape = (ShapeObject *)obj;
		if (shape)
			{
			if(shape->CanMakeBezier())
				{
				shape->MakeBezier(GetCOREInterface()->GetTime(), bShape);
				if (bShape.GetNumVerts() == lmd->SpringList.Count())
					{
//loop through
					int polyCount = bShape.splineCount;
					int prevPoint = -1;
					for (int i = 0; i < polyCount; i++)
						{
						Spline3D *spl = bShape.splines[i];
						for (int j = 0; j < spl->KnotCount(); j++)
							{
//check if not is smooth or bezier
							SplineKnot k;
							k = spl->GetKnot(j);
							if ((k.Ktype() == KTYPE_BEZIER) || (k.Ktype()== KTYPE_BEZIER))
								{
								int a = j*3;
								int b = j*3+1;
								int c = j*3+2;
								AddSpring(lmd, a, b, 0,FALSE);
								AddSpring(lmd, b, c, 0,FALSE);
								if (prevPoint != -1)
									AddSpring(lmd, a, prevPoint, 0,FALSE);
								prevPoint =j*3 + 2;
								}
							else
								{
								if (prevPoint != - 1)
									{
									int a;
									a = j *3+1;
									AddSpring(lmd, a, prevPoint, 0,FALSE);
									}
								prevPoint =j*3 + 1;
								}
							}

						}
					}
				}
			else
				{
				int ct = obj->NumPoints();
				for (int i = 1; i < ct; i++)
					{
					AddSpring(lmd, i-1, i, 0,FALSE);
					}
				if (shape->CurveClosed(0,0))
					AddSpring(lmd, 0, ct-1, 0,FALSE);


				}
			}
		}
	else userDefined = TRUE;



for (k=0; k < lmd->edgeSprings.Count(); k++)
	{
	Point3 pa,pb;
	int a,b;
	a = lmd->edgeSprings[k].v1;
	b = lmd->edgeSprings[k].v2;

	pa = lmd->SpringList[a].LocalPt;
	pb = lmd->SpringList[b].LocalPt;
	float cdist = Length(pa-pb);
	count[a] = count[a]+1;
	count[b] = count[b]+1;
	dists[a] = dists[a]+cdist;
	dists[b] = dists[b]+cdist;
	}
//if ((count.Count() >0) && (count[0]==0) )

tholdRadius = holdRadius;
if (userDefined )
	{
	int iret = DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_VERTEXINFLUENCE_DIALOG),
					hParams,VertexInfluenceDlgProc,(LPARAM)this);	

	}

for (k=0; k<dists.Count();k++)
	{
	if (count[k]!=0) 
		{
		dists[k] = dists[k]/(float)count[k];
		dists[k] = dists[k]*2.0f;
		}
	else
		{
		dists[k] = tholdRadius/4.0f;
		}
	
	}


int spDepth;
float spMult;

pblock2->GetValue(lag_createspringdepth,0,spDepth,FOREVER);
pblock2->GetValue(lag_createspringmult,0,spMult,FOREVER);






float perDone = 0.0f;

//add based on closeness
for (k = 0; k < spDepth; k++)
	{
	for (int i =0; i < lmd->SpringList.Count(); i++)
		{
		perDone = ( float(k*lmd->SpringList.Count()+i)/float(spDepth*lmd->SpringList.Count()) );
		
		TSTR name;
		name.printf("%d%% %s",(int)(perDone*100.f),GetString(IDS_LAG_PERCANCEL));
		SetWindowText(GetDlgItem(hCreateSimple,IDC_PERSTATIC),name);
				
		if (subSelection)
			{
			if (obj->PointSelection(i)>0.0f) 
				{
				Point3 center = lmd->SpringList[i].LocalPt;
				for (int j =0; j < lmd->SpringList.Count(); j++)
					{
					Point3 a = lmd->SpringList[j].LocalPt;
					float dist = Length(center -a);
					if (dist <= dists[i])
						{
#ifndef NO_PATCHES
						if 	(obj->IsSubClassOf(patchObjectClassID))
							{
							PatchObject *pobj = (PatchObject*)obj;
							PatchMesh &mesh = pobj->patch;
							if ((i < mesh.numVerts) && (j < mesh.numVerts))
								AddSpring(lmd, i, j, (k+1),FALSE);
			
							}
						else
#endif // NO_PATCHES
							AddSpring(lmd, i, j, (k+1),FALSE);
						}
					}
				
				SHORT iret = GetAsyncKeyState (VK_ESCAPE);
				if (iret==-32767) 
					{
					i = lmd->SpringList.Count();
					j = lmd->SpringList.Count(); 
					k = spDepth+1;
					}
				}

			}
		else
			{
			Point3 center = lmd->SpringList[i].LocalPt;
			for (int j =0; j < lmd->SpringList.Count(); j++)
				{
				Point3 a = lmd->SpringList[j].LocalPt;
				float dist = Length(center -a);
				if (dist <= dists[i])
					{
#ifndef NO_PATCHES
					if 	(obj->IsSubClassOf(patchObjectClassID))
						{
						PatchObject *pobj = (PatchObject*)obj;
						PatchMesh &mesh = pobj->patch;
						if ((i < mesh.numVerts) && (j < mesh.numVerts))
							AddSpring(lmd, i, j, k+1,FALSE);
			
						}
					else
#endif // NO_PATCHES
						AddSpring(lmd, i, j, k+1,FALSE);
					}
				}
			SHORT iret = GetAsyncKeyState (VK_ESCAPE);
			if (iret==-32767) 
				{
				i = lmd->SpringList.Count();
				j = lmd->SpringList.Count(); 
				k = spDepth+1;
				}
			}


		}
	for (i=0; i<dists.Count();i++)
		{
		dists[i] = dists[i]*spMult;
		float d = dists[i];
		}

	}

delete [] occuppiedList;
occuppiedList = NULL;

SetCursor(LoadCursor(NULL,IDC_ARROW));

TSTR name;
name.printf(" ");
SetWindowText(GetDlgItem(hCreateSimple,IDC_PERSTATIC),name);

}




