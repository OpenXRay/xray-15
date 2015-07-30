#pragma once

#include "../xrRender/PSLibrary.h"

#include "r2_types.h"

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
	CPSLibrary						PSLibrary;
public:
	// feature level
	virtual	GenerationLevel			get_generation() { return IRender_interface::GENERATION_R2; };

	virtual bool					is_sun_static() { return o.sunstatic; };
	virtual DWORD					get_dx_level() { return 0x00000000; };

	// Loading / Unloading
	virtual	void					create();
	virtual	void					destroy() { VERIFY(!"CRender::destroy not implemented."); };
	virtual	void					reset_begin() { VERIFY(!"CRender::reset_begin not implemented."); };
	virtual	void					reset_end() { VERIFY(!"CRender::reset_end not implemented."); };

	virtual	void					level_Load(IReader*) { VERIFY(!"CRender::level_Load not implemented."); };
	virtual void					level_Unload() { VERIFY(!"CRender::level_Unload not implemented."); };

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
		void*							ppConstantTable) { VERIFY(!"CRender::shader_compile not implemented."); return E_NOTIMPL; };

	// Information
	virtual	void					Statistics(CGameFont* F)							{ VERIFY(!"CRender::Statistics not implemented."); };

	virtual LPCSTR					getShaderPath() { VERIFY(!"CRender::getShaderPath not implemented."); return ""; };
	virtual IRender_Sector*			getSector(int id) { VERIFY(!"CRender::getSector not implemented."); return nullptr; };
	virtual IRenderVisual*			getVisual(int id) { VERIFY(!"CRender::getVisual not implemented."); return nullptr; };
	virtual IRender_Sector*			detectSector(const Fvector& P) { VERIFY(!"CRender::detectSector not implemented."); return nullptr; };
	virtual IRender_Target*			getTarget() { VERIFY(!"CRender::getTarget not implemented."); return nullptr; };

	// Main 
	IC		void					set_Frustum(CFrustum*	O)							{ VERIFY(!"CRender::set_Frustum not implemented."); VERIFY(O);	View = O; }
	virtual void					set_Transform(Fmatrix*	M) { VERIFY(!"CRender::set_Transform not implemented."); };
	virtual void					set_HUD(BOOL 		V) { VERIFY(!"CRender::set_HUD not implemented."); };
	virtual BOOL					get_HUD() { VERIFY(!"CRender::get_HUD not implemented."); return false; };
	virtual void					set_Invisible(BOOL 		V) { VERIFY(!"CRender::set_Invisible not implemented."); };
	virtual void					flush() { VERIFY(!"CRender::flush not implemented."); };
	virtual void					set_Object(IRenderable*		O) { VERIFY(!"CRender::set_Object not implemented."); };
	virtual	void					add_Occluder(Fbox2&	bb_screenspace) { VERIFY(!"CRender::add_Occluder not implemented."); };	// mask screen region as oclluded (-1..1, -1..1)
	virtual void					add_Visual(IRenderVisual*	V) { VERIFY(!"CRender::add_Visual not implemented."); };	// add visual leaf	(no culling performed at all)
	virtual void					add_Geometry(IRenderVisual*	V) { VERIFY(!"CRender::add_Geometry not implemented."); };	// add visual(s)	(all culling performed)
	virtual void					add_StaticWallmark(const wm_shader& S, const Fvector& P, float s, CDB::TRI* T, Fvector* V) { VERIFY(!"CRender::add_StaticWallmark not implemented."); };
	virtual void					add_StaticWallmark(IWallMarkArray *pArray, const Fvector& P, float s, CDB::TRI* T, Fvector* V) { VERIFY(!"CRender::add_StaticWallmark not implemented."); };
	virtual void					clear_static_wallmarks() { VERIFY(!"CRender::clear_static_wallmarks not implemented."); };
	virtual void					add_SkeletonWallmark(const Fmatrix* xf, IKinematics* obj, IWallMarkArray *pArray, const Fvector& start, const Fvector& dir, float size) { VERIFY(!"CRender::add_SkeletonWallmark not implemented."); };

	//
	virtual IBlender*				blender_create(CLASS_ID cls);
	virtual void					blender_destroy(IBlender* &);

	virtual IRender_ObjectSpecific*	ros_create(IRenderable* parent) { VERIFY(!"CRender::ros_create not implemented."); return nullptr; };
	virtual void					ros_destroy(IRender_ObjectSpecific* &) { VERIFY(!"CRender::ros_destroy not implemented."); };

	// Lighting/glowing
	virtual IRender_Light*			light_create()						{ VERIFY(!"CRender::light_create not implemented."); return nullptr; };
	virtual void					light_destroy(IRender_Light* p_)	{ VERIFY(!"CRender::light_destroy not implemented."); };
	virtual IRender_Glow*			glow_create()						{ VERIFY(!"CRender::glow_create not implemented."); return nullptr; };
	virtual void					glow_destroy(IRender_Glow* p_)		{ VERIFY(!"CRender::glow_destroy not implemented."); };

	// Models
	virtual IRenderVisual*			model_CreateParticles(LPCSTR name) { VERIFY(!"CRender::model_CreateParticles not implemented."); return nullptr; };
	virtual IRenderVisual*			model_Create(LPCSTR name, IReader*	data = 0) { VERIFY(!"CRender::model_Create not implemented."); return nullptr; };
	virtual IRenderVisual*			model_CreateChild(LPCSTR name, IReader*	data) { VERIFY(!"CRender::model_CreateChild not implemented."); return nullptr; };
	virtual IRenderVisual*			model_Duplicate(IRenderVisual*	V) { VERIFY(!"CRender::model_Duplicate not implemented."); return nullptr; };
	virtual void					model_Delete(IRenderVisual* &	V, BOOL bDiscard = FALSE) { VERIFY(!"CRender::model_Delete not implemented."); };
	virtual void					model_Logging(BOOL bEnable) { VERIFY(!"CRender::model_Logging not implemented."); };
	virtual void					models_Prefetch() { VERIFY(!"CRender::models_Prefetch not implemented."); };
	virtual void					models_Clear(BOOL b_complete) { VERIFY(!"CRender::models_Clear not implemented."); };
	IRenderVisual*					model_CreatePE(LPCSTR name) { VERIFY(!"CRender::model_CreatePE not implemented."); return nullptr; };

	// Occlusion culling
	virtual BOOL					occ_visible(vis_data&	V) { VERIFY(!"CRender::occ_visible not implemented."); return false; };
	virtual BOOL					occ_visible(Fbox&		B) { VERIFY(!"CRender::occ_visible not implemented."); return false; };
	virtual BOOL					occ_visible(sPoly&		P) { VERIFY(!"CRender::occ_visible not implemented."); return false; };

	// Main
	virtual void					Calculate() {};
	virtual void					Render() {};

	virtual void					Screenshot(ScreenshotMode mode = SM_NORMAL, LPCSTR name = 0) { VERIFY(!"CRender::Screenshot not implemented."); };
	virtual	void					Screenshot(ScreenshotMode mode, CMemoryWriter& memory_writer) { VERIFY(!"CRender::Screenshot not implemented."); };
	virtual void					ScreenshotAsyncBegin() { VERIFY(!"CRender::ScreenshotAsyncBegin not implemented."); };
	virtual void					ScreenshotAsyncEnd(CMemoryWriter& memory_writer) { VERIFY(!"CRender::ScreenshotAsyncEnd not implemented."); };

	// Render mode
	virtual void					rmNear() { VERIFY(!"CRender::rmNear not implemented."); };
	virtual void					rmFar() { VERIFY(!"CRender::rmFar not implemented."); };
	virtual void					rmNormal() { VERIFY(!"CRender::rmNormal not implemented."); };
	virtual u32						memory_usage() { VERIFY(!"CRender::memory_usage not implemented."); return 0; };

	// Constructor/destructor
	CRender();
	virtual ~CRender();
protected:
	virtual	void					ScreenshotImpl(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer) { VERIFY(!"CRender::ScreenshotImpl not implemented."); };
};

extern CRender						RImplementation;
