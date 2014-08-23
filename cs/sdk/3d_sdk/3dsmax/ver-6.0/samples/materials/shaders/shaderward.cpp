//////////////////////////////////////////////////////////////////////////////
//
//		Ward Anisotropic Shader plug-in, implementation
//
//		Created: 10/13/98 Kells Elmquist
//
#include "shadersPch.h"
#include "shadersRc.h"
#include "gport.h"
#include "shaders.h"
#include "shaderUtil.h"
#include "toneop.h"

// these control the range of vals presented to the user
#define ALPHA_MIN	0.015f
#define ALPHA_MAX	0.5f
#define SPEC_MAX	0.5f

#define ALPHA_SZ	(ALPHA_MAX - ALPHA_MIN)

// Class Ids
#define WARDSHADER_CLASS_ID		0x2857f440

static Class_ID WardShaderClassID( WARDSHADER_CLASS_ID, 0);
static Class_ID WardShaderDlgClassID( WARDSHADER_CLASS_ID, 0);


/////////////////////////////////////////////////////////////////////
//
//	Basic Panel UI 
//
#define NMBUTS 8
#define N_TR_BUT 7

// tex channel number to button IDC
static int texMButtonsIDC[] = {
	IDC_MAPON_AM, IDC_MAPON_DI, IDC_MAPON_SP, 
	IDC_MAPON_DLEV, IDC_MAPON_SLEV, IDC_MAPON_GLX, IDC_MAPON_GLY,
	IDC_MAPON_TR, 
	-1, -1, -1, -1,  -1, -1, -1, -1 
	};
		
// This array gives the texture map number for given MButton number								
static int texmapFromMBut[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

// Texture Maps
#define WARD_NTEXMAPS	11

// channels ids
#define W_AM	0
#define W_DI	1
#define W_SP	2
#define W_DL	3
#define W_SL	4
#define W_GX	5
#define W_GY	6

// channel names
// sized for nmax  textures
static int texNameIDS[] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, 
	IDS_KE_DIFF_LEVEL, IDS_KE_SPEC_LEVEL, IDS_KE_GLOSSINESS_X, IDS_KE_GLOSSINESS_Y,
	IDS_DS_TRANS, IDS_DS_FILTER, IDS_DS_RL, IDS_DS_RR, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
	IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE, IDS_KE_NONE,
};	

// sized for nmax  textures
// bump, reflection & refraction maps are ignored....done after shading
static int chanType[] = {
	CLR_CHANNEL, CLR_CHANNEL, CLR_CHANNEL, MONO_CHANNEL, 
	MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL, MONO_CHANNEL,  
	CLR_CHANNEL, REFL_CHANNEL, REFR_CHANNEL, UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
	UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL, UNSUPPORTED_CHANNEL,
};	


// what channel corresponds to the stdMat ID's
static int stdIDToChannel[N_ID_CHANNELS] = { 0, 1, 2, 5, 4, -1, 7, 8, -1, 10, 11, -1, -1, -1, -1, -1 };

// Param block parameter names
static int paramNameIDS[] = {
	IDS_DS_AMBIENT,	IDS_DS_DIFFUSE,	IDS_DS_SPECULAR, 
	IDS_KE_DIFF_LEVEL, IDS_KE_SPEC_LEVEL, IDS_KE_GLOSSINESS_X, IDS_KE_GLOSSINESS_Y, 
};


//////////////////////////////////////////////////////////////////////////////////////////
//
//		Ward Parameter Block
//
#define CURRENT_WARD_SHADER_VERSION	1
#define WARD_SHADER_NPARAMS			7
#define WARD_SHADER_PB_VERSION		1

#define WARD_NCOLBOX 3
static int colID[WARD_NCOLBOX] = { IDC_STD_COLOR1, IDC_STD_COLOR2, IDC_STD_COLOR3  };
#define N_AMB_CLR		0

#define PB_AMBIENT_CLR		0
#define PB_DIFFUSE_CLR		1
#define PB_SPECULAR_CLR		2
#define PB_DIFFUSE_LEV		3
#define PB_SPECULAR_LEV		4
#define PB_GLOSSINESS_X		5
#define PB_GLOSSINESS_Y		6

//Current Param Block Descriptor
static ParamBlockDescID WardShaderPB[ WARD_SHADER_NPARAMS ] = {
	{ TYPE_RGBA,  NULL, TRUE,1 },   // ambient
	{ TYPE_RGBA,  NULL, TRUE,2 },   // diffuse
	{ TYPE_RGBA,  NULL, TRUE,3 },   // specular
	{ TYPE_FLOAT, NULL, TRUE,21 },  // diffuse level
	{ TYPE_FLOAT, NULL, TRUE,5 },   // specularLevel
	{ TYPE_FLOAT, NULL, TRUE,4 },   // glossiness x
	{ TYPE_FLOAT, NULL, TRUE,22 },  // glossiness y
}; 

