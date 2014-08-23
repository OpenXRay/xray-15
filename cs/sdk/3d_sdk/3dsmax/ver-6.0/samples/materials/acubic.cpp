/**********************************************************************
 *<
	FILE: ACUBIC.CPP

	DESCRIPTION: Cubic Reflection/Refraction map.

	CREATED BY: Dan Silva

	HISTORY:
				Update 11/17 to param block2 by Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include <bmmlib.h>
#include "render.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "notify.h"
#include "buildver.h"

#ifndef NO_MAPTYPE_REFLECTREFRACT // orb 01-03-2001 Removing map types

static TCHAR *suffixes[6] = { 
	_T("_UP"),
	_T("_DN"),
	_T("_LF"),
	_T("_RT"),
	_T("_FR"),
	_T("_BK"),
	};

// Define this to display the rendered maps:
//#define DBG

extern HINSTANCE hInstance;

static Class_ID acubicClassID(ACUBIC_CLASS_ID,0);

//#define PB_SIZE	    0
//#define PB_BLUR	    1
//#define PB_BLUROFF  2
//#define PB_NEAR     3
//#define PB_FAR      4

enum { acubic_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	acubic_size, acubic_blur, acubic_bluroffset, acubic_near, acubic_far,
	acubic_source, acubic_useatmospheric, acubic_applyblur,
	acubic_frametype, acubic_nthframe,
	acubic_bitmap_names,
	acubic_outputname,
//	acubic_camera,
//	acubic_node

};

#if 0
static TCHAR *meditNames[6] = {
	_T("MAX_UP.JPG"),
	_T("MAX_DN.JPG"),
	_T("MAX_LF.JPG"),
	_T("MAX_RT.JPG"),
	_T("MAX_FR.JPG"),
	_T("MAX_BK.JPG")
	};
#endif

static TCHAR *getCubeName(int i)  {
	switch(i) {
		case 0: return GetString(IDS_DS_UP);  
		case 1: return GetString(IDS_DS_DOWN);  
		case 2: return GetString(IDS_DS_LEFT);  
		case 3: return GetString(IDS_DS_RIGHT);  
		case 4: return GetString(IDS_DS_FRONT);  
		default: return GetString(IDS_DS_BACK);  
		}
	}


Matrix3 TMForView( int i);

//---------------------------------------------------
class EnvSampler {
	public:
		virtual	BMM_Color_64 sample(Point3 n)=0;
	};

static void whoa(){};

void BuildSphereMap(Bitmap *bm, EnvSampler& env) {
	FLOAT dv,du,u,v,phi,cphi,theta;;
	int iu,iv;
	Point3 n;
	int w = bm->Width();
	int h = bm->Height();
	PixelBuf l64(w);
	BMM_Color_64 *pb=l64.Ptr();

	dv = 1.0f/(float)h;
	du = 1.0f/(float)w;
	for (iv = 0,v=0.0f; iv<h; iv++,v+=dv) {
		phi = PI*(.5f - v);
		cphi = (float)cos(phi);
		n.z =  (float)sin(phi);
		for (iu=0,u=0.0f; iu<w; iu++,u+=du) {
			/* compute normal in direction for iv,iu */
			theta = TWOPI*(u-.5f);
			//if (P.reflip) theta = -theta;
			n.y = (float)sin(theta)*cphi;
			n.x = (float)cos(theta)*cphi;

			pb[iu] = env.sample(n);
			}
		bm->PutPixels(0,iv, w, pb);
		}

#ifdef DBG
	bm->Display(_T("Cubic Test"), BMM_UR);
	MessageBox(NULL, _T("The spherical map"), _T(" Auto Cubic Test"), MB_OK|MB_ICONEXCLAMATION);
	bm->UnDisplay();
#endif
	}

//---------------------------------------------------

inline float FMax(float a, float b) { return a>b?a:b; }

void ComputeSphereCoords(Point3 n, Point2& t, Point2& d) {
	t.x = 0.5f + (float)atan2(n.y,n.x)/TWOPI;
	t.y = 0.5f - (float)asin(n.z)/PI;
	float dd = FMax((float)fabs(n.x),(float)fabs(n.y))/(1.0f-n.z*n.z+.1f)/TWOPI;
	d.x = d.y = dd;
	}


//---------------------------------------------------
/*
class ACubic;

class ACubicDlg: public ParamDlg, public  PickObjectProc, public RendProgressCallback, public DADMgr{
	public:
		HWND hwmedit;	 	// window handle of the materials editor dialog
		IMtlParams *ip;
		ACubic *theTex;	// current ACubic being edited.
		HWND hPanel; 		// Rollup pane
		ICustButton *iPick,*iCamPick;
		ISpinnerControl *sizeSpin,*blurSpin,*nthSpin,*blurOffSpin,*nearSpin,*farSpin;
		ICustButton *iFile[6];
		TimeValue curTime; 
		int isActive;
		BOOL valid;
		BOOL camPick;
		BOOL abortRender;

		//-----------------------------
		ACubicDlg(HWND hwMtlEdit, IMtlParams *imp, ACubic *m); 
		~ACubicDlg();
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
		void LoadDialog(BOOL draw);  // stuff params into dialog
		void UpdateMtlDisplay() { ip->MtlChanged(); }
		void ReloadDialog();
		void ActivateDlg(BOOL onOff) {}
		void BrowseInFile(int id);
		void BrowseOutFile();
		void StuffDlgName(int i);
		void StuffOutName();
		void MaybeEnablePick();
		void Invalidate() { valid = FALSE;	InvalidateRect(hPanel,NULL,0); }
		BOOL KeyAtCurTime(int id);
		void Destroy(HWND hWnd) { 
			ReleaseICustButton(iPick);	
			iPick = NULL; 
			ReleaseICustButton(iCamPick);	
			iCamPick = NULL;
			for (int i=0; i<6; i++) {
				ReleaseICustButton(iFile[i]);
				iFile[i] = NULL;	
				}
			}
		void SetUseFile(BOOL onOff);
		BOOL CheckWindowMessages(HWND hWnd);
		int FindFileFromHWND(HWND hw);


		// methods inherited from ParamDlg:
		Class_ID ClassID() {return acubicClassID;  }
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing() { return (ReferenceTarget *)theTex; }
		void DeleteThis() { delete this;  }	
		void SetTime(TimeValue t);

		// From PickObjectProc
		BOOL Pick(INode *node);
		BOOL Filter(INode *node);
		void EnterMode() { if (camPick) iCamPick->SetCheck(TRUE); else 	iPick->SetCheck(TRUE);	}
		void ExitMode() { if (camPick) iCamPick->SetCheck(FALSE); else 	iPick->SetCheck(FALSE);	}
		BOOL AllowMultiSelect() { return FALSE;}

		// From RendProgressCallback
		void SetTitle(const TCHAR *title){};
		int Progress(int done, int total);

		// DADMgr methods
		// called on the draggee to see what if anything can be dragged from this x,y
		SClass_ID GetDragType(HWND hwnd, POINT p) { return BITMAPDAD_CLASS_ID; }
		// called on potential dropee to see if can drop type at this x,y
		BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {
			if (hfrom==hto) return FALSE;
			return (type==BITMAPDAD_CLASS_ID)?1:0;
			}
		int SlotOwner() { return OWNER_MTL_TEX;	}
	    ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
		void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
		BOOL  LetMeHandleLocalDAD() { return 0; } 
	};

*/

//--------------------------------------------------------------
// CubicMap 
//--------------------------------------------------------------
#define VIEW_UP 0
#define VIEW_DN	1
#define VIEW_LF	2
#define VIEW_RT 3
#define VIEW_FR	4
#define VIEW_BK	5

class CubicMap {
	public:
		Bitmap *bitmap[6];
		Bitmap *blurmap;
		int size;
		CubicMap *next;
		int nodeID;
		BOOL tossMapsWhenBlur;
		TimeValue mapTime; // when maps were last rendered
	// Methods
		CubicMap();
		~CubicMap();
		void FreeMaps();
		void FreeCubicMaps();
		int AllocMaps(int sz);
		BMM_Color_64 Sample(Point3 dir);
		int BuildBlurMap();
		void FreeBlurMap();
	};	  

CubicMap::CubicMap() { 
	for (int i=0; i<6; i++) 
		bitmap[i] = NULL; 
	blurmap = NULL;
	next = NULL;
	size = 0; 
	tossMapsWhenBlur = TRUE;
	}

CubicMap::~CubicMap() { 
	FreeMaps();
	}

void CubicMap::FreeCubicMaps() {
	for (int i=0; i<6; i++) {  
		if (bitmap[i]) {
			bitmap[i]->DeleteThis(); 
			bitmap[i] = NULL;
			}
		}
	}

void CubicMap::FreeMaps() {
	FreeCubicMaps();
	FreeBlurMap();
	}

int CubicMap::AllocMaps(int sz) {
	if (bitmap[0]&&size==size)
		return 1;
	FreeMaps();
	BitmapInfo bi;
	bi.SetName(_T(""));
	bi.SetWidth(sz);
	bi.SetHeight(sz);
	bi.SetType(BMM_TRUE_32);
	size = sz;
	for (int i=0; i<6; i++) {
		if (NULL == (bitmap[i] = TheManager->Create(&bi)))
			return 0;
		}
	return 1;
	}


class CubeSampler: public EnvSampler {
	CubicMap *cm;
	public:
		CubeSampler(CubicMap *c) { cm = c; }
		BMM_Color_64 sample(Point3 n){ return cm->Sample(n);	}
	};

void  CubicMap::FreeBlurMap() {
	if (blurmap) {
		blurmap->DeleteThis();
		blurmap = NULL;
		}
	}

int CubicMap::BuildBlurMap() {
	if (bitmap[0]==NULL)
		return 1;
	if (!(blurmap&&blurmap->Width()==4*size && blurmap->Height()==2*size)) {
		BitmapInfo bi;
		bi.SetName(_T(""));
		bi.SetWidth(4*size);
		bi.SetHeight(2*size);
		bi.SetType(BMM_TRUE_32);
		if (blurmap) blurmap->DeleteThis();
		blurmap = TheManager->Create(&bi);
		}
	if (blurmap) {
		CubeSampler cs(this);
		BuildSphereMap(blurmap, cs);
		if (tossMapsWhenBlur)
			FreeCubicMaps(); 
		blurmap->SetFilter(BMM_FILTER_PYRAMID);  
		BitmapFilter *filt = blurmap->Filter();
		if (filt)
			filt->MakeDirty();  // so filter gets recomputed for each frame
		}
	return 1;
	}

