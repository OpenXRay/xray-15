/**********************************************************************
 *<
	FILE: NOISE.CPP

	DESCRIPTION: NOISE 3D Texture map.

	CREATED BY: Dan Silva
				Updated to Param Block2 by Peter Watje 12/1/1998
	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "macrorec.h"

#define DOAA
#define BLENDBAND 4.0f  // blend to average value over this many orders of magnitude


extern HINSTANCE hInstance;

#define SHOW_3DMAPS_WITH_2D

#define NSUBTEX 2
#define NCOLS 2

static Class_ID noiseClassID(NOISE_CLASS_ID,0);

#ifdef RENDER_VER
#define DEFAULT_NOISE_SIZE	0.1f
#else	// !RENDER_VER
#define DEFAULT_NOISE_SIZE	25.f
#endif	// !RENDER_VER

#define NPARAMS 7
#define NOISE_VERSION 4


#define NOISE_REGULAR	0
#define NOISE_FRACTAL	1
#define NOISE_TURB		2

class Noise;
//--------------------------------------------------------------
// Noise: A 3D texture map
//--------------------------------------------------------------

#define XYZGEN_REF	0
#define PBLOCK_REF	1
#define MAP1_REF	2
#define MAP2_REF	3
#define TEXOUT_REF	4

#define NUM_REFS	5


class Noise: public Tex3D { 
	friend class NoisePostLoad;
	static float avgAbsNs;
	Color col[NCOLS];
	float size;
	XYZGen *xyzGen;		   // ref #0
	IParamBlock2 *pblock;   // ref #1	
	Texmap* subTex[NSUBTEX];  // More refs (2 & 3)
	BOOL mapOn[NSUBTEX];
	TextureOutput *texout; // ref #4
	Interval ivalid, cacheValid;
	int noiseType;
	float phase;
	float levels;
	float low, high, smooth, sd, hminusl;
	float avgValue;
	int vers;
	int rollScroll;
	BOOL filter;
#ifdef SHOW_3DMAPS_WITH_2D
	TexHandle *texHandle;
	Interval texHandleValid;
#endif
	CRITICAL_SECTION csect;
//	NoiseDlg *paramDlg;
	float Turb(Point3 p, float lev);
	public:
		Noise();
		~Noise() { 
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			DeleteCriticalSection(&csect);
			}
		static ParamDlg* xyzGenDlg;	
		static ParamDlg* texoutDlg;
		void EnableStuff();
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		void Update(TimeValue t, Interval& valid);
		void UpdateCache(TimeValue t);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) { Interval v; Update(t,v); return ivalid; }
		void ReadSXPData(TCHAR *name, void *sxpdata);

		XYZGen *GetTheXYZGen() { return xyzGen; }

		void SetOutputLevel(TimeValue t, float v) {texout->SetOutputLevel(t,v); }
		void SetColor(int i, Color c, TimeValue t);
		void SetSize(float f, TimeValue t);
		void SetPhase(float f, TimeValue t);
		void SetLevels(float f, TimeValue t);
		void NotifyChanged();
		void SwapInputs(); 

	
		// Evaluate the color of map for the context.
		RGBA EvalColor(ShadeContext& sc);

		// optimized evaluation for monochrome use
		float EvalMono(ShadeContext& sc);

		// For Bump mapping, need a perturbation to apply to a normal.
		// Leave it up to the Texmap to determine how to do this.
		Point3 EvalNormalPerturb(ShadeContext& sc);

#ifdef SHOW_3DMAPS_WITH_2D
		void DiscardTexHandle() {
			if (texHandle) {
				texHandle->DeleteThis();
				texHandle = NULL;
				}
			}
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff) {
			if (!onoff) DiscardTexHandle();
			}
		DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
#endif SHOW_3DMAPS_WITH_2D


		float LimitLevel(Point3 dp, float &smw);
		void ComputeAvgValue();
		float NoiseFunction(Point3 p,  float limLev, float smWidth);

		ULONG LocalRequirements(int subMtlNum) { return xyzGen->Requirements(subMtlNum); }
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  
			xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq); 
			}

		// Methods to access texture maps of material
		int NumSubTexmaps() { return NSUBTEX; }
		Texmap* GetSubTexmap(int i) { return subTex[i]; }
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);

		Class_ID ClassID() {	return noiseClassID; }
		SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
		void GetClassName(TSTR& s) { s= GetString(IDS_RB_NOISE); }  
		void DeleteThis() { delete this; }	

		ULONG UVSpacesNeeded() { return 0; } 

		int NumSubs() { return NUM_REFS; }  
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum) { return subNum; }

		// From ref
 		int NumRefs() { return NUM_REFS; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		BOOL loadOnChecks;
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		int FixLevel0(); 

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		float GetHiThresh();
		void SetHiThresh(float v);
		float GetLowThresh();
		void SetLowThresh(float v);

		// Same as Marble ...
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }
	};

class NoiseClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new Noise; }
	const TCHAR *	ClassName() { return GetString(IDS_RB_NOISE_CDESC); } // mjm - 2.3.99
	SClass_ID		SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID 		ClassID() { return noiseClassID; }
	const TCHAR* 	Category() { return TEXMAP_CAT_3D;  }
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("noise"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static NoiseClassDesc noiseCD;

ClassDesc* GetNoiseDesc() { return &noiseCD;  }
ParamDlg* Noise::xyzGenDlg;	
ParamDlg* Noise::texoutDlg;

enum { noise_params };  // pblock ID
// grad_params param IDs
enum 
{ 
	noise_color1, noise_color2,
	noise_map1, noise_map2,		
	noise_map1_on, noise_map2_on, 
	noise_size, noise_phase, noise_levels, noise_lowthresh, noise_hithresh,
	noise_type,// main grad params 
	noise_coords, noise_output,	  // access for UVW mapping
};



/*
class NoisePBAccessor : public PBAccessor
	{
	public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
		{
		Noise* p = (Noise*)owner;
		p->EnableStuff();
		}
	};
	
static NoisePBAccessor noise_accessor;
*/

// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.
// JBW: since the old path controller kept all parameters as instance data members, this setter callback
// is implemented to to reduce changes to existing code 


class NoisePBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		Noise* p = (Noise*)owner;
		if (p!=NULL)
			{
			switch (id)
				{
				case noise_lowthresh:
					{
					float high = p->GetHiThresh();
					float low = v.f;
					if (low > high) p->SetHiThresh(low+0.001f);
					break;
					}
				case noise_hithresh:
					{
					float low = p->GetLowThresh();
					float high = v.f;
					if (low > high) p->SetLowThresh(high-0.001f);
					break;
					}
				}
			p->EnableStuff();
			}

	}
};

static NoisePBAccessor noise_accessor;


//JBW: here is the new ParamBlock2 descriptor. There is only one block for Noises, a per-instance block.
// for the moment, some of the parameters a Tab<>s to test the Tab system.  Aslo note that all the References kept
// kept in a Noise are mapped here, marked as P_OWNERS_REF so that the paramblock accesses and maintains them
// as references on owning Noise.  You need to specify the refno for these owner referencetarget parameters.
// I even went so far as to expose the UVW mapping and Texture Output sub-objects this way so that they can be
// seen by the scripter and the schema-viewer

// per instance noise block



static ParamBlockDesc2 noise_param_blk ( noise_params, _T("parameters"),  0, &noiseCD, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
	//rollout
	IDD_NOISE, IDS_DS_NOISEPARMS, 0, 0, NULL, 
	// params
	noise_color1,	 _T("color1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR1,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_NOISE_COL1, 
		end,
	noise_color2,	 _T("color2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_COLOR2,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_NOISE_COL2, 
		end,
	noise_map1,		_T("map1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP1,
		p_refno,		MAP1_REF,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_NOISE_TEX1,
		end,
	noise_map2,		_T("map2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_JW_MAP2,
		p_refno,		MAP2_REF,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_NOISE_TEX2,
		end,
	noise_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP1ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON1,
		end,
	noise_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_JW_MAP2ENABLE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_MAPON2,
		end,
	noise_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_NOISESIZE,
		p_default,		DEFAULT_NOISE_SIZE,
		p_range,		0.001, 1000000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_NOISESIZE_EDIT, IDC_NOISESIZE_SPIN, 0.1f, 
		end,
	noise_phase,		_T("phase"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_DS_PHASE,
		p_default,		25.f,
		p_range,		-1000.0, 1000.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_NOISEPHASE_EDIT, IDC_NOISEPHASE_SPIN, 0.1f, 
		end,
	noise_levels,	_T("levels"), TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_NSLEV,
		p_default,		3.0f,
		p_range,		1.0f, 10.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_NOISELEV_EDIT, IDC_NOISELEV_SPIN, 0.1f, 
		end,
	noise_lowthresh, _T("thresholdLow"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_LOWTHRESH,
		p_default,		0.0f,
		p_range,		0.0f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_NOISE_LOWTHRESH, IDC_NOISE_LOWTHRESHSPIN, 0.005f, 
		p_accessor,		&noise_accessor,
		end,
	noise_hithresh, _T("thresholdHigh"), TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_HIGHTHRESH,
		p_default,		1.0f,
		p_range,		0.0f, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_NOISE_HIGHTHRESH, IDC_NOISE_HIGHTHRESHSPIN, 0.005f, 
		p_accessor,		&noise_accessor,
		end,
	noise_type, _T("type"), TYPE_INT,				0,				IDS_NOISE_TYPE,
		p_default,		0,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 3, IDC_NOISE_REGULAR, IDC_NOISE_FRACT, IDC_NOISE_TURB,
		p_accessor,		&noise_accessor,
		end,
	noise_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_COORDINATES,
		p_refno,		XYZGEN_REF, 
		end,
	noise_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_DS_OUTPUT,
		p_refno,		TEXOUT_REF, 
		end,

	end
);


//dialog stuff to get the Set Ref button
class NoiseDlgProc : public ParamMap2UserDlgProc {
//public ParamMapUserDlgProc {
	public:
		Noise *noise;		
		NoiseDlgProc(Noise *m) {noise = m;}		
		BOOL DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {delete this;}
		void SetThing(ReferenceTarget *m) {
			noise = (Noise*)m;
			noise->EnableStuff();
			}

	};



BOOL NoiseDlgProc::DlgProc(
		TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
				{
				case IDC_NOISE_SWAP:
					{
					noise->SwapInputs();
					}
				break;
				}
			break;
		}
	return FALSE;
	}



// Version 1 params
static ParamBlockDescID pbdesc1[] = {
	{ TYPE_FLOAT, NULL, TRUE,noise_size }, 	// size
	{ TYPE_POINT3, NULL, TRUE,noise_color1 },  // col1
	{ TYPE_POINT3, NULL, TRUE,noise_color2 }   // col2
	};

static ParamBlockDescID pbdesc2[] = {
	{ TYPE_FLOAT, NULL, TRUE,noise_size }, 	// size
	{ TYPE_POINT3, NULL, TRUE,noise_color1 },  // col1
	{ TYPE_POINT3, NULL, TRUE,noise_color2 },  // col2
	{ TYPE_FLOAT, NULL, TRUE,noise_phase }, 	// phase
	{ TYPE_FLOAT, NULL, TRUE,noise_levels } 	// levels
	};

static ParamBlockDescID pbdesc3[] = {
	{ TYPE_FLOAT, NULL, TRUE,noise_size }, 	// size
	{ TYPE_RGBA, NULL, TRUE,noise_color1 },  // col1
	{ TYPE_RGBA, NULL, TRUE,noise_color2 },  // col2
	{ TYPE_FLOAT, NULL, TRUE,noise_phase }, 	// phase
	{ TYPE_FLOAT, NULL, TRUE,noise_levels }, 	// levels
	{ TYPE_FLOAT, NULL, TRUE,noise_lowthresh },	// low thresh
	{ TYPE_FLOAT, NULL, TRUE,noise_hithresh },	// high thresh	
	};

// Current version params

static ParamVersionDesc versions[] = {
	ParamVersionDesc(pbdesc1,3,1),	// Version 1 params
	ParamVersionDesc(pbdesc2,4,2),	// Version 2 params
	ParamVersionDesc(pbdesc3,7,3),	// Version 2 params
	};
#define NUM_OLDVERSIONS	3


float Noise::avgAbsNs = -1.0f;

void Noise::Init() {
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference( XYZGEN_REF, GetNewDefaultXYZGen());	
	if (texout) texout->Reset();
	else ReplaceReference( TEXOUT_REF, GetNewDefaultTextureOutput());		
	ivalid.SetEmpty();
	cacheValid.SetEmpty();
	macroRecorder->Disable();  // disable macrorecorder during reset
		SetColor(0, Color(0.0f,0.0f,0.0f), TimeValue(0));
		SetColor(1, Color(1.0f,1.0f,1.0f), TimeValue(0));
		noiseType = NOISE_REGULAR;
		SetSize(DEFAULT_NOISE_SIZE, TimeValue(0));
		SetPhase(.0f,TimeValue(0));
		SetLevels(3.0f,TimeValue(0));
		pblock->SetValue(noise_hithresh,0,1.0f);
	macroRecorder->Enable();
	for (int i=0; i<NSUBTEX; i++) 
		mapOn[i] = 1;
	}

void Noise::Reset() {
	noiseCD.Reset(this, TRUE);	// reset all pb2's
	Init();
	for (int i=0; i<NSUBTEX; i++) 
		DeleteReference(i+2);
	}

void Noise::NotifyChanged() {
	ivalid.SetEmpty();
	cacheValid.SetEmpty();
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}

Noise::Noise() {
	InitializeCriticalSection(&csect);
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	subTex[0] = subTex[1] = NULL;
	pblock = NULL;
	xyzGen = NULL;
	texout = NULL;
//	texHandle = NULL;
	noiseType = NOISE_REGULAR;
	noiseCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	vers = 0;
	rollScroll=0;
	}


RefTargetHandle Noise::Clone(RemapDir &remap) {
	Noise *mnew = new Noise();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	mnew->ReplaceReference(XYZGEN_REF,remap.CloneRef(xyzGen));
	mnew->ReplaceReference(TEXOUT_REF,remap.CloneRef(texout));
	mnew->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));
	mnew->col[0] = col[0];
	mnew->col[1] = col[1];
	mnew->noiseType = noiseType;
	mnew->size = size;
	mnew->avgValue = avgValue;
	mnew->ivalid.SetEmpty();	
	mnew->cacheValid.SetEmpty();
	for (int i = 0; i<NSUBTEX; i++) {
		mnew->subTex[i] = NULL;
		if (subTex[i])
			mnew->ReplaceReference(i+2,remap.CloneRef(subTex[i]));
		mnew->mapOn[i] = mapOn[i];
		}
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
	}


void Noise::EnableStuff() {
	if (pblock) {
		IParamMap2 *map = pblock->GetMap();
		pblock->GetValue( noise_type, 0, noiseType, FOREVER );
		if (map) {
			map->Enable(noise_levels, noiseType==NOISE_REGULAR?FALSE:TRUE);
			}
		}
	}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR Noise::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif

void Noise::UpdateCache(TimeValue t)
	{
	BOOL IsGenuineIntel();//Fix provided by Intel so we need to check which CPU we are running on 
                              //(at the moment of this submission, the function always returns true)


        //Intel's fix is to check the validity before entering the critical section, if it's valid, no need to update
	if ( IsGenuineIntel() &&  cacheValid.InInterval(t))
		return;

	EnterCriticalSection(&csect);
	if (!cacheValid.InInterval(t)) {
		ComputeAvgValue();
		cacheValid = ivalid;
		}
	LeaveCriticalSection(&csect);
	}

ParamDlg* Noise::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
// JBW: the main difference here is the automatic creation of a ParamDlg by the new
// ClassDesc2 function CreateParamDlgs().  This mirrors the way BeginEditParams()
// can be redirected to the ClassDesc2 for automatic ParamMap2 management.  In this 
// case a special subclass of ParamDlg, AutoMParamDlg, defined in IParamm2.h, is 
// created.  It can act as a 'master' ParamDlg to which you can add any number of 
// secondary dialogs and it will make sure all the secondary dialogs are kept 
// up-to-date and deleted as necessary.  

	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* masterDlg = noiseCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the master
	masterDlg->AddDlg(xyzGenDlg);
	masterDlg->AddDlg(texoutDlg);
	noise_param_blk.SetUserDlgProc(new NoiseDlgProc(this));
	EnableStuff();
	return masterDlg;

	}



float Noise::GetHiThresh()
{
if (pblock != NULL)
	return pblock->GetFloat(noise_hithresh, GetCOREInterface()->GetTime());
else return 0;
}
void Noise::SetHiThresh(float v)
{
if (pblock != NULL)
	{
	pblock->SetValue(noise_hithresh,GetCOREInterface()->GetTime(),v);
	noise_param_blk.InvalidateUI(noise_hithresh);
	}

}
float Noise::GetLowThresh()
{
if (pblock != NULL)
	return  pblock->GetFloat(noise_lowthresh, GetCOREInterface()->GetTime());
else return 0;
}
void Noise::SetLowThresh(float v)
{
if (pblock != NULL)
	{
	pblock->SetValue(noise_lowthresh,GetCOREInterface()->GetTime(),v);
	noise_param_blk.InvalidateUI(noise_lowthresh);
}
}

struct Col24 {ULONG r,g,b; };
#define NOISE_VERS 0xC79A0

struct NoiseState {
	ulong version;
	float size;
	float x1,y1,z1;
	float x2,y2,z2;
	Col24 col1,col2;
	long frame1,frame2;
	};

static Color ColrFromCol24(Col24 a) {
	Color c;
	c.r = (float)a.r/255.0f;
	c.g = (float)a.g/255.0f;
	c.b = (float)a.b/255.0f;
	return c;
	}

void Noise::ReadSXPData(TCHAR *name, void *sxpdata) {
	NoiseState *state = (NoiseState*)sxpdata;
	if (state!=NULL && state->version==NOISE_VERS) {
		SetColor(0, ColrFromCol24(state->col1),0);
		SetColor(1, ColrFromCol24(state->col2),0);
		SetSize(state->size,0);
		}
	}

#define NAVG 10000
		
void Noise::ComputeAvgValue() {
#ifdef DOAA
	srand(1345);
	Point3 p;
	int i;
	float sum = 0.0f;
	filter = FALSE;
	for (i=0; i<NAVG; i++) {
		p.x = float(rand())/100.0f;			
		p.y = float(rand())/100.0f;			
		p.z = float(rand())/100.0f;			
		sum += NoiseFunction(p,levels,0.0f);
		}
	avgValue = sum/float(NAVG);
	sum = 0.0f;
	if (avgAbsNs<0.0f) {
#define NAVGNS 10000
		float phase;
		for (i=0; i<NAVGNS; i++) {
			p.x = float(rand())/100.0f;			
			p.y = float(rand())/100.0f;			
			p.z = float(rand())/100.0f;			
			phase = float(rand())/100.0f;
			sum += (float)fabs(noise4(p,phase));
			}
		avgAbsNs = sum/float(NAVGNS);
		}
#endif
	}

BOOL Noise::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((xyzGenDlg!= NULL) && (dlg == xyzGenDlg))
		xyzGenDlg->SetThing(xyzGen);
	else if ((texoutDlg!= NULL) && (dlg == texoutDlg))
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}


void Noise::Update(TimeValue t, Interval& valid) {

	if (pblock == NULL) return;

	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();
		if (xyzGen != NULL)
			xyzGen->Update(t,ivalid);
		if (texout != NULL)
			texout->Update(t,ivalid);
		pblock->GetValue( noise_color1, t, col[0], ivalid );
		col[0].ClampMinMax();
		pblock->GetValue( noise_color2, t, col[1], ivalid );
		col[1].ClampMinMax();
		pblock->GetValue( noise_size, t,   size, ivalid );
		pblock->GetValue( noise_phase, t,  phase, ivalid );
		pblock->GetValue( noise_levels, t,  levels, ivalid );
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,ivalid);
			}		
		pblock->GetValue( noise_hithresh, t, high, ivalid );
		pblock->GetValue( noise_lowthresh, t, low, ivalid );		

		if (high<low) {
			float tmp = low;
			low = high;
			high = tmp;
			}
		pblock->GetValue( noise_map1_on, t, mapOn[0], ivalid);
		pblock->GetValue( noise_map2_on, t, mapOn[1], ivalid);
		pblock->GetValue( noise_type, t, noiseType, ivalid);


		//ComputeAvgValue(); // moved to UpdateCache DDS 10/3/00
		EnableStuff();
		}
	valid &= ivalid;
	}

