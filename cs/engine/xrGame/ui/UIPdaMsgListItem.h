#pragma once
#include "UIStatic.h"
#include "../InventoryOwner.h"

class CUIPdaMsgListItem : public CUIStatic
{
public:
			void		InitPdaMsgListItem				(Fvector2 pos, Fvector2 size);
//	virtual void		InitCharacterInv				(CInventoryOwner* pInvOwner);
	virtual void		SetTextColor					(u32 color);
	virtual void		SetFont							(CGameFont* pFont);
	virtual void		SetColor						(u32 color);
	
	//информация о персонаже
	CUIStatic			UIIcon;
//	CUIStatic			UIName;
	CUIStatic			UITimeText;
	CUIStatic			UICaptionText;
	CUIStatic			UIMsgText;
};