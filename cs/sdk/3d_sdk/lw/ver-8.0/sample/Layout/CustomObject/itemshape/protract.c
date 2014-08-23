// Protract.c - Custom Object Plug-ins
// Arnie Cachelin, Copyright 2001 NewTek, Inc.
// 

#include <lwhost.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwenvel.h>
#include <lwchannel.h>
#include <lwrender.h>
#include <lwcustobj.h>
#include <lwtxtr.h>
#include <lwvparm.h>
#include <lwmath.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include "custdraw.h"

#define VCPY_F(a,b)       ((a)[0] =(float)((b)[0]), (a)[1] =(float)((b)[1]), (a)[2] =(float)((b)[2]))

static	LWXPanelFuncs	*GlobalXPanFun		= NULL;
static	LWMessageFuncs	*Gmessage			= NULL;
static	LWTextureFuncs	*GlobalTextureFuncs	= NULL;
static	LWItemInfo		*GlobalItemInfo		= NULL;
static	LWSceneInfo		*GlobalSceneInfo	= NULL;
static	LWCameraInfo	*GlobalCameraInfo   = NULL;
static	LWInstUpdate	*GlobalLWUpdate		= NULL;
static	LWVParmFuncs    *GlobalVParmFunc	= NULL;
static	float	Red[]		= {0.92f, 0.2f, 0.2f},
				Green[]		= {0.2f, 0.92f, 0.2f},
				Blue[]		= {0.2f, 0.2f, 0.92f},
				Yellow[]	= {0.92f, 0.92f, 0.2f},
				Orange[]	= {0.92f, 0.62f, 0.32f},
				White[]		= {0.92f, 0.92f, 0.92f},
				Black[]		= {0.1f, 0.1f, 0.1f};


static char *errAllocFailed ="Tiny Allocation Failed", *AxisName="XYZ";
static int x_axis[] = {2,2,0}, y_axis[] = {1,0,1}, z_axis[] = {0,1,2};
static char		*shapeList[] = {"Half (180°)", "Full (360°)", NULL};

/* ----------------- Plug-in Instance Definition  -----------------  */

typedef struct st_ProtractData {
	void				*ctxt;
	LWItemID			 self;

	double				 time, scale;
	int					 axis, frame, flags, sense;
	float				clrA[4], clrB[4];

	double				 min, max, mark;
	LWVParmID			 markAngle;
	char				 desc[100];

} ProtractData;

#define PROF_LABEL		1
#define PROF_LIMIT		2
#define PROF_MARK		4
#define PROF_FULL		8

/* ----------------- Plug-in Methods: LWInstanceFuncs  -----------------  */

