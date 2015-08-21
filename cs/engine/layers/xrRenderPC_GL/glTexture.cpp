// Texture.cpp: implementation of the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <gli/gli.hpp>

void fix_texture_name(LPSTR fn)
{
	LPSTR _ext = strext(fn);
	if (_ext &&
		(0 == stricmp(_ext, ".tga") ||
		0 == stricmp(_ext, ".dds") ||
		0 == stricmp(_ext, ".bmp") ||
		0 == stricmp(_ext, ".ogm")))
		*_ext = 0;
}

int get_texture_load_lod(LPCSTR fn)
{
	CInifile::Sect& sect	= pSettings->r_section("reduce_lod_texture_list");
	CInifile::SectCIt it_	= sect.Data.begin();
	CInifile::SectCIt it_e_	= sect.Data.end();

	CInifile::SectCIt it	= it_;
	CInifile::SectCIt it_e	= it_e_;

	for(;it!=it_e;++it)
	{
		if( strstr(fn, it->first.c_str()) )
		{
			if(psTextureLOD<1)
				return 0;
			else
			if(psTextureLOD<3)
				return 1;
			else
				return 2;
		}
	}

	if(psTextureLOD<2)
		return 0;
	else
	if(psTextureLOD<4)
		return 1;
	else
		return 2;
}

u32 calc_texture_size(int lod, u32 mip_cnt, u32 orig_size)
{
	if (1 == mip_cnt)
		return orig_size;

	int _lod = lod;
	float res = float(orig_size);

	while (_lod>0){
		--_lod;
		res -= res / 1.333f;
	}
	return iFloor(res);
}

