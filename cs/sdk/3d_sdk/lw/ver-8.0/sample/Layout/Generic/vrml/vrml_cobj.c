/* vrml_cobj.c - Custom Objects for VRML97 plugin for LightWave 3D v6
 *		Arnie Cachelin		Copyright 2000 NewTek, Inc.
 *
 *	7/20/00
 */

#include <lwhost.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwenvel.h>
#include <lwtxtr.h>
#include <lwmath.h>
#include <lwrender.h>
#include <lwgeneric.h>
#include <lwenvel.h>
#include <lwpanel.h>
#include <lwcustobj.h>
 
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "xvrml.h"
#include "sctags.h"
#include "objectdb.h"

typedef struct st_VRML_ObjData {
	void				*ctxt;
	LWItemID			 self, buddy, trig;
	int					 type, flags, hasLink,nx,nz;
	LWDVector			 radius, delta;
	double				 time, offset, sx, sz, phase, volume, LODRangeList[MAX_LOD_OBJECTS+1] ;
	char				 desc[100];
	char				 URL[256],AudURL[256];
} VRML_ObjData;

#define VROF_AUDIO	1
#define VROF_PROX	2
#define VROF_GRID	4
#define VROF_LINK	8
#define VROF_URL	16
#define VROF_ALOOP	32
#define VROF_LOD	64

static char *vrObjTypes[] = {"Touch","Proximity","Visibility",NULL}; //,"Elevation Grid"
enum { VRO_TOUCH, VRO_PROX,VRO_VIS, VRO_GRID,VRO_URL,   VRO_LAST};

static int xax[] = {1,2,0}, yax[] = {2,0,1}, zax[] = {0,1,2};
static	float	Red[]		= {0.92f, 0.2f, 0.2f, 1.0f},
				Green[]		= {0.2f, 0.92f, 0.2f, 1.0f},
				Blue[]		= {0.2f, 0.2f, 0.92f, 1.0f},
				Yellow[]	= {0.92f, 0.92f, 0.2f, 1.0f},
				Orange[]	= {0.92f, 0.62f, 0.32f, 1.0f},
				Violet[]	= {0.62f, 0.02f, 0.92f, 1.0f},
				White[]		= {0.92f, 0.92f, 0.92f, 1.0f},
				Black[]		= {0.1f, 0.1f, 0.1f, 1.0f};

static char *errAllocFailed ="Tiny Allocation Failed, I don't feel so good";

static	LWXPanelFuncs	*GlobalXPanFun;
static	LWPanelFuncs	*GlobalPanFun;
static	LWMessageFuncs	*Gmessage;
static	LWTextureFuncs	*GlobalTextureFuncs;
static	LWItemInfo		*GlobalItemInfo;
static	LWCameraInfo	*GlobalCameraInfo;
static	LWSceneInfo		*GlobalSceneInfo;
static	LWInstUpdate	*GlobalLWUpdate;

void transformLWD(LWDVector p, LWDVector o, double *m, LWDVector c) {
    p[0] = o[0] * m[0] + o[1] * m[3] + o[2] * m[6] + c[0];
    p[1] = o[0] * m[1] + o[1] * m[4] + o[2] * m[7] + c[1];
    p[2] = o[0] * m[2] + o[1] * m[5] + o[2] * m[8] + c[2];
}

static char	BigBuf[MAX_LIGHT_NAME]="";

