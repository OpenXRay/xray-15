/**********************************************************************
 *<
	FILE: 3dsexp.cpp

	DESCRIPTION:  .3DS file export module

	CREATED BY: Tom Hudson

	HISTORY: created 1 December 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include "3dseres.h"
#include "3dsexp.h"
#include "istdplug.h"
#include "mtldef.h"
#include "stdmat.h"
#include "decomp.h"
#include "helpsys.h"
#include "buildver.h"


// following adjusts for historical artifact
#define VERTICES_FLIP_X

static FILE *wstream;

static BOOL showPrompts;
static BOOL exportSelected;

BOOL operator==(const Matrix3 &m1, const Matrix3 &m2) 
	{
	for (int i=0; i<4; i++) {
		if (m1.GetRow(i) != m2.GetRow(i)) return FALSE;
		}
	return TRUE;
	}

static Point3 colorVal;
static float floatVal;
short Get3DSTVerts(Mesh& mesh, Point2 *tv);

#define EXPORT_MTLS

#pragma pack(1)

struct LocShad2 {
	float bias,shadfilter;
	short shadsize;
	};

struct Camera3DS {
	float x;
	float y;
	float z;
	float tx;
	float ty;
	float tz;
	float bank;
	float focal;
	WORD flags;
	float nearplane;
	float farplane;
	void *appdata;
	};
#pragma pack()

#define PRIMARY_NODE	(1<<14)

#define RDERR(ptr,sz) if (fread((void *)ptr,1,sz,stream)!=(sz)) return(0)
#define WRTERR(ptr,sz) if (fwrite((void *)ptr,1,sz,stream)!=(sz)) return(0)
#define GREAD(ptr,sz) ((fread((void *)ptr,1,sz,stream)!=(sz)) ? 0:1)
#define GWRITE(ptr,sz) ((fwrite((void *)ptr,1,sz,stream)!=(sz)) ? 0:1)

#include <stdarg.h>

#if 0 
void DebugPrint(const TCHAR *format, ...) {
	TCHAR buf[512];
	va_list args;
	va_start(args,format);
	_vsntprintf(buf,512,format,args);
	va_end(args);
	OutputDebugString(buf);
	}
#endif

HINSTANCE hInstance;

TCHAR* GetString(int id)
{
    static TCHAR stBuf[ERROR_MSG_MAX_LEN];
	if (hInstance)
		return LoadString(hInstance, id, stBuf, ERROR_MSG_MAX_LEN) ? stBuf : NULL;
	return NULL;
}


static void MessageBox(int s1, int s2) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	MessageBox(GetActiveWindow(), str1.data(), str2.data(), MB_OK);
	}

static int MessageBox(int s1, int s2, int option = MB_OK) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}

static int Alert(int s1, int s2 = IDS_TH_3DSEXP, int option = MB_OK) {
	return MessageBox(s1, s2, option);
	}

#define LAYERS_BY_OBJECT 0
#define LAYERS_BY_MATERIAL 1
#define LAYERS_ONE_LAYER 2

#define NUM_SOURCES 3 		// # of sources in dialog

class _3DSExport : public SceneExport {
	friend INT_PTR CALLBACK ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
					_3DSExport();
					~_3DSExport();
	int				ExtCount();					// Number of extensions supported
    BOOL            MaxUVs;                     // TRUE if generating extra verts for mismatched UV coords
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options);	// Export file
	BOOL			SupportsOptions(int ext, DWORD options);
	};

// Statics

// Handy file class

class WorkFile {
private:
	FILE *stream;
	
public:
					WorkFile(const TCHAR *filename,const TCHAR *mode) { stream = NULL; Open(filename, mode); };
					~WorkFile() { Close(); };
	FILE *			Stream() { return stream; };
	int				Close() { int result=0; if(stream) result=fclose(stream); stream = NULL; return result; }
	void			Open(const TCHAR *filename,const TCHAR *mode) { Close(); stream = _tfopen(filename,mode); }
	};

// Handy memory worker

class Memory {
	void *ptr;
public:
					Memory() { ptr = NULL; }
					Memory(int amount, BOOL zero = FALSE) { ptr = NULL; Alloc(amount, zero); }
					~Memory() { Free(); }
	void *			Ptr() { return ptr; }
	void *			Realloc(int amount);
	void *			Alloc(int amount, BOOL zero = FALSE);
	void			Free() { if(ptr) free(ptr); ptr = NULL; }
	};

void *Memory::Realloc(int amount) {
	if(ptr)
		ptr = realloc(ptr, amount);
	else
		ptr = malloc(amount);
	return ptr;
	}

void *Memory::Alloc(int amount, BOOL zero) {
	Free();
	ptr = malloc(amount);
	if(ptr && zero) {
		char *p = (char *)ptr;
		for(int i = 0; i < amount; ++i)
			*p++ = 0;
		}
	return ptr;
	}

// Jaguar interface code

int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if ( !controlsInit ) {
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();
		}
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,_T("3DSIMP.DLL: DllMain"),_T("3DSIMP"),MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------

class _3DSClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new _3DSExport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_3DSTUDIO); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0xd1d,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEEXPORT);  }
	};

static _3DSClassDesc _3DSDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_3DSEXPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &_3DSDesc; break;
		default: return 0; break;
		}
	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

//
// ._3DS export module functions follow:
//

_3DSExport::_3DSExport() {
	}

_3DSExport::~_3DSExport() {
	}

int
_3DSExport::ExtCount() {
	return 1;
	}

const TCHAR *
_3DSExport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("3DS");
		}
	return _T("");
	}

const TCHAR *
_3DSExport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_3DSSCENEFILE);
	}
	
const TCHAR *
_3DSExport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_3DSFILE);
	}

const TCHAR *
_3DSExport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
_3DSExport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
_3DSExport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
_3DSExport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
_3DSExport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 200;
	}

void
_3DSExport::ShowAbout(HWND hWnd) {			// Optional
 	}

static INT_PTR CALLBACK
ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static _3DSExport* exp;

	switch(message) {
		case WM_INITDIALOG:
			SetWindowContextHelpId(hDlg, idh_3dsexp_export);
            exp = (_3DSExport*) lParam;
			CenterWindow(hDlg,GetParent(hDlg));
			SetFocus(hDlg); // For some reason this was necessary.  DS-3/4/96
            CheckDlgButton(hDlg, IDC_MAX_UVS, TRUE);
			return FALSE;
		case WM_DESTROY:
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
                    MaxUVs = IsDlgButtonChecked(hDlg, IDC_MAX_UVS);
					EndDialog(hDlg, 1);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_CONTEXTHELP) {
				DoHelp(HELP_CONTEXT, idh_3dsexp_export);
				return FALSE;
			}
		}
	return FALSE;
}

#define OBTYPE_MESH 0
#define OBTYPE_CAMERA 1
#define OBTYPE_OMNILIGHT 2
#define OBTYPE_SPOTLIGHT 3
#define OBTYPE_DUMMY 5
#define OBTYPE_CTARGET 6
#define OBTYPE_LTARGET 7

SceneEntry::SceneEntry(INode *n, Object *o, int t) { 
	node = n; obj = o; type = t; next = NULL; 
	tnode = n->GetTarget();
	}


//- Material Export -------------------------------------------------------------

struct MEntry { SMtl *sm; Mtl *m; };

class MeshMtlList: public Tab<MEntry> {
	public:
	void AddMtl(Mtl *m);
	void ReallyAddMtl(Mtl *m);
	int FindMtl(Mtl *m);
	int FindSName(char *nam);
	~MeshMtlList() {
		for (int i=0; i<Count(); i++) {
			FreeMatRefs((*this)[i].sm);
			delete (*this)[i].sm;
			}
		}
	};

int MeshMtlList::FindMtl(Mtl *m) {
	for (int i=0; i<Count(); i++) 
		if  ( (*this)[i].m == m ) return i;
	return -1;
	}

static int IsStdMulti(Mtl *m) {
	return (m->ClassID()==Class_ID(MULTI_CLASS_ID,0))?1:0; 
	}

int  MeshMtlList::FindSName(char *name) {
	for (int i=0; i<Count(); i++) 
		if  ( strcmp(name,(*this)[i].sm->name)==0) return i;
	return -1;
	}

void MeshMtlList::AddMtl(Mtl *m) {
	if (m==NULL) return;
	Interval v;
	m->Update(0,v);
	if (IsStdMulti(m)) {
		for (int i=0; i<m->NumSubMtls(); i++) {
			Mtl *sub = m->GetSubMtl(i);
			if (sub&&FindMtl(sub)<0) 
				ReallyAddMtl(sub);
			}
		}
	else {
		if (FindMtl(m)<0) 
			ReallyAddMtl(m);
		}
	}

inline Color_24 C24(Color c) {
	Color_24 a;
	a.r = (int)(255.0f*c.r);
	a.g = (int)(255.0f*c.g);
	a.b = (int)(255.0f*c.b);
	return a;
	}

static int MAXMapIndex(int i) {
	switch(i) {
		case Ntex:  return ID_DI;
		case Ntex2: return ID_DI;
		case Nopac: return ID_OP;
		case Nbump: return ID_BU;
		case Nspec: return ID_SP;
		case Nshin: return ID_SH;
		case Nselfi:return ID_SI; 
		case Nrefl: return ID_RL;
		default:    return ID_DI;
		}
	}

int Pcnt(float f) { return (int)(f*100.0f+.5f); }

//gdf convert to 8.3 filename
#define OFFSET(e) ((e < 8 && e != -1) ? e : 8)
static void
EightDotThreeName(CStr& name) {
    CStr fExt;
    CStr fName;
    int  oExt;
    if (name.Length() > 12) {
        oExt = name.last('.');
        fName.Resize(OFFSET(oExt));
        strncpy(fName.data(), name.data(), OFFSET(oExt));
        if (oExt != -1) {
            fExt.Resize(4);
            strncpy(fExt.data(), name.data() + oExt, 4);
        }
        fName.Append(fExt);
	    TCHAR buf[ERROR_MSG_MAX_LEN];
        sprintf(buf, GetString(IDS_EIGHT_DOT_THREE), name.data(), fName.data());
		AddToMsgList(msgList, buf);
        name.Resize(fName.Length() > 12 ? 12 : fName.Length());
        strcpy(name.data(), fName.data());
    }
}
                    

static void ConvertMaxMtlToSMtl(SMtl *s, Mtl *m) {
	Mapping *mp;
	Interval v;
	m->Update(0,v);
	s->amb = C24(m->GetAmbient());
	s->diff = C24(m->GetDiffuse());
	s->spec = C24(m->GetSpecular());
	s->shading = 3;
	s->shininess = Pcnt(m->GetShininess());
	s->shin2pct = Pcnt(m->GetShinStr());
	s->shin3pct = 0;
	s->transparency = Pcnt(m->GetXParency());
	s->wiresize = m->WireSize();
	if (m->ClassID()==Class_ID(DMTL_CLASS_ID,0)) {
		StdMat* std = (StdMat *)m;
		switch(std->GetShading()) {
			case SHADE_CONST: s->shading = 1; break;
			case SHADE_PHONG: s->shading = 3; break;
			case SHADE_METAL: s->shading = 4; break;
			}	
		s->xpfall = Pcnt(std->GetOpacFalloff(0));		
		s->selfipct = Pcnt(std->GetSelfIllum(0));		
		if (std->GetTwoSided()) s->flags |= MF_TWOSIDE;
		if (std->GetWire()) s->flags |= MF_WIRE;
		if (!std->GetFalloffOut()) s->flags |= MF_XPFALLIN;
		if (std->GetFaceMap()) s->flags |= MF_FACEMAP;
		if (std->GetSoften()) s->flags |= MF_PHONGSOFT;
		if (std->GetWireUnits()) s->flags |= MF_WIREABS;
		switch (std->GetTransparencyType()) {
			case TRANSP_FILTER:
			case TRANSP_SUBTRACTIVE: break;
			case TRANSP_ADDITIVE: s->flags |= MF_ADDITIVE;
			}

		for (int i=0; i<8; i++) {
			if (i==Ntex2) continue;
			int n = MAXMapIndex(i);
			Texmap *tx;
			if ((tx=std->GetSubTexmap(n))==NULL) 
				continue;
			if (i==Nrefl) {
#ifndef NO_MAPTYPE_REFLECTREFRACT // orb 01-07-2001
				if (tx->ClassID()==Class_ID(ACUBIC_CLASS_ID,0)) {
					StdCubic *stdcub = (StdCubic*)tx;
					s->map[i] = mp = NewMapping(i,0);
					mp->amt.pct = Pcnt(std->GetTexmapAmt(n,0));			
					mp->use = std->MapEnabled(n);
					AutoCubicParams &ac = mp->map.p.ref.acb;
					ac.flags=AC_ON;
					if (!stdcub->GetDoNth()) ac.flags |= AC_FIRSTONLY;
					ac.nth = stdcub->GetNth();
					ac.size = stdcub->GetSize(0);
					continue;
					}
				else 
#endif // NO_MAPTYPE_REFLECTREFRACT
#ifndef NO_MAPTYPE_FLATMIRROR // orb 01-07-2001
				if (tx->ClassID()==Class_ID(MIRROR_CLASS_ID,0)) {
					StdMirror *stdmir = (StdMirror*)tx;
					s->map[i] = mp = NewMapping(i,0);
					mp->amt.pct = Pcnt(std->GetTexmapAmt(n,0));			
					mp->use = std->MapEnabled(n);
					AutoCubicParams &ac = mp->map.p.ref.acb;
					if (!stdmir->GetDoNth()) ac.flags |= AC_FIRSTONLY;
					ac.nth = stdmir->GetNth();
					ac.flags= AC_MIRROR|AC_ON;
					continue;
					}
#endif // NO_MAPTYPE_FLATMIRROR
				}
			// just do bitmap textures for now
			if (tx->ClassID()!=Class_ID(BMTEX_CLASS_ID,0))
				continue;					
			BitmapTex *bmt = (BitmapTex*)tx;
			s->map[i] = mp = NewMapping(i,0);
			mp->amt.pct = Pcnt(std->GetTexmapAmt(n,0));			
			if (i==Nbump) 
				mp->amt.pct = abs(mp->amt.pct)/100;
			mp->use = std->MapEnabled(n);
			TSTR filename;
			SplitPathFile(TSTR(bmt->GetMapName()),NULL,&filename);
			CStr name(filename);
            
         // gdf convert bitmap filename to 8.3
            EightDotThreeName(name);

         // gdf convert to Uppercase
            name.toUpper();

			strncpy(mp->map.name,name.data(),12);
			if (i==Nrefl) {

				}
			else {
				StdUVGen *uv = bmt->GetUVGen();
				MapParams &par = mp->map.p.tex;
				par.type = MAP_TYPE_UV;
				par.uoffset = uv->GetUOffs(0); 
				par.voffset = -uv->GetVOffs(0); 
				par.uscale = uv->GetUScl(0); 
				par.vscale = uv->GetVScl(0); 
				float ang = uv->GetAng(0);
				par.ang_sin = -(float)sin(ang);
				par.ang_cos = (float)cos(ang);
				par.texblur	 = uv->GetBlur(0)-1.0f;
				int tile = uv->GetTextureTiling();
				if (tile&(U_MIRROR|V_MIRROR))
					par.texflags|= TEX_MIRROR;
				if (tile&(U_WRAP|V_WRAP))
					par.texflags &= ~TEX_NOWRAP;
				else
					par.texflags |= TEX_NOWRAP;
				TextureOutput* texout = bmt->GetTexout();
				if (texout->GetInvert()) 
					par.texflags |= TEX_INVERT;
				if (bmt->GetFilterType()==FILTER_SAT)
					par.texflags |= TEX_SAT;
				}
			}
		}

	}

static void IncrName(char *name, char *inm, int n) {
	char buf[20];
	strncpy(buf,name,13);
	buf[13] = 0;
	sprintf(inm,"%s%d\0",buf,n);
	}

void MeshMtlList::ReallyAddMtl(Mtl *m) {
	MEntry me;
	me.sm = NULL;
	me.m = m;
	SMtl *s = new SMtl;
	memset(s,0,sizeof(SMtl));
	me.sm = s;
	char buf[20];
	strncpy(buf,me.m->GetName(),16);
	if (strlen(buf)==0) strcpy(buf, "Matl");
	buf[16] = 0;
	strcpy(s->name,buf);
	int n=0;
	while (FindSName(s->name)>=0) 
		IncrName(buf,s->name,++n); 
	buf[16] = 0;
	ConvertMaxMtlToSMtl(s,m);
	Append(1,&me,20);
	}								

//--------------------------------------------------------------

class SceneEnumProc : public ITreeEnumProc {
	public:
		Interface	*i;
		SceneEntry *head;
		SceneEntry *tail;
		IScene		*theScene;
		int			count;
		MeshMtlList *mtlList;
		TimeValue	time;
					SceneEnumProc(IScene *scene, TimeValue t, Interface *i, MeshMtlList *ml);
					~SceneEnumProc();
		int			Count() { return count; }
		void		Append(INode *node, Object *obj, int type);
		int			callback( INode *node );
		Box3		Bound();
		SceneEntry *Find(INode *node);
//		SceneEntry *operator[](int index);
		void BuildNames();
	};

SceneEnumProc::SceneEnumProc(IScene *scene, TimeValue t, Interface *i, MeshMtlList *ml) {
	time = t;
	theScene = scene;
	count = 0;
	head = tail = NULL;
	this->i = i;
	mtlList = ml;
	theScene->EnumTree(this);
	}

SceneEnumProc::~SceneEnumProc() {
	while(head) {
		SceneEntry *next = head->next;
		delete head;
		head = next;
		}
	head = tail = NULL;
	count = 0;	
	}

int SceneEnumProc::callback(INode *node) {
	if(exportSelected && node->Selected() == FALSE)
		return TREE_CONTINUE;
	Object *obj = node->EvalWorldState(time).obj;
	if (obj->CanConvertToType(triObjectClassID)) {
		Append(node, obj, OBTYPE_MESH);
		mtlList->AddMtl(node->GetMtl());
		return TREE_CONTINUE;
		}
	if (node->IsTarget()) {
		INode* ln = node->GetLookatNode();
		if (ln) {
			Object *lobj = ln->EvalWorldState(time).obj;
			switch(lobj->SuperClassID()) {
				case LIGHT_CLASS_ID:  Append(node, obj, OBTYPE_LTARGET); break;
				case CAMERA_CLASS_ID: Append(node, obj, OBTYPE_CTARGET); break;
				}
			}
		return TREE_CONTINUE;
		}
	switch (obj->SuperClassID()) { 
		case HELPER_CLASS_ID:
			if ( obj->ClassID()==Class_ID(DUMMY_CLASS_ID,0)) 
				Append(node, obj, OBTYPE_DUMMY);
			break;
		case LIGHT_CLASS_ID: {
            TCHAR buf[ERROR_MSG_MAX_LEN];
			if (obj->ClassID()==Class_ID(OMNI_LIGHT_CLASS_ID,0))
				Append(node, obj, OBTYPE_OMNILIGHT);
			if (obj->ClassID()==Class_ID(SPOT_LIGHT_CLASS_ID,0)) 
				Append(node, obj, OBTYPE_SPOTLIGHT);
            if (obj->ClassID()==Class_ID(DIR_LIGHT_CLASS_ID,0)) {
                sprintf(buf, GetString(IDS_DIR_LIGHT), node->GetName());
		        AddToMsgList(msgList, buf);
				//Append(node, obj, OBTYPE_SPOTLIGHT);
            }
            if (obj->ClassID()==Class_ID(FSPOT_LIGHT_CLASS_ID,0)) {
                sprintf(buf, GetString(IDS_FSPOT_LIGHT), node->GetName());
		        AddToMsgList(msgList, buf);
				//Append(node, obj, OBTYPE_SPOTLIGHT);
            }
            break;
        }
		case CAMERA_CLASS_ID:
			if (obj->ClassID()==Class_ID(LOOKAT_CAM_CLASS_ID,0))
				Append(node, obj, OBTYPE_CAMERA);
			break;
		}
	return TREE_CONTINUE;	// Keep on enumeratin'!
	}


void SceneEnumProc::Append(INode *node, Object *obj, int type) {
	SceneEntry *entry = new SceneEntry(node, obj, type);

	if(tail)
		tail->next = entry;
	tail = entry;
	if(!head)
		head = entry;
	count++;	
	}

Box3 SceneEnumProc::Bound() {
	Box3 bound;
	bound.Init();
	SceneEntry *e = head;
	ViewExp *vpt = i->GetViewport(NULL);
	while(e) {
		Box3 bb;
		e->obj->GetWorldBoundBox(time, e->node, vpt, bb);
		bound += bb;
		e = e->next;
		}
	return bound;
	}

SceneEntry *SceneEnumProc::Find(INode *node) {
	SceneEntry *e = head;
	while(e) {
		if(e->node == node)
			return e;
		e = e->next;
		}
	return NULL;
	}



SceneEnumProc *theSceneEnum = NULL;
MeshMtlList *theMtls = NULL;

class ObjectList {
	public:
		ObjectEntry *head;
		ObjectEntry *tail;
		int			count;
					ObjectList(SceneEnumProc &scene);
					~ObjectList();
		int			Count() { return count; }
		void		Append(SceneEntry *e);
		ObjectEntry *Contains(Object *obj);
		ObjectEntry *Contains(INode *node);
		ObjectEntry *FindLookatNode(INode *node);
	};

ObjectList::ObjectList(SceneEnumProc &scene) {
	head = tail = NULL;
	count = 0;
	// Zip thru the object list and record all unique objects (Some may be used by more than one node)
	int scount = scene.Count();
	for(SceneEntry *se = scene.head; se!=NULL; se = se->next) {
		// can't multiple instance lights and cameras in 3DS  
		// so make them all unique--DS 4/6/96
		if ( (se->type!=OBTYPE_MESH)|| !Contains(se->obj))
			Append(se);
		}
	}

ObjectList::~ObjectList() {
	while(head) {
		ObjectEntry *next = head->next;
		delete head;
		head = next;
		}
	head = tail = NULL;
	count = 0;	
	}

ObjectEntry *ObjectList::Contains(Object *obj) {
	ObjectEntry *e;
	for (e=head; e!=NULL; e = e->next) {
		if(e->entry->obj == obj)
			return e;
		}
	return NULL;
	}

class FindDepNodeEnum: public DependentEnumProc {
	public:
		ReferenceTarget *targ;
		INode *depNode;
		FindDepNodeEnum() { targ = NULL; depNode = NULL; }
		// proc should return 1 when it wants enumeration to halt.
		virtual	int proc(ReferenceMaker *rmaker);
	};

int FindDepNodeEnum::proc(ReferenceMaker *rmaker) {
	if (rmaker->SuperClassID()!=BASENODE_CLASS_ID) return 0;
	INode* node = (INode *)rmaker;
	if (node->GetTarget()==targ) {
		depNode = node;
		return 1;
		}
	return 0;
	}
	
ObjectEntry *ObjectList::FindLookatNode(INode *node) {
	FindDepNodeEnum	 finder;
	ObjectEntry *e;
	for (e=head; e!=NULL; e = e->next) {
		finder.targ = node;
		e->entry->node->EnumDependents(&finder);
		if (finder.depNode) return e;
		}
	return NULL;
	}


ObjectEntry *ObjectList::Contains(INode *node) {
	ObjectEntry *e;
	for (e=head; e!=NULL; e = e->next) {
		if(e->entry->node == node)
			return e;
		}
	return NULL;
	}

void ObjectList::Append(SceneEntry *e) {
	ObjectEntry *entry = new ObjectEntry(e);
	if(tail)
		tail->next = entry;
	tail = entry;
	if(!head)
		head = entry;
	count++;	
	}

ObjectList *theObjects = NULL;

class ObjName {
	public:
		TSTR name;
		ObjName *next;
		ObjName(TSTR n) { name = n; next = NULL; }
	};

class ObjNameList {
	public:
		ObjName *head;
		ObjName *tail;
		int			count;
					ObjNameList() { head = tail = NULL; count = 0; }
					~ObjNameList();
		int			Count() { return count; }
		int			Contains(TSTR &n);
		void		Append(TSTR &n);
		void		MakeUnique(TSTR &n);
	};

ObjNameList::~ObjNameList() {
	while(head) {
		ObjName *next = head->next;
		delete head;
		head = next;
		}
	head = tail = NULL;
	count = 0;	
	}

int ObjNameList::Contains(TSTR &n) {
	ObjName *e = head;
	int index = 0;
	while(e) {
		if(e->name == n)
			return index;
		e = e->next;
		index++;
		}
	return -1;
	}

void ObjNameList::Append(TSTR &n) {
	ObjName *entry = new ObjName(n);
	if(tail)
		tail->next = entry;
	tail = entry;
	if(!head)
		head = entry;
	count++;	
	}

void ObjNameList::MakeUnique(TSTR &n) {
	// First make it less than 10 chars.
	if (n.Length()>10) n.Resize(10);
	
	if(Contains(n) < 0) {
		Append(n);
		return;
		}
	// Make it unique and keep it 10 chars or less
	for(int i = 0; i < 100000; ++i) {
		char buf[12];
		sprintf(buf,"%d",i);
		TSTR num(buf);
		TSTR work = n;
		int totlen = num.Length() + work.Length();
		if(totlen > 10)
			work.Resize(10 - (totlen - 10));
		work = work + num;
		if(Contains(work) < 0) {
			Append(work);
			n = work;
			return;
			}
		}
	// Forget it!
	}

ObjNameList theObjNames;
Tab<int> mtlNumMap;
int mtlnum;
BOOL multiMtl;


// RB:
// Here's what I've done to handle names. After the scene has been
// traversed but before it is actually written to disk, I make
// a unique <10 character name for evey node name. This name is
// then used in place of the node name.
void SceneEnumProc::BuildNames()
	{
	ObjNameList nameList;
	SceneEntry *ptr = head;

	while (ptr) {
//		if (ptr->node->IsTarget()) {
//			ptr->name = _T("");
//			}
//		else {
			ptr->name = ptr->node->GetName();
			nameList.MakeUnique(ptr->name);		
//			}
		ptr = ptr->next;
		}
	}



typedef struct {
	Matrix3 *tm;
	Mesh *mesh;
	INode *node;
	} MeshAndTMAndNode;

static short wrapFlags;

/* Main routine for writing a mesh file!		*/
/* Recursive chunk writer -- keeps track of each chunk	*/
int
dump_mchunk(USHORT tag,FILE *stream,void *data) {
	USHORT count,temp;
	int ox,ix;
	int *intptr;
	long chunkptr,chunkbase,curpos,chunksize,lval;
	SceneEntry *se = (SceneEntry *)data;
	Interval valid;
	Color_f *cf;
	ObjectEntry *oe;
	INode *node;
	Object *obj;
	Mesh *m;

//	DebugPrint(_T("dump_mchunk: tag = %X\n"),tag); 

	chunkbase=ftell(stream);
	WRTERR(&tag,2);
	chunkptr=ftell(stream);		/* Save file ptr for chunk size */
	WRTERR(&chunkptr,4);
	switch(tag)
		{
		case MMAGIC:	/* Mesh editor */
			if(dump_mchunk(MESH_VERSION,stream,NULL)==0)
				return(0);

#ifdef EXPORT_MTLS
			// Save out the Materials...
            for (ix=0; ix<(*theMtls).Count(); ix++)  {
                if (ix == 256) {
                    TCHAR buf[ERROR_MSG_MAX_LEN];
                    sprintf(buf, GetString(IDS_MATERIAL_LIMIT));
		            AddToMsgList(msgList, buf);
                    (*theMtls).SetCount(256);
                    break;
                }
				savemtl= (*theMtls)[ix].sm;
				if(dump_mtlchunk(MAT_ENTRY,stream,NULL)==0)
					return 0;
			}
#endif
			if(dump_mchunk(MASTER_SCALE,stream,NULL)==0)
				return(0);

//			if(dump_mchunk(O_CONSTS,stream,&const_x)==0)
//				return(0);
//			if(dump_mchunk(AMBIENT_LIGHT,stream,NULL)==0)
//				return(0);
//			if(dump_mchunk(DISTANCE_CUE,stream,NULL)==0)
//				return(0);
//			if(ENVFLAG==FOG) {
//				if(dump_mchunk(USE_FOG,stream,NULL)==0)
//				return(0);
//				}
//			else
//			if(ENVFLAG==DISTANCE_CUE) {
//				if(dump_mchunk(USE_DISTANCE_CUE,stream,NULL)==0)
//					return(0);
//				}
//			else
//			if(ENVFLAG==LAYER_FOG) {
//				if(dump_mchunk(USE_LAYER_FOG,stream,NULL)==0)
//					return(0);
//				}
//			if(default_view!=CAMERA && default_view!=SPOTLIGHT) {
//				if(dump_mchunk(DEFAULT_VIEW,stream,NULL)==0)
//					return(0);
//				}
			for (ox = 0,oe = theObjects->head; oe != NULL; oe = oe->next,ox++) {
				int type = oe->entry->type;
				if (type!=OBTYPE_DUMMY&&type!=OBTYPE_LTARGET&&type!=OBTYPE_CTARGET) {
					if (type==OBTYPE_MESH) {
						obj = oe->entry->obj;
						oe->tri = (TriObject *)obj->ConvertToType(theSceneEnum->i->GetTime(), triObjectClassID);
						Mesh &mesh = oe->tri->GetMesh();
                        if (mesh.numFaces > 65530) {
							TCHAR buf[ERROR_MSG_MAX_LEN];
                            sprintf(buf, GetString(IDS_OBJ_TOO_MANY_FACES), oe->entry->name);
							AddToMsgList(msgList, buf);
							continue;
						}
						if (mesh.numVerts > 65530) {
							TCHAR buf[ERROR_MSG_MAX_LEN];
                            sprintf(buf,GetString(IDS_OBJ_TOO_MANY_VERTS), oe->entry->name);
							AddToMsgList(msgList, buf);
							continue;
						}
                    }
					int status = dump_mchunk(NAMED_OBJECT,stream,oe);
					if (!status) return 0;
				}
			}
//			if (MSHappdata!=NULL) 
//				if(dump_mchunk(APP_DATA,stream,MSHappdata)==0)
//					return(0);
			break;
		case MESH_VERSION:
			lval=0x00000003; /* Current version Number */
			WRTERR(&lval,4);
			break;
		case LIN_COLOR_F:
		case COLOR_F:
			cf=(Color_f *)data;
			WRTERR(cf,12);
			break;
		case MASTER_SCALE: {
			int type;
			float scale;
			GetMasterUnitInfo(&type, &scale);
			float msc_factor = (float)GetMasterScale(type);
			WRTERR(&msc_factor,sizeof(float));
			}
			break;
//		case SOLID_BGND:
//			if (P.gamma_correct) {
//				Color gamcol;
//				disp_gammify(&gamcol, &BG.bkgd_solid);
//				if(dump_mchunk(COLOR_F,stream,&gamcol)==0) return(0);
//				if(dump_mchunk(LIN_COLOR_F,stream,&BG.bkgd_solid)==0) return(0);
//				}
//	  		else 
//			if(dump_mchunk(COLOR_F,stream,&BG.bkgd_solid)==0) return(0);
//	 		break;
		case RAY_BIAS:	/* Simple floats */
		case LO_SHADOW_BIAS:	/* Simple floats */
		case SHADOW_FILTER:
		case DL_OUTER_RANGE:
		case DL_INNER_RANGE:
		case DL_MULTIPLIER:
			WRTERR(data,sizeof(float));
			break;
	 	case SHADOW_SAMPLES:	/* Simple ints, written as shorts */
	 	case SHADOW_RANGE:
	 	case SHADOW_MAP_SIZE:
			intptr=(int *)data;
			temp=(USHORT) *intptr;
			WRTERR(&temp,2);
			break;
//		case BIT_MAP:
//			string=(char *)data;
//			split_fn(NULL,gp_buffer,string);
//			WRTERR(gp_buffer,strlen(gp_buffer)+1);
//			break;

		/* Simple strings */

//		case DL_EXCLUDE:
//		case VIEW_CAMERA:
//			string=(char *)data;
//			WRTERR(string,strlen(string)+1);
//			break;
//		case O_CONSTS:
//			WRTERR(data,sizeof(float)*3);
//			break;
//		case AMBIENT_LIGHT:
//			unload_ambient_light(0);
//			if(dump_mchunk(COLOR_F,stream,&BG.amb_light)==0)
//				return(0);
//			break;
		case NAMED_OBJECT: {
			oe=(ObjectEntry *)data;
			node = oe->entry->node;			
			//CStr name(node->GetName());
			CStr name(oe->entry->name); // RB
			if(name.Length()>10)
				name.Resize(10);
			WRTERR(name.data(),(size_t)(name.Length()+1));
			switch(oe->entry->type) {
				case OBTYPE_MESH:
					if(dump_mchunk(N_TRI_OBJECT,stream,oe)==0)
						return(0);
					break;
				case OBTYPE_CAMERA:
					if(dump_mchunk(N_CAMERA,stream,oe)==0)
						return(0);
					break;
				case OBTYPE_OMNILIGHT:
					if(dump_mchunk(N_DIRECT_LIGHT,stream,oe)==0)
						return(0);
					break;
				case OBTYPE_SPOTLIGHT:
					if(dump_mchunk(N_DIRECT_LIGHT,stream,oe)==0)
						return(0);
					break;
				default:
					assert(0);
					break;
				}
//			if(n->flags & NO_HIDDEN) {
//				if(dump_mchunk(OBJ_HIDDEN,stream,n)==0)
//					return(0);
//				}     
//			if(n->flags & NO_DOESNT_CAST) {
//				if(dump_mchunk(OBJ_DOESNT_CAST,stream,n)==0)
//					return(0);
//				}     
//	 		if(n->flags & NO_MATTE) {
//				if(dump_mchunk(OBJ_MATTE,stream,n)==0)
//					return(0);
//				}     
//			if(n->flags & NO_DONT_RCVSHADOW) {
//				if(dump_mchunk(OBJ_DONT_RCVSHADOW,stream,n)==0)
//					return(0);
//				}     
//			if(n->flags & NO_FAST) {
//				if(dump_mchunk(OBJ_FAST,stream,n)==0)
//					return(0);
//				}
//			if(n->flags & NO_FROZEN) {
//				if(dump_mchunk(OBJ_FROZEN,stream,n)==0)
//					return(0);
//				}
			}
			break;
		case OBJ_HIDDEN:	/* Special null chunk */
//		case OBJ_VIS_LOFTER:	/* Special null chunk */
		case OBJ_DOESNT_CAST:	/* Special null chunk */
		case OBJ_DONT_RCVSHADOW: /* Special null chunk */
		case OBJ_MATTE:	/* Special null chunk */
//		case OBJ_PROCEDURAL:	/* Special null chunk */
		case OBJ_FAST:		/* Special null chunk */
		case OBJ_FROZEN:		/* Special null chunk */
			break;
		case APP_DATA: {
			ULONG *plong = (ULONG *)data;
			WRTERR(&plong[1],plong[0]);
			}
	  		break;
		case N_TRI_OBJECT: {
			oe = (ObjectEntry *)data;
			node = oe->entry->node;
			Matrix3 ident(TRUE);
            Matrix3 mat = node->GetNodeTM(theSceneEnum->time);
            
			// RB: For some reason this fixes a problem where
			// the pivot gets screwed up if the object is at 
			// the origin. ???!!!			
			Point3 pos = mat.GetRow(3);
			if (pos.x==0.0f && pos.y==0.0f && pos.z==0.0f) {
				pos.x = 0.001f;
				pos.y = 0.001f;
				pos.z = 0.001f;
				mat.SetTrans(pos);
				}
			
			obj = oe->entry->obj;
			Mesh &mesh = oe->tri->GetMesh();
			MeshAndTMAndNode mt;
			mt.tm = &mat;
			mt.mesh = &mesh;
			mt.node = node;

			// RB 6/11/96: This is kindof a hack, but if the node has
			// WSMs applied then it will already be transformed into
			// world space and so it doesn't need to be transformed again.
			if (node->GetProperty(PROPID_HAS_WSM)) {
				mt.tm = &ident;
				}

         //	if( (t->flags & TRI_HAS_PROCDATA) && t->proc_data!=NULL) {
         //     if(dump_mchunk(PROC_DATA,stream,t)==0)
         //		    return(0);
         //	}
         //	if(strlen(t->proc_name)>0) {
         //	    if(dump_mchunk(PROC_NAME,stream,t)==0)
         //		    return(0);
         //	}



         // gdf hack for people who want the additional texture coord data
            if (MaxUVs && (mesh.numTVerts > 0)) {
                Tab<Vert3ds> verts;
                Tab<Face3ds> faces;
                long    cPtr;
                long    cBegin;
                unsigned short cTag;
                Point3  pt;
                UWORD   cnt;

                ConvertTo3DSFaces(oe, verts, faces);

                if(mesh.getNumVerts()) {
					float xxform = 1.0f;
#ifdef VERTICES_FLIP_X
					Point3 cp = CrossProd(mat.GetRow(0),mat.GetRow(1));
					if (DotProd(cp,mat.GetRow(2))<0)
						xxform = -1.0f;
#endif			
                    cTag = POINT_ARRAY;
                    WriteChunkBegin(cTag, cPtr, cBegin);
                    cnt  = (UWORD) verts.Count();
                    fwrite(&cnt, 1, sizeof(UWORD), wstream);
                    for (int i = 0; i < cnt; i++) {
                        pt = verts[i].pt;
						pt[0] *= xxform;
						pt = pt * mat;
                        fwrite(&pt, sizeof(float), 3, wstream);
                    }
                    WriteChunkEnd(cPtr, cBegin);

                    if (mesh.numTVerts > 0 && mesh.tvFace && mesh.tVerts) {
                        cTag = TEX_VERTS;
                        WriteChunkBegin(cTag, cPtr, cBegin);
                        fwrite(&cnt, 1, sizeof(UWORD), wstream);
                        for (int i = 0; i < cnt; i++) {
                            fwrite(&verts[i].tv.x, sizeof(float), 1, wstream);
                            fwrite(&verts[i].tv.y, sizeof(float), 1, wstream);
                        }
                        WriteChunkEnd(cPtr, cBegin);
					}

                    cTag = MESH_MATRIX;
                    WriteChunkBegin(cTag, cPtr, cBegin);
                    fwrite(mat.GetAddr(), sizeof(float), 12, wstream);
                    WriteChunkEnd(cPtr, cBegin);
				}

                if(mesh.getNumFaces()) {
                    BOOL smoothed = FALSE;
                    cnt = mesh.getNumFaces();
                    cTag = FACE_ARRAY;
                    WriteChunkBegin(cTag, cPtr, cBegin);
                    cnt  = (UWORD) faces.Count();
                    fwrite(&cnt, sizeof(UWORD), 1, wstream);
                    for (int i = 0; i < cnt; i++) {
                        Face face = mesh.faces[i];
                        if (face.smGroup)
                            smoothed = TRUE;
                        fwrite(&faces[i].vNum[0], sizeof(UWORD), 1, wstream);
                        fwrite(&faces[i].vNum[1], sizeof(UWORD), 1, wstream);
                        fwrite(&faces[i].vNum[2], sizeof(UWORD), 1, wstream);
						UWORD wrtFlags = 0;
						if(faces[i].flags & EDGE_A) wrtFlags |= ABLINE;
						if(faces[i].flags & EDGE_B) wrtFlags |= BCLINE;
						if(faces[i].flags & EDGE_C) wrtFlags |= CALINE;
                        fwrite(&wrtFlags, sizeof(UWORD), 1, wstream);
                    }
                    Mtl* mtl;
			        if ((mtl = mt.node->GetMtl()) != NULL) {
				        if (IsStdMulti(mtl)) {
					        multiMtl = TRUE;
                    		mtlNumMap.SetCount(mtl->NumSubMtls());
					        for (int i = 0; i < mtl->NumSubMtls(); i++) {
						        Mtl* sub  = mtl->GetSubMtl(i);
						        if (sub)
                                    mtlNumMap[i] = theMtls->FindMtl(sub);
						        else
                                    mtlNumMap[i] = 0;
						    }
					        for (i = 0; i < mtl->NumSubMtls(); i++) {
						        Mtl* sub  = mtl->GetSubMtl(i);
						        if (sub) {
							        mtlnum = mtlNumMap[i];
							        if (!dump_mchunk(MSH_MAT_GROUP, stream, &mt))
								        return 0;
							    }
						    }
					    } else {
					        mtlnum = theMtls->FindMtl(mtl);
					        multiMtl = FALSE;
					        if (!dump_mchunk(MSH_MAT_GROUP, stream, &mt))
						    return 0;
					    }
				    }

					if(smoothed) {
			        	if(!dump_mchunk(SMOOTH_GROUP, stream, &mesh))
					        return 0;
				    }
                    WriteChunkEnd(cPtr, cBegin);
				}
            } else {
			    if(mesh.getNumVerts()) {
				    if(dump_mchunk(POINT_ARRAY,stream,&mt)==0) {
					    error:
					 // Delete the working object, if necessary
					    if(obj != (Object *)(oe->tri)) {
						    oe->tri->DeleteThis();
						    oe->tri = NULL;
					    }
    					return(0);
					}
	    			if (mesh.numTVerts>0 && mesh.tvFace && mesh.tVerts) {
		    			if(dump_mchunk(TEX_VERTS,stream, oe)==0)
			    			goto error;
                     // if(dump_mchunk(MESH_TEXTURE_INFO,stream,t)==0)
                     //     goto error;
					}
                 // if(dump_mchunk(POINT_FLAG_ARRAY,stream,t)==0)
                 //     return(0);
												
				    if(dump_mchunk(MESH_MATRIX,stream,mat.GetAddr())==0)
    					goto error;
                 // if(dump_mchunk(MESH_COLOR,stream,t)==0)
                 //     return(0);
				}
	    		if(mesh.getNumFaces()) {
                 // objNamePtr = oe->entry->name;
			    	if(dump_mchunk(FACE_ARRAY,stream,&mt)==0)
				    	goto error;
				}
             // if(t->appdata!=NULL) {
             //     if(dump_mchunk(APP_DATA,stream,t->appdata)==0)
             //         return(0);
             //	}     
            }

         // Delete the working object, if necessary
			if(obj != (Object *)(oe->tri)) {
				oe->tri->DeleteThis();
				oe->tri = NULL;
				}
			}
			break;

		case POINT_ARRAY: {
            MeshAndTMAndNode *mt = (MeshAndTMAndNode *)data;
			count=mt->mesh->getNumVerts();
			WRTERR(&count,2);

			float xxform = 1.0f;
#ifdef VERTICES_FLIP_X
			Point3 cp = CrossProd(mt->tm->GetRow(0),mt->tm->GetRow(1));
			if (DotProd(cp,mt->tm->GetRow(2))<0)
				xxform = -1.0f;
#endif			
			for(ix=0; ix<count; ++ix) {
				Point3 v = mt->mesh->verts[ix];
				v[0] *= xxform;
				v = v * *mt->tm;
                WRTERR(&v[0],sizeof(float));
				WRTERR(&v[1],sizeof(float));
				WRTERR(&v[2],sizeof(float));
				}
			}
			break;

		case TEX_VERTS: {
            Tab<UVVert> tverts;
			oe = (ObjectEntry *) data;
			Mesh &mesh = oe->tri->GetMesh();
			int nverts = mesh.getNumVerts();
			if (nverts>65535) nverts = 65535;
         // gdf 
            UWORD nv = (UWORD) nverts;
            WRTERR(&nv,2);
            ConvertTo3DSTVerts(oe, tverts);
            for (int i = 0; i < tverts.Count(); i++) {
                UVVert pt = tverts[i];
                WRTERR(&pt,sizeof(float)*2);
            }

            /*
			Point2 *tv = new Point2[nverts];
			wrapFlags = Get3DSTVerts(mesh,tv);
			UWORD nv = (UWORD)nverts;
			WRTERR(&nv,2);
			for(ix=0; ix<nv; ++ix) {
				WRTERR(&tv[ix],sizeof(float)*2);
				}
			delete [] tv;
            */
			}
			break;

//		case MESH_TEXTURE_INFO:
//			t=(Tri_obj *)data;
//			Mapinfo.maptype=t->maptype;
//			Mapinfo.tile_x=t->tile_x;
//			Mapinfo.tile_y=t->tile_y;
//			Mapinfo.map_x=t->map_x;
//			Mapinfo.map_y=t->map_y;
//			Mapinfo.map_z=t->map_z;
//			Mapinfo.map_scale=t->map_scale;
//			memcpy(Mapinfo.map_matrix,t->map_matrix,sizeof(float)*12);
//			Mapinfo.map_pw=t->map_pw;
//			Mapinfo.map_ph=t->map_ph;
//			Mapinfo.map_ch=t->map_ch;
//			WRTERR(&Mapinfo,sizeof(Mapinfo));
//			break;
//		case POINT_FLAG_ARRAY:
//			t=(Tri_obj *)data;
//			count=t->verts;
//			WRTERR(&count,2);
//			for(ix=0; ix<t->verts; ++ix) {
//				get_vert(t,ix,&v);
//				WRTERR(&v.flags,sizeof(short));
//				}
//			break;
		case MESH_MATRIX:
			WRTERR(data,sizeof(float)*12);
			break;
//		case MESH_COLOR:
//			t=(Tri_obj *)data;
//			WRTERR(&t->color,sizeof(uchar));
//			break;
		case FACE_ARRAY: {
			MeshAndTMAndNode *mt = (MeshAndTMAndNode*)data;
			m = mt->mesh;
			count=m->getNumFaces();
			WRTERR(&count,2);
			BOOL anySmooth = FALSE;
			for(ix=0; ix<count; ++ix) {
				Face f = m->faces[ix];
				if(f.smGroup)
					anySmooth = TRUE;
				Fc_wrt.a=(unsigned short)f.v[0];
				Fc_wrt.b=(unsigned short)f.v[1];
				Fc_wrt.c=(unsigned short)f.v[2];
				Fc_wrt.flags = wrapFlags;	 // TEX_VERTS
				if(f.flags & EDGE_A) Fc_wrt.flags |= ABLINE;
				if(f.flags & EDGE_B) Fc_wrt.flags |= BCLINE;
				if(f.flags & EDGE_C) Fc_wrt.flags |= CALINE;
				WRTERR(&Fc_wrt,8);
				}
#ifdef EXPORT_MTLS
			Mtl *mtl;
			if (NULL!=(mtl=mt->node->GetMtl())) {
				if (IsStdMulti(mtl)) {
					multiMtl = TRUE;
				 // make a table that maps sub-mtl number to the proper index
				 // into theMtls.
					mtlNumMap.SetCount(mtl->NumSubMtls());
					for (int i=0; i<mtl->NumSubMtls(); i++) {
						Mtl *sub  = mtl->GetSubMtl(i);
						if (sub) mtlNumMap[i] = theMtls->FindMtl(sub);
						else mtlNumMap[i] = 0;
						}
					for (i=0; i<mtl->NumSubMtls(); i++) {
						Mtl *sub  = mtl->GetSubMtl(i);
						if (sub) {
							mtlnum = mtlNumMap[i];
							if (dump_mchunk(MSH_MAT_GROUP,stream,mt)==0)
								return 0;
							}
						}
					}
				else {
					mtlnum = theMtls->FindMtl(mtl);
					multiMtl = FALSE;
					if (dump_mchunk(MSH_MAT_GROUP,stream,mt)==0)
						return 0;
					}
				}
#endif

			/* Save smoothing groups if any */

			if(anySmooth) {
				if(dump_mchunk(SMOOTH_GROUP,stream,m)==0)
					return(0);
				}
//			if (t->flags&TRI_BOX_MAP) 
//				if(dump_mchunk(MSH_BOXMAP,stream,t)==0)
//					return(0);
			}
			break;
//		case MSH_BOXMAP: {
//			int i;
//			for (i=0; i<6; i++) {
//				if (t->boxmtl[i]==255)
//					WRTERR(sdefault,8);
//				else {
//					if (!mtl_pointer(t->boxmtl[i],&mtl))
//						return(0);
//		   			WRTERR(mtl->name,strlen(mtl->name)+1);
//					}
//				}
//			}
//			break;
#ifdef EXPORT_MTLS
		case MSH_MAT_GROUP:
			{
			MeshAndTMAndNode *mt = (MeshAndTMAndNode*)data;
			m = mt->mesh;
			SMtl *mtl;
			short temp;
			assert(mtlnum>=0 && mtlnum<theMtls->Count());
			mtl = (*theMtls)[mtlnum].sm;
			assert(mtl);

			if (multiMtl) {
                count = 0;
				for (ix=0; ix<m->numFaces; ix++) {
                 // gdf
                    int faceIndex = m->getFaceMtlIndex(ix);

                    int mtlCnt = mtlNumMap.Count(); // this is to prevent compilier warnings
                    faceIndex = (faceIndex % mtlCnt);

                    int mtlIndex = mtlNumMap[faceIndex];
					if (mtlIndex==mtlnum)
							count++;
					}
				}
			else
				count = m->numFaces;

			WRTERR(mtl->name,strlen(mtl->name)+1);
			temp = (short)count;
			WRTERR(&temp,2);

			if (multiMtl) {
				for(ix=0; ix<m->numFaces; ++ix) {
                 // gdf
                    int faceIndex = m->getFaceMtlIndex(ix);

                    int mtlCnt = mtlNumMap.Count(); // this is to prevent compilier warnings
                    faceIndex = (faceIndex % mtlCnt);

					int mtlIndex = mtlNumMap[faceIndex];
					if (mtlIndex==mtlnum) {
						temp = (short)ix;
						WRTERR(&temp,2);
						if(--count==0)
							break;
						}
					}
				}
			else {
				for(ix=0; ix<m->numFaces; ++ix) {
					temp = (short)ix;
					WRTERR(&temp,2);
					}
				}
			}
			break;
#endif
		case SMOOTH_GROUP:
			m=(Mesh *)data;

			/* Now dump all object faces' smoothing group flags */
			count = m->getNumFaces();
			for(ix=0; ix<count; ++ix) {
				Face f = m->faces[ix];
				WRTERR(&f.smGroup,4);
				}
			break;
		case N_DIRECT_LIGHT:{
            oe = (ObjectEntry *)data;
			node = oe->entry->node;
			obj = oe->entry->obj;
			LightObject *lt= (LightObject *)obj;
			GenLight *gl = (GenLight *)obj;
			LightState ls;
			lt->EvalLightState(theSceneEnum->time, valid, &ls);
			Matrix3 mat = node->GetNodeTM(theSceneEnum->time);			
			Point3 pos = mat.GetRow(3);
			WRTERR(&pos.x,sizeof(float));
			WRTERR(&pos.y,sizeof(float));
			WRTERR(&pos.z,sizeof(float));
			if(dump_mchunk(COLOR_F,stream,&ls.color)==0)
				return(0);
			if(dump_mchunk(DL_OUTER_RANGE,stream,&ls.attenEnd)==0)
				return(0);
			if(dump_mchunk(DL_INNER_RANGE,stream,&ls.attenStart)==0)
				return(0);
			if(dump_mchunk(DL_MULTIPLIER,stream,&ls.intens)==0)
				return(0);
#if 0
			if(d->exclude) {
				Object_list *o=d->exclude;
				while(o) {
					if(dump_mchunk(DL_EXCLUDE,stream,o->name)==0)
						return(0);
					o=o->next;
					}
				}
#endif
			if (oe->entry->type==OBTYPE_SPOTLIGHT) {
				if(dump_mchunk(DL_SPOTLIGHT,stream,data)==0)
				return(0);
				}
			if (!ls.on) {
				if(dump_mchunk(DL_OFF,stream,data)==0)
					return(0);   
				}
			if(ls.useAtten) {
				if(dump_mchunk(DL_ATTENUATE,stream,data)==0)
					return(0);   
				}
//			if(d->appdata!=NULL) {
//				if(dump_mchunk(APP_DATA,stream,d->appdata)==0)
//					return(0);
//				} 
			}    
			break;
		case DL_SPOTLIGHT: {
            oe = (ObjectEntry *)data;
			INode *tnode = oe->entry->tnode;
			obj = oe->entry->obj;

            if (obj->ClassID()==Class_ID(DIR_LIGHT_CLASS_ID,0)) {
                TCHAR buf[ERROR_MSG_MAX_LEN];
                sprintf(buf, GetString(IDS_DIR_LIGHT), oe->entry->name);
		        AddToMsgList(msgList, buf);
                break;
            }

            if (obj->ClassID()==Class_ID(FSPOT_LIGHT_CLASS_ID,0)) {
                TCHAR buf[ERROR_MSG_MAX_LEN];
                sprintf(buf, GetString(IDS_FSPOT_LIGHT), oe->entry->name);
		        AddToMsgList(msgList, buf);
                break;
            }
			LightObject *lt= (LightObject *)obj;
			GenLight *gl = (GenLight *)obj;
			LightState ls;
			lt->EvalLightState(theSceneEnum->time, valid, &ls);
			Matrix3 mat = tnode->GetNodeTM(theSceneEnum->time);			
			Point3 pos = mat.GetRow(3);
			WRTERR(&pos.x,sizeof(float));
			WRTERR(&pos.y,sizeof(float));
			WRTERR(&pos.z,sizeof(float));
			WRTERR(&ls.hotsize,sizeof(float));
			WRTERR(&ls.fallsize,sizeof(float));
			if(dump_mchunk(DL_SPOT_ROLL,stream,data)==0)
				return(0);
			if(ls.shadow) {
				if(dump_mchunk(DL_SHADOWED,stream,NULL)==0)
				return(0);
				}
			if(!gl->GetUseGlobal()) {
				if(dump_mchunk(DL_LOCAL_SHADOW2,stream,gl)==0)
					return(0);
				}
			if(gl->GetConeDisplay()) {
				if(dump_mchunk(DL_SEE_CONE,stream,NULL)==0)
				return(0);
				}
//			if(d->flags & NO_LT_PROJ) {
//				if(dump_mchunk(DL_SPOT_PROJECTOR,stream,d)==0)
//				return(0);
//				}
			if(gl->GetSpotShape()==RECT_LIGHT) {
				if(dump_mchunk(DL_SPOT_RECTANGULAR,stream,NULL)==0)
					return(0);
				if(dump_mchunk(DL_SPOT_ASPECT,stream,gl)==0)
				return(0);
				}
			if(ls.overshoot) {
				if(dump_mchunk(DL_SPOT_OVERSHOOT,stream,NULL)==0)
				return(0);
				}
			if (dump_mchunk(DL_RAY_BIAS,stream,gl)==0)
				return(0);
			if (gl->GetShadowType())  {
				if(dump_mchunk(DL_RAYSHAD,stream,data)==0)
				return(0);   
				}
			}
			break;
		case DL_OFF:		/* Special null chunk */
		case DL_ATTENUATE:	/* Special null chunk */
		case DL_SHADOWED:	/* Special null chunk */

			break;

		case DL_SPOT_ROLL:{
			oe = (ObjectEntry *)data;
			INode *node = oe->entry->node;
			Control *tmCont = node->GetTMController();
			Control *rollCont = tmCont->GetRollController();
			float v;
			rollCont->GetValue(0,&v,valid);	
			WRTERR(&v,sizeof(float));
			break;
			}

		case DL_SPOT_ASPECT: {
			GenLight *gl = (GenLight *)data;
			float f = gl->GetAspect(0);
			WRTERR(&f,sizeof(float));
			break;
			}
		case DL_RAY_BIAS:{
			GenLight *gl = (GenLight *)data;
			float f = gl->GetRayBias(0);
			WRTERR(&f,sizeof(float));
			break;
			}
		case PROC_NAME:	/* external procedure name */
//			t=(Tri_obj *)data;
//			WRTERR(t->proc_name,strlen(t->proc_name)+1);
			break;
		case PROC_DATA:	/* external procedure data chunk */
//			t=(Tri_obj *)data;
//			{
//			ULONG *plong = (ULONG *)t->proc_data;
//			WRTERR(&plong[1],plong[0]);
//			}
			break;
		case DL_LOCAL_SHADOW2:
			{
			GenLight *gl = (GenLight *)data;
			LocShad2 locshad;
			locshad.bias = gl->GetMapBias(0);
			locshad.shadsize = gl->GetMapSize(0);
			locshad.shadfilter = gl->GetMapRange(0);
			WRTERR(&locshad,sizeof(LocShad2));
			}
			break;
		case N_CAMERA: {
			oe = (ObjectEntry *)data;
			node = oe->entry->node;
			CameraObject* camobj = (CameraObject *)oe->entry->obj;
			GenCamera *gc = (GenCamera*)oe->entry->obj;
			CameraState cs;
			camobj->EvalCameraState(theSceneEnum->time, valid, &cs);
			Camera3DS c;
			Matrix3 mat = node->GetNodeTM(theSceneEnum->time);			
			Point3 pos = mat.GetRow(3);
			c.x = pos.x;
			c.y = pos.y;
			c.z = pos.z;
			if (oe->entry->tnode) {
				mat = oe->entry->tnode->GetNodeTM(theSceneEnum->time);			
				pos = mat.GetRow(3);
				}
			c.tx = pos.x;
			c.ty = pos.y;
			c.tz = pos.z;
			c.bank = 0.0f;
			c.focal = 2400.0f/RadToDeg(cs.fov);
			c.flags = 0;
			c.nearplane = gc->GetClipDist(0,0);
			c.farplane = gc->GetClipDist(0,1);
			WRTERR(&c,32);
//			if(c->flags & NO_CAM_CONE) {
//				if(dump_mchunk(CAM_SEE_CONE,stream,NULL)==0)
//				return(0);
//				}
			if(dump_mchunk(CAM_RANGES,stream,gc)==0)
				return(0);
//			if(c->appdata!=NULL) {
//				if(dump_mchunk(APP_DATA,stream,c->appdata)==0)
//				return(0);
//				}     
			}
			break;
		case CAM_RANGES:{
			GenCamera *gc = (GenCamera *)data;
			float cnear = gc->GetEnvRange(0,0);
			float cfar = gc->GetEnvRange(0,1);
			WRTERR(&cnear,sizeof(float));
			WRTERR(&cfar,sizeof(float));
			}
			break;
//		case DL_SPOT_PROJECTOR:
//			d=(Dirlight *)data;
//			WRTERR(d->imgfile,13);
//			break;

		/* Dummy chunks -- no data, just their tag */

		case RAY_SHADOWS:
		case USE_BIT_MAP:
		case USE_SOLID_BGND:
		case USE_V_GRADIENT:
		case USE_FOG:
		case USE_DISTANCE_CUE:
		case USE_LAYER_FOG:
		case FOG_BGND:
		case DCUE_BGND:
		case CAM_SEE_CONE:
		case DL_SEE_CONE:
		case DL_SPOT_OVERSHOOT:
		case DL_SPOT_RECTANGULAR:
		case DL_RAYSHAD:
		case DUMMY:
			break;
		default:		// Unknown chunk!
			assert(0);
			break;
		}

	/* Save file ptr */

	curpos=ftell(stream);

	/* Point back to chunk size location */

	fseek(stream,chunkptr,SEEK_SET);

	/* Calc & write chunk size */

	chunksize=curpos-chunkbase;
	WRTERR(&chunksize,4);

	/* Point back to file end */

	fseek(stream,curpos,SEEK_SET);
	return(1);
	}


