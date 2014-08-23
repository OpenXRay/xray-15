#include "stdafx.h"
#include "uiscrollbar.h"
#include "UI3tButton.h"
#include "UIScrollBox.h"
#include "UIXmlInit.h"
#include "UITextureMaster.h"

CUIScrollBar::CUIScrollBar()
{
	m_iMinPos			= 1;
	m_iMaxPos			= 1;
	m_iPageSize			= 1;
	m_iStepSize			= 1;
	m_iScrollPos		= 0;
	m_hold_delay		= 50.0f;
	m_b_enabled			= true;
	m_mouse_state		= 0;

	m_DecButton			= xr_new<CUI3tButton>();	m_DecButton->SetAutoDelete(true); AttachChild(m_DecButton);
	m_IncButton			= xr_new<CUI3tButton>();	m_IncButton->SetAutoDelete(true); AttachChild(m_IncButton);
	m_ScrollBox			= xr_new<CUIScrollBox>();	m_ScrollBox->SetAutoDelete(true); AttachChild(m_ScrollBox);
	m_StaticBackground	= xr_new<CUIStaticItem>();
}

CUIScrollBar::~CUIScrollBar(void)
{
	xr_delete(m_StaticBackground);
}

void CUIScrollBar::InitScrollBar(Fvector2 pos, float length, bool bIsHorizontal, LPCSTR profile)
{
	string256 _path;
	CUIXml xml_doc;
	xml_doc.Load	(CONFIG_PATH, UI_PATH, "scroll_bar.xml");

	float height = xml_doc.ReadAttribFlt(profile, 0, (bIsHorizontal)?"height":"height_v", 16);
	m_hold_delay = xml_doc.ReadAttribFlt(profile, 0, "hold_delay", 50.0f);
    	
	m_bIsHorizontal = bIsHorizontal;
	if(m_bIsHorizontal)
	{
		inherited::SetWndPos		(pos);
		inherited::SetWndSize		(Fvector2().set(length, height));

        strconcat					(sizeof(_path),_path, profile, ":left_arrow");
		CUIXmlInit::Init3tButton	(xml_doc, _path, 0, m_DecButton);
		m_DecButton->SetWndPos		(Fvector2().set(0,0));

		strconcat					(sizeof(_path),_path, profile, ":right_arrow");
		CUIXmlInit::Init3tButton	(xml_doc, _path, 0, m_IncButton);
		m_IncButton->SetWndPos		(Fvector2().set(length - m_IncButton->GetWidth(), 0.0f));

		m_ScrollBox->SetHorizontal	();

		strconcat					(sizeof(_path),_path, profile, ":box");
		CUIXmlInit::InitStatic		(xml_doc, _path, 0, m_ScrollBox);
		m_IncButton->SetWndPos		(Fvector2().set(0.0f, length/2));

		strconcat					(sizeof(_path),_path, profile, ":back:texture");
		LPCSTR texture				= xml_doc.Read(_path, 0, "");
		R_ASSERT					(texture);
		CUITextureMaster::InitTexture(texture, m_StaticBackground);
		m_ScrollWorkArea			= _max(0,iFloor(GetWidth()-2*height));
	}else{
		inherited::SetWndPos		(pos);
		inherited::SetWndSize		(Fvector2().set(height, length));

		strconcat					(sizeof(_path),_path, profile, ":up_arrow");
		CUIXmlInit::Init3tButton	(xml_doc, _path, 0, m_DecButton);
		m_DecButton->SetWndPos		(Fvector2().set(0,0));

		strconcat					(sizeof(_path),_path, profile, ":down_arrow");
 		CUIXmlInit::Init3tButton	(xml_doc, _path, 0, m_IncButton);
		m_IncButton->SetWndPos		(Fvector2().set(0.0f, length - height));

		m_ScrollBox->SetVertical	();

		strconcat					(sizeof(_path),_path, profile, ":box_v");
		CUIXmlInit::InitStatic		(xml_doc, _path, 0, m_ScrollBox);		
		strconcat					(sizeof(_path),_path, profile, ":back_v:texture");
		LPCSTR texture				= xml_doc.Read(_path, 0, "");
		R_ASSERT					(texture);

		CUITextureMaster::InitTexture(texture, m_StaticBackground);
		m_ScrollWorkArea			= _max(0,iFloor(GetHeight()-2*height));
	}	

	UpdateScrollBar					();
}


