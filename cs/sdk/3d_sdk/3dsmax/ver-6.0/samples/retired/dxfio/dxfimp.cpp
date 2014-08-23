/**********************************************************************
 *<
	FILE: dxfimp.cpp

	DESCRIPTION:  .DXF file import module

	CREATED BY: Tom Hudson

	HISTORY: created 27 November 1995

 *>	Copyright (c) 1995, All Rights Reserved. 
 **********************************************************************/

#include "Max.h"
#include "splshape.h"
#include "dxfires.h"

#ifndef UNDEFINED
#define UNDEFINED	0xffffffff
#endif

#define MAXMID 50

// Release 13 fix
#define R13FIX

// Debugging switches:
//#define DBGDXF
//#define DBGDXF1
//#define DBGBULG
//#define DBGNO
//#define DBGCOL
//#define DBGFINAL

//#define IS_3DSDXF

static BOOL showPrompts;

// ARBAXIS.C stuff...
typedef double point[3];	   /* Three dimensional point */
typedef double vector[4];	   /* Homogeneous coordinate vector */
typedef double matrix[4][4];   /* Transformation matrix */
extern void geta4by4(point normaxis, matrix mat);
// End of ARBAXIS stuff

static void NormTrans(Point3& p, Point3 norm);
static void	RemoveDoubleFaces(Mesh *m);

// Here are the 256 ACI (Autodesk Color Index) colors:
struct ObjectColors {
	BYTE	r, g, b;
}; 

inline DWORD DWORD_FROM_OC(ObjectColors x) {
	return (x.r | (x.g << 8) | (x.b << 16));
	}

static ObjectColors aciColors[256] = {
    {0, 0, 0}, {250, 0, 0}, {250, 250, 0}, {0, 250, 0}, {0, 250, 250},
    {0, 0, 250}, {250, 0, 250}, {250, 250, 250}, {135, 135, 135},
    {203, 203, 203}, {255, 0, 0}, {255, 127, 127}, {165, 0, 0}, {165, 82, 82},
    {127, 0, 0}, {127, 63, 63}, {76, 0, 0}, {76, 38, 38}, {38, 0, 0},
    {38, 19, 19}, {255, 63, 0}, {255, 159, 127}, {165, 41, 0}, {165, 103, 82},
    {127, 31, 0}, {127, 79, 63}, {76, 19, 0}, {76, 47, 38}, {38, 9, 0},
    {38, 23, 19}, {255, 127, 0}, {255, 191, 127}, {165, 82, 0}, {165, 124, 82},
    {127, 63, 0}, {127, 95, 63}, {76, 38, 0}, {76, 57, 38}, {38, 19, 0},
    {38, 28, 19}, {255, 191, 0}, {255, 223, 127}, {165, 124, 0},
    {165, 145, 82}, {127, 95, 0}, {127, 111, 63}, {76, 57, 0}, {76, 66, 38},
    {38, 28, 0}, {38, 33, 19}, {255, 255, 0}, {255, 255, 127}, {165, 165, 0},
    {165, 165, 82}, {127, 127, 0}, {127, 127, 63}, {76, 76, 0}, {76, 76, 38},
    {38, 38, 0}, {38, 38, 19}, {191, 255, 0}, {223, 255, 127},
    {124, 165, 0}, {145, 165, 82}, {95, 127, 0}, {111, 127, 63}, {57, 76, 0},
    {66, 76, 38}, {28, 38, 0}, {33, 38, 19}, {127, 255, 0}, {191, 255, 127},
    {82, 165, 0}, {124, 165, 82}, {63, 127, 0}, {95, 127, 63}, {38, 76, 0},
    {57, 76, 38}, {19, 38, 0}, {28, 38, 19}, {63, 255, 0}, {159, 255, 127},
    {41, 165, 0}, {103, 165, 82}, {31, 127, 0}, {79, 127, 63}, {19, 76, 0},
    {47, 76, 38}, {9, 38, 0}, {23, 38, 19}, {0, 255, 0}, {127, 255, 127},
    {0, 165, 0}, {82, 165, 82}, {0, 127, 0}, {63, 127, 63}, {0, 76, 0},
    {38, 76, 38}, {0, 38, 0}, {19, 38, 19}, {0, 255, 63}, {127, 255, 159},
    {0, 165, 41}, {82, 165, 103}, {0, 127, 31}, {63, 127, 79}, {0, 76, 19},
    {38, 76, 47}, {0, 38, 9}, {19, 38, 23}, {0, 255, 127}, {127, 255, 191},
    {0, 165, 82}, {82, 165, 124}, {0, 127, 63}, {63, 127, 95}, {0, 76, 38},
    {38, 76, 57}, {0, 38, 19}, {19, 38, 28}, {0, 255, 191}, {127, 255, 223},
    {0, 165, 124}, {82, 165, 145}, {0, 127, 95}, {63, 127, 111}, {0, 76, 57},
    {38, 76, 66}, {0, 38, 28}, {19, 38, 33}, {0, 255, 255}, {127, 255, 255},
    {0, 165, 165}, {82, 165, 165}, {0, 127, 127}, {63, 127, 127}, {0, 76, 76},
    {38, 76, 76}, {0, 38, 38}, {19, 38, 38}, {0, 191, 255}, {127, 223, 255},
    {0, 124, 165}, {82, 145, 165}, {0, 95, 127}, {63, 111, 127}, {0, 57, 76},
    {38, 66, 76}, {0, 28, 38}, {19, 33, 38}, {0, 127, 255}, {127, 191, 255},
    {0, 82, 165}, {82, 124, 165}, {0, 63, 127}, {63, 95, 127}, {0, 38, 76},
    {38, 57, 76}, {0, 19, 38}, {19, 28, 38}, {0, 63, 255}, {127, 159, 255},
    {0, 41, 165}, {82, 103, 165}, {0, 31, 127}, {63, 79, 127}, {0, 19, 76},
    {38, 47, 76}, {0, 9, 38}, {19, 23, 38}, {0, 0, 255}, {127, 127, 255},
    {0, 0, 165}, {82, 82, 165}, {0, 0, 127}, {63, 63, 127}, {0, 0, 76},
    {38, 38, 76}, {0, 0, 38}, {19, 19, 38}, {63, 0, 255}, {159, 127, 255},
    {41, 0, 165}, {103, 82, 165}, {31, 0, 127}, {79, 63, 127}, {19, 0, 76},
    {47, 38, 76}, {9, 0, 38}, {23, 19, 38}, {127, 0, 255}, {191, 127, 255},
    {82, 0, 165}, {124, 82, 165}, {63, 0, 127}, {95, 63, 127}, {38, 0, 76},
    {57, 38, 76}, {19, 0, 38}, {28, 19, 38}, {191, 0, 255}, {223, 127, 255},
    {124, 0, 165}, {145, 82, 165}, {95, 0, 127}, {111, 63, 127}, {57, 0, 76},
    {66, 38, 76}, {28, 0, 38}, {33, 19, 38}, {255, 0, 255}, {255, 127, 255},
    {165, 0, 165}, {165, 82, 165}, {127, 0, 127}, {127, 63, 127}, {76, 0, 76},
    {76, 38, 76}, {38, 0, 38}, {38, 19, 38}, {255, 0, 191}, {255, 127, 223},
    {165, 0, 124}, {165, 82, 145}, {127, 0, 95}, {127, 63, 111}, {76, 0, 57},
    {76, 38, 66}, {38, 0, 28}, {38, 19, 33}, {255, 0, 127}, {255, 127, 191},
    {165, 0, 82}, {165, 82, 124}, {127, 0, 63}, {127, 63, 95}, {76, 0, 38},
    {76, 38, 57}, {38, 0, 19}, {38, 19, 28}, {255, 0, 63}, {255, 127, 159},
    {165, 0, 41}, {165, 82, 103}, {127, 0, 31}, {127, 63, 79},  {76, 0, 19},
    {76, 38, 47}, {38, 0, 9}, {38, 19, 23}, {84, 84, 84}, {118, 118, 118},
    {152, 152, 152}, {186, 186, 186}, {220, 220, 220}, {255, 255, 255}
};

static char gp_buffer[256];

HINSTANCE hInstance;

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
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

static int Alert(int s1, int s2 = IDS_TH_DXFIMP, int option = MB_OK) {
	return MessageBox(s1, s2, option);
	}

#define no_RAM() Alert(IDS_TH_OUTOFMEMORY)

//-----------------------------------------------------------------------
struct NameSlot {
	TSTR name;
	int maxid;
	NameSlot *next;
	public:
	NameSlot(TCHAR *t, int id) { name = t; maxid = id; next = NULL; }
	};

NameSlot *nameList=NULL;

static void FreeNameList() {
	while (nameList) {
		NameSlot *ns = nameList;
		nameList = nameList->next;
		delete ns;
		}
	}

static void AddNameToNameList(TCHAR *nm) {
	NameSlot *ns = new NameSlot(nm,1);
	ns->next=nameList;
	nameList = ns;
	}

static void StripName(TCHAR* sname) {
	int n = _tcslen(sname)-1;
	while (n>=0 && sname[n]>=_T('0') && sname[n]<=_T('9') ) 
		sname[n--]=0;
	}

static void MakeNameUnique(TCHAR *name) {
	StripName(name); // strip off trailing numbers
	TCHAR buf[20];
	for (NameSlot *ns = nameList; ns!=NULL; ns = ns->next) {
		if (_tcscmp(name,ns->name.data())==0) {
			int i = ++ns->maxid;
			_stprintf(buf,"%02d\0",i);
			strcat(name,buf);
			return;
			}
		}

	AddNameToNameList(name);
	strcat(name,"01");
	}

//-----------------------------------------------------------------------


// Derive objects from...
#define OBJS_LAYER		0
#define OBJS_COLOR		1
#define OBJS_ENTITY		2
#define OBJS_FILMROLL	3

#define NUM_SOURCES 3 		// # of sources in dialog

class DXFImport : public SceneImport {
	friend INT_PTR CALLBACK ImportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static	int				objsFrom;					// Derive objects from...
	static	float			thresh;						// Weld threshold
	static	BOOL			weld;
	static	float			smoothAngle;				// Smooth angle
	static	BOOL			smooth;
	static	float			arcDegrees;
	static	float			arcSubDegrees;
	static	BOOL			removeDouble;
	static	BOOL			fillPolylines;
	static	BOOL			unifyNormals;
	static	ISpinnerControl* weldSpin;
	static	ISpinnerControl* smoothSpin;
	static	ISpinnerControl* arcSpin;
	static	ISpinnerControl* arcSubSpin;
	static  ImpInterface*   impInt;
	Interface		*gi;						// Generic interface
	long			fileLength;					// Length of the file
	int				lastpct;					// Last percentage displayed on file read
	int				pctcounter;					// Last percentage counter
					DXFImport();
					~DXFImport();
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

// Statics
ISpinnerControl*	DXFImport::weldSpin     = NULL;
ISpinnerControl*	DXFImport::smoothSpin     = NULL;
ISpinnerControl*	DXFImport::arcSpin     = NULL;
ISpinnerControl*	DXFImport::arcSubSpin     = NULL;
ImpInterface*   	DXFImport::impInt		=NULL;
int					DXFImport::objsFrom = OBJS_LAYER;			// Derive objects from...
float				DXFImport::thresh = 0.01f;					// Weld threshold
BOOL				DXFImport::weld = TRUE;
float				DXFImport::smoothAngle = 30.0f;				// Smooth angle
BOOL				DXFImport::smooth = TRUE;
float				DXFImport::arcDegrees = 10.0f;
float				DXFImport::arcSubDegrees = 90.0f;
BOOL				DXFImport::removeDouble = TRUE;
BOOL				DXFImport::fillPolylines = TRUE;
BOOL				DXFImport::unifyNormals = TRUE;

// Handy file class

class WorkFile {
private:
	FILE *stream;
	long length;	
public:
					WorkFile(const TCHAR *filename,const TCHAR *mode) { stream = NULL; Open(filename, mode); };
					~WorkFile() { Close(); };
	FILE *			Stream() { return stream; };
	void			Close() { if(stream) fclose(stream); stream = NULL; }
	void			Open(const TCHAR *filename,const TCHAR *mode);
	long			Length() { return length; }
	};

void WorkFile::Open(const TCHAR *filename,const TCHAR *mode) {
	Close();
	stream = _tfopen(filename,mode);
	if(stream) {
		fseek(stream, 0, SEEK_END);
		length = ftell(stream);
		fseek(stream, 0, SEEK_SET);
		}
	else
		length = 0; 
#ifdef DBGDXF
		DebugPrint("File length:%d\n",length);
#endif
	}

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
			//MessageBox(NULL,_T("DXFIMP.DLL: DllMain"),_T("DXFIMP"),MB_OK);
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

class DXFClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new DXFImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_AUTOCAD); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0xd1f,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEIMPORT);  }
	};

static DXFClassDesc DXFDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_DXFIMPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &DXFDesc; break;
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
// .3DS import module functions follow:
//

DXFImport::DXFImport() {
	lastpct = -1;
	pctcounter = 100;
	}

DXFImport::~DXFImport() {
	}

int
DXFImport::ExtCount() {
	return 1;
	}

const TCHAR *
DXFImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("DXF");
		}
	return _T("");
	}

const TCHAR *
DXFImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_AUTOCADDXFFILE);
	}
	
const TCHAR *
DXFImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_AUTOCAD);
	}

const TCHAR *
DXFImport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
DXFImport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
DXFImport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
DXFImport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
DXFImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
DXFImport::ShowAbout(HWND hWnd) {			// Optional
 	}

static int deriveButtons[] = { IDC_OBJ_LAYER, IDC_OBJ_COLOR, IDC_OBJ_ENTITY };

static INT_PTR CALLBACK
ImportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static DXFImport *imp;

	switch(message) {
		case WM_INITDIALOG:
			imp = (DXFImport *)lParam;
			CheckDlgButton( hDlg, deriveButtons[imp->objsFrom], TRUE);
			CheckDlgButton( hDlg, IDC_WELD, imp->weld);
			CheckDlgButton( hDlg, IDC_AUTOSMOOTH, imp->smooth);
			CheckDlgButton( hDlg, IDC_REMOVEDOUBLES, imp->removeDouble);
			CheckDlgButton( hDlg, IDC_FILLPOLYLINES, imp->fillPolylines);
			CheckDlgButton( hDlg, IDC_UNIFYNORMALS, imp->unifyNormals);

			imp->weldSpin = SetupUniverseSpinner(hDlg, IDC_WELDSPINNER, IDC_WELDENTRY,0,999999,imp->thresh);
			imp->smoothSpin = SetupFloatSpinner(hDlg,IDC_SMOOTHSPINNER, IDC_SMOOTHENTRY, 0,360,imp->smoothAngle);
			imp->arcSpin = SetupFloatSpinner(hDlg,IDC_ARCSPINNER, IDC_ARCENTRY, .001f, 90.0f, imp->arcDegrees);
			imp->arcSubSpin = SetupFloatSpinner(hDlg,IDC_ARCSUB_SPIN, IDC_ARCSUB, 10.0f,180.0f, imp->arcSubDegrees);

			CenterWindow(hDlg,GetParent(hDlg));
			return FALSE;
		case WM_DESTROY:
			if ( imp->weldSpin ) {
				ReleaseISpinner(imp->weldSpin);
				imp->weldSpin = NULL;
				}
			if ( imp->smoothSpin ) {
				ReleaseISpinner(imp->smoothSpin);
				imp->smoothSpin = NULL;
				}
			if ( imp->arcSpin ) {
				ReleaseISpinner(imp->arcSpin);
				imp->arcSpin = NULL;
				}
			if ( imp->arcSubSpin ) {
				ReleaseISpinner(imp->arcSubSpin);
				imp->arcSubSpin = NULL;
				}
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
					// Unload values into DXFImport statics
					imp->thresh = imp->weldSpin->GetFVal();
					imp->smoothAngle = imp->smoothSpin->GetFVal();
					imp->arcDegrees = imp->arcSpin->GetFVal();
					imp->arcSubDegrees = imp->arcSubSpin->GetFVal();
					imp->weld = IsDlgButtonChecked(hDlg, IDC_WELD);
					imp->smooth = IsDlgButtonChecked(hDlg, IDC_AUTOSMOOTH);
					imp->removeDouble = IsDlgButtonChecked(hDlg, IDC_REMOVEDOUBLES);
					imp->fillPolylines = IsDlgButtonChecked(hDlg, IDC_FILLPOLYLINES);
					imp->unifyNormals = IsDlgButtonChecked(hDlg, IDC_UNIFYNORMALS);
					for(int i = 0; i < NUM_SOURCES; ++i) {
						if(IsDlgButtonChecked(hDlg, deriveButtons[i])) {
							imp->objsFrom = i;
							break;
							}
						}
					EndDialog(hDlg, 1);
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		}
	return FALSE;
	}

// Some strings may be longer than this limit; the string reading functions below
// will truncate strings to this limit, and attempt to read past the remainder of
// the string.
#define MAXGTXT 256
#define EOS   0  

// Import types
#define DXF3D 1		// The only one at present

// TRUE if we're inside an insert block
BOOL inInsert = FALSE;

// Forward references
extern short ingroup();
extern int layer_color(char *name);
extern int read_entities(int skiphdr);

// Here's some code which mimics the way 3DS worked -- We keep a list of Mesh objects along with names
// for them.  This list is used by the import module and when it's all done, we dump the meshes into
// the Max database.

// Types (also used in DXFNode):
#define DXF_MESH 0
#define DXF_SHAPE 1

class NamedObject {
	public:
		Mesh mesh;
		BezierShape shape;
		TSTR name;
		int type;
		int color;			// The ACI color index
		NamedObject *next;
		NamedObject(char *n, int t, int c);
		~NamedObject();
	};

NamedObject::NamedObject(char *n, int t, int c) {
	name = n;
	type = t;
	color = c;
	if(type == DXF_SHAPE)
		shape.NewSpline();
	next = NULL;
 	}

NamedObject::~NamedObject() {
	if(type == DXF_SHAPE)
		shape.UpdateSels();
	}

class NamedObjects {
	public:
		NamedObject *head;
		NamedObject *tail;
		int count;
		NamedObjects() { head = tail = NULL; count = 0; }
		~NamedObjects() { DeleteAll(); }
		void DeleteAll();
		int Find(char *name);
		int Add(char *name, int type, int color);
		void Delete(int index);
		int Count() { return count; }
		NamedObject *GetNamedPtr(int index);
	};

void NamedObjects::DeleteAll() {
	NamedObject *next = head;
	while(next) {
		NamedObject *del = next;
		next = del->next;
		delete del;
		}
	head = tail = NULL;
	count = 0;
#ifdef DBGNO
DebugPrint("Deleted all named objects\n");
#endif
	}

int NamedObjects::Find(char *name) {
	NamedObject *ptr = head;
	int index = 0;
	while(ptr) {
		if(strcmp(name, ptr->name) == 0)
			return index;
		index++; 
		ptr = ptr->next;
		}
	return -1;
	}

