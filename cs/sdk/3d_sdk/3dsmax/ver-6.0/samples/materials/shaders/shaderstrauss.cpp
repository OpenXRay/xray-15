//////////////////////////////////////////////////////////////////////////////
//
//		Strauss Shader plug-in, implementation
//
//		Created: 10/26/98 Kells Elmquist
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "macrorec.h"
#include "toneop.h"


// Class Ids
#define STRAUSS_SHADER_CLASS_ID		0x2857f450

static Class_ID StraussShaderClassID( STRAUSS_SHADER_CLASS_ID, 0);

// paramblock2 block and parameter IDs.
enum { strauss_params, };
// shdr_params param IDs
enum 
{ 
	st_diffuse, st_glossiness, st_metalness, 
};

/////////////////////////////////////////////////////////////////////
//
//	Basic Panel UI 
//
#define NMBUTS 4

// tex channel number to button IDC
static int texMButtonsIDC[] = {
	IDC_MAPON_CLR, IDC_MAPON_GL, IDC_MAPON_MT, IDC_MAPON_TR, 
};
		
// This array gives the texture map number for given MButton number								
static int texmapFromMBut[] = { 0, 1, 2, 3 };

// Texture Maps
#define STRAUSS_NTEXMAPS	5

// channels ids used by shader
#define S_DI	0
#define S_GL	1
#define S_MT	2
#define S_TR	3

// channel names
static int texNameIDS[STD2_NMAX_TEXMAPS] = {
	IDS_KE_COLOR,	IDS_KE_GLOSSINESS, IDS_KE_METALNESS, IDS_DS_TRANS, 
	IDS_KE_REFR_FILTER, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
};	

// internal non-local parsable channel map names
static TCHAR* texInternalNames[STD2_NMAX_TEXMAPS] = {
	_T("diffuseMap"),_T("glossinessMap"), _T("metalnessMap"), _T("opacityMap"), 	
	_T("filterMap"), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""),
	_T(""), _T(""), _T(""), _T(""), 
	_T(""), _T(""), _T(""), _T(""),
};	

// sized for nmax textures
// bump, reflection & refraction maps are ignored....done after shading
static int chanType[STD2_NMAX_TEXMAPS] = {
	CLR_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, 
	CLR_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, 
};	

// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { -1, 0, -1, 1, -1, -1, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1 };


//////////////////////////////////////////////////////////////////////////////////////////
//
//		Strauss Parameter Block
//
#define CURRENT_STRAUSS_SHADER_VERSION	2
#define STRAUSS_SHADER_NPARAMS			4
#define STRAUSS_SHADER_PB_VERSION		1

#define STRAUSS_NCOLBOX 1
static int colID[STRAUSS_NCOLBOX] = { IDC_STD_COLOR2  };

//Current Param Block Descriptor
static ParamBlockDescID StraussShaderPB[ STRAUSS_SHADER_NPARAMS ] = {
	{ TYPE_RGBA,  NULL, TRUE,st_diffuse },   // diffuse
	{ TYPE_FLOAT, NULL, TRUE,-1 },  // ambient level
	{ TYPE_FLOAT, NULL, TRUE,st_glossiness },   // glossiness 
	{ TYPE_FLOAT, NULL, TRUE,st_metalness },  // metalness
}; 

#define STRAUSS_NUMOLDVER 1

static ParamVersionDesc oldVersions[STRAUSS_NUMOLDVER] = {
	ParamVersionDesc(StraussShaderPB, STRAUSS_SHADER_NPARAMS, 0),
};


//----------------------------------------------------------------------------------------
// Straus Shader, from CGA Nov 1990, "Realistic lighting model for computer animators"
//----------------------------------------------------------------------------------------
// note: We replace std opacity & std reflection, these are automatic in the strauss model
#define STRAUSS_PARAMS (STD_PARAM_DIFFUSE_CLR+STD_PARAM_GLOSSINESS\
						+STD_PARAM_SPECULAR_LEV+STD_EXTRA_DLG+STD_EXTRA_REFRACTION)

class StraussShaderDlg;

