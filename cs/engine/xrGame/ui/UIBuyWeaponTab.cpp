#include "StdAfx.h"
#include "UIBuyWeaponTab.h"
#include "UITabButtonMP.h"
#include "../string_table.h"
#include "../HUDManager.h"
#include "../UI.h"
#include "UIXmlInit.h"


// CUIBuyWeaponTab::CUIBuyWeaponTab()
// {
// 	m_bActiveState		= true;
// }
// 
// CUIBuyWeaponTab::~CUIBuyWeaponTab()
// {}
// 
// void CUIBuyWeaponTab::Init(CUIXml* xml, char* path)
// {
// 
// 	R_ASSERT3					(xml->NavigateToNode(path,0), "XML node not found", path);
// 	
// 	CUIXmlInit::InitWindow		(*xml, path, 0, this);
// 	int tabsCount				= xml->GetNodesNum(path, 0, "button");
// 
// 	XML_NODE* tab_node			= xml->NavigateToNode(path,0);
// 	xml->SetLocalRoot			(tab_node);
// 
// 	for (int i = 0; i < tabsCount; ++i)
// 	{
// 		CUITabButtonMP *newButton	= xr_new<CUITabButtonMP>();
// 		CUIXmlInit::Init3tButton	(*xml, "button", i, newButton);
// 		newButton->m_btn_id			= xml->ReadAttrib("button",i,"id");
// 		R_ASSERT					(newButton->m_btn_id.size());
// 		AddItem						(newButton);
// 	}
// 
// 	CUITabButtonMP *stubButton		= xr_new<CUITabButtonMP>();
// 	stubButton->m_btn_id			= "stub";
// 	AddItem							(stubButton);
// 	m_sStubId						= stubButton->m_btn_id;
// 
// 	SetActiveTab					(m_sStubId);
// 	
// 	xml->SetLocalRoot				(xml->GetRoot());
// 
// 	SetActiveState					();
// }
// 
// void CUIBuyWeaponTab::OnTabChange(const shared_str& sCur, const shared_str& sPrev)
// {
// 	CUITabControl::OnTabChange		(sCur, sPrev);
// 
// 	if (m_sStubId != sCur)
//         SetActiveState				(false);	
// }
// 
// void CUIBuyWeaponTab::SetActiveState(bool bState)
// {
// 	m_bActiveState					= bState;
// 
// 	WINDOW_LIST::iterator it		= m_ChildWndList.begin();
// 
// 	for(u32 i=0; i<m_ChildWndList.size(); ++i, ++it)
// 		(*it)->Enable(bState);
// 
// 	if (bState)
// 		SetActiveTab				(m_sStubId);
// 
// 	GetButtonById(m_sPushedId)->Enable(true);
// }

void CUIBuyWeaponTab::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if (TAB_CHANGED == msg)
	{
		for (u32 i = 0; i < m_TabsArr.size(); ++i)
		{
			if (m_TabsArr[i] == pWnd)
			{
				m_sPushedId = m_TabsArr[i]->m_btn_id;

				OnTabChange(m_sPushedId, m_sPrevPushedId);
				m_sPrevPushedId = m_sPushedId;							
				break;
			}
		}

		return;
	}

	return inherited::SendMessage(pWnd, msg, pData);
}
