// Custom Object Class Item Display
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
#include <lwmath.h>
#include <lwdisplce.h>
#include <lwmaster.h>
#include <lwlaytool.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include "custdraw.h"

#define VCPY_F(a,b)       ((a)[0] =(float)((b)[0]), (a)[1] =(float)((b)[1]), (a)[2] =(float)((b)[2]))

static	LWXPanelFuncs	*GlobalXPanFun;
static	LWMessageFuncs	*Gmessage;
static	LWTextureFuncs	*GlobalTextureFuncs;
static	LWEnvelopeFuncs	*GlobalEnvelopeFuncs;
static	LWItemInfo		*GlobalItemInfo;
static	LWChannelInfo	*GlobalChannelInfo;
static	LWSceneInfo		*GlobalSceneInfo;
static	LWCameraInfo	*GlobalCameraInfo;
static	LWInstUpdate	*GlobalLWUpdate;
static	float	Red[]		= {0.92f, 0.2f, 0.2f},
				Green[]		= {0.2f, 0.92f, 0.2f},
				Blue[]		= {0.2f, 0.2f, 0.92f},
				Yellow[]	= {0.92f, 0.92f, 0.2f},
				Orange[]	= {0.92f, 0.62f, 0.32f},
				White[]		= {0.92f, 0.92f, 0.92f},
				Black[]		= {0.1f, 0.1f, 0.1f};

/* ----------------- Plug-in Methods: LWInstanceFuncs  -----------------  */

static char *errAllocFailed ="Tiny Allocation Failed, I don't feel so good", *AxisName="XYZ";
static int xax[] = {1,2,0}, yax[] = {2,0,1}, zax[] = {0,1,2};

static const char *ShapeList[] = {"Standard", "Box", "Ball", "Pyramid", "Diamond", "Tetra", "Ring","Grid","None",NULL};		
enum {SHP_STANDARD, SHP_BOX, SHP_BALL, SHP_PYRAMID, SHP_DIAMOND, SHP_TETRA, SHP_RING, SHP_GRID, SHP_EMPTY };
typedef struct st_ItemShapeData {
	void				*ctxt;
	LWItemID			 self, linkTo;
	double				 time, level, scale;
	int					 axis, fill, shape, flags;
	LWFVector			 clrA, clrB, clrT;
	LWDVector			 linkPos;
	char				 label[120];
	char				 desc[100];
} ItemShapeData;



#define LWCUST_TEXT_L	 0
#define LWCUST_TEXT_C	 1
#define LWCUST_TEXT_R	 2

#define ITSHPF_COLOR	1
#define ITSHPF_BCOLOR	2
#define ITSHPF_TCOLOR	4
#define ITSHPF_JUSTIFY	8  // off is centered, on is L unless JUSTR
#define ITSHPF_JUSTR	16 // right-justify if on, left or center from  ITSHPF_JUSTIFY
#define CUSTEXT_JUST(f)		( (f&ITSHPF_JUSTIFY) ? ((f&ITSHPF_JUSTR) ? LWCUST_TEXT_R:LWCUST_TEXT_L):LWCUST_TEXT_C )

#define 	TAG_SIZE		255
#define 	MAX_TAGS		80
static char		TagBuf[TAG_SIZE+1] = "";

// protos from proxypic
int popCnt_Item( LWItemInfo *ItemInfo );
const char *popName_Item( LWItemInfo *ItemInfo, int idx );
int popItemIdx( LWItemInfo *ItemInfo, LWItemID item );
LWItemID popIdxItem( LWItemInfo *ItemInfo, int idx );

// return index of item in list, 
int setItemTag(LWItemInfo *itInfo, LWItemID id, char *key, char *val)
{
	int t,n=0,i, blank=0;
	const char *tag;
	sprintf(TagBuf,"%s=", key);
	strncat(TagBuf, val, TAG_SIZE);
	for(t=0; t<MAX_TAGS; t++)	// store tag changes for current item
	{
		if( (tag=(*itInfo->getTag)(id,t+1)) )
		{
			n++;
			i = strlen(key);
			if(tag[0]==0)
				blank = t+1; // catch empty string tags for later
			if(!strncmp(tag,key,i) )
			{
				(*itInfo->setTag)(id,t+1,TagBuf);
				return n;
			}
		}
		else // end of existing tags
		{
			n++;
			(*itInfo->setTag)(id,blank,TagBuf);	// add tag, or if blank, fill in""
			return blank ? blank:n;
		}
	}
	return n;
}

