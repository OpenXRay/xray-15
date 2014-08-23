///////////////////////////////////////////////////////////////////
//
//	MultiLayer shader
//
//		Created: 11/23/98 Kells Elmquist
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "macrorec.h"
#include "toneop.h"

// Class Ids
#define MULTILAYERSHADER_CLASS_ID		0x2857f470

static Class_ID MultiLayerShaderClassID( MULTILAYERSHADER_CLASS_ID, 0);


// paramblock2 block and parameter IDs.
enum { multiLayer_params, };
// shdr_params param IDs
enum 
{ 
	ml_ambient, ml_diffuse, ml_self_illum_color,
	ml_self_illum_amnt, ml_diffuse_level, ml_diffuse_rough, 
	ml_specular1, ml_specular_level1, ml_glossiness1, ml_anisotropy1, ml_orientation1, 
	ml_specular2, ml_specular_level2, ml_glossiness2, ml_anisotropy2, ml_orientation2, 
	ml_map_channel, ml_ad_texlock, ml_ad_lock, ml_use_self_illum_color, 
};



/////////////////////////////////////////////////////////////////////
//
//	Basic Panel UI 
//
#define MULTI_LAYER_NMBUTS 16
#define N_TR_BUT 15

// tex channel number to button IDC
static int texMButtonsIDC[] = {
	IDC_MAPON_AM, IDC_MAPON_DI, IDC_MAPON_DIFFLEV, IDC_MAPON_DIFFROUGH,
	IDC_MAPON_SPECCLR1, IDC_MAPON_SPECLEV1, IDC_MAPON_GL1, IDC_MAPON_AN1, IDC_MAPON_OR1, 
	IDC_MAPON_SPECCLR2, IDC_MAPON_SPECLEV2, IDC_MAPON_GL2, IDC_MAPON_AN2, IDC_MAPON_OR2, 
	IDC_MAPON_SI, IDC_MAPON_TR, 
	-1,  -1, -1, -1, -1, -1, -1, -1
	};
		
// This array gives the texture map number for given MButton number								
static int texmapFromMBut[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

// Texture Maps
#define MULTI_LAYER_NTEXMAPS	17

// channels ids needed by shader
#define _AMBCLR		0
#define _DIFFCLR	1
#define _DIFFLEV	2
#define _DIFFROUGH	3
#define _SPECCLR1	4
#define _SPECLEV1	5
#define _GL1		6
#define _AN1		7
#define _OR1		8
#define _SPECCLR2	9
#define _SPECLEV2	10
#define _GL2		11
#define _AN2		12
#define _OR2		13
#define _SI			14
#define _TR			15

// channel names
static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_DS_AMBIENT, IDS_DS_DIFFUSE, IDS_KE_DIFF_LEVEL, IDS_KE_DIFF_ROUGH,
	IDS_KE_SPEC_CLR1, IDS_KE_SPEC_LEVEL1, IDS_KE_GLOSS1, IDS_KE_ANISO1, IDS_KE_ORIENTATION1,
	IDS_KE_SPEC_CLR2, IDS_KE_SPEC_LEVEL2, IDS_KE_GLOSS2, IDS_KE_ANISO2, IDS_KE_ORIENTATION2, 
	IDS_KE_SELFILLUM, IDS_DS_TRANS, IDS_DS_FILTER, 
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,  
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, 
};	

// internal non-local parsable channel map names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("ambientMap"), _T("diffuseMap"),	_T("diffuseLevelMap"), _T("diffuseRoughnessMap"),
	_T("specularMap"), _T("specularLevelMap"), _T("glossinessMap"), _T("anisotropyMap"), _T("orientationMap"),
	_T("specularMap2"), _T("specularLevelMap2"), _T("glossinessMap2"), _T("anisotropyMap2"), _T("orientationMap2"),
	_T("selfIllumMap"), _T("opacityMap"), _T("filterMap"),	
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""),
};	


// sized for nmax textures
// bump, reflection & refraction maps are ignored....done after shading
static int chanType[STD2_NMAX_TEXMAPS] = {
	CLR_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, MONO_CHANNEL,
	CLR_CHANNEL, SLEV_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, 
	CLR_CHANNEL, SLEV_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, 
	CLR_CHANNEL, MONO_CHANNEL, CLR_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL,  
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
};	

// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { 0, 1, 4, 6, 5, 14, 15, 16, -1, -1, -1, -1, -1, -1, -1, -1 };

//////////////////////////////////////////////////////////////////////////////////////////
//
//		MultiLayer Parameter Block
//
#define CURRENT_MULTI_LAYER_SHADER_VERSION	2
#define MULTI_LAYER_SHADER_NPARAMS			17
#define MULTI_LAYER_SHADER_PB_VERSION		1
#define MULTI_LAYER_NCOLBOX					5

static int colID[MULTI_LAYER_NCOLBOX] = { IDC_AMB_CLR, IDC_DIFF_CLR, IDC_SI_CLR, IDC_SPEC_CLR1, IDC_SPEC_CLR2 };
#define N_SI_CLR		2

#define PB_AMB_CLR			0
#define PB_DIFF_CLR			1
#define PB_DIFF_LEV			2
#define PB_DIFF_ROUGH		3
#define PB_SPEC_CLR1		4
#define PB_SPEC_LEV1		5
#define PB_GLOSS1			6
#define PB_ANISO1			7
#define PB_SPEC_CLR2		8
#define PB_SPEC_LEV2		9
#define PB_GLOSS2			10
#define PB_ANISO2			11
#define PB_SELFILLUM_CLR	12
#define PB_SELFILLUM_LEV	13
#define PB_ORIENTATION1		14
#define PB_ORIENTATION2		15
#define PB_MAP_CHANNEL		16

#define ANIMATE		TRUE
#define NO_ANIMATE	FALSE

