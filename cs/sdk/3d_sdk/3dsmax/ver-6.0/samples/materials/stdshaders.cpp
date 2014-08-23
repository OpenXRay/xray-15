//////////////////////////////////////////////////////////////////////////////
//
//		Shader plug-ins, implementation
//
//		Created: 8/18/98 Kells Elmquist
//
#include "mtlhdr.h"
#include "mtlres.h"
#include "mtlresOverride.h"
#include "gport.h"
#include "shaders.h"
#include "macrorec.h"
#include "iColorMan.h"
#include "buildver.h"
#include "expmtlControl.h"
#include "toneop.h"

// paramblock2 block and parameter IDs for the standard shaders
// NB these are duplicated in mtls/stdmtl2.h ...
enum { shdr_params, };
// shdr_params param IDs
enum 
{ 
	shdr_ambient, shdr_diffuse, shdr_specular,
	shdr_ad_texlock, shdr_ad_lock, shdr_ds_lock, 
	shdr_use_self_illum_color, shdr_self_illum_amnt, shdr_self_illum_color, 
	shdr_spec_lvl, shdr_glossiness, shdr_soften,
};


// including self-illum color but not filter
 #define NCOLBOX 4
static int colID[NCOLBOX] = { IDC_STD_COLOR1, IDC_STD_COLOR2, IDC_STD_COLOR3, IDC_SI_COLOR };
static int colParamID[NCOLBOX] = { shdr_ambient, shdr_diffuse, shdr_specular, shdr_self_illum_color };
#define N_SI_CLR		3
#define N_AMB_CLR		0

#define PB_AMBIENT_CLR		0
#define PB_DIFFUSE_CLR		1
#define PB_SPECULAR_CLR		2
#define PB_SELFILLUM_CLR	3
#define PB_SELFILLUM		4
#define PB_GLOSSINESS 		5
#define PB_SPEC_LEV			6
#define PB_SOFTEN_LEV		7

static int PB_ID[NCOLBOX] = 
	{ PB_AMBIENT_CLR, PB_DIFFUSE_CLR, PB_SPECULAR_CLR, PB_SELFILLUM_CLR };


static Class_ID StdShaderClassID(STDSHADERS_CLASS_ID,0);
static Class_ID ConstantClassID(STDSHADERS_CLASS_ID+1,0);
static Class_ID BlinnClassID(BLINNClassID,0);
static Class_ID OldBlinnClassID(DMTL_CLASS_ID+34+3,0);
#ifndef USE_LIMITED_STDMTL 
static Class_ID PhongClassID(PHONGClassID,0);
static Class_ID MetalClassID(METALClassID,0);
#endif

static Class_ID StdShaderParamDlgClassID(STDSHADERS_CLASS_ID,0);

extern HINSTANCE hInstance;

static LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

//////////////////////////////////////////////////////////
static HIMAGELIST hLockButtons = NULL;

// mjm - begin - 5.28.99
class StdShaderResourceDelete
{
public:
	StdShaderResourceDelete() {}
	~StdShaderResourceDelete() { if (hLockButtons) ImageList_Destroy(hLockButtons); }
};

static StdShaderResourceDelete theResourceDelete;
// mjm - end

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
 
static void LoadStdShaderResources()
	{
	static BOOL loaded=FALSE;
	if (loaded) return;
	loaded = TRUE;	
	HBITMAP hBitmap, hMask;

	hLockButtons = ImageList_Create(16, 15, TRUE, 2, 0);
	hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DMTL_BUTTONS));
	hMask   = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_DMTL_MASKBUTTONS));
	ImageList_Add(hLockButtons,hBitmap,hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
	}

static TSTR rollupName( Shader* pShader )
{
	Class_ID id = pShader->ClassID();
	if ( id == BlinnClassID )
		return GetString( IDS_KE_BASIC_BLINN );
#ifndef USE_LIMITED_STDMTL 
	else if ( id == PhongClassID )
 		return GetString( IDS_KE_BASIC_PHONG );
	else if ( id == MetalClassID )
 		return GetString( IDS_KE_BASIC_METAL );
#endif 
	else return _T("");
}


inline float PcToFrac(int pc) { return (float)pc/100.0f; }

inline int FracToPc(float f) {
	if (f<0.0) return (int)(100.0f*f - .5f);
	else return (int) (100.0f*f + .5f);
}

// Quadratic
inline float Soften(float r) {
	return r*(2.0f-r);
}

// all the old shaders add the highlights on
void CombineComponentsAdd( IllumParams& ip )
{
	ip.finalC = ip.finalAttenuation * (ip.ambIllumOut + ip.diffIllumOut  + ip.selfIllumOut)
			+ ip.specIllumOut + ip.reflIllumOut + ip.transIllumOut; 
}

 
inline float Abs( float a ) { return (a < 0.0f) ? -a : a; }
inline float LBound( float x, float min = 0.0f ){ return x < min ? min : x; }
inline float UBound( float x, float max = 1.0f ){ return x > max ? max : x; }

///////////////////////////////////////////////////////////////////////////////
//
//	Generic standard shader. supports phong blinn, metal & constant
//
class StdShaderDlg;


class StdShaderImp : public Shader, public ExposureMaterialControl {
friend class StdShaderCB;
friend class StdShaderDlg;
protected:
	IParamBlock2* pblock;   // ref 0
	Interval ivalid;
	TimeValue	curTime;

	StdShaderDlg* paramDlg;

	BOOL selfIllumClrOn;
	BOOL lockDS;
	BOOL lockAD;
	BOOL lockADTex;

	Color ambient;
	Color diffuse;
	Color specular;
	Color selfIllumClr;
	float selfIllum;	
	float softThresh;
	float glossiness;
	float specularLevel;

public:
	StdShaderImp();
	void ConvertParamBlk( ParamBlockDescID *descOld, int oldCount, IParamBlock *oldPB );
    void CopyStdParams( Shader* pFrom );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int){ paramDlg = (StdShaderDlg*)newDlg; }

	virtual void  Illum(ShadeContext &sc, IllumParams &ip){};
	virtual void  AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol){};
	void CombineComponents( ShadeContext &sc, IllumParams& ip ){ CombineComponentsAdd(ip); }
    virtual ULONG SupportStdParams(){ return STD_BASIC+STD_EXTRA; }

	// texture maps
	virtual long  nTexChannelsSupported();
	virtual TSTR  GetTexChannelName( long nTex );
	virtual TSTR  GetTexChannelInternalName( long nTex );
	virtual long  ChannelType( long nTex );
	// map StdMat Channel ID's to the channel number
	virtual long StdIDToChannel( long stdID );

	BOOL KeyAtTime(int id,TimeValue t);
	void DeleteThis(){ delete this; }		
	ULONG GetRequirements( int subMtlNum ){ return isNoExposure() | MTLREQ_PHONG; }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	void Update(TimeValue t, Interval& valid);
	void Reset();
	void NotifyChanged();

	SClass_ID SuperClassID() { return SHADER_CLASS_ID; }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);
	int SubNumToRefNum(int subNum) { return subNum;	}
 	// JBW: add direct ParamBlock access
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; }
	virtual ClassDesc2* GetCD() { return NULL; } // must be overridden

	// From ref
 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	RefTargetHandle Clone( RemapDir &remap=NoRemap(), StdShaderImp* mnew=NULL );
	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
	                            PartID& partID, RefMessage message );

	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// cache for mapping of params, mtl fills in ip
	void GetIllumParams( ShadeContext &sc, IllumParams& ip );

	// Shader specific section
	// these shd always be overridden
	virtual Class_ID ClassID(); 
	virtual TSTR GetName() ;
	virtual float EvalHiliteCurve(float x){ return 0.0f; }

	void SetLockDS(BOOL lock){ lockDS = lock; pblock->SetValue( shdr_ds_lock, 0, lock); }
	BOOL GetLockDS(){ return lockDS; }
	void SetLockAD(BOOL lock){ lockAD = lock; pblock->SetValue( shdr_ad_lock, 0, lock); }
	BOOL GetLockAD(){ return lockAD; }
	void SetLockADTex(BOOL lock){ lockADTex = lock; pblock->SetValue( shdr_ad_texlock, 0, lock); }
	BOOL GetLockADTex(){ return lockADTex; }

	void SetSelfIllum(float v, TimeValue t);		
	void SetSelfIllumClrOn( BOOL on ){ selfIllumClrOn = on; pblock->SetValue( shdr_use_self_illum_color, 0, on); };
	BOOL IsSelfIllumClrOn(){ return selfIllumClrOn; };
	void SetSelfIllumClr(Color c, TimeValue t);		

	void SetAmbientClr(Color c, TimeValue t);		
	void SetDiffuseClr(Color c, TimeValue t);		
	void SetSpecularClr(Color c, TimeValue t);
	void SetGlossiness(float v, TimeValue t);		
	void SetSpecularLevel(float v, TimeValue t);		
	void SetSoftenLevel(float v, TimeValue t);
		
	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return selfIllumClrOn; };
	Color GetAmbientClr(int mtlNum, BOOL backFace){ return ambient;}		
    Color GetDiffuseClr(int mtlNum, BOOL backFace){ return diffuse;}		
	Color GetSpecularClr(int mtlNum, BOOL backFace){ return specular; };
	Color GetSelfIllumClr(int mtlNum, BOOL backFace){ return selfIllumClr; };
	float GetSelfIllum(int mtlNum, BOOL backFace){ return selfIllum; };
	float GetGlossiness(int mtlNum, BOOL backFace){ return glossiness; };	
	float GetSpecularLevel(int mtlNum, BOOL backFace){ return specularLevel; };
	float GetSoftenLevel(int mtlNum, BOOL backFace){ return softThresh; };

	Color GetAmbientClr(TimeValue t);	
	Color GetDiffuseClr(TimeValue t);
	Color GetSpecularClr(TimeValue t);
	float GetGlossiness( TimeValue t);	
	float GetSpecularLevel(TimeValue t);
	float GetSoftenLevel(TimeValue t);
	float GetSelfIllum(TimeValue t);	
	Color GetSelfIllumClr(TimeValue t);	

	using Shader::GetInterface;
	FPInterfaceDesc* GetDesc() { return NULL; }
};

