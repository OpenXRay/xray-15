/**********************************************************************
 *<
	FILE: MIRROR.CPP

	DESCRIPTION: Flat Mirror Reflection.

	CREATED BY: Dan Silva

	HISTORY: 12/4/98 Updated to Param Block 2 Peter Watje

 *>	Copyright (c) 1994, All Rights Reserved.
 ***
 *******************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include <bmmlib.h>
#include "render.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "notify.h"
#include "buildver.h"
#ifndef NO_MAPTYPE_FLATMIRROR // orb 01-03-2001 Removing map types

/*
#define PB_BLUR	      0
#define PB_NSAMT      1
#define PB_NSLEV      2
#define PB_NSSIZ      3
#define PB_NSPHS      4
*/

//#define DBG
extern HINSTANCE hInstance;

static Class_ID mirrorClassID(MIRROR_CLASS_ID,0);

#define DISTORT_NONE  0
#define DISTORT_BUMP  1
#define DISTORT_NOISE 2

#define NOISE_REGULAR 0
#define NOISE_FRACTAL 1
#define NOISE_TURB    2


// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { fmirror_params, };  // pblock ID
// mirror_params param IDs
enum 
{ 
	fmirror_blur, 
	fmirror_nsamt,	fmirror_nslev,	fmirror_nssiz,	fmirror_nsphs,
	fmirror_apply,
	fmirror_nthframe,
	fmirror_frame,
	fmirror_useenviroment,
	fmirror_applytofaceid,
	fmirror_faceid,
	fmirror_distortiontype,
	fmirror_noisetype,
	};


//---------------------------------------------------

class MirrorMap {	
	public:
		Bitmap  *bm;
		IPoint2 org;
		int nodeID;
		float xfact,yfact;
		TimeValue mapTime;  // when the mirror was last rendered
		Matrix3 pltm;  // reflection in plane in camera coords
		MirrorMap *next;
		MirrorMap() { next = NULL; bm = NULL; nodeID = -1; mapTime = 0; }
		~MirrorMap() { FreeMap(); }
		void FreeMap() { if (bm) bm->DeleteThis(); bm = NULL; } 
		
		int AllocMap(int w, int h, BOOL createAlpha);
	};

int MirrorMap::AllocMap(int w, int h, BOOL createAlpha) {
	if ( bm && w==bm->Width() && h==bm->Height())
		return 1;
	BitmapInfo bi;
	if (bm) bm->DeleteThis();
	bi.SetName(_T(""));
	bi.SetWidth(w);
	bi.SetHeight(h);
	bi.SetType(BMM_TRUE_32);
	if (createAlpha)
		bi.SetFlags(MAP_HAS_ALPHA);
	bi.SetCustomFlag(BMM_CUSTOM_GAMMA);
	bi.SetCustomGamma(1.0f);

	bm = TheManager->Create(&bi);
	return 1;
	}

/*
class Mirror;

class MirrorDlg: public ParamDlg {
	public:
		HWND hwmedit;	 	// window handle of the materials editor dialog
		IMtlParams *ip;
		Mirror *theTex;	// current Mirror being edited.
		HWND hPanel; 		// Rollup pane
		ISpinnerControl *blurSpin,*nthSpin;
		ISpinnerControl *nsAmtSpin, *nsLevSpin, *nsSizSpin, *nsPhsSpin, *mtlIdSpin;
		TimeValue curTime; 
		int isActive;
		BOOL valid;

		//-----------------------------
		MirrorDlg(HWND hwMtlEdit, IMtlParams *imp, Mirror *m); 
		~MirrorDlg();
		void EnableNoise();
		BOOL PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
		void LoadDialog(BOOL draw);  // stuff params into dialog
		void ReloadDialog();
		void UpdateMtlDisplay() { ip->MtlChanged(); }
		void ActivateDlg(BOOL onOff);
		void Invalidate() { valid = FALSE;	InvalidateRect(hPanel,NULL,0); }
		BOOL KeyAtCurTime(int id);

		// methods inherited from ParamDlg:
		Class_ID ClassID() {return mirrorClassID;  }
		void SetThing(ReferenceTarget *m);
		ReferenceTarget* GetThing() { return (ReferenceTarget *)theTex; }
		void DeleteThis() { delete this;  }	
		void SetTime(TimeValue t);
	};

*/

class MirrorDlg;

//--------------------------------------------------------------
// Mirror: 
//--------------------------------------------------------------

class Mirror: public StdMirror { 
	friend class MirrorPostLoad;
//	friend class MirrorDlg;
    MirrorMap *maps;
	Interval ivalid;
	int rollScroll;
	static MirrorDlg *paramDlg;
	public:
		BOOL Param1;
		int nth;
		float blur;
		float nsAmt,nsSize,nsPhase,nsAmt2,nsLev;
		BOOL applyBlur;
		BOOL do_nth;
		BOOL useEnvMap;
		BOOL distortType;
		BOOL useMtlID;
		BOOL createAlpha;
		int mtlID;
		int noiseType;
		IParamBlock2 *pblock;   // ref #1
		Mirror();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		// methods inherited from StdMirror:
		void SetDoNth(BOOL onoff) { do_nth = onoff;}
		void SetNth(int n){ nth = n;}
		void SetApplyBlur(BOOL onoff) { applyBlur = onoff; }
		void SetBlur(float b, TimeValue t);
		void SetNsAmt(float v, TimeValue t);
		void SetNsLev(float v, TimeValue t);
		void SetNsSiz(float v, TimeValue t);
		void SetNsPhase(float v, TimeValue t);
		void UseHighDynamicRange(BOOL onoff);
		int IsHighDynamicRange( ) const;
		BOOL GetDoNth() { return do_nth; }
		int GetNth() { return nth;}
		BOOL GetApplyBlur() { return applyBlur;}
		float GetBlur(TimeValue t) { 
//			return pblock->GetFloat(PB_BLUR,t); 
			return pblock->GetFloat(fmirror_blur,t); 
			}
		float NoiseFunc(Point3 p, float levels, float time);
		void EnableStuff();