class StraussShader : public ExposureMaterialControlImp<StraussShader,
								AddExposureMaterialControl<Shader> > {
friend class StraussShaderCB;
friend class StraussShaderDlg;
friend class ExposureMaterialControlImp<StraussShader,
	AddExposureMaterialControl<Shader> >;
protected:
	IParamBlock2		*pblock;   // ref 0
	Interval		ivalid;

	StraussShaderDlg*	paramDlg;

	Color			diffuse;
	float			glossiness;
	float			metalness;

	static CombineComponentsFPDesc msExpMtlControlDesc;

	public:
	StraussShader();
	void DeleteThis(){ delete this; }		
    ULONG SupportStdParams(){ return STRAUSS_PARAMS; }

	// copy std params, for switching shaders
    void CopyStdParams( Shader* pFrom );

	// texture maps
	long nTexChannelsSupported(){ return STRAUSS_NTEXMAPS; }
	TSTR GetTexChannelName( long nChan ){ return GetString( texNameIDS[ nChan ] ); }
	TSTR GetTexChannelInternalName( long nChan ) { return texInternalNames[ nChan ]; }
	long ChannelType( long nChan ) { return chanType[nChan]; }
	long StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

	BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }
	ULONG GetRequirements( int subMtlNum ){ return isNoExposure() | MTLREQ_PHONG; }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int ){ paramDlg = (StraussShaderDlg*)newDlg; }

	Class_ID ClassID() { return StraussShaderClassID; }
	SClass_ID SuperClassID() { return SHADER_CLASS_ID; }
	TSTR GetName() { return GetString( IDS_KE_STRAUSS ); }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i){ return (i==0)? pblock : NULL; }
	TSTR SubAnimName(int i){ return TSTR(GetString( IDS_KE_STRAUSS_PARMS )); };
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

	void GetIllumParams( ShadeContext &sc, IllumParams& ip );

	// Strauss Shader specific section
	void  Illum(ShadeContext &sc, IllumParams &ip);
	float EvalHiliteCurve(float x);

	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol);
	void CombineComponents( ShadeContext &sc, IllumParams& ip );

	void SetGlossiness(float v, TimeValue t){ glossiness= v; pblock->SetValue( st_glossiness, t, v); }
	float GetGlossiness(int mtlNum=0, BOOL backFace=FALSE){ return glossiness; };	
	float GetGlossiness( TimeValue t){return pblock->GetFloat(st_glossiness,t);  }		

	void SetMetalness(float v, TimeValue t){ metalness = v; pblock->SetValue( st_metalness, t, v); }
	float GetMetalness(int mtlNum=0, BOOL backFace=FALSE){ return metalness; };	
	float GetMetalness( TimeValue t){return pblock->GetFloat(st_metalness,t);  }		

	// Std Params
	void SetDiffuseClr(Color c, TimeValue t)		
		{ diffuse = c; pblock->SetValue( st_diffuse, t, c); }

    Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE){ return diffuse;}		
	Color GetDiffuseClr(TimeValue t){ return pblock->GetColor(st_diffuse,t); }		

	// std params not supported
	void SetLockDS(BOOL lock){ }
	BOOL GetLockDS(){ return FALSE; }
	void SetLockAD(BOOL lock){ }
	BOOL GetLockAD(){ return FALSE; }
	void SetLockADTex(BOOL lock){ }
	BOOL GetLockADTex(){ return FALSE; }
	void SetAmbientClr(Color c, TimeValue t){}
	Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE){ return diffuse * 0.5f;}		
	Color GetAmbientClr(TimeValue t){ return diffuse * 0.5f; }		
		
	void SetSpecularClr(Color c, TimeValue t){}
	void SetSpecularLevel(float v, TimeValue t){}		
	Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE){ return Color(0.9f, 0.9f,0.9f); };
	float GetSpecularLevel(TimeValue t);
	float GetSpecularLevel(int mtlNum, BOOL backFace){ return GetSpecularLevel(0); };
	Color GetSpecularClr(TimeValue t){ return Color(0.9f,0.9f,0.9f);}
	void SetSelfIllum(float v, TimeValue t)	{}
	float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE){ return 0.0f; };
	void SetSelfIllumClrOn( BOOL on ){};
	BOOL IsSelfIllumClrOn(){ return TRUE; };
	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return TRUE; }
	void SetSelfIllumClr(Color c, TimeValue t){}
	Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE){ return Color(0,0,0); }
	void SetSoftenLevel(float v, TimeValue t) {}
	float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE){ return DEFAULT_SOFTEN; }
	float GetSoftenLevel(TimeValue t){ return  DEFAULT_SOFTEN; }
	float GetSelfIllum(TimeValue t){ return 0.0f;}		
	Color GetSelfIllumClr(TimeValue t){ return Color(0,0,0);}		

};

