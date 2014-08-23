/**********************************************************************
 *<
	FILE: stdmtl2.h

	DESCRIPTION:

	CREATED BY: Dan Silva modified for shiva by Kells Elmquist

	HISTORY: modified for shiva by Kells Elmquist
	         modified to use ParamBlock2, John Wainwright 11/16/98

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#ifndef __STDMTL2__H
#define __STDMTL2__H

//JH 5/24/03 Adding support for global supersampler
#define GLOBAL_SUPERSAMPLER

#include "shaders.h"
#include "samplers.h"
#include "iparamm2.h"
#include "texmaps.h"
#include "expmtlControl.h"

// StdMtl2 flags values
#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
#define STDMTL_ADD_TRANSP   (1<<0)
#endif // USE_LIMITED_STDMTL

#define STDMTL_FALLOFF_OUT  (1<<1)
#define STDMTL_WIRE		  	(1<<2)
#define STDMTL_2SIDE		(1<<3)
#define STDMTL_SOFTEN       (1<<4)
#define STDMTL_FILT_TRANSP 	(1<<5)
#define STDMTL_WIRE_UNITS	(1<<6)
#define STDMTL_LOCK_AD      (1<<8)
#define STDMTL_LOCK_DS      (1<<9)
#define STDMTL_UNUSED1		(1<<10)
#define STDMTL_LOCK_ADTEX   (1<<11)
#define STDMTL_FACEMAP		(1<<12)
#define STDMTL_OLDSPEC      (1<<13)
#define STDMTL_SSAMP_ON		(1<<14)
#define STDMTL_COLOR_SI		(1<<15)
#define STDMTL_FACETED		(1<<16)

#define STDMTL_ROLLUP0_OPEN  (1<<27)	// shader
#define STDMTL_ROLLUP1_OPEN  (1<<28)	// basic
#define STDMTL_ROLLUP2_OPEN  (1<<29)	// extra
#define STDMTL_ROLLUP3_OPEN  (1<<30)	// maps
#define STDMTL_ROLLUP4_OPEN  (1<<26)	// sampling
#define STDMTL_ROLLUP5_OPEN  (1<<25)	// dynamics
#define STDMTL_ROLLUP6_OPEN  (1<<24)	// effects

// extended standard id's for translucency
#define ID_TL		12		// translucent color
#define ID_TLB		13		// translucent blur

// only needed if the constant shader is included in shaders
#define  CONSTClassID (STDSHADERS_CLASS_ID+1)

#define STDMTL_ROLLUP_FLAGS (STDMTL_ROLLUP0_OPEN|STDMTL_ROLLUP1_OPEN|STDMTL_ROLLUP2_OPEN|STDMTL_ROLLUP3_OPEN \
							|STDMTL_ROLLUP4_OPEN|STDMTL_ROLLUP5_OPEN|STDMTL_ROLLUP6_OPEN)

class StdMtl2Dlg;


// IDs for all the ParamBlocks and their parameters.  One block UI per rollout.
enum { std2_shader, std2_extended, std2_sampling, std_maps, std2_dynamics, };  // pblock IDs
// std2_shader param IDs
enum 
{ 
	std2_shader_type, std2_wire, std2_two_sided, std2_face_map, std2_faceted,
	std2_shader_by_name,  // virtual param for accessing shader type by name
};
// std2_extended param IDs
enum 
{ 
	std2_opacity_type, std2_opacity, std2_filter_color, std2_ep_filter_map,
	std2_falloff_type, std2_falloff_amnt, 
	std2_ior,
	std2_wire_size, std2_wire_units,
	std2_apply_refl_dimming, std2_dim_lvl, std2_refl_lvl,
	std2_translucent_blur, std2_ep_translucent_blur_map,
	std2_translucent_color, std2_ep_translucent_color_map,
};

// std2_sampling param IDs
enum 
{ 
	std2_ssampler, std2_ssampler_qual, std2_ssampler_enable, 
		std2_ssampler_adapt_on, std2_ssampler_adapt_threshold, std2_ssampler_advanced,
		std2_ssampler_subsample_tex_on, std2_ssampler_by_name, 
		std2_ssampler_param0, std2_ssampler_param1, std2_ssampler_useglobal
};
// std_maps param IDs
enum 
{
	std2_map_enables, std2_maps, std2_map_amnts, std2_mp_ad_texlock, 
};
// std2_dynamics param IDs
enum 
{
	std2_bounce, std2_static_friction, std2_sliding_friction,
};


// paramblock2 block and parameter IDs for the standard shaders
// NB these are duplicated in shaders/stdShaders.cpp...
enum { shdr_params, };
// shdr_params param IDs
enum 
{ 
	shdr_ambient, shdr_diffuse, shdr_specular,
	shdr_ad_texlock, shdr_ad_lock, shdr_ds_lock, 
	shdr_use_self_illum_color, shdr_self_illum_amnt, shdr_self_illum_color, 
	shdr_spec_lvl, shdr_glossiness, shdr_soften,
};



#define NUM_REFS		9

// refs
#define OLD_PBLOCK_REF	0		// reference number assignments
#define TEXMAPS_REF		1
#define SHADER_REF		2
#define SHADER_PB_REF	3
#define EXTENDED_PB_REF	4
#define SAMPLING_PB_REF	5
#define MAPS_PB_REF		6
#define DYNMAICS_PB_REF	7
#define SAMPLER_REF		8

// sub anims
#if !defined(NO_OUTPUTRENDERER)	&& !defined(USE_LIMITED_STDMTL)	// russom - 04/19/01
#if !defined( DESIGN_VER )
  #define NUM_SUB_ANIMS	5
#else
  #define NUM_SUB_ANIMS	4 // aszabo|Sep.28.01
#endif // !DESIGN_VER
#else
  #define NUM_SUB_ANIMS	3
#endif	// NO_OUTPUTRENDERER

//#define OLD_PARAMS_SUB		0
#define TEXMAPS_SUB			0
#define SHADER_SUB			1
#define EXTRA_PB_SUB		2
#if  !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)	// russom - 04/19/01
  #define SAMPLING_PB_SUB		3
#if !defined( DESIGN_VER ) // aszabo|Sep.28.01
  #define DYNAMICS_PB_SUB		4
#endif // !DESIGN_VER
#endif	// NO_OUTPUTRENDERER

// these define the evalType parameter for the private
// EvalReflection & EvalRefraction calls
#define EVAL_CHANNEL	0
#define RAY_QUERY		1

class RefmsgKillCounter {
private:
	friend class KillRefmsg;
	LONG	counter;

public:
	RefmsgKillCounter() : counter(-1) {}

	bool DistributeRefmsg() { return counter < 0; }
};

class KillRefmsg {
private:
	LONG&	counter;

public:
	KillRefmsg(RefmsgKillCounter& c) : counter(c.counter) { ++counter; }
	~KillRefmsg() { --counter; }
};

class StdMtl2 : public ExposureMaterialControlImp<StdMtl2,
							AddExposureMaterialControl<StdMat2> >,
				public IReshading {
	typedef ExposureMaterialControlImp<StdMtl2,
		AddExposureMaterialControl<StdMat2> > BaseClass;

	// Animatable parameters
	public:
		// current UI if open
		static ShaderParamDlg* pShaderDlg;
		static IAutoMParamDlg* masterDlg;
		static IAutoMParamDlg* texmapDlg;
		static IAutoMParamDlg* extendedDlg;
		static IAutoMParamDlg* samplingDlg;
		static HWND			   curHwmEdit;
		static IMtlParams*	   curImp;
		static Tab<ClassDesc*> shaderList;
		static Tab<ClassDesc*> samplerList;

		IParamBlock *old_pblock;    // ref 0, for old version loading
		Texmaps* maps;				// ref 1
		Interval ivalid;
		ReshadeRequirements mReshadeRQ; // mjm - 06.02.00
		ReshadeRequirements mInRQ;		// ca - 12/7/00
		ULONG flags;
		int shaderId;
		Shader *pShader;			// ref 2
		// new PB2 paramblocks, one per rollout
		IParamBlock2 *pb_shader;	// ref 3, 4, ...
		IParamBlock2 *pb_extended;	
		IParamBlock2 *pb_sampling;	
		IParamBlock2 *pb_maps;	
		IParamBlock2 *pb_dynamics;	

		Color filter;
		float opacity;	
		float opfall;
		float wireSize;
		float ioRefract;
#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
		float dimIntens;
		float dimMult;
		BOOL dimReflect;
#endif

        Color translucentColor;
        float translucentBlur;

		// sampling 
		int samplerId;
		Sampler* pixelSampler;	// ref 8

		// composite of shader/mtl channel types
		int channelTypes[ STD2_NMAX_TEXMAPS ];
		int stdIDToChannel[ N_ID_CHANNELS ];

		// experiment: override filter, >>>>>>>>>> remove this next api change
//		BOOL	filterOverrideOn;
//		float	filterSz;

		// Kill REFMSG_CHANGE messages. This counter is used to
		// prevent these messages when things really aren't changing.
		// Use the class KillRefmsg
		RefmsgKillCounter	killRefmsg;

		static ExposureMaterialControlDesc msExpMtlControlDesc;

		void SetFlag(ULONG f, ULONG val);
		void EnableMap(int i, BOOL onoff);
		BOOL IsMapEnabled(int i) { return (*maps)[i].mapOn; }
		BOOL KeyAtTime(int id,TimeValue t) { return (id == OPACITY_PARAM) ? pb_extended->KeyFrameAtTime(std2_opacity, t) : FALSE; }
		BOOL AmtKeyAtTime(int i, TimeValue t);
		int  GetMapState( int indx ); //returns 0 = no map, 1 = disable, 2 = mapon
		TSTR  GetMapName( int indx ); 
		void SyncADTexLock( BOOL lockOn );

		// from StdMat
		// these set Approximate colors into the plug in shader
		BOOL IsSelfIllumColorOn();
		void SetSelfIllumColorOn( BOOL on );
		void SetSelfIllumColor(Color c, TimeValue t);		
		void SetAmbient(Color c, TimeValue t);		
		void SetDiffuse(Color c, TimeValue t);		
		void SetSpecular(Color c, TimeValue t);
		void SetShininess(float v, TimeValue t);		
		void SetShinStr(float v, TimeValue t);		
		void SetSelfIllum(float v, TimeValue t);	
		void SetSoften(BOOL onoff) { SetFlag(STDMTL_SOFTEN,onoff); }
		
		void SetTexmapAmt(int imap, float amt, TimeValue t);
		void LockAmbDiffTex(BOOL onoff) { SetFlag(STDMTL_LOCK_ADTEX,onoff); }

		void SetWire(BOOL onoff){ pb_shader->SetValue(std2_wire,0, (onoff!=0) ); }//SetFlag(STDMTL_WIRE,onoff); }
		void SetWireSize(float s, TimeValue t);
#ifndef NO_OUTPUTRENDERER	// russom - 08/03/01
		void SetWireUnits(BOOL onoff) { pb_extended->SetValue(std2_wire_units,0, (onoff!=0) ); } //SetFlag(STDMTL_WIRE_UNITS,onoff); }
#else
		void SetWireUnits(BOOL onoff) { return; };
#endif
		
		void SetFaceMap(BOOL onoff) { pb_shader->SetValue(std2_face_map,0, (onoff!=0) ); } //SetFlag(STDMTL_FACEMAP,onoff); }
		void SetTwoSided(BOOL onoff) { pb_shader->SetValue(std2_two_sided,0, (onoff!=0) ); } //SetFlag(STDMTL_2SIDE,onoff); }
		void SetFalloffOut(BOOL outOn) { pb_extended->SetValue(std2_falloff_type,0, (outOn!=0) ); } //SetFlag(STDMTL_FALLOFF_OUT,onoff); }

		void SetTransparencyType(int type);

		void SetFilter(Color c, TimeValue t);
		void SetOpacity(float v, TimeValue t);		
		void SetOpacFalloff(float v, TimeValue t);		
		void SetIOR(float v, TimeValue t);

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
		void SetDimIntens(float v, TimeValue t);
		void SetDimMult(float v, TimeValue t);
#endif // USE_LIMITED_STDMTL
		
        // translucency
	void SetTranslucentColor(Color& c, TimeValue t);
        void SetTranslucentBlur(float v, TimeValue t);
		
	    int GetFlag(ULONG f) { return (flags&f)?1:0; }

		// >>>Shaders

		// these 3 internal only
		void SetShaderIndx( long shaderId, BOOL update=TRUE );
		long GetShaderIndx(){ return shaderId; }
		void SetShader( Shader* pNewShader );
		void ShuffleTexMaps( Shader* newShader, Shader* oldShader );
		void ShuffleShaderParams( Shader* newShader, Shader* oldShader );

		Shader* GetShader(){ return pShader; }
		void SwitchShader(Shader* pNewShader, BOOL loadDlg = FALSE);
		void SwitchShader(ClassDesc* pNewCD);
		BOOL SwitchShader(Class_ID shaderId);
		int FindShader( Class_ID& findId, ClassDesc** ppCD=NULL );
		BOOL IsShaderInUI() { return pb_shader && pb_shader->GetMap() && pShader && pShader->GetParamDlg(); }

		static void StdMtl2::LoadShaderList();
		static int StdMtl2::NumShaders();
		static ClassDesc* StdMtl2::GetShaderCD(int i);
		static void StdMtl2::LoadSamplerList();
		static int StdMtl2::NumSamplers();
		static ClassDesc* StdMtl2::GetSamplerCD(int i);

		BOOL IsFaceted(){ return GetFlag(STDMTL_FACETED); }
		void SetFaceted( BOOL on ){	pb_shader->SetValue(std2_faceted,0, (on!=0) ); }

		// These utilitys provide R2.5 shaders, ONLY used for Translators
		// Does not & will not work for plug-in shaders
		void SetShading(int s);
		int GetShading();

		// from Mtl
		Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);		
	    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);		
		Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
		float GetShininess(int mtlNum=0, BOOL backFace=FALSE);	
		float GetShinStr(int mtlNum=0, BOOL backFace=FALSE) ;
		float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
		float WireSize(int mtlNum=0, BOOL backFace=FALSE) { return wireSize; }

		// >>>> Self Illumination 
		float GetSelfIllum(int mtlNum, BOOL backFace) ;
		BOOL  GetSelfIllumColorOn(int mtlNum, BOOL backFace);
		Color GetSelfIllumColor(int mtlNum, BOOL backFace);
		
		// >>>> sampling
#ifdef GLOBAL_SUPERSAMPLER
		void SetGlobalSamplingOn( BOOL on )
		{	
		#if !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)		// russom - 04/19/01 - remove SuperSampling
			pb_sampling->SetValue(std2_ssampler_useglobal, 0, on!=0 );
		#endif
		}	
		BOOL GetGlobalSamplingOn()
		{	
		#if !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)		// russom - 04/19/01 - remove SuperSampling
			Interval iv; 
			BOOL on;
			pb_sampling->GetValue(std2_ssampler_useglobal, 0, on, iv );
			return on;
		#else
			return FALSE;
		#endif
		}	
#endif //GLOBAL_SUPERSAMPLER
		void SetSamplingOn( BOOL on )
		{	
		#if !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)		// russom - 04/19/01 - remove SuperSampling
			pb_sampling->SetValue(std2_ssampler_enable, 0, on!=0 );
		#endif
		}	
		BOOL GetSamplingOn()
		{	
		#if !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)		// russom - 04/19/01 - remove SuperSampling
			Interval iv; 
			BOOL on;
			pb_sampling->GetValue(std2_ssampler_enable, 0, on, iv );
			return on;
		#else
			return FALSE;
		#endif
		}	
		void SetSamplingQuality( float quality )
		{	 
		#if !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)		// russom - 04/19/01 - remove SuperSampling
			pb_sampling->SetValue(std2_ssampler_qual, 0, quality );
		#endif
		}	
		float GetSamplingQuality()
		{	
		#if !defined(NO_OUTPUTRENDERER) && !defined(USE_LIMITED_STDMTL)		// russom - 04/19/01 - remove SuperSampling
			Interval iv; 
			float q;
			pb_sampling->GetValue(std2_ssampler_qual, 0, q, iv );
			return q;
		#else
			return 0.0f;
		#endif
		}

		void SwitchSampler(ClassDesc* pNewCD);
		void SwitchSampler(Sampler* pNewSampler);
		BOOL SwitchSampler(Class_ID samplerId);
		static int FindSampler( Class_ID findId, ClassDesc** pNewCD=NULL );
		Sampler * GetPixelSampler(int mtlNum=0, BOOL backFace=FALSE){  return pixelSampler; }	

		// these 2 internal only
		void SetSamplerIndx( long indx, BOOL update=TRUE );
		long  GetSamplerIndx(){ return samplerId; }
		void SetPixelSampler( Sampler * sampler );

		// Dynamics properties
		float GetDynamicsProperty(TimeValue t, int mtlNum, int propID);
		void SetDynamicsProperty(TimeValue t, int mtlNum, int propID, float value);

		// from StdMat
		BOOL GetSoften() { return GetFlag(STDMTL_SOFTEN); }
		BOOL GetFaceMap() { return GetFlag(STDMTL_FACEMAP); }
		BOOL GetTwoSided() { return GetFlag(STDMTL_2SIDE); }
		BOOL GetWire() { return GetFlag(STDMTL_WIRE); }
		BOOL GetWireUnits() { return GetFlag(STDMTL_WIRE_UNITS); }
		BOOL GetFalloffOut() { return GetFlag(STDMTL_FALLOFF_OUT); }  // 1: out, 0: in
		BOOL GetAmbDiffTexLock(){ return GetFlag(STDMTL_LOCK_ADTEX); } 
		int GetTransparencyType() {
#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
			return (flags&STDMTL_FILT_TRANSP)?TRANSP_FILTER:
				flags&STDMTL_ADD_TRANSP ? TRANSP_ADDITIVE: TRANSP_SUBTRACTIVE;
#else
			return TRANSP_FILTER;
#endif // USE_LIMITED_STDMTL

			}
		Color GetFilter(TimeValue t);

		// these are stubs till i figure out scripting
		Color GetAmbient(TimeValue t);		
		Color GetDiffuse(TimeValue t);		
		Color GetSpecular(TimeValue t);
		float GetShininess( TimeValue t);		
		float GetShinStr(TimeValue t);	
		float GetSelfIllum(TimeValue t);
		BOOL  GetSelfIllumColorOn();
		Color GetSelfIllumColor(TimeValue t); 

		float GetOpacity( TimeValue t);		
		float GetOpacFalloff(TimeValue t);		
		float GetWireSize(TimeValue t);
		float GetIOR( TimeValue t);

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
		float GetDimIntens( TimeValue t);
		float GetDimMult( TimeValue t);
#endif // USE_LIMITED_STDMTL

		float GetSoftenLevel( TimeValue t);
		BOOL MapEnabled(int i);
		float GetTexmapAmt(int imap, TimeValue t);

		// internal
		float GetOpacity() { return opacity; }		
		float GetOpacFalloff() { return opfall; }		
		float GetTexmapAmt(int imap);
		Color GetFilter();
		float GetIOR() { return ioRefract; }

      
        // translucency
	Color GetTranslucentColor(TimeValue t);
        float GetTranslucentBlur(TimeValue t);

		StdMtl2(BOOL loading = FALSE);
		~StdMtl2() {
			DiscardTexHandles();
			}
		BOOL ParamWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		BOOL SetDlgThing(ParamDlg* dlg);
		void UpdateTexmaps();
		void UpdateMapButtons();
		void UpdateExtendedMapButtons();
		void UpdateMtlDisplay();
		void UpdateLockADTex( BOOL passOn );
		void UpdateExtraParams( ULONG stdParams );
		void UpdateSamplingParams();
#ifdef GLOBAL_SUPERSAMPLER
		void EnableLocalSamplerControls();
#endif //GLOBAL_SUPERSAMPLER


		Color TranspColor(ShadeContext& sc, float opac, Color& diff);
		void Shade(ShadeContext& sc);
		float EvalDisplacement(ShadeContext& sc); 
		Interval DisplacementValidity(TimeValue t); 
		void Update(TimeValue t, Interval& validr);
		void Reset();
		void OldVerFix(int loadVer);
		void BumpFix();
		Interval Validity(TimeValue t);
		void NotifyChanged();

		// Requirements
		int BuildMaps(TimeValue t, RenderMapsContext &rmc);
		ULONG LocalRequirements(int subMtlNum);
		ULONG Requirements(int subMtlNum);
		void MappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq);

		// Methods to access texture maps of material
		int NumSubTexmaps() { return STD2_NMAX_TEXMAPS; }
		Texmap* GetSubTexmap(int i) { return (*maps)[i].map; }
		int MapSlotType(int i);
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i);
		int SubTexmapOn(int i) { return  MAPACTIVE(i); } 
		long StdIDToChannel( long id ){ return stdIDToChannel[id]; }

		Class_ID ClassID();
		SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
		void GetClassName(TSTR& s) { s = GetString(IDS_KE_STANDARD2); }  

		void DeleteThis();

		int NumSubs() { return NUM_SUB_ANIMS; }  
	    Animatable* SubAnim(int i);
		TSTR SubAnimName(int i);
		int SubNumToRefNum(int subNum);

		// JBW: add direct ParamBlock access
		int	NumParamBlocks() { return 5; }
		IParamBlock2* GetParamBlock(int i);
		IParamBlock2* GetParamBlockByID(BlockID id);

		// From ref
 		int NumRefs() { return NUM_REFS; }
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);

		RefTargetHandle Clone(RemapDir &remap = NoRemap());
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

// begin - ke/mjm - 03.16.00 - merge reshading code
		BOOL SupportsRenderElements(){ return TRUE; }
//		BOOL SupportsReShading(ShadeContext& sc);
		void UpdateReshadeRequirements(RefTargetHandle hTarget, PartID partID); // mjm - 06.02.00
		ReshadeRequirements GetReshadeRequirements() { return mReshadeRQ; } // mjm - 06.02.00
		void PreShade(ShadeContext& sc, IReshadeFragment* pFrag);
		void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip);
// end - ke/mjm - 03.16.00 - merge reshading code

// begin - dds- 04.27.00 - multiple map display support
#define NTEXHANDLES 2
		TexHandle *texHandle[NTEXHANDLES];
		short useSubForTex[NTEXHANDLES];
		short texOpsType[NTEXHANDLES];
		int numTexHandlesUsed;
		Interval texHandleValid;
		void SetTexOps(Material *mtl, int i, int type);
		void DiscardTexHandles();
		BOOL SupportTexDisplay() { return TRUE; }
		BOOL SupportsMultiMapsInViewport() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff);
		void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);
// end - dds- 04.27.00 - multiple map display support

		// --- Material evaluation - from Mtl ---
		bool IsOutputConst( ShadeContext& sc, int stdID );
		bool EvalColorStdChannel( ShadeContext& sc, int stdID, Color& outClr );
		bool EvalMonoStdChannel( ShadeContext& sc, int stdID, float& outVal );

		void* GetInterface(ULONG id);

#ifndef USE_LIMITED_STDMTL // orb 01-14-2002
		float GetReflectionDim(float diffIllumIntensity ){
			if (dimReflect)
				return ((1.0f-dimIntens)*diffIllumIntensity*dimMult + dimIntens);
			else 
				return 1.0f;
		}
#endif
		Color TranspColor( float opac, Color filt, Color diff );

		float GetEffOpacity(ShadeContext& sc, float opac){
			if ( opac != 1.0f || opfall != 0.0f) {
				if (opfall != 0.0f) {	
					Point3 N = (flags & STDMTL_FACETED) ? sc.GNormal() : sc.Normal();
					float d = (float)fabs( DotProd( N, sc.V() ) );
					if (flags & STDMTL_FALLOFF_OUT) d = 1.0f-d;
					return opac * (1.0f - opfall * d);
				} else return opac;
			} else return 1.0f;
		}

		virtual void SetNoExposure(BOOL on) {
			ExposureMaterialControl::SetNoExposure(on);
			ivalid.SetEmpty();
			NotifyChanged(); }
		virtual void SetInvertSelfIllum(BOOL on) {
			ExposureMaterialControl::SetInvertSelfIllum(on);
			ivalid.SetEmpty();
			NotifyChanged(); }
		virtual void SetInvertReflect(BOOL on) {
			ExposureMaterialControl::SetInvertReflect(on);
			ivalid.SetEmpty();
			NotifyChanged(); }
		virtual void SetInvertRefract(BOOL on) {
			ExposureMaterialControl::SetInvertRefract(on);
			ivalid.SetEmpty();
			NotifyChanged(); }

	private:
		// --- Material evaluation ---
		bool EvalChannel( ShadeContext& sc, int channelID, Color& outClr);
		bool EvalBump( ShadeContext& sc, Color& outClr );
		bool EvalReflection( ShadeContext& sc, Color& outClr, int evalType = EVAL_CHANNEL );
		bool EvalReflection( ShadeContext& sc, float& outVal, int evalType = EVAL_CHANNEL );
		bool EvalRefraction( ShadeContext& sc, Color& outClr, int evalType = EVAL_CHANNEL );
		bool EvalRefraction( ShadeContext& sc, float& outVal, int evalType = EVAL_CHANNEL );
		bool EvalDisplacement( ShadeContext& sc, float& outVal );
		bool ShouldEvalSubTexmap( ShadeContext& sc, int id );

	};

Mtl* CreateStdMtl2();

#endif