//корректировка размеров скроллера
void CUIScrollBar::SetWidth(float width)
{
	if(width<=0.0f) width = 1.0f;
	inherited::SetWidth(width);
}

void CUIScrollBar::SetHeight(float height)
{
	if(height<=0.0f) height = 1.0f;
	inherited::SetHeight(height);
	UpdateScrollBar();
}

void CUIScrollBar::SetStepSize(int step)
{
	m_iStepSize				= step;
	UpdateScrollBar			();
}


void CUIScrollBar::SetRange(int iMin, int iMax) 
{
	m_iMinPos				= iMin;  
	m_iMaxPos				= iMax;
	VERIFY					(iMax>=iMin);
	if(iMax<iMin)			iMax=iMin;
	UpdateScrollBar			();
}
void CUIScrollBar::Show(bool b)
{
	if(!m_b_enabled)return;
	inherited::Show(b);
}

void CUIScrollBar::Enable(bool b)
{
	if(!m_b_enabled)return;
	inherited::Enable(b);
}

void CUIScrollBar::UpdateScrollBar()
{
	if (IsShown())
	{
		//уcтановить размер и положение каретки
		if(m_iMaxPos==m_iMinPos)	m_iMaxPos++;
		float box_sz				= float(m_ScrollWorkArea)*float(m_iPageSize ? m_iPageSize : 1)/float(m_iMaxPos-m_iMinPos);
		if(IsRelevant())
		{
			if(m_bIsHorizontal)
			{
				// set width
				clamp					(box_sz,_min(GetHeight(),GetWidth() - m_IncButton->GetWidth() - m_DecButton->GetWidth()),GetWidth() - m_IncButton->GetWidth() - m_DecButton->GetWidth());
				m_ScrollBox->SetWidth	(box_sz);
				m_ScrollBox->SetHeight	(GetHeight());
				// set pos
				int pos					= PosViewFromScroll(iFloor(m_ScrollBox->GetWidth()),iFloor(GetHeight()));
				m_ScrollBox->SetWndPos	(Fvector2().set(float(pos), m_ScrollBox->GetWndRect().top));
				m_IncButton->SetWndPos	(Fvector2().set(GetWidth() - m_IncButton->GetWidth(), 0.0f));
			}else
			{
				// set height
				clamp					(box_sz,_min(GetWidth(),GetHeight()-m_IncButton->GetHeight() - m_DecButton->GetHeight()),GetHeight()-m_IncButton->GetHeight() - m_DecButton->GetHeight());
				m_ScrollBox->SetHeight	(box_sz);
				m_ScrollBox->SetWidth	(GetWidth());
				// set pos
				int pos				= PosViewFromScroll(iFloor(m_ScrollBox->GetHeight()),iFloor(GetWidth()));
				m_ScrollBox->SetWndPos	(Fvector2().set(m_ScrollBox->GetWndRect().left, float(pos)));
				m_IncButton->SetWndPos	(Fvector2().set(0.0f, GetHeight() - m_IncButton->GetHeight()));
			}
		}
	}
	if(IsRelevant())
	{
		m_ScrollBox->SetColor(color_rgba(255,255,255,255));
	}else
	{
		m_ScrollBox->SetColor(color_rgba(255,255,255,0));
	}

	ClampByViewRect();
}

#include "../uicursor.h"
u32 last_hold_time =0;

bool CUIScrollBar::OnKeyboardHold(int dik)
{
	if ( dik == MOUSE_1 && (last_hold_time + m_hold_delay) < Device.dwTimeContinual)//100
	{
		if ( OnMouseDownEx() )
		{
			last_hold_time = Device.dwTimeContinual;
			return true;
		}
	}
	return inherited::OnKeyboardHold(dik);
}

bool CUIScrollBar::OnMouse(float x, float y, EUIMessages mouse_action)
{
	switch ( mouse_action )
	{
		case WINDOW_MOUSE_WHEEL_DOWN:
			TryScrollInc();
			return true;
			break;
		case WINDOW_MOUSE_WHEEL_UP:
			TryScrollDec();
			return true;
			break;
		case WINDOW_LBUTTON_UP:
			m_mouse_state = 0;
			break;
	};

	return inherited::OnMouse(x, y, mouse_action);
}

