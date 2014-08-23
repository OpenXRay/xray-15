/**********************************************************************
 *<
	FILE: stdmtl.cpp

	DESCRIPTION:  default material class

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#include "buildver.h"
#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmtl.h"
#include "gport.h"
#include "hsv.h"
#include "iColorMan.h"

static Shader *GetShader(int s);

#define HITHRESH 0.5f
#define LOTHRESH 0.1f


#if 1
// Quadratic
static inline float Soften(float r) {
	return r*(2.0f-r);
	}
#else
// Cubic
static inline float Soften(float r) {
	return r*r*(3.0f-2.0f*r);
	}
#endif

struct SIllumParams {
	ULONG flags;
	float sh_str, ph_exp, shine, softThresh;
	Color amb,diff,spec;
	Point3 N,V;
	Color diffIllum,specIllum,ambIllum;
	};

// Parameter block indices
#define PB_AMBIENT		0
#define PB_DIFFUSE		1
#define PB_SPECULAR		2
#define PB_SHININESS 	3
#define PB_SHIN_STR		4
#define PB_SELFI		5
#define PB_OPAC			6
#define PB_OPFALL		7
#define PB_FILTER 		8
#define PB_WIRESZ 		9
#define PB_IOR	 		10
#define PB_BOUNCE 		11
#define PB_STATFRIC		12
#define PB_SLIDFRIC		13
#define PB_DIMLEV		14
#define PB_DIMMULT		15
#define PB_SOFTEN		16

//#define OLDFILTER    // scaling instead of additive

#define IDT_MYTIMER 1010
#define DRAGTHRESH 6
#define DITHER_WHEN_INACTIVE // avoids palette conflict probs
#define BUMP_DEF_AMT .30f

#define NCOLBOX 4
static int colID[NCOLBOX] = { IDC_STD_COLOR1, IDC_STD_COLOR2, IDC_STD_COLOR3, IDC_STD_COLOR4 };

// >>>> must become dynamic shader table
extern HINSTANCE hInstance;
static int shadeNameID[] = { IDS_DS_CONSTANT, IDS_DS_PHONG, IDS_DS_BLINN, IDS_DS_METAL };

static IPoint2 GetPoint(LPARAM lp) {
	IPoint2 p;
	MAKEPOINT(lp,p);
	return p;
	}
#define SET_HSV 1
#define SET_RGB 2
#define SET_BOTH (SET_HSV|SET_RGB)

#ifdef USE_STDMTL2_AS_STDMTL
static Class_ID stdmtlClassID(DMTL2_CLASS_ID,0);
#else
static Class_ID stdmtlClassID(DMTL_CLASS_ID,0);
#endif

int numStdMtls = 0;
class StdMtlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new StdMtl(loading); }
	const TCHAR *	ClassName() { return GetString(IDS_DS_STANDARD_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return MATERIAL_CLASS_ID; }
	Class_ID 		ClassID() { return stdmtlClassID; }
	const TCHAR* 	Category() { return _T("");  }
	};

static StdMtlClassDesc stdmtlCD;

ClassDesc* GetStdMtlDesc() { return &stdmtlCD;  }

static HIMAGELIST hLockButtons = NULL;

static BOOL IsButtonChecked(HWND hWnd,int id)
	{
	ICustButton *iBut;
	BOOL res;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	res = iBut->IsChecked();
	ReleaseICustButton(iBut);
	return res;
	}

static void CheckButton(HWND hWnd,int id, BOOL check) {
	ICustButton *iBut;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	iBut->SetCheck(check);
	ReleaseICustButton(iBut);
	}

static void SetupLockButton(HWND hWnd,int id, BOOL check)
	{
	ICustButton *iBut;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	iBut->SetImage(hLockButtons,0,1,0,1,16,15);
	iBut->SetType(CBT_CHECK);
	ReleaseICustButton(iBut);
	}

static void SetupPadLockButton(HWND hWnd,int id, BOOL check) {
	ICustButton *iBut;
	iBut = GetICustButton(GetDlgItem(hWnd,id));
	iBut->SetImage(hLockButtons,2,2,2,2,16,15);
	iBut->SetType(CBT_CHECK);
	ReleaseICustButton(iBut);
	}

static void LoadStdMtlResources()
	{
	static BOOL loaded=FALSE;
	if (loaded) return;
	loaded = TRUE;	
	HBITMAP hBitmap, hMask;

	HINSTANCE hInst = hInstance;
	hLockButtons = ImageList_Create(16, 15, TRUE, 2, 0);
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DMTL_BUTTONS));
	hMask   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DMTL_MASKBUTTONS));
	ImageList_Add(hLockButtons,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	}


static inline float PcToFrac(int pc) {
	return (float)pc/100.0f;	
	}

static inline int FracToPc(float f) {
	if (f<0.0) return (int)(100.0f*f - .5f);
	else return (int) (100.0f*f + .5f);
	}


static LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

#define NMBUTS 8
static int texMButtons[NMBUTS] = {
	IDC_MAPON_AM,	IDC_MAPON_DI,	IDC_MAPON_SP,	IDC_MAPON_FI,
	IDC_MAPON_SH,	IDC_MAPON_SS,	IDC_MAPON_SI,	IDC_MAPON_TR, 
	};
		
// This array gives the text map number for given MButton number								
static int texmapFromMBut[NMBUTS] = { 0, 1, 2, 7, 3, 4, 5, 6 };

// This array gives the MButton number for given Texmap number								
static int mButFromTexmap[NTEXMAPS] = { 0, 1, 2, 4, 5, 6, 7, 3, -1, -1, -1, -1 };

// >>>> these maps either need to be dynamic or allocated w/ a hard maximum.

static int texAmtID[NTEXMAPS] = {
	IDC_AMT_AM,	IDC_AMT_DI,	IDC_AMT_SP,	IDC_AMT_SH, IDC_AMT_SS,
	IDC_AMT_SI,	IDC_AMT_OP, IDC_AMT_FI,	IDC_AMT_BU,	IDC_AMT_RL,	IDC_AMT_RR, IDC_AMT_DP
	};	

static int texOnID[NTEXMAPS] = 	{
	IDC_USEMAP_AM, IDC_USEMAP_DI, IDC_USEMAP_SP, IDC_USEMAP_SH, IDC_USEMAP_SS,
	IDC_USEMAP_SI, IDC_USEMAP_OP, IDC_USEMAP_FI, IDC_USEMAP_BU, IDC_USEMAP_RL, IDC_USEMAP_RR, IDC_USEMAP_DP
	};	

static int texSpinID[NTEXMAPS] = {
	IDC_SPIN_AM, IDC_SPIN_DI, IDC_SPIN_SP,	IDC_SPIN_SH, IDC_SPIN_SS,
	IDC_SPIN_SI, IDC_SPIN_OP, IDC_SPIN_FI, IDC_SPIN_BU,	IDC_SPIN_RL, IDC_SPIN_RR, IDC_SPIN_DP
	};	

static int texMapID[NTEXMAPS] =  {
	IDC_MAP_AM, IDC_MAP_DI, IDC_MAP_SP,	IDC_MAP_SH, IDC_MAP_SS,
	IDC_MAP_SI, IDC_MAP_OP, IDC_MAP_FI, IDC_MAP_BU,	IDC_MAP_RL, IDC_MAP_RR, IDC_MAP_DP
	};	

static int texNameID[NTEXMAPS] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, IDS_DS_SHININESS, IDS_DS_SHIN_STR,
	IDS_DS_SELFI,	IDS_DS_TRANS, IDS_DS_FILTER, IDS_DS_BU, IDS_DS_RL, IDS_DS_RR, IDS_DS_DP
	};	

// which edit control enum
enum EditControl {Hc, Sc, Vc, Rc, Gc, Bc};


//-----------------------------------------------------------------------------
//  Texmaps
//-----------------------------------------------------------------------------
/*******
//#define TEXMAPS_CLASS_ID 0x001200

//#ifdef USE_STDMTL2_AS_STDMTL
//#define TEXMAPS_CLASS_ID 0x001200+33
//#else
#define TEXMAPS_CLASS_ID 0x001200
//#endif

static Class_ID texmapsClassID(TEXMAPS_CLASS_ID,0);

class OldTexmapsClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { 	return new Texmaps(NULL); }
	const TCHAR *	ClassName() { return GetString(IDS_DS_CLASSTEXMAPS); }
	SClass_ID		SuperClassID() { return REF_MAKER_CLASS_ID; }
	Class_ID 		ClassID() { return texmapsClassID; }
	const TCHAR* 	Category() { return _T("");  }
	};


class TexmapsClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { 	return new Texmaps(NULL); }
	const TCHAR *	ClassName() { return GetString(IDS_DS_CLASSTEXMAPS); }
//	SClass_ID		SuperClassID() { return REF_MAKER_CLASS_ID; }
	SClass_ID		SuperClassID() { return TEXMAP_CONTAINER_CLASS_ID; }
	Class_ID 		ClassID() { return texmapsClassID; }
	const TCHAR* 	Category() { return _T("");  }
	};

TexmapSlot::TexmapSlot() { 
	amount = 1.0f; 
	map = NULL; 
	mapOn = FALSE; 
	amtCtrl=NULL; 
	}

void TexmapSlot::Update(TimeValue t,Interval& ivalid) {
	if (IsActive()) 
		map->Update(t,ivalid);			
	if (amtCtrl) {
		amtCtrl->GetValue(t,&amount,ivalid);	
		}
	}

float TexmapSlot::GetAmount(TimeValue t) {
	Interval v;
	float f;
	if (amtCtrl) {
		amtCtrl->GetValue(t,&f,v);	
		return f;
		}
	else return amount;
	} 

Texmaps::Texmaps() {
	loadingOld = FALSE;
	client = NULL;
	}

					
Texmaps::Texmaps(MtlBase *mb) {
	loadingOld = FALSE;
	client = mb;
	}


static TexmapsClassDesc texmapsCD;

ClassDesc* GetTexmapsDesc() { return &texmapsCD;  }

static OldTexmapsClassDesc oldtexmapsCD;

ClassDesc* GetOldTexmapsDesc() { return &oldtexmapsCD;  }

Class_ID Texmaps::ClassID() { return texmapsClassID; }

int Texmaps::NumSubs() { return NTEXMAPS*2; }  

Animatable* Texmaps::SubAnim(int i) {
	if (i&1)
		return txmap[i/2].map;
	else 
		return txmap[i/2].amtCtrl;
	}

TSTR Texmaps::SubAnimName(int i) {
	if (i&1)
		return client->GetSubTexmapTVName(i/2);
	else  {
		TSTR nm;
		nm = GetString(texNameID[i/2]);
		nm += TSTR(GetString(IDS_DS_AMOUNT));
		return nm;
		}
	}

RefTargetHandle Texmaps::GetReference(int i) {
	if (i&1)
		return txmap[i/2].map;
	else 
		return txmap[i/2].amtCtrl;
	}

void Texmaps::SetReference(int i, RefTargetHandle rtarg) {
	if (loadingOld)
		txmap[i].map = (Texmap*)rtarg;
	else {
		if (i&1)
			txmap[i/2].map = (Texmap*)rtarg;
		else 
			txmap[i/2].amtCtrl = (Control*)rtarg;
		}
	}

void Texmaps::DeleteThis() { delete this;}

RefResult Texmaps::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = defaultDim; 
			break;
			}
		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

void Texmaps::RescaleWorldUnits(float f) {
	if (TestAFlag(A_WORK1))
		return;
	SetAFlag(A_WORK1);
	// This code will be replaced in particular implementations
	for (int i=0; i<NumRefs(); i++) {
		if ( (i&1) ==0) 
			continue;  // skip the amount controllers
		ReferenceMaker *srm = GetReference(i);
		if (srm) {
			srm->RescaleWorldUnits(f);
			}
		}
		
	}

RefTargetHandle Texmaps::Clone(RemapDir &remap) {
	Texmaps *tm = new Texmaps(NULL);
	for (int i = 0; i<NTEXMAPS; i++) {
		tm->txmap[i].amount = txmap[i].amount;
		tm->txmap[i].mapOn = txmap[i].mapOn;
		tm->txmap[i].map = NULL;
		if (txmap[i].amtCtrl) 
			tm->ReplaceReference(2*i,remap.CloneRef(txmap[i].amtCtrl));
		if (txmap[i].map) 
			tm->ReplaceReference(2*i+1,remap.CloneRef(txmap[i].map));
		}
		BaseClone(this, tm, remap);
	return tm;
	}

#define TEX_OLD_ONOFF_CHUNK 0x5002
#define TEX_ONOFF_CHUNK 0x5003
#define TEX_AMT0 0x5100
#define TEX_AMT1 0x5101
#define TEX_AMT2 0x5102
#define TEX_AMT3 0x5103
#define TEX_AMT4 0x5104
#define TEX_AMT5 0x5105
#define TEX_AMT6 0x5106
#define TEX_AMT7 0x5107
#define TEX_AMT8 0x5108
#define TEX_AMT9 0x5109
#define TEX_AMTA 0x510A

IOResult Texmaps::Save(ISave *isave) { 
	isave->BeginChunk(TEX_ONOFF_CHUNK);
	ULONG nb,f=0;
	for ( int i=0; i<NTEXMAPS; i++) 
		if (txmap[i].mapOn) f|= (1<<i);
	isave->Write(&f,sizeof(f),&nb);			
	isave->EndChunk();

	for ( i=0; i<NTEXMAPS; i++) {
		if (txmap[i].amount!=1.0f) {
			isave->BeginChunk(TEX_AMT0+i);
			isave->Write(&txmap[i].amount,sizeof(float),&nb);			
			isave->EndChunk();
			}
		}
	return IO_OK;
	}

class TexmapsPostLoad : public PostLoadCallback {
	public:
		Texmaps *tm;
		TexmapsPostLoad(Texmaps *b) {tm=b;}
		void proc(ILoad *iload) {  tm->loadingOld = FALSE; delete this; } 
	};

	
IOResult Texmaps::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case TEX_OLD_ONOFF_CHUNK:
				iload->SetObsolete();
				iload->RegisterPostLoadCallback(new TexmapsPostLoad(this));
				loadingOld = TRUE;
			case TEX_ONOFF_CHUNK:
				{
				ULONG f;
				res = iload->Read(&f,sizeof(f), &nb);
				for (int i=0; i<NTEXMAPS; i++) 
				    txmap[i].mapOn = (f&(1<<i))?1:0;
				}
				break;
			case TEX_AMT0: case TEX_AMT1:
			case TEX_AMT2:	case TEX_AMT3:
			case TEX_AMT4:	case TEX_AMT5:
			case TEX_AMT6:	case TEX_AMT7:
			case TEX_AMT8:	case TEX_AMT9:
			case TEX_AMTA:
				res = iload->Read(&txmap[id-TEX_AMT0].amount,sizeof(float), &nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
                                                      
	}
//-----------------------------------------------------------------------------
**************/
//-----------------------------------------------------------------------------
//class StdMtlDlg;


