#include "stdafx.h"
#include "UIGameCTA.h"

//. #include "UITDMPlayerList.h"
//.#include "UITDMFragList.h"
#include <dinput.h>

#include "UITeamPanels.h"
#include "hudmanager.h"
#include "game_cl_base.h"
#include "game_cl_capture_the_artefact.h"
#include "game_cl_mp.h"

#include "Level.h"
#include "Actor.h"
#include "artefact.h"
#include "inventory.h"
#include "xrServer_Objects_ALife_Items.h"
#include "weapon.h"
#include "WeaponMagazinedWGrenade.h"
#include "WeaponKnife.h"
#include "xr_level_controller.h"

#include "object_broker.h"
//#include "clsid_game.h"
#include "weaponknife.h"

#include "ui/UISkinSelector.h"
//.#include "ui/UIInventoryWnd.h"
#include "ui/UIPdaWnd.h"
#include "ui/UIMapDesc.h"
#include "ui/UISpawnWnd.h"
#include "ui/UIBuyWndBase.h"
#include "ui/UIMpTradeWnd.h"
#include "ui/UIBuyWndShared.h"
#include "ui/UIMoneyIndicator.h"
#include "ui/UIRankIndicator.h"
#include "ui/UIProgressShape.h"
#include "ui/UIMultiTextStatic.h"
#include "ui/UIMessageBoxEx.h"
#include "ui/UIVoteStatusWnd.h"
#include "ui/UIActorMenu.h"

#include "ui/UISkinSelector.h"

#define CTA_GAME_WND_XML	"ui_game_cta.xml"

#define TEAM_PANELS_XML_NAME "ui_team_panels_cta.xml"

#define ROUND_RESULT_COLOR		0xfff0fff0
#define NORMAL_MSG_COLOR		0xffffffff
#define BUY_MSG_COLOR			0xffffff00
#define SPECTRMODE_MSG_COLOR	0xffff0000
#define WARM_UP_COLOR			0xff00ff00
#define TIME_MSG_COLOR			0xffff0000
#define DEMOPLAY_COLOR			0xff00ff00

#define DI2PX(x) float(iFloor((x+1)*float(UI_BASE_WIDTH)*0.5f))
#define DI2PY(y) float(iFloor((y+1)*float(UI_BASE_HEIGHT)*0.5f))
#define SZ(x) x*UI_BASE_WIDTH

