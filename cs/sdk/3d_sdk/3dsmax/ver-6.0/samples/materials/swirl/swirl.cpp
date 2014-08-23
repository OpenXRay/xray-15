/*===========================================================================*\
 | 
 |  FILE:	Swirl.cpp
 |			Twirly Tornado-type material
 |			Main shader and plugin file
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1998
 |			All Rights Reserved.
 |
 |  HIST:	Started 31-7-98
 | 
\*===========================================================================*/


/*===========================================================================*\
 | Includes and global/macro setup
\*===========================================================================*/

#include "swirl.h"

#define NSUBTEX 2
#define NCOLS 2

#define RANDOM( a ) ( ( (float)rand()/(float)RAND_MAX)*(a) )

static Class_ID SwirlClassID(0x72c8577f, 0x39a00a1b);
extern HINSTANCE hInstance;

class Swirl;

#define SWIRL_VERSION 1

#define PB_COL1		0
#define PB_COL2		1
#define PB_HS		2
#define PB_VS		3
#define PB_CV		4
#define PB_VG		5
#define PB_HG		6
#define PB_LS		7
#define PB_SH		8
#define PB_H		9
#define PB_RS		10
#define PB_BF		11
#define PB_QUAD		12
#define PB_LOCK		13
#define PB_FV		14
#define PB_J		15

#define NPARAMS 16

/*===========================================================================*\
 | Dialog class
\*===========================================================================*/

class SwirlDlg: public ParamDlg {
	public:
		// Pointer to main texture class
		Swirl *theTex;

		IMtlParams *ip;

		// HWND Panels
		HWND hwmedit;
		HWND hPanel;

		// DragNDrop manager thingy
		TexDADMgr dadMgr;		

		// Construct/Destruct
		SwirlDlg(HWND hwMtlEdit, IMtlParams *imp, Swirl *m); 
		~SwirlDlg();

		// All those lovely spinners
		ISpinnerControl *HSSpin;
		ISpinnerControl *VSSpin;
		ISpinnerControl *CVSpin;
		ISpinnerControl *HGSpin;
		ISpinnerControl *VGSpin;
		ISpinnerControl *HSpin;
		ISpinnerControl *RSSpin;
		ISpinnerControl *LSSpin;

		IColorSwatch *cs[2];
		TimeValue curTime; 

		// Texture buttons
		ICustButton *iBut[2];

		// Lock-Mortar-Symmetry button + image list
		ICustButton *iLock;
		HIMAGELIST hImageLock;

		ParamDlg *uvGenDlg;
		int isActive;
		BOOL valid;

		// Panel procedure
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

		void UpdateSubTexNames();
		void LoadDialog(BOOL draw);
		void UpdateMtlDisplay();

		void ReloadDialog();
		void ActivateDlg(BOOL onOff);
		void Invalidate() { valid = FALSE;	InvalidateRect(hPanel,NULL,0); }
		BOOL KeyAtCurTime(int id);

		Class_ID ClassID() {return SwirlClassID;  }
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing() { return (ReferenceTarget *)theTex; }
		void DeleteThis() { delete this;  }	
		void SetTime(TimeValue t);

		// DnD stuff
		int FindSubTexFromHWND(HWND hw);
};

int SwirlDlg::FindSubTexFromHWND(HWND hw)
	{
	if (hw==GetDlgItem(hPanel,IDC_SWIRL_TEX1)) return 0;
	if (hw==GetDlgItem(hPanel,IDC_SWIRL_TEX2)) return 1;
	return -1;
	}


/*===========================================================================*\
 | Material sampler
\*===========================================================================*/

class SwirlSampler: public MapSampler {
	// Pointer to main texture class
	Swirl *Swirler;
	public:
		SwirlSampler() { Swirler= NULL; }
		SwirlSampler(Swirl *c) { Swirler= c; }
		void Set(Swirl *c) { Swirler = c; }
		AColor Sample(ShadeContext& sc, float u,float v);
		AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
		float SampleMono(ShadeContext& sc, float u,float v);
		float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
	} ;


/*===========================================================================*\
 | Main texture class -- Swirl
\*===========================================================================*/

class Swirl: public Texmap { 
	friend class SwirlPostLoad;
	friend class SwirlDlg;
	Color col[NCOLS];
	float hs,vs,cv,hg,vg,ls,rs;
	int h,lock;
	
	// map enabled flag
	BOOL mapOn[2];

