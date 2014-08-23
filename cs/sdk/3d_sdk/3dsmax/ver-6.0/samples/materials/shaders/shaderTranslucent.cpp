//////////////////////////////////////////////////////////////////////////////
//
//		Translucent Shader plug-in, implementation
//
//		Created: 9/10/01 Kells Elmquist
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "macrorec.h"
#include "toneop.h"

#define TRANSLUCENT_SHADER_CLASS_ID		0x2857f480

static Class_ID TranslucentShaderClassID( TRANSLUCENT_SHADER_CLASS_ID, 0);
static Class_ID TranslucentShaderDlgClassID( TRANSLUCENT_SHADER_CLASS_ID, 0);

// paramblock2 block and parameter IDs.
enum { translucent_params, };
// shdr_params param IDs
enum 
{ 
	tl_ambient, tl_diffuse, tl_specular,
	tl_self_illum_color, tl_self_illum_amnt,
	tl_glossiness, tl_specular_level, tl_diffuse_level, 
	tl_filter, tl_translucent_color,
	tl_backside_specular,	
	tl_ad_texlock, tl_ad_lock, tl_ds_lock, tl_use_self_illum_color, 
};


/////////////////////////////////////////////////////////////////////
//
//	Translucent Shader Basic Panel UI 
//
#define NMBUTS 10
#define N_SI_BUT 5
#define N_TR_BUT 6

static int texMButtonsIDC[] = {
	IDC_MAPON_AM,	IDC_MAPON_DI,	IDC_MAPON_SP,	IDC_MAPON_SH,
	IDC_MAPON_SS,	IDC_MAPON_SI,	IDC_MAPON_TR, IDC_MAPON_FI2,
	IDC_MAPON_DIFF_LEVEL,	IDC_MAPON_TRANSLUCENT_CLR, 
	};
		
// This array gives the texture map number for given MButton number								
// the ID_s are from stdmat.h
static int nTexmapFromMBut[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

// channel names
#define TL_NTEXMAPS	10

static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, IDS_DS_SHININESS, IDS_DS_SHIN_STR,
	IDS_KE_SELFILLUM, IDS_DS_TRANS, IDS_DS_FILTER,
	IDS_KE_DIFF_LEVEL, IDS_KE_TRANSLUCENT_CLR, IDS_KE_NONE, IDS_KE_NONE, 
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
};	

// internal non-local parsable channel map names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("ambientMap"), _T("diffuseMap"),	_T("specularMap"), _T("glossinessMap"), 
	_T("specularLevelMap"), _T("selfIllumMap"), _T("opacityMap"), _T("filterMap"),
	_T("diffuseLevelMap"),	_T("translucentColorMap"), _T(""), _T(""), 	
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),
};	


// sized for nmax textures
static int channelType[] = {
	CLR_CHANNEL, CLR_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, 
	MONO_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, CLR_CHANNEL, 
	MONO_CHANNEL, CLR_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
};
	

// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { 0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1, 9 };

//////////////////////////////////////////////////////////////////////////////////////////
//
//		Translucent Parameter Block
//
#define CURRENT_TL_SHADER_VERSION	1

#define NCOLBOX 6
//static int colID[NCOLBOX] = { IDC_STD_COLOR1, IDC_STD_COLOR2, IDC_STD_COLOR3, IDC_STD_COLOR4, IDC_SI_COLOR, IDC_TL_COLOR };
static int colParamID[NCOLBOX] = { tl_ambient, tl_diffuse, tl_specular, tl_filter, tl_self_illum_color, tl_translucent_color };
//#define N_SI_CLR		3
//#define N_AMB_CLR		0
////		+ STD_EXTRA_DLG + STD_EXTRA_REFRACTION + STD_EXTRA_REFLECTION
//						+ STD_PARAM_FILTER_CLR + STD_PARAM_DIFFUSE_LEV 

#define TRANSLUCENT_PARAMS (STD_PARAM_AMBIENT_CLR + STD_PARAM_DIFFUSE_CLR + STD_PARAM_SPECULAR_CLR\
						+ STD_PARAM_GLOSSINESS + STD_PARAM_SPECULAR_LEV\
						+ STD_PARAM_SELFILLUM + STD_PARAM_SELFILLUM_CLR + STD_PARAM_SELFILLUM_CLR_ON\
						+ STD_EXTRA_DLG + STD_EXTRA_REFLECTION\
						+ STD_PARAM_DIFFUSE_LEV \
						+ STD_PARAM_LOCKDS + STD_PARAM_LOCKAD + STD_PARAM_LOCKADTEX)

//----------------------------------------------------------------------------------------
//---- Translucent Extended Diffuse Shader, w/ Blinn specular hilite ----------------------
//----------------------------------------------------------------------------------------

class TranslucentShaderDlg;

class TranslucentShader : public ExposureMaterialControlImp<TranslucentShader, CombineComponentsCompShader> {
friend class TranslucentShaderCB;
friend class TranslucentShaderDlg;
friend class ExposureMaterialControlImp<TranslucentShader, CombineComponentsCompShader>;
protected:
	IParamBlock2 *pblock;   // ref 0
	Interval ivalid;
	TimeValue	curTime;

	TranslucentShaderDlg* paramDlg;

	BOOL selfIllumClrOn;
	BOOL lockDS;
	BOOL lockAD;
	BOOL lockADTex;
	BOOL backsideSpecular;

	Color ambient;
	Color diffuse;
	Color specular;
	Color selfIllumClr;
	Color translucentClr;
	Color filterClr;
	float selfIllum;	
//	float softThresh;
	float glossiness;
	float specularLevel;

	float diffLevel;

	static CombineComponentsFPDesc msExpMtlControlDesc;

public:
	TranslucentShader();
	void DeleteThis(){ delete this; }		
    ULONG SupportStdParams(){ return TRANSLUCENT_PARAMS; }
    void CopyStdParams( Shader* pFrom );

	// texture maps
	long  nTexChannelsSupported(){ return TL_NTEXMAPS; }
	TSTR  GetTexChannelName( long nTex ) { return GetString( texNameIDS[ nTex ] ); }
	TSTR  GetTexChannelInternalName( long nTex ) { return texInternalNames[ nTex ]; }
	long  ChannelType( long nTex ){ return channelType[nTex]; }
	// map StdMat Channel ID's to the channel number
	long StdIDToChannel( long stdID ){ return stdIDToChannel[ stdID ]; }

	BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime((ParamID)id,t); }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int ){ paramDlg = (TranslucentShaderDlg*)newDlg; }

	Class_ID ClassID() { return TranslucentShaderClassID; }
	TSTR GetName() { return GetString( IDS_KE_TRANSLUCENT ); }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i){ return (i==0)? pblock : NULL; }
	TSTR SubAnimName(int i){ return TSTR(GetString( IDS_KE_TRANSLUCENT_PARMS )); };
	int SubNumToRefNum(int subNum) { return subNum;	}

 	// JBW: add direct ParamBlock access
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } 

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i){ return (i == 0) ? pblock : NULL; }
	void SetReference(int i, RefTargetHandle rtarg) 
		{ if (i==0) pblock = (IParamBlock2*)rtarg; else assert(0); }
	void NotifyChanged(){ NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); }

	void Update(TimeValue t, Interval& valid);
	void Reset();
	RefTargetHandle Clone( RemapDir &remap=NoRemap() );
	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
	                            PartID& partID, RefMessage message );
	// IO
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	// Shader specific section
	void  Illum(ShadeContext &sc, IllumParams &ip);
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) {
//		rcol *= ip.channels[ID_SS].r * ip.channels[ID_SP] * DEFAULT_K_REFL; 
		rcol *= ip.channels[ID_SP] * DEFAULT_K_REFL; 
	}
	// cache for mapping of params, mtl fills in ip
	void GetIllumParams( ShadeContext &sc, IllumParams& ip );

	float EvalHiliteCurve(float x) {
		double phExp = pow(2.0, glossiness * 10.0); 
		return specularLevel*(float)pow((double)cos(x*PI), phExp );  
	}

	// Translucent Specific 
	void SetDiffuseLevel(float v, TimeValue t)		
			{ diffLevel = v; pblock->SetValue( tl_diffuse_level, t, v); }
	float GetDiffuseLevel(int mtlNum=0, BOOL backFace=FALSE){ return diffLevel; };
	float GetDiffuseLevel(TimeValue t){ return  pblock->GetFloat(tl_diffuse_level,t); }

	void SetBacksideSpecular(BOOL on ){ backsideSpecular = on; pblock->SetValue( tl_backside_specular, 0, on); }
	BOOL GetBacksideSpecular(){ return backsideSpecular; }

	void SetTranslucentClr(Color c, TimeValue t)
		{ translucentClr = c; pblock->SetValue( tl_translucent_color, t, c); }
	Color GetTranslucentClr(int mtlNum=0, BOOL backFace=FALSE){ return  translucentClr; }		
	Color GetTranslucentClr(TimeValue t){ return  pblock->GetColor(tl_translucent_color,t); }		

	void SetFilterClr(Color c, TimeValue t)
		{ filterClr = c; pblock->SetValue( tl_filter, t, c); }
	Color GetFilterClr(TimeValue t){ return  pblock->GetColor(tl_filter,t); }		
	Color GetFilterClr(int mtlNum=0, BOOL backFace=FALSE){ return  filterClr; }		

	// Std Params
	void SetLockDS(BOOL lock){ lockDS = lock; pblock->SetValue( tl_ds_lock, 0, lock); }
	BOOL GetLockDS(){ return lockDS; }
	void SetLockAD(BOOL lock){ lockAD = lock; pblock->SetValue( tl_ad_lock, 0, lock); }
	BOOL GetLockAD(){ return lockAD; }
	void SetLockADTex(BOOL lock){ lockADTex = lock; pblock->SetValue( tl_ad_texlock, 0, lock); }
	BOOL GetLockADTex(){ return lockADTex; }

	void SetSelfIllum(float v, TimeValue t)
		{ selfIllum = v; pblock->SetValue( tl_self_illum_amnt, t, v); }
	void SetSelfIllumClrOn( BOOL on ){ selfIllumClrOn = on; pblock->SetValue( tl_use_self_illum_color, 0, on); };
	BOOL IsSelfIllumClrOn(){ return selfIllumClrOn; };
	void SetSelfIllumClr(Color c, TimeValue t)
		{ selfIllumClr = c; pblock->SetValue( tl_self_illum_color, t, Point3(c.r,c.g,c.b) ); }

	void SetAmbientClr(Color c, TimeValue t)		
		{ ambient = c; pblock->SetValue( tl_ambient, t, c); }
	void SetDiffuseClr(Color c, TimeValue t)		
		{ diffuse = c; pblock->SetValue( tl_diffuse, t, c); }
	void SetSpecularClr(Color c, TimeValue t)
		{ specular = c; pblock->SetValue( tl_specular, t, c); }
	void SetGlossiness(float v, TimeValue t)		
		{ glossiness= v; pblock->SetValue( tl_glossiness, t, v); }
	void SetSpecularLevel(float v, TimeValue t)		
		{ specularLevel = v; pblock->SetValue( tl_specular_level, t, v); }
	void SetSoftenLevel(float v, TimeValue t) {}
//		{ softThresh = v; pblock->SetValue( tl_soften, t, v); }

	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return selfIllumClrOn; };
	Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE){ return ambient;}		
    Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE){ return diffuse;}		
	Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE){ return specular; };
	Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE){ return selfIllumClr; };
	float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE){ return selfIllum; };
	float GetGlossiness(int mtlNum=0, BOOL backFace=FALSE){ return glossiness; };	
	float GetSpecularLevel(int mtlNum=0, BOOL backFace=FALSE){ return specularLevel; };
	float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE){ return 0.0f; };

	Color GetAmbientClr(TimeValue t){ return pblock->GetColor(tl_ambient,t); }		
	Color GetDiffuseClr(TimeValue t){ return pblock->GetColor(tl_diffuse,t); }		
	Color GetSpecularClr(TimeValue t){ return pblock->GetColor(tl_specular,t);	}
	float GetGlossiness( TimeValue t){return pblock->GetFloat(tl_glossiness,t);  }		
	float GetSpecularLevel(TimeValue t){ return  pblock->GetFloat(tl_specular_level,t); }
	float GetSoftenLevel(TimeValue t){ return  0.0f; }
	float GetSelfIllum(TimeValue t){ return  pblock->GetFloat(tl_self_illum_amnt,t); }		
	Color GetSelfIllumClr(TimeValue t){ return  pblock->GetColor(tl_self_illum_color,t); }		
	void  computeTransmission( Color& outFilter,  Color& transluClr, Color& filterClr, float opacity, float shadowDist );

};

///////////// Class Descriptor ////////////////////////
class TranslucentShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new TranslucentShader(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_TRANSLUCENT); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return TranslucentShaderClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Translucent"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

TranslucentShaderClassDesc translucentCD;
ClassDesc * GetTranslucentShaderCD(){ return &translucentCD; }

