/*******************************************************************
 *
 *    DESCRIPTION:  3DStudio R4 Material definitions
 *
 *    AUTHOR:
 *
 *    HISTORY:    
 *
 *******************************************************************/

#ifndef __MTLDEF__H
#define __MTLDEF__H

#define REND_WIRE 0
#define REND_FLAT 1
#define REND_GOURAUD 2
#define REND_PHONG 3
#define REND_METAL 4

/* Material flags field bit definitions	*/
#define MF_TWOSIDE  	(1<<0)	/* Material seen from both sides	*/
#define MF_SELF 	  	(1<<1)	/* Material self-illuminated		*/
#define MF_DECAL 		(1<<2)	/* Material maps act as decals (transparent color)	*/
#define MF_ADDITIVE 	(1<<3)	/* Material uses additive transparency	*/
#define MF_WIRE	 	(1<<4)	/* Material renders as wire frame */
#define MF_NEEDUV 	(1<<5)	/* Material has some UV type maps */
#define MF_NEED3D    (1<<6)	/* Material needs 3D coords for SXP */
#define MF_XPFALLIN  (1<<7)   /* Transparency fall-off to inside	*/
#define MF_MATTE  	(1<<8)	/* Material used as a matte		*/
#define MF_CUBICMAP 	(1<<9)	/* Reflection map is cubic		*/
#define MF_XPFALL   	(1<<10)  /* Do Transparency fall-off		*/
#define MF_SUPERSMP	(1<<11)	/* Super sample material */
#define MF_FACEMAP	(1<<12)	/* Face-texture-coords */
#define MF_PHONGSOFT	(1<<13)	/* Soften phong hilites */
#define MF_WIREABS	(1<<14)	/* Absolute wire size */
#define MF_REFBLUR   (1<<15)	/* blurred reflection map */
/* for clearing temporary flags */
#define MF_CLEAR_TEMPFLAGS ~(MF_NEEDUV|MF_NEED3D|MF_REFBLUR|MF_CUBICMAP)

typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;


typedef struct {
	uchar shade;		/* shading level for auto-cubic */
	uchar aalevel;		/* anti-alias level for auto-cubic */
	ushort flags;		/* auto cubic flags */
	int size;			/* bitmap size for auto-cubic */
	int nth;				/* do nth frame for auto-cubic */
	} AutoCubicParams;


#define CM_UP 0
#define CM_DOWN 1
#define CM_LEFT 2
#define CM_RIGHT 3
#define CM_FRONT 4
#define CM_BACK 5
#define CUBMAP_READY 1

typedef struct cubmap	{
    //	Bitmap *mapptr[6];
	char mapname[6][13];
	short flags;
	short objnum;	/* for auto-cubic= point-of-view object: -1 for user defined*/
	short mtlobj; /* object referred to to find which face is mapped (morphing)*/
	struct cubmap *next;
	// Bitmap *blurmap;  
	int x,y;  /* location for mirrors */
	} Cubmap;

/* Auto-cubic flags */
#define AC_ON  1	  /* if ON this is an auto-cubic mapped material */
#define AC_SHADOW  (1<<1)
#define AC_2SIDE  (1<<2)
#define AC_FIRSTONLY (1<<3)
#define AC_MIRROR (1<<4)

/* rmtl.use flags */
#define MATUSE_XPFALL (1<<0)
#define MATUSE_REFBLUR (1<<1)

/* TextureParam.texflags */
#define TEX_DECAL (1)
#define TEX_MIRROR (1<<1)
#define TEX_UNUSED1 (1<<2)
#define TEX_INVERT (1<<3)
#define TEX_NOWRAP (1<<4)
#define TEX_SAT (1<<5)	 /* summed area table */
#define TEX_ALPHA_SOURCE (1<<6) /* use ALPHA instead of RGB of map */
#define TEX_TINT (1<<7)         /* tint for color */
#define TEX_DONT_USE_ALPHA (1<<8)    /* don't use map alpha */
#define TEX_RGB_TINT (1<<9)    /* Do RGB color transform */