		void NotifyChanged();
	
		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);

		// optimized evaluation for monochrome use
		float EvalMono(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		BOOL HandleOwnViewPerturb() { return TRUE; }

		ULONG LocalRequirements(int subMtlNum) {	
			return MTLREQ_AUTOMIRROR;
			}

		int BuildMaps(TimeValue t, RenderMapsContext &rmc);
		int DoThisFrame(TimeValue t, BOOL fieldRender, TimeValue mapTime);
		MirrorMap *FindMap(int nodeNum);
		void FreeMaps();
		int RenderBegin(TimeValue t, ULONG flags) { return 1;}
		int RenderEnd(TimeValue t) { FreeMaps(); return 1; }

		Class_ID ClassID() {	return mirrorClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_FLATMIRROR); }  
		void DeleteThis() { delete this; }	

		int NumSubs() {return 1; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		int SetProperty(ULONG id, void *data) { 
			 switch (id) {
				case 0x9500000: 
					createAlpha = data?1:0;
					break;
				}
			return 0;
			}

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		bool IsLocalOutputMeaningful( ShadeContext& sc );
	};

class MirrorDlg : public ParamMap2UserDlgProc {
	public:
		Mirror *mirror;
		MirrorDlg(Mirror *m) { mirror = m; }
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) { return FALSE; }		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			mirror = (Mirror *)m;
			mirror->EnableStuff();
			}
	};


MirrorDlg* Mirror::paramDlg = NULL;

class MirrorClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Mirror; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_FLATMIRROR_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return mirrorClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_ENV;  }

	const TCHAR*	InternalName() { return _T("flatMirror"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static MirrorClassDesc mirrorCD;

ClassDesc* GetMirrorDesc() { return &mirrorCD;  }
/*
static INT_PTR CALLBACK  PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	MirrorDlg *theDlg;
	if (msg==WM_INITDIALOG) {
		theDlg = (MirrorDlg*)lParam;
		theDlg->hPanel = hwndDlg;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA,lParam);
		}
	else {
	    if ( (theDlg = (MirrorDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
		}
	theDlg->isActive = 1;
	int	res = theDlg->PanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->isActive = 0;
	return res;
	}

MirrorDlg::MirrorDlg(HWND hwMtlEdit, IMtlParams *imp, Mirror *m) { 
	hwmedit = hwMtlEdit;
	ip = imp;
	hPanel = NULL;
	theTex = m; 
	isActive = 0;
	valid = FALSE;
	hPanel = ip->AddRollupPage( 
		hInstance,
		MAKEINTRESOURCE(IDD_MIRROR),
		PanelDlgProc, 
		GetString(IDS_DS_MIRROR_PARAMS), 
		(LPARAM)this );		
	curTime = imp->GetTime();
	}

void MirrorDlg::ReloadDialog() {
	Interval valid;
	theTex->Update(curTime, valid);
	LoadDialog(FALSE);
	}

void MirrorDlg::SetTime(TimeValue t) {
	Interval valid;
	if (t!=curTime) {
		curTime = t;
		theTex->Update(curTime, valid);
		LoadDialog(FALSE);
		InvalidateRect(hPanel,NULL,0);
		}
	}

MirrorDlg::~MirrorDlg() {
	theTex->paramDlg = NULL;
	ReleaseISpinner(blurSpin);
	ReleaseISpinner(nthSpin);
	ReleaseISpinner(nsAmtSpin);
	ReleaseISpinner(nsLevSpin);
	ReleaseISpinner(nsSizSpin);
	ReleaseISpinner(nsPhsSpin);
	ReleaseISpinner(mtlIdSpin);
	SetWindowLongPtr(hPanel, GWLP_USERDATA, NULL);
	hPanel =  NULL;
	}

void MirrorDlg::EnableNoise() {
	if (theTex&&hPanel) {
		BOOL en = (theTex->distortType==DISTORT_NOISE)?1:0;
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_REG),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_FRAC),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_TURB),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_ANI_EDIT),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_SIZ_EDIT),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_ANI_SPIN),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_SIZ_SPIN),en);
		en = en&&(theTex->noiseType!=NOISE_REGULAR);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_LEV_SPIN),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_LEV_EDIT),en);
		
		en = (theTex->distortType==DISTORT_NONE)?0:1;
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_AMT_SPIN),en);
		EnableWindow(GetDlgItem(hPanel,IDC_MIRNS_AMT_EDIT),en);

		
		}
	}

BOOL MirrorDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg)    {
		case WM_INITDIALOG:
			{
			blurSpin = SetupFloatSpinner(hwndDlg, IDC_ACUBE_BLUR_SPIN, IDC_ACUBE_BLUR_EDIT,0.0f,100.0f,1.0f,.01f);
			nthSpin = SetupIntSpinner(hwndDlg, IDC_ACUBE_NTH_SPIN, IDC_ACUBE_NTH_EDIT,1,1000, 1);

			nsAmtSpin = SetupFloatSpinner(hwndDlg, IDC_MIRNS_AMT_SPIN, IDC_MIRNS_AMT_EDIT,0.0f,10.0f, .5f, .01f);
			nsSizSpin = SetupFloatSpinner(hwndDlg, IDC_MIRNS_SIZ_SPIN, IDC_MIRNS_SIZ_EDIT,0.001f,100.0f,10.0f, .01f);
			nsLevSpin = SetupFloatSpinner(hwndDlg, IDC_MIRNS_LEV_SPIN, IDC_MIRNS_LEV_EDIT,1.0f,10.0f, 2.0f,.01f);
			nsPhsSpin = SetupFloatSpinner(hwndDlg, IDC_MIRNS_ANI_SPIN, IDC_MIRNS_ANI_EDIT,0.0f,100.0f, 0.0f, .01f);

			mtlIdSpin = SetupIntSpinner(hwndDlg, IDC_MIR_MID_SPIN, IDC_MIR_MID_EDIT,1,65536, theTex->mtlID+1);

			CheckRadioButton( hwndDlg, IDC_FIRST_ONLY, IDC_EVERY_NTH, IDC_FIRST_ONLY+theTex->do_nth);
			CheckRadioButton( hwndDlg, IDC_MIRNS_REG, IDC_MIRNS_TURB, IDC_MIRNS_REG+theTex->noiseType);
			CheckRadioButton( hwndDlg, IDC_MIRDIST_NONE, IDC_MIRDIST_NOISE, IDC_MIRDIST_NONE + theTex->distortType);
			EnableNoise();
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
					theTex->applyBlur = GetCheckBox(hwndDlg, IDC_ACUBE_BLUR);			
					break;
				case IDC_USE_ENVMAP:
					theTex->useEnvMap = GetCheckBox(hwndDlg, id);			
					break;
				case IDC_MIRDIST_NONE:
				case IDC_MIRDIST_BUMP:
				case IDC_MIRDIST_NOISE:
					theTex->distortType = id-IDC_MIRDIST_NONE;
					EnableNoise();
					break;
				case IDC_MIRNS_REG:
				case IDC_MIRNS_FRAC:
				case IDC_MIRNS_TURB:
					theTex->noiseType = id-IDC_MIRNS_REG;
					EnableNoise();
					break;
				case IDC_MIR_APPLY_MTLID:
					theTex->useMtlID = GetCheckBox(hwndDlg, id);			
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
		case WM_DESTROY:		break;
		case CC_SPINNER_CHANGE: 
			if (!theHold.Holding()) theHold.Begin();
			switch (id) {
				case IDC_ACUBE_BLUR_SPIN: 
					theTex->SetBlur(blurSpin->GetFVal(),curTime); 	
					blurSpin->SetKeyBrackets(KeyAtCurTime(PB_BLUR));
					break;
				case IDC_ACUBE_NTH_SPIN: 
					theTex->nth = nthSpin->GetIVal(); 	
					break;
				case IDC_MIRNS_AMT_SPIN: 
					theTex->SetNsAmt(nsAmtSpin->GetFVal(),curTime); 	
					nsAmtSpin->SetKeyBrackets(KeyAtCurTime(PB_NSAMT));
					break;
				case IDC_MIRNS_LEV_SPIN: 
					theTex->SetNsLev(nsLevSpin->GetFVal(),curTime); 	
					nsLevSpin->SetKeyBrackets(KeyAtCurTime(PB_NSLEV));
					break;
				case IDC_MIRNS_SIZ_SPIN: 
					theTex->SetNsSiz(nsSizSpin->GetFVal(),curTime); 	
					nsSizSpin->SetKeyBrackets(KeyAtCurTime(PB_NSSIZ));
					break;
				case IDC_MIRNS_ANI_SPIN: 
					theTex->SetNsPhase(nsPhsSpin->GetFVal(),curTime); 	
					nsPhsSpin->SetKeyBrackets(KeyAtCurTime(PB_NSPHS));
					break;
				case IDC_MIR_MID_SPIN: 
					theTex->mtlID = mtlIdSpin->GetIVal()-1; 	
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
    	}
	return FALSE;
	}
 
BOOL MirrorDlg::KeyAtCurTime(int id) { return theTex->pblock->KeyFrameAtTime(id,ip->GetTime()); }

void MirrorDlg::LoadDialog(BOOL draw) {
	if (theTex) {
		Interval valid;
		theTex->Update(curTime,valid);
		blurSpin->SetValue(theTex->blur,FALSE);
		nthSpin->SetValue(theTex->nth,FALSE);
		nsAmtSpin->SetValue(theTex->nsAmt,FALSE);
		nsLevSpin->SetValue(theTex->nsLev,FALSE);
		nsSizSpin->SetValue(theTex->nsSize,FALSE);
		nsPhsSpin->SetValue(theTex->nsPhase,FALSE);
		mtlIdSpin->SetValue(theTex->mtlID+1,FALSE);

		blurSpin->SetKeyBrackets(KeyAtCurTime(PB_BLUR));
		nsAmtSpin->SetKeyBrackets(KeyAtCurTime(PB_NSAMT));
		nsLevSpin->SetKeyBrackets(KeyAtCurTime(PB_NSLEV));
		nsSizSpin->SetKeyBrackets(KeyAtCurTime(PB_NSSIZ));
		nsPhsSpin->SetKeyBrackets(KeyAtCurTime(PB_NSPHS));

		CheckRadioButton( hPanel, IDC_FIRST_ONLY, IDC_EVERY_NTH, IDC_FIRST_ONLY+theTex->do_nth);
		SetCheckBox(hPanel, IDC_ACUBE_BLUR, theTex->applyBlur);
		SetCheckBox(hPanel, IDC_ACUBE_BLUR, theTex->applyBlur);
		SetCheckBox(hPanel, IDC_USE_ENVMAP, theTex->useEnvMap);
		SetCheckBox(hPanel, IDC_MIR_APPLY_MTLID, theTex->useMtlID);
		CheckRadioButton( hPanel, IDC_MIRNS_REG, IDC_MIRNS_TURB, IDC_MIRNS_REG+theTex->noiseType);
		CheckRadioButton( hPanel, IDC_MIRDIST_NONE, IDC_MIRDIST_NOISE, IDC_MIRDIST_NONE + theTex->distortType);
		EnableNoise();
		}
	}

void MirrorDlg::SetThing(ReferenceTarget *m) {
	assert (m->ClassID()==mirrorClassID);
	assert (m->SuperClassID()==TEXMAP_CLASS_ID);
	if (theTex) theTex->paramDlg = NULL;
	theTex = (Mirror *)m;
	if (theTex)
		theTex->paramDlg = this;
	LoadDialog(TRUE);
	}

void MirrorDlg::ActivateDlg(BOOL onOff) {
	}
*/

class MirrorPBAccessor : public PBAccessor
{
public:
void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);    // set from v
};

void Mirror::EnableStuff() {
	if (!pblock) return;
	IParamMap2 *map = pblock->GetMap();
	if (!map) return;
	pblock->GetValue( fmirror_distortiontype, 0, distortType, FOREVER);
	pblock->GetValue( fmirror_noisetype, 0, noiseType, FOREVER);
	switch(distortType) {
		case 0:  // NONE
			map->Enable(fmirror_noisetype, FALSE);
			map->Enable(fmirror_nsamt, FALSE);
			map->Enable(fmirror_nslev, FALSE);
			map->Enable(fmirror_nsphs, FALSE);
			map->Enable(fmirror_nssiz, FALSE);
			break;
		case 1:	 // Use Bump map
			map->Enable(fmirror_noisetype, FALSE);
			map->Enable(fmirror_nsamt, TRUE);
			map->Enable(fmirror_nsphs, FALSE);
			map->Enable(fmirror_nssiz, FALSE);
			map->Enable(fmirror_nslev, FALSE);
			break;
		case 2:  // Builtin noise
			map->Enable(fmirror_noisetype, TRUE);
			map->Enable(fmirror_nsamt, TRUE);
			map->Enable(fmirror_nsphs, TRUE);
			map->Enable(fmirror_nssiz, TRUE);
			map->Enable(fmirror_nslev, noiseType);
			break;
		}
	pblock->GetValue( fmirror_apply, 0, applyBlur, FOREVER);
	map->Enable(fmirror_blur, applyBlur);
	}

void MirrorPBAccessor::Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
	Mirror* p = (Mirror*)owner;
	if (p->pblock == NULL) return;
	IParamMap2 *map = p->pblock->GetMap();
	if (map == NULL) return;
	switch (id)	{
		case fmirror_distortiontype:
			p->distortType = v.i;
			p->EnableStuff();
			break;
		case fmirror_noisetype:
			p->noiseType =v.i;
			p->EnableStuff();
			break;
		}
	}

