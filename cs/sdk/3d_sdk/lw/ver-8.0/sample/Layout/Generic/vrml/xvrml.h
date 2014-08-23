/* xvrml.h - A VRML97 Creation plugin for LightWave 3D v6
 *		Arnie Cachelin		Copyright 1999 NewTek, Inc.
 *
 *	11/7/99
 */

#ifdef _MSWIN
#define FILE_MARK		'\\'
#endif
#ifdef _MACOS
#define	stricmp(a,b)	strcmp(a,b)
#define	strnicmp(a,b,c)	strncmp(a,b,c)
#define FILE_MARK		':'
#endif
#ifdef _XGL
#define FILE_MARK		'/'
#define stricmp strcmp
#define strnicmp strncmp
#endif

#define PI_VERSION			"2.0.13"
#define VRML2_VERSION		"V2.0 utf8"
#define MAX_STR				320
#define MAX_INDENT			63
#define MAX_LIGHT_NAME		MAX_STR
#define LINE_LEN			255
#define INDENT(d)			(strncat(d->indent,"\t",MAX_INDENT))
#define UNINDENT(d)			if(*d->indent) d->indent[strlen(d->indent)-1]=0
#define	RIGHT_HANDZ(z)		(-z)
//#define RADIANS(x)			((x)*0.017453292519943295769236907684886)
//#define DEGREES(x)			((x)*57.2957795130823208767981548141052)
#define BYTE_FRACTION(c)	(((float)c)*0.003921568)      // /255.0)
#define LERP(t,x0,x1)  	((x0) + (t)*((x1)-(x0)))
#define MIN3(a,b,c) ( (a)>(b) ? ((b)>(c)?c:b):((a)>(c)?c:a) )
#define MAX3(a,b,c) ( (a)<(b) ? ((b)>(c)?b:c):((a)>(c)?a:c) )
#define MINDEX(vec) ( (vec[0])>(vec[1]) ? ((vec[1])>(vec[2])?2:1):((vec[0])>(vec[2])? 2:0) )


// Flags bits
#define VRPREF_COMMENTS		(1UL<<30)
#define VRPREF_NORMALS		(1UL<<29)
#define VRPREF_VIEWS		(1UL<<28)
#define VRPREF_NOINSTANCE	(1UL<<27)
#define VRPREF_PROTO_OBJS	(1UL<<25)
#define VRPREF_INCLUDE_OBJS	(1UL<<26)
#define VRPREF_FLY			(1UL<<24)
#define VRPREF_EXAMINE		(1UL<<23)
#define VRPREF_WALK			(1UL<<22)
#define VRPREF_ANY			(VRPREF_FLY|VRPREF_WALK|VRPREF_EXAMINE)
#define VRPREF_HEADLIGHT	(1UL<<21)
#define VRPREF_LOWERCASE    (1UL<<18)
#define VRPREF_OVERWRITE    (1UL<<17)



#define	VDEF_SCENE_CAMERA	"MainSceneCamera"
#define	VDEF_SCENE_AMBIENT	"GlobalAmbientLight"
#define	VDEF_OBJXFORM		"_ObjXForm"
#define	VDEF_XFORM			"_XForm"
#define	VDEF_MOVER			"_Mover"
#define	VDEF_ROTATOR		"_Turner"
#define	VDEF_SIZER			"_Sizer"
#define	VDEF_TIMER			"_Clock"
#define	VDEF_FXTIMER		"_FXClock"
#define	VDEF_SWITCH			"_ClickDetect"
#define	VDEF_OVERSWITCH		"_GropeDetect"
#define	VDEF_PROXSWITCH		"_NearbyDetect"
#define	VDEF_VISSWITCH		"_SeeMeDetect"
#define	VDEF_ENVELOPE		"_Envelope"
#define	VDEF_MORPH			"MorphForms"
#define	VDEF_SCENE_CLOCK	"MasterFrame"
#define VDEF_COORDS_DEF		"_Points"
#define VDEF_OBJ_PROTO		"_ObjectProto"
#define	VDEF_AUDCLIP		"_Sound"
#define	VDEF_LOD			"_LevelOfDetail"
#define VDEF_VERTS			"Points"

#define MAX_LOD_OBJECTS		128
#define EXTERNPROTO_WORKING	

typedef struct st_VRMLContext {
	FILE		*file;
	int			vrprefs;
	char		indent[MAX_INDENT+1];
} VRMLContext;


typedef struct st_VRMLData	{
	VRMLContext		*vr;
	int				flags;
	double			avatarSize, globalLightLev;
	char			wrldir[MAX_STR];	 // dir for .wrl object files for this scene ("d:\obj\" e.g.)
	char			wrlURL[MAX_STR];	 // URL for wrldir	("http://www.myPC.obj..vr.com" e.g.)
	char			texURN[MAX_STR];	 // URN for textures("urn:web3d.org/textures/" e.g.)
	char			author[MAX_STR];
} VRMLData;