//Current Param Block Descriptor
static ParamBlockDescID MultiLayerShaderPB[ MULTI_LAYER_SHADER_NPARAMS ] = {
	{ TYPE_RGBA,  NULL, ANIMATE, ml_ambient },   // ambient color
	{ TYPE_RGBA,  NULL, ANIMATE, ml_diffuse },   // diffuse color
	{ TYPE_FLOAT, NULL, ANIMATE, ml_diffuse_level },  // diffuse level
	{ TYPE_FLOAT, NULL, ANIMATE, ml_diffuse_rough },  // diffuse roughness
	{ TYPE_RGBA,  NULL, ANIMATE, ml_specular1 },   // specular layer 1 color
	{ TYPE_FLOAT, NULL, ANIMATE, ml_specular_level1 },   // layer 1 level
	{ TYPE_FLOAT, NULL, ANIMATE, ml_glossiness1 },   // layer 1 gloss
	{ TYPE_FLOAT, NULL, ANIMATE, ml_anisotropy1 },  // layer 1 anisotropy
	{ TYPE_RGBA,  NULL, ANIMATE, ml_specular2 },   // Specular layer 2 color
	{ TYPE_FLOAT, NULL, ANIMATE, ml_specular_level2 },   // layer 2 level
	{ TYPE_FLOAT, NULL, ANIMATE, ml_glossiness2 },   // layer 2 gloss
	{ TYPE_FLOAT, NULL, ANIMATE, ml_anisotropy2 },  // layer 2 anisotropy
	{ TYPE_RGBA,  NULL, ANIMATE, ml_self_illum_color },	// selfIllumClr 
	{ TYPE_FLOAT, NULL, ANIMATE, ml_self_illum_amnt},	// selfIllumLevel
	{ TYPE_FLOAT, NULL, ANIMATE, ml_orientation1 },	// orientation1
	{ TYPE_FLOAT, NULL, ANIMATE, ml_orientation2 },	// orientation2
	{ TYPE_INT,	  NULL, NO_ANIMATE, ml_map_channel } // map channel
}; 


#define MULTI_LAYER_NUMVER 1

static ParamVersionDesc oldVersions[MULTI_LAYER_NUMVER] = {
	ParamVersionDesc(MultiLayerShaderPB, MULTI_LAYER_SHADER_NPARAMS, 0),
};

//----------------------------------------------------------------------------------------
//---- MultiLayer: A 2 layer physically based anisotropic shader -------------------------------
//----------------------------------------------------------------------------------------

class MultiLayerShaderDlg;

class MultiLayerShader : public ExposureMaterialControlImp<MultiLayerShader, CombineComponentsCompShader> {
friend class MultiLayerShaderCB;
friend class MultiLayerShaderDlg;
friend class ExposureMaterialControlImp<MultiLayerShader, CombineComponentsCompShader>;
protected:
	IParamBlock2	*pblock;   // ref 0
	Interval		ivalid;

	MultiLayerShaderDlg*	paramDlg;

	Color			ambColor;
	Color			diffColor;
	float			diffLevel;
	float			diffRough;

	Color			specColor1;
	float			specLevel1;
	float			gloss1;
	float			aniso1;
	float			orientation1;

	Color			specColor2;
	float			specLevel2;
	float			gloss2;
	float			aniso2;
	float			orientation2;

	float			selfIllumLevel;
	Color			selfIllum;
	BOOL			selfIllumClrOn;


	BOOL			lockAD;
	BOOL			lockADTex;

	static CombineComponentsFPDesc msExpMtlControlDesc;

	public:
	MultiLayerShader();
	void DeleteThis(){ delete this; }	
    ULONG SupportStdParams(){ return (STD_MULTILAYER | STD_EXTRA); }

	// copy std params, for switching shaders
    void CopyStdParams( Shader* pFrom );

	// texture maps
	long  nTexChannelsSupported(){ return MULTI_LAYER_NTEXMAPS; }
	TSTR  GetTexChannelName( long nTex ){ return GetString( texNameIDS[ nTex ] ); }
	TSTR  GetTexChannelInternalName( long nTex ){ return texInternalNames[ nTex ]; }
	long  ChannelType( long nChan ) { return chanType[nChan]; }
	long  StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

	BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int ){ paramDlg = (MultiLayerShaderDlg*)newDlg; }

	Class_ID ClassID() { return MultiLayerShaderClassID; }
	SClass_ID SuperClassID() { return SHADER_CLASS_ID; }
	TSTR GetName() { return GetString( IDS_KE_MULTI_LAYER ); }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i){ return (i==0)? pblock : NULL; }
	TSTR SubAnimName(int i){ return TSTR(GetString( IDS_KE_MULTI_LAYER_PARMS )); };
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

	// cache for mapping of params, mtl fills in ip
	void GetIllumParams( ShadeContext &sc, IllumParams& ip );

	// MultiLayer Shader specific section
	void PreShade(ShadeContext& sc, IReshadeFragment* pFrag);
	void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip);
	void  Illum(ShadeContext &sc, IllumParams &ip);
	void  InternalIllum(ShadeContext &sc, IllumParams &ip, Point3* pTangent );
	float EvalHiliteCurve2(float x, float y, int layer );	// layer is 1 or 2
	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol);

	void SetGlossiness(float v, TimeValue t)		
		{ gloss1= v; pblock->SetValue( ml_glossiness1, t, v); }
	void SetGlossiness1(float v, TimeValue t)		
		{ gloss1= v; pblock->SetValue( ml_glossiness1, t, v); }
	void SetAnisotropy1(float v, TimeValue t)		
		{ aniso1 = v; pblock->SetValue( ml_anisotropy1, t, v); }
	void SetSpecLevel1(float v, TimeValue t)		
		{ specLevel1 = v; pblock->SetValue( ml_specular_level1, t, v); }
	void SetOrientation1(float v, TimeValue t)		
		{ orientation1 = v; pblock->SetValue( ml_orientation1, t, v); }
	void SetOrientation2(float v, TimeValue t)		
		{ orientation2 = v; pblock->SetValue( ml_orientation2, t, v); }
	float GetGlossiness(int mtlNum, BOOL backFace){ return gloss1; }	
	float GetGlossiness1(int mtlNum, BOOL backFace){ return gloss1; }	
	float GetAnisotropy1(int mtlNum, BOOL backFace){ return aniso1; }	
	float GetSpecLevel1(int mtlNum, BOOL backFace){ return specLevel1; }	
	float GetOrientation1(int mtlNum, BOOL backFace){ return orientation1; }	
	float GetOrientation2(int mtlNum, BOOL backFace){ return orientation2; }	
	float GetGlossiness( TimeValue t){return pblock->GetFloat(ml_glossiness1,t);  }		
	float GetGlossiness1( TimeValue t){return pblock->GetFloat(ml_glossiness1,t);  }		
	float GetAnisotropy1( TimeValue t){return pblock->GetFloat(ml_anisotropy1,t);  }		
	float GetSpecLevel1( TimeValue t){return pblock->GetFloat(ml_specular_level1,t);  }		
	float GetOrientation1( TimeValue t){return pblock->GetFloat(ml_orientation1,t);  }		
	float GetOrientation2( TimeValue t){return pblock->GetFloat(ml_orientation2,t);  }		

	void SetGlossiness2(float v, TimeValue t)		
		{ gloss2= v; pblock->SetValue( ml_glossiness2, t, v); }
	void SetAnisotropy2(float v, TimeValue t)		
		{ aniso2 = v; pblock->SetValue( ml_anisotropy2, t, v); }
	void SetSpecLevel2(float v, TimeValue t)		
		{ specLevel2 = v; pblock->SetValue( ml_specular_level2, t, v); }
	float GetGlossiness2(int mtlNum, BOOL backFace){ return gloss2; };	
	float GetAnisotropy2(int mtlNum, BOOL backFace){ return aniso2; };	
	float GetSpecLevel2(int mtlNum, BOOL backFace){ return specLevel2; };	
	float GetGlossiness2( TimeValue t){return pblock->GetFloat(ml_glossiness2,t);  }		
	float GetAnisotropy2( TimeValue t){return pblock->GetFloat(ml_anisotropy2,t);  }		
	float GetSpecLevel2( TimeValue t){return pblock->GetFloat(ml_specular_level2,t);  }		

	void SetDiffuseLevel(float v, TimeValue t)		
		{ diffLevel = v; pblock->SetValue( ml_diffuse_level, t, v); }
	float GetDiffuseLevel(int mtlNum, BOOL backFace){ return diffLevel; }	
	float GetDiffuseLevel( TimeValue t ){ return pblock->GetFloat(ml_diffuse_level,t); }		
	void SetDiffuseRough(float v, TimeValue t)		
		{ diffRough = v; pblock->SetValue( ml_diffuse_rough, t, v); }
	float GetDiffuseRough(int mtlNum, BOOL backFace){ return diffRough; }	
	float GetDiffuseRough( TimeValue t ){ return pblock->GetFloat(ml_diffuse_rough,t); }		

	void SetSpecColor1(Color c, TimeValue t)
		{ specColor1 = c; pblock->SetValue( ml_specular1, t, c); }
	void SetSpecColor2(Color c, TimeValue t)		
		{ specColor2 = c; pblock->SetValue( ml_specular2, t, c); }

    Color GetSpecColor1(int mtlNum, BOOL backFace){ return specColor1;}		
	Color GetSpecColor1(TimeValue t){ return pblock->GetColor(ml_specular1,t);	}
	Color GetSpecColor2(int mtlNum, BOOL backFace){ return specColor2; }
	Color GetSpecColor2(TimeValue t){ return pblock->GetColor(ml_specular2,t); }		

	// Std Params, these define the interactive settings for OGL, etc....
	void SetAmbientClr(Color c, TimeValue t)		
		{ ambColor = c; pblock->SetValue( ml_ambient, t, c); }
	void SetDiffuseClr(Color c, TimeValue t)		
		{ diffColor = c; pblock->SetValue( ml_diffuse, t, c); }
	void SetSpecularClr(Color c, TimeValue t)
		{ SetSpecColor1( c, t ); }
	void SetSelfIllumClr(Color c, TimeValue t)
		{ selfIllum = c; pblock->SetValue( ml_self_illum_color, t, c); }
	void SetSpecularLevel(float v, TimeValue t)
		{SetSpecLevel1( v, t ); }	
	void SetSelfIllum(float v, TimeValue t)
		{ selfIllumLevel = v; pblock->SetValue( ml_self_illum_amnt, t, v); }

	Color GetAmbientClr(int mtlNum, BOOL backFace ){ return ambColor;}		
    Color GetDiffuseClr(int mtlNum, BOOL backFace ){ return diffColor;}		
	Color GetSpecularClr(int mtlNum, BOOL backFace ){ return specColor1; }
	Color GetSelfIllumClr(int mtlNum, BOOL backFace ){ return selfIllum; }
	float GetSelfIllum(int mtlNum, BOOL backFace ){ return selfIllumLevel; }
	float GetSpecularLevel(int mtlNum, BOOL backFace ){ return specLevel1; }	

	Color GetAmbientClr(TimeValue t){ return pblock->GetColor(ml_ambient,t); }		
	Color GetDiffuseClr(TimeValue t){ return pblock->GetColor(ml_diffuse,t); }		
	Color GetSpecularClr(TimeValue t){ return GetSpecColor1( t );	}
	Color GetSelfIllumClr(TimeValue t){ return pblock->GetColor(ml_self_illum_color,t);}		
	float GetSelfIllum(TimeValue t){ return  pblock->GetFloat(ml_self_illum_amnt,t);}		
	float GetSpecularLevel( TimeValue t){return GetSpecLevel1( t ); }		

	void SetSelfIllumClrOn( BOOL on ){ selfIllumClrOn = on; pblock->SetValue( ml_use_self_illum_color, 0, on); }
	BOOL IsSelfIllumClrOn(){ return selfIllumClrOn; }
	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return selfIllumClrOn; }

	void SetLockAD(BOOL lock){ lockAD = lock; pblock->SetValue( ml_ad_lock, 0, lock);  }
	BOOL GetLockAD(){ return lockAD; }
	void SetLockADTex(BOOL lock){ lockADTex = lock; pblock->SetValue( ml_ad_texlock, 0, lock);  }
	BOOL GetLockADTex(){ return lockADTex; }

	// not supported
	void SetSoftenLevel(float v, TimeValue t) {}
	float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE){ return DEFAULT_SOFTEN; }
	float GetSoftenLevel(TimeValue t){ return  DEFAULT_SOFTEN; }
	void SetLockDS(BOOL lock){}
	BOOL GetLockDS(){ return FALSE; }
};

///////////// Class Descriptor ////////////////////////
class MultiLayerShaderClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new MultiLayerShader(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_MULTI_LAYER); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return MultiLayerShaderClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("MultiLayer"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			 // returns owning module handle
};

MultiLayerShaderClassDesc multiLayerCD;
ClassDesc * GetMultiLayerShaderCD(){ return &multiLayerCD; }

