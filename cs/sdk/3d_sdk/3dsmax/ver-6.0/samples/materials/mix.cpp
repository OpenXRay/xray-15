/**********************************************************************
 *<
	FILE: MIX.CPP

	DESCRIPTION: MIX Composite.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "macrorec.h"

#include <d3dx8.h>

#include "IHardwareMaterial.h"

extern HINSTANCE hInstance;

static LRESULT CALLBACK CurveWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


#define NSUBTEX 3    // number of texture map slots
#define NCOLS 2      // number of color swatches

static Class_ID mixClassID(MIX_CLASS_ID,0);

#define TEXOUT_REF 4

class Mix;

//--------------------------------------------------------------
// Mix: A Mix texture map
//--------------------------------------------------------------
#define PB_REF 0
#define SUB1_REF 1
#define SUB2_REF 2
#define SUB3_REF 3
#define PB_TEXOUT 4

class MixDlgProc;

class Mix: public MultiTex { 
	public:
	static ParamDlg* texoutDlg;
	static MixDlgProc *paramDlg;
	TexHandle *texHandle[3];
	int useSubForTex[3];
	int numTexHandlesUsed;
	int texOpsType[3];
	Interval texHandleValid;
	Color col[NCOLS];
	IParamBlock2 *pblock;   // ref #0
	Texmap* subTex[NSUBTEX];  // 3 More refs
	BOOL mapOn[NSUBTEX];
	TextureOutput *texout; // ref #4
	Interval ivalid;
	float mix;
	BOOL useCurve;
	float crvA;
	float crvB;
	BOOL rollScroll;
	public:
		BOOL Param1;
		Mix();
		~Mix() { 
			DiscardTexHandles(); 
			}
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void SetOutputLevel(TimeValue t, float v) {texout->SetOutputLevel(t,v); }
		void SetColor(int i, Color c, TimeValue t);
		void SetMix(float f, TimeValue t);
		void SetCrvA(float f, TimeValue t);
		void SetCrvB(float f, TimeValue t);
		void NotifyChanged();
		void SwapInputs(); 
		void EnableStuff();

		float mixCurve(float x);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {	return mixClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_MIX); }  
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
//		BOOL loadOnChecks;
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);
		void DiscardTexHandles() {
			for (int i=0; i<3; i++) {
				if (texHandle[i]) {
					texHandle[i]->DeleteThis();
					texHandle[i] = NULL;
					}
				}
			texHandleValid.SetEmpty();
			}

		// Multiple map in vp support -- DS 4/24/00
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		BOOL SupportsMultiMapsInViewport() { return TRUE; }
		void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);

		// Multiple map code
		void StuffAlpha(BITMAPINFO* bi1, BITMAPINFO *bi2, BOOL invert);
		void StuffConstAlpha(float falpha, BITMAPINFO *bmi);
		void ConstAlphaBlend(BITMAPINFO *bi1, BITMAPINFO *bi2, float a);
		void ConstScale(BITMAPINFO *bi, float s);
		void AlphaBlendMaps(BITMAPINFO *bi1,BITMAPINFO *bi2,BITMAPINFO *balph);
		void FixupAlpha(BITMAPINFO *bi);
		void SetTexOps(Material *mtl, int i, int type);
		void SetHWTexOps(IHardwareMaterial *pIHWMat, int ntx, int type);

		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }
	};

int numMixs = 0;
ParamDlg* Mix::texoutDlg;
MixDlgProc* Mix::paramDlg;

class MixClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Mix; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_MIX_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return mixClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_COMP;  }

// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("mixTexture"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

	};

static MixClassDesc mixCD;

ClassDesc* GetMixDesc() { return &mixCD;  }
//-----------------------------------------------------------------------------
//  Mix
//-----------------------------------------------------------------------------

enum { mix_params };  // pblock ID
// grad_params param IDs


enum 
{ 
	mix_mix, mix_curvea, mix_curveb, mix_usecurve,
	mix_color1, mix_color2,
	mix_map1, mix_map2, mix_mask,		
	mix_map1_on, mix_map2_on,  mix_mask_on, // main grad params 
	mix_output
};

//JBW: here is the new ParamBlock2 descriptor. There is only one block for Gradients, a per-instance block.
// for the moment, some of the parameters a Tab<>s to test the Tab system.  Aslo note that all the References kept
// kept in a Gradient are mapped here, marked as P_OWNERS_REF so that the paramblock accesses and maintains them
// as references on owning Gradient.  You need to specify the refno for these owner referencetarget parameters.
// I even went so far as to expose the UVW mapping and Texture Output sub-objects this way so that they can be
// seen by the scripter and the schema-viewer

// per instance gradient block

static ParamBlockDesc2 mix_param_blk ( mix_params, _T("parameters"),  0, &mixCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PB_REF, 
	//rollout
	IDD_MIX, IDS_DS_MIX_PARAMS, 0, 0, NULL, 
	// params
	mix_mix, _T("mixAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE,	IDS_PW_MIXAMOUNT,
		p_default,		0.0f,
		p_range,		0.0f, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIX_EDIT, IDC_MIX_SPIN, 0.1f, 
		end,
	mix_curvea, _T("lower"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_LOWER,
		p_default,		0.25f,
		p_range,		0.0f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIXA_EDIT, IDC_MIXA_SPIN, 0.01f, 
		end,
	mix_curveb, _T("upper"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_UPPER,
		p_default,		0.75f,
		p_range,		0.0f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIXB_EDIT, IDC_MIXB_SPIN, 0.01f, 
		end,
	mix_usecurve,	_T("useCurve"), TYPE_BOOL,			0,				IDS_PW_USECURVE,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MIX_USECURVE,
		end,
	mix_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_MIX_COL1, 
		end,
	mix_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_MIX_COL2, 
		end,
	mix_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		SUB1_REF,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MIX_TEX1,
		end,
	mix_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		SUB2_REF,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MIX_TEX2,
		end,
	mix_mask,		_T("mask"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_DS_MASK,
		p_refno,		SUB3_REF,
		p_subtexno,		2,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_MIX_TEX3,
		end,
	mix_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	mix_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	mix_mask_on,	_T("maskEnabled"), TYPE_BOOL,			0,				IDS_PW_MASKENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON3,
		end,
	mix_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_OUTPUT,
		p_refno,		PB_TEXOUT, 
		end,


	end
);

#define MIX_VERSION 2


#define NPARAMS 5

static int name_id[NPARAMS] = {IDS_DS_COLOR1, IDS_DS_COLOR2, IDS_DS_MIXAMT,
	IDS_RB_LOWER,IDS_RB_UPPER };

static ParamBlockDescID pbdesc1[] = {
	{ TYPE_RGBA, NULL, TRUE,mix_color1 },   // col1
	{ TYPE_RGBA, NULL, TRUE,mix_color2 },   // col2
	{ TYPE_FLOAT, NULL, TRUE,mix_mix} 	 // mix
	};   


static ParamBlockDescID pbdesc2[] = {
	{ TYPE_RGBA, NULL, TRUE,mix_color1 },   // col1
	{ TYPE_RGBA, NULL, TRUE,mix_color2 },   // col2
	{ TYPE_FLOAT, NULL, TRUE,mix_mix}, 	 // mix
	{ TYPE_FLOAT, NULL, TRUE,mix_curvea}, 	 // crva
	{ TYPE_FLOAT, NULL, TRUE,mix_curveb} 	 // crvb
	};   

static ParamVersionDesc versions[2] = {
	ParamVersionDesc(pbdesc1,3,1),
	ParamVersionDesc(pbdesc2,5,2),
	};


//dialog stuff to get the Set Ref button
class MixDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Mix *mix;		
		BOOL valid;
		HWND hPanel; 
		MixDlgProc(Mix *m) {
			mix = m;
			valid   = FALSE;
			}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
	};


static void DrawCurve (HWND hWnd,HDC hdc, Mix *mix) {

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_MIXCURVE),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	
	SelectObject(hdc,GetStockObject(BLACK_PEN));
	MoveToEx(hdc,rect.left+1, rect.bottom-1,NULL); 
	float fx,fy;
	int ix,iy;
	float w = (float)rect.w()-2;
	float h = (float)rect.h()-2;
	for (ix =0; ix<w; ix++) {
		fx = (float(ix)+0.5f)/w;
		fy = mix->mixCurve(fx);
		iy = int(h*fy+0.5f);
		LineTo(hdc, rect.left+1+ix, rect.bottom-1-iy);
		}
	WhiteRect3D(hdc,orect,TRUE);

}


BOOL MixDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	Rect rect;
	switch (msg) {
		case WM_PAINT: {
//			em->EnableAffectRegion (t);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd,&ps);
			mix = (Mix*)map->GetParamBlock()->GetOwner(); 
			DrawCurve(hWnd,hdc,mix);
			EndPaint(hWnd,&ps);
			return FALSE;
			}

		case CC_SPINNER_CHANGE:
			mix = (Mix*)map->GetParamBlock()->GetOwner(); 
			mix->Update(GetCOREInterface()->GetTime(),FOREVER);
			GetClientRectP(GetDlgItem(hWnd,IDC_MIXCURVE),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
			return FALSE;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MIX_SWAP:
					{
					mix = (Mix*)map->GetParamBlock()->GetOwner(); 
					mix->SwapInputs();
					}
					break;
				default:
					break;
			}


		default:
			return FALSE;
		}
	return FALSE;
	}


//static ParamVersionDesc curVersion(pbdesc2,5,002);

void Mix::Init() {
	useCurve = 0;
	if (texout) texout->Reset();
	else ReplaceReference( TEXOUT_REF, GetNewDefaultTextureOutput());		
	ivalid.SetEmpty();
	SetColor(0, Color(0.0f,0.0f,0.0f), TimeValue(0));
	SetColor(1, Color(1.0f,1.0f,1.0f), TimeValue(0));
	SetMix(.0f, TimeValue(0));
	SetCrvA(.3f, TimeValue(0));
	SetCrvB(.7f, TimeValue(0));
	for (int i=0; i<NSUBTEX; i++) 
		mapOn[i] = 1;
	}

void Mix::Reset() {
	mixCD.Reset(this, TRUE);	// reset all pb2's
	for (int i=0; i<NSUBTEX; i++) {
		DeleteReference(i+1);	// get rid of maps
		}
	Init();
	}

void Mix::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Mix::Mix() {
	Param1 = FALSE;
	for (int i=0; i<NSUBTEX; i++) {
		subTex[i] = NULL;
		texHandle[i] = NULL;
		}
	texHandleValid.SetEmpty();
	texout = NULL;
	pblock = NULL;
//	paramDlg = NULL;
	crvA = crvB = mix = 0.0f;
	mixCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

float Mix::mixCurve(float x) {
	if (x<crvA)  return 0.0f;
	if (x>=crvB) return 1.0f;
	x = (x-crvA)/(crvB-crvA);
	return (x*x*(3-2*x));
	}

static AColor black(0.0f,0.0f,0.0f,0.0f);

AColor Mix::EvalColor(ShadeContext& sc) {
	AColor c;
	if (sc.GetCache(this,c)) 
		return c; 
	if (gbufID) sc.SetGBufferID(gbufID);

	Texmap *sub[3];
	sub[0] = mapOn[0]?subTex[0]:NULL;
	sub[1] = mapOn[1]?subTex[1]:NULL;
	sub[2] = mapOn[2]?subTex[2]:NULL;
	float m = sub[2]? sub[2]->EvalMono(sc): mix;
	if (useCurve) m = mixCurve(m);
	if (m<.0001f)
		c = texout->Filter(sub[0] ? sub[0]->EvalColor(sc): col[0]);
	else if (m>0.9999f)   
		c = texout->Filter(sub[1] ? sub[1]->EvalColor(sc): col[1]);
	else {
		AColor c0  = sub[0] ? sub[0]->EvalColor(sc): col[0];
		AColor c1  = sub[1] ? sub[1]->EvalColor(sc): col[1];
		c = texout->Filter(m*c1 + (1.0f-m)*c0);
		}
	sc.PutCache(this,c); 
	return c;
	}

float Mix::EvalMono(ShadeContext& sc) {
	float f;
	if (sc.GetCache(this,f)) 
		return f; 
	if (gbufID) sc.SetGBufferID(gbufID);

	Texmap *sub[3];
	sub[0] = mapOn[0]?subTex[0]:NULL;
	sub[1] = mapOn[1]?subTex[1]:NULL;
	sub[2] = mapOn[2]?subTex[2]:NULL;
	float m = sub[2]? sub[2]->EvalMono(sc): mix;
	if (useCurve) m = mixCurve(m);
	if (m==0.0f)
	    f = texout->Filter(sub[0] ? sub[0]->EvalMono(sc): Intens(col[0]));
	else if(m==1.0f)   
		f = texout->Filter(sub[1] ? sub[1]->EvalMono(sc): Intens(col[1]));
	else {
		float c0  = sub[0] ? sub[0]->EvalMono(sc): Intens(col[0]);
		float c1  = sub[1] ? sub[1]->EvalMono(sc): Intens(col[1]);
		f = texout->Filter(m*c1 + (1.0f-m)*c0);
		}
	sc.PutCache(this,f); 
	return f;
	}

Point3 Mix::EvalNormalPerturb(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	Texmap *sub[3];
	sub[0] = mapOn[0]?subTex[0]:NULL;
	sub[1] = mapOn[1]?subTex[1]:NULL;
	sub[2] = mapOn[2]?subTex[2]:NULL;
	float m = sub[2]? sub[2]->EvalMono(sc): mix;
	if (useCurve) m = mixCurve(m);
	if (m==0.0f)
		return texout->Filter(sub[0] ? sub[0]->EvalNormalPerturb(sc): Point3(0,0,0));
	else if(m==1.0f)   
		return texout->Filter(sub[1] ? sub[1]->EvalNormalPerturb(sc): Point3(0,0,0));
	else {
		Point3 p0  = sub[0] ? sub[0]->EvalNormalPerturb(sc): Point3(0,0,0);
		Point3 p1  = sub[1] ? sub[1]->EvalNormalPerturb(sc): Point3(0,0,0);
		return texout->Filter(m*p1 + (1.0f-m)*p0);
		}
	}

RefTargetHandle Mix::Clone(RemapDir &remap) {
	Mix *mnew = new Mix();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(TEXOUT_REF,remap.CloneRef(texout));
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->mix = mix;
	mnew->crvA = crvA;
	mnew->crvB = crvB;
	mnew->useCurve = useCurve;
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+1,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}

void Mix::EnableStuff() {
	if (pblock) {
		IParamMap2 *map = pblock->GetMap();
		if (map) {
			map->Enable(mix_mix, mapOn[2]&&subTex[2]?FALSE:TRUE);
			}
		}
	}

ParamDlg* Mix::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
// JBW: the main difference here is the automatic creation of a ParamDlg by the new
// ClassDesc2 function CreateParamDlgs().  This mirrors the way BeginEditParams()
// can be redirected to the ClassDesc2 for automatic ParamMap2 management.  In this 
// case a special subclass of ParamDlg, AutoMParamDlg, defined in IParamm2.h, is 
// created.  It can act as a 'master' ParamDlg to which you can add any number of 
// secondary dialogs and it will make sure all the secondary dialogs are kept 
// up-to-date and deleted as necessary.  
// Here you see we create the Coordinate, Gradient and Output ParamDlgs in the desired 
// order, and then add the Coordinate and Output dlgs as secondaries to the 
// Gradient master AutoMParamDlg so it will keep them up-to-date automatically

	// create the rollout dialogs
	IAutoMParamDlg* masterDlg = mixCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the master
	paramDlg = new MixDlgProc(this);
	mix_param_blk.SetUserDlgProc(paramDlg);
	masterDlg->AddDlg(texoutDlg);

	EnableStuff();
	return masterDlg;

	}



BOOL Mix::SetDlgThing(ParamDlg* dlg)
{
	EnableStuff();
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((dlg != NULL) && (dlg == texoutDlg))
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}



void Mix::Update(TimeValue t, Interval& valid) {		

	if (Param1)
		{
		pblock->SetValue( mix_map1_on, 0, mapOn[0]);
		pblock->SetValue( mix_map2_on, 0, mapOn[1]);
		pblock->SetValue( mix_mask_on, 0, mapOn[2]);
		pblock->SetValue( mix_usecurve, 0, useCurve);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		texout->Update(t,ivalid);
		pblock->GetValue( mix_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( mix_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( mix_mix, t, mix, ivalid );
		pblock->GetValue( mix_curvea, t, crvA, ivalid );
		pblock->GetValue( mix_curveb, t, crvB, ivalid );

		pblock->GetValue(mix_usecurve,t,useCurve,ivalid);
		pblock->GetValue(mix_map1_on,t,mapOn[0],ivalid);
		pblock->GetValue(mix_map2_on,t,mapOn[1],ivalid);
		pblock->GetValue(mix_mask_on,t,mapOn[2],ivalid);

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	EnableStuff();
	valid &= ivalid;
	}


void Mix::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(mix_color1,0,mix_color2,0);
	mix_param_blk.InvalidateUI(mix_color1);
	mix_param_blk.InvalidateUI(mix_color2);
	mix_param_blk.InvalidateUI(mix_map1);
	mix_param_blk.InvalidateUI(mix_map2);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);

	}

void Mix::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	pblock->SetValue( i==0?mix_color1:mix_color2, t, c);
	}

void Mix::SetMix(float f, TimeValue t) { 
	mix = f; 
	pblock->SetValue( mix_mix, t, f);
	}

void Mix::SetCrvA(float f, TimeValue t) { 
	crvA = f; 
	pblock->SetValue( mix_curvea, t, f);
	if (crvB<crvA) {
		crvB = crvA;
		pblock->SetValue( mix_curveb, t, crvB);
		}

	}

void Mix::SetCrvB(float f, TimeValue t) { 
	crvB = f; 
	pblock->SetValue( mix_curveb, t, f);
	if (crvB<crvA) {
		crvA = crvB;
		pblock->SetValue( mix_curvea, t, crvA);
		}
	}

RefTargetHandle Mix::GetReference(int i) {
	switch(i) {
		case 0:	return pblock ;
		case TEXOUT_REF: return texout;
		default:
			return subTex[i-1];
		}
	}

void Mix::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2 *)rtarg; break;
		case TEXOUT_REF: texout = (TextureOutput *)rtarg; break;
		default:
			subTex[i-1] = (Texmap *)rtarg; break;
		}
	}

static int submapids[3] = { mix_map1, mix_map2, mix_mask };

void Mix::SetSubTexmap(int i, Texmap *m) {
	if (i>2) 
		return;
	ReplaceReference(i+1,m);
	mix_param_blk.InvalidateUI( submapids[i] );
	ivalid.SetEmpty();
	}

TSTR Mix::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		case 2:  return TSTR(GetString(IDS_DS_MIXAMT)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Mix::SubAnim(int i) {
	switch (i) {
		case 0: return pblock;
		case TEXOUT_REF: return texout;
		default: return subTex[i-1]; 
		}
	}

TSTR Mix::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_PARAMETERS));		
		case TEXOUT_REF: return TSTR(GetString(IDS_DS_OUTPUT));
		default: return GetSubTexmapTVName(i-1);
		}
	}

RefResult Mix::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
				ParamID changing_param = pblock->LastNotifyParamID();
//				if ( hTarget != texout ) 
				mix_param_blk.InvalidateUI(changing_param);
				// NotifyChanged();  //DS this is redundant
				}
			else if (hTarget == texout)
				{
				// NotifyChanged();  //DS this is redundant
				}
			DiscardTexHandles(); 

			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define USE_CURVE_CHUNK 0x6000
#define VERS2_CHUNK 0x6001
#define MAPOFF_CHUNK 0x3000
#define PARAM2_CHUNK 0x3010

IOResult Mix::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(PARAM2_CHUNK);
	isave->EndChunk();


	return IO_OK;
	}	
	  
class MixPostLoad : public PostLoadCallback {
	public:
		Mix *tm;
		int version;
		MixPostLoad(Mix *b, int v) {tm=b;version = v;}
		void proc(ILoad *iload) {  
			if (version<2) 
				{
				tm->SetCrvA(.3f, TimeValue(0));
				tm->SetCrvB(.7f, TimeValue(0));
				}
			if (tm->Param1)
				{
				tm->pblock->SetValue( mix_map1_on, 0, tm->mapOn[0]);
				tm->pblock->SetValue( mix_map2_on, 0, tm->mapOn[1]);
				tm->pblock->SetValue( mix_mask_on, 0, tm->mapOn[2]);
				tm->pblock->SetValue( mix_usecurve, 0, tm->useCurve);

				}
			delete this; 
			} 
	};

IOResult Mix::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	int id;
	int version = 0;
	Param1 = TRUE;

//	iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions,1, &curVersion, this,0));
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case USE_CURVE_CHUNK:
				useCurve = TRUE;
				break;
			case VERS2_CHUNK:
				version = 2;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
			case MAPOFF_CHUNK+2:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			case PARAM2_CHUNK:
				Param1 = FALSE;;
				break;

			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 2, &mix_param_blk, this, PB_REF);
	iload->RegisterPostLoadCallback(plcb);

//	if (version<2) 
//	iload->RegisterPostLoadCallback(new MixPostLoad(this,version));
	return IO_OK;
	}

// DDS 4/12/00  Support for multiple texture display in viewports.
void Mix::ActivateTexDisplay(BOOL onoff) {
	if (!onoff) {
		DiscardTexHandles();
//		for (int i=0; i<NSUBTEX; i++) {
//			if (subTex[i]) subTex[i]->ActivateTexDisplay(FALSE);
//			}
		}
	}

class MixAlphaMaker: public TexHandleMaker {
	public:
	MtlMakerCallback *mcb;
	MixAlphaMaker(MtlMakerCallback *c) { mcb = c; }
	TexHandle* CreateHandle(Bitmap *bm, int symflags, int extraFlags) {
		// save alpha channel
		
		return NULL;
		}
	TexHandle* CreateHandle(BITMAPINFO *bminf, int symflags, int extraFlags) {
		return NULL;
		}

	int Size() {
		return mcb->Size();
		}
		
	};


#define BMIDATA(x) ((UBYTE *)((BYTE *)(x) + sizeof(BITMAPINFOHEADER)))

static BOOL SameUV(TextureInfo &ta, TextureInfo &tb) {
	if (ta.uvwSource!=tb.uvwSource) return FALSE;
	if (ta.mapChannel!=tb.mapChannel) return FALSE;
	if (ta.tiling[0]!=tb.tiling[0]) return FALSE;
	if (ta.tiling[1]!=tb.tiling[1]) return FALSE;
	if (ta.tiling[2]!=tb.tiling[2]) return FALSE;
	if (ta.faceMap!=tb.faceMap) return FALSE;
	if (!(ta.textTM==tb.textTM)) return FALSE;
	return TRUE;
	}

static bool SameUV(Texmap *tma, Texmap *tmb)
{
	if (tma->GetUVWSource() != tmb->GetUVWSource()) {
		return false;
	}
	if (tma->GetMapChannel() != tmb->GetMapChannel()) {
		return false;
	}
	if (tma->GetTextureTiling() != tmb->GetTextureTiling()) {
		return false;
	}
	Matrix3 texTMa, texTMb;
	tma->GetUVTransform(texTMa);
	tmb->GetUVTransform(texTMb);
	if (texTMa == texTMb) {
		return true;
	}
	return false;
}

void Mix::FixupAlpha(BITMAPINFO *bi) {
	// compute alpha from the (r+g+b)/3 and stuff it in the alpha channel. 
	int w = bi->bmiHeader.biWidth;
	int h = bi->bmiHeader.biHeight;
	int npix = w*h;
	if (useCurve) {
		UBYTE *a = BMIDATA(bi);
		for (int j=0; j<npix; j++) {
			a[3] =  int(mixCurve(float(a[0]+a[1]+a[2])/(3.0f*255.0f))*255.0f);
			a += 4;
			}
		}
	else {
		UBYTE *a = BMIDATA(bi);
		for (int j=0; j<npix; j++) {
			a[3] = (a[0]+a[1]+a[2])/3;
			a += 4;
			}
		}
	}

void Mix::StuffConstAlpha(float falpha, BITMAPINFO *bi) {
	int alpha = int(falpha*255.0f);
	// compute alpha from the (r+g+b)/3 and stuff it in the alpha channel. 
	int w = bi->bmiHeader.biWidth;
	int h = bi->bmiHeader.biHeight;
	int npix = w*h;
	UBYTE *a = BMIDATA(bi);
	for (int j=0; j<npix; j++) {
		a[3] =  alpha;
		a += 4;
		}
	}

void Mix::StuffAlpha(BITMAPINFO* bi1, BITMAPINFO *bi2, BOOL invert) {
	int w = bi1->bmiHeader.biWidth;
	int h = bi1->bmiHeader.biHeight;
	int npix = w*h;
	UBYTE *a = BMIDATA(bi1);
	UBYTE *b = BMIDATA(bi2);
	if (invert) {
		for (int j=0; j<npix; j++) {
			b[3] =  255-a[3];
			a += 4;
			b += 4;
			}
		}
	else {
		for (int j=0; j<npix; j++) {
			b[3] =  a[3];
			a += 4;
			b += 4;
			}
		}
	}

void Mix::ConstAlphaBlend(BITMAPINFO *bi1, BITMAPINFO *bi2, float s) {
	int w = bi1->bmiHeader.biWidth;
	int h = bi1->bmiHeader.biHeight;
	int npix = w*h;
	UBYTE *a = BMIDATA(bi1);
	UBYTE *b = BMIDATA(bi2);
	int alph = int(s*255.0f);
	int calph = 255-alph;
	for (int j=0; j<npix; j++) {
		a[0] = a[0]*calph/255 + b[0]*alph/255;
		a[1] = a[1]*calph/255 + b[1]*alph/255;
		a[2] = a[2]*calph/255 + b[2]*alph/255;
		a += 4;
		b += 4;
		}
	}
	  
void Mix::ConstScale(BITMAPINFO *bi, float s) {
	int w = bi->bmiHeader.biWidth;
	int h = bi->bmiHeader.biHeight;
	int npix = w*h;
	UBYTE *a = BMIDATA(bi);
	int alph = int(s*255.0f);
	for (int j=0; j<npix; j++) {
		a[0] = a[0]*alph/255;
		a[1] = a[1]*alph/255;
		a[2] = a[2]*alph/255;
		a += 4;
		}
	}

void Mix::AlphaBlendMaps(BITMAPINFO *bi1,BITMAPINFO *bi2,BITMAPINFO *balph) {
	if (bi1==NULL) {
		if (bi2==NULL)
			return;
		int w = bi2->bmiHeader.biWidth;
		int h = bi2->bmiHeader.biHeight;
		int npix = w*h;
		UBYTE *b = BMIDATA(bi2);
		UBYTE *m = BMIDATA(balph);
		for (int j=0; j<npix; j++) {
			int alph = m[4];
			m[0] = b[0]*alph/255;
			m[1] = b[1]*alph/255;
			m[2] = b[2]*alph/255;
			m[3] = 255;
			b += 4;
			m += 4;
			}
		}
	else {
		if (bi2==NULL) {
			int w = bi1->bmiHeader.biWidth;
			int h = bi1->bmiHeader.biHeight;
			int npix = w*h;
			UBYTE *b = BMIDATA(bi1);
			UBYTE *m = BMIDATA(balph);
			for (int j=0; j<npix; j++) {
				int alph = m[4];
				int calph = 255-alph;
				m[0] = b[0]*calph/255;
				m[1] = b[1]*calph/255;
				m[2] = b[2]*calph/255;
				m[3] = 255;
				b += 4;
				m += 4;
				}
			}
		else {
			int w = bi1->bmiHeader.biWidth;
			int h = bi1->bmiHeader.biHeight;
			int npix = w*h;
			UBYTE *a = BMIDATA(bi1);
			UBYTE *b = BMIDATA(bi2);
			UBYTE *m = BMIDATA(balph);
			for (int j=0; j<npix; j++) {
				int alph = m[4];
				int calph = 255-alph;
				m[0] = a[0]*calph/255 + b[0]*alph/255;
				m[1] = a[1]*calph/255 + b[1]*alph/255;
				m[2] = a[2]*calph/255 + b[2]*alph/255;
				m[3] = 255;
				a += 4;
				b += 4;
				m += 4;
				}
			}
		}
	}  



struct TexOps {
	UBYTE colorOp;
	UBYTE colorAlphaSource;
	UBYTE colorScale;
	UBYTE alphaOp;
	UBYTE alphaAlphaSource;
	UBYTE alphaScale;
	};

#define TX_MODULATE 0
#define TX_ALPHABLEND 1

// TBD: these values have to be corrected.
static TexOps txops[4] = {
	{ GW_TEX_MODULATE,    GW_TEX_TEXTURE, GW_TEX_SCALE_1X, GW_TEX_LEAVE,  GW_TEX_TEXTURE, GW_TEX_SCALE_1X }, 
	{ GW_TEX_ALPHA_BLEND, GW_TEX_TEXTURE, GW_TEX_SCALE_1X, GW_TEX_LEAVE,  GW_TEX_TEXTURE, GW_TEX_SCALE_1X }, 
	{ GW_TEX_MODULATE,    GW_TEX_TEXTURE, GW_TEX_SCALE_1X, GW_TEX_LEAVE,  GW_TEX_TEXTURE, GW_TEX_SCALE_1X }, 
	{ GW_TEX_MODULATE,    GW_TEX_TEXTURE, GW_TEX_SCALE_1X, GW_TEX_LEAVE,  GW_TEX_TEXTURE, GW_TEX_SCALE_1X }, 
	};

void Mix::SetTexOps(Material *mtl, int i, int type) {
	TextureInfo *ti = &mtl->texture[i];
	ti->colorOp = txops[type].colorOp;
	ti->colorAlphaSource = txops[type].colorAlphaSource;
	ti->colorScale = txops[type].colorScale;
	ti->alphaOp = txops[type].alphaOp;
	ti->alphaAlphaSource = txops[type].alphaAlphaSource;
	ti->alphaScale = txops[type].alphaScale;
	texOpsType[i] = type;
	}

#define SetTexHandle(i,nbm)  texHandle[i] = cb.MakeHandle(bmi[nbm]); bmi[nbm] = NULL; mtl->texture[i].textHandle = texHandle[i]->GetHandle();

static Color whiteCol(1.0f, 1.0f, 1.0f);

inline void Mix::SetHWTexOps(IHardwareMaterial *pIHWMat, int ntx, int type)
{
	pIHWMat->SetTextureColorArg(ntx, 1, D3DTA_TEXTURE);
	pIHWMat->SetTextureColorArg(ntx, 2, D3DTA_CURRENT);
	pIHWMat->SetTextureAlphaArg(ntx, 1, D3DTA_TEXTURE);
	pIHWMat->SetTextureAlphaArg(ntx, 2, D3DTA_CURRENT);
	switch (type) {
	case TX_MODULATE:
	default:
		pIHWMat->SetTextureColorOp(ntx, D3DTOP_MODULATE);
		pIHWMat->SetTextureAlphaOp(ntx, D3DTOP_SELECTARG2);
		pIHWMat->SetDiffuseColor(whiteCol);
		pIHWMat->SetAmbientColor(whiteCol);
		break;
	case TX_ALPHABLEND:
		pIHWMat->SetTextureColorOp(ntx, D3DTOP_BLENDTEXTUREALPHA);
		pIHWMat->SetTextureAlphaOp(ntx, D3DTOP_SELECTARG2);
		break;
	}
	pIHWMat->SetTextureTransformFlag(ntx, D3DTTFF_COUNT2);
}

void Mix::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
{
	Interval valid;
	BITMAPINFO *bmi[3];
	Texmap *sub[3]; 

	IHardwareMaterial *pIHWMat = (IHardwareMaterial *)GetProperty(PROPID_HARDWARE_MATERIAL);
	if (pIHWMat) {
		// This is only true if Direct3D is in use
		if (texHandleValid.InInterval(t)) {
			pIHWMat->SetNumTexStages(numTexHandlesUsed);
			int nt = numTexHandlesUsed;
			for (int i = 0; i < nt; i++) {
				if (texHandle[i]) {
					pIHWMat->SetTexture(i, texHandle[i]->GetHandle());
					// Kludge to pass in the TextureStage number
					mtl->texture[0].useTex = i;
					cb.GetGfxTexInfoFromTexmap(t, mtl->texture[0], subTex[useSubForTex[i]] ); 		
					// RESTORE TexOps FOR TEXTURE[i]: save an id for each texHandle that designates one of several 
					// predefined sets of texops.
					SetHWTexOps(pIHWMat, i, (nt > 1) ? TX_ALPHABLEND : TX_MODULATE);
				}
			}
			return;
		}
		else {
			DiscardTexHandles();
		}

		int forceW = 0;
		int forceH = 0;

		int nsupport = cb.NumberTexturesSupported();

		numTexHandlesUsed = 0;

		pIHWMat->SetNumTexStages(2);

		for (int i = 0; i < 3; i++) {
			if (i < 2) {
				pIHWMat->SetTexture(i, (DWORD_PTR)NULL);
			}
			bmi[i] = NULL;
			sub[i] = NULL;
			if (mapOn[i]) {
				if (subTex[i]) {
					sub[i] = subTex[i];
				}
			}
		}

		texHandleValid.SetInfinite();
		
		for (i = 0; i < 3; i++) {
			useSubForTex[i] = i;
			texOpsType[i] = 0;
			if ((i < 2) && sub[i]) {
				// Kludge to pass in the TextureStage number
				mtl->texture[0].useTex = i;
				cb.GetGfxTexInfoFromTexmap(t, mtl->texture[0], sub[i]); 		
			}
		}

		// First grab the mixing map, if any
		if (sub[2]) {
			bmi[2] = sub[2]->GetVPDisplayDIB(t, cb, valid, TRUE);
			if (bmi[2]) {
				texHandleValid &= valid;
			}
		}

		if (bmi[2] == NULL) {
			float falpha = useCurve ? mixCurve(mix) : mix;
			if (nsupport >= 2) {
				//No blending map to worry about, can stuff constant alpha into the others without constraining them.
				for (i = 0; i < 2; i++) {
					if (sub[i]) { 
						bmi[i] = sub[i]->GetVPDisplayDIB(t, cb, valid, FALSE, 0, 0); 
						if (bmi[i]) {
							texHandleValid &= valid;
						}
					}
				}
				if (bmi[0])	{
					StuffConstAlpha(1.0f - falpha, bmi[0]); // invert alpha here
					texHandle[0] = cb.MakeHandle(bmi[0]);
					bmi[0] = NULL;
					pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
					SetHWTexOps(pIHWMat, 0, TX_ALPHABLEND);
				}
				if (bmi[1])	{
					StuffConstAlpha(falpha, bmi[1]); 
					texHandle[1] = cb.MakeHandle(bmi[1]);
					bmi[1] = NULL;
					pIHWMat->SetTexture(1, texHandle[1]->GetHandle());
					SetHWTexOps(pIHWMat, 1, TX_ALPHABLEND);
					numTexHandlesUsed = 2;
				}
				else {
					numTexHandlesUsed = 1;
				}
			}
			else {
				bool bSameUV =  false;
				if(sub[0] && sub[1])
					SameUV(sub[0], sub[1]);
				BOOL any = FALSE;
				for (i = 0; i < 2; i++) {
					if (sub[i]) { 
						bmi[i] = sub[i]->GetVPDisplayDIB(t, cb, valid, FALSE, forceW, forceH); 
						if (bmi[i]) {
							texHandleValid &= valid;
							any = TRUE;
							if (bSameUV) {
								forceW = bmi[i]->bmiHeader.biWidth;
								forceH = bmi[i]->bmiHeader.biHeight;
							}
						}
					}
				}
				if (!any) {
					return;
				}
				if (bmi[0] && bmi[1] && bSameUV) {
					ConstAlphaBlend(bmi[0], bmi[1], falpha);
					texHandle[0] = cb.MakeHandle(bmi[0]);
					bmi[0] = NULL;
					pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
				}
				else {
					if (bmi[0]) {
						ConstScale(bmi[0], 1.0 - falpha);
						texHandle[0] = cb.MakeHandle(bmi[0]);
						bmi[0] = NULL;
						pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
					}
					else if (bmi[1]) {
						ConstScale(bmi[1], falpha);
						texHandle[0] = cb.MakeHandle(bmi[1]);
						bmi[1] = NULL;
						pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
						useSubForTex[0] = 1;
					}
				}
				SetHWTexOps(pIHWMat, 0, TX_MODULATE);
				numTexHandlesUsed = 1;
			}
		}
		else { 
			// (bm[2]!=NULL) -- We have a blending map.
			FixupAlpha(bmi[2]);  // move (r+g+b)/3 into alpha channel, and also apply mixing curve.
			if (nsupport) {
				// Mix map supports only 2 textures with the alpha for the blend possibly
				// coming from a third map; however as far as 3D hardware is concerned,
				// it needs it in the second map.
				if (sub[0] && sub[1] && SameUV(sub[0], sub[2]) && SameUV(sub[1], sub[2])) {
					// Have same UV's, so we can get away with stuffing the alpha from bmi[2] into the other two,
					// but have to constrain them to be the same size.
					forceW = bmi[2]->bmiHeader.biWidth; // constrain width
					forceH = bmi[2]->bmiHeader.biHeight; // constrain height
					for (i = 0; i < 2; i++) {
						if (sub[i]) { 
							bmi[i] = sub[i]->GetVPDisplayDIB(t,cb,valid,FALSE,forceW,forceH); 
							if (bmi[i]) {
								texHandleValid &= valid;
							}
						}
					}
					if ((nsupport >= 2) && (bmi[0] || bmi[1])) {
						numTexHandlesUsed = 0;
						if (bmi[0])	{
							StuffAlpha(bmi[2], bmi[0], 1);  
							texHandle[0] = cb.MakeHandle(bmi[0]);
							bmi[0] = NULL;
							pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
							SetHWTexOps(pIHWMat, 0, TX_ALPHABLEND);
							numTexHandlesUsed = 1;
						}
						if (bmi[1])	{
							StuffAlpha(bmi[2], bmi[1],0); 
							texHandle[1] = cb.MakeHandle(bmi[1]);
							bmi[1] = NULL;
							pIHWMat->SetTexture(1, texHandle[1]->GetHandle());
							SetHWTexOps(pIHWMat, 1, TX_ALPHABLEND);
							numTexHandlesUsed = 2;
						}
					}
					else {
						// Do software blend
						// Blend bmi[0] and bmi[1], storing result in bmi[2].
						AlphaBlendMaps(bmi[0], bmi[1], bmi[2]);  
						texHandle[0] = cb.MakeHandle(bmi[2]);
						bmi[2] = NULL;
						pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
						SetHWTexOps(pIHWMat, 0, TX_MODULATE);
						numTexHandlesUsed = 1;
					}
				}
				else {
					//  Just display the single bmi[2] in viewport.
					texHandle[0] = cb.MakeHandle(bmi[2]);
					bmi[2] = NULL;
					pIHWMat->SetTexture(0, texHandle[0]->GetHandle());
					SetHWTexOps(pIHWMat, 0, TX_MODULATE);
					useSubForTex[0] = 2;   // use sub[2] for texture params on subsequent renders
					numTexHandlesUsed = 1;
				}
			}
		}
		for (i = 0; i < 3; i++) {
			if (bmi[i]) {
				free(bmi[i]);
			}
		}
	}
	else {
		if (texHandleValid.InInterval(t)) {
			mtl->texture.SetCount(numTexHandlesUsed);
			int nt = numTexHandlesUsed;
			for (int i=0; i<nt; i++) {
				if (texHandle[i]) {
					mtl->texture[i].textHandle = texHandle[i]->GetHandle();
					cb.GetGfxTexInfoFromTexmap(t, mtl->texture[i], subTex[useSubForTex[i]] ); 		
					// RESTORE TexOps FOR TEXTURE[i]: save an id for each texHandle that designates one of several 
					// predefined sets of texops.
					SetTexOps(mtl,i,texOpsType[i]);
				}
			}
			return;
		}
		else {
			DiscardTexHandles();
		}

		int forceW = 0;
		int forceH = 0;

		int nsupport = cb.NumberTexturesSupported();

		numTexHandlesUsed = 0;

		mtl->texture.SetCount(3);

		for (int i=0; i<3; i++) {
			mtl->texture[i].textHandle = NULL;
			bmi[i] = NULL;
			sub[i]=NULL;
			if (mapOn[i]) {
				if (subTex[i]/*&&subTex[i]->SupportTexDisplay()*/) {
					sub[i] = subTex[i];
					}
				}
			}

		texHandleValid.SetInfinite();
		
		for (i=0; i<3; i++) {
			useSubForTex[i]=i;
			texOpsType[i]=0;
			if (sub[i]) 
				cb.GetGfxTexInfoFromTexmap(t, mtl->texture[i], sub[i] ); 		
			}

		// First grab the mixing map, if any
		if (sub[2]) {
			bmi[2] = sub[2]->GetVPDisplayDIB(t,cb,valid,TRUE); 
			if (bmi[2])
				texHandleValid &= valid;
			}

		if (bmi[2]==NULL) {
			float falpha = useCurve? mixCurve(mix): mix;
			if (nsupport>=2) {
				//No blending map to worry about, can stuff constant alpha into the others without constraining them.
				for (i=0; i<2; i++) {
					if (sub[i]) { 
						bmi[i] = sub[i]->GetVPDisplayDIB(t,cb,valid,FALSE,0,0); 
						if (bmi[i]) 
							texHandleValid &= valid;
						}
					}
				if (bmi[0])	{
					StuffConstAlpha(1.0f-falpha,bmi[0]); // invert alpha here
					SetTexHandle(0,0);
					SetTexOps(mtl,0,TX_ALPHABLEND); ;
					}
				if (bmi[1])	{
					StuffConstAlpha(falpha,bmi[1]); 
					SetTexHandle(1,1);
					SetTexOps(mtl,1,TX_ALPHABLEND);
					numTexHandlesUsed = 2;
					}
				else 
					numTexHandlesUsed = 1;
				}
			else {
				BOOL sameUV = SameUV(mtl->texture[0], mtl->texture[1]);
				BOOL any = FALSE;
				for (i=0; i<2; i++) {
					if (sub[i]) { 
						bmi[i] = sub[i]->GetVPDisplayDIB(t,cb,valid,FALSE,forceW,forceH); 
						if (bmi[i]) {
							texHandleValid &= valid;
							any = TRUE;
							if (sameUV) {
								forceW = bmi[i]->bmiHeader.biWidth;
								forceH = bmi[i]->bmiHeader.biHeight;
								}
							}
						}
					}
				if (!any)
					return;
				if (bmi[0]&&bmi[1]&&sameUV) {
					ConstAlphaBlend(bmi[0],bmi[1],falpha);  
					SetTexHandle(0,0);
					}
				else {
					if (bmi[0]) {
						ConstScale(bmi[0],1.0-falpha);
						SetTexHandle(0,0);
						}
					else if (bmi[1]) {
						ConstScale(bmi[1],falpha);
						SetTexHandle(0,1);
						useSubForTex[0] = 1;
						}
					}
				SetTexOps(mtl,0,TX_MODULATE);
				numTexHandlesUsed = 1;
				}
			}	
		else { 
			// (bm[2]!=NULL) -- We have a blending map.
			FixupAlpha(bmi[2]);  // move (r+g+b)/3 into alpha channel, and also apply mixing curve.
			if (nsupport) {
				// Mix map supports only 2 textures with the alpha for the blend possibly
				// coming from a third map; however as far as 3D hardware is concerned,
				// it needs it in the second map.
				if (SameUV(mtl->texture[0], mtl->texture[2]) && SameUV(mtl->texture[0], mtl->texture[2])) {
					// Have same UV's, so we can get away with stuffing the alpha from bmi[2] into the other two,
					// but have to constrain them to be the same size.
					forceW = bmi[2]->bmiHeader.biWidth; // constrain width
					forceH = bmi[2]->bmiHeader.biHeight; // constrain height
					for (i=0; i<2; i++) {
						if (sub[i]) { 
							bmi[i] = sub[i]->GetVPDisplayDIB(t,cb,valid,FALSE,forceW,forceH); 
							if (bmi[i]) 
								texHandleValid &= valid;
							}
						}
					if (nsupport>=2&&(bmi[0]||bmi[1])) {
						numTexHandlesUsed = 0;
						if (bmi[0])	{
							StuffAlpha(bmi[2],bmi[0],1);  
							SetTexHandle(0,0);
							SetTexOps(mtl,0,TX_ALPHABLEND);
							numTexHandlesUsed = 1;
							}
						if (bmi[1])	{
							StuffAlpha(bmi[2],bmi[1],0); 
							SetTexHandle(1,1);
							SetTexOps(mtl,1,TX_ALPHABLEND);
							numTexHandlesUsed = 2;
							}
						}
					else {
						// Do software blend
						// Blend bmi[0] and bmi[1], storing result in bmi[2].
						AlphaBlendMaps(bmi[0],bmi[1],bmi[2]);  
						SetTexHandle(0,2);
						SetTexOps(mtl,0,TX_MODULATE);
						numTexHandlesUsed = 1;
						}
					}
				else {
					//  Just display the single bmi[2] in viewport.
					mtl->texture[0] = mtl->texture[2];  // Copy texture params from texture 2.
					SetTexHandle(0,2);
					SetTexOps(mtl,0,TX_MODULATE);
					useSubForTex[0] = 2;   // use sub[2] for texture params on subsequent renders
					numTexHandlesUsed = 1;
					}
				}
	#if 0
			else if (nsupport==3) {
				// Hooray! - hardware supports 3 textures -- the way of the future.
				for (i=0; i<2; i++) {
					if (sub[i]) { 
						bmi[i] = sub[i]->GetVPDisplayDIB(t,cb,valid,FALSE,0,0); 
						if (bmi[i])	{
							texHandleValid &= valid;
							}
						}
					}
				if (bmi[0]) {
					SetTexHandle(0,0);
					SetTexOps(mtl,0,TX_ALPHABLEND); // TBD
					}
				if (bmi[1]) {								
					SetTexHandle(1,1);
					SetTexOps(mtl,1,TX_ALPHABLEND); // TBD
					}
				if (bmi[2]) {
					SetTexHandle(2,2);
					SetTexOps(mtl,2,TX_ALPHABLEND); // TBD
					}
				numTexHandlesUsed = 3;
				}
	#endif
			}
  		mtl->texture.SetCount(numTexHandlesUsed);
		for (i=0; i<3; i++)
			if (bmi[i]) 
				free(bmi[i]);
	}
}