#define WARD_NUMOLDVER 1

static ParamVersionDesc oldVersions[WARD_NUMOLDVER] = {
	ParamVersionDesc(WardShaderPB, WARD_SHADER_NPARAMS, 0),
};


//----------------------------------------------------------------------------------------
//---- Ward Elliptical Gaussian Anisotropic Shader ---------------------------------------
//----------------------------------------------------------------------------------------

class WardShaderDlg;

class WardShader : public ExposureMaterialControlImp<WardShader,
								AddExposureMaterialControl<Shader> > {
friend class WardShaderDlg;
friend class ExposureMaterialControlImp<WardShader,
	AddExposureMaterialControl<Shader> >;
protected:
	IParamBlock		*pblock;   // ref 0
	Interval		ivalid;
	TimeValue		curTime;

	WardShaderDlg*	paramDlg;

	BOOL			normalizeOn;
	BOOL			lockDS;
	BOOL			lockAD;
	BOOL			lockADTex;

	Color			ambient;
	Color			diffuse;
	Color			specular;
	float glossinessX;
	float glossinessY;
	float specLevel;
	float diffLevel;

	static CombineComponentsFPDesc msExpMtlControlDesc;

	public:
	WardShader();
	void DeleteThis(){ delete this; }		
    ULONG SupportStdParams(){ return (STD_WARD | STD_EXTRA); }
	// copy std params, for switching shaders
    void CopyStdParams( Shader* pFrom );

	// texture maps
	long  nTexChannelsSupported(){ return WARD_NTEXMAPS; }
	TSTR  GetTexChannelName( long nTex ){ return GetString( texNameIDS[ nTex ] ); }
	long ChannelType( long nChan ) { return chanType[nChan]; }
	long StdIDToChannel( long stdID ){ return stdIDToChannel[stdID]; }

	BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }
	ULONG GetRequirements( int subMtlNum ){ return isNoExposure()+MTLREQ_PHONG+MTLREQ_UV+MTLREQ_BUMPUV; }

	ShaderParamDlg* CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int );
	ShaderParamDlg* GetParamDlg(int){ return (ShaderParamDlg*)paramDlg; }
	void SetParamDlg( ShaderParamDlg* newDlg, int ){ paramDlg = (WardShaderDlg*)newDlg; }

	Class_ID ClassID() { return WardShaderClassID; }
	SClass_ID SuperClassID() { return SHADER_CLASS_ID; }
	TSTR GetName() { return GetString( IDS_KE_WARD ); }
	void GetClassName(TSTR& s) { s = GetName(); }  

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i){ return (i==0)? pblock : NULL; }
	TSTR SubAnimName(int i){ return TSTR(GetString( IDS_KE_WARD_PARMS )); };
	int SubNumToRefNum(int subNum) { return subNum;	}

 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i){ return (i==0)? pblock : NULL; }
	void SetReference(int i, RefTargetHandle rtarg) 
		{ if (i==0) pblock = (IParamBlock*)rtarg; else assert(0); }
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
	void GetIllumParams( ShadeContext& sc, IllumParams& ip );

	// Ward Shader specific section
	void  Illum(ShadeContext &sc, IllumParams &ip);
	void CombineComponents( ShadeContext &sc, IllumParams& ip ){ CombineComponentsAdd( ip ); }
	float EvalHiliteCurve2(float x, float y, int layer=0 );

	void AffectReflection(ShadeContext &sc, IllumParams &ip, Color &rcol) { 
		rcol *= ip.channels[W_SP] * DEFAULT_K_REFL; 
	}

	void SetGlossiness(float v, TimeValue t)		
		{ if( v<=ALPHA_MIN) v = ALPHA_MIN; glossinessX= v; pblock->SetValue( PB_GLOSSINESS_X, t, v); }
	void SetGlossinessY(float v, TimeValue t)		
		{ if( v<=ALPHA_MIN) v = ALPHA_MIN; glossinessY= v; pblock->SetValue( PB_GLOSSINESS_Y, t, v); }
	float GetGlossiness(int mtlNum=0, BOOL backFace=FALSE){ return glossinessX; };	
	float GetGlossinessY(int mtlNum=0, BOOL backFace=FALSE){ return glossinessY; };	
	float GetGlossiness( TimeValue t){return pblock->GetFloat(PB_GLOSSINESS_X,t);  }		
	float GetGlossinessY( TimeValue t){return pblock->GetFloat(PB_GLOSSINESS_Y,t);  }		
	void  SetNormalizeOn( BOOL normOn ) { normalizeOn = normOn; }
	BOOL  GetNormalizeOn(){ return normalizeOn; }

	// Std Params
	BOOL IsFaceted() { return FALSE; }
	void SetLockDS(BOOL lock){ lockDS = lock;  }
	BOOL GetLockDS(){ return lockDS; }
	void SetLockAD(BOOL lock){ lockAD = lock;  }
	BOOL GetLockAD(){ return lockAD; }
	void SetLockADTex(BOOL lock){ lockADTex = lock;  }
	BOOL GetLockADTex(){ return lockADTex; }

	void SetAmbientClr(Color c, TimeValue t)		
		{ ambient = c; pblock->SetValue( PB_AMBIENT_CLR, t, c); }
	void SetDiffuseClr(Color c, TimeValue t)		
		{ diffuse = c; pblock->SetValue( PB_DIFFUSE_CLR, t, c); }
	void SetSpecularClr(Color c, TimeValue t)
		{ specular = c; pblock->SetValue( PB_SPECULAR_CLR, t, c); }
	void SetDiffuseLevel(float v, TimeValue t)		
			{ diffLevel = v; pblock->SetValue( PB_DIFFUSE_LEV, t, v); }
	void SetSpecularLevel(float v, TimeValue t)		
		{ specLevel = v; pblock->SetValue( PB_SPECULAR_LEV, t, v); }

	Color GetAmbientClr(int mtlNum=0, BOOL backFace=FALSE){ return ambient;}		
    Color GetDiffuseClr(int mtlNum=0, BOOL backFace=FALSE){ return diffuse;}		
	Color GetSpecularClr(int mtlNum=0, BOOL backFace=FALSE){ return specular; };
	float GetDiffuseLevel(int mtlNum=0, BOOL backFace=FALSE){ return diffLevel; };
	float GetSpecularLevel(int mtlNum=0, BOOL backFace=FALSE){ return specLevel; };
	Color GetAmbientClr(TimeValue t){ return pblock->GetColor(PB_AMBIENT_CLR,t); }		
	Color GetDiffuseClr(TimeValue t){ return pblock->GetColor(PB_DIFFUSE_CLR,t); }		
	Color GetSpecularClr(TimeValue t){ return pblock->GetColor(PB_SPECULAR_CLR,t);	}
	float GetSpecularLevel(TimeValue t){ return  pblock->GetFloat(PB_SPECULAR_LEV,t); }
	float GetDiffuseLevel(TimeValue t){ return  pblock->GetFloat(PB_DIFFUSE_LEV,t); }

	// not supported
	void SetSelfIllum(float v, TimeValue t)	{}
	float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE){ return 0.0f; };
	void SetSelfIllumClrOn( BOOL on ){};
	BOOL IsSelfIllumClrOn(){ return FALSE; };
	BOOL IsSelfIllumClrOn(int mtlNum, BOOL backFace){ return FALSE; }
	void SetSelfIllumClr(Color c, TimeValue t){}
	Color GetSelfIllumClr(int mtlNum=0, BOOL backFace=FALSE){ return Color(0,0,0); }
	void SetSoftenLevel(float v, TimeValue t) {}
	float GetSoftenLevel(int mtlNum=0, BOOL backFace=FALSE){ return DEFAULT_SOFTEN; }
	float GetSoftenLevel(TimeValue t){ return  DEFAULT_SOFTEN; }
	float GetSelfIllum(TimeValue t){ return 0.0f;}		
	Color GetSelfIllumClr(TimeValue t){ return Color(0,0,0);}		

};