CUIGameCTA::CUIGameCTA()
{
	teamPanels			= new UITeamPanels();
	m_pUITeamSelectWnd	= new CUISpawnWnd();

//-	m_pActorMenu		= new CUIActorMenu();
//-	m_pPdaMenu			= new CUIPdaWnd();
	
	CUIXml							uiXml;
	uiXml.Load						(CONFIG_PATH, UI_PATH, CTA_GAME_WND_XML);
	m_pMoneyIndicator				= new CUIMoneyIndicator();
	m_pMoneyIndicator->InitFromXML	(uiXml);
	m_pRankIndicator				= new CUIRankIndicator();
	m_pRankIndicator->InitFromXml	(uiXml);
	m_pReinforcementInidcator = new CUIProgressShape();
	CUIXmlInit::InitProgressShape	(uiXml, "reinforcement", 0, m_pReinforcementInidcator);

	m_team1_icon					= new CUIStatic();
	m_team2_icon					= new CUIStatic();
	m_team1_score					= new CUIStatic();
	m_team2_score					= new CUIStatic();

	m_pFragLimitIndicator			= new CUIStatic();
	CUIXmlInit::InitStatic			(uiXml,"fraglimit", 0, m_pFragLimitIndicator);

	CUIXmlInit::InitStatic			(uiXml, "team1_icon", 0,	m_team1_icon);
	CUIXmlInit::InitStatic			(uiXml, "team2_icon", 0,	m_team2_icon);

	CUIXmlInit::InitStatic			(uiXml, "team1_score", 0,	m_team1_score);
	CUIXmlInit::InitStatic			(uiXml, "team2_score", 0,	m_team2_score);

	m_round_result_caption =	"round_result";
	GameCaptions()->addCustomMessage(m_round_result_caption, DI2PX(0.0f), DI2PY(-0.1f), SZ(0.03f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, ROUND_RESULT_COLOR, "");
	m_pressbuy_caption = "pressbuy";
	GameCaptions()->addCustomMessage(m_pressbuy_caption, DI2PX(0.0f), DI2PY(0.95f), SZ(0.02f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, NORMAL_MSG_COLOR, "");
	m_pressjump_caption = "pressjump";
	GameCaptions()->addCustomMessage(m_pressjump_caption, DI2PX(0.0f), DI2PY(0.9f), SZ(0.02f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, NORMAL_MSG_COLOR, "");
	m_spectator_caption = "spectator";
	GameCaptions()->addCustomMessage(m_spectator_caption, DI2PX(0.0f), DI2PY(-0.7f), SZ(0.03f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, NORMAL_MSG_COLOR, "");
	m_spectrmode_caption = "spetatormode";
	GameCaptions()->addCustomMessage(m_spectrmode_caption, DI2PX(0.0f), DI2PY(-0.7f), SZ(0.03f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, SPECTRMODE_MSG_COLOR, "");
	m_warm_up_caption =	"warm_up";
	GameCaptions()->addCustomMessage(m_warm_up_caption, DI2PX(0.0f), DI2PY(-0.75f), SZ(0.05f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, WARM_UP_COLOR, "");
	m_time_caption = "timelimit";
	GameCaptions()->addCustomMessage(m_time_caption, DI2PX(0.0f), DI2PY(-0.8f), SZ(0.03f), HUD().Font().pFontGraffiti19Russian, CGameFont::alCenter, TIME_MSG_COLOR, "");
	m_demo_play_caption = "demo_play";
	GameCaptions()->addCustomMessage(m_demo_play_caption, DI2PX(-1.0f), DI2PY(-0.5f), SZ(0.05f), HUD().Font().pFontGraffiti19Russian, CGameFont::alLeft, DEMOPLAY_COLOR, "");

	m_pMapDesc			= NULL;
	m_pCurBuyMenu		= NULL;
	m_pCurSkinMenu		= NULL;
	m_pBuySpawnMsgBox	= NULL;
	m_game				= NULL;
	m_voteStatusWnd		= NULL;

	m_team_panels_shown = false;
}

CUIGameCTA::~CUIGameCTA()
{
	xr_delete	(m_team1_icon);
	xr_delete	(m_team2_icon);
	xr_delete	(m_team1_score);
	xr_delete	(m_team2_score);

	xr_delete	(teamPanels);
	delete_data	(m_pUITeamSelectWnd);

	delete_data	(m_pMapDesc);
	delete_data	(m_pBuySpawnMsgBox);
	xr_delete	(m_voteStatusWnd);
	xr_delete	(m_pCurBuyMenu);
	xr_delete	(m_pCurSkinMenu);
	
	xr_delete	(m_pMoneyIndicator);
	xr_delete	(m_pRankIndicator);
	xr_delete	(m_pFragLimitIndicator);
	xr_delete	(m_pReinforcementInidcator);
}

void CUIGameCTA::ReInitShownUI()
{
	/*
	if (m_pInventoryMenu && m_pInventoryMenu->IsShown())
	{
		m_pInventoryMenu->InitInventory();
	}
	*/
}

void CUIGameCTA::reset_ui()
{
	inherited::reset_ui();
}

bool CUIGameCTA::IsTeamPanelsShown()
{
	VERIFY(teamPanels);
	return m_team_panels_shown;//teamPanels->IsShown();
}
void CUIGameCTA::ShowTeamPanels(bool bShow)
{
	if (bShow) {
		AddDialogToRender(teamPanels);
	} else {
		RemoveDialogToRender(teamPanels);
	}
	m_team_panels_shown = bShow;
}

void CUIGameCTA::UpdateTeamPanels()
{
	teamPanels->NeedUpdatePanels();
	teamPanels->NeedUpdatePlayers();
}

void CUIGameCTA::SetClGame(game_cl_GameState* g)
{
	inherited::SetClGame(g);
	m_game = smart_cast<game_cl_CaptureTheArtefact*>(g);
	VERIFY(m_game);
	
	if (m_pMapDesc)
	{
		if (m_pMapDesc->IsShown())
		{
			HUD().GetUI()->StartStopMenu(m_pMapDesc, true);
		}
		delete_data(m_pMapDesc);
	}
	m_pMapDesc = new CUIMapDesc();
	
	if (m_pBuySpawnMsgBox)
	{
		if (m_pBuySpawnMsgBox->IsShown())
		{
			HUD().GetUI()->StartStopMenu(m_pBuySpawnMsgBox, true);
		}
		delete_data(m_pBuySpawnMsgBox);
	}
	
	m_pBuySpawnMsgBox					= new CUIMessageBoxEx();	
	m_pBuySpawnMsgBox->InitMessageBox	("message_box_buy_spawn");
	m_pBuySpawnMsgBox->SetText			("");
	
	m_game->SetGameUI(this);
	m_pBuySpawnMsgBox->func_on_ok = CUIWndCallback::void_function(m_game, &game_cl_CaptureTheArtefact::OnBuySpawn);
}

void CUIGameCTA::Init()
{
	teamPanels->Init(TEAM_PANELS_XML_NAME, "team_panels_wnd");
}

void CUIGameCTA::AddPlayer(ClientID const & clientId)
{
	teamPanels->AddPlayer(clientId);
}

void CUIGameCTA::RemovePlayer(ClientID const & clientId)
{
	teamPanels->RemovePlayer(clientId);
}
void CUIGameCTA::UpdatePlayer(ClientID const & clientId)
{
	teamPanels->UpdatePlayer(clientId);
}

bool CUIGameCTA::IsTeamSelectShown()
{
	VERIFY(m_pUITeamSelectWnd);
	return m_pUITeamSelectWnd->IsShown();
}
void CUIGameCTA::ShowTeamSelectMenu()
{
	if (Level().IsDemoPlay())
		return;
	VERIFY(m_pUITeamSelectWnd);
	if (!m_pUITeamSelectWnd->IsShown())
	{
		HUD().GetUI()->StartStopMenu(m_pUITeamSelectWnd, true);
	}
}

bool CUIGameCTA::IsMapDescShown()
{
	VERIFY(m_pMapDesc);
	return m_pMapDesc->IsShown();
}
void CUIGameCTA::ShowMapDesc()
{
	if (Level().IsDemoPlay())
		return;
	VERIFY(m_pMapDesc);
	if (!m_pMapDesc->IsShown())
	{
		HUD().GetUI()->StartStopMenu(m_pMapDesc, true);
	}
}

void CUIGameCTA::UpdateBuyMenu(shared_str const & teamSection, shared_str const & costSection)
{
	if (m_pCurBuyMenu)
	{
		if (m_teamSectionForBuyMenu == teamSection)
		{
			if (m_pCurBuyMenu->IsShown())
				HideBuyMenu();
			m_pCurBuyMenu->IgnoreMoneyAndRank(false);
			m_pCurBuyMenu->SetRank(m_game->local_player->rank);
			m_pCurBuyMenu->ClearSlots();
			m_pCurBuyMenu->ClearPreset(_preset_idx_last);
			return;
		}
		xr_delete(m_pCurBuyMenu);
		m_pCurBuyMenu = NULL;
	}
	m_teamSectionForBuyMenu = teamSection;
	/// warning !!!
	m_pCurBuyMenu = new BUY_WND_TYPE();	
	m_pCurBuyMenu->Init(m_teamSectionForBuyMenu, costSection);
	m_costSection = costSection;
	m_pCurBuyMenu->SetSkin(0);
}

bool CUIGameCTA::CanBuyItem(shared_str const & sect_name)
{
	CUIMpTradeWnd* buy_menu = smart_cast<CUIMpTradeWnd*>(m_pCurBuyMenu);
	R_ASSERT(buy_menu);
	return buy_menu->HasItemInGroup(sect_name);
}

void CUIGameCTA::UpdateSkinMenu(shared_str const & teamSection)
{
	game_PlayerState *tempPlayerState = Game().local_player;
	VERIFY2(tempPlayerState, "local_player not initialized");

	if (m_pCurSkinMenu)
	{
		if (m_teamSectionForSkinMenu == teamSection)
		{
			return;
		}
		xr_delete(m_pCurSkinMenu);
		m_pCurSkinMenu = NULL;
	}
	m_teamSectionForSkinMenu = teamSection;
	m_pCurSkinMenu = new CUISkinSelectorWnd(m_teamSectionForSkinMenu.c_str(), static_cast<s16>(tempPlayerState->team));
}


void CUIGameCTA::HideBuyMenu()
{
	R_ASSERT2(m_pCurBuyMenu, "buy menu not initialized");
	if (m_pCurBuyMenu->IsShown())
	{
		HUD().GetUI()->StartStopMenu(m_pCurBuyMenu, true);
	}
}

void CUIGameCTA::ShowBuyMenu()
{
	if (Level().IsDemoPlay())
		return;
	R_ASSERT2(m_pCurBuyMenu, "buy menu not initialized");
	if (!m_pCurBuyMenu->IsShown())
	{
		m_pCurBuyMenu->IgnoreMoneyAndRank(m_game->InWarmUp());
	
		m_pCurBuyMenu->ResetItems();
		m_pCurBuyMenu->SetupPlayerItemsBegin();

		SetPlayerItemsToBuyMenu();
		SetPlayerParamsToBuyMenu();

		m_pCurBuyMenu->SetupPlayerItemsEnd();

		HUD().GetUI()->StartStopMenu(m_pCurBuyMenu, true);
		m_game->OnBuyMenuOpen();
	}
}
/*
void CUIGameCTA::BuyMenuItemIDInserter(u16 const & itemID)
{
	
}*/


void CUIGameCTA::TryToDefuseAllWeapons	(aditional_ammo_t & dest_ammo)
{
	game_PlayerState* ps = Game().local_player;
	VERIFY2(ps, "local player not initialized");
	CActor* actor = smart_cast<CActor*> (Level().Objects.net_Find(ps->GameID));
	R_ASSERT2(actor || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD),
		make_string("bad actor: not found in game (GameID = %d)", ps->GameID).c_str());

	TIItemContainer const & all_items = actor->inventory().m_all;  

	for (TIItemContainer::const_iterator i = all_items.begin(),
		ie = all_items.end(); i != ie; ++i)
	{
		CWeapon* tmp_weapon = smart_cast<CWeapon*>(*i);
		if (tmp_weapon)
			TryToDefuseWeapon(tmp_weapon, all_items, dest_ammo);
	}
}

struct AmmoSearcherPredicate
{
	u16			additional_ammo_count;
	shared_str	ammo_section;

	AmmoSearcherPredicate(u16 ammo_elapsed, shared_str const & ammo_sect) :
		additional_ammo_count(ammo_elapsed),
		ammo_section(ammo_sect)
	{
	}

	bool operator()(PIItem const & item)
	{
		CWeaponAmmo* temp_ammo = smart_cast<CWeaponAmmo*>(item);
		if (!temp_ammo)
			return false;
		
		if (temp_ammo->m_boxCurr >= temp_ammo->m_boxSize)
			return false;
		
		if (temp_ammo->cNameSect() != ammo_section)
			return false;

		if ((temp_ammo->m_boxCurr + additional_ammo_count) < temp_ammo->m_boxSize)
			return false;
		
		return true;
	}

};

void CUIGameCTA::TryToDefuseGrenadeLauncher(CWeaponMagazinedWGrenade const * weapon, TIItemContainer const & all_items, aditional_ammo_t & dest_ammo)
{
	if (!weapon)
		return;

	if (weapon->m_ammoTypes2.size() <= weapon->m_ammoType2)
		return;

	shared_str ammo_section = weapon->m_ammoTypes2[weapon->m_ammoType2];

	VERIFY2(ammo_section.size(), make_string(
		"grenade ammo type of [%s] hasn't section name", weapon->cNameSect().c_str()).c_str());
	if (!ammo_section.size())
		return;

	VERIFY(pSettings->line_exist(ammo_section.c_str(), "box_size"));

	u16 ammo_box_size	= pSettings->r_u16(ammo_section.c_str(), "box_size");
	u16 ammo_elapsed	= static_cast<u16>(weapon->iAmmoElapsed2);
	R_ASSERT2(ammo_elapsed <= 1, make_string(
		"weapon [%s] can't have more than one grenade in grenade launcher",
		weapon->cNameSect().c_str()).c_str());


	while (ammo_elapsed >= ammo_box_size)
	{
		dest_ammo.push_back(ammo_section);
		ammo_elapsed = ammo_elapsed - ammo_box_size;
	}
	if (!ammo_elapsed)
		return;

	AmmoSearcherPredicate ammo_completitor(ammo_elapsed, ammo_section);

	TIItemContainer::const_iterator temp_iter = std::find_if(
		all_items.begin(), all_items.end(), ammo_completitor);

	if (temp_iter == all_items.end())
		return;

	CWeaponAmmo* temp_ammo = smart_cast<CWeaponAmmo*>(*temp_iter);
	R_ASSERT2(temp_ammo, "failed to create ammo after defusing weapon");
	temp_ammo->m_boxCurr = temp_ammo->m_boxSize;
}


void CUIGameCTA::TryToDefuseWeapon(CWeapon const * weapon, TIItemContainer const & all_items, aditional_ammo_t & dest_ammo)
{
	if (!weapon)
		return;

	if (weapon->IsGrenadeLauncherAttached())
		TryToDefuseGrenadeLauncher(smart_cast<CWeaponMagazinedWGrenade const *>(weapon), all_items, dest_ammo);
	
	if (weapon->m_ammoTypes.size() <= weapon->m_ammoType)
		return;

	shared_str ammo_section = weapon->m_ammoTypes[weapon->m_ammoType];

	VERIFY2(ammo_section.size(), make_string(
		"ammo type of [%s] hasn't section name", weapon->cName().c_str()).c_str());
	if (!ammo_section.size())
		return;

	VERIFY(pSettings->line_exist(ammo_section.c_str(), "box_size"));

	u16 ammo_box_size	= pSettings->r_u16(ammo_section.c_str(), "box_size");
	u16 ammo_elapsed	= static_cast<u16>(weapon->GetAmmoElapsed());

	while (ammo_elapsed >= ammo_box_size)
	{
		dest_ammo.push_back(ammo_section);
		ammo_elapsed = ammo_elapsed - ammo_box_size;
	}
	if (!ammo_elapsed)
		return;

	AmmoSearcherPredicate ammo_completitor(ammo_elapsed, ammo_section);

	TIItemContainer::const_iterator temp_iter = std::find_if(
		all_items.begin(), all_items.end(), ammo_completitor);

	if (temp_iter == all_items.end())
		return;

	CWeaponAmmo* temp_ammo = smart_cast<CWeaponAmmo*>(*temp_iter);
	R_ASSERT2(temp_ammo, "failed to create ammo after defusing weapon");
	temp_ammo->m_boxCurr = temp_ammo->m_boxSize;
}

void CUIGameCTA::AdditionalAmmoInserter	(aditional_ammo_t::value_type const & sect_name)
{
	VERIFY(m_pCurBuyMenu);
	
	if (!pSettings->line_exist(m_costSection, sect_name.c_str()))
		return;
	
	m_pCurBuyMenu->ItemToSlot(sect_name.c_str(), 0);
}

void CUIGameCTA::BuyMenuItemInserter(PIItem const & item)
{
	VERIFY(m_pCurBuyMenu);
	if (!item)
		return;
	
	if (item->IsInvalid() || smart_cast<CWeaponKnife*>(&item->object()) )
		return;

	CArtefact* pArtefact = smart_cast<CArtefact*>(item);
	if (pArtefact)
		return;

	if (!pSettings->line_exist(m_costSection, item->object().cNameSect()))
		return;

	if (!item->CanTrade())
		return;

	u8 addons = 0;
	CWeapon* pWeapon = smart_cast<CWeapon*>(item);
	if (pWeapon)
		addons = pWeapon->GetAddonsState();
	
	CWeaponAmmo* pAmmo = smart_cast<CWeaponAmmo*>(item);
	if (pAmmo && (pAmmo->m_boxCurr != pAmmo->m_boxSize))
		return;
	
	m_pCurBuyMenu->ItemToSlot(item->object().cNameSect(), addons);
}

void CUIGameCTA::BuyMenuItemInserter(CInventorySlot const & slot)
{
	BuyMenuItemInserter(slot.m_pIItem);
}

void CUIGameCTA::SetPlayerDefItemsToBuyMenu()
{
	if (m_pCurBuyMenu->IsShown())
		return;
	m_pCurBuyMenu->ResetItems();
	m_pCurBuyMenu->SetupDefaultItemsBegin();
	//---------------------------------------------------------
	u8 KnifeSlot, KnifeIndex;
	m_pCurBuyMenu->GetWeaponIndexByName("mp_wpn_knife", KnifeSlot, KnifeIndex);
	//---------------------------------------------------------
	PRESET_ITEMS		TmpPresetItems;
	PRESET_ITEMS_it		It = PlayerDefItems.begin();
	PRESET_ITEMS_it		Et = PlayerDefItems.end();
	for ( ; It != Et; ++It) 
	{
		PresetItem PIT = *It;
		if (PIT.ItemID == KnifeIndex) continue;
		m_pCurBuyMenu->ItemToSlot(m_pCurBuyMenu->GetWeaponNameByIndex(0, PIT.ItemID), PIT.SlotID);
	};
	//---------------------------------------------------------
	m_pCurBuyMenu->SetupDefaultItemsEnd();
}

void CUIGameCTA::SetPlayerItemsToBuyMenu()
{
	VERIFY(m_pCurBuyMenu);
	game_PlayerState* ps = Game().local_player;
	VERIFY2(ps, "local player not initialized");
	CActor* actor = smart_cast<CActor*> (Level().Objects.net_Find(ps->GameID));
	R_ASSERT2(actor || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD),
		make_string("bad actor: not found in game (GameID = %d)", ps->GameID).c_str());

	if (actor && !ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
	{
		u32 max_addammo_count = actor->inventory().m_all.size();
		aditional_ammo_t add_ammo(
			_alloca(
				sizeof(aditional_ammo_t::value_type) * (max_addammo_count * 2)
			),
			max_addammo_count * 2
		);
		TryToDefuseAllWeapons(add_ammo);
        for (auto& slot : actor->inventory().m_slots)
        {
            BuyMenuItemInserter(slot);
        }
        for (auto& item : actor->inventory().m_belt)
        {
            BuyMenuItemInserter(item);
        }
        for (auto& item : actor->inventory().m_ruck)
        {
            BuyMenuItemInserter(item);
        }
        for (auto& ammo_item : add_ammo)
        {
            AdditionalAmmoInserter(ammo_item);
        }
	} else
	{
		SetPlayerDefItemsToBuyMenu();
	}
}

void CUIGameCTA::ReInitPlayerDefItems()
{
	R_ASSERT					(m_pCurBuyMenu);
	LoadDefItemsForRank	();
	SetPlayerDefItemsToBuyMenu();
}

void CUIGameCTA::SetPlayerParamsToBuyMenu()
{
	VERIFY(m_pCurBuyMenu);

	game_PlayerState* ps = Game().local_player;
	VERIFY2(ps, "local player not initialized");
	CActor* actor = smart_cast<CActor*> (Level().Objects.net_Find(ps->GameID));
	R_ASSERT2(actor || ps->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD),  
		make_string("bad actor: not found in game (GameID = %d)", ps->GameID).c_str());

	m_pCurBuyMenu->SetRank(ps->rank);
	m_pCurBuyMenu->SetMoneyAmount(ps->money_for_round);
}

void CUIGameCTA::GetPurchaseItems(BuyMenuItemsCollection & dest, s32 & moneyDif)
{
	R_ASSERT	(m_game);
	R_ASSERT	(m_pCurBuyMenu);
	preset_items const * tmpPresItems = &(m_pCurBuyMenu->GetPreset(_preset_idx_last));
	if (tmpPresItems->size() == 0)
	{
		tmpPresItems = &(m_pCurBuyMenu->GetPreset(_preset_idx_default));//_preset_idx_origin));
	}
	preset_items::const_iterator pie = tmpPresItems->end();
	for (preset_items::const_iterator pi = tmpPresItems->begin();
		pi != pie; ++pi)
	{
		u8 addon;
		u8 itemId;
		//we just use addon variable as temp storage
		m_pCurBuyMenu->GetWeaponIndexByName(pi->sect_name, addon, itemId);
		
		addon = pi->addon_state;

		for (u32 ic = 0; ic < pi->count; ++ic)
			dest.push_back(std::make_pair(addon, itemId));
	}

	if (m_game->local_player && m_game->local_player->testFlag(GAME_PLAYER_FLAG_VERY_VERY_DEAD))
	{
		u8 KnifeSlot, KnifeIndex;
		m_pCurBuyMenu->GetWeaponIndexByName("mp_wpn_knife", KnifeSlot, KnifeIndex);
		dest.push_back(std::make_pair(KnifeSlot, KnifeIndex));
	}

	moneyDif = m_pCurBuyMenu->GetPresetCost(_preset_idx_origin) - m_pCurBuyMenu->GetPresetCost(_preset_idx_last);
}

CUIGameCTA::BuyMenuItemPair	CUIGameCTA::GetBuyMenuItem(shared_str const & itemSectionName)
{
	VERIFY(m_pCurBuyMenu);
	u8 groupId;
	u8 itemId;
	//we just use addon variable as temp storage
	m_pCurBuyMenu->GetWeaponIndexByName(itemSectionName, groupId, itemId);
	return std::make_pair(groupId, itemId);
}

void CUIGameCTA::ShowSkinMenu(s8 currentSkin)
{
	if (Level().IsDemoPlay())
		return;
	//VERIFY2(m_pCurSkinMenu, "skin menu not initialized");
	if (!m_pCurSkinMenu)
	{
#ifdef CLIENT_CTA_LOG
		Msg("Warning: current skin window not initialized while trying to show it");
#endif
		return;
	}
	if (!m_pCurSkinMenu->IsShown())
	{
		HUD().GetUI()->StartStopMenu(m_pCurSkinMenu, true);
	} else
	{
		VERIFY2(false, "Skin menu already shown");
	}
}

s8 CUIGameCTA::GetSelectedSkinIndex	()
{
	VERIFY(m_pCurSkinMenu);
	return static_cast<s8>(m_pCurSkinMenu->GetActiveIndex());
}

void CUIGameCTA::SetReinforcementTimes(u32 curTime, u32 maxTime)
{
	m_pReinforcementInidcator->SetPos(curTime / 1000, maxTime / 1000);
}

void CUIGameCTA::DisplayMoneyChange(LPCSTR deltaMoney)
{
	m_pMoneyIndicator->SetMoneyChange(deltaMoney);
}

void CUIGameCTA::DisplayMoneyBonus(KillMessageStruct bonus){
	m_pMoneyIndicator->AddBonusMoney(bonus);
}

void CUIGameCTA::ChangeTotalMoneyIndicator(LPCSTR newMoneyString)
{
	m_pMoneyIndicator->SetMoneyAmount(newMoneyString);
}

void CUIGameCTA::SetRank(ETeam team, u8 rank)
{
	m_pRankIndicator->SetRank(static_cast<u8>(team), rank);
	if (m_pCurBuyMenu)
	{
		m_pCurBuyMenu->SetRank(rank);
	}
};

void CUIGameCTA::SetScore(s32 max_score, s32 greenTeamScore, s32 blueTeamScore)
{
	string32 str;
	sprintf_s(str,"%d", greenTeamScore);
	m_team1_score->SetText(str);
	sprintf_s(str,"%d", blueTeamScore);
	m_team2_score->SetText(str);
	if (max_score <= 0)
	{
		strcpy_s(str,"--");
	} else
	{
		sprintf_s(str,"%d", max_score);
	}
	m_pFragLimitIndicator->SetText(str);
	teamPanels->SetArtefactsCount(greenTeamScore, blueTeamScore);
}

void CUIGameCTA::OnFrame()
{
	inherited::OnFrame();
	//inherited::Render();
	m_pMoneyIndicator->Update();
	m_pRankIndicator->Update();
	m_pFragLimitIndicator->Update();
	m_pReinforcementInidcator->Update();
	if (m_voteStatusWnd) 
		m_voteStatusWnd->Update		();
}

void CUIGameCTA::Render()
{
	inherited::Render();
	m_pMoneyIndicator->Draw();
	m_pRankIndicator->Draw();
	m_pReinforcementInidcator->Draw();

	m_team1_icon->Draw();
	m_team2_icon->Draw();
	m_team1_score->Draw();
	m_team2_score->Draw();
	m_pFragLimitIndicator->Draw();
	
	if (m_voteStatusWnd) m_voteStatusWnd->Draw		();
}

void CUIGameCTA::SetRoundResultCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_round_result_caption, str, ROUND_RESULT_COLOR, true);
}

void CUIGameCTA::SetPressBuyMsgCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_pressbuy_caption, str, BUY_MSG_COLOR, true);
}

