/* TxtrChan.c -- A collection of Plugins using textures in different contexts.
 *
 *   Add a Texture to a channel, motion, image or environment using the texture global.
 *	 These are just 5 different implementations of the same plugin. 
 *   Instance functions are the same, but the evaluation and flags functions have different
 *   implementations for each plugin.
 *
 *	 Gregory Duquesne \ Arnie Cachelin.
 */

#include <lwserver.h>
#include <lwhost.h>
#include <lwhandler.h>
#include <lwmath.h>
#include <lwenvel.h>
#include <lwchannel.h>
#include <lwmotion.h>
#include <lwdisplce.h>
#include <lwrender.h>
#include <lwfilter.h>
#include <lwenviron.h>
#include <lwtxtr.h>
#include <lwtexture.h>
#include <lwtxtred.h>
#include <lwxpanel.h>
#include <lwpanel.h>
#include <lwmeshes.h>
#include <lwmeshedt.h>
#include <lwcmdseq.h>

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define VCPY_F(a,b)     ((a)[0] =(float)((b)[0]), (a)[1] =(float)((b)[1]), (a)[2] =(float)((b)[2]))
#define BIGNUM			2e+012;

static	LWPanelFuncs		*GlobalPanFun;
static	LWXPanelFuncs		*GlobalXPanFun;
static	LWMessageFuncs		*Gmessage;
static	LWTextureFuncs		*GlobalTextureFuncs;
static	LWTxtrEdFuncs		*GlobalTxtrEdFuncs;
static	LWObjectFuncs		*GlobalObjectFuncs;
static	LWInstUpdate		*GlobalLWUpdate = NULL;
static  GlobalFunc			*GGlobal=NULL;

LWItemInfo					*GlobalItemFuncs = NULL;
LWMeshInfo					*GlobalMeshesFuncs = NULL;

// subscription IDs for texture editors (one for each plugin class)
static	LWTECltID				chanTxtredID = NULL;
static	LWTECltID				motTxtredID = NULL;
static	LWTECltID				dspTxtredID = NULL;
static	LWTECltID				fltTxtredID = NULL;
static	LWTECltID				envTxtredID = NULL;
static	LWTECltID				vmapTxtredID = NULL;
// texture context for environment plugin class
static	LWTxtrContextID			dspCtxtID = NULL;
static	LWTxtrContextID			envCtxtID = NULL;
static	LWTxtrContextID			vmapCtxtID = NULL;
static	LWTxtrContextID			filtCtxtID = NULL;

/* 
We use the same data structure for the various plugins
*/
typedef struct st_TxtrData {
	void				*ctxt;
	void				*self,*ui;
	double				 offset, scale,bbox[3][2];
	int					 axis,type,vmidx,vtyp;
	char				 desc[100];
	LWTextureID			txtr, rgb_txtr;
	char				*name,vmname[256];
} TxtrData;

/*
This structure is used to allow gradients to access the image filters 
It is passed thru the mp->userData pointer.
*/
typedef struct st_gFilterData {
	int			x,y;
	LWFilterAccess		*filtAcc;
} gFilterData;

#define	CHANNEL				0
#define	MOTION				1
#define	FILTER				2
#define	ENVIRON				3
#define	VMAP				4
#define	DISPLACE			5

LWTxtrContextID		addDisplaceInputParams(LWTextureFuncs		*tf);
LWTxtrContextID		addFilterInputParams(void);


int		NormalDisplacement (long			 		version,
							GlobalFunc				*global,
							LWDisplacementHandler	*local,
							void					*serverData);
int		NormalDisplacement_UI (long		 	version,
							GlobalFunc		*global,
							LWInterface		*UI,
							void			*serverData);

// ----------------------------------------------------------- //

static int xax[] = {1,2,0}, yax[]={2,0,1}, zax[] = {0,1,2};

static char *errAllocFailed ="Tiny Allocation Failed, I don't feel so good";
static int	chanActivateCount = 0;
static int	motActivateCount = 0;
static int	dspActivateCount = 0;
static int	fltActivateCount = 0;
static int	envActivateCount = 0;
static int	vmapActivateCount = 0;

static LWFrame		gFrame = 0;
static LWTime		gTime = 0;

static		MeshEditOp		*op = NULL;

// ----------------------------------------------------------- //

	
void	initMP(LWMicropol	*mp)
{
	memset(mp,0,sizeof(LWMicropol));	
	mp->oXfrm[0] = mp->oXfrm[3] = mp->oXfrm[6] = 1;
	mp->wXfrm[0] = mp->wXfrm[3] = mp->wXfrm[6] = 1;
	mp->oScl[0] = mp->oScl[1] = mp->oScl[2] = 1;
}


static EDError vmapTxtrScan(
	TxtrData				*dat,
	const EDPointInfo		*pi)
{	 
	double		val[4] = {0,0,0,0}, trans,scale;
	float		v,pos[3],rgba[4],vmap[4];
	int			i,dim;
	LWMicropol	*mp;

	XCALL_INIT;	
	
	if (!op || !*dat->vmname)
		return	EDERR_BADARGS;

	if(!pi->flags&EDDF_SELECT)
		return EDERR_NONE;

	mp = dat->self;
	if (dat->scale)
		scale = 1/dat->scale;
	else
		scale = 1;
		
	for (i=0;i<3;i++)
	{
		mp->wPos[i] = pi->position[i];	
		mp->oPos[i] = pi->position[i]*scale;
		mp->vertsWPos[0][i] = (float)pi->position[i];
		pos[i] = 0.0f;

		if (dat->bbox[i][0]>pi->position[i])		//min
			dat->bbox[i][0] = pi->position[i];

		if (dat->bbox[i][1]<pi->position[i])	//max
			dat->bbox[i][1] = pi->position[i];
	}

	mp->verts[0] = pi->pnt;
	mp->weights[0] = 1.0f;
	mp->verts[1] = pi->pnt; // Using only 1 vertex causes getVerxUV to fail!!!
	mp->weights[1] = 0.0f; // could average these.. but why
	mp->verts[2] = pi->pnt; // Using only 2 vertices causes getVerxUV to fail!!!
	mp->weights[2] = 0.0f;
	mp->wPos[dat->axis] += dat->offset;
	mp->oPos[dat->axis] += dat->offset;
	mp->oAxis = mp->wAxis = dat->axis;

	switch(dat->vtyp)
	{
		case 0:
		case 1:
			VSET(vmap,0);
			vmap[3] = 0;
			break;
		case 2:
			VSET(vmap,0);
			vmap[3] = 0;
			if ((*op->pointVSet)(op->state, NULL ,LWID_('R','G','B','A'), dat->vmname))
				dim = (*op->pointVGet)(op->state, pi->pnt, vmap );	 
			break;
	}
	
	VCPY(val,vmap);

	switch(dat->vtyp)
	{
		case 0:
			trans = (*GlobalTextureFuncs->evaluate)(dat->txtr,mp,val);
			v = (float) val[0];
			(*op->pntVMap)(op->state, pi->pnt, LWVMAP_WGHT, dat->vmname, 1, &v );	 
			break;
		case 1:
			trans = (*GlobalTextureFuncs->evaluate)(dat->txtr,mp,val);
			v = (float) val[0];
			pos[dat->axis] = v;
			(*op->pntVMap)(op->state, pi->pnt, LWVMAP_MORF, dat->vmname, 3, pos );	 
			break;
		case 2:
			trans = (*GlobalTextureFuncs->evaluate)(dat->rgb_txtr,mp,val);
			VCPY_F(rgba, val);
			rgba[3] = (float) (1 - trans);
			(*op->pntVMap)(op->state, pi->pnt, LWID_('R','G','B','A'), dat->vmname, 4, rgba );	 
			break;
	}

	return EDERR_NONE;
}