int getItemTag(LWItemInfo *itInfo, LWItemID id, char *key, char *val, int len)
{
	int t,n=0,i;
	const char *tag;
	*val = 0;
	for(t=0; t<MAX_TAGS; t++)	// store tag changes for current item
	{
		if( (tag=(*itInfo->getTag)(id,t+1)) )
		{
			n++;
			i = strlen(key);
			if(!strncmp(tag,key,i))
			{
				//i++; // skip '='
				strncpy(val,&(tag[i]),len-i); // tag = "KEY="tag[i+1]
				return n;
			}
		}
	}
	return 0;
}

void killItemTag(LWItemInfo *itinfo, LWItemID id, char *key)
{
	int t=1,i;
	const char *tag;
	while( (tag=(*itinfo->getTag)(id,t)) )
	{
		i = strlen(key);
		if(!strncmp(tag,key,i))
		{
			(*itinfo->setTag)(id,t,"");
			return;
		}
		t++;
	}
	return;
}

XCALL_(static LWInstance)ItemShapeCreate(void *data, LWItemID id, LWError *err)
{
	ItemShapeData *dat=NULL;
	XCALL_INIT;
	if(dat=malloc(sizeof(ItemShapeData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->fill = 0;
		dat->axis = 1;
		dat->level = 1.0;
		dat->self = id;
		dat->scale = 1.0;
		dat->ctxt = data;
		getItemTag( GlobalItemInfo, id, "LINK=", dat->label, sizeof(dat->label) );
		if(dat->label[0])
			sscanf(dat->label,"%x", &dat->linkTo);
		dat->label[0]=0;
		getItemTag( GlobalItemInfo, id, "LABEL=", dat->label, sizeof(dat->label) );
		if(dat->label[0])
			dat->shape = SHP_EMPTY; // just label for auto-apply
		sprintf(dat->desc," %s %s", ShapeList[dat->shape], dat->label);
	}
	else
		*err = errAllocFailed;
	return dat;
}

XCALL_(static const char *)ItemShapeDescribe(ItemShapeData *dat)
{
	XCALL_INIT;
	if(dat->label[0])
		sprintf(dat->desc," : %s \"%s\"", ShapeList[dat->shape], dat->label);
	else
		sprintf(dat->desc," : %s", ShapeList[dat->shape]);
	return (dat->desc);
}

XCALL_(static void)ItemShapeDestroy(ItemShapeData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		free(dat);
	}
}

XCALL_(static LWError)ItemShapeCopy(ItemShapeData	*to, ItemShapeData	*from)
{
	XCALL_INIT;

	*to = *from;
	return (NULL);
}

XCALL_(static LWError)ItemShapeLoad(ItemShapeData *dat,const LWLoadState	*lState)
{
	unsigned int		v;
	float				sc = 1.0f;
	XCALL_INIT;
	LWLOAD_I4(lState,&v,1);
	if(v>=2)
	{
		LWLOAD_FP(lState,&sc,1);
		dat->scale = sc;
	}
	LWLOAD_I4(lState,&dat->axis,1);
	LWLOAD_I4(lState,&dat->shape,1);
	LWLOAD_I4(lState,&dat->fill,1);
	LWLOAD_I4(lState,&dat->flags,1);
	LWLOAD_FP(lState,dat->clrA,3);
	LWLOAD_FP(lState,dat->clrB,3);
	if(v>=3)
		LWLOAD_FP(lState,dat->clrT,3);
	if(v>=4)
	{
		LWLOAD_FP(lState,&sc,1);
		dat->level = sc;
	}

	LWLOAD_U4(lState,&v,1);
	dat->linkTo = (LWItemID)v;
	LWLOAD_STR(lState, dat->label,sizeof(dat->label));
	return (NULL);
}

XCALL_(LWError)ItemShapeSave(ItemShapeData *dat,const LWSaveState	*sState)
{
	unsigned int		v = 4;
	float				sc = 1.0f;
	XCALL_INIT;
	LWSAVE_I4(sState,&v,1);
	sc = (float)dat->scale;
	LWSAVE_FP(sState,&sc,1);
	LWSAVE_I4(sState,&dat->axis,1);
	LWSAVE_I4(sState,&dat->shape,1);
	LWSAVE_I4(sState,&dat->fill,1);
	LWSAVE_I4(sState,&dat->flags,1);
	LWSAVE_FP(sState,dat->clrA,3);
	LWSAVE_FP(sState,dat->clrB,3);
	LWSAVE_FP(sState,dat->clrT,3); // v3
	sc = (float)dat->level;
	LWSAVE_FP(sState,&sc,1); // v4
	v = (unsigned int)dat->linkTo;
	LWSAVE_U4(sState,&v,1);
	LWSAVE_STR(sState,dat->label);
	return (NULL);
}

/* ----------------- Plug-in Methods: LWRenderFuncs  -----------------  */

XCALL_(static LWError)ItemShapeInit (LWInstance inst, int i)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static LWError)ItemShapeNewTime (LWInstance inst, LWFrame f, LWTime t)
{
	ItemShapeData *dat = (ItemShapeData *)inst;
	XCALL_INIT;
	dat->time = t;
	if(dat->linkTo)
		GlobalItemInfo->param(dat->linkTo, LWIP_W_POSITION, t, dat->linkPos);
	return NULL;
}