// shader parameters
static ParamBlockDesc2 multiLayer_param_blk ( multiLayer_params, _T("shaderParameters"),  0, &multiLayerCD, P_AUTO_CONSTRUCT, 0, 
	// params
	ml_ambient, _T("ambient"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_AMBIENT, 
		p_default, Color(0, 0, 0), 
		end,
	ml_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_DIFFUSE, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	ml_self_illum_color, _T("selfIllumColor"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_SELFILLUM_CLR,	
		p_default,		Color(0, 0, 0), 
		end,
	ml_self_illum_amnt, _T("selfIllumAmount"), TYPE_PCNT_FRAC,	P_ANIMATABLE, IDS_KE_SELFILLUM,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	ml_diffuse_level, _T("diffuseLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_DIFF_LEVEL,
		p_default,		100.0,
		p_range,		0.0, 400.0,
		end,
	ml_diffuse_rough, _T("diffuseRoughness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_DIFF_ROUGH,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	ml_specular1, _T("specular"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_CLR1, 
		p_default, Color(1.0f, 1.0f, 1.0f), 
		end,
	ml_specular_level1, _T("specularLevel"), TYPE_PCNT_FRAC, P_ANIMATABLE,IDS_KE_LEVEL1,
		p_default,	 	10.0,
		p_range,		0.0, 999.0,
		end,
	ml_glossiness1, _T("glossiness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_GLOSS1,
		p_default,		25.0,
		p_range,		0.0, 100.0,
		end,
	ml_anisotropy1, _T("anisotropy"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_ANISO1,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	ml_orientation1, _T("orientation"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_ORIENTATION1,
		p_default,		0.0,
		p_range,		MIN_ORIENT*100.0, MAX_ORIENT*100.0,
		end,
	ml_specular2, _T("specular2"), TYPE_RGBA, P_ANIMATABLE, IDS_KE_CLR2, 
		p_default, Color(1.0f, 1.0f, 1.0f), 
		end,
	ml_specular_level2, _T("specularLevel2"), TYPE_PCNT_FRAC, P_ANIMATABLE,IDS_KE_LEVEL2,
		p_default,	 	0.0,
		p_range,		0.0, 999.0,
		end,
	ml_glossiness2, _T("glossiness2"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_GLOSS2,
		p_default,		25.0,
		p_range,		0.0, 100.0,
		end,
	ml_anisotropy2, _T("anisotropy2"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_ANISO2,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	ml_orientation2, _T("orientation2"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_ORIENTATION2,
		p_default,		0.0,
		p_range,		MIN_ORIENT*100.0, MAX_ORIENT*100.0,
		end,
	ml_map_channel, _T("unused"), TYPE_INT, 0, IDS_KE_MAPCHANNEL,
		p_default,		0,
		p_range,		0, 16,
		end,
	ml_ad_texlock, _T("adTextureLock"), TYPE_BOOL,	0, IDS_JW_ADTEXLOCK, 
		p_default, TRUE, 
		end,
	ml_ad_lock, _T("adLock"), TYPE_BOOL, 0, IDS_JW_ADLOCK, 
		p_default, TRUE, 
		end,
	ml_use_self_illum_color, _T("useSelfIllumColor"), TYPE_BOOL, 0, IDS_JW_SELFILLUMCOLORON,
		p_default, FALSE, 
		end,
	end
);



CombineComponentsFPDesc MultiLayerShader::msExpMtlControlDesc(multiLayerCD);

MultiLayerShader::MultiLayerShader() 
{ 
	pblock = NULL; 
	multiLayerCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	paramDlg = NULL; 
 
	selfIllumClrOn = 0;
	lockAD = lockADTex = TRUE;
	ambColor = diffColor = specColor1 = specColor2 = selfIllum = Color(0.0f,0.0f,0.0f);
	gloss1 = orientation1 = aniso1 = specLevel1 
	= gloss2 = orientation2 = aniso2 = specLevel2 
	= diffLevel = diffRough = selfIllumLevel = 0.0f;
	
	ivalid.SetEmpty(); 
}


void MultiLayerShader::CopyStdParams( Shader* pFrom )
{
	macroRecorder->Disable(); 
	// don't want to see this parameter copying in macrorecorder
		long fromParms = pFrom->SupportStdParams();

		if ( fromParms & STD_PARAM_SELFILLUM_CLR_ON )
			SetSelfIllumClrOn( pFrom->IsSelfIllumClrOn() );

		if ( fromParms & STD_PARAM_AMBIENT_CLR )
			SetAmbientClr( pFrom->GetAmbientClr(0,0), 0 );

		if ( fromParms & STD_PARAM_DIFFUSE_CLR )
			SetDiffuseClr( pFrom->GetDiffuseClr(0,0), 0 );
		
		if ( fromParms & STD_PARAM_SPECULAR_CLR )
			SetSpecularClr( pFrom->GetSpecularClr(0,0), 0 );

		if ( fromParms & STD_PARAM_SELFILLUM_CLR )
			SetSelfIllumClr( pFrom->GetSelfIllumClr(0,0), 0 );

		if ( fromParms & STD_PARAM_SPECULAR_LEV )
			SetSpecularLevel( pFrom->GetSpecularLevel(0,0), 0 );

		if ( fromParms & STD_PARAM_GLOSSINESS )
			SetGlossiness( pFrom->GetGlossiness(0,0), 0 );

		if ( fromParms & STD_PARAM_SELFILLUM )
			SetSelfIllum( pFrom->GetSelfIllum(0,0), 0 );

		if ( fromParms & STD_PARAM_LOCKAD )
			SetLockAD( pFrom->GetLockAD() );

		if ( fromParms & STD_PARAM_LOCKADTEX )
			SetLockADTex( pFrom->GetLockADTex() );

	macroRecorder->Enable();
	ivalid.SetEmpty();	
}


RefTargetHandle MultiLayerShader::Clone( RemapDir &remap )
{
	MultiLayerShader* mnew = new MultiLayerShader();
	mnew->ExposureMaterialControl::operator=(*this);
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	

	mnew->ambColor = ambColor;
	mnew->diffColor = diffColor;

	mnew->diffLevel = diffLevel;
	mnew->diffRough = diffRough;

	mnew->selfIllum = selfIllum;
	mnew->selfIllumLevel = selfIllumLevel;
	mnew->selfIllumClrOn = selfIllumClrOn;

	mnew->specColor1 = specColor1;
	mnew->specLevel1 = specLevel1;
	mnew->gloss1 = gloss1;
	mnew->aniso1 = aniso1;
	mnew->orientation1 = orientation1;

	mnew->specColor2 = specColor2;
	mnew->specLevel2 = specLevel2;
	mnew->gloss2 = gloss2;
	mnew->aniso2 = aniso2;
	mnew->orientation2 = orientation2;

	mnew->lockAD = lockAD;
	mnew->lockADTex = lockADTex;

	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}


void MultiLayerShader::GetIllumParams( ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
	ip.channels[_AMBCLR] = lockAD? diffColor : ambColor;
	ip.channels[_DIFFCLR] = diffColor;
	ip.channels[_DIFFLEV].r = diffLevel;
	ip.channels[_DIFFROUGH].r = diffRough;

	ip.channels[_SPECCLR1] = specColor1;
	ip.channels[_SPECCLR2] = specColor2;

	ip.channels[_SPECLEV1].r = specLevel1;
	ip.channels[_GL1].r = gloss1;
	ip.channels[_AN1].r = aniso1;
	ip.channels[_OR1].r = orientation1 * (1.0f/1.8f); // this range for mono texturing, 0..1

	ip.channels[_SPECLEV2].r = specLevel2;
	ip.channels[_GL2].r = gloss2;
	ip.channels[_AN2].r = aniso2;
	ip.channels[_OR2].r = orientation2 * (1.0f/1.8f);

	if (selfIllumClrOn )
		ip.channels[_SI] = selfIllum;
	else
		ip.channels[_SI].r=ip.channels[_SI].g=ip.channels[_SI].b = selfIllumLevel;
}

static BOOL inUpdate = FALSE;


void MultiLayerShader::Update(TimeValue t, Interval &valid) {
	Point3 p, p2;
	if( inUpdate )
		return;
	inUpdate = TRUE;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

//		pblock->GetValue( ml_ambient, t, p, ivalid );
//		ambColor = Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( ml_diffuse, t, p, ivalid );
		diffColor = Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( ml_ambient, t, p2, ivalid );
		if( lockAD && (p!=p2)){
			pblock->SetValue( ml_ambient, t, diffColor);
			ambColor = diffColor;
		} else {
			pblock->GetValue( ml_ambient, t, p, ivalid );
			ambColor = Bound(Color(p.x,p.y,p.z));
		}
		pblock->GetValue( ml_diffuse_level, t, diffLevel, ivalid );
		diffLevel = Bound(diffLevel, 0.0f, 4.0f );
		pblock->GetValue( ml_diffuse_rough, t, diffRough, ivalid );
		diffRough = Bound(diffRough);

		pblock->GetValue( ml_specular1, t, p, ivalid );
		specColor1 = Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( ml_specular_level1, t, specLevel1, ivalid );
		specLevel1 = Bound( specLevel1, 0.0f, 9.99f );
		pblock->GetValue( ml_glossiness1, t, gloss1, ivalid );
		gloss1 = Bound( gloss1 );
		pblock->GetValue( ml_anisotropy1, t, aniso1, ivalid );
		aniso1 = Bound( aniso1 );
		pblock->GetValue( ml_orientation1, t, orientation1, ivalid );
		orientation1 = Bound( orientation1, (float)MIN_ORIENT, (float)MAX_ORIENT );

		pblock->GetValue( ml_specular2, t, p, ivalid );
		specColor2 = Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( ml_specular_level2, t, specLevel2, ivalid );
		specLevel2 = Bound( specLevel2, 0.0f, 9.99f );
		pblock->GetValue( ml_glossiness2, t, gloss2, ivalid );
		gloss2 = Bound( gloss2 );
		pblock->GetValue( ml_anisotropy2, t, aniso2, ivalid );
		aniso2 = Bound( aniso2 );
		pblock->GetValue( ml_orientation2, t, orientation2, ivalid );
		orientation2 = Bound( orientation2, (float)MIN_ORIENT, (float)MAX_ORIENT );

		pblock->GetValue( ml_self_illum_amnt, t, selfIllumLevel, ivalid );
		selfIllumLevel = Bound(selfIllumLevel);
		pblock->GetValue( ml_self_illum_color, t, p, ivalid );
		selfIllum = Bound(Color(p.x,p.y,p.z));

		// also get the non-animatables in case changed from scripter or other pblock accessors
		pblock->GetValue(ml_ad_lock, t, lockAD, ivalid);
		pblock->GetValue(ml_ad_texlock, t, lockADTex, ivalid);
		pblock->GetValue(ml_use_self_illum_color, t, selfIllumClrOn, ivalid);

	}
	valid &= ivalid;
	inUpdate = FALSE;
}

void MultiLayerShader::Reset()
{
//	multiLayerCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	ivalid.SetEmpty();

	SetAmbientClr(Color(0.2f,0.2f,0.2f),0);
	SetDiffuseClr(Color(0.5f,0.5f,0.5f),0);
	SetDiffuseLevel( 1.0f,0 );
	SetDiffuseRough( 0.0f,0 );

	SetSpecColor1(Color(0.9f,0.9f,0.9f),0);
	SetSpecLevel1(0.05f,0);   
	SetGlossiness1(.50f,0);   
	SetAnisotropy1(0.0f,0);   
	SetOrientation1(0.0f,0);   

	SetSpecColor2(Color(0.9f,0.9f,0.9f),0);
	SetSpecLevel2(0.0f,0);   
	SetGlossiness2(.25f,0);   
	SetAnisotropy2(0.0f,0);   
	SetOrientation2(0.0f,0);   

	SetSelfIllumClr(Color(0.0f,0.0f,0.0f),0);
	SetSelfIllum( 0.0f,0 );
	SetSelfIllumClrOn( FALSE );

	SetLockADTex( TRUE );
	SetLockAD( TRUE );					// > 6/13/02 - 4:48pm --MQM-- was FALSE
}



//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
#define SHADER_HDR_CHUNK 0x4000
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_SELFILLUMCLR_ON_CHUNK	0x5006
#define SHADER_LOCKAD_ON_CHUNK	0x5007
#define SHADER_LOCKADTEX_ON_CHUNK	0x5008
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020

// IO
IOResult MultiLayerShader::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = CURRENT_MULTI_LAYER_SHADER_VERSION;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();
	return IO_OK;
}		

class MultiLayerShaderCB: public PostLoadCallback {
	public:
		MultiLayerShader *s;
		int loadVersion;
	    MultiLayerShaderCB(MultiLayerShader *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {
			// convert old v1 ParamBlock to ParamBlock2
			s->ReplaceReference(0,
				UpdateParameterBlock2(MultiLayerShaderPB, MULTI_LAYER_SHADER_NPARAMS, (IParamBlock*)s->pblock, &multiLayer_param_blk));

			// then set values that were previously stored outside the PB
			s->pblock->SetValue(ml_use_self_illum_color, 0, s->selfIllumClrOn);
			s->pblock->SetValue(ml_ad_lock, 0, s->lockAD);
			s->pblock->SetValue(ml_ad_texlock, 0, s->lockADTex);
		}
};


IOResult MultiLayerShader::Load(ILoad *iload)
{ 
	ULONG nb;
	int id;
	int version = 0;

	lockAD = lockADTex = selfIllumClrOn = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SHADER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
			case SHADER_SELFILLUMCLR_ON_CHUNK:
				selfIllumClrOn = TRUE;
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
	if (version < CURRENT_MULTI_LAYER_SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new MultiLayerShaderCB(this, version));
		iload->SetObsolete();
	}

	return IO_OK;

}

			
///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//
// defualt gloss squared, for renormalized controls
#define DEFAULT_GLOSS2	0.03f	

float MultiLayerShader::EvalHiliteCurve2( float x, float y, int layer )
{
	float g = (layer == 1)? gloss1 : gloss2;
	float a = (layer == 1)? aniso1 : aniso2;
	float specLevel = (layer == 1)? specLevel1 : specLevel2;

	return GaussHiliteCurve2( x, y, specLevel, g, a);

}


void MultiLayerShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol)
{
	//NB kl,aka NL & g cancel out
//	rcol *= ip.channels[_SPECLEV1].r * ip.channels[_SPECCLR1] * DEFAULT_K_REFL; 
	rcol *= ip.channels[_SPECCLR1] * DEFAULT_K_REFL; 
}



static int stopX = -1;
static int stopY = -1;

void MultiLayerShader::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
//	Point3 U = sc.VectorFrom( Point3( 0.01f, 0.0f, 1.0f ), REF_OBJECT );
//	U = Normalize( U );
	Point3 T = GetTangent( sc, 0 );
	pFrag->AddUnitVecChannel( T );
}

void MultiLayerShader::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip)
{
	Point3 T = pFrag->GetUnitVecChannel( nextTexIndex++ );
	InternalIllum( sc, *ip, &T );
}


void MultiLayerShader::Illum(ShadeContext &sc, IllumParams &ip) 
{
	InternalIllum( sc, ip, NULL );
}

void MultiLayerShader::InternalIllum(ShadeContext &sc, IllumParams &ip, Point3* pTangent )
{
	LightDesc *l;
	Color lightCol;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if ( sp.x == stopX && sp.y == stopY )
		sp.x = stopX;
#endif

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
			Color spec(0.0f, 0.0f, 0.0f);
			if (l->affectSpecular) {
				Point3 T = (pTangent)? *pTangent : GetTangent( sc, 0 ); 
				float g1 = GaussHighlight( ip.channels[_GL1].r, ip.channels[_AN1].r,
											ip.channels[_OR1].r, sc.Normal(), sc.V(), L, T, &NL );
										
				Color spec1 = g1 * kL * ip.channels[_SPECLEV1].r * ip.channels[_SPECCLR1] * lightCol;
				//spec1 = Bound( spec1 );

				float g2 = GaussHighlight( ip.channels[_GL2].r, ip.channels[_AN2].r,
							ip.channels[_OR2].r, sc.Normal(), sc.V(), L, T, &NL );

				Color spec2 = g2 * kL * ip.channels[_SPECLEV2].r * ip.channels[_SPECCLR2] * lightCol;

				// composite spec1 over spec2
				spec = spec1;
				if (getUseComposite()) {
					Color rem = 1.0f - Bound( spec1 );
					spec += rem * spec2;
				}
				else {
					spec += spec2;
				}
				ip.specIllumOut += spec;

			} // end, affect specular

		 	// diffuse
			if (l->affectDiffuse){
//				Color d = OrenNayarIllum( ip.N, L, -ip.V, ip.channels[_DIFFROUGH].r * Pi*0.5f, ip.channels[_DIFFCLR], NL );
//				d = d * ip.channels[_DIFFLEV].r; // diff lev
//				ip.diffIllumOut += kL * d * lightCol;
				float diffIntens;
				Color d = OrenNayarIllum( sc.Normal(), L, sc.V(), ip.channels[_DIFFROUGH].r * Pi*0.5f, ip.channels[_DIFFCLR], &diffIntens, NL );
				d = d * ip.channels[_DIFFLEV].r; 
				ip.diffIllumOut += kL * d * lightCol;
				ip.diffIllumIntens += kL * diffIntens * Intens(lightCol);
			}

		} // end, illuminated
	} // for each light

	// Apply mono self illumination
	if ( ! selfIllumClrOn ){
		float si = 0.3333333f * (ip.channels[_SI].r + ip.channels[_SI].g + ip.channels[_SI].b);
//		float si = ip.channels[_SI].r;
		if ( si > 0.0f ) {
			if ( si >= 1.0f ) {
				ip.selfIllumOut +=  ip.channels[_DIFFCLR];
				ip.diffIllumOut = Color( 0.0f, 0.0f, 0.0f );
			} else {
				ip.selfIllumOut += si * ip.channels[_DIFFCLR];;
				ip.diffIllumOut *= (1.0f - si); // * ip.channels[_DIFFCLR]; 
				// fade the ambient down on si: 5/27/99 ke
				ip.ambIllumOut *= 1.0f-si;
			}
		}
	} else {
		// colored self illum, 
		ip.selfIllumOut += ip.channels[_SI];
	}

	// now we can multiply by the clrs
	ip.ambIllumOut *= ip.channels[_AMBCLR]; 

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

	// get the diffuse intensity...unscramble the wavelength dependence
//	float rho, diffIntens;
//	rho = ip.channels[_DIFFCLR].r == 0.0f ? 1.0f : 1.0f / ip.channels[_DIFFCLR].r;
//	diffIntens = ip.diffIllumOut.r * rho;
//	rho = ip.channels[_DIFFCLR].g == 0.0f ? 1.0f : 1.0f / ip.channels[_DIFFCLR].g;
//	diffIntens += ip.diffIllumOut.g * rho;
//	rho = ip.channels[_DIFFCLR].b == 0.0f ? 1.0f : 1.0f / ip.channels[_DIFFCLR].b;
//	diffIntens += ip.diffIllumOut.b * rho;
//	ip.diffIllumIntens = diffIntens * 0.5f;
}



///////////////////////// The dialog class //////////////////////////////////
class MultiLayerShaderDlg : public ShaderParamDlg {
public:
	MultiLayerShader*	pShader;
	StdMat2*	pMtl;
	HPALETTE	hOldPal;
	HWND		hwmEdit;	// window handle of the materials editor dialog
	IMtlParams*	pMtlPar;
	HWND		hwHilite1;   // the hilite windows
	HWND		hwHilite2;
	HWND		hRollup;	// Rollup panel
	TimeValue	curTime;
	BOOL		valid;
	BOOL		isActive;

	IColorSwatch *cs[MULTI_LAYER_NCOLBOX];
	ISpinnerControl *lev1Spin, *gl1Spin, *an1Spin;
	ISpinnerControl *lev2Spin, *gl2Spin, *an2Spin;
	ISpinnerControl *dlevSpin, *droughSpin,*or1Spin, *or2Spin, *trSpin, *siSpin;
	ICustButton* texMBut[MULTI_LAYER_NMBUTS];
	TexDADMgr dadMgr;
	
	MultiLayerShaderDlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~MultiLayerShaderDlg(); 

	// required for correctly operating map buttons
	int FindSubTexFromHWND(HWND hw) {
		for (long i=0; i < MULTI_LAYER_NMBUTS; i++) {
			if (hw == texMBut[i]->GetHwnd()) 
				return texmapFromMBut[i];
		}	
		return -1;
	}

	// Methods
	BOOL PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ); 
	Class_ID ClassID(){ return MultiLayerShaderClassID; }

	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (MultiLayerShader*)theShader; 
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
    void UpdateHilite( long nHilite );
	void UpdateColSwatches();
	void UpdateMapButtons();
	void UpdateOpacity();
	void SetLockADTex(BOOL lock);
	void SetLockAD(BOOL lock);
	void UpdateLockADTex( BOOL passOn);	
	
	void SelectEditColor(int i) { cs[ i ]->EditThis(FALSE); }
};

static INT_PTR CALLBACK  MultiLayerShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	MultiLayerShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (MultiLayerShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (MultiLayerShaderDlg*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* MultiLayerShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	paramDlg = new MultiLayerShaderDlg(hwMtlEdit, imp);
	paramDlg->SetThings( theMtl, this  );

	LoadStdShaderResources();
	if ( hOldRollup ) {
		paramDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_MULTILAYER1),
			MultiLayerShaderDlgProc, 
			GetString(IDS_KE_MULTI_LAYER_BASIC),	// your name here
			(LPARAM)paramDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);
	} else
		paramDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_MULTILAYER1),
			MultiLayerShaderDlgProc, 
			GetString(IDS_KE_MULTI_LAYER_BASIC),	
			(LPARAM)paramDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)paramDlg;	
}