static MirrorPBAccessor fmirror_accessor;


//-----------------------------------------------------------------------------
//  Mirror
//-----------------------------------------------------------------------------

static ParamBlockDesc2 fmirror_param_blk ( fmirror_params, _T("parameters"),  0, &mirrorCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_MIRROR, IDS_DS_MIRROR_PARAMS, 0, 0, NULL, 
	// params
	fmirror_blur,	_T("blurAmount"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_PW_BLURAMOUNT,
		p_default,		1.0,
		p_range,		0.0, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ACUBE_BLUR_EDIT,  IDC_ACUBE_BLUR_SPIN, 0.01f,
		end,

	fmirror_nsamt,	_T("distortionAmount"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_PW_DISTORTIONAMOUNT,
		p_default,		.5,
		p_range,		0.0, 10.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIRNS_AMT_EDIT, IDC_MIRNS_AMT_SPIN, 0.01f,
		p_enabled,		FALSE,
		end,
	fmirror_nslev,	_T("level"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_PW_LEVEL,
		p_default,		2.0,
		p_range,		1.0, 10.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_MIRNS_LEV_EDIT,IDC_MIRNS_LEV_SPIN, 0.01f,
		p_enabled,		FALSE,
		end,
	fmirror_nssiz,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_SIZE,
		p_default,		10.,
		p_range,		0.001, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,   IDC_MIRNS_SIZ_EDIT,IDC_MIRNS_SIZ_SPIN, 0.01f,
		p_enabled,		FALSE,
		end,
	fmirror_nsphs,	_T("phase"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_PHASE,
		p_default,		0.,
		p_range,		0.0, 100.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,    IDC_MIRNS_ANI_EDIT, IDC_MIRNS_ANI_SPIN,0.01f,
		p_enabled,		FALSE,
		end,
	fmirror_apply,	_T("applyBlur"), TYPE_BOOL,			0,				IDS_PW_APPLYBLUR,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ACUBE_BLUR,
		end,
	fmirror_nthframe,	_T("nthFrame"),   TYPE_INT,			0,	IDS_PW_NTHFRAME,
		p_default,		1,
		p_range,		1, 10000,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,  IDC_ACUBE_NTH_EDIT, IDC_ACUBE_NTH_SPIN, 1.0f,
		end,
	fmirror_frame, _T("frame"), TYPE_INT,				0,				IDS_PW_FRAME,
		p_default,		1,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_FIRST_ONLY, IDC_EVERY_NTH,
		end,
	fmirror_useenviroment,	_T("useEnviroment"), TYPE_BOOL,			0,				IDS_PW_USENVIROMENT,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_ENVMAP,
		end,
	fmirror_applytofaceid,	_T("applyToFaceID"), TYPE_BOOL,			0,				IDS_PW_APPLYTOFACEID,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MIR_APPLY_MTLID,
		end,
	fmirror_faceid,	_T("faceID"),   TYPE_INT,			0,	IDS_PW_FACEID,
		p_default,		1,
		p_range,		1, 65536,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT,   IDC_MIR_MID_EDIT, IDC_MIR_MID_SPIN, 1.0f,
		end,

	fmirror_distortiontype, _T("distortionType"), TYPE_INT,				0,				IDS_PW_DISTORTIONTYPE,
		p_default,		0,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 3, IDC_MIRDIST_NONE, IDC_MIRDIST_BUMP,IDC_MIRDIST_NOISE,
		p_accessor,		&fmirror_accessor,
		end,
	fmirror_noisetype, _T("noiseType"), TYPE_INT,				0,				IDS_PW_NOISETYPE,
		p_default,		0,
		p_range,		0,	2,
		p_enabled,		FALSE,
		p_ui,			TYPE_RADIO, 3, IDC_MIRNS_REG, IDC_MIRNS_FRAC,IDC_MIRNS_TURB,
		p_accessor,		&fmirror_accessor,
		end,

	end
);


#define NPARAMS 5
#define MIRROR_VERSION 4

// Version 1 desc
static ParamBlockDescID pbdesc1[] = {
	{ TYPE_INT, NULL, TRUE,fmirror_blur }, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,fmirror_apply} 	// blurOff
	};

// Version 2 desc
static ParamBlockDescID pbdesc2[] = {
	{ TYPE_FLOAT, NULL, TRUE,fmirror_blur } 	// blur
	};

// Version 3 desc
static ParamBlockDescID pbdesc[] = {
	{ TYPE_FLOAT, NULL, TRUE,fmirror_blur}, 	// blur
	{ TYPE_FLOAT, NULL, TRUE,fmirror_nsamt }, 	// noise amount
	{ TYPE_FLOAT, NULL, TRUE,fmirror_nslev }, 	// noise levels
	{ TYPE_FLOAT, NULL, TRUE,fmirror_nssiz }, 	// noise size
	{ TYPE_FLOAT, NULL, TRUE,fmirror_nsphs } 	// noise phase
	};

static ParamVersionDesc versions[3] = {
	ParamVersionDesc(pbdesc1,2,1),
	ParamVersionDesc(pbdesc2,1,2),
	ParamVersionDesc(pbdesc,5,3)
	};

//static ParamVersionDesc curVersion(pbdesc,NPARAMS,MIRROR_VERSION);

void Mirror::Init() {
	ivalid.SetEmpty();
	noiseType = NOISE_REGULAR;
	nth = 1;
	do_nth = TRUE;
	applyBlur = TRUE;
	distortType = DISTORT_NONE;
	useMtlID = FALSE;
	mtlID = 0;
	useEnvMap = TRUE;
	createAlpha = FALSE;
	SetBlur(1.0f, TimeValue(0));
	SetNsAmt(.5f, TimeValue(0));
	SetNsSiz(10.0f, TimeValue(0));
	SetNsLev(2.0f, TimeValue(0));
	SetNsPhase(0.0f, TimeValue(0));
	}

void Mirror::Reset() {
	mirrorCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	}

void Mirror::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Mirror::Mirror() {
	Param1 = FALSE;
//	paramDlg = NULL;
	pblock = NULL;
	maps = NULL;
	mirrorCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

RefTargetHandle Mirror::Clone(RemapDir &remap) {
	Mirror *mnew = new Mirror();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->do_nth = do_nth;
	mnew->applyBlur = applyBlur;
	mnew->distortType = distortType;
	mnew->useEnvMap = useEnvMap;
	mnew->useMtlID = useMtlID;
	mnew->mtlID = mtlID;
	mnew->ivalid.SetEmpty();	
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

ParamDlg* Mirror::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
//	MirrorDlg *dm = new MirrorDlg(hwMtlEdit, imp, this);
//	dm->LoadDialog(TRUE);	
//	paramDlg = dm;
//	return dm;	
	IAutoMParamDlg* masterDlg = mirrorCD.CreateParamDlgs(hwMtlEdit, imp, this);
	paramDlg = new MirrorDlg(this);
	fmirror_param_blk.SetUserDlgProc(paramDlg);
	EnableStuff();
	return masterDlg;

	}

void Mirror::Update(TimeValue t, Interval& valid) {		
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
//		pblock->GetValue( PB_BLUR, t, blur, ivalid );
//		pblock->GetValue( PB_NSAMT, t, nsAmt, ivalid);
		pblock->GetValue( fmirror_blur, t, blur, ivalid );
		pblock->GetValue( fmirror_nsamt, t, nsAmt, ivalid);
		nsAmt2 = nsAmt/10.0f;
//		pblock->GetValue( PB_NSSIZ, t, nsSize, ivalid);
//		pblock->GetValue( PB_NSPHS, t, nsPhase, ivalid);
//		pblock->GetValue( PB_NSLEV, t, nsLev, ivalid);
		pblock->GetValue( fmirror_nssiz, t, nsSize, ivalid);
		pblock->GetValue( fmirror_nsphs, t, nsPhase, ivalid);
		pblock->GetValue( fmirror_nslev, t, nsLev, ivalid);


		pblock->GetValue( fmirror_apply, t, applyBlur, ivalid);
		pblock->GetValue( fmirror_nthframe, t, nth, ivalid);
		pblock->GetValue( fmirror_frame, t, do_nth, ivalid);
		pblock->GetValue( fmirror_useenviroment, t, useEnvMap, ivalid);
		pblock->GetValue( fmirror_applytofaceid, t, useMtlID, ivalid);
		pblock->GetValue( fmirror_faceid, t, mtlID, ivalid);
		--mtlID;
		pblock->GetValue( fmirror_distortiontype, t, distortType, ivalid);
		pblock->GetValue( fmirror_noisetype, t, noiseType, ivalid);
		EnableStuff();
		}
	valid &= ivalid;
	}

void Mirror::FreeMaps() {
	MirrorMap *cm,*nxtcm;
	for (cm = maps; cm!=NULL; cm = nxtcm) {
		nxtcm = cm->next;
	   	delete cm;		
		}
	maps = NULL;
	}


void Mirror::SetBlur(float f, TimeValue t) { 
	blur = f; 
//	pblock->SetValue( PB_BLUR, t, f);
	pblock->SetValue( fmirror_blur, t, f);
	}


void Mirror::SetNsAmt(float v, TimeValue t){
	nsAmt = v; 
	nsAmt2 = nsAmt/10.0f;
//	pblock->SetValue( PB_NSAMT, t, v);
	pblock->SetValue( fmirror_nsamt, t, v);
	}

void Mirror::SetNsLev(float v, TimeValue t){
	nsLev = v; 
//	pblock->SetValue( PB_NSLEV, t, v);
	pblock->SetValue( fmirror_nslev, t, v);
	}

void Mirror::SetNsSiz(float v, TimeValue t) {
	nsSize = v; 
//	pblock->SetValue( PB_NSSIZ, t, v);
	pblock->SetValue( fmirror_nssiz, t, v);
	}

void Mirror::SetNsPhase(float v, TimeValue t) {
	nsPhase =v; 
//	pblock->SetValue( PB_NSPHS, t, v);
	pblock->SetValue( fmirror_nsphs, t, v);
	}

void Mirror::UseHighDynamicRange(BOOL onoff)
{
}

int Mirror::IsHighDynamicRange( ) const
{
	return false;
}

RefTargetHandle Mirror::GetReference(int i) {
	switch(i) {
		case 0:	return pblock ;
		default: return NULL;
		}
	}

void Mirror::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2 *)rtarg; break;
		}
	}
	 