XCALL_(static void)ItemShapeCleanup (LWInstance inst)
{
	XCALL_INIT;
	return;
}

/* ----------------- Plug-in Methods: LWItemFuncs  -----------------  */

XCALL_(static const LWItemID *)ItemShapeUseItems (LWInstance inst)
{
	ItemShapeData *dat = (ItemShapeData *)inst;
	static LWItemID items[] = {NULL, NULL};
	XCALL_INIT;
	items[0] = dat->linkTo;
	return items;
}

XCALL_(static void)ItemShapeChangeID (LWInstance inst, const LWItemID *items)
{
	ItemShapeData *dat = (ItemShapeData *)inst;
	int		 i=0;
	XCALL_INIT;
	if(dat->linkTo)
		while(items[i])
			if(items[i]==dat->linkTo)
			{
				dat->linkTo = items[i+1];
				return;
			}
			else 
				i += 2;
	return;
}



/* ----------------- Plug-in Methods: LWCustomObjHandler  -----------------  */

XCALL_(static unsigned int)ItemShapeFlags (ItemShapeData *dat)
{
	XCALL_INIT;
	return 0;
}	

#define	root3over2	0.866025403784438646763723170752936

XCALL_(static void)ItemShapeEval (ItemShapeData *dat,	const LWCustomObjAccess *cobjAcc)
{
	double  orig[3] = {0.0,0.0,0.0}, vec[3], pt[3];
	int		i,ix,iy;
	float	rgba[4];

	XCALL_INIT;	
	ix = xax[dat->axis];
	iy = yax[dat->axis];
	if(cobjAcc->flags&LWCOFL_SELECTED)
	{
		if(dat->flags&ITSHPF_COLOR)
		{
			VCPY(rgba,dat->clrA);
			rgba[3] = (float)dat->level;
			(*cobjAcc->setColor)(cobjAcc->dispData, rgba);
		}
	}
	else
		if(dat->flags&ITSHPF_BCOLOR)
		{
			VCPY(rgba,dat->clrB);
			rgba[3] = (float)dat->level;
			(*cobjAcc->setColor)(cobjAcc->dispData, rgba);
		}

	switch(dat->shape)
	{
		case SHP_BOX: 
			for(i=0; i<3; i++)
			{
				VCLR(vec);
				vec[i] -= 0.5*dat->scale;
				if(dat->fill)
				{
					if(i==dat->axis)
						co_Rectangle(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, i); // bottom open!
					else
						co_FillRect(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, i);
					vec[i] += 1.0*dat->scale;
					co_FillRect(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, i);
					orig[dat->axis] = 0.505*dat->scale;
				}
				else
				{
					co_Rectangle(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, i);
					vec[i] += 1.0*dat->scale;
					co_Rectangle(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, i);
				}
			}
			break;
		case SHP_BALL: 
			VCLR(vec);
			for(i=0; i<3; i++)
				co_Arc(cobjAcc, orig, 0.0,TWOPI, 0.5*dat->scale, LWCSYS_OBJECT, i);
			break;
		case SHP_PYRAMID:
			VCLR(vec);
			vec[dat->axis] = -0.50*root3over2*dat->scale;
			orig[dat->axis] = -vec[dat->axis];
			if(dat->fill)
			{
				co_FillRect(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, dat->axis);
				vec[ix] += 0.50*dat->scale;
				vec[iy] += 0.50*dat->scale;
				VCPY(pt,vec);
				pt[iy] -= 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, vec,orig,pt,LWCSYS_OBJECT);

				vec[ix] -= 1.0*dat->scale;
				vec[iy] -= 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, vec,orig,pt,LWCSYS_OBJECT);

				pt[ix] -= 1.0*dat->scale;
				pt[iy] += 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, vec,orig,pt,LWCSYS_OBJECT);

				vec[ix] += 1.0*dat->scale;
				vec[iy] += 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, vec,orig,pt,LWCSYS_OBJECT);

			}
			else
			{
				co_Rectangle(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, dat->axis);
				vec[ix] += 0.50*dat->scale;
				vec[iy] += 0.50*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, vec,orig,LWCSYS_OBJECT);
				vec[iy] -= 1.0*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, orig,vec,LWCSYS_OBJECT);
				vec[ix] -= 1.0*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, vec,orig,LWCSYS_OBJECT);
				vec[iy] += 1.0*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, orig,vec,LWCSYS_OBJECT);
			}
			break;
		case SHP_DIAMOND: 
			VCLR(vec);
			VCLR(pt);
			orig[dat->axis] -= 0.7072*dat->scale;//root3over2;
			pt[dat->axis] += 0.7072*dat->scale;//root3over2;
			vec[ix] += 0.5*dat->scale;
			vec[iy] += 0.5*dat->scale;
			if(dat->fill)
			{
				double v[3];
				VCPY(v,vec);
				v[iy] -= 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, pt,vec,v,LWCSYS_OBJECT);
				(*cobjAcc->triangle)(cobjAcc->dispData, orig,vec,v,LWCSYS_OBJECT);
				vec[iy] -= 1.0*dat->scale;
				vec[ix] -= 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, pt,vec,v,LWCSYS_OBJECT);
				(*cobjAcc->triangle)(cobjAcc->dispData, orig,vec,v,LWCSYS_OBJECT);
				v[iy] += 1.0*dat->scale;
				v[ix] -= 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, pt,vec,v,LWCSYS_OBJECT);
				(*cobjAcc->triangle)(cobjAcc->dispData, orig,vec,v,LWCSYS_OBJECT);
				vec[iy] += 1.0*dat->scale;
				vec[ix] += 1.0*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, pt,vec,v,LWCSYS_OBJECT);
				(*cobjAcc->triangle)(cobjAcc->dispData, orig,vec,v,LWCSYS_OBJECT);
			}
			else
			{
				(*cobjAcc->line)(cobjAcc->dispData, pt,vec,LWCSYS_OBJECT);
				(*cobjAcc->line)(cobjAcc->dispData, vec,orig,LWCSYS_OBJECT);
				vec[iy] -= 1.0*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, orig,vec,LWCSYS_OBJECT);
				(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_OBJECT);
				vec[ix] -= 1.0*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, pt,vec,LWCSYS_OBJECT);
				(*cobjAcc->line)(cobjAcc->dispData, vec,orig,LWCSYS_OBJECT);
				vec[iy] += 1.0*dat->scale;
				(*cobjAcc->line)(cobjAcc->dispData, orig,vec,LWCSYS_OBJECT);
				(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_OBJECT);
				VCLR(vec);
				co_Rectangle(cobjAcc, vec, 1.0*dat->scale, 1.0*dat->scale, LWCSYS_OBJECT, dat->axis);
			}
			orig[dat->axis] = 0.7072*dat->scale;//root3over2;
			break;
		case SHP_TETRA: 
			VCLR(vec);
			VCLR(pt);
			vec[dat->axis] = -0.25*dat->scale;
			orig[dat->axis] = 0.5*dat->scale;
			pt[dat->axis] = vec[dat->axis];
			vec[iy] = 0.5*dat->scale;
			(*cobjAcc->line)(cobjAcc->dispData, vec,orig,LWCSYS_OBJECT);
			pt[iy] = -0.25*dat->scale;
			pt[ix] = -0.5*root3over2*dat->scale;
			if(dat->fill)
				(*cobjAcc->triangle)(cobjAcc->dispData, orig, pt,vec, LWCSYS_OBJECT);
			else
			{
				(*cobjAcc->line)(cobjAcc->dispData, orig, pt, LWCSYS_OBJECT);
				(*cobjAcc->line)(cobjAcc->dispData, pt,vec,LWCSYS_OBJECT);
			}
			pt[ix] = 0.5*root3over2*dat->scale;
			if(dat->fill)
				(*cobjAcc->triangle)(cobjAcc->dispData, vec, pt, orig, LWCSYS_OBJECT);
			else
			{
				(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_OBJECT);
				(*cobjAcc->line)(cobjAcc->dispData, pt,orig,LWCSYS_OBJECT);
			}
			VCPY(vec,pt);
			pt[ix] = -0.5*root3over2*dat->scale;
			if(dat->fill)
			{
				(*cobjAcc->triangle)(cobjAcc->dispData, vec, pt, orig, LWCSYS_OBJECT);
				VCPY(orig,pt);
				orig[ix] = 0.5*root3over2*dat->scale;
				(*cobjAcc->triangle)(cobjAcc->dispData, vec, pt, orig, LWCSYS_OBJECT); // bottom (optional!)
				VCLR(orig);
				orig[dat->axis] = 0.5*dat->scale;
			}
			else
				(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_OBJECT);
			break;
		case SHP_RING: 
			if(dat->fill)
			{
				co_FillArc(cobjAcc, orig, 0.0, TWOPI, 1.0*dat->scale, LWCSYS_OBJECT, dat->axis);
				orig[iy] += 1.0*dat->scale;
			}
			else
				co_Arc(cobjAcc, orig, 0.0, TWOPI, 1.0*dat->scale, LWCSYS_OBJECT, dat->axis);
			break;
		case SHP_GRID: 
			VSET(pt, -5.0);
			pt[dat->axis] = 0.0;
			VSET(vec, 10.0);

			co_Grid(cobjAcc, pt, vec, 10, LWCSYS_OBJECT, dat->axis);
		//	if(*dat->label)
			{
				char	pos[]="+ X", neg[]="- X";
				pt[ix] = 0.0;
				pt[dat->axis] = 0.005;
				neg[2] = AxisName[iy];
				pos[2] = AxisName[iy];
				(*cobjAcc->text)(cobjAcc->dispData, pt,neg, 1, LWCSYS_OBJECT);
				pt[iy] = 5;
				(*cobjAcc->text)(cobjAcc->dispData, pt,pos, 1, LWCSYS_OBJECT);
				neg[2] = AxisName[ix];
				pos[2] = AxisName[ix];
				pt[iy] = 0.0;
				pt[ix] = 5;
				(*cobjAcc->text)(cobjAcc->dispData, pt,pos, 1, LWCSYS_OBJECT);
				pt[ix] = -5;
				(*cobjAcc->text)(cobjAcc->dispData, pt,neg, 1, LWCSYS_OBJECT);
			}
			break;
		case SHP_STANDARD:
			VCLR(vec);
			VCLR(pt);
			vec[0] = 0.15;
			pt[0] = -0.15;
			(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_ICON);
			vec[0] = pt[0] = 0.0;
			vec[1] = 0.15;
			pt[1] = -0.15;
			(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_ICON);
			vec[1] = pt[1] = 0.0;
			vec[2] = 0.15;
			pt[2] = -0.15;
			(*cobjAcc->line)(cobjAcc->dispData, vec,pt,LWCSYS_ICON);
		case SHP_EMPTY:
		default:
			break;
	}
	if(dat->linkTo)
	{
		GlobalItemInfo->param(dat->self, LWIP_W_POSITION, dat->time, pt);
		cobjAcc->setPattern(cobjAcc->dispData, LWLPAT_DASH);
		cobjAcc->line(cobjAcc->dispData, dat->linkPos, pt, LWCSYS_WORLD);
	}
	// above switch leaves orig placed for label!!!
	if(*dat->label)
	{
		if(dat->flags&ITSHPF_TCOLOR)
		{
			VCPY(rgba,dat->clrT);
			rgba[3] = (float)dat->level;
			(*cobjAcc->setColor)(cobjAcc->dispData, rgba);
		}
		(*cobjAcc->text)(cobjAcc->dispData, orig, dat->label, CUSTEXT_JUST(dat->flags), LWCSYS_OBJECT);
	}

}