//-----------------------------------------------------------------------------
//  StdMtlDlg
//-----------------------------------------------------------------------------
class StdMtlDlg: public ParamDlg {
	public:
	HWND hwmedit;	 // window handle of the materials editor dialog
	IMtlParams *ip;
	StdMtl *theMtl;	 // current mtl being edited.
	HWND hPanelBasic; // Rollup panel
	HWND hPanelExtra; // Rollup panel
	HWND hPanelTexmap; // Rollup panel
	HWND hPanelDynam; // Rollup panel
	HWND hwHilite;  // the hilite window
	HPALETTE hOldPal;
	TimeValue curTime;
	int instCopy;
	int iStart;
	Rect texRect;
	int isActive;
	IPoint2 pDown,pDrag;
	IPoint2 pLast;
	BOOL dragging,dragAbort;
	Rect colBox[NCOLBOX];
	IColorSwatch *cs[NCOLBOX];
	BOOL animPalOn;
	BOOL valid;
	int editingColor; //0= amb,1= diff,2= spec, 3 = filterCol
	DWORD curRGB;
	int H,S,V;
	ISpinnerControl *hSpin,*sSpin,*vSpin;
	ISpinnerControl *rSpin,*gSpin,*bSpin;
	ISpinnerControl *softSpin;
	ISpinnerControl *shSpin, *ssSpin, *siSpin, *trSpin, *tfSpin, *wireSizeSpin, *iorSpin;
	ISpinnerControl *dimSpin, *dimMultSpin;
	ISpinnerControl* texAmtSpin[NTEXMAPS];
	ISpinnerControl *iBounce, *iStatFrict, *iSlidFrict;
	ICustButton* texMBut[NMBUTS];
	ICustButton *iBut[NTEXMAPS];
	TexDADMgr dadMgr;

	StdMtlDlg(HWND hwMtlEdit, IMtlParams *imp, StdMtl *m); 
	~StdMtlDlg();
	void BuildDialog();  // put up the dialog
	void Invalidate();
	BOOL BasicPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	BOOL ExtraPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	BOOL TexmapPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	BOOL DynamPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	void ActivateDlg(BOOL onOff);
	void LoadDialog(BOOL draw);  // stuff params into dialog
	void SetEditColor(int i);    //0=amb, 1=diff, 2=spec, 3=filterColor
	void SetRGB(DWORD rgb);
	void UpdateColFromSpin(HWND hwndDlg,int indx, ISpinnerControl *spin );
	void UpdateColControls( int  which);
	void UpdateColSwatches();
	void AnimPalette(HDC hdc);
	Color MtlColor(int i);
	TCHAR *ColorName(int i);
	void SetMtlColor(int i, Color c);
	void UpdateMBut(int i);
	void ReloadDialog();
	void EnablePhongStuff();
	void DrawHilite(HDC hdc, Rect& rect);
	void GenDrawHilite(HDC hdc, Rect& rect, SShader &sh);
	void UpdateHilite(); 
	void SetLockAD(BOOL lock);
	void UpdateLockADTex();
	void SetLockDS(BOOL lock);
 	void SetLockADTex(BOOL lock);
	void UpdateMtlDisplay();
	void UpdateTexmapDisplay(int i);
	void UpdateTexmaps();
	void FixFilterButtons();
	void UpdateControlFor(int np);
	void UpdateColorStuff();
	BOOL KeyAtCurTime(int id) { return theMtl->KeyAtTime(id,curTime); } 
	// methods inherited from ParamDLg:
	Class_ID ClassID() {return stdmtlClassID;  }
	void SetThing(ReferenceTarget *m);
	ReferenceTarget* GetThing() { return (ReferenceTarget*)theMtl; }
	void DeleteThis() { delete this;  }	
	void SetTime(TimeValue t);
	int FindSubTexFromHWND(HWND hw);
	};

static int PB_ID[4] = { PB_AMBIENT, PB_DIFFUSE, PB_SPECULAR, PB_FILTER };

int StdMtlDlg::FindSubTexFromHWND(HWND hw) {
	for (int i=0; i<NTEXMAPS; i++) {
		if (hw == iBut[i]->GetHwnd()) 
			return i;
		}	
	for (i=0; i<NMBUTS; i++) {
		if (hw == texMBut[i]->GetHwnd()) 
			return texmapFromMBut[i];
		}	
	return -1;
	}


//-------------------------------------------------------------------


StdMtlDlg::StdMtlDlg(HWND hwMtlEdit, IMtlParams *imp, StdMtl *m) {
	dadMgr.Init(this);
	hwmedit = hwMtlEdit;
	hPanelBasic = hPanelExtra = hPanelTexmap = hPanelDynam = NULL;
	hOldPal = NULL;
	theMtl = m; 
	ip = imp;
	valid = FALSE;
	editingColor = 1; // default to diffuse
	animPalOn = 1;
	isActive = 0;
	instCopy = FALSE;
	for (int i=0; i<NTEXMAPS; i++) iBut[i] = NULL;
	}

	

void StdMtlDlg::Invalidate()
	{
	valid = FALSE;
	InvalidateRect(hPanelBasic,NULL,0);
	InvalidateRect(hPanelExtra,NULL,0);
	InvalidateRect(hPanelTexmap,NULL,0);
#ifndef DESIGN_VER
	InvalidateRect(hPanelDynam,NULL,0);
#endif
	}

void StdMtlDlg::AnimPalette(HDC hdc) {
#if 0
	if (animPalOn) {
		GetGPort()->PlugPalette(hdc);
		GetGPort()->AnimPalette(hdc);
		}
#endif
	}		  

static TCHAR blnk[] = _T(" ");
static TCHAR lcm[] = _T("m");
static TCHAR ucm[] = _T("M");

void StdMtlDlg::UpdateMBut(int j) {
	int i = mButFromTexmap[j];
	if (i<0) return;
	TCHAR* s = (*theMtl->maps)[j].map?((*theMtl->maps)[j].mapOn?ucm:lcm):blnk; 
	texMBut[i]->SetText(s);
	Texmap *t = (*theMtl->maps)[j].map;
	TSTR nm;
	if (t) nm = t->GetFullName();
	else  nm = GetString(IDS_DS_NONE);
	texMBut[i]->SetTooltip(TRUE,nm);
	}

void StdMtlDlg::ReloadDialog() {
	Interval v;
	theMtl->Update(ip->GetTime(),v);
	LoadDialog(FALSE);
	}

void StdMtlDlg::FixFilterButtons() {
	BOOL b = (theMtl->flags&STDMTL_FILT_TRANSP)?1:0;
	EnableWindow(GetDlgItem(hPanelBasic,IDC_FILT), b);
	EnableWindow(GetDlgItem(hPanelBasic,IDC_MAPON_FI), b);
	}

void StdMtlDlg::SetTime(TimeValue t) {
	if (t!=curTime) {
		curTime = t;
		Interval v;
		theMtl->Update(ip->GetTime(),v);
		LoadDialog(TRUE);
		}
	}


// >>>> SelfIllum Color ?????
Color StdMtlDlg::MtlColor(int i) {
	switch(i) {
		case 0: return theMtl->GetAmbient(); 
		case 1: return theMtl->GetDiffuse();
		case 2: return theMtl->GetSpecular();
		case 3: return theMtl->GetFilter();
		default: return Color(0,0,0);
		}
	}

TCHAR *StdMtlDlg::ColorName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_DS_AMBIENT);	 
		case 1:  return GetString(IDS_DS_DIFFUSE);	 
		case 2:  return GetString(IDS_DS_SPECULAR);	 
		default:  return GetString(IDS_DS_FILTER);	 
		}
	}

void StdMtlDlg::SetMtlColor(int i, Color c) {
	switch(i) {
		case 0: 
			theMtl->SetAmbient(c,curTime); 
			if (theMtl->GetFlag(STDMTL_LOCK_AD)) {
				theMtl->SetDiffuse(c,curTime);
				cs[1]->SetColor(MtlColor(1));
				if (theMtl->GetFlag(STDMTL_LOCK_DS)) {
					theMtl->SetSpecular(c,curTime);
					cs[2]->SetColor(MtlColor(2));
					}
				}
			break;
		case 1: 
			theMtl->SetDiffuse(c,curTime); 
			if (theMtl->GetFlag(STDMTL_LOCK_AD)) {
				theMtl->SetAmbient(c,curTime);
				cs[0]->SetColor(MtlColor(0));
				}
			if (theMtl->GetFlag(STDMTL_LOCK_DS)) {
				theMtl->SetSpecular(c,curTime);
				cs[2]->SetColor(MtlColor(2));
				}
			break;
		case 2: 
			theMtl->SetSpecular(c,curTime); 
			if (theMtl->GetFlag(STDMTL_LOCK_DS)) {
				theMtl->SetDiffuse(c,curTime);
				cs[1]->SetColor(MtlColor(1));
				if (theMtl->GetFlag(STDMTL_LOCK_AD)) {
					theMtl->SetAmbient(c,curTime);
					cs[0]->SetColor(MtlColor(0));
					}
				}
			break;
		case 3: 
			theMtl->SetFilter(c,curTime); 
			break;
		}

	}

StdMtlDlg::~StdMtlDlg() {
	int i;
	for (i=0; i<NCOLBOX; i++) {
		ReleaseIColorSwatch(cs[i]);
		}
	if (hPanelBasic) {
		HDC hdc = GetDC(hPanelBasic);
		GetGPort()->RestorePalette(hdc, hOldPal);
		ReleaseDC(hPanelBasic,hdc);
		}

	theMtl->SetFlag(STDMTL_ROLLUP1_OPEN,ip->IsRollupPanelOpen(hPanelBasic));
	theMtl->SetFlag(STDMTL_ROLLUP2_OPEN,ip->IsRollupPanelOpen(hPanelExtra));
	theMtl->SetFlag(STDMTL_ROLLUP3_OPEN,ip->IsRollupPanelOpen(hPanelTexmap));
#ifndef DESIGN_VER
	theMtl->SetFlag(STDMTL_ROLLUP4_OPEN,ip->IsRollupPanelOpen(hPanelDynam));
#endif
	theMtl->rollScroll = ip->GetRollupScrollPos();

	theMtl->SetParamDlg(NULL);
	for (i=0; i<NTEXMAPS; i++) 
		ReleaseISpinner(texAmtSpin[i]);

	for (i=0; i<NMBUTS; i++)
		ReleaseICustButton(texMBut[i]);

	for (i=0; i<NTEXMAPS; i++) {
		ReleaseICustButton(iBut[i]);
		iBut[i] = NULL; 
		}
	
	ReleaseISpinner(hSpin);
	ReleaseISpinner(sSpin);
	ReleaseISpinner(vSpin);
	ReleaseISpinner(rSpin);
	ReleaseISpinner(gSpin);
	ReleaseISpinner(bSpin);
	ReleaseISpinner(softSpin);
	ReleaseISpinner(shSpin);
	ReleaseISpinner(ssSpin);
	ReleaseISpinner(siSpin);
	ReleaseISpinner(trSpin);
	ReleaseISpinner(tfSpin);
	ReleaseISpinner(wireSizeSpin);
	ReleaseISpinner(iorSpin);
	ReleaseISpinner(dimSpin);
	ReleaseISpinner(dimMultSpin);
#ifndef DESIGN_VER
	ReleaseISpinner(iBounce);
	ReleaseISpinner(iStatFrict);
	ReleaseISpinner(iSlidFrict);
#endif

	SetWindowLongPtr(hPanelBasic, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hPanelExtra, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hPanelTexmap, GWLP_USERDATA, NULL);
#ifndef DESIGN_VER
	SetWindowLongPtr(hPanelDynam, GWLP_USERDATA, NULL);
#endif
	hPanelBasic = hPanelExtra = hPanelTexmap = hPanelDynam = NULL;
	}


void StdMtlDlg::UpdateColorStuff() {
	curRGB = MtlColor(editingColor).toRGB();
	RGBtoHSV(curRGB, &H, &S, &V);
	UpdateColControls(SET_HSV|SET_RGB);
	}
		
void StdMtlDlg::UpdateControlFor(int np) {
	Point3 p;
	Interval v;
	TimeValue t = ip->GetTime();
	theMtl->Update(t, v);
	switch(np) {
		case PB_AMBIENT:
			UpdateColSwatches();
		 	if (editingColor==0)
				UpdateColorStuff();
			cs[0]->SetKeyBrackets(theMtl->KeyAtTime(PB_AMBIENT,curTime));
			break;
		case PB_DIFFUSE:
			UpdateColSwatches();
		 	if (editingColor==1)
				UpdateColorStuff();
			cs[1]->SetKeyBrackets(theMtl->KeyAtTime(PB_DIFFUSE,curTime));
			break;
		case PB_SPECULAR:
			UpdateColSwatches();
		 	if (editingColor==2)
				UpdateColorStuff();
			cs[2]->SetKeyBrackets(theMtl->KeyAtTime(PB_SPECULAR,curTime));
			break;
		case PB_SHININESS:
			shSpin->SetValue(FracToPc(theMtl->GetShininess()),FALSE);
			shSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_SHININESS,curTime));
			break;
		case PB_SHIN_STR:
			ssSpin->SetValue(FracToPc(theMtl->GetShinStr()),FALSE);
			ssSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_SHIN_STR,curTime));
			break;
		case PB_SOFTEN:
			softSpin->SetValue(theMtl->GetSoftenLevel(curTime),FALSE);
			softSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_SOFTEN,curTime));
			break;

		// >>>>
		case PB_SELFI:
			siSpin->SetValue(FracToPc(theMtl->GetSelfIll()),FALSE);
			siSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_SELFI,curTime));
			break;
		case PB_OPAC:
			trSpin->SetValue(FracToPc(theMtl->GetOpacity()),FALSE);
			trSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_OPAC,curTime));
			break;
		case PB_OPFALL:
			tfSpin->SetValue(FracToPc(theMtl->GetOpacFalloff()),FALSE);
			tfSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_OPFALL,curTime));
			break;
		case PB_FILTER:
			UpdateColSwatches();
		 	if (editingColor==3)
				UpdateColorStuff();
			cs[3]->SetKeyBrackets(theMtl->KeyAtTime(PB_FILTER,curTime));
			break;
		case PB_WIRESZ:
			wireSizeSpin->SetValue( theMtl->WireSize(),FALSE);
			wireSizeSpin->SetKeyBrackets(theMtl->KeyAtTime(PB_WIRESZ,curTime));
			break;
		case PB_IOR:
			iorSpin->SetValue( theMtl->GetIOR(),FALSE);
			iorSpin->SetKeyBrackets(KeyAtCurTime(PB_IOR));
			break;
