
/**********************************************************************
 *<
	FILE: 3dsimp.h

	DESCRIPTION:  .3DS file import module header file

	CREATED BY: Tom Hudson

	HISTORY: created 26 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma pack(1)


/* 3DS Shape point structure */

struct shppt
{
float x;	/* Control point */
float y;
float z;
float inx;	/* Incoming vector */
float iny;
float inz;
float outx;	/* Outgoing vector */
float outy;
float outz;
unsigned short flags;
};
typedef struct shppt Shppt;

#include "ofile.h"		// 3DS Object file header
#include "cfile.h"		// 3DS Project file header
#include "3dsshape.h"	// 3DS shape file header
#include "kfio.h"		// 3DS KF header
#pragma pack()

struct Bkgrad {
	float midpct;
	Color botcolor;
	Color midcolor;
	Color topcolor;
	};

struct Fogdata {
	float nearplane;
	float neardens;
	float farplane;
	float fardens;
	Color color;
	};

struct LFogData {
	float zmin,zmax;
	float density;
	short type;
	short fog_bg;
	Color color;
	};

struct Distcue {
	float nearplane;
	float neardim;
	float farplane;
	float fardim;
	};

#define ENV_DISTCUE 1
#define ENV_FOG 2
#define ENV_LAYFOG 3

#define BG_SOLID 1
#define BG_GRADIENT 2
#define BG_BITMAP 3

struct BGdata {
	int bgType;
	int envType;
	Color bkgd_solid;
	Color amb_light;
	Bkgrad bkgd_gradient;
	Fogdata fog_data;
	Distcue distance_cue;
	int fog_bg,dim_bg;
	char bkgd_map[81];
	LFogData lfog_data;
	};

class StudioImport : public SceneImport {
public:
					StudioImport();
					~StudioImport();
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	int				DoImport(const TCHAR *name,ImpInterface *i,Interface *gi, BOOL suppressPrompts=FALSE);	// Import file
	};

#define SINGLE_SHAPE 0
#define MULTIPLE_SHAPES 1

class StudioShapeImport : public SceneImport {
	friend INT_PTR CALLBACK ShapeImportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static int		importType;
	int				shapeNumber;
					StudioShapeImport();
					~StudioShapeImport();
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	int				DoImport(const TCHAR *name,ImpInterface *i,Interface *gi, BOOL suppressPrompts=FALSE);	// Import file
	};

// Handy file class

class WorkFile {
private:
	FILE *stream;
public:
					WorkFile(const TCHAR *filename,const TCHAR *mode) { stream = _tfopen(filename,mode); };
					~WorkFile() { if(stream) fclose(stream); stream = NULL; };
	FILE *			Stream() { return stream; };
	};

// Some 3DS structures

#pragma pack(1)
typedef struct {
	float r,g,b;
	} Color_f;

typedef struct {
	unsigned char r,g,b;
	} Color_24;

typedef struct {
	unsigned short tag;
	long size;
	} Chunk_hdr;

typedef struct {
	float x;
	float y;
	float z;
	unsigned short flags;
	} Verts;

typedef struct {
	float u;
	float v;
	} Texverts;

typedef struct {
	unsigned short a;
	unsigned short b;
	unsigned short c;
	unsigned char material;
	unsigned char filler;
	unsigned long sm_group;
	unsigned short flags;
	} Faces;

typedef struct {
	float x;
	float y;
	float z;
	float tx;
	float ty;
	float tz;
	float bank;
	float focal;
	unsigned short flags;
	float nearplane;
	float farplane;
	void *appdata;
	} Camera3DS;

typedef struct dirlight{
	float x;
	float y;
	float z;
	float tx;
	float ty;
	float tz;
	unsigned short flags;
	Color color;
	float hotsize;
	float fallsize;
	float lo_bias;
//	Object_list *exclude;
	int shadsize;
	float in_range,out_range;		/* Attenuation range */
	float shadfilter;	/* size of filter box*/
	char imgfile[13];
	float ray_bias;
	float bank,aspect;	/* Spotlight bank angle, aspect ratio */
	float mult;			/* Light multiplier */
	void *appdata;
	NameTab excList;
	} Dirlight;

typedef struct
	{
	float lo_bias,hi_bias;
	short shadsize,shadsamp,shadrange;
	} Locshad;

typedef struct {
	float bias,shadfilter;
	short shadsize;
	} LocShad2;

#pragma pack()

// 3DS face edge vis flags
#define ABLINE (1<<2)
#define BCLINE (1<<1)
#define CALINE 1