#define WRITEF(ptr,size) fwrite((char *)ptr,1,size,wstream)
#define WERR(ptr,sz) {if (WRITEF(ptr,(sz))!=(sz)) return(0);}
#define WRFLOAT(ptr) WERR(ptr,sizeof(FLOAT))
#define WR3FLOAT(ptr) WERR(ptr,3*sizeof(FLOAT))
#define WRLONG(ptr) WERR(ptr,sizeof(LONG))
#define WRSHORT(ptr) WERR(ptr,sizeof(SHORT))
#define WRSTRING(ptr) WERR(ptr,strlen(ptr)+1)

#define EPS (1.0e-7)
#define NotZero(f) (f<-EPS || f>EPS)

int WriteStd1KeyHdr() {
	short trackflags = 0;
	WRSHORT(&trackflags);
	long lwork = 0;
	WRLONG(&lwork);		// Track min
	WRLONG(&lwork);		// Track max
	long nkeys = 1;
	WRLONG(&nkeys);
	long keytime = 0;
	WRLONG(&keytime);
	short wflags = 0;		// Ignoring tension, continuity, bias, eases
	WRSHORT(&wflags);
	return 1;
	}

static
BOOL IsTCBContol(Control *cont) {
	return (
		cont->ClassID()==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0) ||
		cont->ClassID()==Class_ID(TCBINTERP_POSITION_CLASS_ID,0) ||
		cont->ClassID()==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0) ||
		cont->ClassID()==Class_ID(TCBINTERP_POINT3_CLASS_ID,0) ||
		cont->ClassID()==Class_ID(TCBINTERP_SCALE_CLASS_ID,0));
}

