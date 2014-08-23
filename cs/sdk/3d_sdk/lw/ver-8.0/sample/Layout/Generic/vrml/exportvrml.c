/* ExportVRML.c - A VRML97 Creation plugin for LightWave 3D v6
 *		Arnie Cachelin		Copyright 1999 NewTek, Inc.
 *
 *	11/7/99
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
#include <lwcustobj.h>
#include <lwpanel.h>
 
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

static	LWMessageFuncs		*GlobalMessage=NULL;
static  LWPanelFuncs		*GlobalPanFun=NULL;
LWObjectFuncs		*objFuncs=NULL;
LWCameraInfo		*camInfo=NULL;
GlobalFunc			*GlobalGlobal=NULL;
LWItemInfo			*itemInfo=NULL;
LWObjectInfo		*objInfo=NULL;
LWImageList			*imageList=NULL;
LWTextureFuncs		*texFuncs=NULL;
LWSurfaceFuncs		*surfFuncs=NULL;
LWLightInfo			*lightInfo=NULL;
LWSceneInfo			*sceneInfo=NULL;
LWChannelInfo		*chanInfo=NULL;
LWEnvelopeFuncs		*envInfo=NULL;
LWLayoutGeneric		*generic=NULL;
int GlobalFPS = 30;	

int VRML_Object_UI (GlobalFunc *global, VRMLData *dat, char *buf, int siz);

char *NavModes[] = {"NONE","WALK","EXAMINE","FLY","ANY", NULL};
static char *keyend = "       \n", TagBuf[TAG_SIZE+1]="";


// Critical array n\must match header
static char *VRTags[] = {
	VTAG_URL,	 
	VTAG_SOUND,	
	VTAG_LOD,	
	VTAG_TOUCH,     	
	VTAG_PROXIMITY, 	
	VTAG_VISIBILITY,	
	VTAG_LABEL,	
	VTAG_INCLUDE,
	VTAG_IGNORE,	
	VTAG_TRIGGER,
	VTAG_VRML, 
	VTAG_SEQUENCE, 
	VTAG_HEADLIGHT,	
	VTAG_NAVIGATE,	
	VTAG_VIEWPOINTS,	
	VTAG_ENVIMAGE,	
	VTAG_VERTEXRGB,
	VTAG_MORPH,	
	VTAG_AUTOSTART,	
	VTAG_URN,	
	VTAG_GRID,	
	VTAG_AUTHOR,	
	VTAG_AVATAR,	
	NULL	
};



//   _|~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~| Utility functions |~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~|_
void GetDate(char *datebuf,int siz)
{
	struct tm tp;
	time_t tt;
	tt=time((unsigned long *)&tp);
	strftime(datebuf,siz,"%c",localtime(&tt));
}

void newext(char *s, char *x)
{
 	char *c;
 	if( (c=strrchr(s,'.')) )
	{
 		while(*x) 
 			*c++=*x++;
 	}
	else strcat(s,x);
}
 
void killext(char *s)
{
 	char *c;
 	if( (c=strrchr(s,'.')) )
 		*c=0;
}
 
int swapchars(char *str,char o, char n)
{
	int t=0;
	char *c;
	for(c=str; *c; c++)
		if(*c==o) { *c=n; t++;}
	return(t);
}

char *filepart(char *str)
{
	char *c;
	if( (c=strrchr(str,FILE_MARK)) )
		return(++c);
	else if( (c=strrchr(str,'/')) )
		return(++c);
	else if( (c=strrchr(str,'\\')) )
		return(++c);
	return(str);
}

int exists(char *name)
{
	FILE	*fp;
	if(fp=fopen(name,"r"))
		fclose(fp);
	return(fp ? 1:0);
}

void str_tolower(char *str)
{
	char *c=str;
	while(*c)
	{
		*c = tolower(*c);
		c++;
	}
}


//   _|~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~| VRML file functions |~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~|_

int includeIntoVRML(VRMLData *dat, char *fname)
{	
	int num = 0;				   
	if(fname && *fname)
	{
		FILE *fp;
		char line[LINE_LEN+1];
		if(fp=fopen(fname,"r"))
		{
			fprintf(dat->vr->file,"%s# Including File: %s",dat->vr->indent,fname);
			while(!feof(fp))
				if(fgets(line,LINE_LEN,fp))
				{
					fprintf(dat->vr->file,"%s%s",dat->vr->indent,line);
					num++;
				}
			fclose(fp);
		}
	}
	return num;
}

VRMLContext *openVRML97(char *name)
{
	VRMLContext *dat=NULL;
	if(dat=malloc(sizeof(VRMLContext)))
	{
		dat->indent[0] = 0;
		dat->vrprefs = 0;
		dat->vrprefs |= VRPREF_COMMENTS;
		dat->vrprefs |= VRPREF_PROTO_OBJS;
		dat->vrprefs |= VRPREF_VIEWS;
		if(dat->file=fopen(name,"w"))
		{
			char buf[60];
  			fprintf(dat->file,"#VRML "VRML2_VERSION"\n");
			GetDate(buf,59);
  			fprintf(dat->file,"# Created \t%s  with Arnie's VRML97 plugin for LightWave3D v6 from NewTek, Inc.\n#\t www.NewTek.com\n",buf);
  			fprintf(dat->file,"# Plugin Version: "PI_VERSION"\n");
		}
		else {
			free(dat);
			dat=NULL;
		}
	}
	return(dat);
}

void closeVRML97(VRMLContext *vr)
{
//	fprintf(dat->file,"\n}");
	fclose(vr->file);
	free(vr);
}

void VRML2_AnchorGroupOpen(VRMLData *dat, LWDVector bbox, LWDVector cent, char *url)
{
	if(url && *url)
	{
		fprintf(dat->vr->file,"%sAnchor {\n",dat->vr->indent);
		fprintf(dat->vr->file,"%s\turl  [ \"%s\" ]\n",dat->vr->indent,url);
	}
	else
	{
		fprintf(dat->vr->file,"%sGroup {\n",dat->vr->indent);
	}
	INDENT(dat->vr);
	fprintf(dat->vr->file,"%sbboxCenter %f  %f  %f\n",dat->vr->indent,cent[0],cent[1],cent[2]);
	fprintf(dat->vr->file,"%sbboxSize %f  %f  %f\n",dat->vr->indent,bbox[0],bbox[1],bbox[2]);
	fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
	INDENT(dat->vr);
}

void VRML2_GroupClose (VRMLData *dat)
{
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
}

void VRML2_TransformOpen(VRMLData *dat, char *name)
{
	fprintf(dat->vr->file,"%sDEF %s"VDEF_OBJXFORM"  Transform  {  children [ \n",dat->vr->indent,name);
	INDENT(dat->vr);
}

void VRML2_TransformClose(VRMLData *dat)
{
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s] }  # "VDEF_OBJXFORM" End\n",dat->vr->indent);
}

//   _|~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~| LW Item/Name Functions |~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~|_

char *itemBaseName(LWItemID id)
{
	static char	buf[MAX_LIGHT_NAME];
	const char	*name=NULL;
	char		*c;
	int			 n=0;

	if(id && (name=itemInfo->name(id)) )
	{	
		name = (const char *)filepart((char *)name);
		strncpy(buf,name,MAX_LIGHT_NAME - 1);
		if(c = strrchr(buf,'('))
		{
			c--;
			*c = 0;
		}
		swapchars(buf,':','_');
		swapchars(buf,' ','_');
		return buf;
	}
	return (char*)name;
}

void fixVRMLName(char *str, int len)
{
	if(str && *str)
	{
		swapchars(str,':','_');
		swapchars(str,' ','_');
		if(isdigit(str[0]))
		{
			int n=strlen(str)+1;
			if(n<len)
			{
				while(n>0)
				{
					str[n] = str[n-1];
					n--;
				}
				str[0] = '_';
			}
		}
	}
}


void buildItemVRName(LWItemID id, char *name, int siz)
{	
	char *n;
	n = itemBaseName(id);
	if(n)
	{
		strncpy(name, n, siz-1); 
	}
	else
		strncpy(name, "no_name", siz-1); 
/*
*/
	killext(name);
	fixVRMLName(name, siz);
}


int buildItemVRCloneName(LWItemID id, char *name, int siz)
{	
	char	*n, fmt[20];
	int		clone = 0;
	n = (char*)itemInfo->name(id);
	if(n)
	{
		n = filepart(n);
		sprintf(fmt,"%%%ds (%%d)",siz-1);
		if(2>sscanf(n,fmt,name,&clone))
			clone=0;
	}
	killext(name);
	if(clone) 
	{
		char num[10];
		sprintf(num,"_%d",clone);
		strncat(name,num, siz-1); 
	} 
	fixVRMLName(name, siz);
	return clone;
}


int itemCloneNum(LWItemID id)
{
	const char	*name;
	char		*c;
	int			 n=0;

	if(id && (name=itemInfo->name(id)) )
	{
		name = (const char *)filepart((char *)name);
		if(c = strrchr(name,'('))
			n = atol(c+1);
	}
	return n;
}