//--------------------------------------------------------------
// ACubic: 
//--------------------------------------------------------------
#define NUM_SRC_FILES 6 // mjm - 2.3.99

class ACubic: public StdCubic { 
	friend class ACubicPostLoad;
	friend class ACubicDlgProc;
	int size;
	int nth;
	float blur;
	float blurOff;
	float nearRange,farRange;
//	CRITICAL_SECTION csect;
	BOOL applyBlur;
	BOOL do_nth;
	BOOL useEnvMap;
	BOOL inMedit;
	BitmapInfo biInFile;			// for reading cubic map files
	BitmapInfo biOutFile;			// for rendering cubic map.
	TSTR fileNames[NUM_SRC_FILES];	// source files
	CubicMap *maps;					// when Automatic mode
	Interval ivalid;
	int rollScroll;



//	ACubicDlg *paramDlg;
	public:
		BOOL useFile;
		IMtlParams *ip;
		IParamBlock2 *pblock;  // ref #1
		ACubic();
		~ACubic() {	
//			DeleteCriticalSection(&csect); 
			}
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		// From StdCubic
		void SetSize(int n, TimeValue t);
		void SetDoNth(BOOL onoff) { do_nth = onoff; }
		void SetNth(int n) { nth = n; }
		void SetApplyBlur(BOOL onoff);
		void SetBlur(float b, TimeValue t);
		void SetBlurOffset(float b, TimeValue t);
		void UseHighDynamicRange(BOOL onoff);
		int IsHighDynamicRange( ) const;
		int GetSize(TimeValue t){ return pblock->GetInt(acubic_size,t); }
		BOOL GetDoNth() { return do_nth; }
		int GetNth() { return nth; }
		BOOL GetApplyBlur() { return applyBlur; }
		float GetBlur(TimeValue t) { return pblock->GetFloat(acubic_blur,t); }
		float GetBlurOffset(TimeValue t){ return pblock->GetFloat(acubic_bluroffset,t); }
		int  GetOutFileName(TSTR& fullname, TSTR &fname, int i);
		int WriteBM(Bitmap *bm, TCHAR *name);
		void RenderCubicMap(INode *node);
		void SetUseFile(BOOL onOff);
		float GetNearRange(TimeValue t) { return pblock->GetFloat(acubic_near,t); }
		float GetFarRange(TimeValue t) { return pblock->GetFloat(acubic_far,t); }
	    void SetNearRange(float v, TimeValue t);
	    void SetFarRange(float v, TimeValue t);

		void NotifyChanged();
	
		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);

		// optimized evaluation for monochrome use
		float EvalMono(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		ULONG LocalRequirements(int subMtlNum) { 
			ULONG req = MTLREQ_VIEW_DEP;  // DS 10/18/99 so viewport will update when rotate interactively. (211026)
			if (!useFile)
				req |= MTLREQ_AUTOREFLECT;	
			return req;
			}

		int LoadMapFiles(TimeValue t);
		int BuildMaps(TimeValue t, RenderMapsContext &rmc);
		int DoThisFrame(TimeValue t, BOOL fieldRender, TimeValue mapTime);
		CubicMap *FindMap(int nodeNum);
		void FreeMaps();
		int RenderBegin(TimeValue t, ULONG flags) {  return 1;}
		int RenderEnd(TimeValue t) { if (!useFile) FreeMaps(); return 1; }

		Class_ID ClassID() {	return acubicClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_ACUBIC_NAME); }  
		void DeleteThis() { delete this; }	

		int NumSubs() {return 1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) {
			if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/21/03
			if (useFile) {
				for (int i=0; i<6; i++) {
					biInFile.SetName(fileNames[i]);
					biInFile.EnumAuxFiles(nameEnum,flags);
					}
				}
			StdCubic::EnumAuxFiles( nameEnum, flags ); // LAM - 4/21/03
			}
		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		void UpdateInterface(BOOL on);	

		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc );
};



class ACubicClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new ACubic; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_ACUBIC_NAME_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return acubicClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_ENV;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("reflectRefract"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static ACubicClassDesc acubicCD;

ClassDesc* GetACubicDesc() { return &acubicCD;  }


/*
BOOL ACubicDlg::Filter(INode *node) { 
	if (!camPick)
		return TRUE; 
	ObjectState os = node->EvalWorldState(ip->GetTime());
	return os.obj->SuperClassID()==CAMERA_CLASS_ID;
	}

BOOL ACubicDlg::Pick(INode *node)
	{
	if (node) {
		if (camPick) {
			ObjectState os = node->EvalWorldState(ip->GetTime());
			if (os.obj->SuperClassID()==CAMERA_CLASS_ID) {
				CameraState cs;
				Interval iv;
				CameraObject *cam = (CameraObject *)os.obj;
				cam->EvalCameraState(ip->GetTime(),iv,&cs);
				theTex->SetNearRange(cs.nearRange,ip->GetTime());
				theTex->SetFarRange(cs.farRange,ip->GetTime());
				nearSpin->SetValue(theTex->nearRange,FALSE);
				farSpin->SetValue(theTex->farRange,FALSE);
				}
			}
		else {
			theTex->RenderCubicMap(node);
			}
		}
	return TRUE;
	}



//--------------------------------------------------------------

static INT_PTR CALLBACK  PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	ACubicDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (ACubicDlg*)lParam;
		theDlg->hPanel = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (ACubicDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = 1;
	int	res = theDlg->PanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

ACubicDlg::ACubicDlg(HWND hwMtlEdit, IMtlParams *imp, ACubic *m) { 
	hwmedit = hwMtlEdit;
	ip = imp;
	hPanel = NULL;
	theTex = m; 
	isActive = 0;
	valid = FALSE;
	iPick = iCamPick = NULL;
	for (int i=0; i<6; i++)
		iFile[i] = NULL;
	hPanel = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_AUTO_CUBIC),
		PanelDlgProc, 
		GetString(IDS_DS_ACUBIC_PARAMS), 
		(LPARAM)this );		
	curTime = imp->GetTime();
	}

void ACubicDlg::ReloadDialog() {
	Interval valid;
	theTex->Update(curTime, valid);
	LoadDialog(FALSE);
	}

void ACubicDlg::SetTime(TimeValue t) {
	Interval valid;
	if (t!=curTime) {
		curTime = t;
		theTex->Update(curTime, valid);
		LoadDialog(FALSE);
		InvalidateRect(hPanel,NULL,0);
		}
	}



void ACubicDlg::SetUseFile(BOOL onOff) {
	theTex->SetUseFile(onOff);
	CheckRadioButton( hPanel, IDC_CUBESRC_AUTO,IDC_CUBESRC_FILE, IDC_CUBESRC_AUTO+theTex->useFile);

	// AUTO STUFF
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_AUTO_GRP), !onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_FIRST_ONLY), !onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_EVERY_NTH), !onOff);
	if (onOff) nthSpin->Disable(); else nthSpin->Enable();

	//FILE STUFF
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_RELOAD), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_FILE_GRP), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_FILE_GRP2), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_OUTFILE), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_OUTFILE_NAME), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_UP), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_DN), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_LF), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_RT), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_BK), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_FR), onOff);

	for (int i=0; i<6; i++) 
		iFile[i]->Enable(onOff);
	MaybeEnablePick();
	}

ACubicDlg::~ACubicDlg() {
	ip->EndPickMode();
	theTex->paramDlg = NULL;
	ReleaseISpinner(sizeSpin);
	ReleaseISpinner(blurSpin);
	ReleaseISpinner(blurOffSpin);
	ReleaseISpinner(nthSpin);
	ReleaseISpinner(nearSpin);
	ReleaseISpinner(farSpin);
	SetWindowLongPtr(hPanel, GWLP_USERDATA, NULL);
	hPanel =  NULL;
	}

static int fileButtonID[6]={IDC_FILE_UP,IDC_FILE_DN,IDC_FILE_LF,IDC_FILE_RT,IDC_FILE_FR,IDC_FILE_BK};
static int fileNameID[6]={IDC_CUBE_UP,IDC_CUBE_DN,IDC_CUBE_LF,IDC_CUBE_RT,IDC_CUBE_FR,IDC_CUBE_BK};

static int SideFromID(int id) {
	for (int i=0; i<6; i++) if (fileButtonID[i]==id) return i;
	return 0;
	}

void ACubicDlg::MaybeEnablePick() {
	if (!theTex->useFile) {
		iPick->Disable(); 
		return;
		}
	if (_tcslen(theTex->biOutFile.Name())>0) 
		iPick->Enable(); 
	else 
		iPick->Disable();
	}


BOOL ACubicDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:
			{
			iPick = GetICustButton(GetDlgItem(hwndDlg,IDC_CUBE_PICK));
			iPick->SetType(CBT_CHECK);
			iPick->SetHighlightColor(GREEN_WASH);

			iCamPick = GetICustButton(GetDlgItem(hwndDlg,IDC_ACUBE_PICKCAM));
			iCamPick->SetType(CBT_CHECK);
			iCamPick->SetHighlightColor(GREEN_WASH);

			for (int i=0; i<6; i++) {
				iFile[i] = GetICustButton(GetDlgItem(hwndDlg,fileButtonID[i]));
				iFile[i]->SetDADMgr(this);
				}

			sizeSpin = SetupIntSpinner(hwndDlg, IDC_ACUBE_SIZE_SPIN, IDC_ACUBE_SIZE_EDIT,1,5000,100);
			blurSpin = SetupFloatSpinner(hwndDlg, IDC_ACUBE_BLUR_SPIN, IDC_ACUBE_BLUR_EDIT,0.0f,100.0f,1.0f,.01f);
			blurOffSpin = SetupFloatSpinner(hwndDlg, IDC_ACUBE_BLUROFF_SPIN, IDC_ACUBE_BLUOFF_EDIT,0.0f,1.0f,0.0f,.001f);
			nthSpin = SetupIntSpinner(hwndDlg, IDC_ACUBE_NTH_SPIN, IDC_ACUBE_NTH_EDIT,1,1000, 1);
			nearSpin = SetupFloatSpinner(hwndDlg, IDC_ACUBE_NEAR_SPIN, IDC_ACUBE_NEAR_EDIT,0.0f,10000.0f,0.0f,1.0f);
			farSpin = SetupFloatSpinner(hwndDlg, IDC_ACUBE_FAR_SPIN, IDC_ACUBE_FAR_EDIT,0.0f,10000.0f,500.0f,1.0f);
			CheckRadioButton( hwndDlg, IDC_FIRST_ONLY, IDC_EVERY_NTH, IDC_FIRST_ONLY+theTex->do_nth);
			SetCheckBox(hwndDlg, IDC_ACUBE_BLUR, theTex->applyBlur);
			SetCheckBox(hwndDlg, IDC_USE_ENVMAP, theTex->useEnvMap);
			for (i=0; i<6; i++) 
				StuffDlgName(i);
			StuffOutName();
			SetUseFile(theTex->useFile);
			MaybeEnablePick();
			return TRUE;
			}
			break;
		case WM_COMMAND:  
		    switch (id) {
				case IDC_FIRST_ONLY:
					theTex->do_nth = FALSE;
					break;
				case IDC_EVERY_NTH:
					theTex->do_nth = TRUE;
					break;
				case IDC_ACUBE_BLUR:
					theTex->SetApplyBlur(GetCheckBox(hwndDlg, id));			
					theTex->NotifyChanged();
					break;
				case IDC_USE_ENVMAP:
					theTex->useEnvMap = GetCheckBox(hwndDlg, id);			
					break;
				case IDC_CUBESRC_AUTO:
					SetUseFile(FALSE);
					theTex->NotifyChanged();
					break;
				case IDC_CUBESRC_FILE:
					SetUseFile(TRUE);
					theTex->NotifyChanged();
					break;
				case IDC_CUBE_PICK:
					camPick = FALSE;
					ip->SetPickMode(this); 
					break;
				case IDC_ACUBE_PICKCAM:
					camPick = TRUE;
					ip->SetPickMode(this); 
					break;
				case IDC_CUBE_RELOAD:
					theTex->FreeMaps();
					theTex->NotifyChanged();
					break;
				case IDC_FILE_UP:
				case IDC_FILE_DN:
				case IDC_FILE_LF:
				case IDC_FILE_RT:
				case IDC_FILE_FR:
				case IDC_FILE_BK:
					BrowseInFile(id);
					break;
				case IDC_CUBE_OUTFILE_NAME:
					BrowseOutFile();
					MaybeEnablePick();
					break;
				}
			break;
		case WM_PAINT: 	
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			break;
		case WM_CLOSE: 	break;       
		case WM_DESTROY:
			Destroy(hwndDlg);
			break;
		case CC_SPINNER_CHANGE: 
			if (!theHold.Holding()) theHold.Begin();
			switch (id) {
				case IDC_ACUBE_SIZE_SPIN: 
					theTex->SetSize(sizeSpin->GetIVal(),curTime); 	
					sizeSpin->SetKeyBrackets(KeyAtCurTime(PB_SIZE));
					break;
				case IDC_ACUBE_BLUR_SPIN: 
					theTex->SetBlur(blurSpin->GetFVal(),curTime); 	
					blurSpin->SetKeyBrackets(KeyAtCurTime(PB_BLUR));
					break;
				case IDC_ACUBE_BLUROFF_SPIN: 
					theTex->SetBlurOffset(blurOffSpin->GetFVal(),curTime); 	
					blurOffSpin->SetKeyBrackets(KeyAtCurTime(PB_BLUROFF));
					break;
				case IDC_ACUBE_NTH_SPIN: 
					theTex->nth = nthSpin->GetIVal(); 	
					break;
				case IDC_ACUBE_NEAR_SPIN: 
					theTex->SetNearRange(nearSpin->GetFVal(),curTime); 	
					nearSpin->SetKeyBrackets(KeyAtCurTime(PB_NEAR));
					break;
				case IDC_ACUBE_FAR_SPIN: 
					theTex->SetFarRange(farSpin->GetFVal(),curTime); 	
					farSpin->SetKeyBrackets(KeyAtCurTime(PB_FAR));
					break;
				}
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			theTex->NotifyChanged();
		    UpdateMtlDisplay();
			break;
		case CC_COMMAND:
			switch (code) {
				int i;
				case 8190:  /// DROP
					i = 10;
					break;
				}
			break;
    	}
	return FALSE;
	}


BOOL ACubicDlg::CheckWindowMessages(HWND hWnd)
	{
	MSG msg;
	Interface *iface = GetCOREInterface();
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
//		if (msg.message == WM_QUIT) {
//			PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
//			abortRender = TRUE;
//			return FALSE;
//			}
		// Escape key aborts render
		if (msg.message == WM_KEYDOWN&&msg.wParam==27) {
			PostMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
			abortRender = TRUE;
			return FALSE;
			}
		if (msg.message==WM_PAINT || msg.message==WM_PAINTICON) {
			iface->TranslateAndDispatchMAXMessage(msg);
			}
		}
	return TRUE;
	}

 
int  ACubicDlg::Progress(int done, int total) {
	if (!CheckWindowMessages(hPanel)) 
	   return RENDPROG_ABORT;
	else 
		return RENDPROG_CONTINUE;
	}

BOOL ACubicDlg::KeyAtCurTime(int id) { return theTex->pblock->KeyFrameAtTime(id,ip->GetTime()); }

void ACubicDlg::LoadDialog(BOOL draw) {
	if (theTex) {
		Interval valid;
		theTex->Update(curTime,valid);
		sizeSpin->SetValue(theTex->size,FALSE);
		blurSpin->SetValue(theTex->blur,FALSE);
		blurOffSpin->SetValue(theTex->blurOff,FALSE);
		nthSpin->SetValue(theTex->nth,FALSE);
		nearSpin->SetValue(theTex->nearRange,FALSE);
		farSpin->SetValue(theTex->farRange,FALSE);

		sizeSpin->SetKeyBrackets(KeyAtCurTime(PB_SIZE));
		blurSpin->SetKeyBrackets(KeyAtCurTime(PB_BLUR));
		blurOffSpin->SetKeyBrackets(KeyAtCurTime(PB_BLUROFF));
		nearSpin->SetKeyBrackets(KeyAtCurTime(PB_NEAR));
		farSpin->SetKeyBrackets(KeyAtCurTime(PB_FAR));
		
		CheckRadioButton( hPanel, IDC_FIRST_ONLY, IDC_EVERY_NTH, IDC_FIRST_ONLY+theTex->do_nth);
		StuffOutName();
		SetCheckBox(hPanel, IDC_ACUBE_BLUR, theTex->applyBlur);
		SetCheckBox(hPanel, IDC_USE_ENVMAP, theTex->useEnvMap);
		for (int i=0; i<6; i++) StuffDlgName(i);
		SetUseFile(theTex->useFile);
		MaybeEnablePick();
		}
	}

void ACubicDlg::SetThing(ReferenceTarget *m) {
	ip->EndPickMode();
	assert (m->ClassID()==acubicClassID);
	assert (m->SuperClassID()==TEXMAP_CLASS_ID);
	if (theTex) theTex->paramDlg = NULL;
	theTex = (ACubic *)m;
	if (theTex) theTex->paramDlg = this;
	LoadDialog(TRUE);
	}


void ACubicDlg::StuffDlgName(int i) {
	TSTR fname;
	TSTR fullName = theTex->fileNames[i];
	SplitPathFile(fullName,NULL,&fname);
	iFile[i]->SetText(fname);
	//SetDlgItemText(hPanel, fileButtonID[i], fname);
	}

void ACubicDlg::StuffOutName() {
	TSTR fname;
	TSTR fullName = theTex->biOutFile.Name();
	SplitPathFile(fullName,NULL,&fname);
	SetDlgItemText(hPanel, IDC_CUBE_OUTFILE_NAME, fname);
	}
 
void ACubicDlg::BrowseInFile(int id) {
	int iside = SideFromID(id);
	theTex->biInFile.SetName(theTex->fileNames[iside].data());
	BOOL silent = TheManager->SetSilentMode(TRUE);
	int res = TheManager->SelectFileInputEx(&theTex->biInFile,hPanel,getCubeName(iside));
	TheManager->SetSilentMode(silent);
	if (res) {
		TSTR fullName = theTex->biInFile.Name();
		TSTR path;
		TSTR file;
		TSTR ext;
		if (theTex->fileNames[iside]==fullName) 
			goto done;
		theTex->fileNames[iside] = fullName;
		StuffDlgName(iside);
		SplitFilename(fullName,&path, &file, &ext);
		int n = file.Length();
		if (n>=3) {
			TSTR suf = file.Substr(n-3,3);
			suf.toUpper();
			if (_tcscmp(suffixes[iside],suf)==0) {
				file = file.Substr(0,n-3);
				for (int i=0; i<6; i++) {
					fullName.printf(_T("%s\\%s%s%s"),path.data(),file.data(),suffixes[i],ext.data());
					theTex->fileNames[i] = fullName;
					StuffDlgName(i);
					if (theTex->useFile) {
						theTex->FreeMaps();
						theTex->NotifyChanged();
						}
					}
				}
			else {
				theTex->fileNames[iside] = fullName;
				StuffDlgName(iside);
				}
			}
		}
done:
	theTex->biInFile.SetName(_T(""));
	}



void ACubicDlg::BrowseOutFile() {
	BOOL silent = TheManager->SetSilentMode(TRUE);
	int res = TheManager->SelectFileOutput(&theTex->biOutFile,hPanel,GetString(IDS_DS_SELECT_UPFILE));
	TheManager->SetSilentMode(silent);
	if (res) {
		TSTR fullName = theTex->biOutFile.Name();
		TSTR path;
		TSTR file;
		TSTR ext;
		SplitFilename(fullName,&path, &file, &ext);
		int n = file.Length();
		if (n>=3) {
			TSTR suf = file.Substr(n-3,3);
			suf.toUpper();
			if (_tcscmp(suffixes[0],suf)!=0) {
				for (int i=1; i<6; i++) {
					if (_tcscmp(suffixes[i],suf)==0) {
						file = file.Substr(0,n-3);
						break;
						}
					}
				file += suffixes[0];
				}
			}
		else {
			file += suffixes[0];
			}
		fullName.printf(_T("%s\\%s%s"),path.data(),file.data(),ext.data());
		theTex->biOutFile.SetName(fullName);
		StuffOutName();
		}
	}


int ACubicDlg::FindFileFromHWND(HWND hw) {
	for (int i=0; i<6; i++) {
		if (hw == iFile[i]->GetHwnd()) return i;
		}	
	return -1;
	}


ReferenceTarget *ACubicDlg::GetInstance(HWND hwnd, POINT p, SClass_ID type) {
	DADBitmapCarrier *bmc = GetDADBitmapCarrier();
	int i = FindFileFromHWND(hwnd);
	if (i<0) return NULL;
	bmc->SetName(theTex->fileNames[i]);
	return bmc;
	}

void ACubicDlg::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type) {
	if (dropThis->SuperClassID()!=BITMAPDAD_CLASS_ID) 
		return;
	int i = FindFileFromHWND(hwnd);
	if (i<0) return;
	DADBitmapCarrier *bmc = (DADBitmapCarrier *)dropThis;
	theTex->fileNames[i] = bmc->GetName();
	StuffDlgName(i);
	}
*/


//-----------------------------------------------------------------------------
//  ACubic
//-----------------------------------------------------------------------------

class ACubicAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);    // set from v
};



