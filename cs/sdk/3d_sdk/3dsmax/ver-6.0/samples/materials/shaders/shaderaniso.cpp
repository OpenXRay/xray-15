///////////////////////////////////////////////////////////////////
//
//	Aniso2 .. a Ward derivative shader & dlg panel
//
//		Created: 11/3/98 Kells Elmquist
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "macrorec.h"
#include "toneop.h"

// Class Ids
#define ANISOSHADER_CLASS_ID		0x2857f460

static Class_ID AnisoShaderClassID( ANISOSHADER_CLASS_ID, 0);
static Class_ID AnisoShaderDlgClassID( ANISOSHADER_CLASS_ID, 0);

// paramblock2 block and parameter IDs.
enum { aniso_params, };
// shdr_params param IDs
enum 
{ 
	an_ambient, an_diffuse, an_specular, an_self_illum_color,
	an_diffuse_level, an_specular_level, an_self_illum_amnt, 
	an_glossiness, an_anisotropy, an_orientation, 
	an_map_channel, an_ad_texlock, an_ad_lock, an_ds_lock, an_use_self_illum_color, 
};


/////////////////////////////////////////////////////////////////////
//
//	Basic Panel UI 
//
#define ANISO_NMBUTS 10
#define N_TR_BUT 7

// tex channel number to button IDC
static int texMButtonsIDC[] = {
	IDC_MAPON_AM, IDC_MAPON_DI, IDC_MAPON_SP, 
	IDC_MAPON_DLEV, IDC_MAPON_SLEV, IDC_MAPON_GL, IDC_MAPON_AN,
	IDC_MAPON_OR, IDC_MAPON_SI, IDC_MAPON_TR, 
	};
		
// This array gives the texture map number for given MButton number								
static int texmapFromMBut[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

// Texture Maps
#define ANISO_NTEXMAPS	11

// channels ids needed by shader
#define A_AM	0
#define A_DI	1
#define A_SP	2
#define A_DL	3
#define A_SL	4
#define A_GL	5
#define A_AN	6
#define A_OR	7
#define A_SI	8

// channel names
// sized for nmax  textures
static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, 
	IDS_KE_DIFF_LEVEL, IDS_KE_SPEC_LEVEL, IDS_KE_GLOSSINESS, IDS_KE_ANISOTROPY, 
	IDS_KE_ORIENTATION, IDS_KE_SELFILLUM,
	IDS_DS_TRANS, IDS_DS_FILTER, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
};	

// internal non-local parsable channel map names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("ambientMap"), _T("diffuseMap"),	_T("specularMap"), _T("diffuseLevelMap"), 
	_T("specularLevelMap"), _T("glossinessMap"), _T("anisotropyMap"), _T("orientationMap"),
	_T("selfIllumMap"), _T("opacityMap"), _T("filterMap"),	
	_T(""), _T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""),
};	

// sized for nmax textures
// bump, reflection & refraction maps are ignored....done after shading
static int chanType[] = {
	CLR_CHANNEL, CLR_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, 
	MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL,  
	CLR_CHANNEL, MONO_CHANNEL, CLR_CHANNEL,	UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,
};	

// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { 0, 1, 2, 5, 4, 8, 9, 10, -1, -1, -1, -1,  -1, -1, -1, -1 };

//////////////////////////////////////////////////////////////////////////////////////////
//
//		Aniso Parameter Block
//
#define CURRENT_ANISO_SHADER_VERSION	2
#define ANISO_SHADER_NPARAMS			10
#define ANISO_SHADER_PB_VERSION			1
#define ANISO_NCOLBOX					4

static int colID[ANISO_NCOLBOX] = { IDC_STD_COLOR1, IDC_STD_COLOR2, IDC_STD_COLOR3, IDC_SI_COLOR };
#define N_AMB_CLR		0
#define N_SI_CLR		3

#define PB_AMBIENT_CLR		0
#define PB_DIFFUSE_CLR		1
#define PB_SPECULAR_CLR		2
#define PB_SELFILLUM_CLR	3
#define PB_DIFFUSE_LEV		4
#define PB_SPECULAR_LEV		5
#define PB_SELFILLUM_LEV	6
#define PB_GLOSSINESS		7
#define PB_ANISOTROPY		8
#define PB_ORIENTATION		9

