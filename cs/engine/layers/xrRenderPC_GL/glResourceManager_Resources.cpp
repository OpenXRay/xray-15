#include "stdafx.h"
#pragma hdrstop

#ifndef _EDITOR
#include "../../xrEngine/render.h"
#endif

#include "../xrRender/ResourceManager.h"
#include "../xrRender/tss.h"
#include "../xrRender/blenders\blender.h"
#include "../xrRender/blenders\blender_recorder.h"

#include "../xrRenderGL/glBufferUtils.h"

void fix_texture_name(LPSTR fn);

void simplify_texture(string_path &fn)
{
	if (strstr(Core.Params,"-game_designer"))
	{
		if (strstr(fn, "$user")) return;
		if (strstr(fn, "ui\\")) return;
		if (strstr(fn, "lmap#")) return;
		if (strstr(fn, "act\\")) return;
		if (strstr(fn, "fx\\")) return;
		if (strstr(fn, "glow\\")) return;
		if (strstr(fn, "map\\")) return;
		strcpy_s( fn, "ed\\ed_not_existing_texture" );
	}
}


template <class T>
BOOL	reclaim		(xr_vector<T*>& vec, const T* ptr)
{
	xr_vector<T*>::iterator it	= vec.begin	();
	xr_vector<T*>::iterator end	= vec.end	();
	for (; it!=end; it++)
		if (*it == ptr)	{ vec.erase	(it); return TRUE; }
		return FALSE;
}


//--------------------------------------------------------------------------------------------------------------
SState*		CResourceManager::_CreateState		(SimulatorStates& state_code)
{
	// Search equal state-code 
	for (u32 it=0; it<v_states.size(); it++)
	{
		SState*				C		= v_states[it];;
		SimulatorStates&	base	= C->state_code;
		if (base.equal(state_code))	return C;
	}

	// Create New
	SState*	S						= new SState();
	v_states.push_back				(S);
	state_code.record				(S->state);
	S->dwFlags						|= xr_resource_flagged::RF_REGISTERED;
	S->state_code					= state_code;
	return v_states.back();
}
void		CResourceManager::_DeleteState		(const SState* state)
{
	if (0==(state->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_states,state))						return;
	Msg	("! ERROR: Failed to find compiled stateblock");
}

//--------------------------------------------------------------------------------------------------------------
SPass*		CResourceManager::_CreatePass			(ref_state& _state, ref_ps& _ps, ref_vs& _vs, ref_ctable& _ctable, ref_texture_list& _T, ref_matrix_list& _M, ref_constant_list& _C)
{
	for (u32 it=0; it<v_passes.size(); it++)
		if (v_passes[it]->equal(_state,_ps,_vs,_ctable,_T,_M,_C))
			return v_passes[it];

	SPass*	P				= new SPass();
	P->dwFlags				|=	xr_resource_flagged::RF_REGISTERED;
	P->state				=	_state;
	P->ps					=	_ps;
	P->vs					=	_vs;
	P->constants			=	_ctable;
	P->T					=	_T;
#ifdef _EDITOR
	P->M					=	_M;
#endif
	P->C					=	_C;

	v_passes.push_back			(P);
	return v_passes.back();
}

void		CResourceManager::_DeletePass			(const SPass* P)
{
	if (0==(P->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_passes,P))						return;
	Msg	("! ERROR: Failed to find compiled pass");
}

//--------------------------------------------------------------------------------------------------------------
static BOOL	dcl_equal			(D3DVERTEXELEMENT9* a, D3DVERTEXELEMENT9* b)
{
	// check sizes
	u32 a_size = glBufferUtils::GetDeclLength(a);
	u32 b_size = glBufferUtils::GetDeclLength(b);
	if (a_size != b_size)	return FALSE;
	return 0 == memcmp(a, b, a_size*sizeof(D3DVERTEXELEMENT9));
}