void CUIGameCTA::SetPressJumpMsgCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_pressjump_caption, str, NORMAL_MSG_COLOR, true);
}

void CUIGameCTA::SetSpectatorMsgCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_spectator_caption, str, NORMAL_MSG_COLOR, true);
}

void CUIGameCTA::SetSpectrModeMsgCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_spectrmode_caption, str, SPECTRMODE_MSG_COLOR, true);
}

void CUIGameCTA::SetWarmUpCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_warm_up_caption, str, WARM_UP_COLOR, true);
}
void CUIGameCTA::SetTimeMsgCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_time_caption, str, TIME_MSG_COLOR, true);
}

void CUIGameCTA::SetDemoPlayCaption(LPCSTR str)
{
	GameCaptions()->setCaption(m_demo_play_caption, str, DEMOPLAY_COLOR, true);
}

void CUIGameCTA::ResetCaptions()
{
	//bad ...
	SetRoundResultCaption("");
	SetPressBuyMsgCaption("");
	SetPressJumpMsgCaption("");
	SetSpectatorMsgCaption("");
	SetWarmUpCaption("");
	SetTimeMsgCaption("");
}

bool CUIGameCTA::IsBuySpawnShown()
{
	if (!m_pBuySpawnMsgBox)
		return false;
	
	if (m_pBuySpawnMsgBox->IsShown())
		return true;
	
	return false;
}