//v1 Param Block Descriptor
static ParamBlockDescID AnisoShaderPB[ ANISO_SHADER_NPARAMS ] = {
	{ TYPE_RGBA,  NULL, TRUE, an_ambient },   // ambient
	{ TYPE_RGBA,  NULL, TRUE, an_diffuse },   // diffuse
	{ TYPE_RGBA,  NULL, TRUE, an_specular },   // specular
	{ TYPE_RGBA,  NULL, TRUE, an_self_illum_color },   // selfIllumClr 
	{ TYPE_FLOAT, NULL, TRUE, an_diffuse_level },  // diffuse level
	{ TYPE_FLOAT, NULL, TRUE, an_specular_level },   // specularLevel
	{ TYPE_FLOAT, NULL, TRUE, an_self_illum_amnt },   // selfIllumLevel
	{ TYPE_FLOAT, NULL, TRUE, an_glossiness },   // glossiness 
	{ TYPE_FLOAT, NULL, TRUE, an_anisotropy },  // eccentricity or anisotropy
	{ TYPE_FLOAT, NULL, TRUE, an_orientation },  // orientation
}; 


#define ANISO_NUMOLDVER 1

static ParamVersionDesc oldVersions[ANISO_NUMOLDVER] = {
	ParamVersionDesc(AnisoShaderPB, ANISO_SHADER_NPARAMS, 0),
};

//----------------------------------------------------------------------------------------
//---- Ward Derivative Elliptical Gaussian Anisotropic Shader ----------------------------
//----------------------------------------------------------------------------------------

class AnisoShaderDlg;

class AnisoShader : public ExposureMaterialControlImp<AnisoShader, CombineComponentsCompShader> {
friend class AnisoShaderCB;
friend class AnisoShaderDlg;
friend class ExposureMaterialControlImp<AnisoShader, CombineComponentsCompShader>;
protected:
	IParamBlock2	 *pblock;   // ref 0
	Interval		ivalid;

	AnisoShaderDlg*	paramDlg;

	BOOL			selfIllumClrOn;
	BOOL			lockDS;
	BOOL			lockAD;
	BOOL			lockADTex;

	Color			ambient;
	Color			diffuse;
	Color			specular;
	Color			selfIllum;
	float			glossiness;
	float			orientation;
	float			anisotropy;
	float			specLevel;
	float			diffLevel;
	float			selfIllumLevel;

	static CombineComponentsFPDesc msExpMtlControlDesc;

	public:
	AnisoShader();
	void DeleteThis(){ delete this; }		
    ULONG SupportStdParams(){ return STD_ANISO | STD_EXTRA; }
	// copy std params, for switching shaders
    void CopyStdParams( Shader* pFrom );

	// texture maps
	long  nTexChannelsSupported(){ return ANISO_NTEXMAPS; }
	TSTR  GetTexChannelName( long nTex ){ return GetString( texNameIDS[ nTex ] ); }
	TSTR GetTexChannelInternalName( long nTex ){ return texInternalNames[ nTex ]; }
	long ChannelType( long nChan ) { return chanType[nChan]; }
	long StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

	BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int ){ paramDlg = (AnisoShaderDlg*)newDlg; }

	Class_ID ClassID() { return AnisoShaderClassID; }
	TSTR GetName() { return GetString( IDS_KE_ANISO ); }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i){ return (i==0)? pblock : NULL; }
	TSTR SubAnimName(int i){ return TSTR(GetString( IDS_KE_ANISO_PARMS )); };
	int SubNumToRefNum(int subNum) { return subNum;	}

	// add direct ParamBlock2 access
	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } 

 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i){ return (i==0)? pblock : NULL; }
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


	// Aniso Shader specific section
	void PreShade(ShadeContext& sc, IReshadeFragment* pFrag);
	void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip);
	void  Illum(ShadeContext &sc, IllumParams &ip);
	void  InternalIllum(ShadeContext &sc, IllumParams &ip, Point3* pU );
	float EvalHiliteCurve2(float x, float y, int layer=0 );
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol);
	// cache for mapping of params, mtl fills in ip
	void GetIllumParams( ShadeContext &sc, IllumParams& ip );

	void SetGlossiness(float v, TimeValue t)		
		{ glossiness= v; pblock->SetValue( an_glossiness, t, v); }
	void SetAnisotropy(float v, TimeValue t)		
		{ anisotropy = v; pblock->SetValue( an_anisotropy, t, v); }
	void SetOrientation(float v, TimeValue t)		
		{ orientation = v; pblock->SetValue( an_orientation, t, v); }
	float GetGlossiness(int mtlNum=0, BOOL backFace=FALSE){ return glossiness; };	
	float GetAnisotropy(int mtlNum=0, BOOL backFace=FALSE){ return anisotropy; };	
	float GetOrientation(int mtlNum=0, BOOL backFace=FALSE){ return orientation; };	
	float GetGlossiness( TimeValue t){return pblock->GetFloat(an_glossiness,t);  }		
	float GetAnisotropy( TimeValue t){return pblock->GetFloat(an_anisotropy,t);  }		
	float GetOrientation( TimeValue t){return pblock->GetFloat(an_orientation,t);  }		

	// Std Params
	void SetLockDS(BOOL lock){ lockDS = lock; pblock->SetValue( an_ds_lock, 0, lock); }
	BOOL GetLockDS(){ return lockDS; }
	void SetLockAD(BOOL lock){ lockAD = lock; pblock->SetValue( an_ad_lock, 0, lock);  }
	BOOL GetLockAD(){ return lockAD; }
	void SetLockADTex(BOOL lock){ lockADTex = lock; pblock->SetValue( an_ad_texlock, 0, lock);  }
	BOOL GetLockADTex(){ return lockADTex; }

	void SetAmbientClr(Color c, TimeValue t)		
		{ ambient = c; pblock->SetValue( an_ambient, t, c); }
	void SetDiffuseClr(Color c, TimeValue t)		
		{ diffuse = c; pblock->SetValue( an_diffuse, t, c); }
	void SetSpecularClr(Color c, TimeValue t)
		{ specular = c; pblock->SetValue( an_specular, t, c); }
	void SetSelfIllumClr(Color c, TimeValue t)
		{ selfIllum = c; pblock->SetValue( an_self_illum_color, t, c); }
	void SetDiffuseLevel(float v, TimeValue t)		
		{ diffLevel = v; pblock->SetValue( an_diffuse_level, t, v); }
	void SetSpecularLevel(float v, TimeValue t)		
		{ specLevel = v; pblock->SetValue( an_specular_level, t, v); }
	void SetSelfIllum(float v, TimeValue t)
		{ selfIllumLevel = v; pblock->SetValue( an_self_illum_amnt, t, v); }

	Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE){ return ambient;}		
    Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE){ return diffuse;}		
	Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE){ return specular; }
	Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE){ return selfIllum; }
	float GetDiffuseLevel(int mtlNum=0, BOOL backFace=FALSE){ return diffLevel; }
	float GetSpecularLevel(int mtlNum=0, BOOL backFace=FALSE){ return specLevel; }
	float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE){ return selfIllumLevel; }

	Color GetAmbientClr(TimeValue t){ return pblock->GetColor(an_ambient,t); }		
	Color GetDiffuseClr(TimeValue t){ return pblock->GetColor(an_diffuse,t); }		
	Color GetSpecularClr(TimeValue t){ return pblock->GetColor(an_specular,t);	}
	Color GetSelfIllumClr(TimeValue t){ return pblock->GetColor(an_self_illum_color,t);}		
	float GetSpecularLevel(TimeValue t){ return  pblock->GetFloat(an_specular_level,t); }
	float GetDiffuseLevel(TimeValue t){ return  pblock->GetFloat(an_diffuse_level,t); }
	float GetSelfIllum(TimeValue t){ return  pblock->GetFloat(an_self_illum_amnt,t);}		

	void SetSelfIllumClrOn( BOOL on )
		{ selfIllumClrOn = on; pblock->SetValue( an_use_self_illum_color, 0, on);}
	BOOL IsSelfIllumClrOn(){ return selfIllumClrOn; }
	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return selfIllumClrOn; }

	// not supported
	void SetSoftenLevel(float v, TimeValue t) {}
	float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE){ return DEFAULT_SOFTEN; }
	float GetSoftenLevel(TimeValue t){ return  DEFAULT_SOFTEN; }
};

///////////// Class Descriptor ////////////////////////
class AnisoShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new AnisoShader(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_ANISO); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return AnisoShaderClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Anisotropic"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			 // returns owning module handle
};


AnisoShaderClassDesc anisoCD;
ClassDesc * GetAnisoShaderCD(){ return &anisoCD; }