static BOOL WriteControllerChunk(Control *cont,int type, float scale=1.0f)
	{
	ITCBFloatKey fkey;
	ITCBPoint3Key pkey;
	ITCBRotKey rkey;
	ITCBScaleKey skey;
	ITCBKey *k;	
	int num = cont->NumKeys();
	float fval;
	Point3 pval;
	Quat q, qLast = IdentQuat();
	AngAxis rval;
	ScaleValue sval;
	Interval valid;
	TimeValue t;

	// Set up 'k' to point at the right derived class
	switch (type) {
		case KEY_FLOAT: k = &fkey; break;
		case KEY_POS:   k = &pkey; break;
		case KEY_ROT:   k = &rkey; break;
		case KEY_SCL:   k = &skey; break;
		case KEY_COLOR: k = &pkey; break;
		default: return FALSE;
		}

	// Get the keyframe interface
	IKeyControl *ikeys = GetKeyControlInterface(cont);
	
	// Gotta have some keys
	if (num==NOT_KEYFRAMEABLE || num==0) {
		return FALSE;
		}
	
	// Write track some stuff
	short trackflags = 0;
	WRSHORT(&trackflags);
	long lwork = cont->GetKeyTime(0)/GetTicksPerFrame();
	WRLONG(&lwork);		// Track min
	lwork = cont->GetKeyTime(num-1)/GetTicksPerFrame();
	WRLONG(&lwork);		// Track max
	long nkeys = num;
	WRLONG(&nkeys);

	for (int i=0; i<num; i++) {
		if (IsTCBContol(cont) && ikeys) {
			ikeys->GetKey(i,k);


			// Write key time
			long keytime = k->time/GetTicksPerFrame();
			WRLONG(&keytime);
			
			// Write flags
			short wflags = 0;
			if (k->tens   != 0.0f) wflags |= W_TENS;
			if (k->cont   != 0.0f) wflags |= W_CONT;
			if (k->bias   != 0.0f) wflags |= W_BIAS;
			if (k->easeIn != 0.0f) wflags |= W_EASETO;
			if (k->easeOut!= 0.0f) wflags |= W_EASEFROM;			
			WRSHORT(&wflags);
			
			// Write TCB and ease
			if (k->tens   != 0.0f) WRFLOAT(&k->tens);
			if (k->cont   != 0.0f) WRFLOAT(&k->cont);
			if (k->bias   != 0.0f) WRFLOAT(&k->bias);
			if (k->easeIn != 0.0f) WRFLOAT(&k->easeIn);
			if (k->easeOut!= 0.0f) WRFLOAT(&k->easeOut);
			
			// Write values
			switch (type) {
				case KEY_FLOAT: 
					fkey.val*=scale;
					WRFLOAT(&fkey.val);
					break;
				
				case KEY_SCL:
					WRFLOAT(&skey.val.s.x);
					WRFLOAT(&skey.val.s.y);
					WRFLOAT(&skey.val.s.z);
					break;

				case KEY_COLOR:
				case KEY_POS:
					WRFLOAT(&pkey.val.x);
					WRFLOAT(&pkey.val.y);
					WRFLOAT(&pkey.val.z);
					break;

				case KEY_ROT:
					WRFLOAT(&rkey.val.angle);
					WRFLOAT(&rkey.val.axis.x);
					WRFLOAT(&rkey.val.axis.y);
					WRFLOAT(&rkey.val.axis.z);
					break;
				}

		} else {
			// Sample the control at each key time
			t = cont->GetKeyTime(i);

			// Write key time
			long keytime = t/GetTicksPerFrame();
			WRLONG(&keytime);
			
			// Write flags
			short wflags = 0;			
			WRSHORT(&wflags);
			
			// Write values
			switch (type) {
				case KEY_FLOAT: 
					cont->GetValue(t,&fval,valid);
					fval*=scale;
					WRFLOAT(&fval);
					break;
				
				case KEY_SCL:
					cont->GetValue(t,&sval,valid);
					WRFLOAT(&sval.s.x);
					WRFLOAT(&sval.s.y);
					WRFLOAT(&sval.s.z);
					break;

				case KEY_COLOR:
				case KEY_POS:
					cont->GetValue(t,&pval,valid);
					WRFLOAT(&pval.x);
					WRFLOAT(&pval.y);
					WRFLOAT(&pval.z);
					break;

				case KEY_ROT:
					cont->GetValue(t,&q,valid);
					rval = AngAxis(q/qLast);
                    qLast = q;
					WRFLOAT(&rval.angle);
					WRFLOAT(&rval.axis.x);
					WRFLOAT(&rval.axis.y);
					WRFLOAT(&rval.axis.z);
					break;
				}
			}
		}
	return TRUE;
	}