SDeclaration*	CResourceManager::_CreateDecl	(u32 FVF)
{
	// Search equal code
	for (u32 it = 0; it<v_declarations.size(); it++)
	{
		SDeclaration*		D = v_declarations[it];;
		if (D->dcl_code.empty() && D->FVF == FVF)	return D;
	}

	SDeclaration* D = new SDeclaration();
	glGenVertexArrays(1, &D->vao);

	D->FVF = FVF;
	glBufferUtils::ConvertVertexDeclaration(FVF, D->vao);
	D->dwFlags |= xr_resource_flagged::RF_REGISTERED;
	v_declarations.push_back(D);

	return D;
}

SDeclaration*	CResourceManager::_CreateDecl(D3DVERTEXELEMENT9* dcl)
{
	// Search equal code
	for (u32 it = 0; it<v_declarations.size(); it++)
	{
		SDeclaration*		D = v_declarations[it];;
		if (!D->dcl_code.empty() && dcl_equal(dcl, &*D->dcl_code.begin()))	return D;
	}

	SDeclaration* D = new SDeclaration();
	glGenVertexArrays(1, &D->vao);

	D->FVF = 0;
	u32 dcl_size = glBufferUtils::GetDeclLength(dcl) + 1;
	D->dcl_code.assign(dcl, dcl + dcl_size);
	glBufferUtils::ConvertVertexDeclaration(dcl, D->vao);
	D->dwFlags |= xr_resource_flagged::RF_REGISTERED;
	v_declarations.push_back(D);

	return D;
}

void		CResourceManager::_DeleteDecl		(const SDeclaration* dcl)
{
	if (0==(dcl->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_declarations,dcl))					return;
	Msg	("! ERROR: Failed to find compiled vertex-declarator");
}

//--------------------------------------------------------------------------------------------------------------
SVS*	CResourceManager::_CreateVS		(LPCSTR _name)
{
	string_path			name;
	strcpy_s(name, _name);
	if (0 == ::Render->m_skinning)	strcat(name, "_0");
	if (1 == ::Render->m_skinning)	strcat(name, "_1");
	if (2 == ::Render->m_skinning)	strcat(name, "_2");
	if (3 == ::Render->m_skinning)	strcat(name, "_3");
	if (4 == ::Render->m_skinning)	strcat(name, "_4");
	LPSTR N = LPSTR(name);
	map_VS::iterator I = m_vs.find(N);
	if (I != m_vs.end())	return I->second;
	else
	{
		SVS*	_vs = new SVS();
		_vs->dwFlags |= xr_resource_flagged::RF_REGISTERED;
		m_vs.insert(mk_pair(_vs->set_name(name), _vs));
		//_vs->vs				= NULL;
		//_vs->signature		= NULL;
		VERIFY(strcmpi(name, "null") != 0);

		GLchar*					pErrorBuf = NULL;
		string_path					cname;
		strconcat(sizeof(cname), cname, ::Render->getShaderPath(), _name, ".vs");
		FS.update_path(cname, "$game_shaders$", cname);
		//		LPCSTR						target		= NULL;

		IReader*					fs = FS.r_open(cname);
		//	TODO: OGL: HACK: Implement all shaders. Remove this for PS
		if (!fs)
		{
			string1024			tmp;
			sprintf(tmp, "OGL: %s is missing. Replace with stub_default.vs", cname);
			Msg(tmp);
			strconcat(sizeof(cname), cname, ::Render->getShaderPath(), "stub_default", ".vs");
			FS.update_path(cname, "$game_shaders$", cname);
			fs = FS.r_open(cname);
		}
		R_ASSERT3(fs, "shader file doesnt exist", cname);

		// vertex
		R_ASSERT2(fs, cname);
		_vs->vs = glCreateShader(GL_VERTEX_SHADER);
		GLenum _result = ::Render->shader_compile(name, LPCSTR(fs->pointer()), fs->length(), NULL, NULL, NULL, NULL, 0, &_vs->vs, &pErrorBuf, NULL);
		FS.r_close(fs);

		if (_result == GL_TRUE)
		{
			//	Parse constant, texture, sampler binding
			_vs->constants.parse(&_vs->vs, RC_dest_vertex);
		}
		else
		{
			VERIFY(pErrorBuf);
			Log("! VS: ", _name);
			Log("! error: ", pErrorBuf);
		}
		R_ASSERT2(_result, pErrorBuf);
		xr_free(pErrorBuf);
		return		_vs;
	}
}