#define VWRAP (1<<11)		/* Texture coord V wraps on this face */
#define UWRAP (1<<3)		/* Texture coord U wraps on this face */

// Node list structure

#define OBJ_MESH 0
#define OBJ_OMNILIGHT 1
#define OBJ_SPOTLIGHT 2
#define OBJ_CAMERA 3
#define OBJ_DUMMY 4
#define OBJ_TARGET 5
#define OBJ_OTHER  6 // generated from app data

// 3DS Key structures

#pragma pack(1)
#define KEYHDR  \
	TimeValue time;  \
	float tens,cont,bias; \
	float easeTo,easeFrom;

typedef struct { float p,ds,dd; } PosElem;
typedef struct {
	KEYHDR
	PosElem e[8]; /* enough to be bigger than the biggest key,
					including RotKey */
	} Key;

typedef struct {
	KEYHDR
	PosElem e[1];
	} ScalarKey;

typedef struct {
	KEYHDR
	PosElem e[3];
	} PosKey;	

typedef struct {
	KEYHDR
	float angle;	/* angle of rotation in radians (always >0) */
	float axis[3]; /* axis of rotation (unit vector) */
	float q[4];  	/* quaternion describing orientation */
	float b[4];		/* incoming tangent term */
	float a[4];		/* outgoing tangent term */
	} RotKey;

#ifdef LATER
typedef struct {
	KEYHDR
	Namedobj *object;  
	} MorphKey;	

typedef struct {
	KEYHDR
	} HideKey;
#endif // LATER

typedef struct {
	KEYHDR
	FLOAT c[3];
	} ColorKey;
#pragma pack()

// key types
#define KEY_FLOAT	0
#define KEY_POS		1
#define KEY_ROT		2
#define KEY_SCL		3
#define KEY_COLOR	4

#define NUMTRACKS 8

#define POS_TRACK_INDEX 0
#define ROT_TRACK_INDEX 1
#define SCL_TRACK_INDEX 2
#define FOV_TRACK_INDEX 3
#define ROLL_TRACK_INDEX 4
#define COL_TRACK_INDEX 5
#define HOT_TRACK_INDEX 6
#define FALL_TRACK_INDEX 7


typedef struct {
	union {
		Key key;
		PosKey pos;
		RotKey rot;
		ColorKey col;
		ScalarKey sc;
		} key;
	void *next;
	} KeyList;

// A list of 3DS objects with their names and types
typedef struct {
	void *object;
	Point3 srcPos;
	Point3 targPos;
	TSTR name;
	int type;
	int used;
	int cstShad;
	int rcvShad;
	int mtln;
	Matrix3 tm;
	void *next;
	} WkObjList;

// A list of the nodes and their various keys
typedef struct {
	ImpNode *node;
	short id;
	int type;
	int mnum;
	TSTR name;
	TSTR owner;
	Mesh *mesh;
	ImpNode *parent;
	Matrix3 tm;
	KeyList *posList;
	KeyList *rotList;
	KeyList *scList;
	KeyList *colList;
	KeyList *hotList;
	KeyList *fallList;
	KeyList *fovList;
	KeyList *rollList;
	SHORT trackFlags[NUMTRACKS];
	void *next;
	} WkNodeList;

/* Camera flag bit meanings */

#define NO_CAM_CONE	0x0001
#define NO_CAM_TEMP_APPDATA 0x0002 /* Free appdata after rendering complete  */	

#define NO_CAM_CONE_OFF	(~NO_CAM_CONE)

/* Light flag bit meanings */

#define NO_LT_ON	0x0001
#define NO_LT_SHAD	0x0002
#define NO_LT_LOCAL	0x0004
#define NO_LT_CONE	0x0008
#define NO_LT_RECT	0x0010
#define NO_LT_PROJ	0x0020
#define NO_LT_OVER	0x0040
#define NO_LT_ATTEN	0x0080
#define NO_LT_RAYTR	0x0100
#define NO_LT_TEMP_APPDATA 0x0200 /* Free appdata after rendering complete  */	

#define NO_LT_OFF	(~NO_LT_ON)
#define NO_LT_SHAD_OFF	(~NO_LT_SHAD)
#define NO_LT_LOCAL_OFF	(~NO_LT_LOCAL)
#define NO_LT_CONE_OFF	(~NO_LT_CONE)
#define NO_LT_RECT_OFF	(~NO_LT_RECT)
#define NO_LT_PROJ_OFF	(~NO_LT_PROJ)
#define NO_LT_OVER_OFF	(~NO_LT_OVER)
#define NO_LT_ATTEN_OFF	(~NO_LT_ATTEN)