///////////// Class Descriptor ////////////////////////
class WardShaderClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading) { 	return new WardShader(); }
	const TCHAR *	ClassName() { return GetString(IDS_KE_WARD); }
	SClass_ID		SuperClassID() { return SHADER_CLASS_ID; }
	Class_ID 		ClassID() { return WardShaderClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

WardShaderClassDesc wardCD;
ClassDesc * GetWardShaderCD(){ return &wardCD; }

CombineComponentsFPDesc WardShader::msExpMtlControlDesc(wardCD);

WardShader::WardShader() 
{ 
	lockDS =  normalizeOn = 0;
	lockAD = lockADTex = 1;
	ambient = diffuse = specular = Color(0.0f,0.0f,0.0f);
	glossinessX = glossinessY = specLevel = diffLevel = 0.0f;
	pblock = NULL; 
	paramDlg = NULL; 
	ivalid.SetEmpty(); 
}

void WardShader::CopyStdParams( Shader* pFrom )
{
	SetLockDS( pFrom->GetLockDS() );
	SetLockAD( pFrom->GetLockAD() );
	SetLockADTex( pFrom->GetLockADTex() );

	SetAmbientClr( pFrom->GetAmbientClr(0,0), curTime );
	SetDiffuseClr( pFrom->GetDiffuseClr(0,0), curTime );
	SetSpecularClr( pFrom->GetSpecularClr(0,0), curTime );

	SetDiffuseLevel( 1.0f, curTime );
	SetSpecularLevel( pFrom->GetSpecularLevel(0,0), curTime );
	SetGlossiness( pFrom->GetGlossiness(0,0), curTime );
	SetGlossinessY( pFrom->GetGlossiness(0,0), curTime ); // both the same
	ivalid.SetEmpty();	
}


RefTargetHandle WardShader::Clone( RemapDir &remap )
{
	WardShader* mnew = new WardShader();
	mnew->ExposureMaterialControl::operator=(*this);
	mnew->ReplaceReference(0, remap.CloneRef(pblock));
	mnew->ivalid.SetEmpty();	
	mnew->ambient = ambient;
	mnew->diffuse = diffuse;
	mnew->specular = specular;
	mnew->glossinessX = glossinessX;
	mnew->glossinessY = glossinessY;
	mnew->specLevel = specLevel;
	mnew->diffLevel = diffLevel;
	mnew->lockDS = lockDS;
	mnew->lockAD = lockAD;
	mnew->lockADTex = lockADTex;
	mnew->normalizeOn = normalizeOn;
	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

void WardShader::GetIllumParams( ShadeContext &sc, IllumParams& ip )
{
	ip.stdParams = SupportStdParams();
	ip.channels[W_AM] = ambient;
	ip.channels[W_DI] = diffuse;
	ip.channels[W_SP] = specular;
	ip.channels[W_GX].r = glossinessX;
	ip.channels[W_GY].r = glossinessY;
	ip.channels[W_DL].r = diffLevel;
	ip.channels[W_SL].r = specLevel;
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


void WardShader::Update(TimeValue t, Interval &valid) {
	Point3 p, p2;
	if( inUpdate )
		return;
	inUpdate = TRUE;
	if (!ivalid.InInterval(t)) {
		ivalid.SetInfinite();

//		pblock->GetValue( PB_AMBIENT_CLR, t, p, ivalid );
//		ambient = LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( PB_DIFFUSE_CLR, t, p, ivalid );
		diffuse= LimitColor(Color(p.x,p.y,p.z));
		pblock->GetValue( PB_AMBIENT_CLR, t, p2, ivalid );
		if( lockAD && (p!=p2)){
			pblock->SetValue( PB_AMBIENT_CLR, t, diffuse);
			ambient = diffuse;
		} else {
			pblock->GetValue( PB_AMBIENT_CLR, t, p, ivalid );
			ambient = Bound(Color(p.x,p.y,p.z));
		}
		pblock->GetValue( PB_SPECULAR_CLR, t, p2, ivalid );
		if( lockDS && (p!=p2)){
			pblock->SetValue( PB_SPECULAR_CLR, t, diffuse);
			specular = diffuse;
		} else {
			pblock->GetValue( PB_SPECULAR_CLR, t, p, ivalid );
			specular = Bound(Color(p.x,p.y,p.z));
		}

//		pblock->GetValue( PB_SPECULAR_CLR, t, p, ivalid );
//		specular = LimitColor(Color(p.x,p.y,p.z));

		pblock->GetValue( PB_GLOSSINESS_X, t, glossinessX, ivalid );
		LIMITMINMAX(glossinessX, 0.0001f, 1.0f );
		pblock->GetValue( PB_GLOSSINESS_Y, t, glossinessY, ivalid );
		LIMITMINMAX(glossinessY, 0.0001f, 1.0f );

		pblock->GetValue( PB_SPECULAR_LEV, t, specLevel, ivalid );
		LIMITMINMAX(specLevel,0.0f,4.00f);
		pblock->GetValue( PB_DIFFUSE_LEV, t, diffLevel, ivalid );
		LIMITMINMAX(diffLevel,0.0f,2.0f);
		curTime = t;
	}
	valid &= ivalid;
	inUpdate = FALSE;
}

void WardShader::Reset()
{
	ReplaceReference( 0, CreateParameterBlock( WardShaderPB, WARD_SHADER_NPARAMS, WARD_SHADER_PB_VERSION ) );	
	ivalid.SetEmpty();
	SetAmbientClr(Color(0.1f,0.1f,0.1f),0);
	SetDiffuseClr(Color(0.5f,0.5f,0.5f),0);
	SetSpecularClr(Color(0.9f,0.9f,0.9f),0);
	SetGlossiness(.25f,0);   // change from .4, 5-21-97
	SetGlossinessY(.25f,0);   
	SetSpecularLevel(0.05f,0);   
	SetDiffuseLevel(0.8f,0);   

	SetLockADTex( TRUE );
	SetLockAD( FALSE );
	SetLockDS( FALSE );
	SetNormalizeOn( TRUE );
}



//////////////////////////////////////////////////////////////////////////////////////////
//
//	IO Routines
//
#define SHADER_HDR_CHUNK 0x4000
#define SHADER_LOCKDS_ON_CHUNK 0x5001
#define SHADER_LOCKAD_ON_CHUNK 0x5002
#define SHADER_LOCKADTEX_ON_CHUNK 0x5003
#define SHADER_MAPSON_CHUNK 0x5004
#define SHADER_NORMALIZE_ON_CHUNK 0x5005
#define SHADER_VERS_CHUNK 0x5300
#define SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK	0x5020

#define WARD_SHADER_VERSION  1 

// IO
IOResult WardShader::Save(ISave *isave) 
{ 
ULONG nb;

	isave->BeginChunk(SHADER_VERS_CHUNK);
	int version = WARD_SHADER_VERSION;
	isave->Write(&version,sizeof(version),&nb);			
	isave->EndChunk();

	if (lockDS) {
		isave->BeginChunk(SHADER_LOCKDS_ON_CHUNK);
		isave->EndChunk();
	}
	if (lockAD) {
		isave->BeginChunk(SHADER_LOCKAD_ON_CHUNK);
		isave->EndChunk();
	}

	if (lockADTex) {
		isave->BeginChunk(SHADER_LOCKADTEX_ON_CHUNK);
		isave->EndChunk();
	}
	if (normalizeOn) {
		isave->BeginChunk(SHADER_NORMALIZE_ON_CHUNK);
		isave->EndChunk();
	}

	isave->BeginChunk(SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK);
	ExposureMaterialControl::Save(isave);
	isave->EndChunk();
	return IO_OK;
}		

class WardShaderCB: public PostLoadCallback {
	public:
		WardShader *s;
		int loadVersion;
	    WardShaderCB(WardShader *newS, int loadVers) { s = newS; loadVersion = loadVers; }
		void proc(ILoad *iload) {}
};


IOResult WardShader::Load(ILoad *iload) { 
	ULONG nb;
	int id;
	int version = 0;

	lockAD = lockADTex = lockDS = normalizeOn = 0;

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
			case SHADER_NORMALIZE_ON_CHUNK:
				normalizeOn = TRUE;
				break;
			case SHADER_EXPOSURE_MATERIAL_CONTROL_CHUNK:
				res = ExposureMaterialControl::Load(iload);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	if (version < CURRENT_WARD_SHADER_VERSION ) {
		iload->RegisterPostLoadCallback(new WardShaderCB(this, version));
		iload->SetObsolete();
	}

	return IO_OK;

}

			
///////////////////////////////////////////////////////////////////////////////////////////
// The Shader
//

// defualt gloss squared, for renormalized controls
#define DEFAULT_GLOSS2	0.03f	

float WardShader::EvalHiliteCurve2( float x, float y, int )
{
	double g2 = normalizeOn ? glossinessX * glossinessY : DEFAULT_GLOSS2; 
	double gx2 = glossinessX * glossinessX; 
	double gy2 = glossinessY * glossinessY; 
	
	double t, a;
	double l = sqrt( x*x + y*y );
	if ( l == 0.0 ) {
		a = t = 0.0;
	} else {
		x /= float(l);	y /= float(l);
		t = tan( l*PI*0.5 );
		a = x*x/gx2 + y*y/gy2;
	}
	return specLevel*(float)(exp( -(t * t) * a ) / (4.0 * PI * g2));  
}


static int stopX = -1;
static int stopY = -1;
static int uvChan = 0;

void WardShader::Illum(ShadeContext &sc, IllumParams &ip) {
	LightDesc *l;
	Color lightCol;

#ifdef _DEBUG
	IPoint2 sp = sc.ScreenCoord();
	if ( sp.x == stopX && sp.y == stopY )
		sp.x = stopX;
#endif

	BOOL isShiny= (ip.channels[W_SL].r > 0.0f) ? 1 : 0; 

	for (int i=0; i<sc.nLights; i++) {
		l = sc.Light(i);
		float NL, Kl;
		Point3 L;
		if (l->Illuminate( sc, sc.Normal(), lightCol, L, NL, Kl)) {
			if (l->ambientOnly) {
				ip.ambIllumOut += lightCol;
				continue;
				}
			if (NL<=0.0f) 
				continue;

			// diffuse
			if (l->affectDiffuse){
				ip.diffIllumOut += Kl / Pi * ip.channels[W_DL].r * lightCol;
			}

			// specular  
			if (isShiny && l->affectSpecular) {
				float gx = ip.channels[W_GX].r;
				float gy = ip.channels[W_GY].r;
				assert( gx >= 0.0f && gy >= 0.0f );
				Point3 H = Normalize(L - sc.V() ); // (L + -V)/2
				float NH = DotProd(sc.Normal(), H);	 
				if (NH > 0.0f) {
					float g2 = normalizeOn ? gx * gy : DEFAULT_GLOSS2;
					float norm = 1.0f / (4.0f * PI * g2);
					float NV = -DotProd(sc.Normal(), sc.V() );
					if ( NV <= 0.001f)
						NV = 0.001f;
  
					float g = 1.0f / (float)sqrt( NL * NV );
					if ( g > 6.0f ) g = 6.0f;

					
					//Point3 basisVecs[ 3 ];
					//sc.DPdUVW( basisVecs, uvChan ); // 0 is vtxclr, 1..n is uv channels, max_meshmaps in mesh.h
					//basisVecs[0] = Normalize( basisVecs[0] );

					// This is the new preferred method for getting bump basis vectors -- DS 5/22/00
					Point3 basisVecs[2];
					sc.BumpBasisVectors(basisVecs, 0, uvChan);

					// the line between the tip of vec[0] and its projection on N is tangent
					Point3 T = basisVecs[0] - sc.Normal() * Dot( basisVecs[0], sc.Normal() );
					Point3 B = CrossProd( sc.Normal(), T );
					float x = DotProd( H, T ) / gx;
					float y = DotProd( H, B ) / gy;
					float e = (float)exp( -2.0 * (x*x + y*y) / (1.0+NH) );
					ip.specIllumOut += Kl * ip.channels[W_SL].r * norm * g * e * lightCol;
				}
			}
 		}
	} // for each light

	// now we can multiply by the clrs, 
	ip.ambIllumOut *= ip.channels[W_AM]; 
	ip.diffIllumIntens = Intens(ip.diffIllumOut);
	ip.diffIllumOut *= ip.channels[W_DI]; 
	ip.specIllumOut *= ip.channels[W_SP]; 

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
//	Ward shader dlg panel
//


// The dialog class
class WardShaderDlg : public ShaderParamDlg {
public:
	WardShader*	pShader;
	StdMat2*	pMtl;
	HPALETTE	hOldPal;
	HWND		hwmEdit;	// window handle of the materials editor dialog
	IMtlParams*	pMtlPar;
	HWND		hwHilite;   // the hilite window
	HWND		hRollup;	// Rollup panel
	TimeValue	curTime;
	BOOL		valid;
	BOOL		isActive;

	IColorSwatch *cs[WARD_NCOLBOX];
	ISpinnerControl *dlevSpin, *slevSpin, *glxSpin, *glySpin, *trSpin;
	ICustButton* texMBut[NMBUTS];
	TexDADMgr dadMgr;
	
	WardShaderDlg( HWND hwMtlEdit, IMtlParams *pParams ); 
	~WardShaderDlg(); 

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
	Class_ID ClassID(){ return WardShaderDlgClassID; }

	void SetThing(ReferenceTarget *m){ pMtl = (StdMat2*)m; }
	void SetThings( StdMat2* theMtl, Shader* theShader )
	{	if (pShader) pShader->SetParamDlg(NULL,0);   
		pShader = (WardShader*)theShader; 
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

static INT_PTR CALLBACK  WardShaderDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	WardShaderDlg *theDlg;
	if (msg == WM_INITDIALOG) {
		theDlg = (WardShaderDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	} else {
	    if ( (theDlg = (WardShaderDlg *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA) ) == NULL )
			return FALSE; 
	}
	theDlg->isActive = 1;
	BOOL res = theDlg->PanelProc(hwndDlg, msg, wParam, lParam);
	theDlg->isActive = 0;
	return res;
}


ShaderParamDlg* WardShader::CreateParamDialog(HWND hOldRollup, HWND hwMtlEdit, IMtlParams *imp, StdMat2* theMtl, int rollupOpen, int ) 
{
	Interval v;
	Update(imp->GetTime(),v);
	
	WardShaderDlg *pDlg = new WardShaderDlg(hwMtlEdit, imp);
	pDlg->SetThings( theMtl, this  );

	LoadStdShaderResources();
	if ( hOldRollup ) {
		pDlg->hRollup = imp->ReplaceRollupPage( 
			hOldRollup,
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_WARD3),
			WardShaderDlgProc, 
			GetString(IDS_KE_WARD_BASIC),	// your name here
			(LPARAM)pDlg , 
			// NS: Bugfix 263414 keep the old category and store it for the current rollup
			rollupOpen|ROLLUP_SAVECAT|ROLLUP_USEREPLACEDCAT
			);
	} else
		pDlg->hRollup = imp->AddRollupPage( 
			hInstance,
			MAKEINTRESOURCE(IDD_DMTL_BASIC_WARD3),
			WardShaderDlgProc, 
			GetString(IDS_KE_WARD_BASIC),	
			(LPARAM)pDlg , 
			rollupOpen
			);

	return (ShaderParamDlg*)pDlg;	
}

RefResult WardShader::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
									  PartID& partID, RefMessage message ) 
{
	switch (message) {
		case REFMSG_WANT_SHOWPARAMLEVEL:
			{
			BOOL *pb = (BOOL *)(partID);
			*pb = TRUE;
			return REF_STOP;
			}
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			if (paramDlg) {
				if (hTarget==pblock) {
					int np =pblock->LastNotifyParamNum();
					paramDlg->UpdateDialog( np );
				}
			}
			break;
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_AMBIENT_CLR: 
				case PB_DIFFUSE_CLR: 
				case PB_SPECULAR_CLR: 
					gpd->dim = stdColor255Dim; 
					break;
				case PB_GLOSSINESS_X:
				case PB_GLOSSINESS_Y:
				case PB_SPECULAR_LEV:
				case PB_DIFFUSE_LEV:
				default:
					gpd->dim = stdPercentDim; 
					break;
				}
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name =  GetString(paramNameIDS[gpn->index]);
			return REF_STOP; 
			}
	}
	return(REF_SUCCEED);
}

WardShaderDlg::WardShaderDlg( HWND hwMtlEdit, IMtlParams *pParams)
{
	pMtl = NULL;
	pShader = NULL;
	hwmEdit = hwMtlEdit;
	pMtlPar = pParams;
	dadMgr.Init(this);
	dlevSpin = slevSpin = glxSpin = glySpin = trSpin = NULL;
	hRollup = hwHilite = NULL;
	curTime = pMtlPar->GetTime();
	isActive = valid = FALSE;

	for( long i = 0; i < WARD_NCOLBOX; ++i )
		cs[ i ] = NULL;

	for( i = 0; i < NMBUTS; ++i )
		texMBut[ i ] = NULL;
}

WardShaderDlg::~WardShaderDlg()
{
	HDC hdc = GetDC(hRollup);
	GetGPort()->RestorePalette(hdc, hOldPal);
	ReleaseDC(hRollup, hdc);

	if( pShader ) pShader->SetParamDlg(NULL,0);

	for (long i=0; i < NMBUTS; i++ ){
		ReleaseICustButton( texMBut[i] );
		texMBut[i] = NULL; 
	}

	for (i=0; i<WARD_NCOLBOX; i++)
		if (cs[i]) ReleaseIColorSwatch(cs[i]); // mjm - 5.10.99

 	ReleaseISpinner(slevSpin);
	ReleaseISpinner(dlevSpin);
	ReleaseISpinner(glxSpin);
	ReleaseISpinner(glySpin);
	ReleaseISpinner(trSpin);

	SetWindowLongPtr(hRollup, GWLP_USERDATA, NULL);
	SetWindowLongPtr(hwHilite, GWLP_USERDATA, NULL);
	hwHilite = hRollup = NULL;
}


void  WardShaderDlg::LoadDialog(BOOL draw) 
{
	if (pShader && hRollup) {
		dlevSpin->SetValue(FracToPc(pShader->GetDiffuseLevel()),FALSE);
		dlevSpin->SetKeyBrackets(KeyAtCurTime(PB_DIFFUSE_LEV));

		slevSpin->SetValue(FracToPc(pShader->GetSpecularLevel()/SPEC_MAX),FALSE);
		slevSpin->SetKeyBrackets(KeyAtCurTime(PB_SPECULAR_LEV));

		glxSpin->SetValue( 100.0f * ((ALPHA_MAX - pShader->GetGlossiness())/ALPHA_SZ),FALSE);
		glxSpin->SetKeyBrackets(KeyAtCurTime(PB_GLOSSINESS_X));
		glySpin->SetValue(100.0f * ((ALPHA_MAX - pShader->GetGlossinessY())/ALPHA_SZ),FALSE);
		glySpin->SetKeyBrackets(KeyAtCurTime(PB_GLOSSINESS_Y));

		trSpin->SetValue(FracToPc(pMtl->GetOpacity( curTime )),FALSE);
		trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));

		CheckButton(hRollup, IDC_LOCK_AD, pShader->GetLockAD() );
		CheckButton(hRollup, IDC_LOCK_DS, pShader->GetLockDS() );
		SetCheckBox(hRollup, IDC_NORMALIZE_CHECK, !pShader->GetNormalizeOn() ); 
	 	UpdateLockADTex( FALSE ); //don't send to mtl

		UpdateColSwatches();
		UpdateHilite();
	}
}


