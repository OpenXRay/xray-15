// FVisual.cpp: implementation of the FVisual class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include "../../xrEngine/fmesh.h"
#include "fvisual.h"

#include "../xrRenderDX10/dx10BufferUtils.h"
#include "../xrRenderGL/glBufferUtils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Fvisual::Fvisual()  : dxRender_Visual()
{
	m_fast	=	0;
}

Fvisual::~Fvisual()
{
	xr_delete	(m_fast);
}

void Fvisual::Release	()
{
	dxRender_Visual::Release	();
}

void Fvisual::Load		(const char* N, IReader *data, u32 dwFlags)
{
	dxRender_Visual::Load		(N,data,dwFlags);

	u32					fvf		= 0;
	D3DVERTEXELEMENT9*	vFormat	= 0;
	dwPrimitives				= 0;
	BOOL				loaded_v=false;

	if (data->find_chunk(OGF_GCONTAINER)) {
#ifndef _EDITOR
		// verts
		u32 ID				= data->r_u32					();
		vBase				= data->r_u32					();
		vCount				= data->r_u32					();

		VERIFY				(NULL==p_rm_Vertices);

		p_rm_Vertices		= RImplementation.getVB			(ID);
#ifndef USE_OGL
		p_rm_Vertices->AddRef();
#endif // !USE_OGL
		vFormat				= RImplementation.getVB_Format	(ID);
		loaded_v			= true;

		// indices
		ID					= data->r_u32				();
		iBase				= data->r_u32				();
		iCount				= data->r_u32				();
		dwPrimitives		= iCount/3;

		VERIFY				(NULL==p_rm_Indices);
		p_rm_Indices		= RImplementation.getIB		(ID);
#ifndef USE_OGL
		p_rm_Indices->AddRef();
#endif // !USE_OGL
#endif
#if (RENDER==R_R2) || (RENDER==R_R3) || (RENDER==R_GL)
		// check for fast-vertices
		if (data->find_chunk(OGF_FASTPATH))		{
			destructor<IReader>	geomdef	(data->open_chunk		(OGF_FASTPATH));
			destructor<IReader>	def		(geomdef().open_chunk	(OGF_GCONTAINER));

			// we have fast-mesh
			m_fast						= new IRender_Mesh();

			// verts
			D3DVERTEXELEMENT9*	fmt		= 0;
			ID							= def().r_u32			();
			m_fast->vBase				= def().r_u32			();
			m_fast->vCount				= def().r_u32			();

			VERIFY						(NULL==m_fast->p_rm_Vertices);
			m_fast->p_rm_Vertices		= RImplementation.getVB	(ID,true);
#ifndef USE_OGL
			m_fast->p_rm_Vertices->AddRef();
#endif // !USE_OGL
			fmt							= RImplementation.getVB_Format(ID,true);

			// indices
			ID							= def().r_u32			();
			m_fast->iBase				= def().r_u32			();
			m_fast->iCount				= def().r_u32			();
			m_fast->dwPrimitives			= iCount/3;
		
			VERIFY						(NULL==m_fast->p_rm_Indices);
			m_fast->p_rm_Indices			= RImplementation.getIB	(ID,true);
#ifndef USE_OGL
			m_fast->p_rm_Indices->AddRef();
#endif // !USE_OGL

			// geom
			m_fast->rm_geom.create			(fmt,m_fast->p_rm_Vertices,m_fast->p_rm_Indices);
		}
#endif
	}

	// read vertices
	if (!loaded_v && (dwFlags&VLOAD_NOVERTICES)==0) {
		if (data->find_chunk(OGF_VCONTAINER)) {
#ifndef _EDITOR
			u32 ID				= data->r_u32				();
			vBase				= data->r_u32				();
			vCount				= data->r_u32				();
			VERIFY				(NULL==p_rm_Vertices);
			p_rm_Vertices		= RImplementation.getVB			(ID);
#ifndef USE_OGL
			p_rm_Vertices->AddRef();
#endif // !USE_OGL
			vFormat				= RImplementation.getVB_Format	(ID);
#endif
		} else {
			R_ASSERT			(data->find_chunk(OGF_VERTICES));
			vBase				= 0;
			fvf					= data->r_u32				();
#ifdef USE_OGL
			u32 vStride			= glBufferUtils::GetFVFVertexSize		(fvf);
#else
			u32 vStride			= D3DXGetFVFVertexSize		(fvf);
#endif // USE_OGL
			vCount				= data->r_u32				();

#if defined(USE_OGL)
			VERIFY				(NULL==p_rm_Vertices);
								glBufferUtils::CreateVertexBuffer(&p_rm_Vertices, data->pointer(), vCount*vStride);
#elif defined(USE_DX10) // USE_OGL
			VERIFY				(NULL==p_rm_Vertices);
			R_CHK				(dx10BufferUtils::CreateVertexBuffer(&p_rm_Vertices, data->pointer(), vCount*vStride));
#else	//	USE_DX10
			BOOL	bSoft		= HW.Caps.geometry.bSoftware || (dwFlags&VLOAD_FORCESOFTWARE);
			u32		dwUsage		= D3DUSAGE_WRITEONLY | (bSoft?D3DUSAGE_SOFTWAREPROCESSING:0);
			BYTE*	bytes		= 0;
			VERIFY				(NULL==p_rm_Vertices);
			R_CHK				(HW.pDevice->CreateVertexBuffer(vCount*vStride,dwUsage,0,D3DPOOL_MANAGED,&p_rm_Vertices,0));
			R_CHK				(p_rm_Vertices->Lock(0,0,(void**)&bytes,0));
			CopyMemory			(bytes, data->pointer(), vCount*vStride);
			p_rm_Vertices->Unlock	();
#endif
		}
	}

	// indices
	if (!loaded_v && (dwFlags&VLOAD_NOINDICES)==0) {
		dwPrimitives = 0;
		if (data->find_chunk(OGF_ICONTAINER)) {
#ifndef _EDITOR
			u32 ID				= data->r_u32			();
			iBase				= data->r_u32			();
			iCount				= data->r_u32			();
			dwPrimitives		= iCount/3;
			VERIFY				(NULL==p_rm_Indices);
			p_rm_Indices		= RImplementation.getIB	(ID);
#ifndef USE_OGL
			p_rm_Indices->AddRef	();
#endif // !USE_OGL
#endif
		} else {
			R_ASSERT			(data->find_chunk(OGF_INDICES));
			iBase				= 0;
			iCount				= data->r_u32();
			dwPrimitives		= iCount/3;

#if defined(USE_OGL)
			VERIFY				(NULL==p_rm_Indices);
			glBufferUtils::CreateIndexBuffer(&p_rm_Indices, data->pointer(), iCount*2);
#elif defined(USE_DX10) // USE_OGL
			//BOOL	bSoft		= HW.Caps.geometry.bSoftware || (dwFlags&VLOAD_FORCESOFTWARE);
			//u32		dwUsage		= /*D3DUSAGE_WRITEONLY |*/ (bSoft?D3DUSAGE_SOFTWAREPROCESSING:0);	// indices are read in model-wallmarks code
			//BYTE*	bytes		= 0;

			//VERIFY				(NULL==p_rm_Indices);
			//R_CHK				(HW.pDevice->CreateIndexBuffer(iCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&p_rm_Indices,0));
			//R_CHK				(p_rm_Indices->Lock(0,0,(void**)&bytes,0));
			//CopyMemory		(bytes, data->pointer(), iCount*2);

			VERIFY				(NULL==p_rm_Indices);
			R_CHK				(dx10BufferUtils::CreateIndexBuffer(&p_rm_Indices, data->pointer(), iCount*2));
#else	//	USE_DX10
			BOOL	bSoft		= HW.Caps.geometry.bSoftware || (dwFlags&VLOAD_FORCESOFTWARE);
			u32		dwUsage		= /*D3DUSAGE_WRITEONLY |*/ (bSoft?D3DUSAGE_SOFTWAREPROCESSING:0);	// indices are read in model-wallmarks code
			BYTE*	bytes		= 0;

			VERIFY				(NULL==p_rm_Indices);
			R_CHK				(HW.pDevice->CreateIndexBuffer(iCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&p_rm_Indices,0));
			R_CHK				(p_rm_Indices->Lock(0,0,(void**)&bytes,0));
			CopyMemory		(bytes, data->pointer(), iCount*2);
			p_rm_Indices->Unlock	();
#endif	//	USE_DX10
		}
	}

	if (dwFlags&VLOAD_NOVERTICES || dwFlags&VLOAD_NOINDICES)	return;
	else if (vFormat)
		rm_geom.create(vFormat, p_rm_Vertices, p_rm_Indices);
	else
		rm_geom.create(fvf, p_rm_Vertices, p_rm_Indices);
}

