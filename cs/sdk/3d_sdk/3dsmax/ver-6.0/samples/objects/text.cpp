/**********************************************************************
 *<
	FILE: text.cpp

	DESCRIPTION:  A text spline object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 1 November 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "prim.h" 

#ifndef NO_OBJECT_SHAPES_SPLINES

#include "splshape.h"
#include "iparamm.h"
#include "bezfont.h"
// This is based on the simple spline object...
#include "simpspl.h"
#include "istdplug.h"
#include "buildver.h"

#ifndef WEBVERSION // orb 03-18-02 ECO#777
#define INIT_TEXT_SIZE 100.0f
#else
#define INIT_TEXT_SIZE 12.0f
#endif // WEBVERSION

// Update this on every major MAX release
#define TEXT_VERSION_NUMBER 2

#define MIN_SIZE		float(0)
#define MAX_SIZE		float( 1.0E30)
#define MIN_KERNING		float(-1.0E30)
#define MAX_KERNING		float( 1.0E30)
#define MIN_LEADING		float(-1.0E30)
#define MAX_LEADING		float( 1.0E30)

#define DEF_SIZE		float(0.0)

#define swab(v) ((((v)&0xFF) << 8) + (((v) >> 8) &0xFF))

class TextObjCreateCallBack;

class CharShape {
	public:
		int index;
		BOOL keep;
		float width;
		BezierShape shape;
		CharShape() { index = -1; keep = FALSE; width = 0.0f; }
		CharShape(int i);
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};

CharShape::CharShape(int i) {
	index=i;
	keep = TRUE;
	width = 0.0f;
	}

#define CS_INDEX_CHUNK		0x1000
#define CS_WIDTH_CHUNK		0x1010
#define CS_SHAPE_CHUNK		0x1020
#define CS_FONT_TRUETYPE	0x1030

IOResult CharShape::Save(ISave *isave) {
	ULONG nb;

	isave->BeginChunk(CS_INDEX_CHUNK);
	isave->Write(&index,sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(CS_WIDTH_CHUNK);
	isave->Write(&width,sizeof(float), &nb);
	isave->EndChunk();

	isave->BeginChunk(CS_SHAPE_CHUNK);
	shape.Save(isave);
	isave->EndChunk();

	return IO_OK;
	}

IOResult CharShape::Load(ILoad *iload) {
	ULONG nb;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case CS_INDEX_CHUNK:
				res = iload->Read(&index,sizeof(int), &nb);
				break;
			case CS_WIDTH_CHUNK:
				res = iload->Read(&width,sizeof(float), &nb);
				break;
			case CS_SHAPE_CHUNK:
				res = shape.Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

typedef Tab<CharShape *> CharShapeTab;

// TextObject flags
#define TEXTOBJ_BOLD		(1<<0)	// No longer used
//#define TEXTOBJ_ITALIC		(1<<1)
//#define TEXTOBJ_UNDERLINE		(1<<2)
// RB: 3/01/96 -- moved to istdplug.h

class TextObject: public SimpleSpline, public IParamArray, public ITextObject {
		friend class TextObjCreateCallBack;

	public:
		// We cache our hierarchy so that updates are quicker
		BOOL organized;
		ShapeHierarchy cachedHier;

		// This flag allows us to create text consistent with MAX 1.x versions
		BOOL textVersion;

		// Keep UI update from causing redraw
//		BOOL updatingUI;

		// Manual update?
		BOOL manualUpdate;
		BOOL doUpdate;

		DWORD textFlags;	// See above
		int textAlignment;
		BOOL baselineAligned;
		// Here is the font our text is using, along with a handle to the font (supplied by the
		// MAX Bezier Font Manager
		TSTR fontName;
		BEZFONTHANDLE fontHandle;
		DWORD fontFlags;
		BezFontInfo fontInfo;		// Only valid if fontHandle > 0
		// This is the user's string
		TSTR string;
		// This holds the bezier form of our text string in its normalized
		// size.  When called upon to BuildShape, we scale this according to the
		// size parameter and stuff it into the SimpleSpline's shape.
		BezierShape textShape;
		BOOL textShapeValid;
		// The specific parameters used for the cached shape
		float tsSize, tsKerning, tsLeading;
		// Cached character shapes
		CharShapeTab chShapes;
				
		// Class vars
		static TextObject *editObj;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		static float dlgSize;
		static float dlgKerning;
		static float dlgLeading;
		static TSTR dlgFont;
		static TSTR dlgString;
		static BOOL dlgItalic;
		static BOOL dlgUnderline;
		static BOOL dlgAlignment;

		void BuildShape(TimeValue t,BezierShape& ashape);

		TextObject();
		~TextObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return ::GetString(IDS_TH_TEXT); }
		void InitNodeName(TSTR& s) { s = ::GetString(IDS_TH_TEXT); }		
		Class_ID ClassID() { return Class_ID(TEXT_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(::GetString(IDS_TH_TEXT_CLASS)); }
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		BOOL ValidForDisplay(TimeValue t);
		void UpdateShape(TimeValue t);
		Interval ObjectValidity(TimeValue t);
		void* GetInterface(ULONG id);

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		void SetSize(TimeValue t, float s);
		float GetSize(TimeValue t, Interval& valid = Interval(0,0));
		void SetKerning(TimeValue t, float s);
		float GetKerning(TimeValue t, Interval& valid = Interval(0,0));
		void SetLeading(TimeValue t, float s);
		float GetLeading(TimeValue t, Interval& valid = Interval(0,0));

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		void InvalidateUI() { if (pmapParam) pmapParam->Invalidate(); }

		// Our very own methods!
		
		// This returns TRUE if it's OK to change the text string or font style
		BOOL CanChange();
		
		// This takes our string and uses the MAX Bezier Font system to build
		// the normalized-size shape object
		void ConstructTextShape(float size, float kerning, float leading);
		void InvalidateTextShape();

		// Change the font to the given name and style
		BOOL ChangeFont(TSTR name, DWORD flags);

		// RB: Changes the text to the given string
		BOOL ChangeText(TSTR string);

		// Enable/Disable text controls
		void MaybeDisableControls();				

		// Flush the cached characters
		void FlushCharCache();

		// Update the character shape cache and return the number of items cached
		int UpdateCharCache();

		// Override this because we're not as simple as the usual SimpleSpline
		ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier=NULL);

		// Functions for getting/setting styles
		BOOL GetItalic() { return (textFlags & TEXTOBJ_ITALIC) ? TRUE : FALSE; }
		BOOL GetUnderline() { return (textFlags & TEXTOBJ_UNDERLINE) ? TRUE : FALSE; }
		void SetItalic(BOOL sw);
		void SetUnderline(BOOL sw);

		// Functions for getting/setting alignment
		BOOL SetAlignment(int type);
		int GetAlignment() { return textAlignment; }

		// RB
		TSTR GetFont() {return fontName;}
		TSTR GetString() {return string;}

		void UpdateUI(BOOL needRedraw);

		// Convert text object's flags to equivalent font flags
		DWORD FontFlagsFromTextFlags();

		// Animatable
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
		
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};				

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
TextObject *TextObject::editObj   = NULL;
IParamMap *TextObject::pmapParam  = NULL;
IObjParam *TextObject::ip         = NULL;
Point3 TextObject::crtPos         = Point3(0,0,0);
float TextObject::dlgSize         = INIT_TEXT_SIZE;
float TextObject::dlgKerning      = 0.0f;
float TextObject::dlgLeading      = 0.0f;
TSTR TextObject::dlgFont;
TSTR TextObject::dlgString;
BOOL TextObject::dlgItalic        = FALSE;
BOOL TextObject::dlgUnderline     = FALSE;
BOOL TextObject::dlgAlignment     = FALSE;

//------------------------------------------------------

class TextObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new TextObject; }
	const TCHAR *	ClassName() { return ::GetString(IDS_TH_TEXT_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(TEXT_CLASS_ID,0); }
	const TCHAR* 	Category() { return ::GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static TextObjClassDesc textObjDesc;

ClassDesc* GetTextDesc() {
	// Take advantage of this to load our initial strings
	static BOOL loaded = FALSE;
	if(!loaded) {
		loaded = TRUE;
		textObjDesc.ResetClassParams(FALSE);
		}
	return &textObjDesc;
	}

void TextObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	TextObject::crtPos         = Point3(0,0,0);
	TextObject::dlgSize        = INIT_TEXT_SIZE;
	TextObject::dlgKerning     = 0.0f;
	TextObject::dlgLeading     = 0.0f;
	TextObject::dlgFont        = TSTR(::GetString(IDS_TH_ARIAL));
#ifdef DESIGN_VER
	TextObject::dlgString      = TSTR(::GetString(IDS_TH_VIZ_TEXT));
#else
	TextObject::dlgString      = TSTR(::GetString(IDS_TH_MAX_TEXT));
#endif
	TextObject::dlgItalic      = FALSE;
	TextObject::dlgUnderline   = FALSE;
	TextObject::dlgAlignment   = TEXTOBJ_LEFT;
	}

// Parameter map indices
#define PB_SIZE		0
#define PB_KERNING	1
#define PB_LEADING	2

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_LENGTH		2
#define PB_TI_WIDTH			3

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Size
	ParamUIDesc(
		PB_SIZE,
		EDITTYPE_UNIVERSE,
		IDC_TEXTSIZEEDIT,IDC_TEXTSIZESPINNER,
		MIN_SIZE,MAX_SIZE,
		SPIN_AUTOSCALE),	
	
	// Kerning
	ParamUIDesc(
		PB_KERNING,
		EDITTYPE_UNIVERSE,
		IDC_TEXTKERNINGEDIT,IDC_TEXTKERNINGSPINNER,
		MIN_KERNING,MAX_KERNING,
		SPIN_AUTOSCALE),
	
	// Leading
	ParamUIDesc(
		PB_LEADING,
		EDITTYPE_UNIVERSE,
		IDC_TEXTLEADINGEDIT,IDC_TEXTLEADINGSPINNER,
		MIN_LEADING,MAX_LEADING,
		SPIN_AUTOSCALE)
	};
#define PARAMDESC_LENGTH 3

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_FLOAT, NULL, TRUE, 1 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 } };		

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 } };		
#define PBLOCK_LENGTH	3

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,2,0),
	ParamVersionDesc(descVer1,1,1)
	};
#define NUM_OLDVERSIONS	2

// Current version
#define CURRENT_VERSION	2
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,CURRENT_VERSION);

static HIMAGELIST hTextButtons = NULL;

class DeleteTextButtonResources {
	public:
		~DeleteTextButtonResources() {
			ImageList_Destroy(hTextButtons);
			}
	};
static DeleteTextButtonResources	theTBDelete;

static void LoadTextButtonResources()
	{
	static BOOL loaded=FALSE;
	if (loaded) return;
	loaded = TRUE;	
	HBITMAP hBitmap, hMask;

	hTextButtons = ImageList_Create(14, 15, TRUE, 10, 0);
	hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_TEXT_BUTTONS));
	hMask   = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_TEXT_MASKBUTTONS));
	ImageList_Add(hTextButtons,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	}

class ListBoxFiller : public BezFontEnumProc {
	public:
		HWND control;
		ListBoxFiller(HWND d, int c) { control=GetDlgItem(d, c); }
		BOOL CALLBACK Entry(BezFontInfo &info, LPARAM userInfo);
	};

BOOL CALLBACK ListBoxFiller::Entry(BezFontInfo &info, LPARAM userInfo) {
	if(!control)
		return FALSE;	// Can't use 'em anyway...
	SendMessage(control, CB_ADDSTRING, 0, (LPARAM)info.name.data());
	return TRUE;	// Keep on enumeratin'!	
	}

void TextObject::MaybeDisableControls() {
	if(pmapParam && editObj == this) {
		HWND hDlg = pmapParam->GetHWnd();
		EnableWindow(GetDlgItem(hDlg,IDC_TEXTENTRY),CanChange() ? TRUE : FALSE);
		ICustButton *but = GetICustButton(GetDlgItem(hDlg,IDC_TEXT_UPDATE));
		but->Enable((IsDlgButtonChecked(hDlg,IDC_TEXT_MANUAL_UPDATE)) ? TRUE : FALSE);
		ReleaseICustButton(but);
		}
	}

static void StuffFontName(HWND hDlg, TextObject *ob) {
	int thisFont = SendDlgItemMessage(hDlg, IDC_TEXTFONT, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)ob->fontName.data());
	if(thisFont != CB_ERR) {
		SendDlgItemMessage(hDlg, IDC_TEXTFONT, CB_SETCURSEL, thisFont, 0);
		}
	else {
		SendDlgItemMessage(hDlg, IDC_TEXTFONT, CB_SETCURSEL, (WPARAM)-1, 0);
		SetDlgItemText(hDlg, IDC_TEXTFONT, (LPCTSTR)ob->fontName.data());
		}
	}

static void UpdateAlignment(HWND hDlg, TextObject *ob, int type = -1) {
	if(type >= 0)
		ob->SetAlignment(type);
	else
		type = ob->GetAlignment();
	ICustButton *but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTLEFT));
	but->SetCheck((type==TEXTOBJ_LEFT) ? TRUE : FALSE);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTCENTER));
	but->SetCheck((type==TEXTOBJ_CENTER) ? TRUE : FALSE);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTRIGHT));
	but->SetCheck((type==TEXTOBJ_RIGHT) ? TRUE : FALSE);
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTJUSTIFIED));
	but->SetCheck((type==TEXTOBJ_JUSTIFIED) ? TRUE : FALSE);
	ReleaseICustButton(but);
	}

// This dialog proc handles special fields within the rollup panel
class TextParamDialogProc : public ParamMapUserDlgProc {
	public:
		TextObject *text;
		TextParamDialogProc(TextObject *t) { text=t; }
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() { delete this; }
		void Update(TimeValue t);
	};


BOOL TextParamDialogProc::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	ICustButton *but;
	
	switch ( msg ) {
		case WM_INITDIALOG: {
			LoadTextButtonResources();
			but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTITALIC));
			but->SetType(CBT_CHECK);
			but->SetCheck((text->textFlags & TEXTOBJ_ITALIC) ? TRUE : FALSE);
			but->SetImage( hTextButtons, 1,1,4,4, 14, 15);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTUNDERLINE));
			but->SetType(CBT_CHECK);
			but->SetCheck((text->textFlags & TEXTOBJ_UNDERLINE) ? TRUE : FALSE);
			but->SetImage( hTextButtons, 2,2,5,5, 14, 15);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTLEFT));
			but->SetType(CBT_CHECK);
			but->SetImage( hTextButtons, 6,6,6,6, 14, 15);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTCENTER));
			but->SetType(CBT_CHECK);
			but->SetImage( hTextButtons, 7,7,7,7, 14, 15);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTRIGHT));
			but->SetType(CBT_CHECK);
			but->SetImage( hTextButtons, 8,8,8,8, 14, 15);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTJUSTIFIED));
			but->SetType(CBT_CHECK);
			but->SetImage( hTextButtons, 9,9,9,9, 14, 15);
			ReleaseICustButton(but);
			UpdateAlignment(hWnd, text);

			CheckDlgButton( hWnd, IDC_TEXT_MANUAL_UPDATE, text->manualUpdate);

			// TH 6/1/99 -- Flush the font list
			SendMessage(GetDlgItem(hWnd,IDC_TEXTFONT), CB_RESETCONTENT, 0, (LPARAM)0);

			// Load up the listbox with the available fonts...
			// ... And the active font
			ListBoxFiller filler(hWnd, IDC_TEXTFONT);
			theBezFontManager.EnumerateFonts(filler, 0);

			// See if the current font is present in the list
			StuffFontName(hWnd, text);

			// Stuff the current text into the edit field
			SendMessage(GetDlgItem(hWnd,IDC_TEXTENTRY), WM_SETTEXT, 0, (LPARAM)text->string.data());
			
			// Disable the rest of the controls if can't change...
			text->MaybeDisableControls();
			}			
			return FALSE;	// DB 2/27

		case WM_DESTROY:
			return FALSE;

		case WM_COMMAND: {
			BOOL needRedraw = FALSE;			
			switch( LOWORD(wParam) ) {				
				case IDC_TEXTITALIC:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTITALIC));
							text->SetItalic(but->IsChecked());
							ReleaseICustButton(but);
							needRedraw = TRUE;
							break;
						}
					break;
				case IDC_TEXTUNDERLINE:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							but = GetICustButton(GetDlgItem(hWnd,IDC_TEXTUNDERLINE));
							text->SetUnderline(but->IsChecked());
							ReleaseICustButton(but);
							needRedraw = TRUE;
							break;
						}
					break;
				case IDC_TEXTLEFT:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							UpdateAlignment(hWnd, text, TEXTOBJ_LEFT);
							needRedraw = TRUE;
							break;
						}
					break;
				case IDC_TEXTCENTER:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							UpdateAlignment(hWnd, text, TEXTOBJ_CENTER);
							needRedraw = TRUE;
							break;
						}
					break;
				case IDC_TEXTRIGHT:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							UpdateAlignment(hWnd, text, TEXTOBJ_RIGHT);
							needRedraw = TRUE;
							break;
						}
					break;
				case IDC_TEXTJUSTIFIED:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							UpdateAlignment(hWnd, text, TEXTOBJ_JUSTIFIED);
							needRedraw = TRUE;
							break;
						}
					break;
				case IDC_TEXTFONT: {
					switch(HIWORD(wParam)) {
						case CBN_SETFOCUS:
							DisableAccelerators();					
							break;
						case CBN_KILLFOCUS:
							EnableAccelerators();
							break;
						case CBN_SELCHANGE: {
							int selection = SendDlgItemMessage(hWnd, IDC_TEXTFONT, CB_GETCURSEL, 0, 0);
							if(selection != CB_ERR) {
								int fontNameLength = SendDlgItemMessage(hWnd, IDC_TEXTFONT, CB_GETLBTEXTLEN, selection, 0);
								TSTR work;
								work.Resize(fontNameLength+1);
								SendDlgItemMessage(hWnd, IDC_TEXTFONT, CB_GETLBTEXT, selection, (LPARAM)work.data());
								if(text->ChangeFont(work, text->FontFlagsFromTextFlags())) {
									text->textVersion = TEXT_VERSION_NUMBER;
									text->baselineAligned = TRUE;
									}
								// Stuff back in whatever font the object has now!
								SendDlgItemMessage(hWnd, IDC_TEXTFONT, WM_SETTEXT, 0, (LPARAM)text->fontName.data());
								text->MaybeDisableControls();
								text->ip->RedrawViews(text->ip->GetTime(),REDRAW_INTERACTIVE);
								}
							}
							break;
						}
					}
					break;
				case IDC_TEXTENTRY:
					switch(HIWORD(wParam)) {
						case EN_SETFOCUS:
							DisableAccelerators();					
							break;
						case EN_KILLFOCUS:
							EnableAccelerators();
							break;
						case EN_CHANGE: {
							int len = SendDlgItemMessage(hWnd, IDC_TEXTENTRY, WM_GETTEXTLENGTH, 0, 0);
							TSTR temp;
							temp.Resize(len+1);
							SendDlgItemMessage(hWnd, IDC_TEXTENTRY, WM_GETTEXT, len+1, (LPARAM)temp.data());
							text->string = temp;
//							if(!text->updatingUI)
								needRedraw = TRUE;
							}
							break;
						}
					break;
				case IDC_TEXT_MANUAL_UPDATE:
					text->manualUpdate = IsDlgButtonChecked(hWnd, IDC_TEXT_MANUAL_UPDATE);
					text->MaybeDisableControls();
					if(text->manualUpdate)
						needRedraw = TRUE;
					break;
				case IDC_TEXT_UPDATE:
					switch HIWORD(wParam) {
						case BN_CLICKED:
							text->doUpdate = TRUE;
							needRedraw = TRUE;
							break;
						}
					break;
				}
			if(needRedraw) {
				text->InvalidateTextShape();
				text->ip->RedrawViews(text->ip->GetTime(),REDRAW_INTERACTIVE);
				}
			return FALSE;
			}

		default:
			return FALSE;
		}
	}

void TextParamDialogProc::Update(TimeValue t) {
	DebugPrint("Updating dialog proc\n");
	}

// RB: 3-01-96
void* TextObject::GetInterface(ULONG id)
	{
	if (id==I_TEXTOBJECT) return (ITextObject*)this;
	else return Object::GetInterface(id); 
	}

void TextObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	editObj  = this;

	if (pmapParam) {
		// Left over from last one ceated
		pmapParam->SetParamBlock(pblock);

		// Init the dialog to our values.
		string = dlgString; 
//		UpdateUI(ip->GetTime());
	} else {
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_TEXTPARAM),
			::GetString(IDS_TH_PARAMETERS),
			0);
		}
	if(pmapParam)
		pmapParam->SetUserDlgProc(new TextParamDialogProc(this));
	
	// Open the font
	ChangeFont(fontName, FontFlagsFromTextFlags());

	// Maybe disable the text controls if the font isn't available
	MaybeDisableControls();
	}
		
void TextObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	HWND hDlg = pmapParam->GetHWnd();
	// Save these values in class variables so the next object created will inherit them.
	int selection = SendDlgItemMessage(hDlg, IDC_TEXTFONT, CB_GETCURSEL, 0, 0);
	if(selection != CB_ERR) {
		int fontNameLength = SendDlgItemMessage(hDlg, IDC_TEXTFONT, CB_GETLBTEXTLEN, selection, 0);
		dlgFont.Resize(fontNameLength+1);
		SendDlgItemMessage(hDlg, IDC_TEXTFONT, CB_GETLBTEXT, selection, (LPARAM)dlgFont.data());
		}
	pblock->GetValue(PB_SIZE,ip->GetTime(),dlgSize,FOREVER);
	pblock->GetValue(PB_KERNING,ip->GetTime(),dlgKerning,FOREVER);
	pblock->GetValue(PB_LEADING,ip->GetTime(),dlgLeading,FOREVER);
	int len = SendDlgItemMessage(hDlg, IDC_TEXTENTRY, WM_GETTEXTLENGTH, 0, 0);
	dlgString.Resize(len+1);
	SendDlgItemMessage(hDlg, IDC_TEXTENTRY, WM_GETTEXT, len+1, (LPARAM)dlgString.data());
	ICustButton *but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTITALIC));
	dlgItalic = but->IsChecked();
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTUNDERLINE));
	dlgUnderline = but->IsChecked();
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTLEFT));
	if(but->IsChecked())
		dlgAlignment = TEXTOBJ_LEFT;
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTCENTER));
	if(but->IsChecked())
		dlgAlignment = TEXTOBJ_CENTER;
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTRIGHT));
	if(but->IsChecked())
		dlgAlignment = TEXTOBJ_RIGHT;
	ReleaseICustButton(but);
	but = GetICustButton(GetDlgItem(hDlg,IDC_TEXTJUSTIFIED));
	if(but->IsChecked())
		dlgAlignment = TEXTOBJ_JUSTIFIED;
	ReleaseICustButton(but);

	SimpleSpline::EndEditParams(ip,flags,next);
	this->ip = NULL;
	editObj  = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		}
	
	// Close the font we were using
	if(fontHandle) {
		theBezFontManager.CloseFont(fontHandle);
		fontHandle = 0;
		}
	}

void TextObject::BuildShape(TimeValue t, BezierShape& ashape) {
	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float size, kerning, leading;
	pblock->GetValue(PB_SIZE, t, size, ivalid);
	LimitValue( size, MIN_SIZE, MAX_SIZE );
	pblock->GetValue(PB_KERNING, t, kerning, ivalid);
	pblock->GetValue(PB_LEADING, t, leading, ivalid);

	// Make sure our source shape is updated
	ConstructTextShape(size, kerning, leading);

	// Delete the existing shape and copy our text spline into it
	ashape.NewShape();

	ashape = textShape;

	// Get parameters from SimpleSpline and place them in the BezierShape
	int steps;
	BOOL optimize,adaptive;
	ipblock->GetValue(IPB_STEPS, t, steps, ivalid);
	ipblock->GetValue(IPB_OPTIMIZE, t, optimize, ivalid);
	ipblock->GetValue(IPB_ADAPTIVE, t, adaptive, ivalid);
	ashape.steps = adaptive ? -1 : steps;
	ashape.optimize = optimize;

	// Now add all the necessary points
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

void TextObject::UpdateUI(BOOL needRedraw)
	{
	if(pmapParam && (editObj == this)) {
		HWND hTextParams = pmapParam->GetHWnd();
		if(hTextParams) {
			StuffFontName(hTextParams, this);
			TCHAR oldString[256];
			SendMessage(GetDlgItem(hTextParams,IDC_TEXTENTRY), WM_GETTEXT, 255, (LPARAM)oldString);
			TSTR os(oldString);
			if(!(os == string))
				SendMessage(GetDlgItem(hTextParams,IDC_TEXTENTRY), WM_SETTEXT, 0, (LPARAM)string.data());
			ICustButton *but = GetICustButton(GetDlgItem(hTextParams,IDC_TEXTITALIC));
			but->SetCheck((textFlags & TEXTOBJ_ITALIC) ? TRUE : FALSE);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hTextParams,IDC_TEXTUNDERLINE));
			but->SetCheck((textFlags & TEXTOBJ_UNDERLINE) ? TRUE : FALSE);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hTextParams,IDC_TEXTLEFT));
			but->SetCheck((textAlignment == TEXTOBJ_LEFT) ? TRUE : FALSE);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hTextParams,IDC_TEXTCENTER));
			but->SetCheck((textAlignment == TEXTOBJ_CENTER) ? TRUE : FALSE);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hTextParams,IDC_TEXTRIGHT));
			but->SetCheck((textAlignment == TEXTOBJ_RIGHT) ? TRUE : FALSE);
			ReleaseICustButton(but);
			but = GetICustButton(GetDlgItem(hTextParams,IDC_TEXTJUSTIFIED));
			but->SetCheck((textAlignment == TEXTOBJ_JUSTIFIED) ? TRUE : FALSE);
			ReleaseICustButton(but);
//			InvalidateUI();
			}
		else
			assert(0);
		}
	if(needRedraw) {
		InvalidateTextShape();
//		ip->RedrawViews(ip->GetTime(),REDRAW_INTERACTIVE);
		}
	}

TextObject::TextObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	MakeRefByID(FOREVER, USERPBLOCK, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);

	string = dlgString;
//DebugPrint("Created new TextObject @%p\n",this);
	fontName = dlgFont;
	fontFlags = 0;
	textFlags = (dlgItalic ? TEXTOBJ_ITALIC : 0) | (dlgUnderline ? TEXTOBJ_UNDERLINE : 0);
	fontHandle = 0;
	textShapeValid = FALSE;
	organized = FALSE;
	textAlignment = dlgAlignment;
	manualUpdate = FALSE;
	doUpdate = FALSE;
	textVersion = TEXT_VERSION_NUMBER;
	baselineAligned = TRUE;

	pblock->SetValue(PB_SIZE,0,dlgSize);
	pblock->SetValue(PB_KERNING,0,dlgKerning);
	pblock->SetValue(PB_LEADING,0,dlgLeading);
 	}

TextObject::~TextObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	if(fontHandle) {
		theBezFontManager.CloseFont(fontHandle);
		fontHandle = 0;
		}
	FlushCharCache();	// TH 12/7/00
	UnReadyInterpParameterBlock();
	}

class TextObjCreateCallBack: public CreateMouseCallBack {
	TextObject *ob;
	Point3 p0,p1;
	IPoint2 sp1;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(TextObject *obj) { ob = obj; }
	};

int TextObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;

#ifdef _3D_CREATE
	DWORD snapdim = SNAP_IN_3D;
#else
	DWORD snapdim = SNAP_IN_PLANE;
#endif

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
			vpt->SnapPreview(m,m,NULL, snapdim);
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:
				ob->suspendSnap = TRUE;
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				break;
			case 1:
				p0 = vpt->SnapPoint(m,m,NULL,snapdim);
				mat.SetTrans(p0);
				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return CREATE_STOP;
					}
				break;
			}
		}
	else
	if (msg == MOUSE_ABORT) {
		return CREATE_ABORT;
		}

	return TRUE;
	}

static TextObjCreateCallBack textCreateCB;

CreateMouseCallBack* TextObject::GetCreateMouseCallBack() {
	textCreateCB.SetObj(this);
	return(&textCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle TextObject::Clone(RemapDir& remap) {
	TextObject* newob = new TextObject();
	newob->textVersion = textVersion;
	newob->manualUpdate = manualUpdate;
	newob->doUpdate = doUpdate;
	newob->textFlags = textFlags;
	newob->textAlignment = textAlignment;
	newob->baselineAligned = baselineAligned;
	newob->fontName = fontName;
	newob->fontFlags = fontFlags;
	newob->string = string;
	for(int i = 0; i < chShapes.Count(); ++i) {
		CharShape *cs = new CharShape;
		*cs = *chShapes[i];
		newob->chShapes.Append(1, &cs);
		}
	newob->SimpleSplineClone(this);
	newob->ReplaceReference(USERPBLOCK,pblock->Clone(remap));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL TextObject::ValidForDisplay(TimeValue t) {
	float size;
	pblock->GetValue(PB_SIZE, t, size, ivalid);
	return (size == 0) ? FALSE : TRUE;
	}

void TextObject::UpdateShape(TimeValue t) {
	if ( ivalid.InInterval(t) )
		return;
	BuildShape(t, shape);
	}

Interval TextObject::ObjectValidity(TimeValue time) {
	UpdateShape(time);
//	UpdateUI(time);
	return ivalid;	
	}

ParamDimension *TextObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SIZE:
		case PB_KERNING:
		case PB_LEADING:
			return stdWorldDim;			
		default:
			return defaultDim;
		}
	}

TSTR TextObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_SIZE:
			return TSTR(::GetString(IDS_TH_SIZE));
		case PB_KERNING:
			return TSTR(::GetString(IDS_TH_KERNING));
		case PB_LEADING:
			return TSTR(::GetString(IDS_TH_LEADING));
		default:
			return TSTR(_T(""));
		}
	}

// From ParamArray
BOOL TextObject::SetValue(int i, TimeValue t, int v) 
	{
//	switch (i) {
//		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
//		}		
	return TRUE;
	}

BOOL TextObject::SetValue(int i, TimeValue t, float v)
	{
//	switch (i) {				
//		case PB_TI_LENGTH: crtLength = v; break;
//		case PB_TI_WIDTH: crtWidth = v; break;
//		}	
	return TRUE;
	}

BOOL TextObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
//	switch (i) {
//		case PB_TI_POS: crtPos = v; break;
//		}		
	return TRUE;
	}

BOOL TextObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
//	switch (i) {
//		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
//		}
	return TRUE;
	}

BOOL TextObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
//	switch (i) {		
//		case PB_TI_LENGTH: v = crtLength; break;
//		case PB_TI_WIDTH: v = crtWidth; break;
//		}
	return TRUE;
	}

BOOL TextObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
//	switch (i) {		
//		case PB_TI_POS: v = crtPos; break;		
//		}
	return TRUE;
	}

void TextObject::SetSize( TimeValue t, float s) 
	{
	pblock->SetValue( PB_SIZE, t, s );
	ShapeInvalid();
	if(!manualUpdate)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float TextObject::GetSize( TimeValue t, Interval& valid )
	{
	float f;
	pblock->GetValue( PB_SIZE, t, f, valid );
	return f;
	}

void TextObject::SetKerning( TimeValue t, float s) 
	{
	pblock->SetValue( PB_KERNING, t, s );
	ShapeInvalid();
	if(!manualUpdate)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float TextObject::GetKerning( TimeValue t, Interval& valid )
	{
	float f;
	pblock->GetValue( PB_KERNING, t, f, valid );
	return f;
	}

void TextObject::SetLeading( TimeValue t, float s) 
	{
	pblock->SetValue( PB_LEADING, t, s );
	ShapeInvalid();
	if(!manualUpdate)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

float TextObject::GetLeading( TimeValue t, Interval& valid )
	{
	float f;
	pblock->GetValue( PB_LEADING, t, f, valid );
	return f;
	}

BOOL TextObject::CanChange() {
	return (fontHandle > 0) ? TRUE : FALSE;
	}

class TextLine {
	public:
		BezierShape *shapes;
		int *chars;
		IntTab *chpoly;
		float *widths;
		float *yoffs;
		TextLine(int lines) {
			shapes = new BezierShape [lines];
			chpoly = new IntTab [lines];
			widths = new float [lines];
			yoffs = new float [lines];
			for(int l=0; l < lines; ++l) {
				widths[l] = 0.0f;
				yoffs[l] = 0.0f;
				}
			}
		~TextLine() {
			delete [] shapes;
			delete [] chpoly;
			delete [] widths;
			delete [] yoffs;
			}
		void Underline(int line, float xoffset, float yoffset, float scaleFactor);
	};

void TextLine::Underline(int line, float xoffset, float yoffset, float scaleFactor) {
	Spline3D *spline = shapes[line].NewSpline();

	// Now add all the necessary points
	// We'll add 'em as auto corners initially, have the spline package compute some vectors (because
	// I'm basically lazy and it does a great job, besides) then turn 'em into bezier corners!
	float x1 = 0.0f;
	float y1 = -50.0f * scaleFactor + yoffset;		// Drop it below the baseline a bit
	float x2 = xoffset;
	float y2 = -100.0f * scaleFactor + yoffset;
	Point3 p = Point3(x1, y1, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	p = Point3(x1, y2, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	p = Point3(x2, y2, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	p = Point3(x2, y1, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	spline->SetClosed();
	spline->ComputeBezPoints();
	for(int i = 0; i < 4; ++i)
		spline->SetKnotType(i, KTYPE_BEZIER_CORNER);
	shapes[line].UpdateSels();
	}

void TextObject::ConstructTextShape(float size, float kerning, float leading) {
	int i;
	if(!doUpdate && manualUpdate && pmapParam && pmapParam->GetParamBlock() == pblock)
		return;
	doUpdate = FALSE;
	if(textShapeValid && size==tsSize && kerning==tsKerning && leading==tsLeading)
		return;
//DebugPrint("Constructing textShape, font:[%s]\n",fontName.data());

	// Remember the parameters used for this cached shape
	tsSize = size;
	tsKerning = kerning;
	tsLeading = leading;

	// Get the scaling factor
	float scaleFactor = size / 1000.0f;

	textShape.NewShape();

	// Now update our cached shapes if we have a valid font
	// This only occurs when the UI is up (i.e. in create/modify branch).
	// The rest of the time we rely on the cached characters
	if(fontHandle)
		UpdateCharCache();

	int cacheCount = chShapes.Count();

	int chars = string.Length();
//DebugPrint("String: [%s] %d chars\n",string.data(), chars);
	float xoffset = 0.0f, yoffset = 0.0f;
	// Find out how many lines we have and how many characters in each line
	int lines = 1;
	for(i = 0; i < chars; ++i) {
		UINT index;
		if (isleadbyte(string[i])) {
			union {
				unsigned char b[2];
				unsigned short c;
			} dbcs;
			dbcs.b[0] = string[i];
			dbcs.b[1] = string[++i];
			index = swab(dbcs.c);
		} else {
			index = (UCHAR)string[i];
		}

		if(index == 13)
			lines++;
		}
	// Now load the lines into the work shapes, recording the width of each line's
	// characters and the widest line
	TextLine tLines(lines);
	float widest = -100.0f;
	int line = 0, widestLine;
	for(i = 0; i < chars; ++i) {
		UINT index;
		if (isleadbyte(string[i])) {
			union {
				unsigned char b[2];
				unsigned short c;
			} dbcs;
			dbcs.b[0] = string[i];
			dbcs.b[1] = string[++i];
			index = swab(dbcs.c);
		} else {
			index = (UCHAR)string[i];
		}

		if(index == 13) {
			tLines.widths[line] = xoffset;
			tLines.yoffs[line] = yoffset;
			if(xoffset > widest) {
				widest = xoffset;
				widestLine = line;
				}
			line++;
			xoffset = 0.0f;
			yoffset -= (size + leading);
			continue;
			}
		if(index == 10)
			continue;
		for(int j = 0; j < cacheCount; ++j) {
			if((UINT)chShapes[j]->index == index) {
				// Build a transform to move the shape into position and scale it
				Matrix3 tm(1);
				// Add a shear transform if it's italic!
				if(textFlags & TEXTOBJ_ITALIC) {
					Point3 r2 = tm.GetRow(1);
					r2.x = 0.3f;
					tm.SetRow(1, r2);
					}
				tm *= ScaleMatrix(Point3(scaleFactor, scaleFactor, 1.0f));
				tm *= TransMatrix(Point3(xoffset, yoffset, 0.0f));
				BezierShape shape = chShapes[j]->shape;
				shape.Transform(tm);
				tLines.shapes[line] += shape;
				int chix = tLines.shapes[line].SplineCount();
				tLines.chpoly[line].Append(1, &chix);
				xoffset += chShapes[j]->width * scaleFactor + kerning;
				break;
				}
			}
		}
	// Update width if leftover
	if(xoffset) {
		tLines.widths[line] = xoffset;
		tLines.yoffs[line] = yoffset;
		if(xoffset > widest) {
			widest = xoffset;
			widestLine = line;
			}
		}
	// Do justification if needed
	if(textAlignment == TEXTOBJ_JUSTIFIED) {
		for(line = 0; line < lines; ++line) {
			if(line != widestLine) {
				int chars = tLines.chpoly[line].Count();
				if(chars > 1) {
					int spaces = chars - 1;
					float increment = (widest - tLines.widths[line]) / (float)spaces;
					float offset = increment;
					for(int space = 0; space < spaces; ++space,offset+=increment) {
						Matrix3 tm=TransMatrix(Point3(offset, 0.0f, 0.0f));
						for(int i = tLines.chpoly[line][space]; i < tLines.chpoly[line][space+1]; ++i) {
							tLines.shapes[line].GetSpline(i)->Transform(&tm);
							}
						}
					}
				}
			}
		}
	// Handle underlining if necessary
	if(textFlags & TEXTOBJ_UNDERLINE) {
		for(line = 0; line < lines; ++line) {
			if(textAlignment == TEXTOBJ_JUSTIFIED)
				tLines.Underline(line, widest, tLines.yoffs[line], scaleFactor);
			else
				tLines.Underline(line, tLines.widths[line], tLines.yoffs[line], scaleFactor);
			}
		}
	// Toss 'em all into the output shape with alignment
	for(i = 0; i < lines; ++i) {
		Matrix3 tm(1);
		switch(textAlignment) {
			case TEXTOBJ_LEFT:
				break;	// No action required
			case TEXTOBJ_CENTER:
				tm = TransMatrix(Point3((widest - tLines.widths[i]) / 2.0f, 0.0f, 0.0f));
				break;
			case TEXTOBJ_RIGHT:
				tm = TransMatrix(Point3(widest - tLines.widths[i], 0.0f, 0.0f));
				break;
			case TEXTOBJ_JUSTIFIED: 
				break;
			}
		tLines.shapes[i].Transform(tm);
		textShape += tLines.shapes[i];
		}
	textShape.InvalidateGeomCache();

	// Center up the shape
	Box3 shapeBox = textShape.GetBoundingBox();
	Point3 textCenter = shapeBox.Center();
	Matrix3 centerTM = TransMatrix(-Point3(textCenter.x, baselineAligned ? 0.0f : textCenter.y, 0.0f));
	textShape.Transform(centerTM);
	textShape.InvalidateGeomCache();

	// We need to make sure we dump any cached mesh that's associated with our shape:
	InvalidateGeomCache();

	textShapeValid = TRUE;
	}

void TextObject::InvalidateTextShape() {
	textShapeValid = FALSE;
	organized = FALSE;
	ShapeInvalid();
	if(!manualUpdate || doUpdate)
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}

DWORD TextObject::FontFlagsFromTextFlags() {
	DWORD f = 0;
	return f;
	}

// TH 12/15/00 -- Added this to intercept messages to things depending on us -- Allows us to
// suppress updates when spinners change and we're on manual update
RefResult TextObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	// If this isn't one of our references, pass it on to the ShapeObject...
	if(hTarget != pblock && hTarget != ipblock)
		return ShapeObject::NotifyRefChanged(changeInt, hTarget, partID, message);
	if(message == REFMSG_CHANGE && manualUpdate)
		return REF_STOP;
	return SimpleSpline::NotifyRefChanged(changeInt, hTarget, partID, message);
	}

// Returns TRUE if font actually changed

BOOL TextObject::ChangeFont(TSTR name, DWORD flags) {
	if(!name.Length())
		return FALSE;
	if(name == fontName && flags == fontFlags && fontHandle > 0)
		return FALSE;				// If it's the same, forget it!
	BEZFONTHANDLE newHandle = theBezFontManager.OpenFont(name, flags);
	if(!newHandle) {
		return FALSE;
		}
	if(fontHandle)
		theBezFontManager.CloseFont(fontHandle);
	fontHandle = newHandle;
	if(name != fontName || flags != fontFlags) {
		fontName = name;
		fontFlags = flags;
		FlushCharCache();	// The cached characters are no longer valid!
		}
	theBezFontManager.GetFontInfo(fontName, fontInfo);
	InvalidateTextShape();
	return TRUE;
	}

BOOL TextObject::ChangeText(TSTR string)
	{
	if (CanChange()) {
		this->string = string;
		InvalidateTextShape();
		return TRUE;
	} else {
		return FALSE;
		}
	}

void TextObject::FlushCharCache() {
	int chars = chShapes.Count();
	for(int i = 0; i < chars; ++i)
		delete chShapes[i];
	chShapes.Delete(0, chars);
	}

int TextObject::UpdateCharCache() {
	int chars = chShapes.Count();
	int strChars = string.Length();
	int i,j;
	for(i = 0; i < chars; ++i) {
		CharShape *cs = chShapes[i];
		cs->keep = FALSE;
		int index = cs->index;
		for(j = 0; j < strChars; ++j) {
			if(index == string[i])
				cs->keep = TRUE;
			}
		}
	// Get rid of the ones we don't need anymore
	for(i = chars-1; i >= 0; --i) {
		if(!chShapes[i]->keep) {
			delete chShapes[i];
			chShapes.Delete(i, 1);
			chars--;
			}
		}
	// Now add in the ones we DO need
	for(i = 0; i < strChars; ++i) {
		UINT index;
		if (isleadbyte(string[i])) {
			union {
				unsigned char b[2];
				unsigned short c;
			} dbcs;
			dbcs.b[0] = string[i];
			dbcs.b[1] = string[++i];
			index = swab(dbcs.c);
		} else {
			index = (UCHAR)string[i];
		}
		BOOL add = TRUE;
		for(j = 0; j < chars; ++j) {
			if(index == (UINT)chShapes[j]->index) {
				add = FALSE;
				break;
				}
			}
		if(add) {
			CharShape *s = new CharShape(index);
			if(!theBezFontManager.BuildCharacter(fontHandle, index, 1000.0f, s->shape, s->width, textVersion))
				s->width = 300.0f;	// Fake it if the character isn't there
			chShapes.Append(1, &s, 5);
			chars++;
			}
		}
//DebugPrint("Character cache updated: %d entries\n",chShapes.Count());
	organized = FALSE;
	return chShapes.Count();
	}

ShapeHierarchy &TextObject::OrganizeCurves(TimeValue t, ShapeHierarchy *hier) {
	UpdateShape(t);
	if(!organized) {
		shape.OrganizeCurves(t, &cachedHier);
		organized = TRUE;
		}
	if(hier)
		*hier = cachedHier;
	return cachedHier;
	}

class TextFlagsRestore : public RestoreObj {
	public:
		TextObject *text;
		DWORD oldFlags;
		DWORD newFlags;
		TextFlagsRestore(TextObject *t, DWORD f) { text=t; oldFlags=t->textFlags; newFlags=f; }
		void Restore(int isUndo) { text->textFlags = oldFlags; text->UpdateUI(TRUE); }
		void Redo() { text->textFlags = newFlags; text->UpdateUI(TRUE); }
		int Size() { return 1; }
		TSTR Description() { return TSTR(_T("text flags restore")); }
	};

void TextObject::SetItalic(BOOL sw) {
	DWORD flags;
	if(sw)
		flags = textFlags | TEXTOBJ_ITALIC;
	else
		flags = textFlags & ~TEXTOBJ_ITALIC;
	theHold.Begin();
	if(theHold.Holding())
		theHold.Put(new TextFlagsRestore(this, flags));
	theHold.Accept(::GetString(IDS_DS_PARAMCHG));
	textFlags = flags;
	}

void TextObject::SetUnderline(BOOL sw) {
	DWORD flags;
	if(sw)
		flags = textFlags | TEXTOBJ_UNDERLINE;
	else
		flags = textFlags & ~TEXTOBJ_UNDERLINE;
	theHold.Begin();
	if(theHold.Holding())
		theHold.Put(new TextFlagsRestore(this, flags));
	theHold.Accept(::GetString(IDS_DS_PARAMCHG));
	textFlags = flags;
	}

class TextAlignRestore : public RestoreObj {
	public:
		TextObject *text;
		DWORD oldAlign;
		DWORD newAlign;
		TextAlignRestore(TextObject *t, int a) { text=t; oldAlign=t->textAlignment; newAlign=a; }
		void Restore(int isUndo) { text->textAlignment = oldAlign; text->UpdateUI(TRUE); }
		void Redo() { text->textAlignment = newAlign; text->UpdateUI(TRUE); }
		int Size() { return 1; }
		TSTR Description() { return TSTR(_T("text align restore")); }
	};
BOOL TextObject::SetAlignment(int type) {
	switch(type) {
		case TEXTOBJ_LEFT:
		case TEXTOBJ_CENTER:
		case TEXTOBJ_RIGHT:
		case TEXTOBJ_JUSTIFIED:
			theHold.Begin();
			if(theHold.Holding())
				theHold.Put(new TextAlignRestore(this, type));
			theHold.Accept(::GetString(IDS_DS_PARAMCHG));
			textAlignment = type;
			return TRUE;
		}
	return FALSE;
	}

#define FONTNAME_CHUNK	 		0x1000
#define STRING_CHUNK 			0x1010
#define OLD_TEXT_CHUNK			0x1020	// Was cached text shape	-- OBSOLETE!!
#define CHARSHAPE_CHUNK			0x1030	// cached character shape
#define BOLD_CHUNK				0x1040	// No longer used!
#define ITALIC_CHUNK			0x1050
#define UNDERLINE_CHUNK			0x1060
#define FONT_BOLD_CHUNK			0x1070	// No longer used!
#define ALIGNMENT_CHUNK			0x1080
#define SIMPLESPLINE_DATA_CHUNK	0x2000
#define TEXT_VERSION_CHUNK		0x2010	// If not present, it's 1.x
#define BASELINE_ALIGNED_CHUNK	0x2020

IOResult TextObject::Save(ISave *isave) {
	ULONG nb;
	int bytes;

	isave->BeginChunk(FONTNAME_CHUNK);
	bytes = fontName.Length() + 1;
	isave->Write(&bytes,sizeof(int), &nb);
	isave->Write(fontName.data(),bytes, &nb);
	isave->EndChunk();

	isave->BeginChunk(STRING_CHUNK);
	bytes = string.Length() + 1;
	isave->Write(&bytes,sizeof(int), &nb);
	isave->Write(string.data(),bytes, &nb);
	isave->EndChunk();

	for(int i = 0; i < chShapes.Count(); ++i) { 
		isave->BeginChunk(CHARSHAPE_CHUNK);
		chShapes[i]->Save(isave);
		isave->EndChunk();
		}

	if(textFlags & TEXTOBJ_ITALIC) {
		isave->BeginChunk(ITALIC_CHUNK);
		isave->EndChunk();
		}
	if(textFlags & TEXTOBJ_UNDERLINE) {
		isave->BeginChunk(UNDERLINE_CHUNK);
		isave->EndChunk();
		}
 	if(baselineAligned) {
		isave->BeginChunk(BASELINE_ALIGNED_CHUNK);
		isave->EndChunk();
		}

	isave->BeginChunk(ALIGNMENT_CHUNK);
	isave->Write(&textAlignment, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(SIMPLESPLINE_DATA_CHUNK);
	SimpleSpline::Save(isave);
	isave->EndChunk();

	isave->BeginChunk(TEXT_VERSION_CHUNK);
	isave->Write(&textVersion, sizeof(int), &nb);
	isave->EndChunk();

	return IO_OK;
	}

IOResult TextObject::Load(ILoad *iload) {
	ULONG nb;
	IOResult res;
	int bytes;
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,USERPBLOCK));
	textFlags = 0;
	textVersion = 1;	// The default text version number
	baselineAligned = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case FONTNAME_CHUNK:
				res = iload->Read(&bytes,sizeof(int), &nb);
				fontName.Resize(bytes-1);
				res = iload->Read(fontName.data(), bytes,&nb);
				break;
			case STRING_CHUNK:
				res = iload->Read(&bytes,sizeof(int), &nb);
				string.Resize(bytes-1);
				res = iload->Read(string.data(), bytes, &nb);
				break;
			case CHARSHAPE_CHUNK: {
				CharShape *cs = new CharShape;
				chShapes.Append(1, &cs);
				res = cs->Load(iload);
				}
				break;
			case OLD_TEXT_CHUNK:
				iload->SetObsolete();
				break;
			case ITALIC_CHUNK:
				textFlags |= TEXTOBJ_ITALIC;
				break;
			case UNDERLINE_CHUNK:
				textFlags |= TEXTOBJ_UNDERLINE;
				break;
			case ALIGNMENT_CHUNK:
				res = iload->Read(&textAlignment, sizeof(int), &nb);
				break;
			case SIMPLESPLINE_DATA_CHUNK:
				res = SimpleSpline::Load(iload);
				break;
			case TEXT_VERSION_CHUNK:
				res = iload->Read(&textVersion, sizeof(int), &nb);
				break;
			case BASELINE_ALIGNED_CHUNK:
				baselineAligned = TRUE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}

#endif // NO_OBJECT_SHAPES_SPLINES

