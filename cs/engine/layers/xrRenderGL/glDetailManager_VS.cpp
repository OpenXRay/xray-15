#include "stdafx.h"
#include "../xrRender/DetailManager.h"

#include "../../xrEngine/igame_persistent.h"
#include "../../xrEngine/environment.h"

#include "../xrRenderDX10/dx10BufferUtils.h"

const int			quant	= 16384;
const int			c_hdr	= 10;
const int			c_size	= 4;
const int			c_registers = 256;

#pragma pack(push,1)
struct	vertHW
{
	float		x,y,z;
	short		u,v,t,mid;
};
#pragma pack(pop)

short QC (float v)
{
	int t=iFloor(v*float(quant)); clamp(t,-32768,32767);
	return short(t&0xffff);
}

void CDetailManager::hw_Load()
{
	hw_Load_Geom();
	hw_Load_Shaders();
}

void CDetailManager::hw_Load_Geom()
{
	// Analyze batch-size
	hw_BatchSize = (u32(c_registers - c_hdr) / c_size);
	clamp(hw_BatchSize, (u32)0, (u32)64);
	Msg("* [DETAILS] VertexConsts(%d), Batch(%d)", u32(c_registers), hw_BatchSize);

	// Pre-process objects
	u32			dwVerts = 0;
	u32			dwIndices = 0;
	for (u32 o = 0; o<objects.size(); o++)
	{
		const CDetail& D = *objects[o];
		dwVerts += D.number_vertices*hw_BatchSize;
		dwIndices += D.number_indices*hw_BatchSize;
	}
	u32			vSize = sizeof(vertHW);
	Msg("* [DETAILS] %d v(%d), %d p", dwVerts, vSize, dwIndices / 3);

	// Determine POOL & USAGE
	u32 dwUsage = GL_WRITE_ONLY;

	// Create VB/IB
	glGenBuffers(1, &hw_VB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hw_VB);
	CHK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, dwVerts*vSize, NULL, dwUsage));
	glGenBuffers(1, &hw_IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hw_IB);
	CHK_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, dwIndices * 2, NULL, dwUsage));
	Msg("* [DETAILS] Batch(%d), VB(%dK), IB(%dK)", hw_BatchSize, (dwVerts*vSize) / 1024, (dwIndices * 2) / 1024);

	// Fill VB
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hw_VB);
		vertHW* pV = (vertHW*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, dwUsage);

		for (o = 0; o<objects.size(); o++)
		{
			const CDetail& D = *objects[o];
			for (u32 batch = 0; batch<hw_BatchSize; batch++)
			{
				u32 mid = batch*c_size;
				for (u32 v = 0; v<D.number_vertices; v++)
				{
					const Fvector&	vP = D.vertices[v].P;
					pV->x = vP.x;
					pV->y = vP.y;
					pV->z = vP.z;
					pV->u = QC(D.vertices[v].u);
					pV->v = QC(D.vertices[v].v);
					pV->t = QC(vP.y / (D.bv_bb.max.y - D.bv_bb.min.y));
					pV->mid = short(mid);
					pV++;
				}
			}
		}

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// Fill IB
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hw_IB);
		u16* pI = (u16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, dwUsage);

		for (o = 0; o<objects.size(); o++)
		{
			const CDetail& D = *objects[o];
			u16		offset = 0;
			for (u32 batch = 0; batch<hw_BatchSize; batch++)
			{
				for (u32 i = 0; i<u32(D.number_indices); i++)
					*pI++ = u16(u16(D.indices[i]) + u16(offset));
				offset = u16(offset + u16(D.number_vertices));
			}
		}

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// Declare geometry
	hw_Geom.create(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE4(0), hw_VB, hw_IB);
}

void CDetailManager::hw_Unload()
{
	// Destroy VS/VB/IB
	hw_Geom.destroy();
	glDeleteBuffers(1, &hw_VB);
	glDeleteBuffers(1, &hw_IB);
}