TxtrEventFunc(LWTextureID	txtr, void	*userData,int	eventCode)
{
	TxtrData		*data;

	data= (*GlobalTextureFuncs->userData)(txtr);	
	
	if (eventCode == TXEV_ALTER && GlobalLWUpdate)
	{
		// Before evaluating the texture we need to initiase it
		// This is not necessary at render time, since 
		(*GlobalTextureFuncs->newtime)(txtr,gTime,gFrame);	
		
		// update layout for the plugin class
		if (data->type == CHANNEL)
			(*GlobalLWUpdate)(LWCHANNEL_HCLASS, data);	
		else if (data->type == MOTION)
			(*GlobalLWUpdate)(LWITEMMOTION_HCLASS, data);	
		else if (data->type == DISPLACE)
			(*GlobalLWUpdate)(LWDISPLACEMENT_HCLASS, data);	
		else if (data->type == FILTER)
			(*GlobalLWUpdate)(LWIMAGEFILTER_HCLASS, data);	
		else if (data->type == ENVIRON)
			(*GlobalLWUpdate)(LWENVIRONMENT_HCLASS, data);	
		
		// cleanup texture after evaluation
		(*GlobalTextureFuncs->cleanup)(txtr);		
	}

	if (data->type == VMAP && eventCode == TXEV_ALTER)
	{
		LWModCommand	*local;
		EDError			err;
		int				num;

		(*GlobalTextureFuncs->newtime)(txtr,gTime,gFrame);	

		local = data->ctxt;
		op = (*local->editBegin) (0, 0, OPSEL_USER); // CRASH here on exit, never unsubscribed...
		if (!op)
			return 0;

		num = (*op->pointCount) (op->state, OPLYR_FG, EDCOUNT_ALL);
		if(num)
			err = (*op->pointScan) (op->state, vmapTxtrScan, data, OPLYR_FG);

		(*op->done) (op->state, err, 0);

		// cleanup texture after evaluation
		(*GlobalTextureFuncs->cleanup)(txtr);		
	}

	return	1;
}

// This is the autosize callback. This is being called when the user hits
// the 'automatic sizing' button in the coordinate options.
// Note that the gradient autosize has a different callback.
int	TxtrAutoSizeFunc(LWTextureID	txtr, void	*userData,double	bbox[3][2])
{
	TxtrData		*data;

	data= (*GlobalTextureFuncs->userData)(txtr);	
	memcpy(bbox,data->bbox,6*sizeof(double));	
	return	1;
}

int	GradientAutoSizeFunc(LWTxtrParamDesc	*param,int	paramNb,void	*userData)
{
	TxtrData		*data;
	double			d[3];

	data= userData;	
	switch (paramNb)
	{
	case	1: // X
		param->end = 0.5*fabs(data->bbox[0][1] - data->bbox[0][0]);
		return	1;
	case	2: // Y
		param->end = 0.5*fabs(data->bbox[1][1] - data->bbox[1][0]);
		return	1;
	case	3: // Z
		param->end = 0.5*fabs(data->bbox[2][1] - data->bbox[2][0]);
		return	1;
	case	4: // Distance to center
		d[0] = data->bbox[0][1] - data->bbox[0][0];
		d[1] = data->bbox[1][1] - data->bbox[1][0];
		d[2] = data->bbox[2][1] - data->bbox[2][0];
		param->end = 0.5*sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
		return	1;
	}

	return	0;
}

XCALL_(void)	TxtrNewtime (TxtrData *inst,LWFrame	f,LWTime	t)
{
	XCALL_INIT;

	gTime = t;
	gFrame = f;
	return;
}


/* ----------------- Plug-in Methods: LWInstanceFuncs  -----------------  */