XCALL_(static LWInstance)ProtractCreate(void *data, LWItemID id, LWError *err)
{
	ProtractData *dat=NULL;
	LWChanGroupID cgroup;
	XCALL_INIT;
	if(dat=malloc(sizeof(ProtractData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->frame = 0;
		dat->axis = 0;
		dat->self = id;
		dat->ctxt = data;
		dat->axis = 2;
		dat->clrA[3] = 1.0f;
		dat->clrB[3] = 1.0f;
		if ( dat->markAngle = GlobalVParmFunc->create( LWVP_ANGLE, LWVPDT_NOTXTR )) 
		{
			cgroup = GlobalItemInfo->chanGroup( dat->self );
			GlobalVParmFunc->setup( dat->markAngle, "ProtractorAngle", cgroup,
												NULL, NULL, NULL, NULL );
		}
	}
	else
		*err = errAllocFailed;
	return dat;
}

XCALL_(static const char *)ProtractDescribe(ProtractData *dat)
{
	XCALL_INIT;
	sprintf(dat->desc," ");
	return (dat->desc);
}

XCALL_(static void)ProtractDestroy(ProtractData *dat)
{
	XCALL_INIT;
	if(dat)
	{
      if ( dat->markAngle )
         GlobalVParmFunc->destroy( dat->markAngle );
		free(dat);
	}
}

XCALL_(static LWError)ProtractCopy(ProtractData	*to, ProtractData	*from)
{
	LWVParmID vpid;
	LWItemID id;
	XCALL_INIT;

	id = to->self;
	vpid = to->markAngle;
	*to = *from;
	to->self = id;
	to->markAngle = vpid;
	return GlobalVParmFunc->copy( to->markAngle, from->markAngle );
}

XCALL_(static LWError)ProtractLoad(ProtractData *dat,const LWLoadState	*lState)
{
	int			v;
	float		fv[3] = {0.0f,0.0f,0.0f};
	XCALL_INIT;
	LWLOAD_I4(lState,&v,1);
	LWLOAD_I4(lState,&dat->axis,1);
	LWLOAD_I4(lState,&dat->sense,1);
	LWLOAD_I4(lState,&dat->flags,1);
	LWLOAD_FP(lState,fv,3);
	dat->min  = fv[0];
	dat->max  = fv[1];
	dat->mark = fv[2];

	return GlobalVParmFunc->load( dat->markAngle, lState ); 
}

XCALL_(LWError)ProtractSave(ProtractData *dat,const LWSaveState	*sState)
{
	int			v = 1;
	float		fv[3];
	XCALL_INIT;
	LWSAVE_I4(sState,&v,1);
	LWSAVE_I4(sState,&dat->axis,1);
	LWSAVE_I4(sState,&dat->sense,1);
	LWSAVE_I4(sState,&dat->flags,1);
	fv[0] = (float)dat->min;
	fv[1] = (float)dat->max;
	fv[2] = (float)dat->mark;
	LWSAVE_FP(sState,fv,3);
	return GlobalVParmFunc->save( dat->markAngle, sState );       
}

/* ----------------- Plug-in Methods: LWRenderFuncs  -----------------  */

XCALL_(static LWError)ProtractInit (LWInstance inst, int i)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static LWError)ProtractNewTime (LWInstance inst, LWFrame f, LWTime t)
{
	ProtractData	*dat = (ProtractData *)inst;
	double			 vec[3] = {0.0};
	XCALL_INIT;
	dat->time = t;
	dat->frame = f;
	GlobalVParmFunc->getVal( dat->markAngle, dat->time, NULL, vec );
	dat->mark = vec[0];

	return NULL;
}

XCALL_(static void)ProtractCleanup (LWInstance inst)
{
	XCALL_INIT;
	return;
}

/* ----------------- Plug-in Methods: LWItemFuncs  -----------------  */

XCALL_(static const LWItemID *)ProtractUseItems (LWInstance inst)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static void)ProtractChangeID (LWInstance inst, const LWItemID *items)
{
	XCALL_INIT;
	return;
}



/* ----------------- Plug-in Methods: LWCustomObjHandler  -----------------  */

XCALL_(static unsigned int)ProtractFlags (ProtractData *dat)
{
	XCALL_INIT;
	return 0;
}	


