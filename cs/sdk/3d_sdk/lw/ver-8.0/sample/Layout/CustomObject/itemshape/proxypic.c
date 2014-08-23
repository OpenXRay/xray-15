// ProxyPick.c - Custom Object Plug-ins
// Arnie Cachelin, Copyright 2001 NewTek, Inc.
// 

#include <lwhost.h>
#include <lwserver.h>
#include <lwhandler.h>
#include <lwrender.h>
#include <lwmaster.h>
#include <lwmath.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <string.h>

static	LWXPanelFuncs	*GlobalXPanFun		= NULL;
static	LWMessageFuncs	*Gmessage			= NULL;
static	LWItemInfo		*GlobalItemInfo		= NULL;
static	LWSceneInfo		*GlobalSceneInfo	= NULL;
static	LWCameraInfo	*GlobalCameraInfo   = NULL;
static	LWInstUpdate	*GlobalLWUpdate		= NULL;
static	LWInterfaceInfo		*GlobalLWUI=NULL;
static  GlobalFunc			*GlobalGlobal=NULL;

static char *errAllocFailed ="Tiny Allocation Failed";

/* ----------------- Plug-in Instance Definition  -----------------  */
#define PRXF_PAUSE		1
#define PRXF_LINK		2
#define PRXF_LABELED	4

typedef struct st_ProxyPickData {
	void				*ctxt;
	LWItemID			 proxy, target, oldProxy, oldTarget;
	int					 flags;
	char				 desc[200];
} ProxyPickData;

int setItemTag(LWItemInfo *itInfo, LWItemID id, char *key, char *val);
int getItemTag(LWItemInfo *itInfo, LWItemID id, char *key, char *val, int len);
void killItemTag(LWItemInfo *itinfo, LWItemID id, char *key);

int LW_Execute(const char *command)
{
	int		(*exCmd)(const char *cmd);
	int		t=0;

	if(command && *command)
		if(exCmd=(*GlobalGlobal)("LW Command Interface", GFUSE_TRANSIENT))
			t=(*exCmd)(command);
	return t; 
}


/* ----------------- Plug-in Methods: LWInstanceFuncs  -----------------  */

XCALL_(static LWInstance)ProxyPickCreate(void *data, void *ctxt, LWError *err)
{
	ProxyPickData *dat=NULL;
	XCALL_INIT;
	if(dat=malloc(sizeof(ProxyPickData)))
	{
		memset(dat,0,sizeof(*dat));
		dat->ctxt = data;
		sprintf(dat->desc," %s", "(none)");
	}
	else
		*err = errAllocFailed;
	return dat;
}

XCALL_(static const char *)ProxyPickDescribe(ProxyPickData *dat)
{
	XCALL_INIT;
	if(dat->target && dat->proxy)
	{
		const char *p;
		p = GlobalItemInfo->name(dat->proxy);
		sprintf(dat->desc," %s >--> ",	p);
		p = GlobalItemInfo->name(dat->target);
		strncat(dat->desc,p, sizeof(dat->desc)-1);
	}
	else
		sprintf(dat->desc," (none)");
	return (dat->desc);
}

XCALL_(static void)ProxyPickDestroy(ProxyPickData *dat)
{
	XCALL_INIT;
	if(dat)
	{
		free(dat);
	}
}

XCALL_(static LWError)ProxyPickCopy(ProxyPickData	*to, ProxyPickData	*from)
{
	XCALL_INIT;

	*to = *from;
	return NULL;
}

XCALL_(static LWError)ProxyPickLoad(ProxyPickData *dat,const LWLoadState	*lState)
{
	unsigned int	v;
	XCALL_INIT;
	LWLOAD_I4(lState,&v,1);
	LWLOAD_U4(lState,&v,1);
	dat->target = (LWItemID) v;
	LWLOAD_U4(lState,&v,1);
	dat->proxy = (LWItemID) v;
	LWLOAD_I4(lState,&dat->flags,1);
	return NULL; 
}