/* -----------------                 -----------------  */


XCALL_(int) ItemShape (
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

	GlobalEnvelopeFuncs = (*global) (LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalEnvelopeFuncs )
		return AFUNC_BADGLOBAL;
	GlobalChannelInfo = (*global) (LWCHANNELINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalChannelInfo )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalLWUpdate)
	{
		(*Gmessage->error)("Can't get global",LWINSTUPDATE_GLOBAL);
		return AFUNC_BADGLOBAL;
	}

	local->inst->create  = ItemShapeCreate;
	local->inst->destroy = ItemShapeDestroy;
	local->inst->load    = ItemShapeLoad;
	local->inst->save    = ItemShapeSave;
	local->inst->copy    = ItemShapeCopy;
	local->inst->descln	 = ItemShapeDescribe;

	local->item->useItems    = ItemShapeUseItems;
	local->item->changeID    = ItemShapeChangeID;

	local->rend->init		= ItemShapeInit;
	local->rend->newTime    = ItemShapeNewTime;
	local->rend->cleanup	= ItemShapeCleanup;

	local->evaluate 	= ItemShapeEval;
	local->flags 		= ItemShapeFlags;
	return (AFUNC_OK);
}

/* -----------------  User Interface  ----------------- */

enum  {	CH_AXIS = 0x8601, CH_SHAPE, CH_SIZE, CH_LABL, CH_USELB, CH_LEVL, CH_FILL, 
		CH_COLR, CH_USEC, CH_BCOLR, CH_USEBC, CH_TCOLR, CH_USETC, CH_SCAL, CH_TARG,
		CH_JUST, CH_DISPGR, CH_GRP1, CH_GRP2 };
