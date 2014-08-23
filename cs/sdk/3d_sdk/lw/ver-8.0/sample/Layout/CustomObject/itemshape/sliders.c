// Sliders.c - Custom Object Plug-in
// Arnie Cachelin, Copyright 2001 NewTek, Inc.
// 

#include <lwhost.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwenvel.h>
#include <lwpanel.h>
#include <lwchannel.h>
#include <lwrender.h>
#include <lwcustobj.h>
#include <lwtxtr.h>
#include <lwvparm.h>
#include <lwlaytool.h>
#include <lwmath.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include "custdraw.h"
#include "minvert.h"
#include "icons.h"

static LWPanControlDesc   desc;	 // required by macros in lwpanel.h
static LWValue ival={LWT_INTEGER},ivecval={LWT_VINT},		// required by macros in lwpanel.h
  fval={LWT_FLOAT},fvecval={LWT_VFLOAT},sval={LWT_STRING};

#define VCPY_F(a,b)       ((a)[0] =(float)((b)[0]), (a)[1] =(float)((b)[1]), (a)[2] =(float)((b)[2]))

static	LWPanelFuncs	*GlobalPanFun		= NULL;
static	LWMessageFuncs	*Gmessage			= NULL;
static	LWTextureFuncs	*GlobalTextureFuncs	= NULL;
static	LWItemInfo		*GlobalItemInfo		= NULL;
static	LWSceneInfo		*GlobalSceneInfo	= NULL;
static	LWCameraInfo	*GlobalCameraInfo   = NULL;
static	LWInstUpdate	*GlobalLWUpdate		= NULL;
static	LWVParmFuncs    *GlobalVParmFunc	= NULL;
static	LWChannelInfo	*GlobalChanFun		= NULL;
static	LWEnvelopeFuncs	*GlobalEnvelopeFuncs= NULL;
static	LWInterfaceInfo		*GlobalLWUI=NULL;
static	LWViewportInfo		*GlobalView=NULL;
static  GlobalFunc			*GlobalGlobal=NULL;

static	float			Red[]		= {0.62f, 0.22f, 0.22f, 1.0f},
				Green[]		= {0.22f, 0.62f, 0.22f, 1.0f},
				Blue[]		= {0.22f, 0.32f, 0.62f, 1.0f},
				Yellow[]	= {0.62f, 0.62f, 0.32f, 1.0f},
				Orange[]	= {0.92f, 0.62f, 0.32f, 1.0f},
				Cyan[]		= {0.32f, 0.62f, 0.62f, 1.0f},
				Magenta[]	= {0.62f, 0.32f, 0.62f, 1.0f},
				White[]		= {0.92f, 0.92f, 0.92f, 1.0f},
				HotRed[]		= {1.0f, 0.32f, 0.12f, 1.0f},
				Black[]		= {0.1f, 0.1f, 0.1f, 1.0f},
				FullRGB[]		= {1,1,1,1},
				GhostRGB[]		= {1,1,1,0.4f},
				Shadow[]		= {0.1f, 0.1f, 0.1f, 1};
// these colors are too much!!!
static float *colors[] = {Red, Green, Blue,White,Cyan,Magenta,Yellow,Orange};

#ifdef _MACOS
	#define DEGREE_CHAR	0xA1
#else
	#define DEGREE_CHAR	'°'
#endif

#define MAX_COLOR_PRE	15
unsigned int colorRGBs[MAX_COLOR_PRE+1] = {
	RGB_(0x00,0x00,0x00), // black items
	RGB_(0x00,0x30,0x80), // dk blue items
	RGB_(0x00,0x60,0x00), // dk green items
	RGB_(0x20,0x60,0x70), // dk cyan items
	RGB_(0x70,0x00,0x00), // dk red items
	RGB_(0x70,0x20,0x70), // dk magenta items
	RGB_(0x70,0x50,0x00), // brown items
	RGB_(0xB0,0xB0,0xB0), // gray items
	RGB_(0x20,0xA0,0xF0), // blue items
	RGB_(0x20,0xE0,0x20), // green items
	RGB_(0x60,0xE0,0xF0), // cyan items
	RGB_(0xF0,0x20,0x20), // red items
	RGB_(0xF0,0x60,0xF0), // magenta items
	RGB_(0xF0,0xC0,0x20), // orange items
	RGB_(0xF0,0xF0,0xF0), // white items
	RGB_(0x00,0x00,0x00) // black items
};

char *colorNames[MAX_COLOR_PRE+1] = {
	"Black",
	"Dark Blue",
	"Dark Green",
	"Dark Cyan",
	"Dark Red",
	"Dark Magenta",
	"Brown",
	"Gray",
	"Blue",
	"Green",
	"Cyan",
	"Red",
	"Magenta",
	"Orange",
	"White",
	"Custom"
};

char *colorItems[] = {
	"\03(i:140,16777216) Black",
	"\03(i:140,16810032) Dark Blue",
	"\03(i:140,16777312) Dark Green",
	"\03(i:140,18903136) Dark Cyan",
	"\03(i:140,24117248) Dark Red",
	"\03(i:140,24145952) Dark Magenta",
	"\03(i:140,24117328) Brown",
	"\03(i:140,28356784) Gray",
	"\03(i:140,18935968) Blue",
	"\03(i:140,18882784) Green",
	"\03(i:140,23130336) Cyan",
	"\03(i:140,32514080) Red",
	"\03(i:140,32567392) Magenta",
	"\03(i:140,32514240) Orange",
	"\03(i:140,32567536) White",
	NULL
};


float fRGBs[] = {
	0x00/255.0f, 0x00/255.0f, 0x00/255.0f,
	0x00/255.0f, 0x30/255.0f, 0x80/255.0f,
	0x00/255.0f, 0x60/255.0f, 0x00/255.0f,
	0x20/255.0f, 0x60/255.0f, 0x70/255.0f,
	0x70/255.0f, 0x00/255.0f, 0x00/255.0f,
	0x70/255.0f, 0x20/255.0f, 0x70/255.0f,
	0x70/255.0f, 0x50/255.0f, 0x00/255.0f,
	0xB0/255.0f, 0xB0/255.0f, 0xB0/255.0f,
	0x20/255.0f, 0xA0/255.0f, 0xF0/255.0f,
	0x20/255.0f, 0xE0/255.0f, 0x20/255.0f,
	0x60/255.0f, 0xE0/255.0f, 0xF0/255.0f,
	0xF0/255.0f, 0x20/255.0f, 0x20/255.0f,
	0xF0/255.0f, 0x60/255.0f, 0xF0/255.0f,
	0xF0/255.0f, 0xC0/255.0f, 0x20/255.0f,
	0xF0/255.0f, 0xF0/255.0f, 0xF0/255.0f,
	0x00/255.0f, 0x00/255.0f, 0x00/255.0f
};

extern HUD_Icon IconList[];

static char *errAllocFailed ="Tiny Allocation Failed",
	*errOnlyOne ="There can be only 1 slider bank per object", *AxisName="XYZ";
static int x_axis[] = {2,2,0}, y_axis[] = {1,0,1}, z_axis[] = {0,1,2};
static char		*shapeList[] = {"Half (180°)", "Full (360°)", NULL};

#define NHEADF_LOCKED		1 

typedef struct st_NodeHead *NodeHeadID;

typedef struct st_NodeHead {
	NodeHeadID		prev;
	NodeHeadID		next;
	LWItemID		self;
	int				flags;
	char			name[256];
} NodeHead;

typedef struct st_NodeList {
	int			 num;
	NodeHead	*list;
} NodeList;

NodeList	SliderGlobalNodeList = {0, NULL};

int addNode(NodeList *List, NodeHeadID node)
{
	NodeHeadID		n;
	int				i = 0;
	if(List->list==NULL)
	{
		List->list = node;
		List->num = 1;
		node->next = NULL;
		node->prev = NULL;
		return 1;
	}
	for(n = List->list; n->next && i<=List->num; n = n->next) // get last node
		i++;
	n->next = node;
	node->prev = n;
	node->next = NULL;
	List->num++;
	return List->num;
}

int removeNode(NodeList *List, NodeHeadID node)
{
	if(node && List->list && List->num)
	{
		NodeHeadID		p, n;
		p = node->prev;
		n = node->next;
		if(p)
			p->next = n;
		if(n)
			n->prev = p;
		List->num--;
	}
	return List->num;
}

NodeHeadID findNode(NodeList *List, LWItemID id)
{
	NodeHeadID		n;
	int			i=0;
	if(id && List->list && List->num)
		for(n = List->list; n && i<=List->num; n = n->next, i++) // scan list
		{
			if(id==n->self)
			{
				const char	*name=NULL;
				if(!id)
					return n;
				name = GlobalItemInfo->name(id);
				if(!strcmp(name,n->name))
					return n;
				else
					n->name[0] = 0;
			}
		}
	return NULL;
}

void fixNodeNames(NodeList *List, NodeHeadID skip)
{
	NodeHeadID		n;
	int			i=0;
	if(List->list && List->num)
		for(n = List->list; n && i<=List->num; n = n->next, i++) // scan list
		{
			if( (n!=skip) && n->self && (n->name[0]==0) )
				strncpy(n->name, GlobalItemInfo->name(n->self), sizeof(n->name)-1);
		}
	return;
}

