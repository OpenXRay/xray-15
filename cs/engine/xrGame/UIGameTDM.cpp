#include "stdafx.h"
#include "UIGameTDM.h"

//. #include "UITDMPlayerList.h"
//.#include "UITDMFragList.h"
#include "UIDMStatisticWnd.h"

#include "hudmanager.h"
#include "game_cl_base.h"

#include "game_cl_TeamDeathmatch.h"

#include "ui/UIFrags2.h"
#include "ui/TeamInfo.h"
#include <dinput.h>

#include "object_broker.h"

#include "UITeamPanels.h"

#define MSGS_OFFS 510
#define TEAM_PANELS_TDM_XML_NAME "ui_team_panels_tdm.xml"

//--------------------------------------------------------------------
CUIGameTDM::CUIGameTDM()
{
	m_game					= NULL;
	m_pUITeamSelectWnd		= xr_new<CUISpawnWnd>	();
	m_team1_icon			= NULL;
	m_team2_icon			= NULL;
	m_team1_score			= NULL;
	m_team2_score			= NULL;
}

//--------------------------------------------------------------------
void CUIGameTDM::SetClGame (game_cl_GameState* g)
{
	inherited::SetClGame(g);
	m_game = smart_cast<game_cl_TeamDeathmatch*>(g);
	R_ASSERT(m_game);
}

void CUIGameTDM::Init ()
{
	CUIXml							uiXml, xml2;
	uiXml.Load						(CONFIG_PATH, UI_PATH, "ui_game_tdm.xml");

	m_team1_icon					= xr_new<CUIStatic>();
	m_team2_icon					= xr_new<CUIStatic>();
	m_team1_score					= xr_new<CUIStatic>();
	m_team2_score					= xr_new<CUIStatic>();
	CUIXmlInit::InitStatic			(uiXml, "team1_icon", 0,	m_team1_icon);
	CUIXmlInit::InitStatic			(uiXml, "team2_icon", 0,	m_team2_icon);
	CUIXmlInit::InitStatic			(uiXml, "team1_score", 0,	m_team1_score);
	CUIXmlInit::InitStatic			(uiXml, "team2_score", 0,	m_team2_score);


	xml2.Load(CONFIG_PATH, UI_PATH, "stats.xml");

	CUIFrags2* pFragList		= xr_new<CUIFrags2>();			pFragList->SetAutoDelete(true);
	//-----------------------------------------------------------
	CUIDMStatisticWnd* pStatisticWnd = xr_new<CUIDMStatisticWnd>(); pStatisticWnd->SetAutoDelete(true);

	pFragList->Init(xml2,"stats_wnd","frag_wnd_tdm");
	m_pTeamPanels->Init(TEAM_PANELS_TDM_XML_NAME, "team_panels_wnd");

	float ScreenW = UI_BASE_WIDTH;
	float ScreenH = UI_BASE_HEIGHT;
	//-----------------------------------------------------------
	Frect FrameRect = pFragList->GetWndRect ();
	float FrameW	= FrameRect.right - FrameRect.left;
	float FrameH	= FrameRect.bottom - FrameRect.top;

	pFragList->SetWndPos(Fvector2().set((ScreenW-FrameW)/2.0f, (ScreenH - FrameH)/2.0f));
	//-----------------------------------------------------------
	m_pFragLists->AttachChild(pFragList);
	//-----------------------------------------------------------
	CUIFrags2* pPlayerListT1	= xr_new<CUIFrags2>	();pPlayerListT1->SetAutoDelete(true);
//	CUIFrags2* pPlayerListT2	= xr_new<CUIFrags2>	();pPlayerListT2->SetAutoDelete(true);

	pPlayerListT1->Init(xml2, "players_wnd", "frag_wnd_tdm");
//	pPlayerListT2->Init(xml_doc, "players_wnd", "frag_wnd_tdm");
	//-----------------------------------------------------------
	FrameRect = pPlayerListT1->GetWndRect ();
	FrameW	= FrameRect.right - FrameRect.left;
	FrameH	= FrameRect.bottom - FrameRect.top;

	pPlayerListT1->SetWndPos(Fvector2().set((ScreenW-FrameW)/2.0f, (ScreenH - FrameH)/2.0f));
	//-----------------------------------------------------------
//	FrameRect = pPlayerListT2->GetWndRect ();
//	FrameW	= FrameRect.right - FrameRect.left;
//	FrameH	= FrameRect.bottom - FrameRect.top;

//	pPlayerListT2->SetWndRect(ScreenW/4.0f*3.0f-FrameW/2.0f, (ScreenH - FrameH)/2.0f, FrameW, FrameH);
	//-----------------------------------------------------------
	m_pPlayerLists->AttachChild(pPlayerListT1);
//	m_pPlayerLists->AttachChild(pPlayerListT2);
	//-----------------------------------------------------------
	FrameRect = pStatisticWnd->GetFrameRect ();
	FrameW	= FrameRect.right - FrameRect.left;
	FrameH	= FrameRect.bottom - FrameRect.top;
	pStatisticWnd->SetWndRect(Frect().set((ScreenW-FrameW)/2.0f, (ScreenH - FrameH)/2.0f, FrameW, FrameH));

	m_pStatisticWnds->AttachChild(pStatisticWnd);
}
//--------------------------------------------------------------------
CUIGameTDM::~CUIGameTDM()
{
	xr_delete			(m_team1_icon);
	xr_delete			(m_team2_icon);
	xr_delete			(m_team1_score);
	xr_delete			(m_team2_score);

	delete_data			(m_pUITeamSelectWnd);
}
//--------------------------------------------------------------------
bool CUIGameTDM::IR_OnKeyboardPress(int dik)
{
	switch (dik) {
		case DIK_CAPSLOCK :
		{
			if (m_game)
			{
				if (m_game->Get_ShowPlayerNamesEnabled())
					m_game->Set_ShowPlayerNames( !m_game->Get_ShowPlayerNames() );
				else
					m_game->Set_ShowPlayerNames(true);
				return true;
			};
		}break;
	}
	if(inherited::IR_OnKeyboardPress(dik)) return true;
	return false;
}

