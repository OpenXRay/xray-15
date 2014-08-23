/* vrml_env.c - A VRML97 Envelope/Interpolator stuff
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
#include "eulerangle.h"

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



static double RADperDEG = PI/180;	
#define EulHPB		  EulOrdYXZr 
/* Fill v[] with axis (from origin), return angle for quaternion style rotation */
double GetAngleAxis(double hpb[3],double v[3])
{
	Quat out;
	double	ang=0,s;
	EulerAngles inAngs = {0,0,0,EulHPB};
	inAngs.x = (float)hpb[0];
	inAngs.y = (float)hpb[1];
	inAngs.z = (float)hpb[2];
	out = Eul_ToQuat(inAngs);
	ang = acos(out.w);
	s=sin(ang);
	ang *= 2/RADperDEG;
	if(s)		 /* Is this valid???!!! */
	{
		v[0] = out.x/s;
		v[1] = out.y/s;
		v[2] = out.z/s;
	}
	else if(!ang) v[0] = v[1] = v[2] = 0.0;
	return(ang);
}



// return number of keys in item, in position or rotation channels, put channels found into chan[3]
int itemKeys(LWItemID id, int mode, LWChannelID *chan, double *max)
{
	int				t=0,n = 0;
	LWChanGroupID	grp;
	LWChannelID		ch;
	LWEnvelopeID	env;
	LWEnvKeyframeID key;
	char			*name;
	double			dv;
	if(max)
		*max = 0.0;
	if(id && (grp=itemInfo->chanGroup(id)))
	{
		switch(mode)
		{
			case LWIP_POSITION:
				for(ch = chanInfo->nextChannel(grp,NULL); ch; ch = chanInfo->nextChannel(grp,ch))
				{
					name=(char*)chanInfo->channelName(ch);
					if( !strncmp(name,"Position.",9) )
					{
						if(name[9]=='X')
						{
							t++;
							chan[0] = ch;
						}
						else if(name[9]=='Y')
						{
							t++;
							chan[1] = ch;
						}
						else if(name[9]=='Z')
						{
							t++;
							chan[2] = ch;
						}
					}
					if(t>=3)
						break;
				}
				break;
			case LWIP_ROTATION:
				for(ch = chanInfo->nextChannel(grp,NULL); ch; ch = chanInfo->nextChannel(grp,ch))
				{
					name=(char*)chanInfo->channelName(ch);
					if( !strncmp(name,"Rotation.",9) )
					{
						if(name[9]=='H')
						{
							t++;
							chan[0] = ch;
						}
						else if(name[9]=='P')
						{
							t++;
							chan[1] = ch;
						}
						else if(name[9]=='B')
						{
							t++;
							chan[2] = ch;
						}
					}
					if(t>=3)
						break;
				}
				break;
			case LWIP_SCALING:
				for(ch = chanInfo->nextChannel(grp,NULL); ch; ch = chanInfo->nextChannel(grp,ch))
				{
					name=(char*)chanInfo->channelName(ch);
					if( !strncmp(name,"Scale.",6) )
					{
						if(name[6]=='X')
						{
							t++;
							chan[0] = ch;
						}
						else if(name[6]=='Y')
						{
							t++;
							chan[1] = ch;
						}
						else if(name[6]=='Z')
						{
							t++;
							chan[2] = ch;
						}
					}
					if(t>=3)
						break;
				}
				break;
		}
		for(t=0;t<3;t++)
			if(chan[t] && (env=chanInfo->channelEnvelope(chan[t])))
			{
				key = envInfo->nextKey(env,NULL);	
				while(key)
				{
					n++;
					if(max)
					{
						envInfo->keyGet(env, key,LWKEY_TIME,&dv);
						if(dv>*max)
							*max = dv;
					}
					key = envInfo->nextKey(env,key);	
				}
			}
	}
	return n; // Less than 4 basically means 1 key per channel, no animation
}

int keyStep(LWEnvelopeID env, LWEnvKeyframeID key)
{
	int		st = 0;
	double	fps, val = 0.0;
	if(key)
	{
		fps = (sceneInfo->framesPerSecond);
		envInfo->keyGet(env, key,LWKEY_TIME,&val);
		val *= fps;
		st = (int)(val + 0.5);
	}
	return st;
}

double keyValue(LWEnvelopeID env, LWEnvKeyframeID key)
{
	double val = 0.0;
	if(key)
		envInfo->keyGet(env, key,LWKEY_VALUE,&val);
	return val;
}

double envDuration(LWEnvelopeID env, int *steps)
{
	double		val = 0.0; 
	LWEnvKeyframeID key;
	if(steps)
		*steps = 0;
	if(env)
		for(key= envInfo->nextKey(env,NULL); key; key= envInfo->nextKey(env,key))
		{
			envInfo->keyGet(env, key,LWKEY_TIME,&val);
			if(steps)
				*steps = (*steps) + 1;
		}
	return val;
}