// shader parameters
static ParamBlockDesc2 tl_param_blk ( translucent_params, _T("shaderParameters"),  0, &translucentCD, P_AUTO_CONSTRUCT, 0, 
	// params
	tl_ambient, _T("ambient"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_AMBIENT, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	tl_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_DIFFUSE, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	tl_specular, _T("specular"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_SPECULAR, 
		p_default, Color(.9f, 0.9f, 0.9f), 
		end,
	tl_filter, _T("filter"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_FILTER, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	tl_translucent_color, _T("translucentColor"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_TRANSLUCENT_CLR, 
		p_default, Color(0.0f, 0.0f, 0.0f), 
		end,
	tl_backside_specular, _T("backsideSpecular"), TYPE_BOOL,	0, IDS_KE_BACKSIDE_SPECULAR, 
		p_default, TRUE, 
		end,
	tl_ad_texlock, _T("adTextureLock"), TYPE_BOOL,	0, IDS_JW_ADTEXLOCK, 
		p_default, TRUE, 
		end,
	tl_ad_lock, _T("adLock"), TYPE_BOOL, 0, IDS_JW_ADLOCK, 
		p_default, TRUE, 
		end,
	tl_ds_lock, _T("dsLock"), TYPE_BOOL, 0, IDS_JW_DSLOCK, 
		p_default, FALSE, 
		end,
	tl_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0, IDS_JW_SELFILLUMCOLORON,
		p_default, TRUE, 
		end,
	tl_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	tl_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	tl_specular_level, _T("specularLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE,IDS_KE_SPEC_LEVEL,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	tl_glossiness, _T("glossiness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_GLOSSINESS,
		p_default,		30.0,
		p_range,		0.0, 100.0,
		end,
//	tl_soften, _T("soften"), TYPE_FLOAT, P_ANIMATABLE, IDS_DS_SOFTEN,
//		p_default,		0.0,
//		p_range,		0.0, 1.0,
//		end,
	tl_diffuse_level, _T("diffuseLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_DIFF_LEVEL,
		p_default,		100.0,
		p_range,		0.0, 400.0,
		end,
	end
	);

CombineComponentsFPDesc TranslucentShader::msExpMtlControlDesc(translucentCD);

TranslucentShader::TranslucentShader() 
{ 
	pblock = NULL; 
	translucentCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	paramDlg = NULL; 
 
	lockDS = selfIllumClrOn = FALSE;
	lockAD = lockADTex = backsideSpecular = TRUE;

	ambient = diffuse = specular = selfIllumClr 
		= translucentClr = filterClr =Color(0.0f,0.0f,0.0f);
	 
	glossiness = specularLevel =  diffLevel = selfIllum = 0.0f;
	curTime = 0;
	ivalid.SetEmpty(); 
}

void TranslucentShader::CopyStdParams( Shader* pFrom )
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
//		SetSoftenLevel( pFrom->GetSoftenLevel(0,0), curTime );
	macroRecorder->Enable();
	ivalid.SetEmpty();	
}


RefTargetHandle TranslucentShader::Clone( RemapDir &remap )
{
	TranslucentShader* mnew = new TranslucentShader();
	mnew->ExposureMaterialControl::operator=(*this);
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->ambient = ambient;
	mnew->diffuse = diffuse;
	mnew->specular = specular;
	mnew->filterClr = filterClr;
	mnew->translucentClr = translucentClr;
	mnew->backsideSpecular = backsideSpecular;
	mnew->glossiness = glossiness;
	mnew->specularLevel = specularLevel;
	mnew->diffLevel = diffLevel;
	mnew->selfIllum = selfIllum;
	mnew->selfIllumClr = selfIllumClr;
	mnew->selfIllumClrOn = selfIllumClrOn;
	mnew->lockDS = lockDS;
	mnew->lockAD = lockAD;
	mnew->lockADTex = lockADTex;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

#define	ID_DIFF_LEV		8
#define	ID_TRANS_CLR		9


void TranslucentShader::GetIllumParams(ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
//	ip.shFlags = selfIllumClrOn? SELFILLUM_CLR_ON : 0;
	ip.channels[ID_AM] = lockAD? diffuse : ambient;
	ip.channels[ID_DI] = diffuse;
	ip.channels[ID_FI] = filterClr;
	ip.channels[ID_SP] = lockDS? diffuse : specular;
	ip.channels[ID_SH].r = glossiness;
	ip.channels[ID_SS].r = specularLevel;
	if( selfIllumClrOn )
		ip.channels[ID_SI] = selfIllumClr;
	else
		ip.channels[ID_SI].r = ip.channels[ID_SI].g = ip.channels[ID_SI].b = selfIllum;
	ip.channels[ID_DIFF_LEV].r = diffLevel;
	ip.channels[ID_TRANS_CLR] = translucentClr;
}


#define LIMIT0_1(x) if (x < 0.0f) x = 0.0f; else if (x > 1.0f) x = 1.0f;
#define LIMITMINMAX(x, min, max) if (x < min) x = min; else if (x > max) x = max;

static Color LimitColor(Color c) {
	LIMIT0_1(c.r);
	LIMIT0_1(c.g);
	LIMIT0_1(c.b);
	return c;
}
static BOOL inUpdate = FALSE;

void TranslucentShader::Update(TimeValue t, Interval &valid) {
	Point3 p, p2;
	if( inUpdate )
		return;
	inUpdate = TRUE;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

//		pblock->GetValue( tl_ambient, t, p, ivalid );
//		ambient = LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( tl_diffuse, t, p, ivalid );
		diffuse= LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( tl_ambient, t, p2, ivalid );
		if( lockAD && (p!=p2)){
			pblock->SetValue( tl_ambient, t, diffuse);
			ambient = diffuse;
		} else {
			pblock->GetValue( tl_ambient, t, p2, ivalid );
			ambient = Bound(Color(p2.x,p2.y,p2.z));
		}
		pblock->GetValue( tl_specular, t, p2, ivalid );
		if( lockDS && (p!=p2)){
			pblock->SetValue( tl_specular, t, diffuse);
			specular = diffuse;
		} else {
			pblock->GetValue( tl_specular, t, p, ivalid );
			specular = Bound(Color(p.x,p.y,p.z));
		}
		pblock->GetValue( tl_translucent_color, t, p, ivalid );
		translucentClr = LimitColor(Color(p.x,p.y,p.z));

		pblock->GetValue( tl_filter, t, p, ivalid );
		filterClr = LimitColor(Color(p.x,p.y,p.z));

		pblock->GetValue( tl_glossiness, t, glossiness, ivalid );
		LIMIT0_1(glossiness);
		pblock->GetValue( tl_specular_level, t, specularLevel, ivalid );
		LIMITMINMAX(specularLevel,0.0f,9.99f);
//		pblock->GetValue( tl_soften, t, softThresh, ivalid); 
//		LIMIT0_1(softThresh);

		pblock->GetValue( tl_self_illum_amnt, t, selfIllum, ivalid );
		LIMIT0_1(selfIllum);
		pblock->GetValue( tl_self_illum_color, t, p, ivalid );
		selfIllumClr = LimitColor(Color(p.x,p.y,p.z));

		pblock->GetValue( tl_diffuse_level, t, diffLevel, ivalid );
		LIMITMINMAX(diffLevel,0.0f, 4.00f);
//		pblock->GetValue( tl_roughness, t, diffRough, ivalid );
//		LIMIT0_1(diffRough);
		// also get the non-animatables in case changed from scripter or other pblock accessors
		pblock->GetValue(tl_backside_specular, t, backsideSpecular, ivalid);
		pblock->GetValue(tl_ad_lock, t, lockAD, ivalid);
		pblock->GetValue(tl_ad_texlock, t, lockADTex, ivalid);
		pblock->GetValue(tl_use_self_illum_color, t, selfIllumClrOn, ivalid);

		curTime = t;
	}
	valid &= ivalid;
	inUpdate = FALSE;
}

void TranslucentShader::Reset()
{
//	TranslucentCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	ivalid.SetEmpty();
//	SetSoftenLevel(0.1f,0);
	SetAmbientClr(Color(0.5f, 0.5f, 0.5f),0);
	SetDiffuseClr(Color(0.5f,0.5f,0.5f),0);
	SetFilterClr(Color(0.5f,0.5f,0.5f),0);
	SetTranslucentClr(Color(0.0f,0.0f,0.0f),0);
	SetSpecularClr(Color(0.9f,0.9f,0.9f),0);
	SetGlossiness(.30f,0);   // change from .4, 5-21-97
	SetSpecularLevel(0.0f,0);   
	SetDiffuseLevel(1.0f,0);   
//	SetDiffuseRoughness(0.5f,0);   

	SetSelfIllum(.0f,0);
	SetSelfIllumClr( Color(.0f, .0f, .0f), 0 );
	SetSelfIllumClrOn( FALSE );
	SetLockADTex( TRUE );
	SetLockAD( TRUE );
	SetBacksideSpecular( TRUE );
	SetLockDS( FALSE );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020

// IO
IOResult TranslucentShader::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = CURRENT_TL_SHADER_VERSION;
	isave->Write(&version, sizeof(version), &nb);			
	isave->EndChunk();

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();
	return IO_OK;
}		

class TLShaderCB: public PostLoadCallback {
	public:
		TranslucentShader *s;
		int loadVersion;
	    TLShaderCB(TranslucentShader *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			int i = 0;
		}
};

IOResult TranslucentShader::Load(ILoad *iload) { 
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
			case SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK:
				res = ExposureMaterialControl::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < CURRENT_TL_SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new TLShaderCB(this, version));
		iload->SetObsolete();
	}

	return IO_OK;

}

			
///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//

// shade context execute functions for shadows
//
#define SC_SET_TRANSLUCENT_SHADOW	0x3000	// arg1 sets/resets the shadow translucent flag
#define SC_TRANSLUCENT_SHADOW		0x3001	// returns the shadowTranslucent flag
#define SC_SET_SELF_SHADOW_DISTANCE	0x4000	// arg1 (*float) sets/resets the self-shadow distance for translucent objects
#define SC_SELF_SHADOW_DISTANCE		0x4001	// returns arg1 (*float) the shadow distance for translucent objects

static int stopX = -1;
static int stopY = -1;

// bothsides included in translucency or only back side
static BOOL doOmni = TRUE;

// experimental extinction stuff
static float STEP_SZ = 1.0f;
static float DENSITY_COEFF = 0.01f * STEP_SZ;
static float defaultExtinctionDist = 80.0f;
static float defaultGain = 0.2f;
static float defaultTransmissionCoeff = 0.99f;
static float minTransmissionCoeff = 0.93f;
static float rangeTransmissionCoeff = (0.999f - minTransmissionCoeff);

#define MAX_OPAC		0.9f
#define SOLID_THRESHOLD	(1.0f - 0.003f)	// less than 1/255

// from schlick, gems 4, pp403
inline float fastGain( float a, float t )
{
	float b = 1.0f / a - 2.0f;
	b *= 1.0f - 2.0f * t;
	if( t < 0.5f ){
		return t / ( b + 1.0f );
	} else {
		return ( b - t ) / (b - 1.0f);
	}
}


void  TranslucentShader::computeTransmission( Color& outFilter, Color& transluClr, Color& filtClr, float opacity, float shadowDist )
{
	// Compute the attenuation of the light travelling shadowDist thru our material
	// only called if translucency on

	// shadowDist comes in negative, take abs value
	if( shadowDist < 0.0f )
		shadowDist = -shadowDist;

	// transmission coeff version
	double q = (double)( minTransmissionCoeff + Intens( transluClr ) * rangeTransmissionCoeff );
	double s = shadowDist;
	float atten = (float)pow( q, s);
	float spline = atten * atten * ( 3.0f - 2.0f * atten );

	outFilter.r = outFilter.g = outFilter.b = spline;

/************ s-curve version
//	float extinctionDist = (opacity > 0.01f)? defaultExtinctionDist / opacity : 1000.f;
	float extinctionDist = 40.0f + defaultExtinctionDist * Intens( transluClr );
	
	// get fraction of how far we are between the surface & the extinction dist
	// 0 ==> no attenuation, 1 ==> extinct, full attenuation
	float fraction;
	if( shadowDist > extinctionDist ) 
		fraction = 1.0f; // no light gets thru;
	else
	    fraction = shadowDist / extinctionDist;

	// now spline the fraction using 3x^2 - 2x^3
//	float spline = fraction * fraction * ( 3.0f - 2.0f * fraction );
	float spline = fastGain( defaultGain, fraction );

	outFilter.r = outFilter.g = outFilter.b = 1.0f - spline;
************/


/************* layered version
	// more opacity means more attenuation w/ distance
	// map opacity from 0..1 to 0..MAX_OPAC
	float effTransp = 1.0f - (MAX_OPAC * opacity * DENSITY_COEFF);
	float effOpac = 1.0f - effTransp;
	
	// composite a transparent item over an accumulating image repeatedly.
	// standard math for A over B  ==>  O = A * Aa + (1 - Aa) * B
	// if we assume A & B are premultiplied by their alphas, O = A + (1 - Aa)B
	// A over B over C ==>   O = A + (1 - Aa)( B + (1 - Ba)C ). if A==B==C
	// then : O = A + tA + ttA + tttA + .... where t = 1-Aa. factor the A out
	// to get: O = A ( 1 + t + t^2 + t^3 + ... )
	// in our case A is the effectiveOpacity & t is effectiveTransparency

	float series = 0.0f; // 1 + t + t^2 + t^3....
	float term = 1.0f;
	float d = 0.0f;
	// for each step thru the volume...
	while( d < shadowDist ){
		// attenuation should be exponential w/ distance, which arrives < 0
		series += term;
		term *= effTransp;
		if( (series * effOpac) >= SOLID_THRESHOLD )
			break; // full attenuation, no light can pass

		d += STEP_SZ; // next step toward the light
	}

	// filter color means more no attenuation w/ distance, just tint
	 outFilter = (1.0f - (series * effOpac)) * Color( 1.0f, 1.0f, 1.0f);
********/
}

static const int LIGHT_DESC_AMB_TRANSLUCENT = 0xd144;	// Tells whether the ambient light handle diffuse transmission
static const int LIGHT_DESC_INDIRECT_LIGHT = 0xd145;	// Tells whether the light is indirect illumination

void TranslucentShader::Illum(ShadeContext &sc, IllumParams &ip) 
{
	LightDesc *l;
	Color lightCol;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if ( sp.x == stopX && sp.y == stopY )
		sp.x = stopX;
#endif

	// Blinn style phong
	BOOL isShiny= (ip.channels[ID_SS].r > 0.0f) ? 1 : 0; 
	double phExp = 0.0;
	if (isShiny)
		phExp = pow(2.0, ip.channels[ID_SH].r * 10.0) * 4.0; 

	Color transClr = ip.channels[ID_TRANS_CLR];
	BOOL isTranslucent = ((transClr.r==0.0f) 
						&& (transClr.g==0.0f) 
						&& (transClr.b==0.0f) ) ? FALSE :TRUE;
	
	// set the sc to indicate we want translucent shadows
	int set = TRUE;
	sc.Execute( SC_SET_TRANSLUCENT_SHADOW, INT_PTR( &set ));
	Color filtClr = ip.channels[ID_FI];

	// for each light
	bool transAmbIllum = false;
	Color frontHemiIllumOut( 0,0,0 );
	Color rearHemiIllumOut( 0,0,0 );
	Color transAmbIllumOut( 0, 0, 0);
	for (int i=0; i < sc.nLights; i++) {
		l = sc.Light(i);
		float NL, kL;
		Point3 L, N;


		N = sc.Normal();
	
		// init shadow distance to 0
		float shadowDist = 0.0f;
		sc.Execute( SC_SET_SELF_SHADOW_DISTANCE, INT_PTR( &shadowDist ));

		// call the lights illuminate
		if (l->Illuminate(sc, N, lightCol, L, NL, kL)) {

			// ambient only
			if (l->ambientOnly || l->Execute(LIGHT_DESC_INDIRECT_LIGHT)) {
				if (l->ambientOnly)
					ip.ambIllumOut += lightCol;
				else
					ip.diffIllumOut += lightCol;
				sc.backFace = !sc.backFace;
				if (isTranslucent
						&& l->Execute(LIGHT_DESC_AMB_TRANSLUCENT)
						&& l->Illuminate(sc, -N, lightCol, L, NL, kL)) {
					transAmbIllum = true;
					transAmbIllumOut += lightCol;
				}
				sc.backFace = !sc.backFace;
				continue;
			}

			// diffuse
			if( NL > 0.0f && l->affectDiffuse )
				ip.diffIllumOut += kL * lightCol;

//#define DROPOFF_POWER	6.0f

			if( isTranslucent ){
				// schlicks approx cosine, gems 4, 386
				// using approx cos to a power to do translucent sharp falloff
				// as kL goes to zero, t stays close to 1 then down sharply
//				float t = 1.0f - kL;
//				t /= ( DROPOFF_POWER - DROPOFF_POWER * t + t );
//				frontHemiIllumOut += (1.0f - t) * lightCol;

				// get self-shadow distance
				sc.Execute( SC_SELF_SHADOW_DISTANCE, INT_PTR( &shadowDist ));
				Color transmitted(1.0f,1.0f,1.0f); 
				float kShadow = (NL != 0.0f)? kL / NL : 1.0f;
				if( shadowDist != 0.0f ){
					// self shadowing, dim lightcol by distance,opacity&filter...
					computeTransmission( transmitted, transClr, filtClr, ip.finalOpac, shadowDist );
				}

				frontHemiIllumOut += kShadow * transmitted * lightCol;
			}
		} else {
			if ( !transAmbIllum && isTranslucent ){
				if( l->Illuminate(sc,-N,lightCol,L,NL,kL)){
//					float t = 1.0f - kL;
//					t /= ( DROPOFF_POWER - DROPOFF_POWER * t + t );
//					rearHemiIllumOut += (1.0f - t) * lightCol;

					Color transmitted(1.0f,1.0f,1.0f); 
					float kShadow = (NL != 0.0f)? kL / NL : 1.0f;

					// get self-shadow distance
					sc.Execute( SC_SELF_SHADOW_DISTANCE, INT_PTR( &shadowDist ));
					if( shadowDist != 0.0f ){
						// self shadowing, dim lightcol by distance,opacity&filter...
						computeTransmission( transmitted, transClr, filtClr, ip.finalOpac, shadowDist );
					}

					rearHemiIllumOut += kShadow * transmitted * lightCol;
				}
			}
			continue;
		}

		Color spec;
		if (isShiny && l->affectSpecular 
			&& ( !sc.backFace || (sc.backFace && backsideSpecular)) ){

			Point3 H = Normalize(L-sc.V() ); // (L + -V)/2
			float c = DotProd(N, H);	 
			if (c>0.0f) {
//				if (softThresh != 0.0 && kL < softThresh) {
//					c *= Soften(kL/softThresh);
//				}

				c = (float)pow((double)c, phExp); // could use table lookup for speed
				spec = c * ip.channels[ID_SS].r * lightCol;
				ip.specIllumOut += spec;
			}
		}

	} // for each light

	// restore sc to not translucent
	set = FALSE;
	sc.Execute( SC_SET_TRANSLUCENT_SHADOW, INT_PTR( &set ));

	// Apply mono self illumination
	if ( ! selfIllumClrOn ){
		float si = 0.3333333f * (ip.channels[ID_SI].r + ip.channels[ID_SI].g + ip.channels[ID_SI].b);
		if ( si > 0.0f ) {
			si = Bound( si );
			ip.selfIllumOut = si * ip.channels[ID_DI];
			ip.diffIllumOut *= (1.0f - si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f-si;
		}
	} else {
		// colored self illum, 
		ip.selfIllumOut += ip.channels[ID_SI];
	}

	if (transAmbIllum)
		rearHemiIllumOut = transAmbIllumOut;

	if( isTranslucent ){
		if( doOmni ) {
			ip.transIllumOut = transClr * 
				(frontHemiIllumOut + rearHemiIllumOut - ip.diffIllumOut);	// remove diff so not counted twice
			if (getUseComposite())
				ip.transIllumOut *= (1.0f - ip.diffIllumOut);	// composite diff over, fade by 1-diff
		} else {
			// rear hemi only
			ip.transIllumOut = transClr * rearHemiIllumOut;
			if (getUseComposite())
				ip.transIllumOut *= (1.0f - ip.diffIllumOut);	// composite diff over, fade by 1-diff
		}
	}
	
	// now we can multiply by the clrs
	ip.diffIllumOut *= ip.channels[ID_DI] * ip.channels[ID_DIFF_LEV].r; 
	ip.diffIllumIntens = Intens( ip.diffIllumOut );
	ip.specIllumOut *= ip.channels[ID_SP]; 
	ip.ambIllumOut *= ip.channels[ID_AM]; 

	// do refraction & transparency
	if ( (ip.hasComponents & HAS_REFRACT) ){
		// Set up attenuation opacity for Refraction map. dim diffuse & spec by this
		ip.finalAttenuation = ip.finalOpac * (1.0f - ip.refractAmt);   

		// Make more opaque where specular hilite occurs:
		float max = Max(ip.specIllumOut);
		if (max > 1.0f) max = 1.0f; 
		float newOpac = ip.finalOpac + max - ip.finalOpac * max;

		// Evaluate refraction map, filtered by filter color.
		ip.transIllumOut += ip.channels[ ip.stdIDToChannel[ ID_RR ] ]
			* transpColor(TRANSP_FILTER, newOpac, ip.channels[ID_FI], ip.channels[ID_DI]);

		// no transparency when doing refraction
		ip.finalT.Black();

	} else { 
		// no refraction, transparent?
		// std opacity & compositing
		if ( (ip.hasComponents & HAS_OPACITY) ) {
			ip.finalAttenuation = ip.finalOpac;

			// Make more opaque where specular hilite occurs, so you
			// can still see the hilite:
			float max = Max(ip.specIllumOut);
			if (max > 1.0f) max = 1.0f; 
			float newOpac = ip.finalOpac + max - ip.finalOpac*max;

			// Compute the color of the transparent filter color
			ip.finalT = transpColor(TRANSP_FILTER, newOpac, ip.channels[ID_FI], ip.channels[ID_DI]);
//			if( isTranslucent ){
//				Color tClr = ip.channels[ID_FI] * ip.finalOpac;
//				ip.transIllumOut *= tClr; 
//			}

		} else {
			// opaque, no Refraction map
			ip.finalT.Black();
			ip.finalAttenuation = 1.0f;
			// note we even multiply tranlucency by filter in the opaque case,
			// for consistency under animation
//			if( isTranslucent ){
//				ip.transIllumOut *= ip.channels[ID_FI];
//			}
		}
	}

	// use the standard reflection component
	ShadeReflection( sc, ip, ip.channels[ip.stdIDToChannel[ ID_RL ]] ); 

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

///////////////////////////////////////////////////////////////////
//
//	Translucent shader dlg panel
//

// The dialog class
class TranslucentShaderDlg : public ShaderParamDlg {
public:
	TranslucentShader*	pShader;
	StdMat2*	pMtl;
	HPALETTE	hOldPal;
	HWND		hwmEdit;	// window handle of the materials editor dialog
	IMtlParams*	pMtlPar;
	HWND		hwHilite;   // the hilite window
	HWND		hRollup;	// Rollup panel
	TimeValue	curTime;
	BOOL		valid;
	BOOL		isActive;

	IColorSwatch *cs[NCOLBOX];
//	ISpinnerControl *softSpin;
	ISpinnerControl *shSpin, *ssSpin, *siSpin, *trSpin;
	ISpinnerControl *dlevSpin; //, *roughSpin; //, *rhoSpin;
	ICustButton* texMBut[NMBUTS];
	TexDADMgr dadMgr;
	
	TranslucentShaderDlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~TranslucentShaderDlg(); 

	// required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		for (long i=0; i<NMBUTS; i++) {
			if (hw == texMBut[i]->GetHwnd()) 
				return nTexmapFromMBut[i];
		}	
		return -1;
	}

	// Methods
	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(){ return TranslucentShaderDlgClassID; }
	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (TranslucentShader*)theShader; 
		if(pShader) pShader->SetParamDlg(this,0);
		pMtl = theMtl;
	}

	ReferenceTarget* GetThing(){ return pMtl; } // mtl is the thing! used by DAD...
	Shader* GetShader(){ return pShader; }
	void SetTime(TimeValue t) {
		//DS 2/26/99: added interval test to prevent redrawing when not necessary
		curTime = t; 
		if (!pShader->ivalid.InInterval(t)) {
			Interval v;
			pShader->Update(t,v);
			LoadDialog(TRUE); 
		}
	}		
	BOOL KeyAtCurTime(int id) { return pShader->KeyAtTime(id,curTime); } 
	void DeleteThis() { delete this; }
	void ActivateDlg( BOOL dlgOn ){ isActive = dlgOn; }
	HWND GetHWnd(){ return hRollup; }
	void NotifyChanged(){ pShader->NotifyChanged(); }
	void LoadDialog(BOOL draw);
	void ReloadDialog(){ Interval v; pShader->Update(pMtlPar->GetTime(), v); LoadDialog(FALSE);}
	void UpdateDialog( ParamID paramId ){ ReloadDialog(); }

	void UpdateMtlDisplay(){ pMtlPar->MtlChanged(); } // redraw viewports
    void UpdateHilite( );
	void UpdateColSwatches();
	void UpdateLockADTex(BOOL passOn);
	void UpdateMapButtons();
	void UpdateOpacity();

	void SetLockDS(BOOL lock);
	void SetLockAD(BOOL lock);
	void SetLockADTex(BOOL lock);

	void SelectEditColor(int i) { cs[ i ]->EditThis(FALSE); }
};

static INT_PTR CALLBACK  TranslucentShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	TranslucentShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (TranslucentShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (TranslucentShaderDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* TranslucentShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	TranslucentShaderDlg *pDlg = new TranslucentShaderDlg(hwMtlEdit, imp);
	pDlg->SetThings( theMtl, this  );

	LoadStdShaderResources();
	if ( hOldRollup ) {
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_TRANSLUCENT3),
			TranslucentShaderDlgProc, 
			GetString(IDS_KE_TRANSLUCENT_BASIC),	
			(LPARAM)pDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);
	} else
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_TRANSLUCENT3),
			TranslucentShaderDlgProc, 
			GetString(IDS_KE_TRANSLUCENT_BASIC),	
			(LPARAM)pDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)pDlg;	
}