#define			STR_Axis_TEXT		"Axis"
#define			STR_Shape_TEXT		"Shape"
#define			STR_Filled_TEXT		"Filled"
#define			STR_UseC_TEXT		"Selected Color"
#define			STR_UseBC_TEXT		"Unselected Color"
#define			STR_UseTC_TEXT		"Text Color"
#define			STR_Color_TEXT		" "
#define			STR_Label_TEXT		"Label"
#define			STR_Level_TEXT		"Opacity"
#define			STR_Link_TEXT		"Draw Line To"
#define			STR_Scale_TEXT		"Scale"
#define			STR_Just_TEXT		"Justification"
#define			STR__TEXT		""

static char *JustList[] = {"Left","Center","Right",NULL};

static LWXPanelControl ItemShape_ctrl_list[] = {
	{ CH_AXIS,       STR_Axis_TEXT,          "axis" },
	{ CH_SHAPE,        STR_Shape_TEXT,          "iPopChoice" },
	{ CH_FILL,        STR_Filled_TEXT,          "iBoolean" },
	{ CH_USEC,        STR_UseC_TEXT,          "iBoolean" },
	{ CH_COLR,        STR_Color_TEXT,          "color" },
	{ CH_USEBC,        STR_UseBC_TEXT,          "iBoolean" },
	{ CH_BCOLR,        STR_Color_TEXT,          "color" },
	{ CH_USETC,        STR_UseTC_TEXT,          "iBoolean" },
	{ CH_TCOLR,        STR_Color_TEXT,          "color" },
	{ CH_LABL,        STR_Label_TEXT,          "string" },
	{ CH_JUST,        STR_Just_TEXT,          "iPopChoice" },
	{ CH_SCAL,        STR_Scale_TEXT,          "distance" },
	{ CH_LEVL,        STR_Level_TEXT,          "percent" },
	{ CH_TARG,        STR_Link_TEXT,          "iPopChoice" },
	{ CH_USELB,        "",          "integer" },
	{0}
};
static LWXPanelDataDesc ItemShape_data_descrip[] = {
	{ CH_AXIS,        STR_Axis_TEXT,          "integer" },
	{ CH_SHAPE,        STR_Shape_TEXT,          "integer" },
	{ CH_FILL,        STR_Filled_TEXT,          "integer" },
	{ CH_USEC,        STR_UseC_TEXT,			"integer" },
	{ CH_COLR,        STR_Color_TEXT,          "color" },
	{ CH_USEBC,        STR_UseBC_TEXT,			"integer" },
	{ CH_BCOLR,        STR_Color_TEXT,          "color" },
	{ CH_USETC,        STR_UseTC_TEXT,			"integer" },
	{ CH_TCOLR,        STR_Color_TEXT,          "color" },
	{ CH_LABL,        STR_Label_TEXT,          "string" },
	{ CH_JUST,        STR_Just_TEXT,			"integer" },
	{ CH_SCAL,        STR_Scale_TEXT,          "distance" },
	{ CH_LEVL,        STR_Level_TEXT,          "percent" },
	{ CH_TARG,        STR_Link_TEXT,          "integer" },
	{ CH_USELB,        "",          "integer" },
	{0},
};