int NamedObjects::Add(char *name, int type, int color) {
//	if(Find(name) >= 0)
//		return -1;
	NamedObject *n = new NamedObject(name, type, color);
	if(tail)
		tail->next = n;
	tail = n;
	if(!head)
		head = n;
	count++;
#ifdef DBGNO
DebugPrint("Added named object #%d: [%s], type %d, color %d\n",count, name, type, color);
#endif
	return count-1;
	}

void NamedObjects::Delete(int index) {
	assert(index < count);
	NamedObject *ptr = head;
	NamedObject *prev = NULL;
	while(index) {
		if(!ptr) {
			assert(0);
			return;
			}
		prev = ptr;
		ptr = ptr->next;
		index--;
		}
	if(prev)
		prev->next = ptr->next;
	else
		head = ptr->next;
	count--;
	if(tail == ptr)
		tail = prev;
	delete ptr;
#ifdef DBGNO
DebugPrint("Deleted named object #%d\n",index);
#endif
	}

NamedObject *NamedObjects::GetNamedPtr(int index) {
	assert(index < count);
	NamedObject *ptr = head;
	while(index) {
		if(!ptr) {
			assert(0);
			return NULL;
			}
		ptr = ptr->next;
		index--;
		}
	return ptr;
	}

static NamedObjects namedObjs;

static BOOL AddVerts(Mesh *mesh, int add) {
	if(mesh->setNumVerts(mesh->getNumVerts() + add, TRUE))
		return 1;
	return -1;
	}

static BOOL AddFaces(Mesh *mesh, int add) {
	if(mesh->setNumFaces(mesh->getNumFaces() + add, TRUE))
		return 1;
	return -1;
	}

typedef struct {
	Point3 p;
	float wstrt;
	Point3 a0,b0;	/* start corners of quadrilateral */
	Point3 a1,b1;	/* end corners of quadrilateral */
	Point3 ai,bi;	/* intersection points */
	} PLPoint;

DXFImport *theImport;
static short anyWide;
static Point3 pnorm;
static float lastbulge=0.0f;
static int inpoly=0,inrat=0,insmooth=0,inflat=0,mclosed=0,nclosed=0;
static int mcount,ncount;
static float pl_thick;
static short nvper,isThick;
static int dxf_type,entity,last_entity;
static int vertbase;
static int dxfobj,dxfverts,startvert,dxffaces;
static int meshvert,meshface;
static int filmroll_color;
static int pverts=0;
static Point3 normal;             /* Extrusion direction */
static Matrix3 normtm;
static char binsent[30];           /* Binary DXF file sentinel buffer */
static BOOL binary;
static BOOL inShape;				// TRUE=making shape, FALSE=making mesh
static BOOL errored;
static int entnum;
static short gcode;                /* Group code */
static short gint;                 /* Group integer value */
static double greal;                 /* Group double value */
static FILE *fp;
static short isEntityTM;
static double plsw, plew;            /* Polyline default start and end width */
static BOOL isR13;
static int chknum=0;
static char ename[200];             /* Entity type name */
static Mesh *dxf_mesh;				// Working mesh for DXF import
static BezierShape *dxf_shape;		// Working shape for DXF import
static Spline3D *dxf_spline;		// Working spline
static NamedObject *dxf_n;
static int nplverts;
static short polyClosed;
static PLPoint plq[2];  /* previous 2 pline segs */
static PLPoint pl0;  /* first pline segment */
static short isSpline,splineType;
static short is_3d_pline=0;

// Work buffers
static char *inbuf;                /* General purpose text buffer */
static char *gtextv;               /* Group text value */
static char *tgroup[10];           /* Group store table for text */
static double *rgroup;               /* Group store table for reals */
static short *igroup;              /* Group store table for integers */
static int bad_reason;

// Error codes
#define INVALID_FILE -1
#define PARTIAL_READ -2
#define USER_CANCEL -4

struct bylayer {
	char layername[40];
	short frozen;
	};

struct bycolor {
	short color;
	};

union lc {
	struct bylayer l;
	struct bycolor c;
	};

typedef struct dxfnlist {
	union lc lc;
	char objname[256];
	int type;
	struct dxfnlist *next;
	} DXFNode;

static DXFNode *LLIST=NULL;

typedef struct lofflist {
	char layername[40];
	struct lofflist *next;
	} LoffList;
static LoffList *offlayer=NULL;

typedef struct layer0 {
	char name[40];
	short color;
	struct layer0 *next;
	} Layer;
static Layer *onlayer=NULL;
	
typedef struct blockentry {
	char name[200];
	long fileAddr;
	struct blockentry *next;
	}  Block;

static BOOL EqualVectors(Point3 p1, Point3 p2) {
	return (p1 == p2) ? TRUE : FALSE;
	}

/*------------------- BLOCK CONTEXT ------------------- */
static short blockLevel = 0;
static Matrix3 curtm;
static Block *blockList=NULL, *curBlock=NULL, *lastBlock=NULL;
static char curLayer[40]=""; 	
static short curBlockCol;

size_t Read(FILE *fp, void *ptr, int count) {
	size_t result = fread(ptr, 1, count, fp);
	theImport->pctcounter++;
	if(theImport->pctcounter > 100) {
		theImport->pctcounter = 0;
		long pos = ftell(fp);
		int pct = (theImport->fileLength > 0) ? (pos * 100 / theImport->fileLength / 2) : 100;
		if(pct > theImport->lastpct) {
			theImport->lastpct = pct;
			theImport->gi->ProgressUpdate(pct);
			}				
		}
	return result;
	}

/* InRange: */
#define IR(low,high) ((gcode>=low)&&(gcode<high))

inline void PUTVERT(Mesh *t, int i, const Point3& v) {
#ifdef DBGDXF
	DebugPrint(" put_vert # %d, = %.7f,%.7f,%.7f \n", i, v.x,v.y,v.z);
#endif
	t->verts[i] = v;
	}

inline void PUTFACE(Mesh *t, int i, const Face& f) {
#ifdef DBGDXF2
	DebugPrint(" put_face # %d, = (%d,%d,%d)\n", i, f.v[0],f.v[1],f.v[2]);
#endif
	t->faces[i] = f;
	}

/*  DXFERR  --  Diagnose unspecific error in DXF file  */

static void dxferr() {
	errored = TRUE;
	/*        dumpgroup();*/
	}

void TransPt(Point3 &p) {
	if (blockLevel>0)
		p = p * curtm;
	}

static void ApplyTransform(Point3 &p) {
	if (isEntityTM)
		p = p * normtm;
	TransPt(p);
	}

int user_cancel() {
	if (errored) return(1);
	if(theImport->gi->GetCancel()) {
		bad_reason = USER_CANCEL;
		return 1;
		}
	return(0);
	}

/*  INENT  --  Read in next entity  */

static short inent(void) {
	short i;

	if (gcode != 0) {
		dxferr();
		return FALSE;
		}
	if (!strcmp(gtextv, "ENDSEC"))
		return FALSE;           /* End of entity section */
	if (!strcmp(gtextv, "ENDBLK"))
		return FALSE;           /* End of entity section */
	strcpy(ename, gtextv);

#ifdef DBGDXF1
	DebugPrint(" Entity = %s \n",ename);
#else

#ifdef IS_3DSDXF
	if ( ((++entnum)%10)==0) {
		int nv,nf;
		char oname[20];
		nv=nf=0;
		oname[0]=0;
		if (dxfobj>=0) {
			Namedobj *n = get_named_ptr(dxfobj);
			Tri_obj *t = n->dstruct;
			nv = t->verts;
			nf = t->faces;
			strcpy(oname,n->name);
//			DebugPrint(" Entity #%5d:%8s  Object \"%8s\"  V:%5d  F:%5d  #obs:%3d    \r",
//				entnum++,ename, oname,nv,nf,OBJECTS);
			}
//		else 
//			DebugPrint(" Entity #%5d: %8s          \r",entnum++,ename);
		}

#endif
#endif
	/* Supply defaults to fields  */

	igroup[62] = -1;  /* undefined color */
	igroup[67] = 0;
	rgroup[38] = 0.0;
	rgroup[39] = 0.0;
	isEntityTM = 0;
	normal = Point3(0,0,1);

	for (i = 0; i < 10; i++)
		tgroup[i][0] = EOS;
	if (!strcmp(ename, "TEXT")) {
		rgroup[50] = 0.0;
		rgroup[41] = 1.0;
		rgroup[51] = 0.0;
		igroup[71] = 0;
		igroup[72] = 0;
		} 
	else if (!strcmp(ename, "SHAPE")) {
		rgroup[50] = 0.0;
		rgroup[40] = 1.0;
		rgroup[51] = 0.0;
        } 
	else if (!strcmp(ename, "INSERT")) {
		igroup[66] = 0;
		rgroup[41] = rgroup[42] = rgroup[43] = 1.0;
		rgroup[50] = 0.0;
		igroup[70] = 1;
		igroup[71] = 1;
		rgroup[44] = 0.0;
		rgroup[45] = 0.0;
        } 
	else if (!strcmp(ename, "ATTDEF")) {
		igroup[73] = 0;
		rgroup[50] = 0.0;
		rgroup[41] = 1.0;
		rgroup[51] = 0.0;
		igroup[71] = 0;
		igroup[72] = 0;
        } 
	else if (!strcmp(ename, "ATTRIB")) {
		igroup[73] = 0;
		rgroup[50] = 0.0;
		rgroup[41] = 1.0;
		rgroup[51] = 0.0;
		igroup[71] = 0;
		igroup[72] = 0;
        } 
	else if (!strcmp(ename, "POLYLINE")) {
		igroup[75] = 0;
		igroup[70] = 0;
		rgroup[40] = 0.0;
		rgroup[41] = 0.0;
        }  
	else if (!strcmp(ename, "VERTEX")) {
		rgroup[40] = plsw;
		rgroup[41] = plew;
		rgroup[42] = 0.0;
		igroup[70] = igroup[71] = igroup[72] = igroup[73] = igroup[74] = 0;
		rgroup[50] = 0.0;
		}  
	else if (!strcmp(ename, "3DFACE")) {
		igroup[70] = 0;
		}

	while (TRUE) {
		if (!ingroup()) {
			dxferr();
			return FALSE;
			}
		if (gcode == 0)
			break;
		if (gcode < 10) {
			strncpy(tgroup[gcode], gtextv, MAXGTXT); //256);
			}
		else if (gcode < 60)                  /* reals */
			rgroup[gcode] = greal;
		else if (gcode >= 210 && gcode < 240) /* extrusion dirs */
			normal[gcode / 10 - 21] = (float)greal;
		else if (gcode >= 60 && gcode < 80)   /* ints */
			igroup[gcode] = gint;
		}   

#ifdef DBGCOL
	DebugPrint(" INENT: layer = %s, color = %d \n",tgroup[8],igroup[62]);
#endif

	/* If in a block, and the layer is "0", use the parent's layer */
	if ((blockLevel>0)&&(strcmp(tgroup[8],"0")==0)) {
		strcpy(tgroup[8],curLayer);
#ifdef DBGCOL
		DebugPrint(" Inheriting current layer = %s \n",curLayer);
#endif
		}

	if (igroup[62]==0) {		/* BYBLOCK */
		if (blockLevel>0) {
			igroup[62] = curBlockCol;
#ifdef DBGCOL 
			DebugPrint(" inheriting BLOCK color = %d \n",igroup[62]);
#endif
			}
		else 
			igroup[62] = 7;
		}
	else {
		if (igroup[62]==-1) {
			igroup[62]=layer_color(tgroup[8]);
#ifdef DBGCOL 
			DebugPrint(" INENT:inheriting LAYER (%s) color = %d \n",tgroup[8],igroup[62]);
#endif
			}
		}

#ifdef DBGCOL 
  	DebugPrint(" FINAL color = %d \n",igroup[62]);
#endif

	if (!strcmp(ename, "POLYLINE")) {
		plsw = rgroup[40];
		plew = rgroup[41];
		}

	return TRUE;
	}

#if 0
getStr(char *buf, int maxc, FILE *f) {
	char c;
	for(i=0; i<maxc; i++) {
		if(!fread(&c,1,1,f)) return 0;
		if (c==CR) continue;
		if (c==LF) 
		}	
	}
#endif	


#define NEWLINE 0xA
#define CR 0xD
static char cpeek; 
static char *getstr(FILE *file,  char *buffer,  LONG maxBytes) {
	int nBytes;
	char c;
	for (nBytes=0; nBytes<maxBytes-1;) {
		if(fread(&c,1,1,file)!=1) return NULL;
		if (c==NEWLINE) {
			buffer[nBytes] = 0;
			break;
			}
		if (c==CR) {
			buffer[nBytes] = 0;
			if(fread(&c,1,1,file)!=1) return buffer;
			if (c!=NEWLINE) {
				cpeek=c;
			  	fseek(file,-1,SEEK_CUR);
				}
			break;
			}
		buffer[nBytes++] = c;
		}
	return(buffer);
	}

/*  GETLINE  --  Obtain next line from input file  */

static BOOL getline() {
	inbuf[MAXGTXT-1]=0;
	if (getstr(fp,inbuf,MAXGTXT)==NULL)
		return FALSE;
//	if(fgets(inbuf,256,fp)==NULL)
//		return FALSE;
	int len = strlen(inbuf);
	if(inbuf[len-1] == '\n')
		inbuf[len-1] = 0;
	theImport->pctcounter++;
	if(theImport->pctcounter > 100) {
		theImport->pctcounter = 0;
		long pos = ftell(fp);
		int pct = pos * 100 / theImport->fileLength / 2;
		if(pct > theImport->lastpct) {
			theImport->lastpct = pct;
			theImport->gi->ProgressUpdate(pct);
			}				
		}
	return TRUE;
	}

/*  GETTYPE  --  Get the identifier (gcode) for the data item */

static int gettype(short *type)	{
	unsigned short t;
	if (isR13) {
		if (Read(fp,&t, 2)==2) { 
			*type = (short) t;
			return TRUE;
			}
		return FALSE;
		}
	else {
		BYTE b;
		if (Read(fp,&b, 1)==1) { 
			*type = (short) b;
			if (b==0xff) {
				if (Read(fp,type,2)==2) {
#ifdef DBGDXF
					DebugPrint(" Extended entity %d \n",*type);
#endif
					return(TRUE);
					}
				else
					return(FALSE);
				}
			return TRUE;
			}
		}
	return FALSE;
	}



static int GetCh() {
	char c;
	if (Read(fp, &c, 1)==1) return(c);
	return(0);
	}

/*  GETSTRING  --  Get a string from the binary file */

static void getstr(char *s) {
	char c;
	int n;
	for (n=0;n<MAXGTXT-1;n++) {
		if (Read(fp, &c, 1)!=1) c = 0;
		if (!((*s++)=c)) return;
		}
	if (c)	while ((c=GetCh())!=0) ;
	}

/*  GETSHORT  --  Get an integer (in 8086 order!) from the binary file */

static int getshort(short *ptr) {
	if (Read(fp,ptr, 2)==2) {
		return TRUE;
		}
	return FALSE;
	}

/*  GETREAL  --  Get a double (double) from the binary file in IEEE */

static int getreal(double *ptr) {
	if (Read(fp,ptr,8)==8) {
		return TRUE;
		}
	return FALSE;
	}


static TCHAR decimalSymbol = _T('.');
static TCHAR groupSymbol = _T(',');

static void GetDecSymbolFromRegistry() 
{
    char symbol[80];
	int bufSize = 80;
	if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL, symbol, bufSize) != 0 )
		decimalSymbol = symbol[0];
	if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STHOUSAND, symbol, bufSize) != 0 )
		groupSymbol = symbol[0];
}

TCHAR *FixDecimalSymbol(TCHAR *buf)
	{
	TCHAR *cp = buf;
	while(*cp) {
		if( *cp == _T('.'))
			*cp = decimalSymbol;
		else if (*cp == _T(','))
			*cp = groupSymbol;
		cp++;
		}
	return buf;
	}

//SS 11/14/2000: Abstract the DXF group code-data type association 
// into a function. DXF group codes based on AutoCAD 2000i spec.
// Comments regarding gcode changes are changes made in *this*
// submission; it would seem this code hasn't been touched since
// R13.
#define DXF_GCODE_BINARY	9000 // represented as unsigned chars
#define DXF_GCODE_STRING	9001
#define DXF_GCODE_REAL		9002
#define DXF_GCODE_LONG		9003
#define DXF_GCODE_SHORT		9004
#define DXF_GCODE_CHAR		9005

static bool IsType(short gcode, short datatype)
{
	if (datatype == DXF_GCODE_BINARY)
	{
		// No change from previous values.
		if (gcode==1004 || IR(310,320))
			return true;
	}
	else if (datatype == DXF_GCODE_STRING)
	{
		// 110-140 range changed to type real.
		// 390-400 range added.
		// 410-420 range added.
		// 999 added so that comments can be skipped.
		if (IR(0,10) || IR(100,110) || IR(300,310) || IR(320,370) || 
			IR(390,400) || IR(410,420) || IR(999,1004) || IR(1005,1010))
			return true;
	}
	else if (datatype == DXF_GCODE_REAL)
	{
		// 110-140 range added.
		// 150-170 range not in 2000i spec, but kept as legacy.
		if (IR(10,60) || IR(110,170) || IR(210,240) || IR(1010,1060))
			return true;
	}
	else if (datatype == DXF_GCODE_LONG)
	{
		// No change from previous values.
		if (gcode==1071 || IR(90,100))
			return true;
	}
	else if (datatype == DXF_GCODE_SHORT)
	{
		// 180-210 range not in 2000i spec, but kept as legacy.
		// 270-280 range not in 2000i spec, but kept as legacy.
		// 280-290 range added.
		// 1072-1080 range not in 2000i spec, but kept as legacy.
		if ( IR(60,80) || IR(170,210) || IR(270,290) || IR(370,390) || 
			IR(400,410) || IR(1060,1071) || IR(1072,1080))
			return true;
	}
	else if (datatype == DXF_GCODE_CHAR)
	{
		// 80-90 range not in 2000i spec, but kept as legacy.
		// 280-290 range changed to type short.
		// 290-300 range added.
		if (IR(80,90) || IR(290,300))
			return true;
	}
	// else, unknown range or mismatched type
	return false;
}

/*  INGROUP  --  Read in group from DXF file  */

