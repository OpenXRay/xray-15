/* vrml_pan.c - VRML97 Control Panels
 *		Arnie Cachelin		Copyright 2000 NewTek, Inc.
 *
 *
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
#include <lwxpanel.h>
 
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

extern LWCameraInfo			*camInfo;
extern GlobalFunc			*GlobalGlobal;
extern LWItemInfo			*itemInfo;
extern LWObjectInfo			*objInfo;
extern LWImageList			*imageList;
extern LWTextureFuncs		*texFuncs;
extern LWSurfaceFuncs		*surfFuncs;
extern LWLightInfo			*lightInfo;
extern LWSceneInfo			*sceneInfo;
extern LWChannelInfo		*chanInfo;
extern LWEnvelopeFuncs		*envInfo;

static LWXPanelFuncs	*GlobalLWXPF = NULL;
static	LWFileActivateFunc  *GlobalFileAct = NULL;

const char *navModeName(int flags);

enum  {	CH_FILE = 0x89F1, CH_OSEQ, CH_PROTO, CH_OBURL, CH_MBED, CH_OBPATH, CH_SENSOR, CH_RANGE, CH_HIGH, CH_ITEM, CH_AUTO,
		CH_LINK, CH_LSENSOR,CH_LRANGE,CH_LLEV, CH_LOWER, CH_AUDON, CH_AUDURL, CH_AUDVOL,CH_AUDLOOP,CH_NAV, CH_HEAD, CH_OVERW, CH_VIEW, CH_COMM,
		CH_AUTH, CH_OBJECT, CH_LIGHT, CH_CAMERA, CH_MORF, CH_MFR1, CH_MFR2, CH_MSTEP,CH_MLOOP, CH_TRIGR, CH_ENVIMG, CH_TXURN, CH_ENVURN,
		CH_VGR, CH_MRFGR, CH_ENVGR, CH_ITEMGR, CH_ANIMGR, CH_OBJGR, CH_LTRIGR, CH_LTGR, CH_OBGR,CH_AUDGR, CH_IGNORE, 
		CH_DIV1, CH_TAB};

static LWXPanelControl vrob_ctrl_list[] = {
	// Output Global parameters
	{ CH_FILE,			"Output .wrl",					    "sFileName" },
	{ CH_AUTH,			"Author",							"string" },
	// VRML saving details, not in scene tags 
	{ CH_PROTO,			"Use Prototypes",					"iBoolean" }, 
	{ CH_MBED,			"Embed Objects",					"iBoolean" },
	{ CH_OVERW,			"Overwrite Objects",				"iBoolean" },
	{ CH_LOWER,			"Lowercase Filenames",				"iBoolean" },
	{ CH_OBPATH,		"Local .wrl Path",					"sFileName" },
	{ CH_OBURL,			"VRML Object URL",					"string" },
	{ CH_TXURN,			"Texture URN",						"string" },
	// Object stuff
	{ CH_OBJECT,        "Object",							"iPopChoice" },
	{ CH_IGNORE,        "Ignore Object",					"iBoolean" },
	{ CH_SENSOR,        "Sensor Type",						"iPopChoice" },
	{ CH_RANGE,         "Range",							"distance3" },
	{ CH_LINK,          "Link URL",							"string" },
	{ CH_TRIGR,         "Alternate Trigger",				"iPopChoice" },
	{ CH_AUTO,          "AutoStart",						"iBoolean" },
	{ CH_AUDON,			"Attach Sound",						"iBoolean" },
	{ CH_AUDURL,		"URL",							    "string" },
	{ CH_AUDVOL,		"Volume",						    "percent" },
	{ CH_AUDLOOP,		"Loop Sound",					    "iBoolean" },
	{ CH_MORF,			"Record Morph",					    "iBoolean" },
	{ CH_MLOOP,			"Loop",							    "iBoolean" },
	{ CH_MFR1,			"First Frame",					    "integer" },
	{ CH_MFR2,			"Last Frame",					    "integer" },
	{ CH_MSTEP,			"Frame Step",					    "integer" },
// seq, seqloop, seqautostart
	{ CH_ITEM,			"Scene Item",						"iPopChoice" },
	// Light
//	{ CH_LIGHT,			"Light",							"iPopChoice" },
//	{ CH_LSENSOR,       "Sensor Type",						"iPopChoice" },
//	{ CH_LRANGE,        "Range",							"distance3" },
//	{ CH_LTRIGR,        "Alternate Trigger",				"iPopChoice" },
	// Environment/Camera
	{ CH_NAV,		    "Navigation Mode",					"iPopChoice" },
	{ CH_VIEW,		    "Standard Viewpoints",					"iBoolean" },
	{ CH_HEAD,		    "Headlight",						"iBoolean" },
	{ CH_LLEV,		    "Global Light Scale",				"percent" },
	{ CH_HIGH,		    "Avatar Size",						"distance" },
	{ CH_ENVIMG,	    "Environment Images",				"string" },
	{ CH_ENVURN,			"Image URN",						"string" },

	{0}
};

static LWXPanelDataDesc vrob_data_descrip[] = {
	{ CH_FILE,			"Output .wrl",					    "string" },
	{ CH_AUTH,			"Author",							"string" },

	{ CH_PROTO,			"Use Prototypes",					"integer" },
	{ CH_MBED,			"Embed Objects",					"integer" },
	{ CH_OVERW,			"Overwrite Objects",				"integer" },
	{ CH_LOWER,			"Lowercase Filenames",				"integer" },
	{ CH_OBPATH,		"Local .wrl Path",					"string" },
	{ CH_OBURL,			"VRML Object URL",					"string" },
	{ CH_TXURN,			"Texture URN",						"string" },

	{ CH_OBJECT,        "Object",							"integer" },
	{ CH_IGNORE,        "Ignore Object",					"integer" },
	{ CH_SENSOR,        "Sensor Type",						"integer" },
	{ CH_RANGE,         "Range",							"distance3" },
	{ CH_LINK,          "Link URL",							"string" },
	{ CH_TRIGR,         "Trigger",							"integer" },
	{ CH_AUTO,          "AutoStart",						"integer" },
	{ CH_AUDON,			"Attach Sound",						"integer" },
	{ CH_AUDURL,		"URL",							    "string" },
	{ CH_AUDVOL,		"Volume",						    "percent" },
	{ CH_AUDLOOP,		"Loop Sound",					    "integer" },
	{ CH_MORF,			"Record Morph",					    "integer" },
	{ CH_MFR1,			"First Frame",					    "integer" },
	{ CH_MLOOP,			"Loop",							    "integer" },
	{ CH_MFR2,			"Last Frame",					    "integer" },
	{ CH_MSTEP,			"Frame Step",					    "integer" },

	{ CH_ITEM,			"Scene Item",						"integer" },
//	{ CH_LIGHT,			"Light",							"integer" },
//	{ CH_LSENSOR,       "Sensor Type",						"integer" },
//	{ CH_LRANGE,        "Range",							"distance3" },
//	{ CH_LTRIGR,        "Trigger",							"integer" },

	{ CH_NAV,		    "Navigation Mode",					"integer" },
	{ CH_VIEW,		    "Standard Viewpoints",				"integer" },
	{ CH_HEAD,		    "Headlight",						"integer" },
	{ CH_LLEV,		    "Global Light Scale",				"percent" },
	{ CH_HIGH,		    "Avatar Size",						"distance" },
	{ CH_ENVIMG,	    "Environment Images",				"string" },
	{ CH_ENVURN,		"Image URN",						"string" },

	{0},
};

static char *sensorNames[] = {"Click","Proximity","Visibility","Touch",NULL};
static char *Navigations[] = {"None","Walk","Examine","Fly","Any",NULL};


typedef	struct {
		VRMLData	*vdat;
		char		*nbuf;
		int			siz, flags, audio, audloop, mStart,mEnd,mStep,mLoop;
		LWItemID	id, oid, lid, trig, ltrig, cam;
		LWDVector	range, lrange;
		double		volume;
		char		sound[256], envpic[256];
} VRML_ObjectData;



static int objectCount(void *dat)
{
	LWItemID	id;
	int			n = 1; // none also
	for(id = itemInfo->first(LWI_OBJECT, LWITEM_NULL); id!=LWITEM_NULL;id = itemInfo->next(id) )
		n++;
	return n;
}

static LWItemID objectId(void *dat, int n)
{
	LWItemID	id=LWITEM_NULL;
	int			t = 1; // none also
	if(n)
		for(id = itemInfo->first(LWI_OBJECT, LWITEM_NULL); id!=LWITEM_NULL && t<n; id = itemInfo->next(id) )
			t++;
	return id;
}

static int objectIndex(void *dat, LWItemID obid)
{
	int			n=1; // none also
	LWItemID	id=LWITEM_NULL;
	if(obid)
		for(id = itemInfo->first(LWI_OBJECT, LWITEM_NULL); id!=LWITEM_NULL && obid!=id; id = itemInfo->next(id) )
			n++;
	return (id==LWITEM_NULL) ? 0:n;
}


static const char *objectName(void *dat, int n)
{
	LWItemID	id;
	id = objectId(dat,n);
	return (id==LWITEM_NULL) ? "(none)":itemInfo->name(id);
}


static int itemCount(int *typ)
{
	LWItemID	id;
	int			t,n = 1; // none also
	t = *typ;
	if(!t) 
	{
		t = LWI_OBJECT;
		for(id = itemInfo->first(t, LWITEM_NULL); id!=LWITEM_NULL;id = itemInfo->next(id) )
			n++;
		t = LWI_LIGHT;
	}
	for(id = itemInfo->first(t, LWITEM_NULL); id!=LWITEM_NULL;id = itemInfo->next(id) )
		n++;
	return n;
}

static LWItemID itemId(int *typ, int n)
{
	LWItemID	id=LWITEM_NULL;
	int			ty,t = 1; // none also
	ty = *typ;
	if(!ty) 
	{
		ty = LWI_OBJECT;
		if(n)
			for(id = itemInfo->first(ty, LWITEM_NULL); id!=LWITEM_NULL && t<n; id = itemInfo->next(id) )
				t++;
		if(id!=LWITEM_NULL && t==n)
			return id;
		ty = LWI_LIGHT;
	}
	if(n)
		for(id = itemInfo->first(ty, LWITEM_NULL); id!=LWITEM_NULL && t<n; id = itemInfo->next(id) )
			t++;
	return id;
}

static int itemIndex(int *typ, LWItemID obid)
{
	int			t,n=1; // none also
	LWItemID	id=LWITEM_NULL;
	t = *typ;
	if(!t) 
	{
		t = LWI_OBJECT;
		if(obid)
			for(id = itemInfo->first(t, LWITEM_NULL); id!=LWITEM_NULL && obid!=id; id = itemInfo->next(id) )
				n++;

		if(obid==id)
			return (id==LWITEM_NULL) ? 0:n;
		t = LWI_LIGHT;
	}
	if(obid)
		for(id = itemInfo->first(t, LWITEM_NULL); id!=LWITEM_NULL && obid!=id; id = itemInfo->next(id) )
			n++;
	return (id==LWITEM_NULL) ? 0:n;
}


static const char *itemName(int *typ, int n)
{
	LWItemID	id;
//	if(!*typ) 
//		*typ = LWI_OBJECT;
	id = itemId(typ,n);
	return (id==LWITEM_NULL) ? "(none)":itemInfo->name(id);
}

LWItemID	findAnyItem(char *name)
{
	int cnt, tp=0, i;
	cnt = itemCount(&tp);
	if(name && *name && cnt)
		for(i=0;i<cnt;i++)
			if(!strcmp(name,itemName(&tp,i)))
				return itemId(&tp,i);
	return LWITEM_NULL;
}
// obsolete... multi cameras make ambiguous worlds!!
// NOT
void VRML97_CameraTagsParse(VRML_ObjectData *odat,LWItemID cam)
{	 
	VRMLData *dat = odat->vdat;
	char com[TAG_SIZE];
	if(!cam)
		cam = (*itemInfo->first)(LWI_CAMERA, LWITEM_NULL);
	if(!cam)
		return;
	if(findItemTag(itemInfo,cam,VTAG_HEADLIGHT))
		dat->flags |= VRPREF_HEADLIGHT;
	else
		dat->flags &= ~VRPREF_HEADLIGHT;
	if(findItemTag(itemInfo,cam,VTAG_VIEWPOINTS))
		dat->flags |= VRPREF_VIEWS;
	else
		dat->flags &= ~VRPREF_VIEWS;
	dat->flags |= (VRPREF_ANY);
	getItemTag(itemInfo,cam,VTAG_ENVIMAGE"=", odat->envpic,sizeof(odat->envpic));
	if(getItemTag(itemInfo,cam,VTAG_AUTHOR"=", com,TAG_SIZE))
	{
		strncpy(dat->author, com, sizeof(dat->author));
	}
	if(getItemTag(itemInfo,cam,VTAG_AVATAR"=", com,TAG_SIZE))
	{
		float		fv1=0.0f, fv2=0.0f; 
		if(2==sscanf(com, "%f %f",&fv1,&fv2))
		{
			dat->avatarSize = fv1;
			dat->globalLightLev = fv2;
		}
	}
	if(getItemTag(itemInfo,cam,VTAG_NAVIGATE"=", com,TAG_SIZE))
		switch(com[0])
		{
			 case 'W': case 'w':
				dat->flags &= ~(VRPREF_ANY);
				dat->flags |= VRPREF_WALK;
				break;
			 case 'F': case 'f':
				dat->flags &= ~(VRPREF_ANY);
				dat->flags |= (VRPREF_FLY);
				break;
			 case 'E': case 'e':
				dat->flags &= ~(VRPREF_ANY);
				dat->flags |= (VRPREF_EXAMINE);
				break;
			 case 'N': case 'n':
				dat->flags &= ~(VRPREF_ANY);	 // none is not any
				break;
			 case 'A': case 'a':
			 default:
				dat->flags |= (VRPREF_ANY);
				break;
		}
}


static char	URL_buf[MAX_STR+1];	

static void *VRML_ObjectData_get ( void *myinst, unsigned long vid ) 
{
	VRML_ObjectData *dat = (VRML_ObjectData*)myinst;
	static double	t = 1.0, v[3] = {0.,0.,0.};
	static int		i=0, typ;
	void *result = NULL;

	if ( dat ) 
		switch ( vid ) {
			case CH_FILE:
				result = dat->nbuf;
				break;
			case CH_AUTH:
				result = dat->vdat->author;
				break;
			case CH_AUDON:
				dat->audio = findItemTag(itemInfo, dat->oid,VTAG_SOUND) ? 1:0;
				result = &dat->audio;
				break;
			case CH_AUDLOOP:
				if(dat->oid)
				{
					getItemTag(itemInfo, dat->oid,VTAG_SOUND"=",URL_buf,sizeof(URL_buf));
					sscanf(URL_buf,"%s %d %d",dat->sound,&i,&dat->audloop);
					dat->volume = ((double)i)/100.0;
				}
				result = &dat->audloop;
				break;
			case CH_AUDVOL:
				if(dat->oid)
				{
					getItemTag(itemInfo, dat->oid,VTAG_SOUND"=",URL_buf,sizeof(URL_buf));
					sscanf(URL_buf,"%s %d %d",dat->sound,&i,&dat->audloop);
					dat->volume = ((double)i)/100.0;
				}
				result = &dat->volume;
				break;
			case CH_AUDURL:
				if(dat->oid)
				{
					getItemTag(itemInfo, dat->oid,VTAG_SOUND"=",URL_buf,sizeof(URL_buf));
					sscanf(URL_buf,"%s %d %d",dat->sound,&i,&dat->audloop);
					dat->volume = ((double)i)/100.0;
				}
				result = dat->sound;
				break;
			case CH_OBJECT:
				i = objectIndex(dat,dat->oid);
				result = &i;
				break;
			case CH_TRIGR:
				if(getItemTag(itemInfo, dat->id, VTAG_TRIGGER"=", URL_buf,MAX_STR))
				{
					int cnt, tp=0;
					cnt = itemCount(&tp);
					for(i=0;i<cnt;i++)
						if(!strcmp(URL_buf,itemName(&tp,i)))
						{
							dat->trig = itemId(&tp,i);
							break;
						}
				}
				else
				{
					dat->trig = LWITEM_NULL;
					i = 0;
				}
				result = &i;
				break;
		/*	case CH_LTRIGR:
				i = objectIndex(dat,dat->ltrig);
				result = &i;
				break; */
			case CH_ITEM:
				typ = 0;
				i = itemIndex(&typ,dat->id);
				result = &i;
				break;
			case CH_LIGHT:
				typ = LWI_LIGHT;
				i = itemIndex(&typ,dat->lid);
				result = &i;
				break;
			case CH_MBED:
				i = dat->vdat->flags&VRPREF_INCLUDE_OBJS ? 1:0;
				result = &i;
				break;
			case CH_NAV:
				i = NAVMODE(dat->vdat->flags);
				if(i>4)
					i = 4;
				else if(i==4)
					i = 3;
				result = &i;
				break;
			case CH_HEAD:
				i = dat->vdat->flags&VRPREF_HEADLIGHT ? 1:0;
				result = &i;
				break;
			case CH_HIGH:
				result = &dat->vdat->avatarSize;
				break;
			case CH_LLEV:
				result = &dat->vdat->globalLightLev;
				break;
			case CH_LOWER:
				i = dat->vdat->flags&VRPREF_LOWERCASE ? 1:0;
				result = &i;
				break;
			case CH_OVERW:
				i = dat->vdat->flags&VRPREF_OVERWRITE ? 1:0;
				result = &i;
				break;
			case CH_VIEW:
				i = dat->vdat->flags&VRPREF_VIEWS ? 1:0;
				result = &i;
				break;
			case CH_COMM:
				i = dat->vdat->flags&VRPREF_COMMENTS ? 1:0;
				result = &i;
				break;
			case CH_ENVIMG:
				result = dat->envpic;
				break;
			case CH_OBPATH:
				result = dat->vdat->wrldir;
				break;
			case CH_PROTO:
				i = dat->vdat->flags&VRPREF_PROTO_OBJS ? 1:0;
				result = &i;
				break;
			case CH_OBURL:
				result = dat->vdat->wrlURL;
				break;
			case CH_TXURN:
				result = dat->vdat->texURN;
				break;
			case CH_AUTO:
				i = findItemTag(itemInfo, dat->id, VTAG_AUTOSTART);
				result = &i;
				break;
			case CH_SENSOR:
				if(findItemTag(itemInfo, dat->id, VTAG_TOUCH))
					i = 3;
				else if(getItemTag(itemInfo, dat->id, VTAG_VISIBILITY"=", URL_buf,MAX_STR))
				{
					i = 2;
					sscanf(URL_buf,"=%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				}
				else if(getItemTag(itemInfo, dat->id, VTAG_PROXIMITY"=", URL_buf,MAX_STR))
				{
				//	LWFVector vf;
				//	VCPY(vf,dat->range);
					i = 1;
					sscanf(URL_buf,"=%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				//	sscanf(URL_buf,"=%lf %lf %lf",&(vf[0]),&(vf[1]),&(vf[2]));
				}
				else 
					i = 0;
				result = &i;
				break;
		/*	case CH_LSENSOR:
				if(findItemTag(itemInfo, dat->lid, VTAG_TOUCH))
					i = 3;
				else if(getItemTag(itemInfo, dat->lid, VTAG_VISIBILITY"=", URL_buf,MAX_STR))
				{
					i = 2;
					sscanf(URL_buf,"%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				}
				else if(getItemTag(itemInfo, dat->lid, VTAG_PROXIMITY"=", URL_buf,MAX_STR))
				{
					i = 1;
					sscanf(URL_buf,"%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				}
				else 
					i = 0;
				result = &i;
				break; 
			case CH_LRANGE:
				result = dat->lrange;
				if(getItemTag(itemInfo, dat->lid, VTAG_VISIBILITY"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				else if(getItemTag(itemInfo, dat->lid, VTAG_PROXIMITY"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				break; */
			case CH_RANGE:
				result = dat->range;
				if(getItemTag(itemInfo, dat->id, VTAG_VISIBILITY"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				else if(getItemTag(itemInfo, dat->id, VTAG_PROXIMITY"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%lf %lf %lf",&(dat->range[0]),&(dat->range[1]),&(dat->range[2]));
				break;
			case CH_MORF:
				i = 0;
				if(dat->oid && getItemTag(itemInfo, dat->oid, VTAG_MORPH"=", URL_buf,MAX_STR))
				{
					sscanf(URL_buf,"%d %d %d %d", &dat->mStart, &dat->mEnd, &dat->mStep, &dat->mLoop);
					i = 1;
				}
				result = &i;
				break;
			case CH_MLOOP:
				if(dat->oid && getItemTag(itemInfo, dat->oid, VTAG_MORPH"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%d %d %d %d", &dat->mStart, &dat->mEnd, &dat->mStep, &dat->mLoop);
				result = &dat->mLoop;
				break;
			case CH_MFR1:
				if(dat->oid && getItemTag(itemInfo, dat->oid, VTAG_MORPH"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%d %d %d %d", &dat->mStart, &dat->mEnd, &dat->mStep, &dat->mLoop);
				result = &dat->mStart;
				break;
			case CH_MFR2:
				if(dat->oid && getItemTag(itemInfo, dat->oid, VTAG_MORPH"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%d %d %d %d", &dat->mStart, &dat->mEnd, &dat->mStep, &dat->mLoop);
				result = &dat->mEnd;
				break;
			case CH_MSTEP:
				if(dat->oid && getItemTag(itemInfo, dat->oid, VTAG_MORPH"=", URL_buf,MAX_STR))
					sscanf(URL_buf,"%d %d %d %d", &dat->mStart, &dat->mEnd, &dat->mStep, &dat->mLoop);
				result = &dat->mStep;
				break;
			case CH_IGNORE:
				i = (findItemTag(itemInfo, dat->oid, VTAG_IGNORE)) ? 1:0;
				result = &i;
				break;
			case CH_LINK:
				if(!getItemTag(itemInfo, dat->oid,VTAG_URL"=",URL_buf,MAX_STR))
					URL_buf[0] = 0;
				result = URL_buf;
				break;
			case CH_ENVURN:	
				if(!getItemTag(itemInfo, dat->cam,VTAG_URN"=",URL_buf,MAX_STR))
					URL_buf[0] = 0;
				result = URL_buf;
				break;
		} 
	return result;
}

static int VRML_ObjectData_set ( void *myinst, unsigned long vid, void *value ) 
{
VRML_ObjectData *dat = (VRML_ObjectData*)myinst;
	int			typ, rc=0;
	double		t = 1.0;
	char		buf[100]="";
	if ( dat ) 
		switch ( vid ) {
			case CH_IGNORE:
				rc = *((int*)value);
				if(rc)
					setItemTag(itemInfo, dat->oid,VTAG_IGNORE," ");
				else
					killItemTag(itemInfo, dat->oid,VTAG_IGNORE);
				rc = 1;
				break;
			case CH_FILE:
				strncpy(dat->nbuf,((char*)value),dat->siz);
				rc = 1;
				break;
			case CH_AUTH:
				strncpy(dat->vdat->author,((char*)value),sizeof(dat->vdat->author));
				if(dat->cam)
					if(*((char*)value))
						setItemTag(itemInfo, dat->cam,VTAG_AUTHOR,dat->vdat->author);
					else
						killItemTag(itemInfo, dat->cam,VTAG_AUTHOR);

				rc = 1;
				break;
			case CH_AUDON:
				dat->audio = *(int*)value;
				if(dat->oid)
					if(dat->audio)
					{
						sprintf(URL_buf,"%s %d %d",dat->sound,(int)(dat->volume*100),dat->audloop);
						setItemTag(itemInfo, dat->oid,VTAG_SOUND, URL_buf);
					}
					else
						killItemTag(itemInfo, dat->oid,VTAG_SOUND);
				rc = 1;
				break;
			case CH_AUDLOOP:
				dat->audloop = *(int*)value;
				sprintf(URL_buf,"%s %d %d",dat->sound,(int)(dat->volume*100),dat->audloop);
				if(dat->oid)
					setItemTag(itemInfo, dat->oid,VTAG_SOUND, URL_buf);
				rc = 1;
				break;
			case CH_AUDVOL:
				dat->volume = *(double*)value;
				sprintf(URL_buf,"%s %d %d",dat->sound,(int)(dat->volume*100),dat->audloop);
				if(dat->oid)
					setItemTag(itemInfo, dat->oid,VTAG_SOUND, URL_buf);
				rc = 1;
				break;
			case CH_AUDURL:
				if(dat->oid)
					if(*((char*)value))
					{
						strncpy(dat->sound,((char*)value),sizeof(dat->sound));
						sprintf(URL_buf,"%s %d %d",dat->sound,(int)(dat->volume*100),dat->audloop);
						setItemTag(itemInfo, dat->oid,VTAG_SOUND, URL_buf);
					} 
					else
					{
						killItemTag(itemInfo, dat->oid,VTAG_SOUND);
						dat->sound[0] = 0;
					} 				
				rc = 1;
				break;
			case CH_OBJECT:
				rc = *((int*)value);
				dat->oid = objectId(dat,rc);
				rc = 1;
				break;
			case CH_AUTO:
				rc = *((int*)value);
				if(rc)
					setItemTag(itemInfo, dat->id,VTAG_AUTOSTART,"");
				else
					killItemTag(itemInfo, dat->id,VTAG_AUTOSTART);
				rc = 1;
				break;
			case CH_TRIGR:
				rc = *((int*)value);
				dat->trig = objectId(dat,rc);
				if(dat->id)
					if(dat->trig)
					{
						char nam[300]="";
						buildItemVRCloneName(dat->trig, nam, 299);
						setItemTag(itemInfo, dat->id,VTAG_TRIGGER,nam);
					} 
					else
					{
						killItemTag(itemInfo, dat->id,VTAG_TRIGGER);
					} 				
				rc = 1;
				break;
			case CH_ITEM:
				rc = *((int*)value);
				typ = 0;
				dat->id = itemId(&typ,rc);
				rc = 1;
				break;
			case CH_MBED:
				rc = *((int*)value);
				if(rc)
					dat->vdat->flags |= VRPREF_INCLUDE_OBJS;
				else
					dat->vdat->flags &= ~VRPREF_INCLUDE_OBJS; 
				rc = 1;
				break;
			case CH_HEAD:
				rc = *((int*)value);
				if(rc)
				{
					dat->vdat->flags |= VRPREF_HEADLIGHT;
					if(dat->cam)
						setItemTag(itemInfo, dat->cam,VTAG_HEADLIGHT," ");
				}
				else
				{
					dat->vdat->flags &= ~VRPREF_HEADLIGHT; 
					if(dat->cam)
						killItemTag(itemInfo, dat->cam,VTAG_HEADLIGHT);
				}
				rc = 1;
				break;
			case CH_LOWER:
				rc = *((int*)value);
				if(rc)
					dat->vdat->flags |= VRPREF_LOWERCASE;
				else
					dat->vdat->flags &= ~VRPREF_LOWERCASE; 
				rc = 1;
				break;
			case CH_VIEW:
				rc = *((int*)value);
				if(rc)
				{
					dat->vdat->flags |= VRPREF_VIEWS;
					if(dat->cam)
						setItemTag(itemInfo, dat->cam,VTAG_VIEWPOINTS," ");
				}
				else
				{
					dat->vdat->flags &= ~VRPREF_VIEWS; 
					if(dat->cam)
						killItemTag(itemInfo, dat->cam,VTAG_VIEWPOINTS);
				}
				rc = 1;
				break;
			case CH_OVERW:
				rc = *((int*)value);
				if(rc)
					dat->vdat->flags |= VRPREF_OVERWRITE;
				else
					dat->vdat->flags &= ~VRPREF_OVERWRITE; 
				rc = 1;
				break;
			case CH_NAV:
				rc = *((int*)value);
				if(rc<=2)
					dat->vdat->flags = SET_NAVMODE(dat->vdat->flags,rc); // NONE, WALK, EXAMINE
				else if(rc==3)
					dat->vdat->flags = SET_NAVMODE(dat->vdat->flags,4); // FLY
				else if(rc==4)
					dat->vdat->flags = SET_NAVMODE(dat->vdat->flags,7); // ANY
				if(dat->cam)
					setItemTag(itemInfo, dat->cam,VTAG_NAVIGATE,Navigations[rc]);
				rc = 1;
				break;
			case CH_HIGH:
				dat->vdat->avatarSize = *(double*)value;
				sprintf(buf,"%.5f  %.4f",dat->vdat->avatarSize, dat->vdat->globalLightLev);
				if(dat->cam)
					setItemTag(itemInfo, dat->cam,VTAG_AVATAR,buf);
				rc = 1;
				break;
			case CH_LLEV:
				dat->vdat->globalLightLev = *(double*)value;
				sprintf(buf,"%.5f  %.4f",dat->vdat->avatarSize, dat->vdat->globalLightLev);
				if(dat->cam)
					setItemTag(itemInfo, dat->cam,VTAG_AVATAR,buf);
				rc = 1;
				break;
			case CH_COMM:
				rc = *((int*)value);
				if(rc)
					dat->vdat->flags |= VRPREF_COMMENTS;
				else
					dat->vdat->flags &= ~VRPREF_COMMENTS; 
				rc = 1;
				break;
			case CH_PROTO:
				rc = *((int*)value);
				if(rc)
					dat->vdat->flags |= VRPREF_PROTO_OBJS;
				else
					dat->vdat->flags &= ~VRPREF_PROTO_OBJS; 
				rc = 1;
				break;
			case CH_ENVIMG:
				strncpy(dat->envpic,((char*)value),sizeof(dat->envpic));
				if(dat->envpic[0])
				{
					if(dat->cam)
						setItemTag(itemInfo, dat->cam,VTAG_ENVIMAGE,dat->envpic);
				}
				else
					if(dat->cam)
						killItemTag(itemInfo, dat->cam,VTAG_ENVIMAGE);
				rc = 1;
				break;
			case CH_ENVURN:
				if(dat->cam)
					if(*((char*)value))
						setItemTag(itemInfo, dat->cam,VTAG_URN,((char*)value));
					else
						killItemTag(itemInfo, dat->cam,VTAG_URN);
				rc = 1;
				break;
			case CH_TXURN:
				strncpy(dat->vdat->texURN,((char*)value),sizeof(dat->vdat->texURN));
				rc = 1;
				break;
			case CH_OBPATH:
				strncpy(dat->vdat->wrldir,((char*)value),sizeof(dat->vdat->wrldir));
				rc = strlen(dat->vdat->wrldir);
				if(rc && dat->vdat->wrldir[rc-1]!=FILE_MARK)
				{
					dat->vdat->wrldir[rc]=FILE_MARK;
					dat->vdat->wrldir[rc+1]=0;
				}
				rc = 1;
				break;
			case CH_OBURL:
				strncpy(dat->vdat->wrlURL,((char*)value),sizeof(dat->vdat->wrlURL));
				rc = strlen(dat->vdat->wrlURL);
				if(rc && dat->vdat->wrlURL[rc-1]!=FILE_MARK)
				{
					dat->vdat->wrlURL[rc]=FILE_MARK;
					dat->vdat->wrlURL[rc+1]=0;
				}
			/*	if(*dat->vdat->wrlURL)
				{
					setItemTag(itemInfo, dat->id,VTAG_URL,dat->vdat->wrlURL);
				} 
				else
				{
					killItemTag(itemInfo, dat->id,VTAG_URL);
				} 		*/		
				rc = 1;
				break;
			case CH_SENSOR:
				rc = *((int*)value);
				if(dat->id)
					switch(rc)
					{		 
						case 1:
							sprintf(URL_buf,"%.4f %.4f %.4f",dat->range[0], dat->range[1], dat->range[2]);
							killItemTag(itemInfo, dat->id,VTAG_VISIBILITY);
							killItemTag(itemInfo, dat->id,VTAG_TOUCH);
							setItemTag(itemInfo, dat->id,VTAG_PROXIMITY,URL_buf);
							break;
						case 2:
							sprintf(URL_buf,"%.4f %.4f %.4f",dat->range[0], dat->range[1], dat->range[2]);
							killItemTag(itemInfo, dat->id,VTAG_TOUCH);
							killItemTag(itemInfo, dat->id,VTAG_PROXIMITY);
							setItemTag(itemInfo, dat->id,VTAG_VISIBILITY,URL_buf);
							break;
						case 3:
							killItemTag(itemInfo, dat->id,VTAG_VISIBILITY);
							killItemTag(itemInfo, dat->id,VTAG_PROXIMITY);
							URL_buf[0] = 0;
							setItemTag(itemInfo, dat->id,VTAG_TOUCH,URL_buf);
							break;
						case 0:
						default:
							killItemTag(itemInfo, dat->id,VTAG_VISIBILITY);
							killItemTag(itemInfo, dat->id,VTAG_PROXIMITY);
							killItemTag(itemInfo, dat->id,VTAG_TOUCH);
					} 				
				rc = 1;
				break;
			case CH_RANGE:
				VCPY(dat->range, ((double*)value) );
				if(findItemTag(itemInfo, dat->id, VTAG_VISIBILITY))
				{
					sprintf(URL_buf,"%lf %lf %lf",(dat->range[0]),(dat->range[1]),(dat->range[2]));
					setItemTag(itemInfo, dat->id,VTAG_VISIBILITY,URL_buf);
				}
				else if(findItemTag(itemInfo, dat->id, VTAG_PROXIMITY))
				{
					sprintf(URL_buf,"%lf %lf %lf",(dat->range[0]),(dat->range[1]),(dat->range[2]));
					setItemTag(itemInfo, dat->id,VTAG_PROXIMITY,URL_buf);
				}
				
				rc = 1;
				break;
		/*	case CH_LTRIGR:
				rc = *((int*)value);
				dat->ltrig = objectId(dat,rc);
				if(dat->lid)
					if(dat->ltrig)
					{
						char nam[300]="";
						buildItemVRCloneName(dat->ltrig, nam, 299);
						setItemTag(itemInfo, dat->lid,VTAG_TRIGGER,nam);
					} 
					else
					{
						killItemTag(itemInfo, dat->lid,VTAG_TRIGGER);
					} 				
				rc = 1;
				break;
			case CH_LIGHT:
				rc = *((int*)value);
				typ = LWI_LIGHT;
				dat->lid = itemId(&typ,rc);
				rc = 1;
				break;
			case CH_LSENSOR:
				rc = *((int*)value);
				if(dat->lid)
					switch(rc)
					{		 
						case 1:
							sprintf(URL_buf,"%.4f %.4f %.4f",dat->range[0], dat->range[1], dat->range[2]);
							killItemTag(itemInfo, dat->lid,VTAG_VISIBILITY);
							killItemTag(itemInfo, dat->lid,VTAG_TOUCH);
							setItemTag(itemInfo, dat->lid,VTAG_PROXIMITY,URL_buf);
							break;
						case 2:
							sprintf(URL_buf,"%.4f %.4f %.4f",dat->range[0], dat->range[1], dat->range[2]);
							killItemTag(itemInfo, dat->lid,VTAG_TOUCH);
							killItemTag(itemInfo, dat->lid,VTAG_PROXIMITY);
							setItemTag(itemInfo, dat->lid,VTAG_VISIBILITY,URL_buf);
							break;
						case 3:
							killItemTag(itemInfo, dat->lid,VTAG_VISIBILITY);
							killItemTag(itemInfo, dat->lid,VTAG_PROXIMITY);
							URL_buf[0] = 0;
							setItemTag(itemInfo, dat->lid,VTAG_TOUCH,URL_buf);
							break;
						case 0:
						default:
							killItemTag(itemInfo, dat->lid,VTAG_VISIBILITY);
							killItemTag(itemInfo, dat->lid,VTAG_PROXIMITY);
							killItemTag(itemInfo, dat->lid,VTAG_TOUCH);
					} 				
				rc = 1;
				break;
			case CH_LRANGE:
				VCPY(dat->lrange, ((double*)value) );
				if(findItemTag(itemInfo, dat->lid, VTAG_VISIBILITY))
				{
					sprintf(URL_buf,"%lf %lf %lf",(dat->lrange[0]),(dat->lrange[1]),(dat->lrange[2]));
					setItemTag(itemInfo, dat->lid,VTAG_VISIBILITY,URL_buf);
				}
				else if(findItemTag(itemInfo, dat->lid, VTAG_PROXIMITY))
				{
					sprintf(URL_buf,"%lf %lf %lf",(dat->lrange[0]),(dat->lrange[1]),(dat->lrange[2]));
					setItemTag(itemInfo, dat->lid,VTAG_PROXIMITY,URL_buf);
				}
				rc = 1;
				break; */
			case CH_MORF:
				if(dat->oid)
					if(*(int*)value)
					{
						sprintf(URL_buf,"%d %d %d %d",dat->mStart, dat->mEnd, dat->mStep, dat->mLoop);
						setItemTag(itemInfo, dat->oid,VTAG_MORPH,URL_buf);
					}
					else 
						killItemTag(itemInfo, dat->oid,VTAG_MORPH);				
				rc = 1;
				break;
			case CH_MFR1:
				dat->mStart = *(int*)value;
				sprintf(URL_buf,"%d %d %d %d",dat->mStart, dat->mEnd, dat->mStep, dat->mLoop);
				if(dat->oid)
					setItemTag(itemInfo, dat->oid,VTAG_MORPH,URL_buf);
				rc = 1;
				break;
			case CH_MFR2:
				dat->mEnd = *(int*)value;
				sprintf(URL_buf,"%d %d %d %d",dat->mStart, dat->mEnd, dat->mStep, dat->mLoop);
				if(dat->oid)
					setItemTag(itemInfo, dat->oid,VTAG_MORPH,URL_buf);
				rc = 1;
				break;
			case CH_MSTEP:
				dat->mStep = *(int*)value;
				sprintf(URL_buf,"%d %d %d %d",dat->mStart, dat->mEnd, dat->mStep, dat->mLoop);
				if(dat->oid)
					setItemTag(itemInfo, dat->oid,VTAG_MORPH,URL_buf);
				rc = 1;
				break;
			case CH_MLOOP:
				dat->mLoop = *(int*)value ? 1:0;
				sprintf(URL_buf,"%d %d %d %d",dat->mStart, dat->mEnd, dat->mStep, dat->mLoop);
				if(dat->oid)
					setItemTag(itemInfo, dat->oid,VTAG_MORPH,URL_buf);
				rc = 1;
				break;
			case CH_LINK:
				if(dat->oid)
					if(*((char*)value))
					{
						setItemTag(itemInfo, dat->oid,VTAG_URL,((char*)value));
					} 
					else
					{
						killItemTag(itemInfo, dat->oid,VTAG_URL);
					} 				
				rc = 1;
				break;
			} 
	return rc;
}


void GetCodec ( LWXPanelID panel, int ctrl_id )
{
	VRML_ObjectData		*dat;
	if(dat = (*GlobalLWXPF->getData)(panel, 0))
	{
;
	}
}

static void VRML_UI_notify (LWXPanelID panID, unsigned long cid, unsigned long vid, int event_code)
{
	void *val = NULL;
	void *dat;
	if(event_code==LWXPEVENT_VALUE)
		if(panID) 
			if( GlobalLWXPF )
				if( dat = (*GlobalLWXPF->getData)(panID, 0) )
				{
				}
	return;
}

static int	itype, type,rangeEnableMap[] = {0,1,1,0}, invertMap[] = {1,0};

static LWXPanelHint HUGEhint[] = {
	XpLABEL(0,"Export ISO-VRML97"),
//	XpBUTNOTIFY     ( CH_IGNORE,     GetCodec ),
	XpGROUP_ (CH_VGR),
			XpH ( CH_FILE ),
			XpH ( CH_AUTH ),
			XpEND,

	XpGROUP_ (CH_DIV1),
			XpH ( CH_OBPATH ),
			XpH ( CH_OBURL ),
			XpH ( CH_TXURN ),
			XpEND,

	XpGROUP_ (CH_ANIMGR),
			XpH ( CH_PROTO ),
			XpH ( CH_LOWER ),
			XpH ( CH_MBED ),
			XpH ( CH_OVERW ),
			XpH ( CH_DIV1 ),
			XpEND,

	XpGROUP_ (CH_AUDGR),
			XpH ( CH_AUDON ),
			XpH ( CH_AUDURL ),
			XpH ( CH_AUDLOOP ),
			XpH ( CH_AUDVOL ),
			XpEND,

	XpENABLEMSG_ (CH_AUDON, "Sound is currently disabled"),
			XpH ( CH_AUDURL ),
			XpH ( CH_AUDVOL ),
			XpH ( CH_AUDLOOP ),
			XpEND,

	XpGROUP_ (CH_MRFGR),
			XpH ( CH_MORF ),
			XpH ( CH_MFR1 ),
			XpH ( CH_MLOOP ),
			XpH ( CH_MFR2 ),
			XpH ( CH_AUTO ),
			XpH ( CH_MSTEP ),
			XpEND,

/*	XpLINK_ (CH_MORF),
			XpH ( CH_MFR1 ),
			XpH ( CH_MLOOP ),
			XpH ( CH_MFR2 ),
			XpH ( CH_MSTEP ),
			XpEND, */

	XpENABLE_ (CH_MORF),
			XpH ( CH_MFR1 ),
			XpH ( CH_MLOOP ),
			XpH ( CH_MFR2 ),
			XpH ( CH_MSTEP ),
			XpH ( CH_AUTO ),
			XpEND,

	XpGROUP_ (CH_ITEMGR),
			XpH ( CH_ITEM ),
			XpH ( CH_SENSOR ),
			XpH ( CH_RANGE ),
	//		XpH ( CH_AUTO ),
			XpH ( CH_TRIGR ),
			XpEND,
	XpLINK_ (CH_ITEM),
			XpH ( CH_SENSOR ),
			XpH ( CH_RANGE ),
		//	XpH ( CH_AUTO ),
			XpH ( CH_TRIGR ),
			XpEND,

	XpGROUP_ (CH_OBJGR),
			XpH ( CH_OBJECT ),
			XpH ( CH_IGNORE ),
			XpH ( CH_LINK ),
		//	XpH ( CH_SENSOR ),
		//	XpH ( CH_RANGE ),
		//	XpH ( CH_TRIGR ),
			XpH ( CH_AUDGR ),
			XpH ( CH_MRFGR ),
			XpEND,

	XpLINK_ (CH_OBJECT),
			XpH ( CH_IGNORE ),
			XpH ( CH_LINK ),
			//XpH ( CH_SENSOR ),
			//XpH ( CH_RANGE ),
			//XpH ( CH_TRIGR ),
			XpH ( CH_AUDON ),
			XpH ( CH_AUDURL ),
			XpH ( CH_AUDVOL ),
			XpH ( CH_AUDLOOP ),
			XpH ( CH_MORF ),
	//		XpH ( CH_MRFGR ),
			XpH ( CH_MFR1 ),
			XpH ( CH_MLOOP ),
			XpH ( CH_MFR2 ),
			XpH ( CH_MSTEP ),
			XpH ( CH_AUTO ),
			XpEND,

/*	XpGROUP_ (CH_LTGR),
			XpH ( CH_LIGHT ),
			XpH ( CH_LSENSOR ),
			XpH ( CH_LRANGE ),
			XpH ( CH_LTRIGR ),
			XpEND,

	XpLINK_ (CH_LIGHT),
			XpH ( CH_LSENSOR ),
			XpH ( CH_LRANGE ),
			XpH ( CH_LTRIGR ),
			XpEND,
*/
	XpGROUP_ (CH_ENVGR),
			XpH ( CH_NAV ),
			XpH ( CH_HIGH ),
			XpH ( CH_HEAD ),
			XpH ( CH_VIEW ),
			XpH ( CH_LLEV ),
			XpH ( CH_ENVIMG ),
			XpH ( CH_ENVURN ),
			XpEND,

	XpPOPFUNCS(CH_OBJECT,objectCount, objectName),
	XpPOPFUNCS(CH_LIGHT,itemCount, itemName),
	XpPOPFUNCS(CH_ITEM,itemCount, itemName),
	XpPOPFUNCS(CH_TRIGR,objectCount, objectName),
	XpPOPFUNCS(CH_LTRIGR,objectCount, objectName),

	XpXREQCFG(CH_FILE,LWXPREQ_SAVE,"Export ISO-VRML97","*.wrl"),
	XpXREQCFG(CH_OBPATH,LWXPREQ_DIR,"Local VRML Object Path","*.wrl"),

	XpENABLEMSG_MAP_(CH_SENSOR, rangeEnableMap, "Range is not available with this Sensor type"),
			XpH ( CH_RANGE ),
			XpEND,
	XpSTRLIST(CH_SENSOR,sensorNames),

	XpENABLE_ (CH_ITEM),
			XpH ( CH_SENSOR ),
			XpH ( CH_RANGE ),
	//		XpH ( CH_AUTO ),
			XpH ( CH_TRIGR ),
			XpEND,
	XpENABLE_MAP_ (CH_AUTO, invertMap),
			XpH ( CH_TRIGR ),
			XpEND,

	XpENABLEMSG_(CH_OBJECT, "No Object Selected"),
			XpH ( CH_IGNORE ),
			XpH ( CH_LINK ),
		//	XpH ( CH_SENSOR ),
		//	XpH ( CH_TRIGR ),
		//	XpH ( CH_RANGE ),
			XpH ( CH_AUDGR ),
			XpH ( CH_MRFGR ),
			XpEND,

	XpSTRLIST(CH_NAV,Navigations),
/*
	XpENABLE_MAP_(CH_LSENSOR, rangeEnableMap),
			XpH ( CH_LRANGE ),
			XpEND,

	XpSTRLIST(CH_LSENSOR,sensorNames),

	XpENABLE_(CH_LIGHT),
			XpH ( CH_LSENSOR ),
			XpH ( CH_LRANGE ),
			XpH ( CH_LTRIGR ),
			XpEND,
*/
	XpENABLEMSG_MAP_(CH_MBED, invertMap, "This control applies only to External Objects"),
			XpH ( CH_OVERW ),
			XpH ( CH_OBURL ),
			XpH ( CH_OBPATH ),
			XpEND,

	XpLABEL(CH_ANIMGR,"Scene Settings"),
	XpLABEL(CH_ITEMGR,"Items"),
	XpLABEL(CH_OBJGR,"Objects"),
	XpLABEL(CH_LTGR,"Lights"),
	XpLABEL(CH_ENVGR,"Environment"),

	XpTABS_(CH_TAB),
			XpH ( CH_ANIMGR ),
			XpH ( CH_ITEMGR ),
			XpH ( CH_OBJGR ),
		//	XpH ( CH_LTGR ),
			XpH ( CH_ENVGR ),
			XpEND,

	XpMIN(CH_MSTEP,1),
	XpMIN(CH_MFR1,0),
	XpMIN(CH_MFR2,1),

	XpNARROW(CH_LOWER),
	XpNARROW(CH_PROTO),
	XpLEFT(CH_PROTO),
	XpNARROW(CH_MBED),
	XpNARROW(CH_OVERW),
	XpLEFT(CH_MBED),
	XpNARROW(CH_SENSOR),
	XpNARROW(CH_RANGE),
	XpLEFT(CH_SENSOR),
	XpNARROW(CH_OBJECT),
	XpNARROW(CH_IGNORE),
	XpLEFT(CH_OBJECT),
	XpNARROW(CH_AUDON),
	XpNARROW(CH_AUDURL),
	XpLEFT(CH_AUDON),
	XpNARROW(CH_AUDVOL),
	XpNARROW(CH_AUDLOOP),
	XpLEFT(CH_AUDLOOP),
	XpNARROW(CH_VIEW),
	XpNARROW(CH_HEAD),
	XpLEFT(CH_HEAD),
	XpNARROW(CH_MORF),
	XpNARROW(CH_MFR1),
	XpNARROW(CH_MSTEP),
	XpLEFT(CH_MORF),
	XpNARROW(CH_MLOOP),
	XpNARROW(CH_MFR2),
	XpLEFT(CH_MLOOP),
	XpNARROW(CH_AUTO),
	XpLEFT(CH_AUTO),
	XpEND
};


static LWXPanelID VRML_ObjectXPanel(GlobalFunc *global, VRML_ObjectData *dat)
{
	LWXPanelID     panID = NULL;
	itype = 0;
	type = LWI_LIGHT;
	GlobalLWXPF = (LWXPanelFuncs*)(*global)( LWXPANELFUNCS_GLOBAL, GFUSE_TRANSIENT);
	if ( GlobalLWXPF ) 
	{
		panID = (*GlobalLWXPF->create)( LWXP_VIEW, vrob_ctrl_list );
		if(panID) 
		{
			(*GlobalLWXPF->hint) ( panID, 0, HUGEhint );
			(*GlobalLWXPF->describe)( panID, vrob_data_descrip, VRML_ObjectData_get, VRML_ObjectData_set );
			(*GlobalLWXPF->viewInst)( panID, dat );
			(*GlobalLWXPF->setData)(panID, 0, dat);
			(*GlobalLWXPF->setData)(panID, CH_LIGHT, &type);
			(*GlobalLWXPF->setData)(panID, CH_ITEM, &itype);
	    }
    }
	return panID;
}




int VRML_Object_UI (GlobalFunc *global, VRMLData *dat, char *buf, int siz)
{
	LWXPanelID			panID = NULL;
	VRML_ObjectData		uidata;

	uidata.vdat = dat;
	uidata.id = uidata.oid = LWITEM_NULL;
	uidata.cam = itemInfo->first(LWI_CAMERA, LWITEM_NULL); //sceneInfo->renderCamera(0.0);
	uidata.nbuf = buf;
	uidata.siz = siz;
	uidata.flags = 0;
	uidata.audio = 0;
	uidata.audloop = 1;
	SET_NAVMODE(uidata.flags, 1);
	uidata.volume = 1.0;
	uidata.sound[0] = 0;
	uidata.envpic[0] = 0;
	uidata.mLoop = 1;
	uidata.mStep = 10;
	uidata.mStart = (int)sceneInfo->frameStart;
	uidata.mEnd = (int)sceneInfo->frameEnd;
	VCLR(uidata.range);
	VRML97_CameraTagsParse(&uidata, uidata.cam);
	if (panID=VRML_ObjectXPanel(global, &uidata))
	{
		int	 res;
		res = GlobalLWXPF->post(panID);
		GlobalLWXPF->destroy(panID);
		return res;
	}
	return 0;
}