void Noise::SwapInputs() {
	Color t = col[0]; col[0] = col[1]; col[1] = t;
	Texmap *x = subTex[0];  subTex[0] = subTex[1];  subTex[1] = x;
	pblock->SwapControllers(noise_color1,0,noise_color2,0);
	noise_param_blk.InvalidateUI(noise_color1);
	noise_param_blk.InvalidateUI(noise_color2);
	noise_param_blk.InvalidateUI(noise_map1);
	noise_param_blk.InvalidateUI(noise_map2);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("color1"), mr_reftarg, this, mr_prop, _T("color2"), mr_reftarg, this);
	macroRecorder->FunctionCall(_T("swap"), 2, 0, mr_prop, _T("map1"), mr_reftarg, this, mr_prop, _T("map2"), mr_reftarg, this);
	}

void Noise::SetColor(int i, Color c, TimeValue t) {
    col[i] = c;
	pblock->SetValue( i==0?noise_color1:noise_color2, t, c);
	}

void Noise::SetSize(float f, TimeValue t) { 
	size = f; 
	pblock->SetValue( noise_size, t, f);
	}

void Noise::SetPhase(float f, TimeValue t) { 
	phase = f; 
	pblock->SetValue( noise_phase, t, f);
	}

void Noise::SetLevels(float f, TimeValue t) { 
	levels = f; 
	pblock->SetValue( noise_levels, t, f);
	}