static short ingroup() {
	short wcode = 0;

	if (((++chknum)&15)==0) if (user_cancel()) return FALSE;

	if (binary) {
		/* We're reading a binary file */
		if (!gettype(&gcode)) {	/* End of file */
			return FALSE;
			}
#ifdef DBGINGR
	  	DebugPrint(" Binary INGROUP, gcode = %d ",gcode);
#endif
		if (gcode < 0) { 
#ifdef DBGINGR
			DebugPrint(" Negative gcode: %d \n", gcode);
#endif
//SS 11/15/2000: Since we aren't able to read past the data, we have
// to error here; we can't just ignore it. The DXF import has no choice 
// but to cancel the import. However, negative group codes are not
// supposed to be found in DXF files.
//#ifdef R13FIX
//			return TRUE;
//#else 
			errored = TRUE; return FALSE; 
//#endif
			}
		//if (gcode==1004||IR(310,320)) {
		if (IsType(gcode, DXF_GCODE_BINARY)) {
			unsigned char c;
			Read(fp,&c, 1); /* read number of bytes */
			fseek(fp,c, 1);  /* skip over them */
			}
		//else if (IR(0,10)||IR(100,140)||IR(300,310)||IR(320,370)||IR(1000,1010)){
		else if (IsType(gcode, DXF_GCODE_STRING)) {
			getstr(gtextv);
#ifdef DBGINGR
		  	DebugPrint("getstr = %s \n", gtextv);
#endif
			}
		//else if (IR(10,60)||IR(140,170)||IR(210,240)||IR(1010,1060)) {
		else if (IsType(gcode, DXF_GCODE_REAL)) {
			getreal(&greal);
#ifdef DBGINGR
		  	DebugPrint(" greal = %.4f \n", greal);
#endif
			}
		//else if (gcode==1071||IR(90,100)) {
		else if (IsType(gcode, DXF_GCODE_LONG)) {
			long dum;
			Read(fp,&dum,4);
			}
		//else if ( IR(60,80)||IR(170,210)||IR(270,280)||IR(1070,1080)) {
		else if (IsType(gcode, DXF_GCODE_SHORT)) {
			getshort(&gint);
#ifdef DBGINGR
		  	DebugPrint(" gint = %d \n", gint);
#endif
			}
		//else if (IR(80,90)||IR(280,290)) {
		else if (IsType(gcode, DXF_GCODE_CHAR)) {
			char cdum;
			Read(fp,&cdum,1);
#ifdef DBGINGR
		  	DebugPrint(" char = %x \n", cdum);
#endif
			}					 
		else {              /* unknown gcode */
#ifdef DBGINGR
	  		DebugPrint(" unknown gcode = %d \n",wcode);
#endif
//SS 11/15/2000: Since we aren't able to read past the data, we have
// to error here; we can't just ignore it. The DXF import has no choice 
// but to cancel the import.
//#ifdef R13FIX
//			return TRUE;
//#else 
			errored = TRUE; return FALSE; 
//#endif
			}
#ifdef DBGINGR
	  	DebugPrint("\n");
#endif
        return TRUE;
   	} 
			
	else { /* We're reading an ASCII DXF file */

		// Get the line containing the group code.
		if (getline()) {
			if (sscanf(inbuf, "%hd", &gcode) != 1) {
				// This used to return errored = TRUE, but it
				// was causing files from "Animation Master" to fail to
				// load, because they have a bogus char or two at the end
				// of their file.  I hope this "fix" is harmless
				//     DS 3-14-96
				// errored = TRUE; 
				return FALSE;
				}
#ifdef DBGINGR
			DebugPrint("ASCII INGROUP, gcode = %d /",gcode);
#endif
			wcode=gcode;
			// Extended group code types match up, with the exception of
			// 1071.
			//FIXME: This seems worrysome to me; if an entity is expecting,
			// say, a start point at gcode 10, and also has some xdata at
			// gcode 1010, the xdata value may overwrite the 'real' 10.
			// But, since I don't know when this was added, and don't have
			// a defect on it yet, I'll leave it be.
			if(wcode >= 1000  &&  wcode != 1071)
				wcode -= 1000;
			if (wcode < 0) { 
#ifdef IS_3DSDXF
				DebugPrint(" Negative gcode: %d \n", gcode);
#endif
//SS 11/15/2000: If we return TRUE here, the gcode may be used to access
// into an array; we can't let this happen with a negative index.
//#ifdef R13FIX
//				return TRUE;
//#else 
				errored = TRUE; return FALSE; 
//#endif
				}
			// Get the line containing the value.
			if (!getline()) {
				errored = TRUE;
				return FALSE;
				}
#ifdef DBGINGR
		  	DebugPrint(" %s\n",inbuf);
#endif
			if (wcode < 10) {
				strcpy(gtextv, inbuf);
				}
			else if (wcode < 60) {
				FixDecimalSymbol(inbuf);
				if (sscanf(inbuf, "%lf", &greal) != 1) {
					errored = TRUE;
					return FALSE;
					}
				} 
			else if (wcode >= 210 && wcode < 240) {
				FixDecimalSymbol(inbuf);
				if (sscanf(inbuf, "%lf", &greal) != 1) {
					errored = TRUE;
					return FALSE;
					}
				} 
			else if (wcode >= 60 && wcode < 80) {
				if (sscanf(inbuf, "%hd", &gint) != 1) {
					errored = TRUE;
					return FALSE;
					}
				} 
			else if (wcode==999) {  /* Skip comment */
#ifdef DBGINGR
				DebugPrint("Comment: [%s]\n",inbuf); 
#endif
				}
			else if (wcode >= 140 && wcode < 150) { }
			else if (wcode >= 170 && wcode < 180) { }
			//else if (wcode >= 60 && wcode < 80) { } // handled above
			else if (wcode >= 1000) {	/* Ignore >1000 */  }
			else {  /* unknown gcode */
#ifdef DBGINGR
		  		DebugPrint(" unknown gcode = %d \n",wcode);
#endif
//SS 11/15/2000: This is really the only legitimate place we can run across
// an unknown gcode, and return TRUE. Reason being, we've already read in 
// the line from the file, so we're safe to "move on". In my tests, it didn't
// appear that information that was supposed to be retrieved here would be
// needed later.
#ifdef R13FIX
				return TRUE;
#else 
				errored = TRUE; return FALSE; 
#endif
				}
			return TRUE;
			}
		return FALSE;              /* End of file */
	    }
	} 

#ifdef DBGDXF
void prblocks(void) {
	Block *bl;
	DebugPrint("\n BLOCK LIST------- \n");
	for (bl=blockList; bl!=NULL; bl = bl->next) {
		DebugPrint("    Block [%X]   %s \n",bl->fileAddr, bl->name);
		}
	DebugPrint("--------------------------\n\n");
	}
#endif

/*  INBLOCK  --  Read in next block  */

static short inblock(void) {
	Block *bl;

	if (gcode != 0) { 
		dxferr();	
		return FALSE; 
		}

	if (!strcmp(gtextv, "ENDSEC"))
		return FALSE;	/* End of BLOCKS section */

	if (strcmp(gtextv, "BLOCK")) { dxferr(); return FALSE; }

	bl = (Block *)malloc(sizeof(Block));
	if (bl==NULL) return(FALSE);
	bl->name[0] = 0;
	bl->next = blockList;
	blockList = bl;
	bl->fileAddr = ftell(fp);

#ifdef DBGDXF
	DebugPrint(" INBLOCK, addr = %X \n",bl->fileAddr);
#endif
	while (!errored) {
		if (!ingroup()) {	dxferr();	return FALSE; }
		if (gcode == 0) break;
		if (gcode < 10)
			strncpy(tgroup[gcode], gtextv, MAXGTXT); //256); // text
		}   

	strcpy(bl->name,tgroup[2]);

	if (strcmp(gtextv,"ENDBLK")!=0) {
		while (!errored) {
			if (!ingroup()) {	dxferr();	return FALSE; }
			if (gcode == 0) {
				if (strcmp(gtextv,"ENDBLK")==0) break;
				}
			}   
		}

	/* Read in the ENDBLK entity; */
	while (!errored) {
		if (!ingroup()) { dxferr();	return FALSE; }
		if (gcode == 0) 
			break;
		}   

#ifdef DBGDXF
	DebugPrint(" exit INBLOCK, addr = %X, name =%s \n",bl->fileAddr,bl->name);
	prblocks();
#else
#ifdef IS_3DSDXF
	DebugPrint(" Block read: %s        \r",bl->name);
#endif
#endif
	return TRUE;
	}


/* READ_BLOCKS  */

int read_blocks(void) {
	int nbl=0;

	if (!errored && !ingroup())	{
		dxferr();
		return(0);
		}
	while (!errored && inblock())  nbl++ ;
	if(errored)
		return(0);
	return(1);
	}

/* READ_TABLES */

int read_tables(void) {
	struct lofflist *ol;

	if (!errored && !ingroup()) {
		dxferr();
		return(0);
		}
	while (!errored && inent())	{
		if (!strcmp(ename, "LAYER") && tgroup[2]!=NULL)	{
			if(igroup[62]<0 || (igroup[70] & 1)) {
				if((ol = (LoffList *)malloc(sizeof(LoffList)))==NULL) {
					if(showPrompts)
						no_RAM();
					return(0);
					}
				ol->next=offlayer;
				offlayer=ol;
				strcpy(ol->layername,tgroup[2]);
#ifdef DBGCOL 
				DebugPrint("*** OFF Layer %s\n",ol->layername);
#endif
				}
			else {
				Layer *layer;
				if((layer = (Layer *)malloc(sizeof(Layer)))==NULL) {
					if(showPrompts)
						no_RAM();
					return(0);
					}
				strcpy(layer->name,tgroup[2]);
				layer->color = igroup[62];
				layer->next = onlayer;
#ifdef DBGCOL 
				DebugPrint("*** Layer %s, color = %d \n",layer->name,layer->color);
#endif
				onlayer = layer;
				}
			}
		}
	if(errored)
		return(0);
	return(1);
	}

/*****
See if a given layer is being ignored
*****/

int ignore_layer(char *name) {
	struct lofflist *ol;
	
	if(name==NULL)
		return(0);
	
	ol=offlayer;
	while(ol != NULL) {
		if(strcmp(name,ol->layername)==0)
			return(1);
		ol=ol->next;
		}
	return(0);
	}

static int layer_color(char *name) {
	Layer *l;
	if(name==NULL)	return(0);
	for (l=onlayer; l!=NULL; l= l->next) 
		if(strcmp(name,l->name)==0)
			return(l->color);
	return(0);
	}

/***** Free up  lists *****/
static void free_lists(void)	{
	struct dxfnlist *lp,*nlp;
	struct lofflist *ol,*nol;
	Layer *layer,*nextlayer;
	Block *bl,*nbl;

	/* Free layer list */
	for (lp=LLIST; lp!=NULL; lp = nlp) {
		nlp = lp->next;
		free(lp);
		}
	LLIST=NULL;
	
	/* Free ignored layer list */
	for (ol=offlayer; ol!=NULL; ol = nol) {
		nol = ol->next;
		free(ol);
		}
	offlayer=NULL;

	
	/* Free active layer list */
	for (layer=onlayer; layer!=NULL; layer = nextlayer) {
		nextlayer = layer->next;
		free(layer);
		}
	onlayer=NULL;

	/* Free block list */
	for (bl = blockList; bl!=NULL; bl = nbl) {
		nbl = bl->next;
		free(bl);
		}
	blockList = NULL;
	}

/*------------ POINT ----------------*/
int dxfpoint(void)	{
	return(0);
	}

/*------------ VERT ----------------*/

void load_vert(Point3 &v, int n) {
	v.x = (float)rgroup[n];
	v.y = (float)rgroup[n+10];
	v.z = (float)rgroup[n+20];
	if (rgroup[38]!=0.0) v.z = (float)rgroup[38];
#ifdef DBGDXF
	DebugPrint("load_vert, vert = (%.5f, %.5f %.5f)\n",v.x,v.y,v.z);
#endif
	}

uchar acad_mtl(int colorwk) {
//	char tryname[32];
	/* color 255 is a legal ACAD color, but it is DEFAULT for 3DStudio:
		use 254 instead */
	if (colorwk==255) colorwk = 254; 
	if (colorwk>256||colorwk<=0) return(255);
//	sprintf(tryname,"%s%02d",progstr(KFST0078),colorwk);
//	return(inst_material(tryname,0));
	return 255;
	}

static uchar get_mtl_id() { 
	return(acad_mtl(igroup[62])); 
	}

typedef struct {
	Point3 p;
	float ws,bulge;
	} PVert;

typedef Tab<PVert> PVertTab;

PVertTab pvtab;
#define PV_EL(i)  (&pvtab[i])

void RecPlineVert(Point3 p, float ws, float bulge) {
	PVert pv;
	if (ws>0.0) anyWide = 1;
	pv.p = p;
	pv.ws = ws;
	pv.bulge = bulge;
	pvtab.Append(1, &pv);
	}

/* Find angle for given vector, and its 90-degree offset	*/
/* Returns angles in radians					*/
/* Returns 0 if angle indeterminate				*/

int
find_angle(float deltax,float deltay,float *angle,float *angle90) {
	float awk,a90wk;

	if(deltax==0.0f) {
		if(deltay==0.0f) {
			if(angle)
				*angle=0.0f;
			if(angle90)
				*angle90=0.0f;
			return(0);
			}
		if(deltay<0.0f)
			awk=PI + HALFPI;	// 270
		else
			awk=PI;				// 90
		goto get_90;
		}
	if(deltay==0.0f) {
		if(deltax<0.0f)
			awk=TWOPI;
		else
			awk=0.0f;
		goto get_90;
		}

	awk=(float)atan(deltay/deltax);

	if(deltax<0)
		awk+=PI;
	while(awk<0.0)
		awk+=TWOPI;
	while(awk>=TWOPI)
		awk-=TWOPI;

	get_90:
	a90wk=awk+HALFPI;
	while(a90wk>=TWOPI)
		a90wk-=TWOPI;

	if(angle)
		*angle=awk;
	if(angle90)
		*angle90=a90wk;
	return(1);
	}

/* Find the vector length for a circle segment	*/
/* Returns a unit value (radius=1.0)		*/
/* Angle expressed in radians			*/

static float
veccalc(float angstep) {
	static float lastin = -9999.0f,lastout;
	if(lastin == angstep)
		return lastout;

	float lo,hi,totdist;
	float sinfac=(float)sin(angstep),cosfac=(float)cos(angstep),test;
	int ix,count;
	Spline3D work;
	Point3 k1((float)cos(0.0f),(float)sin(0.0f),0.0f);
	Point3 k2(cosfac,sinfac,0.0f);

	hi=1.5f;
	lo=0.0f;
	count=200;

	/* Loop thru test vectors */

	loop:
	work.NewSpline();
	test=(hi+lo)/2.0f;
	Point3 out = k1 + Point3(0.0f, test, 0.0f);
	Point3 in = k2 + Point3(sinfac * test, -cosfac * test, 0.0f);

 	work.AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,k1,k1,out));
 	work.AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,k2,in,k2));

	totdist=0.0f;
	for(ix=0; ix<10; ++ix) {
		Point3 terp = work.InterpBezier3D(0,(float)ix/10.0f);
		totdist += (float)sqrt(terp.x * terp.x + terp.y * terp.y);
		}
	
	totdist /= 10.0f;
	count--;
	if(totdist==1.0f || count<=0)
		goto done;
	if(totdist>1.0f) {
		hi=test;
		goto loop;
		}
	lo=test;
	goto loop;

	done:
	lastin = angstep;
	lastout = test;
	return test;
	}

/*--------------------------------------------------
Process bulge information for a given segment and
convert to Bezier curve control vertices
-------------------------------------------------*/

class WorkKnot {
	public:
		Point3 p;
		Point3 in;
		Point3 out;
		WorkKnot() { p = in = out = Point3(0,0,0); }
		WorkKnot(Point3 p, Point3 i, Point3 o) { this->p = p; in = i; out = o; }
		WorkKnot(Spline3D *spline, int index);
	};

WorkKnot::WorkKnot(Spline3D *spline, int index) {
	p = spline->GetKnotPoint(index);
	in = spline->GetInVec(index);
	out = spline->GetOutVec(index);
	}

// NEW ALGORITHM ==========================================
// DS 4/7/96

int  proc_bulge(WorkKnot *p1,WorkKnot *p2, WorkKnot *pmid, float bulge)	{
	double angle,wlength;
	double radius,veclen,vecang,chordang,perpang,radang;
	double dx,dy,angsign,rotang;
	int midVerts = 0;

#ifdef DBGBULG
	DebugPrint("\nproc bulge: p1 = (%.3f,%.3f,%.3f), p2 = (%.3f,%.3f,%.3f) bulge=%.3f\n",
		p1->p.x,p1->p.y,p1->p.z,p2->p.x,p2->p.y,p2->p.z,bulge);

#endif

	float maxAng = DegToRad(theImport->arcSubDegrees);

	dx = p2->p.x-p1->p.x;
	dy = p2->p.y-p1->p.y;
	
	wlength = sqrt(dx*dx+dy*dy);
	
	angsign = (bulge>0.0f)?1.0f:-1.0f;
	angle = atan(fabs(bulge))*4.0;
	if (angle==0.0) {
		//straight line:	
		p1->out = p1->p + (p2->p-p1->p)/3.0f;
		p2->in  = p2->p + (p1->p-p2->p)/3.0f;
		return FALSE;
		}

	chordang = atan2(dy,dx);
	radius = float(wlength/(2.0*sin(angle/2.0)));
	vecang = angsign*(angle/2.0);
	rotang = chordang-vecang;

	if (angle>maxAng) {
		Point3 pm = (p1->p+p2->p)/2.0f; // midpoint of chord
		double pd = (wlength/2.0)/fabs(tan(angle/2.0)); // perp dist from center to chord

		perpang = chordang+angsign*HALFPI;
		if (angle>PI) perpang+=PI;

		Point3 norm(cos(perpang),sin(perpang),0.0); // perp to chord, towards center of circle
		Point3 cent(pm+float(pd)*norm);  // center of circle

		midVerts = int(angle/maxAng);
		if (midVerts>=MAXMID) 
			midVerts = MAXMID;

		double newang = angle/double(midVerts+1);
		veclen = veccalc(float(newang)) * radius;

		p1->out.x = p1->p.x + float(veclen*cos(rotang));
		p1->out.y = p1->p.y + float(veclen*sin(rotang));
		p1->out.z = p1->p.z;

		for (int i=0; i<midVerts; i++) {
			rotang += angsign*newang;
			radang = rotang-HALFPI*angsign;
			WorkKnot &pm = pmid[i];
			pm.p = cent + float(radius)*Point3((float)cos(radang),(float)sin(radang),p1->p.z);	
			float xl = float(veclen*cos(rotang)); 
			float yl = float(veclen*sin(rotang));
			pm.in.x  = pm.p.x - xl;
			pm.in.y  = pm.p.y - yl;
			pm.out.x = pm.p.x + xl;
			pm.out.y = pm.p.y + yl;
			pm.out.z = pm.in.z = pm.p.z;
			}

		rotang = chordang+PI+vecang;
		p2->in.x = p2->p.x + float(veclen*cos(rotang));
		p2->in.y = p2->p.y + float(veclen*sin(rotang));
		p2->out.z = p2->p.z;
		}
	else {
		veclen = veccalc((float)angle) * radius;
	
		p1->out.x = p1->p.x + float(veclen*cos(rotang));
		p1->out.y = p1->p.y + float(veclen*sin(rotang));
		p1->out.z = p1->p.z;
	
		rotang = chordang+PI+vecang;
		p2->in.x = p2->p.x + float(veclen*cos(rotang));
		p2->in.y = p2->p.y + float(veclen*sin(rotang));
		p2->out.z = p2->p.z;
		}

	return midVerts;
	}