#define SC_LOWERCASE	VRPREF_LOWERCASE	
#define SC_PROTO_OBJS	VRPREF_PROTO_OBJS	
#define SC_INCLUDE_OBJS	VRPREF_INCLUDE_OBJS	
#define SC_EXAMINE		VRPREF_EXAMINE		
#define SC_WALK			VRPREF_WALK			
#define SC_HEADLIGHT	VRPREF_HEADLIGHT	
#define SC_FLY			VRPREF_FLY
#define SC_ANY			VRPREF_ANY
#define	NAVMODE(flags)	( (flags&VRPREF_ANY)>>22 ) // NONE=0, WALK=1,EXAMINE=2,FLY=4, ANY=7=FLY|WALK|EXAMINE
#define	SET_NAVMODE(flags,mode)	( (flags&(~VRPREF_ANY))|((mode)<<22) ) 

#ifdef DRAFT
#define FIELD_KEY		"keys"
#define FIELD_KEYVALUE	"values"
#define VRML2_VERSION   "Draft #2 V2.0 utf8"
#define FIELD_FRACTION	"fraction"
#else
#define VRML2_VERSION   "V2.0 utf8"
#define FIELD_KEY		"key"
#define FIELD_KEYVALUE	"keyValue"
#define FIELD_FRACTION	"fraction_changed"
#endif

#define COORDS_DEF	"_ObjectPoints"
#define TXCOORDS_DEF	"TextureUVs_SURFACE"
#define NODE_INFO		"Info","string"
#define NODE_TEXTURE	"Texture2","filename"

#define	VTAG_URL		"URL"		 // item URL
#define	VTAG_SOUND		"SOUND"		 //item Sound
#define	VTAG_LOD		"LOD"		 // LOD node
//#define	VTAG_SENSOR		"SENSOR"	 // Sensor type (i.e. touch, proximity, etc.)
#define	VTAG_TOUCH		"TOUCH"      // Over (grope)
#define	VTAG_PROXIMITY 	"PROXIMITY"	 // PROXIMITY=W H D  (active region size)
#define	VTAG_VISIBILITY	"VISIBILITY" // VISIBILITY=W H D  (active region size)
#define	VTAG_LABEL		"LABEL"
#define	VTAG_SPRITE		"BILLBOARD"
#define	VTAG_INCLUDE	"INCLUDE"	 // dump file into output
#define	VTAG_IGNORE		"IGNORE"	 // skip this one
#define	VTAG_TRIGGER	"TRIGGER"	 // Other object for sensor
#define	VTAG_VRML		"VRML"		 // Node creator
#define	VTAG_SEQUENCE	"SEQUENCE"	 // SeqMorph w/ numbered object seq.
#define	VTAG_HEADLIGHT	"HEADLIGHT"	 // Camera tag for Avatar Light
#define	VTAG_NAVIGATE	"NAVIGATE"	 // Camera tag for Avatar Navigation
#define	VTAG_VIEWPOINTS	"VIEWPOINT"	 // Camera tag for naming key n
#define	VTAG_ENVIMAGE	"ENVIRONMENT"	 // 
#define	VTAG_VERTEXRGB	"VERTEXRGB"	 // Object RGB vmap name
#define	VTAG_MORPH		"MORPH"		 // start frame, end frame, step, loop
#define	VTAG_AUTOSTART	"AUTOSTART"	 //  begin animation on load
#define	VTAG_URN		"URN"		 //  image URN: on a camers, ->envimage urn, on obj, texture urn
#define	VTAG_GRID		"GRID"		 //  ElevationGrid - w,h,xn,zn
#define	VTAG_AUTHOR		"AUTHOR"	 //  creator name, comment
#define	VTAG_AVATAR		"AVATAR"	 //  avatar size

enum {	// MUST BE ALIGNED WITH exportvrml.c::VRTags[]
	VTAGID_URL,	
	VTAGID_SOUND,
	VTAGID_LOD,	
	VTAGID_TOUCH,     	
	VTAGID_PROXIMITY, 	
	VTAGID_VISIBILITY,	
	VTAGID_LABEL,	
	VTAGID_INCLUDE,
	VTAGID_IGNORE,
	VTAGID_TRIGGER,	
	VTAGID_VRML,
	VTAGID_SEQUENCE,  
	VTAGID_HEADLIGHT,	
	VTAGID_NAVIGATE,
	VTAGID_VIEWPOINTS,
	VTAGID_ENVIMAGE,
	VTAGID_VERTEXRGB,
	VTAGID_MORPH,
	VTAGID_AUTOSTART,
	VTAGID_URN,
	VTAGID_GRID,
	VTAGID_AUTHOR,
	VTAGID_AVATAR,	
	VTAGID_LAST	
};	

