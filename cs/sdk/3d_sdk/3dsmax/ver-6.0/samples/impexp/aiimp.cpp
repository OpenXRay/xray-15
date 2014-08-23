/**********************************************************************
 *<
	FILE: aiimp.cpp

	DESCRIPTION:  .AI file import module

	CREATED BY: Tom Hudson

	HISTORY: created 28 June 1996

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include <stdio.h>
#include <direct.h>
#include <commdlg.h>
#include "splshape.h"
#include "aires.h"
#include "imtl.h"
#include "aiimp.h"
#include "istdplug.h"
#include "stdmat.h"

HINSTANCE hInstance;

static BOOL showPrompts;

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}


static int MessageBox(int s1, int s2, int option = MB_OK) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}

static int MessageBox2(int s1, int s2, int option = MB_OK) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}

static int MessageBox(TCHAR *s, int s2, int option = MB_OK) {
	TSTR str1(s);
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}


//#define DBGPRINT


// The file stream

static FILE *stream;

// The debugging dump stream

static FILE *dStream;

// Some stuff we're defining (for now)

static BOOL replaceScene =0;

void
split_fn(char *path,char *file,char *pf)
	{
	int ix,jx,bs_loc,fn_loc;
	if(strlen(pf)==0) {
		if(path) *path=0;
		if(file) *file=0;
		return;
		}
	bs_loc=strlen(pf);
	for(ix=bs_loc-1; ix>=0; --ix) {
		if(pf[ix]=='\\')  {
			bs_loc=ix;
			fn_loc=ix+1;
			goto do_split;
			}
		if(pf[ix]==':') {
			bs_loc=ix+1;
			fn_loc=ix+1;
			goto do_split;
			}
		}
	bs_loc= -1;
	fn_loc=0;

	do_split:
	if(file)
		strcpy(file,&pf[fn_loc]);
	if(path) {
		if(bs_loc>0)  {
			for(jx=0; jx<bs_loc; ++jx)
				path[jx]=pf[jx];
			path[jx]=0;
			}
		else  path[0]=0;
		}
	}


//===========================================================


// Handy pointers to importers
AIShapeImport *theShapeImport = NULL;

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
			//MessageBox(NULL,_T("AIIMP.DLL: DllMain"),_T("AIIMP"),MB_OK);
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

class AIShapeClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new AIShapeImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_AISHAPE); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(2,2); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SCENEIMPORT); }
	};

static AIShapeClassDesc AIShapeDesc;

// Statics

int	AIShapeImport::importType = SINGLE_SHAPE;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_AIIMPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &AIShapeDesc; break;
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

AIShapeImport::AIShapeImport() {
	splShape = NULL;
	shape = NULL;
	shapeNumber = 1;		// For name index
	}

AIShapeImport::~AIShapeImport() {
	}

int
AIShapeImport::ExtCount() {
	return 1;
	}

const TCHAR *
AIShapeImport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("AI");
		}
	return _T("");
	}

const TCHAR *
AIShapeImport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_AISHAPEFILE);
	}
	
const TCHAR *
AIShapeImport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_ADOBE_ILLUSTRATOR);
	}

const TCHAR *
AIShapeImport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_TOM_HUDSON);
	}

const TCHAR *
AIShapeImport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_YOST_GROUP);
	}

const TCHAR *
AIShapeImport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
AIShapeImport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
AIShapeImport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
AIShapeImport::ShowAbout(HWND hWnd) {			// Optional
 	}

/* Get a null-terminated string from the file */

int
read_string(char *string,FILE *stream,int maxsize)
{
while(maxsize--)
 {
 RDERR(string,1);
 if(*(string++)==0)
  return(1);
 }
return(0);	/* Too long */
}

static int shapeButtons[] = { IDC_SINGLEOBJECT, IDC_MULTIPLEOBJECTS };
#define NUM_SHAPEBUTTONS 2