static TCHAR* mapStates[] = { _T(" "), _T("m"),  _T("M") };

void WardShaderDlg::UpdateMapButtons() 
{

	for ( long i = 0; i < NMBUTS; ++i ) {
		int nMap = texmapFromMBut[ i ];
		int state = pMtl->GetMapState( nMap );
		texMBut[i]->SetText( mapStates[ state ] );

		TSTR nm	 = pMtl->GetMapName( nMap );
		texMBut[i]->SetTooltip(TRUE,nm);
	}
}


void WardShaderDlg::SetLockAD(BOOL lock)
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

void WardShaderDlg::UpdateOpacity() 
{
	trSpin->SetValue(FracToPc(pMtl->GetOpacity(curTime)),FALSE);
	trSpin->SetKeyBrackets(pMtl->KeyAtTime(OPACITY_PARAM, curTime));
}


void WardShaderDlg::UpdateColSwatches() 
{
	for(int i=0; i < WARD_NCOLBOX; i++) {
		if ( cs[ i ] ) {
			cs[i]->SetKeyBrackets( pShader->KeyAtTime(PB_AMBIENT_CLR+i,curTime) );
			cs[i]->SetColor( GetMtlColor(i, (Shader*)pShader) );
		}
	}
}


void WardShaderDlg::UpdateHilite()
{
	HDC hdc = GetDC(hwHilite);
	Rect r;
	GetClientRect(hwHilite,&r);
	DrawHilite2(hdc, r, pShader );
	ReleaseDC(hwHilite,hdc);
}

