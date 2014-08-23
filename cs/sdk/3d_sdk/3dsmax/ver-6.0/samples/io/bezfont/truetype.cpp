/**********************************************************************
 *<
	FILE: truetype.cpp

	DESCRIPTION:  truetype bezier font file import module

	CREATED BY: Tom Hudson

	HISTORY: created 2 November 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "Max.h"
#include <stdio.h>
#include <direct.h>
#include <commdlg.h>
#include "splshape.h"
#include "bezfont.h"
#include "resource.h"

#define TRUETYPE_LOADER_CLASS_ID 0x1800

HINSTANCE hInstance;

TCHAR *GetString(int id);

class TrueTypeImport : public BezFont {
	private:
		HFONT hFont;
		BezFontMgrEnumProc *eProc;
		LPARAM eInfo;		
	public:
		// required methods from BezFont class
		void EnumerateFonts(BezFontMgrEnumProc &proc, LPARAM userInfo);
		int OpenFont(TSTR name, DWORD flags, DllData *dllData);
		void CloseFont();
		BOOL BuildCharacter(UINT index, float height, BezierShape &shape, float &width, int FontShapeVersion);
		// Our methods
		BOOL DoEnumeration(BezFontInfo &info);
		TrueTypeImport() { hFont = NULL; }
		BOOL CreateCharacterShape(LPTTPOLYGONHEADER lpHeader, int size, BezierShape &shape, int FontShapeVersion);
	};

static BOOL FAR PASCAL EnumCallBack2(ENUMLOGFONT *lplf, NEWTEXTMETRIC *lpntm, DWORD fontType, LPVOID iptr) {
	// If no style specified, toss it!
	if(lplf->elfStyle[0] == 0)
		return TRUE;	// Forget this one but keep enumerating
	TrueTypeImport *imp = (TrueTypeImport *)iptr;
	BezFontInfo info;
	info.name = (char*)lplf->elfFullName;
	info.style = (char*)lplf->elfStyle;
	info.type = BEZFONT_TRUETYPE;
	info.flags = 0;		// No flags defined yet -- Make this zero!
	info.metrics = BezFontMetrics(lpntm);
	return imp->DoEnumeration(info);		
	}

static BOOL FAR PASCAL EnumCallBack(ENUMLOGFONT *lplf, NEWTEXTMETRIC *lpntm, DWORD fontType, LPVOID iptr) {
	// All we're concerned with are TrueType(T.M.Reg.U.S.Pat.Off.) fonts...
	if(fontType & TRUETYPE_FONTTYPE) {
		// Got the basic font name, now let's enumerate its members
		TCHAR work[256];
		_tcscpy(work,lplf->elfLogFont.lfFaceName);
		HDC hdcScreen = GetDC(NULL);
		EnumFontFamilies(hdcScreen, (LPCTSTR)work, (FONTENUMPROC)EnumCallBack2, (LPARAM)iptr);
		}
	return TRUE;
	}

void TrueTypeImport::EnumerateFonts(BezFontMgrEnumProc &proc, LPARAM userInfo) {
	HDC hdcScreen = GetDC(NULL);
	eProc = &proc;
	eInfo = userInfo;
	EnumFontFamilies(hdcScreen, (LPCTSTR)NULL, (FONTENUMPROC)EnumCallBack, (LPARAM)this);
	}

int TrueTypeImport::OpenFont(TSTR name, DWORD flags, DllData *dllData) {
    // build a TrueType font.
    static LOGFONT lf;
//    GetObject(GetStockObject(SYSTEM_FONT), sizeof(lf), &lf);
    lf.lfHeight = 1000;
    lf.lfWidth = 0;
	lf.lfEscapement = 0; 
	lf.lfOrientation = 0; 
	lf.lfWeight = 0; 
	lf.lfItalic = FALSE; 
	lf.lfUnderline = FALSE; 
	lf.lfStrikeOut = FALSE; 
	lf.lfCharSet = DEFAULT_CHARSET; 
//	lf.lfCharSet = SHIFTJIS_CHARSET; 
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS; 
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
	lf.lfQuality = DEFAULT_QUALITY; 
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE; 
    _tcsncpy((TCHAR *)&lf.lfFaceName, name.data(), LF_FACESIZE);
    hFont = CreateFontIndirect(&lf);
	return(hFont ? 1 : 0);
	}

void TrueTypeImport::CloseFont() {
	if(hFont) {
		DeleteObject(hFont);
		hFont = NULL;
		}
	}

class FontReady {
	public:
		HDC hdc;
		LPOUTLINETEXTMETRIC otm;
		HGDIOBJ hOld;
		FontReady(HFONT font);
		~FontReady();
	};

FontReady::FontReady(HFONT font) {
	hdc = GetDC(NULL);
	hOld = SelectObject(hdc, (HGDIOBJ)font);
	otm = NULL;
	DWORD otmSize = GetOutlineTextMetrics(hdc, 0, NULL);
	if(otmSize) {
		otm = (LPOUTLINETEXTMETRIC)malloc(otmSize);
		if(otm)
			GetOutlineTextMetrics(hdc, otmSize, otm);
		}
 	}

FontReady::~FontReady() {
	SelectObject(hdc, (HGDIOBJ)hOld); ReleaseDC(NULL, hdc);
 	if(otm) {
		free(otm);
		otm = NULL;
		}
 	}

class GenericAlloc {
	public:
		void *ptr;
		GenericAlloc(int size) { ptr = malloc(size); }
		~GenericAlloc() { if(ptr) { free(ptr); ptr = NULL; } }
	};

//  BEZIER  --	Manage additions to our bezier curve for the font character

static Spline3D *curSpline = NULL;

static void
bezier(BezierShape &shape,float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
{
//DebugPrint("Bezier creating %.2f %.2f-%.2f %.2f-%.2f %.2f-%.2f %.2f\n",x0,y0,x1,y1,x2,y2,x3,y3);
if(!curSpline)
	curSpline = shape.NewSpline();
int knots = curSpline->KnotCount();
if(knots == 0) {
	curSpline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, Point3(x0,y0,0.0f), Point3(x0,y0,0.0f), Point3(x1,y1,0.0f)));
	curSpline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, Point3(x3,y3,0.0f), Point3(x2,y2,0.0f), Point3(x3,y3,0.0f)));
	}
else {
	// First point of this curve must be the same as the last point on the output curve
	assert(curSpline->GetKnotPoint(knots-1) == Point3(x0,y0,0.0f));
	curSpline->SetOutVec(knots-1, Point3(x1,y1,0.0f));
	if(Point3(x3,y3,0.0f) == curSpline->GetKnotPoint(0)) {
		curSpline->SetInVec(0, Point3(x2,y2,0.0f));
		curSpline->SetClosed();
//DebugPrint("Bezier autoclosed\n");
		curSpline = NULL;
		}
	else
		curSpline->AddKnot(SplineKnot(KTYPE_BEZIER, LTYPE_CURVE, Point3(x3,y3,0.0f), Point3(x2,y2,0.0f), Point3(x3,y3,0.0f)));
	}
}

/****************************************************************************
 *  FUNCTION   : IntFromFixed
 *  RETURNS    : int value approximating the FIXED value.
 ****************************************************************************/