#ifndef DESIGN_VER
		case PB_BOUNCE:
			iBounce->SetValue(theMtl->GetDynamicsProperty(curTime,0,DYN_BOUNCE),FALSE); 
			iBounce->SetKeyBrackets(KeyAtCurTime(PB_BOUNCE));
			break;
		case PB_STATFRIC:
			iStatFrict->SetValue(theMtl->GetDynamicsProperty(curTime,0,DYN_STATIC_FRICTION),FALSE); 
			iStatFrict->SetKeyBrackets(KeyAtCurTime(PB_STATFRIC));
			break;
		case PB_SLIDFRIC:
			iSlidFrict->SetValue(theMtl->GetDynamicsProperty(curTime,0,DYN_SLIDING_FRICTION),FALSE); 
			iSlidFrict->SetKeyBrackets(KeyAtCurTime(PB_SLIDFRIC));
			break;
#endif
		case PB_DIMLEV:
			dimSpin->SetValue( theMtl->GetDimIntens(curTime),FALSE);
			dimSpin->SetKeyBrackets(KeyAtCurTime(PB_DIMLEV));
			break;
		case PB_DIMMULT:
			dimMultSpin->SetValue( theMtl->GetDimMult(curTime),FALSE);
			dimMultSpin->SetKeyBrackets(KeyAtCurTime(PB_DIMMULT));
			break;
		}
	}


static LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = GetWindowLongPtr(hwnd,GWL_ID);
	HWND hwpar = GetParent(hwnd);
	StdMtlDlg *theDlg = (StdMtlDlg *)GetWindowLongPtr(hwpar, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;
    switch (msg)    {
		case WM_COMMAND: 	
			break;
		case WM_MOUSEMOVE: 	
			break;
		case WM_LBUTTONUP: 
			break;
		case WM_PAINT: 	
			{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				theDlg->DrawHilite(hdc, rect);
				}
			EndPaint( hwnd, &ps );
			}													
			break;
		case WM_CREATE:
		case WM_DESTROY: 
			break;
    	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
	} 

int ColIDToIndex(int id) {
	switch (id) {
		case IDC_STD_COLOR1: return 0;
		case IDC_STD_COLOR2: return 1;
		case IDC_STD_COLOR3: return 2;
		case IDC_STD_COLOR4: return 3;
		default: return 0;
		}
	}


static int ShadeFromListID( int i) {
	switch (i) {
		case 0: return SHADE_CONST;
		case 1: return SHADE_PHONG;
		case 2: return SHADE_BLINN;
		case 3: return SHADE_METAL;
		default: return SHADE_PHONG;
		}
	}

static int ListIDFromShade( int i) {
	switch (i) {
		case SHADE_CONST: return 0;
		case SHADE_PHONG: return 1;
		case SHADE_BLINN: return 2;
		case SHADE_METAL: return 3;
		default: return 1;
		}
	}

BOOL StdMtlDlg::BasicPanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:
			{
			int i;
			
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

			for (i=0; i<NCOLBOX; i++) {
   				cs[i] = GetIColorSwatch(GetDlgItem(hwndDlg, colID[i]),
   					MtlColor(i), ColorName(i));
				}

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)HiliteWndProc);

			HWND hwndShading = GetDlgItem(hwndDlg, IDC_SHADING);
			for (i=0; i<NSHADES; i++)
				SendMessage(hwndShading, CB_ADDSTRING, 0, (LPARAM)GetString(shadeNameID[i]));

			hSpin = SetupIntSpinner(hwndDlg, IDC_H_SPIN, IDC_H_EDIT, 0,255, 0);
			sSpin = SetupIntSpinner(hwndDlg, IDC_S_SPIN, IDC_S_EDIT, 0,255, 0);
			vSpin = SetupIntSpinner(hwndDlg, IDC_V_SPIN, IDC_V_EDIT, 0,255, 0);

			rSpin = SetupIntSpinner(hwndDlg, IDC_R_SPIN, IDC_R_EDIT, 0,255, 0);
			gSpin = SetupIntSpinner(hwndDlg, IDC_G_SPIN, IDC_G_EDIT, 0,255, 0);
			bSpin = SetupIntSpinner(hwndDlg, IDC_B_SPIN, IDC_B_EDIT, 0,255, 0);

			shSpin = SetupIntSpinner(hwndDlg, IDC_SH_SPIN, IDC_SH_EDIT, 0,100, 0);
			ssSpin = SetupIntSpinner(hwndDlg, IDC_SS_SPIN, IDC_SS_EDIT, 0,999, 0);
			siSpin = SetupIntSpinner(hwndDlg, IDC_SI_SPIN, IDC_SI_EDIT, 0,100, 0);
			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);

			for (int j=0; j<NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtons[j]));
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
				}

			softSpin = SetupFloatSpinner(hwndDlg, IDC_SOFT_SPIN, IDC_SOFT_EDIT, 0.0f,1.0f,0.0f,.01f);

			SetupLockButton(hwndDlg,IDC_LOCK_AD,FALSE);
			SetupLockButton(hwndDlg,IDC_LOCK_DS,FALSE);
			SetupPadLockButton(hwndDlg,IDC_LOCK_ADTEX, TRUE);
			return TRUE;
			}
			break;
		case WM_COMMAND: 
				{ 
				for ( int i=0; i<NMBUTS; i++) {
					if (id == texMButtons[i]) {
						PostMessage(hwmedit,WM_TEXMAP_BUTTON, texmapFromMBut[i],(LPARAM)theMtl);
						goto exit;
						}
					}
				}
		    switch (id) {
				case IDC_SHADING: {
					if (code==CBN_SELCHANGE) {
						int newshade = ShadeFromListID(SendMessage( GetDlgItem(hwndDlg,IDC_SHADING), CB_GETCURSEL, 0, 0 ));
						int oldshade = theMtl->GetShaderId();
						if ( oldshade != newshade ) {
							theMtl->SetShaderId( newshade );
							softSpin->Enable(theMtl->shading!=SHADE_METAL);
							UpdateMtlDisplay();
							}
						}
					break;
					}

				case IDC_AMB:  
				case IDC_DIFF: 
				case IDC_SPEC: 
				case IDC_FILT: 
					// >>>> SELFI
					SetEditColor(id-IDC_AMB);
					break;

				case IDC_WIRE:
					theMtl->SetFlag(STDMTL_WIRE, GetCheckBox(hwndDlg, IDC_WIRE));			
					theMtl->NotifyChanged();
					UpdateMtlDisplay();
					break;

				case IDC_FACE_MAP:
					theMtl->SetFlag(STDMTL_FACEMAP, GetCheckBox(hwndDlg, IDC_FACE_MAP));			
					UpdateMtlDisplay();
					theMtl->NotifyChanged();
					break;

				case IDC_2SIDE:
					theMtl->SetFlag(STDMTL_2SIDE, GetCheckBox(hwndDlg, IDC_2SIDE));			
					theMtl->NotifyChanged();
					UpdateMtlDisplay();
					break;

				case IDC_SUPER_SAMP:
					theMtl->SetFlag(STDMTL_SSAMP, GetCheckBox(hwndDlg, IDC_SUPER_SAMP));			
					break;

				case IDC_SOFTEN:
					theMtl->SetFlag(STDMTL_SOFTEN, GetCheckBox(hwndDlg, id));			
					theMtl->NotifyChanged();
					UpdateMtlDisplay();
					break;

//				case IDC_OLDSPEC:
//					theMtl->SetFlag(STDMTL_OLDSPEC, GetCheckBox(hwndDlg, id));			
//					theMtl->NotifyChanged();
//					UpdateMtlDisplay();
//					break;

				case IDC_LOCK_AD:
					SetLockAD(IsButtonChecked(hwndDlg, IDC_LOCK_AD));
					break;
				case IDC_LOCK_DS:
					SetLockDS(IsButtonChecked(hwndDlg, IDC_LOCK_DS));
					break;
				case IDC_LOCK_ADTEX:
					SetLockADTex(IsButtonChecked(hwndDlg, IDC_LOCK_ADTEX));
					break;
				}
			break;
		case CC_COLOR_SEL:
			{
			int id = LOWORD(wParam);
			SetEditColor(ColIDToIndex(id));
			}			
			break;
		case CC_COLOR_DROP:
			{
			int id = LOWORD(wParam);
			SetEditColor(ColIDToIndex(id));
			}			
			break;
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			break;
		case CC_COLOR_CHANGE:
			{			
			int id = LOWORD(wParam);
			int buttonUp = HIWORD(wParam); 
			int n = ColIDToIndex(id);
			if (buttonUp) theHold.Begin();
			SetRGB(cs[n]->GetColor());
			SetMtlColor(n,Color(curRGB));
			UpdateColControls(SET_BOTH);
			theMtl->NotifyChanged();
			if (buttonUp) {
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
				UpdateMtlDisplay();				
				}
			}			
			break;
		case WM_PAINT: 
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
		case WM_CLOSE:
			break;       
		case WM_DESTROY: 
			break;
		case CC_SPINNER_CHANGE: 
			if (!theHold.Holding()) theHold.Begin();
			switch (id) {
				case IDC_R_SPIN: UpdateColFromSpin(hwndDlg, Rc, rSpin); break;
				case IDC_G_SPIN: UpdateColFromSpin(hwndDlg, Gc, gSpin); break;
				case IDC_B_SPIN: UpdateColFromSpin(hwndDlg, Bc, bSpin); break;
				case IDC_H_SPIN: UpdateColFromSpin(hwndDlg, Hc, hSpin); break;
				case IDC_S_SPIN: UpdateColFromSpin(hwndDlg, Sc, sSpin); break;
				case IDC_V_SPIN: UpdateColFromSpin(hwndDlg, Vc, vSpin); break;
				case IDC_SH_SPIN: 
					theMtl->SetShininess(PcToFrac(shSpin->GetIVal()), curTime); 
					UpdateHilite();
					break;
				case IDC_SS_SPIN: 
					theMtl->SetShinStr(PcToFrac(ssSpin->GetIVal()),curTime); 
					UpdateHilite();
					break;

					// >>>>
				case IDC_SI_SPIN: 
					theMtl->SetSelfIllum(PcToFrac(siSpin->GetIVal()),curTime); 
					//UpdateColControls(0);  // makes redraw smoother (black magic)
					break;

				case IDC_TR_SPIN: 
					theMtl->SetOpacity(PcToFrac(trSpin->GetIVal()),curTime); 
					//UpdateColControls(0);  // makes redraw smoother (black magic)
					break;
				case IDC_SOFT_SPIN: 
					theMtl->SetSoftenLevel(softSpin->GetFVal(),curTime); 
					break;
				}
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else 
				theHold.Cancel();
			UpdateMtlDisplay();
			break;

    	}
	exit:
	return FALSE;
	}

BOOL StdMtlDlg::ExtraPanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:
			wireSizeSpin = SetupFloatSpinner(hwndDlg, IDC_WIRE_SPIN, IDC_WIRE_EDIT, 0.0f, 10000.0f, 1.0f);
			wireSizeSpin->SetAutoScale();
			tfSpin = SetupIntSpinner(hwndDlg, IDC_TF_SPIN, IDC_TF_EDIT, 0,100, 0);
			iorSpin = SetupFloatSpinner(hwndDlg, IDC_IOR_SPIN, IDC_IOR_EDIT, 0.0f,10.0f,1.5f,.01f);
			dimSpin = SetupFloatSpinner(hwndDlg, IDC_DIM_AMTSPIN, IDC_DIM_AMT, 0.0f,1.0f,1.0f,.01f);
			dimMultSpin = SetupFloatSpinner(hwndDlg, IDC_DIM_MULTSPIN, IDC_DIM_MULT, 0.1f,10.0f,1.0f,.01f);
			SetCheckBox(hwndDlg,IDC_DIM_REFL,theMtl->dimReflect);
			break;
		case WM_COMMAND:  
		    switch (id) {
				case IDC_TR_ADD:
				case IDC_TR_SUB:
				case IDC_TR_SUB2:
					CheckRadioButton( hwndDlg, IDC_TR_ADD,IDC_TR_SUB2,id);
					theMtl->SetFlag(STDMTL_ADD_TRANSP, id==IDC_TR_ADD);
					theMtl->SetFlag(STDMTL_FILT_TRANSP, id==IDC_TR_SUB2);
					FixFilterButtons();
					theMtl->NotifyChanged();
					break;												 

				case IDC_TF_IN:
				case IDC_TF_OUT:
					CheckRadioButton( hwndDlg, IDC_TF_IN,IDC_TF_OUT,id);
					theMtl->SetFlag(STDMTL_FALLOFF_OUT, id==IDC_TF_OUT);
					theMtl->NotifyChanged();
					break;												 

				case IDC_PIXELS:
				case IDC_UNITS:
					CheckRadioButton( hwndDlg, IDC_PIXELS,IDC_UNITS,id);
					theMtl->SetFlag(STDMTL_WIRE_UNITS, id==IDC_UNITS);
					if (theMtl->GetFlag(STDMTL_WIRE)) {
						UpdateMtlDisplay();
						theMtl->NotifyChanged();
						}
					break;												 
				case IDC_DIM_REFL:
					theMtl->dimReflect = GetCheckBox(hwndDlg,IDC_DIM_REFL);
					theMtl->NotifyChanged();
					break;
				}
			break;
		case WM_PAINT: 	
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
		case WM_DESTROY:		break;
			break;
		case CC_SPINNER_CHANGE: 
			if (!theHold.Holding()) theHold.Begin();
			switch(id) {
				case IDC_WIRE_SPIN: 
					theMtl->SetWireSize(wireSizeSpin->GetFVal(),curTime); 
					break;
				case IDC_TF_SPIN: theMtl->SetOpacFalloff(PcToFrac(tfSpin->GetIVal()),curTime); 
					break;
				case IDC_IOR_SPIN: 
					theMtl->SetIOR(iorSpin->GetFVal(),curTime); 
					break;
				case IDC_DIM_AMTSPIN: 
					theMtl->SetDimIntens(dimSpin->GetFVal(),curTime); 
					break;
				case IDC_DIM_MULTSPIN: 
					theMtl->SetDimMult(dimMultSpin->GetFVal(),curTime); 
					break;
				}
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;		
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));			
			else 
				theHold.Cancel();
			UpdateMtlDisplay();
			break;
    default:
        return FALSE;
    	}
	return FALSE;
	}