///////////// Class Descriptor ////////////////////////
class StraussShaderClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new StraussShader(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_STRAUSS); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return StraussShaderClassID; }
	const TCHAR* 	Category() { return _T("");  }
	const TCHAR*	InternalName() { return _T("Strauss"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			 // returns owning module handle
};

StraussShaderClassDesc StraussCD;
ClassDesc * GetStraussShaderCD(){ return &StraussCD; }

// shader parameters
static ParamBlockDesc2 strauss_param_blk ( strauss_params, _T("shaderParameters"),  0, &StraussCD, P_AUTO_CONSTRUCT, 0, 
	// params
	st_diffuse, _T("diffuse"), TYPE_RGBA, P_ANIMATABLE, IDS_DS_DIFFUSE, 
		p_default, Color(0.5f, 0.5f, 0.5f), 
		end,
	st_glossiness, _T("glossiness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_GLOSSINESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	st_metalness, _T("metalness"), TYPE_PCNT_FRAC, P_ANIMATABLE, IDS_KE_METALNESS,
		p_default,		0.0,
		p_range,		0.0, 100.0,
		end,
	end
);


CombineComponentsFPDesc StraussShader::msExpMtlControlDesc(StraussCD);

StraussShader::StraussShader() 
{ 
	pblock = NULL; 
	StraussCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	paramDlg = NULL; 
	diffuse = Color(0.0f,0.0f,0.0f);
	glossiness = metalness = 0.0f;
	ivalid.SetEmpty(); 
}

void StraussShader::CopyStdParams( Shader* pFrom )
{
	macroRecorder->Disable(); 
	// don't want to see this parameter copying in macrorecorder

		SetAmbientClr( pFrom->GetAmbientClr(0,0), 0 );
		SetDiffuseClr( pFrom->GetDiffuseClr(0,0), 0 );
		SetGlossiness( pFrom->GetGlossiness(0,0), 0 );

	macroRecorder->Enable();
	ivalid.SetEmpty();	
}


RefTargetHandle StraussShader::Clone( RemapDir &remap )
{
	StraussShader* mnew = new StraussShader();
	mnew->ExposureMaterialControl::operator=(*this);
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->diffuse = diffuse;
	mnew->glossiness = glossiness;
	mnew->metalness = metalness;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

void StraussShader::GetIllumParams( ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
	ip.channels[S_DI] = diffuse;
	ip.channels[S_GL].r = glossiness;
	ip.channels[S_MT].r = metalness;
}



void StraussShader::Update(TimeValue t, Interval &valid) {
	Point3 p;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

		pblock->GetValue( st_diffuse, t, p, ivalid );
		diffuse= Bound(Color(p.x,p.y,p.z));
		pblock->GetValue( st_glossiness, t, glossiness, ivalid );
		glossiness = Bound(glossiness );
		pblock->GetValue( st_metalness, t, metalness, ivalid );
		metalness = Bound(metalness );
	}
	valid &= ivalid;
}

void StraussShader::Reset()
{
//	StraussCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2

	ivalid.SetEmpty();
	SetDiffuseClr( Color(0.5f,0.5f,0.5f), 0 );
	SetGlossiness( 0.2f,0);   
	SetMetalness( 0.0f,0);   
}


//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
#define SHADER_HDR_CHUNK 0x4000
#define SHADER_MAPSON_CHUNK 0x5004
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_DIMDIFFON_CHUNK 0x5400
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020

// IO
IOResult StraussShader::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = CURRENT_STRAUSS_SHADER_VERSION;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();
	return IO_OK;
}		

class StraussShaderCB: public PostLoadCallback {
	public:
		StraussShader *s;
		int loadVersion;
	    StraussShaderCB(StraussShader *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) 
		{
			s->ReplaceReference(0,
				UpdateParameterBlock2(StraussShaderPB, STRAUSS_SHADER_NPARAMS, (IParamBlock*)s->pblock, &strauss_param_blk));
		}
};


IOResult StraussShader::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
			case SHADER_VERS_CHUNK:
				res = iload->Read(&version,sizeof(version), &nb);
				break;
			case SHADER_DIMDIFFON_CHUNK:
				BOOL dim;
				res = iload->Read(&dim,sizeof(dim), &nb);
				break;
			case SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK:
				res = ExposureMaterialControl::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < CURRENT_STRAUSS_SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new StraussShaderCB(this, version));
		iload->SetObsolete();
	}
	return IO_OK;
}

			
///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//