RefResult MultiLayerShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
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


MultiLayerShaderDlg::MultiLayerShaderDlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	dlevSpin = droughSpin =lev1Spin = lev2Spin = gl1Spin = an1Spin =gl2Spin = an2Spin = NULL;
	or1Spin = or2Spin = trSpin = siSpin = NULL;
	hRollup = hwHilite1 = hwHilite2 =NULL;
	curTime = pMtlPar->GetTime();

	isActive = valid = FALSE;

	for( long i = 0; i < MULTI_LAYER_NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < MULTI_LAYER_NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

MultiLayerShaderDlg::~MultiLayerShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < MULTI_LAYER_NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}

	for (i=0; i<MULTI_LAYER_NCOLBOX; i++)
		if (cs[i]) ReleaseIColorSwatch(cs[i]); // mjm - 5.10.99

 	ReleaseISpinner(dlevSpin);
 	ReleaseISpinner(droughSpin);
 	ReleaseISpinner(lev1Spin);
	ReleaseISpinner(lev2Spin);
	ReleaseISpinner(gl1Spin);
	ReleaseISpinner(an1Spin);
	ReleaseISpinner(gl2Spin);
	ReleaseISpinner(an2Spin);
	ReleaseISpinner(or1Spin);
	ReleaseISpinner(or2Spin);
	ReleaseISpinner(trSpin);
	ReleaseISpinner(siSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite1, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite2, GWLP_USERDATA, NULL);
	hwHilite1 = hwHilite2 = hRollup = NULL;
}