void WardShaderDlg::UpdateLockADTex( BOOL passOn) {
	int lock = 	pShader->GetLockADTex();
	CheckButton(hRollup, IDC_LOCK_ADTEX, lock);

	ShowWindow(GetDlgItem(hRollup, IDC_MAPON_AM), !lock);
	texMBut[ 0 ]->Enable(!lock);

	if ( passOn ) 
		pMtl->SyncADTexLock( lock );
	
//	UpdateMtlDisplay();
}

void WardShaderDlg::SetLockADTex(BOOL lock) {
	pShader->SetLockADTex( lock );
	UpdateLockADTex(TRUE); // passon to mtl
//	UpdateMtlDisplay();
}

void WardShaderDlg::SetLockDS(BOOL lock) 
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
		default: return 0;
	}
}


BOOL WardShaderDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
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

			for (i=0; i<WARD_NCOLBOX; i++) {
   				cs[i] = GetIColorSwatch(GetDlgItem(hwndDlg, colID[i]),
   					GetMtlColor(i, pShader), GetColorName(i));
			}

			hwHilite = GetDlgItem(hwndDlg, IDC_HIGHLIGHT);
			SetWindowLongPtr( hwHilite, GWLP_WNDPROC, (LONG_PTR)Hilite2WndProc);

			slevSpin = SetupIntSpinner(hwndDlg, IDC_SLEV_SPIN, IDC_SLEV_EDIT, 0,400, 0);
			dlevSpin = SetupIntSpinner(hwndDlg, IDC_DLEV_SPIN, IDC_DLEV_EDIT, 0, 400, 0);
			glxSpin = SetupIntSpinner(hwndDlg, IDC_GLX_SPIN, IDC_GLX_EDIT, 0,100, 0);
			glySpin = SetupIntSpinner(hwndDlg, IDC_GLY_SPIN, IDC_GLY_EDIT, 0,100, 0);
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

				case IDC_NORMALIZE_CHECK:
					pShader->SetNormalizeOn( ! GetCheckBox(hwndDlg, IDC_NORMALIZE_CHECK) );
					UpdateHilite();
					NotifyChanged();
					UpdateMtlDisplay();
					break;
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
				case IDC_GLX_SPIN: 
					pShader->SetGlossiness(ALPHA_MAX - PcToFrac( glxSpin->GetIVal() ) * ALPHA_SZ, curTime); 
					UpdateHilite();
					break;
				case IDC_GLY_SPIN: 
					pShader->SetGlossinessY(ALPHA_MAX - PcToFrac(glySpin->GetIVal()) * ALPHA_SZ, curTime); 
					UpdateHilite();
					break;
				case IDC_SLEV_SPIN: 
					pShader->SetSpecularLevel(SPEC_MAX * PcToFrac(slevSpin->GetIVal()),curTime); 
					UpdateHilite();
					break;
				case IDC_DLEV_SPIN: 
					pShader->SetDiffuseLevel(PcToFrac(dlevSpin->GetIVal()),curTime); 
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


