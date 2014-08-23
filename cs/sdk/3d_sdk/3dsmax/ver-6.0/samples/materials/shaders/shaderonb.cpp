//////////////////////////////////////////////////////////////////////////////
//
//		Oren-Nayar Shader plug-in, implementation
//
//		Created: 9/23/98 Kells Elmquist
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "macrorec.h"
#include "toneop.h"

#define NEWSHADERS_CLASS_ID		0x2857f420

static Class_ID OrenNayarBlinnShaderClassID( NEWSHADERS_CLASS_ID+1, 0);
static Class_ID OrenNayarShaderDlgClassID( NEWSHADERS_CLASS_ID+2, 0);

// paramblock2 block and parameter IDs.
enum { onb_params, };
// shdr_params param IDs
enum 
{ 
	onb_ambient, onb_diffuse, onb_specular,
	onb_self_illum_color, onb_self_illum_amnt,
	onb_glossiness, onb_specular_level, onb_soften,
	onb_diffuse_level, onb_roughness, 
	onb_ad_texlock, onb_ad_lock, onb_ds_lock, onb_use_self_illum_color, 
};


/////////////////////////////////////////////////////////////////////
//
//	Oren/Nayar/Blinn Basic Panel UI 
//
#define NMBUTS 9
#define N_SI_BUT 5
#define N_TR_BUT 6

static int texMButtonsIDC[] = {
	IDC_MAPON_AM,	IDC_MAPON_DI,	IDC_MAPON_SP,	IDC_MAPON_SH,
	IDC_MAPON_SS,	IDC_MAPON_SI,	IDC_MAPON_TR, 
	IDC_MAPON_DIFF_LEVEL,	IDC_MAPON_DIFF_ROUGH,	IDC_MAPON_DIFF_RHO, 
	};
		
// This array gives the texture map number for given MButton number								
// the ID_s are from stdmat.h
static int texmapFromMBut[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9 };

// channel names
#define ON_NTEXMAPS	10

static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, IDS_DS_SHININESS, IDS_DS_SHIN_STR,
	IDS_KE_SELFILLUM, IDS_DS_TRANS, IDS_DS_FILTER, 
	IDS_KE_DIFF_LEVEL, IDS_KE_DIFF_ROUGH, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
};	

// internal non-local parsable channel map names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("ambientMap"), _T("diffuseMap"),	_T("specularMap"), _T("glossinessMap"), 
	_T("specularLevelMap"), _T("selfIllumMap"), _T("opacityMap"), _T("filterMap"),
	_T("diffuseLevelMap"), _T("diffuseRoughnessMap"), _T(""), _T(""),	
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),
};	


// sized for nmax textures
static int channelType[] = {
	CLR_CHANNEL, CLR_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, MONO_CHANNEL,
	CLR_CHANNEL, MONO_CHANNEL, CLR_CHANNEL, 
	MONO_CHANNEL, MONO_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
};
	

// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { 0, 1, 2, 3, 4, 5, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1 };

//////////////////////////////////////////////////////////////////////////////////////////
//
//		Oren / Nayar / Blinn Parameter Block
//
#define CURRENT_ON_SHADER_VERSION	2
#define ON_SHADER_NPARAMS			11
#define ON_SHADER_PB_VERSION		1

#define NCOLBOX 4
static int colID[NCOLBOX] = { IDC_STD_COLOR1, IDC_STD_COLOR2, IDC_STD_COLOR3, IDC_SI_COLOR };
static int colParamID[NCOLBOX] = { onb_ambient, onb_diffuse, onb_specular, onb_self_illum_color };
#define N_SI_CLR		3
#define N_AMB_CLR		0

#define PB_AMBIENT_CLR		0
#define PB_DIFFUSE_CLR		1
#define PB_SPECULAR_CLR		2
#define PB_SELFILLUM_CLR	3
#define PB_SELFILLUM		4
#define PB_GLOSSINESS 		5
#define PB_SPECULAR_LEV		6
#define PB_SOFTEN_LEV		7
#define PB_DIFFUSE_LEV		8
#define PB_DIFFUSE_ROUGH	9
#define PB_DIFFUSE_RHO		10