// my magic constants
static float SpecBoost = 1.3f;

// Strauss's Magic Constants
static float kf = 1.12f;
static float kf2 = 1.0f / (kf * kf);
static float kf3 = 1.0f / ((1.0f - kf) * (1.0f - kf));
static float kg = 1.01f;
static float kg2 = 1.0f / (kg * kg);
static float kg3 = 1.0f / ((1.0f - kg) * (1.0f - kg));
static float kj = 0.1f;	//.1 strauss

static float OneOverHalfPi = 1.0f / (0.5f * Pi);

inline float F( float x ){ 
	float xb = Bound( x );
	float xkf = 1.0f / ((xb - kf)*(xb - kf));
	return (xkf - kf2) / (kf3 - kf2);
}

inline float G( float x ){ 
	float xb = Bound( x );
	float xkg = 1.0f / ((xb - kg)*(xb - kg));
	return (kg3 - xkg) / (kg3 - kg2);
}

#define REFL_BRIGHTNESS_ADJUST	3.0f;

void StraussShader::AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rClr ) 
{
	float opac = ip.channels[ S_TR ].r;
	float g = ip.channels[ S_GL ].r;
	float m = ip.channels[ S_MT ].r;
	Color Cd = ip.channels[ S_DI ];

	float rn = opac - (1.0f - g * g * g) * opac;

	// the reflection of the reflection vector is just the view vector
	// so dot(v, r) is 1, to any power is still 1
	float a, b;
//	NB: this has been transformed for existing in-pointing v
	float NV = Dot( sc.V(), sc.Normal() );
	Point3 R = sc.V() - 2.0f * NV * sc.Normal();
	float NR = Dot( sc.Normal(), R );
		a = (float)acos( NR ) * OneOverHalfPi;
		b = (float)acos( NV ) * OneOverHalfPi;
				
	float fa = F( a );
	float j = fa * G( a ) * G( b );
	float rj = Bound( rn + (rn+kj)*j );
	Color white( 1.0f, 1.0f, 1.0f );

	Color Cs = white + m * (1.0f - fa) * (Cd - white);
	rClr *= Cs * rj * REFL_BRIGHTNESS_ADJUST;
}

static int stopX = -1;
static int stopY = -1;

static float	greyVal = 0.3f;
static float	clrVal = 0.3f;

static float	softThresh = 0.15f;