void Fvisual::Render		(float )
{
#if (RENDER==R_R2) || (RENDER==R_R3) || (RENDER==R_GL)
	if (m_fast && RImplementation.phase==CRender::PHASE_SMAP)
	{
		RCache.set_Geometry		(m_fast->rm_geom);
		RCache.Render			(D3DPT_TRIANGLELIST,m_fast->vBase,0,m_fast->vCount,m_fast->iBase,m_fast->dwPrimitives);
		RCache.stat.r.s_static.add	(m_fast->vCount);
	} else {
		RCache.set_Geometry		(rm_geom);
		RCache.Render			(D3DPT_TRIANGLELIST,vBase,0,vCount,iBase,dwPrimitives);
		RCache.stat.r.s_static.add	(vCount);
	}
#else
	RCache.set_Geometry			(rm_geom);
	RCache.Render				(D3DPT_TRIANGLELIST,vBase,0,vCount,iBase,dwPrimitives);
	RCache.stat.r.s_static.add	(vCount);
#endif
}

#define PCOPY(a)	a = pFrom->a
void	Fvisual::Copy			(dxRender_Visual *pSrc)
{
	dxRender_Visual::Copy		(pSrc);

	Fvisual	*pFrom				= dynamic_cast<Fvisual*> (pSrc);

	PCOPY	(rm_geom);

	PCOPY	(p_rm_Vertices);
#ifndef USE_OGL
	if (p_rm_Vertices) p_rm_Vertices->AddRef();
#endif // !USE_OGL
	PCOPY	(vBase);
	PCOPY	(vCount);

	PCOPY	(p_rm_Indices);
#ifndef USE_OGL
	if (p_rm_Indices) p_rm_Indices->AddRef();
#endif // !USE_OGL
	PCOPY	(iBase);
	PCOPY	(iCount);
	PCOPY	(dwPrimitives);

	PCOPY	(m_fast);
}