bool CUIScrollBar::OnMouseDown( int mouse_btn )
{
	if ( mouse_btn == MOUSE_1 )
	{
		if ( OnMouseDownEx() )
		{
			return true;
		}
	}
	return inherited::OnMouseDown(mouse_btn);
}
bool CUIScrollBar::OnMouseDownEx()
{
	Fvector2 cursor_pos = GetUICursor()->GetCursorPosition();
	Frect    box_rect, dec_rect, inc_rect;
	m_ScrollBox->GetAbsoluteRect(box_rect);
	m_DecButton->GetAbsoluteRect(dec_rect);
	m_IncButton->GetAbsoluteRect(inc_rect);

	if ( dec_rect.in(cursor_pos) && (m_mouse_state != 2) )
	{
		TryScrollDec();
		m_mouse_state = 1;
		return true;
	}
	
	if ( inc_rect.in(cursor_pos) && (m_mouse_state != 1) )
	{
		TryScrollInc();
		m_mouse_state = 2;
		return true;
	}
	
	Frect   dec2_rect, inc2_rect;
	if ( m_bIsHorizontal )
	{
		dec2_rect.set( dec_rect.x2, dec_rect.y1, box_rect.x1, box_rect.y2 );
		inc2_rect.set( box_rect.x2, box_rect.y1, inc_rect.x1, inc_rect.y2 );
	}
	else
	{
		dec2_rect.set( dec_rect.x1, dec_rect.y2, box_rect.x2, box_rect.y1 );
		inc2_rect.set( box_rect.x1, box_rect.y2, inc_rect.x2, inc_rect.y1 );
	}
		
	if ( dec2_rect.in(cursor_pos) && (m_mouse_state != 2) )
	{
		TryScrollDec();
		m_mouse_state = 1;
		return true;
	}
	
	if ( inc2_rect.in(cursor_pos) && (m_mouse_state != 1) )
	{
		TryScrollInc();
		m_mouse_state = 2;
		return true;
	}
	return false;
}

void CUIScrollBar::OnMouseUp( int mouse_btn )
{
	m_mouse_state = 0;
}

void CUIScrollBar::ClampByViewRect()
{
	if(m_bIsHorizontal){
		if(m_ScrollBox->GetWndRect().left <= m_DecButton->GetWidth())
			m_ScrollBox->SetWndPos	(Fvector2().set(m_DecButton->GetWidth(), m_ScrollBox->GetWndRect().top));
		else if(m_ScrollBox->GetWndRect().right >= m_IncButton->GetWndPos().x)
			m_ScrollBox->SetWndPos	(Fvector2().set(m_IncButton->GetWndRect().left - m_ScrollBox->GetWidth(), 
									m_ScrollBox->GetWndRect().top));
	}else{
		// limit vertical position (TOP) by position of button	
		if(m_ScrollBox->GetWndRect().top <= m_DecButton->GetHeight())
			m_ScrollBox->SetWndPos	(Fvector2().set(m_ScrollBox->GetWndRect().left, 
									m_DecButton->GetHeight()));
		// limit vertical position (BOTTOM) by position of button
		else if(m_ScrollBox->GetWndRect().bottom >= m_IncButton->GetWndPos().y)
			m_ScrollBox->SetWndPos	(Fvector2().set(m_ScrollBox->GetWndRect().left,
									m_IncButton->GetWndPos().y - m_ScrollBox->GetHeight()));
	}
}

void CUIScrollBar::SetPosScrollFromView(float view_pos, float view_size, float view_offs)
{
	int scroll_size	= ScrollSize();
	float pos			= view_pos-view_offs;
	float work_size	= m_ScrollWorkArea-view_size;
	SetScrollPosClamped	(work_size?iFloor(((pos/work_size)*(scroll_size) + m_iMinPos)):0);
}

int CUIScrollBar::PosViewFromScroll(int view_size, int view_offs)
{
	int work_size	= m_ScrollWorkArea-view_size;
	int scroll_size	= ScrollSize();
	return			scroll_size?(m_iScrollPos*work_size+scroll_size*view_offs-m_iMinPos*work_size)/scroll_size:0;
}

void CUIScrollBar::SetScrollPosClamped(int iPos)
{ 
	m_iScrollPos = iPos; 
	clamp(m_iScrollPos,m_iMinPos,m_iMaxPos-m_iPageSize+1);
}