	UVGen *uvGen;
	IParamBlock *pblock;
	Texmap* subTex[NSUBTEX];
	TexHandle *texHandle;
	Interval texHandleValid;
	Interval ivalid;
	int rollScroll;
	SwirlSampler mysamp;
	SwirlDlg *paramDlg;
	public:
		Swirl();
		~Swirl() {
			DiscardTexHandle();
			}	
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void SetColor(int i, Color c, TimeValue t);

		void SetHS(float f, TimeValue t);
		void SetVS(float f, TimeValue t);
		void SetCV(float f, TimeValue t);
		void SetHG(float f, TimeValue t);
		void SetVG(float f, TimeValue t);
		void SetLS(float f, TimeValue t);
		void SetLOCK(int f, TimeValue t);

		void SetH(int f, TimeValue t);
		void SetRS(float f, TimeValue t);

		AColor SwirlFunc(float u,float v,ShadeContext& sc,float du,float dv);

		void NotifyChanged();
		void SwapInputs(); 
		Bitmap *BuildBitmap(int size);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);
		float MonoEvalFunction(ShadeContext& sc, float u, float v, float du, float dv);
		AColor DispEvalFunc( float u, float v);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Methods for interactive display
		void DiscardTexHandle();
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
		void GetUVTransform(Matrix3 &uvtrans) { uvGen->GetUVTransform(uvtrans); }
		int GetTextureTiling() { return  uvGen->GetTextureTiling(); }
		int GetUVWSource() { return uvGen->GetUVWSource(); }
		int GetMapChannel () { return uvGen->GetMapChannel(); }
		UVGen *GetTheUVGen() { return uvGen; }

		// Requirements
		ULONG LocalRequirements(int subMtlNum) { return uvGen->Requirements(subMtlNum); }
		
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			uvGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
			}

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { 
			return subTex[i]; 
			}
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		void InitSlotType(int sType) { if (uvGen) uvGen->InitSlotType(sType); }
		int SubTexmapOn(int i) { return (mapOn[i] && subTex[i]) ? 1 : 0; } // mjm - 9.30.99

		Class_ID ClassID() {	return SwirlClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_SWIRL); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 2+NSUBTEX; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 2+NSUBTEX; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

	};

class SwirlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Swirl; }
	const TCHAR *	ClassName() { return GetString(IDS_SWIRL); }
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return SwirlClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_2D;  }
	};

static SwirlClassDesc SwirlCD;

ClassDesc* GetSwirlDesc() { return &SwirlCD;  }

static INT_PTR CALLBACK  PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	SwirlDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (SwirlDlg*)lParam;
		theDlg->hPanel = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (SwirlDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = 1;
	int	res = theDlg->PanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

SwirlDlg::SwirlDlg(HWND hwMtlEdit, IMtlParams *imp, Swirl *m) { 
	hwmedit = hwMtlEdit;
	ip = imp;
	hPanel = NULL;
	theTex = m; 
	isActive = 0;
	valid = FALSE;
	curTime = ip->GetTime();
	uvGenDlg = theTex->uvGen->CreateParamDlg(hwmedit, imp);

	// Start up DnD and init the custButtons
	dadMgr.Init(this);
	for (int i=0; i<NSUBTEX; i++) 
	iBut[i] = NULL;
	iLock = NULL;

	hPanel = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_SWIRL),
		PanelDlgProc, 
		GetString(IDS_SWIRLPARAMS), 
		(LPARAM)this );		
	}

// Reload the dialog panel
void SwirlDlg::ReloadDialog() {
	Interval valid;
	theTex->Update(curTime, valid);
	LoadDialog(FALSE);
	}

// Set time
void SwirlDlg::SetTime(TimeValue t) {
	Interval valid;
	if (t!=curTime) {
		uvGenDlg->SetTime(t);
		curTime = t;
		theTex->Update(curTime, valid);
		LoadDialog(FALSE);
		InvalidateRect(hPanel,NULL,0);
		}
	}

SwirlDlg::~SwirlDlg() {
	theTex->paramDlg = NULL;
	ReleaseISpinner(HSSpin);
	ReleaseISpinner(VSSpin);
	ReleaseISpinner(HGSpin);
	ReleaseISpinner(VGSpin);
	ReleaseISpinner(CVSpin);
	ReleaseISpinner(HSpin);
	ReleaseISpinner(RSSpin);
	ReleaseISpinner(LSSpin);

	ReleaseIColorSwatch(cs[0]);
	ReleaseIColorSwatch(cs[1]);

	for (int i=0; i<NSUBTEX; i++) 
		ReleaseICustButton(iBut[i]);
	ReleaseICustButton(iLock);
	ImageList_Destroy(hImageLock);

	SetWindowLongPtr(hPanel, GWLP_USERDATA, NULL);
	uvGenDlg->DeleteThis();
	hPanel =  NULL;
	}