///////////////////////////////////////////////////////////////////
//
//	generic standard shader dlg panel
//
#define NCOLBOX 4

class StdShaderDlg : public ShaderParamDlg {
public:
	StdShaderImp*	pShader;
	StdMat2*	pMtl;
	HPALETTE	hOldPal;
	HWND		hwmEdit;	 // window handle of the materials editor dialog
	IMtlParams*	pMtlPar;
	HWND		hwHilite;    // the hilite window
	HWND		hRollup; // Rollup panel
	TimeValue	curTime;
	DWORD		curRGB;
	BOOL		valid;
	BOOL		isActive;

	IColorSwatch *cs[NCOLBOX];
	ISpinnerControl *softSpin;
	ISpinnerControl *shSpin, *ssSpin, *siSpin, *trSpin;
	ICustButton** texMBut;
	TexDADMgr dadMgr;
	
	StdShaderDlg(HWND hwMtlEdit, IMtlParams *pParams, int nButtons); 
	~StdShaderDlg(); 

	// required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw);

	// Methods
	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(); //{ return StdShaderParamDlgClassID; }
	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader ){
		if (pShader) 
			pShader->SetParamDlg(NULL,0);   // DS 3/11/99
		pShader = (StdShaderImp*)theShader; 
		if( pShader ) 
			pShader->SetParamDlg(this,0); 
		pMtl = theMtl;
	}

	ReferenceTarget* GetThing(){ return (ReferenceTarget*)pMtl; }
	Shader* GetShader(){ return pShader; }
	void SetTime(TimeValue t) {
		//DS 2/26/99: added interval test to prevent redrawing when not necessary
		curTime = t; 
		if (!pShader->ivalid.InInterval(t)) {
			Interval v;
			pShader->Update(t,v);
			LoadDialog(TRUE); 
		} else
			UpdateOpacity();  // always update opacity since it's not in validity computations
	}		
	BOOL KeyAtCurTime(int id) { return pShader->KeyAtTime(id,curTime); } 
	void DeleteThis() { delete this; }
	void ActivateDlg( BOOL dlgOn ){ isActive = dlgOn; }
	HWND GetHWnd(){ return hRollup; }

	// ***** required code
	void UpdateOpacity();
	// ***** 

	void LoadDialog(BOOL draw);
	void ReloadDialog();
	void UpdateDialog( ParamID paramId ){ ReloadDialog(); }
	void NotifyChanged();

	// the rest is all specific to standard shaders
	void UpdateMtlDisplay();
    void UpdateHilite( );
	void UpdateColSwatches();
	void UpdateLockADTex(BOOL passOn);
	void UpdateMapButtons();
    void DrawHilite( HDC hdc, Rect& rect );
	void DisableNonMetalControls();

	void SetLockDS(BOOL lock);
	void SetLockAD(BOOL lock);
	void SetLockADTex(BOOL lock);

	void   SetMtlColor( int i, Color c );
	Color  GetMtlColor( int i );
	TCHAR* GetColorName( int i );

	void SelectEditColor(int i) ;
	void SetCurRGB(DWORD rgb) { curRGB = rgb; }

};

/////////////////////////////////////////////////////////////////////
//
//	Shader Channel Descriptions 
//
#define N_STD_SHADER_CHANNELS 8

#define NMBUTS 7
#define N_SI_BUT 5
#define N_TR_BUT 6

static int texMButtonsIDC[NMBUTS] = {
	IDC_MAPON_AM,	IDC_MAPON_DI,	IDC_MAPON_SP,	IDC_MAPON_SH,
	IDC_MAPON_SS,	IDC_MAPON_SI,	IDC_MAPON_TR, 
};
		
// This array gives the texture map number for given MButton number								
// static int texmapFromMBut[NMBUTS] = { 0, 1, 2, 7, 3, 4, 5, 6 };
// these ID_s are from stdmat.h
static int texmapFromMBut[NMBUTS] = { ID_AM, ID_DI, ID_SP, ID_SS, ID_SH, ID_SI, ID_OP };

// channel names
static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, IDS_DS_SHIN_STR, IDS_DS_SHININESS, 
	IDS_KE_SELFILLUM, IDS_DS_TRANS, IDS_DS_FILTER, 
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,	
#ifndef USE_LIMITED_STDMTL  // orb 01-21-2002 remove std maps
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,	
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,	
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE	
#endif
};	

// internal non-local parsable channel map names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("ambientMap"), _T("diffuseMap"),	_T("specularMap"), _T("specularLevelMap"), _T("glossinessMap"),  
	_T("selfIllumMap"), _T("opacityMap"), _T("filterMap"),
	_T(""), _T(""), _T(""), _T(""),
#ifndef USE_LIMITED_STDMTL  // orb 01-21-2002 remove std maps
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),	
	_T(""), _T(""), _T(""), _T("") 
#endif
};	

static int channelType[] = {
	CLR_CHANNEL, CLR_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, 
	CLR_CHANNEL, MONO_CHANNEL, CLR_CHANNEL, 
	UNSUPPORTED_CHANNEL, 	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
#ifndef USE_LIMITED_STDMTL  // orb 01-21-2002 remove std maps
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
#endif
};

// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { 0, 1, 2, 4, 3, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1 };

long StdShaderImp::StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

///////////////////////////////////////////////////////////////////////////////
//
//	StdShaders Paramter block
//
#define CURRENT_STDSHADER_VERSIOM 2
#define STDSHADER_NPARAMS 8
#define STDSHADER_PB_VERSION   1

//Current Param Block Descriptor
static ParamBlockDescID stdShaderPB[ STDSHADER_NPARAMS ] = {
	{ TYPE_RGBA,  NULL, TRUE, shdr_ambient },		// ambient
	{ TYPE_RGBA,  NULL, TRUE, shdr_diffuse },		// diffuse
	{ TYPE_RGBA,  NULL, TRUE, shdr_specular },		// specular
	{ TYPE_RGBA,  NULL, TRUE, shdr_self_illum_color }, // self-illum color
	{ TYPE_FLOAT, NULL, TRUE, shdr_self_illum_amnt },  // selfIllum
	{ TYPE_FLOAT, NULL, TRUE, shdr_glossiness },		// glossiness
	{ TYPE_FLOAT, NULL, TRUE, shdr_spec_lvl },		// specularLevel
	{ TYPE_FLOAT, NULL, TRUE, shdr_soften },			// soften
}; 

#define NUMOLDVER 1

static ParamVersionDesc oldVersions[NUMOLDVER] = {
	ParamVersionDesc(stdShaderPB,8, 0),
};

static ParamVersionDesc curVersion(stdShaderPB,STDSHADER_NPARAMS,STDSHADER_PB_VERSION);

#define STD_NMAX_TEXMAPS	12

////////////////////////////////////////////////////////////////////////////////////////
long  StdShaderImp::nTexChannelsSupported(){ return N_STD_SHADER_CHANNELS; }
TSTR  StdShaderImp::GetTexChannelName( long nTex ){ return GetString( texNameIDS[ nTex ] ); }
TSTR  StdShaderImp::GetTexChannelInternalName( long nTex ){ return texInternalNames[ nTex ]; }
long  StdShaderImp::ChannelType( long nTex ){ return channelType[ nTex ]; }

Class_ID StdShaderImp::ClassID(){ return StdShaderClassID; }

Color StdShaderImp::GetAmbientClr(TimeValue t)  { return pblock->GetColor(shdr_ambient,t); }		
Color StdShaderImp::GetDiffuseClr(TimeValue t)  { return pblock->GetColor(shdr_diffuse,t); }		
Color StdShaderImp::GetSpecularClr(TimeValue t) { return pblock->GetColor(shdr_specular,t);	}
float StdShaderImp::GetGlossiness( TimeValue t) {return pblock->GetFloat(shdr_glossiness,t);  }		
float StdShaderImp::GetSpecularLevel(TimeValue t)  { return  pblock->GetFloat(shdr_spec_lvl,t); }
float StdShaderImp::GetSoftenLevel(TimeValue t){ return  pblock->GetFloat(shdr_soften,t); }
float StdShaderImp::GetSelfIllum(TimeValue t){ return  pblock->GetFloat(shdr_self_illum_amnt,t); }		
Color StdShaderImp::GetSelfIllumClr(TimeValue t){ return  pblock->GetColor(shdr_self_illum_color,t); }		

//StdShaderImp::StdShaderImp(ClassDesc2* pParentCD) 
StdShaderImp::StdShaderImp() 
{ 
//	lockDS = lockAD = lockADTex = selfIllumClrOn = 0;
	lockDS = selfIllumClrOn = 0;
	lockAD = lockADTex = TRUE;
	pblock = NULL; 
	paramDlg = NULL; 
	Color black(0,0,0);
	ambient = diffuse = specular = selfIllumClr = black;
	glossiness = specularLevel = selfIllum = softThresh = 0.0f;
	ivalid.SetEmpty(); 
}