static INT_PTR CALLBACK
ShapeImportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static AIShapeImport *imp;

	switch(message) {
		case WM_INITDIALOG:
			imp = (AIShapeImport *)lParam;
			CheckDlgButton( hDlg, shapeButtons[imp->importType], TRUE);
			CenterWindow(hDlg,GetParent(hDlg));
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK: {
					// Unload values into AIShapeImport statics
					for(int i = 0; i < NUM_SHAPEBUTTONS; ++i) {
						if(IsDlgButtonChecked(hDlg, shapeButtons[i])) {
							imp->importType = i;
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

/* Get a line terminated with a CR/LF			*/
/* Returns 0 if EOF, 1 if OK, -1 if string too long	*/
/* Null-terminates the string at the CR			*/
/* Optionally squashes spaces out of non-quoted areas	*/

/*static*/ int
getaline(FILE *stream,char *string,int max,int squash)
{
char ch;
int count,gotquote;

count=gotquote=0;
while(1) {
	RDERR(&ch,1);
	if(ch==10) {
		*string = 0;
		return 1;
		}
	else {
		if(ch!=' ' || squash==0 || gotquote) {
			if(ch==34)	/* Quote */
				gotquote=1-gotquote;
		
			if((++count)>max) {
				*(string-1)=0;
				return(-1);
				}
			if(ch==13) {
				*string=0;
				return(1);
				}
			*string++=ch;
			}
		}
	}
}

BOOL AIShapeImport::StartWorkingShape() {
	spline = NULL;
	if(!splShape) {
		splShape = new SplineShape;
		if(!splShape)
			return FALSE;
		shape = &splShape->GetShape();
		shape->NewShape();	// Clean out the shape
		}
	if(!shape) {
		assert(0);
		return FALSE;
		}
	spline = shape->NewSpline();
	if(!spline)
		return FALSE;
	return TRUE;
	}

BOOL AIShapeImport::FinishWorkingShape(BOOL forceFinish, ImpInterface *i) {
	// If nothing going on, forget the whole thing and return success
	if(!splShape)
		return TRUE;
	if(!shape) {
		assert(0);
		return FALSE;
		}
	// Otherwise, see if the current spline is closed
	if(spline) {
		int knots = spline->KnotCount();
		// If not a valid spline, get rid of it!
		if(knots < 2) {
			singularity:
//			DebugPrint("WARN: spline w/%d points\n",knots);
			shape->UpdateSels();	// Make sure it readies the selection set info
			shape->DeleteSpline(shape->SplineCount() - 1);
			spline = NULL;
			}
		else {
			int lastKnot = knots-1;
			if(spline->GetKnotPoint(0) == spline->GetKnotPoint(lastKnot)) {
				spline->SetInVec(0, spline->GetInVec(lastKnot));
				spline->DeleteKnot(lastKnot);
				if(spline->KnotCount() < 2)
					goto singularity;
				spline->SetClosed();
				}
			// Check to be sure knot types are correct -- The default is for
			// smooth beziers, but they may be corners
			for(int k = 0; k < knots; ++k) {
				Point3 in = spline->GetInVec(k);
				Point3 out = spline->GetOutVec(k);
				Point3 knot = spline->GetKnotPoint(k);
				int type = spline->GetKnotType(k);
				if(type == KTYPE_BEZIER) {
					if(in == knot && out == knot)	// If both zero length vector, it's a corner
						spline->SetKnotType(k, KTYPE_CORNER);
					else	// If vectors not collinear, it's a corner!
					if(in == knot || out == knot)	// If zero length vector, it's a bez corner
						spline->SetKnotType(k, KTYPE_BEZIER_CORNER);
					else {	// If vectors not collinear, it's a corner!
						Point3 normIn = Normalize(knot - in);
						Point3 normOut = Normalize(out - knot);
						Point3 lowtest = normIn * 0.98f;
						Point3 hightest = normIn * 1.02f;
						if(!(((normOut.x >= lowtest.x && normOut.x <= hightest.x) || (normOut.x <= lowtest.x && normOut.x >= hightest.x)) && 
						   ((normOut.y >= lowtest.y && normOut.y <= hightest.y) || (normOut.y <= lowtest.y && normOut.y >= hightest.y)) && 
						   ((normOut.z >= lowtest.z && normOut.z <= hightest.z) || (normOut.z <= lowtest.z && normOut.z >= hightest.z))))
							spline->SetKnotType(k, KTYPE_BEZIER_CORNER);
						}
					}
				}
			spline->ComputeBezPoints();
			}
		}
	// If necessary, create the object
	if(importType == MULTIPLE_SHAPES || forceFinish) {
		if(shape->SplineCount()) {
			shape->UpdateSels();	// Make sure it readies the selection set info
			shape->InvalidateGeomCache();
			// create shape object
			ImpNode *node = i->CreateNode();
			if(!node)
				return FALSE;
			node->Reference(splShape);
			Matrix3 tm(1);
			Box3 bbox = shape->GetBoundingBox();
			Point3 center = Point3(bbox.Center().x, bbox.Center().y, 0.0f);
			tm.SetTrans(center);	// TH 7/5/96
			node->SetPivot(-center);	// TH 3/9/99
			node->SetTransform(0,tm);
			i->AddNodeToScene(node);
			TSTR name;
			name.printf(GetString(IDS_TH_SHAPE_NUM),theShapeImport->shapeNumber++);
			node->SetName(name);
			gotStuff = TRUE;
			}
		// Reset the shape stuff
		splShape = NULL;
		shape = NULL;
		spline = NULL;
		}
	return TRUE;
	}

static TCHAR decimalSymbol = _T('.');

static void GetDecSymbolFromRegistry() 
{
    char symbol[80];
	int bufSize = 80;
	if (GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL, symbol, bufSize) != 0 )
		decimalSymbol = symbol[0];
}

TCHAR *FixDecimalSymbol(TCHAR *buf)
	{
	TCHAR *cp = buf;
	while(*cp) {
		if( *cp == _T('.'))
			*cp = decimalSymbol;
		cp++;
		}
	return buf;
	}

static INT_PTR CALLBACK
ImportDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
		case WM_INITDIALOG:
			CheckRadioButton( hDlg, IDC_AI_MERGE, IDC_AI_REPLACE, replaceScene?IDC_AI_REPLACE:IDC_AI_MERGE );
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
AIShapeImport::DoImport(const TCHAR *filename,ImpInterface *i,Interface *gi, BOOL suppressPrompts) {
	// Get a scale factor from points (the file storage) to our units
	double mScale = GetMasterScale(UNITS_INCHES);
	float scaleFactor = float((1.0 / mScale) / 72.0);
	
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;

	WorkFile theFile(filename,_T("rb"));

	if(suppressPrompts) {
		}
	else {
		if (!DialogBox(hInstance, MAKEINTRESOURCE(IDD_MERGEORREPL), gi->GetMAXHWnd(), ImportDlgProc))
			return IMPEXP_CANCEL;
		}

	dStream = i->DumpFile();

	if(!(stream = theFile.Stream())) {
		if(showPrompts)
			MessageBox(IDS_TH_ERR_OPENING_FILE, IDS_TH_3DSIMP);
		return 0;						// Didn't open!
		}

	// Got the file -- Now put up the options dialog!
	if(showPrompts) {
		int result = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SHAPEIMPORTOPTIONS), gi->GetMAXHWnd(), ShapeImportOptionsDlgProc, (LPARAM)this);
		if(result <= 0)
			return IMPEXP_CANCEL;
		}
	else { // Set default parameters here
		importType = MULTIPLE_SHAPES;
		}

	if (replaceScene) {
		if (!i->NewScene())
			return IMPEXP_CANCEL;
		}

	theShapeImport = this;

	int line,count,status,phase,buflen, reason;
	float x1,y1,x2,y2,x3,y3;
	char buffer[256];

	phase=0;
	count=0;
	line=0;
	gotStuff = FALSE;

	GetDecSymbolFromRegistry();

	loop:
	line++;
	if((status=getaline(stream,buffer,255,0))<0)
		{
		reason = IDS_TH_LINE_TOO_LONG;

		corrupted:
		if(showPrompts)
			MessageBox(reason, IDS_TH_3DSIMP);
		FinishWorkingShape(TRUE, i);
		return gotStuff;
		}

	if(status==0) {
		FinishWorkingShape(TRUE, i);
		return gotStuff;	// EOF
		}

	/* Look for appropriate section of file */

	buflen=strlen(buffer);
	switch(phase)
		{
		case 0:	/* Looking for the path */
			buffer[10]=0;
			if(stricmp(buffer,"%%endsetup")==0)
				phase=1;
			break;
		case 1:	/* Loading the path data -- looking for initial 'm' */
			if(buflen<2)
				break;
			if(buffer[buflen-2]==' ' && buffer[buflen-1]=='m') {
				phase=2;
				goto phase2;
				}
			break;
		case 2:
			phase2:
			if(buflen<2)
				break;
			if(buffer[buflen-2]!=' ')
				break;
			switch(buffer[buflen-1]) {
				case 'm': {	/* Moveto */
					FixDecimalSymbol(buffer);
					if(sscanf(buffer,"%f %f",&x1,&y1)!=2) {
	#ifdef DBG1
	DebugPrint("Moveto buffer:%s\n",buffer);
	#endif
						bad_file:
						reason = IDS_TH_INVALIDFILE;
						goto corrupted;
						}
					// If had one working, wrap it up
					FinishWorkingShape(FALSE, i);
					// Start this new spline
					if(!StartWorkingShape()) {
						reason = IDS_TH_NO_RAM;
						goto corrupted;
						}
					Point3 p(x1 * scaleFactor, y1 * scaleFactor, 0.0f);
					SplineKnot k(KTYPE_BEZIER, LTYPE_CURVE, p, p, p);
					spline->AddKnot(k);
					}
					break;
				case 'l':	/* Lineto */
				case 'L': {	/* Lineto corner */
					FixDecimalSymbol(buffer);
					if(sscanf(buffer,"%f %f",&x1,&y1)!=2) {
	#ifdef DBG1
	DebugPrint("Lineto buffer:%s\n",buffer);
	#endif
						goto bad_file;
						}
					Point3 p(x1 * scaleFactor, y1 * scaleFactor, 0.0f);
					SplineKnot k(KTYPE_BEZIER, LTYPE_CURVE, p, p, p);
					spline->AddKnot(k);
					}
					break;
				case 'c':	/* Curveto */
				case 'C': {	/* Curveto corner */
					FixDecimalSymbol(buffer);
					if(sscanf(buffer,"%f %f %f %f %f %f",&x1,&y1,&x2,&y2,&x3,&y3)!=6) {
	#ifdef DBG1
	DebugPrint("Curveto buffer:%s\n",buffer);
	#endif
						goto bad_file;
						}
					int lastKnot = spline->KnotCount() - 1;
					spline->SetOutVec(lastKnot, Point3(x1 * scaleFactor, y1 * scaleFactor, 0.0f));
					Point3 p(x3 * scaleFactor, y3 * scaleFactor, 0.0f);
					Point3 in(x2 * scaleFactor, y2 * scaleFactor, 0.0f);
					SplineKnot k(KTYPE_BEZIER, LTYPE_CURVE, p, in, p);
					spline->AddKnot(k);
					}
					break;
				case 'v':	/* Current/vec */
				case 'V': {	/* Current/vec corner */
					FixDecimalSymbol(buffer);
					if(sscanf(buffer,"%f %f %f %f",&x2,&y2,&x3,&y3)!=4) {
	#ifdef DBG1
	DebugPrint("Current/vec buffer:%s\n",buffer);
	#endif
						goto bad_file;
						}
					Point3 p(x3 * scaleFactor, y3 * scaleFactor, 0.0f);
					Point3 in(x2 * scaleFactor, y2 * scaleFactor, 0.0f);
					SplineKnot k(KTYPE_BEZIER, LTYPE_CURVE, p, in, p);
					spline->AddKnot(k);
					}
					break;
				case 'y':	/* Vec/next */
				case 'Y': {	/* Vec/next corner */
					FixDecimalSymbol(buffer);
					if(sscanf(buffer,"%f %f %f %f",&x1,&y1,&x3,&y3)!=4) {
	#ifdef DBG1
	DebugPrint("vec/next buffer:%s\n",buffer);
	#endif
						goto bad_file;
						}
					int lastKnot = spline->KnotCount() - 1;
					spline->SetOutVec(lastKnot, Point3(x1 * scaleFactor, y1 * scaleFactor, 0.0f));
					Point3 p(x3 * scaleFactor, y3 * scaleFactor, 0.0f);
					SplineKnot k(KTYPE_BEZIER, LTYPE_CURVE, p, p, p);
					spline->AddKnot(k);
					}
					break;
				}
			break;
		}

	count++;
	goto loop;
	}