int getVRTags(VRML_ObjData *dat)
{
	int		t=0;
	char	*c;

	*BigBuf = 0;
	dat->hasLink = 0;
	if(getItemTag(GlobalItemInfo,dat->self,VTAG_TRIGGER"=", BigBuf,sizeof(BigBuf) ))
		dat->trig = findAnyItem(BigBuf);
	else
		dat->trig = NULL;
	getItemTag(GlobalItemInfo,dat->self,VTAG_URL"=", dat->URL,sizeof(dat->URL)-1 );
	if(dat->trig)
	{
		dat->hasLink = 1;
		dat->flags |= VROF_LINK;
	}
	else
		dat->flags &= ~VROF_LINK;
//	dat->type = VRO_TOUCH;
	if(getItemTag(GlobalItemInfo,dat->self,VTAG_VISIBILITY"=", BigBuf,sizeof(BigBuf) ))
	{
	/*	float w=-1,h=-1,d=-1;
		sscanf(BigBuf,"%f %f %f",&w,&h,&d);
		dat->radius[0] = (double)w;
		dat->radius[1] = (double)h;
		dat->radius[2] = (double)d; */
		dat->type = VRO_VIS;
	}
	if(getItemTag(GlobalItemInfo,dat->self,VTAG_PROXIMITY"=", BigBuf,sizeof(BigBuf) ))
	{
		float w=-1,h=-1,d=-1;
		sscanf(BigBuf,"%f %f %f",&w,&h,&d);
		dat->radius[0] = (double)w;
		dat->radius[1] = (double)h;
		dat->radius[2] = (double)d;
		dat->flags |= VROF_PROX;
		dat->type = VRO_PROX;

	}
	else
		dat->flags &= ~VROF_PROX;
	

	if(getItemTag(GlobalItemInfo,dat->self,VTAG_GRID"=", BigBuf,sizeof(BigBuf) ))
	{
		float w=-1,h=-1;
		sscanf(BigBuf,"%f %f %d %d",&w,&h,&dat->nx,&dat->nz);
		dat->sx = w;
		dat->sz = h;
		dat->flags |= VROF_GRID;
	}
	else
		dat->flags &= ~VROF_GRID;
	if(getItemTag(GlobalItemInfo, dat->self,VTAG_SOUND"=",BigBuf,sizeof(BigBuf)))
	{
		int vol=50,loop=0;
		dat->flags |= VROF_AUDIO;
		sscanf(BigBuf,"%90s %d %d",dat->AudURL,&vol,&loop);
		dat->volume = ((double)vol)/100.0;
		if(loop)
			dat->flags |= VROF_ALOOP;
		else
			dat->flags &= ~VROF_ALOOP;
	}
//	else
//		dat->flags &= ~VROF_AUDIO;
	t=0;
	dat->LODRangeList[0] = -1.0;
	while(getItemTagN(GlobalItemInfo, dat->self,VTAG_LOD"=",t ,BigBuf,sizeof(BigBuf)))
	{
		c = strtok(BigBuf, " \n");
		c = strtok(NULL, " \n");
		if(!c || t>=MAX_LOD_OBJECTS)
			break;
		dat->LODRangeList[t] = atof(c);
		t++;
		dat->LODRangeList[t] = -1.0; // next
		dat->flags |= VROF_LOD;
	}

	return dat->flags;
}

int setVRTag(VRML_ObjData *dat, unsigned long tagid)
{
	int		t=-1;
	*BigBuf = 0;

	switch(tagid)
	{
		case VTAGID_SOUND:
			t = 0;
			if(getItemTag(GlobalItemInfo, dat->self,VTAG_SOUND"=",BigBuf,sizeof(BigBuf)))
			{
				int vol=50,loop;
				loop = dat->flags&VROF_ALOOP ? 1:0;
				sscanf(BigBuf,"%254s %d %d",dat->AudURL,&vol,&loop);
				vol = (int)(dat->volume*100.0);
				sprintf(BigBuf,"%254s %d %d",dat->AudURL,vol,loop);
				setItemTag(GlobalItemInfo, dat->self,VTAG_SOUND,BigBuf);
			}
			else if(*dat->AudURL)
			{
				sprintf(BigBuf,"%254s %d 0",dat->AudURL,(int)(dat->volume*100.0));
				setItemTag(GlobalItemInfo, dat->self,VTAG_SOUND,BigBuf);
			}
			break;
		case VTAGID_PROXIMITY:
			t = VRO_PROX;
			sprintf(BigBuf,"%f %f %f",dat->radius[0],dat->radius[1],dat->radius[2]);
			setItemTag(GlobalItemInfo, dat->self,VTAG_PROXIMITY,BigBuf);
			break;
		case VTAGID_VISIBILITY:
			t = VRO_VIS;
			setItemTag(GlobalItemInfo, dat->self,VTAG_VISIBILITY," ");
			break;
		case VTAGID_TOUCH:
			t = VRO_TOUCH;
			killItemTag(GlobalItemInfo, dat->self,VTAG_VISIBILITY);
			killItemTag(GlobalItemInfo, dat->self,VTAG_PROXIMITY);
			break;
		case VTAGID_URL:
			t = VRO_URL;
			if(*dat->URL)
				setItemTag(GlobalItemInfo, dat->self,VTAG_URL,dat->URL);
			break;
		default:
			return 0;
	}
	return t;
}