void StdShaderImp::CopyStdParams( Shader* pFrom )
{
	macroRecorder->Disable();  // don't want to see this parameter copying in macrorecorder
		SetLockDS( pFrom->GetLockDS() );
		SetLockAD( pFrom->GetLockAD() );
		SetLockADTex( pFrom->GetLockADTex() );
		SetSelfIllumClrOn( pFrom->IsSelfIllumClrOn() );

		SetAmbientClr( pFrom->GetAmbientClr(0,0), curTime );
		SetDiffuseClr( pFrom->GetDiffuseClr(0,0), curTime );
		SetSpecularClr( pFrom->GetSpecularClr(0,0), curTime );
		SetSelfIllumClr( pFrom->GetSelfIllumClr(0,0), curTime );

		SetSelfIllum( pFrom->GetSelfIllum(0,0), curTime );
		SetSpecularLevel( pFrom->GetSpecularLevel(0,0), curTime );
		SetGlossiness( pFrom->GetGlossiness(0,0), curTime );
		SetSoftenLevel( pFrom->GetSoftenLevel(0,0), curTime );
	macroRecorder->Enable();

	ivalid.SetEmpty();	
}

void StdShaderImp::ConvertParamBlk( ParamBlockDescID *oldPBDesc, int oldCount, IParamBlock *oldPB )
{
	// old Standrard material loaded, transfer any shader-related parameters from old Mtl PB to shader PB2
	UpdateParameterBlock2(oldPBDesc, oldCount, oldPB, pblock->GetDesc(), pblock);
}


BOOL StdShaderImp::KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime((ParamID)id,t); }
TSTR StdShaderImp::GetName() { return GetString( IDS_KE_STDSHADER ); }

#define LIMIT0_1(x) if (x < 0.0f) x = 0.0f; else if (x > 1.0f) x = 1.0f;
#define LIMITMINMAX(x, min, max) if (x < min) x = min; else if (x > max) x = max;

static Color LimitColor(Color c) {
	LIMIT0_1(c.r);
	LIMIT0_1(c.g);
	LIMIT0_1(c.b);
	return c;
}

RefTargetHandle StdShaderImp::Clone( RemapDir &remap, StdShaderImp* mnew )
{
	if (mnew == NULL ){
		assert( 0 );
		mnew = new StdShaderImp();
	}

	mnew->ExposureMaterialControl::operator=(*this);

	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	

	mnew->ambient = ambient;
	mnew->diffuse = diffuse;
	mnew->specular = specular;
	mnew->glossiness = glossiness;
	mnew->specularLevel = specularLevel;
	mnew->softThresh = softThresh;
	mnew->selfIllum = selfIllum;
	mnew->selfIllumClr = selfIllumClr;
	mnew->selfIllumClrOn = selfIllumClrOn;
	mnew->lockDS = lockDS;
	mnew->lockAD = lockAD;
	mnew->lockADTex = lockADTex;
	
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

void StdShaderImp::GetIllumParams( ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
//	ip.shFlags = selfIllumClrOn? SELFILLUM_CLR_ON : 0;
	ip.channels[ID_AM] = lockAD? diffuse : ambient;
	ip.channels[ID_DI] = diffuse;
	ip.channels[ID_SP] = lockDS? diffuse : specular;
	ip.channels[StdIDToChannel(ID_SH)].r = glossiness;
	ip.channels[StdIDToChannel(ID_SS)].r = specularLevel;
	if( selfIllumClrOn )
		ip.channels[ID_SI] = selfIllumClr;
	else
		ip.channels[ID_SI].r = ip.channels[ID_SI].g = ip.channels[ID_SI].b = selfIllum;
}

static BOOL inUpdate = FALSE;

void StdShaderImp::Update(TimeValue t, Interval &valid) {
	Point3 p,p2;
	if( inUpdate )
		return;
	inUpdate = TRUE;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

		// russom - 08/16/01
		// moved from bottom of method to make sure the current values are always used.
		// get the non-animatables in case changed from scripter or other pblock accessors
		pblock->GetValue(shdr_ds_lock, t, lockDS, ivalid);
		pblock->GetValue(shdr_ad_lock, t, lockAD, ivalid);
		pblock->GetValue(shdr_ad_texlock, t, lockADTex, ivalid);
		pblock->GetValue(shdr_use_self_illum_color, t, selfIllumClrOn, ivalid);

		// DS 10/12/00: added tests to avoid calling SetValue when not necessary, as this causing
		// unnecessary mtl editor re-renders.
		pblock->GetValue( shdr_diffuse, t, p, ivalid );
		diffuse= LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( shdr_ambient, t, p2, ivalid );
		if( lockAD && !(p==p2)){
			//pblock->SetValue( shdr_ambient, t, diffuse);		  // DS 11/6/00 -- removed this
			ambient = diffuse;
		} else {
			//pblock->GetValue( shdr_ambient, t, p, ivalid );   
			ambient = LimitColor(Color(p2.x,p2.y,p2.z));
		}

		pblock->GetValue( shdr_specular, t, p2, ivalid );
		if( lockDS && !(p==p2)){
			//pblock->SetValue( shdr_specular, t, diffuse);     // DS 11/6/00 -- removed this
			specular = diffuse;
		} else {
			//pblock->GetValue( shdr_specular, t, p, ivalid );
			specular = LimitColor(Color(p2.x,p2.y,p2.z));
		}
		pblock->GetValue( shdr_glossiness, t, glossiness, ivalid );
		LIMIT0_1(glossiness);
		pblock->GetValue( shdr_spec_lvl, t, specularLevel, ivalid );
		LIMITMINMAX(specularLevel,0.0f,9.99f);
		pblock->GetValue( shdr_soften, t, softThresh, ivalid); 
		LIMIT0_1(softThresh);

		pblock->GetValue( shdr_self_illum_amnt, t, selfIllum, ivalid );
		LIMIT0_1(selfIllum);
		pblock->GetValue( shdr_self_illum_color, t, p, ivalid );
		selfIllumClr = LimitColor(Color(p.x,p.y,p.z));

		curTime = t;
	}
	valid &= ivalid;
	inUpdate = FALSE;
}

void StdShaderImp::Reset()
{
	if ( pblock==NULL )
		GetCD()->MakeAutoParamBlocks(this);	// make and intialize paramblock2
	
	ivalid.SetEmpty();
	macroRecorder->Disable();  // don't want to see this parameter reset in macrorecorder
		SetSoftenLevel(0.1f,0);
		SetAmbientClr(Color(0.588f,0.588f,0.588f),0);
		SetDiffuseClr(Color(0.588f,0.588f,0.588f),0);
		SetSpecularClr(Color(0.9f,0.9f,0.9f),0);
		SetGlossiness(.10f,0);   // change from .25, 11/6/00
		SetSpecularLevel(.0f,0);   

		SetSelfIllum(.0f,0);
		SetSelfIllumClr( Color(.0f, .0f, .0f), 0 );
		SetSelfIllumClrOn( FALSE );
		SetLockADTex( TRUE );
		SetLockAD( TRUE ); // DS 10/26/00: changed to TRUE
		SetLockDS( FALSE );
	macroRecorder->Enable(); 
}

void StdShaderImp::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

RefResult StdShaderImp::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
									  PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget==pblock)
			{
				// update UI if paramblock changed, possibly from scripter
				ParamID changingParam = pblock->LastNotifyParamID();
				// reload the dialog if present
				if (paramDlg)
					paramDlg->UpdateDialog( changingParam );
			}
			break;
	}
	return(REF_SUCCEED);
}


TSTR StdShaderImp::SubAnimName(int i) { 
	return TSTR(GetString( IDS_DS_PARAMETERS ));
}		

Animatable* StdShaderImp::SubAnim(int i) {
	switch(i) {
		case 0: return pblock; 
		default: assert(0); return NULL;
	}
}

RefTargetHandle StdShaderImp::GetReference(int i) {
	switch(i) {
		case 0: return pblock;
		default: assert(0);	 return NULL;
	}
}

void StdShaderImp::SetReference(int i, RefTargetHandle rtarg) {
	switch(i) {
		case 0:	pblock = (IParamBlock2*)rtarg; return;
		default: assert(0);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
#define SHADER_HDR_CHUNK 0x4000
#define SHADER_SELFILLUM_CLR_ON_CHUNK 0x5000
#define SHADER_LOCKDS_ON_CHUNK 0x5001
#define SHADER_LOCKAD_ON_CHUNK 0x5002
#define SHADER_LOCKADTEX_ON_CHUNK 0x5003
#define SHADER_MAPSON_CHUNK 0x5004
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020

#define SHADER_VERSION  2 

// IO
IOResult StdShaderImp::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = SHADER_VERSION;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();

	return IO_OK;
}		

class StdShaderCB: public PostLoadCallback {
	public:
		StdShaderImp *s;
		int loadVersion;
	    StdShaderCB(StdShaderImp *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			// convert old v1 ParamBlock to ParamBlock2
			ParamBlockDesc2* pbd = s->GetCD()->GetParamBlockDescByID(shdr_params);
			s->ReplaceReference(0,
				UpdateParameterBlock2(stdShaderPB, STDSHADER_NPARAMS, (IParamBlock*)s->pblock, pbd));

			// then set values that were previously stored outside the PB
			s->pblock->SetValue(shdr_use_self_illum_color, 0, s->selfIllumClrOn);
			s->pblock->SetValue(shdr_ds_lock, 0, s->lockDS);
			s->pblock->SetValue(shdr_ad_lock, 0, s->lockAD);
			s->pblock->SetValue(shdr_ad_texlock, 0, s->lockADTex);
		}
};


IOResult StdShaderImp::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;

	selfIllumClrOn = lockAD = lockADTex = lockDS = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SHADER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
			case SHADER_SELFILLUM_CLR_ON_CHUNK:
				selfIllumClrOn = TRUE;
				break;
			case SHADER_LOCKDS_ON_CHUNK:
				lockDS = TRUE;
				break;
			case SHADER_LOCKAD_ON_CHUNK:
				lockAD = TRUE;
				break;
			case SHADER_LOCKADTEX_ON_CHUNK:
				lockADTex = TRUE;
				break;
			case SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK:
				res = ExposureMaterialControl::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new StdShaderCB(this, version));
		iload->SetObsolete();
	}

	return IO_OK;

}