void CUIScrollBar::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if ( pWnd == m_DecButton )
	{
		if ( msg == BUTTON_CLICKED || msg == BUTTON_DOWN )
		{
			TryScrollDec();
		}
	}
	else if ( pWnd == m_IncButton )
	{
		if ( msg == BUTTON_CLICKED || msg == BUTTON_DOWN )
		{
			TryScrollInc();
		}
	}
	else if ( pWnd == m_ScrollBox )
	{
		if ( msg == SCROLLBOX_MOVE )
		{
			//вычислить новое положение прокрутки
			ClampByViewRect();
			if ( m_bIsHorizontal )
			{
				SetPosScrollFromView(m_ScrollBox->GetWndPos().x,m_ScrollBox->GetWidth(),GetHeight());
				if (GetMessageTarget())
					GetMessageTarget()->SendMessage(this, SCROLLBAR_HSCROLL);
			}
			else
			{
				SetPosScrollFromView(m_ScrollBox->GetWndPos().y,m_ScrollBox->GetHeight(),GetWidth());
				if (GetMessageTarget())
					GetMessageTarget()->SendMessage(this, SCROLLBAR_VSCROLL);
			}
		}
	}
	CUIWindow::SendMessage( pWnd, msg, pData );
}

void CUIScrollBar::TryScrollInc()
{
	if(ScrollInc())
		if(m_bIsHorizontal)
			GetMessageTarget()->SendMessage(this, SCROLLBAR_HSCROLL);
		else
			GetMessageTarget()->SendMessage(this, SCROLLBAR_VSCROLL);
}

void CUIScrollBar::TryScrollDec()
{
	if(ScrollDec())
		if(m_bIsHorizontal)
			GetMessageTarget()->SendMessage(this, SCROLLBAR_HSCROLL);
		else
			GetMessageTarget()->SendMessage(this, SCROLLBAR_VSCROLL);

}


bool CUIScrollBar::ScrollDec()
{
	if(m_iScrollPos>m_iMinPos)
	{
		if(m_iScrollPos>m_iStepSize)
			SetScrollPos	(m_iScrollPos-m_iStepSize);
		else
			SetScrollPos	(0);

		return true;
	}

	return false;
}


bool CUIScrollBar::ScrollInc()
{
	if(m_iScrollPos<=(m_iMaxPos-m_iPageSize+1))
	{
		SetScrollPos	(m_iScrollPos+m_iStepSize);
		return true;
	}

	return false;	
}

void CUIScrollBar::Reset()
{
	ResetAll			();
	inherited::Reset	();
}

bool CUIScrollBar::IsRelevant()
{
	bool b_can_inc = (m_iScrollPos <= (m_iMaxPos - m_iPageSize));
	bool b_can_dec = (m_iScrollPos > m_iMinPos);
	return b_can_inc || b_can_dec;
}

void CUIScrollBar::Draw()
{
	//нарисовать фоновую подложку
	Frect rect;
	GetAbsoluteRect(rect);
	if(m_bIsHorizontal)
	{
		if (m_StaticBackground->GetOriginalRect().width()){
			float size	= GetWidth() - m_DecButton->GetWidth() - m_IncButton->GetWidth();
			float w		= m_StaticBackground->GetOriginalRect().width();

			int tile	= iFloor(size/w);
			float rem	= size - tile*w;

			m_StaticBackground->SetTile(tile,1,rem,0.0f);
			m_StaticBackground->SetPos(rect.left + m_DecButton->GetWidth(),rect.top);
		}
	}else{
		if (m_StaticBackground->GetOriginalRect().height()){
			float size	= GetHeight()- m_IncButton->GetHeight() - m_DecButton->GetHeight();
			float h		= m_StaticBackground->GetOriginalRect().height();

			int tile	= iFloor(size/h);
			float rem	= size - tile*h;

			m_StaticBackground->SetTile(1,tile,0.0f,rem);
			m_StaticBackground->SetPos(rect.left,rect.top + m_DecButton->GetHeight());
		}
	}
	
	m_StaticBackground->Render();

	inherited::Draw();
}

void CUIScrollBar::Refresh()
{
	SendMessage(m_ScrollBox, SCROLLBOX_MOVE, NULL);
}