// Dialog Code
static int colID[2] = { IDC_SWIRL_COL1, IDC_SWIRL_COL2 };
static int mapOnId[NSUBTEX] = { IDC_CHKMAP1, IDC_CHKMAP2 };
static int subTexId[NSUBTEX] = { IDC_SWIRL_TEX1, IDC_SWIRL_TEX2 };

BOOL SwirlDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:
			{
			
			HSSpin = SetupFloatSpinner(hwndDlg, IDC_HS_SPIN, IDC_HS_EDIT,-10.0f,10.0f,0.1f);
			HSSpin->SetScale(0.05f);
			VSSpin = SetupFloatSpinner(hwndDlg, IDC_VS_SPIN, IDC_VS_EDIT,-20.0f,20.0f,0.1f);
			VSSpin->SetScale(0.05f);

			CVSpin = SetupFloatSpinner(hwndDlg, IDC_CV_SPIN, IDC_CV_EDIT,0.0f,4.0f,0.1f);
			CVSpin->SetScale(0.01f);
			VGSpin = SetupFloatSpinner(hwndDlg, IDC_VG_SPIN, IDC_VG_EDIT,-10.0f,10.0f,0.1f);
			HGSpin = SetupFloatSpinner(hwndDlg, IDC_HG_SPIN, IDC_HG_EDIT,-10.0f,10.0f,0.1f);
			LSSpin = SetupFloatSpinner(hwndDlg, IDC_LS_SPIN, IDC_LS_EDIT,0.0f,3.0f,0.1f);

			HSpin = SetupIntSpinner(hwndDlg, IDC_H_SPIN, IDC_H_EDIT,0,10,1);
			RSSpin = SetupFloatSpinner(hwndDlg, IDC_RS_SPIN, IDC_RS_EDIT,0.0f,65535.0f,1.0f);


			SetCheckBox(hwndDlg,IDC_LOCK,theTex->lock);
			hImageLock = ImageList_Create(15, 14, ILC_COLOR24| ILC_MASK , 2, 0);
			HBITMAP hLocked		= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LOCK1));
			HBITMAP hLockedM	= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LOCK1_M));

			HBITMAP hUnlocked	= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LOCK2));
			HBITMAP hUnlockedM	= LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_LOCK2_M));
			ImageList_Add(hImageLock, hLocked, hLockedM);
			ImageList_Add(hImageLock, hUnlocked, hUnlockedM);

			iLock = GetICustButton(GetDlgItem(hwndDlg,IDC_LOCK));
			iLock->SetType(CBT_CHECK);
			iLock->SetImage(hImageLock, 0,1,0,1, 15, 14);
			
			for (int i=0; i<NCOLS; i++) 
   				cs[i] = GetIColorSwatch(GetDlgItem(hwndDlg, colID[i]),
   					theTex->col[i],theTex->GetSubTexmapSlotName(i).data());

			for (i=0; i<NSUBTEX; i++) {
				iBut[i] = GetICustButton(GetDlgItem(hwndDlg,subTexId[i]));
				iBut[i]->SetDADMgr(&dadMgr);
				SetCheckBox(hwndDlg, mapOnId[i], theTex->mapOn[i]);
				}

			return TRUE;
			}
			break;
		case WM_COMMAND:  
		    switch (id) {
				case IDC_SWIRL_TEX1: 
					PostMessage(hwmedit,WM_TEXMAP_BUTTON,0 ,(LPARAM)theTex);
					break;
				case IDC_SWIRL_TEX2: 
					PostMessage(hwmedit,WM_TEXMAP_BUTTON,1 ,(LPARAM)theTex);
					break;

				case IDC_TEX_SWAP:
					theTex->SwapInputs(); 
					cs[0]->SetColor(theTex->col[0]);
					cs[1]->SetColor(theTex->col[1]);
					theTex->NotifyChanged();
					UpdateMtlDisplay();
					UpdateSubTexNames();
					LoadDialog(FALSE);
					break;

					// Texture maps on/off
				case IDC_CHKMAP1:
					theTex->mapOn[0] = GetCheckBox(hwndDlg,id);
					theTex->NotifyChanged();
					UpdateMtlDisplay();
					UpdateSubTexNames();
					LoadDialog(FALSE);
					break;
				case IDC_CHKMAP2:
					theTex->mapOn[1] = GetCheckBox(hwndDlg,id);
					theTex->NotifyChanged();
					UpdateMtlDisplay();
					UpdateSubTexNames();
					LoadDialog(FALSE);
					break;

				case IDC_LOCK:
					theTex->SetLOCK(iLock->IsChecked(),curTime); 
					if( iLock->IsChecked() ){
						if(theTex->hg>theTex->vg){
							theTex->SetVG(HGSpin->GetFVal(),curTime); 
							VGSpin->SetValue(HGSpin->GetFVal(),TRUE);
						}
						if(theTex->vg>theTex->hg){
							theTex->SetHG(VGSpin->GetFVal(),curTime); 
							HGSpin->SetValue(VGSpin->GetFVal(),TRUE);
						}
					}
				break;
			}
			break;
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_COLCHANGE));
			else theHold.Cancel();
			break;
		case CC_COLOR_CHANGE:
			{
			int id = LOWORD(wParam);
			int buttonUp = HIWORD(wParam); 
			int n = (id==IDC_SWIRL_COL1)?0:1;
			if (buttonUp) theHold.Begin();
			theTex->SetColor(n,Color(cs[n]->GetColor()),curTime);
			cs[n]->SetKeyBrackets(KeyAtCurTime(n?PB_COL2:PB_COL1));
			if (buttonUp) {
				theHold.Accept(GetString(IDS_PARAMCHANGE));
				theTex->NotifyChanged();
			    UpdateMtlDisplay();
				}
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			ip->RollupMouseMessage(hwndDlg,msg,wParam,lParam);
			return FALSE;
		case WM_PAINT: 	
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			break;
		case WM_CLOSE: 	break;       
		case WM_DESTROY:		break;
		case CC_SPINNER_CHANGE:
			if (!theHold.Holding()) theHold.Begin();
			switch (id) {
				case IDC_HS_SPIN: 
					theTex->SetHS(HSSpin->GetFVal(),curTime); 
					HSSpin->SetKeyBrackets(KeyAtCurTime(PB_HS));
					break;
				case IDC_VS_SPIN: 
					theTex->SetVS(VSSpin->GetFVal(),curTime); 
					VSSpin->SetKeyBrackets(KeyAtCurTime(PB_VS));
					break;
				case IDC_CV_SPIN: 
					theTex->SetCV(CVSpin->GetFVal(),curTime);
					CVSpin->SetKeyBrackets(KeyAtCurTime(PB_CV));
					break;
				case IDC_VG_SPIN: 
					theTex->SetVG(VGSpin->GetFVal(),curTime);
					if(theTex->lock==1) {
						theTex->SetHG(VGSpin->GetFVal(),curTime); 
						HGSpin->SetValue(VGSpin->GetFVal(),TRUE);
					}
					VGSpin->SetKeyBrackets(KeyAtCurTime(PB_VG));
					HGSpin->SetKeyBrackets(KeyAtCurTime(PB_HG));
					break;
				case IDC_HG_SPIN: 
					theTex->SetHG(HGSpin->GetFVal(),curTime); 
					if(theTex->lock==1) {
						theTex->SetVG(HGSpin->GetFVal(),curTime); 
						VGSpin->SetValue(HGSpin->GetFVal(),TRUE);
					}
					HGSpin->SetKeyBrackets(KeyAtCurTime(PB_HG));
					VGSpin->SetKeyBrackets(KeyAtCurTime(PB_VG));
					break;
				case IDC_LS_SPIN: 
					theTex->SetLS(LSSpin->GetFVal(),curTime); 
					LSSpin->SetKeyBrackets(KeyAtCurTime(PB_LS));
					break;
				case IDC_H_SPIN: 
					theTex->SetH(HSpin->GetIVal(),curTime); 
					HSpin->SetKeyBrackets(KeyAtCurTime(PB_H));
					break;
				case IDC_RS_SPIN: 
					theTex->SetRS(RSSpin->GetFVal(),curTime); 
					RSSpin->SetKeyBrackets(KeyAtCurTime(PB_RS));
					break;

			}
			break;
		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			break;		
		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP: 
			if (HIWORD(wParam) || msg==WM_CUSTEDIT_ENTER) theHold.Accept(GetString(IDS_PARAMCHANGE));
			else theHold.Cancel();
			theTex->NotifyChanged();
		    UpdateMtlDisplay();
			break;

    	}
	return FALSE;
	}