void  MultiLayerShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		dlevSpin->SetValue(FracToPc(pShader->GetDiffuseLevel(0,0)),FALSE);
		dlevSpin->SetKeyBrackets(KeyAtCurTime(ml_diffuse_level));
		droughSpin->SetValue(FracToPc(pShader->GetDiffuseRough(0,0)),FALSE);
		droughSpin->SetKeyBrackets(KeyAtCurTime(ml_diffuse_rough));
		lev1Spin->SetValue(FracToPc(pShader->GetSpecLevel1(0,0)),FALSE);
		lev1Spin->SetKeyBrackets(KeyAtCurTime(ml_specular_level1));
		lev2Spin->SetValue(FracToPc(pShader->GetSpecLevel2(0,0)),FALSE);
		lev2Spin->SetKeyBrackets(KeyAtCurTime(ml_specular_level2));

		gl1Spin->SetValue( FracToPc(pShader->GetGlossiness1(0,0)), FALSE);
		gl1Spin->SetKeyBrackets(KeyAtCurTime(ml_glossiness1));
		gl2Spin->SetValue( FracToPc(pShader->GetGlossiness2(0,0)), FALSE);
		gl2Spin->SetKeyBrackets(KeyAtCurTime(ml_glossiness2));

		an1Spin->SetValue( FracToPc(pShader->GetAnisotropy1(0,0)), FALSE);
		an1Spin->SetKeyBrackets(KeyAtCurTime(ml_anisotropy1));
		an2Spin->SetValue( FracToPc(pShader->GetAnisotropy2(0,0)), FALSE);
		an2Spin->SetKeyBrackets(KeyAtCurTime(ml_anisotropy2));

		or1Spin->SetValue( FracToPc(pShader->GetOrientation1(0,0)), FALSE);
		or1Spin->SetKeyBrackets(KeyAtCurTime(ml_orientation1));
		or2Spin->SetValue( FracToPc(pShader->GetOrientation2(0,0)), FALSE);
		or2Spin->SetKeyBrackets(KeyAtCurTime(ml_orientation2));

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

			siSpin->SetValue(FracToPc(pShader->GetSelfIllum(0,0)), FALSE);
			siSpin->SetKeyBrackets(KeyAtCurTime(ml_self_illum_amnt));
		}
		CheckButton(hRollup, IDC_LOCK_AD, pShader->GetLockAD() );

		UpdateLockADTex( 0 );
		UpdateColSwatches();
		UpdateHilite2( hwHilite1, pShader, 1 );
		UpdateHilite2( hwHilite2, pShader, 2 );
	}
}



