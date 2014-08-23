#include "stdafx.h"
#include ".\uiradiobutton.h"
#include "UILines.h"


void CUIRadioButton::InitButton(Fvector2 pos, Fvector2 size)
{
	inherited::InitButton(pos, size);

	if (!m_pLines)
		m_pLines = xr_new<CUILines>();
	m_pLines->SetTextAlignment(CGameFont::alLeft);

    CUI3tButton::InitTexture("ui_radio");
	Frect r = m_background->GetE()->GetStaticItem()->GetRect(); 
	CUI3tButton::SetTextX(r.width());

	CUI3tButton::InitButton(pos, Fvector2().set(size.x, r.height()-5.0f));

	m_pLines->SetWndPos(pos);
	m_pLines->SetWndSize(Fvector2().set(size.x,m_background->GetE()->GetStaticItem()->GetRect().height()));
}

void CUIRadioButton::InitTexture(LPCSTR tex_name){
	// do nothing
}