RefResult TranslucentShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
									  PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (hTarget == pblock){
				// update UI if paramblock changed, possibly from scripter
				ParamID changingParam = pblock->LastNotifyParamID();
				// reload the dialog if present
				if (paramDlg){
					paramDlg->UpdateDialog( changingParam );
				}
			}
			break;
	}
	return(REF_SUCCEED);
}

TranslucentShaderDlg::TranslucentShaderDlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	shSpin = ssSpin = siSpin = trSpin = NULL;
	dlevSpin = NULL; 
	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();
	isActive = valid = FALSE;

	for( long i = 0; i < NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

TranslucentShaderDlg::~TranslucentShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}

	for (i=0; i<NCOLBOX; i++)
		if (cs[i]) ReleaseIColorSwatch(cs[i]); // mjm - 5.10.99
	
 	ReleaseISpinner(shSpin);
	ReleaseISpinner(ssSpin);
	ReleaseISpinner(siSpin);
	ReleaseISpinner(dlevSpin);
	ReleaseISpinner(trSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite, GWLP_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}

void  TranslucentShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		shSpin->SetValue(FracToPc(pShader->GetGlossiness()),FALSE);
		shSpin->SetKeyBrackets(KeyAtCurTime(tl_glossiness));

		ssSpin->SetValue(FracToPc(pShader->GetSpecularLevel()),FALSE);
		ssSpin->SetKeyBrackets(KeyAtCurTime(tl_specular_level));