typedef struct sxpent {
	char name[13];
	ULONG handle;
	void *pdata;
	struct sxpent *next;
	} SXPentry;

#define NMAPTYPES 8
#define Ntex 0
#define Ntex2 1
#define Nopac 2
#define Nbump 3
#define Nspec 4
#define Nshin 5
#define Nselfi 6
#define Nrefl 7

#define USE_tex  (1<<Ntex)
#define USE_tex2  (1<<Ntex2)
#define USE_opac  (1<<Nopac)
#define USE_bump  (1<<Nbump)
#define USE_spec  (1<<Nspec)
#define USE_shin  (1<<Nshin)
#define USE_selfi  (1<<Nselfi)
#define USE_refl  (1<<Nrefl)


#define MAP_TYPE_UV 1
#define MAP_TYPE_SXP 2

typedef struct {
	uchar type; /* MAP_TYPE_UV, MAP_TYPE_SXP */
	ushort texflags;
	void *sxp_data;
	Bitmap *bm;
	float texblur;		/* texture blur  */
	float uscale,vscale;
	float uoffset,voffset;
	float ang_sin,ang_cos;
	Color_24 col1,col2;  /* tinting colors */
	Color_24 rcol,gcol,bcol;  /* RGB tinting colors */
	} MapParams;

typedef struct {
	AutoCubicParams acb;  /* auto-cubic params */
	Bitmap *bm;
	Bitmap *blurbm;
	} RMapParams;


typedef struct {
	uchar ok; 
	uchar kind;		/* Texture(0) or Reflection map (1)*/
	char name[13];
	union {
		MapParams tex;	  /* kind == 0 */
		RMapParams ref;  /* kind == 1 */
		} p;
	} MapData;

typedef struct {
	uchar use;
	union { float f; int pct;} amt;
	MapData map;
	MapData mask;
	} Mapping;

struct SMtl {
	char name[17];		 	/* Material's 16-char name		*/
	Color_24 amb;		 	/* 0-255 triplets			*/
	Color_24 diff;			 /* 0-255 triplets			*/
	Color_24 spec;		 	/* 0-255 triplets			*/
	short transparency;	/* 0-100	*/
	short shading;		 	/* 0=WIRE 1=FLAT 2=GOURAUD 3=PHONG 4=METAL	*/
	ulong flags;		 	/* Material flags	*/
	ushort use;   			/* Use  flags */

	/* Effect percent sliders */
	short shininess;	 	/* 0-100	   */
	short shin2pct;		/* 0-100    */
	short shin3pct;		/* 0-100    */
	short xpfall;			/* 0-100   	*/
	short refblur;			/* 0-100   	*/
	short selfipct;	  	/* 0-100    */
	float wiresize;   /* size of wire frame */

	Mapping *map[8];
	void *appdata;
	};

int get_mtlchunk(FILE *stream,void *data);
int dump_mtlchunk(ushort tag,FILE *stream,void *data);
void init_mtl_struct(SMtl *mtl);

extern SMtl *loadmtl,inmtl;

/* Material library in-memory list */
struct Mliblist	{
	SMtl material;
	Mliblist *next;
	};

/* Current mesh's in-memory list */
struct Mmtllist	{
	SMtl material;
	Mmtllist *next;
	short flags;
	};

extern SMtl *savemtl;
void FreeMatRefs(SMtl *m);
void *XMAlloc(int size);
void *XMAllocZero(int size);
void XMFree(void *p);
void XMFreeAndZero(void **p);
int TexBlurToPct(float tb);
float PctToTexBlur(int p);
void ResetMapData(MapData *md, int n, int ismask);
void InitMappingValues(Mapping *m, int n, int isRmtl);
void FreeMapDataRefs(MapData *md);
void FreeMatRefs(SMtl *m);
void ResetMapping(Mapping *m, int n, int isRmtl);
Mapping *NewMapping(int n,int isRmtl);
void init_mtl_struct(SMtl *mtl);
void set_mtl_decal(SMtl *mtl);

#endif