static int WriteFloatTrack(Control *c, float scale) {
	Interval valid;
	if (!WriteControllerChunk(c,KEY_FLOAT,scale)) {
		float v;
		c->GetValue(0,&v,valid);
		v *= scale;
		if (!WriteStd1KeyHdr()) return 0;
		WRFLOAT(&v);      
		}
	return 1;
	}

static int WriteFloatTrackOrStd1KeyHdr(Control *c, float scale) {
	if (c) {
		if (!WriteFloatTrack(c, scale)) return 0;
		}
	else {
		if (!WriteStd1KeyHdr()) return 0;
		WRFLOAT(&floatVal);
		}
	return 1;
	}

int	KFWriteChunk(unsigned short tag, void *data);

int dump_kchunk(unsigned short tag, FILE *stream, void *data) {
	wstream = stream;
	return(KFWriteChunk(tag,data));
	}

// RB 4-4-96: see comment by OBJECT_NODE_TAG
static BOOL savingLiteralData = FALSE;

int	KFWriteChunk(unsigned short tag, void *data)	{
	long chunkptr,chunkbase,curpos,chunksize;
	Interval valid;
	INode *node;
	SHORT version = KFVERSION;
	Object *ob;
		
	chunkbase = ftell(wstream);
#ifdef DBGKFB
	if(dbgio) {
		printf("\n------>Writing chunk type: ");
		kfprint_ID(tag);
		printf("   Chunkbase = %lx, tag =  %5x.\n",chunkbase,tag);
		}
#endif

	WRSHORT(&tag);
	chunkptr = ftell(wstream);

	WRLONG(&chunkptr);

	switch(tag)	{
		case KFDATA: 
			{
			/* Keyframe main chunk */
			int nodeid;
			if (!KFWriteChunk(KFHDR,data)) return(0); 
			if (!KFWriteChunk(KFSEG,data)) return(0); 
			if (!KFWriteChunk(KFCURTIME,data)) return(0); 

			nodeid=0;
			for (SceneEntry *se = theSceneEnum->head; se!=NULL; se = se->next) {
				switch (se->type)  {
					case OBTYPE_DUMMY:
					case OBTYPE_MESH: 
						se->id = nodeid++;
						if (!KFWriteChunk(OBJECT_NODE_TAG,se)) return(0);
						break;
					case OBTYPE_OMNILIGHT: 
						se->id = nodeid++;
						if (!KFWriteChunk(LIGHT_NODE_TAG,se))return(0);
						break;	  
					case OBTYPE_SPOTLIGHT: 
						se->id = nodeid++;
						if (!KFWriteChunk(SPOTLIGHT_NODE_TAG,se)) return(0);
						break;	  
					case OBTYPE_LTARGET: 
						se->id = nodeid++;
						if (!KFWriteChunk(L_TARGET_NODE_TAG,se)) return(0);
						break;
					case OBTYPE_CTARGET: 
						se->id = nodeid++;
						if (!KFWriteChunk(TARGET_NODE_TAG,se)) return(0);
						break;
					case OBTYPE_CAMERA: 
						se->id = nodeid++;
						if (!KFWriteChunk(CAMERA_NODE_TAG,se))return(0); 
						break;
//					case TARGET_NODE: 
//						{
//						Namedobj *ob = node->object;
//						node->id = nodeid++;
//						switch(ob->type) {
//							case N_CAMERA:
//								if (!KFWriteChunk(TARGET_NODE_TAG,node))
//									return(0);
//								break;
//							case N_DIRECT_LIGHT:
//								if (!KFWriteChunk(L_TARGET_NODE_TAG,node))
//									return(0);
//								break;
//							}
//						}
//						break;
//					case OBTYPE_SPOTLIGHT: 
//						node->id = nodeid++;
//						if (!KFWriteChunk(LIGHT_NODE_TAG,node))return(0);
//						break;	  
//					case SPOTLIGHT_NODE: 
//						node->id = nodeid++;
//						if (!KFWriteChunk(SPOTLIGHT_NODE_TAG,node))return(0);
//						break;
					}
				}
//			/* Do new kinds of nodes later, so 3DS2.0 doesn't get screwed up*/
//			for (i=0; i<nodeCount; i++) {
//				node = nodeList[i];
//				switch (node->type)  {
//					case AMBIENT_NODE: 
//						node->id = -1;
//						if (!KFWriteChunk(AMBIENT_NODE_TAG,node))return(0);
//						break;
//					}
//				}
//			if (KFappdata!=NULL) 
//				if (!KFWriteChunk(APP_DATA,KFappdata))  return(0);
			}
			break;
		case KFHDR: {
			WRSHORT(&version);
			WRSTRING("MAXSCENE");
			long animLength = theSceneEnum->i->GetAnimRange().End()/GetTicksPerFrame();
			WRLONG(&animLength);
#ifdef DBGKFB
			if (dbgio) 
				printf("version = %d, mshName = %s, P.animLength = %d	\n",
					version, mshName, P.animLength);
#endif
			}
			break;
		case KFSEG: {
			Interval i = theSceneEnum->i->GetAnimRange();
			long s = i.Start()/GetTicksPerFrame();
			long e = i.End()/GetTicksPerFrame();
			WRLONG(&s);
			WRLONG(&e);
			}
			break;
		case KFCURTIME: {
			long t = theSceneEnum->i->GetTime();
			WRLONG(&t);
			}
			break;
		case PIVOT:	 {
         // gdf make a 3ds pivot out of a max pivot.
            Point3 pivot = GetPivotOffset(((SceneEntry *)data)->node);
			WR3FLOAT(&pivot[0]);
			}
			break;
		case BOUNDBOX:	 {
			Box3 bb;
			ViewExp *vpt = theSceneEnum->i->GetViewport(NULL);
			SceneEntry *se = (SceneEntry *)data;
			se->obj->GetLocalBoundBox(theSceneEnum->i->GetTime(), se->node, vpt, bb);
			WR3FLOAT(&bb.pmin[0]);
			WR3FLOAT(&bb.pmax[0]);
			}
			break;
		case INSTANCE_NAME:	 {
			CStr *name = (CStr *)data;
			if(name->Length()>10)
				name->Resize(10);
			WERR(name->data(),(size_t)(name->Length()+1));
			}
			break;
//		case MORPH_SMOOTH: {
//			ObjectNode *onode = (ObjectNode *)data;
//			WRFLOAT(&onode->smooth_ang);
//			}
//			break;
		case NODE_ID: {	
			SceneEntry *se = (SceneEntry *)data;
#ifdef DBGKFB
			if (dbgio)
				printf(" writing NODE_ID =%d \n", nd->id);
#endif
			WRSHORT(&se->id);
			}
			break;
		case NODE_HDR: {
			SceneEntry *se = (SceneEntry *)data;
			BOOL isInstance = FALSE;
			node = se->node;
			ob = se->obj;
			// Write out the primary name -- It's the name of the first node using the object.
			// We do this to work with 3DS's system where a primary node uses an object and
			// shares its name, and all other instances use this name as a primary name with
			// a dot (.) followed by the instance name.
			if (se->type == OBTYPE_DUMMY) {
				WRSTRING("$$$DUMMY");
				}
			else if (se->type == OBTYPE_CTARGET||se->type == OBTYPE_LTARGET) {
				// The object name written should be the same as that of the associated
				// light or camera.
				//INode *lan = se->node->GetLookatNode();
				//ObjectEntry *oe = theObjects->Contains(lan);
				ObjectEntry *oe = theObjects->FindLookatNode(se->node);
				assert(oe);
				CStr name(oe->entry->name); 
				if(name.Length()>10)
					name.Resize(10);
				WERR(name.data(),(size_t)(name.Length()+1));
				}
			else {
				TSTR name; 
				
				// for cameras, lights, which can't be multiple instanced in 3DS
				// just get name from  scene entry  -- DS 4/6/96
				if (se->type != OBTYPE_MESH) {
					name = se->name;
					}
				else {
					// search for the object entry 
					ObjectEntry *oe = theObjects->Contains(ob);
					assert(oe);
					name = oe->entry->name; // RB
					// If the name is not equal to the master name then
					// it is an instance.
					if (!(oe->entry->name==se->name)) 
						isInstance = TRUE;
					}
				
				if(name.Length()>10)
					name.Resize(10);
//DebugPrint("Making %s unique\n",CStr(name));
//				theObjNames.MakeUnique(name);
//DebugPrint("Got %s\n",CStr(name));
				CStr cname(name);		// Make it char*
				WERR(cname.data(),(size_t)(cname.Length()+1));
				}
			short zero = 0;
			short fl = isInstance ? 0 : (short)PRIMARY_NODE;
			WRSHORT(&fl);		// Node flags
			WRSHORT(&zero);		// flags 2
			INode *parentNode = node->GetParentNode();
			SceneEntry *pse = theSceneEnum->Find(parentNode);
#ifdef DBGKFB
			if (dbgio)
				printf("nparent = %X \n", nparent);
#endif
			if(pse) {
				WRSHORT(&pse->id);
				}
			else{
				short none = (short)NO_PARENT; 
				WRSHORT(&none);
				}
			}
			break;
//		case APP_DATA:
//		   {
//			ULONG *plong = (ULONG *)data;
//			WERR(&plong[1],plong[0]);
//			}
//			break;
		case OBJECT_NODE_TAG: {
			SceneEntry *se = (SceneEntry *)data;
			ob = se->obj;
			if (!KFWriteChunk(NODE_ID,se)) return(0);
			if (!KFWriteChunk(NODE_HDR,se)) return(0);
//			if (on->appdata) if (!KFWriteChunk(APP_DATA,on->appdata)) return(0);
			if (!KFWriteChunk(PIVOT,se)) return(0);
			//TSTR name(se->node->GetName());
			TSTR name(se->name); // RB
			// MAX doesn't have the notion of instance names, but 3DS does -- we must use the name
			// of the original object as the node name, and this node's name as the instance name.
			// We don't write an instance name if the instance name is the same as the master object's
			// name.
			ObjectEntry *oe = theObjects->Contains(ob);
			assert(oe);
			//TSTR mname(oe->entry->node->GetName());		// Master name
			TSTR mname(oe->entry->name);		// Master name
			if (se->type!=OBTYPE_CTARGET&&se->type!=OBTYPE_LTARGET) {
				if (se->type==OBTYPE_DUMMY || !(mname == name)) { 
					CStr wname(name);
					if (!KFWriteChunk(INSTANCE_NAME,&wname)) return(0); 
					}
				}

            WriteController(se->node);

//			if (on->mtrack.keytab)
//				if (!KFWriteChunk(MORPH_TRACK_TAG, &on->mtrack)) return(0);
//			if (on->htrack.keytab)
//				if (!KFWriteChunk(HIDE_TRACK_TAG, &on->htrack)) return(0);
			if (se->type==OBTYPE_DUMMY) {  /* dummy Node */
				if (!KFWriteChunk(BOUNDBOX, data)) return(0);
				}
//			if (!KFWriteChunk(MORPH_SMOOTH, data)) return(0);
			}
			break;
		case CAMERA_NODE_TAG: {
			SceneEntry *se = (SceneEntry *)data;
			GenCamera *ob = (GenCamera *)se->obj;
			if (!KFWriteChunk(NODE_ID,se)) return(0);
			if (!KFWriteChunk(NODE_HDR,se)) return(0);
//			if (cn->appdata) if (!KFWriteChunk(APP_DATA,cn->appdata)) return(0);
			Control *c;
			
			c = se->node->GetTMController()->GetPositionController();
			if (c) if (!KFWriteChunk(POS_TRACK_TAG, c)) return(0);
			
			floatVal = RadToDeg(ob->GetFOV(0));
			c = ob->GetFOVControl();
			if (!KFWriteChunk(FOV_TRACK_TAG, c)) return(0);

			Control* tmc = se->node->GetTMController();
			c = tmc->GetRollController();
			if (c) if (!KFWriteChunk(ROLL_TRACK_TAG, c)) return(0);
			}
			break;
		case L_TARGET_NODE_TAG:
		case TARGET_NODE_TAG:{
			SceneEntry *se = (SceneEntry *)data;
			if (!KFWriteChunk(NODE_ID,se)) return(0);
			if (!KFWriteChunk(NODE_HDR,se)) return(0);
//			if (tn->appdata) if (!KFWriteChunk(APP_DATA,tn->appdata)) return(0);
			Control *c;
			c = se->node->GetTMController()->GetPositionController();
			if (c) if (!KFWriteChunk(POS_TRACK_TAG, c)) return(0);
			}
			break;
		case LIGHT_NODE_TAG: {
			SceneEntry *se = (SceneEntry *)data;
			GenLight* ob = (GenLight *)se->obj;
			if (!KFWriteChunk(NODE_ID,se)) return(0);
			if (!KFWriteChunk(NODE_HDR,se)) return(0);
//			if (ln->appdata) if (!KFWriteChunk(APP_DATA,ln->appdata)) return(0);
			Control *c;
			
			colorVal = ob->GetRGBColor(0);
			c = ob->GetColorControl();
			if (!KFWriteChunk(COL_TRACK_TAG, c)) return(0);
			
			c = se->node->GetTMController()->GetPositionController();
			if (c) if (!KFWriteChunk(POS_TRACK_TAG, c)) return(0);
			}
			break;

		case SPOTLIGHT_NODE_TAG: {
			SceneEntry *se = (SceneEntry *)data;
			GenLight* ob = (GenLight *)se->obj;
			if (!KFWriteChunk(NODE_ID,se)) return(0);
			if (!KFWriteChunk(NODE_HDR,se)) return(0);
			Control *c;
			c = se->node->GetTMController()->GetPositionController();
			if (c) if (!KFWriteChunk(POS_TRACK_TAG, c)) return(0);

//			if (sln->appdata) if (!KFWriteChunk(APP_DATA,sln->appdata)) return(0);
			
			colorVal = ob->GetRGBColor(0);
			c = ob->GetColorControl();
			if (!KFWriteChunk(COL_TRACK_TAG, c)) return(0);
			
			floatVal = ob->GetHotspot(0);
			c = ob->GetHotSpotControl();
			if (!KFWriteChunk(HOT_TRACK_TAG, c)) return(0);

			floatVal = ob->GetFallsize(0);
			c = ob->GetFalloffControl();
			if (!KFWriteChunk(FALL_TRACK_TAG, c)) return(0);

			Control* tmc = se->node->GetTMController();
			c = tmc->GetRollController();
			if (!KFWriteChunk(ROLL_TRACK_TAG, c)) return(0);
			}
			break;
//		case AMBIENT_NODE_TAG:
//			AmbientNode *an = (AmbientNode *)data;
//			if (!KFWriteChunk(NODE_ID,an)) return(0);
//			if (!KFWriteChunk(NODE_HDR,an)) return(0);
//			if (an->appdata) if (!KFWriteChunk(APP_DATA,an->appdata)) return(0);
//			if (!KFWriteChunk(COL_TRACK_TAG, &an->coltrack)) return(0);
//			break;
        /*
		case SCL_TRACK_TAG: {
			if (!savingLiteralData) {
				Control *c = (Control*)data;
				if (!WriteControllerChunk(c,KEY_SCL)) {
					ScaleValue pos;
					c->GetValue(0,&pos,valid);
					if (!WriteStd1KeyHdr()) return 0;
					WRFLOAT(&pos.s.x);
					WRFLOAT(&pos.s.y);
					WRFLOAT(&pos.s.z);
					}
			} else {
				Point3 *p = (Point3*)data;
				if (!WriteStd1KeyHdr()) return 0;
				WRFLOAT(&p->x);
				WRFLOAT(&p->y);
				WRFLOAT(&p->z);
				}
			}
			break;
		*/		
		case POS_TRACK_TAG: {
			if (!savingLiteralData) {
				Control *c = (Control*)data;
				if (!WriteControllerChunk(c,KEY_POS)) {
					Point3 pos;
					c->GetValue(0,&pos,valid);					
					if (!WriteStd1KeyHdr()) return 0;
					WRFLOAT(&pos.x);
					WRFLOAT(&pos.y);
					WRFLOAT(&pos.z);
					}
			} else {
				Point3 *p = (Point3*)data;
				if (!WriteStd1KeyHdr()) return 0;
				WRFLOAT(&p->x);
				WRFLOAT(&p->y);
				WRFLOAT(&p->z);
				}
			}
			break;

        /*
		case ROT_TRACK_TAG: {
			if (!savingLiteralData) {
				Control *c = (Control*)data;
				if (!WriteControllerChunk(c,KEY_ROT)) {
					Quat rot;
					c->GetValue(0,&rot,valid);
					float angle;
					Point3 axis;
					AngAxisFromQ(rot,&angle,axis);
					if (!WriteStd1KeyHdr()) return 0;
					WRFLOAT(&angle);
					WRFLOAT(&axis.x);
					WRFLOAT(&axis.y);
					WRFLOAT(&axis.z);
					}
			} else {
				Quat *q = (Quat*)data;
				float angle;
				Point3 axis;
				AngAxisFromQ(*q,&angle,axis);
				if (!WriteStd1KeyHdr()) return 0;
				WRFLOAT(&angle);
				WRFLOAT(&axis.x);
				WRFLOAT(&axis.y);
				WRFLOAT(&axis.z);
				}
			}
			break;
        */
		case FOV_TRACK_TAG:
			if (!WriteFloatTrackOrStd1KeyHdr((Control *)data, RAD_TO_DEG))
				return 0;
			break;
		case FALL_TRACK_TAG:
		case HOT_TRACK_TAG: 
			if (!WriteFloatTrackOrStd1KeyHdr((Control *)data,1.0f))
				return 0;
			break;
		case ROLL_TRACK_TAG: 	
			if (!WriteFloatTrackOrStd1KeyHdr((Control *)data, -RAD_TO_DEG))
				return 0;
			break;
		case COL_TRACK_TAG: {
			Control *c = (Control*)data;
			if (!c||!WriteControllerChunk(c,KEY_COLOR)) {
				Point3 pos;
				if (c) c->GetValue(0,&pos,valid);					
				else pos = colorVal;
				if (!WriteStd1KeyHdr()) return 0;
				WRFLOAT(&pos.x);
				WRFLOAT(&pos.y);
				WRFLOAT(&pos.z);
				}
			}
			break;
		case HIDE_TRACK_TAG:
		case MORPH_TRACK_TAG:	
			if (!WriteFloatTrackOrStd1KeyHdr((Control *)data, 1.0f))
				return 0;
			break;
		default:
			assert(0);
			break;
		}   			

	/* Save file ptr */
	curpos=ftell(wstream);

	/* Point back to chunk size location */
	fseek(wstream,chunkptr,SEEK_SET);
	
	/* Calc & write chunk size */
	chunksize=curpos-chunkbase;
#ifdef DBGKFB
	if (dbgio) {
		printf("writing chunkptr = %lx, chunkSize = %ld \n",chunkptr,chunksize);
		}
#endif

	WERR(&chunksize,4);
	
	/* Point back to file end */
	
	fseek(wstream,curpos,SEEK_SET);
	return(1);
	}