BOOL SwirlDlg::KeyAtCurTime(int id) { return theTex->pblock->KeyFrameAtTime(id,ip->GetTime()); }

void SwirlDlg::UpdateSubTexNames() {
	for (int i=0; i<NSUBTEX; i++) {
		Texmap *m = theTex->subTex[i];
		TSTR nm;
		if (m) 	nm = m->GetFullName();
		else 	nm = GetString(IDS_NONE);
		iBut[i]->SetText(nm.data());
		}
	}


void SwirlDlg::LoadDialog(BOOL draw) {
	if (theTex) {
		Interval valid;
		theTex->Update(curTime,valid);
		BOOL kbON;

		HSSpin->SetValue(theTex->hs,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(2,curTime);
		HSSpin->SetKeyBrackets(kbON);

		VSSpin->SetValue(theTex->vs,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(3,curTime);
		VSSpin->SetKeyBrackets(kbON);

		CVSpin->SetValue(theTex->cv,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(4,curTime);
		CVSpin->SetKeyBrackets(kbON);

		VGSpin->SetValue(theTex->vg,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(5,curTime);
		VGSpin->SetKeyBrackets(kbON);

		HGSpin->SetValue(theTex->hg,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(6,curTime);
		HGSpin->SetKeyBrackets(kbON);

		LSSpin->SetValue(theTex->ls,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(7,curTime);
		LSSpin->SetKeyBrackets(kbON);

		RSSpin->SetValue(theTex->rs,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(10,curTime);
		RSSpin->SetKeyBrackets(kbON);

		HSpin->SetValue(theTex->h,FALSE);
		kbON = theTex->pblock->KeyFrameAtTime(9,curTime);
		HSpin->SetKeyBrackets(kbON);

		
		iLock->SetCheck( theTex->lock );

		SetCheckBox(hPanel, IDC_CHKMAP1, theTex->mapOn[0]);
		SetCheckBox(hPanel, IDC_CHKMAP2, theTex->mapOn[1]);

		cs[0]->SetColor(theTex->col[0]);
		cs[1]->SetColor(theTex->col[1]);
		UpdateSubTexNames();
		}
	}

void SwirlDlg::SetThing(ReferenceTarget *m) {
	assert (m->ClassID()==SwirlClassID);
	assert (m->SuperClassID()==TEXMAP_CLASS_ID);
	if (theTex) theTex->paramDlg = NULL;
	theTex = (Swirl *)m;
	uvGenDlg->SetThing(theTex->uvGen);
	if (theTex)
		theTex->paramDlg = this;
	LoadDialog(TRUE);
	}

void SwirlDlg::UpdateMtlDisplay() { 
	theTex->DiscardTexHandle();  
	ip->MtlChanged();  
	}

void SwirlDlg::ActivateDlg(BOOL onOff) {
	for (int i=0; i<NCOLS; i++)
		cs[i]->Activate(onOff);
	}

AColor SwirlSampler::SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) {
	return Swirler->EvalFunction(sc, u, v, du, dv);
	}
AColor SwirlSampler::Sample(ShadeContext& sc, float u,float v) {
	return Swirler->EvalFunction(sc, u, v, 0.0f, 0.0f);
	}

float SwirlSampler::SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv) {
	return Swirler->MonoEvalFunction(sc, u, v, du, dv);
	}
float SwirlSampler::SampleMono(ShadeContext& sc, float u,float v) {
	return Swirler->MonoEvalFunction(sc, u, v, 0.0f, 0.0f);
	}



static ParamBlockDescID pbdesc2[] = {
	{ TYPE_RGBA, NULL, TRUE,0 },
	{ TYPE_RGBA, NULL, TRUE,1 },
	{ TYPE_FLOAT, NULL, TRUE,2 }, 
	{ TYPE_FLOAT, NULL, TRUE,3 }, 
	{ TYPE_FLOAT, NULL, TRUE,4 }, 
	{ TYPE_FLOAT, NULL, TRUE,5 }, 
	{ TYPE_FLOAT, NULL, TRUE,6 }, 
	{ TYPE_FLOAT, NULL, TRUE,7 }, 
	{ TYPE_FLOAT, NULL, FALSE,8 }, 
	{ TYPE_INT, NULL, TRUE,9 }, 
	{ TYPE_FLOAT, NULL, TRUE,10 },
	{ TYPE_FLOAT, NULL, FALSE,11}, 
	{ TYPE_INT, NULL, FALSE,12 },
	{ TYPE_INT, NULL, FALSE,13 },
	{ TYPE_FLOAT, NULL, FALSE,14}, 
	{ TYPE_FLOAT, NULL, FALSE,15}, 
	};


void Swirl::Reset() {
	if (uvGen) uvGen->Reset();
	else ReplaceReference( 0, GetNewDefaultUVGen());	
	ReplaceReference( 1, CreateParameterBlock( pbdesc2, NPARAMS, SWIRL_VERSION) );	
	ivalid.SetEmpty();
	DeleteReference(2);
	DeleteReference(3);

	SetColor(0, Color(0.0f,0.1f,0.2f), TimeValue(0));
	SetColor(1, Color(0.9f,0.58f,0.3f), TimeValue(0));

	SetHS(2.0f, TimeValue(0));
	SetVS(1.0f, TimeValue(0));
	SetCV(0.4f, TimeValue(0));

	SetVG(-0.5f, TimeValue(0));
	SetHG(-0.5f, TimeValue(0));

	SetLS(1.0f, TimeValue(0));
	SetH(4, TimeValue(0));
	SetRS(RANDOM(65535.0f), TimeValue(0));

	SetLOCK(1, TimeValue(0));
	mapOn[0] = mapOn[1] = 1;

	}

void Swirl::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Swirl::Swirl() {
	mysamp.Set(this);
	texHandle = NULL;
	subTex[0] = subTex[1] = NULL;
	mapOn[0] = mapOn[1] = FALSE; // mjm - 9.30.99 - just in case
	pblock = NULL;
	uvGen = NULL;
	paramDlg = NULL;
	Reset();
	rollScroll=0;
	}


void Swirl::DiscardTexHandle() {
	if (texHandle) {
		texHandle->DeleteThis();
		texHandle = NULL;
		}
	}

void Swirl::ActivateTexDisplay(BOOL onoff) {
	if (!onoff) 
		DiscardTexHandle();
	}

DWORD_PTR Swirl::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}

	Bitmap *bm;
	Interval v;
	Update(t,v);
	bm = BuildBitmap(thmaker.Size());
	texHandle = thmaker.CreateHandle(bm,uvGen->SymFlags());
	texHandleValid.SetInfinite();
	bm->DeleteThis();

	Color ac;
	pblock->GetValue( PB_COL1, t, ac, texHandleValid );
	pblock->GetValue( PB_COL2, t, ac, texHandleValid );
	pblock->GetValue( PB_HS, t, hs, texHandleValid );
	pblock->GetValue( PB_VS, t, vs, texHandleValid );
	pblock->GetValue( PB_CV, t, cv, texHandleValid );
	pblock->GetValue( PB_VG, t, vg, texHandleValid );
	pblock->GetValue( PB_HG, t, hg, texHandleValid );
	pblock->GetValue( PB_RS, t, rs, texHandleValid );
	pblock->GetValue( PB_LS, t, ls, texHandleValid );
	pblock->GetValue( PB_H, t, h, texHandleValid );
	pblock->GetValue( PB_LOCK, t, lock, texHandleValid );
		
	return texHandle->GetHandle();
	}

inline UWORD FlToWord(float r) {
	return (UWORD)(65535.0f*r);
	}

Bitmap *Swirl::BuildBitmap(int size) {
	float u,v;
	BitmapInfo bi;
	bi.SetName(_T("SwrTemp"));
	bi.SetWidth(size);
	bi.SetHeight(size);
	bi.SetType(BMM_TRUE_32);
	Bitmap *bm = TheManager->Create(&bi);
	if (bm==NULL) return NULL;
	PixelBuf l64(size);
	float d = 1.0f/float(size);
	v = 1.0f - 0.5f*d;

	for (int y=0; y<size; y++) {
		BMM_Color_64 *p64=l64.Ptr();
		u = 0.0f;
		for (int x=0; x<size; x++, p64++) {
			AColor c = DispEvalFunc(u,v);
			p64->r = FlToWord(c.r); 
			p64->g = FlToWord(c.g); 
			p64->b = FlToWord(c.b);
			p64->a = 0xffff; 
			u += d;
			}
		bm->PutPixels(0,y, size, l64.Ptr()); 
		v -= d;
		}
	return bm;
	}


AColor Swirl::SwirlFunc(float u,float v,ShadeContext& sc,float du,float dv ){

	float offset = ls;
	float scale = hs;
	float twist = vs;
	float omega = cv;
	float octaves = (float)h;

	Point3 Ptexture,PtN;
	float rsq;                // Used in calculation of swirl 
	float angle;              // Swirl angle 
	float sine, cosine;       // sin and cos of angle 
	float l, o, a, i;         // Loop control for fractal sum 
	float value;              // Fractal sum is stored here 


	u+=hg; v+=vg;

		rsq = (u)*(u) + (v)*(v); 


	  angle = twist * TWOPI * rsq;
	  sine = (float)sin (angle);
	  cosine = (float)cos (angle);

	  Point3 PP (v*cosine - u*sine,
			  v*sine + u*cosine,(float)rs);

	  /* Compute VLfBm */
	  l = 1;  o = 1;  a = 0;
	  for (i = 0;  i < octaves;  i += 1) {
		  a += o * noise3 (PP * l);
		  l *= 2;
		  o *= omega;
		}

		value =  ((offset * scale) * a);
 
		AColor r1 = mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0]; 
		AColor r2 = mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1]; 

		AColor rslt = (r1*value)+(r2*(1.0f-value));
		rslt.ClampMinMax();

		return rslt;

}


AColor Swirl::DispEvalFunc( float u, float v) {
	
	float offset = ls;
	float scale = hs;
	float twist = vs;
	float omega = cv;
	float octaves = (float)h;

	Point3 Ptexture,PtN;
	float rsq;                // Used in calculation of twist 
	float angle;              // Twist angle 
	float sine, cosine;       // sin and cos of angle 
	float l, o, a, i;         // Loop control for fractal sum 
	float value;              // Fractal sum is stored here 


	u+=hg; v+=vg;

		rsq = (u)*(u) + (v)*(v); 

	  angle = twist * TWOPI * rsq;
	  sine = (float)sin (angle);
	  cosine = (float)cos (angle);

	  Point3 PP (v*cosine - u*sine,
			  v*sine + u*cosine,(float)rs);

	  /* Compute VLfBm */
	  l = 1;  o = 1;  a = 0;
	  for (i = 0;  i < octaves;  i += 1) {
		  a += o * noise3 (PP * l);
		  l *= 2;
		  o *= omega;
		}

		value =  ((offset * scale) * a);
 
		AColor r1 = col[0]; 
		AColor r2 = col[1]; 

		AColor k = (r1*value)+(r2*(1.0f-value));
		k.ClampMinMax();

	return k;
	
}


AColor Swirl::EvalFunction(ShadeContext& sc, float u, float v, float du, float dv) {

return SwirlFunc(u,v,sc,du,dv);

}



float Swirl::MonoEvalFunction(ShadeContext& sc, float u, float v, float du, float dv) {

	AColor k = SwirlFunc(u,v,sc,du,dv);
	k.ClampMinMax();
	
	return Intens(k); 
}


AColor Swirl::EvalColor(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	return uvGen->EvalUVMap(sc,&mysamp);
	}

float Swirl::EvalMono(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	return uvGen->EvalUVMapMono(sc,&mysamp);
	}

Point3 Swirl::EvalNormalPerturb(ShadeContext& sc) {
	Point3 dPdu, dPdv;
	if (gbufID) sc.SetGBufferID(gbufID);
	uvGen->GetBumpDP(sc,dPdu,dPdv);
	Point2 dM = (.02f)*uvGen->EvalDeriv(sc,&mysamp);
	return dM.x*dPdu+dM.y*dPdv;
	}

RefTargetHandle Swirl::Clone(RemapDir &remap) {
	Swirl *mnew = new Swirl();
	*((MtlBase*)mnew) = *((MtlBase*)this);
	mnew->ReplaceReference(0,remap.CloneRef(uvGen));
	mnew->ReplaceReference(1,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];

	mnew->hs = hs;
	mnew->vs = vs;
	mnew->cv = cv;
	mnew->vg = vg;
	mnew->hg = hg;
	mnew->rs = rs;
	mnew->ls = ls;
	mnew->h = h;
	mnew->lock=lock;
	
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		mnew->mapOn[i] = mapOn[i];
		if (subTex[i])
			mnew->ReplaceReference(i+2,remap.CloneRef(subTex[i]));
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Swirl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	SwirlDlg *dm = new SwirlDlg(hwMtlEdit, imp, this);
	paramDlg = dm;
	dm->LoadDialog(TRUE);	
	return dm;	
	}


void Swirl::Update(TimeValue t, Interval& valid) {		
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		uvGen->Update(t,ivalid);
		pblock->GetValue( PB_COL1, t, col[0], ivalid );
		col[0].ClampMinMax();

		pblock->GetValue( PB_COL2, t, col[1], ivalid );
		col[1].ClampMinMax();

		pblock->GetValue( PB_HS, t, hs, ivalid );
		pblock->GetValue( PB_VS, t, vs, ivalid );
		pblock->GetValue( PB_CV, t, cv, ivalid );
		pblock->GetValue( PB_VG, t, vg, ivalid );
		pblock->GetValue( PB_HG, t, hg, ivalid );
		pblock->GetValue( PB_RS, t, rs, ivalid );
		pblock->GetValue( PB_LS, t, ls, ivalid );
		pblock->GetValue( PB_H, t, h, ivalid );
		pblock->GetValue( PB_LOCK, t, lock, ivalid );



		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}


void Swirl::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	pblock->SetValue( i==0?PB_COL1:PB_COL2, t, c);
	}

