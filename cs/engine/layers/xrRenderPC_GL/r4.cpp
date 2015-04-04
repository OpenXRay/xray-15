#include "stdafx.h"
#include "r4.h"

CRender										RImplementation;

extern ENGINE_API BOOL r2_sun_static;
extern ENGINE_API BOOL r2_advanced_pp;	//	advanced post process and effects

void					CRender::create()
{
	//Device.seqFrame.Add(this, REG_PRIORITY_HIGH + 0x12345678);

	m_skinning = -1;
	m_MSAASample = -1;

	// hardware
	o.smapsize = 2048;
	o.mrt = TRUE;
	o.mrtmixdepth = TRUE;

	// Check for NULL render target support
	o.nullrt = false;

	// SMAP / DST
	o.HW_smap_FETCH4 = FALSE;
	o.HW_smap = true;
	o.HW_smap_PCF = o.HW_smap;
	if (o.HW_smap)
	{
		o.HW_smap_FORMAT = GL_DEPTH_COMPONENT24;
		Msg("* HWDST/PCF supported and used");
	}

	o.fp16_filter = true;
	o.fp16_blend = true;

	if (o.mrtmixdepth)		o.albedo_wo = FALSE;
	else if (o.fp16_blend)	o.albedo_wo = FALSE;
	else					o.albedo_wo = TRUE;

	// nvstencil on NV40 and up
	o.nvstencil = FALSE;
	if (strstr(Core.Params, "-nonvs"))		o.nvstencil = FALSE;

	// nv-dbt
	o.nvdbt = glewIsSupported("GL_EXT_depth_bounds_test");
	if (o.nvdbt)		Msg("* NV-DBT supported and used");

	// options (smap-pool-size)
	if (strstr(Core.Params, "-smap1536"))	o.smapsize = 1536;
	if (strstr(Core.Params, "-smap2048"))	o.smapsize = 2048;
	if (strstr(Core.Params, "-smap2560"))	o.smapsize = 2560;
	if (strstr(Core.Params, "-smap3072"))	o.smapsize = 3072;
	if (strstr(Core.Params, "-smap4096"))	o.smapsize = 4096;

	// gloss
	char*	g = strstr(Core.Params, "-gloss ");
	o.forcegloss = g ? TRUE : FALSE;
	if (g)				{
		o.forcegloss_v = float(atoi(g + xr_strlen("-gloss "))) / 255.f;
	}

	// options
	o.bug = (strstr(Core.Params, "-bug")) ? TRUE : FALSE;
	o.sunfilter = (strstr(Core.Params, "-sunfilter")) ? TRUE : FALSE;
	//.	o.sunstatic			= (strstr(Core.Params,"-sunstatic"))?	TRUE	:FALSE	;
	o.sunstatic = r2_sun_static;
	o.advancedpp = r2_advanced_pp;
	//o.volumetricfog = ps_r2_ls_flags.test(R3FLAG_VOLUMETRIC_SMOKE);
	o.sjitter = (strstr(Core.Params, "-sjitter")) ? TRUE : FALSE;
	o.depth16 = (strstr(Core.Params, "-depth16")) ? TRUE : FALSE;
	o.noshadows = (strstr(Core.Params, "-noshadows")) ? TRUE : FALSE;
	o.Tshadows = (strstr(Core.Params, "-tsh")) ? TRUE : FALSE;
	o.mblur = (strstr(Core.Params, "-mblur")) ? TRUE : FALSE;
	o.distortion_enabled = (strstr(Core.Params, "-nodistort")) ? FALSE : TRUE;
	o.distortion = o.distortion_enabled;
	o.disasm = (strstr(Core.Params, "-disasm")) ? TRUE : FALSE;
	o.forceskinw = (strstr(Core.Params, "-skinw")) ? TRUE : FALSE;

	o.ssao_blur_on = ps_r2_ls_flags_ext.test(R2FLAGEXT_SSAO_BLUR) && (ps_r_ssao != 0);
	o.ssao_opt_data = ps_r2_ls_flags_ext.test(R2FLAGEXT_SSAO_OPT_DATA) && (ps_r_ssao != 0);
	o.ssao_half_data = ps_r2_ls_flags_ext.test(R2FLAGEXT_SSAO_HALF_DATA) && o.ssao_opt_data && (ps_r_ssao != 0);
	o.ssao_hdao = ps_r2_ls_flags_ext.test(R2FLAGEXT_SSAO_HDAO) && (ps_r_ssao != 0);
	o.ssao_hbao = !o.ssao_hdao && ps_r2_ls_flags_ext.test(R2FLAGEXT_SSAO_HBAO) && (ps_r_ssao != 0);

	xrRender_apply_tf();
	//::PortalTraverser.initialize();
}

CRender::CRender()
{
}

CRender::~CRender()
{
}