/* Main routine for writing MSH+KFB! 	*/
/* Recursive chunk writer -- keeps track of each chunk	*/

int	dump_3dchunk(USHORT tag,FILE *stream,void *data)	{
	long chunkptr,chunkbase,curpos,chunksize,lval;
	chunkbase=ftell(stream);
	WRTERR(&tag,2);
	chunkptr=ftell(stream);		/* Save file ptr for chunk size */
	WRTERR(&chunkptr,4);
  	wstream = stream;
	switch(tag)	{
		case M3DMAGIC:	/* Mesh editor */
			if(dump_3dchunk(M3D_VERSION,stream,NULL)==0)
				return(0);
			if (dump_mchunk(MMAGIC,stream,NULL)==0)
				return(0);
			if(dump_kchunk(KFDATA,stream,NULL)==0)
				return(0);
			break;
		case M3D_VERSION:
			lval= 3;
			WRTERR(&lval,4);
			break;
		}
	/* Save file ptr */
	curpos=ftell(stream);
	
	/* Point back to chunk size location */
	fseek(stream,chunkptr,SEEK_SET);
	
	/* Calc & write chunk size */
	
	chunksize=curpos-chunkbase;
	WRTERR(&chunksize,4);

	/* Point back to file end */
	fseek(stream,curpos,SEEK_SET);
	return(1);
	}

