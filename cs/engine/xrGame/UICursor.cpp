#include "stdafx.h"
#include "uicursor.h"

#include <xrEngine/CustomHUD.h>
#include "UI.h"
#include "HUDManager.h"
#include "ui/UIStatic.h"


#define C_DEFAULT	color_xrgb(0xff,0xff,0xff)

CUICursor::CUICursor()
:m_static(NULL)
{    
	bVisible				= false;
	vPos.set				(0.f,0.f);
	InitInternal			();
	Device.seqRender.Add	(this,2);
}
//--------------------------------------------------------------------
CUICursor::~CUICursor	()
{
	xr_delete				(m_static);
	Device.seqRender.Remove	(this);
}

void CUICursor::OnScreenRatioChanged()
{
	xr_delete					(m_static);
	InitInternal				();
}

void CUICursor::InitInternal()
{
	m_static					= new CUIStatic();
	m_static->InitTextureEx		("ui\\ui_ani_cursor", "hud\\cursor");
	Frect						rect;
	rect.set					(0.0f,0.0f,40.0f,40.0f);
	m_static->SetOriginalRect	(rect);
	Fvector2					sz;
	sz.set						(rect.rb);
	if(UI()->is_16_9_mode())
		sz.x					/= 1.2f;

	m_static->SetWndSize		(sz);
	m_static->SetStretchTexture	(true);
}

//--------------------------------------------------------------------
u32 last_render_frame = 0;
void CUICursor::OnRender	()
{
	if( !IsVisible() ) return;
#ifdef DEBUG
	VERIFY(last_render_frame != Device.dwFrame);
	last_render_frame = Device.dwFrame;

	if(bDebug)
	{
	CGameFont* F		= UI()->Font()->pFontDI;
	F->SetAligment		(CGameFont::alCenter);
	F->SetHeightI		(0.02f);
	F->OutSetI			(0.f,-0.9f);
	F->SetColor			(0xffffffff);
	Fvector2			pt = GetCursorPosition();
	F->OutNext			("%f-%f",pt.x, pt.y);
	}
#endif

	m_static->SetWndPos	(vPos);
	m_static->Update	();
	m_static->Draw		();
}

Fvector2 CUICursor::GetCursorPosition()
{
	return  vPos;
}

Fvector2 CUICursor::GetCursorPositionDelta()
{
	Fvector2 res_delta;

	res_delta.x = vPos.x - vPrevPos.x;
	res_delta.y = vPos.y - vPrevPos.y;
	return res_delta;
}

void CUICursor::UpdateCursorPosition()
{

	POINT		p;
	BOOL r		= GetCursorPos(&p);
	R_ASSERT	(r);

	vPrevPos = vPos;

	vPos.x			= (float)p.x * (UI_BASE_WIDTH/(float)Device.dwWidth);
	vPos.y			= (float)p.y * (UI_BASE_HEIGHT/(float)Device.dwHeight);
	clamp			(vPos.x, 0.f, UI_BASE_WIDTH);
	clamp			(vPos.y, 0.f, UI_BASE_HEIGHT);
}

void CUICursor::SetUICursorPosition(Fvector2 pos)
{
	vPos		= pos;
	POINT		p;
	p.x			= iFloor(vPos.x / (UI_BASE_WIDTH/(float)Device.dwWidth));
	p.y			= iFloor(vPos.y / (UI_BASE_HEIGHT/(float)Device.dwHeight));

	SetCursorPos(p.x, p.y);
}