static float FloatFromFixed(FIXED f) {
    return f.value + (float)f.fract / 65536.0f;
	}

/****************************************************************************
 *  FUNCTION   : FixedFromDouble
 *  RETURNS    : FIXED value representing the given double.
 ****************************************************************************/
static FIXED FixedFromDouble(double d)
{
    long l;

    l = (long) (d * 65536L);
    return *(FIXED *)&l;
}

BOOL TrueTypeImport::CreateCharacterShape(LPTTPOLYGONHEADER lpHeader, int size, BezierShape &shape, int fontShapeVersion) {
    LPTTPOLYGONHEADER lpStart;
    LPTTPOLYCURVE lpCurve;
    WORD i;
	POINTFX work;
//DebugPrint("Start of letter\n");
    lpStart = lpHeader;
    while ((DWORD_PTR)lpHeader < (DWORD_PTR)(((LPSTR)lpStart) + size)) {
		if (lpHeader->dwType == TT_POLYGON_TYPE) {

		    // Get to first curve.
	    	lpCurve = (LPTTPOLYCURVE) (lpHeader + 1);
//		    iFirstCurve = cTotal;
//DebugPrint("Poly start\n");

		    while ((DWORD_PTR)lpCurve < (DWORD_PTR)(((LPSTR)lpHeader) + lpHeader->cb)) {
				if (lpCurve->wType == TT_PRIM_LINE)	{
				    work = *(LPPOINTFX)((LPSTR)lpCurve - sizeof(POINTFX));
					Point3 p0(FloatFromFixed(work.x), FloatFromFixed(work.y), 0.0f);
				    for (i = 0; i < lpCurve->cpfx; i++) {
						work = lpCurve->apfx[i];
						Point3 p1(FloatFromFixed(work.x), FloatFromFixed(work.y), 0.0f);
						bezier(shape, p0.x, p0.y, p0.x, p0.y, p1.x, p1.y, p1.x, p1.y);
						p0 = p1;
				    	}
					}
				else
				if (lpCurve->wType == TT_PRIM_QSPLINE)	{
				    //**********************************************
				    // Format assumption:
				    //   The bytes immediately preceding a POLYCURVE
				    //   structure contain a valid POINTFX.
				    //
				    //   If this is first curve, this points to the 
				    //      pfxStart of the POLYGONHEADER.
				    //   Otherwise, this points to the last point of
				    //      the previous POLYCURVE.
				    //
				    //	 In either case, this is representative of the
				    //      previous curve's last point.
				    //**********************************************
				    work = *(LPPOINTFX)((LPSTR)lpCurve - sizeof(POINTFX));
					Point3 p0(FloatFromFixed(work.x), FloatFromFixed(work.y), 0.0f);
				    Point3 p1, p2, p3, pa, pb;
				    for (i = 0; i < lpCurve->cpfx;) {
						// This point is off the curve -- Hold onto it
						work = lpCurve->apfx[i++];
						pa = Point3(FloatFromFixed(work.x), FloatFromFixed(work.y), 0.0f);

						// Calculate the C point.
						if (i == (lpCurve->cpfx - 1))  {
							// It's the last point, and therefore on the curve.
							// We need to compute the bezier handles between it and p0...
							work = lpCurve->apfx[i++];
							p3 = Point3(FloatFromFixed(work.x), FloatFromFixed(work.y), 0.0f);
							}     
						else  {
							// It's not the last point -- Need to compute midpoint to get
							// a point on the curve
 							work = lpCurve->apfx[i];	// Don't inc i -- We'll use it next time around!
							Point3 pb(FloatFromFixed(work.x), FloatFromFixed(work.y), 0.0f);
							p3 = (pa + pb) / 2.0f;
							}

						// Also compute the appropriate handles...
						if(fontShapeVersion == 1) {		// Release 1.x-compatible
							p1 = (p0 + pa) / 2.0f;
							p2 = (pa + p3) / 2.0f;
							}
						else{
							p1 = (p0 + 2.0f * pa) / 3.0f;	// Release 2
							p2 = (p3 + 2.0f * pa) / 3.0f;
							}

						// Let's add it to the output bezier!
						bezier(shape, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);

						// New A point for next slice of spline.
						p0 = p3;
						}
					}
				else
					; // error, error, error

				// Move on to next curve.
				lpCurve = (LPTTPOLYCURVE)&(lpCurve->apfx[i]);
		    	}
//DebugPrint("Poly end\n");

			if(curSpline) {
				curSpline->SetClosed();
				curSpline = NULL;
				}

		    // Move on to next polygon.
	    	lpHeader = (LPTTPOLYGONHEADER)(((LPSTR)lpHeader) + lpHeader->cb);
			}
		else
			; // error, error, error
		}
//DebugPrint("End of letter\n");

	// Make sure any splines are closed and reset
	if(curSpline) {
		curSpline->SetClosed();
		curSpline = NULL;
		}

	// Update the selection set info -- Just to be safe
	shape.UpdateSels();
	
	return TRUE;
	}