void	CResourceManager::_DeleteVS			(const SVS* vs)
{
	if (0==(vs->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	LPSTR N				= LPSTR		(*vs->cName);
	map_VS::iterator I	= m_vs.find	(N);
	if (I!=m_vs.end())	{
		m_vs.erase(I);
		return;
	}
	Msg	("! ERROR: Failed to find compiled vertex-shader '%s'",*vs->cName);
}

//--------------------------------------------------------------------------------------------------------------
SPS*	CResourceManager::_CreatePS			(LPCSTR _name)
{
	string_path			name;
	strcpy_s(name, _name);
	if (0 == ::Render->m_MSAASample)	strcat(name, "_0");
	if (1 == ::Render->m_MSAASample)	strcat(name, "_1");
	if (2 == ::Render->m_MSAASample)	strcat(name, "_2");
	if (3 == ::Render->m_MSAASample)	strcat(name, "_3");
	if (4 == ::Render->m_MSAASample)	strcat(name, "_4");
	if (5 == ::Render->m_MSAASample)	strcat(name, "_5");
	if (6 == ::Render->m_MSAASample)	strcat(name, "_6");
	if (7 == ::Render->m_MSAASample)	strcat(name, "_7");
	LPSTR N = LPSTR(name);
	map_PS::iterator I = m_ps.find(N);
	if (I != m_ps.end())	return		I->second;
	else
	{
		SPS*	_ps = new SPS();
		_ps->dwFlags |= xr_resource_flagged::RF_REGISTERED;
		m_ps.insert(mk_pair(_ps->set_name(name), _ps));
		VERIFY(strcmpi(name, "null") != 0);

		// Open file
		string_path					cname;
		strconcat(sizeof(cname), cname, ::Render->getShaderPath(), _name, ".ps");
		FS.update_path(cname, "$game_shaders$", cname);

		// duplicate and zero-terminate
		IReader*		R = FS.r_open(cname);
		//	TODO: DX10: HACK: Implement all shaders. Remove this for PS
		if (!R)
		{
			string1024			tmp;
			//	TODO: HACK: Test failure
			//Memory.mem_compact();
			sprintf(tmp, "OGL: %s is missing. Replace with stub_default.ps", cname);
			Msg(tmp);
			strconcat(sizeof(cname), cname, ::Render->getShaderPath(), "stub_default", ".ps");
			FS.update_path(cname, "$game_shaders$", cname);
			R = FS.r_open(cname);
		}
		R_ASSERT2(R, cname);
		u32				size = R->length();
		char*			data = xr_alloc<char>(size + 1);
		CopyMemory(data, R->pointer(), size);
		data[size] = 0;
		FS.r_close(R);

		// Compile
		GLchar* pErrorBuf = NULL;
		_ps->ps = glCreateShader(GL_FRAGMENT_SHADER);
		GLenum _result = ::Render->shader_compile(name, data, size, NULL, NULL, NULL, NULL, 0, &_ps->ps, &pErrorBuf, NULL);
		xr_free(data);

		if (_result == GL_TRUE)
		{
			//	Parse constant, texture, sampler binding
			_ps->constants.parse(&_ps->ps, RC_dest_pixel);
		}
		else
		{
			VERIFY(pErrorBuf);
			Log("! PS: ", _name);
			Msg("error is %s", pErrorBuf);
		}
		R_ASSERT2(_result, pErrorBuf);
		xr_free(pErrorBuf);

		if (_result == GL_FALSE)
			Msg("Can't compile shader %s", _name);

		CHECK_OR_EXIT(
			_result != GL_FALSE,
			make_string("Your video card doesn't meet game requirements\n\nPixel Shaders v1.1 or higher required")
			);
		return			_ps;
	}
}
void	CResourceManager::_DeletePS			(const SPS* ps)
{
	if (0==(ps->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	LPSTR N				= LPSTR		(*ps->cName);
	map_PS::iterator I	= m_ps.find	(N);
	if (I!=m_ps.end())	{
		m_ps.erase(I);
		return;
	}
	Msg	("! ERROR: Failed to find compiled pixel-shader '%s'",*ps->cName);
}

R_constant_table*	CResourceManager::_CreateConstantTable	(R_constant_table& C)
{
	if (C.empty())		return NULL;
	for (u32 it=0; it<v_constant_tables.size(); it++)
		if (v_constant_tables[it]->equal(C))	return v_constant_tables[it];
	v_constant_tables.push_back			(new R_constant_table(C));
	v_constant_tables.back()->dwFlags	|=	xr_resource_flagged::RF_REGISTERED;
	return v_constant_tables.back		();
}
void				CResourceManager::_DeleteConstantTable	(const R_constant_table* C)
{
	if (0==(C->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_constant_tables,C))				return;
	Msg	("! ERROR: Failed to find compiled constant-table");
}

//--------------------------------------------------------------------------------------------------------------
CRT*	CResourceManager::_CreateRT		(LPCSTR Name, u32 w, u32 h,	D3DFORMAT f, u32 SampleCount )
{
	R_ASSERT(Name && Name[0] && w && h);

	// ***** first pass - search already created RT
	LPSTR N = LPSTR(Name);
	map_RT::iterator I = m_rtargets.find	(N);
	if (I!=m_rtargets.end())	return		I->second;
	else
	{
		CRT *RT					= new CRT();
		RT->dwFlags				|=	xr_resource_flagged::RF_REGISTERED;
		m_rtargets.insert		(mk_pair(RT->set_name(Name),RT));
		if (Device.b_is_Ready)	RT->create	(Name,w,h,f);
		return					RT;
	}
}
void	CResourceManager::_DeleteRT		(const CRT* RT)
{
	if (0==(RT->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	LPSTR N				= LPSTR		(*RT->cName);
	map_RT::iterator I	= m_rtargets.find	(N);
	if (I!=m_rtargets.end())	{
		m_rtargets.erase(I);
		return;
	}
	Msg	("! ERROR: Failed to find render-target '%s'",*RT->cName);
}

//--------------------------------------------------------------------------------------------------------------
void	CResourceManager::DBG_VerifyGeoms	()
{
}

SGeometry*	CResourceManager::CreateGeom	(D3DVERTEXELEMENT9* decl, GLuint vb, GLuint ib)
{
	R_ASSERT(decl && vb);

	SDeclaration* dcl = _CreateDecl(decl);
	u32 vb_stride = glBufferUtils::GetDeclVertexSize(decl);

	// ***** first pass - search already loaded shader
	for (u32 it = 0; it<v_geoms.size(); it++)
	{
		SGeometry& G = *(v_geoms[it]);
		if ((G.dcl == dcl) && (G.vb == vb) && (G.ib == ib) && (G.vb_stride == vb_stride))	return v_geoms[it];
	}

	SGeometry *Geom = new SGeometry();
	Geom->dwFlags |= xr_resource_flagged::RF_REGISTERED;
	Geom->dcl = dcl;
	Geom->vb = vb;
	Geom->vb_stride = vb_stride;
	Geom->ib = ib;
	v_geoms.push_back(Geom);
	return Geom;
}

SGeometry*	CResourceManager::CreateGeom	(u32 FVF, GLuint vb, GLuint ib)
{
	R_ASSERT(FVF && vb);

	SDeclaration* dcl = _CreateDecl(FVF);
	u32 vb_stride = glBufferUtils::GetFVFVertexSize(FVF);

	// ***** first pass - search already loaded shader
	for (u32 it = 0; it<v_geoms.size(); it++)
	{
		SGeometry& G = *(v_geoms[it]);
		if ((G.dcl == dcl) && (G.vb == vb) && (G.ib == ib) && (G.vb_stride == vb_stride))	return v_geoms[it];
	}

	SGeometry *Geom = new SGeometry();
	Geom->dwFlags |= xr_resource_flagged::RF_REGISTERED;
	Geom->dcl = dcl;
	Geom->vb = vb;
	Geom->vb_stride = vb_stride;
	Geom->ib = ib;
	v_geoms.push_back(Geom);
	return Geom;
}

void		CResourceManager::DeleteGeom		(const SGeometry* Geom)
{
	if (0==(Geom->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(v_geoms,Geom))							return;
	Msg	("! ERROR: Failed to find compiled geometry-declaration");
}

//--------------------------------------------------------------------------------------------------------------
CTexture* CResourceManager::_CreateTexture	(LPCSTR _Name)
{
	// DBG_VerifyTextures	();
	if (0==xr_strcmp(_Name,"null"))	return 0;
	R_ASSERT		(_Name && _Name[0]);
	string_path		Name;
	strcpy_s			(Name,_Name); //. andy if (strext(Name)) *strext(Name)=0;
	fix_texture_name (Name);

#ifdef	DEBUG
	simplify_texture(Name);
#endif	//	DEBUG

	// ***** first pass - search already loaded texture
	LPSTR N			= LPSTR(Name);
	map_TextureIt I = m_textures.find	(N);
	if (I!=m_textures.end())	return	I->second;
	else
	{
		CTexture *	T		= new CTexture();
		T->dwFlags			|=	xr_resource_flagged::RF_REGISTERED;
		m_textures.insert	(mk_pair(T->set_name(Name),T));
		T->Preload			();
		if (Device.b_is_Ready && !bDeferredLoad) T->Load();
		return		T;
	}
}
void	CResourceManager::_DeleteTexture		(const CTexture* T)
{
	// DBG_VerifyTextures	();

	if (0==(T->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	LPSTR N					= LPSTR		(*T->cName);
	map_Texture::iterator I	= m_textures.find	(N);
	if (I!=m_textures.end())	{
		m_textures.erase(I);
		return;
	}
	Msg	("! ERROR: Failed to find texture surface '%s'",*T->cName);
}

#ifdef DEBUG
void	CResourceManager::DBG_VerifyTextures	()
{
	map_Texture::iterator I		= m_textures.begin	();
	map_Texture::iterator E		= m_textures.end	();
	for (; I!=E; I++) 
	{
		R_ASSERT(I->first);
		R_ASSERT(I->second);
		R_ASSERT(I->second->cName);
		R_ASSERT(0==xr_strcmp(I->first,*I->second->cName));
	}
}
#endif

//--------------------------------------------------------------------------------------------------------------
CMatrix*	CResourceManager::_CreateMatrix	(LPCSTR Name)
{
	R_ASSERT(Name && Name[0]);
	if (0==stricmp(Name,"$null"))	return NULL;

	LPSTR N = LPSTR(Name);
	map_Matrix::iterator I = m_matrices.find	(N);
	if (I!=m_matrices.end())	return I->second;
	else
	{
		CMatrix* M			= new CMatrix();
		M->dwFlags			|=	xr_resource_flagged::RF_REGISTERED;
		M->dwReference		=	1;
		m_matrices.insert	(mk_pair(M->set_name(Name),M));
		return			M;
	}
}
void	CResourceManager::_DeleteMatrix		(const CMatrix* M)
{
	if (0==(M->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	LPSTR N					= LPSTR		(*M->cName);
	map_Matrix::iterator I	= m_matrices.find	(N);
	if (I!=m_matrices.end())	{
		m_matrices.erase(I);
		return;
	}
	Msg	("! ERROR: Failed to find xform-def '%s'",*M->cName);
}
void	CResourceManager::ED_UpdateMatrix		(LPCSTR Name, CMatrix* data)
{
	CMatrix*	M	= _CreateMatrix	(Name);
	*M				= *data;
}
//--------------------------------------------------------------------------------------------------------------
CConstant*	CResourceManager::_CreateConstant	(LPCSTR Name)
{
	R_ASSERT(Name && Name[0]);
	if (0==stricmp(Name,"$null"))	return NULL;

	LPSTR N = LPSTR(Name);
	map_Constant::iterator I	= m_constants.find	(N);
	if (I!=m_constants.end())	return I->second;
	else
	{
		CConstant* C		= new CConstant();
		C->dwFlags			|=	xr_resource_flagged::RF_REGISTERED;
		C->dwReference		=	1;
		m_constants.insert	(mk_pair(C->set_name(Name),C));
		return	C;
	}
}
void	CResourceManager::_DeleteConstant		(const CConstant* C)
{
	if (0==(C->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	LPSTR N				= LPSTR				(*C->cName);
	map_Constant::iterator I	= m_constants.find	(N);
	if (I!=m_constants.end())	{
		m_constants.erase(I);
		return;
	}
	Msg	("! ERROR: Failed to find R1-constant-def '%s'",*C->cName);
}

void	CResourceManager::ED_UpdateConstant	(LPCSTR Name, CConstant* data)
{
	CConstant*	C	= _CreateConstant	(Name);
	*C				= *data;
}

//--------------------------------------------------------------------------------------------------------------
bool	cmp_tl	(const std::pair<u32,ref_texture>& _1, const std::pair<u32,ref_texture>& _2)	{
	return _1.first < _2.first;
}
STextureList*	CResourceManager::_CreateTextureList(STextureList& L)
{
	std::sort	(L.begin(),L.end(),cmp_tl);
	for (u32 it=0; it<lst_textures.size(); it++)
	{
		STextureList*	base		= lst_textures[it];
		if (L.equal(*base))			return base;
	}
	STextureList*	lst		= new STextureList(L);
	lst->dwFlags			|=	xr_resource_flagged::RF_REGISTERED;
	lst_textures.push_back	(lst);
	return lst;
}
void			CResourceManager::_DeleteTextureList(const STextureList* L)
{
	if (0==(L->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(lst_textures,L))					return;
	Msg	("! ERROR: Failed to find compiled list of textures");
}
//--------------------------------------------------------------------------------------------------------------
SMatrixList*	CResourceManager::_CreateMatrixList(SMatrixList& L)
{
	BOOL bEmpty = TRUE;
	for (u32 i=0; i<L.size(); i++)	if (L[i]) { bEmpty=FALSE; break; }
	if (bEmpty)	return NULL;

	for (u32 it=0; it<lst_matrices.size(); it++)
	{
		SMatrixList*	base		= lst_matrices[it];
		if (L.equal(*base))			return base;
	}
	SMatrixList*	lst		= new SMatrixList(L);
	lst->dwFlags			|=	xr_resource_flagged::RF_REGISTERED;
	lst_matrices.push_back	(lst);
	return lst;
}
void			CResourceManager::_DeleteMatrixList ( const SMatrixList* L )
{
	if (0==(L->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(lst_matrices,L))					return;
	Msg	("! ERROR: Failed to find compiled list of xform-defs");
}
//--------------------------------------------------------------------------------------------------------------
SConstantList*	CResourceManager::_CreateConstantList(SConstantList& L)
{
	BOOL bEmpty = TRUE;
	for (u32 i=0; i<L.size(); i++)	if (L[i]) { bEmpty=FALSE; break; }
	if (bEmpty)	return NULL;

	for (u32 it=0; it<lst_constants.size(); it++)
	{
		SConstantList*	base		= lst_constants[it];
		if (L.equal(*base))			return base;
	}
	SConstantList*	lst		= new SConstantList(L);
	lst->dwFlags			|=	xr_resource_flagged::RF_REGISTERED;
	lst_constants.push_back	(lst);
	return lst;
}
void			CResourceManager::_DeleteConstantList(const SConstantList* L )
{
	if (0==(L->dwFlags&xr_resource_flagged::RF_REGISTERED))	return;
	if (reclaim(lst_constants,L))					return;
	Msg	("! ERROR: Failed to find compiled list of r1-constant-defs");
}