void CUIGameCTA::ShowBuySpawn(s32 spawn_cost)
{
	VERIFY			(m_pBuySpawnMsgBox);
	VERIFY			(Game().local_player);
	
	if (m_pBuySpawnMsgBox->IsShown())
		return;

	CStringTable	st;
	LPCSTR	format_str		= st.translate("mp_press_yes2pay").c_str();
	VERIFY(format_str);
	size_t	pay_frm_size	= xr_strlen(format_str)*sizeof(char) + 64;
	PSTR	pay_frm_str		= static_cast<char*>(_alloca(pay_frm_size));
	
	sprintf_s(
		pay_frm_str,
		pay_frm_size,
		format_str, 
		abs(Game().local_player->money_for_round),
		abs(spawn_cost)
	);

	m_pBuySpawnMsgBox->SetText(pay_frm_str);
	HUD().GetUI()->StartStopMenu(m_pBuySpawnMsgBox, true);
}

void CUIGameCTA::HideBuySpawn()
{
	if (IsBuySpawnShown())
	{
		HUD().GetUI()->StartStopMenu(m_pBuySpawnMsgBox, true);
	}
}

void CUIGameCTA::SetVoteMessage(LPCSTR str)
{
	if (m_voteStatusWnd)
	{
		xr_delete						(m_voteStatusWnd);
	}
	if (str) {
		CUIXml							uiXml;
		uiXml.Load						(CONFIG_PATH, UI_PATH, "ui_game_dm.xml");
		m_voteStatusWnd					= new UIVoteStatusWnd();
		m_voteStatusWnd->InitFromXML	(uiXml);
		m_voteStatusWnd->Show			(true);
		m_voteStatusWnd->SetVoteMsg		(str);
	}
};