void StdShaderImp::SetAmbientClr(Color c, TimeValue t) 
{
	ambient =c;
	pblock->SetValue( shdr_ambient, t, Point3(c.r,c.g,c.b));
}
			
void StdShaderImp::SetDiffuseClr(Color c, TimeValue t) 
{
	Point3 p;
	Interval iv;
	pblock->SetValue( shdr_diffuse, t, Point3(c.r,c.g,c.b));

	pblock->GetValue( shdr_diffuse, t, p, iv );
	diffuse = LimitColor(Color(p.x,p.y,p.z));

}
			
void StdShaderImp::SetSpecularClr(Color c, TimeValue t) 
{
    specular = c;
	pblock->SetValue( shdr_specular, t, Point3(c.r,c.g,c.b));
}
			
			
void StdShaderImp::SetGlossiness(float v, TimeValue t) 
{
	glossiness = v;
	pblock->SetValue( shdr_glossiness, t, v);
}
			
void StdShaderImp::SetSpecularLevel(float v, TimeValue t) 
{
	specularLevel = v;
	pblock->SetValue( shdr_spec_lvl, t, v);
}

void StdShaderImp::SetSoftenLevel(float v, TimeValue t) 
{
	softThresh = v;
	pblock->SetValue( shdr_soften, t, v);
}

void StdShaderImp::SetSelfIllum(float v, TimeValue t) 
{
	selfIllum =v;
	pblock->SetValue( shdr_self_illum_amnt, t, v);
}

void StdShaderImp::SetSelfIllumClr(Color c, TimeValue t) 
{
	selfIllumClr = c;
	pblock->SetValue( shdr_self_illum_color, t, Point3(c.r,c.g,c.b) );
}
		
static INT_PTR CALLBACK  StdShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	StdShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (StdShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (StdShaderDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}



ShaderParamDlg* StdShaderImp::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) {
	Interval v;
	Update(imp->GetTime(),v);
	StdShaderDlg *pDlg = new StdShaderDlg(hwMtlEdit, imp, NMBUTS );
	pDlg->SetThings( theMtl, theMtl->GetShader() );
	LoadStdShaderResources();
	if ( hOldRollup )
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC7),
			StdShaderDlgProc, 
			rollupName(theMtl->GetShader()),	
			(LPARAM)pDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);		
	else 
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC7),
			StdShaderDlgProc, 
			rollupName(theMtl->GetShader()),	
			(LPARAM)pDlg , 
			rollupOpen
			);	
	return pDlg;	
}


///////////////////////////////////////////////////////////////////
//
//	generic standard shader dlg panel
//
StdShaderDlg::StdShaderDlg( HWND hwMtlEdit, IMtlParams *pParams, int nMapButtons )
{
	pShader = NULL;
	pMtl = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;

	dadMgr.Init(this);
	
	shSpin = softSpin = ssSpin = siSpin = trSpin = NULL;
	for( long i = 0; i < NCOLBOX; ++i )
		cs[ i ] = NULL;

	texMBut = new ICustButton*[ NMBUTS ];
	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;

	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();
	curRGB = 0;
	isActive = valid = FALSE;
}

StdShaderDlg::~StdShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if ( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < NMBUTS; i++ )
	{
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}
// mjm - begin - 5.10.99
	delete[] texMBut;

	for (i=0; i<NCOLBOX; i++)
		if (cs[i]) ReleaseIColorSwatch(cs[i]);
// mjm - end

	ReleaseISpinner(shSpin);
	ReleaseISpinner(ssSpin);
	ReleaseISpinner(softSpin);
	ReleaseISpinner(siSpin);
	ReleaseISpinner(trSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite, GWLP_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}

int StdShaderDlg::FindSubTexFromHWND(HWND hw)
{
	for (long i=0; i<NMBUTS; i++) {
		if (hw == texMBut[i]->GetHwnd()) 
			return texmapFromMBut[i];
	}	
	return -1;
}

Class_ID StdShaderDlg::ClassID(){ return StdShaderParamDlgClassID; }

void  StdShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		Interval v;
		
		IRollupWindow* pRollup = pMtlPar->GetMtlEditorRollup();
		pRollup->SetPanelTitle( pRollup->GetPanelIndex(hRollup), rollupName(pShader) );

		shSpin->SetValue(FracToPc(pShader->GetGlossiness(0,0)),FALSE);
		shSpin->SetKeyBrackets(KeyAtCurTime(shdr_glossiness));

		ssSpin->SetValue(FracToPc(pShader->GetSpecularLevel(0,0)),FALSE);
		ssSpin->SetKeyBrackets(KeyAtCurTime(shdr_spec_lvl));

		softSpin->SetValue(pShader->GetSoftenLevel(0,0),FALSE);
		softSpin->SetKeyBrackets(KeyAtCurTime(shdr_soften));

		trSpin->SetValue(FracToPc(pMtl->GetOpacity( curTime )),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		softSpin->Enable( ! (pShader->SupportStdParams() & STD_PARAM_METAL) );

		CheckButton(hRollup, IDC_LOCK_AD, pShader->GetLockAD() );
		CheckButton(hRollup, IDC_LOCK_DS, pShader->GetLockDS() );
	 	UpdateLockADTex( FALSE ); //don't send to mtl
		DisableNonMetalControls();

		// >>>> color selfIllum
		BOOL colorSelfIllum = pShader->IsSelfIllumClrOn();
		SetCheckBox(hRollup,IDC_SI_COLORON, colorSelfIllum ); 
		if( colorSelfIllum ) {
//			ShowWindow( siSpin->GetHwnd(), SW_HIDE );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_EDIT), SW_HIDE );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_SPIN), SW_HIDE );
			ShowWindow( cs[N_SI_CLR]->GetHwnd(), SW_SHOW );
		} else {
			// disable the color swatch
			ShowWindow( cs[N_SI_CLR]->GetHwnd(), SW_HIDE );
			// show self-illum slider
//			ShowWindow( siSpin->GetHwnd(), SW_SHOW );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_EDIT), SW_SHOW );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_SPIN), SW_SHOW );

			siSpin->SetValue(FracToPc(pShader->GetSelfIllum(0,0)), FALSE);
			siSpin->SetKeyBrackets(KeyAtCurTime(shdr_self_illum_amnt));
		}

		UpdateColSwatches();
		UpdateHilite();
	}
}


void StdShaderDlg::NotifyChanged() 
{
	pShader->NotifyChanged();
}

void StdShaderDlg::ReloadDialog() 
{
	Interval v;
	pShader->Update(pMtlPar->GetTime(), v);
	LoadDialog(FALSE);
}


void StdShaderDlg::UpdateMtlDisplay()
{
	pMtlPar->MtlChanged(); // redraw viewports
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void StdShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm	 = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}


void StdShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}

Color StdShaderDlg::GetMtlColor(int i) 
{
	switch(i) {
		case 0:  return pShader->GetAmbientClr(0,0); 
		case 1:  return pShader->GetDiffuseClr(0,0);
		case 2:  return pShader->GetSpecularClr(0,0);
		case 3:  return pShader->GetSelfIllumClr(0,0);
		default: return Color(0,0,0);
	}
}

TCHAR *StdShaderDlg::GetColorName(int i) {
	switch(i) {
		case 0:  return GetString(IDS_DS_AMBIENT);	 
		case 1:  return GetString(IDS_DS_DIFFUSE);	 
		case 2:  return GetString(IDS_DS_SPECULAR);	 
		case 3:  return GetString(IDS_KE_SELFILLUM_CLR);	 
		default: return GetString(IDS_KE_NOSUCH_CLR);	 
	}
}

void StdShaderDlg::SetMtlColor(int i, Color c) {
	switch(i) {
		case 0: //ambient
			pShader->SetAmbientClr(c,curTime); 
			if ( pShader->GetLockAD() ){
				pShader->SetDiffuseClr(c, curTime);
				cs[1]->SetColor( c );
				if (pShader->GetLockDS() ){
					pShader->SetSpecularClr(c,curTime);
					cs[2]->SetColor(c);
				}
			}
			break;
		case 1: //diffuse
			pShader->SetDiffuseClr(c,curTime); 
			if (pShader->GetLockAD() ){
				pShader->SetAmbientClr(c,curTime);
				cs[0]->SetColor(c);
			}
			if ( pShader->GetLockDS() ){
				pShader->SetSpecularClr(c,curTime);
				cs[2]->SetColor(c);
				}
			break;
		case 2: // specular
			pShader->SetSpecularClr(c,curTime); 
			if (pShader->GetLockDS() ){
				pShader->SetDiffuseClr(c,curTime);
				cs[1]->SetColor(c);
				if (pShader->GetLockAD() ){
					pShader->SetAmbientClr(c,curTime);
					cs[0]->SetColor(c);
					}
				}
			break;
		case 3: 
			pShader->SetSelfIllumClr(c,curTime); 
			break;
	}
}

void StdShaderDlg::SelectEditColor(int i) {
	cs[ i ]->EditThis(FALSE);
	curRGB = GetMtlColor(i).toRGB();
}

void StdShaderDlg::SetLockAD(BOOL lock)
{
	if (lock) {
		if (IDYES!=MessageBox(hwmEdit, GetString(IDS_DS_LOCKAD), GetString(IDS_DS_LOCKCOL), MB_YESNO)) {
			CheckButton(hRollup, IDC_LOCK_AD, FALSE);	
			return;	
		}
		// set ambient color to diffuse
		pShader->SetAmbientClr( pShader->GetDiffuseClr(0,0), 0 );
		UpdateColSwatches();
	}
	pShader->SetLockAD(lock);
}