XCALL_(static LWInstance)VRML_ObjCreate(void *data, LWItemID id, LWError *err)
{
	VRML_ObjData *dat=NULL;
	XCALL_INIT;
	if(dat=malloc(sizeof(VRML_ObjData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->volume = 1.0;
		dat->sx = 1.0;
		dat->sz = 1.0;
		dat->nx = 4;
		dat->nz = 4;
		dat->self = id;
		VSET(dat->radius,0.5);
		dat->type = VRO_PROX;

		getVRTags(dat);
		if(0>dat->type)
			dat->type = VRO_PROX;

		dat->ctxt = data;
		sprintf(dat->desc," %s", vrObjTypes[dat->type] );
	}
	else
		*err = errAllocFailed;
	return dat;
}


XCALL_(static void)VRML_ObjDestroy(VRML_ObjData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		free(dat);
	}
}


XCALL_(static LWError)VRML_ObjCopy(VRML_ObjData	*to, VRML_ObjData	*from)
{
	LWItemID	id;
	XCALL_INIT;

	id = to->self;
	*to = *from;
	to->self = id;
	return (NULL);
}

XCALL_(static LWError)VRML_ObjLoad(VRML_ObjData *dat,const LWLoadState	*lState)
{
	int			i=0;
	XCALL_INIT;
	LWLOAD_I4(lState,&i,1); // version
	LWLOAD_I4(lState,&i,1);
	dat->flags = i;
	LWLOAD_I4(lState,&i,1);
	dat->type = i;

	return (NULL);
}

XCALL_(LWError)VRML_ObjSave(VRML_ObjData *dat,const LWSaveState	*sState)
{
	int			i=1;
	XCALL_INIT;
	LWSAVE_I4(sState,&i,1); // version
	LWSAVE_I4(sState,&dat->flags,1); 
	LWSAVE_I4(sState,&dat->type,1); 
	return (NULL);
}

XCALL_(static const char *)VRML_ObjDescribe (LWInstance inst)
{
	VRML_ObjData *dat = (VRML_ObjData *)inst;
	XCALL_INIT;
	
	sprintf(dat->desc," %s", vrObjTypes[dat->type] );

	return (dat->desc);
}

/* ----------------- Plug-in Methods: LWItemFuncs  -----------------  */

XCALL_(static const LWItemID *)VRML_ObjUseItems (LWInstance inst)
{
	VRML_ObjData *dat = (VRML_ObjData *)inst;
	static LWItemID		ids[] = {NULL,NULL,NULL};
	XCALL_INIT;
	ids[0] = dat->trig;
	ids[1] = dat->buddy;
	return (NULL);
}

XCALL_(static void)VRML_ObjChangeID (LWInstance inst, const LWItemID *items)
{
	XCALL_INIT;
	return;
}


/* ----------------- Plug-in Methods: LWRenderFuncs  -----------------  */

XCALL_(static LWError)VRML_ObjInit (LWInstance inst, int i)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static LWError)VRML_ObjNewTime (LWInstance inst, LWFrame f, LWTime t)
{
	VRML_ObjData *dat = (VRML_ObjData *)inst;
	XCALL_INIT;
	dat->time = t;
	getVRTags(dat);
	return NULL;
}

XCALL_(static void)VRML_ObjCleanup (LWInstance inst)
{
	XCALL_INIT;
	return;
}

/* ----------------- Plug-in Methods: LWCustomObjHandler  -----------------  */

XCALL_(static unsigned int)VRML_ObjFlags (VRML_ObjData *inst)
{
	XCALL_INIT;

	return 0;
}	


void co_Line(const LWCustomObjAccess *cobjAcc, LWDVector p0, LWDVector p1, int csys)
{
	LWDVector h1,h0;
	VCPY(h0,p0);
	VCPY(h1,p1);
	cobjAcc->line(cobjAcc->dispData, h0,h1,csys);
}

