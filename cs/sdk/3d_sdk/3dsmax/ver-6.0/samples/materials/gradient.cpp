/**********************************************************************
 *<
	FILE: gradient2.cpp

	DESCRIPTION: A simple gradient texture map.
				 Ed. 2 using ParamBlock2's

	CREATED BY: Rolf Berteig
			    Ed. 2 John Wainwright

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include <bmmlib.h>
#include "stdmat.h"
#include "iparamm2.h"
#include "buildver.h"

#ifndef NO_MAPTYPE_GRADIENT	 // orb 01-03-2001 Removing map types

extern HINSTANCE hInstance;

#define NSUBTEX 3
#define NCOLS 3

static Class_ID gradClassID(GRADIENT_CLASS_ID,0);

#define NOISE_REGULAR	0
#define NOISE_FRACTAL	1
#define NOISE_TURB		2

#define GRAD_LINEAR	0
#define GRAD_RADIAL	1

class Gradient;

#define UVGEN_REF	0
#define PBLOCK_REF	1
#define MAP1_REF	2
#define MAP2_REF	3
#define MAP3_REF	4
#define TEXOUT_REF	5

#define NUM_REFS	6

class Gradient;

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs
enum { grad_params, };  // pblock ID
// grad_params param IDs
enum 
{ 
	grad_color1, grad_color2, grad_color3, grad_map1, grad_map2,		// main grad params 
	grad_map3, grad_map1_on, grad_map2_on, grad_map3_on,	
	grad_center, grad_type,					
	grad_amount, grad_noise_type, grad_size, grad_phase, grad_levels,	// noise
	grad_low_thresh, grad_high_thresh, grad_thresh_smooth,				// noise threshold
	grad_coords, grad_output,											// access for UVW mapping & output refs
};

//--------------------------------------------------------------
// MySampler: gradient sample function
//--------------------------------------------------------------
class GradSampler: public MapSampler {
	Gradient *grad;
	public:
		GradSampler() { grad= NULL; }
		GradSampler(Gradient *c) { grad= c; }
		void Set(Gradient *c) { grad = c; }
		AColor Sample(ShadeContext& sc, float u,float v);
		AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
//		float SampleMono(ShadeContext& sc, float u,float v);
//		float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
	} ;

// JBW: the Gradient dialog class has gone, all UI is managed automatically by auto-generated
// ParamMap2's from the paramblock descriptor

class Gradient: public GradTex { 
	public:	
		static ParamDlg* uvGenDlg;	
		static ParamDlg* texoutDlg;
		Color col[NCOLS];
		int type, noiseType;
		float amount, size, phase, size1, center, levels, low, high, smooth, sd, hminusl;
		UVGen *uvGen;				// ref #0
// JBW: use an IParamBlock2
		IParamBlock2 *pblock;		// ref #1		
		Texmap* subTex[NSUBTEX];	// More refs: 2,3,4  (also accessible as Texmap* params in pblock)
		BOOL mapOn[NSUBTEX];
		TextureOutput *texout;		// ref #5
		TexHandle *texHandle;
		Interval texHandleValid;
		Interval ivalid;
		int rollScroll;
		GradSampler mysamp;

		Gradient();
		~Gradient() { DiscardTexHandle(); }
		void EnableStuff();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		BOOL SetDlgThing(ParamDlg* dlg);
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) {Interval v; Update(t,v); return ivalid;}

		StdUVGen* GetUVGen() { return (StdUVGen *)uvGen; }
		TextureOutput* GetTexout() { return texout; }
		void SetOutputLevel(TimeValue t, float v) {texout->SetOutputLevel(t,v); }
		void SetMidPoint(float m, TimeValue t=0) {	pblock->SetValue(grad_center, t, m);	}
		void NotifyChanged();		
		Bitmap *BuildBitmap(int size);
		float NoiseFunc(Point3 p);

		// Evaluate the color of map for the context.
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);
		AColor DispEvalFunc( float u, float v);		
		float gradFunc(float u, float v);

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
		int GetMapChannel () { return uvGen->GetMapChannel (); }
		UVGen *GetTheUVGen() { return uvGen; }

		// Requirements
		ULONG LocalRequirements(int subMtlNum) {
			return uvGen->Requirements(subMtlNum); 
			}

		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			uvGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
			}

		// Methods to access texture maps of material
		int NumSubTexmaps() {return NSUBTEX;}
		Texmap* GetSubTexmap(int i) {return subTex[i];}		
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		void InitSlotType(int sType) {if (uvGen) uvGen->InitSlotType(sType);}

		Class_ID ClassID() {return gradClassID;}
		SClass_ID SuperClassID() {return TEXMAP_CLASS_ID;}
		void GetClassName(TSTR& s) {s=GetString(IDS_RB_GRADIENT);}
		void DeleteThis() {delete this;}

		// from Animatable
		int NumSubs() {return NUM_REFS;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) {return subNum;}
// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock

		// From ref
 		int NumRefs() {return NUM_REFS;}
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave); 

		// From Texmap
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }
};

// JBW: need to use the new ClassDesc2 for ParamBlock2 classes
class GradientClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return GetAppID() != kAPP_VIZR;}
	void *			Create(BOOL loading) {return new Gradient;}
	const TCHAR *	ClassName() {return GetString(IDS_RB_GRADIENT_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() {return TEXMAP_CLASS_ID;}
	Class_ID 		ClassID() {return gradClassID;}
	const TCHAR* 	Category() {return TEXMAP_CAT_2D;}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("Gradient"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static GradientClassDesc gradCD;
ClassDesc* GetGradientDesc() { return &gradCD; }
ParamDlg* Gradient::uvGenDlg;	
ParamDlg* Gradient::texoutDlg;

//-----------------------------------------------------------------------------
//  GradSampler
//-----------------------------------------------------------------------------
AColor GradSampler::SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) {
	return grad->EvalFunction(sc, u, v, du, dv);
	}

AColor GradSampler::Sample(ShadeContext& sc, float u,float v) {
	return grad->EvalFunction(sc, u, v, 0.0f, 0.0f);
	}


class GradientDlgProc : public ParamMap2UserDlgProc 
	{
	public:
		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return FALSE; }
		void SetThing(ReferenceTarget *m) {	
			Gradient *theGrad = (Gradient *)m;
			if (theGrad) theGrad->EnableStuff();
			}
		void DeleteThis() { }
	};

static GradientDlgProc gradientDlgProc;

class GradientPBAccessor : public PBAccessor
	{
	public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
		{
		Gradient* p = (Gradient*)owner;
		p->EnableStuff();
		}
	};
	
static GradientPBAccessor gradient_accessor;

//-----------------------------------------------------------------------------
//  Gradient
//-----------------------------------------------------------------------------

//JBW: here is the new ParamBlock2 descriptor. There is only one block for Gradients, a per-instance block.
// for the moment, some of the parameters a Tab<>s to test the Tab system.  Aslo note that all the References kept
// kept in a Gradient are mapped here, marked as P_OWNERS_REF so that the paramblock accesses and maintains them
// as references on owning Gradient.  You need to specify the refno for these owner referencetarget parameters.
// I even went so far as to expose the UVW mapping and Texture Output sub-objects this way so that they can be
// seen by the scripter and the schema-viewer

// shader rollout dialog proc

// per instance gradient block
static ParamBlockDesc2 grad_param_blk ( grad_params, _T("parameters"),  0, &gradCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_GRADIENT, IDS_RB_GRADIENTPARAMS, 0, 0, &gradientDlgProc, 
	// params
	grad_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_GRAD_COL1, 
		end,
	grad_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_GRAD_COL2, 
		end,
	grad_color3,	 _T("color3"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR3,	
		p_default,		Color(1.0,1.0,1.0),
		p_ui,			TYPE_COLORSWATCH, IDC_GRAD_COL3, 
		end,
	grad_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		MAP1_REF,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_GRAD_TEX1,
		end,
	grad_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		MAP2_REF,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_GRAD_TEX2,
		end,
	grad_map3,		_T("map3"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP3,
		p_refno,		MAP3_REF,
		p_subtexno,		2,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_GRAD_TEX3,
		end,
	grad_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	grad_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	grad_map3_on,	_T("map3Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP3ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON3,
		end,
	grad_center,	_T("color2Pos"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_CENTER2,
		p_default,		0.5,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_CENTER, IDC_GRAD_CENTERSPIN, SPIN_AUTOSCALE, 
		end,
	grad_type,		_T("gradientType"),	TYPE_INT,				0,			IDS_GRAD_TYPE,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2, IDC_GRAD_LINEAR, IDC_GRAD_RADIAL, 
		end,
	grad_amount,	_T("noiseAmount"), TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_NOISEAMT,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_AMOUNT, IDC_GRAD_AMOUNTSPIN, 0.1, 
		end,
	grad_noise_type, _T("noiseType"), TYPE_INT,				0,				IDS_NOISE_TYPE,
		p_default,		0,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 3, IDC_GRAD_REGULAR, IDC_GRAD_FRACT, IDC_GRAD_TURB,
		p_accessor,		&gradient_accessor,
		end,
	grad_size,		_T("noiseSize"), TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_NOISESIZE,
		p_default,		1.0,
		p_range,		0.0, 999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_SIZE, IDC_GRAD_SIZESPIN, 0.1, 
		end,
	grad_phase,		_T("noisePhase"), TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_NSPHS,
		p_default,		0.0,
		p_range,		0.0, 999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_PHASE, IDC_GRAD_PHASESPIN, 0.1, 
		end,
	grad_levels,	_T("noiseLevels"), TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_NSLEV,
		p_default,		4.0,
		p_range,		0.0, 10.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_LEVELS, IDC_GRAD_LEVELSSPIN, 0.05, 
		end,
	grad_low_thresh, _T("noiseThresholdLow"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_LOWTHRESH,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_LOWTHRESH, IDC_GRAD_LOWTHRESHSPIN, 0.005, 
		end,
	grad_high_thresh, _T("noiseThresholdHigh"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_HIGHTHRESH,
		p_default,		1.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_HIGHTHRESH, IDC_GRAD_HIGHTHRESHSPIN, 0.005, 
		end,
	grad_thresh_smooth, _T("noiseThresholdSMooth"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_THRESHOLDSMOOTHING,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRAD_THRESHSMOOTH, IDC_GRAD_THRESHSMOOTHSPIN, 0.005, 
		end,
	grad_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_COORDINATES,
		p_refno,		UVGEN_REF, 
		end,
	grad_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_OUTPUT,
		p_refno,		TEXOUT_REF, 
		end,
	end
	);

// JBW: the old version ParamBlockDescID arrays are used to permit old version Gradients to be 
// loaded.  Note that the ID in each DescID is replaced with the ParamBlock2 ID so that the 
// automatic post-load callback canfix things up properly

#define GRADIENT_VERSION 4

static ParamBlockDescID pbdesc1[] = {	
	{ TYPE_POINT3, NULL, TRUE, grad_color1 }, 	// col1
	{ TYPE_POINT3, NULL, TRUE, grad_color2 }, 	// col2
	{ TYPE_POINT3, NULL, TRUE, grad_color3 }, 	// col3
	{ TYPE_INT, NULL, FALSE, grad_type }, 		// type
	{ TYPE_FLOAT, NULL, TRUE, grad_amount }, 	// amount
	{ TYPE_FLOAT, NULL, TRUE, grad_size }, 		// size
	{ TYPE_FLOAT, NULL, TRUE, grad_phase }, 	// phase
	};

static ParamBlockDescID pbdesc2[] = {	
	{ TYPE_POINT3, NULL, TRUE, grad_color1 }, 	// col1
	{ TYPE_POINT3, NULL, TRUE, grad_color2 }, 	// col2
	{ TYPE_POINT3, NULL, TRUE, grad_color3 }, 	// col3
	{ TYPE_INT,    NULL, FALSE , grad_type }, 	// type
	{ TYPE_FLOAT, NULL, TRUE, grad_amount }, 	// amount
	{ TYPE_FLOAT, NULL, TRUE, grad_size }, 	// size
	{ TYPE_FLOAT, NULL, TRUE, grad_phase }, 	// phase
	{ TYPE_FLOAT, NULL, TRUE, grad_center }, 	// center
	};

static ParamBlockDescID pbdesc3[] = {	
	{ TYPE_POINT3, NULL, TRUE, grad_color1 }, 	// col1
	{ TYPE_POINT3, NULL, TRUE, grad_color2 }, 	// col2
	{ TYPE_POINT3, NULL, TRUE, grad_color3 }, 	// col3
	{ TYPE_INT,    NULL, FALSE , grad_type }, 	// type
	{ TYPE_FLOAT, NULL, TRUE, grad_amount }, 	// amount
	{ TYPE_FLOAT, NULL, TRUE, grad_size }, 		// size
	{ TYPE_FLOAT, NULL, TRUE, grad_phase }, 	// phase
	{ TYPE_FLOAT, NULL, TRUE, grad_center }, 	// center
	{ TYPE_INT, NULL, FALSE, -1 }, 				// turbulence
	{ TYPE_FLOAT, NULL, TRUE, grad_levels }, 	// levels
	};

static ParamBlockDescID pbdesc4[] = {	
	{ TYPE_POINT3, NULL, TRUE, grad_color1 }, 	// col1
	{ TYPE_POINT3, NULL, TRUE, grad_color2 }, 	// col2
	{ TYPE_POINT3, NULL, TRUE, grad_color3 }, 	// col3
	{ TYPE_INT,    NULL, FALSE , grad_type }, 	// type
	{ TYPE_FLOAT, NULL, TRUE, grad_amount }, 	// amount
	{ TYPE_FLOAT, NULL, TRUE, grad_size }, 		// size
	{ TYPE_FLOAT, NULL, TRUE, grad_phase }, 	// phase
	{ TYPE_FLOAT, NULL, TRUE, grad_center }, 	// center
	{ TYPE_INT, NULL, FALSE, grad_noise_type }, 	// noise type
	{ TYPE_FLOAT, NULL, TRUE, grad_levels }, 		// levels
	{ TYPE_FLOAT, NULL, TRUE, grad_low_thresh }, 	// low thresh
	{ TYPE_FLOAT, NULL, TRUE, grad_high_thresh }, 	// high thresh
	{ TYPE_FLOAT, NULL, TRUE, grad_thresh_smooth }, // thresh smoothing
	};

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc1, 7, 1), 	
	ParamVersionDesc(pbdesc2, 8, 2), 
	ParamVersionDesc(pbdesc3, 10, 3), 
	ParamVersionDesc(pbdesc4, 13, 4), 
	};

#define NUM_OLDVERSIONS	4

void Gradient::Init() 
	{
	if (uvGen) uvGen->Reset();
	else ReplaceReference( UVGEN_REF, GetNewDefaultUVGen());	
	if (texout) texout->Reset();
	else ReplaceReference( TEXOUT_REF, GetNewDefaultTextureOutput());
	ivalid.SetEmpty();
	}

void Gradient::Reset() 
	{
	gradCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	DeleteReference(MAP1_REF);
	DeleteReference(MAP2_REF);
	DeleteReference(MAP3_REF);
	}

void Gradient::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Gradient::Gradient() 
	{
	texHandle = NULL;
	subTex[0] = subTex[1] = subTex[2] =NULL;
	pblock    = NULL;
	uvGen     = NULL;
	texout    = NULL;
	mysamp.Set(this);
	// JBW: ask the ClassDesc to make the P_AUTO_CONSTRUCT blocks
	gradCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	rollScroll=0;
	}

void Gradient::DiscardTexHandle() 
	{
	if (texHandle) {
		texHandle->DeleteThis();
		texHandle = NULL;
		}
	}

void Gradient::ActivateTexDisplay(BOOL onoff) {
	if (!onoff) 
		DiscardTexHandle();
	}

BITMAPINFO* Gradient::GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono, BOOL forceW, BOOL forceH) {
	Bitmap *bm;
	Interval v;
	Update(t,v);
	bm = BuildBitmap(thmaker.Size());
	BITMAPINFO *bmi = thmaker.BitmapToDIB(bm,uvGen->SymFlags(),0,forceW,forceH);
	bm->DeleteThis();
	valid.SetInfinite();
	Color ac;
	pblock->GetValue( grad_color1, t, ac, valid );
	pblock->GetValue( grad_color2, t, ac, valid );
	pblock->GetValue( grad_color3, t, ac, valid );
	return bmi;
	}

DWORD_PTR Gradient::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
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

Bitmap *Gradient::BuildBitmap(int size) {
	float u,v;
	BitmapInfo bi;
	bi.SetName(GetString(IDS_RB_GRADTEMP));
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

static float junk = 1.0f;
float Gradient::NoiseFunc(Point3 p)
	{
	float res;
	switch (noiseType) {
		case NOISE_TURB: {
			float sum = 0.0f;
			float l,f = 1.0f;			
			for (l = levels; l>=1.0f; l-=1.0f) {				
				sum += (float)fabs(noise3(p*f))/f;
				f *= 2.0f;
				}
			if (l>0.0f) {				
				sum += l*(float)fabs(noise3(p*f))/f;
				}
			res = sum;
			break;
			}
			
		case NOISE_REGULAR:
			// multiply by junk (1.0) to avoid compiler bug
			res = junk*noise3(p);
			break;

		case NOISE_FRACTAL:
			if (levels==1.0f) {
				res = junk*noise3(p);
			} else {
				float sum = 0.0f;
				float l,f = 1.0f;				
				for (l = levels; l>=1.0f; l-=1.0f) {					
					sum += noise3(p*f)/f;
					f *= 2.0f;
					}
				if (l>0.0f) {					
					sum += l*noise3(p*f)/f;
					}
				res = sum;
				}
			break;
		}
	
	if (low<high) {
		res = 2.0f * sramp((res+1.0f)/2.0f,low,high,sd) - 1.0f;
		}
	return res;
	}

float Gradient::gradFunc(float u, float v) {
	float a;
	if (type==GRAD_LINEAR) {
		a = v;	
	} else {
		u-=0.5f;
		v-=0.5f;
		a = (float)sqrt(u*u+v*v)*2.0f;
		if (a>1.0f) a = 1.0f;
		}
	
	if (amount!=0.0f) {
		a += amount*NoiseFunc(Point3(u*size1+1.0f,v*size1+1.0f,phase));
		if (a<0.0f) a = 0.0f;
		if (a>1.0f) a = 1.0f;
		}
//	a = (a*a*(3-2*a));
	return a;
	}

// simplified evaluation for interactive render
AColor Gradient::DispEvalFunc(float u, float v) 
	{
	float a = gradFunc(u,v);

	if (a<center) {
		a = a/center;
		return col[2]*(1.0f-a) + col[1]*a;
	} else 
	if (a>center) {
		a = (a-center)/(1.0f-center);
		return col[1]*(1.0f-a) + col[0]*a;
	} else return col[1];
	}

AColor Gradient::EvalFunction(
		ShadeContext& sc, float u, float v, float du, float dv) 
	{	
	int n1=0, n2=0;
	float a = gradFunc(u,v);

	if (a<center) {
		a = a/center;
		n1 = 2;
		n2 = 1;		
	} else 
	if (a>center) {
		a = (a-center)/(1.0f-center);		
		n1 = 1;
		n2 = 0;
	} else {
		return (mapOn[1]&&subTex[1]) ? subTex[1]->EvalColor(sc): col[1];		
		}

	Color c1, c2;
	c1 = mapOn[n1]&&subTex[n1] ? subTex[n1]->EvalColor(sc): col[n1];	
	c2 = mapOn[n2]&&subTex[n2] ? subTex[n2]->EvalColor(sc): col[n2];
	return c1*(1.0f-a) + c2*a;
	}

static AColor black(0.0f,0.0f,0.0f,0.0f);

AColor Gradient::EvalColor(ShadeContext& sc) {
	if (!sc.doMaps) 
		return black;
	AColor c;
	if (sc.GetCache(this,c)) 
		return c; 
	if (gbufID) sc.SetGBufferID(gbufID);
	c = texout->Filter(uvGen->EvalUVMap(sc,&mysamp));
	sc.PutCache(this,c); 
	return c;
	}

float Gradient::EvalMono(ShadeContext& sc) {
	if (!sc.doMaps) 
		return 0.0f;
	float f;
	if (sc.GetCache(this,f)) 
		return f; 
	if (gbufID) sc.SetGBufferID(gbufID);
	f = texout->Filter(uvGen->EvalUVMapMono(sc,&mysamp));
	sc.PutCache(this,f); 
	return f;
	}

#define EVALSUBPERTURB(aa,daa,i)   \
	if (sub[i]) {	aa = sub[i]->EvalMono(sc);	daa = sub[i]->EvalNormalPerturb(sc);		} \
	else {	 aa = Intens(col[i]);	 daa = Point3(0.0f,0.0f,0.0f);		 }

Point3 Gradient::EvalNormalPerturb(ShadeContext& sc) 
	{
	Point3 dPdu, dPdv;
	if (!sc.doMaps) return Point3(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
	Point2 dM = uvGen->EvalDeriv(sc,&mysamp);
	uvGen->GetBumpDP(sc,dPdu,dPdv);

#if 0
	// Blinn's algorithm
	Point3 N = sc.Normal();
	Point3 uVec = CrossProd(N,dPdv);
	Point3 vVec = CrossProd(N,dPdu);
	Point3 np = -dM.x*uVec+dM.y*vVec;
#else 
	// Lazy algorithm
	Point3 np = dM.x*dPdu+dM.y*dPdv;
//	return texout->Filter(dM.x*dPdu+dM.y*dPdv);
#endif
	Texmap* sub[3];
	for (int i=0; i<3; i++) 
		sub[i] = mapOn[i]?subTex[i]:NULL;
	if (sub[0]||sub[1]||sub[2]) {
		// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
		float a,b,k;
		Point3 da,db;
		Point2 UV, dUV;
		uvGen->GetUV(sc, UV,dUV);
		k = gradFunc(UV.x,UV.y);
		if (k<=center) {	
			k = k/center; 		
			EVALSUBPERTURB(a,da,2);
			EVALSUBPERTURB(b,db,1);
			} 
		else {
			k = (k-center)/(1.0f-center);		
			EVALSUBPERTURB(a,da,1);
			EVALSUBPERTURB(b,db,0);
			}
		np = (b-a)*np + k*(db-da) + da;
		}
	return texout->Filter(np);
	}

RefTargetHandle Gradient::Clone(RemapDir &remap) 
	{
	Gradient *mnew = new Gradient();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(0,remap.CloneRef(uvGen));
	mnew->ReplaceReference(TEXOUT_REF,remap.CloneRef(texout));
	mnew->ReplaceReference(1,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->col[2] = col[2];	
	mnew->ivalid.SetEmpty();	
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+2,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}


void Gradient::EnableStuff() {
	if (pblock) {
		IParamMap2 *map = pblock->GetMap();
		pblock->GetValue( grad_noise_type, 0, noiseType, FOREVER );
		if (map) {
			map->Enable(grad_levels, noiseType==NOISE_REGULAR?FALSE:TRUE);
			}
		}
	}


ParamDlg* Gradient::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
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
	uvGenDlg = uvGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = gradCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(uvGenDlg);
	masterDlg->AddDlg(texoutDlg);
	EnableStuff();
	return masterDlg;
}

BOOL Gradient::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if (dlg == uvGenDlg)
		uvGenDlg->SetThing(uvGen);
	else if (dlg == texoutDlg)
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}

void Gradient::Update(TimeValue t, Interval& valid) 
	{
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		uvGen->Update(t,ivalid);
		texout->Update(t,ivalid);
		pblock->GetValue( grad_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( grad_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( grad_color3, t, col[2], ivalid );
		col[2].ClampMinMax();		
		pblock->GetValue( grad_map1_on, t, mapOn[0], ivalid);
		pblock->GetValue( grad_map2_on, t, mapOn[1], ivalid);
		pblock->GetValue( grad_map3_on, t, mapOn[2], ivalid);
		pblock->GetValue( grad_type, t, type, ivalid );
		pblock->GetValue( grad_noise_type, t, noiseType, ivalid );
		pblock->GetValue( grad_amount, t, amount, ivalid );
		pblock->GetValue( grad_size, t, size, ivalid );
		pblock->GetValue( grad_phase, t, phase, ivalid );
		pblock->GetValue( grad_center, t, center, ivalid );
		pblock->GetValue( grad_levels, t, levels, ivalid );
		pblock->GetValue( grad_high_thresh, t, high, ivalid );
		pblock->GetValue( grad_low_thresh, t, low, ivalid );
		pblock->GetValue( grad_thresh_smooth, t, smooth, ivalid );		
		if (low>high) {
			float temp = low;
			low = high;
			high = temp;
			}
		hminusl = (high-low);
		sd = hminusl*0.5f*smooth;
		if (size!=0.0f) size1 = 20.0f/size;
		else size1 = 0.0f;
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}
		EnableStuff();
		}
	valid &= ivalid;
	}


RefTargetHandle Gradient::GetReference(int i) {
	switch(i) {
		case 0: return uvGen;
		case 1:	return pblock ;
		case TEXOUT_REF: return texout;
		default:return subTex[i-2];
		}
	}

void Gradient::SetReference(int i, RefTargetHandle rtarg) 
{
	switch(i) {
		case 0: uvGen = (UVGen *)rtarg; break;
		case 1:	pblock = (IParamBlock2*)rtarg; break;
		case TEXOUT_REF: texout = (TextureOutput *)rtarg; break;
		default: 
			subTex[i-2] = (Texmap *)rtarg; 
// JBW: cause UI updating for auto-UI dialigs by calling InvalidateUI() on the Block descriptror
			grad_param_blk.InvalidateUI();
			break;
		}
}

void Gradient::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2,m);
	if (i==0)
		{
		grad_param_blk.InvalidateUI(grad_map1);
		ivalid.SetEmpty();
		}
	else if (i==1)
		{
		grad_param_blk.InvalidateUI(grad_map2);
		ivalid.SetEmpty();
		}	
	else if (i==2)
		{
		grad_param_blk.InvalidateUI(grad_map3);
		ivalid.SetEmpty();
		}	

	}

TSTR Gradient::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_RB_COLOR1); 
		case 1:  return GetString(IDS_RB_COLOR2);
		case 2:  return GetString(IDS_RB_COLOR3);
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Gradient::SubAnim(int i) {
	switch (i) {
		case 0: return uvGen;
		case 1: return pblock;
		case TEXOUT_REF: return texout;
		default: return subTex[i-2]; 
		}
	}

TSTR Gradient::SubAnimName(int i) {
	switch (i) {
		case 0: return TSTR(GetString(IDS_DS_COORDINATES));		
		case 1: return TSTR(GetString(IDS_DS_PARAMETERS));		
		case TEXOUT_REF: return TSTR(GetString(IDS_DS_OUTPUT));
		default: return GetSubTexmapTVName(i-2);
		}
	}

RefResult Gradient::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) 
{
	switch (message) 
	{
		case REFMSG_CHANGE:
		{
			ivalid.SetEmpty();
			if (hTarget == pblock) {
				// see if this message came from a changing parameter in the pblock,
				// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				if (hTarget != uvGen && hTarget != texout) 
					grad_param_blk.InvalidateUI(changing_param);
				if (changing_param != -1)
					DiscardTexHandle();
				}
			// notify our dependents that we've changed
			// NotifyChanged();  //DS this is redundant
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

IOResult Gradient::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
// JBW: removed the MAPOFF_CHUNK saving as this is now part of the ParamBlock2
	return IO_OK;
	}	

IOResult Gradient::Load(ILoad *iload) 
{ 
	IOResult res;
	int id;
// JBW:  continue to recognize old-version chunks for map enable switches, but
// we don't write them any more (they are in the ParamBlock2)
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
			case MAPOFF_CHUNK+2:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	// JBW: register old version ParamBlock to ParamBlock2 converter
	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &grad_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);
	return IO_OK;
}

#endif // NO_MAPTYPE_GRADIENT	