// v1 Param Block Descriptor
static ParamBlockDescID ONShaderPB[ ON_SHADER_NPARAMS ] = {
	{ TYPE_RGBA,  NULL, TRUE, onb_ambient },		// ambient
	{ TYPE_RGBA,  NULL, TRUE, onb_diffuse },		// diffuse
	{ TYPE_RGBA,  NULL, TRUE, onb_specular },		// specular
	{ TYPE_RGBA,  NULL, TRUE, onb_self_illum_color }, // self-illum color
	{ TYPE_FLOAT, NULL, TRUE, onb_self_illum_amnt },  // selfIllum
	{ TYPE_FLOAT, NULL, TRUE, onb_glossiness },		// glossiness
	{ TYPE_FLOAT, NULL, TRUE, onb_specular_level },		// specularLevel
	{ TYPE_FLOAT, NULL, TRUE, onb_soften },			// soften
	{ TYPE_FLOAT, NULL, TRUE, onb_roughness },		// diffuse roughness
	{ TYPE_FLOAT, NULL, TRUE, -1 },					// diffuse reflectivity
	{ TYPE_FLOAT, NULL, TRUE, onb_diffuse_level },  // diffuse level
}; 

#define ON_NUMOLDVER 1

static ParamVersionDesc oldVersions[ON_NUMOLDVER] = {
	ParamVersionDesc(ONShaderPB, 8, 0),
};

static ParamVersionDesc curVersion(ONShaderPB, ON_SHADER_NPARAMS, ON_SHADER_PB_VERSION);

//----------------------------------------------------------------------------------------
//---- Oren-Nayar Extended Diffuse Shader, w/ Blinn specular hilite ----------------------
//----------------------------------------------------------------------------------------

class OrenNayarShaderDlg;

class OrenNayarBlinnShader : public ExposureMaterialControlImp<OrenNayarBlinnShader, CombineComponentsCompShader> {
friend class ONShaderCB;
friend class OrenNayarShaderDlg;
friend class ExposureMaterialControlImp<OrenNayarBlinnShader, CombineComponentsCompShader>;
protected:
	IParamBlock2 *pblock;   // ref 0
	Interval ivalid;
	TimeValue	curTime;

	OrenNayarShaderDlg* paramDlg;

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

	float diffLevel;
	float diffRough;
	float diffRho;

	static CombineComponentsFPDesc msExpMtlControlDesc;

public:
	OrenNayarBlinnShader();
	void DeleteThis(){ delete this; }		
    ULONG SupportStdParams(){ return STD_ONB | STD_EXTRA; }
    void CopyStdParams( Shader* pFrom );

	// texture maps
	long  nTexChannelsSupported(){ return ON_NTEXMAPS; }
	TSTR  GetTexChannelName( long nTex ) { return GetString( texNameIDS[ nTex ] ); }
	TSTR  GetTexChannelInternalName( long nTex ) { return texInternalNames[ nTex ]; }
	long  ChannelType( long nTex ){ return channelType[nTex]; }
	// map StdMat Channel ID's to the channel number
	long StdIDToChannel( long stdID ){ return stdIDToChannel[ stdID ]; }

	BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime((ParamID)id,t); }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int ){ paramDlg = (OrenNayarShaderDlg*)newDlg; }

	Class_ID ClassID() { return OrenNayarBlinnShaderClassID; }
	TSTR GetName() { return GetString( IDS_KE_OREN_NAYAR_BLINN ); }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i){ return (i==0)? pblock : NULL; }
	TSTR SubAnimName(int i){ return TSTR(GetString( IDS_KE_OREN_PARMS )); };
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

	// OrenNayar Specific 
	void SetDiffuseLevel(float v, TimeValue t)		
			{ diffLevel = v; pblock->SetValue( onb_diffuse_level, t, v); }
	float GetDiffuseLevel(int mtlNum=0, BOOL backFace=FALSE){ return diffLevel; };
	float GetDiffuseLevel(TimeValue t){ return  pblock->GetFloat(onb_diffuse_level,t); }
	void SetDiffuseRoughness(float v, TimeValue t)		
			{ diffRough = v; pblock->SetValue( onb_roughness, t, v); }
	float GetDiffuseRoughness(int mtlNum=0, BOOL backFace=FALSE){ return diffRough; };
	float GetDiffuseRoughness(TimeValue t){ return  pblock->GetFloat(onb_roughness, t); }

	// Std Params
	void SetLockDS(BOOL lock){ lockDS = lock; pblock->SetValue( onb_ds_lock, 0, lock); }
	BOOL GetLockDS(){ return lockDS; }
	void SetLockAD(BOOL lock){ lockAD = lock; pblock->SetValue( onb_ad_lock, 0, lock); }
	BOOL GetLockAD(){ return lockAD; }
	void SetLockADTex(BOOL lock){ lockADTex = lock; pblock->SetValue( onb_ad_texlock, 0, lock); }
	BOOL GetLockADTex(){ return lockADTex; }

	void SetSelfIllum(float v, TimeValue t)
		{ selfIllum = v; pblock->SetValue( onb_self_illum_amnt, t, v); }
	void SetSelfIllumClrOn( BOOL on ){ selfIllumClrOn = on; pblock->SetValue( onb_use_self_illum_color, 0, on); };
	BOOL IsSelfIllumClrOn(){ return selfIllumClrOn; };
	void SetSelfIllumClr(Color c, TimeValue t)
		{ selfIllumClr = c; pblock->SetValue( onb_self_illum_color, t, Point3(c.r,c.g,c.b) ); }

	void SetAmbientClr(Color c, TimeValue t)		
		{ ambient = c; pblock->SetValue( onb_ambient, t, c); }
	void SetDiffuseClr(Color c, TimeValue t)		
		{ diffuse = c; pblock->SetValue( onb_diffuse, t, c); }
	void SetSpecularClr(Color c, TimeValue t)
		{ specular = c; pblock->SetValue( onb_specular, t, c); }
	void SetGlossiness(float v, TimeValue t)		
		{ glossiness= v; pblock->SetValue( onb_glossiness, t, v); }
	void SetSpecularLevel(float v, TimeValue t)		
		{ specularLevel = v; pblock->SetValue( onb_specular_level, t, v); }
	void SetSoftenLevel(float v, TimeValue t) 
		{ softThresh = v; pblock->SetValue( onb_soften, t, v); }

	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return selfIllumClrOn; };
	Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE){ return ambient;}		
    Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE){ return diffuse;}		
	Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE){ return specular; };
	Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE){ return selfIllumClr; };
	float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE){ return selfIllum; };
	float GetGlossiness(int mtlNum=0, BOOL backFace=FALSE){ return glossiness; };	
	float GetSpecularLevel(int mtlNum=0, BOOL backFace=FALSE){ return specularLevel; };
	float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE){ return softThresh; };

	Color GetAmbientClr(TimeValue t){ return pblock->GetColor(onb_ambient,t); }		
	Color GetDiffuseClr(TimeValue t){ return pblock->GetColor(onb_diffuse,t); }		
	Color GetSpecularClr(TimeValue t){ return pblock->GetColor(onb_specular,t);	}
	float GetGlossiness( TimeValue t){return pblock->GetFloat(onb_glossiness,t);  }		
	float GetSpecularLevel(TimeValue t){ return  pblock->GetFloat(onb_specular_level,t); }
	float GetSoftenLevel(TimeValue t){ return  pblock->GetFloat(onb_soften,t); }
	float GetSelfIllum(TimeValue t){ return  pblock->GetFloat(onb_self_illum_amnt,t); }		
	Color GetSelfIllumClr(TimeValue t){ return  pblock->GetColor(onb_self_illum_color,t); }		
};

///////////// Class Descriptor ////////////////////////
class OrenNayarBlinnShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new OrenNayarBlinnShader(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_OREN_NAYAR_BLINN); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return OrenNayarBlinnShaderClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("OrenNayarBlinn"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
};

