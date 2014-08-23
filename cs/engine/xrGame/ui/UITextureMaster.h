// file:		UITextureMaster.h
// description:	holds info about shared textures. able to initialize external
//				through IUITextureControl interface
// created:		11.05.2005
// author:		Serge Vynnychenko
// mail:		narrator@gsc-game.kiev.ua
//
// copyright 2005 GSC Game World

#pragma once

class IUISimpleTextureControl;
#include "ui_defs.h"

struct TEX_INFO{
	shared_str	file;
	Frect		rect;
	LPCSTR		get_file_name	()	{return *file;}
	Frect		get_rect		()	{return rect;}
};

class CUITextureMaster{
public:

	static void ParseShTexInfo			(LPCSTR xml_file);
	static void FreeTexInfo				();

	static void		InitTexture			(LPCSTR texture_name,		IUISimpleTextureControl* tc);
	static void		InitTexture			(LPCSTR texture_name, const char* shader_name, IUISimpleTextureControl* tc);
	static float	GetTextureHeight	(LPCSTR texture_name);
	static float	GetTextureWidth		(LPCSTR texture_name);
	static Frect	GetTextureRect		(LPCSTR texture_name);
	static LPCSTR	GetTextureFileName	(LPCSTR texture_name);
	static void		GetTextureShader	(LPCSTR texture_name, ui_shader& sh);
	static TEX_INFO	FindItem			(LPCSTR texture_name, LPCSTR def_texture_name);
protected:
	IC	static bool IsSh					(const char* texture_name);

	static xr_map<shared_str, TEX_INFO>					m_textures;

};