void co_Rectangle(const LWCustomObjAccess *cobjAcc, double pos[3], double w, double h, int csys, int axis)
{
	double dx,dy,stp[3],endp[3],z;
	int axx,axy;
	z = pos[axis];
	axx = xax[axis];
	axy = yax[axis];
	dx = 0.5*w;
	dy = 0.5*h;
	stp[axis] = endp[axis] = z;
	stp[axx] = pos[axx] - dx;
	stp[axy] = pos[axy] + dy;
	endp[axx] = pos[axx] + dx;
	endp[axy] = pos[axy] + dy;
	(*cobjAcc->line)(cobjAcc->dispData, stp,endp,csys); // in icon mode, overwrites stp, endp
	stp[axis] = endp[axis] = z;
	endp[axx] = pos[axx] + dx;
	endp[axy] = pos[axy] + dy;

	stp[axx] = pos[axx] + dx;
	stp[axy] = pos[axy] - dy;
	(*cobjAcc->line)(cobjAcc->dispData, endp,stp,csys);
	stp[axis] = endp[axis] = z;
	stp[axx] = pos[axx] + dx;
	stp[axy] = pos[axy] - dy;

	endp[axx] = pos[axx] - dx;
	endp[axy] = pos[axy] - dy;
	(*cobjAcc->line)(cobjAcc->dispData, stp,endp,csys);
	stp[axis] = endp[axis] = z;
	endp[axx] = pos[axx] - dx;
	endp[axy] = pos[axy] - dy;

	stp[axx] = pos[axx] - dx;
	stp[axy] = pos[axy] + dy;
	(*cobjAcc->line)(cobjAcc->dispData, endp,stp,csys);
}

void co_Box(const LWCustomObjAccess *cobjAcc, double pos[3], double siz[3], int csys)
{
	double stp[3];
	int ax, i,j;

	for(ax=0; ax<3; ax++)
	{
		VCPY(stp,pos);
		stp[ax] -= 0.5*siz[ax];
		i = (ax+1)%3;
		j = (i+1)%3;
		co_Rectangle(cobjAcc,stp,siz[i],siz[j],csys,ax);
		stp[ax] += siz[ax];
		co_Rectangle(cobjAcc,stp,siz[i],siz[j],csys,ax);
	}
}

void co_Grid(const LWCustomObjAccess *cobjAcc, double pos[3], double siz[3], int div, int csys, int axis)
{
	double dx,dy,stp[3],endp[3],z;
	int axx,axy,i;
	z = pos[axis];
	axx = xax[axis];
	axy = yax[axis];
	dx = siz[axx]/div;
	dy = siz[axy]/div;
	stp[axis] = endp[axis] = z;
	stp[axx] = pos[axx];
	stp[axy] = pos[axy];
	endp[axx] = pos[axx];
	endp[axy] = pos[axy];

	endp[axx] += siz[axx];
	for(i=0;i<=div;i++)
	{
		co_Line(cobjAcc, stp, endp, csys);
		stp[axy] += dy;
		endp[axy] += dy;
	}

	stp[axx] = pos[axx];
	stp[axy] = pos[axy];
	endp[axx] = pos[axx];
	endp[axy] = pos[axy];

	endp[axy] += siz[axy];
	for(i=0;i<=div;i++)
	{
		co_Line(cobjAcc, stp, endp, csys);
 		stp[axx] += dx;
		endp[axx] += dx;
	}

}

double CameraDistanceSQ( LWItemID id, LWTime t)
{
	double		d=0.0, obj[3], cam[3];
	LWItemID	camId;

	camId = (*GlobalSceneInfo->renderCamera)(t);
	(*GlobalItemInfo->param)(id,LWIP_W_POSITION,t,obj);
	(*GlobalItemInfo->param)(camId,LWIP_W_POSITION,t,cam);
	t = cam[0] - obj[0];
	d = t*t;
	t = cam[1] - obj[1];
	d += t*t;
	t = cam[2] - obj[2];
	d += t*t;
	return d;
}