XCALL_(LWError)ProxyPickSave(ProxyPickData *dat,const LWSaveState	*sState)
{
	unsigned int	v = 1;
	XCALL_INIT;
	LWSAVE_I4(sState,&v,1);
	v = (unsigned int)dat->target;
	LWSAVE_U4(sState,&v,1);
	v = (unsigned int)dat->proxy;
	LWSAVE_U4(sState,&v,1);
	LWSAVE_I4(sState,&dat->flags,1);
	return NULL; 
}

/* ----------------- Plug-in Methods: LWRenderFuncs  -----------------  */

XCALL_(static LWError)ProxyPickInit (LWInstance inst, int i)
{
	XCALL_INIT;
	return (NULL);
}

XCALL_(static LWError)ProxyPickNewTime (LWInstance inst, LWFrame f, LWTime t)
{
	ProxyPickData	*dat = (ProxyPickData *)inst;
	XCALL_INIT;
	return NULL;
}

XCALL_(static void)ProxyPickCleanup (LWInstance inst)
{
	XCALL_INIT;
	return;
}

/* ----------------- Plug-in Methods: LWItemFuncs  -----------------  */

XCALL_(static const LWItemID *)ProxyPickUseItems (LWInstance inst)
{
	ProxyPickData *dat = (ProxyPickData *)inst;
	static LWItemID items[] = {NULL, NULL, NULL};
	XCALL_INIT;
	items[0] = dat->target;
	items[1] = dat->proxy;
	return items;
}

XCALL_(static void)ProxyPickChangeID (LWInstance inst, const LWItemID *items)
{
	ProxyPickData *dat = (ProxyPickData *)inst;
	int		 i=0;
	XCALL_INIT;
	if(dat->target || dat->proxy)
		while(items[i])
		{
			if(items[i]==dat->target)
				dat->target = items[i+1];
			else if(items[i]==dat->proxy)
				dat->proxy = items[i+1]; 
			i += 2;
		}
	return;
}



/* ----------------- Plug-in Methods: LWCustomObjHandler  -----------------  */

XCALL_(static unsigned int)ProxyPickFlags (ProxyPickData *dat)
{
	XCALL_INIT;
	return 0;
}	


XCALL_(static double)ProxyPickEvent (ProxyPickData *dat,	const LWMasterAccess *mac)
{
	char			*c = (char*)mac->eventData, cmd[80];
	unsigned int	 id=0;
	XCALL_INIT;
	if(dat->target && dat->proxy && !(dat->flags&PRXF_PAUSE))
		switch(mac->eventCode)
		{
			case LWEVNT_NOTHING:
				break;
			case LWEVNT_SELECT:
				sprintf(cmd,"SelectItem %x", dat->target);
				break;
			case LWEVNT_COMMAND:
				*cmd = 0;
				if(!strncmp(c,"SelectItem",10))
				{
					sscanf(c,"SelectItem %x", &id);
					if((id==(unsigned int)dat->proxy))
						sprintf(cmd,"SelectItem %x", dat->target);
				}
				else if(!strncmp(c,"AddToSelection",14))
				{
					sscanf(c,"AddToSelection %x", &id);
					if((id==(unsigned int)dat->proxy))
					{
						sprintf(cmd,"RemoveFromSelection %x", dat->proxy);
						mac->evaluate(mac->data, cmd);
						if(GlobalItemInfo->type(dat->proxy)==GlobalItemInfo->type(dat->target))
							sprintf(cmd,"AddToSelection %x", dat->target);
						else
						{
							LWItemType type = GlobalItemInfo->type(dat->target);
							switch(type)
							{
								case LWI_OBJECT	:
									mac->evaluate(mac->data, "EditObjects");
									break;
								case LWI_LIGHT	:
									mac->evaluate(mac->data, "EditLights");
									break;
								case LWI_CAMERA	:
									mac->evaluate(mac->data, "EditCameras");
									break;
								case LWI_BONE	:
									mac->evaluate(mac->data, "EditBones");
									break;
							}
							sprintf(cmd,"AddToSelection %x", dat->target);
						}
					}
				}
				else 
					break;
				if(*cmd)
					mac->evaluate(mac->data, cmd);

				break;
			default:
				break;
		}
	return (0.0);
}

