#pragma once

#include "UIDialogWnd.h"
#include "UIWndCallback.h"
#include "../../xrServerEntities/inventory_space.h"
#include "UIHint.h"

class CUICharacterInfo;
class CUIDragDropListEx;
class CUICellItem;
class ui_actor_state_wnd;
class CUIItemInfo;
class CUIFrameLineWnd;
class CUIStatic;
class CUI3tButtonEx;
class CInventoryOwner;
class CInventoryBox;
class CUIInventoryUpgradeWnd;
class UIInvUpgradeInfo;
class CUIMessageBoxEx;
class CUIPropertiesBox;
class CTrade;

namespace inventory { namespace upgrade {
	class Upgrade;
} } // namespace upgrade, inventory

enum EDDListType{
		iInvalid,
		iActorSlot,
		iActorBag,
		iActorBelt,

		iActorTrade,
		iPartnerTradeBag,
		iPartnerTrade,
		iDeadBodyBag,
		iListTypeMax
};

enum EMenuMode{
		mmUndefined,
		mmInventory,
		mmTrade,
		mmUpgrade,
		mmDeadBodySearch,
};

class CUIActorMenu :	public CUIDialogWnd, 
						public CUIWndCallback
{
	typedef CUIDialogWnd		inherited;
	typedef inventory::upgrade::Upgrade 	Upgrade_type;

protected:
	enum eActorMenuSndAction{	eSndOpen	=0,
								eSndClose,
								eItemToSlot,
								eItemToBelt,
								eItemToRuck,
								eProperties,
								eDropItem,
								eAttachAddon,
								eDetachAddon,
								eItemUse,
								eSndMax};

	EMenuMode					m_currMenuMode;
	ref_sound					sounds					[eSndMax];
	void						PlaySnd					(eActorMenuSndAction a);

	UIHint*						m_hint_wnd;
	CUIItemInfo*				m_ItemInfo;
	CUICellItem*				m_InfoCellItem;
	u32							m_InfoCellItem_timer;
	CUICellItem*				m_pCurrentCellItem;
	CUICellItem*				m_upgrade_selected;
	CUIPropertiesBox*			m_UIPropertiesBox;

	ui_actor_state_wnd*			m_ActorStateInfo;
	CUICharacterInfo*			m_ActorCharacterInfo;
	CUICharacterInfo*			m_PartnerCharacterInfo;

	CUIDragDropListEx*			m_pInventoryBeltList;
	CUIDragDropListEx*			m_pInventoryPistolList;
	CUIDragDropListEx*			m_pInventoryAutomaticList;
	CUIDragDropListEx*			m_pInventoryOutfitList;
	CUIDragDropListEx*			m_pInventoryDetectorList;
	CUIDragDropListEx*			m_pInventoryBagList;

	CUIDragDropListEx*			m_pTradeActorBagList;
	CUIDragDropListEx*			m_pTradeActorList;
	CUIDragDropListEx*			m_pTradePartnerBagList;
	CUIDragDropListEx*			m_pTradePartnerList;
	CUIDragDropListEx*			m_pDeadBodyBagList;
	enum						{e_af_count = 5};
	CUIStatic*					m_belt_list_over[e_af_count];

	CUIInventoryUpgradeWnd*		m_pUpgradeWnd;
	
	CUIStatic*					m_LeftBackground;

	UIInvUpgradeInfo*			m_upgrade_info;
	CUIMessageBoxEx*			m_message_box_yes_no;
	CUIMessageBoxEx*			m_message_box_ok;

	CInventoryOwner*			m_pActorInvOwner;
	CInventoryOwner*			m_pPartnerInvOwner;
	CInventoryBox*				m_pInvBox;

	CUIStatic*					m_ActorMoney;
	CUIStatic*					m_PartnerMoney;

	// bottom ---------------------------------
	CUIStatic*					m_ActorBottomInfo;
	CUIStatic*					m_ActorWeight;
	CUIStatic*					m_ActorWeightMax;
	
	CUIStatic*					m_PartnerBottomInfo;
	CUIStatic*					m_PartnerWeight;
	float						m_PartnerWeight_end_x;
//*	CUIStatic*					m_PartnerWeightMax;

	// delimiter ------------------------------
	CUIStatic*					m_LeftDelimiter;
	CUIStatic*					m_PartnerTradeCaption;
	CUIStatic*					m_PartnerTradePrice;
	CUIStatic*					m_PartnerTradeWeightMax;

	CUIStatic*					m_RightDelimiter;
	CUIStatic*					m_ActorTradeCaption;
	CUIStatic*					m_ActorTradePrice;
	CUIStatic*					m_ActorTradeWeightMax;

	CTrade*						m_actor_trade;
	CTrade*						m_partner_trade;

	CUI3tButtonEx*				m_trade_button;
	CUI3tButtonEx*				m_takeall_button;
	CUI3tButtonEx*				m_exit_button;
	CUIStatic*					m_clock_value;

	u32							m_last_time;
	bool						m_repair_mode;
	u32							m_trade_partner_inventory_state;
public:
	void						SetMenuMode					(EMenuMode mode);
	void						SetActor					(CInventoryOwner* io);
	void						SetPartner					(CInventoryOwner* io);
	void						SetInvBox					(CInventoryBox* box);
	void						SetSimpleHintText			(LPCSTR text);

private:
	void						PropertiesBoxForSlots		(PIItem item, bool& b_show);
	void						PropertiesBoxForWeapon		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void						PropertiesBoxForAddon		(PIItem item, bool& b_show);
	void						PropertiesBoxForUsing		(PIItem item, bool& b_show);
	void						PropertiesBoxForDrop		(CUICellItem* cell_item, PIItem item, bool& b_show);
	void						PropertiesBoxForRepair		(PIItem item, bool& b_show);

protected:			
	void						Construct					();
	void						InitCallbacks				();

	void						InitCellForSlot				(u32 slot_idx);
	void						InitInventoryContents		(CUIDragDropListEx* pBagList);
	void						ClearAllLists				();
	void						BindDragDropListEvents		(CUIDragDropListEx* lst);
	
	EDDListType					GetListType					(CUIDragDropListEx* l);
	CUIDragDropListEx*			GetListByType				(EDDListType t);
	CUIDragDropListEx*			GetSlotList					(u32 slot_idx);
	
	xr_vector<EDDListType>		m_allowed_drops				[iListTypeMax];
	bool						AllowItemDrops				(EDDListType from, EDDListType to);

	bool		xr_stdcall		OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusReceive			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusLost				(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusedUpdate			(CUICellItem* itm);
	bool						OnItemDropped				(PIItem itm, CUIDragDropListEx* new_owner, CUIDragDropListEx* old_owner);

	void						ResetMode					();
	void						InitInventoryMode			();
	void						DeInitInventoryMode			();
	void						InitTradeMode				();
	void						DeInitTradeMode				();
	void						InitUpgradeMode				();
	void						DeInitUpgradeMode			();
	void						InitDeadBodySearchMode		();
	void						DeInitDeadBodySearchMode	();

	void						CurModeToScript				();
	void						RepairEffect_CurItem		();

	void						SetCurrentItem				(CUICellItem* itm);
	CUICellItem*				CurrentItem					();
	PIItem						CurrentIItem				();

	void						InfoCurItem					(CUICellItem* cell_item); //on update item

	void						ActivatePropertiesBox		();
	void						TryHidePropertiesBox		();
	void		xr_stdcall		ProcessPropertiesBoxClicked	(CUIWindow* w, void* d);
	
	void						CheckDistance				();
	void						UpdateItemsPlace			();

	void						SetupUpgradeItem			();
	void						UpdateUpgradeItem			();
	void						TrySetCurUpgrade			();
	void						UpdateButtonsLayout			();

	// inventory
	bool						ToSlot						(CUICellItem* itm, bool force_place);
	bool						ToBag						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToBelt						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						TryUseItem					(CUICellItem* cell_itm);

	void						UpdateActorMP				();
	void						UpdateOutfit				();
	void						MoveArtefactsToBag			();
	bool						TryActiveSlot				(CUICellItem* itm);
	void		xr_stdcall		TryRepairItem				(CUIWindow* w, void* d);
	bool						CanUpgradeItem				(PIItem item);

	bool						ToActorTrade				(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToPartnerTrade				(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToPartnerTradeBag			(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToDeadBodyBag				(CUICellItem* itm, bool b_use_cursor_pos);

	void						AttachAddon					(PIItem item_to_upgrade);
	void						DetachAddon					(LPCSTR addon_name);

	void						SendEvent_Item2Slot			(PIItem	pItem, u16 parent);
	void						SendEvent_Item2Belt			(PIItem	pItem, u16 parent);
	void						SendEvent_Item2Ruck			(PIItem	pItem, u16 parent);
	void						SendEvent_Item_Drop			(PIItem	pItem, u16 parent);
	void						SendEvent_Item_Eat			(PIItem	pItem, u16 parent);
	void						SendEvent_ActivateSlot		(u32 slot, u16 recipient);
	void						DropAllCurrentItem			();
	void						OnPressUserKey				();

	// trade
	void						InitPartnerInventoryContents();
	void						ColorizeItem				(CUICellItem* itm, bool colorize);
	float						CalcItemsWeight				(CUIDragDropListEx* pList);
	u32							CalcItemsPrice				(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying);
	void						UpdatePrices				();
	bool						CanMoveToPartner			(PIItem pItem);
	void						TransferItems				(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying);

public:
								CUIActorMenu				();
	virtual						~CUIActorMenu				();

	virtual bool				StopAnyMove					();
	virtual void				SendMessage					(CUIWindow* pWnd, s16 msg, void* pData = NULL);
	virtual void				Draw						();
	virtual void				Update						();
	virtual void				Show						();
	virtual void				Hide						();

	virtual bool				OnKeyboard					(int dik, EUIMessages keyboard_action);
	virtual bool				OnMouse						(float x, float y, EUIMessages mouse_action);
	virtual void				OnMouseMove					();

	void						CallMessageBoxYesNo			(LPCSTR text);
	void						CallMessageBoxOK			(LPCSTR text);
	void		xr_stdcall		OnMesBoxYes					(CUIWindow*, void*);

	void						OnInventoryAction			(PIItem pItem, u16 action_type);
	void						ShowRepairButton			(bool status);
	bool						SetInfoCurUpgrade			(Upgrade_type* upgrade_type, CInventoryItem* inv_item );
	void						SeparateUpgradeItem			();
	PIItem						get_upgrade_item			();
	bool						DropAllItemsFromRuck		(bool quest_force = false); //debug func

	void						UpdateActor					();
	void						UpdatePartnerBag			();
	void						UpdateDeadBodyBag			();

	void		xr_stdcall		OnBtnPerformTrade			(CUIWindow* w, void* d);
	void		xr_stdcall		OnBtnExitClicked			(CUIWindow* w, void* d);
	void		xr_stdcall		TakeAllFromPartner			(CUIWindow* w, void* d);
	void						TakeAllFromInventoryBox		();

	IC	UIHint*					get_hint_wnd				() { return m_hint_wnd; }

}; // class CUIActorMenu
