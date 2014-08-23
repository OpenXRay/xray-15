// CUIEditBox.cpp: ввод строки с клавиатуры
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <dinput.h>
#include "uieditbox.h"
#include "../HUDManager.h"
#include "UIColorAnimatorWrapper.h"


CUIEditBox::CUIEditBox()
{
	AttachChild(&m_frameLine);
}

CUIEditBox::~CUIEditBox(void)
{
}	

void CUIEditBox::InitCustomEdit(Fvector2 pos, Fvector2 size)
{
	m_frameLine.SetWndPos			(Fvector2().set(0,0));
	m_frameLine.SetWndSize			(size);
	CUICustomEdit::InitCustomEdit	(pos, size);
}

void CUIEditBox::InitTextureEx(LPCSTR texture, LPCSTR  shader)
{
	m_frameLine.InitTexture(texture, shader);
}

void CUIEditBox::InitTexture(LPCSTR texture)
{
	InitTextureEx(texture, "hud\\default");
}

void CUIEditBox::SetCurrentValue(){
	SetText(GetOptStringValue());
}

void CUIEditBox::SaveValue(){
	CUIOptionsItem::SaveValue();
	SaveOptStringValue(GetText());
}

bool CUIEditBox::IsChanged(){
	return 0 != xr_strcmp(GetOptStringValue(),GetText());
}



