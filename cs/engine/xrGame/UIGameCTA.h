#ifndef UIGAMECTA
#define UIGAMECTA

#include "UIGameCustom.h"
#include "game_base.h"
#include "inventory.h"


class UITeamPanels;

//-class CUIActorMenu;
//-class CUIPdaWnd;
class CUIMapDesc;
class CUIMoneyIndicator;
class CUIRankIndicator;
class UITeamPanels;
class CUISpawnWnd;
class IBuyWnd;
class CWeapon;
class CWeaponMagazinedWGrenade;
class CUISkinSelectorWnd;
class CUIProgressShape;
class CUIMessageBoxEx;
class UIVoteStatusWnd;
class game_cl_CaptureTheArtefact;

/// This class used to control UI part of client for Capture the Artefact mp game mode.
class CUIGameCTA : public CUIGameCustom
{
private:
	CUISpawnWnd			*m_pUITeamSelectWnd;
	CUIStatic*			m_team1_icon;
	CUIStatic*			m_team2_icon;
	CUIStatic*			m_team1_score;
	CUIStatic*			m_team2_score;
	CUIStatic*			m_pFragLimitIndicator;
	game_cl_CaptureTheArtefact*			m_game;

//-	CUIActorMenu*					m_pActorMenu;
//-	CUIPdaWnd*						m_pPdaMenu;
	CUIMapDesc*						m_pMapDesc;
	CUIMoneyIndicator*				m_pMoneyIndicator;
	CUIRankIndicator*				m_pRankIndicator;
	CUIProgressShape*				m_pReinforcementInidcator;
	CUIMessageBoxEx*				m_pBuySpawnMsgBox;
	UIVoteStatusWnd*				m_voteStatusWnd;
	
	shared_str						m_teamSectionForBuyMenu;
	IBuyWnd							*m_pCurBuyMenu;
	shared_str						m_teamSectionForSkinMenu;
	CUISkinSelectorWnd				*m_pCurSkinMenu;
	shared_str						m_costSection;

	/// This window shows the player lists.
	UITeamPanels *teamPanels;
	bool							m_team_panels_shown;


	shared_str					m_spectator_caption;
	shared_str					m_pressjump_caption;
	shared_str					m_pressbuy_caption;
	shared_str					m_round_result_caption;	
	shared_str					m_force_respawn_time_caption;
	shared_str					m_spectrmode_caption;
	shared_str					m_warm_up_caption;
	shared_str					m_time_caption;
	shared_str					m_demo_play_caption;

	struct PresetItem
	{
		u8	SlotID;
		u8	ItemID;
		s16	BigID;
		PresetItem (u8 Slot, u8 Item) { set(Slot, Item); };
		PresetItem (s16 Big) { set(Big); };
		bool	operator ==		(const s16& ID) 
		{ 
			return (BigID) == (ID);
		}
		void		set(s16 Big) { SlotID = u8((Big>>0x08) & 0x00ff); ItemID = u8(Big & 0x00ff); BigID = Big;}
		void		set(u8 Slot, u8 Item) { SlotID = Slot; ItemID = Item; BigID = (s16(SlotID) << 0x08) | s16(ItemID); };
	};
	
	DEF_VECTOR					(PRESET_ITEMS, PresetItem);
	
	PRESET_ITEMS				PlayerDefItems;

	typedef CUIGameCustom inherited;

	typedef buffer_vector<shared_str> aditional_ammo_t;
			void		TryToDefuseAllWeapons		(aditional_ammo_t & dest_ammo);
			void		TryToDefuseWeapon			(CWeapon const * weapon, TIItemContainer const & all_items, aditional_ammo_t & dest_ammo);
			void		TryToDefuseGrenadeLauncher	(CWeaponMagazinedWGrenade const * weapon, TIItemContainer const & all_items, aditional_ammo_t & dest_ammo);
			void		AdditionalAmmoInserter		(aditional_ammo_t::value_type const & sect_name);
			
			


			void		BuyMenuItemInserter(PIItem const & item);
			void		BuyMenuItemInserter(CInventorySlot const & slot);
			void		SetPlayerItemsToBuyMenu();
			void		SetPlayerParamsToBuyMenu();
			void		SetPlayerDefItemsToBuyMenu();
			void		LoadTeamDefaultPresetItems(const shared_str& caSection);
			void		LoadDefItemsForRank();
			s16			GetBuyMenuItemIndex		(u8 Addons, u8 ItemID);

public:
	typedef				std::pair<u8, u8>			BuyMenuItemPair;
	typedef				xr_vector<BuyMenuItemPair>	BuyMenuItemsCollection;

						CUIGameCTA				();
	virtual				~CUIGameCTA				();
	virtual void		ReInitShownUI			();
	virtual	void		reset_ui				();
	virtual void		SetClGame				(game_cl_GameState* g);
	virtual	void		Init					();
	virtual void		OnFrame					();
	virtual void		Render					();
	
	virtual bool		IR_OnKeyboardPress		(int dik);
	virtual bool		IR_OnKeyboardRelease	(int dik);

			bool		IsTeamPanelsShown		();
			void		ShowTeamPanels			(bool bShow);
			void		UpdateTeamPanels		();
			
			bool		IsTeamSelectShown		();
			void		ShowTeamSelectMenu		();

			bool		IsMapDescShown			();
			void		ShowMapDesc				();

			void		UpdateBuyMenu			(shared_str const & teamSection, shared_str const & costSection);
			bool		CanBuyItem				(shared_str const & sect_name);
			
			void		ShowBuyMenu				();
			void		HideBuyMenu				();
			BuyMenuItemPair	GetBuyMenuItem		(shared_str const & itemSectionName);
			void		GetPurchaseItems		(BuyMenuItemsCollection & dest, s32 & moneyDif);

			void		ReInitPlayerDefItems	();

			bool		IsBuySpawnShown			();
			void		ShowBuySpawn			(s32 spawn_cost);
			void		HideBuySpawn			();

			

			void		UpdateSkinMenu			(shared_str	const & teamSection);
			void		ShowSkinMenu			(s8 currentSkin);
			s8			GetSelectedSkinIndex	();


			void		AddPlayer				(ClientID const & clientId);
			void		RemovePlayer			(ClientID const & clientId);
			void		UpdatePlayer			(ClientID const & clientId);

			void		SetReinforcementTimes	(u32 curTime, u32 maxTime);

	virtual void		ChangeTotalMoneyIndicator	(LPCSTR newMoneyString);
	virtual void		DisplayMoneyChange			(LPCSTR deltaMoney);
	virtual void		DisplayMoneyBonus			(KillMessageStruct bonus);
			
			void		SetRank(ETeam team, u8 rank);
			void		SetScore(s32 max_score, s32 greenTeamScore, s32 blueTeamScore);

			void		SetRoundResultCaption(LPCSTR str);
			void		SetPressBuyMsgCaption(LPCSTR str);
			void		SetPressJumpMsgCaption(LPCSTR str);
			void		SetSpectatorMsgCaption(LPCSTR str);
			void		SetSpectrModeMsgCaption			(LPCSTR str);
			void		SetWarmUpCaption				(LPCSTR str);
			void		SetTimeMsgCaption				(LPCSTR str);
			void		SetDemoPlayCaption				(LPCSTR str);
			void		ResetCaptions();

			void		SetVoteMessage(LPCSTR str);
			void		SetVoteTimeResultMsg(LPCSTR str);
};

#endif