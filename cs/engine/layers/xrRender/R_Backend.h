#ifndef r_backendH
#define r_backendH
#pragma once

//#define RBackend_PGO

#ifdef	RBackend_PGO
#define PGO(a)	a
#else
#define PGO(a)
#endif

#include "r_DStreams.h"
#include "r_constants_cache.h"
#include "r_backend_xform.h"
#include "r_backend_hemi.h"
#include "r_backend_tree.h"
#include "fvf.h"

const	u32		CULL_CCW			= D3DCULL_CCW;
const	u32		CULL_CW				= D3DCULL_CW;
const	u32		CULL_NONE			= D3DCULL_NONE;

///		detailed statistic
struct	R_statistics_element	{
	u32		verts,dips;
	ICF		void	add			(u32 _verts)	{ verts+=_verts; dips++; }
};
struct	R_statistics			{
	R_statistics_element		s_static		;
	R_statistics_element		s_flora			;
	R_statistics_element		s_flora_lods	;
	R_statistics_element		s_details		;
	R_statistics_element		s_ui			;
	R_statistics_element		s_dynamic		;
	R_statistics_element		s_dynamic_sw	;
	R_statistics_element		s_dynamic_inst	;
	R_statistics_element		s_dynamic_1B	;
	R_statistics_element		s_dynamic_2B	;
	R_statistics_element		s_dynamic_3B	;
	R_statistics_element		s_dynamic_4B	;
};

#pragma warning(push)
#pragma warning(disable:4324)
class  ECORE_API CBackend
{
public:
#ifdef USE_DX10
	enum
	{
		MaxCBuffers = 14
	};
#endif	//	USE_DX10

public:
	// Dynamic geometry streams
	_VertexStream					Vertex;
	_IndexStream					Index;
#ifdef USE_OGL
	GLuint							QuadIB;
	GLuint							old_QuadIB;
#else
	ID3DIndexBuffer*				QuadIB;
	ID3DIndexBuffer*				old_QuadIB;
#endif // USE_OGL
	R_xforms						xforms;
	R_hemi							hemi;
	R_tree							tree;

#ifdef	USE_DX10
	ref_cbuffer						m_aVertexConstants[MaxCBuffers];
	ref_cbuffer						m_aPixelConstants[MaxCBuffers];
	ref_cbuffer						m_aGeometryConstants[MaxCBuffers];
	D3D10_PRIMITIVE_TOPOLOGY		m_PrimitiveTopology;
	ID3D10InputLayout*				m_pInputLayout;
	DWORD							dummy0;	//	Padding to avoid warning	
	DWORD							dummy1;	//	Padding to avoid warning	
	DWORD							dummy2;	//	Padding to avoid warning	
#endif
private:
	// Render-targets
#ifdef USE_OGL
	GLuint							pFB;
	GLuint							pRT[4];
	GLuint							pZB;
#else
	ID3DRenderTargetView*			pRT[4];
	ID3DDepthStencilView*			pZB;
#endif // USE_OGL

	// Vertices/Indices/etc
#if defined(USE_DX10) || defined(USE_OGL)
	SDeclaration*					decl;
#else	//	USE_DX10 || USE_OGL
	IDirect3DVertexDeclaration9*	decl;
#endif	//	USE_DX10 || USE_OGL
#ifdef USE_OGL
	GLuint							vb;
	GLuint							ib;
#else
	ID3DVertexBuffer*				vb;
	ID3DIndexBuffer*				ib;
#endif // USE_OGL
	u32								vb_stride;

	// Pixel/Vertex constants
	ALIGN(16)	R_constants			constants;
	R_constant_table*				ctable;