static int vert (void)	{
	Point3 v;
	Face f;
	static WorkKnot lastKnot;

#ifdef DBGDXF
	DebugPrint(" VERT: (%.3f, %.3f, %.3f): ptct=?? isSpline=%d\n",rgroup[10],rgroup[20],rgroup[30],/*ptct,*/isSpline);
#endif
	if (dxf_type==DXF3D) {
		/* ---- 3D --------- */
		if(inShape) {
			if(igroup[70] & 16)
				return(0);	   /* spline frame control point */
	
			load_vert(v,10);
	
			NormTrans(v, pnorm);  // ????
	
			//TransPt(v);  /* for blocks */	 // DS 4/6/96 -- this is done later in ApplyTransform

			Point3 in = v;
			Point3 out = v;

			WorkKnot k(v,v,v);
			WorkKnot kmid[MAXMID];

			/* If previous vertex had a bulge, process it here */
	
			int midpts = 0;
			if(lastbulge!=0.0) {
				midpts = proc_bulge(&lastKnot,&k,kmid,lastbulge);
				int lastpt = dxf_spline->KnotCount() - 1;
				if(lastpt >= 0)
					dxf_spline->SetOutVec(lastpt, lastKnot.out);
				}
			
			assert(dxf_spline != NULL);

			for (int i=0; i<midpts; i++)
				dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, kmid[i].p, kmid[i].in, kmid[i].out));

			dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, k.p, k.in, k.out));
			if(inpoly && polyClosed)
				dxf_spline->SetClosed();

			if(inpoly)
				pverts++;
	
			lastKnot = k;
			}
		else {
			if(inrat) {
				if((igroup[70] & 192)==192) { 	/* Rat vert */
					if(meshvert>=dxfverts)	{
						dxfverts+=10;
						dxf_mesh->setNumVerts(dxf_mesh->getNumVerts() + 10, TRUE);
						}
					load_vert(v,10);
//					TransPt(v);  /* for blocks */
					ApplyTransform(v);  // entity tm  & block tm DS 4/5/96
					dxf_mesh->setVert(meshvert++, v);
					}
				else
				if((igroup[70] & 192)==128) {	/* Rat face */
	
					/* Don't add face w/less than 3 verts */
	
					if(igroup[71]==0 || igroup[72]==0 || igroup[73]==0)	{
						bad_rat:
						if(showPrompts)
							Alert(IDS_TH_INVALIDRATSNEST);

						killmesh:
						if (dxfobj>=0)
							namedObjs.Delete(dxfobj);
						return(1);
						}
					if((abs(igroup[71])==abs(igroup[74])) ||
							(abs(igroup[72])==abs(igroup[74])) ||
							(abs(igroup[73])==abs(igroup[74])))
						igroup[74]=0;
					if(abs(igroup[71])>(meshvert-vertbase) ||
							abs(igroup[72])>(meshvert-vertbase) ||
							abs(igroup[73])>(meshvert-vertbase) || 
							abs(igroup[74])>(meshvert-vertbase))
						goto bad_rat;
					if(meshface>=(dxffaces-1))	{
						/* Could be two tri-faces */
						dxffaces+=10;
						dxf_mesh->setNumFaces(dxf_mesh->getNumFaces() + 10, TRUE);
						}
					f.setMatID(get_mtl_id());
					f.smGroup=0;
					if(igroup[74]==0) {
						f.flags |= EDGE_ALL;
						if(igroup[71]<0)
							f.flags &= ~EDGE_A;
						if(igroup[72]<0)
							f.flags &= ~EDGE_B;
						if(igroup[73]<0)
							f.flags &= ~EDGE_C;
						f.v[0]=abs(igroup[71])-1+vertbase;
						f.v[1]=abs(igroup[72])-1+vertbase;
						f.v[2]=abs(igroup[73])-1+vertbase;
						if(f.v[0]!=f.v[1] && f.v[0]!=f.v[2] && f.v[1]!=f.v[2])
							dxf_mesh->faces[meshface++] = f;
						}
					else
						{
						f.flags=EDGE_A | EDGE_B;
						if(igroup[71]<0)
							f.flags &= ~EDGE_A;
						if(igroup[72]<0)
							f.flags &= ~EDGE_B;
						f.v[0]=abs(igroup[71])-1+vertbase;
						f.v[1]=abs(igroup[72])-1+vertbase;
						f.v[2]=abs(igroup[73])-1+vertbase;
						if(f.v[0]!=f.v[1] && f.v[0]!=f.v[2] && f.v[1]!=f.v[2])
							dxf_mesh->faces[meshface++] = f;
	
						f.flags=EDGE_A | EDGE_B;
						if(igroup[73]<0)
							f.flags &= ~EDGE_A;
						if(igroup[74]<0)
							f.flags &= ~EDGE_B;
						f.v[0]=abs(igroup[73])-1+vertbase;
						f.v[1]=abs(igroup[74])-1+vertbase;
						f.v[2]=abs(igroup[71])-1+vertbase;
						if(f.v[0]!=f.v[1] && f.v[0]!=f.v[2] && f.v[1]!=f.v[2])
							dxf_mesh->faces[meshface++] = f;
						}
					}
				}
			else
			if(insmooth) {
				if(igroup[70] & 8) {
					if(meshvert>=dxfverts) {
						if(showPrompts)
							Alert(IDS_TH_INVALIDSPLINEMESH);
						goto killmesh;
						}
					load_vert(v,10);
//					TransPt(v);  /* for blocks */
					ApplyTransform(v);  // entity tm  & block tm DS 4/5/96
					dxf_mesh->setVert(meshvert++, v);
					}
				}
			else
			if (inflat) {
				if(igroup[70] & 64) {
					if(meshvert>=dxfverts) {
						if(showPrompts)
							Alert(IDS_TH_INVALID3DMESH);
						goto killmesh;
						}
					load_vert(v,10);
//					TransPt(v);  /* for blocks */
					ApplyTransform(v);  // entity tm  & block tm DS 4/5/96
					dxf_mesh->setVert(meshvert++, v);
					}
				}
			if (inpoly) {
				Point3 p;
			
				if(igroup[70] & 16)
					return(0);	   /* spline frame control point */

				load_vert(p, 10);

				RecPlineVert(p, (float)rgroup[40], (float)rgroup[42]);
				}
			}
		}
	lastbulge = (float)rgroup[42];
	return(0);
	}

/* Find a named object's number */
/* Returns -1 if not found */

int
nobj_number(char *name) {
	return namedObjs.Find(name);
	}

#if 0
/* Strip numbers off the end of a name -- prepares it for next naming */

void
name_strip(char *name)
	{
	while(strlen(name)>0 && name[strlen(name)-1]>='0' && name[strlen(name)-1]<='9')
		name[strlen(name)-1]=0;
	}

void
inc_name(char *string,int limit) {
	int ix,len,numlen;
	long count;
	char number[20];

	len=strlen(string);
	ix=len-1;

	while(ix>=0 && isdigit(string[ix]))
	 --ix;
	++ix;

	if(ix!=len)
	 count=atol(&string[ix])+1;
	else
	 count=1;

	loop:
	sprintf(number,"%02ld",count);
	numlen=strlen(number);
	if(numlen>limit)
	 {
	 count=1;
	 goto loop;
	 }
	if((ix+numlen)>limit)
	 ix=limit-numlen;
	strcpy(&string[ix],number);
	}

#endif

void
nextname(char *iprefix,char *output) {
	strcpy(output,iprefix);
	MakeNameUnique(output);
#if 0
	name_strip(output);
	loop:
	inc_name(output,32);
	while(namedObjs.Find(output)>=0)
		goto loop;
#endif
	}

static int find_layer(int type) {
	DXFNode *lp;
	for (lp=LLIST; lp!=NULL; lp = lp->next) {
	 	if(strcmp(tgroup[8],lp->lc.l.layername)==0 && lp->type == type)	
	 		return(nobj_number(lp->objname));
		}
	return(-1);
	}

static int find_color(int type) {
	DXFNode *lp;
	for (lp=LLIST; lp!=NULL; lp = lp->next) {
		if(lp->lc.c.color==igroup[62] && lp->type == type)
	 		return(nobj_number(lp->objname));
		}
	return(-1);
	}					  

static int find_flmcol(int type) {
	DXFNode *lp;
	for (lp=LLIST; lp!=NULL; lp = lp->next) {
		if(lp->lc.c.color==filmroll_color && lp->type == type)
	 		return(nobj_number(lp->objname));
		}
	return(-1);
	}

/*--------------------------------------------------------
Find a DXF entity's corresponding object name.
If it's not in list, try adding it.
If name found, load its info into mesh variables and return 1.
If not found, try creating.  If error, return 0.
------------------------------------------------------*/

int find_dxfobj(int type) {
	int d1,d2,colorwk,addcolor;
	DXFNode *l;
	char tryname[64],tryname2[64],dd1[2]="A",dd2[2]="A";
	char finalname[64], name[64];
	BOOL created = FALSE;

	if(theImport->objsFrom==OBJS_ENTITY)	{ 
		/* Entity */ 
		if(blockLevel>0) {
			/* inside block: use block name for single entity */
			if(curBlock==lastBlock)
				goto got_obj;
			lastBlock = curBlock;
			for(int i = 0; i < 32; ++i)
				tryname[i] = 0;							
			if (curBlock->name[0]=='*') 
				strncpy(tryname,curBlock->name+1,20);
			else 
				strncpy(tryname,curBlock->name,20);
			}
		else {
			if(entity==last_entity)
				goto got_obj;
			last_entity=entity;
			strcpy(tryname,"Entity");
			}
		if(type == DXF_SHAPE)
			sprintf(finalname,"Shape_%s",tryname);
		else
			strcpy(finalname,tryname);
		nextname(finalname,name);
		//strcpy(name,finalname);
		
		colorwk = igroup[62];
		if(colorwk > 0 && colorwk < 256)
			addcolor = colorwk;
		else
		if(colorwk > 255)
			addcolor = layer_color(tgroup[8]);
		else
		if(colorwk == 0) {
			if(inInsert)
				addcolor = curBlockCol;
			else
				addcolor = 7;
			}
		else
			assert(0);

		if((dxfobj = namedObjs.Add(name, type, addcolor))<0)
			goto cantmake;
		if(type == DXF_MESH) {
			dxf_mesh = &(namedObjs.GetNamedPtr(dxfobj)->mesh);
			dxf_shape = NULL;
			dxf_spline = NULL;
			meshface=meshvert=0;
			}
		else {
			dxf_shape = &(namedObjs.GetNamedPtr(dxfobj)->shape);
assert(dxf_shape != NULL);
			if(dxf_shape->SplineCount())
				dxf_spline = dxf_shape->splines[dxf_shape->SplineCount()-1];
			else
				dxf_spline = NULL;
			dxf_mesh = NULL;
			}

#ifdef DBGDXF
		DebugPrint("*********** STARTING OBJECT: %s \n", name);
#endif
#ifndef IS_3DSDXF
//		dxf_ob_prompt(name);
#endif
		goto got_obj;
		}
	
	switch(theImport->objsFrom) {
		case OBJS_LAYER:	/* Layer */
			if ((dxfobj=find_layer(type))>=0) goto got_obj;
			break;
		case OBJS_COLOR:	/* Color */
			if ((dxfobj=find_color(type))>=0) goto got_obj;
			break;
		case OBJS_FILMROLL:	/* Filmroll */
			if ((dxfobj=find_flmcol(type))>=0) goto got_obj;
			break;   
			}
	
	/* Not in list, try adding */
	
	if((l=(DXFNode *)malloc(sizeof(struct dxfnlist)))==NULL)
		goto noram;
	
	/* Now try creating object */
	
	switch(theImport->objsFrom) {
		case OBJS_LAYER:	/* Layer */
			if(strlen(tgroup[8])==0)
				sprintf(finalname,"%s%s",type==DXF_SHAPE?"Shape_":"","Unnamed");
			else {
				strncpy(tryname2,tgroup[8],8);
				tryname2[8]=0;
				sprintf(finalname,"%s%s",type==DXF_SHAPE?"Shape_":"",tryname2);
				}
			addcolor = layer_color(tgroup[8]);
			break;
		case OBJS_COLOR:	/* Color */
			addcolor = colorwk = igroup[62];
			goto testcolor;
		case OBJS_FILMROLL:	/* Filmroll color */
			addcolor = colorwk = filmroll_color;
	
			testcolor:
			if(colorwk>256 || colorwk<0)
				colorwk=0;
			sprintf(tryname,"%s%s%02d",type==DXF_SHAPE?"Shape_":"","Color",colorwk);
			if(namedObjs.Find(tryname)<0) {
				strcpy(name,tryname);
				goto make_dxf_obj;
				}
			for(d1=0; d1<26; ++d1) {
				dd1[0]=(d1==0) ? 0:'A'+d1;
				for(d2=0; d2<26; ++d2) {
					dd2[0]='A'+d2;
					sprintf(finalname,"%s%s%s",tryname,dd1,dd2);
					if(namedObjs.Find(finalname)<0)	{
						strcpy(name,finalname);
						goto make_dxf_obj;
						}
					}
				}
			break;
		}
	
	nextname(finalname,name);
	
	make_dxf_obj:
	if(addcolor < 1 || addcolor > 255)
		addcolor = 7;	// This is just a guess, but it seems like the normal default
	if((dxfobj=namedObjs.Add(name, type, addcolor))<0) {
		free(l);	/* Free up allocated layer slot */
    cantmake:
    noram:
		if(showPrompts)
			no_RAM();
		return(0);
		}
	else {
#ifdef DBGDXF
		DebugPrint("*********** STARTING OBJECT: %s \n", name);
#endif

#ifndef IS_3DSDXF
//		dxf_ob_prompt(name);
#endif
		created = TRUE;
		}
	
	meshface=meshvert=0;
	
	/* Now new element into list */
	l->next = LLIST;
	LLIST = l;
	
	/* Plug in the appropriate matching variable */
	
	switch(theImport->objsFrom)	{
		case OBJS_LAYER:	/* Layer */
			strcpy(l->lc.l.layername,tgroup[8]);
			break;
		case OBJS_COLOR:	/* Color */
			l->lc.c.color=igroup[62];
			break;
		case OBJS_FILMROLL:
			l->lc.c.color=filmroll_color;
			break;
		}
	strcpy(l->objname,name);
	l->type = type;		// Shape or mesh
		
  got_obj:

	dxf_n = namedObjs.GetNamedPtr(dxfobj);

	if (dxf_n==NULL) 	return(0);
	if(type==DXF_MESH) {
		dxf_mesh = &dxf_n->mesh;
		dxf_shape = NULL;
		dxf_spline = NULL;
		vertbase=meshvert=dxfverts=dxf_mesh->getNumVerts();
		meshface=dxffaces=dxf_mesh->getNumFaces();
		}
	else {
		dxf_shape = &dxf_n->shape;
assert(dxf_shape != NULL);
		if(created) {
			if(dxf_shape->SplineCount())
				dxf_spline = dxf_shape->splines[dxf_shape->SplineCount()-1];
			else
				dxf_spline = NULL;
			}
		else {
			dxf_spline = dxf_shape->NewSpline();
			}
		dxf_mesh = NULL;
		}
	return(1);
	}

static void ovflow_msg(char *name, int nv, int nf) {
	DebugPrint("Overflow: [%s] %d %d\n",name,nv,nf);
	}

static void object_overflow(int error) {
	int nverts, nfaces;
	if((dxfobj>=0)&&(error==-2)) {
		NamedObject *n = namedObjs.GetNamedPtr(dxfobj);
		Mesh *m = &n->mesh;
		nverts = m->getNumVerts();
		nfaces = m->getNumFaces();
		ovflow_msg(n->name,nverts,nfaces);
		namedObjs.Delete(dxfobj);
		dxfobj=-1;
		}
	else 	{
		if(dxfobj>=0)
			namedObjs.Delete(dxfobj);
		dxfobj = -1;
		if(showPrompts)
			no_RAM();
		}
	} 

/*---------- FACE3D -----------------*/

static int face3d (void) {
	Point3 v1,v2,v3,v4;
	Face f;
	int res;
	if(dxf_type==DXF3D) {
		entity++;
		if(find_dxfobj(DXF_MESH)==0)
			return(1);
	
		f.smGroup=0L;
		f.setMatID(get_mtl_id());
	
		load_vert(v1,10);
		load_vert(v2,11);
		load_vert(v3,12);
		load_vert(v4,13);
		TransPt(v1);  /* for blocks */
		TransPt(v2);  /* for blocks */
		TransPt(v3);  /* for blocks */
		TransPt(v4);  /* for blocks */
#ifdef DBGDXF
	  	DebugPrint(" FACE3D vertex = (%.4f,%.4f,%.4f) \n",v1.x,v1.y, v1.z);
#endif

		if(v3.x==v4.x && v3.y==v4.y && v3.z==v4.z) {
			if(res=AddVerts(dxf_mesh,3)<0) {
				noram:
				object_overflow(res); 
				return(1);
				}
			if(res=AddFaces(dxf_mesh,1)<0)
				goto noram;
	
			PUTVERT(dxf_mesh,dxfverts++,v1);
			PUTVERT(dxf_mesh,dxfverts++,v2);
			PUTVERT(dxf_mesh,dxfverts++,v3);
	
			f.v[0]=dxfverts-3;
			f.v[1]=dxfverts-2;
			f.v[2]=dxfverts-1;
			f.flags=EDGE_A | EDGE_B | EDGE_C;
			if(igroup[70] & 1)
				f.flags &= ~EDGE_A;
			if(igroup[70] & 2)
				f.flags &= ~EDGE_B;
			if(igroup[70] & 4)
				f.flags &= ~EDGE_C;
			PUTFACE(dxf_mesh,dxffaces++,f);
			}
		else {
			if(res=AddVerts(dxf_mesh,4)<0)
				goto noram;
			if(res=AddFaces(dxf_mesh,2)<0)
				goto noram;
	
			PUTVERT(dxf_mesh,dxfverts++,v1);
			PUTVERT(dxf_mesh,dxfverts++,v2);
			PUTVERT(dxf_mesh,dxfverts++,v3);
			PUTVERT(dxf_mesh,dxfverts++,v4);
	
			f.v[0]=dxfverts-4;
			f.v[1]=dxfverts-3;
			f.v[2]=dxfverts-2;
			f.flags=EDGE_A | EDGE_B;
			if(igroup[70] & 1)
				f.flags &= ~EDGE_A;
			if(igroup[70] & 2)
				f.flags &= ~EDGE_B;
			PUTFACE(dxf_mesh,dxffaces++,f);
	
			f.v[0]=dxfverts-2;
			f.v[1]=dxfverts-1;
			f.v[2]=dxfverts-4;
			f.flags=EDGE_A | EDGE_B;
			if(igroup[70] & 4)
				f.flags &= ~EDGE_A;
			if(igroup[70] & 8)
				f.flags &= ~EDGE_B;
			PUTFACE(dxf_mesh,dxffaces++,f);
			}
		}
	return(0);
	}