static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void MultiLayerShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < MULTI_LAYER_NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm	 = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE, nm);
	}
}


void MultiLayerShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}


void MultiLayerShaderDlg::UpdateColSwatches() 
{
	cs[0]->SetColor( pShader->GetAmbientClr( curTime ) );
	cs[0]->SetKeyBrackets( pShader->KeyAtTime(ml_ambient, curTime) );

	cs[1]->SetColor( pShader->GetDiffuseClr( curTime ) );
	cs[1]->SetKeyBrackets( pShader->KeyAtTime(ml_diffuse, curTime) );

	cs[2]->SetColor( pShader->GetSelfIllumClr( curTime ) );
	cs[2]->SetKeyBrackets( pShader->KeyAtTime(ml_self_illum_color, curTime) );

	cs[3]->SetColor( pShader->GetSpecColor1( curTime ) );
	cs[3]->SetKeyBrackets( pShader->KeyAtTime(ml_specular1, curTime) );

	cs[4]->SetColor( pShader->GetSpecColor2( curTime ) );
	cs[4]->SetKeyBrackets( pShader->KeyAtTime(ml_specular2, curTime) );
}

void MultiLayerShaderDlg::SetLockAD(BOOL lock)
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


void MultiLayerShaderDlg::UpdateLockADTex( BOOL passOn)
{
	int lock = 	pShader->GetLockADTex();
	CheckButton(hRollup, IDC_LOCK_ADTEX, lock);

	ShowWindow(GetDlgItem(hRollup, IDC_MAPON_AM), !lock);
	texMBut[ 0 ]->Enable(!lock);

	if ( passOn ) {
		pMtl->SyncADTexLock( lock );
	}
//	UpdateMtlDisplay();
}

void MultiLayerShaderDlg::SetLockADTex(BOOL lock)
{
	pShader->SetLockADTex( lock );
	UpdateLockADTex(TRUE); // passon to mtl
//	UpdateMtlDisplay();
}