#define		ARC_STEP		RADIANS(12.0)   //  30 segs = 360/12

void co_Arc(const LWCustomObjAccess *cobjAcc, double pos[3], double startAngle,double endAngle, double rad, int csys, int ax)
{
	double  endp[3], st[3], a, da = ARC_STEP;
	endp[yax[ax]] = rad*sin(RADIANS(startAngle));
	endp[xax[ax]] = rad*cos(RADIANS(startAngle));
	endp[ax] = pos[ax]; // endp[zax[ax]] = 0;
	st[ax] = pos[ax];
	for(a = RADIANS(startAngle) + da; a< RADIANS(endAngle); a += da)
	{
		st[xax[ax]] = endp[xax[ax]];
		st[yax[ax]] = endp[yax[ax]];
		endp[yax[ax]] = rad*sin(a);
		endp[xax[ax]] = rad*cos(a);
		(*cobjAcc->line)(cobjAcc->dispData, st,endp,csys);
	}
}



XCALL_(static void)VRML_ObjEval (VRML_ObjData *dat,	const LWCustomObjAccess *cobjAcc)
{
	double  v, vel[3], vec[3] = {0.0,0.0,0.0}, orig[3] = {0.0,0.0,0.0}, holdvec[3];
	float	rgb[] = {0.7f, 0.64f, 0.6f, 1.0f};
	int		ax = 1, i, t;
	XCALL_INIT;	

	if(dat->flags&VROF_LOD && (0.0 <= dat->LODRangeList[0]) )
	{
		char	buf[16]="";
		(*GlobalItemInfo->param)(dat->self, LWIP_W_POSITION, dat->time, orig);
		v = CameraDistanceSQ(dat->self, dat->time);
		if(v>dat->LODRangeList[0]*dat->LODRangeList[0])
		{
			i = 0;
			while( (0.0 <= dat->LODRangeList[i+1]) && ((v>=dat->LODRangeList[i+1]*dat->LODRangeList[i+1])) )
				i++;
		}
		else 
			i = -1;
		t = 0;
		while( dat->LODRangeList[t]>=0.0 && t<MAX_LOD_OBJECTS)
		{
			if(i==t)
			{
				(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_SOLID);
				if(cobjAcc->flags&LWCOFL_SELECTED)
					co_Arc(cobjAcc,orig,-175,175,(dat->LODRangeList[t]), LWCSYS_WORLD, 1);
				else
					co_Arc(cobjAcc,orig,0,360,(dat->LODRangeList[t]), LWCSYS_WORLD, 1);
			}
			else
			{
				(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_LONGDOT);
				if(cobjAcc->flags&LWCOFL_SELECTED)
					co_Arc(cobjAcc,orig,-175,175,(dat->LODRangeList[t]), LWCSYS_WORLD, 1);
				else
					co_Arc(cobjAcc,orig,0,360,(dat->LODRangeList[t]), LWCSYS_WORLD, 1);
			}
			if(cobjAcc->flags&LWCOFL_SELECTED)
			{
				VCPY(vec,orig);
				vec[2] -= dat->LODRangeList[t];
				sprintf(buf, "%.3f",dat->LODRangeList[t]);
				(*cobjAcc->text)(cobjAcc->dispData, vec,buf, 0, LWCSYS_WORLD);
			}
			t++;
		}
	}
	if(dat->flags&VROF_AUDIO)
	{
		for(i=0; i<20; i++)
		{
			VCPY(vec, orig);
			vec[2] += 0.5;
			v = i*PI*2;
			v *= 0.05; // 1/20
			vec[0] += 0.5*cos(v);
			vec[1] += 0.5*sin(v);
			VCPY(holdvec,vec);
			if(i) 
				(*cobjAcc->line)(cobjAcc->dispData, vel, vec, LWCSYS_ICON); // vel and vec are scaled by gridsize...
			VCPY(vel,holdvec);
		}
		VCPY(vec, orig);
		vec[0] += 0.5;
		vec[2] += 0.5;
		co_Line(cobjAcc, vel, vec, LWCSYS_ICON);


		co_Line(cobjAcc, orig, vec, LWCSYS_ICON);
		vec[0] -= 1.0;
		co_Line(cobjAcc, orig, vec, LWCSYS_ICON);
		vec[0] += 0.5;
		co_Line(cobjAcc, orig, vec, LWCSYS_ICON);
		vec[1] -= 0.5;
		co_Line(cobjAcc, orig, vec, LWCSYS_ICON);
		vec[1] += 1.0;
		co_Line(cobjAcc, orig, vec, LWCSYS_ICON);
		co_Rectangle(cobjAcc, orig, 1.0,1.0,LWCSYS_ICON,2);
		ax = 0;
		if(cobjAcc->flags&LWCOFL_SELECTED)
			while(ax<=10)
			{
				VSET(rgb,0.5f);
				rgb[1] += 0.2f;
				rgb[0] += (float)ax/20;
				rgb[2] += (float)(10.0f-(float)ax)/20;
				(*cobjAcc->setColor)(cobjAcc->dispData, rgb);
				for(i=0; i<9; i++)
				{
					VCPY(vec, orig);
					v = i*PI*0.5;
					v *= 0.125; // 1/8
					v -= PI*0.25;
					vec[2] += (0.5 + (double)ax/20) * cos(v);
					vec[1] += (0.5 + (double)ax/20) * sin(v);
					VCPY(holdvec,vec);
					if(i) 
						(*cobjAcc->line)(cobjAcc->dispData, vel, vec, LWCSYS_ICON); // vel and vec are scaled by gridsize...
					VCPY(vel,holdvec);
				}
				ax++;
				if(dat->volume<(double)ax/10)
					break;
			}
	}
	if(dat->flags &VROF_PROX)
	{
		if(cobjAcc->flags&LWCOFL_SELECTED)
			(*cobjAcc->setColor)(cobjAcc->dispData, rgb);
		(*GlobalItemInfo->param)(dat->self, LWIP_W_POSITION, dat->time, orig);
		co_Box(cobjAcc,orig,dat->radius,LWCSYS_WORLD);
	}
	if(dat->flags&VROF_GRID)
	{
		(*GlobalItemInfo->param)(dat->self, LWIP_W_POSITION, dat->time, orig);
		(*GlobalItemInfo->param)(dat->self, LWIP_SCALING, dat->time, vel);
		if(cobjAcc->flags&LWCOFL_SELECTED)
		{
			VSET(rgb,0.65f);
			rgb[1] = 0.25f;
			VSCL(rgb, 1.2f);
			(*cobjAcc->setColor)(cobjAcc->dispData, rgb);
		}
		vel[0] *= dat->sx;
		vel[2] *= dat->sz;
		co_Grid(cobjAcc, orig, vel,dat->nx,LWCSYS_WORLD,1);
	}
	if(cobjAcc->flags&LWCOFL_SELECTED)
	{
		if(dat->trig && (dat->flags&VROF_LINK) )
		{
			(*GlobalItemInfo->param)(dat->self, LWIP_W_POSITION, dat->time, holdvec);
			(*GlobalItemInfo->param)(dat->trig, LWIP_W_POSITION, dat->time, vel);
			(*cobjAcc->setColor)(cobjAcc->dispData, Violet);
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_LONGDOT);
			co_Line(cobjAcc, holdvec, vel, LWCSYS_WORLD);
			(*cobjAcc->setPattern)(cobjAcc->dispData, LWLPAT_SOLID);
		}
		if(*dat->URL)
		{
			VCLR(orig);
			orig[1] -= 0.075;
			(*cobjAcc->text)(cobjAcc->dispData, orig,dat->URL, 0, LWCSYS_ICON);
		}
	}

}

