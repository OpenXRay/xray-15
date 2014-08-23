/**********************************************************************
 *<
	FILE: aiexp.cpp

	DESCRIPTION:  .AI file export module

	CREATED BY: Tom Hudson

	HISTORY: created 4 September 1998

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include "aieres.h"
#include "shape.h"

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

static int Alert(int s1, int s2 = IDS_TH_AIEXP, int option = MB_OK) {
	return MessageBox(s1, s2, option);
	}


class AIExport : public SceneExport {
	friend INT_PTR CALLBACK ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static	int		layersFrom;					// Derive layers from...
					AIExport();
					~AIExport();
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

// Max interface code

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
			//MessageBox(NULL,_T("AIEXP.DLL: DllMain"),_T("AIEXP"),MB_OK);
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

class AIClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new AIExport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_ADOBE); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0x3fe3,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEEXPORT);  }
	};

static AIClassDesc AIDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_AIEXPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &AIDesc; break;
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
// .AI export module functions follow:
//

AIExport::AIExport() {
	}

AIExport::~AIExport() {
	}

int
AIExport::ExtCount() {
	return 1;
	}

const TCHAR *
AIExport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("AI");
		}
	return _T("");
	}

const TCHAR *
AIExport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_AIFILE);
	}
	
const TCHAR *
AIExport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_ADOBE);
	}

const TCHAR *
AIExport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
AIExport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
AIExport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
AIExport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
AIExport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
AIExport::ShowAbout(HWND hWnd) {			// Optional
 	}

#ifdef MAYBE
static INT_PTR CALLBACK
ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static AIExport *exp;

	switch(message) {
		case WM_INITDIALOG:
			exp = (AIExport *)lParam;
			CheckDlgButton( hDlg, deriveButtons[exp->layersFrom], TRUE);
			CenterWindow(hDlg,GetParent(hDlg));
			SetFocus(hDlg); // For some reason this was necessary.  DS-3/4/96
			return FALSE;
		case WM_DESTROY:
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
					// Unload values into AIExport statics
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
#endif //MAYBE

class MySceneEntry {
	public:
		INode *node;
		Object *obj;
		ShapeObject *so;
		BezierShape shape;
		MySceneEntry *next;
		MySceneEntry(INode *n, Object *o, ShapeObject *s) { node = n; obj = o; so = s; next = NULL; }
		~MySceneEntry() { if (so != obj) so->DeleteThis(); }
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
		MySceneEntry *Append(INode *node, Object *o, ShapeObject *so);
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
	// We only deal with shapes...
	if(obj->SuperClassID() == SHAPE_CLASS_ID) {
		// Must be able to convert to beziers for Adobe export
		ShapeObject *so = (ShapeObject *)obj;
		if(so->CanMakeBezier()) {
			MySceneEntry *entry = Append(node, obj, so);
			so->MakeBezier(time, entry->shape);
			}
		else {
			// Get rid of this work object!
			if(so != obj)
				so->DeleteThis();
			}
		}
	return TREE_CONTINUE;	// Keep on enumeratin'!
	}

MySceneEntry *MySceneEnumProc::Append(INode *node, Object *obj, ShapeObject *so) {
	MySceneEntry *entry = new MySceneEntry(node, obj, so);
	if(tail)
		tail->next = entry;
	tail = entry;
	if(!head)
		head = entry;
	count++;
	return entry;
	}

Box3 MySceneEnumProc::Bound() {
	Box3 bound;
	bound.Init();
	MySceneEntry *e = head;
	ViewExp *vpt = i->GetViewport(NULL);
	while(e) {
		bound += e->shape.GetBoundingBox();
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

#define AIWRITE(s) if(fprintf(stream,"%s",s)<0) goto wrterr;

int
ai_save(const TCHAR *filename, ExpInterface *ei, Interface *gi, AIExport *exp) {
	int ox;
	TimeValue t = gi->GetTime();

	// Get a scale factor from points (the file storage) to our units
	double mScale = GetMasterScale(UNITS_INCHES);
	float scaleFactor = float(mScale * 72.0);

	if(showPrompts) {
#ifdef MAYBE
		// Put up the options dialog to find out how they want the file written!
		int result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EXPORTOPTIONS), 
			gi->GetMAXHWnd(), ExportOptionsDlgProc, (LPARAM)exp);
		if(result <= 0)
			return 0;
#endif //MAYBE
		}
	else {	// Set default parameters here
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
	bbox.Scale(scaleFactor);

	// Let's get busy!
	
	// First, write the prolog stuff...
//adobe_minmax(all,&minx,&miny,&maxx,&maxy);
	char gp_buffer[256];
	AIWRITE("%!PS-Adobe-3.0\n");
	AIWRITE("%%Creator: 3D Studio MAX(TM) .AI Export Version 1.0\n");
	AIWRITE("%%Title: (Shape Export)\n");
	_snprintf(gp_buffer,256,"%%%%BoundingBox:%d %d %d %d\n",(int)bbox.pmin.x,(int)bbox.pmin.y,(int)bbox.pmax.x,(int)bbox.pmax.y);
	AIWRITE(gp_buffer);
	AIWRITE("%%DocumentNeededResources: procset Adobe_packedarray 2.0 0\n");
	AIWRITE("%%+ procset Adobe_cshow 1.1 0\n");
	AIWRITE("%%+ procset Adobe_customcolor 1.0 0\n");
	AIWRITE("%%+ procset Adobe_Illustrator_AI3 1.0 1\n");
	AIWRITE("%%EndComments\n");
	AIWRITE("%%BeginProlog\n");
	AIWRITE("%%IncludeResource: procset Adobe_packedarray 2.0 0\n");
	AIWRITE("Adobe_packedarray /initialize get exec\n");
	AIWRITE("%%IncludeResource: procset Adobe_cshow 1.1 0\n");
	AIWRITE("%%IncludeResource: procset Adobe_customcolor 1.0 0\n");
	AIWRITE("%%IncludeResource: procset Adobe_Illustrator_AI3 1.0 1\n");
	AIWRITE("%%EndProlog\n");
	AIWRITE("%%BeginSetup\n");
	AIWRITE("Adobe_cshow /initialize get exec\n");
	AIWRITE("Adobe_customcolor /initialize get exec\n");
	AIWRITE("Adobe_IllustratorA_AI3 /initialize get exec\n");
	AIWRITE("%%EndSetup\n");
	AIWRITE("0.000 0.000 0.000 1.000 K\n");

	/* That takes care of the prolog garbage, now write our shapes */

	for(ox = 0; ox < myScene.Count(); ++ox) {
		BezierShape &shape = myScene[ox]->shape;
		// Write each spline
		int polys = shape.SplineCount();
		for(int poly = 0; poly < polys; ++poly) {
			Spline3D *spline = shape.GetSpline(poly);
			int knots = spline->KnotCount();
			for(int ix = 0; ix < knots; ++ix) {
				if(ix == 0) {
					Point3 p = spline->GetKnotPoint(ix) * scaleFactor;
					_snprintf(gp_buffer,256,"%.3f %.3f m\n",p.x,p.y);
					FixDecSymbol(gp_buffer);
					AIWRITE(gp_buffer);
					}
				else {
					Point3 lp = spline->GetKnotPoint(ix-1) * scaleFactor;
					Point3 lout = spline->GetOutVec(ix-1) * scaleFactor;
					Point3 p = spline->GetKnotPoint(ix) * scaleFactor;
					Point3 in = spline->GetInVec(ix) * scaleFactor;
					if(lout.x == lp.x && lout.y == lp.y && in.x == p.x && in.y == p.y) {
						_snprintf(gp_buffer,256,"%.3f %.3f L\n",p.x,p.y);
						FixDecSymbol(gp_buffer);
						AIWRITE(gp_buffer);
						}
					else {
						_snprintf(gp_buffer,256,"%.3f %.3f %.3f %.3f %.3f %.3f C\n",
							lout.x,lout.y, in.x, in.y, p.x, p.y);
						FixDecSymbol(gp_buffer);
						AIWRITE(gp_buffer);
						}
					}
				}
			if(spline->Closed()) {
				Point3 p0 = spline->GetKnotPoint(0) * scaleFactor;
				Point3 in0 = spline->GetInVec(0) * scaleFactor;
				Point3 pn = spline->GetKnotPoint(knots-1) * scaleFactor;
				Point3 outn = spline->GetOutVec(knots-1) * scaleFactor;
				if(in0.x == p0.x && in0.y == p0.y && outn.x == pn.x && outn.y == pn.y) {
					_snprintf(gp_buffer,256,"%.3f %.3f L\n",p0.x,p0.y);
					FixDecSymbol(gp_buffer);
					AIWRITE(gp_buffer);
					}
				else {
					_snprintf(gp_buffer,256,"%.3f %.3f %.3f %.3f %.3f %.3f C\n",
						outn.x, outn.y, in0.x, in0.y, p0.x, p0.y);
					FixDecSymbol(gp_buffer);
					AIWRITE(gp_buffer);
					}
				AIWRITE("s\n");
				}
			else
				AIWRITE("S\n");
			}
		}

	AIWRITE("%%Trailer\n");
	AIWRITE("Adobe_Illustrator_AI3 /terminate get exec\n");
	AIWRITE("Adobe_customcolor /terminate get exec\n");
	AIWRITE("Adobe_cshow /terminate get exec\n");
	AIWRITE("Adobe_packedarray /terminate get exec\n");
	AIWRITE("%%EOF\n");
	if(theFile.Close()) {
		wrterr:
		if(showPrompts)
			Alert(IDS_TH_WRITEERROR);
		theFile.Close();
		remove(filename);
		return 0;
		}

	if(theFile.Close())
		goto wrterr;

	return 1;	
	}

int
AIExport::DoExport(const TCHAR *filename,ExpInterface *ei,Interface *gi, BOOL suppressPrompts, DWORD options) {
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	int status;
		
	status=ai_save(filename, ei, gi, this);

	if(status == 0)
		return 1;		// Dialog cancelled
	if(status > 0)	{
#ifdef DBGDXF
DebugPrint("AI status OK!\n");
#endif
		}
#ifdef DBGDXF
	else
	if(status < 0)
		DebugPrint("Error somewhere in AI!\n");
#endif

	return(status);
	}

BOOL AIExport::SupportsOptions(int ext, DWORD options) {
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
	}