	// Shaders/State
#ifdef USE_OGL
	GLuint							pp;
	GLuint							ps;
	GLuint							vs;
	SState*							state;
#else
	ID3DState*						state;
	ID3DPixelShader*				ps;
	ID3DVertexShader*				vs;
#ifdef	USE_DX10
	ID3DGeometryShader*				gs;
#endif	//	USE_DX10
#endif // USE_OGL

#ifdef DEBUG
	LPCSTR							ps_name;
	LPCSTR							vs_name;
#ifdef	USE_DX10
	LPCSTR							gs_name;
#endif	//	USE_DX10
#endif
	u32								stencil_enable;
	u32								stencil_func;
	u32								stencil_ref;
	u32								stencil_mask;
	u32								stencil_writemask;
	u32								stencil_fail;
	u32								stencil_pass;
	u32								stencil_zfail;
	u32								colorwrite_mask;
	u32								cull_mode;
	u32								z_enable;
	u32								z_func;
	u32								alpha_ref;

	// Lists
	STextureList*					T;
	SMatrixList*					M;
	SConstantList*					C;

	// Lists-expanded
	CTexture*						textures_ps	[CTexture::mtMaxPixelShaderTextures];	// stages
	//CTexture*						textures_vs	[5	];	// dmap + 4 vs
	CTexture*						textures_vs	[CTexture::mtMaxVertexShaderTextures];	// 4 vs
#ifdef	USE_DX10
	CTexture*						textures_gs	[CTexture::mtMaxGeometryShaderTextures];	// 4 vs
#endif	//	USE_DX10
#ifdef _EDITOR
	CMatrix*						matrices	[8	];	// matrices are supported only for FFP
#endif

	void							Invalidate	();
public:
	struct _stats
	{
		u32								polys;
		u32								verts;
		u32								calls;
		u32								vs;
		u32								ps;
#ifdef	DEBUG
		u32								decl;
		u32								vb;
		u32								ib;
		u32								states;			// Number of times the shader-state changes
		u32								textures;		// Number of times the shader-tex changes
		u32								matrices;		// Number of times the shader-xform changes
		u32								constants;		// Number of times the shader-consts changes
#endif
		u32								xforms;
		u32								target_rt;
		u32								target_zb;

		R_statistics					r	;
	}									stat;
public:
	IC	CTexture*					get_ActiveTexture			(u32 stage)
	{
		if (stage<CTexture::rstVertex)			return textures_ps[stage];
		else if (stage<CTexture::rstGeometry)	return textures_vs[stage-CTexture::rstVertex];
#ifdef	USE_DX10
		else									return textures_gs[stage-CTexture::rstGeometry];
#else	//	USE_DX10
		VERIFY(!"Invalid texture stage");
		return 0;
#endif	//	USE_DX10
	}

#ifdef	USE_DX10
	IC	void						get_ConstantDirect	(shared_str& n, u32 DataSize, void** pVData, void** pGData, void** pPData);
#else	USE_DX10
#ifndef USE_OGL
	IC	R_constant_array&			get_ConstantCache_Vertex	()			{ return constants.a_vertex;	}
	IC	R_constant_array&			get_ConstantCache_Pixel		()			{ return constants.a_pixel;		}
#endif // USE_OGL
#endif	//	USE_DX10

	// API
	IC	void						set_xform			(u32 ID, const Fmatrix& M);
	IC	void						set_xform_world		(const Fmatrix& M);
	IC	void						set_xform_view		(const Fmatrix& M);
	IC	void						set_xform_project	(const Fmatrix& M);
	IC	const Fmatrix&				get_xform_world		();
	IC	const Fmatrix&				get_xform_view		();
	IC	const Fmatrix&				get_xform_project	();

#ifdef USE_OGL
	IC	void						set_FB(GLuint FB = 0);
	IC	void						set_RT(GLuint RT, u32 ID = 0);
	IC	void						set_ZB(GLuint ZB);
	IC	GLuint						get_FB();
	IC	GLuint						get_RT(u32 ID = 0);
	IC	GLuint						get_ZB();
#else
	IC	void						set_RT(ID3DRenderTargetView* RT, u32 ID = 0);
	IC	void						set_ZB(ID3DDepthStencilView* ZB);
	IC	ID3DRenderTargetView*		get_RT(u32 ID = 0);
	IC	ID3DDepthStencilView*		get_ZB();
#endif // USE_OGL

	IC	void						set_Constants		(R_constant_table* C);
	IC	void						set_Constants		(ref_ctable& C)						{ set_Constants(&*C);			}

