// lwobjacc.h -- Definitions for LWObjectAccess global plugin.
// By Arnie Cachelin Copyright 1996 NewTek, Inc.
// Last Update:
//    9/23/97    - ObjHelp.destroyArray()
//      1/98 Don't stomp textureflags when TWRP precedes TFLG...

#define OBJECT_ACCESS_NAME  "LWObjectAccess"
#define OBJECT_HELP_NAME  "LWObjectHelp"

#define MAX_POINTS_PER_POLYGON 4000
typedef   unsigned short   polID;
typedef   unsigned short   pntID;
typedef   unsigned short   srfID;
typedef struct st_LWPoint
{
   float x;
   float y;
   float z;
} LWPoint;

typedef struct st_LWUV
{
   float u;
   float v;
} LWUV;

typedef struct st_LWPolygon
{
   pntID *plist;
   short npoints;
   srfID surface; // >0 ==> therefore 1 greater than index used by f'ns... arrrgh.
} LWPolygon;

typedef struct st_LWSurface
{
   char        *name;
   int            size;    // may be 0 if no/empty SURF chunk found...
   unsigned char  *data;   // NULL if size==0
} LWSurface;

// Texture flags
#define TXF_AXIS_X         1
#define TXF_AXIS_Y         2
#define TXF_AXIS_Z         4
#define TXF_WORLDCOORD     8
#define TXF_NEGATIVE    16
#define TXF_PIXBLEND    32
#define TXF_ANTIALIAS      64
#define TXF_UDECAL         128          // not REPEAT
#define TXF_VDECAL         256

// Surface flags
#define SUF_LUMINOUS             1
#define SUF_OUTLINE                    2
#define SUF_SMOOTHING                  4
#define SUF_COLORHILIGHT            8
#define SUF_COLORFILTER                16
#define SUF_EDGEOPAQUE                 32
#define SUF_EDGETRANSPARENT            64
#define SUF_SHARPTERMINATOR            128
#define SUF_DOUBLESIDED             256
#define SUF_ADDITIVE             512
#define SUF_SHADOWALPHA             1024


typedef enum {TT_PLANAR,TT_CYLINDRICAL,TT_SPHERICAL,TT_CUBIC} TxType;
typedef enum { TA_X,TA_Y,TA_Z} TxAxis;

typedef struct st_LWTexture{
   float xTextureCenter;
   float yTextureCenter;
   float zTextureCenter;
   float xTextureSize;
   float yTextureSize;
   float zTextureSize;
   TxAxis textureAxis;
   TxType textureType;
   int   textureFlags;
   float wTiles, hTiles;
   float  uoffset,voffset,uscale,vscale,angle,smAngle;     // not really used...
// char  *textureName;
} LWTexture;

typedef struct st_surfAttr {
   float       su_red,su_green,su_blue;
   unsigned short su_flags,su_lumi,su_diff,su_spec,su_refl,su_tran;
   char        *su_image;
   LWTexture      su_tx;
} surfAttr;

typedef void      *LWObjectID;
typedef LWPoint   *LWPointID;
typedef LWPolygon    *LWPolygonID;
typedef LWSurface    *LWSurfaceID;

enum {BB_XMIN,BB_XMAX,BB_YMIN,BB_YMAX,BB_ZMIN,BB_ZMAX}; // bounding box array indices

typedef struct st_ObjectAccess {
   LWObjectID     (*create)(const char *);   //, float time
   void        (*destroy)(LWObjectID);
   int            (*pointCount)(LWObjectID);
   int            (*polyCount)(LWObjectID);
   int            (*surfCount)(LWObjectID);
   LWPointID      (*pointGet)(LWObjectID, pntID);
   LWPolygonID    (*polyGet)(LWObjectID, polID);
   LWSurfaceID    (*surfGet)(LWObjectID, srfID);
   double         (*boundBox)(LWObjectID, double *);  // returns rmax,   fills double bbox[6]
} ObjectAccess;

typedef struct st_ObjectHelp {
   void        (*polyNormal)(ObjectAccess *, LWObjectID, LWPolygon *, LWPoint *);      // fill LWPoint
   LWPoint     *(*createNormals)(ObjectAccess *, LWObjectID);                       // allocate array filled with vertex then face normals -- must be freed with destroyArray!
   LWUV        *(*createUVs)(ObjectAccess *, LWObjectID, LWPoint *, LWTexture *);      // allocate array of LWUV normals -- must be freed with destroyArray!
   int            (*getSurfAttr)(LWSurfaceID, surfAttr *);                       // fill surfAttr, return SURF bytes read
   void        (*getTextureUV)(LWTexture *,LWPoint *,LWUV *,LWPoint *);  // fill LWUV , args: tx,spot,uv,norm  set axis for TT_CUBIC per poly.
   void        (*destroyArray)(void *);                        // free LWUV array from createUVs(), or LWPoint array from createNormals()
} ObjectHelp;