static ACubicAccessor acubic_accessor;


static ParamBlockDesc2 acubic_param_blk ( acubic_params, _T("parameters"),  0, &acubicCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_AUTO_CUBIC, IDS_DS_ACUBIC_PARAMS, 0, 0, NULL, 
	// params
	acubic_size,	_T("size"),   TYPE_INT,			P_ANIMATABLE,	IDS_RB_SIZE,
		p_default,		100,
		p_range,		1, 5000,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_ACUBE_SIZE_EDIT, IDC_ACUBE_SIZE_SPIN, 1.0f, 
		end,
	acubic_blur,	_T("blur"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_BLUR,
		p_default,		1.f,
		p_range,		0.f, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ACUBE_BLUR_EDIT, IDC_ACUBE_BLUR_SPIN, .01f, 
		end,
	acubic_bluroffset,	_T("blurOffset"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_BLUROFFS,
		p_default,		0.f,
		p_range,		0.f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ACUBE_BLUOFF_EDIT, IDC_ACUBE_BLUROFF_SPIN, .001f, 
		end,
	acubic_near,	_T("near"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_FALLNEAR,
		p_default,		0.f,
		p_range,		0.0f,10000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ACUBE_NEAR_EDIT, IDC_ACUBE_NEAR_SPIN, 1.0f, 
		p_dim, 			stdWorldDim,
		end,

	acubic_far,		_T("far"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_FALLFAR,
		p_default,		500.f,
		p_range,		0.0f,10000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_UNIVERSE, IDC_ACUBE_FAR_EDIT, IDC_ACUBE_FAR_SPIN, 1.0f, 
		p_dim, 			stdWorldDim,
		end,

	acubic_source, _T("source"), TYPE_INT,				0,			IDS_PW_SOURCE,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_CUBESRC_AUTO, IDC_CUBESRC_FILE,
		p_accessor,		&acubic_accessor,
		end,

	acubic_useatmospheric,	_T("useAtmosphericMap"), TYPE_BOOL,			0,		IDS_PW_USEATMOSPHERIC,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_ENVMAP,
		p_accessor,		&acubic_accessor,
		end,
	acubic_applyblur,	_T("apply"), TYPE_BOOL,			0,		IDS_PW_APPLY,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ACUBE_BLUR,
		p_accessor,		&acubic_accessor,
		end,

	acubic_frametype, _T("frametype"), TYPE_INT,				0,			IDS_PW_FRAMETYPE,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_FIRST_ONLY, IDC_EVERY_NTH,
		end,

	acubic_nthframe,		_T("nthframe"),   TYPE_INT,			0,	IDS_PW_NTHFRAME,
		p_default,		1,
		p_range,		1,1000,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_ACUBE_NTH_EDIT, IDC_ACUBE_NTH_SPIN, 1.0f, 
		end,

	acubic_bitmap_names,	_T("bitmapName"),   TYPE_STRING_TAB,		6,	0,	IDS_DS_BITMAP,
		p_enabled,		FALSE,
//		p_ui, 			TYPE_BITMAPBUTTON, IDC_FILE_UP,IDC_FILE_DN,IDC_FILE_LF,IDC_FILE_RT,IDC_FILE_FR,IDC_FILE_BK,
		end,

	acubic_outputname,		_T("outputname"),   TYPE_STRING,			0,	IDS_PW_OUTPUTNAME,
		end,

	end
);



class ACubicDlgProc;

void ACubicAccessor::Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
{
ACubic* p = (ACubic*)owner;
switch (id)
	{
/*
	case acubic_camera:
		{

		if (v.r!= NULL)
			{
			INode *node =  (INode*) v.r;
			ObjectState os = node->EvalWorldState(t);
			if (os.obj->SuperClassID()==CAMERA_CLASS_ID) 
				{
				CameraState cs;
				Interval iv;
				CameraObject *cam = (CameraObject *)os.obj;
				cam->EvalCameraState(t,iv,&cs);
				p->SetNearRange(cs.nearRange,t);
				p->SetFarRange(cs.farRange,t);
				}
//			Interval iv;
//			p->pblock->SetValue(acubic_camera,t,NULL);
			}
		v.r = NULL;
		break;
		}
	case acubic_node:
		{
		if (v.r!= NULL)
			{
			p->RenderCubicMap((INode*)v.r);
//			Interval iv;
//			p->pblock->SetValue(acubic_node,t,NULL);
			p->pblock->RefDeleted(acubic_node);
			}
		v.r = NULL;
		break;
		}
*/
	case acubic_source:
		{
		if (v.i == 0)
			{
			if (p->pblock != NULL)
				{
				p->useFile = FALSE;
				p->UpdateInterface(FALSE);
				}
			}
		else
			{
			if (p->pblock != NULL)
				{
				p->useFile = TRUE;
				p->UpdateInterface(TRUE);
				}
			}
		break;
		}

	}
}



#define NPARAMS 5
#define ACUBIC_VERSION 4


// Version 1 desc
static ParamBlockDescID pbdesc1[] = {
	{ TYPE_INT, NULL, TRUE,acubic_size }, 	// size
	{ TYPE_FLOAT, NULL, TRUE,acubic_blur } 	// blur
	};

// Version 2 desc
static ParamBlockDescID pbdesc2[] = {
	{ TYPE_INT, NULL, TRUE,acubic_size }, 	// size
	{ TYPE_FLOAT, NULL, TRUE,acubic_blur }, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,acubic_bluroffset } 	// blurOffs
	};


static ParamBlockDescID pbdesc[] = {
	{ TYPE_INT,  NULL,  TRUE,acubic_size }, 	// size
	{ TYPE_FLOAT, NULL, TRUE,acubic_blur }, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,acubic_bluroffset }, 	// blurOffs
	{ TYPE_FLOAT, NULL, TRUE,acubic_near },	// near
	{ TYPE_FLOAT, NULL, TRUE,acubic_far } 	// far
	};


static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc1,2,1),	// Version 1 params
	ParamVersionDesc(pbdesc2,3,2),	// Version 2 params
	ParamVersionDesc(pbdesc, 5,3),	// Version 2 params
	};
#define NUM_OLDVERSIONS	3



static int fileButtonID[6]={IDC_FILE_UP,IDC_FILE_DN,IDC_FILE_LF,IDC_FILE_RT,IDC_FILE_FR,IDC_FILE_BK};
static int SideFromID(int id) {
	for (int i=0; i<6; i++) if (fileButtonID[i]==id) return i;
	return 0;
	}



class PickControlNode : 
		public PickObjectProc
//		public PickModeCallback
		 {
	public:				
		ACubic *theTex;
		BOOL pickCam;
		HWND hWnd;
		PickControlNode() {theTex=NULL;}
//		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);		
		BOOL Pick(INode *node);		
		void EnterMode();
		void ExitMode();		
		BOOL Filter(INode *node);
	};


BOOL PickControlNode::Filter(INode *node)
	{
	if (pickCam)
		{
		ObjectState os = node->EvalWorldState(0);
		if (os.obj->SuperClassID()==CAMERA_CLASS_ID) 
			return TRUE;
		return FALSE;
		}

	else return TRUE;
/*	node->BeginDependencyTest();
	mod->NotifyDependents(FOREVER,0,REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest()) {		
		return FALSE;
	} else {
		return TRUE;
		}
*/
	}
/*
BOOL PickControlNode::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
	{	
	if (ip->PickNode(hWnd,m,this)) {
		return TRUE;
	} else {
		return FALSE;
		}
	}
*/
BOOL PickControlNode::Pick(INode *node)
	{

	if (node) {
		if (pickCam)
			{
			ObjectState os = node->EvalWorldState(GetCOREInterface()->GetTime());
			if (os.obj->SuperClassID()==CAMERA_CLASS_ID) 
				{
				CameraState cs;
				Interval iv;
				CameraObject *cam = (CameraObject *)os.obj;
				cam->EvalCameraState(GetCOREInterface()->GetTime(),iv,&cs);
				theTex->SetNearRange(cs.nearRange,GetCOREInterface()->GetTime());
				theTex->SetFarRange(cs.farRange,GetCOREInterface()->GetTime());
				}
			}
		else
			{
			theTex->RenderCubicMap(node);

			}


		}
	return TRUE;
	}

void PickControlNode::EnterMode()
	{
	ICustButton *iBut ;
	if(pickCam)
		iBut = GetICustButton(GetDlgItem(hWnd,IDC_ACUBE_PICKCAM));
	else iBut = GetICustButton(GetDlgItem(hWnd,IDC_CUBE_PICK));
	if (iBut) iBut->SetCheck(TRUE);
	ReleaseICustButton(iBut);
	}

void PickControlNode::ExitMode()
	{
	ICustButton *iBut ;
	if(pickCam)
		iBut = GetICustButton(GetDlgItem(hWnd,IDC_ACUBE_PICKCAM));
	else iBut = GetICustButton(GetDlgItem(hWnd,IDC_CUBE_PICK));
	if (iBut) iBut->SetCheck(FALSE);
	ReleaseICustButton(iBut);
	}