/* ----------------- Plug-in Activation  -----------------  */

XCALL_(int) CustomObj (
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
	if(!(GlobalSceneInfo=(*global)(LWSCENEINFO_GLOBAL,GFUSE_TRANSIENT)))
	{
		(*Gmessage->error)("Can't get global",LWSCENEINFO_GLOBAL);
		return AFUNC_BADGLOBAL;
	}

	local->inst->create  = VRML_ObjCreate;
	local->inst->destroy = VRML_ObjDestroy;
	local->inst->load    = VRML_ObjLoad;
	local->inst->save    = VRML_ObjSave;
	local->inst->copy    = VRML_ObjCopy;
	local->inst->descln	 = VRML_ObjDescribe;

	local->item->useItems    = VRML_ObjUseItems;
	local->item->changeID    = VRML_ObjChangeID;

	local->rend->init		= VRML_ObjInit;
	local->rend->newTime    = VRML_ObjNewTime;
	local->rend->cleanup	= VRML_ObjCleanup;

	local->evaluate 	= VRML_ObjEval;
	local->flags 		= VRML_ObjFlags;
	return (AFUNC_OK);
}



enum  {	CH_OFFSET = 0x8001, CH_HASLINK, CH_SIZE, CH_TYPE, CH_AUD, CH_VOL, CH_URL, CH_AURL, CH_LOOP, CH_LINK, 
	CH_STAB, CH_ATAB, CH_WTAB, CH_SGRP, CH_AGRP, CH_WGRP };