//		softSpin->SetValue(pShader->GetSoftenLevel(),FALSE);
//		softSpin->SetKeyBrackets(KeyAtCurTime(tl_soften));

		trSpin->SetValue(FracToPc(pMtl->GetOpacity( curTime )),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		dlevSpin->SetValue(FracToPc(pShader->GetDiffuseLevel()),FALSE);
		dlevSpin->SetKeyBrackets(KeyAtCurTime(tl_diffuse_level));

//		roughSpin->SetValue(FracToPc(pShader->GetDiffuseRoughness()),FALSE);
//		roughSpin->SetKeyBrackets(KeyAtCurTime(tl_roughness));

		CheckButton(hRollup, IDC_LOCK_AD, pShader->GetLockAD() );
		CheckButton(hRollup, IDC_LOCK_DS, pShader->GetLockDS() );
	 	UpdateLockADTex( FALSE ); //don't send to mtl
		SetCheckBox(hRollup,IDC_BACKSIDE_SPECULAR, pShader->GetBacksideSpecular() ); 

		BOOL colorSelfIllum = pShader->IsSelfIllumClrOn();
		SetCheckBox(hRollup,IDC_SI_COLORON, colorSelfIllum ); 
		if( colorSelfIllum ) {
//			ShowWindow( siSpin->GetHwnd(), SW_HIDE );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_EDIT), SW_HIDE );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_SPIN), SW_HIDE );

			ShowWindow( cs[4]->GetHwnd(), SW_SHOW );
		} else {
			// disable the color swatch
			ShowWindow( cs[4]->GetHwnd(), SW_HIDE );
			// show self-illum slider
//			ShowWindow( siSpin->GetHwnd(), SW_SHOW );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_EDIT), SW_SHOW );
			ShowWindow( GetDlgItem(hRollup, IDC_SI_SPIN), SW_SHOW );

			siSpin->SetValue(FracToPc(pShader->GetSelfIllum()), FALSE);
			siSpin->SetKeyBrackets(KeyAtCurTime(tl_self_illum_amnt));
		}

		UpdateColSwatches();
		UpdateHilite();
	}
}


void TranslucentShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void TranslucentShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = nTexmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm	 = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}



void TranslucentShaderDlg::SetLockAD(BOOL lock)
{
	if (lock) {
		if (IDYES!=MessageBox(hwmEdit, GetString(IDS_DS_LOCKAD), GetString(IDS_DS_LOCKCOL), MB_YESNO)) {
			CheckButton(hRollup, IDC_LOCK_AD, FALSE);	
			return;	
		}
		// set ambient color to diffuse
		pShader->SetAmbientClr( pShader->GetDiffuseClr(), 0 );
		UpdateColSwatches();
	}
	pShader->SetLockAD(lock);
}


void TranslucentShaderDlg::UpdateColSwatches() 
{
	cs[0]->SetKeyBrackets( pShader->KeyAtTime( colParamID[0], curTime) );
	cs[0]->SetColor( pShader->GetAmbientClr(0,0) );

	cs[1]->SetKeyBrackets( pShader->KeyAtTime( colParamID[1], curTime) );
	cs[1]->SetColor( pShader->GetDiffuseClr(0,0) );
	
	cs[2]->SetKeyBrackets( pShader->KeyAtTime( colParamID[2], curTime) );
	cs[2]->SetColor( pShader->GetSpecularClr(0,0) );
	
	cs[3]->SetKeyBrackets( pShader->KeyAtTime( colParamID[3], curTime) );
	cs[3]->SetColor( pShader->GetFilterClr(0,0) );

	cs[4]->SetKeyBrackets( pShader->KeyAtTime( colParamID[4], curTime) );
	cs[4]->SetColor( pShader->GetSelfIllumClr(0,0) );
		
	cs[5]->SetKeyBrackets( pShader->KeyAtTime( colParamID[5], curTime) );
	cs[5]->SetColor( pShader->GetTranslucentClr(0,0) );
}


void TranslucentShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}

void TranslucentShaderDlg::UpdateLockADTex( BOOL passOn) 
{
	int lock = 	pShader->GetLockADTex();
	CheckButton(hRollup, IDC_LOCK_ADTEX, lock);

	ShowWindow(GetDlgItem(hRollup, IDC_MAPON_AM), !lock);
	texMBut[ 0 ]->Enable(!lock);

	if ( passOn ) 
		pMtl->SyncADTexLock( lock );

//	UpdateMtlDisplay();
}

void TranslucentShaderDlg::SetLockADTex(BOOL lock) 
{
	pShader->SetLockADTex( lock );
	UpdateLockADTex(TRUE); // passon to mtl
//	UpdateMtlDisplay();
	}

void TranslucentShaderDlg::SetLockDS(BOOL lock) 
{
	if (lock) {
		if (IDYES!=MessageBox(hwmEdit, GetString(IDS_KE_LOCKDS),GetString(IDS_DS_LOCKCOL), MB_YESNO)) {
			CheckButton(hRollup, IDC_LOCK_DS, FALSE);	
			return;	
		}
		pShader->SetSpecularClr( pShader->GetDiffuseClr(), 0 );
		UpdateColSwatches();
	}
	pShader->SetLockDS( lock );
}

static int ColorIDCToIndex(int id) {
	switch (id) {
		case IDC_AMBIENT_COLOR: return 0;
		case IDC_DIFFUSE_COLOR: return 1;
		case IDC_SPECULAR_COLOR: return 2;
		case IDC_FILTER_COLOR: return 3;
		case IDC_SELFILLUM_COLOR: return 4;
		case IDC_TRANSLUCENT_COLOR: return 5;
		default: return 0;
	}
}