LWItemID findItem(int type, const char *name)
{
	LWItemID	 id;
	const char	*n;

	id = itemInfo->first(type,NULL);
	while(id)
	{
		n = itemInfo->name(id);
		if(n && !strcmp(n,name)) 
		{
			return id; 
		}
		id = itemInfo->next(id);
	}
	return NULL;
}

// make sure clone definition can be seen from level of original VRML def.
int checkCloneItemScope(LWItemID id)
{
	LWItemID orig,prnt;
	char	*c,name[MAX_LIGHT_NAME];

	strncpy(name,itemInfo->name(id),MAX_LIGHT_NAME);
	if(c = strrchr(name,'('))
	{
		c[1] = '1';
		c[2] = ')';
		c[3] = 0;
	}
	if(orig=findItem(LWI_OBJECT,name))
	{	
		prnt = itemInfo->parent(id);
		if(prnt == itemInfo->parent(orig))
			return 1;
		while(prnt)
		{ 
			if(prnt==orig) 
				return 1;
			prnt = itemInfo->parent(id);
		} 
	}
	return 0;
}

LWItemID *findMovingRootItem(LWItemID id)
{
	LWItemID	root = NULL, prnt;
	int			k;
	prnt = itemInfo->parent(id);
	while(prnt)
	{
		itemAnimationTime(prnt,&k);
		if(k>3) 
			root = prnt; 
		else
			return root;
		prnt = itemInfo->parent(prnt);
	}
	return root;
}

// Is object Ignorable? Yes, if it or a parent has an IGNORE tag. 
int ignoreItem(LWItemID id)
{
	LWItemID mom;
	if(findItemTag(itemInfo,id,VTAG_IGNORE))
		return 1;
	mom = itemInfo->parent(id);
	while(mom!=LWITEM_NULL)
	{
		if(findItemTag(itemInfo,mom,VTAG_IGNORE))
			return 1;
		mom = itemInfo->parent(mom);
	}
	return 0;
}

void OpenItemTransform(VRMLData *dat, LWItemID obj, char *oname)
{
	LWItemID tob;
	double v[3],a,hpb[3], scl[3], pos[3];
	tob=obj;

	itemInfo->param(obj, LWIP_POSITION, 0.0, pos);
	itemInfo->param(obj, LWIP_SCALING, 0.0, scl);
	itemInfo->param(obj, LWIP_ROTATION, 0.0, hpb);
	hpb[0] =  RIGHT_HANDZ(hpb[0]);
	hpb[1] =  RIGHT_HANDZ(hpb[1]);
	hpb[2] =  hpb[2];
	a=GetAngleAxis(hpb,v);
	
//	if(obj->o_cloneNum)
//		fprintf(dat->vr->file,"%sDEF %s_%d"VDEF_XFORM" Transform {\n",dat->vr->indent,oname,obj->o_cloneNum); 
//	else
		fprintf(dat->vr->file,"%sDEF %s"VDEF_XFORM" Transform {\n",dat->vr->indent,filepart(oname) ); 
	INDENT(dat->vr);
		if(a) fprintf(dat->vr->file,"%s rotation %.4f %.4f %.4f %.4f \n",dat->vr->indent,v[0],v[1],v[2],RADIANS(a));
	if( (scl[0]!=1.0) || (scl[1]!=1.0) || (scl[2]!=1.0) )
		fprintf(dat->vr->file,"%s scale  %.4f %.4f %.4f\n",dat->vr->indent,scl[0],scl[1],scl[2]);
//	fprintf(dat->vr->file,"%s translation  %.4f %.4f %.4f\n",dat->vr->indent,
//		pos[0] + obj->o_pivot[0],pos[1] + obj->o_pivot[1],RIGHT_HANDZ(pos[2] + obj->o_pivot[2]));
	fprintf(dat->vr->file,"%s translation  %.4f %.4f %.4f\n",dat->vr->indent, pos[0],pos[1],RIGHT_HANDZ(pos[2]));
	fprintf(dat->vr->file,"%s children  [ \n",dat->vr->indent);
}

void CloseItemTransform(VRMLData *dat, char *oname)
{
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s] } # end of %s"VDEF_XFORM"\n",dat->vr->indent,oname);
}

int OpenItemPivotTransform(VRMLData *dat, LWItemID obj, char *oname)
{
	double piv[3];
	itemInfo->param(obj, LWIP_PIVOT, 0.0, piv);
	if(piv[0]==0.0 && piv[1]==0.0 && piv[2]==0.0)
		return 0;
	fprintf(dat->vr->file,"%sDEF %s_PivotPoint Transform {\n",dat->vr->indent,filepart(oname)); 
	INDENT(dat->vr);
	fprintf(dat->vr->file,"%s translation  %.4f %.4f %.4f\n",dat->vr->indent,-piv[0],-piv[1],-RIGHT_HANDZ(piv[2]));
	fprintf(dat->vr->file,"%s children  [ \n",dat->vr->indent);
	return 1;
}

void CloseItemPivotTransform(VRMLData *dat, LWItemID obj,  char *oname)
{
	double piv[3];
	itemInfo->param(obj, LWIP_PIVOT, 0.0, piv);
	if(piv[0]==0.0 && piv[1]==0.0 && piv[2]==0.0)
		return;
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s] } # end of %s_PivotPoint\n",dat->vr->indent,oname);
}


//   _|~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~| VRML Node Functions |~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~|_

void VRML97_ProtoInstWrite(VRMLData *dat, LWItemID id,char *name)
{
 	char tname[300]="";
	if(id)
		buildItemVRName(id, tname,299);	
	else
		strncpy(tname,name,299); // LOD nodes use wrong id
	fprintf(dat->vr->file,"\n%sDEF %s %s"VDEF_OBJ_PROTO" { }\n\n",dat->vr->indent,name,tname);
}
	 
void VRML97_ExternProtoWrite(VRMLData *dat, LWItemID id, char *name)
{	
	char tname[300];
	if(id)
		buildItemVRName(id, tname,299);	
	else
		strncpy(tname,name,299); // LOD nodes use wrong id

	if(dat->vr->vrprefs&VRPREF_LOWERCASE)
		str_tolower(tname);

#ifndef EXTERNPROTO_WORKING
	fprintf(dat->vr->file,"#%s EXTERNPROTO %s"VDEF_OBJ_PROTO" [ exposedField SFFloat fraction ] [ \"%s.wrl\" , \"%s%s.wrl\" ]\n",dat->vr->indent,name,tname,dat->wrlURL,tname); 
#else
//	fprintf(dat->vr->file,"%sEXTERNPROTO %s"VDEF_OBJ_PROTO" [ exposedField SFFloat fraction ] [ \"%s.wrl\" , \"%s%s.wrl\" ]\n",dat->vr->indent,name,tname,dat->wrlURL,tname); 
	fprintf(dat->vr->file,"%sEXTERNPROTO %s"VDEF_OBJ_PROTO" [ eventIn SFFloat set_fraction ] [ \"%s.wrl\" , \"%s%s.wrl\" ]\n",dat->vr->indent,name,tname,dat->wrlURL,tname); 
#endif
}

	 
void VRML97_InlineObjWrite(VRMLData *dat, char *file, char *tname)
{	 
	if(dat->vr->vrprefs&VRPREF_LOWERCASE)
		str_tolower(tname);
 	fprintf(dat->vr->file,"%sInline {\n",dat->vr->indent);
 	INDENT(dat->vr);
 	fprintf(dat->vr->file,"%surl  [ \"%s.wrl\" , \"%s%s.wrl\" ] # local then URL\n",dat->vr->indent,tname,dat->wrlURL,tname);
	UNINDENT(dat->vr);
 	fprintf(dat->vr->file,"%s}  ,\n",dat->vr->indent);
}


void VRML97_InlineObjDefWrite(VRMLData *dat,LWItemID obj,void *fukd, char *name)
{	 
	ObjectDB	*lwo = (ObjectDB*)fukd;
	LWDVector bb, cnt;
	double R=0.0;
	int cloneNum;
	char tname[300];

	VSET(bb,1.0);
	VCLR(cnt);

	buildItemVRName(obj, tname, sizeof(tname)); // no clone number here!!!

	if(lwo)
	{
		R = boundsCenter(lwo,bb, cnt);	// just a quick peek.
	}	 
	cloneNum = itemCloneNum(obj);
	if(objInfo->numPolygons(obj)) // skip NULLs	if( !(obj->o_flags&OBJF_NULL) )
	{
		/*if( (dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
		{
		 	fprintf(dat->vr->file,"%sUSE %s \n",dat->vr->indent,name);
		}	  
		else */
		
		if((cloneNum>1) && checkCloneItemScope(obj) )
		{
		 	fprintf(dat->vr->file,"%sUSE %s \n",dat->vr->indent,tname);
		}	  
		else // NOT clone obj  or out of scope of DEF
		{			
		//	if(cloneNum)
		//	 	fprintf(dat->vr->file,"%sDEF %s_%d Inline {   #Bounding Radius: %.4f \n",dat->vr->indent,name,cloneNum,R);
		//	else
			 	fprintf(dat->vr->file,"%sDEF %s Inline {   #Bounding Radius: %.4f \n",dat->vr->indent,tname,R);
		 	INDENT(dat->vr);
			if(dat->vr->vrprefs&VRPREF_LOWERCASE)
				str_tolower(tname);
		 	fprintf(dat->vr->file,"%surl  [ \"%s.wrl\" , \"%s%s.wrl\" ] #try local then URL?!\n",dat->vr->indent,tname,dat->wrlURL,tname);
		  	fprintf(dat->vr->file,"%sbboxSize %.4f %.4f %.4f\n",dat->vr->indent,
			 	bb[0], bb[1],bb[2] );
		 	fprintf(dat->vr->file,"%sbboxCenter %.4f %.4f %.4f\n",dat->vr->indent,
			 	cnt[0], cnt[1],RIGHT_HANDZ(cnt[2]) );
			UNINDENT(dat->vr);
		 	fprintf(dat->vr->file,"%s}  ,\n",dat->vr->indent);
		}
	}
	else // NULL obj
	{
		if( (cloneNum==0 ) )
		 	fprintf(dat->vr->file,"%s# DEF %s  , # just a NULL object\n",dat->vr->indent,name);
		else // clone obj
		 	fprintf(dat->vr->file,"%s# DEF %s_%d  , # just a NULL object\n",dat->vr->indent,name,cloneNum);
	}
}

