/**********************************************************************
 *<
	FILE: noisemod.cpp

	DESCRIPTION:  A Noise Modifier

	CREATED BY: Rolf Berteig

	HISTORY: created 18 October 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"

#ifndef NO_MODIFIER_NOISE // JP Morel - June 28th 2002

#include "iparamm.h"
#include "simpmod.h"
#include "simpobj.h"
#include "texutil.h"

// in mods.cpp
extern HINSTANCE hInstance;

#define BIGFLOAT	float(999999)
#define NOISEOSM_CLASS_ID	0xf997b124

class NoiseMod : public SimpleMod {	
	public:
		static IParamMap *pmapParam;

		NoiseMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_NOISEMOD); }  
		virtual Class_ID ClassID() {return Class_ID(NOISEOSM_CLASS_ID,0);}		
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		TCHAR *GetObjectName() { return GetString(IDS_RB_NOISE); }
		IOResult Load(ILoad *iload);

		// From simple mod
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
		Interval GetValidity(TimeValue t);
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);
		void InvalidateUI() {if (pmapParam) pmapParam->Invalidate();}
	};

class NoiseDeformer: public Deformer {
	public:
		Matrix3 tm,invtm;
		int fractal;
		float scale, rough, iterations;
		Point3 strength;
		float time;

		NoiseDeformer();			
		NoiseDeformer(
			ModContext &mc,
			TimeValue t,
			int seed, float scale, int fractal, float iterations,
			float rough, int anim, float freq,			
			Point3 strength,
			TimeValue phase,
			Matrix3& modmat, Matrix3& modinv);
		Point3 Map(int i, Point3 p); 
	};

#define NOISEWSM_CLASSID	Class_ID(NOISEOSM_CLASS_ID,1)

class NoiseWSM : public SimpleOSMToWSMObject {
	public:
		NoiseWSM() {}
		NoiseWSM(NoiseMod *m) : SimpleOSMToWSMObject(m) {}
		void DeleteThis() { delete this; }
		SClass_ID SuperClassID() {return WSM_OBJECT_CLASS_ID;}
		Class_ID ClassID() {return NOISEWSM_CLASSID;} 
		TCHAR *GetObjectName() {return GetString(IDS_RB_NOISE);}
		RefTargetHandle Clone(RemapDir& remap)
			{NoiseWSM *newobj = new NoiseWSM((NoiseMod*)mod->Clone(remap));
		newobj->SimpleOSMToWSMClone(this,remap);
		BaseClone(this, newobj, remap);
		return newobj;}
	};


//--- ClassDescriptor and class vars ---------------------------------

IParamMap *NoiseMod::pmapParam = NULL;



class NoiseClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new NoiseMod; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_NOISE_CLASS); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(NOISEOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static NoiseClassDesc noiseDesc;
extern ClassDesc* GetNoiseModDesc() { return &noiseDesc; }

class NoiseWSMClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
		{if (loading) return new NoiseWSM; else return new NoiseWSM(new NoiseMod);}
	const TCHAR *	ClassName() { return GetString(IDS_RB_NOISE_CLASS); }
	SClass_ID		SuperClassID() { return WSM_OBJECT_CLASS_ID; }
	Class_ID		ClassID() { return NOISEWSM_CLASSID; }
	const TCHAR* 	Category() {return GetSpaceWarpCatString(SPACEWARP_CAT_MODBASED);}
	};

static NoiseWSMClassDesc noiseWSMDesc;
extern ClassDesc* GetNoiseWSMDesc() { return &noiseWSMDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_SEED			0
#define PB_SCALE		1
#define PB_FRACTAL		2
#define PB_ROUGH		3
#define PB_ITERATIONS	4
#define PB_ANIMATE		5
#define PB_FREQ			6
#define PB_PHASE		7
#define PB_STRENGTH		8


//
//
// Parameters


static ParamUIDesc descParam[] = {
	// Seed
	ParamUIDesc(
		PB_SEED,
		EDITTYPE_INT,
		IDC_MODNOISE_SEED,IDC_MODNOISE_SEEDSPIN,
		0.0f,99999999.0f,
		0.1f),

	// Scale
	ParamUIDesc(
		PB_SCALE,
		EDITTYPE_FLOAT,
		IDC_MODNOISE_SCALE,IDC_MODNOISE_SCALESPIN,
		0.0f,BIGFLOAT,
		SPIN_AUTOSCALE),
	
	// Fractal
	ParamUIDesc(PB_FRACTAL,TYPE_SINGLECHEKBOX,IDC_MODNOISE_FRACTAL),
	
	// Roughness
	ParamUIDesc(
		PB_ROUGH,
		EDITTYPE_FLOAT,
		IDC_MODNOISE_ROUGHNESS,IDC_MODNOISE_ROUGHNESSSPIN,
		0.0f,1.0f,
		0.005f),	

	// Iterations
	ParamUIDesc(
		PB_ITERATIONS,
		EDITTYPE_FLOAT,
		IDC_MODNOISE_ITERATIONS,IDC_MODNOISE_ITERATIONSSPIN,
		1.0f,10.0f,
		0.01f),	

	// Animate
	ParamUIDesc(PB_ANIMATE,TYPE_SINGLECHEKBOX,IDC_MODNOISE_ANIMATE),

	// Frequency
	ParamUIDesc(
		PB_FREQ,
		EDITTYPE_FLOAT,
		IDC_MODNOISE_FREQ,IDC_MODNOISE_FREQSPIN,
		0.00001f,BIGFLOAT,
		0.01f),
	
	// Phase
	ParamUIDesc(
		PB_PHASE,
		EDITTYPE_TIME,
		IDC_MODNOISE_PHASE,IDC_MODNOISE_PHASESPIN,
		(float)-999999999,(float)999999999,
		10.0f),

	// Strength
	ParamUIDesc(			  
		PB_STRENGTH,
		EDITTYPE_UNIVERSE,		                       
		IDC_MODNOISE_XSTRENGTH,IDC_MODNOISE_XSTRENGTHSPIN,
		IDC_MODNOISE_YSTRENGTH,IDC_MODNOISE_YSTRENGTHSPIN,
		IDC_MODNOISE_ZSTRENGTH,IDC_MODNOISE_ZSTRENGTHSPIN,
		-BIGFLOAT,BIGFLOAT,		
		SPIN_AUTOSCALE),	
	};
#define PARAMDESC_LENGH 9

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },	
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_FLOAT, NULL, FALSE, 5 },
	{ TYPE_POINT3, NULL, TRUE, 6 } };


static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_FLOAT, NULL, TRUE, 7 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_FLOAT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, TRUE, 8 },
	{ TYPE_POINT3, NULL, TRUE, 6 } };
#define PBLOCK_LENGTH	9

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,7,0)
	};
#define NUM_OLDVERSIONS	1

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);


//--- NoiseDlgProc -------------------------------


class NoiseDlgProc : public ParamMapUserDlgProc {
	public:
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void SetState(HWND hWnd);
		void DeleteThis() {}
	};
static NoiseDlgProc theNoiseProc;

void NoiseDlgProc::SetState(HWND hWnd)
	{
	ISpinnerControl *spin1 = GetISpinner(GetDlgItem(hWnd,IDC_MODNOISE_ROUGHNESSSPIN));
	ISpinnerControl *spin2 = GetISpinner(GetDlgItem(hWnd,IDC_MODNOISE_ITERATIONSSPIN));
	if (IsDlgButtonChecked(hWnd,IDC_MODNOISE_FRACTAL)) {
		spin1->Enable();
		spin2->Enable();
		EnableWindow(GetDlgItem(hWnd,IDC_MODNOISE_ROUGHNESSLABEL),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_MODNOISE_ITERATIONSLABEL),TRUE);
	} else {
		spin1->Disable();
		spin2->Disable();
		EnableWindow(GetDlgItem(hWnd,IDC_MODNOISE_ROUGHNESSLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_MODNOISE_ITERATIONSLABEL),FALSE);
		}
	ReleaseISpinner(spin1);
	ReleaseISpinner(spin2);
	}

BOOL NoiseDlgProc::DlgProc(
		TimeValue t,IParamMap *map,
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			SetState(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_MODNOISE_FRACTAL:
					SetState(hWnd);
					break;
				}
			break;
		}
	return FALSE;
	}

//--- Noise methods -------------------------------


NoiseMod::NoiseMod() : SimpleMod()
	{	
	MakeRefByID(FOREVER, SIMPMOD_PBLOCKREF, 
		CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	
	pblock->SetValue(PB_SCALE,0,100.0f);
	pblock->SetValue(PB_FRACTAL,0,0);	
	pblock->SetValue(PB_FREQ,0,0.25f);
	pblock->SetValue(PB_ITERATIONS,0,6.0f);

#ifdef DESIGN_VER
		pblock->SetValue(PB_ANIMATE,0,1);
		pblock->SetValue(PB_PHASE,GetAnimStart(),GetAnimStart());
#else
		SuspendAnimate();
		AnimateOn();
		pblock->SetValue(PB_PHASE,GetAnimStart(),GetAnimStart());
		pblock->SetValue(PB_PHASE,GetAnimEnd(),GetAnimEnd());
		ResumeAnimate();
#endif
	}

IOResult NoiseMod::Load(ILoad *iload)
	{
	Modifier::Load(iload);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,SIMPMOD_PBLOCKREF));
	return IO_OK;
	}

void NoiseMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SimpleMod::BeginEditParams(ip,flags,prev);
		
	pmapParam = CreateCPParamMap(
		descParam,PARAMDESC_LENGH,
		pblock,
		ip,
		hInstance,
#ifdef DESIGN_VER
		MAKEINTRESOURCE(IDD_NOISEPARAM_VIZ),
#else
		MAKEINTRESOURCE(IDD_NOISEPARAM),
#endif
		GetString(IDS_RB_PARAMETERS),
		0);	
	pmapParam->SetUserDlgProc(&theNoiseProc);
	}
		
void NoiseMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	SimpleMod::EndEditParams(ip,flags,next);
	DestroyCPParamMap(pmapParam);
	}

Interval NoiseMod::GetValidity(TimeValue t)
	{
	float f;	
	int i, animate;
	Point3 p;
	Interval valid = FOREVER;
	pblock->GetValue(PB_SEED,t,i,valid);
	pblock->GetValue(PB_SCALE,t,f,valid);
	pblock->GetValue(PB_FRACTAL,t,i,valid);
	pblock->GetValue(PB_ROUGH,t,f,valid);
	pblock->GetValue(PB_ITERATIONS,t,f,valid);
	pblock->GetValue(PB_ANIMATE,t,animate,valid);
	pblock->GetValue(PB_FREQ,t,f,valid);		
	pblock->GetValue(PB_STRENGTH,t,p,valid);
	if (animate) pblock->GetValue(PB_PHASE,t,i,valid);
	if (animate) valid.SetInstant(t);
	return valid;
	}

RefTargetHandle NoiseMod::Clone(RemapDir& remap) 
	{	
	NoiseMod* newmod = new NoiseMod();	
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock->Clone(remap));
	newmod->SimpleModClone(this);
	BaseClone(this, newmod, remap);
	return(newmod);
	}

NoiseDeformer::NoiseDeformer() 
	{ 
	tm.IdentityMatrix();
	invtm.IdentityMatrix();
	}

Point3 NoiseDeformer::Map(int i, Point3 p)
	{	
	Point3 d, sp;
	p = p * tm;
	sp = p * scale + Point3(0.5f,0.5f,0.5f);
	if (fractal) {
		d.x = fBm1(Point3(sp.y,sp.z,time),rough,2.0f,iterations);
		d.y = fBm1(Point3(sp.x,sp.z,time),rough,2.0f,iterations);
		d.z = fBm1(Point3(sp.x,sp.y,time),rough,2.0f,iterations);
	} else {
		d.x = noise3(Point3(sp.y,sp.z,time));
		d.y = noise3(Point3(sp.x,sp.z,time));
		d.z = noise3(Point3(sp.x,sp.y,time));
		}	
	return (p+(d*strength)) * invtm;	
	}

NoiseDeformer::NoiseDeformer(
		ModContext &mc,
		TimeValue t,
		int seed, float scale, int fractal, float iterations,
		float rough, int anim, float freq,
		Point3 strength,
		TimeValue phase,
		Matrix3& modmat, Matrix3& modinv)
	{
	tm    = modmat;
	invtm = modinv;	
	this->scale      = (scale==0.0f) ? 0.00001f : (1.0f/scale);
	this->fractal    = fractal;
	this->rough      = 1.0f-rough;
	this->strength   = strength;
	this->iterations = iterations;
	if (anim) {
		time = float(phase) * float(0.005) * freq * 
			1200.0f/float(TIME_TICKSPERSEC) + Perm(seed);
	} else {
		time = (float)Perm(seed);
		}
	} 


Deformer& NoiseMod::GetDeformer(
		TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
	{
	int fractal, seed, anim;
	float scale, rough, freq, iterations;
	TimeValue phase;
	Point3 strength;

	pblock->GetValue(PB_SEED,t,seed,FOREVER);
	pblock->GetValue(PB_SCALE,t,scale,FOREVER);
	pblock->GetValue(PB_FRACTAL,t,fractal,FOREVER);
	pblock->GetValue(PB_ITERATIONS,t,iterations,FOREVER);
	pblock->GetValue(PB_ROUGH,t,rough,FOREVER);
	pblock->GetValue(PB_ANIMATE,t,anim,FOREVER);
	pblock->GetValue(PB_FREQ,t,freq,FOREVER);
	pblock->GetValue(PB_PHASE,t,phase,FOREVER);
	pblock->GetValue(PB_STRENGTH,t,strength,FOREVER);
	if (iterations<1.0f) iterations = 1.0f;
	if (scale<0.00001f) scale = 0.00001f;

	static NoiseDeformer deformer;
	deformer = NoiseDeformer(mc,t,seed,scale,fractal,iterations,rough,anim,freq,strength,phase,mat,invmat);
	return deformer;
	}


ParamDimension *NoiseMod::GetParameterDim(int pbIndex)
	{
	switch (pbIndex) {
		case PB_SEED:		return defaultDim;
		case PB_SCALE:		return defaultDim;
		case PB_FRACTAL:	return defaultDim;
		case PB_ROUGH:		return defaultDim;
		case PB_STRENGTH:	return stdWorldDim;
		case PB_PHASE:		return stdTimeDim;
		case PB_ITERATIONS:	return defaultDim;
		default:			return defaultDim;
		}
	}

TSTR NoiseMod::GetParameterName(int pbIndex)
	{
	switch (pbIndex) {
		case PB_SEED:		return GetString(IDS_RB_SEED);
		case PB_SCALE:		return GetString(IDS_RB_SCALE);
		case PB_FRACTAL:	return GetString(IDS_RB_FRACTAL);
		case PB_ROUGH:		return GetString(IDS_RB_ROUGH);
		case PB_STRENGTH:	return GetString(IDS_RB_STRENGTH2);
		case PB_PHASE:		return GetString(IDS_RB_PHASE);
		case PB_ITERATIONS:	return GetString(IDS_RB_ITERATIONS);
		default:			return TSTR(_T(""));
		}
	}

#endif // NO_MODIFIER_NOISE 