static PickControlNode thePickMode;


//dialog stuff to get the Set Ref button
class ACubicDlgProc : public ParamMap2UserDlgProc, public DADMgr {
//public ParamMapUserDlgProc {
	public:
		HWND hPanel;
		ACubic *theTex;		
		ICustButton *iFile[6];
		ICustButton *oFile; // mjm - 1.26.99
//		ICustButton *iPick;
		ICustButton *iPick,*iCamPick;

		BOOL camPick;


		ACubicDlgProc(ACubic *m) {
			theTex = m;
			for (int i=0; i<6; i++) {
				iFile[i] = NULL;	
				}
			oFile = NULL; // mjm - 1.26.99
			iPick = NULL;
			iCamPick = NULL;

			}		
		~ACubicDlgProc() {
			Destroy();
			}

		void Destroy() { 
			if (theTex->ip)
				theTex->ip->EndPickMode();
			if (iPick)
				ReleaseICustButton(iPick);	
			iPick = NULL; 
			if (iCamPick)
				ReleaseICustButton(iCamPick);	
			iCamPick = NULL; 

			for (int i=0; i<6; i++) {
				if (iFile)
					ReleaseICustButton(iFile[i]);
				iFile[i] = NULL;	
				}
			if (oFile) ReleaseICustButton(oFile); // mjm - 1.26.99
			oFile = NULL; // mjm - 1.26.99
			}


		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void BrowseOutFile();
		void StuffOutName();
		void BrowseInFile(int id);

		void StuffDlgName(int i);
		int FindFileFromHWND(HWND hw);


		void SetUseFile(BOOL onOff);
		void MaybeEnablePick(); 

		void DeleteThis() {delete this;}





		// DADMgr methods
		// called on the draggee to see what if anything can be dragged from this x,y
		SClass_ID GetDragType(HWND hwnd, POINT p) { return BITMAPDAD_CLASS_ID; }
		// called on potential dropee to see if can drop type at this x,y
		BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {
			if (hfrom==hto) return FALSE;
      //aszabo|Nov.15.01 - Drop method does not handle dropping NULL ref targets, so don't allow dropping them
			return (type==BITMAPDAD_CLASS_ID && dropThis != NULL)?1:0;
			}
		int SlotOwner() { return OWNER_MTL_TEX;	}
	    ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
		void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type);
		BOOL  LetMeHandleLocalDAD() { return 0; } 

		void SetThing(ReferenceTarget *m) {
			theTex = (ACubic*)m;
//			ReloadDialog();
			}


	};



void ACubicDlgProc::StuffDlgName(int i) {
	TSTR fname;
	TSTR fullName = theTex->fileNames[i];
	SplitPathFile(fullName,NULL,&fname);
	iFile[i]->SetText(fname);
	//SetDlgItemText(hPanel, fileButtonID[i], fname);
	}


int ACubicDlgProc::FindFileFromHWND(HWND hw) {
	if (hw == oFile->GetHwnd())
		return NUM_SRC_FILES; // outfile

	for (int i=0; i<NUM_SRC_FILES; i++)
	{
		if (hw == iFile[i]->GetHwnd())
			return i; // one of the infiles
	}
	return -1; // no file
}


ReferenceTarget *ACubicDlgProc::GetInstance(HWND hwnd, POINT p, SClass_ID type)
{
	DADBitmapCarrier *bmc = GetDADBitmapCarrier();
	int i = FindFileFromHWND(hwnd);
	if (i<0)
		return NULL;
// mjm - begin - 2.2.99
	else if (i == NUM_SRC_FILES) // outfile
	{
		TCHAR *name;
		theTex->pblock->GetValue(acubic_outputname,0,name,FOREVER);
		TSTR nameStr = name;
		bmc->SetName(nameStr);
	}
	else // one of the infiles
// mjm - end
		bmc->SetName(theTex->fileNames[i]);
	return bmc;
}

void ACubicDlgProc::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type) {
	if (dropThis->SuperClassID()!=BITMAPDAD_CLASS_ID) 
		return;
	int i = FindFileFromHWND(hwnd);
	if (i<0)
		return;
// mjm - begin - 2.2.99
	else if (i == NUM_SRC_FILES) // outfile
	{
		DADBitmapCarrier *bmc = (DADBitmapCarrier *)dropThis;
		theTex->pblock->SetValue(acubic_outputname,0,bmc->GetName());
		StuffOutName();
	}
	else // one of the infiles
// mjm - end
	{
		DADBitmapCarrier *bmc = (DADBitmapCarrier *)dropThis;
		theTex->fileNames[i] = bmc->GetName();
		theTex->pblock->SetValue(acubic_bitmap_names,0,theTex->fileNames[i],i);
		StuffDlgName(i);
	}
}



void ACubicDlgProc::StuffOutName() {
	TSTR fname;
	TCHAR *name;
	Interval iv;
	theTex->pblock->GetValue(acubic_outputname,0,name,iv);
//	TSTR fullName = theTex->biOutFile.Name();
	TSTR fullName = name;
	SplitPathFile(fullName,NULL,&fname);
	oFile->SetText(fname); // mjm - 1.26.99
//	SetDlgItemText(hPanel, IDC_CUBE_OUTFILE_NAME, fname); // mjm - 1.26.99
	}
 


void ACubicDlgProc::BrowseOutFile() {
	BOOL silent = TheManager->SetSilentMode(TRUE);
	int res = TheManager->SelectFileOutput(&theTex->biOutFile,hPanel,GetString(IDS_DS_SELECT_UPFILE));
	TheManager->SetSilentMode(silent);
	if (res) {
		TSTR fullName = theTex->biOutFile.Name();
		TSTR path;
		TSTR file;
		TSTR ext;
		SplitFilename(fullName,&path, &file, &ext);
		int n = file.Length();
		if (n>=3) {
			TSTR suf = file.Substr(n-3,3);
			suf.toUpper();
			if (_tcscmp(suffixes[0],suf)!=0) {
				for (int i=1; i<6; i++) {
					if (_tcscmp(suffixes[i],suf)==0) {
						file = file.Substr(0,n-3);
						break;
						}
					}
				file += suffixes[0];
				}
			}
		else {
			file += suffixes[0];
			}
		fullName.printf(_T("%s\\%s%s"),path.data(),file.data(),ext.data());
		theTex->biOutFile.SetName(fullName);
		theTex->pblock->SetValue(acubic_outputname,0,fullName);
		StuffOutName();
		theTex->UpdateInterface(theTex->useFile);
		}
	}

void ACubicDlgProc::BrowseInFile(int id) {
	int iside = SideFromID(id);
	theTex->biInFile.SetName(theTex->fileNames[iside].data());
	BOOL silent = TheManager->SetSilentMode(TRUE);
	int res = TheManager->SelectFileInputEx(&theTex->biInFile,hPanel,GetString(IDS_ACUBIC_SELECT_INPUT_FILE));
//	int res = TheManager->SelectFileInputEx(&theTex->biInFile,hPanel,getCubeName(iside));
	TheManager->SetSilentMode(silent);
	if (res) {
		TSTR fullName = theTex->biInFile.Name();
		TSTR path;
		TSTR file;
		TSTR ext;
		if (theTex->fileNames[iside]==fullName) 
			goto done;
		theTex->fileNames[iside] = fullName;
		theTex->pblock->SetValue(acubic_bitmap_names,0,fullName,iside);
		StuffDlgName(iside);
		SplitFilename(fullName,&path, &file, &ext);
		int n = file.Length();
		if (n>=3) {
			TSTR suf = file.Substr(n-3,3);
			suf.toUpper();
			if (_tcscmp(suffixes[iside],suf)==0) {
				file = file.Substr(0,n-3);
				for (int i=0; i<6; i++) {
					fullName.printf(_T("%s\\%s%s%s"),path.data(),file.data(),suffixes[i],ext.data());
					theTex->fileNames[i] = fullName;
					theTex->pblock->SetValue(acubic_bitmap_names,0,fullName,i);
					StuffDlgName(i);
					if (theTex->useFile) {
						theTex->FreeMaps();
						theTex->NotifyChanged();
						}
					}
				}
			else {
				theTex->fileNames[iside] = fullName;
				theTex->pblock->SetValue(acubic_bitmap_names,0,fullName,iside);
				StuffDlgName(iside);
				}
			}
		}
done:
	theTex->biInFile.SetName(_T(""));
	}



BOOL ACubicDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	hPanel = hWnd;
	switch (msg) {

		case WM_INITDIALOG:
			{

			iPick = GetICustButton(GetDlgItem(hWnd,IDC_CUBE_PICK));
			iPick->SetType(CBT_CHECK);
			iPick->SetHighlightColor(GREEN_WASH);

			iCamPick = GetICustButton(GetDlgItem(hWnd,IDC_ACUBE_PICKCAM));
			iCamPick->SetType(CBT_CHECK);
			iCamPick->SetHighlightColor(GREEN_WASH);

			for (int i=0; i<6; i++) {
				iFile[i] = GetICustButton(GetDlgItem(hWnd,fileButtonID[i]));
				iFile[i]->SetDADMgr(this);
				}

			oFile = GetICustButton( GetDlgItem(hWnd,IDC_CUBE_OUTFILE_NAME) ); // mjm - 1.26.99
			oFile->SetDADMgr(this);

			// > 10/30/02 - 10:55am --MQM-- this doesn't do much good....should remove?
			for (i=0; i<6; i++) 
				StuffDlgName(i);
			StuffOutName();

			return TRUE;
			}
			break;

		case WM_DESTROY:
			Destroy();
			break;


		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_FILE_UP:
				case IDC_FILE_DN:
				case IDC_FILE_LF:
				case IDC_FILE_RT:
				case IDC_FILE_FR:
				case IDC_FILE_BK:
					{
					BrowseInFile(LOWORD(wParam));
					break;
					}

				case IDC_CUBE_RELOAD:
					{
					theTex->FreeMaps();
					theTex->NotifyChanged();
//					check->SwapInputs();
					break;
					}

				case IDC_CUBE_OUTFILE_NAME:
					{
					BrowseOutFile();
					break;
					}
				case IDC_ACUBE_PICKCAM:
					{
					theTex->ip->EndPickMode();
					thePickMode.hWnd  = hWnd;					
					thePickMode.pickCam  = TRUE;					
					thePickMode.theTex = theTex;
					theTex->ip->SetPickMode(&thePickMode);
					break;
					}
				case IDC_CUBE_PICK:
					{
					theTex->ip->EndPickMode();
					thePickMode.hWnd  = hWnd;					
					thePickMode.pickCam  = FALSE;					
					thePickMode.theTex = theTex;
					theTex->ip->SetPickMode(&thePickMode);
					break;
					}

				}
			break;
		}
	return FALSE;
	}