enum {
	VRSENSOR_TOUCH,
	VRSENSOR_PROXIMITY,
	VRSENSOR_VISIBILITY
};	
#define	VTAGID_MAX	VTAGID_GRID

VRMLContext *openVRML97(char *name);
void closeVRML97(VRMLContext *dat);
void VRML2_AnchorGroupOpen(VRMLData *dat, LWDVector bbox, LWDVector cent, char *url);
void VRML2_TransformOpen(VRMLData *dat, char *name);
void VRML2_GroupClose (VRMLData *dat);
int VRML2_ObjWrite(VRMLData *dat, void *obj, char *url, char *comment);
int VRML2_ProtoObjWrite(VRMLData *dat, void *obj, char *url, char *comment);
void VRML2_FogWrite (VRMLContext *dat);
void VRML2_WorldInfoWrite(VRMLData *dat, char *title, int argc, char **argv);
void VRML97_ViewsWrite(VRMLContext *vr , int flags, LWDVector bbox, LWDVector cent, char *name);
void VRML2_TransformClose(VRMLData *dat);
int VRML97_TriggerWrite(VRMLData *dat, LWItemID id, void *lwo, int flags, char *name);
void VRML97_NavInfoWrite (VRMLData *dat);
int VRML97_SceneObjectsSave(VRMLData *dat);
int VRML97_ProtoMorphWrite(VRMLData *dat, LWItemID	id, char *url, int fstart, int fend, int tweens);
int VRML97_ProtoObjWrite(VRMLData *dat, void *obj, char *url, char *comment);

void GetDate(char *datebuf,int siz);
double boundsCenter(void* obj, double* bbox, double* cent);
int lwo2VRML2(GlobalFunc *global,void *lwo, char *vrml, char *url, char *comment, int flags);
int lwo2VRML97(GlobalFunc *global,LWItemID id, char *vrml, char *url, char *comment, int flags);
LWItemID findObjectID(char *name);
int VRMLPointArrayWrite(VRMLData *dat, void *odb);
int VRML97_ProtoShapeWrite(VRMLData *dat, void* odb, int srfIdx,char *name);

void VRML97_SceneWrite (VRMLData *dat);
void VRML97_AmbientLightWrite (VRMLData *dat);
void VRML97_LightWrite (VRMLData *dat, LWItemID *lt, char *name, int hasEnv);
char *VRML97_CameraWrite (VRMLData *dat, LWItemID cam);
void VRML97_FogWrite (VRMLContext *vr);
void VRML97_ExternProtoWrite(VRMLData *dat, LWItemID id, char *name);
void VRML97_ProtoInstWrite(VRMLData *dat, LWItemID id, char *name);
void VRML97_InlineObjWrite(VRMLData *dat, char *file, char *tname);
void VRML97_InlineObjDefWrite(VRMLData *dat, LWItemID id, void *obj, char *name);
void VRML97_TextWrite (VRMLData *dat,char *mes);
int VRML97_ObjTagsFlags(VRMLData *dat,LWItemID obj);
int VRML97_ObjTagsParse(VRMLData *dat,LWItemID obj, char *oname);
//void VRML97_CameraTagsParse(VRMLData *dat,LWItemID cam);
int VRML97_LODNodeWrite(VRMLData *dat,LWItemID id, char *oname);
void VRML97_BackgroundWrite (VRMLData *dat);
void VRML97_FloorWrite (VRMLContext *vr, float x, float z, int nx, int nz);

void buildItemVRName(LWItemID id, char *name, int siz);
int buildItemVRCloneName(LWItemID id, char *name, int siz);
char *itemBaseName(LWItemID id);
LWItemID *findMovingRootItem(LWItemID id);
int ignoreItem(LWItemID id);
double lightIntensity(LWItemID lit, double t, LWEnvelopeID *env);

LWItemID findAnyItem(char *name);


void OpenItemTransform(VRMLData *dat, LWItemID obj, char *oname);
void CloseItemTransform(VRMLData *dat, char *oname);
int OpenItemPivotTransform(VRMLData *dat, LWItemID obj, char *oname);
void CloseItemPivotTransform(VRMLData *dat, LWItemID obj,  char *oname);

// vrml_env.c
double itemAnimationTime(LWItemID item, int *keys);
int VRML97_ValueInterpolatorWrite(VRMLData *dat, LWChannelID chan, char *name, int *pre, int *post);
double VRML97_MoveInterpolatorWrite(VRMLData *dat, LWItemID item, char *name, int *pre, int *post);

double GetAngleAxis(double hpb[3],double v[3]);


char *filepart(char *str);
void killext(char *s);
void str_tolower(char *str);
int exists(char *name);
int swapchars(char *str,char o, char n);
void fixVRMLName(char *str, int len);