static void *ItemShapeData_get ( void *myinst, unsigned long vid ) 
{
  ItemShapeData *dat = (ItemShapeData*)myinst;
  void *result = NULL;
  static int val = 0;
  static double rgba[4];

  if ( dat ) 
	  switch ( vid ) {
		case CH_AXIS:
			result = &dat->axis;
			break;
		case CH_SHAPE:
			result = &dat->shape;
			break;
		case CH_FILL:
			result = &dat->fill;
			break;
		case CH_COLR:
			VCPY(rgba,dat->clrA);
			result = &rgba;
			break;
		case CH_BCOLR:
			VCPY(rgba,dat->clrB);
			result = &rgba;
			break;
		case CH_TCOLR:
			VCPY(rgba,dat->clrT);
			result = &rgba;
			break;
		case CH_USEC:
			val = dat->flags&ITSHPF_COLOR ? 1 : 0;
			result = &val;
			break;
		case CH_USEBC:
			val = dat->flags&ITSHPF_BCOLOR ? 1 : 0;
			result = &val;
			break;
		case CH_USETC:
			val = dat->flags&ITSHPF_TCOLOR ? 1 : 0;
			result = &val;
			break;
		case CH_LABL:
			result = &dat->label;
			break;
		case CH_JUST:
			val = CUSTEXT_JUST(dat->flags);
			result = &val;
			break;
		case CH_USELB:
			val = (int)dat->label[0];
			result = &val;
			break;
		case CH_LEVL:
			result = &dat->level;
			break;
		case CH_SCAL:
			result = &dat->scale;
			break;
		case CH_TARG:
			val = popItemIdx(GlobalItemInfo, dat->linkTo);
			result = &val;
			break;
	  } 
  return result;
}