void StdShaderDlg::UpdateColSwatches() 
{
	for(int i=0; i < NCOLBOX; i++) {
		if ( cs[ i ] ) {
			cs[i]->SetKeyBrackets( pShader->KeyAtTime(colParamID[i],curTime) );
			cs[i]->SetColor( GetMtlColor(i) );
		}
	}
}

//-HiLite Curve Control------------------------------------------------------

static LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = GetWindowLongPtr(hwnd,GWL_ID);
	HWND hwParent = GetParent(hwnd);
	StdShaderDlg *theDlg = (StdShaderDlg *)GetWindowLongPtr(hwParent, GWLP_USERDATA);
	if (theDlg==NULL) return FALSE;

    switch (msg) {
		case WM_COMMAND: 	
		case WM_MOUSEMOVE: 	
		case WM_LBUTTONUP: 
		case WM_CREATE:
		case WM_DESTROY: 
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
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
} 


static void VertLine(HDC hdc,int x, int ystart, int yend) 
{
	MoveToEx(hdc, x, ystart, NULL); 
	if (ystart <= yend)
		LineTo(hdc, x, yend+1);
	else 
		LineTo(hdc, x, yend-1);
}

void StdShaderDlg::DrawHilite(HDC hdc, Rect& rect)
{
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
		float v = pShader->EvalHiliteCurve( (float)i/((float)npts*2.0f) );
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


void StdShaderDlg::DisableNonMetalControls() 
{
	BOOL b = (pShader->SupportStdParams() & STD_PARAM_METAL) ? 0 : 1;
//	EnableWindow( GetDlgItem(hRollup,  IDC_SOFT_EDIT), b);
//	EnableWindow( GetDlgItem(hRollup,  IDC_SPEC), b);
//	EnableWindow( GetDlgItem(hRollup,  IDC_MAPON_SP), b);
//	EnableWindow( GetDlgItem(hRollup,  IDC_LOCK_DS), b);

	ShowWindow( GetDlgItem(hRollup,  IDC_SOFT_EDIT), b);
	ShowWindow( GetDlgItem(hRollup,  IDC_SOFT_SPIN), b);
	ShowWindow( GetDlgItem(hRollup,  IDC_MAPON_SP), b);
	ShowWindow( GetDlgItem(hRollup,  IDC_LOCK_DS), b);
	ShowWindow( GetDlgItem(hRollup,  IDC_SPEC_TEXT), b);
	ShowWindow( GetDlgItem(hRollup,  IDC_SOFTEN_TEXT), b);
	ShowWindow( cs[2]->GetHwnd(), b );
}

void StdShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r);
	ReleaseDC(hwHilite,hdc);
}

void StdShaderDlg::UpdateLockADTex( BOOL passOn) {
	int lock = 	pShader->GetLockADTex();
	CheckButton(hRollup, IDC_LOCK_ADTEX, lock);

	ShowWindow(GetDlgItem(hRollup, IDC_MAPON_AM), !lock);
	texMBut[ 0 ]->Enable(!lock);

	if ( passOn ) {
		pMtl->SyncADTexLock( lock );
	}
//	UpdateMtlDisplay();
}


void StdShaderDlg::SetLockADTex(BOOL lock) {
	pShader->SetLockADTex( lock );
	UpdateLockADTex(TRUE); // passon to mtl
	NotifyChanged();
	UpdateMtlDisplay();
	}

void StdShaderDlg::SetLockDS(BOOL lock) 
{
	if (lock) {
		if (IDYES!=MessageBox(hwmEdit, GetString(IDS_DS_LOCK_DS),GetString(IDS_DS_LOCKCOL), MB_YESNO)) {
			CheckButton(hRollup, IDC_LOCK_DS, FALSE);	
			return;	
		}
		pShader->SetSpecularClr( pShader->GetDiffuseClr(0,0), 0 );
		UpdateColSwatches();
	}
	pShader->SetLockDS( lock );
}

int _ColIDCToIndex(int id) {
	switch (id) {
		case IDC_STD_COLOR1: return 0;
		case IDC_STD_COLOR2: return 1;
		case IDC_STD_COLOR3: return 2;
		case IDC_SI_COLOR: return 3;
		default: return 0;
		}
	}


BOOL StdShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG:
			{
			int i;
			
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

			for (i=0; i<NCOLBOX; i++) {
   				cs[i] = GetIColorSwatch(GetDlgItem(hwndDlg, colID[i]),
   					GetMtlColor(i), GetColorName(i));
			}

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)HiliteWndProc);

			shSpin = SetupIntSpinner(hwndDlg, IDC_SH_SPIN, IDC_SH_EDIT, 0,100, 0);
			ssSpin = SetupIntSpinner(hwndDlg, IDC_SS_SPIN, IDC_SS_EDIT, 0,999, 0);
			softSpin = SetupFloatSpinner(hwndDlg, IDC_SOFT_SPIN, IDC_SOFT_EDIT, 0.0f,1.0f,0.0f,.01f);
			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);

			for (int j=0; j<NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
			}

			SetupLockButton(hwndDlg,IDC_LOCK_AD,FALSE);
			SetupLockButton(hwndDlg,IDC_LOCK_DS,FALSE);
			SetupPadLockButton(hwndDlg,IDC_LOCK_ADTEX, TRUE);