Animatable* Mirror::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		default: return NULL;
		}
	}

TSTR Mirror::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));
		default: return TSTR("");		
		}
	}

static int nameID[] = { IDS_DS_BLUR, IDS_DS_DISTAMT, IDS_DS_NSLEV, IDS_DS_NSSIZ, IDS_DS_NSPHS };

RefResult Mirror::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget == pblock)
				{
				ivalid.SetEmpty();
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				fmirror_param_blk.InvalidateUI(changing_param);
			// notify our dependents that we've changed
				NotifyChanged();
				}

//			ivalid.SetEmpty();
//			if (paramDlg) 
//				paramDlg->Invalidate();
			break;
/*
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_BLUR: 
				case PB_NSAMT: 
				case PB_NSLEV: 
				case PB_NSPHS: 
					gpd->dim = defaultDim; break;
				case PB_NSSIZ: 
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

static void FlipAxis(Matrix3& tm, int k) {
	MRow* m = tm.GetAddr();
	for (int i=0; i<4; i++) m[i][k] = -m[i][k];
	}

/* build reflection matrix for plane p */
static void BuildReflMatrix(Matrix3& rm, float *p) {
	MRow* m = rm.GetAddr();
	m[0][0] = 1.0f-2.0f*p[0]*p[0];		
	m[1][1] = 1.0f-2.0f*p[1]*p[1];		
	m[2][2] = 1.0f-2.0f*p[2]*p[2];		
	m[0][1] = m[1][0] = -2.0f*p[0]*p[1];		
	m[0][2] = m[2][0] = -2.0f*p[0]*p[2];		
	m[1][2] = m[2][1] = -2.0f*p[1]*p[2];		
	m[3][0] = -2.0f*p[0]*p[3];		
	m[3][1] = -2.0f*p[1]*p[3];		
	m[3][2] = -2.0f*p[2]*p[3];		
	rm.SetNotIdent();
	}


