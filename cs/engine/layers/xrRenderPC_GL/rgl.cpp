#include "stdafx.h"
#include "rgl.h"

CRender										RImplementation;

extern ENGINE_API BOOL r2_sun_static;
extern ENGINE_API BOOL r2_advanced_pp;	//	advanced post process and effects


CRender::CRender()
{
}

CRender::~CRender()
{
}

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

char* CRender::LoadIncludes(LPCSTR pSrcData, UINT SrcDataLen, xr_vector<char*>& includes)
{
	char* srcData = xr_alloc<char>(SrcDataLen);
	memcpy(srcData, pSrcData, SrcDataLen);
	srcData[SrcDataLen] = '\0';

	string_path fn, path;
	char* str = srcData;
	while (strstr(str, "#include") != nullptr)
	{
		// get filename
		str = strstr(str, "#include");
		str[0] = str[1] = '/';
		str = strchr(str, '"') + 1; // Skip quotation
		size_t len = strchr(str, '"') - str;
		strncpy(fn, str, len);
		fn[len] = '\0';

		// create path
		strconcat(sizeof(path), path, ::Render->getShaderPath(), fn);
		FS.update_path(path, "$game_shaders$", path);
		while (char* sep = strchr(path, '/')) *sep = '\\';

		// open and read file
		IReader* R = FS.r_open(path);
		R_ASSERT2(R, path);
		includes.push_back(LoadIncludes((char*)R->pointer(), R->length(), includes));
		FS.r_close(R);
	}

	// remove include directives from source data
	return srcData;
}

struct SHADER_MACRO {
	char *Define = "#define ", *Name = "\n", *Definition = "\n", *EOL = "\n";
};