int stringWidth(char *text) 
{
	if(GlobalPanFun && text)
		return GlobalPanFun->drawFuncs->textWidth(NULL, text);
	else
		return 0;
}
/* ----------------- Plug-in Instance Definition  -----------------  */

#define		MAX_CHANS	64
#define		MAX_NAMES	24 // channel nesting
typedef struct st_ChannelHud {
	LWChannelID			chan;
	LWItemID			parentItem;
	int				level;
	char				*names[MAX_NAMES];
	double				min, max, v0;
	float				color[4];
	int				flags;
	char				label[80];
} CHUD;

#define CHF_CURRENT		1
#define CHF_HOT			2
#define CHF_ANGLE		4
#define CHF_PERCENT		8
#define CHF_CUSTLABL		16

#define SLIDE_HMARG		1.25

typedef struct st_SlidersData {
	NodeHead			 node; // MUST start with embedded NodeHead for tool to find!!!
	void				*ctxt;
	LWControl			*chanTree, *list, *min, *max, *rgb, *lab, *pre;
	CHUD				 chud[MAX_CHANS];
	double				 time, slideW, slideH, offset[2], tw;
	int				 cur, frame, flags, num, dirty, dx;
	short				 maxLabel, spot[4][2], curView;
	LWDVector			 sizeHandle, moveHandle, foldHandle,envHandle;
	LWFVector			 clrA, clrB;
	LWDVector			 orig;
	char				 desc[100];
} SlidersData;

#define SLF_AUTOKEY		1
#define SLF_AUTORANGE		2
#define SLF_XFINIT		4
#define SLF_MOVING		8
#define SLF_FOLDED		16
#define SLF_FLOCK		32
#define SLF_ACTIVE		64

#define CHAN(d,i)	(d->chud[i].chan)
#define CHENV(d,i)	(GlobalChanFun->channelEnvelope(d->chud[i].chan))

char *chName(SlidersData  *dat, int i);

int channelNamesMatch(LWChannelInfo *chanFun, LWChannelID chanA, LWChannelID chanB)
{
	LWChanGroupID		prnt=NULL, grpA=NULL, grpB=NULL;
	if(chanA==chanB) 
		return 1;
	if(!chanA || !chanB) 
		return 0;
	if(strcmp(chanFun->channelName(chanA), chanFun->channelName(chanB)) )
		return 0;

	grpA = chanFun->channelParent(chanA);
	grpB = chanFun->channelParent(chanB);
	while( grpA && grpB )
	{
		if(grpA==grpB)
			return 1;
		if(strcmp(chanFun->groupName(grpA), chanFun->groupName(grpB)) )
			return 0;
		grpA = chanFun->groupParent(grpA);
		grpB = chanFun->groupParent(grpB);
	}
	if(!grpA || !grpB) 
		return grpA==grpB ? 1:0;
	return 1;
}


const char *ItemGroupName( LWItemInfo *ItemInfo, LWItemID id )
{
	if(ItemInfo->type(id)!=LWI_BONE)
		return ItemInfo->name(id);
	else 
	{
		LWItemID	prnt;
		prnt = ItemInfo->parent(id);
		while( prnt && ItemInfo->type(prnt)==LWI_BONE)
			prnt = ItemInfo->parent(prnt);
		return prnt ? ItemInfo->name(prnt):NULL;
	}
}

