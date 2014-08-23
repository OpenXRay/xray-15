#include <lwserver.h>
#include <lwhost.h>
#include <lwhandler.h>
#include <lwmath.h>
#include <lwdisplce.h>
#include <lwrender.h>
#include <lwtxtr.h>
#include <lwtexture.h>
#include <lwtxtred.h>
#include <lwmeshes.h>
#include <lwxpanel.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>


// ------------------------------------------------------- //


typedef struct ndPoint {
	double		norm[3],opos[3],w;	// norm,opos
	LWPntID		v;			// vertexID
	int			n,next;		// occurences (to average normals), next point
}ndPoint;

typedef struct normData {
	LWMeshInfoID	mesh;
	double			xax[3],yax[3],zax[3],trans[3],max;
	ndPoint			*point;
	int				*idx;

	int				np,nv,na,first,last,cur;
}normData;

typedef struct st_NormDisp {
	void				*ui,*vmap;
	LWItemID			id;
	double				bbox[3][2],att,amp;
	char				desc[100];
	LWTextureID			txtr;
	char				*name;
	normData			*nd;
	int					vnb,np,attenuation,cache,init,render,type;
	char				vmname[256];
	
	LWMicropol	mp;
} NormDisp;


// ------------------------------------------------------- //

extern	LWItemInfo			*GlobalItemFuncs;
extern	LWMeshInfo			*GlobalMeshesFuncs;

static	LWXPanelFuncs		*GlobalXPanFun= NULL;
static	LWMessageFuncs		*Gmessage= NULL;
static	LWTextureFuncs		*GlobalTextureFuncs= NULL;
static	LWTxtrEdFuncs		*GlobalTxtrEdFuncs= NULL;
static	LWObjectFuncs		*GlobalObjectFuncs= NULL;
static	LWInstUpdate		*GlobalLWUpdate = NULL;
static  GlobalFunc			*GGlobal=NULL;

static	LWTECltID			dspTxtredID = NULL;
static	LWTxtrContextID		dspCtxtID = NULL;

static	LWFrame				gFrame = 0;
static	LWTime				gTime = 0;

static int xax[] = {1,2,0}, yax[]={2,0,1}, zax[] = {0,1,2};

static char *errAllocFailed ="Tiny Allocation Failed, I don't feel so good";
static int	dspActivateCount = 0;

// ------------------------------------------------------- //


normData	*ndCreate ()
{
	normData			*dat;

	dat = malloc(sizeof(normData));
	dat->point = NULL;
	dat->idx = NULL;
	dat->np = 0;
	dat->first = -1;
	dat->max = 0;

	return dat;
}

static	void	ndClear (normData	*dat)
{
	if (dat->point)
		free(dat->point);
	if (dat->idx)
		free(dat->idx);

	dat->point = NULL;
	dat->idx = NULL;
}
static	void	ndDestroy (normData	*dat)
{
	ndClear(dat);
	free(dat);
}

static void	normalize(double	v[3])
{
	double	n;

	n = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	if (n>0)
	{
		n = 1/n;
		v[0] = n*v[0];	v[1] = n*v[1];	v[2] = n*v[2];
	}
}

static	void	addPoint(normData	*dat,LWPntID	ver,LWPolID	pol,double	op[3],double	wp[3],double	n[3],double	weight)
{
	int			i;
	ndPoint		*p,*last;

	for (i=dat->np-1;i>=0;i--) // search in reverse is much more efficient
	{
		p = &dat->point[i];
		if (p->v == ver)
		{
			p->norm[0] += (float)n[0];
			p->norm[1] += (float)n[1];
			p->norm[2] += (float)n[2];
			p->w += weight;
			p->n ++;
			return;
		}
	}

	if (dat->np > dat->na)
		return;
	
	if (dat->np == 0)
	{
		dat->first = 0;
		dat->last = 0;
		dat->cur = 0;
	}
	else
	{
		last = &dat->point[dat->last];
		last->next = dat->np;
		dat->last = dat->np;
	}

	p = &dat->point[dat->np];
	for (i=0;i<3;i++)
	{
		p->norm[i] = n[i];
		p->opos[i] = op[i];
	}

	p->v = ver;
	p->n = 1;
	p->w = weight;
	p->next = -1;

	dat->np++;
}