HRESULT	CRender::shader_compile(
	LPCSTR							name,
	LPCSTR                          pSrcData,
	UINT                            SrcDataLen,
	void*							_pDefines,
	void*							_pInclude,
	LPCSTR                          pFunctionName,
	LPCSTR                          pTarget,
	DWORD                           Flags,
	void*							_ppShader,
	void*							_ppErrorMsgs,
	void*							_ppConstantTable)
{
	xr_vector<char*>				includes;
	SHADER_MACRO					defines[128];
	int								def_it = 0;
	char							c_smapsize[32];
	char							c_gloss[32];
	char							c_sun_shafts[32];
	char							c_ssao[32];
	char							c_sun_quality[32];
	const char*						srcData;

	// TODO: OGL: Implement these parameters.
	VERIFY(!_pDefines);
	VERIFY(!_pInclude);
	VERIFY(!pFunctionName);
	VERIFY(!pTarget);
	VERIFY(!Flags);
	VERIFY(!_ppConstantTable);

	// open included files
	srcData = LoadIncludes(pSrcData, SrcDataLen, includes);

	// options
	{
		sprintf(c_smapsize, "%d", u32(o.smapsize));
		defines[def_it].Name = "SMAP_size";
		defines[def_it].Definition = c_smapsize;
		def_it++;
	}
	if (o.fp16_filter)		{
		defines[def_it].Name = "FP16_FILTER";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.fp16_blend)		{
		defines[def_it].Name = "FP16_BLEND";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.HW_smap)			{
		defines[def_it].Name = "USE_HWSMAP";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.HW_smap_PCF)			{
		defines[def_it].Name = "USE_HWSMAP_PCF";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.HW_smap_FETCH4)			{
		defines[def_it].Name = "USE_FETCH4";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.sjitter)			{
		defines[def_it].Name = "USE_SJITTER";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.Tshadows)			{
		defines[def_it].Name = "USE_TSHADOWS";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.mblur)			{
		defines[def_it].Name = "USE_MBLUR";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.sunfilter)		{
		defines[def_it].Name = "USE_SUNFILTER";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.sunstatic)		{
		defines[def_it].Name = "USE_R2_STATIC_SUN";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (o.forcegloss)		{
		sprintf(c_gloss, "%f", o.forcegloss_v);
		defines[def_it].Name = "FORCE_GLOSS";
		defines[def_it].Definition = c_gloss;
		def_it++;
	}
	if (o.forceskinw)		{
		defines[def_it].Name = "SKIN_COLOR";
		defines[def_it].Definition = "1";
		def_it++;
	}

	if (o.ssao_blur_on)
	{
		defines[def_it].Name = "USE_SSAO_BLUR";
		defines[def_it].Definition = "1";
		def_it++;
	}

	if (o.ssao_opt_data)
	{
		defines[def_it].Name = "SSAO_OPT_DATA";
		if (o.ssao_half_data)
			defines[def_it].Definition = "2";
		else
			defines[def_it].Definition = "1";
		def_it++;
	}

	if (o.ssao_hdao)
	{
		defines[def_it].Name = "HDAO";
		defines[def_it].Definition = "1";
		def_it++;
	}

	if (o.ssao_hbao)
	{
		defines[def_it].Name = "USE_HBAO";
		defines[def_it].Definition = "1";
		def_it++;
	}

	// skinning
	if (m_skinning<0)		{
		defines[def_it].Name = "SKIN_NONE";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (0 == m_skinning)		{
		defines[def_it].Name = "SKIN_0";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (1 == m_skinning)		{
		defines[def_it].Name = "SKIN_1";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (2 == m_skinning)		{
		defines[def_it].Name = "SKIN_2";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (3 == m_skinning)		{
		defines[def_it].Name = "SKIN_3";
		defines[def_it].Definition = "1";
		def_it++;
	}
	if (4 == m_skinning)		{
		defines[def_it].Name = "SKIN_4";
		defines[def_it].Definition = "1";
		def_it++;
	}

	//	Igor: need restart options
	if (RImplementation.o.advancedpp && ps_r2_ls_flags.test(R2FLAG_SOFT_WATER))
	{
		defines[def_it].Name = "USE_SOFT_WATER";
		defines[def_it].Definition = "1";
		def_it++;
	}

	if (RImplementation.o.advancedpp && ps_r2_ls_flags.test(R2FLAG_SOFT_PARTICLES))
	{
		defines[def_it].Name = "USE_SOFT_PARTICLES";
		defines[def_it].Definition = "1";
		def_it++;
	}

	if (RImplementation.o.advancedpp && ps_r2_ls_flags.test(R2FLAG_DOF))
	{
		defines[def_it].Name = "USE_DOF";
		defines[def_it].Definition = "1";
		def_it++;
	}

	if (RImplementation.o.advancedpp && ps_r_sun_shafts)
	{
		sprintf_s(c_sun_shafts, "%d", ps_r_sun_shafts);
		defines[def_it].Name = "SUN_SHAFTS_QUALITY";
		defines[def_it].Definition = c_sun_shafts;
		def_it++;
	}

	if (RImplementation.o.advancedpp && ps_r_ssao)
	{
		sprintf_s(c_ssao, "%d", ps_r_ssao);
		defines[def_it].Name = "SSAO_QUALITY";
		defines[def_it].Definition = c_ssao;
		def_it++;
	}

	if (RImplementation.o.advancedpp && ps_r_sun_quality)
	{
		sprintf_s(c_sun_quality, "%d", ps_r_sun_quality);
		defines[def_it].Name = "SUN_QUALITY";
		defines[def_it].Definition = c_sun_quality;
		def_it++;
	}

	if (RImplementation.o.advancedpp && ps_r2_ls_flags.test(R2FLAG_STEEP_PARALLAX))
	{
		defines[def_it].Name = "ALLOW_STEEPPARALLAX";
		defines[def_it].Definition = "1";
		def_it++;
	}

	// Compile sources list
	size_t def_len = def_it * 4;
	size_t sources_len = includes.size() + def_len + 2;
	const char** sources = xr_alloc<const char*>(sources_len);
	sources[0] = "#version 330\n";
	memcpy(sources + 1, includes.data(), includes.size() * sizeof(char*));
	memcpy(sources + includes.size() + 1, defines, def_len * sizeof(char*));
	sources[sources_len - 1] = srcData;

	// Compile the shader
	GLuint _shader = *(GLuint*)_ppShader;
	R_ASSERT(_shader);
	CHK_GL(glShaderSource(_shader, sources_len, sources, nullptr));
	CHK_GL(glCompileShader(_shader));

	// Free string resources
	xr_free(sources);
	xr_free(srcData);
	for (xr_vector<char*>::iterator it = includes.begin(); it != includes.end(); it++)
		xr_free(*it);

	// Get the compilation result
	GLint _result;
	glGetShaderiv(_shader, GL_COMPILE_STATUS, &_result);

	// Get the compilation log, if requested
	if (_ppErrorMsgs)
	{
		GLint _length;
		GLchar** _pErrorMsgs = (GLchar**)_ppErrorMsgs;
		glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &_length);
		*_pErrorMsgs = xr_alloc<GLchar>(_length);
		glGetShaderInfoLog(_shader, _length, nullptr, *_pErrorMsgs);
	}

	return		_result;
}

IRender_DetailModel*	CRender::model_CreateDM(IReader*	F)
{
	CDetail*	D = new CDetail();
	D->Load(F);
	return D;
}