XCALL_(static void)ProtractEval (ProtractData *dat,	const LWCustomObjAccess *cobjAcc)
{
	double  v, da, dr, orig[3] = {0.0,0.0,0.0}, vec[3], vel[3];
	int		i,ix,iy, t;
	char	buf[20]="";

	XCALL_INIT;	

	ix = x_axis[dat->axis];
	iy = y_axis[dat->axis];

	if(cobjAcc->flags&LWCOFL_SELECTED)
		(*cobjAcc->setColor)(cobjAcc->dispData, dat->clrA);
	else
		(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_DOT);

	VCLR(vel);
	vel[ix] = 1.0;
	t = dat->flags&PROF_FULL ? 72:36; // 5deg. incr.
	da = 1.0/t;
	for(i=0; i<=t; i++)
	{
		VCLR(vec);
		v = i*(dat->flags&PROF_FULL ? TWOPI:PI);
		v *= da;
		vec[ix] += cos(v);
		vec[iy] += sin(v);

		if(cobjAcc->flags&LWCOFL_SELECTED)
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_DOT);
		(*cobjAcc->line)(cobjAcc->dispData, vel,vec,LWCSYS_OBJECT);

		VCPY(vel,vec);
		dr = i&1 ? 0.95:0.90;
		if(!(i%9))
			dr *= 0.8;
		VSCL(vel,dr);
		VADD(vec, orig);
		VADD(vel, orig);
		if(!(i&1) && cobjAcc->flags&LWCOFL_SELECTED)
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_SOLID);
		(*cobjAcc->line)(cobjAcc->dispData, vel,vec,LWCSYS_OBJECT);
		if(dat->flags&PROF_LABEL && !(i%3)  && ( !(dat->flags&PROF_FULL) || i<t ) )
		{
			sprintf(buf,"%d",(i*10)/2);
			(*cobjAcc->text)(cobjAcc->dispData, vec, buf, 1, LWCSYS_OBJECT);
		}
		VCPY(vel,vec);
	}
	(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_SOLID);
	VCPY(vec, orig);
	if(dat->flags&PROF_MARK)
	{
		VCPY(dat->clrB,Yellow);
		if(cobjAcc->flags&LWCOFL_SELECTED)
			(*cobjAcc->setColor)(cobjAcc->dispData, dat->clrB);
		vec[ix] += cos(dat->mark);
		vec[iy] += sin(dat->mark);
		VCPY(vel,vec);
		VSCL(vel,0.75);
		(*cobjAcc->line)(cobjAcc->dispData, vel,vec,LWCSYS_OBJECT);
	}
	if(dat->flags&PROF_LIMIT)
	{
		VCPY(dat->clrB,Red);
		if(cobjAcc->flags&LWCOFL_SELECTED)
			(*cobjAcc->setColor)(cobjAcc->dispData, dat->clrB);
		VCPY(vec, orig);
		vec[ix] += cos(dat->min);
		vec[iy] += sin(dat->min);
		(*cobjAcc->line)(cobjAcc->dispData, orig,vec,LWCSYS_OBJECT);
		VCPY(vec, orig);
		vec[ix] += cos(dat->max);
		vec[iy] += sin(dat->max);
		(*cobjAcc->line)(cobjAcc->dispData, orig,vec,LWCSYS_OBJECT);
		if(dat->max>dat->min)
			co_Arc(cobjAcc, orig,(dat->min),(dat->max), 0.35,LWCSYS_OBJECT, dat->axis );
		else if(dat->max<dat->min)
			co_Arc(cobjAcc, orig,(dat->min),(dat->max+TWOPI), 0.35,LWCSYS_OBJECT, dat->axis );

	}
}

/* -----------------                 -----------------  */