RefTargetHandle Noise::GetReference(int i) {
	switch(i) {
		case XYZGEN_REF: return xyzGen;
		case PBLOCK_REF: return pblock;
		case TEXOUT_REF: return texout;
		default:return subTex[i-2];
		}
	}

void Noise::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case XYZGEN_REF: xyzGen = (XYZGen *)rtarg; break;
		case PBLOCK_REF: pblock = (IParamBlock2 *)rtarg; break;
		case TEXOUT_REF: texout = (TextureOutput *)rtarg; break;
		default: subTex[i-2] = (Texmap *)rtarg; break;
		}
	}

void Noise::SetSubTexmap(int i, Texmap *m) {
	ReplaceReference(i+2,m);

	if (i==0)
		{
		noise_param_blk.InvalidateUI(noise_map1);
		ivalid.SetEmpty();
		cacheValid.SetEmpty();
		}	
	else if (i==1)
		{
		noise_param_blk.InvalidateUI(noise_map2);
		ivalid.SetEmpty();
		cacheValid.SetEmpty();
		}	

//	if (paramDlg)
//		paramDlg->UpdateSubTexNames();
	}

TSTR Noise::GetSubTexmapSlotName(int i) {
	switch(i) {
		case 0:  return TSTR(GetString(IDS_DS_COLOR1)); 
		case 1:  return TSTR(GetString(IDS_DS_COLOR2)); 
		default: return TSTR(_T(""));
		}
	}
	 