//----------------------------------------------------------------------------
// ---  Drag-and-drop for Texture Maps ----
//----------------------------------------------------------------------------


BOOL StdMtlDlg::TexmapPanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:{
			for (int i=0; i<NTEXMAPS; i++) {
				if (i==ID_BU||i==ID_DP)
					texAmtSpin[i] = SetupIntSpinner(hwndDlg, texSpinID[i],texAmtID[i], -999,999, 0);
				else 
					texAmtSpin[i] = SetupIntSpinner(hwndDlg, texSpinID[i],texAmtID[i], 0,100, 0);
				HWND hw = GetDlgItem(hwndDlg, texMapID[i]);
//				WNDPROC oldp = (WNDPROC)GetWindowLongPtr(hw, GWLP_WNDPROC);
//				SetWindowLongPtr( hw, GWLP_WNDPROC, (LONG_PTR)TexSlotWndProc);
//				SetWindowLongPtr( hw, GWLP_USERDATA, (LONG)oldp);
				iBut[i] = GetICustButton(GetDlgItem(hwndDlg, texMapID[i]));
				iBut[i]->SetDADMgr(&dadMgr);
				}
			SetupPadLockButton(hwndDlg,IDC_LOCK_ADTEX, TRUE);
			}
			break;
		case WM_COMMAND:  
			{
			for (int i=0; i<NTEXMAPS; i++) {
				if (id == texOnID[i]) {
					theMtl->EnableMap(i,GetCheckBox(hwndDlg, id));
					UpdateMBut(i);
					goto exit;
					}
				if (id == texMapID[i]) {
					PostMessage(hwmedit,WM_TEXMAP_BUTTON, i ,(LPARAM)theMtl);
					goto exit;
					}
				}
			}

		    switch (id) {
				case IDC_LOCK_ADTEX:
					SetLockADTex(IsButtonChecked(hwndDlg, IDC_LOCK_ADTEX));
					break;
				}
			break;
		case WM_PAINT: 	
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
		case WM_DESTROY:		break;
		case CC_SPINNER_CHANGE:    
			{
			if (!theHold.Holding()) theHold.Begin();
			for (int i=0; i<NTEXMAPS; i++) {
				if (id == texSpinID[i]) {
					theMtl->SetTexmapAmt(i,PcToFrac(texAmtSpin[i]->GetIVal()), curTime);
					texAmtSpin[i]->SetKeyBrackets(theMtl->AmtKeyAtTime(i,curTime));
					break;
					}
				}
			}
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;		
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else 
				theHold.Cancel();
			UpdateMtlDisplay();
			break;
	    default:
    	    return FALSE;
    	}
	exit:
	return FALSE;
	}


BOOL StdMtlDlg::DynamPanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:
			iBounce =    SetupFloatSpinner(hwndDlg, IDC_BOUNCE_SPIN, IDC_BOUNCE_EDIT, 0.0f,1.0f,1.0f,.01f);
			iStatFrict = SetupFloatSpinner(hwndDlg, IDC_STATFRIC_SPIN, IDC_STATFRIC_EDIT, 0.0f,1.0f,0.0f,.01f);
			iSlidFrict = SetupFloatSpinner(hwndDlg, IDC_SLIDFRIC_SPIN, IDC_SLIDFRIC_EDIT, 0.0f,1.0f,0.0f,.01f);
			break;
		case WM_COMMAND:  
			break;
		case WM_DESTROY:		break;
		case CC_SPINNER_CHANGE:    
			{
			if (!theHold.Holding()) theHold.Begin();
			switch(id) {
				case IDC_BOUNCE_SPIN: 
					theMtl->SetDynamicsProperty(curTime,0,DYN_BOUNCE,iBounce->GetFVal()); 
					break;
				case IDC_STATFRIC_SPIN: 
					theMtl->SetDynamicsProperty(curTime,0,DYN_STATIC_FRICTION,iStatFrict->GetFVal()); 
					break;
				case IDC_SLIDFRIC_SPIN: 
					theMtl->SetDynamicsProperty(curTime,0,DYN_SLIDING_FRICTION,iSlidFrict->GetFVal()); 
					break;
				}
			}
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;		
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else 
				theHold.Cancel();
			break;
	    default:
    	    return FALSE;

    	}
	return FALSE;
	}