double chanDuration(LWChannelID chan, int *steps)
{
	double		val = 0.0; 
	LWEnvKeyframeID key;
	LWEnvelopeID	env;
	if(steps)
		*steps = 0;
	if(chan && (env = chanInfo->channelEnvelope(chan)) )
		for(key= envInfo->nextKey(env,NULL); key; key= envInfo->nextKey(env,key))
		{
			envInfo->keyGet(env, key,LWKEY_TIME,&val);
			if(steps)
				*steps = (*steps) + 1;
		}
	return val;
}

int itemNextStep(LWEnvelopeID *env, LWEnvKeyframeID *k, int *step, LWDVector val)
{
	int i, curstep, newstep;
	double	t;
	i = MINDEX(step);
	curstep = step[i];
	newstep	= curstep;
	t = curstep;
	t /= sceneInfo->framesPerSecond;

	if(step[0]==curstep && k[0])
	{
		val[0] = keyValue(env,k[0]);
		if(k[0] = envInfo->nextKey(env[0],k[0]))
		{
			step[0] = keyStep(env,k[0]);
			newstep = step[0];
		}
	}
	else
	{
		val[0] = envInfo->evaluate(env[0], t);
	}
	if(step[1]==curstep && k[1])
	{
		val[1] = keyValue(env,k[1]);
		if(k[1] = envInfo->nextKey(env[1],k[1]))
		{
			step[1] = keyStep(env,k[1]);
			newstep = newstep>curstep ? MIN(newstep,step[1]):step[1];
		}
	}
	else
	{
		val[1] = envInfo->evaluate(env[1], t);
	}

	if(step[2]==curstep && k[2])
	{
		val[2] = keyValue(env,k[2]);
		if(k[2] = envInfo->nextKey(env[2],k[2]))	
		{
			step[2] = keyStep(env,k[2]);
			newstep = newstep>curstep ? MIN(newstep,step[2]):step[2];
		}
	}
	else
	{
		val[2] = envInfo->evaluate(env[2], t);
	}

	if(!k[0])
		step[0] = newstep;
	if(!k[1])
		step[1] = newstep;
	if(!k[2])
		step[2] = newstep;

	return curstep;
}

double itemAnimationTime(LWItemID item, int *keys)
{
	double t = 0.0;
	LWChannelID chan[3];
	int		k[3]={0,0,0}, typ;
	LWDVector tim;

	VCLR(tim);
	typ = itemInfo->type(item);
	k[0] = itemKeys(item, LWIP_POSITION, chan,tim);
	k[1] = itemKeys(item, LWIP_ROTATION, chan,tim+1);
	if(typ==LWI_OBJECT)
		k[2] = itemKeys(item, LWIP_SCALING, chan,tim+2);
	else if(typ==LWI_LIGHT)
	{
		LWEnvelopeID intens;
		lightIntensity(item,0.0,&intens);
		if(intens)
			tim[2] = envDuration(intens, &(k[2]));
		k[2] += 2; // catch up to 3-channel types
	}
	t = MAX3(tim[0],tim[1],tim[2]);
	if(keys)
		*keys = MAX3(k[0],k[1],k[2]);
	return t;
}

static char *keyend = "       \n";

int VRML97_ValueInterpolatorWrite(VRMLData *dat, LWEnvelopeID env, char *name, int *pre, int *post)
{
	LWEnvKeyframeID		key;
	double				a,t;
	int					st, k=0;

	a = envDuration(env,&st);
	if(st)
	{
		a = 1.0/st;
	 	fprintf(dat->vr->file,"%sDEF %s"VDEF_ENVELOPE" ScalarInterpolator { \n",dat->vr->indent,name);
	 	INDENT(dat->vr);

		fprintf(dat->vr->file,"%s"FIELD_KEY"  [ \n",dat->vr->indent);
		for(key = envInfo->nextKey(env,NULL); key; key=envInfo->nextKey(env,key), k++)
		{
			st = keyStep(env, key);
			fprintf(dat->vr->file,"%s %.4f,%c",dat->vr->indent,((float)st)*a,keyend[k&7]);
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);

		fprintf(dat->vr->file,"%s"FIELD_KEYVALUE"  [ \n",dat->vr->indent);
		k = 0;
		for(key = envInfo->nextKey(env,NULL); key; key=envInfo->nextKey(env,key), k++)
		{
			t = keyValue(env, key);
			fprintf(dat->vr->file,"%s %.4f,%c",dat->vr->indent,t,keyend[k&7]);
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);

	 	UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		
		envInfo->egGet(env,NULL,LWENVTAG_PREBEHAVE,pre);
		envInfo->egGet(env,NULL,LWENVTAG_POSTBEHAVE,post);

//		fprintf(dat->vr->file,"%sROUTE "VDEF_SCENE_CLOCK"."FIELD_FRACTION" TO %s"VDEF_ENVELOPE".set_fraction \n",dat->vr->indent,name);
	}	
	return k;
}