LWItemID chanGroupItem(LWItemInfo *ItemInfo, LWChanGroupID group)
{
	LWItemID		id, bon;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(ItemInfo->chanGroup(id)==group)
			return id;
		for(bon = ItemInfo->first(LWI_BONE, id); bon!= LWITEM_NULL; bon = ItemInfo->next(bon))
		{
			if(ItemInfo->chanGroup(bon)==group)
				return bon;
		}
	}
	for(id = ItemInfo->first(LWI_LIGHT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
		if(ItemInfo->chanGroup(id)==group)
			return id;
	for(id = ItemInfo->first(LWI_CAMERA, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
		if(ItemInfo->chanGroup(id)==group)
			return id;
	return LWITEM_NULL;
}


LWItemID findParentID(LWChannelInfo *chanFun, LWItemInfo *ItemInfo, LWChannelID chan)
{
	LWChanGroupID		prnt=NULL, grp=NULL;
	LWItemID		id;
	if(!chan) 
		return LWITEM_NULL;
	grp = chanFun->channelParent(chan);
	id = chanGroupItem(ItemInfo, grp);
	if(id)
		return id;

	prnt = chanFun->groupParent(grp);
	while( chanFun->groupParent(prnt) )
	{
		grp = prnt;
		id = chanGroupItem(ItemInfo, grp);
		if(id)
			return id;
		prnt = chanFun->groupParent(grp);
	}
	prnt = grp; // second level group, not root level ?
	return chanGroupItem(ItemInfo, prnt);
}


int loadNames(const LWLoadState	*lState, char **names, int size)
{
	char	name[301];
	int		lev = 0;
	while( lev<size && (0<(*lState->readStr)(lState->readData, name, 300)) )
	{
		if(names[lev]=malloc(strlen(name)+2))
			strcpy(names[lev++], name);
	}
	if(lev<size)
		names[lev] = NULL;
	return lev;
}

LWChannelID acquireTarget(LWChannelInfo *chanFun, char **names, int size)
{
	const char			*gn;
	int				 i, found = 0;
	LWChanGroupID			 prnt=NULL, grp=NULL;
	LWChannelID			 chan=NULL;

	for(i=0; i<size-1; i++)
	{
		grp = (*chanFun->nextGroup)(prnt,grp);
		gn = NULL;
		found = 0;
		while(grp)
		{
			gn = (*chanFun->groupName)(grp);
			if(gn && !strcmp(names[i],gn) )
			{
				prnt = grp;
				grp = NULL;
//				gn = NULL; // mark as parent found
				found = 1;
				break;
			}
			grp = (*chanFun->nextGroup)(prnt,grp);
			found = 0;
		}

	}
	if(found)
		if(i==size-1) // channel name
		{
			chan = (*chanFun->nextChannel)(prnt,chan);
			while(chan)
			{
				gn = (*chanFun->channelName)(chan);
				if(gn && !strcmp(names[i],gn) )
				{
					return chan;
				}
				chan = (*chanFun->nextChannel)(prnt,chan);
			}
		}
	return NULL;
}

void freeNames(char **names, int lev)
{
	int					 i;
	for(i=0; i<lev; i++)
		if(names[i])
		{
			free(names[i]);
			names[i] = NULL;
		}
}

LWChannelID loadTarget(LWChannelInfo *chanFun, const LWLoadState	*lState,char **names, int size, int *lev)
{
	LWChannelID			 chan=NULL;

	*lev = loadNames(lState, names, size);
	chan = acquireTarget(chanFun, names, *lev);
	return chan;
}

int findNames(LWChannelInfo *chanFun, LWChannelID target , char **names, int size)
{
	const char	*c;
	int		lev = 0;
	int gens = 0,p,i;
	LWChanGroupID	prnt, root=NULL;
	prnt = (*chanFun->channelParent)(target);
	while(prnt)// count up parent generations
	{
		root = prnt;
		gens++;
		prnt = (*chanFun->groupParent)(root);
	}
	if(gens>1) 
		gens--; // skip root

	for(i=gens, lev=0; i>0; i--)
	{
		prnt = (*chanFun->channelParent)(target);
		for(p=1;p<i;p++)
			prnt = (*chanFun->groupParent)(prnt);

		c = (*chanFun->groupName)(prnt);
		if(lev<size)
			if(names[lev]=malloc(strlen(c)+2))
				strcpy(names[lev++], c);
	}
	c = (*chanFun->channelName)(target);
	if(lev<size)
		if(names[lev]=malloc(strlen(c)+2))
			strcpy(names[lev++], c);
	if(lev<size)
		names[lev] = NULL;

	return lev;
}


void saveTarget(LWChannelInfo *chanFun, LWChannelID target ,const LWSaveState	*sState)
{
	int gens = 0,p,i;
	LWChanGroupID	prnt, root=NULL;
	prnt = (*chanFun->channelParent)(target);
	while(prnt)
	{
		root = prnt;
		gens++;
		prnt = (*chanFun->groupParent)(root);
	}
	if(gens>1) 
		gens--; // skip root
	for(i=gens; i>0; i--)
	{
		prnt = (*chanFun->channelParent)(target);
		for(p=1;p<i;p++)
			prnt = (*chanFun->groupParent)(prnt);
		(*sState->writeStr)(sState->writeData,(*chanFun->groupName)(prnt));
	}
	(*sState->writeStr)(sState->writeData,(*chanFun->channelName)(target));
	return ;
}

static int chanEvent(SlidersData *dat, LWChannelID ch, int ev, void *edat)
{
	if(ev==LWEEVNT_DESTROY)// Someone whacked my envelope!
	{
		int v;
		for(v=0; v<dat->num; v++)
		{
			if(dat->chud[v].chan == ch)
			{
				dat->chud[v].chan = NULL; 
				break;
			}
		}
	}
	else if(ev==LWEEVNT_RENAME) // group has been renamed!
	{
		int v;
		for(v=0; v<dat->num; v++)
		{
			if(dat->chud[v].chan == ch)
			{
				freeNames(dat->chud[v].names, dat->chud[v].level);
				dat->chud[v].level = findNames(GlobalChanFun, dat->chud[v].chan, dat->chud[v].names, 30);
				if( !(dat->chud[v].flags&CHF_CUSTLABL) )
				{
					strncpy(dat->chud[v].label, chName(dat, v), 79);
					v = stringWidth(dat->chud[v].label);
					if(v>dat->maxLabel)
						dat->maxLabel = v;
				}
				break;
			}
		}
	}
	return 0;
}

static int LW_Execute(const char *command)
{
	int		(*exCmd)(const char *cmd);
	int		t=0;

	if(command && *command)
		if(exCmd=(*GlobalGlobal)("LW Command Interface", GFUSE_TRANSIENT))
			t=(*exCmd)(command);
	return t; 
}

int addGraphEnv(LWEnvelopeID env, int add)
{
	char			 buf[250];
	if(!env)
		return -1;
	sprintf(buf,"GE_SetEnvID %d %d",env, add);
	return LW_Execute(buf);	
}

int setLayoutTime(LWTime t)
{
	LWSceneInfo		*si;
	double			 fps = 30.0;
	int				 fr;
	char			 buf[30];

	if(si=(*GlobalGlobal)(LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT))
		fps = si->framesPerSecond;
	fr = (int)(0.5 + t*fps);
	sprintf(buf,"%s %d","GoToFrame",fr);
	return LW_Execute(buf);
}

static double prevKeyTime(LWEnvelopeID env, double time)
{
	if(env)
	{
		LWEnvKeyframeID key, prev= NULL;
		double	t;
		for(key=GlobalEnvelopeFuncs->nextKey(env,NULL); key; key=GlobalEnvelopeFuncs->nextKey(env,key))
		{
			GlobalEnvelopeFuncs->keyGet(env,key,LWKEY_TIME,&t);
			if(t>=time)
			{
				if(prev)
				{
					GlobalEnvelopeFuncs->keyGet(env,prev,LWKEY_TIME,&t);
					return t;
				}
				return time;
			}
			prev = key;
		}
		if(prev)
		{
			GlobalEnvelopeFuncs->keyGet(env,prev,LWKEY_TIME,&t);
			return t;
		}
	}
	return time;
}

static double nextKeyTime(LWEnvelopeID env, double time)
{
	if(env)
	{
		LWEnvKeyframeID key;
		double	t;
		for(key=GlobalEnvelopeFuncs->nextKey(env,NULL); key; key=GlobalEnvelopeFuncs->nextKey(env,key))
		{
			GlobalEnvelopeFuncs->keyGet(env,key,LWKEY_TIME,&t);
			if(t>time)
				return t;
		}
	}
	return time;
}


// transform src into v using 3x3 mat[]
void applyMat(LWDVector v, double *mat, LWDVector src)
{
	v[0] = mat[0]*src[0] + mat[3]*src[1] + mat[6]*src[2];
	v[1] = mat[1]*src[0] + mat[4]*src[1] + mat[7]*src[2];
	v[2] = mat[2]*src[0] + mat[5]*src[1] + mat[8]*src[2];
}


int checkAutoKey(SlidersData *dat)
{
	return -1;


	GlobalLWUI = (*GlobalGlobal) (LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT);
	if(GlobalLWUI)
	{
		if( (GlobalLWUI->generalFlags&LWGENF_AUTOKEY) && (GlobalLWUI->autoKeyCreate!=LWAKC_OFF))
			dat->flags |= SLF_AUTOKEY;
		else
			dat->flags &= ~SLF_AUTOKEY;
		return 1;
	}
	return 0;
}

/* ----------------- Plug-in Methods: LWInstanceFuncs  -----------------  */

XCALL_(static LWInstance)SlidersCreate(void *data, LWItemID id, LWError *err)
{
	SlidersData *dat=NULL;
	XCALL_INIT;
	if(findNode(&SliderGlobalNodeList, id))
	{
		*err = errOnlyOne;
		return NULL;
	}
	if(dat=malloc(sizeof(SlidersData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->frame = 0;
		dat->cur = 0;
		dat->flags = SLF_AUTOKEY;
		dat->node.self = id;
		strncpy(dat->node.name, GlobalItemInfo->name(id), sizeof(dat->node.name)-1);
		dat->ctxt = data;
		dat->slideH = 12;
		dat->slideW = 256;
		VCPY(dat->clrA, Yellow);
		VCPY(dat->clrB, Blue);
		addNode(&SliderGlobalNodeList, (NodeHeadID)dat);
	}
	else
		*err = errAllocFailed;
	return dat;
}

XCALL_(static const char *)SlidersDescribe(SlidersData *dat)
{
	XCALL_INIT;
	sprintf(dat->desc," ");
	return (dat->desc);
}

XCALL_(static void)SlidersDestroy(SlidersData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		int		v;
		removeNode(&SliderGlobalNodeList, (NodeHeadID)dat );
		for(v=0; v<dat->num; v++)
		{
			GlobalChanFun->setChannelEvent(dat->chud[v].chan, NULL, dat);
			freeNames(dat->chud[v].names, dat->chud[v].level);
		}
		
		free(dat);
	}
}

XCALL_(static LWError)SlidersCopy(SlidersData	*to, SlidersData	*from)
{
	NodeHead tnod;
	XCALL_INIT;
	tnod = to->node;
	*to = *from;
	to->node = tnod;
	return NULL;
}

XCALL_(static LWError)SlidersLoad(SlidersData *dat,const LWLoadState	*lState)
{
	int			v, version, level, len;
	float		fv[9] = {0.0f,0.0f,0.0f};
	LWItemID			id;
	XCALL_INIT;
	LWLOAD_I4(lState,&version,1);
	LWLOAD_I4(lState,&dat->num,1);
	LWLOAD_I4(lState,&dat->flags,1);
	LWLOAD_FP(lState,fv,1);
	LWLOAD_I2(lState,dat->spot[0],2);
	LWLOAD_I2(lState,dat->spot[1],2);
	LWLOAD_I2(lState,dat->spot[2],2);
	LWLOAD_I2(lState,dat->spot[3],2);
	dat->slideW = fv[0];
	dat->maxLabel = 0;
	for(v=0; v<dat->num; v++)
	{
		dat->chud[v].names[0] = NULL;
		dat->chud[v].chan = loadTarget(GlobalChanFun,lState,dat->chud[v].names,MAX_NAMES, &level);
		dat->chud[v].level = level;

		if(version>1)
		{
			LWLOAD_U4(lState, &level, 1);
			id = (LWItemID)level;
			dat->chud[v].parentItem = id;
			if(!id)
				dat->chud[v].parentItem = findParentID(GlobalChanFun, GlobalItemInfo, dat->chud[v].chan);
			else // could be wrong names, so verify
			{
				id = findParentID(GlobalChanFun, GlobalItemInfo, dat->chud[v].chan);
				if(id != dat->chud[v].parentItem)
					dat->chud[v].chan = NULL;
			}
		}
		else
			dat->chud[v].parentItem = findParentID(GlobalChanFun, GlobalItemInfo, dat->chud[v].chan);

		if(dat->chud[v].chan)
			GlobalChanFun->setChannelEvent(dat->chud[v].chan, chanEvent, dat);

		LWLOAD_FP(lState,fv,2);
		dat->chud[v].min = fv[0];
		dat->chud[v].max = fv[1];
		LWLOAD_FP(lState,fv,3);
		VCPY(dat->chud[v].color, fv);
		dat->chud[v].color[3] = 1;
		LWLOAD_I4(lState,&dat->chud[v].flags,1);
		LWLOAD_STR(lState,dat->chud[v].label, sizeof(dat->chud[v].label)-1);

		len = stringWidth(dat->chud[v].label);//  strlen(dat->chud[v].label);
		if(len>dat->maxLabel)
			dat->maxLabel = len;
	}

	return NULL;
}

XCALL_(LWError)SlidersSave(SlidersData *dat,const LWSaveState	*sState)
{
	int		v = 2, id;
	float		fv[9];
	XCALL_INIT;
	LWSAVE_I4(sState,&v,1);
	LWSAVE_I4(sState,&dat->num,1);
	LWSAVE_I4(sState,&dat->flags,1);
	fv[0] = (float)dat->slideW;
	LWSAVE_FP(sState,fv,1);
	LWSAVE_I2(sState,dat->spot[0],2);
	LWSAVE_I2(sState,dat->spot[1],2);
	LWSAVE_I2(sState,dat->spot[2],2);
	LWSAVE_I2(sState,dat->spot[3],2);
	for(v=0; v<dat->num; v++)
	{
		if(dat->chud[v].chan)
			saveTarget(GlobalChanFun, dat->chud[v].chan, sState);
		id = (int)dat->chud[v].parentItem;
		LWSAVE_U4(sState, &id, 1);
		fv[0] = (float)dat->chud[v].min;
		fv[1] = (float)dat->chud[v].max;
		LWSAVE_FP(sState,fv,2);
		VCPY_F(fv, dat->chud[v].color);
		LWSAVE_FP(sState,fv,3);
		LWSAVE_I4(sState,&dat->chud[v].flags,1);
		LWSAVE_STR(sState,dat->chud[v].label);
	}
	return NULL;
}

/* ----------------- Plug-in Methods: LWRenderFuncs  -----------------  */

XCALL_(static LWError)SlidersInit (LWInstance inst, int i)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static LWError)SlidersNewTime (LWInstance inst, LWFrame f, LWTime t)
{
	SlidersData	*dat = (SlidersData *)inst;
	double		 vec[3] = {0.0};
	int		 v;
	XCALL_INIT;
	dat->time = t;
	dat->frame = f;
	for(v=0; v<dat->num; v++)
		if(!dat->chud[v].chan)
		{
			if(dat->chud[v].chan = acquireTarget(GlobalChanFun, dat->chud[v].names, dat->chud[v].level))
			{
				GlobalChanFun->setChannelEvent(dat->chud[v].chan, chanEvent, dat);
				dat->chud[v].parentItem = findParentID(GlobalChanFun, GlobalItemInfo, dat->chud[v].chan);

			}
			else if(dat->chud[v].parentItem)
			{
				char *nam, *c;
				c = (char*)ItemGroupName(GlobalItemInfo, dat->chud[v].parentItem);
				if(c && strcmp(dat->chud[v].names[0], c) ) // name changes.. like loadfromscene
				{
					if(c && (nam=malloc(strlen(c)+2)) )
					{
						strcpy(nam, c);
						free(dat->chud[v].names[0]);
						dat->chud[v].names[0] = nam;
						if(dat->chud[v].chan = acquireTarget(GlobalChanFun, dat->chud[v].names, dat->chud[v].level))
						{
							int	len;
							GlobalChanFun->setChannelEvent(dat->chud[v].chan, chanEvent, dat);
			//  why overwrite custom label???
							if( !(dat->chud[v].flags&CHF_CUSTLABL) )
							{
								strncpy(dat->chud[v].label, chName(dat, v), 79);
								len = stringWidth(dat->chud[v].label);
								if(len>dat->maxLabel)
									dat->maxLabel = len;
							}
			
						}
					}

				}
			}
		}

	return NULL;
}

XCALL_(static void)SlidersCleanup (LWInstance inst)
{
	XCALL_INIT;
	return;
}

/* ----------------- Plug-in Methods: LWItemFuncs  -----------------  */

XCALL_(static const LWItemID *)SlidersUseItems (LWInstance inst)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static void)SlidersChangeID (SlidersData *dat, const LWItemID *items)
{
	const LWItemID	*id;
	int		 v, len;
	XCALL_INIT;
	for(id=items; id[0]!=LWITEM_NULL; id+=2)
	{
		if(dat->node.self==id[0])
		{
			dat->node.self = id[1];
			//break;
		}
		for(v=0; v<dat->num; v++)
			if(dat->chud[v].parentItem==id[0])
			{
				dat->chud[v].parentItem=id[1];
				if(CHAN(dat,v))
					if( !(dat->chud[v].flags&CHF_CUSTLABL) )
					{
						strncpy(dat->chud[v].label, chName(dat, v), 79);
						len = stringWidth(dat->chud[v].label);
						if(len>dat->maxLabel)
							dat->maxLabel = len;
					}
			}
	}
	return;
}



/* ----------------- Plug-in Methods: LWCustomObjHandler  -----------------  */
XCALL_(static unsigned int)SlidersFlags (SlidersData *dat)
{
	XCALL_INIT;
	return LWCOF_VIEWPORT_INDEX|LWCOF_NO_DEPTH_BUFFER;
}	

static int SLCSYS = LWCSYS_WORLD; //LWCSYS_OBJECT

// keep origin positioned such that move handle remains onscreen, 
// pixel coords relative to view center
// return bits for x,y changed
int constrainMoveHandle(SlidersData *dat, int viewport)
{
	int		limx, limy, x, y, res=3;

	GlobalView->rect(viewport, &limx, &limy, &x, &y);
	limx = x>>1;
	limy = y>>1;
	x = (int)(dat->orig[0] + dat->moveHandle[0])-8;
	y = (int)(dat->orig[1] + dat->moveHandle[1])+8;

	if(x < -limx)
		dat->orig[0] = -(limx + dat->moveHandle[0]-8);
	else if(x > limx)
		dat->orig[0] = (limx - dat->moveHandle[0]-8);
	else
		res -= 1;

	if(y < -limy)
		dat->orig[1] = -(limy + dat->moveHandle[1]-8);
	else if(y > limy)
		dat->orig[1] = (limy - dat->moveHandle[1]-8);
	else
		res -= 2;
	if(res)
	{
		dat->spot[viewport][0] = (short)dat->orig[0];
		dat->spot[viewport][1] = (short)dat->orig[1];
	}
	return res;
}

#define MIN_SLIDE_W	80
int constrainSizeHandle(SlidersData *dat, int viewport, int dx)
{
	int		limx, limy, x, y;

	GlobalView->rect(viewport, &limx, &limy, &x, &y);
	limx = x - 56;
	if( (dat->slideW>MIN_SLIDE_W && dx<0) || (dat->slideW<limx && dx>0))
		return limx;

	return 0;
}

void drawIcon(const LWCustomObjAccess *cob, Matrix m, LWDVector corn1, LWDVector corn2, int id, int flags)
{
	LWDVector	a,b,c, d, p;
	double		uv[][2] = {{0,0},{1,0},{1,1},{0,1}};
	HUD_Icon		*ike;

	if(id<ICON_MAX)
		ike = &(IconList[id]);
	else return;

	cob->setTexture(cob->dispData,ike->resW,ike->rgba);
	if(flags&1)
		cob->setUVs(cob->dispData,uv[3],uv[2],uv[1],uv[0]);
	else
		cob->setUVs(cob->dispData,uv[0],uv[1],uv[2],uv[3]);
	VCLR(d);
	d[2] = -1;
	MatrixApply(p, m,d); // raise icons off plane by 1 pixel

	VCPY(a,corn1);
	VCPY(b,corn1);
	VCPY(c,corn2);
	VCPY(d,corn2);
	b[0] = corn2[0];
	d[0] = corn1[0];
	HUD_Point(GlobalView, cob, m, a);
	HUD_Point(GlobalView, cob, m, b);
	HUD_Point(GlobalView, cob, m, c);
	HUD_Point(GlobalView, cob, m, d);
	VADD(a,p);
	VADD(b,p);
	VADD(c,p);
	VADD(d,p);
	cob->quad(cob->dispData, a, b, c, d, SLCSYS);
}

static int AddShadow = 1;
#define GLCHAR_WIDE	7
XCALL_(static void)SlidersEval (SlidersData *dat,	const LWCustomObjAccess *cobjAcc)
{
	LWEnvelopeID	env;	
	double		tv, val, orig[3] = {0.0,0.0,0.0}, vec[3], del[3], home[3];//, mat[9]= {0.0,0.0,0.0};
	int		i, fl;
	char		numbuf[32]="";
	float		bkg[4] = {0.25f, 0.25f, 0.25f, 0.15f }, knob[4];
	Matrix		xf;
	XCALL_INIT;	
	
	if(dat->flags&SLF_FOLDED)
		cobjAcc->setPattern(cobjAcc->dispData, LWLPAT_DOT);
	else if( !(dat->flags&SLF_ACTIVE) )
		cobjAcc->setPattern(cobjAcc->dispData, LWLPAT_LONGDOT);
//	if(dat->curView!=cobjAcc->view)
//		cobjAcc->setPattern(cobjAcc->dispData, LWLPAT_DASH);

	HUDPosition(GlobalView, cobjAcc->view, home, cobjAcc->viewDir);
	VCLR(dat->sizeHandle);
	dat->sizeHandle[1] = dat->num*SLIDE_HMARG*dat->slideH +8;
	dat->sizeHandle[0] = dat->slideW + 16;
	VCPY(dat->foldHandle, dat->sizeHandle);
	dat->foldHandle[0] += 16;
	VCPY(dat->envHandle, dat->sizeHandle);
	dat->envHandle[0] -= 16;
	VCLR(dat->moveHandle);
	dat->moveHandle[1] = dat->sizeHandle[1];
	dat->moveHandle[0] = -8;

	dat->orig[0] = dat->spot[cobjAcc->view][0];
	dat->orig[1] = dat->spot[cobjAcc->view][1];
	constrainMoveHandle(dat, cobjAcc->view);

	HUD_Transform(GlobalView, cobjAcc->view, xf);

	VCPY(vec, dat->moveHandle); 
	vec[0] += 8;
	VCPY(del, vec); 
	vec[1] -= 8;
	VADD(vec, dat->orig);
	VADD(del, dat->orig);
	HUD_Point(GlobalView, cobjAcc, xf, vec);
	HUD_Point(GlobalView,  cobjAcc,xf, del);
	VSUB(del, vec);
	VCPY(orig, dat->sizeHandle); 
	orig[0] -= 8;
	orig[1] -= 8;
	VADD(orig, dat->orig);
	HUD_Point(GlobalView, cobjAcc, xf, orig);

	VADD(vec, orig);
	VSCL(vec,0.5);
	VADDS(vec, del, 0.5);
	cobjAcc->text(cobjAcc->dispData, vec, GlobalItemInfo->name(dat->node.self), LWJUST_CENTER, SLCSYS);

	cobjAcc->setColor(cobjAcc->dispData, FullRGB);
	VCPY(vec, dat->sizeHandle);
	VADD(vec, dat->orig);
	vec[1] -= 8;
	vec[0] -= 8;
	VCPY(del, vec);
	del[0] += 16;
	del[1] += 16;
	drawIcon(cobjAcc, xf, del, vec, ICON_SIZE, 0);

	vec[0] += 32;

	VCPY(del, vec);
	del[0] -= 16;
	del[1] += 16;
	drawIcon(cobjAcc, xf, del, vec, dat->flags&SLF_FOLDED ? ICON_FOLD:ICON_UNFOLD, 0);
	VCPY(vec, dat->envHandle);
	VADD(vec, dat->orig);
	vec[1] -= 8;
	vec[0] += 8;
	VCPY(del, vec);
	del[0] -= 16;
	del[1] += 16;
	drawIcon(cobjAcc, xf, del, vec, ICON_ENV, 0);

	VCPY(vec, dat->moveHandle);
	vec[1] -= 8;
	vec[0] += 8;
	VADD(vec, dat->orig);
	VCPY(del, vec);
	del[0] -= 16;
	del[1] += 16;
	drawIcon(cobjAcc, xf, del, vec, ICON_DRAG, 0);

// Box around title
	VCPY(vec, dat->moveHandle);
	VADD(vec, dat->orig);
	vec[0] += 8;
	vec[1] += 8;
	VCPY(knob, bkg);
	knob[3] = 0.25f;
	cobjAcc->setColor(cobjAcc->dispData, knob);
	HUD_Box(GlobalView, cobjAcc, xf, vec, dat->slideW-8, 16, 1);
	if(dat->flags&SLF_FOLDED)
		return;

//	if(!(dat->flags&SLF_FOLDED))
	{
		VCPY(vec, dat->moveHandle);
		VADD(vec, dat->orig);
		vec[2] += 2;
		vec[1] -= 9;
		vec[0] -= 4+dat->maxLabel;//GLCHAR_WIDE*(1+dat->maxLabel);
		cobjAcc->setColor(cobjAcc->dispData, bkg);
		HUD_Box(GlobalView, cobjAcc, xf, vec, (4+dat->maxLabel)+dat->slideW+96, (dat->num)*SLIDE_HMARG*dat->slideH+2, 1);
		vec[1] += 17;
		HUD_Box(GlobalView, cobjAcc, xf, vec, (dat->maxLabel-4), SLIDE_HMARG*dat->slideH+1, 1);
		vec[0] = dat->sizeHandle[0] + dat->orig[0] + 8; 
		HUD_Box(GlobalView, cobjAcc, xf, vec, 64, SLIDE_HMARG*dat->slideH+1, 1);
	}

	checkAutoKey(dat);
	VCPY(orig, dat->orig); 
	orig[1] += (dat->num-1)*SLIDE_HMARG*dat->slideH + 2;

	for(i=0; i<dat->num; i++)
		if(dat->chud[i].chan)
		{
			cobjAcc->setColor(cobjAcc->dispData, dat->chud[i].color);
			VCPY(vec, orig);
			vec[1] += 4;
			HUD_Line(GlobalView, cobjAcc, xf, vec, dat->slideW, 0);
			if(0 && AddShadow)
			{
				HUD_Line(GlobalView, cobjAcc, xf, vec, dat->slideW-1, 1);
				cobjAcc->setColor(cobjAcc->dispData, dat->chud[i].color);
			}
			VCPY(vec, orig);
			vec[0] -= 8;
			if(0 & AddShadow)
			{
				VCPY(del, vec);
				del[0] +=1;
				del[1] -=1;
				del[2] +=1;
				HUD_Point(GlobalView, cobjAcc, xf, del);
				cobjAcc->setColor(cobjAcc->dispData,Shadow);
				cobjAcc->text(cobjAcc->dispData, del, dat->chud[i].label, LWJUST_RIGHT, SLCSYS);
				cobjAcc->setColor(cobjAcc->dispData, dat->chud[i].color);
			}
			HUD_Point(GlobalView, cobjAcc, xf, vec);
			cobjAcc->text(cobjAcc->dispData, vec, dat->chud[i].label, LWJUST_RIGHT, SLCSYS);
			fl = 0;
			env = CHENV(dat,i);
			if( env && (dat->chud[i].max > dat->chud[i].min))
			{
				VCPY(vec, orig);
				val = GlobalEnvelopeFuncs->evaluate(env, dat->time);
				tv = val;
			//	if( !(dat->flags&SLF_AUTOKEY) )
					if( !GlobalEnvelopeFuncs->findKey(env, dat->time) )
						fl |= HUDF_LOCK;

				if(val>dat->chud[i].max)
				{
					fl |= HUDF_OVERMAX;
					if(dat->flags&SLF_AUTORANGE)
						dat->chud[i].max = val;
					else
						val = dat->chud[i].max;
				}
				if(val<dat->chud[i].min)
				{
					fl |= HUDF_UNDERMIN;
					if(dat->flags&SLF_AUTORANGE)
						dat->chud[i].min = val; 
					else
						val = dat->chud[i].min;
				}
				vec[0] = (val - dat->chud[i].min)/(dat->chud[i].max - dat->chud[i].min);
				vec[0] *= dat->slideW;
				vec[0] += orig[0];
				vec[1] += 4;
				fl |= dat->chud[i].flags&CHF_CURRENT ? HUDF_FILL:0;
				VCPY(knob, dat->chud[i].color);
				VSCL(knob, 1.25);
				knob[3] = 1;
				if((dat->chud[i].flags&CHF_CURRENT))
				{
					VADDS(knob,Shadow,2);
					knob[0] = MIN(knob[0], 1);
					knob[1] = MIN(knob[1], 1);
					knob[2] = MIN(knob[2], 1);
				}
				if(fl&HUDF_LOCK)
					knob[3] = 0.33333f;

				cobjAcc->setColor(cobjAcc->dispData, knob); // needed for sprite color
				VCPY(del, vec);
				del[0] += 4;
				vec[0] -= 4;
				del[1] += 7;
				vec[1] -= 7;
				drawIcon(cobjAcc, xf, del, vec, ICON_KNOB,0);
				cobjAcc->setColor(cobjAcc->dispData, FullRGB); // needed for sprite color
				vec[0] += 4;
				vec[1] += 7;

				VCPY(vec, orig);
				vec[0] += dat->slideW+8;
				vec[1] -= 2;
				VCPY(del,vec);
				del[0] += 16;
				del[1] += dat->slideH;
				drawIcon(cobjAcc, xf, del, vec, ICON_FOLD, 0);

				vec[0] += 32;
				VCPY(del,vec);
				del[0] -= 16;
				del[1] += dat->slideH;
				drawIcon(cobjAcc, xf,del, vec, ICON_FOLD, 0);
				vec[0] += 4;
				vec[1] += 2;

				if(fl&(HUDF_UNDERMIN|HUDF_OVERMAX))
					cobjAcc->setColor(cobjAcc->dispData,HotRed);
				else
					cobjAcc->setColor(cobjAcc->dispData,dat->chud[i].color);

				if(dat->chud[i].flags&CHF_ANGLE)
					sprintf(numbuf, "%.2f%c", DEGREES(tv),DEGREE_CHAR);
				else if(dat->chud[i].flags&CHF_PERCENT)
					sprintf(numbuf, "%.2f%c", 100*tv, '%');
				else
					sprintf(numbuf, "%.3f", tv);

				
				if(0 && AddShadow)
				{
					VCPY(del, vec);
					del[0] +=1;
					del[1] -=1;
					del[2] +=1;
					HUD_Point(GlobalView, cobjAcc, xf, del);
					cobjAcc->setColor(cobjAcc->dispData,Shadow);
					cobjAcc->text(cobjAcc->dispData, del, numbuf, LWJUST_LEFT, SLCSYS);
					cobjAcc->setColor(cobjAcc->dispData, dat->chud[i].color);
				}
				HUD_Point(GlobalView, cobjAcc, xf, vec);
				cobjAcc->text(cobjAcc->dispData, vec, numbuf, LWJUST_LEFT, SLCSYS);
			}
			orig[1] -= SLIDE_HMARG*dat->slideH;
		}
}


/* -----------------                 -----------------  */


XCALL_(int) Sliders (
	long			 	 version,
	GlobalFunc			*global,
	LWCustomObjHandler	*local,
	void				*serverData)
{
	XCALL_INIT;
	if (version != LWCUSTOMOBJ_VERSION)
		return (AFUNC_BADVERSION);

	GlobalGlobal = global;
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
 	GlobalChanFun = (*global) (LWCHANNELINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalChanFun )
	{
		(*Gmessage->error)("Can't get global",LWCHANNELINFO_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
	GlobalEnvelopeFuncs = (*global) (LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalEnvelopeFuncs )
		return AFUNC_BADGLOBAL;

	GlobalLWUI = (*global) (LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalLWUI)
	{
		(*Gmessage->error)("Can't get global",LWINTERFACEINFO_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
	GlobalView = (*global) (LWVIEWPORTINFO_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalView)
	{
		(*Gmessage->error)("Can't get global",LWVIEWPORTINFO_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
	GlobalPanFun = (LWPanelFuncs*)(*global)( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalPanFun)
	{
		(*Gmessage->error)("Can't get global",LWPANELFUNCS_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
	local->inst->create  = SlidersCreate;
	local->inst->destroy = SlidersDestroy;
	local->inst->load    = SlidersLoad;
	local->inst->save    = SlidersSave;
	local->inst->copy    = SlidersCopy;
	local->inst->descln	 = SlidersDescribe;

	local->item->useItems    = SlidersUseItems;
	local->item->changeID    = SlidersChangeID;

	local->rend->init		= SlidersInit;
	local->rend->newTime    = SlidersNewTime;
	local->rend->cleanup	= SlidersCleanup;

	local->evaluate 	= SlidersEval;
	local->flags 		= SlidersFlags;
	return (AFUNC_OK);
}

/* -----------------  User Interface  ----------------- */

static const char *richtextColorString( LWDVector rgb )
{
	int n=0, t;
	unsigned char colr[3];
	double v;
	static char	buf[30]="";

	for(t=0; t<3; t++)
	{
		v = 255*rgb[t] + 0.5;
		colr[t] = (unsigned char)((int)CLAMP(v,0,255));
	}
	n = RGB_(colr[0], colr[1], colr[2]);
	sprintf(buf,"\03(i:140,%d)",n); 
	return buf;
}

static int popCnt_Color( void *dat )
{
	return MAX_COLOR_PRE+1;
}

static char *popName_Color( void *dat, int idx )
{
	static char	buf[30]="";
	if(idx==MAX_COLOR_PRE)
		return "   Custom";
	else
	{
		sprintf(buf,"\03(i:140,%d) %s",colorRGBs[idx], colorNames[idx] ); 
		return buf;
	}
}


char *chName(SlidersData  *dat, int i)
{
	static char namBuf[255];
	if(!dat) 
		return "";
	if(i>=0 && i<dat->num && dat->chud[i].chan)
	{
		sprintf(namBuf,"%s:%s",
			GlobalChanFun->groupName(GlobalChanFun->channelParent(dat->chud[i].chan)),
			GlobalChanFun->channelName(dat->chud[i].chan));
		return	namBuf;
	}
	else return "";
}

char *chListName(SlidersData  *dat, int i)
{
	static char namBuf[255];
	if(!dat) 
		return "";
	if(i>=0 && i<dat->num && dat->chud[i].chan)
	{
		sprintf(namBuf,"%s:%s",
			GlobalChanFun->groupName(GlobalChanFun->channelParent(dat->chud[i].chan)),
			GlobalChanFun->channelName(dat->chud[i].chan));
		if(strcmp(namBuf,  dat->chud[i].label))
		{
			strncat(namBuf, " : ",sizeof(namBuf));
			strncat(namBuf,  dat->chud[i].label,sizeof(namBuf));
		}
		return	namBuf;
	}
	else return "";
}

int chCount(SlidersData  *dat)
{
	if(!dat) 
		return 0;
	return dat->num;
}



void	maxEvent(LWControl *ctl, SlidersData  *dat)
{
	double			t;
	GET_FLOAT(ctl,t);
	if(dat->chud[dat->cur].flags&CHF_ANGLE)
		dat->chud[dat->cur].max = RADIANS(t);
	else if(dat->chud[dat->cur].flags&CHF_PERCENT)
		dat->chud[dat->cur].max = 0.01*(t);
	else
		dat->chud[dat->cur].max = (t);
}

void	minEvent(LWControl *ctl, SlidersData  *dat)
{
	double			t;
	GET_FLOAT(ctl,t);
	if(dat->chud[dat->cur].flags&CHF_ANGLE)
		dat->chud[dat->cur].min = RADIANS(t);
	else if(dat->chud[dat->cur].flags&CHF_PERCENT)
		dat->chud[dat->cur].min = 0.01*(t);
	else
		dat->chud[dat->cur].min = (t);
}

void	labEvent(LWControl *ctl, SlidersData  *dat)
{
	char			buf[80];
	int			i, len;
	GET_STR(ctl,buf,79);
	if(strncmp(dat->chud[dat->cur].label, buf,79))
		dat->chud[dat->cur].flags |= CHF_CUSTLABL;
	else
		return;
	strcpy(dat->chud[dat->cur].label, buf);
	dat->maxLabel = 0;
	for(i=0; i<dat->num; i++)
	{
		len = stringWidth(dat->chud[i].label);//strlen(dat->chud[i].label);
		if(len>dat->maxLabel)
			dat->maxLabel = len;
	}
	RENDER_CON(dat->list);
}

void	preEvent(LWControl *ctl, SlidersData  *dat)
{
	int			i, rgb[3];
	GET_INT(ctl,i);
	if(i<MAX_COLOR_PRE)
		VCPY(dat->chud[dat->cur].color, (&(fRGBs[3*i])) );
	for(i=0; i<3; i++)
		rgb[i] = (int)(255.0*dat->chud[dat->cur].color[i] + 0.5);
	SETV_IVEC(dat->rgb, rgb);
}

void	rgbEvent(LWControl *ctl, SlidersData  *dat)
{
	int			color[3];
	GETV_IVEC(ctl,color);
	dat->chud[dat->cur].color[0] = (float)color[0];
	dat->chud[dat->cur].color[0] /= 255.0;
	dat->chud[dat->cur].color[1] = (float)color[1];
	dat->chud[dat->cur].color[1] /= 255.0;
	dat->chud[dat->cur].color[2] = (float)color[2];
	dat->chud[dat->cur].color[2] /= 255.0;
	SET_INT(dat->pre, MAX_COLOR_PRE);
}

void showChannel(SlidersData  *dat)
{
	int			i, rgb[3];
	SET_INT(dat->list, dat->cur);
	if(dat->chud[dat->cur].flags&CHF_ANGLE)
	{
		SET_FLOAT(dat->max, DEGREES(dat->chud[dat->cur].max));
		SET_FLOAT(dat->min, DEGREES(dat->chud[dat->cur].min));
	}
	else if(dat->chud[dat->cur].flags&CHF_PERCENT)
	{
		SET_FLOAT(dat->max, 100*(dat->chud[dat->cur].max));
		SET_FLOAT(dat->min, 100*(dat->chud[dat->cur].min));
	}
	else
	{
		SET_FLOAT(dat->max, dat->chud[dat->cur].max);
		SET_FLOAT(dat->min, dat->chud[dat->cur].min);
	}

	for(i=0; i<3; i++)
		rgb[i] = (int)(255.0*dat->chud[dat->cur].color[i] + 0.5);
	SETV_IVEC(dat->rgb, rgb);
	SET_STR(dat->lab, dat->chud[dat->cur].label, 80);
	RENDER_CON(dat->list);
	RENDER_CON(dat->lab);
	RENDER_CON(dat->rgb);
}

int addChannel(SlidersData  *dat, LWChannelID chan)
{
	int			t, len;
	LWItemID		id;
	if(chan && dat->num<MAX_CHANS)
	{
		id = findParentID(GlobalChanFun, GlobalItemInfo, chan);
		for(t=0; t<dat->num; t++) // prevent duplicate entries!
			if(	dat->chud[t].chan==chan || 
				( (dat->chud[t].parentItem==id) && channelNamesMatch(GlobalChanFun, dat->chud[t].chan, chan) ))
			{
				Gmessage->warning("Duplicate channel ID or channel name", GlobalChanFun->channelName(chan));
				return 0;
			}

		dat->chud[dat->num].chan = chan;
		dat->chud[dat->num].level = findNames(GlobalChanFun, chan, dat->chud[dat->num].names, 30);
		dat->chud[dat->num].min = 0.0;
		t = GlobalChanFun->channelType(dat->chud[dat->num].chan);
		if(t==LWET_ANGLE)
		{
			dat->chud[dat->num].max = PI+PI;
			dat->chud[dat->num].flags |= CHF_ANGLE;
		}
		else
		{
			dat->chud[dat->num].max = 1.0;
			if(t==LWET_PERCENT)
				dat->chud[dat->num].flags |= CHF_PERCENT;
		}
		dat->cur = dat->num++;
		GlobalChanFun->setChannelEvent(dat->chud[dat->cur].chan, chanEvent, dat);
		dat->chud[dat->cur].parentItem = id;
		strncpy(dat->chud[dat->cur].label, chName(dat, dat->cur), 79);
		
		len = stringWidth(dat->chud[dat->cur].label);//strlen(dat->chud[dat->cur].label);
		if(len>dat->maxLabel)
			dat->maxLabel = len;
		t = CLAMP((MAX_COLOR_PRE-dat->cur-1),0,MAX_COLOR_PRE-1);
		VCPY(dat->chud[dat->cur].color, &(fRGBs[3*t]) );
		dat->chud[dat->cur].color[3] = 1;
		SET_INT(dat->pre, t);
		RENDER_CON(dat->list);
		return 1;
	}
	return 0;
}

void	upEvent(LWControl *ctl, SlidersData  *dat)
{
	int			t;
	CHUD			spare;
	if( dat->cur>0 )
	{
		t = dat->cur - 1;
		spare = dat->chud[t];
		dat->chud[t] = dat->chud[dat->cur];
		dat->chud[dat->cur] = spare;
		dat->cur--;
		SET_INT(dat->list, dat->cur);
	}
}

void	downEvent(LWControl *ctl, SlidersData  *dat)
{
	int			t;
	CHUD			spare;
	t = dat->cur + 1;
	if( t<dat->num )
	{
		spare = dat->chud[t];
		dat->chud[t] = dat->chud[dat->cur];
		dat->chud[dat->cur] = spare;
		dat->cur++;
		SET_INT(dat->list, dat->cur);
	}
}


void	lstEvent(LWControl *ctl, SlidersData  *dat)
{
	int			t;
	GET_INT(ctl,t);
	if( t>=0 && t<dat->num && dat->cur!=t )
	{
		dat->cur = t;
		SET_INT(dat->pre, MAX_COLOR_PRE);
		showChannel(dat);
		SET_INT(dat->chanTree, (int)dat->chud[dat->cur].chan);
	}
}


void	chTreeEvent(LWControl *ctl, SlidersData  *dat)
{
	int			t;//, i, rgb[3];
	GET_INT(ctl,t);
	addChannel(dat, (LWChannelID) t);
	showChannel(dat);
}

void	addEvent(LWControl *ctl, SlidersData  *dat)
{
	int			t;//, i, rgb[3];
	GET_INT(dat->chanTree,t);
	addChannel(dat, (LWChannelID) t);
	showChannel(dat);
}

void	remEvent(LWControl *ctl, SlidersData  *dat)
{
	int			t;
	if(dat->cur>=0 && dat->num)
	{
		GlobalChanFun->setChannelEvent(dat->chud[dat->cur].chan, NULL, dat);
		freeNames(dat->chud[dat->cur].names, dat->chud[dat->cur].level);
		for(t = dat->cur; t<dat->num; t++)
		{
			dat->chud[t] = dat->chud[t+1];
		}
		dat->num--;
		if(dat->cur)
		{
			dat->cur--;
			SET_INT(dat->pre, MAX_COLOR_PRE);
			showChannel(dat);
		}
		RENDER_CON(dat->list);
	}
}

XCALL_(static LWError) panOptions(SlidersData 	*dat)
{
	LWPanelID		panID;
	XCALL_INIT;
	if(panID = PAN_CREATE(GlobalPanFun,"Sliders"))	
	{
		LWControl	*chan, *lst,*add,*rem, *min, *max, *rgb, *lab, *up, *down;
		int x,y,x1 = 160, t, w;
		if(!(chan = CHANNEL_CTL(GlobalPanFun,panID," All Channels",256,190) ))
			goto controlError;	
		dat->chanTree = chan;
		CON_SETEVENT(chan, chTreeEvent, dat);

		if(!(add = WBUTTON_CTL(GlobalPanFun,panID,"Add Channel", 256) ))
			goto controlError;
		CON_SETEVENT(add, addEvent, dat);

		if(!(lst = LISTBOX_CTL(GlobalPanFun,panID,"Selected Channels",252,11,chListName,chCount) ))
			goto controlError;	
		CON_SETEVENT(lst, lstEvent, dat);
		dat->list = lst;
		x = CON_X(chan);
		x += CON_W(chan) + 4;
		y = CON_Y(chan);
		MOVE_CON(lst,x,y);
		w = CON_W(lst);
		if(!(up = WBUTTON_CTL(GlobalPanFun,panID,"Move Up", w/2 - 2) ))
			goto controlError;
		y += CON_H(lst) - 6;
		MOVE_CON(up,x,y);
		t = x + CON_W(up);
		CON_SETEVENT(up, upEvent, dat);
		if(!(down = WBUTTON_CTL(GlobalPanFun,panID,"Move Down", w/2 - 2) ))
			goto controlError;
		MOVE_CON(down,t,y);
		CON_SETEVENT(down, downEvent, dat);

		if(!(rem = WBUTTON_CTL(GlobalPanFun,panID,"Remove Channel", w) ))
			goto controlError;
		CON_SETEVENT(rem, remEvent, dat);
		y += CON_H(up);
		MOVE_CON(rem,x,y);
		if(!(min = FLOAT_CTL(GlobalPanFun,panID,"Range Min") ))
			goto controlError;
		dat->min = min;
		CON_SETEVENT(min, minEvent, dat);
		y += CON_H(rem);
		t = x - CON_LW(min);
		MOVE_CON(min,t,y);
		if(!(max = FLOAT_CTL(GlobalPanFun,panID,"Range Max") ))
			goto controlError;
		dat->max = max;
		CON_SETEVENT(max, maxEvent, dat);
		y += CON_H(min);
		t = x - CON_LW(max);
		MOVE_CON(max,t,y);

		if(!(lab = STR_CTL(GlobalPanFun,panID,"Label", 45) ))
			goto controlError;
		dat->lab = lab;
		CON_SETEVENT(lab, labEvent, dat);
		y += CON_H(max);
		t = x - CON_LW(lab);
		MOVE_CON(lab,t,y);

		if(!(rgb = POPDOWN_CTL(GlobalPanFun,panID,"Color Preset", colorItems) ))
			goto controlError;
		dat->pre = rgb;
		CON_SETEVENT(dat->pre, preEvent, dat);
		SET_INT(dat->pre, MAX_COLOR_PRE);
		t = x - CON_W(rgb) - 8;
		y += CON_H(lab);
		MOVE_CON(rgb,t,y);

		if(!(rgb = MINIRGB_CTL(GlobalPanFun,panID," ") ))
			goto controlError;
		dat->rgb = rgb;
		CON_SETEVENT(rgb, rgbEvent, dat);
		t = x - CON_LW(rgb);
		MOVE_CON(rgb,t,y);
		PAN_SETH(GlobalPanFun, panID, 0);

		if((*GlobalPanFun->open)(panID,PANF_BLOCKING))
		{
			PAN_KILL(GlobalPanFun,panID);
			return NULL;
		}
controlError:
		PAN_KILL(GlobalPanFun,panID);
		return "Yow!";

	}
	return NULL;
}


XCALL_(int) Sliders_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	XCALL_INIT;
	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);
	GlobalPanFun = (LWPanelFuncs*)(*global)( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalPanFun)
		return (AFUNC_BADGLOBAL);

	UI->panel	= NULL;
	UI->options	= panOptions;
	UI->command	= NULL; 
	return AFUNC_OK;
}



// ----------- Sliders Layout Tool --------------------------

XCALL_(void) SlidersTool_done(SlidersData *dat)
{
	XCALL_INIT;
	dat->node.flags &= ~NHEADF_LOCKED; // unlocked when not in use!
}

static int xax[] = {1,2,0}, yax[] = {2,0,1}, zax[] = {0,1,2};
XCALL_(void) SlidersTool_draw(SlidersData *dat, LWCustomObjAccess *cob)
{
	XCALL_INIT;
}

XCALL_(const char *) SlidersTool_help(SlidersData *dat, LWToolEvent *eve)
{
	static char	help[128];
	XCALL_INIT;
	sprintf(help,"Drag sliders to adjust channels");
	return help;
}

XCALL_(int) SlidersTool_dirty(SlidersData *dat)
{
	XCALL_INIT;
	return dat->dirty;
}


XCALL_(int) SlidersTool_count(SlidersData *dat, LWToolEvent *eve)
{
	XCALL_INIT;
	return 3*dat->num+4;
}

XCALL_(int) SlidersTool_handle(SlidersData *dat, LWToolEvent *eve, int i, LWDVector pos)
{
	LWDVector		vec, dir, home;
	double			val;
	Matrix			xf;
	int			h, c
	HUD_Transform(GlobalView, eve->portAxis, xf);
	VCLR(vec);  
	vec[2] = 1;  // get view dir by transforming Z vector
	MatrixApply(dir, xf, vec);
	HUDPosition(GlobalView, eve->portAxis, home, dir); // initialize home
	dat->orig[0] = dat->spot[eve->portAxis][0];
	dat->orig[1] = dat->spot[eve->portAxis][1];
	if(i<3*dat->num)
	{
		c = i/3; // channel
		h = i%3; // handle: knob, prev, next
		VCLR(vec);
		if(h==1)
		{
			VCPY(vec, dat->sizeHandle);
			vec[1] = 4+dat->slideH*SLIDE_HMARG*(dat->num-1-c);
			VADD(vec, dat->orig);
			MatrixApply(pos, xf, vec);
			VADD(pos, home);
			return 2;
		}
		else if(h==2)
		{
			VCPY(vec, dat->foldHandle);
			vec[1] = 4+dat->slideH*SLIDE_HMARG*(dat->num-1-c);
			VADD(vec, dat->orig);
			MatrixApply(pos, xf, vec);
			VADD(pos, home);
			return 2;
		}
		if(CHENV(dat,c))
		{
			val = GlobalEnvelopeFuncs->evaluate(CHENV(dat,c), dat->time);

			if(val>dat->chud[c].max)
				val = dat->chud[c].max;
			if(val<dat->chud[c].min)
				val = dat->chud[c].min;
		}
		else
			val = dat->chud[c].min;

		vec[0] += (val - dat->chud[c].min)/(dat->chud[c].max - dat->chud[c].min);
		vec[0] *= dat->slideW;
		vec[1] = 4+dat->slideH*SLIDE_HMARG*(dat->num-1-c);

		VADD(vec, dat->orig);
		MatrixApply(pos, xf, vec);
		VADD(pos, home);
		return 4;
	}
	else if(i==3*dat->num)
	{
		VCPY(vec, dat->sizeHandle);
		VADD(vec, dat->orig);
		MatrixApply(pos, xf, vec);
		VADD(pos, home);
		return 1;
	}
	else if(i==3*dat->num+1)
	{
		VCPY(vec, dat->moveHandle);
		VADD(vec, dat->orig);
		MatrixApply(pos, xf, vec);
		VADD(pos, home);
		return 1;
	}
	else if(i==3*dat->num+2)
	{
		VCPY(vec, dat->foldHandle);
		VADD(vec, dat->orig);
		MatrixApply(pos, xf, vec);
		VADD(pos, home);
		return 1;
	}
	else if(i==3*dat->num+3)
	{
		VCPY(vec, dat->envHandle);
		VADD(vec, dat->orig);
		MatrixApply(pos, xf, vec);
		VADD(pos, home);
		return 1;
	}
	else
		return -1;
}

XCALL_(int) SlidersTool_start(SlidersData *dat, LWToolEvent *eve)
{
	XCALL_INIT;
	return 0;
}

XCALL_(int) SlidersTool_adjust(SlidersData *dat, LWToolEvent *eve, int i)
{
	double				val;
	LWEnvKeyframeID			key;
	int				h,c, w;
	LWDVector			vec, dir, home;
	Matrix				xf;

	XCALL_INIT;
	HUD_Transform(GlobalView, eve->portAxis, xf);
	VCLR(vec);
	vec[2] = 1; // get view dir by transforming Z vector
	MatrixApply(dir, xf, vec);
	HUDPosition(GlobalView, eve->portAxis, home, dir); // initialize home
	dat->orig[0] = dat->spot[eve->portAxis][0];
	dat->orig[1] = dat->spot[eve->portAxis][1];
	dat->curView = eve->portAxis;
	if(i<3*dat->num)
	{
		c = i/3; // channel
		h = i%3; // handle: knob, prev, next
		dat->chud[c].flags |= CHF_CURRENT;
		if(!CHENV(dat,c))
			return i;
		if( !(dat->chud[c].flags&CHF_HOT) )
		{
			dat->chud[c].v0 = GlobalEnvelopeFuncs->evaluate(CHENV(dat,c), dat->time); 
			dat->chud[c].flags |= CHF_HOT;
		}
		if(dat->cur != c)
		{
			dat->chud[dat->cur].flags &= ~CHF_CURRENT;
			dat->cur = c;
		}
		dat->dirty = LWT_DIRTY_WIREFRAME;
		if(h==1)
		{
			double newt;
			newt = prevKeyTime(CHENV(dat,c), dat->time);
			if(newt<dat->time)
				setLayoutTime(newt);
		}
		else if(h==2)
		{
			double newt;
			newt = nextKeyTime(CHENV(dat,c), dat->time);
			if(newt>dat->time)
				setLayoutTime(newt);
			
		}
		else
		{
			val = (eve->dx) * (dat->chud[c].max - dat->chud[c].min)/dat->slideW;
			val += dat->chud[c].v0;
			if( !(dat->flags&SLF_AUTORANGE) )
				val = CLAMP(val, dat->chud[c].min, dat->chud[c].max);
			key = GlobalEnvelopeFuncs->findKey(CHENV(dat,c), dat->time);
			if(key)
				GlobalEnvelopeFuncs->keySet(CHENV(dat,c), key, LWKEY_VALUE ,&val);
			else if(dat->flags&SLF_AUTOKEY)
				key = GlobalEnvelopeFuncs->createKey (CHENV(dat,c), dat->time, val);
		}
	
	}
	else if(i==3*dat->num)
	{
		if(eve->dx)
		{
			if(w=constrainSizeHandle(dat, eve->portAxis, eve->dx))
			{
				dat->slideW = dat->tw + eve->dx;
				if(dat->slideW<MIN_SLIDE_W)
					dat->slideW = MIN_SLIDE_W;
				else if(dat->slideW>w)
					dat->slideW = w;
				dat->dx = eve->dx;
			}
		}
	}
	else if(i==3*dat->num+2)
	{
		if(!(dat->flags&SLF_FLOCK))
		{
			dat->flags |= SLF_FLOCK;
			if( dat->flags&SLF_FOLDED )
				dat->flags &= ~SLF_FOLDED;	
			else
				dat->flags |= SLF_FOLDED;
		}
	}
	else if(i==3*dat->num + 1)// move sliders
	{
		dat->orig[0] = dat->offset[0] + eve->dx;	
		dat->orig[1] = dat->offset[1] - eve->dy;
		if(!constrainMoveHandle(dat, eve->portAxis))
		{
			dat->spot[eve->portAxis][0] = (short)dat->orig[0];
			dat->spot[eve->portAxis][1] = (short)dat->orig[1];
		}
	
	}
	else if(i==3*dat->num+3) // envHandle
	{
		if(CHENV(dat,0))
			GlobalEnvelopeFuncs->edit(NULL, CHENV(dat,0), 0, dat);
		for(c=1; c<dat->num; c++)
			if(CHENV(dat,c))
				addGraphEnv(CHENV(dat,c), c ? 1:0);
	}
	else
		dat->curView = -1;
	return i;
}

XCALL_(int) SlidersTool_down(SlidersData *dat, LWToolEvent *eve)
{
	int		i;
	XCALL_INIT;
	checkAutoKey(dat);

	dat->flags |= SLF_XFINIT;
	dat->flags &= ~SLF_MOVING;
	dat->flags &= ~SLF_FLOCK; // debounce fold button
	dat->dirty = LWT_DIRTY_WIREFRAME;
	dat->tw = dat->slideW;
	dat->dx	= 0;
	dat->curView	= -1;
	dat->orig[0] = dat->spot[eve->portAxis][0];
	dat->orig[1] = dat->spot[eve->portAxis][1];
	dat->offset[0] = dat->orig[0];
	dat->offset[1] = dat->orig[1];
	for(i=0; i<dat->num; i++)
		dat->chud[i].flags &= ~(CHF_HOT|CHF_CURRENT);
	if(eve->flags&LWTOOLF_ALT_BUTTON)
		dat->flags |= SLF_MOVING;
	return 0;
}

XCALL_(void) SlidersTool_event(SlidersData *dat, int code)
{
	XCALL_INIT;
	switch(code)
	{
		case LWT_EVENT_DROP:
			dat->flags &= ~SLF_ACTIVE;
			break;
		case LWT_EVENT_RESET:		
			dat->flags |= SLF_ACTIVE;
			break;
		case LWT_EVENT_ACTIVATE:
			dat->flags |= SLF_ACTIVE;
			break;
	}
}

XCALL_(LWXPanelID) SlidersTool_panel(SlidersData *dat)
{
	LWXPanelID		panID = NULL;
	XCALL_INIT;
	return panID;
}


XCALL_(int) Sliders_Tool (
	long		 	 version,
	GlobalFunc		*global,
	LWLayoutTool	*local,
	void			*serverData)
{
	LWItemID		 sel=LWITEM_NULL,*sid;
	SlidersData		*dat=NULL;
	XCALL_INIT;
	if (version != LWLAYOUTTOOL_VERSION)
		return (AFUNC_BADVERSION);

	Gmessage = (*global) (LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!Gmessage )
		return AFUNC_BADGLOBAL;

	GlobalEnvelopeFuncs = (*global) (LWENVELOPEFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalEnvelopeFuncs )
		return AFUNC_BADGLOBAL;
	GlobalChanFun = (*global) (LWCHANNELINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalChanFun )
		return AFUNC_BADGLOBAL;
	GlobalItemInfo = (*global) (LWITEMINFO_GLOBAL, GFUSE_TRANSIENT);
	if (!GlobalItemInfo )
		return AFUNC_BADGLOBAL;
	GlobalLWUI = (*global) (LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalLWUI)
	{
		(*Gmessage->error)("Can't get global",LWINTERFACEINFO_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
	GlobalGlobal = global;

	fixNodeNames(&SliderGlobalNodeList, NULL);

	for(sid = (LWItemID*)GlobalLWUI->selItems; *sid; sid++)
	{
		dat = (SlidersData*)findNode(&SliderGlobalNodeList, *sid);
		if(dat)
			break;
	}
	if(!dat)
		for(sel=GlobalItemInfo->first(LWI_OBJECT, LWITEM_NULL); sel!=LWITEM_NULL; sel=GlobalItemInfo->next(sel) )
		{
			dat = (SlidersData*)findNode(&SliderGlobalNodeList, sel);
			if(dat)
				break;
		}
	if(!dat)
	{ // should give applyserver option
		(*Gmessage->warning)("No Sliders Custom Object Selected","Select object or apply Plug-in");
		return AFUNC_BADLOCAL;
	}
	local->instance = dat;
	local->tool->done		 = SlidersTool_done;
	local->tool->draw		 = SlidersTool_draw;
	local->tool->help		 = SlidersTool_help;
	local->tool->dirty		 = SlidersTool_dirty;
	local->tool->count		 = SlidersTool_count;
	local->tool->handle		 = SlidersTool_handle;
	local->tool->start		 = SlidersTool_start;
	local->tool->adjust		 = SlidersTool_adjust;
	local->tool->down		 = SlidersTool_down;
	local->tool->event		 = SlidersTool_event;
	local->tool->panel		 = SlidersTool_panel;
	return AFUNC_OK;
}