/* // mjm - 5.10.99 - isn't this already created above when i == N_SI_CLR?
			// create both a self-illum color as well as a spinner
			cs[N_SI_CLR] = GetIColorSwatch(GetDlgItem(hwndDlg, colID[N_SI_CLR] ),
   											GetMtlColor(N_SI_CLR), GetColorName(N_SI_CLR));
*/
			// self-illum spinner
			siSpin = SetupIntSpinner(hwndDlg, IDC_SI_SPIN, IDC_SI_EDIT, 0,100, 0);
			
			if( pShader->IsSelfIllumClrOn() ) {
				// enable the color swatch, disable the spinner
				ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_HIDE );
				ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_HIDE );
			} else {
				// disable the color swatch
				ShowWindow( cs[N_SI_CLR]->GetHwnd(), SW_HIDE );
			}

			LoadDialog(TRUE);
			}
			break;

		case WM_COMMAND: 
			{
			for ( int i=0; i<NMBUTS; i++) {
				if (id == texMButtonsIDC[i]) {
					PostMessage(hwmEdit,WM_TEXMAP_BUTTON, texmapFromMBut[i],(LPARAM)pMtl );
					UpdateMapButtons();
					goto exit;
					}
				}
			}

		    switch (id) {

				case IDC_LOCK_AD:
					SetLockAD(IsButtonChecked(hwndDlg, IDC_LOCK_AD));
					UpdateMtlDisplay();
					break;
				
				case IDC_LOCK_DS:
					SetLockDS(IsButtonChecked(hwndDlg, IDC_LOCK_DS));
					UpdateMtlDisplay();
					break;
				
				case IDC_LOCK_ADTEX:{
					BOOL on = IsButtonChecked(hwndDlg, IDC_LOCK_ADTEX);
					SetLockADTex(on);
					UpdateMtlDisplay();
				} break;

				case IDC_SI_COLORON:{
					int isOn = GetCheckBox(hwndDlg, IDC_SI_COLORON );
					pShader->SetSelfIllumClrOn( isOn );			
					if ( isOn ) {
						// enable the color swatch, disable the spinner
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_HIDE );
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_HIDE );
						ShowWindow( cs[N_SI_CLR]->GetHwnd(), SW_SHOW );
					} else {
						// disable the color swatch
						ShowWindow( cs[N_SI_CLR]->GetHwnd(), SW_HIDE );
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_SHOW );
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_SHOW );
					}
				    NotifyChanged();
					UpdateMtlDisplay();
				}
				break;
			}
			break;
		case CC_COLOR_SEL:
			{
			int id = LOWORD(wParam);
			SelectEditColor(_ColIDCToIndex(id));
			}			
			break;
		case CC_COLOR_DROP:
			{
			int id = LOWORD(wParam);
			SelectEditColor(_ColIDCToIndex(id));
			UpdateMtlDisplay();				
			}			
			break;
		case CC_COLOR_BUTTONDOWN:
			theHold.Begin();
			break;
		case CC_COLOR_BUTTONUP:
			if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else theHold.Cancel();
			UpdateMtlDisplay();				
			break;
		case CC_COLOR_CHANGE:
			{			
			int id = LOWORD(wParam);
			int buttonUp = HIWORD(wParam); 
			int n = _ColIDCToIndex(id);
			if (buttonUp) theHold.Begin();
			curRGB = cs[n]->GetColor();
			SetMtlColor(n, Color(curRGB));
			if (buttonUp) {
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
				// DS: 5/3/99-  this was commented out. I put it back in, because
				// it is necessary for the Reset button in the color picker to 
				// update the viewport.				
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
				case IDC_SH_SPIN: 
					pShader->SetGlossiness(PcToFrac(shSpin->GetIVal()), curTime); 
					UpdateHilite();
					break;
				case IDC_SS_SPIN: 
					pShader->SetSpecularLevel(PcToFrac(ssSpin->GetIVal()),curTime); 
					UpdateHilite();
					break;
				case IDC_SOFT_SPIN: 
					pShader->SetSoftenLevel(softSpin->GetFVal(),curTime); 
					break;
				case IDC_SI_SPIN: 
					pShader->SetSelfIllum(PcToFrac(siSpin->GetIVal()),curTime); 
					break;
				//******** >>>><<<< required handling for opacity....must be present in all dialogs
				case IDC_TR_SPIN: 
					pMtl->SetOpacity(PcToFrac( trSpin->GetIVal()),curTime); 
					break;
			}
//			UpdateMtlDisplay();
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


//----------------------------------------------------------------------------------------
//- Constant & Phong Shader  -------------------------------------------------------------
//----------------------------------------------------------------------------------------
#define	SELFILLUM_FRAC		1.0f

void phongIllum(ShadeContext &sc, IllumParams &ip, float softThresh, BOOL clrSelfIllum ) 
{
LightDesc *l;
Color lightCol;
BOOL isShiny;
Point3 R;

	if (isShiny = (ip.channels[stdIDToChannel[ID_SS]].r > 0.0f) ) 
		R = sc.ReflectVector(); // no use of ip.N?, no, it's in sc too

	double phExp = pow(2.0, ip.channels[stdIDToChannel[ID_SH]].r * 10.0); // expensive.!!	TBD

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, kL;
		Point3 L;
		if (l->Illuminate(sc, sc.Normal(), lightCol, L, NL, kL) ){
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;
			// diffuse
			if (l->affectDiffuse)
				ip.diffIllumOut += kL * lightCol;
#define LIGHT_TRACER_CHANGES 1
#if LIGHT_TRACER_CHANGES
			// > 4/18/02 - 2:11am --MQM-- don't do specular on regathering hits to reduce variance
			if ( ( !SHADECONTEXT_IS_REGATHERING(sc) ) && ( isShiny && l->affectSpecular ) ) {
#else
			if (isShiny && l->affectSpecular) {
#endif
				// specular (Phong2) 
				float c = DotProd(L,R);
				if (c > 0.0f) {
					if (softThresh != 0.0 && kL < softThresh ){
						float r = kL/softThresh;
						c *= Soften(r);
					}
					c = (float)pow((double)c, (double)phExp); // could use table lookup for speed
					ip.specIllumOut += c * ip.channels[stdIDToChannel[ID_SS]].r * lightCol;
				}
			}
		}
	}
	
	// Apply mono self illumination
//	BOOL clrSelfIllum = (ip.shFlags & SELFILLUM_CLR_ON) ? 1 : 0;
	if ( ! clrSelfIllum ){
		// changed back, fixed in getIllumParams, KE 4/27
		float si = 0.3333333f * (ip.channels[ID_SI].r + ip.channels[ID_SI].g + ip.channels[ID_SI].b);
		//		float si = ip.channels[ID_SI].r; // DS: 4/23/99
//		ip.diffIllumOut = (si>=1.0f) ?  Color(1.0f,1.0f,1.0f) 
//			: ip.diffIllumOut * (1.0f-si) + si;
		if ( si > 0.0f ) {
			si = UBound( si );
			ip.selfIllumOut = si * ip.channels[ID_DI];
			ip.diffIllumOut *= (1.0f-si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f-si;
		}
	}
	else {
		// colored self-illum
		ip.selfIllumOut += ip.channels[ID_SI];
		}

	// now we can multiply by the clrs,
	ip.ambIllumOut *= ip.channels[ID_AM]; 
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= ip.channels[ID_DI]; 
	ip.specIllumOut *= ip.channels[ID_SP]; 

}


class Constant2: public ExposureMaterialControlImp<Constant2, StdShaderImp> {
public:
	static ExposureMaterialControlDesc msExpMtlControlDesc;

	BOOL IsFaceted() { return TRUE; }
	Class_ID 		ClassID() { return ConstantClassID; }
	TSTR GetName() { return GetString( IDS_KE_CONSTANT ); }
	void Illum(ShadeContext &sc, IllumParams &ip){ 
		phongIllum( sc, ip, softThresh, selfIllumClrOn ); 
		ShadeTransmission(sc, ip, ip.channels[ID_RR], ip.refractAmt);
		ShadeReflection( sc, ip, ip.channels[ID_RL] ); 

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL) {
			if (isInvertSelfIllum())
				sc.globContext->pToneOp->RGBToScaled(ip.selfIllumOut);
			if (isInvertReflect() && (ip.hasComponents & HAS_REFLECT))
				sc.globContext->pToneOp->RGBToScaled(ip.reflIllumOut);
			if (isInvertRefract() && (ip.hasComponents & HAS_REFRACT))
				sc.globContext->pToneOp->RGBToScaled(ip.transIllumOut);
		}

		CombineComponents( sc, ip ); 
		}
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) { 
		rcol *= ip.channels[ID_SP];
	};
	float EvalHiliteCurve(float x) {
		double phExp = pow(2.0, glossiness * 10.0); // expensive.!!	TBD
		return specularLevel*(float)pow((double)cos(x*PI), phExp );  
	}
	RefTargetHandle Clone( RemapDir &remap ){ Constant2* s = new Constant2(); 
											  StdShaderImp::Clone(remap, s); 
											  BaseClone(this, s, remap);
											  return s;	
											}
	ClassDesc2* GetCD();
//	Constant2();
};

class ConstantShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new Constant2(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_CONSTANT); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return ConstantClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Constant"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};
ConstantShaderClassDesc constCD;
ClassDesc * GetConstantShaderCD(){ return &constCD; }
ClassDesc2* Constant2::GetCD() { return &constCD; }
//Constant2::Constant2(){ StdShaderImp(); }

ExposureMaterialControlDesc Constant2::msExpMtlControlDesc(constCD,
	IDS_EXPOSURE_MATERIAL_CONTROL,
	IDS_NO_EXPOSURE,
	IDS_INVERTSELFILLUM,
	IDS_INVERTREFLECT,
	IDS_INVERTREFRACT
);


/****************8
static BOOL breakCircle = FALSE;

// parameter setter callback, reflect any ParamBlock-mediated param setting in instance data members.
class StdShadersPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)    // set from v
	{
		StdShaderImp* s = (StdShaderImp*)owner;
		switch (id)
		{	// enforce ad & ds lock
			case shdr_ambient: {
				if( s->GetLockAD() ){
					if( ! breakCircle ){
						breakCircle = TRUE;
						s->SetDiffuseClr( s->GetAmbientClr(t), t );
						breakCircle = FALSE;
					}
				}
			} break;
			case shdr_diffuse: {
				if( ! breakCircle ){
					breakCircle = TRUE;
					if( s->GetLockAD() ){
						s->SetAmbientClr( s->GetDiffuseClr( t ), t );
					}
					if( s->GetLockDS() ){
						s->SetSpecularClr( s->GetDiffuseClr( t ), t );
					}
					breakCircle = FALSE;
				}
			} break;
			case shdr_specular: {
				if( s->GetLockDS() ){
					if( ! breakCircle ){
						breakCircle = TRUE;
						s->SetDiffuseClr( s->GetSpecularClr( t ), t );
						breakCircle = FALSE;
					}
				}
			} break;
		}
	}
};

static StdShadersPBAccessor stdShadersPBAccessor;
*************************/

// shader parameters
static ParamBlockDesc2 const_param_blk ( shdr_params, _T("shaderParameters"),  0, &constCD, P_AUTO_CONSTRUCT, 0, 
	// params
	shdr_ambient,	 _T("ambient"),	TYPE_RGBA,	P_ANIMATABLE,	IDS_DS_AMBIENT,	
		p_default,		Color(0, 0, 0), 
//		p_accessor,		&stdShadersPBAccessor,
		end,
	shdr_diffuse,	 _T("diffuse"),	TYPE_RGBA,	P_ANIMATABLE,	IDS_DS_DIFFUSE,	
		p_default,		Color(0.5f, 0.5f, 0.5f), 
//		p_accessor,		&stdShadersPBAccessor,
		end,
	shdr_specular,	 _T("specular"), TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_SPECULAR,	
		p_default,		Color(1.0f, 1.0f, 1.0f), 
//		p_accessor,		&stdShadersPBAccessor,
		end,
	shdr_ad_texlock, _T("adTextureLock"),	TYPE_BOOL,		0,				IDS_JW_ADTEXLOCK,	
		p_default,		TRUE, 
		end,
	shdr_ad_lock,	_T("adLock"),	TYPE_BOOL,				0,				IDS_JW_ADLOCK,	
		p_default,		FALSE, 
		end,
	shdr_ds_lock,	_T("dsLock"),	TYPE_BOOL,				0,				IDS_JW_DSLOCK,	
		p_default,		FALSE, 
		end,
	shdr_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0,		IDS_JW_SELFILLUMCOLORON,	
		p_default,		FALSE, 
		end,
	shdr_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	shdr_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE,	IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	shdr_spec_lvl,	_T("specularLevel"),TYPE_PCNT_FRAC,		P_ANIMATABLE,	IDS_KE_SPEC_LEVEL,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	shdr_glossiness, _T("glossiness"),	TYPE_PCNT_FRAC,		P_ANIMATABLE,	IDS_KE_GLOSSINESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	shdr_soften,		_T("soften"),		TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_SOFTEN,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		end,
	end
	);

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
class Phong2: public ExposureMaterialControlImp<Phong2, StdShaderImp> {
public:
	static ExposureMaterialControlDesc msExpMtlControlDesc;

	Class_ID 		ClassID() { return PhongClassID; }
	TSTR GetName() { return GetString( IDS_KE_PHONG ); }
	void Illum(ShadeContext &sc, IllumParams &ip){
		phongIllum( sc, ip, softThresh, selfIllumClrOn ); 
		ShadeTransmission(sc, ip, ip.channels[ID_RR], ip.refractAmt);
		ShadeReflection( sc, ip, ip.channels[ID_RL] ); 

		if (sc.globContext != NULL && sc.globContext->pToneOp != NULL) {
			if (isInvertSelfIllum())
				sc.globContext->pToneOp->RGBToScaled(ip.selfIllumOut);
			if (isInvertReflect() && (ip.hasComponents & HAS_REFLECT))
				sc.globContext->pToneOp->RGBToScaled(ip.reflIllumOut);
			if (isInvertRefract() && (ip.hasComponents & HAS_REFRACT))
				sc.globContext->pToneOp->RGBToScaled(ip.transIllumOut);
		}

		CombineComponents( sc, ip ); 
	}
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) { 
		rcol *= ip.channels[ID_SP];
	};
	float EvalHiliteCurve(float x) {
		double phExp = pow(2.0, glossiness * 10.0); 
		return specularLevel*(float)pow((double)cos(x*PI), phExp );  
	}
	RefTargetHandle Clone( RemapDir &remap ){ Phong2* s = new Phong2(); 
											  StdShaderImp::Clone(remap, s); 
											  BaseClone(this, s, remap);
											  return s;	
											}
	ClassDesc2* GetCD();