// is main object first or last in list.. range values from near to far, level objs most complex first
// LOD { MFFloat range [] MFNode level [] SFVec3f center 0 0 0}

int VRML97_LODNodeWrite(VRMLData *dat,LWItemID id, char *oname)
{
	char *com_LOD,*c,tag[TAG_SIZE+1];
	float LODRangeList[MAX_LOD_OBJECTS] = {0.0f,0.0f};
	int ob=0,i=0;

	fprintf(dat->vr->file,"%s  DEF %s LOD {   \n",dat->vr->indent,oname);
 	INDENT(dat->vr);
	fprintf(dat->vr->file,"%s level [ ",dat->vr->indent);	  // spec says to leave this to browser
 	INDENT(dat->vr);
	fprintf(dat->vr->file,"\n");
	// find, print nodes
	VRML97_ProtoInstWrite(dat,id,oname);
	while(getItemTagN(itemInfo,id,VTAG_LOD"=",ob ,tag,TAG_SIZE))
	{
		com_LOD = strtok(tag, " \n");
		c = strtok(NULL, " \n");
		LODRangeList[ob++] = (float)atof(c);
		strncpy(tag, filepart(com_LOD),TAG_SIZE); 
		killext(tag);
	//	VRML97_ProtoInstWrite(dat,id,tag); // uses ID not LOD name!!!
		VRML97_ProtoInstWrite(dat,NULL,tag); 
	}
 	UNINDENT(dat->vr);

	// print ranges
	fprintf(dat->vr->file," ] \n%s range [",dat->vr->indent);
 	INDENT(dat->vr);
	for(i=0;i<ob;i++)
	{
		fprintf(dat->vr->file," %05f%c",LODRangeList[i],(i==ob-1 ? ' ':','));
		if((i&0x07)==7)
			fprintf(dat->vr->file,"\n%s",dat->vr->indent);
	}
 	UNINDENT(dat->vr);
	fprintf(dat->vr->file," ] \n",dat->vr->indent);

 	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s} #End LOD \n",dat->vr->indent);
	return 0;
}


int VRML97_LODProtosWrite(VRMLData *dat,LWItemID id, char *oname)
{
	char *com_LOD,tag[TAG_SIZE+1];
	int ob=0;
	while(getItemTagN(itemInfo,id,VTAG_LOD"=",ob ,tag,TAG_SIZE))
	{
		com_LOD = strtok(tag, " \n");
		strncpy(tag, filepart(com_LOD),TAG_SIZE); 
		killext(tag);
		VRML97_ExternProtoWrite(dat, NULL, tag);
		ob++;
	}
	return ob;
}





int VRML97_TriggerWrite(VRMLData *dat, LWItemID id, void *fukd, int flags, char *name)
{
	ObjectDB	*lwo = (ObjectDB*)fukd;
	char tag[TAG_SIZE];
	int sensor=VRSENSOR_TOUCH;
	LWDVector bb, cnt;
	double R=0.0;

	VSET(bb,1.0);
	VCLR(cnt);
	if( (flags&(1UL<<VTAGID_PROXIMITY)) || (flags&(1UL<<VTAGID_VISIBILITY))  )
		if(lwo)
		{
			R = boundsCenter(lwo,bb, cnt);	// just a quick peek.
		}	 

	if(flags&(1UL<<VTAGID_PROXIMITY))
		if(getItemTag(itemInfo,id,VTAG_PROXIMITY"=", tag,TAG_SIZE))
		{
			float w=-1,h=-1,d=-1;
			sscanf(tag,"%f %f %f",&w,&h,&d);
			fprintf(dat->vr->file,"%sDEF %s"VDEF_PROXSWITCH" ProximitySensor {\n",dat->vr->indent,name);
			fprintf(dat->vr->file,"%s\tcenter %.4f %.4f %.4f\n",dat->vr->indent,
			 	cnt[0], cnt[1],RIGHT_HANDZ(cnt[2]) );
			if((w>0)&&(h>0)&&(d>0))
				fprintf(dat->vr->file,"%s\tsize %.4f %.4f %.4f\n",dat->vr->indent,w,h,d );
			else
				fprintf(dat->vr->file,"%s\tsize %.4f %.4f %.4f\n",dat->vr->indent,
			 	bb[0], bb[1],RIGHT_HANDZ(bb[2]) );
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		}
	if(flags&(1UL<<VTAGID_VISIBILITY))
		if(getItemTag(itemInfo,id,VTAG_VISIBILITY"=", tag,TAG_SIZE))
		{
			float w=-1,h=-1,d=-1;
			sscanf(tag,"%f %f %f",&w,&h,&d);
			fprintf(dat->vr->file,"%sDEF %s"VDEF_VISSWITCH" VisibilitySensor {\n",dat->vr->indent,name);
		 	fprintf(dat->vr->file,"%s\tcenter %.4f %.4f %.4f\n",dat->vr->indent,
			 	cnt[0], cnt[1],RIGHT_HANDZ(cnt[2]) );
			if((w>0)&&(h>0)&&(d>0))
				fprintf(dat->vr->file,"%s\tsize %.4f %.4f %.4f\n",dat->vr->indent, w,h,d );
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		}

	if(!findMovingRootItem(id))
		fprintf(dat->vr->file,",\n%sDEF %s"VDEF_SWITCH"  TouchSensor { enabled TRUE } \n",dat->vr->indent,name);
	return sensor;
}



void VRML2_WorldInfoWrite(VRMLData *dat, char *title, int argc, char **argv)
{	 
	if(title && *title)
		fprintf(dat->vr->file,"%sWorldInfo { title \"%s\" \n",dat->vr->indent,title);
	if(argc)
	{
		int i;
		fprintf(dat->vr->file,"%s\t info [\n",dat->vr->indent);
		for(i=0; i<argc && argv[i]; i++)
			fprintf(dat->vr->file,"%s\t  \"%s\" \n",dat->vr->indent,argv[i]);
		fprintf(dat->vr->file,"%s\t ]\n",dat->vr->indent);
	}
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
}