/****************************************************************************
 *  FUNCTION   : IdentityMat
 *  PURPOSE    : Fill in matrix to be the identity matrix.
 *  RETURNS    : none.
 ****************************************************************************/
static void IdentityMat(LPMAT2 lpMat)
{
    lpMat->eM11 = FixedFromDouble(1.0);
    lpMat->eM12 = FixedFromDouble(0.0);
    lpMat->eM21 = FixedFromDouble(0.0);
    lpMat->eM22 = FixedFromDouble(1.0);
}

BOOL TrueTypeImport::BuildCharacter(UINT index, float height, BezierShape &shape, float &width, int fontShapeVersion) {
    assert(hFont);
	if(!hFont)
		return 0;

	// Set up for the font and release it when this function returns
	FontReady fontRdy(hFont);
			    
	// allocate space for the bitmap/outline
	GLYPHMETRICS gm;
    // init it to prevent UMR in GetGlyphOutline
    gm.gmBlackBoxX = 
    gm.gmBlackBoxY = 
    gm.gmptGlyphOrigin.x =
    gm.gmptGlyphOrigin.y =
    gm.gmCellIncX = 
    gm.gmCellIncY = 0; 

	// Give it an identity matrix
	MAT2 mat;
	IdentityMat(&mat);

	DWORD size = GetGlyphOutline(fontRdy.hdc, index, GGO_NATIVE, &gm, 0, NULL, &mat);
	if(size != GDI_ERROR && size > 0) {
		GenericAlloc mem(size);
		if(!mem.ptr)
			goto failure;
		if ((GetGlyphOutline(fontRdy.hdc, index, GGO_NATIVE, &gm, size, mem.ptr, &mat)) != size)
			goto failure;
		curSpline = NULL;	// reset the current spline pointer
		if(!CreateCharacterShape((TTPOLYGONHEADER *)mem.ptr, size, shape, fontShapeVersion))
			goto failure;
		// Make sure the height matches the request
		float scaleFactor = height / 1000.0f;
		Matrix3 tm = ScaleMatrix(Point3(scaleFactor, scaleFactor, 0.0f));
		shape.Transform(tm);
		width = float(gm.gmCellIncX) * scaleFactor;
		return TRUE;
		}

	// Character wasn't found!
	failure:
	width = 0.0f;
	return FALSE;
	}

BOOL TrueTypeImport::DoEnumeration(BezFontInfo &info) {
	return eProc->Entry(info, eInfo, NULL);
	} 

// Jaguar interface code

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,L"TRUETYPE.DLL: DllMain",L"TRUETYPE",MB_OK);
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

class TTClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TrueTypeImport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_CLASSNAME); }
	SClass_ID		SuperClassID() { return BEZFONT_LOADER_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(TRUETYPE_LOADER_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_CATEGORY);  }
	};

static TTClassDesc TTDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_LIB_DESCRIPTION); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &TTDesc; break;
		default: return 0; break;
		}

	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