/*--------- Track flags bits------------ */

/*-- This bit causes the spline to be cyclic */
#define ANIM_CYCLIC 1
/*-- This bit causes a track to continue "modulo" its duration */
#define ANIM_LOOP  (1<<1)
/*-- This bit is used by anim.c, but clients need not worry about it*/
#define ANIM_NEGWRAP (1<<2)

#define X_LOCKED (1<<3)
#define Y_LOCKED (1<<4)
#define Z_LOCKED (1<<5)
#define ALL_LOCKED (X_LOCKED|Y_LOCKED|Z_LOCKED)
#define TRACK_ATKEY (1<<6)

/* these flags specify which coords are NOT inherited from parent */ 
#define LNKSHFT 7
#define NO_LNK_X (1<<LNKSHFT)
#define NO_LNK_Y (1<<(LNKSHFT+1))
#define NO_LNK_Z (1<<(LNKSHFT+2))
#define LASTAXIS_SHFT 10
#define LASTAXIS_MASK (3<<LASTAXIS_SHFT)

// A worker object for dealing with creating the objects.
// Useful in the chunk-oriented 3DS file format

// Worker types

#define WORKER_IDLE		0
#define WORKER_MESH		1
#define WORKER_KF		2
#define WORKER_SHAPE	3
#define WORKER_LIGHT	4
#define WORKER_CAMERA	5

struct SMtl;

struct MtlName{
	char s[20];
	};


struct ExclListSaver {
	GenLight *lt;
	NameTab nametab;
	};

class ObjWorker {
public:
	Tab<ExclListSaver*> exclSaver;
	int okay;
	ImpInterface *i;
	Interface *ip;
	TSTR name;
	int mode;
	int gotverts;
	int verts;
	int tverts;
	int gottverts;
	int gotfaces;
	int faces;
	int cstShad;
	int rcvShad;
	BOOL gotM3DMAGIC;
	Mtl  *sceneMtls[256];
	MtlName mtlNames[256];
	TriObject *object;
	GenLight *light;
	Dirlight studioLt;
	GenCamera *camera;
	Camera3DS studioCam;
	SplineShape *splShape;
	BezierShape *shape;
	Spline3D *spline;
	Mesh *mesh;
	ImpNode *thisNode;
	ImpNode *parentNode;
	short id;
	WkObjList *objects;
	WkNodeList *nodes;
	WkNodeList *workNode;
	Matrix3 tm;
	Point3 pivot;
	DummyObject *dummy;
	int isDummy;
	int lightType;
	TSTR nodename;
	// Time stuff:
	BOOL lengthSet;
	TimeValue length;
	BOOL segmentSet;
	Interval segment;	
	SHORT trackFlags[NUMTRACKS];
	MtlList loadMtls;
	Tab<Mesh *> loadMtlMesh;
	void *appdata;
	DWORD appdataLen;
	Tab<UVVert>newTV;