int VRML97_TagNodeWrite(VRMLData *dat,LWItemID obj, char *oname, char *comment)
{
	int i,len,t=0,s=0;
	char *c,*b;	
	float	fval=-1.0f, z=1.0f;

	for(i=0; VRTags[i]; i++)
	{	
		len=strlen(VRTags[i]);
		if( !strnicmp( comment, VRTags[i], len) )	 // should stash lengths somewhere between loops...
			break;
	}
	if( (i<=VTAGID_MAX) && VRTags[i] )
	{
		switch(i)
		{
			case VTAGID_SOUND:  // SOUND=<url> [<volume> <loop?> ]
				c = strtok(comment," \n");
				c = &c[strlen("SOUND=")];
				if(b = strtok(NULL," \n"))
				{
					fval = (float)atof(b);
					b = strtok(NULL," \n");
				}
				if(fval<0.1) fval = 1.0f;
				else if(fval>1.0f) fval = 1.0f;
/*
			 	fprintf(dat->vr->file,"%sDEF %s"VDEF_AUDCLIP" AudioClip { \n",dat->vr->indent,oname);
			 	INDENT(dat->vr);
				fprintf(dat->vr->file,"%s url [ \"%s\" , \"%s%s\" ] \n",dat->vr->indent,c,dat->wrlURL,c);
				if(b && ((*b=='t') || (*b=='T')))
					fprintf(dat->vr->file,"%s loop TRUE \n",dat->vr->indent);
				fprintf(dat->vr->file,"%s description \"Sound for %s\" \n",dat->vr->indent,oname);
				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s}  ,\n",dat->vr->indent);
			 	fprintf(dat->vr->file,"%sSound { \n",dat->vr->indent);
			 	INDENT(dat->vr);
				if(fval<1.0)
					fprintf(dat->vr->file,"%s intensity %.4f\n",dat->vr->indent,fval);
				fprintf(dat->vr->file,"%s minFront 0.001  maxFront 100000\n",dat->vr->indent);
				fprintf(dat->vr->file,"%s minBack 0.001  maxBack 100000\n",dat->vr->indent);
				fprintf(dat->vr->file,"%s source  USE %s"VDEF_AUDCLIP"\n",dat->vr->indent,oname);
				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s}  ,\n",dat->vr->indent);
*/

			 	fprintf(dat->vr->file,"%sSound { \n",dat->vr->indent);
			 	INDENT(dat->vr);
				if(fval<1.0)
					fprintf(dat->vr->file,"%s intensity %.4f\n",dat->vr->indent,fval);
				fprintf(dat->vr->file,"%s minFront 0.001  maxFront 100000\n",dat->vr->indent);
				fprintf(dat->vr->file,"%s minBack 0.001  maxBack 100000\n",dat->vr->indent);
				fprintf(dat->vr->file,"%s source  DEF %s"VDEF_AUDCLIP" AudioClip {\n",dat->vr->indent,oname);
			 	INDENT(dat->vr);
				fprintf(dat->vr->file,"%s url [ \"%s\" , \"%s%s\" ] \n",dat->vr->indent,c,dat->wrlURL,c);
				if(b && ((*b=='t') || (*b=='T')))
					fprintf(dat->vr->file,"%s loop TRUE \n",dat->vr->indent);
				fprintf(dat->vr->file,"%s description \"Sound for %s\" \n",dat->vr->indent,oname);
				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s}  ,\n",dat->vr->indent);
				UNINDENT(dat->vr);
				fprintf(dat->vr->file,"%s}  ,\n",dat->vr->indent);
				break;
			case VTAGID_LABEL:
				c = strtok(comment," \n");
				c = &c[len+1];		// [strlen("INCLUDE=")];
				VRML97_TextWrite(dat,c);
				break;
			case VTAGID_INCLUDE:
				c = strtok(comment," \n");
				c = &c[strlen("INCLUDE=")];
				includeIntoVRML(dat,c);
				break;
			case VTAGID_VRML:	// VRML=nodeName { node fields } 
				c = strtok(comment," \n");
				c = &c[strlen("VRML=")];
				if(*c)
					fprintf(dat->vr->file,"%s %s  ,\n",dat->vr->indent,c);
				break;
			case VTAGID_GRID:
	//			c = strtok(comment," \n");
	//			c = &c[len+1];
				sscanf(comment,"GRID=%d %d %f %f",&t,&s,&fval,&z);
				VRML97_FloorWrite(dat->vr,fval,z,t,s);
				break;
			case VTAGID_TOUCH:
			case VTAGID_PROXIMITY:
			case VTAGID_VISIBILITY:
			case VTAGID_LOD:
			case VTAGID_MORPH:
			case VTAGID_AUTOSTART:
			case VTAGID_IGNORE:	// handled elsewhere
			case VTAGID_URL:	// handled elsewhere
			case VTAGID_SEQUENCE:	// handled elsewhere
			default:
				break;
		};
		return i+1;
	}
	return 0;
}

int VRML97_ObjTagsParse(VRMLData *dat,LWItemID obj, char *oname)
{
	char			*com, tag[TAG_SIZE+1];
	unsigned int	cnum=0,flag=0;
	int				n=1; 

	while(com = (char*)itemInfo->getTag(obj,n))
	{ 
		strncpy(tag,com,TAG_SIZE);
		cnum = VRML97_TagNodeWrite(dat,obj,oname, tag) - 1;	// + strlen("// "));
		flag |= 1UL<<cnum;
		n++;
	}
	return flag;
}

int VRML97_ObjTagsFlags(VRMLData *dat,LWItemID obj)
{
	char			*com;
	unsigned int	cnum=0,flag=0;
	int				i,len,n=1; 

	while(com = (char*)itemInfo->getTag(obj,n))
	{ 
		
		for(i=0; VRTags[i]; i++)
		{	
			len=strlen(VRTags[i]);
			if( !strnicmp( com, VRTags[i], len) )	 
				break;
		}
		if( (i<=VTAGID_MAX) && VRTags[i] )
		{
			flag |= 1UL<<i;
		}
		n++;
	}
	return flag;
}

const char *navModeName(int flags)
{
	int n;
	n=NAVMODE(flags);
	switch(n)
	{
		case 0:
		case 1:
		case 2:
			return(NavModes[n]);
		case 4:
			return("FLY");
		case 7:
		default:
			return("ANY");
	}

}

void VRML97_NavInfoWrite (VRMLData *dat)
{
//	int n;
 	fprintf(dat->vr->file,"%sNavigationInfo { \n",dat->vr->indent);
 	INDENT(dat->vr);
	if( (dat->vr->vrprefs&VRPREF_HEADLIGHT) )
		fprintf(dat->vr->file,"%sheadlight TRUE \n",dat->vr->indent);
	else
		fprintf(dat->vr->file,"%sheadlight FALSE \n",dat->vr->indent);

	fprintf(dat->vr->file,"%stype \"%s\"\n",dat->vr->indent,navModeName(dat->vr->vrprefs));
/*	n=NAVMODE(dat->vr->vrprefs);
	n=7;
	switch(n)
	{
		case 0:
		case 1:
		case 2:
			fprintf(dat->vr->file,"%stype \"%s\"\n",dat->vr->indent,NavModes[n]);
			break;
		case 4:
			fprintf(dat->vr->file,"%stype \"%s\"\n",dat->vr->indent,"FLY");
			break;
		case 7:
		default:
			fprintf(dat->vr->file,"%stype \"%s\"\n",dat->vr->indent,"ANY");
			break;
	} */

	fprintf(dat->vr->file,"%savatarSize %.4f\n",dat->vr->indent,dat->avatarSize);
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n\n",dat->vr->indent);
}


int VRML97_SceneObjectsSave (VRMLData *dat)
{
	LWItemID	obj;
	ObjectDB	*odb;
  	char name[MAX_LIGHT_NAME],  file[MAX_LIGHT_NAME],*com_url=NULL,*com_LOD=NULL;
	int objNum=0,sound=0,comflags=0, cloneNum = 0;

	for(obj=itemInfo->first(LWI_OBJECT,0); obj!=LWITEM_NULL; obj=itemInfo->next(obj))	
	{
		if(!findItemTag(itemInfo, obj, VTAG_IGNORE))
		{
			if(objInfo->numPolygons(obj)) // skip NULLs
			//	if(odb = getObjectDB( obj, GlobalGlobal ) )
				{
					cloneNum = itemCloneNum(obj);
					if(cloneNum <= 1)
						if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
						{
								buildItemVRName(obj, name, sizeof(name));
								if( (dat->vr->vrprefs&VRPREF_PROTO_OBJS) )
								{
									VRML97_ExternProtoWrite(dat,obj,name);
									VRML97_LODProtosWrite(dat, obj, name);
								}
								if(dat->vr->vrprefs&VRPREF_LOWERCASE)
									str_tolower(name);
								strncpy(file,dat->wrldir,MAX_LIGHT_NAME);
								strncat(file,name,MAX_LIGHT_NAME);
								strncat(file,".wrl",MAX_LIGHT_NAME);
								if(!com_url) 
									com_url="";
								if((dat->vr->vrprefs&VRPREF_OVERWRITE) || (!exists(file)) )	  // may exist w/ uppercase name, except on unix!!!!
								{
									lwo2VRML97(GlobalGlobal,obj,file,com_url,"",dat->vr->vrprefs);
								//	lwo2VRML2(GlobalGlobal,odb,file,com_url,"",flags);
					 				objNum++;
								}
			 			}
			 			else  
							if(odb = getObjectDB( obj, GlobalGlobal ) )
			 				{
								if( (dat->vr->vrprefs&VRPREF_PROTO_OBJS) )
										VRML97_ProtoObjWrite(dat,odb,"","");
								freeObjectDB(odb);
			 				}	
				//	freeObjectDB(odb);
				}
		}
	}
	return objNum;
}


int VRML97_MorphCoordsAdd(VRMLData *dat, LWItemID id)
{
	int i=0, otherpos=1;
	ObjectDB *odb;
	if(odb = getObjectDB( id, GlobalGlobal ) )
	{
		for(i=0; i<odb->npoints; i++)
		{
			if(i&1) 
				fprintf(dat->vr->file,"%f %f %f,\n",
						odb->pt[i].pos[otherpos][0],
						odb->pt[i].pos[otherpos][1],
						RIGHT_HANDZ(odb->pt[i].pos[otherpos][2]) );
			else 
				fprintf(dat->vr->file,"%s%f %f %f, ",dat->vr->indent,
						odb->pt[i].pos[otherpos][0],
						odb->pt[i].pos[otherpos][1],
						RIGHT_HANDZ(odb->pt[i].pos[otherpos][2]) );

		}
		freeObjectDB(odb);
	}
	return(i);
}