Animatable* Noise::SubAnim(int i) {
	switch (i) {
		case XYZGEN_REF: return xyzGen;
		case PBLOCK_REF: return pblock;
		case TEXOUT_REF: return texout;
		default: return subTex[i-2]; 
		}
	}

TSTR Noise::SubAnimName(int i) {
	switch (i) {
		case XYZGEN_REF: return TSTR(GetString(IDS_DS_COORDINATES));		
		case PBLOCK_REF: return TSTR(GetString(IDS_DS_PARAMETERS));		
		case TEXOUT_REF: return TSTR(GetString(IDS_DS_OUTPUT));
		default: return GetSubTexmapTVName(i-2);
		}
	}

static int nameID[] = {IDS_DS_NOISESIZE, IDS_DS_COLOR1, IDS_DS_COLOR2, IDS_DS_PHASE, IDS_DS_LEVELS,
						IDS_RB_LOWTHRESH, IDS_RB_HIGHTHRESH};

RefResult Noise::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message ) {
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			cacheValid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
//				if (hTarget != xyzGen && hTarget != texout ) 
				noise_param_blk.InvalidateUI(changing_param);
				// notify our dependents that we've changed
				// NotifyChanged();  //DS this is redundant
#ifdef SHOW_3DMAPS_WITH_2D
				if (changing_param != -1)
					DiscardTexHandle();
#endif
				}