static LWXPanelControl ctrl_list[] = {
	{ CH_TYPE,		"Sensor Type",		"iPopChoice" },
	{ CH_AUD,		"Enable Sound",			"iBoolean" },
	{ CH_LINK,		"Link to Trigger",	"iBoolean" },
	{ CH_VOL,		"Volume",			"percent" },
	{ CH_SIZE,		"Range",			"distance3" },
	{ CH_URL,		"Web Link URL",		"string" },
	{ CH_AURL,		"Sound URL",		"string" },
	{ CH_LOOP,		"Loop",				"iBoolean" },
	{ CH_HASLINK,	"-hide-",			"integer" },
	{0}
};
static LWXPanelDataDesc data_descrip[] = {
	{ CH_TYPE,		"Sensor Type",		"integer" },
	{ CH_AUD,		"Enable Sound",			"integer" },
	{ CH_LINK,		"Link to Trigger",	"integer" },
	{ CH_VOL,		"Volume",			"percent" },
	{ CH_SIZE,		"Range",			"distance3" },
	{ CH_URL,		"Web Link URL",		"string" },
	{ CH_AURL,		"Sound URL",		"string" },
	{ CH_LOOP,		"Loop",				"integer" },
	{ CH_HASLINK,	"-hide-",			"integer" },
	{0},
};


static void *VRML_ObjData_get ( void *myinst, unsigned long vid ) 
{
	VRML_ObjData *dat = (VRML_ObjData*)myinst;
	void *result = NULL;
	static int	ival = 0;
	static double val = 0.0;

	if ( dat ) 
		switch ( vid ) {
			case CH_VOL:
				result = &dat->volume;
				break;
			case CH_URL:
				result = dat->URL;
				break;
			case CH_AURL:
				result = dat->AudURL;
				break;
			case CH_SIZE:
				result = &dat->radius;
				break;
			case CH_LINK:
				ival = dat->flags&VROF_LINK ? 1:0;
				result = &ival;
				break;
			case CH_LOOP:
				ival = dat->flags&VROF_ALOOP ? 1:0;
				result = &ival;
				break;
			case CH_AUD:
				ival = dat->flags&VROF_AUDIO ? 1:0;
				result = &ival;
				break;
			case CH_TYPE:
				result = &dat->type;
				break;
			case CH_HASLINK:
				ival = dat->hasLink ? 1:0;
				result = &ival;
				break;
	  } 
	return result;
}