// shader parameters
static ParamBlockDesc2 aniso_param_blk ( aniso_params, _T("shaderParameters"),  0, &anisoCD, P_AUTO_CONSTRUCT, 0, 
	// params
	an_ambient, _T("ambient"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_AMBIENT, 
		p_default, Color(0, 0, 0), 
		end,
	an_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_DIFFUSE, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	an_specular, _T("specular"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_SPECULAR, 
		p_default, Color(1.0f, 1.0f, 1.0f), 
		end,
	an_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	an_diffuse_level, _T("diffuseLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_DIFF_LEVEL,
		p_default,		100.0,
		p_range,		0.0, 400.0,
		end,
	an_specular_level, _T("specularLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE,IDS_KE_SPEC_LEVEL,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	an_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	an_glossiness, _T("glossiness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_GLOSSINESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	an_anisotropy, _T("anisotropy"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_ANISOTROPY,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	an_orientation, _T("orientation"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_ORIENTATION,
		p_default,		0.0,
		p_range,		MIN_ORIENT*100.0, MAX_ORIENT*100.0,
		end,
	an_map_channel, _T("unused"), TYPE_INT, 0, IDS_KE_MAPCHANNEL,
		p_default,		0,
		p_range,		0, 16,
		end,
	an_ad_texlock, _T("adTextureLock"), TYPE_BOOL,	0, IDS_JW_ADTEXLOCK, 
		p_default, TRUE, 
		end,
	an_ad_lock, _T("adLock"), TYPE_BOOL, 0, IDS_JW_ADLOCK, 
		p_default, TRUE, 
		end,
	an_ds_lock, _T("dsLock"), TYPE_BOOL, 0, IDS_JW_DSLOCK, 
		p_default, FALSE, 
		end,
	an_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0, IDS_JW_SELFILLUMCOLORON,
		p_default, TRUE, 
		end,
	end
);


CombineComponentsFPDesc AnisoShader::msExpMtlControlDesc(anisoCD);

AnisoShader::AnisoShader() 
{ 
	pblock = NULL; 
	anisoCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	paramDlg = NULL; 

	lockDS = selfIllumClrOn = FALSE;
	lockAD = lockADTex = TRUE;
	ambient = diffuse = specular = selfIllum = Color(0.0f,0.0f,0.0f);
	glossiness = orientation = anisotropy = specLevel = diffLevel = selfIllumLevel = 0.0f;
	ivalid.SetEmpty(); 
}

void AnisoShader::CopyStdParams( Shader* pFrom )
{
	macroRecorder->Disable(); 
	// don't want to see this parameter copying in macrorecorder
		SetLockDS( pFrom->GetLockDS() );
		SetLockAD( pFrom->GetLockAD() );
		SetLockADTex( pFrom->GetLockADTex() );
		SetSelfIllumClrOn( pFrom->IsSelfIllumClrOn() );

		SetAmbientClr( pFrom->GetAmbientClr(0,0), 0 );
		SetDiffuseClr( pFrom->GetDiffuseClr(0,0), 0 );
		SetSpecularClr( pFrom->GetSpecularClr(0,0), 0 );
		SetSelfIllumClr( pFrom->GetSelfIllumClr(0,0), 0 );

		SetSpecularLevel( pFrom->GetSpecularLevel(0,0), 0 );
		SetGlossiness( pFrom->GetGlossiness(0,0), 0 );
		SetSelfIllum( pFrom->GetSelfIllum(0,0), 0 );
	macroRecorder->Enable();
	ivalid.SetEmpty();	

}


RefTargetHandle AnisoShader::Clone( RemapDir &remap )
{
	AnisoShader* mnew = new AnisoShader();
	mnew->ExposureMaterialControl::operator=(*this);
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->ambient = ambient;
	mnew->diffuse = diffuse;
	mnew->specular = specular;
	mnew->selfIllum = selfIllum;
	mnew->selfIllumLevel = selfIllumLevel;
	mnew->glossiness = glossiness;
	mnew->anisotropy = anisotropy;
	mnew->orientation = orientation;
	mnew->specLevel = specLevel;
	mnew->diffLevel = diffLevel;
	mnew->lockDS = lockDS;
	mnew->lockAD = lockAD;
	mnew->lockADTex = lockADTex;
	mnew->selfIllumClrOn = selfIllumClrOn;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

void AnisoShader::GetIllumParams( ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
	ip.channels[A_AM] = lockAD? diffuse : ambient;
	ip.channels[A_DI] = diffuse;
	ip.channels[A_SP] = lockDS? diffuse : specular;
	ip.channels[A_GL].r = glossiness;
	ip.channels[A_AN].r = anisotropy;
	ip.channels[A_OR].r = orientation * (1.0f/1.8f);
	ip.channels[A_DL].r = diffLevel;
	ip.channels[A_SL].r = specLevel;
	if (selfIllumClrOn )
		ip.channels[A_SI] = selfIllum;
	else
		ip.channels[A_SI].r = ip.channels[A_SI].g = ip.channels[A_SI].b = selfIllumLevel;

}
static BOOL inUpdate = FALSE;

void AnisoShader::Update(TimeValue t, Interval &valid) {
	Point3 p, p2;
	if( inUpdate )
		return;
	inUpdate = TRUE;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

		pblock->GetValue( an_diffuse, t, p, ivalid );
		diffuse= Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( an_ambient, t, p2, ivalid );
		if( lockAD && (p!=p2)){
			pblock->SetValue( an_ambient, t, diffuse);
			ambient = diffuse;
		} else {
			pblock->GetValue( an_ambient, t, p, ivalid );
			ambient = Bound(Color(p.x,p.y,p.z));
		}
		pblock->GetValue( an_specular, t, p2, ivalid );
		if( lockDS && (p!=p2)){
			pblock->SetValue( an_specular, t, diffuse);
			specular = diffuse;
		} else {
			pblock->GetValue( an_specular, t, p, ivalid );
			specular = Bound(Color(p.x,p.y,p.z));
		}

		pblock->GetValue( an_glossiness, t, glossiness, ivalid );
		glossiness = Bound( glossiness, 0.0001f, 1.0f );
		pblock->GetValue( an_anisotropy, t, anisotropy, ivalid );
		anisotropy = Bound(anisotropy);
		pblock->GetValue( an_orientation, t, orientation, ivalid );
		orientation = Bound( orientation, (float)MIN_ORIENT, (float)MAX_ORIENT );

		pblock->GetValue( an_specular_level, t, specLevel, ivalid );
		specLevel = Bound(specLevel,0.0f,9.99f);
		pblock->GetValue( an_diffuse_level, t, diffLevel, ivalid );
		diffLevel = Bound(diffLevel,0.0f,4.0f);
		pblock->GetValue( an_self_illum_amnt, t, selfIllumLevel, ivalid );
		selfIllumLevel = Bound(selfIllumLevel);
		pblock->GetValue( an_self_illum_color, t, p, ivalid );
		selfIllum = Bound(Color(p.x,p.y,p.z));

		// also get the non-animatables in case changed from scripter or other pblock accessors
		pblock->GetValue(an_ds_lock, t, lockDS, ivalid);
		pblock->GetValue(an_ad_lock, t, lockAD, ivalid);
		pblock->GetValue(an_ad_texlock, t, lockADTex, ivalid);
		pblock->GetValue(an_use_self_illum_color, t, selfIllumClrOn, ivalid);

	}
	valid &= ivalid;
	inUpdate = FALSE;

}

void AnisoShader::Reset()
{
//	anisoCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	ivalid.SetEmpty();
	SetAmbientClr(Color(0.1f,0.1f,0.1f),0);
	SetDiffuseClr(Color(0.5f,0.5f,0.5f),0);
	SetSpecularClr(Color(0.9f,0.9f,0.9f),0);
	SetSelfIllumClr(Color(0.0f,0.0f,0.0f),0);
	SetSelfIllum( 0.0f,0 );
	SetGlossiness(.25f,0);   
	SetAnisotropy(0.5f,0);   
	SetOrientation(0.0f,0);   
	SetSpecularLevel(0.05f,0);   
	SetDiffuseLevel(1.0f,0);   

	SetLockADTex( TRUE );
	SetLockAD( TRUE );
	SetLockDS( FALSE );
	SetSelfIllumClrOn( FALSE );
}



//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
#define SHADER_HDR_CHUNK 0x4000
#define SHADER_LOCKDS_ON_CHUNK 0x5001
#define SHADER_LOCKAD_ON_CHUNK 0x5002
#define SHADER_LOCKADTEX_ON_CHUNK 0x5003
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_SELFILLUMCLR_ON_CHUNK	0x5006
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020
// IO
IOResult AnisoShader::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = CURRENT_ANISO_SHADER_VERSION;
	isave->Write(&version,sizeof(version), &nb);			
	isave->EndChunk();

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();
	return IO_OK;
}		

class AnisoShaderCB: public PostLoadCallback {
	public:
		AnisoShader *s;
		int loadVersion;
	    AnisoShaderCB(AnisoShader *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			// convert old v1 ParamBlock to ParamBlock2
			s->ReplaceReference(0,
				UpdateParameterBlock2(AnisoShaderPB, ANISO_SHADER_NPARAMS, (IParamBlock*)s->pblock, &aniso_param_blk));

			// then set values that were previously stored outside the PB
			s->pblock->SetValue(an_use_self_illum_color, 0, s->selfIllumClrOn);
			s->pblock->SetValue(an_ds_lock, 0, s->lockDS);
			s->pblock->SetValue(an_ad_lock, 0, s->lockAD);
			s->pblock->SetValue(an_ad_texlock, 0, s->lockADTex);
		}
};


IOResult AnisoShader::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;

	lockAD = lockADTex = lockDS = selfIllumClrOn = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SHADER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
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
			case SHADER_SELFILLUMCLR_ON_CHUNK:
				selfIllumClrOn = TRUE;
				break;
			case SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK:
				res = ExposureMaterialControl::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < CURRENT_ANISO_SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new AnisoShaderCB(this, version));
		iload->SetObsolete();
	}

	return IO_OK;
}

			
///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//

float AnisoShader::EvalHiliteCurve2( float x, float y, int )
{
	return GaussHiliteCurve2( x, y, specLevel, glossiness, anisotropy );
}


void AnisoShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
{
	//NB kl,aka NL & g cancel out
//	rcol *= ip.channels[A_SL].r * ip.channels[ID_SP] * DEFAULT_K_REFL;
	rcol *= ip.channels[A_SP] * DEFAULT_K_REFL; 
}


static int stopX = -1;
static int stopY = -1;

void AnisoShader::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
	Point3 T = GetTangent( sc, 0 );
	pFrag->AddUnitVecChannel( T );
}

void AnisoShader::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip)
{
	Point3 T = pFrag->GetUnitVecChannel( nextTexIndex++ );
	InternalIllum( sc, *ip, &T );
}