static INT_PTR CALLBACK  BasicPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	StdMtlDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (StdMtlDlg*)lParam;
		theDlg->hPanelBasic = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (StdMtlDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	BOOL res;
	theDlg->isActive = 1;
	res = theDlg->BasicPanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

static INT_PTR CALLBACK  ExtraPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	StdMtlDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (StdMtlDlg*)lParam;
		theDlg->hPanelExtra = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (StdMtlDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	BOOL res;
	theDlg->isActive = 1;
	res = theDlg->ExtraPanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

static INT_PTR CALLBACK  TexmapPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	StdMtlDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (StdMtlDlg*)lParam;
		theDlg->hPanelTexmap = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (StdMtlDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	BOOL res;
	theDlg->isActive = 1;
	res = theDlg->TexmapPanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}


static INT_PTR CALLBACK  DynamPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	StdMtlDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (StdMtlDlg*)lParam;
		theDlg->hPanelDynam = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (StdMtlDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	BOOL res;
	theDlg->isActive = 1;
	res = theDlg->DynamPanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

void StdMtlDlg::UpdateTexmapDisplay(int i) {
	texAmtSpin[i]->SetValue(FracToPc(theMtl->GetTexmapAmt(i)),FALSE);
	texAmtSpin[i]->SetKeyBrackets(theMtl->AmtKeyAtTime(i,curTime));
	SetCheckBox(hPanelTexmap, texOnID[i], theMtl->IsMapEnabled(i));
	Texmap *t = (*theMtl->maps)[i].map;
	TSTR nm;
	if (t) 
		nm = t->GetFullName();
	else 
		nm = GetString(IDS_DS_NONE);
	iBut[i]->SetText(nm.data());
//	SetDlgItemText(hPanelTexmap, texMapID[i], nm.data());
	UpdateMBut(i);
	}

void StdMtlDlg::UpdateTexmaps() {
	for (int i=0; i<NTEXMAPS; i++)
		UpdateTexmapDisplay(i);
	}

void  StdMtlDlg::LoadDialog(BOOL draw) {
	if (theMtl&&hPanelBasic) {
		Interval v;
//		assert(hPanelBasic);  

		// BASIC PANEL
		HWND hwndShading = GetDlgItem(hPanelBasic, IDC_SHADING);
		SendMessage( hwndShading, CB_SETCURSEL, ListIDFromShade((WPARAM)theMtl->GetShaderId()), (LPARAM)0 );
		ULONG flg = theMtl->flags;
		SetCheckBox(hPanelBasic,IDC_WIRE,  (flg&STDMTL_WIRE)?1:0); 
		SetCheckBox(hPanelBasic,IDC_2SIDE, (flg&STDMTL_2SIDE)?1:0); 
		SetCheckBox(hPanelBasic,IDC_SUPER_SAMP, (flg&STDMTL_SSAMP)?1:0); 

//		SetCheckBox(hPanelBasic,IDC_SOFTEN, (flg&STDMTL_SOFTEN)?1:0); 
//		SetCheckBox(hPanelBasic,IDC_OLD_SPEC, (flg&STDMTL_OLDSPEC)?1:0); 
		SetCheckBox(hPanelBasic,IDC_FACE_MAP, (flg&STDMTL_FACEMAP)?1:0); 
		CheckRadioButton( hPanelBasic, IDC_AMB,IDC_FILT, IDC_AMB+editingColor);
		shSpin->SetValue(FracToPc(theMtl->GetShininess()),FALSE);
		shSpin->SetKeyBrackets(KeyAtCurTime(PB_SHININESS));

		ssSpin->SetValue(FracToPc(theMtl->GetShinStr()),FALSE);
		ssSpin->SetKeyBrackets(KeyAtCurTime(PB_SHIN_STR));

		// >>>>
		siSpin->SetValue(FracToPc(theMtl->GetSelfIll()),FALSE);
		siSpin->SetKeyBrackets(KeyAtCurTime(PB_SELFI));

		trSpin->SetValue(FracToPc(theMtl->GetOpacity()),FALSE);
		trSpin->SetKeyBrackets(KeyAtCurTime(PB_OPAC));

		tfSpin->SetValue(FracToPc(theMtl->GetOpacFalloff()),FALSE);
		tfSpin->SetKeyBrackets(KeyAtCurTime(PB_OPFALL));

		softSpin->SetValue(theMtl->softThresh,FALSE);
		softSpin->SetKeyBrackets(KeyAtCurTime(PB_SOFTEN));
		softSpin->Enable(theMtl->shading!=SHADE_METAL);

		CheckButton(hPanelBasic, IDC_LOCK_AD, theMtl->GetFlag(STDMTL_LOCK_AD));
		CheckButton(hPanelBasic, IDC_LOCK_DS, theMtl->GetFlag(STDMTL_LOCK_DS));
	 	UpdateLockADTex();
		FixFilterButtons();
		EnablePhongStuff();

		curRGB = MtlColor(editingColor).toRGB();
		RGBtoHSV(curRGB, &H, &S, &V);
		UpdateColControls(SET_HSV|SET_RGB);

		UpdateColSwatches();
		UpdateHilite();

		// EXTRA PANEL
		int b,c;
		b = theMtl->GetFlag(STDMTL_ADD_TRANSP);
		c = theMtl->GetFlag(STDMTL_FILT_TRANSP);
		CheckRadioButton(hPanelExtra,IDC_TR_ADD,IDC_TR_SUB2,b?IDC_TR_ADD:c?IDC_TR_SUB2:IDC_TR_SUB);

		b = theMtl->GetFlag(STDMTL_FALLOFF_OUT);
		CheckRadioButton(hPanelExtra,IDC_TF_IN,IDC_TF_OUT,b?IDC_TF_OUT:IDC_TF_IN);

		b = theMtl->GetFlag(STDMTL_WIRE_UNITS);
		CheckRadioButton(hPanelExtra,IDC_PIXELS,IDC_UNITS,b?IDC_UNITS:IDC_PIXELS);

		wireSizeSpin->SetValue( theMtl->WireSize(),FALSE);
		wireSizeSpin->SetKeyBrackets(KeyAtCurTime(PB_WIRESZ));

		iorSpin->SetValue( theMtl->GetIOR(),FALSE);
		iorSpin->SetKeyBrackets(KeyAtCurTime(PB_IOR));

		SetCheckBox(hPanelExtra,IDC_DIM_REFL,theMtl->dimReflect);

		dimSpin->SetValue( theMtl->GetDimIntens(curTime),FALSE);
		dimSpin->SetKeyBrackets(KeyAtCurTime(PB_DIMLEV));

		dimMultSpin->SetValue( theMtl->GetDimMult(curTime),FALSE);
		dimMultSpin->SetKeyBrackets(KeyAtCurTime(PB_DIMMULT));


		// TEXMAP PANEL & BASIC PANEL
		for (int i=0; i<NTEXMAPS; i++) 
			UpdateTexmapDisplay(i);

#ifndef	 DESIGN_VER
		// DYNAMICS PANEL
		iBounce->SetValue(theMtl->GetDynamicsProperty(curTime,0,DYN_BOUNCE),FALSE); 
		iBounce->SetKeyBrackets(KeyAtCurTime(PB_BOUNCE));

		iStatFrict->SetValue(theMtl->GetDynamicsProperty(curTime,0,DYN_STATIC_FRICTION),FALSE); 
		iStatFrict->SetKeyBrackets(KeyAtCurTime(PB_STATFRIC));

		iSlidFrict->SetValue(theMtl->GetDynamicsProperty(curTime,0,DYN_SLIDING_FRICTION),FALSE); 
		iSlidFrict->SetKeyBrackets(KeyAtCurTime(PB_SLIDFRIC));
#endif
		}
	}

void StdMtlDlg::SetRGB(DWORD rgb) {
	curRGB = rgb;
	RGBtoHSV (curRGB, &H, &S, &V);
	}


void StdMtlDlg::UpdateColSwatches() {
	for(int i=0; i<NCOLBOX; i++) {
		cs[i]->SetKeyBrackets(theMtl->KeyAtTime(PB_ID[i],curTime));
		cs[i]->SetColor(MtlColor(i));
		}
	}

void StdMtlDlg::UpdateColControls( int  which) {
	HDC hdc;
    hdc = GetDC (hPanelBasic);
	BOOL key = KeyAtCurTime(PB_ID[editingColor]);
    if (which & SET_HSV) {
		hSpin->SetValue((int)H,FALSE);
		sSpin->SetValue((int)S,FALSE);
		vSpin->SetValue((int)V,FALSE);
		hSpin->SetKeyBrackets(key);
		sSpin->SetKeyBrackets(key);
		vSpin->SetKeyBrackets(key);
	    }
    if (which & SET_RGB) {
		rSpin->SetValue((int)GetRValue (curRGB),FALSE);
		gSpin->SetValue((int)GetGValue (curRGB),FALSE);
		bSpin->SetValue((int)GetBValue (curRGB),FALSE);
		rSpin->SetKeyBrackets(key);
		gSpin->SetKeyBrackets(key);
		bSpin->SetKeyBrackets(key);
	    }
	ReleaseDC(hPanelBasic,hdc);
	}

void StdMtlDlg::UpdateColFromSpin(HWND hwndDlg,int indx, ISpinnerControl *spin ) {
    int r, g, b;
    int *vals[6] = {&H, &S, &V, &r, &g, &b};
    int update = 0;
    r = GetRValue (curRGB);
    g = GetGValue (curRGB);
    b = GetBValue (curRGB);
    *vals[indx] = spin->GetIVal();
    switch (indx) {
        case Hc: case Sc: case Vc:
            // Changing the HSV settings.  Compute new RGB. 
            curRGB = HSVtoRGB (H, S, V);
            update |= SET_RGB; // Update only the RGB controls. 
            break;
        case Rc: case Gc:  case Bc:
            // Changing the RGB settings.  Compute new HSV. 
			SetRGB(RGB(r,g,b));
            update |= SET_HSV; // Update only the HSV controls. 
            break;
	    }
	SetMtlColor(editingColor,Color(curRGB));
	UpdateColControls(update);
	cs[editingColor]->SetColor(MtlColor(editingColor));
	}

void StdMtlDlg::UpdateMtlDisplay() {
	ip->MtlChanged(); // redraw viewports
	}


void StdMtlDlg::ActivateDlg(BOOL onOff) {
	UpdateColControls(0);
	for(int i=0; i<NCOLBOX; i++) 
		cs[i]->Activate(onOff);
//	UpdateColSwatches();
	}

void StdMtlDlg::SetEditColor(int i) {
	editingColor = i;
	cs[editingColor]->EditThis(FALSE);
	curRGB = MtlColor(i).toRGB();
	RGBtoHSV(curRGB, &H, &S, &V);
	UpdateColControls(SET_HSV|SET_RGB);
	if (hPanelBasic) {
		CheckRadioButton( hPanelBasic, IDC_AMB,IDC_FILT, IDC_AMB+i);
		//cs[i]->SetCheck(check);
		}
	}


void StdMtlDlg::SetLockAD(BOOL lock) {
	if (lock) {
		if (IDYES!=MessageBox(hwmedit, GetString(IDS_DS_LOCKAD), GetString(IDS_DS_LOCKCOL), MB_YESNO)) {
			CheckButton(hPanelBasic, IDC_LOCK_AD, FALSE);	
			return;	
			}
		if (editingColor==0) 
			SetMtlColor(1, MtlColor(0));
		else 
			SetMtlColor(0, MtlColor(1));
		UpdateColControls(SET_HSV|SET_RGB);
		UpdateColSwatches();
		}
	theMtl->SetFlag(STDMTL_LOCK_AD,lock);
	}

static int IndexForMapID(int id) {
	for (int i=0; i<NTEXMAPS; i++) if (id==texMapID[i]) return i;
	return 0;
	}
void StdMtlDlg::UpdateLockADTex() {
	int lock = 	theMtl->GetFlag(STDMTL_LOCK_ADTEX);
	CheckButton(hPanelBasic, IDC_LOCK_ADTEX,lock);
	CheckButton(hPanelTexmap, IDC_LOCK_ADTEX,lock);
	ShowWindow(GetDlgItem(hPanelBasic,IDC_MAPON_AM), !lock);
	iBut[IndexForMapID(IDC_MAP_AM)]->Enable(!lock);
	}

void StdMtlDlg::SetLockADTex(BOOL lock) {
	theMtl->SetFlag(STDMTL_LOCK_ADTEX,lock);
	UpdateLockADTex();
	theMtl->NotifyChanged();
	UpdateMtlDisplay();
	}

void StdMtlDlg::SetLockDS(BOOL lock) {
	if (lock) {
		if (IDYES!=MessageBox(hwmedit, GetString(IDS_DS_LOCK_DS),GetString(IDS_DS_LOCKCOL), MB_YESNO)) {
			CheckButton(hPanelBasic, IDC_LOCK_DS, FALSE);	
			return;	
			}
		if (editingColor==2) 
			SetMtlColor(1, MtlColor(2));
		else
			SetMtlColor(2, MtlColor(1));
		UpdateColControls(SET_HSV|SET_RGB);
		UpdateColSwatches();
		}
	theMtl->SetFlag(STDMTL_LOCK_DS,lock);
	}

void StdMtlDlg::SetThing(ReferenceTarget *m) {
	assert (m->SuperClassID()==MATERIAL_CLASS_ID);
	assert (m->ClassID()==stdmtlClassID);
	if (theMtl) 
		theMtl->paramDlg = NULL;
	theMtl = (StdMtl *)m;
	if (theMtl)
		theMtl->paramDlg = this;
	LoadDialog(TRUE);
	if (hPanelBasic)
		for (int i=0; i<4; i++) cs[i]->InitColor(MtlColor(i));
	}

static void VertLine(HDC hdc,int x, int ystart, int yend) {
	MoveToEx(hdc,x,ystart,NULL); 
	if (ystart<=yend)
		LineTo(hdc, x, yend+1);
	else 
		LineTo(hdc, x, yend-1);
	}


//-------------------------------------------------------

//-------------------------------------------------------
void StdMtlDlg::GenDrawHilite(HDC hdc, Rect& rect, SShader &sh) {
	int w,h,npts,xcen,ybot,ytop,ylast,i,iy;
	HPEN linePen = (HPEN)GetStockObject(WHITE_PEN);
	HPEN fgPen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNFACE));
	HPEN bgPen = CreatePen(PS_SOLID,0,GetCustSysColor(COLOR_BTNSHADOW));
	w = rect.w();
	h = rect.h()-3;
	npts = (w-2)/2;
	xcen = rect.left+npts;
	ybot = rect.top+h;
	ytop = rect.top+2;
	ylast = -1;
	for (i=0; i<npts; i++) {
		float v = sh.EvalHilite((float)i/((float)npts*2.0f));
		if (v>2.0f) v = 2.0f; // keep iy from wrapping
		iy = ybot-(int)(v*((float)h-2.0f));

		if (iy<ytop) iy = ytop;

		SelectPen(hdc, fgPen);
		VertLine(hdc,xcen+i,ybot,iy);
		VertLine(hdc,xcen-i,ybot,iy);

		if (iy-1>ytop) {
			// Fill in above curve
			SelectPen(hdc,bgPen);
			VertLine(hdc,xcen+i, ytop, iy-1);
			VertLine(hdc,xcen-i, ytop, iy-1);
			}
		if (ylast>=0) {
			SelectPen(hdc,linePen);
			VertLine(hdc,xcen+i-1,iy-1,ylast);
			VertLine(hdc,xcen-i+1,iy-1,ylast);
			}

		ylast = iy;
		}
	SelectObject( hdc, linePen );
	DeleteObject(fgPen);
	DeleteObject(bgPen);
	WhiteRect3D(hdc, rect, 1);
	}

void StdMtlDlg::DrawHilite(HDC hdc, Rect& rect) {
	if (theMtl==NULL) return;
	theMtl->UpdateShader();
	GenDrawHilite(hdc,rect,*theMtl->curShader);
	}

void StdMtlDlg::EnablePhongStuff() {
	BOOL b = theMtl->shading==SHADE_METAL?0:1;
	EnableWindow(GetDlgItem(hPanelBasic,IDC_SOFTEN), b);
	EnableWindow(GetDlgItem(hPanelBasic,IDC_SPEC), b);
	EnableWindow(GetDlgItem(hPanelBasic,IDC_MAPON_SP), b);
	EnableWindow(GetDlgItem(hPanelTexmap,IDC_USEMAP_SP), b);
	EnableWindow(GetDlgItem(hPanelTexmap,IDC_AMT_SP), b);
	EnableWindow(GetDlgItem(hPanelTexmap,IDC_SPIN_SP), b);
	EnableWindow(GetDlgItem(hPanelTexmap,IDC_MAP_SP), b);
	EnableWindow(GetDlgItem(hPanelTexmap,IDC_MAP_SP), b);
	if (b) cs[2]->Enable(); else cs[2]->Disable();
	}

void StdMtlDlg::UpdateHilite() {
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r);
	ReleaseDC(hwHilite,hdc);
	}


void StdMtlDlg::BuildDialog() {
	if ((theMtl->flags&(STDMTL_ROLLUP_FLAGS))==0) 
		theMtl->flags |= STDMTL_ROLLUP1_OPEN;
	hPanelBasic = ip->AddRollupPage( 
		hInstance,	//getResMgr().getHInst(RES_ID_DS), 
		MAKEINTRESOURCE(IDD_DMTL_BASIC),
		BasicPanelDlgProc, 
		GetString(IDS_DS_BASIC), 
		(LPARAM)this,
		theMtl->flags&STDMTL_ROLLUP1_OPEN?0:APPENDROLL_CLOSED
		);		

	hPanelExtra = ip->AddRollupPage( 
		hInstance,	//getResMgr().getHInst(RES_ID_DS), 
		MAKEINTRESOURCE(IDD_DMTL_EXTRA),
		ExtraPanelDlgProc, 
		GetString(IDS_DS_EXTRA), 
		(LPARAM)this,		
		theMtl->flags&STDMTL_ROLLUP2_OPEN?0:APPENDROLL_CLOSED
		);		

   	hPanelTexmap = ip->AddRollupPage( 
		hInstance,	//getResMgr().getHInst(RES_ID_DS), 
		MAKEINTRESOURCE(IDD_DMTL_TEXMAP),
		TexmapPanelDlgProc, 
		GetString(IDS_DS_TEXMAP), 
		(LPARAM)this,
		theMtl->flags&STDMTL_ROLLUP3_OPEN?0:APPENDROLL_CLOSED
		);		

#ifndef DESIGN_VER
   	hPanelDynam = ip->AddRollupPage( 
		hInstance,	//getResMgr().getHInst(RES_ID_DS), 
		MAKEINTRESOURCE(IDD_DMTL_DYNAM),
		DynamPanelDlgProc, 
		GetString(IDS_DS_DYNAMICS), 
		(LPARAM)this,
		theMtl->flags&STDMTL_ROLLUP4_OPEN?0:APPENDROLL_CLOSED
		);		
#endif

	ip->SetRollupScrollPos(theMtl->rollScroll);
	}


//-----------------------------------------------------------------------------
//  StdMtl
//-----------------------------------------------------------------------------


#define NPARAMS 17
#define STDMTL_PBVERSION   9

//Current Param Block Descriptor
static ParamBlockDescID stdmtlPB[NPARAMS] = {
	{ TYPE_RGBA, NULL, TRUE,1 },    // ambient
	{ TYPE_RGBA, NULL, TRUE,2 },    // diffuse
	{ TYPE_RGBA, NULL, TRUE,3 },    // specular
	{ TYPE_FLOAT, NULL, TRUE,4 },   // shininess
	{ TYPE_FLOAT, NULL, TRUE,5 },   // shini_strength
	{ TYPE_FLOAT, NULL, TRUE,6 },   // self-illum
	{ TYPE_FLOAT, NULL, TRUE,7 },   // opacity
	{ TYPE_FLOAT, NULL, TRUE,8 },	// opfalloff
	{ TYPE_RGBA,  NULL, TRUE,9 },   // filter
	{ TYPE_FLOAT, NULL, TRUE,10 },  // wireSize
	{ TYPE_FLOAT, NULL, TRUE,11 },  // index of refraction
	{ TYPE_FLOAT, NULL, TRUE,12 },  // bounce
	{ TYPE_FLOAT, NULL, TRUE,13 },  // static friction
	{ TYPE_FLOAT, NULL, TRUE,14 },  // sliding friction
	{ TYPE_FLOAT, NULL, TRUE,15 },  // reflect dim level
	{ TYPE_FLOAT, NULL, TRUE,16 },   // reflect dim multiplier 
	{ TYPE_FLOAT, NULL, TRUE,17 }   // soften
	}; 

#define NUMOLDVER 9

static ParamVersionDesc oldVersions[NUMOLDVER] = {
	ParamVersionDesc(stdmtlPB,8, 0),
	ParamVersionDesc(stdmtlPB,9, 1),
	ParamVersionDesc(stdmtlPB,9, 2),
	ParamVersionDesc(stdmtlPB,10,3),
	ParamVersionDesc(stdmtlPB,11,4),
	ParamVersionDesc(stdmtlPB,14,5),
	ParamVersionDesc(stdmtlPB,15,6),
	ParamVersionDesc(stdmtlPB,15,7),
	ParamVersionDesc(stdmtlPB,16,8)
	};

static ParamVersionDesc curVersion(stdmtlPB,NPARAMS,STDMTL_PBVERSION);

void StdMtl::Reset() {
	SuspendSetKeyMode();
//	shading = SHADE_PHONG;
	SetShadingNoNotify(SHADE_BLINN);
	flags = STDMTL_FILT_TRANSP|STDMTL_LOCK_ADTEX|STDMTL_SOFTEN;
	dimReflect = FALSE;
	ReplaceReference( 0, CreateParameterBlock( stdmtlPB, NPARAMS, STDMTL_PBVERSION ) );	
	ReplaceReference( 1, new Texmaps((MtlBase*)this));	
	ivalid.SetEmpty();
	SetDimIntens(0.0f,0);
	SetDimMult(3.0f,0);
	SetSoftenLevel(0.1f,0);
	SetAmbient(Color(0.1f,0.1f,0.1f),0);
	SetDiffuse(Color(0.5f,0.5f,0.5f),0);
	SetSpecular(Color(0.9f,0.9f,0.9f),0);
	SetOpacity(1.0f,0);
	SetFilter(Color(.5f,.5f,.5f),0);
	SetShininess(.25f,0);   // change from .4, 5-21-97
	SetShinStr(.05f,0);      // change from .3, 5-21-97   
// >>>> 
	SetSelfIllum(.0f,0);
	SetOpacFalloff(0.0f,0);
	SetWireSize(1.0f,0);
	SetIOR(1.5f,0);
	SetTexmapAmt(ID_BU, BUMP_DEF_AMT, 0);
	SetDynamicsProperty(0,0,DYN_BOUNCE,1.0f);
	SetDynamicsProperty(0,0,DYN_STATIC_FRICTION,0.0f);
	SetDynamicsProperty(0,0,DYN_SLIDING_FRICTION,0.0f);
	ResumeSetKeyMode();
	}