#ifdef SHOW_3DMAPS_WITH_2D
			else if (hTarget == xyzGen) 
				{
				DiscardTexHandle();
				}
#endif

			break;
		}
	return(REF_SUCCEED);
	}


#define MTL_HDR_CHUNK 	0x4000
#define DO_TURB_CHUNK 	0x1000
#define NOISETYPE_CHUNK	0x1010
#define NOISEVERS1_CHUNK 0x2001
#define MAPOFF_CHUNK 0x3000

IOResult Noise::Save(ISave *isave) { 
	IOResult res;
//	ULONG nb;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();
	return IO_OK;
	}	

int Noise::FixLevel0() {
	// old files had level==0: this is to fix them
	if (pblock) {
		float l;
		Interval ivalid;
		pblock->GetValue( noise_levels, 0,  l, ivalid );
		if (l<1.0f) {
			pblock->SetValue( noise_levels, 0,  1.0f );
			return 1;
			}
		}
	return 0;
	}

class NoisePostLoad : public PostLoadCallback {
	public:
		Noise *n;
		NoisePostLoad(Noise *ns) {n = ns;}
		void proc(ILoad *iload) {  
			if (n->FixLevel0())
				iload->SetObsolete();

			if (n->loadOnChecks)
				{
				macroRecorder->Disable();  
					n->pblock->SetValue( noise_map1_on, 0, n->mapOn[0]);
					n->pblock->SetValue( noise_map2_on, 0, n->mapOn[1]);
					n->pblock->SetValue( noise_type, 0, n->noiseType);
				macroRecorder->Enable(); 
				}
			delete this; 


			} 
	};

	
