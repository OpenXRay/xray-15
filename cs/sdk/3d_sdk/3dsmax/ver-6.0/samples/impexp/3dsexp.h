
/**********************************************************************
 *<
	FILE: 3dsexp.h

	DESCRIPTION:  .3DS file export module header file

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
#include "3dsshape.h"	// 3DS shape file header
#include "kfio.h"		// 3DS KF header
#pragma pack()


// Some 3DS structures

#pragma pack(1)
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
#pragma pack()

// 3DS face edge vis flags
#define ABLINE (1<<2)
#define BCLINE (1<<1)
#define CALINE 1

#define VWRAP (1<<11)		/* Texture coord V wraps on this face */
#define UWRAP (1<<3)		/* Texture coord U wraps on this face */

// Node list structure

//struct WkObjList;
typedef struct {
	TriObject *object;
	TCHAR name[11];
	int used;
	void *next;
	} WkObjList;

//struct WkNodeList;
typedef struct {
	ImpNode *node;
	short id;
	TCHAR name[11];
	Mesh *mesh;
	ImpNode *parent;
	Matrix3 tm;
	void *next;
	} WkNodeList;

// 3DS Key structures

#pragma pack(1)

struct fc_wrt
{
unsigned short a;
unsigned short b;
unsigned short c;
unsigned short flags;
} Fc_wrt;

struct color_24
{
uchar r;
uchar g;
uchar b;
};
typedef struct color_24 Color_24;

struct color_f
{
float r;
float g;
float b;
};
typedef struct color_f Color_f;

// key types
#define KEY_FLOAT	0
#define KEY_POS		1
#define KEY_ROT		2
#define KEY_SCL		3
#define KEY_COLOR	4

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

typedef struct {
	KEYHDR
	FLOAT c[3];
	} ColorKey;
#endif // LATER
#pragma pack()

typedef struct {
	union {
		Key key;
		PosKey pos;
		RotKey rot;
		} key;
	void *next;
	} KeyList;

class SceneEntry {
	public:
		TSTR name;
		INode *node,*tnode;
		Object *obj;
		int type;		// See above
		int id;
		SceneEntry *next;
		SceneEntry(INode *n, Object *o, int t);
		void SetID(int id) { this->id = id; }
};

// We need to maintain a list of the unique objects in the scene
class ObjectEntry {
	public:
		TriObject *tri;
		SceneEntry *entry;
		ObjectEntry *next;
		ObjectEntry(SceneEntry *e) { entry = e; next = NULL;  tri = NULL; }
};

#define ERROR_MSG_MAX_LEN   128
#define NeedsKeys(nkeys)    ((nkeys) > 0 || (nkeys) == NOT_KEYFRAMEABLE)
#define KEYS_POS	        (1<<1)
#define KEYS_ROT	        (1<<2)
#define KEYS_SCL	        (1<<3)

static Tab<TSTR*> msgList;

//gdf TRUE if generating extra 3DS verts for mismatched UV coords
static BOOL MaxUVs;

struct Vert3ds {
    Point3 pt;
    UVVert tv;
};

struct Face3ds {
    int vNum[3];
    int flags;
};

void   ConvertTo3DSFaces(ObjectEntry* oe, Tab<Vert3ds>& verts, Tab<Face3ds>& faces);
static BOOL WriteController(INode* node);
static Point3 GetPivotOffset(INode* node);
inline BOOL ApproxEqual(float a, float b);
static BOOL IsTCBControl(Control *cont);
static BOOL UndoParentsOffset(INode* node, Point3& pt, Quat& rOff);
static Matrix3 GetLocalNodeTM(INode* node, TimeValue t);
void WriteChunkBegin(unsigned short cTag, long& cPtr, long& cBegin);
void WriteChunkEnd(long& cPtr, long& cBegin);
static BOOL WriteTCBKeysChunk(INode* node, Control* cont, int type);
static BOOL WriteLinearKeysChunk(INode* node, int type);
void ConvertTo3DSTVerts(ObjectEntry* oe, Tab<UVVert>& tv);
INT_PTR CALLBACK MsgListDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
void AddToMsgList(Tab<TSTR*>& mList, TCHAR* msg);
