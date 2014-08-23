// file:		UITextureMaster.h
// description:	holds info about shared textures. able to initialize external controls
//				through IUITextureControl interface
// created:		11.05.2005
// author:		Serge Vynnychenko
// mail:		narrator@gsc-game.kiev.ua
//
// copyright 2005 GSC Game World


#include "StdAfx.h"
#include "UITextureMaster.h"
#include "uiabstract.h"
#include "xrUIXmlParser.h"
#include "../Include/xrRender/UIShader.h"

xr_map<shared_str, TEX_INFO>	CUITextureMaster::m_textures;

void CUITextureMaster::FreeTexInfo()
{
	m_textures.clear();
}

void CUITextureMaster::ParseShTexInfo(LPCSTR xml_file)
{
	CUIXml						xml;
	xml.Load					(CONFIG_PATH, "ui\\textures_descr", xml_file);

	int files_num				= xml.GetNodesNum("",0,"file");


	for(int fi=0; fi<files_num; ++fi)
	{
		XML_NODE* root_node			= xml.GetLocalRoot();
		shared_str file				= xml.ReadAttrib("file", fi, "name"); 

		XML_NODE* node				= xml.NavigateToNode("file", fi);

//.		Msg("-%s",file.c_str());
		int num						= xml.GetNodesNum(node, "texture");
		for (int i = 0; i<num; i++)
		{
			TEX_INFO info;

			info.file = file;

			info.rect.x1 = xml.ReadAttribFlt(node, "texture",i,"x");
			info.rect.x2 = xml.ReadAttribFlt(node, "texture",i,"width") + info.rect.x1;
			info.rect.y1 = xml.ReadAttribFlt(node, "texture",i,"y");
			info.rect.y2 = xml.ReadAttribFlt(node, "texture",i,"height") + info.rect.y1;
			shared_str id = xml.ReadAttrib	(node, "texture",i,"id");
//.			Msg("--%s",id.c_str());

			m_textures.insert(mk_pair(id,info));
		}

		xml.SetLocalRoot		(root_node);
	}
}

bool CUITextureMaster::IsSh(const char* texture_name){
	return strstr(texture_name,"\\") ? false : true;
}

void CUITextureMaster::InitTexture(const char* texture_name, IUISimpleTextureControl* tc){
	xr_map<shared_str, TEX_INFO>::iterator	it;

	it = m_textures.find(texture_name);

	if (it != m_textures.end())
	{
		tc->CreateShader(*((*it).second.file));
		tc->SetOriginalRectEx((*it).second.rect);
		return;
	}
	tc->CreateShader(texture_name);
}

void CUITextureMaster::InitTexture(const char* texture_name, const char* shader_name, IUISimpleTextureControl* tc){

	xr_map<shared_str, TEX_INFO>::iterator	it;

	it = m_textures.find(texture_name);

	if (it != m_textures.end())
	{
		tc->CreateShader(*((*it).second.file), shader_name);
		tc->SetOriginalRectEx((*it).second.rect);
		return;
	}
	tc->CreateShader(texture_name, shader_name);
}

float CUITextureMaster::GetTextureHeight(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return (*it).second.rect.height();
	R_ASSERT3(false,"CUITextureMaster::GetTextureHeight Can't find texture", texture_name);
	return 0;
}

Frect CUITextureMaster::GetTextureRect(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);
	if (it != m_textures.end())
		return (*it).second.rect;

	R_ASSERT3(false,"CUITextureMaster::GetTextureHeight Can't find texture", texture_name);
	return Frect();
}

float CUITextureMaster::GetTextureWidth(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return (*it).second.rect.width();
	R_ASSERT3(false,"CUITextureMaster::GetTextureHeight Can't find texture", texture_name);
	return 0;
}

LPCSTR CUITextureMaster::GetTextureFileName(const char* texture_name){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return *((*it).second.file);
	R_ASSERT3(false,"CUITextureMaster::GetTextureFileName Can't find texture", texture_name);
	return 0;
}

TEX_INFO CUITextureMaster::FindItem(LPCSTR texture_name, LPCSTR def_texture_name)
{
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	if (it != m_textures.end())
		return (it->second);
	else{
		R_ASSERT2(m_textures.find(def_texture_name)!=m_textures.end(),texture_name);
		return FindItem	(def_texture_name,NULL);
	}
}

void CUITextureMaster::GetTextureShader(LPCSTR texture_name, ui_shader& sh){
	xr_map<shared_str, TEX_INFO>::iterator	it;
	it = m_textures.find(texture_name);

	R_ASSERT3(it != m_textures.end(), "can't find texture", texture_name);

	sh->create("hud\\default", *((*it).second.file));	
}