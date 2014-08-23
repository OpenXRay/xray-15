/**********************************************************************
 *<
	FILE: dxfexp.cpp

	DESCRIPTION:  .DXF file export module

	CREATED BY: Tom Hudson

	HISTORY: created 30 November 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include "dxferes.h"


#include <stdarg.h>

static BOOL showPrompts;
static BOOL exportSelected;

static TCHAR decimalSymbol = _T('.');

static void GetDecSymbolFromRegistry() {
    HKEY hKey;
    DWORD dwType, numBytes;
	long status;
    unsigned char symbol[80];
	decimalSymbol = _T('.');
	status = RegOpenKeyEx (HKEY_CURRENT_USER, 
		      "Control Panel\\International", 
		      0, KEY_READ, &hKey);
    if (status == ERROR_SUCCESS) {
		if (ERROR_SUCCESS==RegQueryValueEx (hKey, "sDecimal", 0, &dwType, 
			 symbol, &numBytes))
			decimalSymbol = symbol[0];
	    }
	}

TCHAR *FixDecSymbol(TCHAR *buf)
	{
	TCHAR *cp = buf;
	while(*cp) {
		if( *cp == decimalSymbol)
			*cp = _T('.');
		cp++;
		}
	return buf;
	}

int Xprintf(FILE *stream, const TCHAR *format, ...) {
	TCHAR buf[512];
	va_list args;
	va_start(args,format);
	_vsntprintf(buf,512,format,args);
	va_end(args);
	FixDecSymbol(buf);
	return fputs(buf, stream);
	}

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

static int Alert(int s1, int s2 = IDS_DXFEXP, int option = MB_OK) {
	return MessageBox(s1, s2, option);
	}

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

static int ColDist(ObjectColors a, ObjectColors b) {
	int dr = a.r-b.r;
	int dg = a.g-b.g;
	int db = a.b-b.b;
	return dr*dr + dg*dg + db*db;
	}

static int FindACIColor(DWORD color) {
	ObjectColors dc;
	dc.r = (BYTE)(color&255);
	dc.g = (BYTE)((color>>8)&255);
	dc.b = (BYTE)((color>>16)&255);
	int ibest=-1;
	int dist=1000000;
	for (int i=1; i<255; i++) {
		int d = ColDist(aciColors[i],dc);
		if (d<dist) {
			dist = d;
			ibest = i;
			}
		}
	return ibest;
	}

#define no_RAM() Alert(IDS_TH_OUTOFMEMORY)

#define LAYERS_BY_OBJECT 0
#define LAYERS_BY_MATERIAL 1
#define LAYERS_ONE_LAYER 2

#define NUM_SOURCES 3 		// # of sources in dialog

class DXFExport : public SceneExport {
	friend INT_PTR CALLBACK ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static	int		layersFrom;					// Derive layers from...
					DXFExport();
					~DXFExport();
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
	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);	// Export file
	BOOL			SupportsOptions(int ext, DWORD options);
	};

// Statics
int					DXFExport::layersFrom = LAYERS_BY_OBJECT;			// Derive layers from...

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
	void *			Create(BOOL loading = FALSE) { return new DXFExport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_AUTOCAD); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0xd1e,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEEXPORT);  }
	};

static DXFClassDesc DXFDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_DXFEXPORTDLL); }

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
// .DXF export module functions follow:
//

DXFExport::DXFExport() {
	}

DXFExport::~DXFExport() {
	}

int
DXFExport::ExtCount() {
	return 1;
	}

const TCHAR *
DXFExport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("DXF");
		}
	return _T("");
	}

const TCHAR *
DXFExport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_AUTOCADDXFFILE);
	}
	
const TCHAR *
DXFExport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_AUTOCAD);
	}

const TCHAR *
DXFExport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
DXFExport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
DXFExport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
DXFExport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
DXFExport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
DXFExport::ShowAbout(HWND hWnd) {			// Optional
 	}

static int deriveButtons[] = { IDC_BYOBJECT, IDC_BYMATERIAL, IDC_1LAYER };

static INT_PTR CALLBACK
ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static DXFExport *exp;

	switch(message) {
		case WM_INITDIALOG:
			exp = (DXFExport *)lParam;
			CheckDlgButton( hDlg, deriveButtons[exp->layersFrom], TRUE);
			CenterWindow(hDlg,GetParent(hDlg));
			SetFocus(hDlg); // For some reason this was necessary.  DS-3/4/96
			return FALSE;
		case WM_DESTROY:
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
					// Unload values into DXFExport statics
					for(int i = 0; i < NUM_SOURCES; ++i) {
						if(IsDlgButtonChecked(hDlg, deriveButtons[i])) {
							exp->layersFrom = i;
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

/* Fix our names (perhaps lowercase, perhaps with spaces) for DXF */