		void						set_Textures		(STextureList* T);
	IC	void						set_Textures		(ref_texture_list& T)				{ set_Textures(&*T);			}

#ifdef _EDITOR
	IC	void						set_Matrices		(SMatrixList* M);
	IC	void						set_Matrices		(ref_matrix_list& M)				{ set_Matrices(&*M);			}
#endif

	IC	void						set_Element			(ShaderElement* S, u32	pass=0);
	IC	void						set_Element			(ref_selement& S, u32	pass=0)		{ set_Element(&*S,pass);		}

	IC	void						set_Shader			(Shader* S, u32 pass=0);
	IC	void						set_Shader			(ref_shader& S, u32 pass=0)			{ set_Shader(&*S,pass);			}

	ICF	void						set_States			(SState* _state);
	ICF	void						set_States			(ref_state& _state)					{ set_States(&*_state); }

#if defined(USE_DX10) || defined(USE_OGL)
	ICF  void						set_Format			(SDeclaration* _decl);
#else	//	USE_DX10 || USE_OGL
	ICF  void						set_Format			(IDirect3DVertexDeclaration9* _decl);
#endif	//	USE_DX10 || USE_OGL

#ifdef USE_OGL
	ICF void						set_PS				(GLuint _ps, LPCSTR _n=0);
#else
	ICF void						set_PS				(ID3DPixelShader* _ps, LPCSTR _n = 0);
#endif // USE_OGL
	ICF void						set_PS				(ref_ps& _ps)						{ set_PS(_ps->ps,_ps->cName.c_str());				}

#ifdef	USE_DX10
	ICF void						set_GS				(ID3DGeometryShader* _gs, LPCSTR _n=0);
	ICF void						set_GS				(ref_gs& _gs)						{ set_GS(_gs->gs,_gs->cName.c_str());				}
#endif	//	USE_DX10