/*
void MixDlg::DrawCurve(HDC hdc, Rect& rect) {
	HPEN blackPen = (HPEN)GetStockObject(BLACK_PEN);
	SelectPen(hdc,(HPEN)GetStockObject(BLACK_PEN));
	SelectPen(hdc,(HBRUSH)GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left, rect.top, rect.right, rect.bottom);
	MoveToEx(hdc,rect.left+1, rect.bottom-1,NULL); 
	float fx,fy;
	int ix,iy;
	float w = (float)rect.w()-2;
	float h = (float)rect.h()-2;
	for (ix =0; ix<w; ix++) {
		fx = (float(ix)+0.5f)/w;
		fy = theTex->mixCurve(fx);
		iy = int(h*fy+0.5f);
		LineTo(hdc, rect.left+1+ix, rect.bottom-1-iy);
		}
	}

void MixDlg::UpdateCurve() {
	HDC hdc = GetDC(hwCurve);
	Rect r;
	GetClientRect(hwCurve,&r);
	DrawCurve(hdc, r);
	ReleaseDC(hwCurve,hdc);
	}


BOOL MixDlg::CurveWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	switch(msg) {
		case WM_COMMAND: 	break;
		case WM_MOUSEMOVE: 		break;
		case WM_LBUTTONUP: 	break;
		case WM_PAINT: 	
			{
			PAINTSTRUCT ps;
			Rect rect;
			HDC hdc = BeginPaint( hwnd, &ps );
			if (!IsRectEmpty(&ps.rcPaint)) {
				GetClientRect( hwnd, &rect );
				DrawCurve(hdc, rect);
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

static LRESULT CALLBACK CurveWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	HWND hwpar = GetParent(hwnd);
	MixDlg *theDlg = (MixDlg *)GetWindowLongPtr(hwpar, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;
	int	res = theDlg->CurveWindowProc(hwnd,msg,wParam,lParam);
	return res;
	} 

*/