#pragma once

class CRender	:	public IRender_interface
{
public:
	struct		_options	{
		u32		bug : 1;

		u32		ssao_blur_on : 1;
		u32		ssao_opt_data : 1;
		u32		ssao_half_data : 1;
		u32		ssao_hbao : 1;
		u32		ssao_hdao : 1;

		u32		smapsize : 16;
		u32		depth16 : 1;
		u32		mrt : 1;
		u32		mrtmixdepth : 1;
		u32		fp16_filter : 1;
		u32		fp16_blend : 1;
		u32		albedo_wo : 1;						// work-around albedo on less capable HW
		u32		HW_smap : 1;
		u32		HW_smap_PCF : 1;
		u32		HW_smap_FETCH4 : 1;

		u32		HW_smap_FORMAT : 32;

		u32		nvstencil : 1;
		u32		nvdbt : 1;

		u32		nullrt : 1;

		u32		distortion : 1;
		u32		distortion_enabled : 1;
		u32		mblur : 1;

		u32		sunfilter : 1;
		u32		sunstatic : 1;
		u32		sjitter : 1;
		u32		noshadows : 1;
		u32		Tshadows : 1;						// transluent shadows
		u32		disasm : 1;
		u32		advancedpp : 1;	//	advanced post process (DOF, SSAO, volumetrics, etc.)

		u32		forcegloss : 1;
		u32		forceskinw : 1;
		float	forcegloss_v;
	}			o;
public:
	// feature level
	virtual	GenerationLevel			get_generation() { return IRender_interface::GENERATION_R2; };

	virtual bool					is_sun_static() { return o.sunstatic; };
	virtual DWORD					get_dx_level() { return 0x00000000; };

	// Loading / Unloading
	virtual	void					create();
	virtual	void					destroy() {};
	virtual	void					reset_begin() {};
	virtual	void					reset_end() {};

	virtual	void					level_Load(IReader*) {};
	virtual void					level_Unload() {};

	virtual HRESULT					shader_compile(
		LPCSTR							name,
		LPCSTR                          pSrcData,
		UINT                            SrcDataLen,
		void*							pDefines,
		void*							pInclude,
		LPCSTR                          pFunctionName,
		LPCSTR                          pTarget,
		DWORD                           Flags,
		void*							ppShader,
		void*							ppErrorMsgs,
		void*							ppConstantTable) { return E_NOTIMPL; };

	// Information
	virtual	void					Statistics(CGameFont* F)							{};

	virtual LPCSTR					getShaderPath() { return ""; };
	virtual IRender_Sector*			getSector(int id) { return nullptr; };
	virtual IRenderVisual*			getVisual(int id) { return nullptr; };
	virtual IRender_Sector*			detectSector(const Fvector& P) { return nullptr; };
	virtual IRender_Target*			getTarget() { return nullptr; };

	// Main 
	IC		void					set_Frustum(CFrustum*	O)							{ VERIFY(O);	View = O; }
	virtual void					set_Transform(Fmatrix*	M) {};
	virtual void					set_HUD(BOOL 		V) {};
	virtual BOOL					get_HUD() { return false; };
	virtual void					set_Invisible(BOOL 		V) {};
	virtual void					flush() {};
	virtual void					set_Object(IRenderable*		O) {};
	virtual	void					add_Occluder(Fbox2&	bb_screenspace) {};	// mask screen region as oclluded (-1..1, -1..1)
	virtual void					add_Visual(IRenderVisual*	V) {};	// add visual leaf	(no culling performed at all)
	virtual void					add_Geometry(IRenderVisual*	V) {};	// add visual(s)	(all culling performed)
	virtual void					add_StaticWallmark(const wm_shader& S, const Fvector& P, float s, CDB::TRI* T, Fvector* V) {};
	virtual void					add_StaticWallmark(IWallMarkArray *pArray, const Fvector& P, float s, CDB::TRI* T, Fvector* V) {};
	virtual void					clear_static_wallmarks() {};
	virtual void					add_SkeletonWallmark(const Fmatrix* xf, IKinematics* obj, IWallMarkArray *pArray, const Fvector& start, const Fvector& dir, float size) {};

	virtual IRender_ObjectSpecific*	ros_create(IRenderable* parent) { return nullptr; };
	virtual void					ros_destroy(IRender_ObjectSpecific* &) {};

	// Lighting/glowing
	virtual IRender_Light*			light_create() { return nullptr; };
	virtual void					light_destroy(IRender_Light* p_)							{ };
	virtual IRender_Glow*			glow_create() { return nullptr; };
	virtual void					glow_destroy(IRender_Glow* p_)							{ };

	// Models
	virtual IRenderVisual*			model_CreateParticles(LPCSTR name) { return nullptr; };
	virtual IRenderVisual*			model_Create(LPCSTR name, IReader*	data = 0) { return nullptr; };
	virtual IRenderVisual*			model_CreateChild(LPCSTR name, IReader*	data) { return nullptr; };
	virtual IRenderVisual*			model_Duplicate(IRenderVisual*	V) { return nullptr; };
	virtual void					model_Delete(IRenderVisual* &	V, BOOL bDiscard = FALSE) {};
	virtual void					model_Logging(BOOL bEnable) {};
	virtual void					models_Prefetch() {};
	virtual void					models_Clear(BOOL b_complete) {};

	// Occlusion culling
	virtual BOOL					occ_visible(vis_data&	V) { return false; };
	virtual BOOL					occ_visible(Fbox&		B) { return false; };
	virtual BOOL					occ_visible(sPoly&		P) { return false; };

	// Main
	virtual void					Calculate() {};
	virtual void					Render() {};

	virtual void					Screenshot(ScreenshotMode mode = SM_NORMAL, LPCSTR name = 0) {};
	virtual	void					Screenshot(ScreenshotMode mode, CMemoryWriter& memory_writer) {};
	virtual void					ScreenshotAsyncBegin() {};
	virtual void					ScreenshotAsyncEnd(CMemoryWriter& memory_writer) {};

	// Render mode
	virtual void					rmNear() {};
	virtual void					rmFar() {};
	virtual void					rmNormal() {};
	virtual u32						memory_usage() { return 0; };

	// Constructor/destructor
	CRender();
	virtual ~CRender();
protected:
	virtual	void					ScreenshotImpl(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer) { };
};

extern CRender						RImplementation;