IOResult Noise::Load(ILoad *iload) { 
	ULONG nb;
	IOResult res;
	int id;
	vers = 0;
//	iload->RegisterPostLoadCallback(new ParamBlockPLCB(oldVersions, NUM_OLDVERSIONS, &curVersion, this, 1 /*ref # */ ));
	loadOnChecks = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case DO_TURB_CHUNK:
				noiseType = NOISE_TURB;
				loadOnChecks = TRUE;
				break;
			case NOISETYPE_CHUNK:
				res = iload->Read(&noiseType,sizeof(noiseType),&nb);
				loadOnChecks = TRUE;
				break;
			case NOISEVERS1_CHUNK:
				vers = 1;
				break;
			case MAPOFF_CHUNK+0:
			case MAPOFF_CHUNK+1:
				mapOn[id-MAPOFF_CHUNK] = 0; 
				loadOnChecks = TRUE;
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	// JBW: register old version ParamBlock to ParamBlock2 converter
	if (loadOnChecks)
	{
		ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &noise_param_blk, this, PBLOCK_REF);
		iload->RegisterPostLoadCallback(plcb);
	}

	iload->RegisterPostLoadCallback(new NoisePostLoad(this));



	return IO_OK;
	}

#define NOISE01(p) ((1.0f+noise4(p,phase))*.5f)

float Noise::Turb(Point3 p, float lev) {
	float sum = 0.0f;
	float l,f = 1.0f;
	float ml = levels;
	for (l = lev; l >= 1.0f; l-=1.0f, ml-=1.0f) {
		sum += (float)fabs(noise4(p*f,phase))/f;
		f *= 2.0f;
		}
	if (l>0.0f)
		sum += l*(float)fabs(noise4(p*f,phase))/f;

#ifdef DOAA
	if (filter&&(ml>l)) {
		float r = 0;
	    if (ml<1.0f) {
			r += (ml-l)/f;
			}
		else  {
			r  += (1.0f-l)/f;
			ml -= 1.0f;
			f  *= 2.0f;
			for (l = ml; l >=1.0f; l-=1.0f) {
				r += 1.0f/f;
				f *= 2.0f;
				}
			if (l>0.0f)
				r+= l/f;
			sum += r*avgAbsNs;
			}
		}
#endif
	return sum;
	}

float SmoothThresh(float x, float a, float b, float d) {
	float al = a-d;  
	float ah = a+d;	
	float bl = b-d;
	float bh = b+d;
	if (x<al) return 0.0f;
	if (x>bh) return 1.0f;
	if (x<ah) {
		float u = (x-al)/(ah-al);
		u = u*u*(3-2*u);  // smooth cubic curve 
		return u*(x-a)/(b-a);
		}
	if (x<bl){ 
		return (x-a)/(b-a);
		}
	else  {
		float u = (x-bl)/(bh-bl);
		u = u*u*(3-2*u);  // smooth cubic curve 
			return (1.0f-u)*(x-a)/(b-a) + u;
		}
	}

float Noise::NoiseFunction(Point3 p, float limitLev, float smWidth) {
	float res;
	float lev = limitLev;
	if (limitLev<(1.0f-BLENDBAND)) return avgValue;

	if (lev<1.0f) lev = 1.0f;
	switch (noiseType) {
		case NOISE_TURB:
			res = Turb(p,lev);
			break;
	
    	case NOISE_REGULAR:
			res = NOISE01(p);
			break;

		case NOISE_FRACTAL:
			{
			float sum = 0.0f;
			float l, f = 1.0f;
			for (l = lev; l >= 1.0f; l-=1.0f) {				
				sum += noise4(p*f,phase)/f;
				f *= 2.0f;
				}
			if (l>0.0f)				
				sum += l*noise4(p*f,phase)/f;				
			res = 0.5f*(sum+1.0f);
			}
			break;
		}
	if (low<high) {
		//res = threshold(res,low,high);
		res = filter? SmoothThresh(res,low,high, smWidth): threshold(res,low,high);
		}
	if (res<0.0f) res = 0.0f;
	else if (res>1.0f) res = 1.0f;

	if (filter) {
		if (limitLev<1.0f) {
			float u = (limitLev+BLENDBAND-1.0f)/BLENDBAND;
			res = u*res + (1.0f-u)*avgValue;
			}
		}
	return res;   
	}



static AColor black(0.0f,0.0f,0.0f,0.0f);

inline float logb2(float x) { return float(log(x)/.6931478); }