void StraussShader::Illum(ShadeContext &sc, IllumParams &ip) 
{
	LightDesc *l;
	Color lightClr;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if ( sp.x == stopX && sp.y == stopY )
		sp.x = stopX;
#endif

	float opac = ip.channels[ S_TR ].r;
	float g = ip.channels[ S_GL ].r;
	float m = ip.channels[ S_MT ].r;
	Color Cd = ip.channels[ S_DI ];
//	BOOL dimDiffuse = ip.hasComponents & HAS_REFLECT;
 	BOOL dimDiffuse = ip.hasComponents & HAS_REFLECT_MAP;

	float rd;
 	float g3 = Cube( g );
	if ( dimDiffuse )
		rd = (1.0f - g3) * opac;
	else
		rd = (1.0f - m * g3) * opac;	//ke 10/28/98

	float rn = opac - (1.0f - g3) * opac;

	float h = (g == 1.0f ) ? 600.0f : 3.0f / (1.0f - g );
	float d = 1.0f - m * g;

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		float NL, Kl;
		Point3 L;
		if (l->Illuminate( sc, sc.Normal(), lightClr, L, NL, Kl)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightClr;
				continue;
			}
			if (NL<=0.0f) 
				continue;

			// diffuse
			if (l->affectDiffuse){
				ip.diffIllumOut += Kl * d * rd * lightClr;
			}

			// specular  
			if (l->affectSpecular) {
				// strauss uses the reflected LIGHT vector
				Point3 R = L - 2.0f * NL * sc.Normal();
				R = Normalize( R );

				float RV = -Dot(R, sc.V() );
				
				float s;
				if (RV < 0.0f) {
					// soften
					if ( NL < softThresh )
						RV *= SoftSpline2( NL / softThresh );
					// specular function
					s = SpecBoost * (float)pow( -RV, h);
				} else
					continue;

				float a, b;
				a = (float)acos( NL ) * OneOverHalfPi;
				b = (float)acos( -Dot(sc.Normal(), sc.V()) ) * OneOverHalfPi;
				
				float fa = F( a );
				float j = fa * G( a ) * G( b );
				float rj = rn > 0.0f ? Bound( rn + (rn+kj)*j ) : rn;
				Color Cl = lightClr;
				// normalize the light color in case it's really bright
				float I = NormClr( Cl );
				Color Cs = Cl + m * (1.0f - fa) * (Cd - Cl);

				ip.specIllumOut += s * rj * I * Cs;

			} // end, if specular
 		}	// end, illuminate

	} // for each light

	// now we can multiply by the clrs, except specular, which is already done
	ip.ambIllumOut *= 0.5f * rd * Cd; 
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= Cd; 

	// next due reflection
	if ( ip.hasComponents & HAS_REFLECT ){
		Color rc = ip.channels[ ip.stdIDToChannel[ ID_RL ] ];
		AffectReflection(sc, ip, rc);
		ip.reflIllumOut = rc * ip.reflectAmt;
	}

	// last do refraction/ opacity
	if ( (ip.hasComponents & HAS_REFRACT) ){
		// Set up attenuation opacity for Refraction map. dim diffuse & spec by this
		ip.finalAttenuation = ip.finalOpac * (1.0f - ip.refractAmt);   

		// Make more opaque where specular hilite occurs:
		float max = Max(ip.specIllumOut);
		if (max > 1.0f) max = 1.0f; 
	   	float newOpac = ip.finalAttenuation + max - ip.finalAttenuation * max;

		// Evaluate refraction map, filtered by filter color.
//		Color tClr = ((StdMat2*)(ip.pMtl))->TranspColor( newOpac, ip.channels[filtChan], ip.channels[diffChan]);
		Color tClr = transpColor( TRANSP_FILTER, newOpac, Cd, Cd );
		ip.transIllumOut = ip.channels[ ip.stdIDToChannel[ ID_RR ] ] * tClr;

		// no transparency when doing refraction
		ip.finalT.Black();

	} else {
		// no refraction, transparent?
		ip.finalAttenuation = opac;
		if (ip.hasComponents & HAS_OPACITY)	{
			//	ip.finalT = Cd * (1.0f-opac);
			Cd = greyVal * Color( 1.0f, 1.0f, 1.0f ) + clrVal * Cd;
			ip.finalT = transpColor( TRANSP_FILTER, opac, Cd, Cd );
		}
	}

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

float StraussShader::GetSpecularLevel(TimeValue t)
{
	float g3 = Cube( glossiness );

//	float j = F( 0.0f ) * G( 0.0f ) * G( 0.0f );
//	float j = G( 0.0f ) * G( 0.0f );
//	float rj = Bound( g3 + (g3+kj)*j );
//	return rj;
	return g3;
}

void StraussShader::CombineComponents( ShadeContext &sc, IllumParams& ip )
{
	float o = (ip.hasComponents & HAS_REFRACT) ? ip.finalAttenuation : 1.0f;

	ip.finalC = o * (ip.ambIllumOut + ip.diffIllumOut) + ip.specIllumOut 
				+ ip.reflIllumOut + ip.transIllumOut; 
}

///////////////////////////////////////////////////////////////////////////////////
//
//	Strauss shader dlg panel
//
class StraussShaderDlg : public ShaderParamDlg {
public:
	StraussShader*	pShader;
	StdMat2*	pMtl;
	HPALETTE	hOldPal;
	HWND		hwmEdit;	// window handle of the materials editor dialog
	IMtlParams*	pMtlPar;
	HWND		hwHilite;   // the hilite window
	HWND		hRollup;	// Rollup panel
	TimeValue	curTime;
	BOOL		valid;
	BOOL		isActive;

	IColorSwatch *cs[STRAUSS_NCOLBOX];
	ISpinnerControl *glSpin, *mtSpin, *trSpin;
	ICustButton* texMBut[NMBUTS];
	TexDADMgr dadMgr;
	
	StraussShaderDlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~StraussShaderDlg(); 

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
	Class_ID ClassID(){ return StraussShaderClassID; }

	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (StraussShader*)theShader; 
		if (pShader)pShader->SetParamDlg(this,0); 
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
	void UpdateMapButtons();
	void UpdateOpacity();

	void SelectEditColor(int i) { cs[ i ]->EditThis(FALSE); }
};