void Swirl::SwapInputs() {
	BOOL bt = mapOn[0]; mapOn[0] = mapOn[1]; mapOn[1] = bt;
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(PB_COL1,PB_COL2);
	}



void Swirl::SetHS(float f, TimeValue t) { 
	hs = f; 
	pblock->SetValue( PB_HS, t, f);
	}
void Swirl::SetVS(float f, TimeValue t) { 
	vs = f; 
	pblock->SetValue( PB_VS, t, f);
	}
void Swirl::SetCV(float f, TimeValue t) { 
	cv = f; 
	pblock->SetValue( PB_CV, t, f);
	}
void Swirl::SetVG(float f, TimeValue t) { 
	vg = f; 
	pblock->SetValue( PB_VG, t, f);
	}
void Swirl::SetHG(float f, TimeValue t) { 
	hg = f; 
	pblock->SetValue( PB_HG, t, f);
	}
void Swirl::SetLS(float f, TimeValue t) { 
	ls = f; 
	pblock->SetValue( PB_LS, t, f);
	}


void Swirl::SetH(int f, TimeValue t) { 
	h = f; 
	pblock->SetValue( PB_H, t, f);
	}
void Swirl::SetRS(float f, TimeValue t) { 
	rs = f; 
	pblock->SetValue( PB_RS, t, f);
	}