void CUIGameCTA::SetVoteTimeResultMsg(LPCSTR str)
{
	if (m_voteStatusWnd)
		m_voteStatusWnd->SetVoteTimeResultMsg(str);
}

bool CUIGameCTA::IR_OnKeyboardPress(int dik)
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
	
	EGameActions cmd  = get_binded_action(dik);
	switch ( cmd )
	{
	case kINVENTORY: 
	case kBUY:
	case kSKIN:
	case kTEAM:
	case kMAP:

	case kSPEECH_MENU_0:
	case kSPEECH_MENU_1:
	case kSPEECH_MENU_2:
	case kSPEECH_MENU_3:
	case kSPEECH_MENU_4:
	case kSPEECH_MENU_5:
	case kSPEECH_MENU_6:
	case kSPEECH_MENU_7:
	case kSPEECH_MENU_8:
	case kSPEECH_MENU_9:
		{
			return Game().OnKeyboardPress( cmd );
		}break;
	}
	
	if(inherited::IR_OnKeyboardPress(dik))
		return true;

	return false;
}

bool CUIGameCTA::IR_OnKeyboardRelease(int dik)
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

s16	CUIGameCTA::GetBuyMenuItemIndex		(u8 Addons, u8 ItemID)
{
	s16	ID = (s16(Addons) << 0x08) | s16(ItemID);
	return ID;
};