#ifdef MAYBE
/*_LINE_*/
static int line2d() {
	Shppt p;
	if((ptct+2)>SHAPE_MAX) {
		snprintf(gp_buffer,256,progstr(R3ST0240));
		continu_line(gp_buffer);
		bad_reason=EXCEEDS_SETUP;
		return(1);
		}
	p.x=rgroup[10];
	p.y=rgroup[20];
	p.z = p.inx=p.iny=p.inz=
	p.outx=p.outy=p.outz=0.0;
	p.flags=POLYSTART;
	if (rgroup[38]!=0.0) p.z = rgroup[38];
 	ApplyTransform(p); // includes Block transform. too
	datacopy(&shpdata[ptct++],&p,sizeof(Shppt));
	p.x=rgroup[11];
	p.y=rgroup[21];
	p.flags=POLYEND;
 	ApplyTransform(p); // includes Block transform. too
	datacopy(&shpdata[ptct++],&p,sizeof(Shppt));
	return(0);
	}
#endif // MAYBE

static int AddVertsFaces(int nv, int nf) {
	int res;
#ifdef DBGDXF
	DebugPrint("AddVertsFaces, nv=%d, nf = %d \n",nv,nf);
#endif
	if (nv>0) 
		if (res=AddVerts(dxf_mesh,nv) < 0) { 
			object_overflow(res); 
			return(0); 
			}
	if (nf>0) 
		if (res=AddFaces(dxf_mesh,nf) < 0) { 
			object_overflow(res); 
			return(0); 
			}
	return(1);
	}

static void PutAFace(int a, int b, int c, int smg, int fl) {
	Face f;
	f.setMatID(get_mtl_id());
#ifdef DBGDXFFACES
	DebugPrint("PutAFace %d,%d,%d (dxfverts=%d) mtl=%d\n",a,b,c,dxfverts,f.getMatID());
#endif
	if (smg>=0)	f.smGroup = 1<<smg;
	else f.smGroup = 0;
	f.flags |= (fl & EDGE_ALL);
	f.v[0] = a;	f.v[1] = b;  f.v[2] = c;
	PUTFACE(dxf_mesh,dxffaces++,f);
	}

static int put_quad(Point3 *p) {
	Face f;
	Point3 v;
	int i;
	if (find_dxfobj(DXF_MESH)==0) return(1);	
	if (!AddVertsFaces(4, 2)) return(1);
	for (i=0; i<4; i++) {
		v = p[i];
#ifdef DBGDXF
		DebugPrint(" putquad: vert = %.4f,%.4f,%.4f \n",v.x,v.y,v.z);
#endif
	 	ApplyTransform(v); 
		PUTVERT(dxf_mesh,dxfverts++, v);
		}
	PutAFace(dxfverts-4,dxfverts-3,dxfverts-2, -1, EDGE_A|EDGE_B);
	PutAFace(dxfverts-2,dxfverts-1,dxfverts-4, -1, EDGE_A|EDGE_B);
	return(0);
	}

static int put_line_segment(Point3 *p) {
	if (find_dxfobj(DXF_SHAPE)==0)
		return(1);	
	// DS 6-17-97: this used to always create a NewSpline, but find_dxfobj already
	// has created a spline.
	Spline3D *spline = (dxf_spline)? dxf_spline : dxf_shape->NewSpline(); 
	spline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, p[0], p[0], p[0]));
	spline->AddKnot(SplineKnot(KTYPE_AUTO, LTYPE_CURVE, p[1], p[1], p[1]));
	spline->ComputeBezPoints();				// Let spline package compute vectors
	spline->SetKnotType(0, KTYPE_BEZIER);	// And make the vectors final!
	spline->SetKnotType(1, KTYPE_BEZIER);
	dxf_shape->UpdateSels();
	return(0);
	}

static int line3d(void) {
	Point3 offs;
	if (rgroup[39]!=0.0) {	/* extruded line */
		Point3 p[4];
#ifdef DBGDXF
		DebugPrint("line3d Extruded \n");
#endif
		entity++;
		load_vert(p[0],10);
		load_vert(p[1],11);

		offs = normal * (float)rgroup[39];
		p[2] = p[1] + offs;
		p[3] = p[0] + offs;

		return (put_quad(p));
		}
	inShape = TRUE; // DS 2-29-96  fixes QA# 116080
//	if(inShape) {
		Point3 p[2];
#ifdef DBGDXF
		DebugPrint("line3d to shape \n");
#endif
		entity++;
		load_vert(p[0],10);
		load_vert(p[1],11);

		ApplyTransform(p[0]);   // DS 4/6/96
		ApplyTransform(p[1]);   // DS 4/6/96


		return (put_line_segment(p));
//		}
//	return(0);
	}

static int line (void) {
	switch (dxf_type) {
//		case DXF2D: return (line2d());
		case DXF3D: return (line3d());
		}
	return(0);
	}

static int MakeSolidArcObj(float astrt,float aend,short isCircle) {
	Point3 v1,v2;
	Face f;
	float angstep,rad,t;
	Point3 pt;
	int i,nv1,nv2,nsteps,nverts,nstart,ncent1,ncent2;
	float stepsize = DEG_TO_RAD*theImport->arcDegrees;	   /* circle subdivisions size for polygon arc approximation */

	entity++;
	if (find_dxfobj(DXF_MESH)==0) return(1);	

	load_vert(pt,10);
	t = (float)rgroup[39];
	rad = (float)rgroup[40];

	if (isCircle) {
		nsteps = (int)ceil(TWOPI/stepsize);
		angstep = TWOPI / (float)nsteps;
		nverts = 2*nsteps;
		if (!AddVertsFaces(nverts+2,nsteps*4)) 
		return(1);
		}
	else {
		nsteps = (int)ceil((aend-astrt)/stepsize);			
		angstep = (aend-astrt)/nsteps;
		nverts = 2*(nsteps+1);
		if (!AddVertsFaces(nverts,nsteps*2)) 
		return(1);
		}

	if (isCircle) {
		/* output center verts */
		v1 = pt;
		v2 = v1 + Point3(0.0f, 0.0f, t);
		ApplyTransform(v1);	 
		ApplyTransform(v2);
		ncent1 = dxfverts;
		PUTVERT(dxf_mesh,dxfverts++, v1);
		ncent2 = dxfverts;
		PUTVERT(dxf_mesh,dxfverts++, v2);
		}

	nstart = dxfverts;
	for (i=0; i<=nsteps; i++) {
		v1.x = pt.x + rad * (float)cos(astrt+angstep*i);
		v1.y = pt.y + rad * (float)sin(astrt+angstep*i);
		v1.z = pt.z;
		v2.x = v1.x;
		v2.y = v1.y;
		v2.z = v1.z + t; // should this be in directio of normal ?????
		if (i==nsteps&&isCircle) { 
			/* connect cyclically */
			nv1 = dxfverts-2;
			nv2 = nstart;
			}
		else {
			/* output two verts */
			ApplyTransform(v1);	 /* includes Block transf too */
			ApplyTransform(v2);
			PUTVERT(dxf_mesh,dxfverts++, v1);
			PUTVERT(dxf_mesh,dxfverts++, v2);
			nv1 = dxfverts-4;
			nv2 = dxfverts-2;
			}
		if (i>0) {
			/* output two side faces */
			PutAFace(nv1+1,  nv1,  nv2, 1, EDGE_A | EDGE_B);
			PutAFace(nv2, nv2+1, nv1+1, 1, EDGE_A | EDGE_B);
			if (isCircle) {
				/* output cap faces */
				PutAFace(nv2,nv1,ncent1,0,EDGE_A);
				PutAFace(nv1+1,nv2+1,ncent2,0,EDGE_A);
				}
			}
		}
	return(0);
	}

static int MakeDisk() {
	Point3 v1;
	Face f;
	float angstep,rad;
	Point3 pt;
	int i,nv1,nv2,nsteps,nstart,ncent1;
	double stepsize = DEG_TO_RAD*theImport->arcDegrees;	   /* circle subdivisions size for polygon arc approximation */

	entity++;
	if (find_dxfobj(DXF_MESH)==0) return(1);	

	load_vert(pt,10);
	rad = (float)rgroup[40];

	nsteps = (int)ceil(TWOPI/stepsize);
	angstep = TWOPI / (float)nsteps;
	if (!AddVertsFaces(nsteps+1,nsteps)) 
		return(1);

	/* output center vert */
	v1 = pt;
	ApplyTransform(v1);	 
	ncent1 = dxfverts;
	PUTVERT(dxf_mesh,dxfverts++, v1);

	nstart = dxfverts;
	for (i=0; i<=nsteps; i++) {
		v1.x = pt.x + rad * (float)cos(angstep*i);
		v1.y = pt.y + rad * (float)sin(angstep*i);
		v1.z = pt.z;
		if (i==nsteps) { 
			nv1 = dxfverts-1;
			nv2 = nstart;
			}
		else {
			ApplyTransform(v1);	 /* includes Block transf too */
			PUTVERT(dxf_mesh,dxfverts++, v1);
			nv1 = dxfverts-2;
			nv2 = dxfverts-1;
			}
		if (i>0) /* output cap face */
			PutAFace(nv1,nv2,ncent1,0,EDGE_A);
		}
	return(0);
	}



static void MakeNormTM(Matrix3& tm, Point3 ni) {
	matrix m;
	point n;
	n[0] = ni.x;
	n[1] = ni.y;
	n[2] = ni.z;
	geta4by4(n, m);
	Point3 row0((float)m[0][0], (float)m[1][0], (float)m[2][0]);
	Point3 row1((float)m[0][1], (float)m[1][1], (float)m[2][1]);
	Point3 row2((float)m[0][2], (float)m[1][2], (float)m[2][2]);
	tm.SetRow(0,row0);
	tm.SetRow(1,row1);
	tm.SetRow(2,row2);
	tm.SetRow(3, Point3(0,0,0));
	}



/*  NORMTEST  --  Test a normal to see if it's been set.	 */

static int normtest (Point3 norm) {
	return (norm[0] == 0.0 && norm[1] == 0.0 && norm[2] == 1.0);
	}


static void NormTrans(Point3& p, Point3 norm) {
//	if (!normtest(norm)) 
//	    p += norm;
	}

void pmat(TCHAR *txt, Matrix3 &m) {
	MRow *n = m.GetAddr();
	DebugPrint("\n--Matrix --- %s --- \n",txt);
	for (int j=0; j<4; j++) {
		DebugPrint(" %.4f  %.4f  %.4f \n", n[j][0], n[j][1], n[j][2]); 
		}
	}

static void CompEntityTransform(Point3 &norm) {
	if (normtest(norm)) {
		isEntityTM = 0;
		normtm.IdentityMatrix();
		}
	else {
		isEntityTM = 1;
		MakeNormTM(normtm, norm);
		}
#ifdef DBGDXF
	DebugPrint(" CompEntityTransform: isEntityTM= %d:  norm = %.4f,%.4f,%.4f\n",
		isEntityTM,norm[0],norm[1],norm[2]);
	pmat(" norm transform", normtm);
#endif
	}

/* Calc incoming & outgoing vectors for a point */

#define POLY_CIRC_TENSION 0.551785439253f

void
calc_vecs(WorkKnot *p,WorkKnot *pl,WorkKnot *pn) {
	float i_len,o_len,i_pct,o_pct,totlen;

	/* Calc deltas for the points */

	/* Calc some temps to speed things up */

	Point3 ddi = p->p - pl->p;		/* Ctl-to-last */
	Point3 ddo = pn->p - p->p;		/* Ctl-to-next */
	Point3 dln = pn->p - pl->p;	/* Last-to-next */

	dln*=POLY_CIRC_TENSION;

	i_len=Length(ddi);
	o_len=Length(ddo);
	totlen=i_len+o_len;
	if(totlen==0) {
		p->in = p->out = p->p;
		return;
		}
	i_pct = i_len / totlen;
	o_pct = 1.0f - i_pct;

	/* Figure out vectors */

	p->in = p->p - i_pct * dln;
	p->out = p->p + o_pct * dln;
	}

/*------ CIRC --- circle */
static int circ(void)	{
	CompEntityTransform(normal);
	if (dxf_type==DXF3D) {
		 /* 3D Loading */
		if (rgroup[39]!=0.0) {	/* extruded circle */
			return(MakeSolidArcObj(0.0f, TWOPI,1));
			}
		else {
			if(DXFImport::fillPolylines)
				return(MakeDisk());
			else {
				WorkKnot p1,p2,p3,p4;
				Point3 pt;
	
				entity++;
				if (find_dxfobj(DXF_SHAPE)==0) return(1);	

				load_vert(pt,10);

				NormTrans(pt, normal);

				p1.p = p1.in = p1.out = Point3(0,0,0);
				p1.p.x = pt.x;
				p1.p.y = pt.y + (float)rgroup[40];
				p1.p.z = pt.z;
				ApplyTransform(p1.p);  /* for blocks */
				ApplyTransform(p1.in);  /* for blocks */
				ApplyTransform(p1.out);  /* for blocks */
				dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, p1.p, p1.in, p1.out));

				p2.p = p2.in = p2.out = Point3(0,0,0);
				p2.p.x = pt.x+(float)rgroup[40];
				p2.p.y = pt.y;
				p2.p.z = pt.z;
				ApplyTransform(p2.p);  /* for blocks */
				ApplyTransform(p2.in);  /* for blocks */
				ApplyTransform(p2.out);  /* for blocks */
				dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, p2.p, p2.in, p2.out));

				p3.p = p3.in = p3.out = Point3(0,0,0);
				p3.p.x = pt.x;
				p3.p.y = pt.y-(float)rgroup[40];
				p3.p.z = pt.z;
				ApplyTransform(p3.p);  /* for blocks */
				ApplyTransform(p3.in);  /* for blocks */
				ApplyTransform(p3.out);  /* for blocks */
				dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, p3.p, p3.in, p3.out));

				p4.p = p4.in = p4.out = Point3(0,0,0);
				p4.p.x = pt.x-(float)rgroup[40];
				p4.p.y = pt.y;
				p4.p.z = pt.z;
				ApplyTransform(p4.p);  /* for blocks */
				ApplyTransform(p4.in);  /* for blocks */
				ApplyTransform(p4.out);  /* for blocks */
				dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, p4.p, p4.in, p4.out));
				dxf_spline->SetClosed();

				calc_vecs(&p1,&p4,&p2);
				dxf_spline->SetInVec(0, p1.in);
				dxf_spline->SetOutVec(0, p1.out);
				calc_vecs(&p2,&p1,&p3);
				dxf_spline->SetInVec(1, p2.in);
				dxf_spline->SetOutVec(1, p2.out);
				calc_vecs(&p3,&p2,&p4);
				dxf_spline->SetInVec(2, p3.in);
				dxf_spline->SetOutVec(2, p3.out);
				calc_vecs(&p4,&p3,&p1);
				dxf_spline->SetInVec(3, p4.in);
				dxf_spline->SetOutVec(3, p4.out);
				return 0;
				}
			}
		}
	return(0);
	}

/* --------------- ARC ---------------------*/
static int arc(void)	{
	CompEntityTransform(normal);
	if (dxf_type==DXF3D) {
		/* 3D Loading */
		if (rgroup[39]!=0.0) {	/* extruded arc */
			float aend,astrt;
			astrt=(float)rgroup[50];
			aend=(float)rgroup[51];
			while(aend<astrt)
				aend+=360.0f;
			astrt/=RAD_TO_DEG;	/* Convert degrees to radians */
			aend/=RAD_TO_DEG;
			return(MakeSolidArcObj(astrt,aend,0));
			}
		else {
			WorkKnot p;
			Point3 pt;
			int ix;
			float astrt,aend,arcstep,arcvec,vecwk,sinfac,cosfac,xvec,yvec,angle;
	
			entity++;
			if (find_dxfobj(DXF_SHAPE)==0) return(1);	

			load_vert(pt,10);
			

			NormTrans(pt, normal);
	
			astrt = (float)rgroup[50];
			aend = (float)rgroup[51];
			while(aend<astrt)
				aend += 360.0f;

#ifdef DBGDXF
DebugPrint("Got arc %.4f to %.4f\n",astrt,aend);
#endif

			astrt *= DEG_TO_RAD;
			aend *= DEG_TO_RAD;
			float maxAng = DegToRad(theImport->arcSubDegrees);
			int nsegs = (int)((float(aend-astrt)+maxAng-.001f)/maxAng);

			arcstep =(aend-astrt)/nsegs;	/* Our arcs take 4 pts */
			arcvec = veccalc(arcstep);	/* Calc bezier control vector length */
			vecwk = arcvec*(float)rgroup[40];		/* Scale vector by radius */
	
			angle=astrt;

			for(ix=0; ix<nsegs+1; ++ix,angle+=arcstep) {
				sinfac = (float)sin(angle);
				cosfac = (float)cos(angle);
				
				p.p.x = pt.x + (float)rgroup[40]*cosfac;
				p.p.y = pt.y + (float)rgroup[40]*sinfac;
				
				p.p.z = p.in.z = p.out.z = pt.z;

				xvec = vecwk*sinfac;
				yvec = vecwk*cosfac;
				
				p.in.x = p.p.x + xvec;	  p.in.y = p.p.y - yvec;
				p.out.x = p.p.x - xvec;	  p.out.y = p.p.y + yvec;

				ApplyTransform(p.p); 
				ApplyTransform(p.in);
				ApplyTransform(p.out);  
				dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, p.p, p.in, p.out));
				}
			}
		}
	return(0);
	}

typedef int BFunc(Point3 p, float wstart);