int VRML97_MorphWrite(VRMLData *dat, LWItemID id, int startframe, int endFrame, int tweens)
{
	int step, k=0, tot = 0, nameOK=1;
	LWCommandCode	setFrame, refresh;
	DynaValue		arg={DY_INTEGER}, res={0};
	float	fv=1.0f;

	if(!generic) return 0;

	setFrame = generic->lookup(generic->data, "GoToFrame");
	refresh = generic->lookup(generic->data, "Refresh");

	step = tweens>0 ? (int)((endFrame-startframe)*(fv=1.0f/tweens)) : endFrame-startframe;
	if(step<=0)
		step = 1;
	fprintf(dat->vr->file,"%sDEF "VDEF_MORPH" CoordinateInterpolator { \n",dat->vr->indent);
 	INDENT(dat->vr);
		fprintf(dat->vr->file,"%sset_fraction IS set_fraction\n",dat->vr->indent);
		fprintf(dat->vr->file,"%s"FIELD_KEYVALUE"  [ \n",dat->vr->indent);
		for(k=startframe; (k<=endFrame); k+=step)
		{
			arg.intv.value = k;
			tot = generic->execute(generic->data, setFrame, 1, &arg, &res);
			tot = generic->execute(generic->data, refresh, 0, &arg, &res);
			tot = generic->execute(generic->data, refresh, 0, &arg, &res);
			VRML97_MorphCoordsAdd(dat,id);
			fprintf(dat->vr->file,"%s # Frame %d\n",dat->vr->indent,k);
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
		fprintf(dat->vr->file,"%s"FIELD_KEY"  [ \n",dat->vr->indent);
		for(k=startframe; (k<=endFrame) && (tot<1024); k+=step)
		{
			fprintf(dat->vr->file,"%s %.4f,%c",dat->vr->indent,(k-startframe)*fv,keyend[k&7]);
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
 	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	return tot;
}

int VRML97_ProtoMorphWrite(VRMLData *dat, LWItemID	id, char *url, int fstart, int fend, int tweens)
{
	int i=0,srfs=0,surf, ret=0;
	char *fname;
	ObjectDB *lwo = NULL;
	char *name;	  
	LWDVector bb,cnt;

	if(!generic) return 0;

	lwo = getObjectDB( id, GlobalGlobal );
	if(lwo)
	{
		char oname[MAX_STR+1];
	//	fname = lwo->filename; // wrong w/ layers
		fname = (char*)itemInfo->name(lwo->id);
		strncpy(oname,fname,MAX_STR);
		name = filepart(oname);
		srfs=lwo->nsurfaces;
		if(dat->vr->vrprefs&VRPREF_COMMENTS)
			fprintf(dat->vr->file,"#\tObject: %s \n",name);
		killext(name);

 // PROTO header
		fprintf(dat->vr->file,"%sPROTO %s"VDEF_OBJ_PROTO" [",dat->vr->indent,name);		
	//	fprintf(dat->vr->file," exposedField SFFloat fraction 0.0");
		fprintf(dat->vr->file," eventIn SFFloat set_fraction");
		fprintf(dat->vr->file," ]\n");

 // PROTO body
		fprintf(dat->vr->file,"%s{\n",dat->vr->indent,name);		
		INDENT(dat->vr);

	/*	fprintf(dat->vr->file,"%sDEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
		VRMLPointArrayWrite(dat,lwo);
		fprintf(dat->vr->file,"}\n",dat->vr->indent); */
		boundsCenter(lwo, bb, cnt);
		if(url && *url)
		{
			fprintf(dat->vr->file,"%sAnchor {\n",dat->vr->indent);
			fprintf(dat->vr->file,"%s\turl  [ \"%s\" ]\n",dat->vr->indent,url);
			INDENT(dat->vr);
			fprintf(dat->vr->file,"%sbboxCenter %.4f  %.4f  %.4f\n",dat->vr->indent,cnt[0],cnt[1],cnt[2]);
  			fprintf(dat->vr->file,"%sbboxSize   %.4f  %.4f  %.4f\n",dat->vr->indent,bb[0],bb[1],bb[2]);
			fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
			INDENT(dat->vr);
		}
		else if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) ) 
		{
			fprintf(dat->vr->file,"%sGroup {\n",dat->vr->indent);
			INDENT(dat->vr);
			fprintf(dat->vr->file,"%sbboxCenter %.4f  %.4f  %.4f\n",dat->vr->indent,cnt[0],cnt[1],cnt[2]);
  			fprintf(dat->vr->file,"%sbboxSize   %.4f  %.4f  %.4f\n",dat->vr->indent,bb[0],bb[1],bb[2]);
			fprintf(dat->vr->file,"%schildren  [ \n",dat->vr->indent);
			INDENT(dat->vr);
		}
		VRML2_TransformOpen(dat,name); // needed for multiple surfaces!!!

/*		Done in ProtoShapeWrite for surf==0
		fprintf(dat->vr->file,"%sShape { geometry IndexedFaceSet { coord\n",dat->vr->indent);
		INDENT(dat->vr);
		fprintf(dat->vr->file,"%sDEF "VDEF_VERTS" Coordinate { ",dat->vr->indent);
		VRMLPointArrayWrite(dat,lwo);
		fprintf(dat->vr->file,"}\n",dat->vr->indent);
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"}}\n",dat->vr->indent); */
		

		VRML97_ProtoShapeWrite(dat,lwo,0,name);	  // write first one w/out a comma!
		for(surf=1; surf<srfs; surf++)
		{
			fprintf(dat->vr->file,"%s,\n",dat->vr->indent);
			VRML97_ProtoShapeWrite(dat,lwo,surf,name);
		}

//		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )  // for easier parsing at Atari (?)
			VRML2_TransformClose(dat);

		if( (url && *url) || !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS))
		{
//			VRML2_GroupClose(dat);
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s]\n",dat->vr->indent);
			UNINDENT(dat->vr);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		}
		freeObjectDB(lwo);
		
		VRML97_MorphWrite(dat, id, fstart, fend, tweens);
		fprintf(dat->vr->file,"%sROUTE "VDEF_MORPH".value_changed TO "VDEF_VERTS".point\n",dat->vr->indent);

		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s} # End PROTO body \n",dat->vr->indent,name);	
			
 // PROTO instantiation
		if( !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )	//if( !(dat->vr->vrprefs&VRPREF_NOINSTANCE) )
			fprintf(dat->vr->file,"%s %s"VDEF_OBJ_PROTO" { } # Default instance, for viewing this file directly, or inlining\n",dat->vr->indent,name);		
		if( (dat->vr->vrprefs&VRPREF_VIEWS)	&& !(dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
			VRML97_ViewsWrite(dat->vr,0, bb, cnt, name);
	}
	return srfs;
}

/* Place BEFORE lights, shapes */
char *VRML97_CameraWrite (VRMLData *dat, LWItemID cam)
{
	double v[3],hpb[3],a;			//	 FOVmin =  2*atan(1/zoomFactor) FOVmax =  2*atan( (1/zoomFactor)*(SMIN/SMAX) )
	static char name[MAX_LIGHT_NAME];

	buildItemVRCloneName(cam,name, MAX_LIGHT_NAME);
	fprintf(dat->vr->file,"%sDEF %s Viewpoint { \n",dat->vr->indent, name);
	itemInfo->param(cam, LWIP_POSITION, 0.0, v);
	fprintf(dat->vr->file,"%s\tposition  %.4f %.4f %.4f\n",dat->vr->indent,v[0],v[1],RIGHT_HANDZ(v[2]));

	itemInfo->param(cam, LWIP_ROTATION, 0.0, hpb);
	hpb[0] =  RIGHT_HANDZ(hpb[0]);
	hpb[1] =  RIGHT_HANDZ(hpb[1]);
	a = GetAngleAxis(hpb, v);
	fprintf(dat->vr->file,"%s\torientation  %.4f %.4f %.4f %.4f\n",dat->vr->indent,v[0],v[1],v[2],RADIANS(a));
	camInfo->fovAngles(cam,0.0,&a,v);
	fprintf(dat->vr->file,
		"%s\tfieldOfView %f # %.2f deg.\n%s\tdescription \"%s\"    jump FALSE\n",dat->vr->indent,a,DEGREES(a),dat->vr->indent,name);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
	return name;
}
  

void VRML97_AmbientLightWrite (VRMLData *dat)
{
	LWDVector rgb;
	lightInfo->ambient(0.0,rgb);
	fprintf(dat->vr->file,"%sDEF "VDEF_SCENE_AMBIENT" PointLight { \n",dat->vr->indent);
	fprintf(dat->vr->file,"%s\tambientIntensity  %.4f\n",dat->vr->indent,dat->globalLightLev);	 
	fprintf(dat->vr->file,"%s\tcolor  %.4f %.4f %.4f\n",dat->vr->indent,rgb[0],rgb[1],rgb[2]);
	fprintf(dat->vr->file,"%s\tintensity  0.0 \ton  TRUE\n",dat->vr->indent);	 
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
}