static int	NormalAxis (const double      norm[3])
{
    double		ax[3];

    ax[0] = ABS (norm[0]);
    ax[1] = ABS (norm[1]);
    ax[2] = ABS (norm[2]);

    if (ax[0] > ax[1] && ax[0] > ax[2])
		return 0;
    else if (ax[1] >= ax[0] && ax[1] > ax[2])
		return 1;
    else
		return 2;
}

static void		getPointWPos(NormDisp	*inst,LWPntID	v,double	wPos[3])
{
	float			p[3];
	normData		*dat;
	LWMeshInfo		*mesh;

	dat = inst->nd;
	mesh = dat->mesh;

	if (inst->cache)
	{
		(*mesh->pntBasePos)(mesh,v,p);
		wPos[0] = VDOT(dat->xax,p);
		wPos[1] = VDOT(dat->yax,p);
		wPos[2] = VDOT(dat->zax,p);
		VADD(wPos,dat->trans);
	}
	else
	{
		(*mesh->pntOtherPos)(mesh,v,p);
		VCPY(wPos,p);
	}
}

static	double	ndPolToNormAndWeight(NormDisp	*inst,LWPolID	pol,double	norm[3])
{
	normData		*dat;
	LWMeshInfo		*mesh;
	float			a[3],b[3],c[3],p[3],p0[3],d[3];
	double			w;
	int				i,n;

	dat = inst->nd;
	mesh = dat->mesh;
	n = (*mesh->polSize)(mesh,pol);
	if (n<3)
		return	0;

	w = 0;
	
	if (inst->cache)
		(*mesh->pntBasePos)(mesh,(*mesh->polVertex)(mesh,pol,0),p0);
	else
		(*mesh->pntOtherPos)(mesh,(*mesh->polVertex)(mesh,pol,0),p0);

	VCPY(a,p0);
	for (i=1;i<n;i++)
	{
		if (inst->cache)
			(*mesh->pntBasePos)(mesh,(*mesh->polVertex)(mesh,pol,i),p);
		else
			(*mesh->pntOtherPos)(mesh,(*mesh->polVertex)(mesh,pol,i),p);

		if (i==1)
			VCPY(b,p);
		else if (i==2)
			VCPY(c,p);

		d[0] = p[0] - p0[0];
		d[1] = p[1] - p0[1];
		d[2] = p[2] - p0[2];
		w += VDOT(d,d);
		VCPY(p0,p);
	}
	w = sqrt(w/n);

	VSUB3(a,a,b);
	VSUB3(b,b,c);
	VCROSS(norm,a,b);
	normalize(norm);

	return	w;
}

static	int		ndScanPolys(NormDisp	*inst,LWPolID	pol)
{
	LWMeshInfo		*mesh;
	normData		*dat;
	LWPntID			v;
	float			p[3];
	double			wnorm[3],oPos[3],wPos[3],w;
	int				i,nv;

	dat = inst->nd;
	mesh = dat->mesh;
	nv = (*mesh->polSize)(mesh,pol);
	w = ndPolToNormAndWeight(inst,pol,wnorm);

	for (i=0;i<nv;i++)
	{
		v = (*mesh->polVertex)(mesh,pol,i);		
		(*mesh->pntBasePos)(mesh,v,p);
		VCPY(oPos,p);
		getPointWPos(inst,v,wPos);
		addPoint(dat,v,pol,oPos,wPos,wnorm,w);
	}
	
	return	0;
}