static int  DoBulge(Point3 ps, Point3 pe, float ws, float we, float bulge, short closing, BFunc *dothis) {
	float wstrt, wend, midx,midy, perp;
	Point3 n, q;
	float wlength,dx,dy,angle,radius,angstep,ang0,dw,ang;
	int nsteps,i,angsign;
	float cx,cy;

#ifdef DBGDXF1
	DebugPrint(" DoBulge, ws= %4f, we = %4f\n",ws,we);
#endif
	wstrt = ws;
	wend = we;

	dx = pe[0]-ps[0];
	dy = pe[1]-ps[1];
	wlength = (float)sqrt(dx*dx+dy*dy);

	angle = (float)atan(bulge)*4.0f;
	if(angle<0) {
		angsign= -1;
		angle = -angle;
		}
	else 
	if(angle>0)
		angsign=1;
	else
		angsign=0;

	if (angle==0.0f) angle = .00001f;
	radius = wlength/(2.0f * (float)sin(angle*.5));

	nsteps = (int)ceil(angle/(DEG_TO_RAD*theImport->arcDegrees));
	angstep = angle/nsteps;

	n = Normalize(Point3(-dy, dx, 0.0f));
	if (angsign==-1.0) {
		n = -n;
		angstep = -angstep;
		}

	midx = .5f*(pe.x+ps.x); 
	midy = .5f*(pe.y+ps.y);

	perp = radius*(float)cos(.5*angle);
	cx = midx + perp*n.x;
	cy = midy + perp*n.y;
	
	
	ang0 = (radius==0.0)?0.0f:(float)atan2(ps.y-cy,ps.x-cx);

	dw = (wend-wstrt)/((float)nsteps);

#ifdef DBGDXF1
	DebugPrint(" cx = %.3f, cy = %.3f, radius = %.3f, ang0=%3f, angstep=%3f, nsteps =%d\n",
		cx,cy,radius,ang0,angstep,nsteps);
#endif
	for (i=1; i<nsteps; i++) {
		ang = ang0 + i*angstep;
		q = Point3(cx + radius*(float)cos(ang), cy + radius*(float)sin(ang), ps.z);
		(*dothis)(q, wstrt+i*dw);
		}
	if (!closing) (*dothis)(pe, we);
	return(0);	
	}

/* find intersection of lines defined by (a,b) and (c,d) */
/* returns 1 if valid intersection, 0 if parallel lines */

static int Int2PntLines(Point3 p1, Point3 q1, Point3 p2, Point3 q2, Point3 &p) {
	double dx1,dx2,dy1,dy2,r1,r2,den;
	dx1 = q1.x-p1.x; 	
	dy1 = q1.y-p1.y;
	dx2 = q2.x-p2.x; 	
	dy2 = q2.y-p2.y;
	den = dx1*dy2-dx2*dy1;
	if (den==0.0) 
		return(0);
	r1 = p1.y*dx1-p1.x*dy1;
	r2 = p2.y*dx2-p2.x*dy2;
	p.x = (float)((r1*dx2-r2*dx1)/den);	
	p.y = (float)((r1*dy2-r2*dy1)/den);	
	p.z = p1.z;
	return(1);
	}

static void IntsctSegments(PLPoint *p1, PLPoint *p2) {
	if (!Int2PntLines(p1->a0,p1->a1,p2->a0,p2->a1, p2->ai))
		p2->ai = p2->a0;
	if (!Int2PntLines(p1->b0,p1->b1,p2->b0,p2->b1, p2->bi))
		p2->bi = p2->b0;
#ifdef DBGDXF
	DebugPrint(" IntsctSegs: (%.3f,%.3f,%.3f) (%.3f,%.3f,%.3f) \n",
		p2->ai[0], p2->ai[1],p2->ai[2],p2->bi[0],p2->bi[1],p2->bi[2]);
#endif
	}

static void put_2verts(Point3 p, int i1, int i2) {
	Point3 v = p;

	ApplyTransform(v);	 /* includes Block transf too */
	PUTVERT(dxf_mesh,i1,v);
	if (isThick) {
		v.x = p.x;
		v.y = p.y;
		v.z = p.z+pl_thick;
		ApplyTransform(v);	 /* includes Block transf too */
		PUTVERT(dxf_mesh,i2,v);
		}
	}

static void put_dbl_vert(Point3 p) {
	put_2verts(p,dxfverts,dxfverts+1);
	dxfverts++;
	if (isThick) dxfverts++;
	}	

#define POLSTART 1
#define POLEND 2

static void PutSideFaces(int n1, int n2, int type) {
	int f1,f2;

#ifdef DBGDXF1
	DebugPrint(" PutSideFaces\n");
#endif
	f1 = (type == POLSTART)? (int)(EDGE_A|EDGE_B) : (int)EDGE_A; 		
	f2 = (type == POLEND)? (int)(EDGE_A|EDGE_B) : (int)EDGE_A; 		

	if (isThick) {
		PutAFace(n2+2, n1+2, n1, 0, f1);   /* bottom 0 */
		PutAFace(n2+1, n1+1, n1+3, 2, f1);	/* top 0 */
		PutAFace(n2+3, n1+3, n1+2, 3, EDGE_A|EDGE_B);	/* side 0 */
		PutAFace(n2, n1, n1+1, 1, EDGE_A|EDGE_B);	  /* side 0 */

		PutAFace(n1, n2, n2+2, 0, f2);			/* bottom 1 */
		PutAFace(n1+3, n2+3, n2+1, 2, f2);			/* top 1 */
		PutAFace(n1+1, n2+1, n2, 1, EDGE_A|EDGE_B);  	/* side	1 */
		PutAFace(n1+2, n2+2, n2+3, 3, EDGE_A|EDGE_B);	/* side 1 */
		}
	else {
		PutAFace(n2+1, n1+1, n1, 0, f1);
		PutAFace(n1, n2, n2+1, 0, f2);
		}
	}

static void ComputeLineEnds(PLPoint *pl0, PLPoint *pl1) {
	Point3 n;
	float hws,hwe;
	n = Normalize(Point3(pl0->p.y - pl1->p.y, pl1->p.x - pl0->p.x, 0.0f));
	
	hws = pl0->wstrt*.5f;
	hwe = pl1->wstrt*.5f; /* force width to be continuous */

	pl0->a0 = pl0->p + n * hws;
	pl0->b0 = pl0->p - n * hws;
	pl0->a1 = pl1->p + n * hwe;
	pl0->b1 = pl1->p - n * hwe;
	}

static int AddPlineVert(Point3 p, float wstart) {
	PLPoint plp;
	int res;

#ifdef DBGDXF1
	DebugPrint(" AddPlineVert:  nplverts=%d, polyClosed = %d, isThick = %d\n",nplverts,polyClosed,isThick);
	DebugPrint("    0 = (%.3f,%.3f,%.3f) \n",p[0],p[1],p[2]);
#endif
	plp.p = p;
	plp.wstrt = wstart;

	if (nplverts>0)	{
		ComputeLineEnds(&plq[1],&plp); /* compute plq[1].a0,a1,b0,b1 */
		}

	if (nplverts==1) {
		pl0 = plq[1];
		if (!polyClosed) {
			if (!AddVertsFaces(nvper,isThick?2:0)) return(1);
			put_dbl_vert(plq[1].a0); 
			put_dbl_vert(plq[1].b0);
			/* cap starting end */
			if (isThick) {
				PutAFace(dxfverts-3, dxfverts-4, dxfverts-2, 4,EDGE_A|EDGE_B);
				PutAFace(dxfverts-2, dxfverts-1, dxfverts-3, 4,EDGE_A|EDGE_B);
				}
			}
		}

	if (nplverts>1) {
		IntsctSegments(&plq[0],&plq[1]);  /* compute plq[1].ai, bi */
		if (res=AddVerts(dxf_mesh,nvper) < 0) { 
			object_overflow(res); 
			return(1); 
			}
		put_dbl_vert(plq[1].ai);
		put_dbl_vert(plq[1].bi);
		if (nplverts>2||(!polyClosed)) {
			if (res=AddFaces(dxf_mesh,isThick?8:2) < 0) { 
				object_overflow(res); 
				return(1); 
				}
			PutSideFaces(dxfverts-2*nvper,dxfverts-nvper,(nplverts==2)?POLSTART:0);
			}
		}
	plq[0] = plq[1];
	plq[1] = plp;
	nplverts++;
	return(0);
	}

static int AddPlineBulge(Point3 p, float we, float bulge, short closing) {
	return DoBulge(plq[1].p,p,plq[1].wstrt,we,bulge,closing,AddPlineVert);
	}

static int count_steps(float bulge) {
	float angle = (float)atan(bulge)*4.0f;
	if(angle<0) angle = -angle; 
	return (int)ceil(angle/(DEG_TO_RAD*theImport->arcDegrees));
	}

/* Calculate a 3D triangle's normal */

static void
t3normal(Point3 a, Point3 b, Point3 c, Point3 &norm) {
	Point3 v1, v2;

	/* Figure two vectors */

	v1 = b - a;
	v2 = c - a;

	norm = -Normalize(v1 ^ v2);	// Get cross product's unit vector
	}

static int DoCap(int nstart, int npoints, int flip) {
	int nend = nstart + npoints - 1;
	PolyShape shape;
	shape.SetNumLines(1);

	PolyLine &line = shape.lines[0];
	line.SetNumPts(npoints);

	int i,norm_axis;
	Point3 n, norm;

	/* find normal axis */
	norm = Point3(0,0,0);
	for (i=0; i<npoints-2; i++) {
		Point3 va = dxf_mesh->verts[nstart+i];
		Point3 vb = dxf_mesh->verts[nstart+i+1];
		Point3 vc = dxf_mesh->verts[nstart+i+2];
		t3normal(va, vb, vc, n);
		norm += n;
		}
	norm = Normalize(norm);
	norm_axis = (fabs(norm.x)>fabs(norm.y))?0:1;
	norm_axis = (fabs(norm[norm_axis])>fabs(norm.z))?norm_axis:2;

	for (i=0; i<npoints; i++) {
		Point3 v = dxf_mesh->verts[nstart+i];
		switch(norm_axis) {
			case 0:
				line.pts[i] = PolyPt(Point3(v.y, v.z, 0.0f), POLYPT_VISEDGE, nstart+i);
				break;
			case 1:
				line.pts[i] = PolyPt(Point3(v.z, v.x, 0.0f), POLYPT_VISEDGE, nstart+i);
				break;
			case 2:
				line.pts[i] = PolyPt(Point3(v.x, v.y, 0.0f), POLYPT_VISEDGE, nstart+i);
				break;
			}
		}
	line.Close();
	BOOL reversed = FALSE;
	if(line.IsClockWise()) {
		reversed = TRUE;
		line.Reverse();
		}
		
	// Organize the shape
	ShapeHierarchy hier;
	shape.OrganizeCurves(0, &hier);		// Time is irrelevant here -- Make it zero

	// Cap it!
	int oldfaces = dxf_mesh->getNumFaces();
	MeshCapInfo capInfo;
	shape.MakeCap(0, capInfo, CAPTYPE_MORPH);
	// Build information for capping
	MeshCapper capper(shape);
	int vert = nstart;
	MeshCapPoly &capline = capper[0];
	int lverts = line.numPts;
	// If we reversed the input polygon, reverse the points here
	if(reversed) {
		for(int v = lverts-1; v >= 0; --v)
			capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
		}
	else {
		for(int v = 0; v < lverts; ++v)
			capline.SetVert(v, vert++);			// Gives this vert's location in the mesh!
		}
	capper.CapMesh(*dxf_mesh, capInfo, hier.reverse[0] ? TRUE : FALSE, 16);

	dxffaces = dxf_mesh->getNumFaces();
	return(1);
	}

static int nCapPoints,ivert;

static int out_vert(Point3 p, float wstart) {
	Point3 v;
	v = p;
	PUTVERT(dxf_mesh,ivert,v);
	if (isThick) {
		v += Point3(0.0f, 0.0f, pl_thick);
		PUTVERT(dxf_mesh,ivert+nCapPoints,v);
		}
	ivert++;
	return 1;
	}

static int FinishThinPline(void) {
	PVert *pv,*pvlast;
	Point3 v;
	int nTotal,i,k,npv,icap,vstart,res = 1,err;
	float lastb;

#ifdef DBGDXF1
	DebugPrint("FinishThinPline\n");
#endif

	npv = pvtab.Count();

	ivert = vstart = dxfverts;

	/* Count number of verts required */
	nCapPoints = 0;
	lastb = 0.0f;
	for (i=0; i<npv; i++) {
		pv = PV_EL(i);
		if (lastb!=0.0)
			nCapPoints += count_steps(lastb);
		else
			nCapPoints++;
		lastb = pv->bulge;
		}
	if (polyClosed&&lastb!=0.0) 
		nCapPoints += (count_steps(lastb)-1);
		
	nTotal = isThick?2*nCapPoints:nCapPoints;

	if (err=AddVerts(dxf_mesh,nTotal) < 0) {
		object_overflow(err); 
		goto bye;
		}
	dxfverts += nTotal;

	/* create verts */
	lastb = 0.0f;
	for (icap=0; icap<npv; icap++) {
		pv = PV_EL(icap);
		if (lastb!=0) {
			if (DoBulge(pvlast->p, pv->p, 1.0f, 1.0f, lastb,0, out_vert))
				goto bye;
			}
		else 
			out_vert(pv->p, 0.0f);
		lastb = pv->bulge;
		pvlast = pv;
		}

	if (polyClosed) {
		if (lastb!=0) {
			pv = PV_EL(0);
			if (DoBulge(pvlast->p, pv->p, 1.0f, 1.0f, lastb,1, out_vert))
				goto bye;
			}
		}

	/* do first Cap */
	if (polyClosed && theImport->fillPolylines) 
		DoCap(vstart,nCapPoints,0);

	if (isThick) { /* do the side faces */
		int nf = polyClosed? nCapPoints*2 : (nCapPoints-1)*2;
		if (err=AddFaces(dxf_mesh,nf) < 0) {
			object_overflow(err); 
			goto bye; 
			}
		for (i=0; i<nCapPoints-1; i++) {
			k = vstart+i;
			PutAFace(k, k+1, k+nCapPoints+1,1,EDGE_A | EDGE_B);
			PutAFace(k+nCapPoints+1, k+nCapPoints, k,1,EDGE_A | EDGE_B);
			}
		if (polyClosed) {
			PutAFace(vstart+nCapPoints-1,vstart,vstart+nCapPoints,1,EDGE_A | EDGE_B);
			PutAFace(vstart+nCapPoints,vstart+nTotal-1,vstart+nCapPoints-1,1,EDGE_A | EDGE_B);
			if(theImport->fillPolylines)
				DoCap(vstart+nCapPoints,nCapPoints,1);
			}
		}

	/* transform points */

	for (i=vstart; i<dxfverts; i++) {
		v = dxf_mesh->verts[i];
		ApplyTransform(v);	 /* includes Block transf too */
		PUTVERT(dxf_mesh,i,v);
		}

	res = 0;
 bye:
	return(res);
	}
	
static int FinishWidePline(void) {
	int n = pvtab.Count();
	int i;
	float lastb = 0.0f;
	PVert *pv;
	int res = 1;
#ifdef DBGDXF1
	DebugPrint("FinishWidePline  n = %d, closed = %d \n",n,polyClosed);
#endif
	for (i=0; i<n; i++) {
		pv = PV_EL(i);
		if (lastb!=0) {
			if (AddPlineBulge(pv->p, pv->ws,lastb,0)) goto bye;
			}
		else {
			if (AddPlineVert(pv->p,pv->ws)) goto bye;
			}
		lastb = pv->bulge;
		}
	if (polyClosed) {
		if (lastb!=0) {
			if (AddPlineBulge(pl0.p, pl0.wstrt, lastb,1)) goto bye;
			}
		/* ---- close the tube ---- */
		if (EqualVectors(plq[1].p, pl0.p)) {
			if (!AddVertsFaces(nvper,isThick?16:4)) return(1);
			IntsctSegments(&plq[0],&pl0); /* compute pl0.ai,bi */
			put_dbl_vert(pl0.ai); 
			put_dbl_vert(pl0.bi); 
			PutSideFaces(dxfverts-2*nvper,dxfverts-nvper,0);
			PutSideFaces(dxfverts-nvper,startvert,0);
			}
		else {
			if (!AddVertsFaces(2*nvper,isThick?24:6)) return(1);
			ComputeLineEnds(&plq[1],&pl0); /* compute plq[1].a0,a1,b0,b1 */
			IntsctSegments(&plq[0],&plq[1]); /* compute plq[1].ai,bi */
			IntsctSegments(&plq[1],&pl0);  /* compute pl0.ai,bi */
			put_dbl_vert(plq[1].ai); 
			put_dbl_vert(plq[1].bi); 
			put_dbl_vert(pl0.ai); 
			put_dbl_vert(pl0.bi); 
			PutSideFaces(dxfverts-3*nvper,dxfverts-2*nvper,0);
			PutSideFaces(dxfverts-2*nvper,dxfverts-nvper,0);
			PutSideFaces(dxfverts-nvper,startvert,0);
			}
		}
	else {
		/* --- Cap end at last point --- */
		if (!AddVertsFaces(nvper,isThick?10:2)) return(1);
		put_dbl_vert(plq[0].a1); 
		put_dbl_vert(plq[0].b1); 
		PutSideFaces(dxfverts-2*nvper,dxfverts-nvper,POLEND);  
		if (isThick) {
			PutAFace(dxfverts-1, dxfverts-2, dxfverts-4, 4,EDGE_A | EDGE_B);
			PutAFace(dxfverts-4, dxfverts-3, dxfverts-1, 4,EDGE_A | EDGE_B);
			}
		}
	res = 0;
 bye:
	return(res);
	}

static void SetThick(void) {
	pl_thick = (float)rgroup[39];
	if (pl_thick!=0.0) { isThick=1; nvper = 4;	}
	else {	isThick=0;	nvper = 2;	}
	}