double VRML97_MoveInterpolatorWrite(VRMLData *dat, LWItemID item, char *name, int *pre, int *post)
{
	int					i,st[3],kst, tot = 0, mov,rot,siz, tpre=0,tpost=0;
	LWEnvKeyframeID		k[3];
	LWEnvelopeID		env[3];
	LWDVector			v, hpb;
	LWChannelID			chan[3] = {NULL, NULL, NULL},rchan[3] = {NULL, NULL, NULL},schan[3] = {NULL, NULL, NULL};
	double				dur,s,a=1.0, duration = 0.0;

	s = 1.0/sceneInfo->framesPerSecond;
	*pre = *post = 0;
	mov = 3<itemKeys(item, LWIP_POSITION, chan,v) ? 1:0;
	rot = 3<itemKeys(item, LWIP_ROTATION, rchan,v+1) ? 1:0;
	siz = 3<itemKeys(item, LWIP_SCALING, schan,v+2) ? 1:0;
	dur = MAX3(v[0],v[1],v[2]);
	if(dur>0.0)	
		a = 1.0/dur;
	a *= s; //  ==1/maxstep
	if(mov)
	{
		fprintf(dat->vr->file,"%sDEF %s"VDEF_MOVER" PositionInterpolator { \n",dat->vr->indent,name);
		INDENT(dat->vr);

		env[0] = chan[0] ? chanInfo->channelEnvelope(chan[0]) : NULL;
		env[1] = chan[1] ? chanInfo->channelEnvelope(chan[1]) : NULL;
		env[2] = chan[2] ? chanInfo->channelEnvelope(chan[2]) : NULL;

		fprintf(dat->vr->file,"%s"FIELD_KEY"  [ \n",dat->vr->indent);
		k[0] = env[0]  ? envInfo->nextKey(env[0],NULL) : NULL;	
		k[1] = env[1]  ? envInfo->nextKey(env[1],NULL) : NULL;	
		k[2] = env[2]  ? envInfo->nextKey(env[2],NULL) : NULL;	
		st[0] = keyStep(env[0],k[0]);
		st[1] = keyStep(env[1],k[1]);
		st[2] = keyStep(env[2],k[2]);
		i = 0;
		while( k[0] || k[1] || k[2] )
		{
			kst = itemNextStep(env,k,st,v);
			fprintf(dat->vr->file,"%s %.4f,%c",dat->vr->indent,
				((float)kst)*a,keyend[i&7]);  // step as fraction of interpolator, scaled with clock
			i++;
			if(kst>duration) duration = kst; 
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);


		fprintf(dat->vr->file,"%s"FIELD_KEYVALUE"  [ \n",dat->vr->indent);			
		k[0] = envInfo->nextKey(env[0],NULL);	
		k[1] = envInfo->nextKey(env[1],NULL);	
		k[2] = envInfo->nextKey(env[2],NULL);	
		st[0] = keyStep(env[0],k[0]);
		st[1] = keyStep(env[1],k[1]);
		st[2] = keyStep(env[2],k[2]);

		while( k[0] || k[1] || k[2] )
		{
			kst = itemNextStep(env,k,st,v);
			fprintf(dat->vr->file,"%s %.4f %.4f %.4f,\n",dat->vr->indent,v[0],v[1],RIGHT_HANDZ(v[2]));
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);

		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
		
		tot+=2;

		if(env[0])
		{
			envInfo->egGet(env[0],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[0],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
		if(env[1])
		{
			envInfo->egGet(env[1],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[1],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
		if(env[2])
		{
			envInfo->egGet(env[2],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[2],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
	}
	if(siz)
	{
		fprintf(dat->vr->file,"%sDEF %s"VDEF_SIZER" PositionInterpolator { \n",dat->vr->indent,name);
		INDENT(dat->vr);


		env[0] = schan[0] ? chanInfo->channelEnvelope(schan[0]) : NULL;
		env[1] = schan[1] ? chanInfo->channelEnvelope(schan[1]) : NULL;
		env[2] = schan[2] ? chanInfo->channelEnvelope(schan[2]) : NULL;

		fprintf(dat->vr->file,"%s"FIELD_KEY"  [ \n",dat->vr->indent);
		k[0] = env[0]  ? envInfo->nextKey(env[0],NULL) : NULL;	
		k[1] = env[1]  ? envInfo->nextKey(env[1],NULL) : NULL;	
		k[2] = env[2]  ? envInfo->nextKey(env[2],NULL) : NULL;	
		st[0] = keyStep(env[0],k[0]);
		st[1] = keyStep(env[1],k[1]);
		st[2] = keyStep(env[2],k[2]);

		i = 0;
		while( k[0] || k[1] || k[2] )
		{
			kst = itemNextStep(env,k,st,v);
			fprintf(dat->vr->file,"%s %.4f,%c",dat->vr->indent,
				((float)kst)*a,keyend[i&7]);
			i++;
			if(kst>duration) duration = kst; 
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);


		fprintf(dat->vr->file,"%s"FIELD_KEYVALUE"  [ \n",dat->vr->indent);			
		k[0] = envInfo->nextKey(env[0],NULL);	
		k[1] = envInfo->nextKey(env[1],NULL);	
		k[2] = envInfo->nextKey(env[2],NULL);	
		st[0] = keyStep(env[0],k[0]);
		st[1] = keyStep(env[1],k[1]);
		st[2] = keyStep(env[2],k[2]);

		while( k[0] || k[1] || k[2] )
		{
			kst = itemNextStep(env,k,st,v);
			if(v[0] < 0.0) v[0] = - v[0];  // negative scale illegal!!!
			if(v[1] < 0.0) v[1] = - v[1];
			if(v[2] < 0.0) v[2] = - v[2];
			fprintf(dat->vr->file,"%s %.4f %.4f %.4f,\n",dat->vr->indent,v[0],v[1],v[2]);
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);


		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
//			fprintf(dat->vr->file,"%sROUTE "VDEF_SCENE_CLOCK"."FIELD_FRACTION" TO %s"VDEF_SIZER".set_fraction \n",dat->vr->indent,name);
//			fprintf(dat->vr->file,"%sROUTE %s"VDEF_SIZER".value_changed TO %s"VDEF_XFORM".scale \n",dat->vr->indent,name,name);
		tot+=2;
		if(env[0])
		{
			envInfo->egGet(env[0],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[0],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
		if(env[1])
		{
			envInfo->egGet(env[1],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[1],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
		if(env[2])
		{
			envInfo->egGet(env[2],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[2],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
	}
	if(rot)
	{
		fprintf(dat->vr->file,"%sDEF %s"VDEF_ROTATOR" OrientationInterpolator { \n",dat->vr->indent,name);
		INDENT(dat->vr);


		env[0] = rchan[0] ? chanInfo->channelEnvelope(rchan[0]) : NULL;
		env[1] = rchan[1] ? chanInfo->channelEnvelope(rchan[1]) : NULL;
		env[2] = rchan[2] ? chanInfo->channelEnvelope(rchan[2]) : NULL;

		fprintf(dat->vr->file,"%s"FIELD_KEY"  [ \n",dat->vr->indent);
		k[0] = env[0]  ? envInfo->nextKey(env[0],NULL) : NULL;	
		k[1] = env[1]  ? envInfo->nextKey(env[1],NULL) : NULL;	
		k[2] = env[2]  ? envInfo->nextKey(env[2],NULL) : NULL;	
		st[0] = keyStep(env[0],k[0]);
		st[1] = keyStep(env[1],k[1]);
		st[2] = keyStep(env[2],k[2]);

		i = 0;
		fprintf(dat->vr->file,dat->vr->indent);
		while( k[0] || k[1] || k[2] )
		{
			kst = itemNextStep(env,k,st,v);
			fprintf(dat->vr->file,"  %.4f,%c",((float)kst)*a,keyend[i&7]);
			i++;
			if(kst>duration) duration = kst; 
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);


		fprintf(dat->vr->file,"%s"FIELD_KEYVALUE"  [ \n",dat->vr->indent);			
		k[0] = envInfo->nextKey(env[0],NULL);	
		k[1] = envInfo->nextKey(env[1],NULL);	
		k[2] = envInfo->nextKey(env[2],NULL);	
		st[0] = keyStep(env[0],k[0]);
		st[1] = keyStep(env[1],k[1]);
		st[2] = keyStep(env[2],k[2]);

		while( k[0] || k[1] || k[2] )
		{
			kst = itemNextStep(env,k,st,v);
		//	hpb[0] =  -RADIANS(v[0]);
		//	hpb[1] =  -RADIANS(v[1]);
		//	hpb[2] =  RADIANS(v[2]);
			hpb[0] =  -(v[0]);
			hpb[1] =  -(v[1]);
			hpb[2] =  (v[2]);
			a=GetAngleAxis(hpb,v);
			fprintf(dat->vr->file,"%s %.4f %.4f %.4f %.4f,\n",dat->vr->indent,
				v[0],v[1],v[2],RADIANS(a));
		}
		fprintf(dat->vr->file,"%s]\n",dat->vr->indent);


		
		UNINDENT(dat->vr);
		fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
//			fprintf(dat->vr->file,"%sROUTE "VDEF_SCENE_CLOCK"."FIELD_FRACTION" TO %s"VDEF_ROTATOR".set_fraction \n",dat->vr->indent,name);
//			fprintf(dat->vr->file,"%sROUTE %s"VDEF_ROTATOR".value_changed TO %s"VDEF_XFORM".rotation \n",dat->vr->indent,name,name);
		tot+=2;
		if(env[0])
		{
			envInfo->egGet(env[0],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[0],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
		if(env[1])
		{
			envInfo->egGet(env[1],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[1],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
		if(env[2])
		{
			envInfo->egGet(env[2],NULL,LWENVTAG_PREBEHAVE,&tpre);
			envInfo->egGet(env[2],NULL,LWENVTAG_POSTBEHAVE,&tpost);
			*pre |= tpre;
			*post |= tpost;
		}
	}
	return duration*s;
}

void VRML97_SceneClockWrite (VRMLData *dat, char *name, int loop, int enable, double duration, char *tag)
{
	fprintf(dat->vr->file,"%sDEF %s%s TimeSensor  {\n",dat->vr->indent,name,tag);
	INDENT(dat->vr);
	if(enable)
		fprintf(dat->vr->file,"%s startTime 0\n",dat->vr->indent);
	else // set with touch sensor's touchtime
		fprintf(dat->vr->file,"%s startTime -1 \n",dat->vr->indent);   
	if(loop)
		fprintf(dat->vr->file,"%s loop TRUE   stopTime 0\n",dat->vr->indent);
	else
		fprintf(dat->vr->file,"%s loop FALSE   stopTime 0\n",dat->vr->indent);// scn->startFrame/scn->fps);
	fprintf(dat->vr->file,"%s cycleInterval %.4f  \n",dat->vr->indent,duration);
	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
}


// connect 	trigger's Touch sensor to object's timer's start time
void VRML97_TouchTriggerRoute(VRMLData *dat, char *trig, char *name, char *clock)
{
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_SWITCH".touchTime TO %s%s.startTime \n",dat->vr->indent,trig,name,clock);
}

// connect 	trigger's Touch sensor to object's timer's start time
void VRML97_GropeTriggerRoute(VRMLData *dat, char *trig, char *name, char *clock)
{
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_SWITCH".isOver TO %s%s.set_enabled \n",dat->vr->indent,trig,name,clock);
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_SWITCH".touchTime TO %s%s.startTime \n",dat->vr->indent,trig,name,clock);
}

// connect 	trigger's Proximity sensor to object's timer's start time
void VRML97_ProximityTriggerRoute(VRMLData *dat, char *trig, char *name, char *clock)
{
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_PROXSWITCH".enterTime TO %s%s.startTime \n",dat->vr->indent,trig,name,clock);
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_PROXSWITCH".exitTime TO %s%s.stopTime \n",dat->vr->indent,trig,name,clock);
}

// connect 	trigger's Visibility sensor to object's timer's start time
void VRML97_VisibilityTriggerRoute(VRMLData *dat, char *trig, char *name, char *clock)
{
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_VISSWITCH".isActive TO %s%s.set_enabled \n",dat->vr->indent,trig,name,clock);
	fprintf(dat->vr->file,"%sROUTE %s"VDEF_VISSWITCH".enterTime TO %s%s.startTime \n",dat->vr->indent,trig,name,clock);
}

//Connect detectors (sensors)
void VRML97_TriggerRoutesWrite(VRMLData *dat, char *trig, char *name, char *clock,int flags)
{
	name = filepart(name);
	VRML97_TouchTriggerRoute(dat,trig,name,clock);
	if(flags&(1UL<<VTAGID_PROXIMITY))		// trigger flags???
		VRML97_ProximityTriggerRoute(dat,trig,name,clock);
	if(flags&(1UL<<VTAGID_VISIBILITY))
		VRML97_VisibilityTriggerRoute(dat,trig,name,clock);
	if(flags&(1UL<<VTAGID_TOUCH))
		VRML97_GropeTriggerRoute(dat,trig,name,clock);
	return;
}

// connect timers to interpolators
int VRML97_TimerRoutesWrite(VRMLData *dat, LWItemID id, LWChannelID chan, char *name) //, int swtch)
{
	int tot=0;
	if(!id && !chan) 
		return tot;	 
	name = filepart(name);
	if(id)	 
	{	 
		LWDVector tim;
		int		k[3];
		LWChannelID ch[3];
		int mov=0,siz=0,rot=0, typ;
		name = filepart(name);
		typ = itemInfo->type(id);
		k[0] = itemKeys(id, LWIP_POSITION, ch,tim);
		k[1] = itemKeys(id, LWIP_ROTATION, ch,tim+1);
		if(typ==LWI_OBJECT)
		{
			k[2] = itemKeys(id, LWIP_SCALING, ch,tim+2);
			tot = MAX3(k[0],k[1],k[2]);
		}
		else
			tot = MAX(k[0],k[1]);

		if(typ==LWI_OBJECT)
		{
			if(k[0]>3)	 //XYZ
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_MOVER".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_MOVER".value_changed TO %s"VDEF_XFORM".set_translation \n",dat->vr->indent,name,name);
				tot++;
			}
			if(k[2]>3)	 	// Scale
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_SIZER".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_SIZER".value_changed TO %s"VDEF_XFORM".set_scale \n",dat->vr->indent,name,name);
				tot++;
			}
			if(k[1]>3)		// HPB
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_ROTATOR".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_ROTATOR".value_changed TO %s"VDEF_XFORM".set_rotation \n",dat->vr->indent,name,name);
				tot++;
			}
		}
		else if(typ==LWI_LIGHT)	// write ROUTES for intensity, if necessary
		{
			LWEnvelopeID	env;
			if(k[0]>3)	 //XYZ
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_MOVER".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_MOVER".value_changed TO %s"VDEF_XFORM".set_translation \n",dat->vr->indent,name,name);
				tot++;
			}
			if(k[1]>3)		// HPB
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_ROTATOR".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_ROTATOR".value_changed TO %s"VDEF_XFORM".set_rotation \n",dat->vr->indent,name,name);
				tot++;
			}
			lightIntensity(id, 0.0, &env);
			if(env)
			{
				tim[2] = envDuration(env, &rot);
				if(rot>1) // keys!!
				{
					fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_ENVELOPE".set_fraction \n",dat->vr->indent,name,name);
					tot++;
					fprintf(dat->vr->file,"%sROUTE %s"VDEF_ENVELOPE".value_changed TO %s.set_intensity \n",dat->vr->indent,name,name);
					tot++;
				}
			}
		}
		else // Camera: write ROUTES for ViewPoint position, orientation, not Transform
		{
			if(k[0]>3)	 //XYZ
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_MOVER".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_MOVER".value_changed TO %s.set_position \n",dat->vr->indent,name,name);
				tot++;
			}
			if(k[1]>3)		// HPB
			{
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_ROTATOR".set_fraction \n",dat->vr->indent,name,name);
				tot++;
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_ROTATOR".value_changed TO %s.set_orientation \n",dat->vr->indent,name,name);
				tot++;
			}

		}/*		if(tot && swtch)
		{
			fprintf(dat->vr->file,"%sROUTE %s"VDEF_SWITCH".touchTime TO %s"VDEF_TIMER".startTime \n",dat->vr->indent,name,name);
			tot++;
		} */
	}
	else if(chan)
	{
		fprintf(dat->vr->file,"%sROUTE %s"VDEF_TIMER"."FIELD_FRACTION" TO %s"VDEF_ENVELOPE".set_fraction \n",dat->vr->indent,name,name);
		tot++;
	}
	return tot;
}




static char tagBuf[MAX_LIGHT_NAME]="";
// recursive...
static int VRML97_SceneObjWrite (VRMLData *dat,LWItemID id, char *oname)
{
	unsigned int comflags=0,sound=0;
	int k,s,t=1,typ,clockLoop=1, clockTrigger=0,sclockLoop=1, sclockTrigger=1,csiz=0,LOD=0, animated=0; 
	ObjectDB	*odb=NULL;
	LWItemID	kid;
	double	dur=0.0;
 	char cname[84], *com_url=NULL;

	*cname = 0;
	*tagBuf = 0;
	if(getItemTag(itemInfo,id,VTAG_IGNORE"=", tagBuf,MAX_LIGHT_NAME))
	{
			com_url = strtok(tagBuf,"\n");
			fprintf(dat->vr->file,"%s### Intentionally Ignoring %s, and descendants ( %s ) \n",dat->vr->indent,oname,com_url);
			return 0;
	}
	typ = itemInfo->type(id);
	if(typ==LWI_OBJECT)
	{
		odb = getObjectDB(id, GlobalGlobal);

	/*	if(getItemTag(itemInfo,id,VTAG_SEQUENCE"=", cname,MAX_LIGHT_NAME))
		{
			com_url = strtok(cname,"\n");
			if(*com_url)
				sscanf(com_url,"%d %d",&sclockLoop,&sclockTrigger);
			com_url = NULL;
		} */
		if(getItemTag(itemInfo,id,VTAG_MORPH"=", tagBuf,MAX_LIGHT_NAME))
		{
			com_url = strtok(tagBuf,"\n");
			if(*com_url)
			{
				sscanf(com_url,"%d %d %d %d",&k,&t,&s,&sclockLoop);
				t -= k;
			}
			com_url = NULL;
		}
		if(getItemTag(itemInfo,id,VTAG_URL"=", tagBuf,MAX_LIGHT_NAME))
		{
			com_url = strtok(tagBuf,"\n");
			fprintf(dat->vr->file,"%sAnchor { url [ %s ]  children\n",dat->vr->indent,com_url);
		}  // Handle Collision, LOD, BillBoard here too!?
		LOD = findItemTag(itemInfo, id, VTAG_LOD);
	}
	strncpy(tagBuf,(oname),MAX_LIGHT_NAME);
	oname = filepart(oname);
	OpenItemTransform(dat,id,oname);
	if(typ==LWI_OBJECT)
	{
		OpenItemPivotTransform(dat,id,oname);
		if(LOD)
			VRML97_LODNodeWrite(dat,id,oname);
		else
		{
			if( (dat->vr->vrprefs&VRPREF_INCLUDE_OBJS) )
			{
				if( (dat->vr->vrprefs&VRPREF_PROTO_OBJS) )
				{
					if(objInfo->numPolygons(id)) // skip NULLs
						VRML97_ProtoInstWrite(dat,id,oname);
				}
				else
				{
					if(odb)
						VRML2_ObjWrite(dat,odb,"","");
				}
			}
			else
			{
				if( (dat->vr->vrprefs&VRPREF_PROTO_OBJS) )
				{
	#ifndef EXTERNPROTO_WORKING
					VRML97_InlineObjDefWrite(dat,id,odb,oname);
	#else
					if(objInfo->numPolygons(id)) // skip NULLs
						VRML97_ProtoInstWrite(dat,id,oname);
				//	VRML97_ExternProtoWrite(dat,id,oname);
	#endif		
				}
				else
					VRML97_InlineObjDefWrite(dat,id,odb,oname);
			}
		}
	}
	else if(typ==LWI_LIGHT)
		VRML97_LightWrite(dat, id, oname, 0);

	comflags=VRML97_ObjTagsParse(dat,id,oname);
	k = 1;
	for(kid=itemInfo->firstChild(id); kid!=LWITEM_NULL; kid=itemInfo->nextChild(id, kid) )	
	{
		fprintf(dat->vr->file,"%s \t\t # child #%d of %s\n",dat->vr->indent,k++,oname);
		buildItemVRCloneName(kid,cname,84);
		VRML97_SceneObjWrite(dat,kid, cname);	
	}
	sprintf(cname,"%s",oname);

	dur = itemAnimationTime(id,&k);

	//	if( ( k || obj->morfTarg 
	if( ( (k>3) || (comflags&(1UL<<VTAGID_SOUND)) || (comflags&(1UL<<VTAGID_MORPH)) ) )	// any animation
	{
		animated = 1;
		VRML97_TriggerWrite(dat,id,odb,comflags,oname);
	}


	if(typ==LWI_OBJECT)
	{
		CloseItemPivotTransform(dat,id,oname);	// enclose children
		CloseItemTransform(dat,oname);
		if(com_url)
			fprintf(dat->vr->file,"%s} #end Anchor\n",dat->vr->indent);
	}
	else
		CloseItemTransform(dat,oname);

	dur = VRML97_MoveInterpolatorWrite(dat,id,cname, &clockTrigger, &clockLoop);
	if(dur>0.0)
	{
		clockLoop = clockLoop>1 ? 1:0;//(obj->o_motEnv->flags&MOTF_STOP) ? 0:1;
		clockTrigger = clockTrigger>1 ? 1:0; //comflags&(1UL<<VTAGID_AUTOSTART) ? 1:0; //(obj->o_motEnv->flags&(MOTF_STOP|MOTF_REPEAT)) ? 1:0;
		VRML97_SceneClockWrite(dat,cname, clockLoop, clockTrigger,dur, VDEF_TIMER);
	}  
	if(typ==LWI_LIGHT)
	{
		LWEnvelopeID intens;
		lightIntensity(id,0.0,&intens);
		if(intens)
		{
			dur = envDuration(intens, &clockLoop);
			if( (dur>0.0) && clockLoop)
				VRML97_ValueInterpolatorWrite(dat,intens,cname,&clockTrigger,  &clockLoop);
			VRML97_SceneClockWrite(dat,cname, clockLoop, clockTrigger,dur, VDEF_TIMER);
		}
	}
	else if(typ==LWI_OBJECT)
	{
		if( comflags&(1UL<<VTAGID_MORPH) )
		{
			sclockTrigger = comflags&(1UL<<VTAGID_AUTOSTART) ? 1:0;
			if(t)
				dur = ((double)t)/sceneInfo->framesPerSecond;
			VRML97_SceneClockWrite(dat,cname, sclockLoop, sclockTrigger,dur, VDEF_FXTIMER);
		}
	}
	return comflags;

}
 
static int saveObjs = 1;

void VRML97_SceneWrite (VRMLData *dat)
{	 
	LWItemID	cam, id, NextO;
//  	Object3D	*MyObject;
	double		dur;
  	char *c,name[MAX_LIGHT_NAME],  file[MAX_LIGHT_NAME],*com_url=NULL,*com_LOD=NULL,*args[3],buf[60]="";
	int		flags=0,comflags=0, k,m;
	
	if(args[0] = dat->author)
	{
		GetDate(buf,59);
	  	sprintf(name," Lightwave3D VRML97 Scene Created %s",buf);
	 	args[1] = name;
 		VRML2_WorldInfoWrite(dat,"World Author",2,args);   
	}
	cam = itemInfo->first(LWI_CAMERA, LWITEM_NULL);//VRML97_CameraTagsParse(dat,cam);
	VRML97_NavInfoWrite(dat);
 	if(saveObjs)
		VRML97_SceneObjectsSave(dat);

	fprintf(dat->vr->file,"\n%sTransform { children  \n",dat->vr->indent);
 	fprintf(dat->vr->file,"%sDEF SCENE_ROOT"VDEF_XFORM" Transform { children [ \n",dat->vr->indent);
 	INDENT(dat->vr);
// scene prescan

	while( cam ) 
	{
		c = VRML97_CameraWrite(dat,cam);
		dur = VRML97_MoveInterpolatorWrite(dat, cam,c, &k, &m);
		if( dur>0.0 )
		{
			fprintf(dat->vr->file," ,\n");
			VRML97_SceneClockWrite(dat,c, m>1 ? 1:0, k>1 ? 1:0, dur, VDEF_TIMER);	// loop==0 ...
		}  
		fprintf(dat->vr->file," ,\n");
		cam = itemInfo->next(cam);
	}

// environment output	
	VRML97_AmbientLightWrite(dat);	 
  	fprintf(dat->vr->file," ,\n");
//		if(VRML97_ValueInterpolatorWrite(dat,Scene->lws_ambEnv,VDEF_SCENE_AMBIENT,Scene->lws_ambEnv->steps)) //Scene->endFrame))
//		  	fprintf(dat->vr->file," ,\n");

	VRML97_FogWrite(dat->vr);
  	fprintf(dat->vr->file," ,\n");

	VRML97_BackgroundWrite(dat);
  	fprintf(dat->vr->file," ,\n");

// Object heirarchy scan
	for(id=itemInfo->first(LWI_OBJECT,0); id!=LWITEM_NULL; id=itemInfo->next(id))	// should check all item types
		 	if(itemInfo->parent(id)==LWITEM_NULL) // root level
			{
				buildItemVRCloneName(id,name,MAX_LIGHT_NAME);
				VRML97_SceneObjWrite(dat,id,name);
			  	fprintf(dat->vr->file,"%s ,\n",dat->vr->indent);
			}

// root level lights & kids missed above
	for(id=itemInfo->first(LWI_LIGHT,0); id!=LWITEM_NULL; id=itemInfo->next(id))	
		 	if(itemInfo->parent(id)==LWITEM_NULL) // root level
			{
				buildItemVRCloneName(id,name,MAX_LIGHT_NAME);
				if(lightInfo->type(id)==LWLIGHT_DISTANT)
					VRML97_LightWrite(dat, id, name, 0);
				else
					VRML97_SceneObjWrite(dat,id,name);
			  	fprintf(dat->vr->file,"%s ,\n",dat->vr->indent);
			}


	VRML97_SceneClockWrite(dat,VDEF_SCENE_CLOCK,1,1,dur,VDEF_TIMER);
  	fprintf(dat->vr->file," ,\n");
 	UNINDENT(dat->vr);
	fprintf(dat->vr->file,"%s  ]  }\n",dat->vr->indent);
	fprintf(dat->vr->file,"%s}\n",dat->vr->indent);
//scene post-scan

// 	if(Scene->lws_ambEnv)
//		VRML2_TimerRoutesWrite(dat,Scene->lws_ambEnv,VDEF_SCENE_AMBIENT);//,0);	
//object post-scan
	for(id=itemInfo->first(LWI_OBJECT,0); id!=LWITEM_NULL; id=itemInfo->next(id))	// should check all item types
 	{
		k = 0;
		if(ignoreItem(id))
			continue;

		buildItemVRCloneName(id,name,MAX_LIGHT_NAME);
		comflags = VRML97_ObjTagsFlags(dat,id);
		dur = itemAnimationTime(id,&k);
		if( (k>3) || (comflags&(1UL<<VTAGID_SOUND)) || (comflags&(1UL<<VTAGID_MORPH)) )
		{							  
			char *trigg = name;
			int trigflags = comflags;
			if(getItemTag(itemInfo, id,VTAG_TRIGGER"=",file,sizeof(file)))
				trigg = file;
			else if(NextO = findMovingRootItem(id))
			{
				buildItemVRCloneName(NextO,file,MAX_LIGHT_NAME);
				trigflags = VRML97_ObjTagsFlags(dat,NextO);
				trigg = file;
				// trigflags |= comflags;
			}
			if(dur>0)
			{
				VRML97_TimerRoutesWrite(dat,id,NULL,name);//,(MyObject->o_motEnv->flags&(MOTF_STOP|MOTF_REPEAT)) ? 1:0); 
				VRML97_TriggerRoutesWrite(dat,trigg,name,VDEF_TIMER,trigflags);
		//		VRML2_TouchTriggerRoute(dat,trigg,name);
			}
			if( (comflags&(1UL<<VTAGID_MORPH)) ) 
			{
				VRML97_TriggerRoutesWrite(dat,trigg,name,VDEF_FXTIMER,trigflags);
				fprintf(dat->vr->file,"%sROUTE %s"VDEF_FXTIMER"."FIELD_FRACTION" TO %s.set_fraction \n",dat->vr->indent,name,name);
			}
			if( (comflags&(1UL<<VTAGID_SOUND)) ) 
				VRML97_TriggerRoutesWrite(dat,trigg,name,VDEF_AUDCLIP,trigflags);
		}
	}

//Light post-scan
	for(id=itemInfo->first(LWI_LIGHT,0); id!=LWITEM_NULL; id=itemInfo->next(id))	// should check all item types
 	{
		if(ignoreItem(id))
			continue;
		buildItemVRCloneName(id,name,MAX_LIGHT_NAME);
		comflags = VRML97_ObjTagsFlags(dat,id);
		dur = itemAnimationTime(id,&k);
		if( (k>3) )
		{							  
			char *trigg = name;
			int trigflags = comflags;
			if(getItemTag(itemInfo, id,VTAG_TRIGGER"=",file,sizeof(file)))
				trigg = file;
			else if(NextO = findMovingRootItem(id))
			{
				buildItemVRCloneName(NextO,file,MAX_LIGHT_NAME);
				trigflags = VRML97_ObjTagsFlags(dat,NextO);
				trigg = file;
				// trigflags |= comflags;
			}
			if(dur>0)
			{
				VRML97_TimerRoutesWrite(dat,id,NULL,name);//,(MyObject->o_motEnv->flags&(MOTF_STOP|MOTF_REPEAT)) ? 1:0); 
				VRML97_TriggerRoutesWrite(dat,trigg,name,VDEF_TIMER,trigflags);
		//		VRML2_TouchTriggerRoute(dat,trigg,name);
			}
		}
	}
	for(cam=itemInfo->first(LWI_CAMERA,0); cam!=LWITEM_NULL; cam=itemInfo->next(cam))	
	{
		comflags = VRML97_ObjTagsFlags(dat,cam);
		dur = itemAnimationTime(cam,&k);
		buildItemVRCloneName(cam,name,MAX_LIGHT_NAME);
		VRML97_TimerRoutesWrite(dat,cam,NULL,name);//,0);	
	}
}