int
_3ds_save(const TCHAR *filename, ExpInterface *ei, Interface *gi, _3DSExport* exp) {
	// Put up the options dialog to find out how they want the file written!

	if(showPrompts) {
		int result = DialogBoxParam(hInstance,
									MAKEINTRESOURCE(IDD_EXPORTOPTIONS),
//									GetActiveWindow(),
									gi->GetMAXHWnd(),
									ExportOptionsDlgProc,
									(LPARAM) exp);
		if (result <= 0) 
		    return 0;
		}
    else {	// Set default parameters here
		MaxUVs = TRUE;
		}

    MeshMtlList myMtls;

	// Make sure there are nodes we're interested in!
	// Ask the scene to enumerate all its nodes so we can determine if there are any we can use
	SceneEnumProc myScene(ei->theScene, gi->GetTime(), gi, &myMtls);

	// Any useful nodes?
	if(!myScene.Count()) {
		if(showPrompts)
			Alert(IDS_TH_NODATATOEXPORT);
		return 1;
		}

	// Construct unique names < 10 chars.
	myScene.BuildNames();

	ObjectList myObjects(myScene);

	theSceneEnum = &myScene;
	theObjects = &myObjects;
	theMtls = &myMtls;


	WorkFile theFile(filename,"wb");
	FILE *stream = theFile.Stream();

		
	if(!stream) {
		if(showPrompts)
			Alert(IDS_TH_CANTCREATE);
		return(0);
		}
	
	int error=0;
	
	if(dump_3dchunk(M3DMAGIC,stream,NULL)==0)
		error=1;
	
	if(theFile.Close())
		error=1;
	
	if(error) {
		if(showPrompts)
			Alert(IDS_TH_WRITEERROR);
		remove(filename);
		return(0);
		}
	
	return 1;	
	}