void
dxf_fix_name(TSTR &name)
	{
	int ix;
	
	//SS 12/17/99 defect 205944: The previous list of invalid characters
	// was woefully inadequate. I have made 2 changes: first, I am not
	// uppercasing the string, since A2K supports (stores) the case of
	// the name; previous versions of AutoCAD should uppercase it for us.
	// Second, I've decided to limit the valid characters to those allowed
	// in versions prior to A2K. If at some point we decide to "move on"
	// and support their ESNs (extended symbol names), these are the
	// characters that are still disallowed: <>/\":?*|,=`	
	//name.toUpper();
	for(ix=0; ix<name.Length(); ++ix)
		{
		//if(name[ix]==' '||name[ix]=='/'||name[ix]>127||name[ix]=='#')
		if(!(isalnum(name[ix]) || name[ix]=='_' || name[ix]=='$' || name[ix]=='-'))
			name[ix]='_';
		}
	if (name.Length()>31) name.Resize(31);
	}

class MySceneEntry {
	public:
		INode *node;
		Object *obj;
		MySceneEntry *next;
		MySceneEntry(INode *n, Object *o) { node = n; obj = o; next = NULL; }
	};

class MySceneEnumProc : public ITreeEnumProc {
	public:
		Interface	*i;
		MySceneEntry *head;
		MySceneEntry *tail;
		IScene		*theScene;
		int			count;
		TimeValue	time;
					MySceneEnumProc(IScene *scene, TimeValue t, Interface *i);
					~MySceneEnumProc();
		int			Count() { return count; }
		void		Append(INode *node, Object *obj);
		int			callback( INode *node );
		Box3		Bound();
		MySceneEntry *operator[](int index);
	};

MySceneEnumProc::MySceneEnumProc(IScene *scene, TimeValue t, Interface *i) {
	time = t;
	theScene = scene;
	count = 0;
	head = tail = NULL;
	this->i = i;
	theScene->EnumTree(this);
	}

MySceneEnumProc::~MySceneEnumProc() {
	while(head) {
		MySceneEntry *next = head->next;
		delete head;
		head = next;
		}
	head = tail = NULL;
	count = 0;	
	}

int MySceneEnumProc::callback(INode *node) {
	if(exportSelected && node->Selected() == FALSE)
		return TREE_CONTINUE;
	Object *obj = node->EvalWorldState(time).obj;
	if(obj->CanConvertToType(triObjectClassID)) {
		Append(node, obj);
		}
	return TREE_CONTINUE;	// Keep on enumeratin'!
	}

void MySceneEnumProc::Append(INode *node, Object *obj) {
	MySceneEntry *entry = new MySceneEntry(node, obj);
	if(tail)
		tail->next = entry;
	tail = entry;
	if(!head)
		head = entry;
	count++;	
	}

Box3 MySceneEnumProc::Bound() {
	Box3 bound;
	bound.Init();
	MySceneEntry *e = head;
	ViewExp *vpt = i->GetViewport(NULL);
	while(e) {
		Box3 bb;
		e->obj->GetWorldBoundBox(time, e->node, vpt, bb);
		bound += bb;
		e = e->next;
		}
	return bound;
	}

MySceneEntry *MySceneEnumProc::operator[](int index) {
	MySceneEntry *e = head;
	while(index) {
		e = e->next;
		index--;
		}
	return e;
	}

static int ColorNumber(int i) {
	// want to avoid 0 (by block) and 256 (by layer).
	return (i%255)+1;
	}