MirrorMap *Mirror::FindMap(int nodeNum) {
	MirrorMap *cm;
	for (cm=maps; cm!=NULL; cm = cm->next)
		if (cm->nodeID==nodeNum) return cm;
	return NULL;
	}

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

static void whoa(){}

int Mirror::DoThisFrame(TimeValue t, BOOL fieldRender, TimeValue mapTime) {
	if (!do_nth) {
		if (t!=mapTime) return 0;  // only do it on first frame.
		}
	if (nth==1) return 1;   // need every one
	TimeValue del = abs(t - mapTime);
	if (del==0) return 1; // repeated iterations on a frame are ok
	if (fieldRender) del*=2;
	return  (del>=nth*GetTicksPerFrame())?1:0;
	}

static Point4 MyTransformPlane(const Matrix3& tm, const Matrix3& invTm, float* p)
{
	Point3 N;
	// Transforming a plane normal requires multiplying by the
	// transpose of the inverse.
	const MRow* a = invTm.GetAddr();
    N.x = p[0] * a[0][0] + p[1] * a[0][1] + p[2] * a[0][2];
    N.y = p[0] * a[1][0] + p[1] * a[1][1] + p[2] * a[1][2];
    N.z = p[0] * a[2][0] + p[1] * a[2][1] + p[2] * a[2][2];
	N = FNormalize(N);

    return Point4(N.x, N.y, N.z, p[3] - DotProd(tm.GetRow(3), N));
}

int Mirror::BuildMaps(TimeValue t, RenderMapsContext &rmc) {
	SubRendParams srp;
	rmc.GetSubRendParams(srp);
  	MirrorMap *mir = FindMap(rmc.NodeRenderID());
	if (mir&&!DoThisFrame(t,srp.fieldRender, mir->mapTime))
		return 1;
	RenderGlobalContext *gc = rmc.GetGlobalContext();
	if (gc->inMtlEdit) 
		return 1;
	ViewParams vp;
	
	if (useMtlID)
		rmc.SetSubMtlIndex(mtlID);

	float plane[4];
	Box2 sbox;
    rmc.FindMtlPlane(plane);
	rmc.GetCurrentViewParams(vp);
	Matrix3 worldToCam = vp.affineTM;
	Matrix3 tm = Inverse(vp.affineTM);  // get tm = camToWorld
	if (vp.projType==PROJ_PERSPECTIVE) {
		// If this is a perspective projection, and the Camera is behind plane
		// of the mirror, then quit.
		if ( DotProd(tm.GetRow(3),Point3(plane[0],plane[1],plane[2]))+plane[3]<0.0f ) 
			return 1;
		}
	else  {
		// If this is an ortho projection, and if view vector is
		// looking at edge or back of mirror, we punt.
		if (DotProd(tm.GetRow(2),Point3(plane[0],plane[1],plane[2]))<.0001f )
			return 1; 
		}

	Matrix3 rm;
	// compute a matrix that reflects in plane
	BuildReflMatrix(rm, plane);
	vp.affineTM  = Inverse(tm*rm);

	// Flip so parity is positive -- otherwise it wont render right. This
	// will be corrected for when we access the bitmap.
	FlipAxis(vp.affineTM, X_AXIS);	

	rmc.FindMtlScreenBox(sbox, &vp.affineTM, rmc.SubMtlIndex());

	int xmin,xmax,ymin,ymax;
	xmax = sbox.right;
	xmin = sbox.left;
	ymax = sbox.bottom;
	ymin = sbox.top;
	if (srp.fieldRender) {
		ymin *= 2;
		ymax *= 2;
		}
	if ( srp.rendType==RENDTYPE_REGION || srp.rendType==RENDTYPE_REGIONCROP ) {
		int rxmax = srp.devWidth - srp.xmin;
		int rxmin = srp.devWidth - srp.xmax;
		ymin = MAX(ymin,srp.ymin);
		ymax = MIN(ymax,srp.ymax);
		xmin = MAX(xmin,rxmin);
		xmax = MIN(xmax,rxmax);
		}

	// turn off baking while in flat mirrors....
	if ( srp.rendType==RENDTYPE_BAKE_SEL  || srp.rendType==RENDTYPE_BAKE_ALL )
		srp.rendType = RENDTYPE_NORMAL;

	srp.ymin = MAX(ymin,0);
	srp.ymax = MIN(ymax,srp.devHeight);
	srp.xmin = MAX(xmin,0);
	srp.xmax = MIN(xmax,srp.devWidth);
	srp.xorg = srp.xmin;
	srp.yorg = srp.ymin;
	srp.doingMirror = TRUE;
	srp.doEnvMap = useEnvMap;

	int w = srp.xmax-srp.xmin;
	int h = srp.ymax-srp.ymin;


	// We need to compare the values instead of looking at "w" or "h". The min and max can sometime be assigned to negative or positive infinite.
	// If the negative infinite gets substrated by something different than 0, the resulting value will make result switch from a huge negative to a huge positive.
	if(srp.xmax<=srp.xmin)
		return 1;
	if(srp.ymax<=srp.ymin)
		return 1;

#ifdef DBG
	w = ((w+3)/4)*4;   // For some reason this needs to be a multiple of 4 for bm->Display
#endif

  	if (mir==NULL) {
	  	mir = new MirrorMap;
		mir->nodeID = rmc.NodeRenderID();
		mir->next = maps;
		maps = mir;
		}
	mir->mapTime = t;	
	mir->pltm = tm*rm*worldToCam; // tm for mirror reflection in camera space
	mir->AllocMap(w, h, createAlpha);
	mir->org.x = srp.xorg-srp.devWidth/2;
	mir->org.y = srp.yorg-srp.devHeight/2;
//	mir->org.y = srp.fieldRender ?srp.yorg/2:srp.yorg;
	if (gc) {
		mir->xfact = nsAmt2*gc->xscale/float(w); 
		mir->yfact = nsAmt2*gc->yscale/float(h); 
		}

	Point4 camPlane;
	// CA - 12/05/01, #313539 - TransformPlane only works on rotations.
	// If the camera had a skew, that was real bad.
	camPlane = MyTransformPlane(vp.affineTM, Inverse(vp.affineTM), plane);
//	camPlane = TransformPlane(vp.affineTM, plane);

	// Render the mirror map.
	srp.fieldRender = FALSE;  // DS 6/1/00: Since the mirror map is only rendered once for use on both fields, render it 
	                           // normally to get full vertical res.

	// > 10/29/01 - 3:37pm --MQM-- 
	// broadcast message at the start of any reflect/refract map render
	if ( gc && !gc->inMtlEdit )
		BroadcastNotification( NOTIFY_BEGIN_RENDERING_REFLECT_REFRACT_MAP, (void*)gc );

	if (!rmc.Render(mir->bm, vp, srp, &camPlane,1))
		return 0;

	if (srp.fieldRender) {
		// Double the lines, otherwise the blur and distortion wont work right,
		// because the blank lines in between get averaged in and darken it.
		int	evenLines = srp.evenLines; 

		PixelBuf l64(w);
		if (evenLines) {
			for (int i=0; i<h; i+=2) {
				BMM_Color_64 *p64=l64.Ptr();
				if (i+1<h) {
					mir->bm->GetPixels(0,i,  w, p64); 
					mir->bm->PutPixels(0,i+1,w, p64);				
					}
				}
			}
		else {
			for (int i=0; i<h; i+=2) {
				BMM_Color_64 *p64=l64.Ptr();
				if (i+1<h) {
					mir->bm->GetPixels(0,i+1,w, p64); 
					mir->bm->PutPixels(0,i  ,w, p64);				
					}
				}
			}
		}

#ifdef DBG
	mir->bm->UnDisplay();
	mir->bm->Display(_T("Mirror Test"), BMM_UR);
	MessageBox(NULL, GetName(), _T(" Mirror Test"), MB_OK|MB_ICONEXCLAMATION);
#endif

	if (applyBlur) {
		// I tried pyramids here, but SATs looked much better. 
		//  maybe we should give users a choice?
		mir->bm->SetFilter(BMM_FILTER_SUM); 
//		mir->bm->SetFilter(BMM_FILTER_PYRAMID); 
		BitmapFilter *filt = mir->bm->Filter();
		if (filt)
			filt->MakeDirty();  // so filter gets recomputed for each frame
		}
	else 
		mir->bm->SetFilter(BMM_FILTER_NONE); 
	return 1;
	}