static int VRML_ObjData_set ( void *myinst, unsigned long vid, void *value ) 
{
	VRML_ObjData *dat = (VRML_ObjData*)myinst;
	int rc=0;
	if ( dat ) 
	switch ( vid ) {
		case CH_URL:
			strncpy( dat->URL, (char*)value, sizeof(dat->URL) );
			rc = 1;
			setVRTag(dat, VTAGID_URL);
			break;
		case CH_AURL:
			strncpy( dat->AudURL, (char*)value, sizeof(dat->AudURL) );
			rc = 1;
			setVRTag(dat, VTAGID_SOUND);
			break;
		case CH_VOL:
			dat->volume = *((double*)value);
			rc = 1;
			setVRTag(dat, VTAGID_SOUND);
			break;
		case CH_SIZE:
			VCPY(dat->radius,(double*)value);
			rc = 1;
			setVRTag(dat, VTAGID_PROXIMITY);
			break;
		case CH_LINK:
			rc = *((int*)value);
			if(rc)
				dat->flags |= VROF_LINK;
			else
				dat->flags &= ~VROF_LINK;

			rc = 1;
			break;
		case CH_LOOP:
			rc = *((int*)value);
			if(rc)
				dat->flags |= VROF_ALOOP;
			else
				dat->flags &= ~VROF_ALOOP;

			rc = 1;
			break;
		case CH_AUD:
			rc = *((int*)value);
			if(rc)
			{
				dat->flags |= VROF_AUDIO;
				setVRTag(dat, VTAGID_SOUND);
			}
			else
				dat->flags &= ~VROF_AUDIO;
			rc = 1;
			break;
		case CH_HASLINK:
			rc = 1;
			break;
		case CH_TYPE:
			dat->type = *((int*)value);
			switch(dat->type)
			{
				case	VRO_TOUCH:
					setVRTag(dat, VTAGID_TOUCH);
					break;
				case	VRO_VIS:
					setVRTag(dat, VTAGID_VISIBILITY);
					break;
				case	VRO_PROX:
				default:
					dat->type = VRO_PROX;
					setVRTag(dat, VTAGID_PROXIMITY);
			}
			rc = 1;
		break;
	} 
	if(rc)
		(*GlobalLWUpdate)(LWCUSTOMOBJ_HCLASS, dat);

	return rc;
}

static int	volEnableMap[]		= {1,0,0,0,0,0,0,0,0,0,0}, 
			rangeEnableMap[]	= {0,1,0,0,0,0,0,0,0,0,0};

static LWXPanelID VRML_ObjXPanel(GlobalFunc *global, VRML_ObjData *dat)
{
	LWXPanelFuncs *lwxpf = NULL;
	LWXPanelID     panID = NULL;
	static LWXPanelHint hint[] = {
		XpDELETE(CH_HASLINK),
		XpLABEL(0,"VRML Property Display"),
		XpSTRLIST(CH_TYPE, vrObjTypes),
		XpGROUP_(CH_WGRP),
			XpH(CH_TYPE),
			XpH(CH_URL),
			XpEND,
		XpGROUP_(CH_SGRP),
			XpH(CH_AUD),
			XpH(CH_VOL),
			XpH(CH_AURL),
			XpH(CH_LOOP),
			XpEND,
		XpGROUP_(CH_AGRP),
			XpH(CH_LINK),
			XpH(CH_SIZE),
			XpEND,
		XpLABEL(CH_AGRP,"Sensor"),
		XpLABEL(CH_SGRP,"Sound"),
		XpTABS_(CH_STAB),
			XpH(CH_SGRP),
			XpH(CH_AGRP),
			XpEND,
		XpENABLEMSG_(CH_HASLINK,"No External Trigger Attached"),
			XpH(CH_LINK),
			XpEND,
		XpENABLE_(CH_AUD),
			XpH(CH_VOL),
			XpH(CH_AURL),
			XpH(CH_LOOP),
			XpEND,
		XpENABLE_MAP_(CH_TYPE,rangeEnableMap),
			XpH(CH_SIZE),
			XpEND,
		XpEND
	};

	lwxpf = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if ( lwxpf ) 
	{
		panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );
		if(panID) 
		{
			(*lwxpf->hint) ( panID, 0, hint );
			(*lwxpf->describe)( panID, data_descrip, VRML_ObjData_get, VRML_ObjData_set );
			(*lwxpf->viewInst)( panID, dat );
			(*lwxpf->setData)(panID, 0, dat);
			(*lwxpf->setData)(panID, CH_TYPE, dat);
	    }
    }
	return panID;
}

XCALL_(LWError)VRML_ObjCommand(VRML_ObjData *dat, const char *cmd)
{
	XCALL_INIT;
	return "Unknown Command";
}

XCALL_(int) CustomObj_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	XCALL_INIT;
	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);
	UI->panel	= VRML_ObjXPanel(global, UI->inst);
	UI->options	= NULL;
	UI->command	= VRML_ObjCommand; 
	return AFUNC_OK;
}