int
dxf_save(const TCHAR *filename, ExpInterface *ei, Interface *gi, DXFExport *exp) {

	if(showPrompts) {
		// Put up the options dialog to find out how they want the file written!
		int result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EXPORTOPTIONS), 
			gi->GetMAXHWnd(), ExportOptionsDlgProc, (LPARAM)exp);
		if(result <= 0)
			return 0;
		}
	else {	// Set default parameters here
		DXFExport::layersFrom = LAYERS_BY_OBJECT;
		}

	// Make sure there are nodes we're interested in!
	// Ask the scene to enumerate all its nodes so we can determine if there are any we can use
	MySceneEnumProc myScene(ei->theScene, gi->GetTime(), gi);

	// Any useful nodes?
	if(!myScene.Count()) {
		if(showPrompts)
			Alert(IDS_TH_NODATATOEXPORT);
		return 1;
		}

	GetDecSymbolFromRegistry();

	// Output the file as text
	WorkFile theFile(filename,_T("w"));
	FILE *stream = theFile.Stream();
	if(!stream) {
		if(showPrompts)
			Alert(IDS_TH_CANTCREATE);
		return -1;						// Didn't open!
		}

	// Build a bounding box for the objects we're exporting
	Box3 bbox = myScene.Bound();

	// Let's get busy!

	int ox,a,b,c;
	int ix;
	TSTR layername;
	
	/* Write header stuff */
	
	if(fprintf(stream,"0\nSECTION\n2\nHEADER\n")<0)
		{
		wrterr:
		if(showPrompts)
			Alert(IDS_TH_WRITEERROR);
		theFile.Close();
		remove(filename);
		return 0;
		}
	
	if(fprintf(stream,"9\n$ACADVER\n1\nAC1008\n")<0)
		goto wrterr;
	if(fprintf(stream,"9\n$UCSORG\n10\n0.0\n20\n0.0\n30\n0.0\n")<0)
		goto wrterr;
	if(fprintf(stream,"9\n$UCSXDIR\n10\n1.0\n20\n0.0\n30\n0.0\n")<0)
		goto wrterr;
	if(fprintf(stream,"9\n$TILEMODE\n70\n1\n")<0)
		goto wrterr;
	if(fprintf(stream,"9\n$UCSYDIR\n10\n0.0\n20\n1.0\n30\n0.0\n")<0)
		goto wrterr;
	if(Xprintf(stream,"9\n$EXTMIN\n10\n%f\n20\n%f\n30\n%f\n",bbox.pmin.x,bbox.pmin.y,bbox.pmin.z)<0)
		goto wrterr;
	if(Xprintf(stream,"9\n$EXTMAX\n10\n%f\n20\n%f\n30\n%f\n",bbox.pmax.x,bbox.pmax.y,bbox.pmax.z)<0)
		goto wrterr;
	if(fprintf(stream,"0\nENDSEC\n")<0)
		goto wrterr;
	
	/* Tables section */
	
	if(fprintf(stream,"0\nSECTION\n2\nTABLES\n")<0)
		goto wrterr;
	
	/* Continuous line type */
	if(fprintf(stream,"0\nTABLE\n2\nLTYPE\n70\n1\n0\nLTYPE\n2\nCONTINUOUS\n70\n64\n3\nSolid line\n72\n65\n73\n0\n40\n0.0\n")<0)
		goto wrterr;
	if(fprintf(stream,"0\nENDTAB\n")<0)
		goto wrterr;
	
	/* Object names for layers */
	if(fprintf(stream,"0\nTABLE\n2\nLAYER\n70\n%d\n",myScene.Count())<0)
		goto wrterr;
	switch(exp->layersFrom) {
		case LAYERS_BY_MATERIAL: {
			NameTab mtlNames;
			for(ox=0; ox<myScene.Count(); ++ox) {
				INode *n = myScene[ox]->node;
				Mtl *mtl = n->GetMtl();
				if (mtl==NULL) layername = GetString(IDS_NO_MATERIAL);
				else layername = mtl->GetName();
				if (!mtlNames.FindName(layername)) {
					mtlNames.AddName(layername);
					dxf_fix_name(layername);
					if(fprintf(stream,"0\nLAYER\n2\n%s\n70\n0\n62\n%d\n6\nCONTINUOUS\n",
						layername.data(),
						FindACIColor(n->GetWireColor())			//ColorNumber(ox)
						)<0)
						goto wrterr;
					}
				}
			}
			break;
		case LAYERS_BY_OBJECT:
			for(ox=0; ox<myScene.Count(); ++ox) {
				INode *n = myScene[ox]->node;
				layername = TSTR(n->GetName());
				dxf_fix_name(layername);
				if(fprintf(stream,"0\nLAYER\n2\n%s\n70\n0\n62\n%d\n6\nCONTINUOUS\n",
					layername.data(),FindACIColor(n->GetWireColor()))<0)
						goto wrterr;
				}
			break;
		case LAYERS_ONE_LAYER:
			if(fprintf(stream,"0\nLAYER\n2\n0\n70\n0\n62\n7\n6\nCONTINUOUS\n")<0)
				goto wrterr;
		}
	if(fprintf(stream,"0\nENDTAB\n")<0)
		goto wrterr;
	
	/* Default style */
	if(fprintf(stream,"0\nTABLE\n2\nSTYLE\n70\n1\n0\nSTYLE\n2\nSTANDARD\n70\n0\n40\n0.0\n41\n1.0\n50\n0.0\n71\n0\n42\n0.2\n3\ntxt\n4\n\n0\nENDTAB\n")<0)
		goto wrterr;
	
	/* Default View? */
	
	/* UCS */ 
	if(fprintf(stream,"0\nTABLE\n2\nUCS\n70\n0\n0\nENDTAB\n")<0)
		goto wrterr;
	
	if(fprintf(stream,"0\nENDSEC\n")<0)
		goto wrterr;
	
	/* Entities section */
	
	if(fprintf(stream,"0\nSECTION\n2\nENTITIES\n")<0)
		goto wrterr;
	for(ox=0; ox<myScene.Count(); ++ox)
		{
		INode *n = myScene[ox]->node;
		Object *obj = myScene[ox]->obj;
		TriObject *tri = (TriObject *)obj->ConvertToType(gi->GetTime(), triObjectClassID);
		Mesh &mesh = tri->GetMesh();
		Matrix3 tm = n->GetObjectTM(gi->GetTime());
		if (mesh.numFaces==0)
			continue;
		switch(exp->layersFrom) {
			case LAYERS_BY_MATERIAL:{
				Mtl *mtl = n->GetMtl();
				if (mtl==NULL) layername = GetString(IDS_NO_MATERIAL);
				else layername = mtl->GetName();
				dxf_fix_name(layername);
				}
				break;
			case LAYERS_BY_OBJECT:
				layername = TSTR(n->GetName());
				dxf_fix_name(layername);
				break;
			case LAYERS_ONE_LAYER:
				layername = TSTR(_T("0"));
				break;
			}

		int verts = mesh.getNumVerts();
		int faces = mesh.getNumFaces();
		if(fprintf(stream,"0\nPOLYLINE\n8\n%s\n66\n1\n70\n64\n71\n%u\n72\n%u\n",
								layername.data(),verts,faces)<0) {
			write_error:
			// Delete the working object, if necessary
			if(obj != (Object *)tri)
				tri->DeleteThis();
			goto wrterr;
			}
		if (fprintf(stream,"62\n%d\n",FindACIColor(n->GetWireColor()))<0) goto write_error;

		Point3 vert;
		Face face;
		for(ix=0; ix<verts; ++ix) {
			vert = mesh.verts[ix] * tm;
			if(Xprintf(stream,
				"0\nVERTEX\n8\n%s\n10\n%.6f\n20\n%.6f\n30\n%.6f\n70\n192\n",
					layername.data(),vert.x,vert.y,vert.z)<0)
				goto write_error;
			}

		BOOL isFlipped = DotProd(CrossProd(tm.GetRow(0),tm.GetRow(1)),tm.GetRow(2)) < 0.0f ? 1 : 0;

		for(ix=0; ix<faces; ++ix)
			{
			face = mesh.faces[ix];
			if (isFlipped) {
				a=face.v[0]+1;
				b=face.v[2]+1;
				c=face.v[1]+1;
				if(!(face.flags & EDGE_C))	a= -a;
				if(!(face.flags & EDGE_B))	b= -b;
				if(!(face.flags & EDGE_A))	c= -c;
				}
			else {
				a=face.v[0]+1;
				b=face.v[1]+1;
				c=face.v[2]+1;
				if(!(face.flags & EDGE_A))	a= -a;
				if(!(face.flags & EDGE_B))	b= -b;
				if(!(face.flags & EDGE_C))	c= -c;
				}
			if(fprintf(stream,
				"0\nVERTEX\n8\n%s\n10\n0\n20\n0\n30\n0\n70\n128\n71\n%d\n72\n%d\n73\n%d\n",
								layername.data(),a,b,c)<0)
				goto write_error;
			}
		if(fprintf(stream,"0\nSEQEND\n8\n%s\n",layername.data())<0)
			goto write_error;
		// Delete the working object, if necessary
		if(obj != (Object *)tri)
			tri->DeleteThis();
		}
	
	if(fprintf(stream,"0\nENDSEC\n0\nEOF\n")<0)
		goto wrterr;
	
	if(theFile.Close())
		goto wrterr;

	return 1;	
	}

int
DXFExport::DoExport(const TCHAR *filename,ExpInterface *ei,Interface *gi, BOOL suppressPrompts, DWORD options) {
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	int status;
		
	status=dxf_save(filename, ei, gi, this);

	if(status == 0)
		return 1;		// Dialog cancelled
	if(status > 0)	{
#ifdef DBGDXF
DebugPrint("DXF status OK!\n");
#endif
		}
#ifdef DBGDXF
	else
	if(status < 0)
		DebugPrint("Error somewhere in DXF!\n");
#endif

	return(status);
	}

BOOL DXFExport::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
	}