OrenNayarBlinnShaderClassDesc orenNayarBlinnCD;
ClassDesc * GetOrenNayarBlinnShaderCD(){ return &orenNayarBlinnCD; }

// shader parameters
static ParamBlockDesc2 onb_param_blk ( onb_params, _T("shaderParameters"),  0, &orenNayarBlinnCD, P_AUTO_CONSTRUCT, 0, 
	// params
	onb_ambient, _T("ambient"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_AMBIENT, 
		p_default, Color(0, 0, 0), 
		end,
	onb_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_DIFFUSE, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	onb_specular, _T("specular"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_SPECULAR, 
		p_default, Color(1.0f, 1.0f, 1.0f), 
		end,
	onb_ad_texlock, _T("adTextureLock"), TYPE_BOOL,	0, IDS_JW_ADTEXLOCK, 
		p_default, TRUE, 
		end,
	onb_ad_lock, _T("adLock"), TYPE_BOOL, 0, IDS_JW_ADLOCK, 
		p_default, TRUE, 
		end,
	onb_ds_lock, _T("dsLock"), TYPE_BOOL, 0, IDS_JW_DSLOCK, 
		p_default, FALSE, 
		end,
	onb_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0, IDS_JW_SELFILLUMCOLORON,
		p_default, TRUE, 
		end,
	onb_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	onb_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	onb_specular_level, _T("specularLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE,IDS_KE_SPEC_LEVEL,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	onb_glossiness, _T("glossiness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_GLOSSINESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	onb_soften, _T("soften"), TYPE_FLOAT, P_ANIMATABLE, IDS_DS_SOFTEN,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		end,
	onb_diffuse_level, _T("diffuseLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_DIFF_LEVEL,
		p_default,		100.0,
		p_range,		0.0, 400.0,
		end,
	onb_roughness, _T("diffuseRoughness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_DIFF_ROUGH,
		p_default,		50.0,
		p_range,		0.0, 100.0,
		end,
	end
	);

CombineComponentsFPDesc OrenNayarBlinnShader::msExpMtlControlDesc(orenNayarBlinnCD);

OrenNayarBlinnShader::OrenNayarBlinnShader() 
{ 
	pblock = NULL; 
	orenNayarBlinnCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	paramDlg = NULL; 
 
	lockDS = selfIllumClrOn = FALSE;
	lockAD = lockADTex = TRUE;

	ambient = diffuse = specular = selfIllumClr = Color(0.0f,0.0f,0.0f);
	glossiness = specularLevel = softThresh 
		= diffRough = diffRho =  diffLevel = selfIllum = 0.0f;
	curTime = 0;
	ivalid.SetEmpty(); 
}

void OrenNayarBlinnShader::CopyStdParams( Shader* pFrom )
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


RefTargetHandle OrenNayarBlinnShader::Clone( RemapDir &remap )
{
	OrenNayarBlinnShader* mnew = new OrenNayarBlinnShader();
	mnew->ExposureMaterialControl::operator=(*this);
	mnew->ReplaceReference(0,remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->ambient = ambient;
	mnew->diffuse = diffuse;
	mnew->specular = specular;
	mnew->glossiness = glossiness;
	mnew->specularLevel = specularLevel;
	mnew->diffLevel = diffLevel;
	mnew->diffRough = diffRough;
	mnew->diffRho = diffRho;
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

#define	ID_DIFF_LEV		8
#define	ID_DIFF_ROUGH	9


void OrenNayarBlinnShader::GetIllumParams(ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
//	ip.shFlags = selfIllumClrOn? SELFILLUM_CLR_ON : 0;
	ip.channels[ID_AM] = lockAD? diffuse : ambient;
	ip.channels[ID_DI] = diffuse;
	ip.channels[ID_SP] = lockDS? diffuse : specular;
	ip.channels[ID_SH].r = glossiness;
	ip.channels[ID_SS].r = specularLevel;
	if( selfIllumClrOn )
		ip.channels[ID_SI] = selfIllumClr;
	else
		ip.channels[ID_SI].r = ip.channels[ID_SI].g = ip.channels[ID_SI].b = selfIllum;
	ip.channels[ID_DIFF_LEV].r = diffLevel;
	ip.channels[ID_DIFF_ROUGH].r = diffRough;
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

void OrenNayarBlinnShader::Update(TimeValue t, Interval &valid) {
	Point3 p, p2;
	if( inUpdate )
		return;
	inUpdate = TRUE;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

//		pblock->GetValue( onb_ambient, t, p, ivalid );
//		ambient = LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( onb_diffuse, t, p, ivalid );
		diffuse= LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( onb_ambient, t, p2, ivalid );
		if( lockAD && (p!=p2)){
			pblock->SetValue( onb_ambient, t, diffuse);
			ambient = diffuse;
		} else {
			pblock->GetValue( onb_ambient, t, p, ivalid );
			ambient = Bound(Color(p.x,p.y,p.z));
		}
		pblock->GetValue( onb_specular, t, p2, ivalid );
		if( lockDS && (p!=p2)){
			pblock->SetValue( onb_specular, t, diffuse);
			specular = diffuse;
		} else {
			pblock->GetValue( onb_specular, t, p, ivalid );
			specular = Bound(Color(p.x,p.y,p.z));
		}
//		pblock->GetValue( onb_specular, t, p, ivalid );
//		specular = LimitColor(Color(p.x,p.y,p.z));

		pblock->GetValue( onb_glossiness, t, glossiness, ivalid );
		LIMIT0_1(glossiness);
		pblock->GetValue( onb_specular_level, t, specularLevel, ivalid );
		LIMITMINMAX(specularLevel,0.0f,9.99f);
		pblock->GetValue( onb_soften, t, softThresh, ivalid); 
		LIMIT0_1(softThresh);

		pblock->GetValue( onb_self_illum_amnt, t, selfIllum, ivalid );
		LIMIT0_1(selfIllum);
		pblock->GetValue( onb_self_illum_color, t, p, ivalid );
		selfIllumClr = LimitColor(Color(p.x,p.y,p.z));

		pblock->GetValue( onb_diffuse_level, t, diffLevel, ivalid );
		LIMITMINMAX(diffLevel,0.0f, 4.00f);
		pblock->GetValue( onb_roughness, t, diffRough, ivalid );
		LIMIT0_1(diffRough);

		// also get the non-animatables in case changed from scripter or other pblock accessors
		pblock->GetValue(onb_ds_lock, t, lockDS, ivalid);
		pblock->GetValue(onb_ad_lock, t, lockAD, ivalid);
		pblock->GetValue(onb_ad_texlock, t, lockADTex, ivalid);
		pblock->GetValue(onb_use_self_illum_color, t, selfIllumClrOn, ivalid);

		curTime = t;
	}
	valid &= ivalid;
	inUpdate = FALSE;
}

void OrenNayarBlinnShader::Reset()
{
//	orenNayarBlinnCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	ivalid.SetEmpty();
	SetSoftenLevel(0.1f,0);
	SetAmbientClr(Color(0.1f,0.1f,0.1f),0);
	SetDiffuseClr(Color(0.5f,0.5f,0.5f),0);
	SetSpecularClr(Color(0.9f,0.9f,0.9f),0);
	SetGlossiness(.25f,0);   // change from .4, 5-21-97
	SetSpecularLevel(0.0f,0);   
	SetDiffuseLevel(1.0f,0);   
	SetDiffuseRoughness(0.5f,0);   

	SetSelfIllum(.0f,0);
	SetSelfIllumClr( Color(.0f, .0f, .0f), 0 );
	SetSelfIllumClrOn( FALSE );
	SetLockADTex( TRUE );
	SetLockAD( TRUE );					// > 6/13/02 - 4:48pm --MQM-- was FALSE
	SetLockDS( FALSE );
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
//#define SHADER_HDR_CHUNK 0x4000
#define SHADER_SELFILLUM_CLR_ON_CHUNK 0x5000
#define SHADER_LOCKDS_ON_CHUNK 0x5001
#define SHADER_LOCKAD_ON_CHUNK 0x5002
#define SHADER_LOCKADTEX_ON_CHUNK 0x5003
#define SHADER_MAPSON_CHUNK 0x5004
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020

#
// IO
IOResult OrenNayarBlinnShader::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = CURRENT_ON_SHADER_VERSION;
	isave->Write(&version, sizeof(version), &nb);			
	isave->EndChunk();

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();

	return IO_OK;
}		

class ONShaderCB: public PostLoadCallback {
	public:
		OrenNayarBlinnShader *s;
		int loadVersion;
	    ONShaderCB(OrenNayarBlinnShader *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			// convert old v1 ParamBlock to ParamBlock2
			s->ReplaceReference(0,
				UpdateParameterBlock2(ONShaderPB, ON_SHADER_NPARAMS, (IParamBlock*)s->pblock, &onb_param_blk));

			// then set values that were previously stored outside the PB
			s->pblock->SetValue(onb_use_self_illum_color, 0, s->selfIllumClrOn);
			s->pblock->SetValue(onb_ds_lock, 0, s->lockDS);
			s->pblock->SetValue(onb_ad_lock, 0, s->lockAD);
			s->pblock->SetValue(onb_ad_texlock, 0, s->lockADTex);
		}
};

IOResult OrenNayarBlinnShader::Load(ILoad *iload) { 
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
	if (version < CURRENT_ON_SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new ONShaderCB(this, version));
		iload->SetObsolete();
	}

	return IO_OK;

}

			
///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//

//static BOOL colorDiffuseOn = TRUE;
static int stopX = 520;
static int stopY = 122;

#define RHO_EPSILON		0.001f

void OrenNayarBlinnShader::Illum(ShadeContext &sc, IllumParams &ip) 
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

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		float NL, kL;
		Point3 L;
		if (l->Illuminate( sc, sc.Normal(), lightCol, L, NL, kL)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;

			// specular  
			Color spec( 0.0f, 0.0f, 0.0f );
			if (isShiny && l->affectSpecular) {
				Point3 H = Normalize(L-sc.V() ); // (L + -V)/2
				float c = DotProd(sc.Normal(), H);	 
				if (c>0.0f) {
					if (softThresh != 0.0 && kL < softThresh) {
						c *= Soften(kL/softThresh);
					}
					c = (float)pow((double)c, phExp); // could use table lookup for speed
					spec = c * ip.channels[ID_SS].r * lightCol;
					ip.specIllumOut += spec;
				}
			}

			// diffuse
			if (l->affectDiffuse){
				float diffIntens;
				Color d = OrenNayarIllum( sc.Normal(), L, sc.V(), ip.channels[ID_DIFF_ROUGH].r * Pi*0.5f, ip.channels[ID_DI], &diffIntens, NL );
				d = d * ip.channels[ID_DIFF_LEV].r; 
				ip.diffIllumOut += kL * d * lightCol;
				ip.diffIllumIntens += kL * diffIntens * Intens(lightCol);
			}
 		}
	} // for each light

	// Apply mono self illumination
	if ( ! selfIllumClrOn ){
		float si = 0.3333333f * (ip.channels[ID_SI].r + ip.channels[ID_SI].g + ip.channels[ID_SI].b);
//		float si = ip.channels[ID_SI].r;  //DS: 4/23/99
		if ( si > 0.0f ) {
			si = Bound( si );
			ip.selfIllumOut = si * ip.channels[ID_DI];
			ip.diffIllumOut *= (1.0f - si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f-si;
			}
		}
	else {
	// colored self illum, 
		ip.selfIllumOut += ip.channels[ID_SI];
	}

	
	// get the diffuse intensity...unscramble the wavelength dependence
//	float rho, diffIntens;
//	rho = ip.channels[ID_DI].r == 0.0f ? 1.0f : 1.0f / ip.channels[ID_DI].r;
//	diffIntens = ip.diffIllumOut.r * rho;
//	rho = ip.channels[ID_DI].g == 0.0f ? 1.0f : 1.0f / ip.channels[ID_DI].g;
//	diffIntens += ip.diffIllumOut.g * rho;
//	rho = ip.channels[ID_DI].b == 0.0f ? 1.0f : 1.0f / ip.channels[ID_DI].b;
//	diffIntens += ip.diffIllumOut.b * rho;
//	ip.diffIllumIntens = diffIntens * 0.5f;
	// now we can multiply by the clrs
	ip.specIllumOut *= ip.channels[ID_SP]; 
	ip.ambIllumOut *= ip.channels[ID_AM]; 

	int chan = ip.stdIDToChannel[ ID_RR ];
	ShadeTransmission(sc, ip, ip.channels[chan], ip.refractAmt);
	chan = ip.stdIDToChannel[ ID_RL ];
	ShadeReflection( sc, ip, ip.channels[chan] ); 

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
//	OrenNayar shader dlg panel
//

// The dialog class
class OrenNayarShaderDlg : public ShaderParamDlg {
public:
	OrenNayarBlinnShader*	pShader;
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
	ISpinnerControl *softSpin;
	ISpinnerControl *shSpin, *ssSpin, *siSpin, *trSpin;
	ISpinnerControl *dlevSpin, *roughSpin; //, *rhoSpin;
	ICustButton* texMBut[NMBUTS];
	TexDADMgr dadMgr;
	
	OrenNayarShaderDlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~OrenNayarShaderDlg(); 

	// required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		for (long i=0; i<NMBUTS; i++) {
			if (hw == texMBut[i]->GetHwnd()) 
				return texmapFromMBut[i];
		}	
		return -1;
	}

	// Methods
	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(){ return OrenNayarShaderDlgClassID; }
	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (OrenNayarBlinnShader*)theShader; 
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

static INT_PTR CALLBACK  OrenNayarShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	OrenNayarShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (OrenNayarShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (OrenNayarShaderDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* OrenNayarBlinnShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	OrenNayarShaderDlg *pDlg = new OrenNayarShaderDlg(hwMtlEdit, imp);
	pDlg->SetThings( theMtl, this  );

	LoadStdShaderResources();
	if ( hOldRollup ) {
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_ONB2),
			OrenNayarShaderDlgProc, 
			GetString(IDS_DS_ON_BASIC),	
			(LPARAM)pDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);
	} else
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_ONB2),
			OrenNayarShaderDlgProc, 
			GetString(IDS_DS_ON_BASIC),	
			(LPARAM)pDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)pDlg;	
}

RefResult OrenNayarBlinnShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
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

OrenNayarShaderDlg::OrenNayarShaderDlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	shSpin = softSpin = ssSpin = siSpin = trSpin = NULL;
	dlevSpin = roughSpin = NULL; 
	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();
	isActive = valid = FALSE;

	for( long i = 0; i < NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

OrenNayarShaderDlg::~OrenNayarShaderDlg()
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
	ReleaseISpinner(softSpin);
	ReleaseISpinner(siSpin);
	ReleaseISpinner(dlevSpin);
	ReleaseISpinner(roughSpin);
	ReleaseISpinner(trSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite, GWLP_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}

void  OrenNayarShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		shSpin->SetValue(FracToPc(pShader->GetGlossiness()),FALSE);
		shSpin->SetKeyBrackets(KeyAtCurTime(onb_glossiness));

		ssSpin->SetValue(FracToPc(pShader->GetSpecularLevel()),FALSE);
		ssSpin->SetKeyBrackets(KeyAtCurTime(onb_specular_level));

		softSpin->SetValue(pShader->GetSoftenLevel(),FALSE);
		softSpin->SetKeyBrackets(KeyAtCurTime(onb_soften));

		trSpin->SetValue(FracToPc(pMtl->GetOpacity( curTime )),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		dlevSpin->SetValue(FracToPc(pShader->GetDiffuseLevel()),FALSE);
		dlevSpin->SetKeyBrackets(KeyAtCurTime(onb_diffuse_level));

		roughSpin->SetValue(FracToPc(pShader->GetDiffuseRoughness()),FALSE);
		roughSpin->SetKeyBrackets(KeyAtCurTime(onb_roughness));

		CheckButton(hRollup, IDC_LOCK_AD, pShader->GetLockAD() );
		CheckButton(hRollup, IDC_LOCK_DS, pShader->GetLockDS() );
	 	UpdateLockADTex( FALSE ); //don't send to mtl

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

			siSpin->SetValue(FracToPc(pShader->GetSelfIllum()), FALSE);
			siSpin->SetKeyBrackets(KeyAtCurTime(onb_self_illum_amnt));
		}

		UpdateColSwatches();
		UpdateHilite();
	}
}


void OrenNayarShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void OrenNayarShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm	 = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}