void AnisoShader::Illum(ShadeContext &sc, IllumParams &ip) 
{
	InternalIllum( sc, ip, NULL );
}

void AnisoShader::InternalIllum(ShadeContext &sc, IllumParams &ip, Point3* pTangent )
{
	LightDesc *l;
	Color lightCol;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if ( sp.x == stopX && sp.y == stopY )
		sp.x = stopX;
#endif

	BOOL isShiny= (ip.channels[A_SL].r > 0.0f) ? 1 : 0; 

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
				// get tangent, use passed value or get new one
				Point3 T = (pTangent)? *pTangent : GetTangent( sc, 0 ); 

				float g = GaussHighlight( ip.channels[A_GL].r, ip.channels[A_AN].r,
											ip.channels[A_OR].r, sc.Normal(), sc.V(), L, T, &NL );
										
				spec = g * kL * ip.channels[A_SL].r * lightCol;
				ip.specIllumOut += spec;

			} // end, isSpecular

			// diffuse
			if (l->affectDiffuse)
				ip.diffIllumOut += kL * ip.channels[A_DL].r * lightCol;

		} // end, is illuminated
	} // for each light

	// Apply mono self illumination
	if ( ! selfIllumClrOn ){
		float si = 0.3333333f * (ip.channels[A_SI].r + ip.channels[A_SI].g + ip.channels[A_SI].b);
		//float si = ip.channels[A_SI].r;
		if ( si > 0.0f ) {
			si = Bound( si );
			ip.selfIllumOut = si * ip.channels[A_DI];
			ip.diffIllumOut *= (1.0f-si);
			// fade the ambient down on si: 5/27/99 ke
			ip.ambIllumOut *= 1.0f-si;
		}
	} else {
		// colored self illum, 
		ip.selfIllumOut = ip.channels[A_SI];
	}

	// now we can multiply by the clrs,
	ip.ambIllumOut *= ip.channels[A_AM]; 
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= ip.channels[A_DI]; 
	ip.specIllumOut *= ip.channels[A_SP]; 

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