static int ItemShapeData_set ( void *myinst, unsigned long vid, void *value ) 
{
	ItemShapeData *dat = (ItemShapeData*)myinst;
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
			case CH_BCOLR:
				VCPY(rgba,((double*)value));
				VCPY_F(dat->clrB, rgba);
				rc = 1;
				break;
			case CH_TCOLR:
				VCPY(rgba,((double*)value));
				VCPY_F(dat->clrT, rgba);
				rc = 1;
				break;
			case CH_USEC:
				rc = *((int*)value);
				if(rc)
					dat->flags |= ITSHPF_COLOR;
				else
					dat->flags &= ~ITSHPF_COLOR;
				rc = 1;
				break;
			case CH_USEBC:
				rc = *((int*)value);
				if(rc)
					dat->flags |= ITSHPF_BCOLOR;
				else
					dat->flags &= ~ITSHPF_BCOLOR;
				rc = 1;
				break;
			case CH_USETC:
				rc = *((int*)value);
				if(rc)
					dat->flags |= ITSHPF_TCOLOR;
				else
					dat->flags &= ~ITSHPF_TCOLOR;
				rc = 1;
				break;
			case CH_FILL:
				dat->fill = *((int*)value);
				rc = 1;
				break;
			case CH_SHAPE:
				dat->shape = *((int*)value);
				rc = 1;
				break;
			case CH_LABL:
				strncpy(dat->label, ((char*)value), sizeof(dat->label)-1 );
				setItemTag(GlobalItemInfo, dat->self, "LABEL",dat->label);
				rc = 1;
				break;
			case CH_JUST:
				rc = *((int*)value);
				dat->flags &= ~(ITSHPF_JUSTIFY|ITSHPF_JUSTR); // center
				if(rc==0) // left
					dat->flags |= ITSHPF_JUSTIFY;
				else if(rc==2) // right
					dat->flags |= ITSHPF_JUSTIFY|ITSHPF_JUSTR;
				rc = 1;
				break;
			case CH_USELB:
				rc = 1;
				break;
			case CH_LEVL:
				dat->level = *((double*)value);
				rc = 1;
				break;
			case CH_SCAL:
				dat->scale = *((double*)value);
				rc = 1;
				break;
			case CH_TARG:
				rc = *((int*)value);
				dat->linkTo = popIdxItem(GlobalItemInfo, rc);
				rc = 1;
				break;
		} 
	if(rc)
		(*GlobalLWUpdate)(LWCUSTOMOBJ_HCLASS, dat);
	return rc;
}

static int   levEnable[] = {0,0,0,0,0,0,0,0,0,0,0,0,0},
		fillEnable[] = {0,1,0,1,1,1,1,0,0,0,0,0,0};		