static INT_PTR CALLBACK  StraussShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	StraussShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (StraussShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (StraussShaderDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* StraussShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	StraussShaderDlg *pDlg = new StraussShaderDlg(hwMtlEdit, imp);
	pDlg->SetThings( theMtl, this  );

	LoadStdShaderResources();
	if ( hOldRollup ) {
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_STRAUSS3),
			StraussShaderDlgProc, 
			GetString(IDS_KE_STRAUSS_BASIC),	// your name here
			(LPARAM)pDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);
	} else
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_STRAUSS3),
			StraussShaderDlgProc, 
			GetString(IDS_KE_STRAUSS_BASIC),	
			(LPARAM)pDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)pDlg;	
}

RefResult StraussShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
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


float StraussShader::EvalHiliteCurve( float x )
{
	float op = ( paramDlg )? paramDlg->pMtl->GetOpacity(0) : 1.0f;
	float rn = op - (1.0f - glossiness * glossiness * glossiness) * op;

	float h = (glossiness == 1.0f)? 600.0f : 3.0f / (1.0f - glossiness );
	float s = (float)pow( cos(x*PI), h);

	float a = 0.5f;
	float b = x;

	float fa = F( a );
	float j = fa * G( a ) * G( b );
	float rj = Min( 1.0f, rn + (rn+kj)*j );
	float IllumOut = SpecBoost * s * rj;

	return IllumOut;
}


StraussShaderDlg::StraussShaderDlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	glSpin = mtSpin = trSpin = NULL;
	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();
	isActive = valid = FALSE;

	for( long i = 0; i < STRAUSS_NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

StraussShaderDlg::~StraussShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}

	for (i=0; i<STRAUSS_NCOLBOX; i++)
		if (cs[i]) ReleaseIColorSwatch(cs[i]); // mjm - 5.10.99
	
	ReleaseISpinner(glSpin);
	ReleaseISpinner(mtSpin);
	ReleaseISpinner(trSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite, GWLP_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}


void  StraussShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		glSpin->SetValue( FracToPc( pShader->GetGlossiness() ),FALSE);
		glSpin->SetKeyBrackets(KeyAtCurTime(st_glossiness));
		
		mtSpin->SetValue( FracToPc( pShader->GetMetalness() ), FALSE);
		mtSpin->SetKeyBrackets(KeyAtCurTime(st_metalness));

		trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		UpdateColSwatches();
		UpdateHilite();
	}
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void StraussShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}


void StraussShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
	UpdateHilite();
}

void StraussShaderDlg::UpdateColSwatches() 
{
	cs[0]->SetKeyBrackets( pShader->KeyAtTime(st_diffuse,curTime) );
	cs[0]->SetColor( pShader->GetDiffuseClr() );
}


void StraussShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}



static int ColorIDCToIndex(int idc) {
	switch (idc) {
		case IDC_COLOR: return 0;
		default: return 0;
	}
}


BOOL StraussShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
    switch (msg) {
		case WM_INITDIALOG:
			{
			HDC theHDC = GetDC(hwndDlg);
			hOldPal = GetGPort()->PlugPalette(theHDC);
			ReleaseDC(hwndDlg,theHDC);

			HWND hwndCS = GetDlgItem(hwndDlg, IDC_COLOR);
			cs[0] = GetIColorSwatch( hwndCS, pShader->GetDiffuseClr(), GetString(IDS_KE_COLOR) );

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)HiliteWndProc);

			glSpin = SetupIntSpinner(hwndDlg, IDC_GL_SPIN, IDC_GL_EDIT, 0,100, 0);
			mtSpin = SetupIntSpinner(hwndDlg, IDC_MT_SPIN, IDC_MT_EDIT, 0,100, 0);
			trSpin = SetupIntSpinner(hwndDlg, IDC_TR_SPIN, IDC_TR_EDIT, 0,100, 0);

			for (int j=0; j<NMBUTS; j++) {
				texMBut[j] = GetICustButton(GetDlgItem(hwndDlg,texMButtonsIDC[j]));
				assert( texMBut[j] );
				texMBut[j]->SetRightClickNotify(TRUE);
				texMBut[j]->SetDADMgr(&dadMgr);
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

			break; // WM_COMMAND

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
			pShader->SetDiffuseClr(curColor, curTime); 
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
				case IDC_GL_SPIN: 
					pShader->SetGlossiness(PcToFrac( glSpin->GetIVal() ), curTime); 
					UpdateHilite();
					break;
				case IDC_MT_SPIN: 
					pShader->SetMetalness(PcToFrac(mtSpin->GetIVal()), curTime); 
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