static int pline(void) {
	int mverts,mfaces,res;

	inpoly=insmooth=inflat=FALSE;
	inShape = FALSE;  // DS 2-28-96	 -- fixes QA# 116096

	is_3d_pline = igroup[70]&(8+16+64)?1:0;
	polyClosed = (igroup[70] & 1)?1:0;
	pnorm = normal;

#ifdef DBGDXF
	DebugPrint(" PLINE: normal = %.4f, %.4f, %.4f \n", normal[0],normal[1],normal[2]);
#endif

	if (dxf_type==DXF3D) {
		isEntityTM = 0;
		/* ------ 3D ------*/
		if(igroup[70] & 64)	{ /* polyface mesh */
			entity++;
			if(find_dxfobj(DXF_MESH)==0)
				return(1);		/* Errored! */
#ifdef DBGDXF
			DebugPrint(" PLINE:  PolyFace mesh\n");
#endif
			inrat=TRUE;
			}
		else
		if(igroup[70] & 16) { 	/*3D polygon mesh */
			mclosed=nclosed=FALSE;
			if(igroup[70] & 1)
				mclosed=TRUE;
			if(igroup[70] & 32)
				nclosed=TRUE;
			if(igroup[75]) {
				mcount=igroup[73];
				ncount=igroup[74];
				}
			else {
				mcount=igroup[71];
				ncount=igroup[72];
				}

			if (ncount<=0||mcount<=0) {
#if 0
				DebugPrint("    Null Poly mesh: grp[70] =%X  grp[75] = %X  Ncount = %d  Mcount = %d \n",
					igroup[70], igroup[75], ncount,mcount);
#endif
				return(0);
				}

			entity++;
			if(find_dxfobj(DXF_MESH)==0)
				return(1);		/* Errored! */
	
#ifdef DBGDXF
			DebugPrint(" PLINE:  3D Polygon mesh\n");
#endif
			mverts=mcount*ncount;
	
			if(mclosed && nclosed)
				mfaces=mcount*ncount*2;
			else
			if(mclosed)
				mfaces=mcount*(ncount-1)*2;
			else
			if(nclosed)
				mfaces=(mcount-1)*ncount*2;
			else
				mfaces=(mcount-1)*(ncount-1)*2;

			if(res=AddVerts(dxf_mesh,mverts)<0) {
				noram:
				object_overflow(res); 
				return(1); 
				}
			dxfverts+=mverts;
			if(res=AddFaces(dxf_mesh,mfaces)<0)
				goto noram;
			dxffaces+=mfaces;
	
	/* Set flags which tell what to do with verts */
	
			if(igroup[75])
				insmooth=TRUE;
			else
				inflat=TRUE;
			}
		else {	// Polyline or spline
			/* if closed, thick, or wide, input as 3D solid */
			if ((polyClosed&&DXFImport::fillPolylines)||rgroup[39]!=0.0||rgroup[40]!=0) { 
				entity++;
				if (find_dxfobj(DXF_MESH)==0) return(1);	
#ifdef DBGDXF
				DebugPrint(" PLINE: As 3d solid: closed=%d  thick =%.3f, wide=%.3f\n", polyClosed, rgroup[39],rgroup[40]);
#endif
				SetThick();
				lastbulge=0.0f;
				nplverts=0;
				startvert = dxfverts;
				pverts=0;
				inpoly = TRUE;
				anyWide=0;
				pvtab.Delete(0, pvtab.Count());
				inShape = FALSE;
				}
			else {		// We're making a shape object!
				entity++;
				if (find_dxfobj(DXF_SHAPE)==0)
					return(1);	
#ifdef DBGDXF
				DebugPrint("Started pline shape, closed=%d\n",polyClosed);
#endif
				SetThick();		// This will be OFF
				lastbulge=0.0f;
				nplverts=0;
				pverts=0;
				inpoly = TRUE;
				anyWide=0;
				pvtab.Delete(0, pvtab.Count());
				inShape = TRUE;
				isSpline = (igroup[70]&4)?1:0;
				if (isSpline)
					splineType = igroup[75];
				}
			}
		}
	return(0);
	}

/* ---------------- SEQEND ---------------*/
static int seqend (void)	{
	int fixmesh;
	int m,n,p1,p2,p3,p4;
	Face f;
	
#ifdef DBGDXF
	DebugPrint(" SEQEND: dxf_type = %d, inpoly=%d anyWide=%d  inrat=%d closed=%d\n",
		dxf_type,inpoly,anyWide,inrat,polyClosed);
#endif
	if (!is_3d_pline)
		CompEntityTransform(pnorm);
	fixmesh=0;
	if (inpoly) {
		inpoly = FALSE;
#ifdef MAYBE
		if (dxf_type==DXF2D||(dxf_type==DXFPATH&&!isSpline))	{

			/* If handling a bulge on closed polygon, wrap it up! */
			if((shpdata[ptct-1].flags & POLYCLOSED) && lastbulge!=0.0) {
				polys = poly_count();
				pstrt = poly_start(polys-1);
				proc_bulge(&shpdata[ptct-1],&shpdata[pstrt],lastbulge);
				}
			}
		else if (dxf_type==DXFPATH) {
			if (isSpline) {
				if (COUNT(splineTab)>0) {
#ifdef DBGSPLINE
					{
					int i,count;
					FPoint3 *sp;
					count = COUNT(splineTab);
					sp = SPLINE_EL(0);
					DebugPrint("\n\nSPLINE: polyClosed = %d\n",polyClosed);
					for (i=0; i<count; i++) {
						DebugPrint(" point[%d] = (%2f,%.2f,%.2f) \n",i,sp[i].x,sp[i].y,sp[i].z);
						}
					DebugPrint("\n");
					}
#endif
					if((COUNT(splineTab)+1)>PATH_MAX)	{
						DisposTab(&splineTab);
						snprintf(gp_buffer,256,progstr(R3ST0527));
						continu_line(gp_buffer);
						bad_reason=EXCEEDS_SETUP;
						return(1);
						}
					switch (splineType) {
						case 5: QuadraticSpline(); break;
						case 6: CubicSpline(); break;
						}
					}
				DisposTab(&splineTab);
				}
			}
		else
#endif // MAYBE
		if (dxf_type==DXF3D) {
			if (anyWide)  {
				if (pvtab.Count()<2) {
					//TSTR s1(GetString(IDS_TH_BADPOLYLINE));
					//TSTR s2(GetString(IDS_TH_DXFIMP));
					//MessageBox(GetActiveWindow(), s1.data(), s2.data(), MB_OK);
					pvtab.Delete(0, pvtab.Count());
					inrat = insmooth = inflat = FALSE;
					return (0);
					//bad_reason=INVALID_FILE;
					//return(1);
					}
				FinishWidePline();
				}
			else { 
				if(inShape) {
					/* If handling a bulge on closed polygon, wrap it up! */
					if(dxf_spline->Closed() && lastbulge!=0.0f) {
						WorkKnot pstrt(dxf_spline,0);
						int lastknot = dxf_spline->KnotCount() - 1; 
						WorkKnot pend(dxf_spline, lastknot);
						WorkKnot pmid[MAXMID];
						int midpts = proc_bulge(&pend,&pstrt,pmid,lastbulge);
						dxf_spline->SetInVec(0, pstrt.in);
						dxf_spline->SetOutVec(lastknot, pend.out);
						for (int i=0; i<midpts; i++)
							dxf_spline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, pmid[i].p, pmid[i].in, pmid[i].out));
						}
					// transform all points in shape  --DS 3-15-96
					for (int i=0; i<dxf_spline->KnotCount()*3; i++) {
						Point3 p = dxf_spline->GetVert(i);
						ApplyTransform(p);
						dxf_spline->SetVert(i, p);
						}
					}
				else
					FinishThinPline();
				}
			pvtab.Delete(0, pvtab.Count());
			}
		}	  
	if(inrat) {
		inrat=FALSE;
		fixmesh=1;
		}
	if(insmooth) {
		insmooth=FALSE;
		f.smGroup=1L;
		goto finish_mesh;
		}
	if(inflat)	{
		inflat=FALSE;
		f.smGroup=0L;
		
		finish_mesh:
		fixmesh=1;
		f.setMatID(get_mtl_id());
		for(m=0; m<mcount; ++m) {
			for(n=0; n<ncount; ++n) {
				p1=m*ncount+n;
				p2=m*ncount+n+1;
				p3=p2+ncount;
				p4=p1+ncount;
				if(n==(ncount-1)) {
					if(nclosed==0)
						goto nextn;
					p2-=ncount;
					p3-=ncount;
					}
				if(m==(mcount-1)){
					if(mclosed==0)
						goto nextn;
					p3-=(mcount*ncount);
					p4-=(mcount*ncount);
					}
				f.flags=EDGE_A | EDGE_B;
				f.v[0]=p1+vertbase;
				f.v[1]=p2+vertbase;
				f.v[2]=p3+vertbase;
				PUTFACE(dxf_mesh,meshface++,f);
	
				f.v[0]=p3+vertbase;
				f.v[1]=p4+vertbase;
				f.v[2]=p1+vertbase;
				PUTFACE(dxf_mesh,meshface++,f);
	
				nextn:;
				}
			}
		}
	if(fixmesh) {
		dxf_mesh->setNumVerts(meshvert, TRUE);
		dxf_mesh->setNumFaces(meshface, TRUE);
		}
	return(0);
	}


static void ConcatTransf(Point3 base, Point3 ins, Point3 scl, float ang, Point3 n) {
	int i;
	Matrix3 tm, tmnorm;
	MRow *tmrow = tm.GetAddr();
	float fang = ang*DEG_TO_RAD;
	tm.IdentityMatrix();
	tm.Translate(-base);
	if (scl.x!=1.0) { for (i=0; i<4; i++) tmrow[i][0]*=scl.x; tm.SetNotIdent(); }
	if (scl.y!=1.0) { for (i=0; i<4; i++) tmrow[i][1]*=scl.y; tm.SetNotIdent(); }
	if (scl.z!=1.0) { for (i=0; i<4; i++) tmrow[i][2]*=scl.z; tm.SetNotIdent(); }
	if (ang!=0.0)
		tm.RotateZ(fang);
	tm.Translate(ins);
	if (!normtest(n)) {
		MakeNormTM(tmnorm,n);
		tm = tm * tmnorm;	
		}
	curtm = tm * curtm;
	}

Block *find_block(char *s) {
	Block *b;
	for (b = blockList; b!=NULL; b= b->next) {
		if (strcmp(s,b->name)==0) return(b);
		}
	return(NULL);
	}

/* INSERT block */

int insert() {
	long svaddr;
	short sv_gcode;
	char *sv_gtextv=NULL;
	char *sv_layer=NULL;
	Block *sv_block,*b;
	short sv_block_col;	
	Point3 inspt,basept,sclfct;
	float rotang;
	float dcol,drow,col_offset,row_offset;
	short ncol,nrow,i,j;
	short error = 0;
	Point3 norm;
	Matrix3 svtm;

	if ((b = find_block(tgroup[2]))==NULL)  {
#ifdef IS_3DSDXF
		DebugPrint(" Couldn't find block %s\n", tgroup[2]);
#endif
		return(0);
		}
	
#ifdef DBGDXF1
	DebugPrint(" INSERT, level# %d,  block %s, layer = %s,gtextv=%s \n", blockLevel,tgroup[2], tgroup[8],gtextv);
#endif

	/*------ Save State--------- */
	svaddr = ftell(fp);
	sv_gcode = gcode;
	sv_block = curBlock;	
	sv_block_col = curBlockCol;	
	sv_gtextv = (char *)malloc(strlen(gtextv)+1);
	if (sv_gtextv==NULL) return(1);

	strcpy(sv_gtextv,gtextv);	
	svtm = curtm;

	sv_layer = (char *)malloc(strlen(curLayer)+1);
	if (sv_layer==NULL) {
		error = 1;
		goto bail;
		}
	strcpy(sv_layer,curLayer);

	/*--- Stash the insert point, scaling, rotation, repeat spacing & count ---*/
	load_vert(inspt,10);
	norm = normal;
	sclfct = Point3((float)rgroup[41],(float)rgroup[42],(float)rgroup[43]);
	rotang = (float)rgroup[50];
	ncol = igroup[70];
	nrow = igroup[71];
	dcol = (float)rgroup[44];
	drow = (float)rgroup[45];
	strcpy(curLayer,tgroup[8]);
	curBlock = b;
	curBlockCol = igroup[62];
	inInsert = TRUE;

#ifdef  DBGCOL 
	DebugPrint(" BlockCol = %d \n", curBlockCol);
#endif

#ifdef  DBGDXF1 
	DebugPrint(" InsPoint = (%.3f,%.3f,%.3f) norm=(%.3f,%.3f,%.3f)\n",
		inspt[0],inspt[1],inspt[2],norm[0],norm[1],norm[2]);
	DebugPrint(" Angle = %.4f, Scaling= (%.3f,%.3f,%.3f)\n",
		rotang,sclfct[0],sclfct[1],sclfct[2]);
#endif


	for (i=0; i<ncol; i++) {
		col_offset = i*dcol;
		for (j=0; j<nrow; j++) {
			row_offset = j*drow;

			/* position "read-head" at start of block */
			fseek(fp, b->fileAddr, SEEK_SET);
			gcode = 0;
			strcpy(gtextv,"BLOCK");
			curBlock = b;
			
			rgroup[10] = rgroup[20] = rgroup[30] = 0.0;

			/* Read in rest of BLOCK entity, to next 0-group */
			if (!inent()||errored) {
				inInsert = FALSE;
				return(FALSE);
				}
		
			/*---- Get the block's base point ----*/
			load_vert(basept,10);

			basept.y-=row_offset;
			basept.x-=col_offset;
		
			/* compute and concatenate transformation */
			ConcatTransf(basept,inspt,sclfct,rotang,norm);

#ifdef  DBGDXF1 
			DebugPrint(" basePoint = (%.3f,%.3f,%.3f) \n",
				basept[0],basept[1],basept[2]);
			pmat(" curtm", curtm);
#endif
		
			/*---- insert the block's contents--- */
			blockLevel++;
			if (!read_entities(1)) {
				error = 1;
				goto outahere;
				}
			blockLevel--;

			/* restore transform */
			curtm = svtm;
			}
		}	

	outahere:
	inInsert = FALSE;

	/*------ Restore State--------- */
	fseek(fp, svaddr, SEEK_SET);
	gcode = sv_gcode;
	curBlock = sv_block;
	curBlockCol = sv_block_col;	
	strcpy(gtextv,sv_gtextv);
	strcpy(curLayer,sv_layer);

#ifdef DBGDXF1
	DebugPrint(" LEAVING INSERT, level# %d,gtextv=%s,ename=%s\n\n", blockLevel,
		gtextv,ename);
#endif
bail:
	if (sv_gtextv) free(sv_gtextv);
	if (sv_layer) free(sv_layer);
	return(error);
	}

void CompPlaneEqn(float *plane, Point3 p0, Point3 p1, Point3 p2) {
	Point3 e1 = p1 - p0;
	Point3 e2 = p2 - p1;
	Point3 cross = Normalize(CrossProd(e1,e2));
	plane[0] = cross.x;
	plane[1] = cross.y;
	plane[2] = cross.z;
	plane[3] = -DotProd(p0,cross);
	}

float DistanceToPlane(Point3 p, float *plane) {
	Point3 planeVec(plane[0], plane[1], plane[2]);
	return(DotProd(planeVec,p) + plane[3]);
	}

/* 3 points a,b,c define a plane: compare Point p with them */
int PointInsideFace(Point3 p, Point3 a, Point3 b, Point3 c) {
	float plane[4];
	CompPlaneEqn(plane,a,b,c);
	return(DistanceToPlane(p,plane)<0.0);
	}

static int VertInsideFace(int nv, int nf) {
	Point3 va,vb,vc,vx;
	Face face;
	face = dxf_mesh->faces[nf];
	va = dxf_mesh->verts[face.v[0]];
	vb = dxf_mesh->verts[face.v[1]];
	vc = dxf_mesh->verts[face.v[2]];
	vx = dxf_mesh->verts[nv];
	return(PointInsideFace(vx, va, vb, vc));
	}

void flipface(Face *f) {
	int ab,bc;
#ifdef DBGNORM
 	DebugPrint("flipping \n");
#endif
	int hold = f->v[0];
	f->v[0] = f->v[2];
	f->v[2] = hold;
	ab = f->flags & EDGE_A;
	bc = f->flags & EDGE_B;
	f->flags &= (~EDGE_A & ~EDGE_B);
	if(ab)
		f->flags |= EDGE_B;
	if(bc)
		f->flags |= EDGE_A;
	}

static void FlipFaces(int nf, int n) {
	int i;
	Face *flist = dxf_mesh->faces;
	for (i=0; i<n; i++) 
		flipface(&flist[nf+i]);
	}

#define PFace(a,b,c) PutAFace(a,b,c, -1, EDGE_ALL)

static void PQuad(int a,int b,int c,int d) {
	PutAFace(a,b,c, -1, EDGE_A|EDGE_B);
	PutAFace(c,d,a, -1, EDGE_A|EDGE_B);
	}

/* _SOLID_ */
static int solid (void) {
	Point3 v[4];
	int i, nsides,nv,dbl,nf,nfaces;
	if (dxf_type==DXF3D) {
		entity++;
		if (find_dxfobj(DXF_MESH)==0) return(1);	
		nv = dxfverts;
		nf = dxffaces;
		CompEntityTransform(normal);
		SetThick();
		dbl = (isThick)?2:1;
		for (i=0; i<4; i++)	
			load_vert(v[i],10+i);
		if (EqualVectors(v[2],v[3])) {
			nsides = 3;
 			nfaces = isThick?8:1;
			if(!AddVertsFaces(dbl*3,nfaces)) return(1);
			}
		else {
			nsides = 4;
 			nfaces = isThick?12:2;
			if(!AddVertsFaces(dbl*4,nfaces)) return(1);
			}
		
		for (i=0; i<nsides; i++) 
			put_dbl_vert(v[i]);
			
		if (nsides==3) {
			PFace(nv, nv+2*dbl, nv+1*dbl); 
			if (isThick) {
				PQuad(nv,nv+2,nv+3,nv+1);				
				PQuad(nv+2,nv+4,nv+5,nv+3);				
				PQuad(nv+4,nv+0,nv+1,nv+5);				
				PFace(nv+1,nv+3,nv+5);
				}
			}
		else {
			PQuad(nv, nv+2*dbl, nv+3*dbl, nv+1*dbl);
			if (isThick) {
				PQuad(nv, nv+2, nv+3, nv+1);
				PQuad(nv+1, nv+3, nv+7, nv+5);
				PQuad(nv+3, nv+2, nv+6, nv+7);
				PQuad(nv+4, nv+5, nv+7, nv+6);
				PQuad(nv, nv+1, nv+5, nv+4);
				}
			}
		/* check the face parity: */
		if (isThick) {
			if (!VertInsideFace(nv+4,(nsides==3)?nf+1:nf+2)) 
				FlipFaces(nf,nfaces);
			}
		}
	return(0);
	}

static int trace(void) {
	return(solid());
	}

/* READ_ENTITIES  */

int read_entities(int skiphdr) {

	/* Read the first Entity header */
	if (!skiphdr) {
		if (!errored && !ingroup())	{
			dxferr();
			return(0);
			}
		}
	while (!errored && inent())	{
		if(ignore_layer(tgroup[8])||igroup[67])	   {
			/* No action, just ignore the entity */
#ifdef DBGDXF1
			if (igroup[67])
				DebugPrint(" Ignoring PAPER SPACE entity %s, layer =%s \n",ename,tgroup[8]);
			else 
				DebugPrint(" Ignoring entity on OFF LAYER %s \n",tgroup[8]);
#endif
			}
		else {
#ifdef DBGDXF
			DebugPrint(" Processing %s entity layer = %s, \n",ename,tgroup[8]);
#endif
			if (!strcmp(ename, "VERTEX")) {	errored=vert();	}
			else
			if (!strcmp(ename, "3DFACE")) {	errored=face3d(); }
			else
			if (!strcmp(ename, "LINE"))	{ errored=line(); }
			else
			if (!strcmp(ename, "POINT")) { errored=dxfpoint(); }
			else 
			if (!strcmp(ename, "CIRCLE"))	{ errored=circ(); }
			else
			if (!strcmp(ename, "ARC"))	{ errored=arc(); }
			else
			if (!strcmp(ename, "POLYLINE"))	{ errored=pline(); }
			else
			if (!strcmp(ename, "SOLID")) {	errored=solid();}
			else
			if (!strcmp(ename, "TRACE")) {	errored=trace();}
			else
			if (!strcmp(ename, "SEQEND")) {	errored=seqend();}
			else
			if (!strcmp(ename, "INSERT")) {	errored=insert(); }
#ifdef IS_3DSDXF
			else
			if (!strcmp(ename, "TEXT")) {	}
			else
			if (!strcmp(ename, "ATTDEF")) {	}
			else 
			if (!strcmp(ename, "ATTRIB")) {	}
			else {
	  			DebugPrint("     Unknown Entity: %s \n", ename);
				}
#endif
			}
		}
	if(errored)
		return(0);
	return(1);
	}