void CDetailManager::hw_Load_Shaders()
{
	// Create shader to access constant storage
	ref_shader		S;	S.create("details\\set");
	R_constant_table&	T0	= *(S->E[0]->passes[0]->constants);
	R_constant_table&	T1	= *(S->E[1]->passes[0]->constants);
	hwc_consts			= T0.get("consts");
	hwc_wave			= T0.get("wave");
	hwc_wind			= T0.get("dir2D");
	hwc_array			= T0.get("array");
	hwc_s_consts		= T1.get("consts");
	hwc_s_xform			= T1.get("xform");
	hwc_s_array			= T1.get("array");
}

void CDetailManager::hw_Render()
{
	// Render-prepare
	//	Update timer
	//	Can't use Device.fTimeDelta since it is smoothed! Don't know why, but smoothed value looks more choppy!
	float fDelta = Device.fTimeGlobal-m_global_time_old;
	if ( (fDelta<0) || (fDelta>1))	fDelta = 0.03;
	m_global_time_old = Device.fTimeGlobal;

	m_time_rot_1	+= (PI_MUL_2*fDelta/swing_current.rot1);
	m_time_rot_2	+= (PI_MUL_2*fDelta/swing_current.rot2);
	m_time_pos		+= fDelta*swing_current.speed;

	//float		tm_rot1		= (PI_MUL_2*Device.fTimeGlobal/swing_current.rot1);
	//float		tm_rot2		= (PI_MUL_2*Device.fTimeGlobal/swing_current.rot2);
	float		tm_rot1		= m_time_rot_1;
	float		tm_rot2		= m_time_rot_2;

	Fvector4	dir1,dir2;
	dir1.set				(_sin(tm_rot1),0,_cos(tm_rot1),0).normalize().mul(swing_current.amp1);
	dir2.set				(_sin(tm_rot2),0,_cos(tm_rot2),0).normalize().mul(swing_current.amp2);

	// Setup geometry and DMA
	RCache.set_Geometry		(hw_Geom);

	// Wave0
	float		scale			=	1.f/float(quant);
	Fvector4	wave;
	Fvector4	consts;
	consts.set				(scale,		scale,		ps_r__Detail_l_aniso,	ps_r__Detail_l_ambient);
	//wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	Device.fTimeGlobal*swing_current.speed);
	wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	m_time_pos);
	//RCache.set_c			(&*hwc_consts,	scale,		scale,		ps_r__Detail_l_aniso,	ps_r__Detail_l_ambient);				// consts
	//RCache.set_c			(&*hwc_wave,	wave.div(PI_MUL_2));	// wave
	//RCache.set_c			(&*hwc_wind,	dir1);																					// wind-dir
	//hw_Render_dump			(&*hwc_array,	1, 0, c_hdr );
	hw_Render_dump(consts, wave.div(PI_MUL_2), dir1, 1, 0);

	// Wave1
	//wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	Device.fTimeGlobal*swing_current.speed);
	wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	m_time_pos);
	//RCache.set_c			(&*hwc_wave,	wave.div(PI_MUL_2));	// wave
	//RCache.set_c			(&*hwc_wind,	dir2);																					// wind-dir
	//hw_Render_dump			(&*hwc_array,	2, 0, c_hdr );
	hw_Render_dump(consts, wave.div(PI_MUL_2), dir2, 2, 0);

	// Still
	consts.set				(scale,		scale,		scale,				1.f);
	//RCache.set_c			(&*hwc_s_consts,scale,		scale,		scale,				1.f);
	//RCache.set_c			(&*hwc_s_xform,	Device.mFullTransform);
	//hw_Render_dump			(&*hwc_s_array,	0, 1, c_hdr );
	hw_Render_dump(consts, wave.div(PI_MUL_2), dir2, 0, 1);
}

void CDetailManager::hw_Render_dump(const Fvector4 &consts, const Fvector4 &wave, const Fvector4 &wind, u32 var_id, u32 lod_id)
{
	VERIFY(!"CDetailManager::hw_Render_dump is unimplemented");
}