void ACubicDlgProc::SetUseFile(BOOL onOff) {

	// AUTO STUFF
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_AUTO_GRP), !onOff);
//	EnableWindow(GetDlgItem(hPanel,IDC_FIRST_ONLY), !onOff);
//	EnableWindow(GetDlgItem(hPanel,IDC_EVERY_NTH), !onOff);

	IParamMap2 *map = theTex->pblock->GetMap();
	map->Enable(acubic_nthframe, !onOff);
	map->Enable(acubic_frametype, !onOff);
	map->Enable(acubic_nthframe, !onOff);

//	if (onOff) nthSpin->Disable(); else nthSpin->Enable();

	//FILE STUFF
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_RELOAD), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_FILE_GRP), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_FILE_GRP2), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_OUTFILE), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_OUTFILE_NAME), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_UP), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_DN), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_LF), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_RT), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_BK), onOff);
	EnableWindow(GetDlgItem(hPanel,IDC_CUBE_FR), onOff);

	for (int i=0; i<6; i++) 
		iFile[i]->Enable(onOff);
	oFile->Enable(onOff); // mjm - 1.26.99
	MaybeEnablePick();
	}

void ACubicDlgProc::MaybeEnablePick() {
	if (!theTex->useFile) {
		IParamMap2 *map = theTex->pblock->GetMap();
//		map->Enable(acubic_node, FALSE);
		iPick->Enable(FALSE);
		return;
		}
	TCHAR *name=NULL;
	Interval iv;
	theTex->pblock->GetValue(acubic_outputname,0,name,iv);
//	if (_tcslen(theTex->biOutFile.Name())>0) 
	if ((name) && (_tcslen(name)>0) )
		{
		IParamMap2 *map = theTex->pblock->GetMap();
//		map->Enable(acubic_node, TRUE);
		iPick->Enable(TRUE);
		}
	else 
		{
		IParamMap2 *map = theTex->pblock->GetMap();
		iPick->Enable(FALSE);
//		map->Enable(acubic_node, FALSE);
		}
	}


void ACubic::UpdateInterface(BOOL on)
{
ACubicDlgProc *paramDlg =  (ACubicDlgProc*)acubic_param_blk.GetUserDlgProc();
if (paramDlg)
	paramDlg->SetUseFile(on);
}


void ACubic::Init() {
	nth = 1;
	do_nth = TRUE;
	useFile = FALSE;
	inMedit= FALSE;
	applyBlur = TRUE;
	useEnvMap = TRUE;
//	ReplaceReference( 0, CreateParameterBlock( pbdesc, NPARAMS, ACUBIC_VERSION) );	
	ivalid.SetEmpty();
	SetSize(100, TimeValue(0));
	SetBlur(1.0f, TimeValue(0));
	SetBlurOffset(.0f, TimeValue(0));
	SetNearRange(0.0f,0);
	SetFarRange(500.0f,0);
	}

void ACubic::Reset() {
	Init();
	acubicCD.Reset(this, TRUE);	// reset all pb2's
	}

void ACubic::SetUseFile(BOOL onOff) {
	useFile = onOff;
	if (!useFile) FreeMaps();
	}

void ACubic::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}



ACubic::ACubic() {
//	InitializeCriticalSection(&csect);
//	paramDlg = NULL;
	ip = NULL;
	pblock = NULL;
	maps = NULL;
	acubicCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

RefTargetHandle ACubic::Clone(RemapDir &remap) {
	ACubic *mnew = new ACubic();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->do_nth = do_nth;
	mnew->applyBlur = applyBlur;
	mnew->ivalid.SetEmpty();	
	mnew->biInFile = biInFile;
	mnew->biOutFile = biOutFile;
	mnew->useFile = useFile;
	mnew->useEnvMap = useEnvMap;
//	for (int i=0; i<6; i++) mnew->fileNames[i]= fileNames[i]; 
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* ACubic::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
//	ACubicDlg *dm = new ACubicDlg(hwMtlEdit, imp, this);
//	dm->LoadDialog(TRUE);	
//	paramDlg = dm;
//	return dm;	
	// create the rollout dialogs
	ip = imp;
	IAutoMParamDlg* masterDlg = acubicCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	acubic_param_blk.SetUserDlgProc(new ACubicDlgProc(this));
	BOOL on;
	Interval iv;
	pblock->GetValue(acubic_source,0,on,iv);
	UpdateInterface(on);
	return masterDlg;


	}

void ACubic::Update(TimeValue t, Interval& valid) {		

//	pblock->SetValue(acubic_node,t,(INode *) NULL);
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		pblock->GetValue( acubic_size, t, size, ivalid );
		pblock->GetValue( acubic_blur, t, blur, ivalid );
		pblock->GetValue( acubic_bluroffset, t, blurOff, ivalid );
		pblock->GetValue( acubic_near, t, nearRange, ivalid );
		pblock->GetValue( acubic_far, t, farRange, ivalid );

		pblock->GetValue( acubic_nthframe, t, nth, ivalid );
		pblock->GetValue( acubic_applyblur, t, applyBlur, ivalid );
		pblock->GetValue( acubic_frametype, t, do_nth, ivalid );

		pblock->GetValue( acubic_useatmospheric, t, useEnvMap, ivalid );
		pblock->GetValue( acubic_source, t, useFile, ivalid );
		

		for (int i = 0 ; i < 6; i++)
			{
			TCHAR *name;
			pblock->GetValue( acubic_bitmap_names,t,name,ivalid,i);
			fileNames[i] = name;
			}	

		// > 10/30/02 - 10:36am --MQM-- 
		// be shure to update the UI.  this probably isn't the best fix...
		ACubicDlgProc *paramDlg =  (ACubicDlgProc*)acubic_param_blk.GetUserDlgProc();
		if ( paramDlg != NULL )  
		{
			for ( i = 0;  i < 6;  i++ ) 
				paramDlg->StuffDlgName(i);
			paramDlg->StuffOutName();
		}

		}
	valid &= ivalid;
	}

void ACubic::FreeMaps() {
	CubicMap *cm,*nxtcm;
	for (cm = maps; cm!=NULL; cm = nxtcm) {
		nxtcm = cm->next;
	   	delete cm;		
		}
	maps = NULL;
	}

static void ACubicMsg(const TCHAR* msg) {
	if (GetCOREInterface()->GetQuietMode()) {
		// Obsolete GG: 01/29/99
		// GetCOREInterface()->NetLog(_T("%s: %s \n"), GetString(IDS_DS_REFL_ERROR),msg);
		GetCOREInterface()->Log()->LogEntry(SYSLOG_ERROR,NO_DIALOG,NULL,_T("%s: %s"),GetString(IDS_DS_REFL_ERROR),msg);
	} else 
		MessageBox(NULL, msg, GetString(IDS_DS_REFL_ERROR), MB_TASKMODAL);
	}

static void ACubicNotSquareMsg(const TCHAR* name) {
	if (name==NULL) return;
	TCHAR msg[256];
	wsprintf(msg,GetString(IDS_DS_NOT_SQUARE), name);
	ACubicMsg(msg);
	}

static void ACubicWrongSizeMsg(const TCHAR* name) {
	if (name==NULL) return;
	TCHAR msg[256];
	wsprintf(msg,GetString(IDS_DS_WRONG_SIZE), name);
	ACubicMsg(msg);
	}

int ACubic::LoadMapFiles(TimeValue t) {
	unsigned short status;

	// > 11/7/02 - 4:29pm --MQM-- 
	// the renderer can call this _before_ calling Update(),
	// so we better make sure we have the right parameters loaded up.
	Interval ivalid;
	ivalid.SetEmpty();
	Update( t, ivalid );

//	if (useFile||inMedit) {
	if (useFile) {
		BOOL silent = TheManager->SetSilentMode(TRUE);
		BOOL calcBlur = FALSE;
		if (maps==NULL) {
			maps = new CubicMap;
			maps->tossMapsWhenBlur = FALSE;
			}
		SetCursor(LoadCursor(NULL,IDC_WAIT));

//		PBBitmap *bp;
//		pblock->GetValue( acubic_bitmaps,t,bp,ivalid,i);

		biInFile.SetCurrentFrame(t/GetTicksPerFrame());
		int mapSize = -1;
		for (int i=0; i<6; i++) {	
//			PBBitmap *bp;
//			pblock->GetValue( acubic_bitmaps,t,bp,ivalid,i);
//			if (bp != NULL)
				{
//				biInFile = bp->bi;
				biInFile.SetName(fileNames[i]);
				if (maps->bitmap[i]==NULL) {
//					maps->bitmap[i] = bp->bm;

					maps->bitmap[i] = TheManager->Load(&biInFile,&status);
					calcBlur = TRUE;
					}
				if (maps->bitmap[i]!=NULL) {
					int w = maps->bitmap[i]->Width();
					int h = maps->bitmap[i]->Height();
					if (w!=h) {
						ACubicNotSquareMsg(fileNames[i]);
//						ACubicNotSquareMsg(bp->bi.Name());
						FreeMaps();
						fileNames[i] = _T("");
						pblock->SetValue(acubic_bitmap_names,0,fileNames[i],i);
						ACubicDlgProc *paramDlg =  (ACubicDlgProc*)acubic_param_blk.GetUserDlgProc();
						if (paramDlg)
							paramDlg->StuffDlgName(i);
						goto bail;
						}
					if (mapSize<0) mapSize = w; 
					else {
						if (mapSize!=w) {
//							ACubicWrongSizeMsg(bp->bi.Name());
							ACubicWrongSizeMsg(fileNames[i]);
							FreeMaps();
							goto bail;
							}
						}
					fileNames[i] = biInFile.Name(); // DS 2/14/2000
					if (maps->bitmap[i]->Storage()->bi.CurrentFrame()!=biInFile.CurrentFrame()) {
						status = maps->bitmap[i]->GoTo(&biInFile);
						if (status!=BMMRES_SINGLEFRAME)
							calcBlur = TRUE;
						}
					}
				}
			}
		maps->size = mapSize;
		if (applyBlur) {
			if (calcBlur||!maps->blurmap) 
				maps->BuildBlurMap();
			}
		else 
			maps->FreeBlurMap();
		bail:
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		TheManager->SetSilentMode(silent);
		}
	return 1;
	}