GLuint	CRender::texture_load(LPCSTR fRName, u32& ret_msize, GLenum& ret_desc)
{
	GLuint					pTexture = 0;
	string_path				fn;
	u32						dwWidth, dwHeight;
	u32						img_size = 0;
	int						img_loaded_lod = 0;
	gli::gl::format			fmt;
	u32						mip_cnt = u32(-1);
	// validation
	R_ASSERT(fRName);
	R_ASSERT(fRName[0]);

	// make file name
	string_path				fname;
	strcpy_s(fname, fRName); //. andy if (strext(fname)) *strext(fname)=0;
	fix_texture_name(fname);
	IReader* S = NULL;
	//if (FS.exist(fn,"$game_textures$",fname,	".dds")	&& strstr(fname,"_bump"))	goto _BUMP;
	//if (!FS.exist(fn, "$game_textures$", fname, ".dds") && strstr(fname, "_bump"))	goto _BUMP_from_base;
	if (FS.exist(fn, "$level$", fname, ".dds"))							goto _DDS;
	if (FS.exist(fn, "$game_saves$", fname, ".dds"))							goto _DDS;
	if (FS.exist(fn, "$game_textures$", fname, ".dds"))							goto _DDS;


#ifdef _EDITOR
	ELog.Msg(mtError, "Can't find texture '%s'", fname);
	return 0;
#else

	Msg("! Can't find texture '%s'", fname);
	R_ASSERT(FS.exist(fn, "$game_textures$", "ed\\ed_not_existing_texture", ".dds"));
	goto _DDS;

	//	xrDebug::Fatal(DEBUG_INFO,"Can't find texture '%s'",fname);

#endif

_DDS:
	{
		// Load and get header
		S = FS.r_open(fn);
#ifdef DEBUG
		Msg("* Loaded: %s[%d]b", fn, S->length());
#endif // DEBUG
		img_size = S->length();
		R_ASSERT(S);
		gli::storage IMG = gli::load_dds((char*)S->pointer(), img_size);
		if (IMG.faces() > 1)										goto _DDS_CUBE;
		else														goto _DDS_2D;

	_DDS_CUBE:
		{
			gli::textureCube Texture(IMG);
			R_ASSERT(!Texture.empty());
			gli::gl GL;
			mip_cnt = Texture.levels();
			dwWidth = Texture.dimensions().x;
			dwHeight = Texture.dimensions().y;
			fmt = GL.translate(Texture.format());

			glGenTextures(1, &pTexture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, pTexture);
			CHK_GL(glTexStorage2D(GL_TEXTURE_CUBE_MAP, mip_cnt, fmt.Internal, dwWidth, dwHeight));
			for (size_t i = 0; i < mip_cnt; i++)
			{
				if (gli::is_compressed(Texture.format()))
				{
					CHK_GL(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP, i, 0, 0, dwWidth, dwHeight,
						fmt.External, Texture[i].size(), Texture[i].data()));
				}
				else {
					CHK_GL(glTexSubImage2D(GL_TEXTURE_CUBE_MAP, i, 0, 0, dwWidth, dwHeight,
						fmt.External, fmt.Type, Texture[i].data()));
				}
			}
			FS.r_close(S);

			// OK
			ret_msize = calc_texture_size(img_loaded_lod, mip_cnt, img_size);
			ret_desc = GL_TEXTURE_CUBE_MAP;
			return					pTexture;
		}
	_DDS_2D:
		{
			// Check for LMAP and compress if needed
			strlwr(fn);


			// Load   SYS-MEM-surface, bound to device restrictions
			gli::texture2D Texture(IMG);
			R_ASSERT(!Texture.empty());
			gli::gl GL;
			mip_cnt = Texture.levels();
			dwWidth = Texture.dimensions().x;
			dwHeight = Texture.dimensions().y;
			fmt = GL.translate(Texture.format());

			glGenTextures(1, &pTexture);
			glBindTexture(GL_TEXTURE_2D, pTexture);
			CHK_GL(glTexStorage2D(GL_TEXTURE_2D, mip_cnt, fmt.Internal, dwWidth, dwHeight));
			for (size_t i = 0; i < mip_cnt; i++)
			{
				if (gli::is_compressed(Texture.format()))
				{
					CHK_GL(glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, dwWidth, dwHeight,
						fmt.External, Texture[i].size(), Texture[i].data()));
				}
				else {
					CHK_GL(glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, dwWidth, dwHeight,
						fmt.External, fmt.Type, Texture[i].data()));
				}
			}
			FS.r_close(S);

			// OK
			img_loaded_lod = get_texture_load_lod(fn);
			ret_msize = calc_texture_size(img_loaded_lod, mip_cnt, img_size);
			ret_desc = GL_TEXTURE_2D;
			return					pTexture;
		}
	}
	/*
	_BUMP:
	{
	// Load   SYS-MEM-surface, bound to device restrictions
	D3DXIMAGE_INFO			IMG;
	IReader* S				= FS.r_open	(fn);
	msize					= S->length	();
	ID3DTexture2D*		T_height_gloss;
	R_CHK(D3DXCreateTextureFromFileInMemoryEx(
	HW.pDevice,	S->pointer(),S->length(),
	D3DX_DEFAULT,D3DX_DEFAULT,	D3DX_DEFAULT,0,D3DFMT_A8R8G8B8,
	D3DPOOL_SYSTEMMEM,			D3DX_DEFAULT,D3DX_DEFAULT,
	0,&IMG,0,&T_height_gloss	));
	FS.r_close				(S);
	//TW_Save						(T_height_gloss,fname,"debug-0","original");

	// Create HW-surface, compute normal map
	ID3DTexture2D*	T_normal_1	= 0;
	R_CHK(D3DXCreateTexture		(HW.pDevice,IMG.Width,IMG.Height,D3DX_DEFAULT,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&T_normal_1));
	R_CHK(D3DXComputeNormalMap	(T_normal_1,T_height_gloss,0,0,D3DX_CHANNEL_RED,_BUMPHEIGH));
	//TW_Save						(T_normal_1,fname,"debug-1","normal");

	// Transfer gloss-map
	TW_Iterate_1OP				(T_normal_1,T_height_gloss,it_gloss_rev);
	//TW_Save						(T_normal_1,fname,"debug-2","normal-G");

	// Compress
	fmt								= D3DFMT_DXT5;
	ID3DTexture2D*	T_normal_1C	= TW_LoadTextureFromTexture(T_normal_1,fmt,psTextureLOD,dwWidth,dwHeight);
	//TW_Save						(T_normal_1C,fname,"debug-3","normal-G-C");

	#if RENDER==R_R2
	// Decompress (back)
	fmt								= D3DFMT_A8R8G8B8;
	ID3DTexture2D*	T_normal_1U	= TW_LoadTextureFromTexture(T_normal_1C,fmt,0,dwWidth,dwHeight);
	// TW_Save						(T_normal_1U,fname,"debug-4","normal-G-CU");

	// Calculate difference
	ID3DTexture2D*	T_normal_1D = 0;
	R_CHK(D3DXCreateTexture(HW.pDevice,dwWidth,dwHeight,T_normal_1U->GetLevelCount(),0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&T_normal_1D));
	TW_Iterate_2OP				(T_normal_1D,T_normal_1,T_normal_1U,it_difference);
	// TW_Save						(T_normal_1D,fname,"debug-5","normal-G-diff");

	// Reverse channels back + transfer heightmap
	TW_Iterate_1OP				(T_normal_1D,T_height_gloss,it_height_rev);
	// TW_Save						(T_normal_1D,fname,"debug-6","normal-G-diff-H");

	// Compress
	fmt								= D3DFMT_DXT5;
	ID3DTexture2D*	T_normal_2C	= TW_LoadTextureFromTexture(T_normal_1D,fmt,0,dwWidth,dwHeight);
	// TW_Save						(T_normal_2C,fname,"debug-7","normal-G-diff-H-C");
	_RELEASE					(T_normal_1U	);
	_RELEASE					(T_normal_1D	);

	//
	string256			fnameB;
	strconcat			(fnameB,"$user$",fname,"X");
	ref_texture			t_temp		= dxRenderDeviceRender::Instance().Resources->_CreateTexture	(fnameB);
	t_temp->surface_set	(T_normal_2C	);
	_RELEASE			(T_normal_2C	);	// texture should keep reference to it by itself
	#endif

	// release and return
	// T_normal_1C	- normal.gloss,		reversed
	// T_normal_2C	- 2*error.height,	non-reversed
	_RELEASE			(T_height_gloss	);
	_RELEASE			(T_normal_1		);
	return				T_normal_1C;
	}
_BUMP_from_base:
	{
		Msg("! auto-generated bump map: %s", fname);
		//////////////////
		if (strstr(fname, "_bump#"))
		{
			R_ASSERT2(FS.exist(fn, "$game_textures$", "ed\\ed_dummy_bump#", ".dds"), "ed_dummy_bump#");
			S = FS.r_open(fn);
			R_ASSERT2(S, fn);
			img_size = S->length();
			goto		_DDS_2D;
		}
		if (strstr(fname, "_bump"))
		{
			R_ASSERT2(FS.exist(fn, "$game_textures$", "ed\\ed_dummy_bump", ".dds"), "ed_dummy_bump");
			S = FS.r_open(fn);

			R_ASSERT2(S, fn);

			img_size = S->length();
			goto		_DDS_2D;
		}
		//////////////////

		*strstr(fname, "_bump") = 0;
		R_ASSERT2(FS.exist(fn, "$game_textures$", fname, ".dds"), fname);

		// Load   SYS-MEM-surface, bound to device restrictions
		D3DXIMAGE_INFO			IMG;
		S = FS.r_open(fn);
		img_size = S->length();
		ID3DTexture2D*		T_base;
		R_CHK2(D3DXCreateTextureFromFileInMemoryEx(
			HW.pDevice, S->pointer(), S->length(),
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8,
			D3DPOOL_SYSTEMMEM, D3DX_DEFAULT, D3DX_DEFAULT,
			0, &IMG, 0, &T_base), fn);
		FS.r_close(S);

		// Create HW-surface
		ID3DTexture2D*	T_normal_1 = 0;
		R_CHK(D3DXCreateTexture(HW.pDevice, IMG.Width, IMG.Height, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &T_normal_1));
		R_CHK(D3DXComputeNormalMap(T_normal_1, T_base, 0, D3DX_NORMALMAP_COMPUTE_OCCLUSION, D3DX_CHANNEL_LUMINANCE, _BUMPHEIGH));

		// Transfer gloss-map
		TW_Iterate_1OP(T_normal_1, T_base, it_gloss_rev_base);

		// Compress
		fmt = D3DFMT_DXT5;
		img_loaded_lod = get_texture_load_lod(fn);
		ID3DTexture2D*	T_normal_1C = TW_LoadTextureFromTexture(T_normal_1, fmt, img_loaded_lod, dwWidth, dwHeight);
		mip_cnt = T_normal_1C->GetLevelCount();

#if RENDER==R_R2	
		// Decompress (back)
		fmt = D3DFMT_A8R8G8B8;
		ID3DTexture2D*	T_normal_1U = TW_LoadTextureFromTexture(T_normal_1C, fmt, 0, dwWidth, dwHeight);

		// Calculate difference
		ID3DTexture2D*	T_normal_1D = 0;
		R_CHK(D3DXCreateTexture(HW.pDevice, dwWidth, dwHeight, T_normal_1U->GetLevelCount(), 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &T_normal_1D));
		TW_Iterate_2OP(T_normal_1D, T_normal_1, T_normal_1U, it_difference);

		// Reverse channels back + transfer heightmap
		TW_Iterate_1OP(T_normal_1D, T_base, it_height_rev_base);

		// Compress
		fmt = D3DFMT_DXT5;
		ID3DTexture2D*	T_normal_2C = TW_LoadTextureFromTexture(T_normal_1D, fmt, 0, dwWidth, dwHeight);
		_RELEASE(T_normal_1U);
		_RELEASE(T_normal_1D);

		// 
		string256			fnameB;
		strconcat(sizeof(fnameB), fnameB, "$user$", fname, "_bumpX");
		ref_texture			t_temp = dxRenderDeviceRender::Instance().Resources->_CreateTexture(fnameB);
		t_temp->surface_set(T_normal_2C);
		_RELEASE(T_normal_2C);	// texture should keep reference to it by itself
#endif
		// T_normal_1C	- normal.gloss,		reversed
		// T_normal_2C	- 2*error.height,	non-reversed
		_RELEASE(T_base);
		_RELEASE(T_normal_1);
		ret_msize = calc_texture_size(img_loaded_lod, mip_cnt, img_size);
		return				T_normal_1C;
	}
	*/
}