XCALL_(static LWInstance)TxtrChanCreate(void *data, LWChannelID chan, LWError *err)
{
	TxtrData *dat=NULL;
	XCALL_INIT;
	
	if(dat=malloc(sizeof(TxtrData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->offset = 0.0;
		dat->scale = 1.0;
		dat->self = chan;
		dat->ctxt = data;
	//	dat->name = NULL;
		dat->type = CHANNEL;
		// create texture (of scalar type)
		dat->txtr = (*GlobalTextureFuncs->create)(TRT_SCALAR,"TextureChannel",NULL,dat);

		sprintf(dat->desc," Scale: %.2f", dat->scale);
	}
	else
		err = &errAllocFailed;

	chanActivateCount ++;
	return dat;
}

XCALL_(static LWInstance)TxtrMotionCreate(void *data, LWItemID item, LWError *err)
{
	TxtrData *dat=NULL;
	XCALL_INIT;
	
	if(dat=malloc(sizeof(TxtrData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->offset = 0.0;
		dat->scale = 1.0;
		dat->self = (void *)item;
		dat->ctxt = data;
	//	dat->name = NULL;
		dat->type = MOTION;
		dat->axis = 1;
		// create texture (of scalar type)
		dat->txtr = (*GlobalTextureFuncs->create)(TRT_SCALAR,"TextureMotion",NULL,dat);
		(*GlobalTextureFuncs->setEnvGroup)(dat->txtr,(*GlobalItemFuncs->chanGroup)(item));
	
		sprintf(dat->desc," Scale: %.2f", dat->scale);
	}
	else
		err = &errAllocFailed;

	motActivateCount ++;
	return dat;
}

XCALL_(static LWInstance)TxtrDisplaceCreate(void *data, LWItemID item, LWError *err)
{
	TxtrData *dat=NULL;
	XCALL_INIT;
	
	if(dat=malloc(sizeof(TxtrData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->offset = 0.0;
		dat->scale = 1.0;
		dat->self = (void *)item;
		dat->ctxt = data;
	//	dat->name = NULL;
		dat->type = DISPLACE;
		dat->axis = 1;
		if (!dspCtxtID)
			dspCtxtID = addDisplaceInputParams(GlobalTextureFuncs);
		// create texture (of displacement type)
		dat->txtr = (*GlobalTextureFuncs->create)(TRT_DISPLACEMENT,"DisplacementTexture",dspCtxtID,dat);
		(*GlobalTextureFuncs->setEnvGroup)(dat->txtr,(*GlobalItemFuncs->chanGroup)(item));
	
		sprintf(dat->desc,"");
	}
	else
		err = &errAllocFailed;

	dspActivateCount ++;
	return dat;
}

XCALL_(static LWInstance)TxtrFilterCreate(void *data, void* ctxt, LWError *err)
{
	TxtrData *dat=NULL;
	XCALL_INIT;

	
	if(dat=malloc(sizeof(TxtrData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->offset = 0.0;
		dat->scale = 1.0;
		dat->self = ctxt;
		dat->ctxt = data;
	//	dat->name = NULL;
		dat->type = FILTER;
		dat->axis = 2;
		if (!filtCtxtID)
			filtCtxtID = addFilterInputParams();
		// create texture (of color type)
		dat->txtr = (*GlobalTextureFuncs->create)(TRT_COLOR,"TextureFilter",filtCtxtID,dat);
	
		sprintf(dat->desc," Scale: %.2f", dat->scale);
	}
	else
		err = &errAllocFailed;

	fltActivateCount ++;
	return dat;
}

LWTxtrContextID		addEnvironInputParams(void);

XCALL_(static LWInstance)TxtrEnvironCreate(void *data, void* ctxt, LWError *err)
{
	TxtrData *dat=NULL;
	XCALL_INIT;
	
	if(dat=malloc(sizeof(TxtrData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->offset = 0.0;
		dat->scale = 1.0;
		dat->self = ctxt;
		dat->ctxt = data;
	//	dat->name = NULL;
		dat->type = ENVIRON;
		dat->axis = 2;

		// create a texture context for additional gradient input parameters
		if (!envCtxtID)
			envCtxtID = addEnvironInputParams();
		// create texture (of color type) using the texture context
		dat->txtr = (*GlobalTextureFuncs->create)(TRT_COLOR,"TextureEnvironment",envCtxtID,dat);
	
		sprintf(dat->desc," Scale: %.2f", dat->scale);
	}
	else
		*err = errAllocFailed;

	envActivateCount ++;
	return dat;
}



XCALL_(static void)TxtrChanDestroy(TxtrData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		chanActivateCount --;
		if (!chanActivateCount && GlobalTxtrEdFuncs && chanTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(chanTxtredID);
			chanTxtredID = NULL;		
		}

		(*GlobalTextureFuncs->destroy)(dat->txtr);
		free(dat);
	}
}

XCALL_(static void)TxtrMotionDestroy(TxtrData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		motActivateCount --;
		if (!motActivateCount && GlobalTxtrEdFuncs && motTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(motTxtredID);
			motTxtredID = NULL;		
		}

		(*GlobalTextureFuncs->destroy)(dat->txtr);
		free(dat);
	}
}

XCALL_(static void)TxtrDisplaceDestroy(TxtrData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		dspActivateCount --;
		if (!dspActivateCount && GlobalTxtrEdFuncs && dspTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(dspTxtredID);
			dspTxtredID = NULL;		
		}

		(*GlobalTextureFuncs->destroy)(dat->txtr);
		free(dat);
	}
}

XCALL_(static void)TxtrFilterDestroy(TxtrData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		fltActivateCount --;
		if (!fltActivateCount && GlobalTxtrEdFuncs && fltTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(fltTxtredID);
			fltTxtredID = NULL;
		}

		(*GlobalTextureFuncs->destroy)(dat->txtr);
		free(dat);
	}
}

XCALL_(static void)TxtrEnvironDestroy(TxtrData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		envActivateCount --;
		if (!envActivateCount && GlobalTxtrEdFuncs && envTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(envTxtredID);
			envTxtredID = NULL;
		}

		if (!envActivateCount && GlobalTextureFuncs && envCtxtID)
		{
			(*GlobalTextureFuncs->contextDestroy)(envCtxtID);
			envCtxtID = NULL;
		}

		(*GlobalTextureFuncs->destroy)(dat->txtr);
		free(dat);
	}
}


XCALL_(static LWError)TxtrChanCopy(TxtrData	*to, TxtrData	*from)
{
	LWTextureID		txtr;
	XCALL_INIT;

	(*GlobalTextureFuncs->copy)(to->txtr,from->txtr);

	txtr = to->txtr;
	*to = *from;
	to->txtr = txtr;

	return (NULL);
}

XCALL_(static LWError)TxtrChanLoad(TxtrData *inst,const LWLoadState	*lState)
{
	float	fp;
	int		ax;
	XCALL_INIT;

	LWLOAD_FP(lState,&fp,1);
	inst->scale = fp;
	LWLOAD_FP(lState,&fp,1);
	inst->offset = fp;
	LWLOAD_I4(lState,&ax,1);
	inst->axis = ax;

	(*GlobalTextureFuncs->load)(inst->txtr,lState);
	return (NULL);
}

XCALL_(LWError)TxtrChanSave(TxtrData *inst,const LWSaveState	*sState)
{
	float	fp;
	XCALL_INIT;

	fp = (float)inst->scale;
	LWSAVE_FP(sState,&fp,1);
	fp = (float)inst->offset;
	LWSAVE_FP(sState,&fp,1);
	LWSAVE_I4(sState,&inst->axis,1);

	(*GlobalTextureFuncs->save)(inst->txtr,sState);
	return (NULL);
}

XCALL_(static const char *)TxtrChanDescribe (TxtrData *inst)
{
	TxtrData *dat = (TxtrData *)inst;
	XCALL_INIT;
	
	sprintf(dat->desc," Scale: %.2f",dat->scale);
	return (dat->desc);
}


/* ----------------- Plug-in Methods: LWMotionHandler  -----------------  */

XCALL_(static unsigned int)TxtrMotionFlags (TxtrData *inst)
{
	XCALL_INIT;

	return 0;
}
	
XCALL_(static void) TxtrMotionEval (TxtrData *dat,const LWItemMotionAccess *motAcc)
{
	double		val = 0.0,pos[3];
	LWMicropol	mp;
	int			i;

	XCALL_INIT;	

	/* fill-in the micropolygon structure 
	*/
	initMP(&mp);
	gTime = motAcc->time;

	(*motAcc->getParam)(LWIP_W_POSITION,motAcc->time ,pos);
	for (i=0;i<3;i++)
		mp.wPos[i] = pos[i];

	(*motAcc->getParam)(LWIP_POSITION,motAcc->time ,pos);
	for (i=0;i<3;i++)
		mp.oPos[i] = pos[i];

	mp.gNorm[dat->axis] = mp.wNorm[dat->axis] = 1;
	mp.oAxis = mp.wAxis = dat->axis;
	// there is no way we can determine what the spot size is in this context
	// so we just set it to a small value that won't affect image antialiasing.
	mp.spotSize = 0.001;
	mp.txVal = pos[dat->axis];
	
	// evaluate the texture
	(*GlobalTextureFuncs->evaluate)(dat->txtr,&mp,&val);
	// set item's position with new value
	pos[dat->axis] += dat->scale*val + dat->offset;	
	(*motAcc->setParam)(LWIP_POSITION, pos);
}

/* ----------------- Plug-in Methods: LWMotionHandler  -----------------  */

XCALL_(static unsigned int)TxtrDisplaceFlags (TxtrData *inst)
{
	XCALL_INIT;
	
	return	LWDMF_WORLD;
}


XCALL_(static void) TxtrDisplaceEval (TxtrData *dat,LWDisplacementAccess *dspAcc)
{
	double		vec[3];
	LWMicropol	mp;
	int			i;

	XCALL_INIT;	

	/* fill-in the micropolygon structure 
	*/
	initMP(&mp);

	for (i=0;i<3;i++)
	{
		mp.wPos[i] = dspAcc->source[i];
		mp.oPos[i] = dspAcc->oPos[i];
		mp.vertsWPos[0][i] = (float)dspAcc->source[i];

		if (dat->bbox[i][0]>dspAcc->oPos[i])		//min
			dat->bbox[i][0] = dspAcc->oPos[i];

		if (dat->bbox[i][1]<dspAcc->oPos[i])	//max
			dat->bbox[i][1] = dspAcc->oPos[i];
	}

	mp.verts[0] = dspAcc->point;
	mp.weights[0] = 1.0f;

	mp.gNorm[dat->axis] = mp.wNorm[dat->axis] = 1;
	mp.oAxis = mp.wAxis = dat->axis;
	mp.spotSize = 0.001;

	GlobalMeshesFuncs = dspAcc->info;
	
	// evaluate the texture
	VSET(vec,0);
	(*GlobalTextureFuncs->evaluate)(dat->txtr,&mp,vec);
	for (i=0;i<3;i++)
		dspAcc->source[i] += vec[i];
}


/* ----------------- Plug-in Methods: LWChannelHandler  -----------------  */

XCALL_(static unsigned int)TxtrChanFlags (TxtrData *inst)
{
	XCALL_INIT;

	return 0;
}	

XCALL_(static void)TxtrChanEval (TxtrData *dat,	const LWChannelAccess *chanAcc)
{
	double		val = 0.0, value;
	LWMicropol	mp;

	XCALL_INIT;	

	gFrame = chanAcc->frame;
	gTime = chanAcc->time;

	value = chanAcc->value;
	/* fill-in the micropolygon structure 
	*/
	initMP(&mp);
	mp.wPos[dat->axis] = mp.oPos[dat->axis] = gTime;	
	mp.gNorm[dat->axis] = mp.wNorm[dat->axis] = 1;
	mp.oAxis = mp.wAxis = dat->axis;
	mp.spotSize = 0.001;
	mp.txVal = gTime;

	(*GlobalTextureFuncs->evaluate)(dat->txtr,&mp,&val);

	val *= dat->scale;
	val += dat->offset;
	val += chanAcc->value;
	(*chanAcc->setChannel)(chanAcc->chan, val);
}

/* ----------------- Plug-in Methods: LWFilterlHandler  -----------------  */

XCALL_(static unsigned int)TxtrFiltFlags (TxtrData *inst)
{
	int		flags = 0;
	XCALL_INIT;
	
	flags = (1 << LWBUF_SPECIAL) | (1 << LWBUF_LUMINOUS) | (1 << LWBUF_DIFFUSE) | (1 << LWBUF_SPECULAR);
	flags |= (1 << LWBUF_MIRROR) | (1 << LWBUF_TRANS) | (1 << LWBUF_SHADING);
	flags |= (1 << LWBUF_SHADOW) | (1 << LWBUF_DEPTH) | (1 << LWBUF_DIFFSHADE) | (1 << LWBUF_SPECSHADE);
	return flags;
}	

static	int		rec = 0;

XCALL_(static void)TxtrFiltProcess (TxtrData *dat,	const LWFilterAccess *filtAcc)
{
	double		val = 0.0, RGB[3],trans;
	LWMicropol	mp;
	gFilterData	acc;
	int		x,y;
	float		*r,*g,*b, *a, rgb[3], sx,sy;
	XCALL_INIT;	

	gFrame = filtAcc->frame;
	if (rec)
		return;
	else
		rec++;

	for(y=0; y<filtAcc->height; y++)
	{
		r = (*filtAcc->getLine)(LWBUF_RED, y);
		g = (*filtAcc->getLine)(LWBUF_GREEN, y);
		b = (*filtAcc->getLine)(LWBUF_BLUE, y);
		a = (*filtAcc->getLine)(LWBUF_ALPHA, y);
		if (!r || !g || !b || !a)
		{
			rec--;
			return;
		}

		for(x=0; x<filtAcc->width; x++)
		{
			rgb[0] = r[x];
			rgb[1] = g[x];
			rgb[2] = b[x];
			(*filtAcc->setRGB)(x,y,rgb);
			(*filtAcc->setAlpha)(x,y,a[x]);
		}
	}

	initMP(&mp);
	acc.filtAcc = filtAcc;
	mp.userData = &acc;

	mp.wPos[dat->axis] = mp.oPos[dat->axis] = dat->offset;
	if (dat->scale)
	{
		sx = (float)(1.0/filtAcc->width/dat->scale);
		sy = (float)(1.0/filtAcc->height/dat->scale);
	}
	else
	{
		sx = (float)(1.0/filtAcc->width);
		sy = (float)(1.0/filtAcc->height);
	}

	for(y=0; y<filtAcc->height; y++)
	{
		r = (*filtAcc->getLine)(LWBUF_RED, y);
		g = (*filtAcc->getLine)(LWBUF_GREEN, y);
		b = (*filtAcc->getLine)(LWBUF_BLUE, y);
		if (!r || !g || !b)
		{
			rec--;
			return;
		}

		for(x=0; x<filtAcc->width; x++)
		{
			acc.x = x;
			acc.y = y;

			mp.wPos[xax[dat->axis]] = mp.oPos[xax[dat->axis]] = (float)x*sx - .5f;	
			mp.wPos[yax[dat->axis]] = mp.oPos[yax[dat->axis]] = (float)-y*sy + .5f;
			mp.wNorm[dat->axis] = 1;
		//	mp.gNorm[dat->axis] = 1;

			mp.oAxis = mp.wAxis = dat->axis;
			mp.spotSize = 1/filtAcc->width;
			mp.txVal = 0.33333*(r[x]+g[x]+b[x]); // should be LUMA()?

			RGB[0] = r[x];	RGB[1] = g[x];	RGB[2] = b[x];
			VCPY(mp.gNorm,RGB); // use color for Normal? Cheating? will it hurt anything?

			// the original color is passed to the evaluation function
			// this color will be modified with the texture color affected
			// by its alpha channel.
			trans = (*GlobalTextureFuncs->evaluate)(dat->txtr,&mp,RGB);
			rgb[0] = (float)(RGB[0]);
			rgb[1] = (float)(RGB[1]);
			rgb[2] = (float)(RGB[2]);
			(*filtAcc->setRGB)(x,y,rgb);
		}
	}

	rec--;
}

	
/* ----------------- Plug-in Methods: LWEnvironment  -----------------  */

XCALL_(static unsigned int)TxtrEnvironFlags (TxtrData *inst)
{
	XCALL_INIT;

	return 0;
}	

XCALL_(static LWError)TxtrEnvironProcess (TxtrData *dat, LWEnvironmentAccess *envAcc)
{
	double		val = 0.0, trans,scale;
	LWMicropol	mp;
	int			i;

	XCALL_INIT;	

	if (dat->scale)
		scale = 1/dat->scale;
	else
		scale = 1;

	initMP(&mp);
	for (i=0;i<3;i++)
	{
		mp.wPos[i] = envAcc->dir[i]*scale;	
		mp.oPos[i] = envAcc->dir[i]*scale;
		mp.gNorm[i] = mp.wNorm[i] = envAcc->dir[i];
	}
	mp.wPos[dat->axis] += dat->offset;
	mp.oPos[dat->axis] += dat->offset;
	mp.oAxis = mp.wAxis = dat->axis;

	envAcc->color[0] = 0;
	envAcc->color[1] = 0;
	envAcc->color[2] = 0;
	trans = (*GlobalTextureFuncs->evaluate)(dat->txtr,&mp,envAcc->color);

	return	NULL;
}

// Environment gradient input parameter funcs:
// 1/ set an init function
static gParamData	envInitParam(void	*userData,LWTxtrParamDesc	*param,int	paramNb,LWTime	t,LWFrame	f)
{
	return	NULL;	
}
// 2/ set a cleanup function
static void	envCleanupParam(LWTxtrParamDesc	*param,int	paramNb,gParamData	data)
{
	return;	
}
// 3/ evaluate parameter by switching on input parameter.
// Note that parameter 0 is "previous layer", this parameter always exists.
static double	envEvalParam(LWTxtrParamDesc	*param,int	paramNb,LWMicropol	*mp,gParamData		data)
{
	double		t=0,vec[3],norm;

	vec[0] = mp->wPos[0];
	vec[1] = mp->wPos[1];
	vec[2] = mp->wPos[2];
	norm = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
	if (norm)
	{
		vec[0] /= norm;
		vec[1] /= norm;
		vec[2] /= norm;
	}

	switch (paramNb)
	{
	case	1:	// heading
		return	atan2(vec[0],vec[2]);
	case	2:	// pitch
		return	-asin(vec[1]);
	}

	return	t;
}

// This is where we create the texture context. It is made of the funcs we created
// below plus some input parameter descriptions.
LWTxtrContextID		addEnvironInputParams(void)
{
	LWTxtrParamFuncs		envCtxtFuncs;
	LWTxtrContextID			ctxt;

	static LWTxtrParamDesc	envDesc[] ={
		{"Heading",-3.1415,3.1415,LWIPT_ANGLE,LWGF_FIXED_START | LWGF_FIXED_END,LWGI_NONE},
		{"Pitch",-1.5707,1.5707,LWIPT_ANGLE,LWGF_FIXED_START | LWGF_FIXED_END,LWGI_NONE},
		{NULL,0,10,0,0,0,NULL}
	};

	// set gradient funcs
	envCtxtFuncs.paramEvaluate = envEvalParam;
	envCtxtFuncs.paramTime = envInitParam;
	envCtxtFuncs.paramCleanup = envCleanupParam;
	// create texture context
	ctxt = (*GlobalTextureFuncs->contextCreate)(envCtxtFuncs);
	// add gradient input params
	(*GlobalTextureFuncs->contextAddParam)(ctxt,envDesc[0]);
	(*GlobalTextureFuncs->contextAddParam)(ctxt,envDesc[1]);

	return	ctxt;
}


// Displacement gradient input parameter funcs:

#define		DCM_CAMDIST				1
#define		DCM_DIST				2
#define		DCM_DISTX				3
#define		DCM_DISTY				4
#define		DCM_DISTZ				5
#define		DCM_VMAP				6

static double		getVerxValue(char	*vmapName,LWMicropol	*mp)
{
	void			*vmap=NULL;
	LWPntID			v = NULL;
	float			vm0[3];
	double			val = 0;
	int				i,r;

	vmap = (*GlobalMeshesFuncs->pntVLookup)(GlobalMeshesFuncs,LWVMAP_WGHT,vmapName);		
	if (!vmap)
        return	0;
	else
		(*GlobalMeshesFuncs->pntVSelect)(GlobalMeshesFuncs,vmap);		

	for (i=0;i<4;i++)
	{
		v =	mp->verts[i];
		if (v)
		{
			r = (*GlobalMeshesFuncs->pntVGet)(GlobalMeshesFuncs,v,vm0);		
			if (r)
				val += vm0[0]*mp->weights[i]; 
		}
	}

	return	val;
}

// Same function, for modeler
static double		getVerxValueMod(char	*vmapName,LWMicropol	*mp)
{
	void			*vmap=NULL;
	LWPntID			v = NULL;
	float			vm0[3];
	double			val = 0;
	int				i,r;

	vmap = (*op->pointVSet) (op->state, NULL, LWVMAP_WGHT, vmapName);
	if (!vmap)
        return	0;	

	for (i=0;i<4;i++)
	{
		v =	mp->verts[i];
		if (v)
		{
			r = (*op->pointVGet)(op->state,v,vm0);		
			if (r)
				val += vm0[0]*mp->weights[i]; 
		}
	}

	return	val;
}

static double		gdistance(double	*a,double	*b)
{
	double	c[3] = {0,0,0};

	if (a)
	{
		c[0] = b[0]-a[0];
		c[1] = b[1]-a[1];
		c[2] = b[2]-a[2];
	}
	else
	{
		c[0] = b[0];
		c[1] = b[1];
		c[2] = b[2];
	}

	return (sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]));
}

static gParamData	dspInitParam(void	*userData,LWTxtrParamDesc	*param,int	paramNb,LWTime	t,LWFrame	f)
{
	double	*pos;

	if (paramNb>DCM_VMAP)
		return 0;
	
	switch (paramNb)
	{
	case	DCM_DIST:
	case	DCM_DISTX:
	case	DCM_DISTY:
	case	DCM_DISTZ:
	case	DCM_CAMDIST:
		pos = malloc(3*sizeof(double));	// item's position
		if (param->itemID)
			(*GlobalItemFuncs->param)(param->itemID,LWIP_W_POSITION,t,pos);
		else
		{
			pos[0]=pos[1]=pos[2]= 0;
		}
		return	pos;
	default:
		return	NULL;
	}	
}
static void	dspCleanupParam(LWTxtrParamDesc	*param,int	paramNb,gParamData	data)
{
	if (paramNb>DCM_VMAP)
		return;
	
	switch (paramNb)
	{
	case	DCM_DIST:
	case	DCM_DISTX:
	case	DCM_DISTY:
	case	DCM_DISTZ:
	case	DCM_CAMDIST:
		if (data)
			free(data);
		return;
	default:
		return;
	}	
}
static double	dspEvalParam(LWTxtrParamDesc	*param,int	paramNb,LWMicropol	*mp,void		*data)
{
	double		t=0;
	double		*oPos;

	if (paramNb>DCM_VMAP)
		return 0;
	
	switch (paramNb)
	{
	case	DCM_DIST:	// distance to object
		if (!data)
			return	0;
		oPos = data;
		t = gdistance(oPos,mp->wPos);
		break;
	case	DCM_DISTX:	// distance X
		if (!data)
			return	0;
		oPos = data;
		if (oPos)
			t = fabs(mp->wPos[0] - oPos[0]);
		else
			t = fabs(mp->wPos[0]);
		break;
	case	DCM_DISTY:	// distance Y
		if (!data)
			return	0;
		oPos = data;
		if (oPos)
			t = fabs(mp->wPos[1] - oPos[1]);
		else
			t = fabs(mp->wPos[1]);
		break;
	case	DCM_DISTZ:	// distance Z
		if (!data)
			return	0;
		oPos = data;
		if (oPos)
			t = fabs(mp->wPos[2] - oPos[2]);
		else
			t = fabs(mp->wPos[2]);
		break;
	case	DCM_CAMDIST: // distance to camera
		t = gdistance(mp->wPos,mp->raySource);
		break;
	case	DCM_VMAP: // vmap
		if (!param->itemName)
			return	0;
		t = getVerxValue(param->itemName,mp);
		break;
	}

	return	t;
}

LWTxtrContextID		addDisplaceInputParams(LWTextureFuncs		*tf)
{
	LWTxtrParamFuncs		dspCtxtFuncs;
	LWTxtrContextID			ctxt;

	static LWTxtrParamDesc	dspDesc[] ={
		{"Distance to Camera",0,10,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_NONE},
		{"Distance to Object",0,10,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_OBJECT},
		{"X Distance to Object",0,10,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_OBJECT},
		{"Y Distance to Object",0,10,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_OBJECT},
		{"Z Distance to Object",0,10,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_OBJECT},
		{"Weight Map",-1,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_VMAP},
		{NULL,0,10,0,0,0,NULL}
	};

	GlobalTextureFuncs = tf;
	// set gradient funcs
	dspCtxtFuncs.paramEvaluate = dspEvalParam;
	dspCtxtFuncs.paramTime = dspInitParam;
	dspCtxtFuncs.paramCleanup = dspCleanupParam;
	// create texture context
	ctxt = (*GlobalTextureFuncs->contextCreate)(dspCtxtFuncs);
	// add gradient input params
	(*GlobalTextureFuncs->contextAddParam)(ctxt,dspDesc[0]);
	(*GlobalTextureFuncs->contextAddParam)(ctxt,dspDesc[1]);
	(*GlobalTextureFuncs->contextAddParam)(ctxt,dspDesc[2]);
	(*GlobalTextureFuncs->contextAddParam)(ctxt,dspDesc[3]);
	(*GlobalTextureFuncs->contextAddParam)(ctxt,dspDesc[4]);
	(*GlobalTextureFuncs->contextAddParam)(ctxt,dspDesc[5]);

	return	ctxt;
}


// VMap gradient input parameter funcs:
static gParamData	vmapInitParam(void	*userData,LWTxtrParamDesc	*param,int	paramNb,LWTime	t,LWFrame	f)
{
	return	NULL;	
}
static void	vmapCleanupParam(LWTxtrParamDesc	*param,int	paramNb,gParamData	data)
{
	return;	
}
static double	vmapEvalParam(LWTxtrParamDesc	*param,int	paramNb,LWMicropol	*mp,gParamData		data)
{
	double		norm;

	switch (paramNb)
	{
	case	1:	// X
		return	fabs(mp->wPos[0]);
	case	2:	// Y
		return	fabs(mp->wPos[1]);
	case	3:	// Z
		return	fabs(mp->wPos[2]);
	case	4:	// Distance to center
		norm = sqrt(mp->wPos[0]*mp->wPos[0] + mp->wPos[1]*mp->wPos[1] + mp->wPos[2]*mp->wPos[2]);
		return	norm;
	case	5: // vmap
		if (!param->itemName)
			return	0;
		return getVerxValueMod(param->itemName,mp);
	default:
		return	0;
	}
}

LWTxtrContextID		addVMapInputParams(void)
{
	LWTxtrParamFuncs		vmapCtxtFuncs;
	LWTxtrContextID			ctxt;
	int						i = 0;

	static LWTxtrParamDesc	vmapDesc[] ={
		{"X",0,1,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_NONE},
		{"Y",0,1,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_NONE},
		{"Z",0,1,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_NONE},
		{"Distance to center",0,1,LWIPT_DISTANCE,LWGF_FIXED_START | LWGF_AUTOSIZE,LWGI_NONE},
		{"Weight Map",-1,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_VMAP},
		{NULL,0,10,0,0,0,NULL}
	};

	// set gradient funcs
	vmapCtxtFuncs.paramEvaluate = vmapEvalParam;
	vmapCtxtFuncs.paramTime = vmapInitParam;
	vmapCtxtFuncs.paramCleanup = vmapCleanupParam;
	// create texture context
	ctxt = (*GlobalTextureFuncs->contextCreate)(vmapCtxtFuncs);
	// add gradient input params
	while (vmapDesc[i].name)
		(*GlobalTextureFuncs->contextAddParam)(ctxt,vmapDesc[i++]);

	return	ctxt;
}


// Filter gradient input parameter funcs:


static gParamData filterInitParam(void *userData, LWTxtrParamDesc *param, int paramNb, LWTime t, LWFrame f)
{
	return	NULL;	
}
static void	filterCleanupParam(LWTxtrParamDesc *param, int paramNb, gParamData data)
{
	return;	
}
static double	filterEvalParam(LWTxtrParamDesc *param, int paramNb, LWMicropol *mp, gParamData data)
{
	LWFilterAccess		*filtAcc;
	gFilterData		*acc;
	float			*lin;

	acc = mp->userData;
	filtAcc = acc->filtAcc;

	switch (paramNb)
	{
	case	1:	// X
		return	 mp->oPos[xax[mp->oAxis]];
	case	2:	// Y
		return	mp->oPos[yax[mp->oAxis]]; //mp->wPos[1];
	case	3:	// V
		return	mp->txVal;
	case	4:	// R
		return	mp->gNorm[0]; // cheating to store rgb here?
	case	5:	// G
		return	mp->gNorm[1];
	case	6:	// B
		return	mp->gNorm[2];
	case	7:	// 
		lin = (*filtAcc->getLine)(LWBUF_SPECIAL, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	8:	// 
		lin = (*filtAcc->getLine)(LWBUF_LUMINOUS, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	9:	// 
		lin = (*filtAcc->getLine)(LWBUF_DIFFUSE, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	10:	// 
		lin = (*filtAcc->getLine)(LWBUF_SPECULAR, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	11:	// 
		lin = (*filtAcc->getLine)(LWBUF_MIRROR, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	12:	// 
		lin = (*filtAcc->getLine)(LWBUF_TRANS, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	13:	// 
		lin = (*filtAcc->getLine)(LWBUF_SHADING, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	14:	// 
		lin = (*filtAcc->getLine)(LWBUF_SHADOW, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	15:	// 
		lin = (*filtAcc->getLine)(LWBUF_DEPTH, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	16:	// 
		lin = (*filtAcc->getLine)(LWBUF_DIFFSHADE, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	case	17:	// 
		lin = (*filtAcc->getLine)(LWBUF_SPECSHADE, acc->y);
		if (!lin)
			return 0;
		else
			return	(double)lin[acc->x];
	default:
		return	0;
	}
}


LWTxtrContextID		addFilterInputParams(void)
{
	static LWTxtrParamFuncs		filtCtxtFuncs;
	static LWTxtrContextID			ctxt;
	int						i = 0;

	static LWTxtrParamDesc	filtDesc[] ={
		{"Image X",-0.5,0.5,LWIPT_FLOAT,LWGF_FIXED_START,LWGI_NONE},
		{"Image Y",-0.5,0.5,LWIPT_FLOAT,LWGF_FIXED_START,LWGI_NONE},
		{"Brightness",0,1.0,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Red",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Green",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Blue",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Special",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Luminous",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Diffuse",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Specular",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Mirror",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Trans",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Shading",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Shadow",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Depth",0,1,LWIPT_DISTANCE,LWGF_FIXED_START,LWGI_NONE},
		{"Diffuse Shading",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{"Specular Shading",0,1,LWIPT_PERCENT,LWGF_FIXED_START,LWGI_NONE},
		{NULL,0,10,0,0,0,NULL}
	};

	// set gradient funcs
	filtCtxtFuncs.paramEvaluate = filterEvalParam;
	filtCtxtFuncs.paramTime = filterInitParam;
	filtCtxtFuncs.paramCleanup = filterCleanupParam;
	// create texture context
	ctxt = (*GlobalTextureFuncs->contextCreate)(filtCtxtFuncs);
	// add gradient input params
	while (filtDesc[i].name)
		(*GlobalTextureFuncs->contextAddParam)(ctxt,filtDesc[i++]);

	return	ctxt;
}



/* ----------------- Plug-in Activation  -----------------  */

XCALL_(static int) TxtrChannel (
	long			 	 version,
	GlobalFunc			*global,
	LWChannelHandler	*local,
	void				*serverData)
{
	XCALL_INIT;
	if (version != LWCHANNEL_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) ("Info Messages", GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTextureFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	GGlobal = global;

	local->inst->create  = TxtrChanCreate;
	local->inst->destroy = TxtrChanDestroy;
	local->inst->load    = TxtrChanLoad;
	local->inst->save    = TxtrChanSave;
	local->inst->copy    = TxtrChanCopy;

	local->evaluate 	= TxtrChanEval;
	local->flags 		= TxtrChanFlags;
	local->inst->descln		= TxtrChanDescribe;
	return (AFUNC_OK);
}

XCALL_(static int) TxtrMotion (
	long			 	 version,
	GlobalFunc			*global,
	LWItemMotionHandler	*local,
	void				*serverData)
{
	XCALL_INIT;
	if (version != LWCHANNEL_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) ("Info Messages", GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalItemFuncs = (*global) (LWITEMINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalItemFuncs )
		return AFUNC_BADGLOBAL;

	GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTextureFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	GGlobal = global;

	local->inst->create  = TxtrMotionCreate;
	local->inst->destroy = TxtrMotionDestroy;
	local->inst->load    = TxtrChanLoad;
	local->inst->save    = TxtrChanSave;
	local->inst->copy    = TxtrChanCopy;

	local->evaluate 	= TxtrMotionEval;
	local->flags 		= TxtrMotionFlags;
	local->inst->descln		= TxtrChanDescribe;
	return (AFUNC_OK);
}

XCALL_(static int) TxtrDisplace (
	long			 		version,
	GlobalFunc				*global,
	LWDisplacementHandler	*local,
	void					*serverData)
{
	XCALL_INIT;
	if (version != LWCHANNEL_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) ("Info Messages", GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalItemFuncs = (*global) (LWITEMINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalItemFuncs )
		return AFUNC_BADGLOBAL;

	GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTextureFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	GGlobal = global;

	local->inst->create  = TxtrDisplaceCreate;
	local->inst->destroy = TxtrDisplaceDestroy;
	local->inst->load    = TxtrChanLoad;
	local->inst->save    = TxtrChanSave;
	local->inst->copy    = TxtrChanCopy;

	local->evaluate 	= TxtrDisplaceEval;
	local->flags 		= TxtrDisplaceFlags;
	local->inst->descln		= NULL;
	return (AFUNC_OK);
}

XCALL_(static int) TxtrFilter (
	long					 version,
	GlobalFunc				*global,
	LWImageFilterHandler	*local,
	void					*serverData)
{
	XCALL_INIT;
	if (version != LWCHANNEL_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) ("Info Messages", GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTextureFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	GGlobal = global;

	local->inst->create  = TxtrFilterCreate;
	local->inst->destroy = TxtrFilterDestroy;
	local->inst->load    = TxtrChanLoad;
	local->inst->save    = TxtrChanSave;
	local->inst->copy    = TxtrChanCopy;

	local->process 	= TxtrFiltProcess;
	local->flags 		= TxtrFiltFlags;
	local->inst->descln		= TxtrChanDescribe;
	return (AFUNC_OK);
}

XCALL_(static int) TxtrEnviron (
	long					 version,
	GlobalFunc				*global,
	LWEnvironmentHandler	*local,
	void					*serverData)
{
	XCALL_INIT;
	if (version != LWCHANNEL_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) ("Info Messages", GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTextureFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;


	GGlobal = global;

	local->inst->create  = TxtrEnvironCreate;
	local->inst->destroy = TxtrEnvironDestroy;
	local->inst->load    = TxtrChanLoad;
	local->inst->save    = TxtrChanSave;
	local->inst->copy    = TxtrChanCopy;

	local->evaluate 	= TxtrEnvironProcess;
	local->flags 		= TxtrEnvironFlags;
	local->inst->descln		= TxtrChanDescribe;
	return (AFUNC_OK);
}

/* -----------------  User Interface  ----------------- */

#define STR_Offset_TEXT		"Offset"
#define STR_Scale_TEXT		"Scale"
#define STR_Axis_TEXT		"Axis"
#define STR_VTyp_TEXT		"VMap Type"
#define STR_VMap_TEXT		"VMap Name"
#define STR_Txtr_TEXT		"Texture"

enum  {	CH_OFFSET = 0x8001, CH_SCALE, CH_AXIS, CH_VTYP, CH_VMAP, CH_TXTR};
static	char	*axisList[] = {"X","Y","Z",0};
static	char	*vmapType[] = {"Weight Map","Morph Target","Vertex Color",0};

static LWXPanelControl ctrl_list[] = {
	{ CH_OFFSET,       STR_Offset_TEXT,         "distance" },
	{ CH_SCALE,        STR_Scale_TEXT,          "distance" },
	{ CH_AXIS,         STR_Axis_TEXT,           "iChoice" },
    { CH_VTYP,		   STR_VTyp_TEXT,			"iPopChoice" },
  //  { CH_VMAP,		   STR_VMap_TEXT,			"iPopChoice" },
    { CH_VMAP,		   STR_VMap_TEXT,			"sPresetText" },
	{0}
};
static LWXPanelDataDesc data_descrip[] = {
	{ CH_OFFSET,        STR_Offset_TEXT,          "distance" },
	{ CH_SCALE,        STR_Scale_TEXT,          "distance" },
	{ CH_AXIS,        STR_Axis_TEXT,          "integer"  },
    { CH_VTYP,		 STR_VTyp_TEXT,			"integer"   },
   // { CH_VMAP,		STR_VMap_TEXT,		"integer"  },
    { CH_VMAP,		STR_VMap_TEXT,		"string"  },
	{0},
};


XCALL_( static int )
popCnt_VMAP( TxtrData *dat )
{
 	switch (dat->vtyp)
	{
		case 0:
			return GlobalObjectFuncs->numVMaps( LWVMAP_WGHT);// + 1;
		case 1:
			return GlobalObjectFuncs->numVMaps( LWVMAP_MORF);// + 1;
		case 2:
			return GlobalObjectFuncs->numVMaps( LWID_('R','G','B','A'));// + 1;
	}

	return	0;
}
XCALL_( static const char * )
popName_VMAP( TxtrData *dat, int idx )
{
 	switch (dat->vtyp)
	{
		case 0:
			return GlobalObjectFuncs->vmapName( LWVMAP_WGHT, idx );
		case 1:
			return GlobalObjectFuncs->vmapName( LWVMAP_MORF, idx );
		case 2:
			return GlobalObjectFuncs->vmapName( LWID_('R','G','B','A'), idx );
	}

	return	0;
}

int vmapIndex(TxtrData *dat, char *name, LWID type)
{
	int		n,i;
	n = popCnt_VMAP(dat);
	for(i=0;i<n;i++)
		if(!strcmp(name,popName_VMAP(dat,i)))
			return i;
	return -1;
}

XCALL_( static int )
popCnt_VTYP( TxtrData *dat )
{
   return 3;
}
XCALL_( static const char * )
popName_VTYP( TxtrData *dat, int idx )
{
   if (idx<=2)
	   return vmapType[idx];
   else
	   return	NULL;
}

void *NoiseData_get ( void *myinst, unsigned long vid ) 
{
  TxtrData *dat = (TxtrData*)myinst;
  void *result = NULL;
  static double val = 0.0;
  static int	ival = 0;
  if ( dat ) 
	  switch ( vid ) {
		case CH_OFFSET:
		  val = dat->offset;
		  result = &val;
		  break;
		case CH_SCALE:
		  val = dat->scale;
		  result = &val;
		  break;
		case CH_AXIS:
		  ival = dat->axis;
		  result = &ival;
		  break;
         case CH_VTYP:
          ival = dat->vtyp;
		  result = &ival;
          break;
        case CH_VMAP:
		  result = dat->vmname;
          break;
	  } 

  return result;
}

int NoiseData_set ( void *myinst, unsigned long vid, void *value ) 
{
  TxtrData *dat = (TxtrData*)myinst;
  int		rc=0,ival;

  if ( dat ) 
	  switch ( vid ) {
		case CH_OFFSET:
		  dat->offset = *((double*)value);
		  rc = 1;
		  break;
		case CH_SCALE:
		  dat->scale = *((double*)value);
		  rc = 1;
		  break;
		case CH_AXIS:
		  dat->axis = *((int*)value);
		  rc = 1;
		  break;
        case CH_VTYP:
          ival = *(( int * ) value );
		  if (ival != dat->vtyp)
		  {
			dat->vtyp = ival;
			dat->vmname[0] = 0;
		  }
		  rc = 1;
		  break;
        case CH_VMAP:
  			strncpy(dat->vmname ,((char*)value), sizeof(dat->vmname) );
		  rc = 1;
		  break;
	  }

  if (dat->type == VMAP && rc)
	TxtrEventFunc(dat->txtr,dat,TXEV_ALTER);
  
  return rc;
}

void	XPaneChangeFunc( LWXPanelID	pan,unsigned long	cid,unsigned long	vid,int		event_type )
{
	TxtrData *dat;

	dat = (*GlobalXPanFun->getData)(pan,0);
	if (event_type == LWXPEVENT_VALUE && GlobalLWUpdate)
	{
		(*GlobalTextureFuncs->newtime)(dat->txtr,gTime,gFrame);	
		
		if (dat->type == CHANNEL)
			(*GlobalLWUpdate)(LWCHANNEL_HCLASS, dat);	
		else if (dat->type == MOTION)
			(*GlobalLWUpdate)(LWITEMMOTION_HCLASS, dat);	
		else if (dat->type == FILTER)
			(*GlobalLWUpdate)(LWIMAGEFILTER_HCLASS, dat);	
		else if (dat->type == ENVIRON)
			(*GlobalLWUpdate)(LWENVIRONMENT_HCLASS, dat);	

		(*GlobalTextureFuncs->cleanup)(dat->txtr);		
	}
}

void CH_cb_destroy ( TxtrData	*data ) 
{
	if (!data)
		return;

	data->ui = NULL;

	if (data->type == VMAP) // Never gets called, non-modal xpanel never destroyed:
	{			// Crash on modeler quit
		if (vmapTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(vmapTxtredID);
			vmapTxtredID = NULL;
		}
		if (vmapCtxtID)
		{
			(*GlobalTextureFuncs->contextDestroy)(vmapCtxtID);
			vmapCtxtID = NULL;
		}

		if (data->txtr)
		{
			(*GlobalTextureFuncs->destroy)(data->txtr);
			data->txtr = NULL;
		}				
		if (data->rgb_txtr)
		{
			(*GlobalTextureFuncs->destroy)(data->rgb_txtr);
			data->rgb_txtr = NULL;
		}
	}
	return;
}

void TxtrButtonEvent(LWXPanelID		panID,unsigned long cid)
{
	TxtrData 	*dat = NULL;

	dat = (*GlobalXPanFun->getData)(panID,CH_TXTR);
	if (!dat)
		return;

	if (dat->type == CHANNEL)
		(*GlobalTxtrEdFuncs->open)(chanTxtredID,dat->txtr,dat->name);
	else if (dat->type == MOTION)
		(*GlobalTxtrEdFuncs->open)(motTxtredID,dat->txtr,dat->name);
	else if (dat->type == FILTER)
		(*GlobalTxtrEdFuncs->open)(fltTxtredID,dat->txtr,dat->name);
	else if (dat->type == ENVIRON)
		(*GlobalTxtrEdFuncs->open)(envTxtredID,dat->txtr,dat->name);
	else if (dat->type == VMAP)
	{
		if(dat->vtyp==2)
			(*GlobalTxtrEdFuncs->open)(vmapTxtredID,dat->rgb_txtr,dat->name);
		else
			(*GlobalTxtrEdFuncs->open)(vmapTxtredID,dat->txtr,dat->name);
	}
}


LWXPanelID TxtrXPanel(GlobalFunc *global, TxtrData *dat)
{
	LWXPanelFuncs *lwxpf = NULL;
	LWXPanelID     panID = NULL;
	static LWXPanelHint hint[] = {
		XpLABEL(0,"Apply Texture"),
		XpSTRLIST (CH_AXIS,axisList),
		XpPOPFUNCS( CH_VTYP, popCnt_VTYP, popName_VTYP ),
		XpPOPFUNCS( CH_VMAP, popCnt_VMAP, popName_VMAP ),
		XpLINK(CH_VTYP,CH_VMAP),
		// texture button
		XpADD(CH_TXTR,"vButton",NULL),
		XpLABEL(CH_TXTR,STR_Txtr_TEXT),
		XpBUTNOTIFY(CH_TXTR,TxtrButtonEvent),
		XpCHGNOTIFY(XPaneChangeFunc),	
		XpDESTROYNOTIFY(CH_cb_destroy),	
		XpEND
	};

	static LWXPanelHint removeVMap[] = {
		XpDELETE(CH_VTYP),
		XpDELETE(CH_VMAP),
		XpEND
	};

	lwxpf = GlobalXPanFun;
	if ( lwxpf ) 
	{
		panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );
		if(panID) 
		{
			(*lwxpf->hint) ( panID, 0, hint );
			if (dat->type != VMAP)
				(*lwxpf->hint) ( panID, 0, removeVMap );
	
			(*lwxpf->describe)( panID, data_descrip, NoiseData_get, NoiseData_set );
			(*lwxpf->viewInst)( panID, dat );
			(*lwxpf->setData)(panID,CH_TXTR,dat);
			(*lwxpf->setData)(panID,CH_VMAP,dat);
			(*lwxpf->setData)(panID,0,dat);
		}
    }

	dat->ui = panID;
	return panID;
}

static LWPanControlDesc   desc;	 // required by macros in lw_panl.h
static LWValue ival={LWT_INTEGER},ivecval={LWT_VINT},		// required by macros in lw_panl.h
  fval={LWT_FLOAT},fvecval={LWT_VFLOAT},sval={LWT_STRING};

static int XComUI(char *dat)
{
	LWPanelID		 panID;
	XCALL_INIT;

	if(panID = PAN_CREATE(GlobalPanFun,"Execute Command"))	
	{
		LWControl	*nm;

		if(!(nm = STR_CTL(GlobalPanFun,panID,"Command",80) ))
			goto controlError;

		if((*GlobalPanFun->open)(panID,PANF_CANCEL|PANF_BLOCKING))
		{
			GET_STR(nm,dat,80);
			PAN_KILL(GlobalPanFun,panID);
			return 1;
		}

controlError:
		PAN_KILL(GlobalPanFun,panID); 
		return 0;
	}
	return 0;
}


// ----------------------------------------------------------------- //


XCALL_(static int) TxtrChannel_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	TxtrData	*dat;
	XCALL_INIT;

	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);

	GlobalTxtrEdFuncs = (*global) (LWTXTREDFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTxtrEdFuncs )
		return AFUNC_BADGLOBAL;

	GlobalXPanFun = (*global) (LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalXPanFun )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	dat = UI->inst;
	if (!chanTxtredID && dat->type == CHANNEL)
		chanTxtredID = (*GlobalTxtrEdFuncs->subscribe)("TextureChannel",TEF_ALL - TEF_USEBTN,NULL,NULL,NULL,TxtrEventFunc);
	if (!fltTxtredID && dat->type == FILTER)
		fltTxtredID = (*GlobalTxtrEdFuncs->subscribe)("TextureFilter",TEF_ALL - TEF_USEBTN,NULL,NULL,NULL,TxtrEventFunc);
	if (!motTxtredID && dat->type == MOTION)
		motTxtredID = (*GlobalTxtrEdFuncs->subscribe)("TextureMotion",TEF_ALL - TEF_USEBTN,NULL,NULL,NULL,TxtrEventFunc);
	if (!envTxtredID && dat->type == ENVIRON)
		envTxtredID = (*GlobalTxtrEdFuncs->subscribe)("TextureEnvironment",TEF_ALL - TEF_USEBTN,NULL,NULL,NULL,TxtrEventFunc);

	UI->panel = TxtrXPanel(global,dat);

	return AFUNC_OK;
}

XCALL_(static int) TxtrDisp_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	TxtrData	*dat;
	char		title[80];
	XCALL_INIT;

	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);

	GlobalTxtrEdFuncs = (*global) (LWTXTREDFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTxtrEdFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	dat = UI->inst;
	if (!dspTxtredID && dat->type == DISPLACE)
	{
		dspTxtredID = (*GlobalTxtrEdFuncs->subscribe)("DisplacementTexture",TEF_ALL  - TEF_USEBTN,dat,NULL,TxtrAutoSizeFunc,TxtrEventFunc);
		(*GlobalTxtrEdFuncs->setGradientAutoSize) (dspTxtredID,GradientAutoSizeFunc);		
	}

	if (dat->self)
		sprintf(title,"%s",(*GlobalItemFuncs->name)(dat->self));
	else
		sprintf(title,"");

	if (dspTxtredID)
		(*GlobalTxtrEdFuncs->open)(dspTxtredID,dat->txtr,title);

	return AFUNC_OK;
}



	XCALL_(int)
TxtrVMap (
	long			 version,
	GlobalFunc		*global,
	LWModCommand	*local,
	void			*serverData)
{
	static	TxtrData		dat;
	static	LWModCommand	command;
	static	LWMicropol		mp;
	LWXPanelID			panID = NULL;

 	XCALL_INIT;

	if (version != LWMODCOMMAND_VERSION)
		return AFUNC_BADVERSION;

	GlobalXPanFun = (*global) (LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalXPanFun )
		return AFUNC_BADGLOBAL;

	GlobalTxtrEdFuncs = (*global) (LWTXTREDFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTxtrEdFuncs )
		return AFUNC_BADGLOBAL;

	GlobalTextureFuncs = (*global) (LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTextureFuncs )
		return AFUNC_BADGLOBAL;

	GlobalObjectFuncs = (*global) (LWOBJECTFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalObjectFuncs )
		return AFUNC_BADGLOBAL;

	command = *local;

	if (!vmapActivateCount)
	{
		memset(&dat,0,sizeof(dat));
		dat.scale = 1.0;
		dat.ctxt = &command;
		dat.type = VMAP;
		dat.vtyp = 0;
		dat.name = NULL;
		dat.vmidx = -1;

		dat.bbox[0][0]= BIGNUM;
		dat.bbox[1][0]= BIGNUM;
		dat.bbox[2][0]= BIGNUM;
		dat.bbox[0][1]= -BIGNUM;
		dat.bbox[1][1]= -BIGNUM;
		dat.bbox[2][1]= -BIGNUM;

		initMP(&mp);
		dat.self = &mp;

		vmapTxtredID = (*GlobalTxtrEdFuncs->subscribe)("TextureVMap",TEF_ALL - TEF_USEBTN,&dat,NULL,TxtrAutoSizeFunc,TxtrEventFunc);	
		(*GlobalTxtrEdFuncs->setGradientAutoSize)(vmapTxtredID,GradientAutoSizeFunc);
		if (!vmapCtxtID)
			vmapCtxtID = addVMapInputParams();
		dat.txtr = (*GlobalTextureFuncs->create)(TRT_SCALAR,"TextureVMap",vmapCtxtID,&dat); // never destroyed???
		dat.rgb_txtr = (*GlobalTextureFuncs->create)(TRT_COLOR,"TextureVMap",vmapCtxtID,&dat);
	}

	if (panID = TxtrXPanel(global,&dat))
		(*GlobalXPanFun->open)(panID); 
//	(*GlobalXPanFun->destroy)(panID);
	vmapActivateCount ++;

	return AFUNC_OK;
}

	XCALL_(int)
XCommand (
	long			 version,
	GlobalFunc		*global,
	LWModCommand	*local,
	void			*serverData)
{
	static	char		dat[80];
	LWXPanelID			panID = NULL;

 	XCALL_INIT;
	if (version != LWMODCOMMAND_VERSION)
		return AFUNC_BADVERSION;

	GlobalPanFun = (*global) (LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalPanFun )
		return AFUNC_BADGLOBAL;

	XComUI(dat);
	(*local->evaluate)(local->data,dat);
	return AFUNC_OK;
}

static ServerTagInfo 
Env_tags[] = { 
	{"Textured Environment",SRVTAG_USERNAME}, 
	{"",0} },
Ch_tags[] = { 
	{"Textured Channel",SRVTAG_USERNAME}, 
	{"",0} },
Mo_tags[] = { 
	{"Textured Motion",SRVTAG_USERNAME}, 
	{"",0} },
Disp_tags[] = { 
	{"Textured Displacement",SRVTAG_USERNAME}, 
	{"",0} },
Norm_tags[] = { 
	{"Normal Displacement",SRVTAG_USERNAME}, 
	{"",0} },
vm_tags[] = {  
	{"Textured Point Map",SRVTAG_USERNAME}, 
	{"point maps",SRVTAG_CMDGROUP}, 
	{"tools/points",SRVTAG_MENU}, 
	{"",0}},
xc_tags[] = {  
	{"Execute Command",SRVTAG_USERNAME}, 
	{"",0}},
Ft_tags[] = { 
	{"Textured Filter",SRVTAG_USERNAME}, 
	{"",0} };



ServerRecord ServerDesc[] = {
	// Layout plugs
	{ LWCHANNEL_HCLASS,		"LW_TextureChannel",		TxtrChannel, Ch_tags},
	{ LWCHANNEL_ICLASS,		"LW_TextureChannel",		TxtrChannel_UI, Ch_tags },
	{ LWITEMMOTION_HCLASS,	"LW_TextureMotion",		TxtrMotion, Mo_tags},
	{ LWITEMMOTION_ICLASS,	"LW_TextureMotion",		TxtrChannel_UI, Mo_tags },
	{ LWIMAGEFILTER_HCLASS,	"LW_TextureFilter",		TxtrFilter, Ft_tags},
	{ LWIMAGEFILTER_ICLASS,	"LW_TextureFilter",		TxtrChannel_UI, Ft_tags },
	{ LWENVIRONMENT_HCLASS,	"LW_TextureEnvironment",TxtrEnviron, Env_tags },
	{ LWENVIRONMENT_ICLASS,	"LW_TextureEnvironment",TxtrChannel_UI, Env_tags },
	{ LWDISPLACEMENT_HCLASS,	"LW_DisplacementTexture",TxtrDisplace, Disp_tags },
	{ LWDISPLACEMENT_ICLASS,	"LW_DisplacementTexture",TxtrDisp_UI, Disp_tags },
	{ LWDISPLACEMENT_HCLASS,	"LW_NormalDisplacement",NormalDisplacement, Norm_tags },
	{ LWDISPLACEMENT_ICLASS,	"LW_NormalDisplacement",NormalDisplacement_UI, Norm_tags },
	// Modeler plugs
	{ LWMODCOMMAND_CLASS,	"LW_TextureVMap",		TxtrVMap, vm_tags },
	{ LWMODCOMMAND_CLASS,	"LW_XCommand",		XCommand, xc_tags },
	{ NULL }
};