float  Noise::LimitLevel(Point3 dp, float &smw ) {
#ifdef DOAA
#define epsilon	0.00001f

	if (filter) {
		float m = (float(fabs(dp.x) + fabs(dp.y) + fabs(dp.z))/3.0f)/size;
		if ( m < epsilon ) m = epsilon;
		float l = logb2(1/m);
		float smWidth = m*.2f;
		if (smWidth>.4f) smWidth = .4f;
		smw = smWidth;
		return  (levels<l)?levels:l;	  
		}
	else 
#endif 
		{
		smw = 0.0f;
		return levels;
		}
	}

RGBA Noise::EvalColor(ShadeContext& sc) {
	Point3 p,dp;
	if (!sc.doMaps) return black;

	AColor c;
	if (sc.GetCache(this,c)) 
		return c; 

	if (gbufID) sc.SetGBufferID(gbufID);

	//IPoint2 ps = sc.ScreenCoord();
  	UpdateCache(sc.CurTime());  // DS 10/3/00
	xyzGen->GetXYZ(sc,p,dp);
	p /= size;	   
	filter = sc.filterMaps;
	
	float smw;
	float limlev = LimitLevel(dp,smw);
    float d = NoiseFunction(p,limlev,smw);

	RGBA c0 = mapOn[0]&&subTex[0] ? subTex[0]->EvalColor(sc): col[0];
	RGBA c1 = mapOn[1]&&subTex[1] ? subTex[1]->EvalColor(sc): col[1];
	c = texout->Filter((1.0f-d)*c0 + d*c1);
	
	sc.PutCache(this,c); 
	return c;
	}

float Noise::EvalMono(ShadeContext& sc) {
	Point3 p,dp;
	if (!sc.doMaps) 	return 0.0f;

	float f;
	if (sc.GetCache(this,f)) 
		return f; 

	if (gbufID) sc.SetGBufferID(gbufID);
  	UpdateCache(sc.CurTime());  // DS 10/3/00
	xyzGen->GetXYZ(sc,p,dp);
	p /= size;
	filter = sc.filterMaps;
	float smw;
	float limlev = LimitLevel(dp, smw);
    float d = NoiseFunction(p,limlev,smw);
	float c0 = mapOn[0]&&subTex[0] ? subTex[0]->EvalMono(sc): Intens(col[0]);
	float c1 = mapOn[1]&&subTex[1] ? subTex[1]->EvalMono(sc): Intens(col[1]);
	f = texout->Filter((1.0f-d)*c0 + d*c1);
	sc.PutCache(this,f); 
	return f;
	}


Point3 Noise::EvalNormalPerturb(ShadeContext& sc) {
	Point3 p,dp;
	if (!sc.doMaps) return Point3(0,0,0);
	if (gbufID) sc.SetGBufferID(gbufID);
  	UpdateCache(sc.CurTime());  // DS 10/3/00
	xyzGen->GetXYZ(sc,p,dp);
	p /= size;
	filter = sc.filterMaps;
	float smw;
	float limlev = LimitLevel(dp,smw);
	float del,d;
	d = NoiseFunction(p,limlev,smw);
	//del = (dp.x+dp.y+dp.z)/(size*3.0f);
	del = .1f;
	Point3 np;					  
	Point3 M[3];
	xyzGen->GetBumpDP(sc,M);

	np.x = (NoiseFunction(p+del*M[0],limlev,smw) - d)/del;
	np.y = (NoiseFunction(p+del*M[1],limlev,smw) - d)/del;
	np.z = (NoiseFunction(p+del*M[2],limlev,smw) - d)/del;

	np = sc.VectorFromNoScale(np, REF_OBJECT);

	Texmap *sub0 = mapOn[0]?subTex[0]:NULL;
	Texmap *sub1 = mapOn[1]?subTex[1]:NULL;
	if (sub0||sub1) {
		// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
		float a,b;
		Point3 da,db;
		if (sub0) { 	a = sub0->EvalMono(sc); 	da = sub0->EvalNormalPerturb(sc);		}
		else {	 a = Intens(col[0]);	 da = Point3(0.0f,0.0f,0.0f);		 }
		if (sub1) { 	b = sub1->EvalMono(sc); 	db = sub1->EvalNormalPerturb(sc);	}
		else {	 b = Intens(col[1]);	 db= Point3(0.0f,0.0f,0.0f);		 }
		np = (b-a)*np + d*(db-da) + da;
		}
	else 
		np *= Intens(col[1])-Intens(col[0]);
	return texout->Filter(np);
	}