StdMtl::StdMtl(BOOL loading) {
	paramDlg = NULL;
	pblock = NULL;
	maps = NULL;
	ambient = diffuse = specular = filter = Color(0.0f,0.0f,0.0f);
	SetShadingNoNotify(SHADE_PHONG);
	flags = STDMTL_FILT_TRANSP|STDMTL_LOCK_ADTEX|STDMTL_SOFTEN;
	dimReflect = FALSE;
	dimIntens = 0.0f;
	dimMult = 2.0f;
	ivalid.SetEmpty();
	if (!loading)
		Reset();
	rollScroll = 0;
	flags |= STDMTL_ROLLUP1_OPEN;
	}

RefTargetHandle StdMtl::Clone(RemapDir &remap) {
	//DebugPrint(" Cloning STDMTL %d \n",++numStdMtls);
	StdMtl *mnew = new StdMtl();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->ReplaceReference(1,remap.CloneRef(maps));
	mnew->ivalid.SetEmpty();	
	mnew->flags = flags;
	mnew->ambient = ambient;
	mnew->diffuse = diffuse;
	mnew->specular = specular;
	mnew->filter = filter;
	mnew->shininess = shininess;
	mnew->phongexp = phongexp;
	mnew->shine_str = shine_str;
	mnew->self_illum = self_illum;
	mnew->opacity = opacity;
	mnew->opfall = opfall;
	mnew->wireSize = wireSize;
	mnew->ioRefract = ioRefract;
	mnew->dimReflect = dimReflect;
	mnew->softThresh = softThresh;
	mnew->SetShadingNoNotify(shading);
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* StdMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	Interval v;
	Update(imp->GetTime(),v);
	StdMtlDlg *dm = new StdMtlDlg(hwMtlEdit, imp, this);
	LoadStdMtlResources();
	dm->BuildDialog();
	dm->LoadDialog(FALSE);	 
	SetParamDlg(dm);
	return dm;	
	}

#define LIMIT0_1(x) if (x<0.0f) x = 0.0f; else if (x>1.0f) x = 1.0f;
#define LIMITMINMAX(x,min,max) if (x<min) x = min; else if (x>max) x = max;

static Color LimitColor(Color c) {
	LIMIT0_1(c.r);
	LIMIT0_1(c.g);
	LIMIT0_1(c.b);
	return c;
	}

void StdMtl::Update(TimeValue t, Interval &valid) {
	Point3 p;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( PB_AMBIENT, t, p, ivalid );
		ambient = LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( PB_DIFFUSE, t, p, ivalid );
		diffuse= LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( PB_SPECULAR, t, p, ivalid );
		specular = LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( PB_FILTER, t, p, ivalid );
		filter = LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( PB_SHININESS, t, shininess, ivalid );
		pblock->GetValue( PB_SHIN_STR, t, shine_str, ivalid );
		// >>>>>
		pblock->GetValue( PB_SELFI, t, self_illum, ivalid );
		pblock->GetValue( PB_OPAC, t, opacity, ivalid );

		pblock->GetValue( PB_OPFALL, t, opfall, ivalid );
		pblock->GetValue( PB_WIRESZ, t, wireSize, ivalid );
		pblock->GetValue( PB_IOR, t, ioRefract, ivalid );
		pblock->GetValue( PB_DIMLEV, t, dimIntens, ivalid );
		pblock->GetValue( PB_DIMMULT, t, dimMult, ivalid );
		pblock->GetValue( PB_SOFTEN, t, softThresh, ivalid); 

		LIMIT0_1(opacity);
		LIMIT0_1(self_illum);
		LIMIT0_1(shininess);
		LIMITMINMAX(shine_str,0.0f,9.99f);
		LIMIT0_1(opfall);
		LIMIT0_1(dimIntens);
		LIMIT0_1(softThresh);

		phongexp = (float)pow(2.0,shininess*10.0);
	
		for (int i=0; i<NTEXMAPS; i++) 	{
			if (MAPACTIVE(i)) 
				maps->txmap[i].Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}

//-----------------------------------------------------------------------
// DS - 4/7/97: Changed Opacity, Self-illumination, SHininess, Shininess strengh
// so that the map amount blends between the corresponding slider 
// setting and the map value.  This code fixes up old files so they
// will render the same way. This does not correctly handle animated values
// for the amount or parameter sliders.
//-----------------------------------------------------------------------

void StdMtl::OldVerFix(int loadVer) {
	if (loadVer<8) {
		Interval v;
		Update(0,v);
		if (MAPACTIVE(ID_OP)) {
			if (maps->txmap[ID_OP].amount != 1.0f) 
				SetOpacity(0.0f,0);
			}
		if (MAPACTIVE(ID_SI)) {
			if (maps->txmap[ID_SI].amount != 1.0f) 
				// >>>>>
				SetSelfIllum(0.0f,0);
			}	
		if (MAPACTIVE(ID_SS)) {
			float amt = maps->txmap[ID_SS].amount;
			SetTexmapAmt(ID_SS,amt*shine_str,0);
			SetShinStr(0.0f,0);
			}
		if (MAPACTIVE(ID_SH)) {
			float amt = maps->txmap[ID_SH].amount;
			SetTexmapAmt(ID_SH,amt*shininess,0);
			SetShininess(0.0f,0);
			}
		}
	if (loadVer<9) {
		if (flags&STDMTL_SOFTEN) 
			SetSoftenLevel(.6f,0);
		else 
			SetSoftenLevel(0.0f,0);
		}
	}



ULONG StdMtl::Requirements(int subMtlNum) {
	ULONG req = 0;
	
	switch (shading) {
		case SHADE_CONST:
			req |= MTLREQ_DONTMERGE_FRAGMENTS;
			break;
		case SHADE_PHONG:
		case SHADE_BLINN:
		case SHADE_METAL: req |= MTLREQ_PHONG; break;
		}

	if (opacity!=1.0f||MAPACTIVE(ID_OP)||opfall>0.0f) 
		req |= MTLREQ_TRANSP;

	if (opacity!=1.0f)
		req |= MTLREQ_TRANSP_IN_VP;

	for (int i=0; i<NTEXMAPS; i++) {
		if (MAPACTIVE(i))	
			req |= (*maps)[i].map->Requirements(subMtlNum);
		}
	if (MAPACTIVE(ID_BU)) {
		ULONG bmpreq = (*maps)[ID_BU].map->Requirements(subMtlNum);
		if (bmpreq&MTLREQ_UV)
			req |= MTLREQ_BUMPUV;
		if (bmpreq&MTLREQ_UV2)
			req |= MTLREQ_BUMPUV2;
		}
	if (flags&STDMTL_WIRE) 	req|= MTLREQ_WIRE;
	if (flags&STDMTL_2SIDE) req|= MTLREQ_2SIDE;
	if (flags&STDMTL_SSAMP) req|= MTLREQ_SUPERSAMPLE;
	if (flags&STDMTL_WIRE_UNITS) req|= MTLREQ_WIRE_ABS;
	if (flags&STDMTL_FACEMAP) req |= MTLREQ_FACEMAP;
	if (flags&STDMTL_ADD_TRANSP) req |= MTLREQ_ADDITIVE_TRANSP;
	if (MAPACTIVE(ID_DP)) req |= MTLREQ_DISPLACEMAP;
	return req;		
	}

Interval StdMtl::Validity(TimeValue t) {
	Interval v;
	Update(t,v);
	return ivalid;
	}

void StdMtl::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

void StdMtl::SetFlag(ULONG f, ULONG val) {
	if (val) flags|=f; 
	else flags &= ~f;
	}

void StdMtl::SetTransparencyType(int type) {
	switch (type) {
		case TRANSP_SUBTRACTIVE:  
			SetFlag(STDMTL_ADD_TRANSP,0); 
			SetFlag(STDMTL_FILT_TRANSP,0); 
			break;
		case TRANSP_ADDITIVE:  
			SetFlag(STDMTL_ADD_TRANSP,1); 
			SetFlag(STDMTL_FILT_TRANSP,0); 
			break;
		case TRANSP_FILTER:  
			SetFlag(STDMTL_ADD_TRANSP,0); 
			SetFlag(STDMTL_FILT_TRANSP,1); 
			break;
		}
	}

void StdMtl::DeleteThis() {
	//DebugPrint(" Deleting STDMTL %d \n",numStdMtls--);
    delete this;
	}

TSTR StdMtl::SubAnimName(int i) { 
	return TSTR(GetString(i==0?IDS_DS_PARAMETERS:IDS_DS_TEXMAPS));
	}		

Animatable* StdMtl::SubAnim(int i) {
	switch(i) {
		case 0: return pblock; 
		case 1: return maps;
		default: assert(0); return NULL;
		}
	 }

RefTargetHandle StdMtl::GetReference(int i) {
	switch(i) {
		case 0: return pblock;
		case 1: return maps;
		default: assert(0);	 return NULL;
		}
	}

void StdMtl::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock*)rtarg; return;
		case 1:	{
			maps = (Texmaps*)rtarg; 
			if (maps!=NULL)
				maps->client = this;
			return;
			}
		default: assert(0);
		}
	}

void StdMtl::SetSubTexmap(int i, Texmap *m) {
	assert(i<NTEXMAPS);
	maps->ReplaceReference(2*i+1,m);
	if (m!=NULL) {
		EnableMap(i,TRUE);
		if (maps->txmap[i].amtCtrl==NULL) {			
			maps->ReplaceReference(2*i, NewDefaultFloatController());
			maps->txmap[i].amtCtrl->SetValue(TimeValue(0),&maps->txmap[i].amount);
			}
		}
	else {
		if (maps->txmap[i].amtCtrl!=NULL)			
			maps->DeleteReference(2*i);
		SetTexmapAmt(i, i==ID_BU?BUMP_DEF_AMT:1.0f,TimeValue(0));
		EnableMap(i,FALSE);
		}
	if (m&&(i==ID_RL||i==ID_RR)) {
		UVGen* uvg0 = m->GetTheUVGen();
		if (uvg0&&uvg0->IsStdUVGen()) {
			StdUVGen *uvg = (StdUVGen*)uvg0;
			uvg->InitSlotType(MAPSLOT_ENVIRON);
			uvg->SetCoordMapping(UVMAP_SPHERE_ENV);
			}
		}
	if (paramDlg)
		paramDlg->UpdateTexmapDisplay(i);
	}

static int nameID[NPARAMS] = { 
	IDS_DS_AMBIENT, IDS_DS_DIFFUSE, IDS_DS_SPECULAR,  IDS_DS_SHININESS,IDS_DS_SHIN_STR,
	IDS_DS_SELFI,IDS_DS_OPACITY,IDS_DS_XPFALL,IDS_DS_FILTER,IDS_DS_WIRESZ,
    IDS_DS_IOR,IDS_DS_BOUNCE,IDS_DS_STATFRIC,IDS_DS_SLIDFRIC,IDS_DS_DIMLEV,IDS_DS_DIMMULT,
    IDS_DS_SOFTEN};

RefResult StdMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_WANT_SHOWPARAMLEVEL:
			{
			BOOL *pb = (BOOL *)(partID);
			*pb = TRUE;
			return REF_STOP;
			}
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (paramDlg) {
				if (hTarget==pblock) {
					int np =pblock->LastNotifyParamNum();
					paramDlg->UpdateControlFor(np);
					}
				else// if (!paramDlg->isActive)  
					paramDlg->Invalidate();
				}
			break;
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_AMBIENT: 
				case PB_DIFFUSE: 
				case PB_SPECULAR: 
				case PB_FILTER: 
					gpd->dim = stdColor255Dim; 
					break;
				case PB_BOUNCE:
				case PB_STATFRIC:
				case PB_SLIDFRIC:
				case PB_DIMLEV:
				case PB_DIMMULT:
				case PB_SOFTEN:
				case PB_WIRESZ:
				case PB_IOR:
					gpd->dim = defaultDim; 
					break;
				case PB_SHININESS:
				case PB_SHIN_STR:
				case PB_SELFI:
				case PB_OPAC:
				case PB_OPFALL:
				default:
					gpd->dim = stdPercentDim; 
					break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name =  GetString(nameID[gpn->index]);
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

Class_ID StdMtl::ClassID() { 
	return stdmtlClassID; 
	}

TSTR StdMtl::GetSubTexmapSlotName(int i) {
	return GetString(texNameID[i]);
	}


void StdMtl::EnableMap(int i, BOOL onoff) { 
	maps->txmap[i].mapOn = onoff;
	NotifyChanged();
	}

// forward
SShader *GetShaderFromId(int s);

void StdMtl::SetShadingNoNotify( int s) {
	shading = s;
	curShader = GetShaderFromId(shading);
	}

void StdMtl::SetShaderId( long s) {
	long old = shading;
	SetShadingNoNotify(s);
	if (old!=shading) {
		NotifyChanged();
		if (paramDlg) {
			paramDlg->UpdateHilite();
			paramDlg->EnablePhongStuff();
			}
		}
	}

void StdMtl::SetAmbient(Color c, TimeValue t) {
	ambient =c;
	pblock->SetValue( PB_AMBIENT, t, Point3(c.r,c.g,c.b));
	NotifyChanged();
	}
			
void StdMtl::SetDiffuse(Color c, TimeValue t) {
	Point3 p;
	Interval iv;
	pblock->SetValue( PB_DIFFUSE, t, Point3(c.r,c.g,c.b));
	pblock->GetValue( PB_DIFFUSE, t, p, ivalid );
	diffuse= LimitColor(Color(p.x,p.y,p.z));
	NotifyChanged();
	}
			