	float hook_x, hook_y;
					ObjWorker(ImpInterface *iptr,Interface *ip);
					~ObjWorker() { FinishUp(); FreeObjList(); FreeNodeList(); i->RedrawViews(); }
	int				StartMesh(const char *name);
	int				StartLight(const char *name);
	int				CreateLight(int type);
	int				StartCamera(const char *name);
	int				CreateCamera(int type);
	int				StartKF(ImpNode *node);
	int				StartShape();
	int				StartSpline();
	int				AddShapePoint(Shppt *p);
	int				CloseSpline();
	int				FinishShape();
	int				FinishUp();
	void			SetTm(Matrix3 *transform) { tm = *transform; }
	int				SetVerts(int count);
	int				SetTVerts(int count);
	int				GetVerts() { return verts; }
	int				SetFaces(int count);
	int				GetFaces() { return faces; }
	int				PutVertex(int index,Verts *v);
	int				PutTVertex(int index,UVVert *v);
	int				PutFace(int index,Faces *f);
	int				PutSmooth(int index,unsigned long smooth);
	int 			PutFaceMtl(int index, int imtl);
	void 			SetTVerts(int nf, Faces *f);
	DWORD 			AddNewTVert(UVVert p);
	void			Reset();
	void			Abandon();
	int				AddObject(Object *obj,int type,const TCHAR *name, Matrix3* tm, int mtlNum=-1);
	int				AddNode(ImpNode *node,const TCHAR *name,int type,Mesh *mesh,char *owner,int mtlNum=-1);
	int				SetNodeId(ImpNode *node,short id);
	int				SetNodesParent(ImpNode *node,ImpNode *parent);
	void *			FindObject(char *name, int &type, int &cstShad, int &rcvShad, int &mtlNum);
	int				UseObject(char *name);
	int				CompleteScene();
	int 			SetupEnvironment();
	int 			FixupExclusionLists();
	ImpNode *		FindNode(char *name);
	ImpNode *		FindNodeFromId(short id);
	WkNodeList *	FindEntry(char *name);
	WkNodeList *	FindEntryFromId(short id);
	WkNodeList *	FindNodeListEntry(ImpNode *node);
	WkObjList *		FindObjListEntry(TSTR &name);
	void *			FindObjFromNode(ImpNode *node);
	int				FindTypeFromNode(ImpNode *node, Mesh **mesh);
	TCHAR *			NodeName(ImpNode *node);
	void			FreeObjList();
	void			FreeNodeList();
	ImpNode *		ThisNode() { return thisNode; }
	ImpNode *		ParentNode() { return parentNode; }
	void			SetParentNode(ImpNode *node) { parentNode = node; }
	void			SetPivot(Point3 p) { pivot = p; }
	int				AddPositionKey(PosKey *key) { return AddKey(&workNode->posList,(Key *)key); }
	int				AddRotationKey(RotKey *key) { return AddKey(&workNode->rotList,(Key *)key); }
	int				AddScaleKey(PosKey *key) { return AddKey(&workNode->scList,(Key *)key); }
	int				AddColorKey(ColorKey *key) { return AddKey(&workNode->colList,(Key *)key); }
	int				AddHotKey(ScalarKey *key) { return AddKey(&workNode->hotList,(Key *)key); }
	int				AddFallKey(ScalarKey *key) { return AddKey(&workNode->fallList,(Key *)key); }
	int				AddFOVKey(ScalarKey *key) { return AddKey(&workNode->fovList,(Key *)key); }
	int				AddRollKey(ScalarKey *key) { return AddKey(&workNode->rollList,(Key *)key); }
	int				AddKey(KeyList **list,Key *data);
	void			FreeKeyList(KeyList **list);
	int				SetTransform(ImpNode *node,Matrix3& m);
	Matrix3			GetTransform(ImpNode *node);
	int				ReadyDummy();
	ImpNode *		MakeDummy(const TCHAR *name);
	ImpNode *		MakeANode(const TCHAR *name, BOOL target,char *owner);
	void			SetDummy(int x) { isDummy = x; }
	int				IsDummy() { return isDummy; }
	int				SetDummyBounds(Point3& min,Point3& max);
	void			SetNodeName(const TCHAR *name) { nodename = name; }
	void			SetInstanceName(ImpNode *node, const TCHAR *iname);
	void			SetAnimLength(TimeValue l) { length = l; lengthSet = TRUE; }
	void			SetSegment(Interval seg) { segment = seg; segmentSet = TRUE; }
	void			SetControllerKeys(Control *cont,KeyList *keys,int type,float f=1.0f,float aspect=-1.0f);
	void            MakeControlsTCB(Control *tmCont,SHORT *tflags);
	Mtl*			GetMaxMtl(int i);
	void 			AssignMtl(INode *theINode, Mesh *mesh);
	void 			AssignMtl(WkNodeList* wkNode);
	int 			GetMatNum(char *name);
	void 			AddMeshMtl(SMtl *mtl);
	void 			FreeUnusedMtls();
	int				LoadAppData(FILE *stream,DWORD chunkSize);
	void			ParseIKData(INode *node);
	};


int skip_chunk(FILE *stream);
int get_next_chunk(FILE *stream,Chunk_hdr *hdr);
int SkipRead(FILE *stream,long bytes);
int read_string(char *string,FILE *stream,int maxsize);
int load_app_data(FILE *stream,void **pdata, int size);

#define RDERR(ptr,count) { if(!fread(ptr,count,1,stream)) return 0; }
#define RD3FLOAT(p) RDERR(p,3*sizeof(FLOAT))
#define RDFLOAT(p) 	RDERR(p,sizeof(FLOAT))
#define RDLONG(p) RDERR(p,sizeof(LONG))
#define RDSHORT(p) RDERR(p,sizeof(SHORT))
#define DUMNUM 0x7fff