static void free_dxf_objects() {
	namedObjs.DeleteAll();
	}

static BOOL replaceScene = FALSE;



static INT_PTR CALLBACK
ImportDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_INITDIALOG:
			CheckRadioButton( hDlg, IDC_3DS_MERGE, IDC_3DS_REPLACE, replaceScene?IDC_3DS_REPLACE:IDC_3DS_MERGE );
			CenterWindow(hDlg,GetParent(hDlg));
			SetFocus(hDlg); // For some reason this was necessary.  DS-3/4/96
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
	            	replaceScene = IsDlgButtonChecked(hDlg,IDC_3DS_REPLACE);
					EndDialog(hDlg, 1);
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
		}
	return FALSE;
	}


int
dxf_load(const TCHAR *filename, DXFImport *imp) {
	BOOL ram_ok;
	int i;

	// Store import pointer
	theImport = imp;

	// Start out trying the file as text
	WorkFile theFile(filename,_T("r"));
	fp = theFile.Stream();
	if(!fp) {
		badopen:
		if(showPrompts)
			Alert(IDS_TH_ERR_OPENING_FILE);
		return -1;						// Didn't open!
		}
	imp->fileLength = theFile.Length();
		
	// Got the file -- Now put up the options dialog!
	if (showPrompts) {
		int result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_IMPORTOPTIONS), 
			imp->gi->GetMAXHWnd(), ImportOptionsDlgProc, (LPARAM)imp);
		if(result <= 0)
			return 0;
		}


	if (replaceScene) {
		if (!imp->impInt->NewScene())
			return IMPEXP_CANCEL;
		}
	
	binsent[18] = 0;
	
	/*  Peek at the header to see if it's binary  */
	
	if ((Read(fp,binsent,18) != 18) ||
		  	(strcmp(binsent, "AutoCAD Binary DXF") != 0))	{
		fseek(fp,0L,SEEK_SET);
		binary = FALSE;
		GetDecSymbolFromRegistry();
		}
	else {
		/* This is a binary DXF file.  To insure correct processing
			of binary information we must close the file and re-open
			it with the binary I/O attribute. */

		// Reopen the file as binary
		theFile.Close();
		theFile.Open(filename,_T("rb"));
		fp = theFile.Stream();
		if (!fp)
			goto badopen;

		// see if its R13 or later, which has WORD instead of BYTE codes
		Read(fp,binsent,24);
		isR13 = (binsent[23]==0)?1:0;
	
		/* Skip past the header */
		fseek(fp, 22L, SEEK_SET);  
		binary = TRUE;
		}
	
//	please_wait(progstr(STR0560));

	/*  Allocate dynamic storage to read file  */
	
	ram_ok=FALSE;

	Memory inBuf, gTextV, tGroup[10], rGroup, iGroup;
	if(!(inbuf = (char *)inBuf.Alloc(MAXGTXT,TRUE)))
		goto bailout;
	if(!(gtextv = (char *)gTextV.Alloc(MAXGTXT,TRUE)))
		goto bailout;
	for (i = 0; i < 10; i++) {
		if(!(tgroup[i] = (char *)tGroup[i].Alloc(MAXGTXT,TRUE)))
			goto bailout;
		}
	if(!(rgroup = (double *)rGroup.Alloc(60 * sizeof(double),TRUE)))
		goto bailout;
	if(!(igroup = (short *)iGroup.Alloc(80 * sizeof(short),TRUE)))
		goto bailout;
	
	ram_ok=TRUE;
	entnum=0;
	
	/*  Ignore all of DXF file before ENTITIES section  */
	/*  This code could be modified to recognized other sections */

	curtm.IdentityMatrix();	

	blockLevel=0;
	errored = FALSE;
	while (!errored && ingroup()){
		if (gcode == 0 && !strcmp(gtextv, "SECTION")) {
			if (!ingroup())	{  dxferr(); break;	}
			if (gcode != 2) {  dxferr(); break;	}
			if (!strcmp(gtextv, "BLOCKS"))
				read_blocks();
			else
			if (!strcmp(gtextv, "TABLES"))
				read_tables();
			else
			if (!strcmp(gtextv, "ENTITIES"))
				read_entities(0);
			}
		}

	/* Release dynamic storage used whilst reading DXF file */
	
 bailout:
	if(!ram_ok) {
		if(showPrompts)
			no_RAM();
		return(-1);
		}
	else
	if (errored) {
		
		free_dxf_objects();	

		switch(bad_reason) {
			case INVALID_FILE:
				if(showPrompts)
					Alert(IDS_TH_ERRORINDXF);
				break;
			case PARTIAL_READ:
				if(showPrompts)
					Alert(IDS_TH_PARTIALREAD);
				break;
			case USER_CANCEL:	// No need to return an error state on this
				return 1;
			default:
				if(showPrompts)
					Alert(IDS_TH_UNKNOWNERR);
				break;
			}
		return(-1);
		}
	return 1;
	}

// Weld a mesh based on a weld threshold.
// There are two parts to this procedure:
// (1) Build a BitArray noting the verts to weld together
// (2) Weld 'em!
// This code "borrowed" from Rolf's Edit Mesh modifier and altered to suit my needs

DWORD GetOtherIndex(Face *f,DWORD v0,DWORD v1)
	{
	for (int i=0; i<3; i++) {
		if (f->v[i]!=v0 && f->v[i]!=v1) return f->v[i];
		}
	// Invalid face
	return 0;
	}

int GetEdgeIndex(Face &f,DWORD v0,DWORD v1)
	{
	int j0, j1;
	for (int i=0; i<3; i++) {
		if (f.v[i]==v0) j0 = i;
		if (f.v[i]==v1) j1 = i;
		}
	switch (j0) {
		case 0: return j1==1?0:2;
		case 1: return j1==2?1:0;
		case 2: return j1==0?2:1;
		default:
			assert(0);
			return 0;
		}	
	}

void MeshDeleteFaces(Mesh &mesh,BitArray& set)
	{
	int j = 0;
	for (int i=0; i<mesh.getNumFaces(); i++) {
		if (set[i]) continue;		
		mesh.faces[j] = mesh.faces[i];		
		if (mesh.tvFace) {
			mesh.tvFace[j] = mesh.tvFace[i];
			}
		j++;
		}
	if (j!=mesh.getNumFaces()) {
		mesh.setNumFaces(j,TRUE);
		}
	}

void MeshDeleteVertices(Mesh &mesh,BitArray& set)
	{
	int j = 0;
	for (int i=0; i<mesh.getNumVerts(); i++) {
		if (set[i]) continue;		
		mesh.verts[j] = mesh.verts[i];		
		j++;
		}
	if (j!=mesh.getNumVerts()) {
		mesh.setNumVerts(j,TRUE);
		}
	}

void WeldVertSet(Mesh *mesh,BitArray &set)
	{
	int i;
	int firstSel = -1, numSel=0;
	Point3 pt(0,0,0), dt;

	// Find the first selected vert and find the average of all
	// selected verts
	for (i=0; i<set.GetSize(); i++) {
		if (set[i]) {
			if (firstSel<0) firstSel = i;
			numSel++;
			pt += mesh->verts[i];
			}
		}
	if (firstSel < 0) return;
	pt /= float(numSel);

	// Move the first point to the average position of all verts
	// getting welded.
	dt = pt-mesh->verts[firstSel];
	mesh->verts[firstSel] = pt;

	// Go through each face. If exactly one of the faces vertices is
	// going to be welded, Then remap it. If more than one is going to
	// be welded then delete the face.
	BitArray delSet;
	delSet.SetSize(mesh->getNumFaces());
	for (i=0; i<mesh->getNumFaces(); i++) {
		int ct = 0;
		for (int j=0; j<3; j++) {
			if (set[mesh->faces[i].v[j]]) ct++;			
			}

		if (ct > 1) delSet.Set(i); // Delete it
		else if (ct == 1) {
			// Remap it.
			DWORD v[3] = {mesh->faces[i].v[0],mesh->faces[i].v[1],mesh->faces[i].v[2]};
			for (int j=0; j<3; j++) {
				if (set[mesh->faces[i].v[j]]) {
					v[j] = firstSel;
					mesh->faces[i].v[j] = firstSel;
					break;
					}
				}
			}
		}
	int j=0;
	for (i=0; i<mesh->getNumFaces(); i++) {
		if (!delSet[i])
			j++;
		}
	MeshDeleteFaces(*mesh,delSet);	
	
	// Build the delMap. This is a table with one entry per vertex. The
	// entry is the number of vertices deleted before this vertex.
	DWORDTab delMap;
	DWORD deleted = 0;
	delMap.SetCount(mesh->getNumVerts());
	for (i=0; i<mesh->getNumVerts(); i++) {
		delMap[i] = deleted;
		if (set[i] && i!=firstSel) {
			deleted++;
			}
		}

	// Now remap the faces using the delMap
	for (i=0; i<mesh->getNumFaces(); i++) {
		DWORD v[3];
		v[0] = mesh->faces[i].v[0] - delMap[mesh->faces[i].v[0]];
		v[1] = mesh->faces[i].v[1] - delMap[mesh->faces[i].v[1]];
		v[2] = mesh->faces[i].v[2] - delMap[mesh->faces[i].v[2]];		
		mesh->faces[i].v[0] = v[0];
		mesh->faces[i].v[1] = v[1];
		mesh->faces[i].v[2] = v[2];
		}

	// Go through all selected vertices, except the first and delete them.	
	set.Set(firstSel,FALSE);
	j=firstSel+1;
	for (i=firstSel+1; i<mesh->getNumVerts(); i++) {
		if (!set[i])
			j++;
		}
	MeshDeleteVertices(*mesh,set);

	mesh->InvalidateEdgeList();
	mesh->InvalidateGeomCache();	
	}


static void WeldVerts(Mesh *mesh,float thresh)
	{
	int i, j;	
	// So we can compare length squared
	thresh = thresh * thresh;

#ifdef DBGFINAL
int origVerts = mesh->getNumVerts();
#endif // DBGFINAL
	
	for (i=0; i<mesh->getNumVerts(); i++) {
		// We'll flag verts that are within the threshold for the ith vert
		BitArray set;
		set.SetSize(mesh->getNumVerts());			
		set.Set(i);
		int found = 0;

		for (j=0; j<mesh->getNumVerts(); j++) {
			if (i==j) continue;

			if (LengthSquared(mesh->verts[i]-mesh->verts[j]) < thresh) {
				set.Set(j);
				found++;
				}
			}
		
		// If we found any to weld, weld away!
		if (found)
			WeldVertSet(mesh,set);
		}

#ifdef DBGFINAL
int finalVerts = mesh->getNumVerts();
if(finalVerts != origVerts)
	DebugPrint("Weld dropped vert count to %d from %d\n",finalVerts,origVerts);
#endif // DBGFINAL
	}

inline Point3 FaceNormal( Mesh& mesh, DWORD fi )
	{
	Face *f = &mesh.faces[fi];
	Point3 v1 = mesh.verts[f->v[1]] - mesh.verts[f->v[0]];
	Point3 v2 = mesh.verts[f->v[2]] - mesh.verts[f->v[1]];
	return v1^v2;
	}

/* Finalize the 3D DXF or filmroll load */

BOOL dxf_finalstep(ImpInterface *iface) {
	int ox;
	NamedObject *n;
	Mesh *m;
	BezierShape *s;

#ifdef xxDBGFINAL
	prblocks();
#endif
	
	free_lists();

#ifdef xxDBGDXF
		    dump_objects(); 
#endif
	theImport->gi->ProgressUpdate(0, FALSE, GetString(IDS_TH_FINALIZING));
	/* Fix up the new objects */
	for(ox=0; ox<namedObjs.Count(); ++ox) {
		theImport->gi->ProgressUpdate(50 + ox * 100 / namedObjs.Count() / 2);
		if (user_cancel()) break;
		n=namedObjs.GetNamedPtr(ox);
		switch(n->type) {
			case DXF_MESH: {
				m = &n->mesh;
		//		check_faces(m);
				int verts = m->getNumVerts();
				int faces = m->getNumFaces();
#ifdef DBGFINAL
				DebugPrint("     Object %s created, %d Verts,  %d Faces\n", n->name, verts, faces);
#endif
				if(verts || faces) {
					if (theImport->weld) {
#ifdef DBGFINAL
						DebugPrint("     		Welding (%f)...", theImport->thresh);
#endif
						WeldVerts(m, theImport->thresh);

#ifdef DBGFINAL
						DebugPrint("    %d Verts,  %d Faces\n", m->getNumVerts(), m->getNumFaces());
#endif
						}
					if (theImport->weld || theImport->removeDouble) {
#ifdef DBGFINAL
						DebugPrint("     		Removing double faces...");
#endif
						RemoveDoubleFaces(m);
						m->RemoveDegenerateFaces();

#ifdef DBGFINAL
						DebugPrint("    %d Verts,  %d Faces\n", m->getNumVerts(), m->getNumFaces());
#endif
						}
					if (user_cancel()) break;

					m->RemoveIllegalFaces();

					if (theImport->unifyNormals) {
#ifdef DBGFINAL
						DebugPrint("     		Unifying Normals...\n");
#endif
						m->UnifyNormals(FALSE);
						}
					if (user_cancel()) break;
					if (theImport->smooth) {
#ifdef DBGFINAL
						DebugPrint("     		Smoothing...\n");
#endif
						m->AutoSmooth(theImport->smoothAngle * DEG_TO_RAD,FALSE);
						}
			
					// Now add the objects to the MAX database...

					m->buildNormals();
					m->EnableEdgeList(1);

					ImpNode *node = iface->CreateNode();
					if(node) {
						TriObject *tri = CreateNewTriObject();
						// Now find the center of the vertices and use that as the pivot
						int verts = m->getNumVerts();
						Point3 accum(0,0,0);
						for(int i = 0; i < verts; ++i)
							accum += m->verts[i];
						Point3 delta = accum / (float)verts;
						for(i = 0; i < verts; ++i)
							m->verts[i] -= delta;
						tri->GetMesh() = *m;
						node->Reference(tri);
						Matrix3 tm;
						tm.IdentityMatrix();		// Reset initial matrix to identity
						tm.SetTrans(delta);			// Add in the center point
						node->SetTransform(0,tm);
						iface->AddNodeToScene(node);
						node->SetName(_T(n->name));
						// Get access to the actual node for the next part
						INode *inode = node->GetINode();
						inode->SetWireColor(DWORD_FROM_OC(aciColors[n->color]));
						}
					else
						return FALSE;

					}
				}
				break;
			case DXF_SHAPE: {
				s = &n->shape;
				// Delete any empty splines
				int splines = s->SplineCount();
				s->UpdateSels();
				for(int i = splines - 1; i >= 0; --i) {
					Spline3D *spline = s->splines[i];
					if(spline->KnotCount() < 2)
						s->DeleteSpline(i);
					}
				// Now add the shape to the scene if it's got anything left!
				if(s->SplineCount()) {
					s->UpdateSels();

					ImpNode *node = iface->CreateNode();
					if(node) {
						SplineShape *shape = new SplineShape;
						// Now find the center of the vertices and use that as the pivot
						int polys = s->SplineCount();
						Point3 accum(0,0,0);
						int verts = 0;
						for(int poly = 0; poly < polys; ++poly) {
							Spline3D *spline = s->splines[poly];
							int knots = spline->KnotCount();
							verts += knots;
							for(int k = 0; k < knots; ++k)
								accum += spline->GetKnotPoint(k);
							}
						Point3 delta = accum / (float)verts;
						Matrix3 tm = TransMatrix(-delta);
						for(poly = 0; poly < polys; ++poly)
							s->splines[poly]->Transform(&tm);

						shape->shape = *s;
						node->Reference(shape);
						tm.IdentityMatrix();		// Reset initial matrix to identity
						tm.SetTrans(delta);			// Add in the center point
						node->SetTransform(0,tm);
						iface->AddNodeToScene(node);
						node->SetName(_T(n->name));
						// Get access to the actual node for the next part
						INode *inode = node->GetINode();
						inode->SetWireColor(DWORD_FROM_OC(aciColors[n->color]));
						}
					else
						return FALSE;

					}
				}
				break;
			}
		}
#ifdef DBGFINAL
				DebugPrint("Final complete!\n");
#endif
	theImport->gi->ProgressUpdate(100);

	return TRUE;
	}

// Dummy thread routine
DWORD WINAPI dummy(LPVOID arg) { return 0; }

int
DXFImport::DoImport(const TCHAR *filename,ImpInterface *iface,Interface *gi, BOOL suppressPrompts) {
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;

	int status;

	impInt = iface;
	this->gi = gi;

	inInsert = FALSE;		
	dxf_type= DXF3D;
	curBlock = lastBlock = NULL;
	last_entity=entity=0;
	dxf_mesh = NULL;
	dxf_shape = NULL;
	dxf_spline = NULL;
	chknum = 0;
	//FIXME: how many other statics need to be initialized here?

	if(suppressPrompts) {	// Set default parameters here
		objsFrom = OBJS_LAYER;			// Derive objects from...
		thresh = 0.01f;					// Weld threshold
		weld = TRUE;
		smoothAngle = 30.0f;				// Smooth angle
		smooth = TRUE;
		arcDegrees = 10.0f;
		arcSubDegrees = 90.0f;
		removeDouble = TRUE;
		fillPolylines = TRUE;
		unifyNormals = TRUE;
		}
	else {
		if (!DialogBox(hInstance, MAKEINTRESOURCE(IDD_MERGEORREPL), gi->GetMAXHWnd(), ImportDlgProc))
			return IMPEXP_CANCEL;
		}
	
	gi->ProgressStart(GetString(IDS_TH_IMPORTINGDXF), TRUE, dummy, 0);
	status = dxf_load(filename, this);

	if(status == 0)
		status = IMPEXP_CANCEL;	
	else {
		if(status > 0)	{
			if(!dxf_finalstep(iface))
				DebugPrint("Error loading into MAX\n");
			}
		}

	// Get rid of our named objects list
	free_dxf_objects();

	FreeNameList(); 

	// Get rid of the progress bar
	gi->ProgressEnd();

	return(status<=0?IMPEXP_FAIL:status);
	}

static void	RemoveDoubleFaces(Mesh *m) {
	Edge *edgeList;
	int numedges;
	// clear all flags
	for (int i=0; i<m->numFaces; i++)	
		m->faces[i].flags &= ~FACE_WORK;
	if ((edgeList = m->MakeEdgeList(&numedges,1))==NULL) 
		return;
	delete[] edgeList;

	// Now delete all the flagged faces
	m->DeleteFlaggedFaces();
	}