//	Phong2();
};

class PhongShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new Phong2(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_PHONG); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return PhongClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Phong2"); }		// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

PhongShaderClassDesc phongCD;
ClassDesc * GetPhongShaderCD(){ return &phongCD; }
ClassDesc2* Phong2::GetCD() { return &phongCD; }

// shader parameters
static ParamBlockDesc2 phong2_param_blk ( shdr_params, _T("shaderParameters"),  0, &phongCD, P_AUTO_CONSTRUCT + P_USE_PARAMS, 
	// pblock refno
	0, 
	// use params from existing descriptor
	&const_param_blk
	);

ExposureMaterialControlDesc Phong2::msExpMtlControlDesc(phongCD,
	IDS_EXPOSURE_MATERIAL_CONTROL,
	IDS_NO_EXPOSURE,
	IDS_INVERTSELFILLUM,
	IDS_INVERTREFLECT,
	IDS_INVERTREFRACT
);
#endif // USE_LIMITED_STDMTL 
				 

//----------------------------------------------------------------------------------------
//- Blinn2 Shader  ------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class Blinn2: public ExposureMaterialControlImp<Blinn2, StdShaderImp> {
	public:
	static ExposureMaterialControlDesc msExpMtlControlDesc;

	Class_ID 		ClassID() { return BlinnClassID; }
	TSTR GetName() { return GetString( IDS_KE_BLINN ); }
	void Illum(ShadeContext &sc, IllumParams &ip);
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) 
		{ rcol *= ip.channels[ID_SP]; };
	float EvalHiliteCurve(float x) {
		double phExp = pow(2.0, glossiness * 10.0); // expensive.!!	TBD
		return specularLevel*(float)pow((double)cos(x*PI), phExp );  
		}
	RefTargetHandle Clone( RemapDir &remap ){ Blinn2* s = new Blinn2(); 
											  StdShaderImp::Clone(remap, s); 
											  BaseClone(this, s, remap);
											  return s;	
											}
	ClassDesc2* GetCD();
};

class BlinnShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new Blinn2(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLINN); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return BlinnClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Blinn2"); }		// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

BlinnShaderClassDesc blinnCD;
ClassDesc * GetBlinnShaderCD(){ return &blinnCD; }
ClassDesc2* Blinn2::GetCD() { return &blinnCD; }


// fix for loading alpha files >>>>>>< !!!! out for release
class OldBlinnShaderClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void *			Create(BOOL loading) { 	return new Blinn2(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_BLINN); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return OldBlinnClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

OldBlinnShaderClassDesc oldBlinnCD;
ClassDesc * GetOldBlinnShaderCD(){ return &oldBlinnCD; }

// shader parameters
//static ParamBlockDesc2 blinn2_param_blk ( shdr_params, _T("shaderParameters"),  0, &blinnCD, P_AUTO_CONSTRUCT + P_USE_PARAMS, 
static ParamBlockDesc2 blinn2_param_blk ( shdr_params, _T("shaderParameters"),  0, &blinnCD, P_AUTO_CONSTRUCT, 0, 
//	// use params from existing descriptor
//	&const_param_blk
	// params
	shdr_ambient,	 _T("ambient"),	TYPE_RGBA,	P_ANIMATABLE,	IDS_DS_AMBIENT,	
		p_default,		Color(0, 0, 0), 
//		p_accessor,		&stdShadersPBAccessor,
		end,
	shdr_diffuse,	 _T("diffuse"),	TYPE_RGBA,	P_ANIMATABLE,	IDS_DS_DIFFUSE,	
		p_default,		Color(0.5f, 0.5f, 0.5f), 
//		p_accessor,		&stdShadersPBAccessor,
		end,
	shdr_specular,	 _T("specular"), TYPE_RGBA,				P_ANIMATABLE,	IDS_DS_SPECULAR,	
		p_default,		Color(1.0f, 1.0f, 1.0f), 
//		p_accessor,		&stdShadersPBAccessor,
		end,
	shdr_ad_texlock, _T("adTextureLock"),	TYPE_BOOL,		0,				IDS_JW_ADTEXLOCK,	
		p_default,		TRUE, 
		end,
	shdr_ad_lock,	_T("adLock"),	TYPE_BOOL,				0,				IDS_JW_ADLOCK,	
		p_default,		FALSE, 
		end,
	shdr_ds_lock,	_T("dsLock"),	TYPE_BOOL,				0,				IDS_JW_DSLOCK,	
		p_default,		FALSE, 
		end,
	shdr_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0,		IDS_JW_SELFILLUMCOLORON,	
		p_default,		FALSE, 
		end,
	shdr_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	shdr_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE,	IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	shdr_spec_lvl,	_T("specularLevel"),TYPE_PCNT_FRAC,		P_ANIMATABLE,	IDS_KE_SPEC_LEVEL,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	shdr_glossiness, _T("glossiness"),	TYPE_PCNT_FRAC,		P_ANIMATABLE,	IDS_KE_GLOSSINESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	shdr_soften,		_T("soften"),		TYPE_FLOAT,			P_ANIMATABLE,	IDS_DS_SOFTEN,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		end,
	end

	);

ExposureMaterialControlDesc Blinn2::msExpMtlControlDesc(blinnCD,
	IDS_EXPOSURE_MATERIAL_CONTROL,
	IDS_NO_EXPOSURE,
	IDS_INVERTSELFILLUM,
	IDS_INVERTREFLECT,
	IDS_INVERTREFRACT
);

void Blinn2::Illum(ShadeContext &sc, IllumParams &ip) {
	LightDesc *l;
	Color lightCol;

	// Blinn style phong
	BOOL is_shiny= (ip.channels[StdIDToChannel(ID_SS)].r > 0.0f) ? 1:0; 
	double phExp = pow(2.0, ip.channels[StdIDToChannel(ID_SH)].r * 10.0) * 4.0; // expensive.!!	TBD

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;
#ifdef WEBVERSION
		if ( l ) { // l may be == null if a light class is not available in the current config
#endif
		if (l->Illuminate(sc,sc.Normal(),lightCol,L,NL,diffCoef)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;

			// diffuse
			if (l->affectDiffuse)
				ip.diffIllumOut += diffCoef * lightCol;

			// specular (Phong2) 
#if LIGHT_TRACER_CHANGES
			// > 4/18/02 - 2:11am --MQM-- don't do specular on regathering hits to reduce variance
			if ( ( !SHADECONTEXT_IS_REGATHERING(sc) ) && ( is_shiny && l->affectSpecular ) ) {
#else
			if (is_shiny&&l->affectSpecular) {
#endif
				Point3 H = Normalize( L-sc.V() );
				float c = DotProd(sc.Normal(),H);	 
				if (c>0.0f) {
					if (softThresh!=0.0 && diffCoef<softThresh) {
						c *= Soften(diffCoef/softThresh);
						}
					c = (float)pow((double)c, phExp); // could use table lookup for speed
					ip.specIllumOut += c * ip.channels[StdIDToChannel(ID_SS)].r * lightCol;
					}
				}
 			}
#ifdef WEBVERSION
		} // if ( l ) 
#endif
		}


	// Apply mono self illumination
	if ( ! selfIllumClrOn ){
		// lerp between diffuse & white
		// changed back, fixed in getIllumParams, KE 4/27
		float si = 0.3333333f * (ip.channels[ID_SI].r + ip.channels[ID_SI].g + ip.channels[ID_SI].b);
		if ( si > 0.0f ) {
			si = UBound( si );
			ip.selfIllumOut = si * ip.channels[ID_DI];
			ip.diffIllumOut *= (1.0f-si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f-si;
		}
	} else {
		// colored self illum, 
		ip.selfIllumOut += ip.channels[ID_SI];
	}
	// now we can multiply by the clrs, 
	ip.ambIllumOut *= ip.channels[ID_AM]; 
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= ip.channels[ID_DI]; 
	ip.specIllumOut *= ip.channels[ID_SP]; 

	ShadeTransmission(sc, ip, ip.channels[ID_RR], ip.refractAmt);
	ShadeReflection( sc, ip, ip.channels[ID_RL] ); 

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL) {
		if (isInvertSelfIllum())
			sc.globContext->pToneOp->RGBToScaled(ip.selfIllumOut);
		if (isInvertReflect() && (ip.hasComponents & HAS_REFLECT))
			sc.globContext->pToneOp->RGBToScaled(ip.reflIllumOut);
		if (isInvertRefract() && (ip.hasComponents & HAS_REFRACT))
			sc.globContext->pToneOp->RGBToScaled(ip.transIllumOut);
	}

	CombineComponents( sc, ip ); 
}

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
//----------------------------------------------------------------------------------------
//- Metal2 Shader  ------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class Metal2: public ExposureMaterialControlImp<Metal2, StdShaderImp> {
	public:
	static ExposureMaterialControlDesc msExpMtlControlDesc;