void ACubic::SetSize(int s, TimeValue t) { 
	size = s; 
	pblock->SetValue( acubic_size, t, s);
	}

void ACubic::SetApplyBlur(BOOL onoff) { 
	if (applyBlur==onoff) return;
	applyBlur = onoff; 
	if (useFile&&maps) {
		if (applyBlur) {
			SetCursor(LoadCursor(NULL,IDC_WAIT));
			maps->BuildBlurMap();
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			}
		else 
			maps->FreeBlurMap();
		}
	}

void ACubic::SetBlur(float f, TimeValue t) { 
	blur = f; 
	pblock->SetValue( acubic_blur, t, f);
	}

void ACubic::SetBlurOffset(float f, TimeValue t) { 
	blurOff = f; 
	pblock->SetValue( acubic_bluroffset, t, f);
	}

void ACubic::SetNearRange(float f, TimeValue t) { 
	nearRange = f; 
	pblock->SetValue( acubic_near, t, f);
	}

void ACubic::SetFarRange(float f, TimeValue t) { 
	farRange = f; 
	pblock->SetValue( acubic_far, t, f);
	}

void ACubic::UseHighDynamicRange(BOOL onoff)
{
}

int ACubic::IsHighDynamicRange( ) const
{
	return false;
}

RefTargetHandle ACubic::GetReference(int i) {
	switch(i) {
		case 0:	return pblock ;
		default: return NULL;
		}
	}

void ACubic::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2 *)rtarg; break;
		}
	}
	 
Animatable* ACubic::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: return NULL;
		}
	}

TSTR ACubic::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));
		default: return TSTR("");		
		}
	}

int  ACubic::GetOutFileName(TSTR &fullname, TSTR &fname, int i) {
	TSTR path;
	TSTR ext;
	TSTR upname;// = biOutFile.Name();
	Interval iv;
	TCHAR *tname;
	pblock->GetValue(acubic_outputname,0,tname,iv);
	upname = tname;
	if (upname.Length()==0) return 0;
	SplitFilename(upname,&path, &fname, &ext);
	int n = fname.Length();
	///assert(n>=3);
	fname = fname.Substr(0,n-3);
	fname += suffixes[i];
	TSTR fullName;
	fullname.printf(_T("%s\\%s%s"),path.data(),fname.data(),ext.data());
	return 1;
	}

int ACubic::WriteBM(Bitmap *bm, TCHAR *name) {
	TSTR upname = biOutFile.Name();
	biOutFile.SetName(name);
	if (bm->OpenOutput(&biOutFile) != BMMRES_SUCCESS) goto bail;
	if (bm->Write(&biOutFile,BMM_SINGLEFRAME) != BMMRES_SUCCESS) 
		goto bail;
	bm->Close(&biOutFile);
	biOutFile.SetName(upname);
	return 1;

	bail:
	bm->Close(&biOutFile);
	biOutFile.SetName(upname);
	return 0;
	}


void ACubic::RenderCubicMap(INode *node)
	{
	int res;
	BOOL success = 0;
	TSTR fname,fullname;

	if (size<=0) 
		return;
	Interface *ip = GetCOREInterface();
	BOOL wasHid = node->IsNodeHidden();
	node->Hide(TRUE);

	// Create a blank bitmap
	Bitmap *bm = NULL;
	biOutFile.SetWidth(size);
	biOutFile.SetHeight(size);
	biOutFile.SetType(BMM_TRUE_64);
	biOutFile.SetAspect(1.0f);
	biOutFile.SetCurrentFrame(0);
	biOutFile.SetFlags( MAP_HAS_ALPHA );	// > 9/3/02 - 5:30pm --MQM-- this required for renderer to deal with tga -- bug #432553
	bm = TheManager->Create(&biOutFile);

	Matrix3 nodeTM = node->GetNodeTM(ip->GetTime());
	Matrix3 tm;	
	INode* root = ip->GetRootNode();		
	bm->Display(GetString(IDS_DS_ACUBIC_NAME));

	// NEW WAY
	ViewParams vp;
	vp.projType = PROJ_PERSPECTIVE;
	vp.hither = .001f;
	vp.yon = 1.0e30f;
	vp.fov = PI/2.0f;
	vp.nearRange = nearRange;
	vp.farRange = farRange;
	BOOL saveUseEnvMap = ip->GetUseEnvironmentMap();
	ip->SetUseEnvironmentMap(useEnvMap);

	res = ip->OpenCurRenderer(&vp); 
	for (int i=0; i<6; i++) {
		tm = TMForView(i);
		tm.PreTranslate(-nodeTM.GetTrans()); 
		vp.affineTM = tm;
		if (!GetOutFileName(fullname,fname,i)) 
			goto fail;
		bm->SetWindowTitle(fname);
		res = ip->CurRendererRenderFrame(ip->GetTime(),bm,NULL,1.0f,&vp);
		if (!res) 
			goto fail;
		if (!WriteBM(bm, fullname)) goto fail;
		}
	success = 1;
	fail:
	ip->CloseCurRenderer();	
	ip->SetUseEnvironmentMap(saveUseEnvMap);

	bm->DeleteThis();
	node->Hide(wasHid);
	if (success) {
		for (i=0; i<6; i++) {
			GetOutFileName(fullname,fname,i);
			fileNames[i] = fullname; 

			
			biOutFile.SetName(fullname);
			pblock->SetValue( acubic_bitmap_names,ip->GetTime(), fullname,i);
//			fileNames[i] = bp->bi.Name();
			ACubicDlgProc *paramDlg =  (ACubicDlgProc*)acubic_param_blk.GetUserDlgProc();

//			GetUserDlgProc(ParamBlockDesc2* pbd);
			if (paramDlg) paramDlg->StuffDlgName(i);
			}
		if (useFile) {
			FreeMaps();
			NotifyChanged();
			}
		}
	}


static int nameID[] = {IDS_DS_MAPSIZE, IDS_DS_BLUR, IDS_DS_BLUROFFS, IDS_DS_NEARRANGE, IDS_DS_FARRANGE };

RefResult ACubic::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget == pblock)
				{
				ivalid.SetEmpty();
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				acubic_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
				}

//			ivalid.SetEmpty();
//			if (paramDlg) 
//				paramDlg->Invalidate();
			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_SIZE: 
				case PB_BLUR: 
				case PB_BLUROFF: 
					gpd->dim = defaultDim; break;
				case PB_NEAR: 
				case PB_FAR: 
					gpd->dim = stdWorldDim; break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = TSTR(GetString(nameID[gpn->index]));
			return REF_STOP; 
			}
*/
		}
	return(REF_SUCCEED);
	}


inline Point2 CompUV(float x, float y, float z) {
	return Point2( 0.5f*(x/z+1.0f), 0.5f*(y/z+1.0f));
	}


// Compute TM to transform points from World coords to Camera coords
// for each of the 6 views.
Matrix3 TMForView( int i) {
	Matrix3 m;
	m.IdentityMatrix();
	switch (i) {
		case VIEW_UP:    // looking UP
			m.RotateX(-PI);	
			break;
		case VIEW_DN:    // looking down (top view)
			break;
		case VIEW_LF: 	 // looking to left
			m.RotateX(-.5f*PI);	
			m.RotateY(-.5f*PI);
			break;
		case VIEW_RT:     // looking to right
			m.RotateX(-.5f*PI);	
			m.RotateY(+.5f*PI);
			break;
		case VIEW_FR:       // looking to front (back view)
			m.RotateX(-.5f*PI);	
			m.RotateY(PI);
			break;
		case VIEW_BK:         // looking to back
			m.RotateX(-.5f*PI);	
			break;
		}
	return m;
	}

int ACubic::DoThisFrame(TimeValue t, BOOL fieldRender, TimeValue mapTime) {
	if (!do_nth) {
		if (t!=mapTime) return 0;  // only do it on first frame.
		}
	if (nth==1) return 1;   // need every one
	TimeValue del = abs(t - mapTime);
	if (del==0) return 1; // repeated iterations on a frame are ok
	if (fieldRender) del*=2;
	return  (del>=nth*GetTicksPerFrame())?1:0;
	}

CubicMap *ACubic::FindMap(int nodeNum) {
	CubicMap *cm;
	for (cm=maps; cm!=NULL; cm = cm->next)
		if (cm->nodeID==nodeNum) return cm;
	return NULL;
	}

int ACubic::BuildMaps(TimeValue t, RenderMapsContext &rmc) {
	if (useFile) return 1;
	RenderGlobalContext *gc = rmc.GetGlobalContext();
	if (gc){
		if (gc->inMtlEdit&&(gc->envMap==NULL)) return 1;
		}
	SubRendParams srp;
	rmc.GetSubRendParams(srp);
	CubicMap *cm = FindMap(rmc.NodeRenderID());
	if (cm&&!DoThisFrame(t,srp.fieldRender,cm->mapTime))
		return 1;
	INode *node = rmc.GetNode();
	Matrix3 tm = node->GetNodeTM(t);
	Point3 pos = tm.GetRow(3); //nodes world position
	ViewParams vp;
	vp.projType = PROJ_PERSPECTIVE;
	vp.hither = .001f;
	vp.yon = 1.0e30f;
	vp.fov = PI/2.0f;
	vp.nearRange = nearRange;
	vp.farRange = farRange;

	srp.devWidth = size;
	srp.devHeight = size;
	srp.devAspect = 1.0f;
	srp.xmin = srp.ymin = 0;
	srp.xmax = srp.ymax = size;
	srp.xorg =srp.yorg = 0;
	srp.fieldRender = 0;
	srp.rendType = RENDTYPE_NORMAL;
	srp.doEnvMap = useEnvMap;
	srp.doingMirror = FALSE;
	if (cm==NULL) {
		cm = new CubicMap;
		cm->nodeID = rmc.NodeRenderID();
		cm->next = maps;
		maps = cm;
		}
	cm->mapTime = t;
	if (cm->AllocMaps(size)) {
		if (gc&&gc->inMtlEdit) 
			SetCursor(LoadCursor(NULL,IDC_WAIT));
		for (int i=0; i<6; i++) {	
			tm = TMForView(i);
			tm.PreTranslate(-pos); 
			vp.affineTM = tm;

			// > 10/29/01 - 3:37pm --MQM-- 
			// broadcast message at the start of any reflect/refract map render
			if ( gc && !gc->inMtlEdit )
				BroadcastNotification( NOTIFY_BEGIN_RENDERING_REFLECT_REFRACT_MAP, (void*)gc );

			if (!rmc.Render(cm->bitmap[i], vp, srp, 0)) {
				cm->FreeMaps();
				return 0;
				}
#ifdef DBG
			cm->bitmap[i]->Display(getCubeName(i), BMM_UR);
			MessageBox(NULL, getCubeName(i), _T(" Auto Cubic Test"), MB_OK|MB_ICONEXCLAMATION);
			cm->bitmap[i]->UnDisplay();
#endif
			}
		if (gc&&gc->inMtlEdit) 
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		}

	if (applyBlur) {
		cm->BuildBlurMap();
		}
	return 1;
	}