BOOL TranslucentShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG:
			{
			
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

			cs[0] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_AMBIENT_COLOR), pShader->GetAmbientClr(0,0), GetString(IDS_DS_AMBIENT) );
			cs[1] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_DIFFUSE_COLOR), pShader->GetDiffuseClr(0,0), GetString(IDS_DS_DIFFUSE) );
			cs[2] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SPECULAR_COLOR), pShader->GetSpecularClr(0,0), GetString(IDS_DS_SPECULAR) );
			cs[3] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_FILTER_COLOR), pShader->GetFilterClr(0,0), GetString(IDS_DS_SELFI) );
			cs[4] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SELFILLUM_COLOR), pShader->GetSelfIllumClr(0,0), GetString(IDS_DS_FILTER) );
			cs[5] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_TRANSLUCENT_COLOR), pShader->GetTranslucentClr(0,0), GetString(IDS_KE_TRANSLUCENT_CLR) );

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)HiliteWndProc);

			shSpin = SetupIntSpinner(hwndDlg, IDC_SH_SPIN, IDC_SH_EDIT, 0,100, 0);
			ssSpin = SetupIntSpinner(hwndDlg, IDC_SS_SPIN, IDC_SS_EDIT, 0,999, 0);
			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);
			dlevSpin = SetupIntSpinner(hwndDlg, IDC_DIFFLEV_SPIN, IDC_DIFFLEV_EDIT, 0, 400, 0);

			for (int j=0; j<NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
			}

			SetupLockButton(hwndDlg,IDC_LOCK_AD,FALSE);
			SetupLockButton(hwndDlg,IDC_LOCK_DS,FALSE);
			SetupPadLockButton(hwndDlg,IDC_LOCK_ADTEX, TRUE);

			siSpin = SetupIntSpinner(hwndDlg, IDC_SI_SPIN, IDC_SI_EDIT, 0,100, 0);
			
			if( pShader->IsSelfIllumClrOn() ) {
				// enable the color swatch, disable the spinner
				ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_HIDE );
				ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_HIDE );
			} else {
				// disable the color swatch
				ShowWindow( cs[4]->GetHwnd(), SW_HIDE );
			}
			LoadDialog(TRUE);
		}
		break;

		case WM_COMMAND: 
			{
			for ( int i=0; i<NMBUTS; i++) {
				if (id == texMButtonsIDC[i]) {
					PostMessage(hwmEdit,WM_TEXMAP_BUTTON, nTexmapFromMBut[i],(LPARAM)pMtl );
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

				case IDC_BACKSIDE_SPECULAR:{
					int isOn = GetCheckBox(hwndDlg, IDC_BACKSIDE_SPECULAR );
					pShader->SetBacksideSpecular( isOn );			
//				    NotifyChanged();
				}

				case IDC_SI_COLORON:{
					int isOn = GetCheckBox(hwndDlg, IDC_SI_COLORON );
					pShader->SetSelfIllumClrOn( isOn );			
					if ( isOn ) {
						// enable the color swatch, disable the spinner
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_HIDE );
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_HIDE );
						ShowWindow( cs[4]->GetHwnd(), SW_SHOW );
					} else {
						// disable the color swatch
						ShowWindow( cs[4]->GetHwnd(), SW_HIDE );
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_SHOW );
						ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_SHOW );
					}
				    NotifyChanged();
				}
				break;
			}
			break;
		case CC_COLOR_SEL: {
			int id = LOWORD(wParam);
			SelectEditColor(ColorIDCToIndex(id));
		}			
		break;
		case CC_COLOR_DROP:	{
			int id = LOWORD(wParam);
			SelectEditColor(ColorIDCToIndex(id));
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
		case CC_COLOR_CHANGE: {			
			int id = LOWORD(wParam);
			int buttonUp = HIWORD(wParam); 
			int n = ColorIDCToIndex(id);
			if (buttonUp) theHold.Begin();
			Color curColor(cs[n]->GetColor());
			switch( n ) {
			case 0: pShader->SetAmbientClr(curColor, curTime);
					if ( pShader->GetLockAD() )
							pShader->SetDiffuseClr(curColor, curTime);
					break;
			case 1: pShader->SetDiffuseClr(curColor, curTime); break;
			case 2: pShader->SetSpecularClr(curColor, curTime); break;
			case 3: pShader->SetFilterClr(curColor, curTime); break;
			case 4: pShader->SetSelfIllumClr(curColor, curTime); break;
			case 5: pShader->SetTranslucentClr(curColor, curTime); break;
			}
//			SetMtlColor(n, curColor, pShader, cs, curTime);
			if (buttonUp) {
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
				// DS: 5/11/99-  this was commented out. I put it back in, because
				// it is necessary for the Reset button in the color picker to 
				// update the viewport.				
				UpdateMtlDisplay();  
				}
		} break;
		case WM_PAINT: 
			if (!valid) {
				valid = TRUE;
				ReloadDialog();
				}
			return FALSE;
		case WM_CLOSE:
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
//				case IDC_SOFT_SPIN: 
//					pShader->SetSoftenLevel(softSpin->GetFVal(),curTime); 
//					break;
				case IDC_SI_SPIN: 
					pShader->SetSelfIllum(PcToFrac(siSpin->GetIVal()),curTime); 
					break;
				case IDC_DIFFLEV_SPIN: 
					pShader->SetDiffuseLevel(PcToFrac(dlevSpin->GetIVal()),curTime); 
					break;
//				case IDC_DIFFROUGH_SPIN: 
//					pShader->SetDiffuseRoughness(PcToFrac(roughSpin->GetIVal()),curTime); 
//					break;
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