void StdMtl::SetSpecular(Color c, TimeValue t) {
    specular =c;
	pblock->SetValue( PB_SPECULAR, t, Point3(c.r,c.g,c.b));
	NotifyChanged();
	}
			
void StdMtl::SetFilter(Color c, TimeValue t) {
    filter =c;
	pblock->SetValue( PB_FILTER, t, Point3(c.r,c.g,c.b));
	if (opacity!=1.0f||MAPACTIVE(ID_OP)) 
		NotifyChanged();
	}
			
void StdMtl::SetShininess(float v, TimeValue t) {
	shininess =v;
	phongexp = (float)pow(2.0,shininess*10.0);
	pblock->SetValue( PB_SHININESS, t, v);
	NotifyChanged();
	}
			
void StdMtl::SetShinStr(float v, TimeValue t) {
	shine_str =v;
	pblock->SetValue( PB_SHIN_STR, t, v);
	NotifyChanged();
	}

// >>>>			
void StdMtl::SetSelfIllum(float v, TimeValue t) {
	self_illum =v;
	pblock->SetValue( PB_SELFI, t, v);
	NotifyChanged();
	}
			
void StdMtl::SetOpacity(float v, TimeValue t) {
    opacity = v;
	pblock->SetValue( PB_OPAC, t, v);
	NotifyChanged();
	}
			
void StdMtl::SetOpacFalloff(float v, TimeValue t) {
	opfall = v;
	pblock->SetValue( PB_OPFALL, t, v);
	if (opacity!=1.0f||MAPACTIVE(ID_OP)) 
		NotifyChanged();
	}		

void StdMtl::SetWireSize(float v, TimeValue t) {
	wireSize = v;
	pblock->SetValue( PB_WIRESZ, t, v);
	if (flags&STDMTL_WIRE) 
		NotifyChanged();
	}

void StdMtl::SetIOR(float v, TimeValue t) {
	ioRefract = v;
	pblock->SetValue( PB_IOR, t, v);
  	NotifyChanged();
	}

void StdMtl::SetDimIntens(float v, TimeValue t) {
	dimIntens = v;
	pblock->SetValue( PB_DIMLEV, t, v);
  	NotifyChanged();
	}

void StdMtl::SetDimMult(float v, TimeValue t) {
	dimMult = v;
	pblock->SetValue( PB_DIMMULT, t, v);
  	NotifyChanged();
	}

void StdMtl::SetSoftenLevel(float v, TimeValue t) {
	softThresh = v;
	pblock->SetValue( PB_SOFTEN, t, v);
  	NotifyChanged();
	}

void StdMtl::SetTexmapAmt(int imap, float amt, TimeValue t) {
	if (maps->txmap[imap].amtCtrl) 
		maps->txmap[imap].amtCtrl->SetValue(t,&amt);
	maps->txmap[imap].amount = amt;
	if (maps->txmap[imap].IsActive())
		NotifyChanged();
	}

BOOL StdMtl::AmtKeyAtTime(int i, TimeValue t) {
	if (maps->txmap[i].amtCtrl) 
		return 	maps->txmap[i].amtCtrl->IsKeyAtTime(t,0);
	else
		return FALSE;
	}

long StdMtl::GetShaderId() {
	return shading;
	}

void StdMtl::UpdateShader() {
	curShader->SetShininess(shininess,shine_str);
	}

Color StdMtl::GetAmbient(int mtlNum, BOOL backFace) {   return ambient;}
Color StdMtl::GetDiffuse(int mtlNum, BOOL backFace) { return diffuse;}
Color StdMtl::GetSpecular(int mtlNum, BOOL backFace) {	return specular;	}
Color StdMtl::GetFilter() {	return filter;	}

float StdMtl::GetTexmapAmt(int imap) {	return maps->txmap[imap].amount;	}

float StdMtl::GetTexmapAmt(int imap, TimeValue t) {	return maps->txmap[imap].GetAmount(t); 	}


Color StdMtl::GetAmbient(TimeValue t)  { return pblock->GetColor(PB_AMBIENT,t);}		
Color StdMtl::GetDiffuse(TimeValue t)  { return pblock->GetColor(PB_DIFFUSE,t);	}		
Color StdMtl::GetSpecular(TimeValue t) { return pblock->GetColor(PB_SPECULAR,t);	}
Color StdMtl::GetFilter(TimeValue t)   { return pblock->GetColor(PB_FILTER,t);	}
float StdMtl::GetShininess( TimeValue t) {return pblock->GetFloat(PB_SHININESS,t);  }		
float StdMtl::GetShinStr(TimeValue t)  { return  pblock->GetFloat(PB_SHIN_STR,t);	}
// >>>>
float StdMtl::GetSelfIllum(TimeValue t){ return  pblock->GetFloat(PB_SELFI,t);	}		
float StdMtl::GetOpacity( TimeValue t) { return  pblock->GetFloat(PB_OPAC,t); }		
float StdMtl::GetOpacFalloff(TimeValue t){ return  pblock->GetFloat(PB_OPFALL,t);}		
float StdMtl::GetWireSize(TimeValue t) { return  pblock->GetFloat(PB_WIRESZ,t);}
float StdMtl::GetIOR( TimeValue t)     { return  pblock->GetFloat(PB_IOR,t);}
float StdMtl::GetDimIntens( TimeValue t)   { return  pblock->GetFloat(PB_DIMLEV,t); }
float StdMtl::GetDimMult( TimeValue t)   { return  pblock->GetFloat(PB_DIMMULT,t); }
float StdMtl::GetSoftenLevel( TimeValue t)   { return  pblock->GetFloat(PB_SOFTEN,t); }
BOOL StdMtl::MapEnabled(int i)         { return maps->txmap[i].mapOn;}


float StdMtl::GetDynamicsProperty(TimeValue t, int mtlNum, int propID) {
	float val;
	Interval ivalid;
	switch(propID) {
		case DYN_BOUNCE:
			pblock->GetValue(PB_BOUNCE,t,val,ivalid);	
			return val;
		case DYN_STATIC_FRICTION:
			pblock->GetValue(PB_STATFRIC,t,val,ivalid);	
			return val;
		case DYN_SLIDING_FRICTION:
			pblock->GetValue(PB_SLIDFRIC,t,val,ivalid);	
			return val;
		default: 
			assert(0);
			return 0.0f;
		}
	}

void StdMtl::SetDynamicsProperty(TimeValue t, int mtlNum, int propID, float value){
	switch(propID) {
		case DYN_BOUNCE: 
			pblock->SetValue( PB_BOUNCE, t, value);
			break;
		case DYN_STATIC_FRICTION:
			pblock->SetValue( PB_STATFRIC, t, value);
			break;
		case DYN_SLIDING_FRICTION:
			pblock->SetValue( PB_SLIDFRIC, t, value);
			break;
		default:
			assert(0);
			break;
		}
	}

#define MTL_HDR_CHUNK 0x4000
#define STDMTL_FLAGS_CHUNK 0x5000
#define STDMTL_SHADING_CHUNK 0x5004
#define STDMTL_TEX_ONOFF_CHUNK 0x5002
#define STDMTL_TEX_AMT0 0x5100
#define STDMTL_TEX_AMT1 0x5101
#define STDMTL_TEX_AMT2 0x5102
#define STDMTL_TEX_AMT3 0x5103
#define STDMTL_TEX_AMT4 0x5104
#define STDMTL_TEX_AMT5 0x5105
#define STDMTL_TEX_AMT6 0x5106
#define STDMTL_TEX_AMT7 0x5107
#define STDMTL_TEX_AMT8 0x5108
#define STDMTL_TEX_AMT9 0x5109
#define STDMTL_TEX_AMTA 0x510A

//#define STDMTL_BUMP1_CHUNK 0x5200
#define STDMTL_VERS_CHUNK 0x5300
#define STDMTL_DIM_REFLECT 0x5400

#define STDMTL_VERSION  9 

// IO
IOResult StdMtl::Save(ISave *isave) { 
	ULONG nb;
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(STDMTL_FLAGS_CHUNK);
	isave->Write(&flags,sizeof(flags),&nb);			
	isave->EndChunk();

	isave->BeginChunk(STDMTL_SHADING_CHUNK);
	isave->Write(&shading,sizeof(shading),&nb);			
	isave->EndChunk();

	isave->BeginChunk(STDMTL_VERS_CHUNK);
	int version = STDMTL_VERSION;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	if (dimReflect) {
		isave->BeginChunk(STDMTL_DIM_REFLECT);
		isave->EndChunk();
		}
	return IO_OK;
	}		

class StdMtlCB: public PostLoadCallback {
	public:
		StdMtl *m;
		int loadVersion;
	    StdMtlCB(StdMtl *s, int loadVers) { m = s; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			m->OldVerFix(loadVersion);
			delete this; 
			} 
	};


IOResult StdMtl::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;
	IOResult res;
// .......... use this???
	iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,NUMOLDVER, &curVersion, this,0));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				ivalid.SetEmpty();
				break;
			case STDMTL_FLAGS_CHUNK:
				res = iload->Read(&flags,sizeof(flags), &nb);
				break;
			case STDMTL_SHADING_CHUNK:
				res = iload->Read(&shading,sizeof(shading), &nb);
				if (shading<0||shading>SHADE_BLINN) {
					shading = SHADE_PHONG;
					}
				SetShadingNoNotify(shading);
				break;
			case STDMTL_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
			case STDMTL_TEX_ONOFF_CHUNK:
				{
				ULONG f;
				res = iload->Read(&f,sizeof(f), &nb);
				for (int i=0; i<NTEXMAPS; i++) 
				    maps->txmap[i].mapOn = (f&(1<<i))?1:0;
				}
				break;
			case STDMTL_DIM_REFLECT:
				dimReflect = TRUE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	if (version<9) {
		iload->RegisterPostLoadCallback(new StdMtlCB(this,version));
		iload->SetObsolete();
		}

	return IO_OK;
                                                      
	}

// Composite  c_over on top of c. Assume c_over has pre-multiplied
// alpha.
inline void AlphaCompCol(Color& c,  RGBA c_over) {
	float ia = 1.0f - c_over.a;
	c.r = c.r*ia + c_over.r;
	c.g = c.g*ia + c_over.g;
	c.b = c.b*ia + c_over.b;
	} 

static inline float Intens(RGBA& c) {	return (c.r+c.g+c.b)/3.0f;	}

static Color blackCol(0.0f,0.0f,0.0f);
static Color whiteCol(1.0f,1.0f,1.0f);


#define MAX3(a,b,c) ((a)>(b)?((a)>(c)?(a):(c)):((b)>(c)?(b):(c)))

static inline float MaxRGB(Color c) { 	return MAX3(c.r,c.g,c.b); }

Color StdMtl::TranspColor(ShadeContext& sc, float opac, Color& diff) {
	// Compute the color of the transparent filter color
	if (flags&STDMTL_ADD_TRANSP) {
		float f = 1.0f - opac;
		return Color(f,f,f);   //XXX
		}
	else 
		{
		// Transparent Filter color mapping
		if (flags&STDMTL_FILT_TRANSP) {
			Color filt = filter;
			if (MAPACTIVE(ID_FI)) 
				AlphaCompCol(filt,(*maps)[ID_FI].Eval(sc)); 
#ifdef OLDFILTER
			if (opac>0.5f) {
				// lerp (filt --> black) as opac goes ( 0.5--> 1.0)
				float s= 2.0f*(opac-.5f);
				return (1.0f-s)*filt;
				}
			else {
				// lerp (white --> filt) as opac goes (0.0 --> .5 )
				float s = 2.0f*opac;
				return (1.0f-s) + filt*s;
				}
#else 
			if (opac>0.5f) {
				// darken as opac goes ( 0.5--> 1.0)
				// so that max component reaches 0.0f when opac reaches 1.0
				// find max component of filt
				float m = (filt.r>filt.g)?filt.r:filt.g;
				if(filt.b>m) m = filt.b;
				Color fc;
				float d = 2.0f*(opac-.5f)*m;
				fc = filt-d;
				if (fc.r<0.0f) fc.r = 0.0f;
				if (fc.g<0.0f) fc.g = 0.0f;
				if (fc.b<0.0f) fc.b = 0.0f;
				return fc;
				}
			else {
				// lighten as opac goes ( 0.5--> 0.0)
				// so that min component reaches 1.0f when opac reaches 1.0
				// find min component of filt
				float m = (filt.r<filt.g)?filt.r:filt.g;
				if(filt.b<m) m = filt.b;
				Color fc;
				float d = (1.0f-2.0f*opac)*(1.0f-m);
				fc = filt+d;
				if (fc.r>1.0f) fc.r = 1.0f;
				if (fc.g>1.0f) fc.g = 1.0f;
				if (fc.b>1.0f) fc.b = 1.0f;
				return fc;
				}
#endif
			}
		else {
			Color f = (1.0f-diff);  // original 3DS transparency 
			return  (1.0f-opac)*f;
			}
		}
	}



//----------------------------------------------------------------------------------------
//- Phong Shader  ------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


class Phong: public SShader {
	float fs;
	float shin_str;
	public:
	void Illum(ShadeContext &sc, SIllumParams &ip);
	void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.spec; };
	void SetShininess(float shininess, float shineStr) {
		fs = (float)pow(2.0,shininess*10.0);
		shin_str = shineStr;
		}
	float EvalHilite(float x) {
		return shin_str*(float)pow((double)cos(x*PI),(double)fs);  
		}
	};

static void whoa(){}

void Phong::Illum(ShadeContext &sc, SIllumParams &ip) {
	LightDesc *l;
	Color lightCol;
	BOOL is_shiny;
	Point3 R;
	if (is_shiny=(ip.sh_str>0.0f)) 
    R = sc.ReflectVector();

//	IPoint2 sp = sc.ScreenCoord();
//	if (sp.x==260&&sp.y==180)
//		whoa();
	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) {
			if (l->ambientOnly) {
				ip.ambIllum += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;
			// diffuse
			if (l->affectDiffuse)
				ip.diffIllum += diffCoef*lightCol;
			if (is_shiny&&l->affectSpecular) {
				// specular (Phong) 
				float c = DotProd(L,R);
				if (c>0.0f) {
					if (ip.softThresh!=0.0&&diffCoef<ip.softThresh) {
						float r = diffCoef/ip.softThresh;
						c *= Soften(r);
						}
					c = (float)pow((double)c, (double)ip.ph_exp); // could use table lookup for speed
					ip.specIllum += c*ip.sh_str*lightCol;
					}
				}
			}
		}
	ip.specIllum *= ip.spec; 
	}