static BMM_Color_64 black64 = {0,0,0,0};

// The "canonical" view is looking in the -z direction,
// with x to the right, y up, i.e. VIEW_DN,
// other views are mapped into this canonical rep

BMM_Color_64 CubicMap::Sample(Point3 v) {
	float wx,wy,wz;
	Color rcol;
	Bitmap *refmap=NULL;
	Point3 rv;
	Point2 uv;
	wx = (float)fabs(v.x);  wy = (float)fabs(v.y);  wz = (float)fabs(v.z); 
	if (wx>=wy && wx>=wz) {
		if(v.x<0) {	refmap = bitmap[VIEW_LF];	uv = CompUV(-v.y, -v.z,  v.x);	}
		else	  { refmap = bitmap[VIEW_RT];   uv = CompUV( v.y, -v.z, -v.x);	}
		}
	else
	if(wy>=wx && wy>=wz) {
		if(v.y>0) { refmap = bitmap[VIEW_BK];	uv = CompUV( -v.x, -v.z, -v.y);	}
		else {  	refmap = bitmap[VIEW_FR];	uv = CompUV(  v.x, -v.z,  v.y); }
		}
	else
	if(wz>=wx && wz>=wy) {
		if(v.z<0) {	refmap = bitmap[VIEW_DN];	uv = CompUV( -v.x, -v.y,  v.z);	}
		else     {	refmap = bitmap[VIEW_UP];	uv = CompUV( -v.x,  v.y, -v.z);	}
		}

	if (refmap==NULL) return black64;
	if (uv.x<0.0f) uv.x = 0.0f; else if (uv.x>1.0f) uv.x = 1.0f;
	if (uv.y<0.0f) uv.y = 0.0f; else if (uv.y>1.0f) uv.y = 1.0f;
	int x = (int)(uv.x*(float)(size-1));
	int y = (int)((1.0f-uv.y)*(float)(size-1));
	BMM_Color_64 c;
	//refmap->GetPixels(x,y,1,&c);
	refmap->GetLinearPixels(x,y,1,&c);
	return c;
	}

static AColor black(0.0f,0.0f,0.0f,0.0f);
			   
RGBA ACubic::EvalColor(ShadeContext& sc) {
	CubicMap *cm=NULL;
//	IPoint2 sp = sc.ScreenCoord();
//	if (sp.x==128&&sp.y==106)
//		whoa();

//	if (sc.InMtlEditor()&&!useFile) {
//		EnterCriticalSection(&csect);
//		if (!maps) {
//			inMedit = 1;
//			LoadMapFiles(sc.CurTime());
//			inMedit = 0;
//			}
//		LeaveCriticalSection(&csect);
//		cm = maps;
//		}
//	else {
		if (useFile) {
			cm = maps;
			}
		else {
			if (gbufID) sc.SetGBufferID(gbufID);
			int id = sc.NodeID();
			cm = FindMap(id);
			}
//		}

	if (cm) {
		Point3 V = sc.VectorTo(sc.V(),REF_WORLD);
		if (applyBlur&&cm->blurmap) {
			BMM_Color_64 c;
			Point2 uv, duv;
			ComputeSphereCoords(V,uv,duv);
			duv *= (blur*sc.Curve());
			duv.x += blurOff;
			duv.y += blurOff;
			cm->blurmap->GetFiltered(uv.x,uv.y,duv.x,duv.y,&c);
			return c;
			}
		else 
			return cm->Sample(V);
		}
	return Color(0,0,0);
	}

float ACubic::EvalMono(ShadeContext& sc) {
	return Intens(EvalColor(sc));
	}

Point3 ACubic::EvalNormalPerturb(ShadeContext& sc) {
	return Point3(0,0,0);
	}

#define MTL_HDR_CHUNK 0x4000
#define DONT_DO_NTH_CHUNK 0x1000
#define NTH_CHUNK 0x1001
#define DONT_APPLY_BLUR_CHUNK 0x1002
#define DONT_USE_ENV_CHUNK 0x1003
#define USE_FILE_CHUNK 0x1004
#define NAME_CHUNK 0x1010
#define ACUBE_IO_IN_CHUNK 0x1020
#define ACUBE_IO_OUT_CHUNK 0x1030
#define PARAM2_CHUNK 0x1040

IOResult ACubic::Save(ISave *isave) { 
	IOResult res;
//	ULONG nb;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
/*
	if (!do_nth) {
		isave->BeginChunk(DONT_DO_NTH_CHUNK);
		isave->EndChunk();
		}
	if (!applyBlur) {
		isave->BeginChunk(DONT_APPLY_BLUR_CHUNK);
		isave->EndChunk();
		}
	if (!useEnvMap) {
		isave->BeginChunk(DONT_USE_ENV_CHUNK);
		isave->EndChunk();
		}
	if (useFile) {
		isave->BeginChunk(USE_FILE_CHUNK);
		isave->EndChunk();
		}
	for (int i=0; i<6; i++) {
		if (fileNames[i].Length()>0) {
			isave->BeginChunk(NAME_CHUNK+i);
			isave->WriteWString(fileNames[i]);
			isave->EndChunk();
			}
		}
	isave->BeginChunk(NTH_CHUNK);
	isave->Write(&nth,sizeof(nth),&nb);			
	isave->EndChunk();

	isave->BeginChunk(ACUBE_IO_IN_CHUNK);
	biInFile.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(ACUBE_IO_OUT_CHUNK);
	biOutFile.Save(isave);
	isave->EndChunk();
*/

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();

	return IO_OK;
	}	

class ACubicPostLoad : public PostLoadCallback {
	public:
		ACubic *cube;
		BOOL Param1;
		ACubicPostLoad(ACubic *b, BOOL p) {cube=b; Param1 = p;}
		void proc(ILoad *iload) {
			if (cube->pblock->GetVersion()!=ACUBIC_VERSION) {
				switch (cube->pblock->GetVersion()) {
					case 1:
//						cube->ReplaceReference(0,
//							UpdateParameterBlock(
//								pbdesc1, 2, cube->pblock,
//								pbdesc,  5, ACUBIC_VERSION));
						iload->SetObsolete();
						cube->SetNearRange(0.0f,0);
						cube->SetFarRange(500.0f,0);
						break;
					case 2:
//						cube->ReplaceReference(0,
//							UpdateParameterBlock(
//								pbdesc2, 3, cube->pblock,
//								pbdesc,  5, ACUBIC_VERSION));
						iload->SetObsolete();
						cube->SetNearRange(0.0f,0);
						cube->SetFarRange(500.0f,0);
						break;
					default:
						if (Param1)
							{
							TimeValue t = 0;
							cube->pblock->SetValue( acubic_frametype, t, cube->do_nth);
							cube->pblock->SetValue( acubic_applyblur, t, cube->applyBlur);
							cube->pblock->SetValue( acubic_useatmospheric, t, cube->useEnvMap);
							cube->pblock->SetValue( acubic_source, t, cube->useFile);
							cube->pblock->SetValue( acubic_nthframe, t, cube->nth);


							TSTR nm = cube->biOutFile.Name();
							cube->pblock->SetValue( acubic_outputname,t,nm);
							for (int i = 0 ; i < 6; i++)
								{
								cube->pblock->SetValue( acubic_bitmap_names,t,cube->fileNames[i],i);
								}	

							}
//						assert(0);
						break;
					}
				}
			//waitPostLoad--;
			delete this;
			}
	};

IOResult ACubic::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	int cid;
	BOOL Param1 = TRUE;
//	TSTR fileNames[6];    // source files


	while (IO_OK==(res=iload->OpenChunk())) {
		switch(cid=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case DONT_DO_NTH_CHUNK:
				do_nth = FALSE;
				break;
			case DONT_APPLY_BLUR_CHUNK:
				applyBlur = FALSE;
				break;
			case DONT_USE_ENV_CHUNK:
				useEnvMap = FALSE;
				break;
			case USE_FILE_CHUNK:
				useFile = TRUE;
				break;
			case NTH_CHUNK:
				iload->Read(&nth,sizeof(nth),&nb);			
				break;
			case NAME_CHUNK+0:
			case NAME_CHUNK+1:
			case NAME_CHUNK+2:
			case NAME_CHUNK+3:
			case NAME_CHUNK+4:
			case NAME_CHUNK+5:
				{
				TCHAR *buf;
				if (IO_OK==iload->ReadWStringChunk(&buf)) 
					fileNames[cid-NAME_CHUNK] = buf;
				}
				break;
			case ACUBE_IO_IN_CHUNK:
				res = biInFile.Load(iload);
				break;
			case ACUBE_IO_OUT_CHUNK:
				res = biOutFile.Load(iload);
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &acubic_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new ACubicPostLoad(this,Param1));
	return IO_OK;
	}

bool ACubic::IsLocalOutputMeaningful( ShadeContext& sc )
{
	if ( useFile && !maps )
		return false;

	if ( !useFile )
	{
		int id = sc.NodeID();
		CubicMap* cm = FindMap(id);
		if ( cm == NULL )
			return false;
	}
	return true;
}

#endif // NO_MAPTYPE_REFLECTREFRACT