/* -----------------                 -----------------  */


XCALL_(int) ProxyPick (
	long			 	 version,
	GlobalFunc			*global,
	LWMasterHandler	*local,
	void				*serverData)
{
	XCALL_INIT;
	if (version != LWMASTER_VERSION)
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
	GlobalLWUI = (*global) (LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT);
	if(!GlobalLWUI)
	{
		(*Gmessage->error)("Can't get global",LWINTERFACEINFO_GLOBAL);
		return AFUNC_BADGLOBAL;
	}
	GlobalGlobal = global;

	local->inst->create			= ProxyPickCreate;
	local->inst->destroy		= ProxyPickDestroy;
	local->inst->load			= ProxyPickLoad;
	local->inst->save			= ProxyPickSave;
	local->inst->copy			= ProxyPickCopy;
	local->inst->descln			= ProxyPickDescribe;

	local->item->useItems		= ProxyPickUseItems;
	local->item->changeID		= ProxyPickChangeID;

	local->type					= LWMAST_SCENE;
	local->event 				= ProxyPickEvent;
	local->flags 				= ProxyPickFlags;
	return (AFUNC_OK);
}

/* -----------------  User Interface  ----------------- */
int popCnt_Item( LWItemInfo *ItemInfo )
{
	LWItemID id=LWITEM_NULL, bon;
	int n=1;

	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		n++;
		for(bon = ItemInfo->first(LWI_BONE, id); bon!= LWITEM_NULL; bon = ItemInfo->next(bon))
			n++;
	}
	for(id = ItemInfo->first(LWI_LIGHT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
		n++;

	for(id = ItemInfo->first(LWI_CAMERA, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
		n++;

	return n;
}

const char *popName_Item( LWItemInfo *ItemInfo, int idx )
{
	int n=0;
	LWItemID id, bon;
	static char	buf[256];
	if(idx==0)
		return "(none)";
	idx--;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return ItemInfo->name(id);
		n++;
		for(bon = ItemInfo->first(LWI_BONE, id); bon!= LWITEM_NULL; bon = ItemInfo->next(bon))
		{
			if(n==idx)
			{
				strncpy(buf,ItemInfo->name(id),255);
				strncat(buf,"-->",255);
				strncat(buf,ItemInfo->name(bon),255);
				return buf;
			}
			n++;
		}
	}
	for(id = ItemInfo->first(LWI_LIGHT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return ItemInfo->name(id);
		n++;
	}
	for(id = ItemInfo->first(LWI_CAMERA, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return ItemInfo->name(id);
		n++;
	}
	return "";
}

int popItemIdx( LWItemInfo *ItemInfo, LWItemID item )
{
	int n=1;
	LWItemID id, bon;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(id==item)
			return n;
		n++;
		for(bon = ItemInfo->first(LWI_BONE, id); bon!= LWITEM_NULL; bon = ItemInfo->next(bon))
		{
			if(bon==item)
				return n;
			n++;
		}
	}
	for(id = ItemInfo->first(LWI_LIGHT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(id==item)
			return n;
		n++;
	}
	for(id = ItemInfo->first(LWI_CAMERA, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(id==item)
			return n;
		n++;
	}
   return 0;
}

LWItemID popIdxItem( LWItemInfo *ItemInfo, int idx )
{
	int n=0;
	LWItemID id, bon;
	if(idx==0)
		return LWITEM_NULL;
	idx--;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return id;
		n++;
		for(bon = ItemInfo->first(LWI_BONE, id); bon!= LWITEM_NULL; bon = ItemInfo->next(bon))
		{
			if(n==idx)
				return bon;
			n++;
		}
	}
	for(id = ItemInfo->first(LWI_LIGHT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return id;
		n++;
	}
	for(id = ItemInfo->first(LWI_CAMERA, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return id;
		n++;
	}
	return LWITEM_NULL;
}

int popCnt_Object( LWItemInfo *ItemInfo )
{
	int n=1;
	LWItemID id;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
		n++;
	return n;
}


const char *popName_Object( LWItemInfo *ItemInfo, int idx )
{
	int n=0;
	LWItemID id;
	if(idx==0)
		return "(none)";
	idx--;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return ItemInfo->name(id);
		n++;
	}
	return "";
}

int popObjectIdx( LWItemInfo *ItemInfo, LWItemID item )
{
	int n=1;
	LWItemID id;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(id==item)
			return n;
		n++;
	}
   return 0;
}

LWItemID popIdxObject( LWItemInfo *ItemInfo, int idx )
{
	int n=0;
	LWItemID id;
	if(idx==0)
		return LWITEM_NULL;
	idx--;
	for(id = ItemInfo->first(LWI_OBJECT, LWITEM_NULL); id!= LWITEM_NULL; id = ItemInfo->next(id) )
	{
		if(n==idx)
			return id;
		n++;
	}
	return LWITEM_NULL;
}


enum  {	CH_PROX = 0x8601, CH_TARG, CH_LABL, CH_COLR, CH_MARK, CH_GRP1, CH_GRP2 };
#define			STR_Proxy_TEXT		"Proxy Object"
#define			STR_Target_TEXT		"Target Item"
#define			STR_Label_TEXT		"Apply Label"
#define			STR__TEXT		""

static LWXPanelControl ProxyPick_ctrl_list[] = {
	{ CH_PROX,			STR_Proxy_TEXT,				"iPopChoice" },
	{ CH_TARG,			STR_Target_TEXT,			"iPopChoice" },
	{ CH_LABL,			STR_Label_TEXT,				"vButton" },
	{0}
};
static LWXPanelDataDesc ProxyPick_data_descrip[] = {
	{ CH_PROX,			STR_Proxy_TEXT,				"integer" },
	{ CH_TARG,			STR_Target_TEXT,			"integer" },
	{ CH_LABL,			STR_Label_TEXT,				"integer" },
	{0},
};


static void *ProxyPickData_get ( void *myinst, unsigned long vid ) 
{
  ProxyPickData *dat = (ProxyPickData*)myinst;
  void *result = NULL;
  static int val = 0;
  static double rgba[4];

  if ( dat ) 
	  switch ( vid ) {
		case CH_PROX:
			val = popObjectIdx(GlobalItemInfo, dat->proxy);
			result = &val;
			break;
		case CH_TARG:
			val = popItemIdx(GlobalItemInfo, dat->target);
			result = &val;
			break;
	} 
  return result;
}

static int ProxyPickData_set ( void *myinst, unsigned long vid, void *value ) 
{
	ProxyPickData *dat = (ProxyPickData*)myinst;
	double		rgba[4]={0.0};
	int			rc=0;
	LWItemID	id;
	if ( dat ) 
		switch ( vid ) {
			case CH_PROX:
				rc = *((int*)value);
				id = popIdxObject(GlobalItemInfo, rc);
				if(dat->target)	
					if(dat->proxy != id)
						dat->oldProxy = dat->proxy;
				dat->proxy = id;
				rc = 1; 
				break;
			case CH_TARG:
				rc = *((int*)value);
				dat->oldTarget = dat->target;
				dat->target = popIdxItem(GlobalItemInfo, rc);
				rc = 1; 
				break;
		} 
	return rc;
}

void ApplyCustObj( LWXPanelID panel, int ctrl_id )
{
	ProxyPickData		*dat;
	if(dat = (*GlobalXPanFun->getData)(panel, ctrl_id))
	{
		char		cmd[40]="";
		LWItemID	pro=LWITEM_NULL,sel=LWITEM_NULL,*sid = (LWItemID*)GlobalLWUI->selItems;
		if(sid)
			sel = *sid; // just reselect 1st selected... oh well

		if( dat->proxy!=dat->oldProxy )
			pro = dat->oldProxy ? dat->oldProxy:dat->proxy;
		else
			pro = dat->proxy;

		if(pro && (dat->target!=dat->oldTarget || dat->proxy!=dat->oldProxy) )
		{
			dat->flags |= PRXF_PAUSE; // next select will trigger event otherwise.. Doooh!
			if(dat->flags&PRXF_LABELED)
			{
				killItemTag(GlobalItemInfo, dat->proxy, "LABEL");
				killItemTag(GlobalItemInfo, dat->proxy, "LINK");
				sprintf(cmd,"SelectItem %x", pro);
				LW_Execute(cmd);
				LW_Execute("RemoveServer CustomObjHandler 0");
				dat->oldProxy = LWITEM_NULL;
			}

			if(dat->target && dat->proxy)	
			{
				setItemTag(GlobalItemInfo, dat->proxy, "LABEL",(char*)GlobalItemInfo->name(dat->target));
				sprintf(cmd,"%x", dat->target);
				setItemTag(GlobalItemInfo, dat->proxy, "LINK",cmd);
				sprintf(cmd,"SelectItem %x", dat->proxy);
				LW_Execute(cmd);
				LW_Execute("ApplyServer CustomObjHandler LW_ItemShape");
				dat->flags |= PRXF_LABELED;
				dat->oldProxy = LWITEM_NULL;
			}
			if(sel)
			{
				sprintf(cmd,"SelectItem %x", sel);
				LW_Execute(cmd);
			}
			LW_Execute("Refresh");
			dat->flags &= ~PRXF_PAUSE; 
		}
	}
}

static int   levEnable[] = {0,1,0,1,0};		
static LWXPanelID ProxyPickXPanel(GlobalFunc *global, ProxyPickData *dat)
{
	LWXPanelID     panID = NULL;
	static LWXPanelHint hint[] = {
		XpLABEL(0,"ProxyPicker Object"),
		XpPOPFUNCS(CH_PROX,popCnt_Object, popName_Object),
		XpPOPFUNCS(CH_TARG,popCnt_Item, popName_Item),
		XpBUTNOTIFY(CH_LABL, ApplyCustObj),
		XpGROUP_(CH_GRP1),
			XpH(CH_PROX),
			XpH(CH_TARG),
			XpEND,
		XpGROUP_(CH_GRP2),
			XpH(CH_LABL),
			XpEND,
		XpEND
	};

	GlobalXPanFun = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if ( GlobalXPanFun ) 
	{
		panID = (*GlobalXPanFun->create)( LWXP_VIEW, ProxyPick_ctrl_list );
		if(panID) 
		{
			(*GlobalXPanFun->hint) ( panID, 0, hint );
			(*GlobalXPanFun->describe)( panID, ProxyPick_data_descrip, ProxyPickData_get, ProxyPickData_set );
			(*GlobalXPanFun->viewInst)( panID, dat );
			(*GlobalXPanFun->setData)(panID, CH_PROX, GlobalItemInfo);
			(*GlobalXPanFun->setData)(panID, CH_TARG, GlobalItemInfo);
			(*GlobalXPanFun->setData)(panID, CH_LABL, dat);
			(*GlobalXPanFun->setData)(panID, 0, dat);
	    }
    }
	return panID;
}

XCALL_(int) ProxyPick_UI (
	long		 	version,
	GlobalFunc		*global,
	LWInterface		*UI,
	void			*serverData)
{
	XCALL_INIT;
	if (version != LWINTERFACE_VERSION)
		return (AFUNC_BADVERSION);
	UI->panel	= ProxyPickXPanel(global, UI->inst);
	UI->options	= NULL;
	UI->command	= NULL; 
	return AFUNC_OK;
}
