#include "stdafx.h"
#include "uiscrollbox.h"
#include "../uicursor.h"

CUIScrollBox::CUIScrollBox()
{
	m_bIsHorizontal			= true;
}

void CUIScrollBox::SetHorizontal()
{
	m_bIsHorizontal = true;
}

void CUIScrollBox::SetVertical()
{
	m_bIsHorizontal = false;
}

bool CUIScrollBox::OnMouse(float x, float y, EUIMessages mouse_action)
{
	Fvector2	border;
	if ( m_bIsHorizontal )
	{
		border.x = 512.0f; // :)
		border.y = 512.0f;
	}
	else
	{
		border.x = 512.0f;
		border.y = 512.0f;
	}

	bool over_x = ( x >= -border.x && x < (GetWidth()  + border.x) );
	bool over_y = ( y >= -border.y && y < (GetHeight() + border.y) );
		
	bool cursor_over = false;
	if ( over_x && over_y )
	{
		cursor_over = true;
	}

	bool im_capturer = (GetParent()->GetMouseCapturer() == this);

	if ( mouse_action == WINDOW_LBUTTON_DOWN )
	{
		GetParent()->SetCapture(this, true);
		return true;
	}
	if ( mouse_action == WINDOW_LBUTTON_UP )
	{		
		GetParent()->SetCapture(this, false);
		return true;
	}
	
	if(im_capturer && mouse_action == WINDOW_MOUSE_MOVE && cursor_over)
	{
		Fvector2	pos		= GetWndPos();
		Fvector2	delta	= GetUICursor()->GetCursorPositionDelta();

		if(m_bIsHorizontal)
			pos.x				+= delta.x;
		else
			pos.y				+= delta.y;

		SetWndPos			(pos);

		GetMessageTarget()->SendMessage(this, SCROLLBOX_MOVE);
	}

	if ( !cursor_over )
	{
		GetParent()->SetCapture(this, false);
	}
	return true;
}

void CUIScrollBox::Draw()
{
	if(m_bIsHorizontal){
		if (m_UIStaticItem.GetOriginalRect().width())
		{
			int tile		= iFloor(GetWidth()/m_UIStaticItem.GetOriginalRect().width());
			float rem		= GetWidth()-tile*m_UIStaticItem.GetOriginalRect().width();
			m_UIStaticItem.SetTile(tile,1,rem,0);
		}
	}else{
		if (m_UIStaticItem.GetOriginalRect().height())
		{
			int tile		= iFloor(GetHeight()/m_UIStaticItem.GetOriginalRect().height());
			float rem		= GetHeight()-tile*m_UIStaticItem.GetOriginalRect().height();
			m_UIStaticItem.SetTile(1,tile,0,rem);
		}
	}
	 inherited::Draw();
}