	ICF void						set_VS				(ref_vs& _vs);
#ifdef	USE_DX10
	ICF void						set_VS				(SVS* _vs);
protected:	//	In DX10 we need input shader signature which is stored in ref_vs
#endif	//	USE_DX10
#ifdef USE_OGL
	ICF void						set_VS				(GLuint _vs, LPCSTR _n=0);
#else
	ICF void						set_VS				(ID3DVertexShader* _vs, LPCSTR _n=0);
#endif // USE_OGL

#ifdef	USE_DX10
public:
#endif	//	USE_DX10

#ifdef USE_OGL
	ICF	void						set_Vertices(GLuint _vb, u32 _vb_stride);
	ICF	void						set_Indices(GLuint _ib);
#else
	ICF	void						set_Vertices(ID3DVertexBuffer* _vb, u32 _vb_stride);
	ICF	void						set_Indices(ID3DIndexBuffer* _ib);
#endif // USE_OGL
	ICF void						set_Geometry		(SGeometry* _geom);
	ICF void						set_Geometry		(ref_geom& _geom)					{	set_Geometry(&*_geom);		}
	IC  void						set_Stencil			(u32 _enable, u32 _func=D3DCMP_ALWAYS, u32 _ref=0x00, u32 _mask=0x00, u32 _writemask=0x00, u32 _fail=D3DSTENCILOP_KEEP, u32 _pass=D3DSTENCILOP_KEEP, u32 _zfail=D3DSTENCILOP_KEEP);
	IC  void						set_Z				(u32 _enable);
	IC  void						set_ZFunc			(u32 _func);
	IC  void						set_AlphaRef		(u32 _value);
	IC  void						set_ColorWriteEnable(u32 _mask = D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
	IC  void						set_CullMode		(u32 _mode);
	IC  u32							get_CullMode		(){return cull_mode;}
	void							set_ClipPlanes		(u32 _enable, Fplane*	_planes=NULL, u32 count=0);
	void							set_ClipPlanes		(u32 _enable, Fmatrix*	_xform =NULL, u32 fmask=0xff);
	IC	void						set_Scissor			(Irect*	rect=NULL);

	// constants
	ICF	ref_constant				get_c				(LPCSTR			n)													{ if (ctable)	return ctable->get(n);else return 0;}
	ICF	ref_constant				get_c				(shared_str&	n)													{ if (ctable)	return ctable->get(n);else return 0;}

	// constants - direct (fast)
	ICF	void						set_c				(R_constant* C, const Fmatrix& A)									{ if (C)		constants.set(C,A);					}
	ICF	void						set_c				(R_constant* C, const Fvector4& A)									{ if (C)		constants.set(C,A);					}
	ICF	void						set_c				(R_constant* C, float x, float y, float z, float w)					{ if (C)		constants.set(C,x,y,z,w);			}
	ICF	void						set_ca				(R_constant* C, u32 e, const Fmatrix& A)							{ if (C)		constants.seta(C,e,A);				}
	ICF	void						set_ca				(R_constant* C, u32 e, const Fvector4& A)							{ if (C)		constants.seta(C,e,A);				}
	ICF	void						set_ca				(R_constant* C, u32 e, float x, float y, float z, float w)			{ if (C)		constants.seta(C,e,x,y,z,w);		}
#if defined(USE_DX10) || defined(USE_OGL)
	ICF	void						set_c				(R_constant* C, float A)											{ if (C)		constants.set(C,A);					}
	ICF	void						set_c				(R_constant* C, int A)												{ if (C)		constants.set(C,A);					}
#endif	//	USE_DX10 || USE_OGL


	// constants - LPCSTR (slow)
	ICF	void						set_c				(LPCSTR n, const Fmatrix& A)										{ if(ctable)	set_c	(&*ctable->get(n),A);		}
	ICF	void						set_c				(LPCSTR n, const Fvector4& A)										{ if(ctable)	set_c	(&*ctable->get(n),A);		}
	ICF	void						set_c				(LPCSTR n, float x, float y, float z, float w)						{ if(ctable)	set_c	(&*ctable->get(n),x,y,z,w);	}
	ICF	void						set_ca				(LPCSTR n, u32 e, const Fmatrix& A)									{ if(ctable)	set_ca	(&*ctable->get(n),e,A);		}
	ICF	void						set_ca				(LPCSTR n, u32 e, const Fvector4& A)								{ if(ctable)	set_ca	(&*ctable->get(n),e,A);		}
	ICF	void						set_ca				(LPCSTR n, u32 e, float x, float y, float z, float w)				{ if(ctable)	set_ca	(&*ctable->get(n),e,x,y,z,w);}
#if defined(USE_DX10) || defined(USE_OGL)
	ICF	void						set_c				(LPCSTR n, float A)													{ if(ctable)	set_c	(&*ctable->get(n),A);		}
	ICF	void						set_c				(LPCSTR n, int A)													{ if(ctable)	set_c	(&*ctable->get(n),A);		}
#endif	//	USE_DX10 || USE_OGL

	// constants - shared_str (average)
	ICF	void						set_c				(shared_str& n, const Fmatrix& A)									{ if(ctable)	set_c	(&*ctable->get(n),A);			}
	ICF	void						set_c				(shared_str& n, const Fvector4& A)									{ if(ctable)	set_c	(&*ctable->get(n),A);			}
	ICF	void						set_c				(shared_str& n, float x, float y, float z, float w)					{ if(ctable)	set_c	(&*ctable->get(n),x,y,z,w);	}
	ICF	void						set_ca				(shared_str& n, u32 e, const Fmatrix& A)							{ if(ctable)	set_ca	(&*ctable->get(n),e,A);		}
	ICF	void						set_ca				(shared_str& n, u32 e, const Fvector4& A)							{ if(ctable)	set_ca	(&*ctable->get(n),e,A);		}
	ICF	void						set_ca				(shared_str& n, u32 e, float x, float y, float z, float w)			{ if(ctable)	set_ca	(&*ctable->get(n),e,x,y,z,w);}
#if defined(USE_DX10) || defined(USE_OGL)
	ICF	void						set_c				(shared_str& n, float A)											{ if(ctable)	set_c	(&*ctable->get(n),A);		}
	ICF	void						set_c				(shared_str& n, int A)												{ if(ctable)	set_c	(&*ctable->get(n),A);		}
#endif	//	USE_DX10 || USE_OGL

	ICF	void						Render				(D3DPRIMITIVETYPE T, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC);
	ICF	void						Render				(D3DPRIMITIVETYPE T, u32 startV, u32 PC);

	// Device create / destroy / frame signaling
	void							RestoreQuadIBData	();	// Igor: is used to test bug with rain, particles corruption
	void							CreateQuadIB		();
	void							OnFrameBegin		();
	void							OnFrameEnd			();
	void							OnDeviceCreate		();
	void							OnDeviceDestroy		();

	// Debug render
	void dbg_DP						(D3DPRIMITIVETYPE pt, ref_geom geom, u32 vBase, u32 pc);
	void dbg_DIP					(D3DPRIMITIVETYPE pt, ref_geom geom, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC);
#ifndef USE_OGL
#ifdef	USE_DX10
	//	TODO: DX10: Implement this.
	IC void	dbg_SetRS				(D3DRENDERSTATETYPE p1, u32 p2)
	{ VERIFY(!"Not implemented"); }
	IC void	dbg_SetSS				(u32 sampler, D3DSAMPLERSTATETYPE type, u32 value)
	{ VERIFY(!"Not implemented"); }
#else	//	USE_DX10
	IC void	dbg_SetRS				(D3DRENDERSTATETYPE p1, u32 p2)
	{ CHK_DX(HW.pDevice->SetRenderState(p1,p2)); }
	IC void	dbg_SetSS				(u32 sampler, D3DSAMPLERSTATETYPE type, u32 value)
	{ CHK_DX(HW.pDevice->SetSamplerState(sampler,type,value)); }
#endif	//	USE_DX10
#endif // !USE_OGL
#ifdef DEBUG

	void dbg_Draw					(D3DPRIMITIVETYPE T, FVF::L* pVerts, int vcnt, u16* pIdx, int pcnt);
	void dbg_Draw					(D3DPRIMITIVETYPE T, FVF::L* pVerts, int pcnt);
	IC void dbg_DrawAABB			(Fvector& T, float sx, float sy, float sz, u32 C)						{	Fvector half_dim;	half_dim.set(sx,sy,sz); Fmatrix	TM;	TM.translate(T); dbg_DrawOBB(TM,half_dim,C);	}
	void dbg_DrawOBB				(Fmatrix& T, Fvector& half_dim, u32 C);
	IC void dbg_DrawTRI				(Fmatrix& T, Fvector* p, u32 C)											{	dbg_DrawTRI(T,p[0],p[1],p[2],C);	}
	void dbg_DrawTRI				(Fmatrix& T, Fvector& p1, Fvector& p2, Fvector& p3, u32 C);
	void dbg_DrawLINE				(Fmatrix& T, Fvector& p1, Fvector& p2, u32 C);
	void dbg_DrawEllipse			(Fmatrix& T, u32 C);
#endif

	CBackend()						{	Invalidate(); };

#ifdef	USE_DX10
private:
	//	DirectX 10 internal functionality
	//void CreateConstantBuffers();
	//void DestroyConstantBuffers();
	void	ApplyVertexLayout();
	void	ApplyRTandZB();
	void	ApplyPrimitieTopology( D3D10_PRIMITIVE_TOPOLOGY Topology );
	bool	CBuffersNeedUpdate(ref_cbuffer	buf1[MaxCBuffers], ref_cbuffer	buf2[MaxCBuffers], u32	&uiMin, u32	&uiMax);

private:
	ID3DBlob*				m_pInputSignature;

	bool					m_bChangedRTorZB;
#endif	//	USE_DX10
};
#pragma warning(pop)

extern  ECORE_API CBackend			RCache;

#ifndef _EDITOR
#ifdef USE_OGL
#	include "GLUtils.h"
#else
#	include "D3DUtils.h"
#endif // USE_OGL
#endif

#endif