void CUIGameCTA::LoadTeamDefaultPresetItems	(const shared_str& caSection)
{
	if (!pSettings->line_exist(caSection, "default_items")) return;
	if (!m_pCurBuyMenu) return;

	PlayerDefItems.clear();
	
	string256			ItemName;
	string4096			DefItems;
	// Читаем данные этого поля
	strcpy_s(DefItems, pSettings->r_string(caSection, "default_items"));
	u32 count	= _GetItemCount(DefItems);
	// теперь для каждое имя оружия, разделенные запятыми, заносим в массив
	for (u32 i = 0; i < count; ++i)
	{
		_GetItem(DefItems, i, ItemName);

		u8 SlotID, ItemID;
		m_pCurBuyMenu->GetWeaponIndexByName(ItemName, SlotID, ItemID);
		if (SlotID == 0xff || ItemID == 0xff) continue;
//		s16 ID = GetBuyMenuItemIndex(SlotID, ItemID);
		s16 ID = GetBuyMenuItemIndex(0, ItemID);
		PlayerDefItems.push_back(ID);
	};
};

void CUIGameCTA::LoadDefItemsForRank()
{
	R_ASSERT(m_pCurBuyMenu);
	R_ASSERT(m_game);
	R_ASSERT(m_game->local_player);
	//---------------------------------------------------
	game_PlayerState* local_player = m_game->local_player;
	LoadTeamDefaultPresetItems(m_game->getTeamSection(local_player->team));
	//---------------------------------------------------
	string16 RankStr;
	string256 ItemStr;
	string256 NewItemStr;
	char tmp[5];
	for (int i=1; i<=local_player->rank; i++)
	{
		strconcat(sizeof(RankStr),RankStr,"rank_",itoa(i,tmp,10));
		if (!pSettings->section_exist(RankStr)) continue;
		for (u32 it=0; it<PlayerDefItems.size(); it++)
		{
//			s16* pItemID = &(PlayerDefItems[it]);
//			char* ItemName = pBuyMenu->GetWeaponNameByIndex(u8(((*pItemID)&0xff00)>>0x08), u8((*pItemID)&0x00ff));
			PresetItem *pDefItem = &(PlayerDefItems[it]);
			const shared_str& ItemName = m_pCurBuyMenu->GetWeaponNameByIndex(pDefItem->SlotID, pDefItem->ItemID);
			if (!ItemName.size()) continue;
			strconcat(sizeof(ItemStr),ItemStr, "def_item_repl_", ItemName.c_str() );
			if (!pSettings->line_exist(RankStr, ItemStr)) continue;

			strcpy_s(NewItemStr,sizeof(NewItemStr),pSettings->r_string(RankStr, ItemStr));

			u8 SlotID, ItemID;
			m_pCurBuyMenu->GetWeaponIndexByName(NewItemStr, SlotID, ItemID);
			if (SlotID == 0xff || ItemID == 0xff) continue;

//			s16 ID = GetBuyMenuItemIndex(SlotID, ItemID);
			s16 ID = GetBuyMenuItemIndex(0, ItemID);

//			*pItemID = ID;
			pDefItem->set(ID);			
		}
	}
	//---------------------------------------------------------
	for (u32 it=0; it<PlayerDefItems.size(); it++)
	{
//		s16* pItemID = &(PlayerDefItems[it]);
//		char* ItemName = pBuyMenu->GetWeaponNameByIndex(u8(((*pItemID)&0xff00)>>0x08), u8((*pItemID)&0x00ff));
		PresetItem *pDefItem = &(PlayerDefItems[it]);
		const shared_str& ItemName = m_pCurBuyMenu->GetWeaponNameByIndex(pDefItem->SlotID, pDefItem->ItemID);
		if ( !ItemName.size() ) continue;
		if (!xr_strcmp(*ItemName, "mp_wpn_knife")) continue;
		if (!pSettings->line_exist(ItemName, "ammo_class")) continue;
		
		string1024 wpnAmmos, BaseAmmoName;
		strcpy_s(wpnAmmos, pSettings->r_string(ItemName, "ammo_class"));
		_GetItem(wpnAmmos, 0, BaseAmmoName);

		u8 SlotID, ItemID;
		m_pCurBuyMenu->GetWeaponIndexByName(BaseAmmoName, SlotID, ItemID);
		if (SlotID == 0xff || ItemID == 0xff) continue;

//		s16 ID = GetBuyMenuItemIndex(SlotID, ItemID);

		s16 ID = GetBuyMenuItemIndex(0, ItemID);
		PlayerDefItems.push_back(ID);
		PlayerDefItems.push_back(ID);
	};
};