void OrenNayarShaderDlg::SetLockAD(BOOL lock)
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


void OrenNayarShaderDlg::UpdateColSwatches() 
{
	for(int i=0; i < NCOLBOX; i++) {
		if ( cs[ i ] ) {
			cs[i]->SetKeyBrackets( pShader->KeyAtTime(colParamID[i],curTime) );
			cs[i]->SetColor( GetMtlColor(i, (Shader*)pShader) );
		}
	}
}


void OrenNayarShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}

void OrenNayarShaderDlg::UpdateLockADTex( BOOL passOn) 
{
	int lock = 	pShader->GetLockADTex();
	CheckButton(hRollup, IDC_LOCK_ADTEX, lock);

	ShowWindow(GetDlgItem(hRollup, IDC_MAPON_AM), !lock);
	texMBut[ 0 ]->Enable(!lock);

	if ( passOn ) 
		pMtl->SyncADTexLock( lock );

//	UpdateMtlDisplay();
}

void OrenNayarShaderDlg::SetLockADTex(BOOL lock) 
{
	pShader->SetLockADTex( lock );
	UpdateLockADTex(TRUE); // passon to mtl
//	UpdateMtlDisplay();
	}

void OrenNayarShaderDlg::SetLockDS(BOOL lock) 
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
		case IDC_STD_COLOR1: return 0;
		case IDC_STD_COLOR2: return 1;
		case IDC_STD_COLOR3: return 2;
		case IDC_SI_COLOR: return 3;
		default: return 0;
	}
}