static int ColorIDCToIndex(int idc) {
	switch (idc) {
		case IDC_AMB_CLR:   return 0;
		case IDC_DIFF_CLR:  return 1;
		case IDC_SI_CLR:	  return 2;
		case IDC_SPEC_CLR1: return 3;
		case IDC_SPEC_CLR2: return 4;
		default: return 0;
	}
}


BOOL MultiLayerShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG:
			{
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

   			cs[0] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_AMB_CLR), 
						pShader->GetAmbientClr(0,0), GetString(IDS_DS_AMBIENT) );

   			cs[1] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_DIFF_CLR), 
						pShader->GetDiffuseClr(0,0), GetString(IDS_DS_DIFFUSE) );

   			cs[2] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SI_CLR), 
						pShader->GetSelfIllumClr(0,0), GetString(IDS_KE_SELFILLUM_CLR) );

			cs[3] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SPEC_CLR1), 
						pShader->GetSpecColor1(0,0), GetString(IDS_KE_SPEC_CLR1) );

   			cs[4] = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SPEC_CLR2), 
						pShader->GetSpecColor2(0,0), GetString(IDS_KE_SPEC_CLR2) );


			hwHilite1 = GetDlgItem(hwndDlg, IDC_HIGHLIGHT1 );
			SetWindowLongPtr( hwHilite1, GWLP_WNDPROC, (LONG_PTR)Hilite2Layer1WndProc);
			hwHilite2 = GetDlgItem(hwndDlg, IDC_HIGHLIGHT2 );
			SetWindowLongPtr( hwHilite2, GWLP_WNDPROC, (LONG_PTR)Hilite2Layer2WndProc);

			dlevSpin = SetupIntSpinner(hwndDlg, IDC_DIFFLEV_SPIN, IDC_DIFFLEV_EDIT, 0,400, 0);
			droughSpin = SetupIntSpinner(hwndDlg, IDC_DIFFROUGH_SPIN, IDC_DIFFROUGH_EDIT, 0,100, 0);
			lev1Spin = SetupIntSpinner(hwndDlg, IDC_SPEC_LEV1_SPIN, IDC_SPEC_LEV1_EDIT, 0,999, 0);
			gl1Spin = SetupIntSpinner(hwndDlg, IDC_GL1_SPIN, IDC_GL1_EDIT, 0,100, 0);
			an1Spin = SetupIntSpinner(hwndDlg, IDC_AN1_SPIN, IDC_AN1_EDIT, 0,100, 0);

			lev2Spin = SetupIntSpinner(hwndDlg, IDC_SPEC_LEV2_SPIN, IDC_SPEC_LEV2_EDIT, 0, 999, 0);
			gl2Spin = SetupIntSpinner(hwndDlg, IDC_GL2_SPIN, IDC_GL2_EDIT, 0,100, 0);
			an2Spin = SetupIntSpinner(hwndDlg, IDC_AN2_SPIN, IDC_AN2_EDIT, 0,100, 0);
			
			or1Spin = SetupIntSpinner(hwndDlg, IDC_OR1_SPIN, IDC_OR1_EDIT, -9999, 9999, 0);
			or2Spin = SetupIntSpinner(hwndDlg, IDC_OR2_SPIN, IDC_OR2_EDIT, -9999, 9999, 0);
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
	
			for (int j=0; j < MULTI_LAYER_NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
			}

			SetupLockButton(hwndDlg,IDC_LOCK_AD,FALSE);
			SetupPadLockButton(hwndDlg,IDC_LOCK_ADTEX, TRUE);

			LoadDialog(TRUE);
		}
		break;

		case WM_COMMAND: 
			{
			for ( int i=0; i < MULTI_LAYER_NMBUTS; i++) {
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
			switch ( n ) {
			  case 0:	pShader->SetAmbientClr( curColor, curTime ); 
						if ( pShader->GetLockAD() )
							pShader->SetDiffuseClr(curColor, curTime);
				  break;
			  case 1:	pShader->SetDiffuseClr( curColor, curTime ); 
						if ( pShader->GetLockAD() )
							pShader->SetAmbientClr( curColor, curTime ); 
				  break;
			  case 2:	pShader->SetSelfIllumClr( curColor, curTime ); break;
			  case 3:	pShader->SetSpecColor1( curColor, curTime ); break;
			  case 4:	pShader->SetSpecColor2( curColor, curTime ); break;
			}
			if (buttonUp) {
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
				NotifyChanged();
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
				case IDC_DIFFLEV_SPIN: 
					pShader->SetDiffuseLevel( PcToFrac( dlevSpin->GetIVal() ), curTime ); 
					break;
				case IDC_DIFFROUGH_SPIN: 
					pShader->SetDiffuseRough( PcToFrac( droughSpin->GetIVal() ), curTime ); 
					break;
				case IDC_SPEC_LEV1_SPIN: 
					pShader->SetSpecLevel1( PcToFrac( lev1Spin->GetIVal() ), curTime ); 
					UpdateHilite2( hwHilite1, pShader, 1 );
					break;
				case IDC_GL1_SPIN: 
					pShader->SetGlossiness1( PcToFrac( gl1Spin->GetIVal() ), curTime ); 
					UpdateHilite2( hwHilite1, pShader, 1 );
					break;
				case IDC_AN1_SPIN: 
					pShader->SetAnisotropy1( PcToFrac( an1Spin->GetIVal() ), curTime ); 
					UpdateHilite2( hwHilite1, pShader, 1 );
					break;

				case IDC_GL2_SPIN: 
					pShader->SetGlossiness2( PcToFrac( gl2Spin->GetIVal() ), curTime ); 
					UpdateHilite2( hwHilite2, pShader, 2 );
					break;
				case IDC_AN2_SPIN: 
					pShader->SetAnisotropy2( PcToFrac( an2Spin->GetIVal() ), curTime ); 
					UpdateHilite2( hwHilite2, pShader, 2 );
					break;
				case IDC_SPEC_LEV2_SPIN: 
					pShader->SetSpecLevel2( PcToFrac( lev2Spin->GetIVal() ), curTime ); 
					UpdateHilite2( hwHilite2, pShader, 2 );
					break;

				case IDC_OR1_SPIN: 
					pShader->SetOrientation1( PcToFrac(or1Spin->GetIVal()), curTime ); 
					break;
				case IDC_OR2_SPIN: 
					pShader->SetOrientation2( PcToFrac(or2Spin->GetIVal()), curTime ); 
					break;
				case IDC_SI_SPIN: 
					pShader->SetSelfIllum(PcToFrac(siSpin->GetIVal()),curTime); 
					break;
					//***** >>>><<<< required handling for opacity....must be present in all dialogs
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