	Class_ID 		ClassID() { return MetalClassID; }
	TSTR GetName() { return GetString( IDS_KE_METAL ); }
	void Illum(ShadeContext &sc, IllumParams &ip);
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
		{ rcol *= ip.channels[ID_DI]; };
	float EvalHiliteCurve(float x);
    ULONG SupportStdParams(){ return STD_BASIC_METAL+STD_EXTRA; }

	BOOL IsChannelSupported( long nTex ){ return ( nTex==ID_SP ) ? FALSE : TRUE; }
	RefTargetHandle Clone( RemapDir &remap ){ Metal2* s = new Metal2(); 
											  StdShaderImp::Clone(remap, s); 
											  BaseClone(this, s, remap);
											  return s;	
											}
	ClassDesc2* GetCD();
//	Metal2();
};

class MetalShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new Metal2(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_METAL); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return MetalClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Metal2"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

MetalShaderClassDesc metalCD;
ClassDesc* GetMetalShaderCD(){ return &metalCD; }
ClassDesc2* Metal2::GetCD() { return &metalCD; }
//Metal2::Metal2(){ StdShaderImp(); }

/*******
// shader parameters
static ParamBlockDesc2 metal2_param_blk ( shdr_params, _T("shaderParameters"),  0, &metalCD, P_AUTO_CONSTRUCT + P_USE_PARAMS, 
	// pblock refno
	0, 
	// use params from existing descriptor
	&const_param_blk
	);
********/

// shader parameters
static ParamBlockDesc2 metal2_param_blk ( shdr_params, _T("shaderParameters"),  0, &metalCD, P_AUTO_CONSTRUCT, 0, 
	// params
	shdr_ambient,	 _T("ambient"),	TYPE_RGBA,	P_ANIMATABLE,	IDS_DS_AMBIENT,	
		p_default,		Color(0, 0, 0), 
		end,
	shdr_diffuse,	 _T("diffuse"),	TYPE_RGBA,	P_ANIMATABLE,	IDS_DS_DIFFUSE,	
		p_default,		Color(0.5f, 0.5f, 0.5f), 
		end,
	shdr_specular,	 _T("specular"), TYPE_RGBA,				0,	IDS_DS_SPECULAR,	
		p_default,		Color(1.0f, 1.0f, 1.0f), 
		end,
	shdr_ad_texlock, _T("adTextureLock"),	TYPE_BOOL,		0,				IDS_JW_ADTEXLOCK,	
		p_default,		TRUE, 
		end,
	shdr_ad_lock,	_T("adLock"),	TYPE_BOOL,				0,				IDS_JW_ADLOCK,	
		p_default,		FALSE, 
		end,
	shdr_ds_lock,	_T("dsLock"),	TYPE_BOOL,				0,				IDS_JW_DSLOCK,	
		p_default,		FALSE, 
		end,
	shdr_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0,		IDS_JW_SELFILLUMCOLORON,	
		p_default,		FALSE, 
		end,
	shdr_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	shdr_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE,	IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	shdr_spec_lvl,	_T("specularLevel"),TYPE_PCNT_FRAC,		P_ANIMATABLE,	IDS_KE_SPEC_LEVEL,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	shdr_glossiness, _T("glossiness"),	TYPE_PCNT_FRAC,		P_ANIMATABLE,	IDS_KE_GLOSSINESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	shdr_soften,		_T("soften"),		TYPE_FLOAT,			0,	IDS_DS_SOFTEN,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		end,
	end
	);


ExposureMaterialControlDesc Metal2::msExpMtlControlDesc(metalCD,
	IDS_EXPOSURE_MATERIAL_CONTROL,
	IDS_NO_EXPOSURE,
	IDS_INVERTSELFILLUM,
	IDS_INVERTREFLECT,
	IDS_INVERTREFRACT
);

float _CompK(float f0) { return float(2.0*sqrt(f0)/sqrt(1.0-f0)); }

float _fres_metal(float c, float k) 
{
	float b,rpl,rpp,c2;
	b = k*k + 1.0f;
	c2 = c*c;
	rpl = (b*c2-2*c+1)/(b*c2+2*c+1);
	rpp = (b-2*c+c2)/(b+2*c+c2);
	return(.5f*(rpl+rpp));
}

void Metal2::Illum(ShadeContext &sc, IllumParams &ip)
{
	LightDesc *l;
	Color lightCol;
	BOOL gotKav = FALSE;
	float kav, fav0, m2inv,NV;
	
	BOOL is_shiny;
	if (ip.channels[StdIDToChannel(ID_SS)].r != 0.0f) {		// spec lev	
		NV = -DotProd(sc.Normal(), sc.V() );  // N dot V: view vector is TOWARDS us.
		is_shiny = 1;
		float r = 1.0f-ip.channels[StdIDToChannel(ID_SH)].r;	//gloss
		if (r<=0.0f) r = .00001f;
		if (r>=1.0f) r = .99999f;//ke, 2/26/99
		m2inv = 1.0f/(r*r);  
		}
	else 
		is_shiny = 0;
	
	for (int i=0; i < sc.nLights; i++) {
		l = sc.Light(i);
		register float NL, diffCoef;
		Point3 L;

		if (!l->Illuminate(sc, sc.Normal(), lightCol, L, NL, diffCoef)) 
			continue;

		if (l->ambientOnly) {
			ip.ambIllumOut += lightCol;
			continue;
			}

		// diffuse
		if (NL>0.0f&&l->affectDiffuse)  // TBD is the NL test necessary?
			ip.diffIllumOut += diffCoef * lightCol;

#if LIGHT_TRACER_CHANGES
		// > 4/18/02 - 2:11am --MQM-- don't do specular on regathering hits to reduce variance
		if ( ( !SHADECONTEXT_IS_REGATHERING(sc) ) && ( is_shiny && l->affectSpecular ) ) {
#else
		if (is_shiny&&l->affectSpecular) { // SPECULAR 
#endif
			Color fcol;
			float LH,NH,VH;
		    float sec2;  // Was double?? TBD
			Point3 H;
	
			if (NV<0.0f) continue;

			H = Normalize(L-sc.V());

			LH = DotProd(L,H);  // cos(phi)   
			NH = DotProd(sc.Normal(),H);  // cos(alpha) 
			if (NH==0.0f) continue;
			VH = -DotProd(sc.V(), H);

			// compute geometrical attenuation factor 
			float G = (NV<NL)? (2.0f*NV*NH/VH): (2.0f*NL*NH/VH);
			if (G>0.0f) {
				// Compute (approximate) indices of refraction
				//	this can be factored out for non-texture-mapped mtls
				if (!gotKav) {
					fav0 = Intens(ip.channels[ID_DI]);
					if (fav0>=1.0f) fav0 = .9999f;
					kav = _CompK(fav0);	
					gotKav = TRUE;
				}

				float fav = _fres_metal(LH,kav);
				float t = (fav-fav0)/(1.0f-fav0);
				fcol = (1.0f-t)*ip.channels[ID_DI] + Color(t,t,t);

				// Beckman distribution  (from Cook-Torrance paper)
				sec2 = 1.0f/(NH*NH);  // 1/sqr(cos) 
				float D = (.5f/PI)*sec2*sec2*m2inv*(float)exp((1.0f-sec2)*m2inv);  					
				if (G>1.0f) G = 1.0f;
				float Rs = ip.channels[StdIDToChannel(ID_SS)].r * D * G / (NV+.05f);	
				ip.specIllumOut += fcol * Rs * lightCol;
				}
			} 
		}
	ip.diffIllumOut *= LBound( 1.0f - Abs(ip.channels[StdIDToChannel(ID_SS)].r) ); //ke, 2/26/99

	// Apply mono self illumination
//	BOOL clrSelfIllum = (ip.shFlags & SELFILLUM_CLR_ON) ? 1 : 0;
//	if ( ! clrSelfIllum ){
	if ( ! selfIllumClrOn ){
		// lerp between diffuse illum & white
		// changed back, fixed in getIllumParams, KE 4/27
		float si = 0.3333333f * (ip.channels[ID_SI].r + ip.channels[ID_SI].g + ip.channels[ID_SI].b);
		if ( si > 0.0f ) {
			si = UBound( si );
			ip.selfIllumOut = si * ip.channels[ID_DI];
			ip.diffIllumOut *= (1.0f-si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f-si;
		}
	} else {
		// colored self illum, 
		ip.selfIllumOut += ip.channels[ID_SI];
	}

	// now we can multiply by the clrs, save ambient
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= ip.channels[ID_DI]; 
	ip.ambIllumOut *= ip.channels[ID_AM]; 

	ShadeTransmission(sc, ip, ip.channels[ID_RR], ip.refractAmt);
	ShadeReflection( sc, ip, ip.channels[ID_RL] ); 

	if (sc.globContext != NULL && sc.globContext->pToneOp != NULL) {
		if (isInvertSelfIllum())
			sc.globContext->pToneOp->RGBToScaled(ip.selfIllumOut);
		if (isInvertReflect() && (ip.hasComponents & HAS_REFLECT))
			sc.globContext->pToneOp->RGBToScaled(ip.reflIllumOut);
		if (isInvertRefract() && (ip.hasComponents & HAS_REFRACT))
			sc.globContext->pToneOp->RGBToScaled(ip.transIllumOut);
	}

	CombineComponents( sc, ip ); 

}


float Metal2::EvalHiliteCurve(float x) 
{
	float r = 1.0f - glossiness;
	if (r == 0.0f) r = .00001f;
	float fm2inv = 1.0f/(r*r);  

	float c = (float)cos(x*PI);
	float sec2 = 1.0f/(c*c);	  // 1/sqr(cos) 
	return specularLevel *(.5f/PI)*sec2*sec2*fm2inv*(float)exp((1.0f-sec2)*fm2inv);  					
}

//-------------------------------------------------------------------------

#endif // USE_LIMITED_STDMTL