static	void	getNorm(NormDisp	*inst,LWDisplacementAccess *dspAcc,double	wnorm[3])
{
	normData	*dat;
	ndPoint		*p,*prev;
	double		eps = 0.0001,d[3],att;
	double		norm[3] = {0,0,0};

	dat = inst->nd;
	if (inst->type == 0)
	{
		LWMeshInfo	*mesh;
		float		morph[3];
	
		mesh = dspAcc->info;
		if (inst->vmap && (*mesh->pntVGet)(mesh,dspAcc->point,morph))
			VCPY(wnorm,morph);
		else
			VSET(wnorm,0);
	
		return;
	}
	else if (dat->first == -1)
		dat->first = 0;
	else if (dat->max == 0)
		return;

	if (inst->attenuation)
		att = inst->att;
	else
		att = -1;

	if (!inst->cache || inst->init)
	{
		p = &dat->point[dat->first];
		dat->cur = dat->first;
	}
	else
	{
		dat->cur = dat->idx[inst->vnb];
		if (dat->cur>=dat->na || dat->cur<0)
		{
			VSET(wnorm,0);
			return;
		}

		p = &dat->point[dat->cur];
		VCPY(norm,p->norm);
		if (att != -1 && att<1)
		{
			double		w;
			w = p->w/dat->max;
			w = pow(w,1-att);
			VSCL(norm,w);
		}

		if (inst->cache)
		{
			wnorm[0] = VDOT(dat->xax,norm);
			wnorm[1] = VDOT(dat->yax,norm);
			wnorm[2] = VDOT(dat->zax,norm);
		}
		else
			VCPY(wnorm,norm);
	}

	prev = NULL;
	while (p)
	{
		VSUB3(d,p->opos,dspAcc->oPos);
		if (VDOT(d,d)<eps)
		{
			// remove point from the list to accelerate search
			if (prev)
				prev->next = p->next;
			else
				dat->first = p->next;

			VCPY(norm,p->norm);
			if (att != -1 && att<1)
			{
				double		w;

				w = p->w/dat->max;
				w = pow(w,1-att);
				VSCL(norm,w);
			}

			if (inst->cache)
			{
				wnorm[0] = VDOT(dat->xax,norm);
				wnorm[1] = VDOT(dat->yax,norm);
				wnorm[2] = VDOT(dat->zax,norm);
			}
			else
				VCPY(wnorm,norm);

			if (inst->init)
				dat->idx[inst->vnb] = dat->cur;
			return;
		}
		else
			prev = p;

		if (p->next != -1)
		{
			dat->cur = p->next;
			p = &dat->point[p->next];
		}
		else
			p = NULL;
	}
}

static	void	EvaluateNormals(NormDisp	*inst,LWMeshInfo	*mesh)
{
	normData			*dat;
	ndPoint				*p;
	int					i;
	double				av;
	
	dat = inst->nd;
	dat->mesh = mesh;

	// allocate point buffer
	dat->point = calloc(dat->na,sizeof(ndPoint));
	if (!dat->point)
		return;
	dat->idx = calloc(dat->na,sizeof(int));
	if (!dat->idx)
		return;

	dat->np = 0;
	(*mesh->scanPolys)(dat->mesh,ndScanPolys,inst);
	// average and normalise point normals, save to Normal VMap

	for (i=0;i<dat->np;i++)
	{
		p = &dat->point[i];
		av = 1.0/(double)p->n;
		// average and normalise normal vetor
		VSCL(p->norm,av);
		p->w *= av;
		if (dat->max<p->w)
			dat->max = p->w;
		normalize(p->norm);
	}
}


// ----------------------------------------------------------- //
LWTxtrContextID		addDisplaceInputParams(LWTextureFuncs		*tf);
void				initMP(LWMicropol	*mp);


