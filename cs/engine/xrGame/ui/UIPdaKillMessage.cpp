// File:		UIPdaKillMessage.cpp
// Description:	HUD message about player death. Implementation of visual behavior
// Created:		10.03.2005
// Author:		Serge Vynnychenko
// Mail:		narrator@gsc-game.kiev.ua
// 
// Copyright 2005 GSC GameWorld

#include "StdAfx.h"
#include "UIPdaKillMessage.h"
#include "UIInventoryUtilities.h"
#include "../Include/xrRender/UIShader.h"

using namespace InventoryUtilities;

const int INDENT = 3;

CUIPdaKillMessage::CUIPdaKillMessage()
{
	SetTextComplexMode(true);
	AttachChild(&m_victim_name);m_victim_name.SetTextComplexMode(false);
	AttachChild(&m_killer_name);m_killer_name.SetTextComplexMode(false);	
	AttachChild(&m_initiator);m_initiator.SetTextComplexMode(false);
	AttachChild(&m_ext_info);m_ext_info.SetTextComplexMode(false);
}

CUIPdaKillMessage::~CUIPdaKillMessage(){

}

void CUIPdaKillMessage::Init(KillMessageStruct& msg){
#ifdef DEBUG
	R_ASSERT2(GetWidth(),  "CUIPdaKillMessage::Init(msg) - need to call ::Init(x, y, width, height) before");
	R_ASSERT2(GetHeight(), "CUIPdaKillMessage::Init(msg) - need to call ::Init(x, y, width, height) before");
#endif	

	float x		= 0;
	float width	= 0;

	width = InitText(m_killer_name, x, msg.m_killer);		x += width ? width + INDENT : 0;
	width = InitIcon(m_initiator,   x, msg.m_initiator);	x += width ? width + INDENT : 0;
	width = InitText(m_victim_name, x, msg.m_victim);		x += width ? width + INDENT : 0;
			InitIcon(m_ext_info,	x, msg.m_ext_info);

	Fvector2 sz		= GetWndSize();
	sz.x			= _max(sz.x, x+m_ext_info.GetWidth());
	SetWndSize		(sz);
}

float CUIPdaKillMessage::InitText(CUIStatic& refStatic, float x, PlayerInfo& info)
{

	if ( 0 == xr_strlen(info.m_name))
		return 0.0f;

	CGameFont* pFont					= GetFont();
	float _eps							= pFont->SizeOf_(' ');
	UI()->ClientToScreenScaledWidth		(_eps); //add one letter

	float height						= pFont->CurrentHeight_();
	float y								= (GetHeight() - height)/2;

	refStatic.SetWndPos					(Fvector2().set(x, y));
	refStatic.SetHeight					(GetHeight());
	refStatic.SetEllipsis				(1, 0);
	refStatic.SetText					(info.m_name.c_str());
	refStatic.AdjustWidthToText			();
	refStatic.SetWidth					(refStatic.GetWidth()+_eps);
	refStatic.SetTextColor				(info.m_color);

	return		refStatic.GetWidth		();
}

void CUIPdaKillMessage::SetTextColor(u32 color)
{	
	m_victim_name.SetTextColor(subst_alpha(m_victim_name.GetTextColor(),color_get_A(color)));
	m_killer_name.SetTextColor(subst_alpha(m_killer_name.GetTextColor(),color_get_A(color)));
	CUIStatic::SetTextColor(color);
}

void CUIPdaKillMessage::SetColor(u32 color){	
	m_initiator.SetColor(color);
	m_ext_info.SetColor(color);
	CUIStatic::SetColor(color);
}

float CUIPdaKillMessage::InitIcon(CUIStatic& refStatic, float x, IconInfo& info){
	if ( 0 == info.m_rect.width())
		return 0;

	//if (info.m_shader == NULL)
	if (!info.m_shader->inited())
		return 0;

	float		y = 0;
	float		selfHeight = GetHeight();
	float		scale = 0;
	Frect		rect = info.m_rect;

	float width = rect.width();
	float height = rect.height();
	
	scale = selfHeight/height;
	if (scale > 1)
		scale = 1;
	width  = width*scale;
	height = height*scale;
	y = (selfHeight - height) /2;

	refStatic.SetWndPos(Fvector2().set(x,y));
	refStatic.SetWndSize(Fvector2().set(width,height));

	refStatic.SetOriginalRect(info.m_rect);
	refStatic.SetShader(info.m_shader);
	refStatic.SetStretchTexture(true);

	return width;
}

void CUIPdaKillMessage::SetFont(CGameFont* pFont){
	m_victim_name.SetFont(pFont);
	m_killer_name.SetFont(pFont);
	CUIStatic::SetFont(pFont);
}