double lightIntensity(LWItemID lit, double t, LWEnvelopeID *env)
{
	double intens = 1.0;
	LWChanGroupID	grp;
	LWChannelID		ch;
		
	*env = NULL;
	if(lit && (grp=itemInfo->chanGroup(lit)) )
	{
		for(ch = chanInfo->nextChannel(grp,NULL); ch; ch = chanInfo->nextChannel(grp,ch))
			if(!strcmp("Intensity",chanInfo->channelName(ch)))
			{
				intens = chanInfo->channelEvaluate(ch,t);
				if(env)
					if(*env = chanInfo->channelEnvelope(ch))
					{
						LWEnvKeyframeID key = envInfo->nextKey(*env,NULL);
						if(!key && (key = envInfo->nextKey(*env,key)) )
							*env = NULL;// 1 key doesn't count
					}
			}
	}
	return intens;
}

void VRML97_LightWrite (VRMLData *dat, LWItemID *lt, char *name, int hasEnv)
{
	LWDVector		rgb, hpb,pos;
	double			a,lev;
	LWEnvelopeID	intEnv = NULL;

	lightInfo->color(lt, 0.0,rgb);
	lev = lightIntensity(lt, 0.0, &intEnv)*dat->globalLightLev;
	itemInfo->param(lt,LWIP_POSITION,0.0,pos);
	itemInfo->param(lt,LWIP_ROTATION,0.0,hpb);
	if(hasEnv)
	{
		double v[3];
		hpb[0] =  RIGHT_HANDZ((hpb[0]));
		hpb[1] =  RIGHT_HANDZ((hpb[1]));
		hpb[2] =  (hpb[2]);
		a=GetAngleAxis(hpb,v);
	
		fprintf(dat->vr->file,"%sDEF %s"VDEF_XFORM" Transform {\n",dat->vr->indent,name); 
		INDENT(dat->vr);
		if(a) fprintf(dat->vr->file,"%s rotation %.4f %.4f %.4f %.4f \n",dat->vr->indent,v[0],v[1],v[2],RADIANS(a));
		fprintf(dat->vr->file,"%s translation  %.4f %.4f %.4f\n",dat->vr->indent, pos[0],pos[1],RIGHT_HANDZ(pos[2]));
		fprintf(dat->vr->file,"%s children  [ \n",dat->vr->indent);
	}

	switch(lightInfo->type(lt))
	{
	case LWLIGHT_DISTANT:
			fprintf(dat->vr->file,"%sDEF %s DirectionalLight {\n",dat->vr->indent,name);
			fprintf(dat->vr->file,"%s\tcolor  %.4f %.4f %.4f\n",dat->vr->indent,rgb[0],rgb[1],rgb[2]);
			if(hasEnv)
				fprintf(dat->vr->file,"%s\tdirection  0.0 0.0 -1.0\n",dat->vr->indent);	 
			else
			{
				rgb[2] = cos((hpb[1]));  //z
				rgb[1] = -sin((hpb[1])); //y  
				rgb[0] = rgb[2]*sin((hpb[0]));
				rgb[2] *= cos((hpb[0]));
				fprintf(dat->vr->file,"%s\tdirection  %.4f %.4f %.4f\n",dat->vr->indent,rgb[0],rgb[1],RIGHT_HANDZ(rgb[2]));	 
			}
			if(intEnv)
				fprintf(dat->vr->file,"%s\tintensity  0.0  # Intensity Envelope\n",dat->vr->indent);	 
			else
				fprintf(dat->vr->file,"%s\tintensity  %.4f\n",dat->vr->indent,lev);	 
			fprintf(dat->vr->file,"%s\ton  TRUE\n",dat->vr->indent);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
			break;
		case LWLIGHT_SPOT:
			fprintf(dat->vr->file,"%sDEF %s SpotLight {\n",dat->vr->indent,name);
			fprintf(dat->vr->file,"%s\tcolor  %.4f %.4f %.4f\n",dat->vr->indent,rgb[0],rgb[1],rgb[2]);
			if(intEnv)
				fprintf(dat->vr->file,"%s\tintensity  0.0  # Intensity Envelope\n",dat->vr->indent);	 
			else
				fprintf(dat->vr->file,"%s\tintensity  %.4f\n",dat->vr->indent,lev);	 
			if(hasEnv)
			{
				fprintf(dat->vr->file,"%s#\tlocation  0.0 0.0 0.0\n",dat->vr->indent);
				fprintf(dat->vr->file,"%s\tdirection  0.0 0.0 -1.0\n",dat->vr->indent);	 
			}	 
			else
			{
				fprintf(dat->vr->file,"%s\tlocation  %.4f %.4f %.4f\n",dat->vr->indent,pos[0],pos[1],RIGHT_HANDZ(pos[2]));
				rgb[2] = cos((hpb[1]));  //z
				rgb[1] = -sin((hpb[1])); //y  
				rgb[0] = rgb[2]*sin((hpb[0]));
				rgb[2] *= cos((hpb[0]));
				fprintf(dat->vr->file,"%s\tdirection  %.4f %.4f %.4f\n",dat->vr->indent,rgb[0],rgb[1],RIGHT_HANDZ(rgb[2]));	 
			}
			//fprintf(dat->vr->file,"%s\tdropOffRate  0\n",dat->vr->indent);

			lightInfo->coneAngles(lt, 0.0, &a, &lev);
			fprintf(dat->vr->file,"%s\tcutOffAngle  %.4f\n",dat->vr->indent,a);
			fprintf(dat->vr->file,"%s\ton  TRUE\n",dat->vr->indent);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
			break;
		case LWLIGHT_POINT:
		default:
			fprintf(dat->vr->file,"%sDEF %s PointLight {\n",dat->vr->indent,name);
			fprintf(dat->vr->file,"%s\tcolor  %.4f %.4f %.4f\n",dat->vr->indent,rgb[0],rgb[1],rgb[2]);
			if(intEnv)
				fprintf(dat->vr->file,"%s\tintensity  0.0  # Intensity Envelope\n",dat->vr->indent);	 
			else
				fprintf(dat->vr->file,"%s\tintensity  %.4f\n",dat->vr->indent,lev);	 
			if(!hasEnv)
				fprintf(dat->vr->file,"%s\tlocation  %.4f %.4f %.4f\n",dat->vr->indent,pos[0],pos[1],RIGHT_HANDZ(pos[2]));
			fprintf(dat->vr->file,"%s\ton  TRUE\n",dat->vr->indent);
			fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
			break;
	}
	if(hasEnv)
	{
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s] } # end of %s"VDEF_XFORM"\n",dat->vr->indent,name);
	}
}


#define SQUEEZE_LEVELS		6
static double squeezeColor(double sq, double frac, LWDVector min, LWDVector max, double *rgb)
{
	double val = 0.0  ,c=0.0;
	int i;
	val = pow(frac,sq);
	for(i=0;i<3;i++)
	{
		c = val*(max[i] - min[i]); // should be >0 ... 
		rgb[i] = (min[i] + c);
	}
	return val;
}