// The dialog class
class AnisoShaderDlg : public ShaderParamDlg {
public:
	AnisoShader*	pShader;
	StdMat2*	pMtl;
	HPALETTE	hOldPal;
	HWND		hwmEdit;	// window handle of the materials editor dialog
	IMtlParams*	pMtlPar;
	HWND		hwHilite;   // the hilite window
	HWND		hRollup;	// Rollup panel
	TimeValue	curTime;
	BOOL		valid;
	BOOL		isActive;

	IColorSwatch *cs[ANISO_NCOLBOX];
	ISpinnerControl *dlevSpin, *slevSpin, *glSpin, *anSpin, *orSpin, *trSpin, *siSpin;
	ICustButton* texMBut[ANISO_NMBUTS];
	TexDADMgr dadMgr;
	
	AnisoShaderDlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~AnisoShaderDlg(); 

	// required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		for (long i=0; i < ANISO_NMBUTS; i++) {
			if (hw == texMBut[i]->GetHwnd()) 
				return texmapFromMBut[i];
		}	
		return -1;
	}

	// Methods
	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(){ return AnisoShaderDlgClassID; }

	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (AnisoShader*)theShader; 
		if(pShader) pShader->SetParamDlg(this,0);
		pMtl = theMtl;
	}
	ReferenceTarget* GetThing(){ return pMtl; } // mtl is the thing! (for DAD!)
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