int
_3DSExport::DoExport(const TCHAR *filename,ExpInterface *ei,Interface *gi, BOOL suppressPrompts, DWORD options) {
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	int status;

    msgList.SetCount(0);

	status = _3ds_save(filename, ei, gi, this);

 // gdf output errors
    if (msgList.Count()) {
		if(showPrompts)
			CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ERROR_MSGS), GetActiveWindow(), MsgListDlgProc);
        for (int i = 0; i < msgList.Count(); i++) {
            delete msgList[i];
            msgList[i] = NULL;
        }
        msgList.Delete(0, msgList.Count());
    }

	if(status == 0)
		return 1;		// Dialog cancelled
	if(status < 0)
		return 0;		// Real, honest-to-goodness error
	return(status);
	}

BOOL _3DSExport::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
	}

#ifdef HOLDING

	for(ox=0; ox<myScene.Count(); ++ox)
		{
		INode *n = myScene[ox]->node;
		Object *obj = myScene[ox]->obj;
		TriObject *tri = (TriObject *)obj->ConvertToType(gi->GetTime(), triObjectClassID);
		Mesh &mesh = tri->mesh;
		Matrix3 tm = n->GetObjectTM(gi->GetTime());

		int verts = mesh.getNumVerts();
		int faces = mesh.getNumFaces();

		Point3 vert;
		Face face;
		for(ix=0; ix<verts; ++ix) {
			vert = mesh.verts[ix] * tm;
			}
		for(ix=0; ix<faces; ++ix)
			{
			face = mesh.faces[ix];
			}

		// Delete the working object, if necessary
		if(obj != (Object *)tri)
			tri->DeleteThis();
		}
#endif

short Get3DSTVerts(Mesh& mesh, Point2 *tv) {
	int nv = mesh.getNumVerts();
 	int nf = mesh.getNumFaces();
	short wrap = 0;
	BitArray done(nv);
	for (int j=0; j<nf; j++) {
		Face& face = mesh.faces[j];
		TVFace& tvface = mesh.tvFace[j];
		for (int k=0; k<3; k++)  {
			// get the texture vertex.
			Point3 uvw = mesh.tVerts[tvface.t[k]];
			Point2 v(uvw.x,uvw.y);
			// stuff it into the 3DSr4 vertex
			int vert = face.v[k];
			if (vert>65535) continue;
			if (!done[vert]) {
				tv[vert] = v;
				done.Set(vert,1);
				}
			else {
				if (v.x!=tv[vert].x) {
					wrap |= UWRAP;
					if (v.x<tv[vert].x) tv[vert].x = v.x;
					}
				if (v.y!=tv[vert].y) {
					wrap |= VWRAP;
					if (v.y<tv[vert].y) tv[vert].y = v.y;
					}
				}
			}
		}
	return wrap;
	}

// gdf
void
AddToMsgList(Tab<TSTR*>& mList, TCHAR* msg)
{
    TCHAR buf[ERROR_MSG_MAX_LEN];
    strncpy(buf, msg, ERROR_MSG_MAX_LEN);
    buf[ERROR_MSG_MAX_LEN - 1] = '\0';
    TSTR* errStr = new TSTR(buf);
    mList.Insert(mList.Count(), 1, &errStr);
}

// gdf
void
AddLineToListDlg(HWND hDlg, const TCHAR* buf) {
    SendDlgItemMessage(hDlg, IDC_ERROR_MSG_LIST, LB_ADDSTRING, NULL, (LPARAM) buf);
}

// gdf output errors
void BuildMsgList(HWND hDlg)
{
    AddLineToListDlg(hDlg, GetString(IDS_START));
    for (int i = 0; i < msgList.Count(); i++) {
        AddLineToListDlg(hDlg, _T(msgList[i]->data()));
    }
    AddLineToListDlg(hDlg, GetString(IDS_END));
}

//gdf
INT_PTR
CALLBACK MsgListDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    int tabs[3] = {24,24,24};
    switch (iMsg)
    {
	    case WM_INITDIALOG:
			CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
            SendDlgItemMessage(hDlg, IDC_ERROR_MSG_LIST, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hDlg, IDC_ERROR_MSG_LIST, LB_SETTABSTOPS, (WPARAM) 3,(LPARAM) tabs);
			BuildMsgList(hDlg);
			return TRUE;

	    case WM_COMMAND:
			switch(LOWORD(wParam))
            {
				case IDOK:	
					EndDialog(hDlg, TRUE); 
					break;
			}
			break;
	}
    return FALSE;
}

// gdf check to see if two UVVerts are the same
BOOL 
UVVertEqual(UVVert tv0, UVVert tv1) {
    return (tv0.x == tv1.x &&
            tv0.y == tv1.y &&
            tv0.y == tv1.y);
}

// gdf convert MAX UVVert into 3ds UVVert this will fail if uvvert is not the
// same for a shared vert.  FIXME add wrapFlags
void
ConvertTo3DSTVerts(ObjectEntry* oe, Tab<UVVert>& tv)
{
    Mesh mesh = oe->tri->GetMesh();
	BitArray vWritten;
    vWritten.SetSize(mesh.getNumVerts());
    vWritten.ClearAll();
    BOOL wMsg = FALSE;

    tv.SetCount(mesh.getNumVerts());
    if (mesh.getNumVerts() != mesh.getNumTVerts()) {
        TCHAR buf[ERROR_MSG_MAX_LEN];
        sprintf(buf, GetString(IDS_UVVERT_MISMATCH), oe->entry->name);
		AddToMsgList(msgList, buf);
        wMsg = TRUE;
    }
	for (int i = 0; i < mesh.getNumFaces(); i++) {
        Face face     = mesh.faces[i];
        TVFace tvFace = mesh.tvFace[i];
        for (int j = 0; j < 3; j++) {
            UVVert tvert  = mesh.getTVert(tvFace.t[j]);
            if (vWritten[face.v[j]] && !UVVertEqual(tv[face.v[j]], tvert)) {
                if (!wMsg ) {
                    TCHAR buf[ERROR_MSG_MAX_LEN];
                    sprintf(buf, GetString(IDS_UVVERT_TOO_MANY_PER_VERT), oe->entry->name);
		            AddToMsgList(msgList, buf);
                    wMsg = TRUE;
                }
            }
            tv[face.v[j]] = tvert;
            vWritten.Set(face.v[j]);
        }
    }
}

// gdf remove the rotational and scale comonents from the max pivot
// to get the pivot offset for 3ds
static Point3
GetPivotOffset(INode* node)
{
 // get the pivot offset and remove the rotational/scale component
    Matrix3 mat(TRUE);
    Quat qRot = node->GetObjOffsetRot();
    qRot.MakeMatrix(mat);
 // max scales the object if the pivot is scaled so skip.
 // m.SetScale( ((SceneEntry *)data)->node->GetObjOffsetScale().s );
    Point3 pivot = -node->GetObjOffsetPos();
    mat = Inverse(mat);
    Point3 pOff = VectorTransform(mat, pivot);
    return pOff;
}

// gdf used for reducing keys
inline BOOL
ApproxEqual(float a, float b)
{
    float eps = float(1.0e-5);
    float d = (float) fabs(a-b);
    return d < eps;
}

// gdf see if we can write TCB keys instead of generating keys by
// sampling frames.
static BOOL
IsTCBControl(Control *cont)
{
	return (cont->ClassID()==Class_ID(TCBINTERP_POSITION_CLASS_ID,0) ||
		    cont->ClassID()==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0) ||
    		cont->ClassID()==Class_ID(TCBINTERP_SCALE_CLASS_ID,0));
}

// gdf Get the tranform matrix that take a point from its local coordinate
// system to it's parent's coordinate system
static Matrix3
GetLocalNodeTM(INode* node, TimeValue t)
{
    Matrix3 tm(TRUE);
    if (node) {
        tm = node->GetNodeTM(t);
        if (!node->GetParentNode()->IsRootNode()) {
            Matrix3 ip = Inverse(node->GetParentNode()->GetNodeTM(t));
            tm = tm * ip;
        }
    }
    return tm;
}

// gdf remove the offset effects of the parent from the child.
static BOOL
UndoParentsOffset(INode* node, Point3& pt, Quat& rOff)
{
    INode* parent;
    Matrix3 mat(TRUE);
    Quat pOff;
    if (node && pt && rOff) {
        if (!node->GetParentNode()->IsRootNode()) {
            parent = node->GetParentNode();
            pOff   = parent->GetObjOffsetRot();
            if (pOff == IdentQuat()) return FALSE;
            pOff.MakeMatrix(mat);
            mat   = Inverse(mat);
            pt    = VectorTransform(mat, pt);
            rOff  = rOff/pOff;
            return TRUE;
        }
    }
    return FALSE;
}

// gdf get the file info before adding chunk
void
WriteChunkBegin(unsigned short cTag, long& cPtr, long& cBegin)
{
    cBegin = ftell(wstream);
    fwrite((char *) &cTag, 1, sizeof(SHORT), wstream);
    cPtr = ftell(wstream);
    fwrite((char *) &cPtr, 1, sizeof(LONG), wstream);
}
    
// gdf fixup the chunk header.
void
WriteChunkEnd(long& cPtr, long& cBegin)
{
    long cPos;
    long cSize;
    cPos = ftell(wstream);
    fseek(wstream, cPtr, SEEK_SET);
    cSize = cPos - cBegin;
    fwrite((char *) &cSize, 1, sizeof(LONG), wstream);
    fseek(wstream, cPos, SEEK_SET);
}

// gdf write a TCB PRS track
static BOOL
WriteTCBKeysChunk(INode* node, Control* cont, int type)
{
    IKeyControl*    ikeys;
	ITCBPoint3Key   pkey;
	ITCBRotKey      rkey;
	ITCBScaleKey    skey;
	ITCBKey*        k;
    Quat qLast = IdentQuat();
    int num = cont->NumKeys();
    Point3 firstpKey;
	
 // Get the keyframe interface to the TCB Controller
	ikeys = GetKeyControlInterface(cont);

 // Gotta have some keys
	if (num == NOT_KEYFRAMEABLE || num == 0 || !ikeys) {
	    return FALSE;
	}

 // Set up 'k' to point at the right derived class
	switch (type)
    {
		case KEY_POS:   k = &pkey; break;
		case KEY_ROT:   k = &rkey; break;
		case KEY_SCL:   k = &skey; break;
		default:        assert(FALSE);
	}

 // Write track some stuff
    short trackflags = 0;
    WRSHORT(&trackflags);
    long lwork = cont->GetKeyTime(0)/GetTicksPerFrame();
    WRLONG(&lwork);
    lwork = cont->GetKeyTime(num-1)/GetTicksPerFrame();
    WRLONG(&lwork);
    long nkeys = num;
    WRLONG(&nkeys);

    for (int i = 0; i < num; i++) {
        ikeys->GetKey(i,k);

     // Write key time
		long keytime = k->time/GetTicksPerFrame();
		WRLONG(&keytime);

	 // Write flags
		short wflags = 0;
		if (k->tens   != 0.0f) wflags |= W_TENS;
		if (k->cont   != 0.0f) wflags |= W_CONT;
		if (k->bias   != 0.0f) wflags |= W_BIAS;
		if (k->easeIn != 0.0f) wflags |= W_EASETO;
		if (k->easeOut!= 0.0f) wflags |= W_EASEFROM;			
		WRSHORT(&wflags);

     // Write TCB and ease
		if (k->tens   != 0.0f) WRFLOAT(&k->tens);
		if (k->cont   != 0.0f) WRFLOAT(&k->cont);
		if (k->bias   != 0.0f) WRFLOAT(&k->bias);
		if (k->easeIn != 0.0f) WRFLOAT(&k->easeIn);
		if (k->easeOut!= 0.0f) WRFLOAT(&k->easeOut);

	 // Write values
		switch (type)
        {
			case KEY_POS:
                {
                    if (!node->GetParentNode()->IsRootNode()) {
                        if (!(node->GetParentNode()->GetObjOffsetRot() == IdentQuat())) {
                            Quat q;
                            UndoParentsOffset(node, pkey.val, q);
                        }
                    }
                    WRFLOAT(&pkey.val.x);
			        WRFLOAT(&pkey.val.y);
			        WRFLOAT(&pkey.val.z);
                }
				break;

            case KEY_ROT:
                {
                    if (i) {
                        WRFLOAT(&rkey.val.angle);
			            WRFLOAT(&rkey.val.axis.x);
                        WRFLOAT(&rkey.val.axis.y);
                        WRFLOAT(&rkey.val.axis.z);
                    } else {
                        AngAxis aa;
                        Quat qkey = QFromAngAxis(rkey.val.angle, rkey.val.axis);
                     // remove the pivot's rotation
                        Quat qOff = qkey / Inverse(node->GetObjOffsetRot());
                     // remove the parents rotation
                        if (!node->GetParentNode()->IsRootNode()) {
                            if (!(node->GetParentNode()->GetObjOffsetRot() == IdentQuat())) {
                                Point3 p;
                                UndoParentsOffset(node, p, qOff);
                            }
                        }

                        AngAxisFromQ(qOff, &aa.angle, aa.axis);
                        WRFLOAT(&aa.angle);
			            WRFLOAT(&aa.axis.x);
                        WRFLOAT(&aa.axis.y);
                        WRFLOAT(&aa.axis.z);
                    }
                }
            	break;

            case KEY_SCL:
				WRFLOAT(&skey.val.s.x);
				WRFLOAT(&skey.val.s.y);
				WRFLOAT(&skey.val.s.z);
				break;
		}
    }
	return TRUE;
}

