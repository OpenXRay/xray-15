#include "StdAfx.h"
#include "UITabButtonMP.h"
#include "../HUDManager.h"

CUITabButtonMP::CUITabButtonMP()
{
	m_orientationVertical		= true;
	m_text_ident_cursor_over.set(0,0);
	m_text_ident_normal.set		(0,0);
}


void CUITabButtonMP::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	if (this == pWnd)
		m_bIsEnabled = true;

	CUITabButton::SendMessage(pWnd, msg, pData);
}

void CUITabButtonMP::UpdateTextAlign()
{
	if (m_bCursorOverWindow)
	{
		m_TextOffset	= m_text_ident_cursor_over;
	}else
	{
		m_TextOffset	= m_text_ident_normal;
	}
}


void CUITabButtonMP::Update()
{

	// cunning code : use Highlighting even if is disabled

	bool tempEnabled		= m_bIsEnabled;
	m_bIsEnabled			= m_bCursorOverWindow ? true : m_bIsEnabled;
	inherited::Update		();
	m_bIsEnabled			= tempEnabled;
	UpdateTextAlign			();
}

void CUITabButtonMP::Draw()
{
//.	UI()->PushScissor(UI()->ScreenRect(),true);

	CUITabButton::Draw();

//.	UI()->PopScissor();
}