void VRML97_BackgroundWrite (VRMLData *dat)
{													 
	int lev;
	double	frac=1.0/SQUEEZE_LEVELS, gSq,sSq;
	char *end="\n ",*c; 
	LWCompInfo		*ci;
	LWBackdropInfo	*bi;
	LWItemID		cam;
	LWDVector		 sky,nad,gnd,rgb,zen = {0.0,0.0,0.0};
	char *start[2], com[TAG_SIZE]="";

 	ci = (*GlobalGlobal)(LWCOMPINFO_GLOBAL, GFUSE_TRANSIENT);
	if(!ci)
		return;
	bi = (*GlobalGlobal)(LWBACKDROPINFO_GLOBAL, GFUSE_TRANSIENT);
	if(!bi)
		return;
	cam = sceneInfo->renderCamera(0.0);

	fprintf(dat->vr->file,"%sBackground { \n",dat->vr->indent);
 	INDENT(dat->vr);

	start[0] = " ";
	start[1] = dat->vr->indent;

	if(ci->bg)
	{
		c = (char*)imageList->filename(ci->bg,0);
		if(c)
		if(dat->vr->vrprefs&VRPREF_LOWERCASE)
		{
			char	buf[300];
			strncpy(buf,c,299);
			str_tolower(buf);
			fprintf(dat->vr->file,"%sbackUrl [ \"%s\" ]\n",dat->vr->indent,c);
		}
		else
			fprintf(dat->vr->file,"%sbackUrl [ \"%s\" ]\n",dat->vr->indent,c);
	}
	else
	{
		(*bi->color)(0.0,zen,sky,gnd,nad);
		if(bi->type==LWBACK_GRADIENT)
		{
			(*bi->squeeze)(0.0,&gSq, &sSq);
			if(gSq==1.0)
			{
				fprintf(dat->vr->file,"%sgroundAngle  1.5707958\n",dat->vr->indent);
				fprintf(dat->vr->file,"%sgroundColor  [ %.4f %.4f %.4f, %.4f %.4f %.4f ]\n",dat->vr->indent,
					nad[0], nad[1], nad[2],
					gnd[0], gnd[1], gnd[2] );
			}
			else 
			{
				fprintf(dat->vr->file,"%sgroundAngle  [ ",dat->vr->indent);
				for(lev = 1; lev<=SQUEEZE_LEVELS; lev++)
					fprintf(dat->vr->file,"%f , %c",1.5707958*frac*lev,keyend[lev&7]);
				fprintf(dat->vr->file," ]\n%sgroundColor  [ %.4f %.4f %.4f,	 # Squeeze: %3f\n",dat->vr->indent,
					nad[0], nad[1], nad[2],	gSq);
				for(lev = 1; lev<=SQUEEZE_LEVELS; lev++)
				{
					squeezeColor(gSq,frac*lev,nad,gnd,rgb);
					fprintf(dat->vr->file,"%s\t %.4f %.4f %.4f,	%c", start[lev&1], rgb[0], rgb[1], rgb[2], end[lev&1]);
				}
				fprintf(dat->vr->file,"%s]\n ",dat->vr->indent);
			}
			if(sSq==1.0)
			{
				fprintf(dat->vr->file,"%sskyAngle  1.5707958\n",dat->vr->indent);
				fprintf(dat->vr->file,"%sskyColor  [ %.4f %.4f %.4f, %.4f %.4f %.4f ]\n",dat->vr->indent,
					zen[0], zen[1], zen[2],
					sky[0], sky[1], sky[2]);
			}
			else
			{
				fprintf(dat->vr->file,"%sskyAngle  [  ",dat->vr->indent);
				for(lev = 1; lev<=SQUEEZE_LEVELS; lev++)
					fprintf(dat->vr->file,"%f , %c",1.5707958*frac*lev,keyend[lev&7]);
				fprintf(dat->vr->file," ]\n%sskyColor  [ %.4f %.4f %.4f,	# Squeeze: %3f\n",dat->vr->indent,
					zen[0], zen[1], zen[2],sSq);
				for(lev = 1; lev<=SQUEEZE_LEVELS; lev++)
				{
					squeezeColor(sSq,frac*lev, zen, sky, rgb);
					fprintf(dat->vr->file,"%s\t %.4f %.4f %.4f,	%c", start[lev&1], rgb[0], rgb[1], rgb[2], end[lev&1]);
				}
				fprintf(dat->vr->file,"%s]\n ",dat->vr->indent);
			}
		}
		else
		{
			fprintf(dat->vr->file,"%sskyColor  [ %.4f %.4f %.4f ]\n",dat->vr->indent, zen[0], zen[1], zen[2]);
		}
	}
	if(cam && getItemTag(itemInfo,cam,VTAG_ENVIMAGE"=", com,TAG_SIZE))
	{
		c = strtok(c," \n");
		TagBuf[0] = 0;
		getItemTag(itemInfo,cam,VTAG_URN,TagBuf,TAG_SIZE);
		if(*TagBuf)
		{
			fprintf(dat->vr->file, "%sbackUrl [ \"%s__back.jpg\"  , \"%s%s__back.jpg\"  ]\n",dat->vr->indent,c,TagBuf,c);
			fprintf(dat->vr->file,"%sfrontUrl [ \"%s__front.jpg\" , \"%s%s__front.jpg\" ]\n",dat->vr->indent,c,TagBuf,c);
			fprintf(dat->vr->file, "%sleftUrl [ \"%s__left.jpg\"  , \"%s%s__left.jpg\"  ]\n",dat->vr->indent,c,TagBuf,c);
			fprintf(dat->vr->file,"%srightUrl [ \"%s__right.jpg\" , \"%s%s__right.jpg\" ]\n",dat->vr->indent,c,TagBuf,c);
			fprintf(dat->vr->file,  "%stopUrl [ \"%s__up.jpg\"    , \"%s%s__up.jpg\"    ]\n",dat->vr->indent,c,TagBuf,c);
		}
		else
		{
			fprintf(dat->vr->file, "%sbackUrl [ \"%s__back.jpg\" ]\n",dat->vr->indent,c);
			fprintf(dat->vr->file,"%sfrontUrl [ \"%s__front.jpg\" ]\n",dat->vr->indent,c);
			fprintf(dat->vr->file, "%sleftUrl [ \"%s__left.jpg\" ]\n",dat->vr->indent,c);
			fprintf(dat->vr->file,"%srightUrl [ \"%s__right.jpg\" ]\n",dat->vr->indent,c);
			fprintf(dat->vr->file,  "%stopUrl [ \"%s__up.jpg\" ]\n",dat->vr->indent,c);
		}

	} 
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
}
     


void VRML2_BackWrite (VRMLContext *vr, LWFVector rgb)
{													 
 	fprintf(vr->file,"%sBackground { \n",vr->indent);
 	INDENT(vr);
	fprintf(vr->file,"%sskyColor  [ %.4f %.4f %.4f ]\n",vr->indent,rgb[0],rgb[1],rgb[2]);
	UNINDENT(vr);
	fprintf(vr->file,"%s}\n\n",vr->indent);
}

void VRML97_FogWrite (VRMLContext *vr)
{
	LWFogInfo *fi;
	LWDVector color = {0.0,0.0,0.0};

	fi = (*GlobalGlobal)(LWFOGINFO_GLOBAL, GFUSE_TRANSIENT);
	if(fi)
		if(fi->type!=LWFOG_NONE)
		{
			(*fi->color)(0.0,color);
 			fprintf(vr->file,"%sFog { \n",vr->indent);
 			INDENT(vr);
			fprintf(vr->file,"%scolor  %.4f %.4f %.4f \n",vr->indent,	color[0], color[1], color[2] );
			fprintf(vr->file,"%svisibilityRange %.4f\n",vr->indent,(*fi->maxDist)(0.0));
			if(fi->type!=LWFOG_LINEAR)
				fprintf(vr->file,"%sfogType \"EXPONENTIAL\"\n",vr->indent);
			else
				fprintf(vr->file,"%sfogType \"LINEAR\"\n",vr->indent);
			UNINDENT(vr);
			fprintf(vr->file,"%s}\n\n",vr->indent);
		}
}

void VRML97_TextWrite (VRMLData *dat,char *mes)
{
 	fprintf(dat->vr->file,"%sText { \n",dat->vr->indent);
 	INDENT(dat->vr);
	fprintf(dat->vr->file,"%sstring [ \"%s\" ]\n",dat->vr->indent,mes);
	fprintf(dat->vr->file,"%sfontStyle FontStyle { family \"SANS\" size 1.0}\n",dat->vr->indent);
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
}

void VRML97_FloorWrite (VRMLContext *vr, float x, float z, int nx, int nz)
{
	LWBackdropInfo	*bi;
	LWDVector		 sky,nad,gnd,zen = {0.0,0.0,0.0};
	bi = (*GlobalGlobal)(LWBACKDROPINFO_GLOBAL, GFUSE_TRANSIENT);
	if(bi)
		(*bi->color)(0.0,zen,sky,gnd,nad);

	//nx = (nx>1 ? nx:2);
 	//nz = (nz>1 ? nz:2);

	fprintf(vr->file,"%sElevationGrid { \n",vr->indent);
 	INDENT(vr);
	fprintf(vr->file,"%sheight  [ 0.0 0.0   0.0 0.0 ]  \n",vr->indent);
	fprintf(vr->file,"%sxDimension %d\n%szDimension %d\n",vr->indent,2,vr->indent,2);
	fprintf(vr->file,"%sxSpacing %.3f\n%szSpacing %.3f\n",vr->indent,x*0.5,vr->indent,z*0.5);
	fprintf(vr->file,"%scolorPerVertex FALSE\n",vr->indent);
	fprintf(vr->file,"%scolor  Color { color	%.4f %.4f %.4f }\n",vr->indent,nad[0],nad[1],nad[2]);
	fprintf(vr->file,"%snormal Normal{ vector	0 1.0 0 }\n",vr->indent);
	UNINDENT(vr);
	fprintf(vr->file,"%s}\n\n",vr->indent);
}


void xyztohp(double x,double y,double z,double *h,double *p)
{
    if (x == 0.0 && z == 0.0) {
        *h = 0.0;
        if (y != 0.0)   // y = sin(p)*cos(h), x=cos(p)*cos(h), z=sin(h)
            *p = (y < 0.0) ? -HALFPI : HALFPI;
        else
            *p = 0.0;
    }
    else {
        if (z == 0.0)
            *h = (x < 0.0) ? HALFPI : -HALFPI;
        else 
            *h = -atan2(x , z);
 		if (*h < 0) *h += PI*2.0; /* 0 <= *h < PI */
        x = sqrt(x * x + z * z);
        if (x == 0.0)
            *p = (y < 0.0) ? -HALFPI : HALFPI;
        else
            *p = atan2(y , x);
    }
}

