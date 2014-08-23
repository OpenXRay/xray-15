//////////////////////////////////////////////////////////////////////
// UIPdaMsgListItem.cpp: элемент окна списка в основном 
// экране для сообщений PDA
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIPdaMsgListItem.h"
#include "../Entity.h"
#include "../../xrServerEntities/character_info.h"
#include "UIInventoryUtilities.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "uicoloranimatorwrapper.h"
#include "object_broker.h"

#define PDA_MSG_MAINGAME_CHAR "maingame_pda_msg.xml"

using namespace InventoryUtilities;


void CUIPdaMsgListItem::SetFont(CGameFont* pFont)
{
	UITimeText.SetFont		(pFont);
	UICaptionText.SetFont	(pFont);
	UIMsgText.SetFont		(pFont);
}

void CUIPdaMsgListItem::InitPdaMsgListItem(Fvector2 pos, Fvector2 size)
{
	CUIStatic::SetWndPos	(pos);
	CUIStatic::SetWndSize	(size);

	CUIXml					uiXml;
	uiXml.Load				(CONFIG_PATH, UI_PATH,PDA_MSG_MAINGAME_CHAR);

	CUIXmlInit				xml_init;
	AttachChild				(&UIIcon);
	xml_init.InitStatic		(uiXml, "icon_static", 0, &UIIcon);

	/*AttachChild(&UIName);
	if(uiXml.NavigateToNode	("name_static",0))
		xml_init.InitStatic	(uiXml, "name_static", 0, &UIName);
	else
	{
		UIName.Show			(false);
		UIName.Enable		(false);
	}*/
	AttachChild				(&UITimeText);
	xml_init.InitStatic		(uiXml, "time_static", 0, &UITimeText);

	AttachChild				(&UICaptionText);
	xml_init.InitStatic		(uiXml, "caption_static", 0, &UICaptionText);

	AttachChild				(&UIMsgText);
	xml_init.InitStatic		(uiXml, "msg_static", 0, &UIMsgText);
}

void CUIPdaMsgListItem::SetTextColor(u32 color)
{
	UITimeText.SetTextColor		(color);
	UICaptionText.SetTextColor	(color);
	UIMsgText.SetTextColor		(color);
}

void CUIPdaMsgListItem::SetColor(u32 color)
{
	UIIcon.SetColor(color);
}

/*
void CUIPdaMsgListItem::InitCharacterInv(CInventoryOwner* pInvOwner)
{
	VERIFY(pInvOwner);

//	string256 str;
//	sprintf_s(str, "name: %s", pInvOwner->Name());
//	UIName.SetText			(str);

	UIIcon.InitTexture		( pInvOwner->CharacterInfo().IconName().c_str() );
/*
	UIIcon.SetShader(GetCharIconsShader());
	UIIcon.GetUIStaticItem().SetOriginalRect(
					float(pInvOwner->CharacterInfo().TradeIconX()*ICON_GRID_WIDTH),
					float(pInvOwner->CharacterInfo().TradeIconY()*ICON_GRID_HEIGHT),
					float(pInvOwner->CharacterInfo().TradeIconX()+CHAR_ICON_WIDTH*ICON_GRID_WIDTH),
					float(pInvOwner->CharacterInfo().TradeIconY()+CHAR_ICON_HEIGHT*ICON_GRID_HEIGHT));
*
}
*/