inline float FMax(float a, float b) { return (a>b?a:b); }

float Mirror::NoiseFunc(Point3 p, float levels, float time)
	{
	float res;
	switch (noiseType) {
		case NOISE_TURB: {
			float sum = 0.0f;
			float l,f = 1.0f;			
			for (l = levels; l>=1.0f; l-=1.0f) {				
				sum += (float)fabs(noise4(p*f,time))/f;
				f *= 2.0f;
				}
			if (l>0.0f) {				
				sum += l*(float)fabs(noise4(p*f,time))/f;
				}
			res = sum;
			break;
			}
			
		case NOISE_REGULAR:
			res = noise4(p,time);
			break;

		case NOISE_FRACTAL:
			if (levels==1.0f) {
				res = noise4(p,time);
			} else {
				float sum = 0.0f;
				float l,f = 1.0f;				
				for (l = levels; l>=1.0f; l-=1.0f) {					
					sum += noise4(p*f,time)/f;
					f *= 2.0f;
					}
				if (l>0.0f) {					
					sum += l*noise4(p*f,time)/f;
					}
				res = sum;
				}
			break;
		}
	
//	if (low<high) {
//		res = 2.0f * sramp((res+1.0f)/2.0f,low,high,sd) - 1.0f;
//		}
	return res;
	}

static inline Point3 ReflectVector(Point3 V, Point3 N) {   return V-2.0f*DotProd(V,N)*N; }

static BMM_Color_64 black64 = {0,0,0,0};
static AColor black(0.0f,0.0f,0.0f,0.0f);
static RGBA blackrgba(0.0f,0.0f,0.0f,1.0f);
#define RGC_DOINGMIRROR 100

bool Mirror::IsLocalOutputMeaningful( ShadeContext& sc ) 
{ 
	if (sc.InMtlEditor()) 
		return false;
	MirrorMap *mir = FindMap( sc.NodeID() );
	if ( mir != NULL )
	{
		if ( useMtlID && ( sc.mtlNum != mtlID ) ) 
			return false;
		if ( sc.globContext == NULL )
			return false;
	}
	return true; 
}