//----------------------------------------------------------------------------------------
//- Blinn Shader  ------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class Blinn: public SShader {
	float fs;
	float shin_str;
	public:
	void Illum(ShadeContext &sc, SIllumParams &ip);
	void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.spec; };
	void SetShininess(float shininess, float shineStr) {
		fs = (float)pow(2.0,shininess*10.0);
		shin_str = shineStr;
		}
	float EvalHilite(float x) {
		return shin_str*(float)pow((double)cos(x*PI),(double)fs);  
		}
	};


void Blinn::Illum(ShadeContext &sc, SIllumParams &ip) {
	LightDesc *l;
	Color lightCol;

	// Blinn style phong
	BOOL is_shiny=(ip.sh_str>0.0f)?1:0; 
	double ph_exp = double(ip.ph_exp)*4.0; // This is to make the hilite compatible with normal phong
	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
		if (l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) {
			if (l->ambientOnly) {
				ip.ambIllum += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;

			// diffuse
			if (l->affectDiffuse)
				ip.diffIllum += diffCoef*lightCol;

			// specular (Phong) 
			if (is_shiny&&l->affectSpecular) {
				Point3 H = FNormalize(L-ip.V);
				float c = DotProd(ip.N,H);	 
				if (c>0.0f) {
					if (ip.softThresh!=0.0&&diffCoef<ip.softThresh) {
						c *= Soften(diffCoef/ip.softThresh);
						}
					c = (float)pow((double)c, ph_exp); // could use table lookup for speed
					ip.specIllum += c*ip.sh_str*lightCol;
					}
				}
 			}
		}
	ip.specIllum *= ip.spec; 
	}

//----------------------------------------------------------------------------------------
//- Metal Shader  ------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class Metal: public SShader {
	float fm2inv, fshin_str;
	public:
	void Illum(ShadeContext &sc, SIllumParams &ip);
	void AffectReflMap(ShadeContext &sc, SIllumParams &ip, Color &rcol) { rcol *= ip.diff; };
	void SetShininess(float shininess, float shineStr);
	float EvalHilite(float x);
	};

float CompK(float f0) {
	return float(2.0*sqrt(f0)/sqrt(1.0-f0));
	}

float fres_metal(float c, float k) {
	float b,rpl,rpp,c2;
	b = k*k + 1.0f;
	c2 = c*c;
	rpl = (b*c2-2*c+1)/(b*c2+2*c+1);
	rpp = (b-2*c+c2)/(b+2*c+c2);
	return(.5f*(rpl+rpp));
	}

void Metal::Illum(ShadeContext &sc, SIllumParams &ip) {
	LightDesc *l;
	Color lightCol;
	BOOL gotKav = FALSE;
	float kav, fav0, m2inv,NV;
	
	//IPoint2 sp = sc.ScreenCoord();

	BOOL is_shiny;
	if (ip.sh_str>0.0f) {
		NV = -DotProd(ip.N,ip.V);  // N dot V: view vector is TOWARDS us.
		is_shiny = 1;
		float r = 1.0f-ip.shine;
		if (r==0.0f) r = .00001f;
		m2inv = 1.0f/(r*r);  
		}
	else 
		is_shiny = 0;
	

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;

		if (!l->Illuminate(sc,ip.N,lightCol,L,NL,diffCoef)) 
			continue;

		if (l->ambientOnly) {
			ip.ambIllum += lightCol;
			continue;
			}

		// diffuse
		if (NL>0.0f&&l->affectDiffuse)  // TBD is the NL test necessary?
			ip.diffIllum += diffCoef*lightCol;

		if (is_shiny&&l->affectSpecular) { // SPECULAR 
			Color fcol;
			float LH,NH,VH;
		    float sec2;  // Was double?? TBD
			Point3 H;
	
			if (NV<0.0f) continue;

			H = FNormalize(L-ip.V);

			LH = DotProd(L,H);  // cos(phi)   
			NH = DotProd(ip.N,H);  // cos(alpha) 
			if (NH==0.0f) continue;
			VH = -DotProd(ip.V,H);

			// compute geometrical attenuation factor 
			float G = (NV<NL)? (2.0f*NV*NH/VH): (2.0f*NL*NH/VH);
			if (G>0.0f) {
				// Compute (approximate) indices of refraction
				//	this can be factored out for non-texture-mapped mtls
				if (!gotKav) {
					fav0 = Intens(ip.diff);
					if (fav0>=1.0f) fav0 = .9999f;
					kav = CompK(fav0);	
					gotKav = TRUE;
					}

				float fav = fres_metal(LH,kav);
				float t = (fav-fav0)/(1.0f-fav0);
				fcol = (1.0f-t)*ip.diff + Color(t,t,t);

				// Beckman distribution  (from Cook-Torrance paper)
				sec2 = 1.0f/(NH*NH);  // 1/sqr(cos) 
				float D = (.5f/PI)*sec2*sec2*m2inv*(float)exp((1.0f-sec2)*m2inv);  					
				if (G>1.0f) G = 1.0f;
				float Rs = ip.sh_str*D*G/(NV+.05f);	
				ip.specIllum += fcol*Rs*lightCol;
				}
			} 
		}
	ip.diffIllum *= 1.0f - ip.sh_str;
	}


void Metal::SetShininess(float shininess, float shineStr) {
	float r = 1.0f-shininess;
	if (r==0.0f) r = .00001f;
	fm2inv = 1.0f/(r*r);  
	fshin_str = shineStr;
	}
			
float Metal::EvalHilite(float x) {
	float c = (float)cos(x*PI);
	float sec2 = 1.0f/(c*c);	  /* 1/sqr(cos) */
	return fshin_str*(.5f/PI)*sec2*sec2*fm2inv*(float)exp((1.0f-sec2)*fm2inv);  					
	}

//-------------------------------------------------------------------------


static Phong phongShader;
static Blinn blinnShader;
static Metal metalShader;

static SShader *shaders[4] = {
	&phongShader, // CONST
	&phongShader,
	&metalShader,
	&blinnShader
	};

static SShader *GetShaderFromId(int s) { return shaders[s]; };

#define DOMAP(i) (sc.doMaps&&(*maps)[i].IsActive())

void StdMtl::Shade(ShadeContext& sc) {
	SIllumParams ip;
	Color lightCol,rescol, diffIllum0;
	float opac;
	RGBA mval;
	Point3 N0,P;
	BOOL bumped = FALSE;
	if (gbufID) sc.SetGBufferID(gbufID);

	ip.flags = flags;
	ip.softThresh = softThresh;
	ip.diffIllum = blackCol;
	ip.specIllum = blackCol;
	ip.ambIllum  = blackCol;
	ip.amb = ambient;
	ip.diff = diffuse;
	ip.spec = specular;

	opac =  opacity;

	if (sc.mode==SCMODE_SHADOW) {
		// Opacity mapping;
		if (DOMAP(ID_OP)) 
			opac = (*maps)[ID_OP].LerpEvalMono(sc,opac);

		// "Shadow mode": This just computes the transparency, which is all 
		// you need for shadowing.
		if (opac!=1.0f||opfall!=0.0f) {
			if (opfall!=0.0f) {	
				ip.N = (shading==SHADE_CONST)?sc.GNormal():sc.Normal();
				ip.V = sc.V();  // get unit view vector
				float d = (float)fabs(DotProd(ip.N,ip.V));
				if (flags&STDMTL_FALLOFF_OUT) d = 1.0f-d;
				opac *= (1.0f-opfall*d);
				}

		 	// Transparency may use diffuse color mapping
			if ((flags&STDMTL_ADD_TRANSP|STDMTL_FILT_TRANSP)==0) {
				if (DOMAP(ID_DI)) {
					mval = (*maps)[ID_DI].Eval(sc);
				    AlphaCompCol(ip.diff,mval); 
					}
				}
			// Compute the transpareny color
			sc.out.t = TranspColor(sc, opac, ip.diff);
			}
		else 
			sc.out.t = blackCol;
		return;
		}

	N0 = ip.N = sc.Normal();

	if (shading==SHADE_CONST) {
		ip.N = sc.GNormal();
		bumped = TRUE;
		sc.SetNormal(ip.N);
		}

	P = sc.P();
	ip.V = sc.V();  // get unit view vector

	// Do texture mapping

	// Bump mapping: Do this FIRST so other maps can use the perturbed normal
	if (DOMAP(ID_BU)) {
		Point3 dn = (*maps)[ID_BU].EvalNormalPerturb(sc);
		bumped = TRUE;
		ip.N = FNormalize(ip.N + (sc.backFace?-dn:dn));
		sc.SetNormal(ip.N);
		}

	// Diffuse color mapping
	if (DOMAP(ID_DI)) {
		mval = (*maps)[ID_DI].Eval(sc);
	    AlphaCompCol(ip.diff,mval); 
		if (flags&STDMTL_LOCK_ADTEX) 
		    AlphaCompCol(ip.amb,mval); 
		}

 	// Ambient color mapping
	if (!(flags&STDMTL_LOCK_ADTEX)) {
		if (DOMAP(ID_AM)) {
			AlphaCompCol(ip.amb, (*maps)[ID_AM].Eval(sc)); 
			}	
		}

	// Specular color mapping
	if (DOMAP(ID_SP)) 
	    AlphaCompCol(ip.spec,(*maps)[ID_SP].Eval(sc)); 

	// Opacity mapping;
	if (DOMAP(ID_OP)) {
		opac = (*maps)[ID_OP].LerpEvalMono(sc,opac);
		}

	// Shininess mapping
	if (DOMAP(ID_SH)) {
		//ip.shine = shininess*(*maps)[ID_SH].EvalMono(sc); 
		ip.shine = (*maps)[ID_SH].LerpEvalMono(sc,shininess); 
		ip.ph_exp = (float)pow(2.0,ip.shine*10.0); // expensive.!!	TBD
		}
	else {
		ip.shine = shininess;
		ip.ph_exp = phongexp;
		}

	// Shininess strength mapping
	ip.sh_str = (DOMAP(ID_SS))? (*maps)[ID_SS].LerpEvalMono(sc,shine_str): shine_str; 

	// Self illumination mapping
	float self_i = (DOMAP(ID_SI))? (*maps)[ID_SI].LerpEvalMono(sc,self_illum) : self_illum;

	curShader->Illum(sc,ip);

	diffIllum0 = ip.diffIllum; // save this for reflection dimming

	// Apply self illumination
	if (self_i>0.0f) {
		ip.diffIllum = (self_i>=1.0f) ?  whiteCol: ip.diffIllum*(1.0f-self_i) + self_i;
		}

	ip.diffIllum = ip.amb*(sc.ambientLight+ip.ambIllum) +  ip.diff*ip.diffIllum;		

	if (DOMAP(ID_RR)) {
		// Set up for opacity for Refraction map.
		opac *= 1.0f-(*maps)[ID_RR].amount;   

		// Make more opaque where specular hilite occurs:
		float a_s = MaxRGB(ip.specIllum);
		if (a_s>1.0f) a_s = 1.0f; 
		sc.out.c = opac*ip.diffIllum + ip.specIllum;
	   	opac = opac + a_s - opac*a_s;

		// Evaluate refraction map, filtered by filter color.
		Texmap *refrmap = (*maps)[ID_RR].map;
		AColor rcol;
		sc.SetIOR(ioRefract);
		if (refrmap->HandleOwnViewPerturb()) 
			rcol = refrmap->EvalColor(sc);
		else  
			rcol = sc.EvalEnvironMap(refrmap, sc.RefractVector(ioRefract));
		sc.out.c += Color(rcol.r,rcol.g,rcol.b)*TranspColor(sc, opac, ip.diff);
		sc.out.t.Black();  // no transparency when doing refraction
		}
	else {
		if (opac!=1.0f||opfall!=0.0f) {
			if (opfall!=0.0f) {	
				float d = (float)fabs(DotProd(ip.N,ip.V));
				if (flags&STDMTL_FALLOFF_OUT) d = 1.0f-d;
				opac *= (1.0f-opfall*d);
				}
			
			// Make more opaque where specular hilite occurs, so you
			// can still see the hilite:
			float a_s = MaxRGB(ip.specIllum);
			if (a_s>1.0f) a_s = 1.0f; 
			sc.out.c = opac*ip.diffIllum + ip.specIllum;
		   	opac = opac + a_s - opac*a_s;

			// Compute the color of the transparent filter color
			sc.out.t = TranspColor(sc, opac, ip.diff);
			}
		else {
			sc.out.t = blackCol;
			sc.out.c = ip.diffIllum + ip.specIllum;
			}
		}

	// Evaluate reflection map.
	if (DOMAP(ID_RL)) {
		AColor rcol;
		Texmap *reflmap = (*maps)[ID_RL].map;
		if (reflmap->HandleOwnViewPerturb()) {
			sc.TossCache(reflmap);
			rcol = reflmap->EvalColor(sc);
			}
		else 
			rcol = sc.EvalEnvironMap(reflmap, sc.ReflectVector());
		Color rc(rcol.r,rcol.g,rcol.b);
		curShader->AffectReflMap(sc,ip,rc);
		float r = rcol.a*(*maps)[ID_RL].amount;
		if (dimReflect) {
			float dimfact = ((1.0f-dimIntens)*Intens(diffIllum0)*dimMult + dimIntens);
			r *= dimfact;
			}
		sc.out.c += (rc*r);
		}
	if (bumped) sc.SetNormal(N0);
	}

float StdMtl::EvalDisplacement(ShadeContext& sc) {
	if (DOMAP(ID_DP))
		return (*maps)[ID_DP].EvalMono(sc);
	else return 0.0f; 
	}

Interval StdMtl::DisplacementValidity(TimeValue t) {
	if (MAPACTIVE(ID_DP)) { 
		Interval iv;
		iv.SetInfinite();
		maps->txmap[ID_DP].Update(t,iv);
		return iv;
		}
	else 
		return FOREVER;
	}