static INT_PTR CALLBACK  AnisoShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	AnisoShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (AnisoShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (AnisoShaderDlg*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* AnisoShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	paramDlg = new AnisoShaderDlg(hwMtlEdit, imp);
	paramDlg->SetThings( theMtl, this  );

	LoadStdShaderResources();
	if ( hOldRollup ) {
		paramDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_WARD4),
			AnisoShaderDlgProc, 
			GetString(IDS_KE_ANSIO_BASIC),	// your name here
			(LPARAM)paramDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);
	} else
		paramDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_WARD4),
			AnisoShaderDlgProc, 
			GetString(IDS_KE_ANSIO_BASIC),	
			(LPARAM)paramDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)paramDlg;	
}


RefResult AnisoShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
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



AnisoShaderDlg::AnisoShaderDlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	dlevSpin = slevSpin = glSpin = anSpin = orSpin = trSpin = siSpin = NULL;
	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();

	isActive = valid = FALSE;

	for( long i = 0; i < ANISO_NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < ANISO_NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

AnisoShaderDlg::~AnisoShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < ANISO_NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}

	for (i=0; i<ANISO_NCOLBOX; i++)
		if (cs[i]) ReleaseIColorSwatch(cs[i]); // mjm - 5.10.99
	
 	ReleaseISpinner(slevSpin);
	ReleaseISpinner(dlevSpin);
	ReleaseISpinner(glSpin);
	ReleaseISpinner(anSpin);
	ReleaseISpinner(orSpin);
	ReleaseISpinner(trSpin);
	ReleaseISpinner(siSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite, GWLP_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}


void  AnisoShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		dlevSpin->SetValue(FracToPc(pShader->GetDiffuseLevel()),FALSE);
		dlevSpin->SetKeyBrackets(KeyAtCurTime(an_diffuse_level));

		slevSpin->SetValue(FracToPc(pShader->GetSpecularLevel()),FALSE);
		slevSpin->SetKeyBrackets(KeyAtCurTime(an_specular_level));

		glSpin->SetValue( FracToPc(pShader->GetGlossiness()), FALSE);
		glSpin->SetKeyBrackets(KeyAtCurTime(an_glossiness));

		anSpin->SetValue( FracToPc(pShader->GetAnisotropy()), FALSE);
		anSpin->SetKeyBrackets(KeyAtCurTime(an_anisotropy));

		orSpin->SetValue( FracToPc(pShader->GetOrientation()), FALSE);
		orSpin->SetKeyBrackets(KeyAtCurTime(an_orientation));

		trSpin->SetValue(FracToPc(pMtl->GetOpacity( curTime )),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		//  color selfIllum
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
			siSpin->SetKeyBrackets(KeyAtCurTime(PB_SELFILLUM_LEV));
		}

		CheckButton(hRollup, IDC_LOCK_AD, pShader->GetLockAD() );
		CheckButton(hRollup, IDC_LOCK_DS, pShader->GetLockDS() );
	 	UpdateLockADTex( FALSE ); //don't send to mtl

		UpdateColSwatches();
		UpdateHilite();
	}
}

static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void AnisoShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < ANISO_NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm	 = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);

	}
}


void AnisoShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}


void AnisoShaderDlg::SetLockAD(BOOL lock)
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


void AnisoShaderDlg::UpdateColSwatches() 
{
	for(int i=0; i < ANISO_NCOLBOX; i++) {
		if ( cs[ i ] ) {
			cs[i]->SetKeyBrackets( pShader->KeyAtTime(PB_AMBIENT_CLR+i,curTime) );
			cs[i]->SetColor( GetMtlColor(i, (Shader*)pShader) );
		}
	}
}


void AnisoShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite2(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}

void AnisoShaderDlg::UpdateLockADTex( BOOL passOn) {
	int lock = 	pShader->GetLockADTex();
	CheckButton(hRollup, IDC_LOCK_ADTEX, lock);

	ShowWindow(GetDlgItem(hRollup, IDC_MAPON_AM), !lock);
	texMBut[ 0 ]->Enable(!lock);

	if ( passOn ) {
		pMtl->SyncADTexLock( lock );
	}
//	UpdateMtlDisplay();
}

void AnisoShaderDlg::SetLockADTex(BOOL lock) {
	pShader->SetLockADTex( lock );
	UpdateLockADTex(TRUE); // pass on to mtl
}

void AnisoShaderDlg::SetLockDS(BOOL lock) 
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

static int ColorIDCToIndex(int idc) {
	switch (idc) {
		case IDC_STD_COLOR1: return 0;
		case IDC_STD_COLOR2: return 1;
		case IDC_STD_COLOR3: return 2;
		case IDC_SI_COLOR: return 3;
		default: return 0;
	}
}


BOOL AnisoShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
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

			for (i=0; i<ANISO_NCOLBOX; i++) {
   				cs[i] = GetIColorSwatch(GetDlgItem(hwndDlg, colID[i]),
   					GetMtlColor(i, pShader), GetColorName(i));
			}

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)Hilite2WndProc);

			slevSpin = SetupIntSpinner(hwndDlg, IDC_SLEV_SPIN, IDC_SLEV_EDIT, 0, 999, 0);
			dlevSpin = SetupIntSpinner(hwndDlg, IDC_DLEV_SPIN, IDC_DLEV_EDIT, 0, 400, 0);
			glSpin = SetupIntSpinner(hwndDlg, IDC_GL_SPIN, IDC_GL_EDIT, 0,100, 0);
			anSpin = SetupIntSpinner(hwndDlg, IDC_AN_SPIN, IDC_AN_EDIT, 0,100, 0);
			orSpin = SetupIntSpinner(hwndDlg, IDC_OR_SPIN, IDC_OR_EDIT, -9999, 9999, 0);
			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);
			siSpin = SetupIntSpinner(hwndDlg, IDC_SI_SPIN, IDC_SI_EDIT, 0,100, 0);

			if( pShader->IsSelfIllumClrOn() ) {
				// enable the color swatch, disable the spinner
				ShowWindow( GetDlgItem(hwndDlg, IDC_SI_EDIT), SW_HIDE );
				ShowWindow( GetDlgItem(hwndDlg, IDC_SI_SPIN), SW_HIDE );
			} else {
				// disable the color swatch
				ShowWindow( cs[N_SI_CLR]->GetHwnd(), SW_HIDE );
			}
	
			for (int j=0; j<ANISO_NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
			}

			SetupLockButton(hwndDlg,IDC_LOCK_AD,FALSE);
			SetupLockButton(hwndDlg,IDC_LOCK_DS,FALSE);
			SetupPadLockButton(hwndDlg,IDC_LOCK_ADTEX, TRUE);

			LoadDialog(TRUE);
		}
		break;

		case WM_COMMAND: 
			{
			for ( int i=0; i<ANISO_NMBUTS; i++) {
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
			// >>>>>>>>>< is redundant w/ ln 1189 below?
			if (buttonUp) {
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
				// DS: 5/3/99-  this was commented out. I put it back in, because
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
				case IDC_GL_SPIN: 
					pShader->SetGlossiness(PcToFrac( glSpin->GetIVal() ), curTime); 
					UpdateHilite();
					break;
				case IDC_AN_SPIN: 
					pShader->SetAnisotropy(PcToFrac( anSpin->GetIVal() ), curTime); 
					UpdateHilite();
					break;
				case IDC_OR_SPIN: 
					pShader->SetOrientation( PcToFrac(orSpin->GetIVal() ), curTime); 
					break;
				case IDC_SLEV_SPIN: 
					pShader->SetSpecularLevel( PcToFrac(slevSpin->GetIVal()),curTime); 
					UpdateHilite();
					break;
				case IDC_DLEV_SPIN: 
					pShader->SetDiffuseLevel(PcToFrac(dlevSpin->GetIVal()),curTime); 
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
			// >>>>< duplicate?
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