BOOL OrenNayarShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
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
   					GetMtlColor(i, pShader), GetColorName(i));
			}

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)HiliteWndProc);

			shSpin = SetupIntSpinner(hwndDlg, IDC_SH_SPIN, IDC_SH_EDIT, 0,100, 0);
			ssSpin = SetupIntSpinner(hwndDlg, IDC_SS_SPIN, IDC_SS_EDIT, 0,999, 0);
			softSpin = SetupFloatSpinner(hwndDlg, IDC_SOFT_SPIN, IDC_SOFT_EDIT, 0.0f,1.0f,0.0f,.01f);
			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);
			dlevSpin = SetupIntSpinner(hwndDlg, IDC_DIFFLEV_SPIN, IDC_DIFFLEV_EDIT, 0, 400, 0);
			roughSpin = SetupIntSpinner(hwndDlg, IDC_DIFFROUGH_SPIN, IDC_DIFFROUGH_EDIT, 0, 100, 0);

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
   											GetMtlColor(N_SI_CLR, pShader), GetColorName(N_SI_CLR));
*/
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
//					UpdateMtlDisplay();
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
			SetMtlColor(n, curColor, pShader, cs, curTime);
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
				case IDC_SOFT_SPIN: 
					pShader->SetSoftenLevel(softSpin->GetFVal(),curTime); 
					break;
				case IDC_SI_SPIN: 
					pShader->SetSelfIllum(PcToFrac(siSpin->GetIVal()),curTime); 
					break;
				case IDC_DIFFLEV_SPIN: 
					pShader->SetDiffuseLevel(PcToFrac(dlevSpin->GetIVal()),curTime); 
					break;
				case IDC_DIFFROUGH_SPIN: 
					pShader->SetDiffuseRoughness(PcToFrac(roughSpin->GetIVal()),curTime); 
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