// gdf sample frame and write PRS track.  do some very trivial key reduction
static BOOL
WriteLinearKeysChunk(INode* node, int type)
{
    Point3 pval;
	AngAxis rval;
	ScaleValue sval;
	Interval valid;
	
    int i;
    Matrix3 tm;
    TimeValue t;
    AffineParts parts;
    Quat q, qLast;
    Point3 s, sLast, p;
    AngAxis a;

 // Tables of keyframe values
    Tab<Point3>     pKeys;
    Tab<TimeValue>  pFrames;
    Tab<AngAxis>    rKeys;
    Tab<TimeValue>  rFrames;
    Tab<Point3>     sKeys;
    Tab<TimeValue>  sFrames;
    
    TimeValue start = theSceneEnum->i->GetAnimRange().Start();
    TimeValue end   = theSceneEnum->i->GetAnimRange().End();
    TimeValue frames = (end - start)/GetTicksPerFrame();
    frames++;

	//SS 6/23/2003: type is not a bit-flag; we should be testing for equality.
    //if (type && KEY_POS) {
    if (type == KEY_POS) {
        pKeys.SetCount(frames);
        pFrames.SetCount(frames);
    }
    //if (type && KEY_ROT) {
    if (type == KEY_ROT) {
        rKeys.SetCount(frames);
        rFrames.SetCount(frames);
    }
    //if (type && KEY_SCL) {
    if (type == KEY_SCL) {
        sKeys.SetCount(frames);
        sFrames.SetCount(frames);
    }

    qLast = IdentQuat();
 // Sample the control at every frame
    for (i = 0, t = start; i < frames; i++, t += GetTicksPerFrame()) {
        tm = GetLocalNodeTM(node, t);
        decomp_affine(tm, &parts);
        s = ScaleValue(parts.k, parts.u).s;
        if (parts.f < 0.0f) s = - s;
        p = parts.t;
        q = parts.q;
        a = AngAxis(q / qLast);
        qLast = q;

        switch (type)
        {
            case KEY_POS: 
                {
                    Point3 pkey = parts.t;
                    if (!node->GetParentNode()->IsRootNode()) {
                        if (!(node->GetParentNode()->GetObjOffsetRot() == IdentQuat())) {
                            Quat q;
                            UndoParentsOffset(node, pkey, q);
                        }
                    }
                    pKeys[i] = pkey;
                    pFrames[i] = i;
                }
               	break;

			case KEY_SCL:
                sKeys[i] = s;
                sFrames[i] = i;
    			break;

            case KEY_ROT:
                {
                    if (i) {
                        rKeys[i] = a;
                        rFrames[i] = i;
                    } else {
                        AngAxis aa;
                        Quat qkey = parts.q;
                     // remove the pivot's rotation
                        Quat qOff = qkey / Inverse(node->GetObjOffsetRot());
                     // remove the parents rotation
                        if (!node->GetParentNode()->IsRootNode()) {
                            if (!(node->GetParentNode()->GetObjOffsetRot() == IdentQuat())) {
                                Point3 p;
                                UndoParentsOffset(node, p, qOff);
                            }
                        }
                        AngAxisFromQ(qOff, &aa.angle, aa.axis);
                        rKeys[i] = aa;
                        rFrames[i] = i;
                    }
                }
                break;
		}
    }

 // reduce the keys
    if (frames >= 3) {
	    BOOL* used = new BOOL[frames];
		//SS 6/23/2003 defect 463143: It would seem the original intent was to initialize
		// the boolean array. Initializing to TRUE fixes the bug---start with the assumption
		// that we need all the keys, then reduce from there.
        //for (i = 0; i < frames; i++) used[i];
        for (i = 0; i < frames; i++)
			used[i] = TRUE;
        Point3 pPt0, qPt0;
        AngAxis pA0, qA0; 
        for (i = 1; i < frames; i++) {
            switch (type)
            {
                case KEY_POS:
                    pPt0 = pKeys[i];
                    qPt0 = pKeys[i - 1];
                    if (ApproxEqual(pPt0.x, qPt0.x) &&
                        ApproxEqual(pPt0.y, qPt0.y) &&
                        ApproxEqual(pPt0.z, qPt0.z))
                        used[i] = FALSE;
                    else {
                        used[i - 1] = TRUE;
                    }
               	    break;

			    case KEY_SCL:
                    pPt0 = sKeys[i];
                    qPt0 = sKeys[i - 1];
                    if (ApproxEqual(pPt0.x, qPt0.x) &&
                        ApproxEqual(pPt0.y, qPt0.y) &&
                        ApproxEqual(pPt0.z, qPt0.z))
                        used[i] = FALSE;
                    else {
                        used[i - 1] = TRUE;
                    }
                    break;

                case KEY_ROT:
                    pA0 = rKeys[i];
                    qA0 = rKeys[i - 1];
                    if (ApproxEqual(pA0.axis.x, qA0.axis.x) &&
                        ApproxEqual(pA0.axis.x, qA0.axis.x) &&
                        ApproxEqual(pA0.axis.x, qA0.axis.x) &&
                        ApproxEqual(pA0.angle,  qA0.angle))
                        used[i] = FALSE;
                    else {
                        used[i - 1] = TRUE;
                    }
                    break;
		    }
        }

        int j = 0;
		//SS 6/23/2003: This doesn't do anything, and seems as though it was used when
		// debugging this code. I've removed it.
        //for (i = 0; i < frames; i++) {
        //    if (used[i]) j++;
        //}
        //j = 0;
        for (i = 0; i < frames; i++) {
            if (used[i]) {
                switch (type)
                {
                    case KEY_POS:
                        pKeys[j] = pKeys[i];
                        pFrames[j] = pFrames[i];
                        j++;
               	    break;

    			    case KEY_SCL:
                        sKeys[j] = sKeys[i];
                        sFrames[j] = sFrames[i];
                        j++;
           			break;

                    case KEY_ROT:
                        rKeys[j] = rKeys[i];
                        rFrames[j] = rFrames[i];
                        j++;
                    break;
		        }

            }
        }
        switch (type)
        {
            case KEY_POS:
                pKeys.SetCount(j);
                pFrames.SetCount(j);
                break;

    	    case KEY_SCL:
                sKeys.SetCount(j);
                sFrames.SetCount(j);
       			break;

            case KEY_ROT:
                rKeys.SetCount(j);
                rFrames.SetCount(j);
                break;
        }

        delete [] used;
    }

 // Write trackheader stuff
    short trackflags = 0;
    WRSHORT(&trackflags);
    long lwork = 0;
    WRLONG(&lwork);
    lwork = frames - 1;   //gdf FIXME "0"
    WRLONG(&lwork);

    switch (type) 
    {
        case KEY_POS:
            {
                long nkeys = pFrames.Count();
                WRLONG(&nkeys);
             // write the keys
                for (i = 0; i < pFrames.Count(); i++) {
                    WRLONG(&pFrames[i]);
                 // Write flags
		            short wflags = 0;			
		            WRSHORT(&wflags);
	             // Write values
                    WRFLOAT(&pKeys[i].x);
                    WRFLOAT(&pKeys[i].y);
                    WRFLOAT(&pKeys[i].z);
                }
            }
           	break;

        case KEY_SCL:
            {
                long nkeys = sFrames.Count();
                WRLONG(&nkeys);
             // write the keys
                for (i = 0; i < sFrames.Count(); i++) {
                    WRLONG(&sFrames[i]);
                 // Write flags
		            short wflags = 0;			
		            WRSHORT(&wflags);
	             // Write values
                    WRFLOAT(&sKeys[i].x);
                    WRFLOAT(&sKeys[i].y);
                    WRFLOAT(&sKeys[i].z);
                }
            }
            break;

        case KEY_ROT:
            {
                long nkeys = rFrames.Count();
                WRLONG(&nkeys);
             // write the keys
                for (i = 0; i < rFrames.Count(); i++) {
                    WRLONG(&rFrames[i]);
                 // Write flags
		            short wflags = 0;			
		            WRSHORT(&wflags);
	             // Write values
                    WRFLOAT(&rKeys[i].angle);
			        WRFLOAT(&rKeys[i].axis.x);
			        WRFLOAT(&rKeys[i].axis.y);
			        WRFLOAT(&rKeys[i].axis.z)
                }
            }
            break;
    }
	return TRUE;
}

// gdf write the tracks
static BOOL
WriteController(INode* node) {

    Interval valid;
    TimeValue start = theSceneEnum->i->GetAnimRange().Start();
    unsigned short cTag;
    long cPtr;
    long cBegin;

    int Track_Flags;
    int Force_Keys;

    Class_ID cid;
    
    Control* cpos = node->GetTMController()->GetPositionController();
	Control* crot = node->GetTMController()->GetRotationController();
	Control* cscl = node->GetTMController()->GetScaleController();

    cid = node->GetTMController()->ClassID();
    if (cid == Class_ID(PRS_CONTROL_CLASS_ID, 0)) {
        assert(cpos && crot && cscl);
        Track_Flags = 0;
        Force_Keys  = 0;
     // this is a special case (path controller) modifies both pos and rot
        Class_ID cID = cpos->ClassID();
        if (cID == Class_ID(HYBRIDINTERP_COLOR_CLASS_ID, 0)) Force_Keys |= KEYS_ROT;
    } else {
     // if it's not a PRS controller (transform controller) try to write PRS
        Track_Flags = KEYS_POS | KEYS_ROT | KEYS_SCL;
        Force_Keys  = 0;
    }

 // write POSITION Track keys
    cTag = POS_TRACK_TAG;
    WriteChunkBegin(cTag, cPtr, cBegin);
    if (!cpos || (Force_Keys & KEYS_POS)) {
     // this is a transform controller that doesn't support
     // support GetPositionController()
        if (WriteLinearKeysChunk(node, KEY_POS)) {
            Track_Flags &= ~KEYS_POS;
        }
	} else {
        if (NeedsKeys(cpos->NumKeys()) || (Track_Flags & KEYS_POS)) {
            if (IsTCBControl(cpos)) {
                if (WriteTCBKeysChunk(node, cpos, KEY_POS)) {
                    Track_Flags &= ~KEYS_POS;
                }
            } else {
                if (WriteLinearKeysChunk(node, KEY_POS)) {
                    Track_Flags &= ~KEYS_POS;
                }
            }
        } else {
            Point3 pkey;
            cpos->GetValue(start, &pkey, valid);
            if (!node->GetParentNode()->IsRootNode()) {
                if (!(node->GetParentNode()->GetObjOffsetRot() == IdentQuat())) {
                    Quat q;
                    UndoParentsOffset(node, pkey, q);
                }
            }
            
            if (!WriteStd1KeyHdr()) return 0;
            WRFLOAT(&pkey.x);
			WRFLOAT(&pkey.y);
			WRFLOAT(&pkey.z);
            Track_Flags &= ~KEYS_POS;
		}
     
    }
    WriteChunkEnd(cPtr, cBegin);

 // write ROTATION Track keys
    cTag = ROT_TRACK_TAG;
    WriteChunkBegin(cTag, cPtr, cBegin);
    if (!crot || (Force_Keys & KEYS_ROT)) {
     // this is a transform controller that doesn't support
	 // support GetRotationController()
        if (WriteLinearKeysChunk(node, KEY_ROT)) {
            Track_Flags &= ~KEYS_ROT;
        }
	} else {
        if (NeedsKeys(crot->NumKeys()) || (Track_Flags & KEYS_ROT)) {
            if (IsTCBControl(crot)) {
                if (WriteTCBKeysChunk(node, crot, KEY_ROT)) {
                    Track_Flags &= ~KEYS_ROT;
                }
            } else {
                if (WriteLinearKeysChunk(node, KEY_ROT)) {
                   Track_Flags &= ~KEYS_ROT;
                }
            }
        } else {
            Quat rkey;
            AngAxis aa;
            crot->GetValue(start, &rkey, valid);
         // remove the pivot's rotation
            Quat qOff = rkey / Inverse(node->GetObjOffsetRot());
         // remove the parents rotation
            if (!node->GetParentNode()->IsRootNode()) {
                if (!(node->GetParentNode()->GetObjOffsetRot() == IdentQuat())) {
                    Point3 p;
                    UndoParentsOffset(node, p, qOff);
                }
            }
            if (!WriteStd1KeyHdr()) return 0;
            AngAxisFromQ(qOff, &aa.angle, aa.axis);
            WRFLOAT(&aa.angle);
			WRFLOAT(&aa.axis.x);
            WRFLOAT(&aa.axis.y);
            WRFLOAT(&aa.axis.z);
			Track_Flags &= ~KEYS_ROT;
	    }
    }
    WriteChunkEnd(cPtr, cBegin);

 // write SCALE Track keys    
    cTag = SCL_TRACK_TAG;
    WriteChunkBegin(cTag, cPtr, cBegin);

    if (!cscl || (Force_Keys & KEYS_SCL)) {
     // this is a transform controller that doesn't support
	 // support GetScaleController()
        if (WriteLinearKeysChunk(node, KEY_SCL)) {
            Track_Flags &= ~KEYS_SCL;
        }
	} else {
        if (NeedsKeys(cscl->NumKeys()) || (Track_Flags & KEYS_SCL)) {
            if (IsTCBControl(cscl)) {
                if (WriteTCBKeysChunk(node, cscl, KEY_SCL)) {
                    Track_Flags &= ~KEYS_SCL;
                }
            } else {
                if (WriteLinearKeysChunk(node, KEY_SCL)) {
                    Track_Flags &= ~KEYS_SCL;
                }
            }
        } else {
            ScaleValue skey;
            cscl->GetValue(start, &skey, valid);
            if (!WriteStd1KeyHdr()) return 0;
            WRFLOAT(&skey.s.x);
		    WRFLOAT(&skey.s.y);
		    WRFLOAT(&skey.s.z);
            Track_Flags &= ~KEYS_SCL;
	    }
    }
    WriteChunkEnd(cPtr, cBegin);
    
    return TRUE;
}

// gdf convert MAX UVFaces into .3ds faces by adding extra 3ds verts
// this is just to get the data into .3ds
void
ConvertTo3DSFaces(ObjectEntry* oe, Tab<Vert3ds>& verts, Tab<Face3ds>& faces)
{
    Face3ds tmpFace;
    Vert3ds tmpVert;
    Mesh mesh = oe->tri->GetMesh();
    int numVerts = ((mesh.getNumVerts() < 65535) ? mesh.getNumVerts() : 65535);
 
	BitArray vWritten;
    vWritten.SetSize(mesh.getNumVerts());
    vWritten.ClearAll();

    verts.SetCount(mesh.getNumVerts());
    faces.SetCount(mesh.getNumFaces());

	for (int i = 0; i < mesh.getNumFaces(); i++) {
        Face   face   = mesh.faces[i];
        TVFace tvFace = mesh.tvFace[i];

        for (int k = 0; k < 3; k++) {
            tmpFace.vNum[k] = face.v[k];
        }
        tmpFace.flags = face.flags;

        for (int j = 0; j < 3; j++) {
            UVVert tVert = mesh.tVerts[tvFace.t[j]];
            Point3 vert  = mesh.verts[face.v[j]];
            tmpVert.pt    = vert;
            tmpVert.tv    = tVert;

            if (vWritten[face.v[j]]) {
                if (!UVVertEqual(verts[face.v[j]].tv, tmpVert.tv)) {
                    tmpFace.vNum[j] = verts.Count();
                    verts.Insert(verts.Count(), 1, &tmpVert);
                }
            } else {
                verts[face.v[j]] = tmpVert;
                vWritten.Set(face.v[j]);
            }
        }

        faces[i] = tmpFace;
    }
}