static LWXPanelID ItemShapeXPanel(GlobalFunc *global, ItemShapeData *dat)
{
	LWXPanelFuncs *lwxpf = NULL;
	LWXPanelID     panID = NULL;
	static LWXPanelHint hint[] = {
		XpDELETE     (CH_USELB),
		XpLABEL(0,"ItemShape Object"),
		XpSTRLIST(CH_SHAPE,ShapeList),
		XpSTRLIST(CH_JUST,JustList),
		XpPOPFUNCS(CH_TARG,popCnt_Item, popName_Item),
		XpMIN(CH_LEVL, 0),
		XpMAX(CH_LEVL, 1),
		XpGROUP_(CH_GRP1),
			XpH(CH_SHAPE),
			XpH(CH_AXIS),
			XpH(CH_SCAL),
			XpH(CH_FILL),
			XpEND,
		XpGROUP_(CH_GRP2),
			XpH(CH_LABL),
			XpH(CH_JUST),
			XpH(CH_TARG),
			XpEND,
		XpGROUP_(CH_DISPGR),
			XpH(CH_USEC),
			XpH(CH_COLR),
			XpH(CH_USEBC),
			XpH(CH_BCOLR),
			XpH(CH_USETC),
			XpH(CH_TCOLR),
			XpH(CH_LEVL),
			XpEND,
		XpENABLE_(CH_USEC),
			XpH(CH_COLR),
			XpEND,
		XpENABLE_(CH_USEBC),
			XpH(CH_BCOLR),
			XpEND,
		XpENABLE_(CH_USETC),
			XpH(CH_TCOLR),
			XpEND,
		XpENABLE_MAP_(CH_SHAPE,fillEnable),
			XpH(CH_FILL),
			XpEND,
		XpENABLEMSG_(CH_USELB,"This control is disabled because there is no label"),
			XpH(CH_USETC),
			XpH(CH_TCOLR),
			XpH(CH_JUST),
			XpEND,
		XpNARROW(CH_USEC),
		XpNARROW(CH_COLR),
		XpLEFT(CH_USEC),
		XpNARROW(CH_USEBC),
		XpNARROW(CH_BCOLR),
		XpLEFT(CH_USEBC),
		XpNARROW(CH_USETC),
		XpNARROW(CH_TCOLR),
		XpLEFT(CH_USETC),
		XpNARROW(CH_FILL),
		XpNARROW(CH_LEVL),
//		XpLEFT(CH_FILL),
		XpEND
	};

	lwxpf = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if ( lwxpf ) 
	{
		panID = (*lwxpf->create)( LWXP_VIEW, ItemShape_ctrl_list );
		if(panID) 
		{
		//	(*lwxpf->hint) ( panID, 0, common_hint );
			(*lwxpf->hint) ( panID, 0, hint );
			(*lwxpf->describe)( panID, ItemShape_data_descrip, ItemShapeData_get, ItemShapeData_set );
			(*lwxpf->viewInst)( panID, dat );
			(*lwxpf->setData)(panID, 0, dat);
			(*lwxpf->setData)(panID, CH_TARG, GlobalItemInfo);
	    }
    }
	return panID;
}

XCALL_(int) ItemShape_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	XCALL_INIT;
	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);
	UI->panel	= ItemShapeXPanel(global, UI->inst);
	UI->options	= NULL;
	UI->command	= NULL; 
	return AFUNC_OK;
}



static ServerTagInfo 
Shape_tags[] = { 
	{"Item Shape",SRVTAG_USERNAME|LANGID_USENGLISH}, 
	{NULL,0} },
Slide_tags[] = { 
	{"Sliders",SRVTAG_USERNAME|LANGID_USENGLISH}, 
	{"Tool_Sliders", SRVTAG_SELECTCMD},
	{NULL,0} },
pro_tags[] = { 
	{"Protractor",SRVTAG_USERNAME|LANGID_USENGLISH}, 
	{NULL,0} },
Lat_tags[] = { 
	{"Lattice",SRVTAG_USERNAME|LANGID_USENGLISH}, 
	{NULL,0} };


XCALL_(int) Lattice_UI (long version, GlobalFunc *global, LWInterface *UI, void *serverData);
XCALL_(int) Lattice (long version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
XCALL_(int) LatticeDisp (long version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
XCALL_(int) Protract_UI (long version, GlobalFunc *global, LWInterface *UI, void *serverData);
XCALL_(int) Protract (long version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
XCALL_(int) ProxyPick (long version, GlobalFunc *global, LWMasterHandler *local, void *serverData);
XCALL_(int) ProxyPick_UI (long version, GlobalFunc *global, LWInterface *UI, void *serverData);
XCALL_(int) Sliders_UI (long version, GlobalFunc *global, LWInterface *UI, void *serverData);
XCALL_(int) Sliders (long version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
XCALL_(int) Sliders_Tool (long version, GlobalFunc *global, LWLayoutTool *local, void *serverData);

ServerRecord ServerDesc[] = {
	{ LWCUSTOMOBJ_HCLASS,		"LW_ItemShape",			ItemShape, Shape_tags },
	{ LWCUSTOMOBJ_ICLASS,		"LW_ItemShape",		ItemShape_UI, Shape_tags },
	{ LWCUSTOMOBJ_HCLASS,		"LW_Protract",			Protract, pro_tags },
	{ LWCUSTOMOBJ_ICLASS,		"LW_Protract",		Protract_UI,  pro_tags},
	{ LWCUSTOMOBJ_HCLASS,		"Sliders",			Sliders, Slide_tags },
	{ LWCUSTOMOBJ_ICLASS,		"Sliders",		Sliders_UI},
	{ LWMASTER_HCLASS,		"ProxyPick",			ProxyPick },
	{ LWMASTER_ICLASS,		"ProxyPick",		ProxyPick_UI},
	{ LWLAYOUTTOOL_CLASS,		"Sliders",		Sliders_Tool},
	{ NULL }
};