void VRML97_ViewsWrite(VRMLContext *vr , int flags, LWDVector bbox, LWDVector cent, char *name)
{
	double rx,ry,rz,R,hpb[3];
	double v[3],a;
	char	vname[MAX_LIGHT_NAME]="";

	rx = 2*bbox[0];  // xo
	ry = 2*bbox[1];
	rz = 2*bbox[2];
	R = sqrt(rx*rx+ry*ry+rz*rz);
	if(R==0) R=5.0;
	xyztohp(-rx,-ry,rz,&(hpb[0]),&(hpb[1]));
	hpb[2] = 0.0;
	a=GetAngleAxis(hpb,v);
	rx += cent[0];  // xo
	ry += cent[1];
	rz += cent[2];

	if(name && *name)
	{
		strncat(vname,name,MAX_LIGHT_NAME);
		strncat(vname,"_",MAX_LIGHT_NAME);
	}
	fprintf(vr->file," NavigationInfo { type \"EXAMINE\" }\n");
	fprintf(vr->file," DEF	%sStdPerspective Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f   orientation %f %f %f %f \n",rx,ry,rz,v[0],v[1],v[2],RADIANS(a));
	fprintf(vr->file,"\tfieldOfView %f      description \"%sStdPerspective\" }\n",2*atan(bbox[0]/R), vname);

	fprintf(vr->file," DEF  %sFront Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f    orientation 0.0 1.0 0.0 0 \n",cent[0],cent[1],rz);
	fprintf(vr->file,"\tfieldOfView %f      description \"%sFront\" }\n",2*atan(bbox[0]/R), vname);

	fprintf(vr->file," DEF  %sBack Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f    orientation 0.0 1.0 0.0 3.1415926 \n",cent[0],cent[1],-rz);
	fprintf(vr->file,"\tfieldOfView %f      description \"%sBack\" }\n",2*atan(bbox[0]/R), vname);

	fprintf(vr->file," DEF %sRight_Side  Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f    orientation 0.0 1.0 0.0 1.5708 \n",rx,cent[1],cent[2]);
	fprintf(vr->file,"\tfieldOfView %f      description \"%sRight_Side\" }\n",2*atan(bbox[2]/R), vname);

	fprintf(vr->file," DEF %sLeft_Side  Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f    orientation 0.0 1.0 0.0 -1.5708 \n",-rx,cent[1],cent[2]);
	fprintf(vr->file,"\tfieldOfView %f      description \"%sLeft_Side\" }\n",2*atan(bbox[2]/R), vname);

	fprintf(vr->file," DEF %sOverhead  Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f    orientation 1.0 0.0 0.0 -1.5708 \n",cent[0],ry,cent[2]);
	fprintf(vr->file,"\tfieldOfView %f      description \"%sOverhead\" }\n",2*atan(bbox[0]/R), vname);
 
	fprintf(vr->file," DEF %sUnderneath  Viewpoint {\n",vname);
	fprintf(vr->file,"\tposition %f %f %f    orientation 1.0 0.0 0.0 1.5708 \n",cent[0],-ry,cent[2]);
	fprintf(vr->file,"\tfieldOfView %f      description \"%sUnderneath\" }\n\n",2*atan(bbox[0]/R), vname);
}

/*
int sceneStepper(int fstart, int fend, int tweens)
{
	LWCommandCode	setFrame, refresh;
	DynaValue		arg={DY_INTEGER}, res={0};
	int				f, ret=0, step;

	if(!generic) return 0;

	setFrame = generic->lookup(generic->data, "GoToFrame");
	refresh = generic->lookup(generic->data, "Refresh");
	if(setFrame && refresh)
	{
		arg.intv.value = 0;
		ret = generic->execute(generic->data, setFrame, 1, &arg, &res);
		step = tweens>0 ? (fend-fstart)/tweens : fend-fstart;
		if(step<=0)
			step = 1;
		for(f=fstart; f<=fend; f+= step)
		{
			arg.intv.value = f;
			ret = generic->execute(generic->data, setFrame, 1, &arg, &res);
			ret = generic->execute(generic->data, refresh, 0, &arg, &res);
		}
	}


}
*/

//   _|~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~| Plug-In Functions |~~*~~*~~*~~|~~*~~*~~*~~|~~*~~*~~*~~|_


static char cmd[600] = "", v_auth[222]="";
static int old_flags = VRPREF_ANY|VRPREF_LOWERCASE|VRPREF_PROTO_OBJS;
static 	VRMLData			dat;
XCALL_(static int) ExportVRML (
	long		 	version,
	GlobalFunc		*global,
	LWLayoutGeneric	*local,
	void			*serverData)
{
	LWDirInfoFunc		*dirInfo;
	char				*c, *buf, name[44]="Test.wrl";
	int					n, sz;
	
	XCALL_INIT;

	dat.flags = old_flags;
	*dat.wrldir = 0;
	*dat.wrlURL = 0;
	*dat.texURN = 0;
	dat.globalLightLev = 1.0;
	strncpy(dat.author , "Me",sizeof(dat.author));

	if (version != LWLAYOUTGENERIC_VERSION)
		return (AFUNC_BADVERSION);
	GlobalGlobal = global;
	generic = local;
	if (!(GlobalMessage=(*global)(LWMESSAGEFUNCS_GLOBAL,GFUSE_TRANSIENT)) )
		return (AFUNC_BADGLOBAL);
	if (!(itemInfo=(*global)(LWITEMINFO_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWITEMINFO_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(objInfo=(*global)(LWOBJECTINFO_GLOBAL,GFUSE_TRANSIENT)) ) 
	{
		GlobalMessage->error("Can't get global service:" ,LWOBJECTINFO_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(objFuncs=(*global)(LWOBJECTFUNCS_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWOBJECTFUNCS_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(lightInfo=(*global)(LWLIGHTINFO_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWLIGHTINFO_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(camInfo=(*global)(LWCAMERAINFO_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWCAMERAINFO_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(chanInfo=(*global)(LWCHANNELINFO_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWCHANNELINFO_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(envInfo=(*global)(LWENVELOPEFUNCS_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWENVELOPEFUNCS_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(surfFuncs=(*global)(LWSURFACEFUNCS_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWSURFACEFUNCS_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(texFuncs=(*global)(LWTEXTUREFUNCS_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWTEXTUREFUNCS_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(sceneInfo=(*global)(LWSCENEINFO_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWSCENEINFO_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(imageList=(*global)(LWIMAGELIST_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWIMAGELIST_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}
	if (!(dirInfo=(*global)(LWDIRINFOFUNC_GLOBAL,GFUSE_TRANSIENT)) )
	{
		GlobalMessage->error("Can't get global service:" ,LWDIRINFOFUNC_GLOBAL);
		return (AFUNC_BADGLOBAL);
	}

	GlobalFPS = (int)sceneInfo->framesPerSecond;
	sz = sizeof(cmd) - 1;
	buf = &(cmd[0]);
	if(sceneInfo->name[0]=='(') // "(unnamed)"
	{
		strncpy(name,sceneInfo->name+1,sizeof(name));
		name[strlen(name)-1] = 0;
	}
	else
		strncpy(name,sceneInfo->name,sizeof(name));
	killext(name);
	strncat(name,".wrl",sizeof(name));
	strncpy(dat.author,v_auth,sizeof(dat.author));
	if(!buf[0])
	{
		c = (char*)dirInfo("Content");
		if(c)
		{
			strncpy(buf,c,sz);
			strncpy(dat.wrldir,c,sizeof(dat.wrldir));
			n = strlen(dat.wrldir);
			dat.wrldir[n++] = FILE_MARK;
			dat.wrldir[n] = 0;

		}
		n = strlen(buf);
		buf[n++] = FILE_MARK;
		buf[n] = 0;
		strncat(buf,name,sz);
	}
	dat.avatarSize = 2.;
	if(VRML_Object_UI(global, &dat, buf, sz))
	{

		old_flags = dat.flags;
		strncpy(v_auth,dat.author,sizeof(v_auth));
		if(dat.vr = openVRML97(buf))
		{
			LWDVector		 bbox = {1.,1.,1.}, cent={0.,0.,0.};
			LWFVector		 sky = {0.32f,0.64f,0.88f};

			dat.vr->vrprefs = dat.flags;//|VRPREF_LOWERCASE|VRPREF_PROTO_OBJS;
			VRML97_SceneWrite(&dat);

			closeVRML97(dat.vr);
			dat.vr = NULL;
			GlobalMessage->info("Saved VRML97 File:",buf);
		}
	}
	return AFUNC_OK;
}

int CustomObj(long version, GlobalFunc *global, LWCustomObjHandler *local, void *serverData);
int CustomObj_UI(long version, GlobalFunc *global, LWInterface *UI, void *serverData);

ServerTagInfo 	si_tags[] = { 
		{"Export VRML97", SRVTAG_BUTTONNAME|LANGID_USENGLISH}, 
		{"Export Scene As VRML97",SRVTAG_USERNAME|LANGID_USENGLISH}, 
		{"file",SRVTAG_CMDGROUP}, 
		{"file",SRVTAG_MENU}, 
		{"",0} };

ServerTagInfo 	co_tags[] = { 
		{"VRML97 Object", SRVTAG_BUTTONNAME|LANGID_USENGLISH}, 
		{"VRML97 Custom Object",SRVTAG_USERNAME|LANGID_USENGLISH}, 
		{"",0} };

ServerRecord ServerDesc[] = {
	{ LWLAYOUTGENERIC_CLASS,	"VRML97_SceneExport",		ExportVRML, si_tags},
	{ LWCUSTOMOBJ_HCLASS,	"VRML97_Object",		CustomObj, co_tags},
	{ LWCUSTOMOBJ_ICLASS,	"VRML97_Object",		CustomObj_UI, co_tags},
	{ NULL }
};