bool CUIGameTDM::IR_OnKeyboardRelease(int dik)
{
	switch (dik) {
		case DIK_CAPSLOCK :
			{
				if (m_game)
				{
					if (!m_game->Get_ShowPlayerNamesEnabled())						
						m_game->Set_ShowPlayerNames(false);
					return true;
				};
			}break;
	}
	if(inherited::IR_OnKeyboardRelease(dik)) return true;
	
	return false;
}

void CUIGameTDM::OnFrame()
{
	inherited::OnFrame();
	m_team1_icon->Update();
	m_team2_icon->Update();
	m_team1_score->Update();
	m_team2_score->Update();
}

void CUIGameTDM::Render()
{
	m_team1_icon->Draw();
	m_team2_icon->Draw();
	inherited::Render();
	m_team1_score->Draw();
	m_team2_score->Draw();
}

void CUIGameTDM::SetScoreCaption(int t1, int t2)
{
	string32				str;
	sprintf_s					(str,"%d", t1);
	m_team1_score->SetText	(str);

	sprintf_s					(str,"%d", t2);
	m_team2_score->SetText	(str);
	
	m_pTeamPanels->SetArtefactsCount(t1, t2);
}

void CUIGameTDM::SetFraglimit(int local_frags, int fraglimit)
{
	string64 str;
	if(fraglimit)
		sprintf_s(str,"%d", fraglimit);
	else
		sprintf_s(str,"%s", "--");

	m_pFragLimitIndicator->SetText(str);
}

void CUIGameTDM::reset_ui				()
{
	inherited::reset_ui();
	m_pUITeamSelectWnd->Reset();
}