XCALL_(int) Protract (
	long			 	 version,
	GlobalFunc			*global,
	LWCustomObjHandler	*local,
	void				*serverData)
{
	XCALL_INIT;
	if (version != LWCUSTOMOBJ_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) (LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalItemInfo = (*global) (LWITEMINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalItemInfo )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalLWUpdate)
	{
		(*Gmessage->error)("Can't get global",LWINSTUPDATE_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
 	GlobalVParmFunc = (*global) (LWVPARMFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalVParmFunc)
	{
		(*Gmessage->error)("Can't get global",LWVPARMFUNCS_GLOBAL);
		return AFUNC_BADGLOBAL;
	}

	local->inst->create  = ProtractCreate;
	local->inst->destroy = ProtractDestroy;
	local->inst->load    = ProtractLoad;
	local->inst->save    = ProtractSave;
	local->inst->copy    = ProtractCopy;
	local->inst->descln	 = ProtractDescribe;

	local->item->useItems    = ProtractUseItems;
	local->item->changeID    = ProtractChangeID;

	local->rend->init		= ProtractInit;
	local->rend->newTime    = ProtractNewTime;
	local->rend->cleanup	= ProtractCleanup;

	local->evaluate 	= ProtractEval;
	local->flags 		= ProtractFlags;
	return (AFUNC_OK);
}

/* -----------------  User Interface  ----------------- */

enum  {	CH_AXIS = 0x8601, CH_SHAPE, CH_SIZE, CH_LABL, CH_LEVL, CH_FILL, CH_COLR, CH_MARK,
	CH_BCOLR, CH_LIMIT, CH_MIN, CH_MAX,  
		CH_DISPGR, CH_GRP1, CH_GRP2 };
#define			STR_Axis_TEXT		"Axis"
#define			STR_Shape_TEXT		"Shape"
#define			STR_UseMark_TEXT	"Show Mark"
#define			STR_Limits_TEXT		"Show Range"
#define			STR_Color_TEXT		" "
#define			STR_Label_TEXT		"Label"
#define			STR_Angle_TEXT		"Mark Angle"
#define			STR_Min_TEXT		"Min. Angle"
#define			STR_Max_TEXT		"Max. Angle"
#define			STR__TEXT		""

static LWXPanelControl Protract_ctrl_list[] = {
	{ CH_AXIS,			STR_Axis_TEXT,				"axis" },
	{ CH_SHAPE,			STR_Shape_TEXT,				"iPopChoice" },
	{ CH_COLR,			STR_Color_TEXT,				"color" },
	{ CH_LIMIT,			STR_Limits_TEXT,			"iBoolean" },
	{ CH_MIN,			STR_Min_TEXT,				"angle" },
	{ CH_MAX,			STR_Max_TEXT,				"angle" },
	{ CH_MARK,			STR_UseMark_TEXT,			"iBoolean" },
	{ CH_LEVL,			STR_Angle_TEXT,				"angle-env" },
	{ CH_LABL,			STR_Label_TEXT,				"iBoolean" },
	{0}
};
static LWXPanelDataDesc Protract_data_descrip[] = {
	{ CH_AXIS,			STR_Axis_TEXT,				"integer" },
	{ CH_SHAPE,			STR_Shape_TEXT,				"integer" },
	{ CH_COLR,			STR_Color_TEXT,				"color" },
	{ CH_LIMIT,			STR_Limits_TEXT,			"integer" },
	{ CH_MIN,			STR_Min_TEXT,				"angle" },
	{ CH_MAX,			STR_Max_TEXT,				"angle" },
	{ CH_MARK,			STR_UseMark_TEXT,			"integer" },
	{ CH_LEVL,			STR_Angle_TEXT,				"angle-env" },
	{ CH_LABL,			STR_Label_TEXT,				"integer" },
	{0},
};


static void *ProtractData_get ( void *myinst, unsigned long vid ) 
{
  ProtractData *dat = (ProtractData*)myinst;
  void *result = NULL;
  static int val = 0;
  static double rgba[4];

  if ( dat ) 
	  switch ( vid ) {
		case CH_AXIS:
			result = &dat->axis;
			break;
		case CH_SHAPE:
			val = dat->flags&PROF_FULL ? 1 : 0;
			result = &val;
			break;
		case CH_FILL:
			result = &dat->sense;
			break;
		case CH_COLR:
			VCPY(rgba,dat->clrA);
			result = &rgba;
			break;
		case CH_MARK:
			val = dat->flags&PROF_MARK ? 1 : 0;
			result = &val;
			break;
		case CH_LIMIT:
			val = dat->flags&PROF_LIMIT ? 1 : 0;
			result = &val;
			break;
		case CH_LABL:
			val = dat->flags&PROF_LABEL ? 1 : 0;
			result = &val;
			break;
		case CH_MIN:
			result = &dat->min;
			break;
		case CH_MAX:
			result = &dat->max;
			break;
		case CH_LEVL:
			result = dat->markAngle;
			break;
	  } 
  return result;
}

static int ProtractData_set ( void *myinst, unsigned long vid, void *value ) 
{
	ProtractData *dat = (ProtractData*)myinst;
	double rgba[4]={0.0};
	int rc=0;
	if ( dat ) 
		switch ( vid ) {
			case CH_AXIS:
				dat->axis = *((int*)value);
				rc = 1;
				break;
			case CH_COLR:
				VCPY(rgba,((double*)value));
				VCPY_F(dat->clrA, rgba);
				rc = 1;
				break;
			case CH_FILL:
				dat->sense = *((int*)value);
				rc = 1;
				break;
			case CH_SHAPE:
				rc = *((int*)value);
				if(rc)
					dat->flags |= PROF_FULL;
				else
					dat->flags &= ~PROF_FULL;
				rc = 1;
				break;
			case CH_LABL:
				rc = *((int*)value);
				if(rc)
					dat->flags |= PROF_LABEL;
				else
					dat->flags &= ~PROF_LABEL;
				rc = 1;
				break;
			case CH_MARK:
				rc = *((int*)value);
				if(rc)
					dat->flags |= PROF_MARK;
				else
					dat->flags &= ~PROF_MARK;
				rc = 1;
				break;
			case CH_LIMIT:
				rc = *((int*)value);
				if(rc)
					dat->flags |= PROF_LIMIT;
				else
					dat->flags &= ~PROF_LIMIT;
				rc = 1;
				break;
			case CH_MIN:
				dat->min = *((double*)value);
				rc = 1;
				break;
			case CH_MAX:
				dat->max = *((double*)value);
				rc = 1;
				break;
			case CH_LEVL:
				rc = 1;
				break;
		} 
	if(rc)
		(*GlobalLWUpdate)(LWCUSTOMOBJ_HCLASS, dat);
	return rc;
}

static int   levEnable[] = {0,1,0,1,0};		
static LWXPanelID ProtractXPanel(GlobalFunc *global, ProtractData *dat)
{
	LWXPanelFuncs *lwxpf = NULL;
	LWXPanelID     panID = NULL;
	static LWXPanelHint hint[] = {
		XpLABEL(0,"Protractor Object"),
		XpSTRLIST(CH_SHAPE,shapeList),
		XpGROUP_(CH_DISPGR),
			XpH(CH_SHAPE),
			XpH(CH_AXIS),
			XpH(CH_LABL),
			XpH(CH_COLR),
			XpEND,
		XpGROUP_(CH_GRP2),
			XpH(CH_MARK),
			XpH(CH_LEVL),
			XpEND,
		XpGROUP_(CH_GRP1),
			XpH(CH_LIMIT),
			XpH(CH_MIN),
			XpH(CH_MAX),
			XpEND,
		XpENABLE_(CH_MARK),
			XpH(CH_LEVL),
			XpEND,
		XpENABLE_(CH_LIMIT),
			XpH(CH_MIN),
			XpH(CH_MAX),
			XpEND,
		XpNARROW(CH_MARK),
		XpNARROW(CH_LEVL),
		XpLEFT(CH_MARK),
		XpNARROW(CH_LIMIT),
		XpNARROW(CH_MIN),
		XpLEFT(CH_LIMIT),
		XpEND
	};

	lwxpf = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if ( lwxpf ) 
	{
		panID = (*lwxpf->create)( LWXP_VIEW, Protract_ctrl_list );
		if(panID) 
		{
			(*lwxpf->hint) ( panID, 0, hint );
			(*lwxpf->describe)( panID, Protract_data_descrip, ProtractData_get, ProtractData_set );
			(*lwxpf->viewInst)( panID, dat );
			(*lwxpf->setData)(panID, 0, dat);
	    }
    }
	return panID;
}

XCALL_(int) Protract_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	XCALL_INIT;
	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);
	UI->panel	= ProtractXPanel(global, UI->inst);
	UI->options	= NULL;
	UI->command	= NULL; 
	return AFUNC_OK;
}