RGBA Mirror::EvalColor(ShadeContext& sc) {
	BMM_Color_64 c;
	IPoint2 s;
	int id = sc.NodeID();
	if (sc.InMtlEditor()) return blackrgba;
	MirrorMap *mir = FindMap(id);
	if (gbufID) sc.SetGBufferID(gbufID);
	if (mir) {
		if (useMtlID && (sc.mtlNum!=mtlID)) return blackrgba;
		RenderGlobalContext *gc = sc.globContext;
		// [attilas|7.6.2000] check for valid pointer befor using it.
		if (gc==NULL) 
			return blackrgba;
		if (gc->Execute(RGC_DOINGMIRROR))
			return blackrgba;
		s = sc.ScreenCoord();
		int w = mir->bm->Width(); 
		int h = mir->bm->Height();
		float nsx,nsy,nsblur;
		switch (distortType) {
			case DISTORT_NOISE: {
				Point3 p = sc.PObj();
				nsx = NoiseFunc(p/nsSize,nsLev,nsPhase)*nsAmt2;
				nsy = NoiseFunc(Point3(p.y+23.0f,p.z+12.0f,p.x+35.0f)/nsSize,nsLev,nsPhase)*nsAmt2;
				Point3 dp = sc.DPObj();
				float a = float(fabs(dp.x)+fabs(dp.y)+fabs(dp.z));
				// A heuristic attempt at anti-aliasing.
				// Increase the sample size porportional to the size of the sample
				// area in Noise space (dp/nsSize), &the amplitude of perturbation
				// applied in uv space (nsAmt).  This seems to work, but the scale
				// factor is arbitrary, and may need tuning.
				nsblur = blur + a*nsAmt/nsSize;
				}
				break;
			case DISTORT_BUMP: {
				// reflection vector for the bumped normal
				Point3 view = sc.V();
				Point3 R1 = ReflectVector(view,sc.Normal());
				// reflect in mirror( in camera space)
				R1 = VectorTransform(mir->pltm,R1);
				// compare with current view vector
				Point3 dv = R1-view;  // the difference
				nsx =    dv.x*mir->xfact; 
				nsy =   -dv.y*mir->yfact;
	  			nsblur = blur;
				}
				break;
			case DISTORT_NONE:
				nsx = nsy = 0.0f; 
				nsblur = 0.0f;
				break;
			}

		int xorg = mir->org.x + gc->devWidth/2;
		int yorg = mir->org.y + gc->devHeight/2;

		if (applyBlur) {
			float du = 1.0f/float(w);
			float dv = 1.0f/float(h);
			float du2,dv2;
			float u = float(gc->devWidth-1-s.x-xorg)*du+0.5f*du; 
			float v = float(s.y-yorg)*dv+0.5f*dv;
			if (distortType) {
				u += nsx; 
				if (u<0.0f) u = 0.0f; else if (u>1.0f) u = 1.0f;
				v += nsy; 
				if (v<0.0f) v = 0.0f; else if (v>1.0f) v = 1.0f;
				du2 = du*nsblur; 
				dv2 = dv*nsblur; 
				}
			else { 
				du2 = du*blur; 
				dv2 = dv*blur; 
				}
			mir->bm->GetFiltered(u,v, du2, dv2,&c);
			}
		else {
			int ix = gc->devWidth-1-s.x-xorg;
			int iy = s.y-yorg;
			if (distortType) {
				ix += int(nsx*w);
				if (ix<0) ix = 0; else if (ix>=w) ix = 1-1;
				iy += int(nsy*h);
				if (iy<0) iy = 0; else if (iy>=w) iy = 1-1;
				}
			mir->bm->GetLinearPixels(ix,iy,1,&c);
#if 0
			FILE *f = sc.DebugFile();
			if (f) {
				fprintf(f,"w=%d, h=%d org = (%d, %d) ", w, mir->bm->Height(),mir->org.x, mir->org.y);
				fprintf(f, "s = (%d, %d), c = (%x, %x, %x, %x)\n",
					s.x,s.y,c.r,c.g,c.b,c.a);
				}
#endif
			}
		return c;
		}
	else 
		return black;
	}

float Mirror::EvalMono(ShadeContext& sc) {
	return Intens(EvalColor(sc));
	}

Point3 Mirror::EvalNormalPerturb(ShadeContext& sc) {
	return Point3(0,0,0);
	}

#define MTL_HDR_CHUNK 0x4000
#define DONT_DO_NTH_CHUNK 0x1000
#define NTH_CHUNK 0x1001
#define DONT_APPLY_BLUR_CHUNK 0x1002
#define DONT_USE_ENV_CHUNK 0x1003
#define DO_NOISE_CHUNK 0x1004
#define NOISE_TYPE_CHUNK 0x1005
#define DISTORT_TYPE_CHUNK 0x1020
#define USE_MTLID_CHUNK 0x1030
#define MTLID_CHUNK 0x1040
#define PARAM2_CHUNK 0x1050
IOResult Mirror::Save(ISave *isave) { 
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
	isave->BeginChunk(DISTORT_TYPE_CHUNK);
	isave->Write(&distortType,sizeof(distortType),&nb);			
	isave->EndChunk();

	isave->BeginChunk(NTH_CHUNK);
	isave->Write(&nth,sizeof(nth),&nb);			
	isave->EndChunk();

	isave->BeginChunk(NOISE_TYPE_CHUNK);
	isave->Write(&noiseType,sizeof(noiseType),&nb);			
	isave->EndChunk();

	isave->BeginChunk(MTLID_CHUNK);
	isave->Write(&mtlID,sizeof(mtlID),&nb);			
	isave->EndChunk();

	if (useMtlID) {
		isave->BeginChunk(USE_MTLID_CHUNK);
		isave->EndChunk();
		}

*/
	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();
	return IO_OK;
	}

class MirrorPostLoad : public PostLoadCallback {
	public:
		Mirror *n;
		BOOL Param1;
		MirrorPostLoad(Mirror *ns, BOOL b) {n = ns; Param1 = b;}
		void proc(ILoad *iload) {  
			if (Param1)
				{
				TimeValue t = 0;
				n->pblock->SetValue( fmirror_apply, t, n->applyBlur);
				n->pblock->SetValue( fmirror_nthframe, t, n->nth);
				n->pblock->SetValue( fmirror_frame, t, n->do_nth);
				n->pblock->SetValue( fmirror_useenviroment, t, n->useEnvMap);
				n->pblock->SetValue( fmirror_applytofaceid, t, n->useMtlID);
				n->pblock->SetValue( fmirror_faceid, t, n->mtlID+1);
				n->pblock->SetValue( fmirror_distortiontype, t, n->distortType);
				n->pblock->SetValue( fmirror_noisetype, t, n->noiseType);

				}

			delete this; 


			} 
	};

		

IOResult Mirror::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
//	iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,2, &curVersion, this,0));
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
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
			case DO_NOISE_CHUNK:
				distortType = DISTORT_NOISE;
				break;
			case NTH_CHUNK:
				iload->Read(&nth,sizeof(nth),&nb);			
				break;
			case NOISE_TYPE_CHUNK:
				iload->Read(&noiseType,sizeof(noiseType),&nb);			
				break;
			case DISTORT_TYPE_CHUNK:
				iload->Read(&distortType,sizeof(distortType),&nb);			
				break;
			case MTLID_CHUNK:
				iload->Read(&mtlID,sizeof(mtlID),&nb);			
				break;
			case USE_MTLID_CHUNK:
				useMtlID = TRUE;
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 3, &fmirror_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);

	iload->RegisterPostLoadCallback(new MirrorPostLoad(this,Param1));

	return IO_OK;
	}

#endif // NO_MAPTYPE_FLATMIRROR