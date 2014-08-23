 /**********************************************************************
 *<
	FILE: CHECKER.CPP

	DESCRIPTION: CHECKER 2D Texture map.

	CREATED BY: Dan Silva
				Update 11/17 to param block2 by Peter Watje

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include <bmmlib.h>
#include "iparamm2.h"
#include "macrorec.h"

extern HINSTANCE hInstance;
#define SHOW_3DMAPS_WITH_2D

#define NPARAMS 3

#define NSUBTEX 2
#define NCOLS 2

#define UVGEN_REF	0
#define PBLOCK_REF	1
#define MAP1_REF	2
#define MAP2_REF	3

#define NUM_REFS	4

static Class_ID checkerClassID(CHECKER_CLASS_ID,0);

class Checker;

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { checker_params, };  // pblock ID
// checker_params param IDs
enum 
{ 
	checker_blur, checker_color1, checker_color2,
	checker_map1, checker_map2,		
	checker_map1_on, checker_map2_on, // main grad params 
	checker_coords,	  // access for UVW mapping

};

class Checker;

//--------------------------------------------------------------
// CheckSampler: checker sample function
//--------------------------------------------------------------
class CheckSampler: public MapSampler {
	Checker *check;
	public:
		CheckSampler() { check= NULL; }
		CheckSampler(Checker *c) { check= c; }
		void Set(Checker *c) { check = c; }
		AColor Sample(ShadeContext& sc, float u,float v);
		AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
		float SampleMono(ShadeContext& sc, float u,float v);
		float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
	} ;


//--------------------------------------------------------------
// Checker: A 2D texture map
//--------------------------------------------------------------

class Checker: public Texmap { 
	friend class CheckerPostLoad;
	Color col[NCOLS];
	float blur;
	UVGen *uvGen;		   // ref #0

	Texmap* subTex[NSUBTEX];  // More refs
	TexHandle *texHandle;
	Interval texHandleValid;
	Interval ivalid;
	int rollScroll;
	CheckSampler mysamp;
	public:
		Checker();
		~Checker() {
			DiscardTexHandle();
			}	
		IParamBlock2 *pblock;   // ref #1
		BOOL mapOn[2];
		static ParamDlg* uvGenDlg;	
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }

		void SetColor(int i, Color c, TimeValue t);
		void SetBlur(float f, TimeValue t);
		void NotifyChanged();
		void SwapInputs(); 
		Bitmap *BuildBitmap(int size);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);
		float MonoEvalFunction(ShadeContext& sc, float u, float v, float du, float dv);
		float CheckerFunction( float u, float v, float du, float dv);
		AColor DispEvalFunc( float u, float v);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

		// Methods for interactive display
		void DiscardTexHandle();
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		BITMAPINFO* GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, BOOL forceW=0, BOOL forceH=0);
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

		Class_ID ClassID() {	return checkerClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_DS_CHECKER); }  
		void DeleteThis() { delete this; }	

		int NumSubs() { return 2+NSUBTEX; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		// From ref
 		int NumRefs() { return 2+NSUBTEX; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		BOOL Param1;
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }
	};



int numCheckers = 0;
class CheckerClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading) { 	return new Checker; }
	const TCHAR *	ClassName() { return GetString(IDS_DS_CHECKER_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return checkerClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_2D;  }
// PW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("Checker"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static CheckerClassDesc checkerCD;
ClassDesc* GetCheckerDesc() { return &checkerCD;  }

ParamDlg* Checker::uvGenDlg;	


//-----------------------------------------------------------------------------
//  Checker
//-----------------------------------------------------------------------------

//JBW: here is the new ParamBlock2 descriptor. There is only one block for Gradients, a per-instance block.
// for the moment, some of the parameters a Tab<>s to test the Tab system.  Aslo note that all the References kept
// kept in a Gradient are mapped here, marked as P_OWNERS_REF so that the paramblock accesses and maintains them
// as references on owning Gradient.  You need to specify the refno for these owner referencetarget parameters.
// I even went so far as to expose the UVW mapping and Texture Output sub-objects this way so that they can be
// seen by the scripter and the schema-viewer

// per instance checker block

static ParamBlockDesc2 checker_param_blk ( checker_params, _T("parameters"),  0, &checkerCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_CHECKER, IDS_DS_CHECKPARMS, 0, 0, NULL, 
	// params
	checker_blur,	_T("soften"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_CHECK_BLUR,
		p_default,		0.0,
		p_range,		0.0, 5.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT,  IDC_BLUR_EDIT, IDC_BLUR_SPIN, 0.01f,
		end,
	checker_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_CHECK_COL1, 
		end,
	checker_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(1,1,1), 
		p_ui,			TYPE_COLORSWATCH, IDC_CHECK_COL2, 
		end,
	checker_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		MAP1_REF,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CHECK_TEX1,
		end,
	checker_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		MAP2_REF,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CHECK_TEX2,
		end,
	checker_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_CHKMAP1,
		end,
	checker_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_CHKMAP2,
		end,
	checker_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_COORDINATES,
		p_refno,		UVGEN_REF, 
		end,
	end
);



//dialog stuff to get the Set Ref button
class CheckerDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Checker *check;		
		CheckerDlgProc(Checker *m) {check = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			check = (Checker*)m;
//			ReloadDialog();
			}

	};



BOOL CheckerDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_CHECK_SWAP:
					{
					check->SwapInputs();
					}
				}
			break;
		}
	return FALSE;
	}

//-----------------------------------------------------------------------------
//  CheckSampler
//-----------------------------------------------------------------------------
AColor CheckSampler::SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) {
	return  check->EvalFunction(sc, u, v, du, dv);
	}
AColor CheckSampler::Sample(ShadeContext& sc, float u,float v) {
	return check->EvalFunction(sc, u, v, 0.0f, 0.0f);
	}

float CheckSampler::SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv) {
	return check->MonoEvalFunction(sc, u, v, du, dv);
	}
float CheckSampler::SampleMono(ShadeContext& sc, float u,float v) {
	return check->MonoEvalFunction(sc, u, v, 0.0f, 0.0f);
	}

//-----------------------------------------------------------------------------
//  Checker
//-----------------------------------------------------------------------------

#define CHECKER_VERSION 1


static int name_id[NPARAMS] = {IDS_DS_CHECK_BLUR,IDS_DS_COLOR1, IDS_DS_COLOR2};

static ParamBlockDescID pbdesc2[] = {
	{ TYPE_FLOAT, NULL, TRUE,checker_blur }, // blur
	{ TYPE_RGBA, NULL, TRUE,checker_color1 },  // col1
	{ TYPE_RGBA, NULL, TRUE,checker_color2 }   // col2
	};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc2, 3, 1), 
	};


void Checker::Init() {
	if (uvGen) uvGen->Reset();
	else ReplaceReference( 0, GetNewDefaultUVGen());	
	ivalid.SetEmpty();
	macroRecorder->Disable();  // disable macrorecorder during reset
		SetColor(checker_color1, Color(0.0f,0.0f,0.0f), TimeValue(0));
		SetColor(checker_color2, Color(1.0f,1.0f,1.0f), TimeValue(0));
		SetBlur(.0f, TimeValue(0));
		mapOn[0] = mapOn[1] = 1;
	macroRecorder->Enable();
	}

void Checker::Reset() {
	checkerCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(2);
	DeleteReference(3);
	Init();
	}

void Checker::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Checker::Checker() {
	mysamp.Set(this);
	texHandle = NULL;
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	uvGen = NULL;
//	paramDlg = NULL;
	checkerCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	Param1 = FALSE;
	}


void Checker::DiscardTexHandle() {
	if (texHandle) {
		texHandle->DeleteThis();
		texHandle = NULL;
		}
	}

void Checker::ActivateTexDisplay(BOOL onoff) {
	if (!onoff) 
		DiscardTexHandle();
	}

BITMAPINFO* Checker::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH) {
	Bitmap *bm;
	Interval v;
	Update(t,v);
	bm = BuildBitmap(thmaker.Size());
	BITMAPINFO *bmi = thmaker.BitmapToDIB(bm,uvGen->SymFlags(),0,forceW,forceH);
	bm->DeleteThis();
	valid.SetInfinite();
	Color ac;

	pblock->GetValue( checker_color1, t, ac, valid);
	pblock->GetValue( checker_color2, t, ac, valid );
	float b;
	pblock->GetValue( checker_blur, t, b, valid );
	return bmi;
	}

DWORD_PTR Checker::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}

inline UWORD FlToWord(float r) {
	return (UWORD)(65535.0f*r);
	}

Bitmap *Checker::BuildBitmap(int size) {
	float u,v;
	BitmapInfo bi;
	bi.SetName(_T("checkerTemp"));
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

// This is the integral of a square wave function
static inline float sintegral(float x) {
	float f = (float)floor(x);
	return (float)(f*0.5f + fmax(0.0f,(x-f)-.5f));
	}

float Checker::CheckerFunction(float u, float v, float du, float dv) {
	du = blur+du; 
	dv = blur+dv;  
	float hdu = du*.5f;
	float hdv = dv*.5f;
	// DS 10/31/00: changed "||" to "&&" to avoid divide by zero when du or dv is zero (# 266070)
	if (du!=0.0f&&dv!=0.0f) {
		float s = (sintegral(u+hdu)-sintegral(u-hdu))/du;
		float t = (sintegral(v+hdv)-sintegral(v-hdv))/dv;
		return   s*t + (1.0f-s)*(1.0f-t);
		}
	else {
		u = u - (float)floor(u);
		v = v - (float)floor(v);
		return ((u>.5f)^(v>.5f))?0.0f:1.0f;
		}
	}

// simplified evaluation for interactive render
AColor Checker::DispEvalFunc( float u, float v) {
	float a = CheckerFunction(u,v,0.0f, 0.0f);
	if (a==0.0f) return  col[0];
	else if (a==1.0f)return  col[1];
	else return a*col[1] + (1.0f-a)*col[0];
	}


// This function should only be called for samples within the 0,1 
// box, so a lot of this complication is not necessary.
AColor Checker::EvalFunction(ShadeContext& sc, float u, float v, float du, float dv) {
	float a = CheckerFunction(u,v,du,dv);
	if (a<=.0005f) 
		return mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
	else if (a>=.9995f)
		return mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
	else {
		AColor c0  = mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
		AColor c1  = mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
		return a*c1 + (1.0f-a)*c0;
		}
	}


// This function should only be called for samples within the 0,1 
// box, so a lot of this complication is not necessary.
float Checker::MonoEvalFunction(ShadeContext& sc, float u, float v, float du, float dv) {
	float a = CheckerFunction(u,v,du,dv);
	if (a<=.0005f) 
		{
		float level;
		level = mapOn[0]&&subTex[0] ? subTex[0]->EvalMono(sc): Intens(col[0]);
		return level;
		}
	else if (a>=.9995f)
		return mapOn[1]&&subTex[1] ? subTex[1]->EvalMono(sc): Intens(col[1]);
	else {
		float c0  = mapOn[0]&&subTex[0] ? subTex[0]->EvalMono(sc): Intens(col[0]);
		float c1  = mapOn[1]&&subTex[1] ? subTex[1]->EvalMono(sc): Intens(col[1]);
		return a*c1 + (1.0f-a)*c0;
		}
	}

static AColor black(0.0f,0.0f,0.0f,0.0f);

AColor Checker::EvalColor(ShadeContext& sc) {
	if (!sc.doMaps) return black;
	if (gbufID) sc.SetGBufferID(gbufID);
	return uvGen->EvalUVMap(sc,&mysamp);
	}

float Checker::EvalMono(ShadeContext& sc) {
	if (!sc.doMaps) 	return 0.0f;
	if (gbufID) sc.SetGBufferID(gbufID);
	return uvGen->EvalUVMapMono(sc,&mysamp);
	}

Point3 Checker::EvalNormalPerturb(ShadeContext& sc) {
	Point3 dPdu, dPdv;
	if (!sc.doMaps) return Point3(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
	uvGen->GetBumpDP(sc,dPdu,dPdv);
	Point2 dM = uvGen->EvalDeriv(sc,&mysamp);
	Point3 np = dM.x*dPdu+dM.y*dPdv;


	Texmap *sub0 = mapOn[0]?subTex[0]:NULL;
	Texmap *sub1 = mapOn[1]?subTex[1]:NULL;
	if (sub0||sub1) {
		// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da)
		float a,b,k;
		Point3 da,db;
		Point2 UV, dUV;
		uvGen->GetUV(sc, UV,dUV);
		k = CheckerFunction(UV.x, UV.y, dUV.x, dUV.y);
		if (sub0) {
			a = sub0->EvalMono(sc);
			da = sub0->EvalNormalPerturb(sc);
			}
		else {
			 a = Intens(col[0]);
			 da = Point3(0.0f,0.0f,0.0f);
			 }
		if (sub1) {
			b = sub1->EvalMono(sc);
			db = sub1->EvalNormalPerturb(sc);
			}
		else {
			 b = Intens(col[1]);
			 db= Point3(0.0f,0.0f,0.0f);
			 }
		np = (b-a)*np + k*(db-da);
		}
	else 
		np *= Intens(col[1])-Intens(col[0]);
	return np;
	}

RefTargetHandle Checker::Clone(RemapDir &remap) {
	Checker *mnew = new Checker();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(uvGen));
	mnew->ReplaceReference(1,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->blur = blur;
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

ParamDlg* Checker::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {

// JBW: the main difference here is the automatic creation of a ParamDlg by the new
// ClassDesc2 function CreateParamDlgs().  This mirrors the way BeginEditParams()
// can be redirected to the ClassDesc2 for automatic ParamMap2 management.  In this 
// case a special subclass of ParamDlg, AutoMParamDlg, defined in IParamm2.h, is 
// created.  It can act as a 'master' ParamDlg to which you can add any number of 
// secondary dialogs and it will make sure all the secondary dialogs are kept 
// up-to-date and deleted as necessary.  

	// create the rollout dialogs
	uvGenDlg = uvGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = checkerCD.CreateParamDlgs(hwMtlEdit, imp, this);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(uvGenDlg);
//attach a dlg proc to handle the swap button 
	checker_param_blk.SetUserDlgProc(new CheckerDlgProc(this));

	return masterDlg;
	}


BOOL Checker::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if (dlg == uvGenDlg)
		uvGenDlg->SetThing(uvGen);
	else 
		return FALSE;
	return TRUE;
}




void Checker::Update(TimeValue t, Interval& valid) {		

	if (Param1)  //this is a hack to fix old 2.5 files check boxes
		{
		pblock->SetValue( checker_map1_on, 0, mapOn[0]);
		pblock->SetValue( checker_map2_on, 0, mapOn[1]);
		Param1 = FALSE;
		}

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		uvGen->Update(t,ivalid);
		pblock->GetValue( checker_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( checker_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( checker_blur, t, blur, ivalid );
		pblock->GetValue( checker_map1_on, t, mapOn[0], ivalid);
		pblock->GetValue( checker_map2_on, t, mapOn[1], ivalid);

		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		}
	valid &= ivalid;
	}


void Checker::SetColor(int i, Color c, TimeValue t) {
	if (i== checker_color1)
		col[0] = c;
	else col[1] = c;

	pblock->SetValue( i, t, c);
	}

void Checker::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(checker_color1,0,checker_color2,0);
	checker_param_blk.InvalidateUI(checker_color1);
	checker_param_blk.InvalidateUI(checker_color2);
	checker_param_blk.InvalidateUI(checker_map1);
	checker_param_blk.InvalidateUI(checker_map2);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
	}

void Checker::SetBlur(float f, TimeValue t) { 
	blur = f; 
	pblock->SetValue( checker_blur, t, f);
	}

RefTargetHandle Checker::GetReference(int i) {
	switch(i) {
		case 0: return uvGen;
		case 1:	return pblock ;
		default:return subTex[i-2];
		}
	}

void Checker::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0: uvGen = (UVGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2 *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
	}

void Checker::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2,m);
	if (i==0)
		{
		checker_param_blk.InvalidateUI(checker_map1);
		ivalid.SetEmpty();
		}
	else if (i==1)
		{
		checker_param_blk.InvalidateUI(checker_map2);
		ivalid.SetEmpty();
		}

	}

TSTR Checker::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Checker::SubAnim(int i) {
	switch (i) {
		case 0: return uvGen;
		case 1: return pblock;
		default: return subTex[i-2]; 
		}
	}

TSTR Checker::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_COORDINATES));		
		case 1: return TSTR(GetString(IDS_DS_PARAMETERS));		
		default: return GetSubTexmapTVName(i-2);
		}
	}

RefResult Checker::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			{
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//				if (hTarget != uvGen  && hTarget != pblock ) 
					checker_param_blk.InvalidateUI(changing_param);
				if (changing_param != -1)
					DiscardTexHandle();
				}
			break;
			}
		case REFMSG_UV_SYM_CHANGE:
			DiscardTexHandle();  
			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 0x4000
#define MAPOFF_CHUNK 0x1000
#define PARAM2_CHUNK 0x1010

IOResult Checker::Save(ISave *isave) { 
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
	 

//watje
class CheckerPostLoadCallback:public  PostLoadCallback
{
public:
	Checker      *s;
	CheckerPostLoadCallback(Checker *r) {s=r;}
	void proc(ILoad *iload);
};

/*
void CheckerPostLoadCallback::proc(ILoad *iload)
{

	if (s->Param1)
		{
		s->pblock->SetValue( checker_map1_on, 0, s->mapOn[0]);
		s->pblock->SetValue( checker_map2_on, 0, s->mapOn[1]);
		}

	delete this;

}
 */


IOResult Checker::Load(ILoad *iload) { 
//	ULONG nb;
	IOResult res;
	int id;
	Param1 = TRUE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
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
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, 1, &checker_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

//copy loaded values into the new param block
//	CheckerPostLoadCallback* checkerplcb = new CheckerPostLoadCallback(this);
//	iload->RegisterPostLoadCallback(checkerplcb);

	return IO_OK;
	}