void Swirl::SetLOCK(int f, TimeValue t) { 
	lock = f; 
	pblock->SetValue( PB_LOCK, t, f);
	}



RefTargetHandle Swirl::GetReference(int i) {
	switch(i) {
		case 0: return uvGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
		}
	}

void Swirl::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: uvGen = (UVGen *)rtarg; break;
		case 1:	pblock = (IParamBlock *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
	}

void Swirl::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2,m);
	ivalid.SetEmpty();
	if (paramDlg)
		paramDlg->UpdateSubTexNames();
	}

TSTR Swirl::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_SWIRL_NAME)); 
		case 1:  return TSTR(GetString(IDS_BASE)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Swirl::SubAnim(int i) {
	switch (i) {
		case 0: return uvGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
		}
	}

TSTR Swirl::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_COORDS));		
		case 1: return TSTR(GetString(IDS_PARAMS));		
		default: return GetSubTexmapTVName(i-2);
		}
	}


/*===========================================================================*\
 | Notification handler
\*===========================================================================*/

RefResult Swirl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget!=uvGen) {
				if (paramDlg&&!paramDlg->isActive) 
					paramDlg->Invalidate();
				}
			break;

		case REFMSG_UV_SYM_CHANGE:
			DiscardTexHandle();  
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_HS: 
				case PB_VS: 
				case PB_CV: 
				case PB_VG: 
				case PB_HG: 
				case PB_LS: 
				case PB_RS: 
				case PB_LOCK: 
				case PB_H: gpd->dim = defaultDim; break;
				case PB_COL1: 
				case PB_COL2: gpd->dim = stdColor255Dim; break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) {
			case PB_COL1:	gpn->name= TSTR(GetString(IDS_SWIRL_NAME));break;
			case PB_COL2:	gpn->name= TSTR(GetString(IDS_BASE));break;
			case PB_HS:		gpn->name= TSTR(GetString(IDS_SWIRLINTENS));break;
			case PB_VS:		gpn->name= TSTR(GetString(IDS_TWIST));break;
			case PB_CV:		gpn->name= TSTR(GetString(IDS_COLCONTRAST));break;
			case PB_VG:		gpn->name= TSTR(GetString(IDS_CENTERY));break;
			case PB_HG:		gpn->name= TSTR(GetString(IDS_CENTERX));break;
			case PB_RS:		gpn->name= TSTR(GetString(IDS_RANDOMSEED));break;
			case PB_LS:		gpn->name= TSTR(GetString(IDS_SWIRLAMT));break;
			case PB_H:		gpn->name= TSTR(GetString(IDS_DETAIL));break;
			case PB_LOCK:	gpn->name= TSTR(GetString(IDS_LOCK));break;
			}
				return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}


/*===========================================================================*\
 | ISave and ILoad stuff
\*===========================================================================*/

#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000

IOResult Swirl::Save(ISave *isave) { 
	IOResult res;
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	if (!mapOn[0]) {
		isave->BeginChunk(MAPOFF_CHUNK+0);
		isave->EndChunk();
	}
	if (!mapOn[1]) {
		isave->BeginChunk(MAPOFF_CHUNK+1);
		isave->EndChunk();
	}
	return IO_OK;
	}	
	  
IOResult Swirl::Load(ILoad *iload) { 
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}