static	int		TxtrEventFunc(LWTextureID	txtr, void	*userData,int	eventCode)
{
	NormDisp		*data;

	data= (*GlobalTextureFuncs->userData)(txtr);	
	if (eventCode == TXEV_ALTER && GlobalLWUpdate)
	{
		(*GlobalTextureFuncs->newtime)(txtr,gTime,gFrame);	
		(*GlobalLWUpdate)(LWDISPLACEMENT_HCLASS, data);	
		(*GlobalTextureFuncs->cleanup)(txtr);		
	}

	return	1;
}

// This is the autosize callback. This is being called when the user hits
// the 'automatic sizing' button in the coordinate options.
// Note that the gradient autosize has a different callback.
static	int	TxtrAutoSizeFunc(LWTextureID	txtr, void	*userData,double	bbox[3][2])
{
	NormDisp		*data;

	data= (*GlobalTextureFuncs->userData)(txtr);	
	memcpy(bbox,data->bbox,6*sizeof(double));	
	return	1;
}

static	int	GradientAutoSizeFunc(LWTxtrParamDesc	*param,int	paramNb,void	*userData)
{
	NormDisp		*data;
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

XCALL_(LWError)	NormDispInit (NormDisp *inst,int	flag)
{
	return	NULL;
}

XCALL_(LWError)	NormDispNewtime (NormDisp *inst,LWFrame	f,LWTime	t)
{
	XCALL_INIT;

	gTime = t;
	gFrame = f;
	inst->vnb = 0;
	return	NULL;
}

XCALL_(void)	NormDispCleanup (NormDisp *inst)
{
	XCALL_INIT;
	return;
}

XCALL_(static LWInstance)NormDispCreate(void *data, LWItemID item, LWError *err)
{
	NormDisp *dat=NULL;
	XCALL_INIT;
	
	if(dat=malloc(sizeof(NormDisp)))
	{
		memset(dat,0,sizeof(*dat));
		dat->id = (void *)item;
		if (!dspCtxtID)
			dspCtxtID = addDisplaceInputParams(GlobalTextureFuncs);

		// create texture (of displacement type)
		dat->txtr = (*GlobalTextureFuncs->create)(TRT_PERCENT,"NormalDisplacementTexture",dspCtxtID,dat);
		(*GlobalTextureFuncs->setEnvGroup)(dat->txtr,(*GlobalItemFuncs->chanGroup)(item));
	
		dat->nd = ndCreate();
		dat->type = 1;
		dat->cache = 1;
		dat->init = 1;
		dat->attenuation = 1;
		dat->att = 0.5;
		dat->amp = 1.0;
		sprintf(dat->desc,"");
		sprintf(dat->vmname,"");
	}
	else
		err = &errAllocFailed;

	dspActivateCount ++;
	return dat;
}


XCALL_(static void)NormDispDestroy(NormDisp *inst)
{
	XCALL_INIT;
	if(inst)
	{
		dspActivateCount --;
		if (!dspActivateCount && GlobalTxtrEdFuncs && dspTxtredID)
		{
			(*GlobalTxtrEdFuncs->unsubscribe)(dspTxtredID);
			dspTxtredID = NULL;		

			if (dspCtxtID)
				(*GlobalTextureFuncs->contextDestroy)(dspCtxtID);
			dspCtxtID = NULL;
		}

		(*GlobalTextureFuncs->destroy)(inst->txtr);
		if (inst->nd)
			ndDestroy(inst->nd);
	
		free(inst);
	}
}

XCALL_(static LWError)NormDispCopy(NormDisp	*to, NormDisp	*from)
{
	(*GlobalTextureFuncs->copy)(to->txtr,from->txtr);
	to->attenuation = from->attenuation;
	to->att = from->att;
	to->amp = from->amp;
	to->type = from->type;
	to->cache = from->cache;
	sprintf(to->vmname,"%s",from->vmname);
	return (NULL);
}

#define		ND_VERSION	3
XCALL_(static LWError)NormDispLoad(NormDisp *inst,const LWLoadState	*lState)
{
	int		version;
	float	fp;

	LWLOAD_I4(lState,&version,1);
	LWLOAD_FP(lState,&fp,1);
	inst->amp = fp;
	LWLOAD_I4(lState,&inst->attenuation,1);
	LWLOAD_FP(lState,&fp,1);
	inst->att = fp;
	if (version >= 2)
		LWLOAD_I4(lState,&inst->cache,1);
	if (version >= 3)
	{
		LWLOAD_I4(lState,&inst->type,1);
		LWLOAD_STR(lState,inst->vmname,256);
	}

	(*GlobalTextureFuncs->load)(inst->txtr,lState);

	return (NULL);
}
XCALL_(LWError)NormDispSave(NormDisp *inst,const LWSaveState	*sState)
{
	int		version = ND_VERSION;
	float	fp;

	LWSAVE_I4(sState,&version,1);
	fp = (float)inst->amp;
	LWSAVE_FP(sState,&fp,1);
	LWSAVE_I4(sState,&inst->attenuation,1);
	fp = (float)inst->att;
	LWSAVE_FP(sState,&fp,1);
	LWSAVE_I4(sState,&inst->cache,1);
	LWSAVE_I4(sState,&inst->type,1);
	LWSAVE_STR(sState,inst->vmname);

	(*GlobalTextureFuncs->save)(inst->txtr,sState);
	
	return (NULL);
}
XCALL_(static const char *)NormDispDescribe (NormDisp *inst)
{
	sprintf(inst->desc,"Normal Displacement");
	return (inst->desc);
}
XCALL_(static unsigned int)NormDispFlags (NormDisp *inst)
{
	if (inst->type == 0)
		return	LWDMF_BEFOREBONES;
	else
		return	LWDMF_WORLD;
}


XCALL_(static void) NormDispEval (NormDisp	*inst,LWDisplacementAccess *dspAcc)
{
	double		vec[3],norm[3];
	int			i;

	XCALL_INIT;	

	GlobalMeshesFuncs = dspAcc->info;

	if (inst->type == 0)
	{
		inst->vmap = (*GlobalMeshesFuncs->pntVLookup)(GlobalMeshesFuncs,LWVMAP_MORF,inst->vmname);
		if (!(*GlobalMeshesFuncs->pntVSelect)(GlobalMeshesFuncs,inst->vmap))
			inst->vmap = NULL;
	}

	// if first eval
	if (!inst->vnb)
	{
		normData	*dat;
		LWMeshInfo	*mesh;

		dat = inst->nd;
		mesh = dspAcc->info;
		dat->na = dat->nv = (*mesh->numPoints)(mesh);
		if (!inst->cache)
		{
			inst->init = 1;
			ndClear(inst->nd);
		}
		else
		{
			// if cache on and same number of points: use cache
			if (inst->np == dat->na && (mesh == dat->mesh) && inst->nd->point)
			{
				inst->init = 0;
				inst->nd->first = 0;
				inst->nd->cur = 0;
			}
			else // point count different: recompute normals
			{
				inst->init = 1;
				ndClear(inst->nd);
			}
		}

		inst->np = dat->na;
		if (inst->init && inst->type == 1)
			EvaluateNormals(inst,dspAcc->info);
			
		initMP(&inst->mp);
		(*GlobalItemFuncs->param)(inst->id,LWIP_W_POSITION,gTime,dat->trans);
		(*GlobalItemFuncs->param)(inst->id,LWIP_W_RIGHT,gTime,dat->xax);
		(*GlobalItemFuncs->param)(inst->id,LWIP_W_UP,gTime,dat->yax);
		(*GlobalItemFuncs->param)(inst->id,LWIP_W_FORWARD,gTime,dat->zax);
	}

	VSET(norm,0);
	getNorm(inst,dspAcc,norm);	

	for (i=0;i<3;i++)
	{
		inst->mp.wPos[i] = dspAcc->source[i];
		inst->mp.oPos[i] = dspAcc->oPos[i];
		inst->mp.gNorm[i] = inst->mp.wNorm[i] = norm[i];

		if (inst->bbox[i][0]>dspAcc->oPos[i])		//min
			inst->bbox[i][0] = dspAcc->oPos[i];
		if (inst->bbox[i][1]<dspAcc->oPos[i])	//max
			inst->bbox[i][1] = dspAcc->oPos[i];
		
		inst->mp.vertsWPos[0][i] = (float)dspAcc->source[i];
	}

	inst->mp.verts[0] = dspAcc->point;
	inst->mp.weights[0] = 1.0f;
	inst->mp.verts[1] = dspAcc->point; // Using only 1 vertex causes getVerxUV to fail!!!
	inst->mp.weights[1] = 0.0f; // Could average identical values
	inst->mp.verts[2] = dspAcc->point; // Using only 2 verts causes getVerxUV to fail!!!
	inst->mp.weights[2] = 0.0f;
	inst->mp.oAxis = inst->mp.wAxis = NormalAxis(norm);
	inst->mp.spotSize = 0.001;
	if (inst->type == 0)
		inst->mp.txVal = sqrt(VDOT(norm,norm));
	
	// evaluate the texture
	VSET(vec,0);
	(*GlobalTextureFuncs->evaluate)(inst->txtr,&inst->mp,vec);
	for (i=0;i<3;i++)
		dspAcc->source[i] += norm[i]*vec[0]*inst->amp;

	inst->vnb ++;	
	if ((inst->vnb == inst->np) && inst->nd)
		inst->vnb = 0;
}
	
const LWItemID *	NormDispUseItems(NormDisp	*inst)
{
	return	NULL;
}
	
static void	NormDispChangeID(NormDisp	*inst, const LWItemID *list)
{
	int		i=0;

	while (list[i])
	{
		if (list[i] == inst->id)
			inst->id = list[i+1];

		i = i + 2;
	}
}

// ------------------------------------------------------------- //


int	NormalDisplacement (long			 		version,
						GlobalFunc				*global,
						LWDisplacementHandler	*local,
						void					*serverData)
{
	XCALL_INIT;
	if (version != LWDISPLACEMENT_VERSION)
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

	GGlobal = global;

	local->inst->create  = NormDispCreate;
	local->inst->destroy = NormDispDestroy;
	local->inst->load    = NormDispLoad;
	local->inst->save    = NormDispSave;
	local->inst->copy    = NormDispCopy;
	local->inst->descln		= NULL;

	local->rend->init		= NormDispInit;
	local->rend->newTime    = NormDispNewtime;
	local->rend->cleanup    = NormDispCleanup;

	local->item->useItems    = NormDispUseItems;
	local->item->changeID    = NormDispChangeID;

	local->evaluate 	= NormDispEval;
	local->flags 		= NormDispFlags;
	return (AFUNC_OK);
}

/* -----------------  User Interface  ----------------- */

#define STR_Amp_TEXT		"Displacement Amplitude"
#define STR_Att_TEXT		"Detail Attenuation"
#define STR_Bias_TEXT		"Attenuation Bias"
#define STR_Cach_TEXT		"Cache Normals"
#define STR_Type_TEXT		"Displacement Direction"
#define STR_VMap_TEXT		"MorphMap"
#define STR_Txtr_TEXT		"Texture"

enum  {	CH_AMP = 0x8001,CH_TYPE, CH_VMAP, CH_ATT, CH_BIAS, CH_CACH, CH_TXTR, CH_MGRP, CH_NGRP};

static LWXPanelControl ctrl_list[] = {
	{ CH_AMP,         STR_Amp_TEXT,           "distance" },
	{ CH_TYPE,         STR_Type_TEXT,           "iPopChoice" },
	{ CH_VMAP,         STR_VMap_TEXT,           "iPopChoice" },
	{ CH_ATT,         STR_Att_TEXT,           "iBoolean" },
	{ CH_BIAS,         STR_Bias_TEXT,           "percent" },
	{ CH_CACH,         STR_Cach_TEXT,           "iBoolean" },
	{0}
};
static LWXPanelDataDesc data_descrip[] = {
	{ CH_AMP,         STR_Amp_TEXT,           "distance" },
	{ CH_TYPE,         STR_Type_TEXT,           "integer" },
	{ CH_VMAP,         STR_VMap_TEXT,           "integer" },
	{ CH_ATT,        STR_Att_TEXT,          "integer"  },
	{ CH_BIAS,         STR_Bias_TEXT,           "percent" },
	{ CH_CACH,         STR_Cach_TEXT,           "integer" },
	{0},
};

static int	popCnt_TYPE( NormDisp *dat )
{
	return 2;
}
static const char *popName_TYPE( NormDisp *dat, int idx )
{
	static	char	*typeList[] = {"MorphMap","Normals",NULL};
	return typeList[idx];
}

static int	popCnt_VMAP( NormDisp *dat )
{
	return GlobalObjectFuncs->numVMaps( LWVMAP_MORF);
}
static const char *popName_VMAP( NormDisp *dat, int idx )
{
	return GlobalObjectFuncs->vmapName( LWVMAP_MORF, idx );
}

static int mapIndex(NormDisp *dat, char *name)
{
	int		n,i;
	n = popCnt_VMAP(dat);
	for(i=0;i<n;i++)
		if(!strcmp(name,popName_VMAP(dat,i)))
			return i;
	return -1;
}

static	void *ndData_get ( void *myinst, unsigned long vid ) 
{
	NormDisp	*dat = (NormDisp*)myinst;
	void		*result = NULL;
	static int		ival = 0;
	static double	fp;

	if (!dat)
		return	NULL;

	switch ( vid ) 
	{
 		case CH_TYPE:
			ival = dat->type;
			result = &ival;
			break;
        case CH_VMAP:
			ival = mapIndex(dat,dat->vmname);
			result = &ival;
			break;
		case CH_CACH:
			ival = dat->cache;
			result = &ival;
			break;
		case CH_ATT:
			ival = dat->attenuation;
			result = &ival;
			break;
		case CH_BIAS:
			fp = dat->att;
			result = &fp;
			break;
		case CH_AMP:
			fp = dat->amp;
			result = &fp;
			break;
	} 
	return result;
}

static	int ndData_set ( void *myinst, unsigned long vid, void *value ) 
{
	NormDisp	*dat = (NormDisp*)myinst;
	int			rc=0;

	if ( !dat )
		return	0;

	switch ( vid ) 
	{
		case CH_TYPE:
			dat->type = *((int*)value);
			rc = 1;
			break;
        case CH_VMAP:
  			sprintf(dat->vmname,"%s" ,(char *)(GlobalObjectFuncs->vmapName( LWVMAP_MORF, *((int*)value) )));
			rc = 1;
			break;
		case CH_CACH:
			dat->cache = *((int*)value);
			dat->init = 1;
			ndClear(dat->nd);
			rc = 1;
			break;
		case CH_ATT:
			dat->attenuation = *((int*)value);
			rc = 1;
			break;
		case CH_BIAS:
			dat->att = *((double*)value);
			rc = 1;
			break;
		case CH_AMP:
			dat->amp = *((double*)value);
			rc = 1;
			break;
	}  
	return rc;
}

static	void	XPaneChangeFunc( LWXPanelID	pan,unsigned long	cid,unsigned long	vid,int		event_type )
{
	NormDisp	 *dat;

	dat = (*GlobalXPanFun->getData)(pan,0);
	if (event_type == LWXPEVENT_VALUE && GlobalLWUpdate)
	{
		(*GlobalTextureFuncs->newtime)(dat->txtr,gTime,gFrame);			
		(*GlobalLWUpdate)(LWDISPLACEMENT_HCLASS, dat);	
		(*GlobalTextureFuncs->cleanup)(dat->txtr);		
	}
}

static void ND_cb_destroy ( NormDisp	*data ) 
{
	if (!data)
		return;

	data->ui = NULL;
	return;
}

static void TxtrButtonEvent(LWXPanelID		panID,unsigned long cid)
{
	NormDisp 	*dat = NULL;

	dat = (*GlobalXPanFun->getData)(panID,CH_TXTR);
	if (!dat || !dspTxtredID)
		return;

	(*GlobalTxtrEdFuncs->open)(dspTxtredID,dat->txtr,dat->name);
}

static	int		vtypeMap[] = {1,0};
static	int		itypeMap[] = {0,1};

LWXPanelID ndXPanel(GlobalFunc *global, NormDisp *dat)
{
	LWXPanelFuncs *lwxpf = NULL;
	LWXPanelID     panID = NULL;
	static LWXPanelHint hint[] = 
	{
		// texture button
		XpADD(CH_TXTR,"vButton",NULL),
		XpLABEL(CH_TXTR,STR_Txtr_TEXT),
		XpPOPFUNCS( CH_VMAP, popCnt_VMAP, popName_VMAP ),
		XpPOPFUNCS( CH_TYPE, popCnt_TYPE, popName_TYPE ),
		XpBUTNOTIFY(CH_TXTR,TxtrButtonEvent),

		XpMIN (CH_BIAS,0),	
		XpMAX (CH_BIAS,1),
		XpENABLE_ (CH_ATT),XpH(CH_BIAS),NULL,

		XpGROUP_ (CH_MGRP),
			XpH(CH_VMAP),
			NULL,

		XpGROUP_ (CH_NGRP),
			XpH(CH_ATT),
			XpH(CH_BIAS),
			XpH(CH_CACH),
			NULL,	
		
		XpENABLE_MAP_ (CH_TYPE,vtypeMap),XpH(CH_MGRP),NULL,
		XpENABLE_MAP_ (CH_TYPE,itypeMap),XpH(CH_NGRP),NULL,

		XpCHGNOTIFY(XPaneChangeFunc),	
		XpDESTROYNOTIFY(ND_cb_destroy),	
		XpEND
	};

	if (!dspTxtredID)
	{
		dspTxtredID = (*GlobalTxtrEdFuncs->subscribe)("NormalDisplacementTexture",TEF_ALL  - TEF_USEBTN,dat,NULL,TxtrAutoSizeFunc,TxtrEventFunc);
		(*GlobalTxtrEdFuncs->setGradientAutoSize) (dspTxtredID,GradientAutoSizeFunc);
	}

	lwxpf = GlobalXPanFun;
	if ( lwxpf ) 
	{
		panID = (*lwxpf->create)( LWXP_VIEW, ctrl_list );
		if(panID) 
		{
			(*lwxpf->hint) ( panID, 0, hint );
	
			(*lwxpf->describe)( panID, data_descrip, ndData_get, ndData_set );
			(*lwxpf->viewInst)( panID, dat );
			(*lwxpf->setData)(panID,CH_TXTR,dat);
			(*lwxpf->setData)(panID,0,dat);
		}
    }

	dat->ui = panID;
	return panID;
}


int	NormalDisplacement_UI (long		 	version,
							GlobalFunc		*global,
							LWInterface		*UI,
							void			*serverData)
{
	NormDisp	*dat;

	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);

	GlobalTxtrEdFuncs = (*global) (LWTXTREDFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalTxtrEdFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUpdate = (*global) (LWINSTUPDATE_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalLWUpdate )
		return AFUNC_BADGLOBAL;

	GlobalXPanFun = (*global) (LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalXPanFun )
		return AFUNC_BADGLOBAL;

	GlobalObjectFuncs = (*global) (LWOBJECTFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalObjectFuncs )
		return AFUNC_BADGLOBAL;

	dat = UI->inst;
	UI->panel = ndXPanel(global,dat);

	return AFUNC_OK;
}
