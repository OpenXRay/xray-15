#include "stdafx.h"
#include "UIActorMenu.h"
#include "UIActorStateInfo.h"
#include "../actor.h"
#include "../uigamesp.h"
#include "../hudmanager.h"
#include "../inventory.h"
#include "../inventory_item.h"
#include "../InventoryBox.h"
#include "object_broker.h"
#include "../ai/monsters/BaseMonster/base_monster.h"
#include "UIInventoryUtilities.h"
#include "game_cl_base.h"

#include "UICursor.h"
#include "UICellItem.h"
#include "UICharacterInfo.h"
#include "UIItemInfo.h"
#include "UIDragDropListEx.h"
#include "UIInventoryUpgradeWnd.h"
#include "UI3tButton.h"
#include "UIMessageBoxEx.h"
#include "UIPropertiesBox.h"
#include "UIMainIngameWnd.h"

void CUIActorMenu::SetActor(CInventoryOwner* io)
{
	R_ASSERT			(!IsShown());
	m_last_time			= Device.dwTimeGlobal;
	m_pActorInvOwner	= io;
	
	if ( IsGameTypeSingle() )
	{
		if ( io )
			m_ActorCharacterInfo->InitCharacter	(m_pActorInvOwner->object_id());
		else
			m_ActorCharacterInfo->ClearInfo();
	}
	else
	{
		UpdateActorMP();
	}
}

void CUIActorMenu::SetPartner(CInventoryOwner* io)
{
	R_ASSERT			(!IsShown());
	m_pPartnerInvOwner	= io;
	if ( m_pPartnerInvOwner )
	{
		CBaseMonster* monster = smart_cast<CBaseMonster*>( m_pPartnerInvOwner );
		if ( monster || m_pPartnerInvOwner->use_simplified_visual() ) 
		{
			m_PartnerCharacterInfo->ClearInfo();
			if ( monster )
			{
				shared_str monster_tex_name = pSettings->r_string( monster->cNameSect(), "icon" );
				m_PartnerCharacterInfo->UIIcon().InitTexture( monster_tex_name.c_str() );
				m_PartnerCharacterInfo->UIIcon().SetStretchTexture( true );
			}
		}
		else 
		{
			m_PartnerCharacterInfo->InitCharacter( m_pPartnerInvOwner->object_id() );
		}
		SetInvBox( NULL );
	}
	else
	{
		m_PartnerCharacterInfo->ClearInfo();
	}
}

void CUIActorMenu::SetInvBox(CInventoryBox* box)
{
	R_ASSERT			(!IsShown());
	m_pInvBox = box;
	if ( box )
	{
		m_pInvBox->m_in_use = true;
		SetPartner( NULL );
	}
}

void CUIActorMenu::SetMenuMode(EMenuMode mode)
{
	SetCurrentItem( NULL );
	m_hint_wnd->set_text( NULL );
	
	CActor* actor = smart_cast<CActor*>( m_pActorInvOwner );
	if ( actor )	{	actor->PickupModeOff();	}
	
	if ( mode != m_currMenuMode )
	{
		switch(m_currMenuMode)
		{
		case mmUndefined:
			break;
		case mmInventory:
			DeInitInventoryMode();
			break;
		case mmTrade:
			DeInitTradeMode();
			break;
		case mmUpgrade:
			DeInitUpgradeMode();
			break;
		case mmDeadBodySearch:
			DeInitDeadBodySearchMode();
			break;
		default:
			R_ASSERT(0);
			break;
		}

		HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(false);

		m_currMenuMode = mode;
		switch(mode)
		{
		case mmUndefined:
#ifdef DEBUG
			Msg("* now is Undefined mode");
#endif // #ifdef DEBUG
			ResetMode();
			break;
		case mmInventory:
			InitInventoryMode();
#ifdef DEBUG
			Msg("* now is Inventory mode");
#endif // #ifdef DEBUG
			break;
		case mmTrade:
			InitTradeMode();
#ifdef DEBUG
			Msg("* now is Trade mode");
#endif // #ifdef DEBUG
			break;
		case mmUpgrade:
			InitUpgradeMode();
#ifdef DEBUG
			Msg("* now is Upgrade mode");
#endif // #ifdef DEBUG
			break;
		case mmDeadBodySearch:
			InitDeadBodySearchMode();
#ifdef DEBUG
			Msg("* now is DeadBodySearch mode");
#endif // #ifdef DEBUG
			break;
		default:
			R_ASSERT(0);
			break;
		}
		CurModeToScript();
	}//if

	if ( m_pActorInvOwner )
	{
		UpdateOutfit();
		UpdateActor();
	}
	UpdateButtonsLayout();
}

void CUIActorMenu::PlaySnd(eActorMenuSndAction a)
{
	if (sounds[a]._handle())
        sounds[a].play					(NULL, sm_2D);
}

void CUIActorMenu::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	CUIWndCallback::OnEvent		(pWnd, msg, pData);
}

void CUIActorMenu::Show()
{
	SetMenuMode							(m_currMenuMode);
	inherited::Show						();
	PlaySnd								(eSndOpen);
	m_ActorStateInfo->Show				(true);
	m_ActorStateInfo->UpdateActorInfo	(m_pActorInvOwner);
}

void CUIActorMenu::Hide()
{
	inherited::Hide						();
	PlaySnd								(eSndClose);
	SetMenuMode							(mmUndefined);
	m_ActorStateInfo->Show				(false);
}

void CUIActorMenu::Draw()
{
	inherited::Draw();
	HUD().GetUI()->UIMainIngameWnd->DrawZoneMap();
	m_ItemInfo->Draw();
	m_hint_wnd->Draw();
}

void CUIActorMenu::Update()
{	
	{ // all mode
		m_last_time = Device.dwTimeGlobal;
		m_ActorStateInfo->UpdateActorInfo( m_pActorInvOwner );
	}

	switch ( m_currMenuMode )
	{
	case mmUndefined:
		break;
	case mmInventory:
		{
			m_clock_value->SetText( InventoryUtilities::GetGameTimeAsString( InventoryUtilities::etpTimeToMinutes ).c_str() );
			HUD().GetUI()->UIMainIngameWnd->UpdateZoneMap();
			break;
		}
	case mmTrade:
		{
			if(m_pPartnerInvOwner->inventory().ModifyFrame() != m_trade_partner_inventory_state)
				InitPartnerInventoryContents	();
			CheckDistance					();
			break;
		}
	case mmUpgrade:
		{
			UpdateUpgradeItem();
			CheckDistance();
			break;
		}
	case mmDeadBodySearch:
		{
			CheckDistance();
			break;
		}
	default: R_ASSERT(0); break;
	}
	
	inherited::Update();
	m_ItemInfo->Update();
	m_hint_wnd->Update();
}
bool CUIActorMenu::StopAnyMove()  // true = актёр не идёт при открытом меню
{
	switch ( m_currMenuMode )
	{
	case mmInventory:
		return false;
	case mmUndefined:
	case mmTrade:
	case mmUpgrade:
	case mmDeadBodySearch:
		return true;
	}
	return true;
}

void CUIActorMenu::CheckDistance()
{
	CGameObject* pActorGO	= smart_cast<CGameObject*>(m_pActorInvOwner);
	CGameObject* pPartnerGO	= smart_cast<CGameObject*>(m_pPartnerInvOwner);
	CGameObject* pBoxGO		= smart_cast<CGameObject*>(m_pInvBox);
	VERIFY( pActorGO && (pPartnerGO || pBoxGO) );

	if ( pPartnerGO )
	{
		if ( ( pActorGO->Position().distance_to( pPartnerGO->Position() ) > 3.0f ) &&
			!m_pPartnerInvOwner->NeedOsoznanieMode() )
		{
			GetHolder()->StartStopMenu( this, true ); // hide actor menu
		}
	}
	else //pBoxGO
	{
		VERIFY( pBoxGO );
		if ( pActorGO->Position().distance_to( pBoxGO->Position() ) > 3.0f )
		{
			GetHolder()->StartStopMenu( this, true ); // hide actor menu
		}
	}
}

EDDListType CUIActorMenu::GetListType(CUIDragDropListEx* l)
{
	if(l==m_pInventoryBagList)			return iActorBag;
	if(l==m_pInventoryBeltList)			return iActorBelt;

	if(l==m_pInventoryAutomaticList)	return iActorSlot;
	if(l==m_pInventoryPistolList)		return iActorSlot;
	if(l==m_pInventoryOutfitList)		return iActorSlot;
	if(l==m_pInventoryDetectorList)		return iActorSlot;
	

	if(l==m_pTradeActorBagList)			return iActorBag;
	if(l==m_pTradeActorList)			return iActorTrade;
	if(l==m_pTradePartnerBagList)		return iPartnerTradeBag;
	if(l==m_pTradePartnerList)			return iPartnerTrade;
	if(l==m_pDeadBodyBagList)			return iDeadBodyBag;

	R_ASSERT(0);
	
	return iInvalid;
}

CUIDragDropListEx* CUIActorMenu::GetListByType(EDDListType t)
{
	switch(t)
	{
		case iActorBag:
			{
				if(m_currMenuMode==mmTrade)
					return m_pTradeActorBagList;
				else
					return m_pInventoryBagList;
			}break;
		default:
			{
				R_ASSERT("invalid call");
			}break;
	}
	return NULL;
}

CUICellItem* CUIActorMenu::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUIActorMenu::CurrentIItem()
{
	return	(m_pCurrentCellItem)? (PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUIActorMenu::SetCurrentItem(CUICellItem* itm)
{
	m_repair_mode = false;
	m_pCurrentCellItem = itm;
	if ( !itm )
	{
		InfoCurItem( NULL );
	}
	TryHidePropertiesBox();

	if ( m_currMenuMode == mmUpgrade )
	{
		SetupUpgradeItem();
	}
}

// ================================================================

void CUIActorMenu::InfoCurItem( CUICellItem* cell_item )
{
	if ( !cell_item )
	{
		m_ItemInfo->InitItem( NULL );
		return;
	}
	PIItem current_item = (PIItem)cell_item->m_pData;

	PIItem compare_item = NULL;
	u32    compare_slot = current_item->GetSlot();
	if ( compare_slot != NO_ACTIVE_SLOT )
	{
		compare_item = m_pActorInvOwner->inventory().m_slots[compare_slot].m_pIItem;
	}
	
	m_ItemInfo->InitItem	( current_item, compare_item );
	float dx_pos = GetWndRect().left;
	m_ItemInfo->AlignHintWndPos( Frect().set( 0.0f, 0.0f, 1024.0f - dx_pos, 768.0f ), 10.0f, dx_pos );
}

bool CUIActorMenu::OnItemStartDrag(CUICellItem* itm)
{
	InfoCurItem( NULL );
	return false; //default behaviour
}

bool CUIActorMenu::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	InfoCurItem			(NULL);
	return				false;
}

bool CUIActorMenu::OnItemDrop(CUICellItem* itm)
{
	InfoCurItem( NULL );
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	if ( old_owner==new_owner || !old_owner || !new_owner )
	{
		return false;
	}

	EDDListType t_new		= GetListType(new_owner);
	EDDListType t_old		= GetListType(old_owner);
	
	if ( !AllowItemDrops(t_old, t_new) )
	{
		Msg("incorrect action [%d]->[%d]",t_old, t_new);
		return true;
	}
	switch(t_new)
	{
		case iActorSlot:
		{
			if(GetSlotList(CurrentIItem()->GetSlot())==new_owner)
				ToSlot	(itm, true);
		}break;
		case iActorBag:
		{
			ToBag	(itm, true);
		}break;
		case iActorBelt:
		{
			ToBelt	(itm, true);
		}break;
		case iActorTrade:
		{
			ToActorTrade(itm, true);
		}break;
		case iPartnerTrade:
		{
			if(t_old!=iPartnerTradeBag)	
				return false;
			ToPartnerTrade(itm, true);
		}break;
		case iPartnerTradeBag:
		{
			if(t_old!=iPartnerTrade)	
				return false;
			ToPartnerTradeBag(itm, true);
		}break;
		case iDeadBodyBag:
		{
			ToDeadBodyBag(itm, true);
		}break;
	};

	OnItemDropped			(CurrentIItem(), new_owner, old_owner);
	
	UpdateItemsPlace();

	return true;
}

bool CUIActorMenu::OnItemDbClick(CUICellItem* itm)
{
	InfoCurItem( NULL );
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	EDDListType t_old					= GetListType(old_owner);

	switch ( t_old )
	{
		case iActorSlot:
		{
			if ( m_currMenuMode == mmDeadBodySearch )
				ToDeadBodyBag	( itm, false );
			else
				ToBag			( itm, false );
			break;
		}
		case iActorBag:
		{
			if ( m_currMenuMode == mmTrade )
			{
				ToActorTrade( itm, false );
				break;
			}else
			if ( m_currMenuMode == mmDeadBodySearch )
			{
				ToDeadBodyBag( itm, false );
				break;
			}
			if ( TryUseItem( itm ) )
			{
				break;
			}
			if ( TryActiveSlot( itm ) )
			{
				break;
			}
			if ( !ToSlot( itm, false ) )
			{
				if ( !ToBelt( itm, false ) )
				{
					ToSlot( itm, true );
				}
			}
			break;
		}
		case iActorBelt:
		{
			ToBag( itm, false );
			break;
		}
		case iActorTrade:
		{
			ToBag( itm, false );
			break;
		}
		case iPartnerTradeBag:
		{
			ToPartnerTrade( itm, false );
			break;
		}
		case iPartnerTrade:
		{
			ToPartnerTradeBag( itm, false );
			break;
		}
		case iDeadBodyBag:
		{
			ToBag( itm, false );
			break;
		}

	}; //switch 

	UpdateItemsPlace();
	return true;
}

void CUIActorMenu::UpdateItemsPlace()
{
	switch ( m_currMenuMode )
	{
	case mmUndefined:
		break;
	case mmInventory:
		
		break;
	case mmTrade:
		UpdatePrices();
		break;
	case mmUpgrade:
		SetupUpgradeItem();
		break;
	case mmDeadBodySearch:
		UpdateDeadBodyBag();
		break;
	default:
		R_ASSERT(0);
		break;
	}

	if ( m_pActorInvOwner )
	{
		UpdateOutfit();
		UpdateActor();
	}
}

bool CUIActorMenu::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem( itm );
	InfoCurItem( NULL );
	ActivatePropertiesBox();
	return false;
}

bool CUIActorMenu::OnItemFocusReceive(CUICellItem* itm)
{
	InfoCurItem( NULL );
	return true;
}

bool CUIActorMenu::OnItemFocusLost(CUICellItem* itm)
{
	if ( itm )
	{
		itm->m_selected = false;
	}
	InfoCurItem( NULL );
	return true;
}

bool CUIActorMenu::OnItemFocusedUpdate(CUICellItem* itm)
{
	if ( itm )
	{
		itm->m_selected = true;
	}
	VERIFY( m_ItemInfo );
	if ( Device.dwTimeGlobal < itm->FocusReceiveTime() + m_ItemInfo->delay )
	{
		return true; //false
	}
	if ( CUIDragDropListEx::m_drag_item || m_UIPropertiesBox->IsShown() )
	{
		return true;
	}	
	
	InfoCurItem( itm );
	return true;
}

void CUIActorMenu::ClearAllLists()
{
	m_pInventoryBagList->ClearAll				(true);
	
	m_pInventoryBeltList->ClearAll				(true);
	m_pInventoryOutfitList->ClearAll			(true);
	m_pInventoryDetectorList->ClearAll			(true);
	m_pInventoryPistolList->ClearAll			(true);
	m_pInventoryAutomaticList->ClearAll			(true);

	m_pTradeActorBagList->ClearAll				(true);
	m_pTradeActorList->ClearAll					(true);
	m_pTradePartnerBagList->ClearAll			(true);
	m_pTradePartnerList->ClearAll				(true);
	m_pDeadBodyBagList->ClearAll				(true);
}

bool CUIActorMenu::OnMouse( float x, float y, EUIMessages mouse_action )
{
	inherited::OnMouse( x, y, mouse_action );
	return true; // no click`s
}

void CUIActorMenu::OnMouseMove()
{
	//SetInfoItem( NULL );
	inherited::OnMouseMove		();
}

bool CUIActorMenu::OnKeyboard(int dik, EUIMessages keyboard_action)
{
/*
	if (UIPropertiesBox.GetVisible())
	{	UIPropertiesBox.OnKeyboard(dik, keyboard_action); }
*/
	InfoCurItem( NULL );
	if ( is_binded(kDROP, dik) )
	{
		if ( WINDOW_KEY_PRESSED == keyboard_action && CurrentIItem() && !CurrentIItem()->IsQuestItem() )
		{

			SendEvent_Item_Drop		(CurrentIItem(), m_pActorInvOwner->object_id());
			SetCurrentItem			(NULL);
		}
		return true;
	}
	
	if ( is_binded(kSPRINT_TOGGLE, dik) )
	{
		if ( WINDOW_KEY_PRESSED == keyboard_action )
		{
			OnPressUserKey();
		}
		return true;
	}	
	
	if ( is_binded(kUSE, dik) )
	{
		if ( WINDOW_KEY_PRESSED == keyboard_action )
		{
			GetHolder()->StartStopMenu( this, true );
		}
		return true;
	}	

	if( inherited::OnKeyboard(dik,keyboard_action) )return true;

	return false;
}

void CUIActorMenu::OnPressUserKey()
{
	switch ( m_currMenuMode )
	{
	case mmUndefined:		break;
	case mmInventory:		break;
	case mmTrade:			
		OnBtnPerformTrade( this, 0 );
		break;
	case mmUpgrade:			
		TrySetCurUpgrade();
		break;
	case mmDeadBodySearch:	
		TakeAllFromPartner( this, 0 );
		break;
	default:
		R_ASSERT(0);
		break;
	}
}

bool  CUIActorMenu::AllowItemDrops(EDDListType from, EDDListType to)
{
	xr_vector<EDDListType>& v = m_allowed_drops[to];
	xr_vector<EDDListType>::iterator it = std::find(v.begin(), v.end(), from);
	
	return(it!=v.end());
}

void CUIActorMenu::CallMessageBoxYesNo( LPCSTR text )
{
	m_message_box_yes_no->SetText( text );
	m_message_box_yes_no->func_on_ok = CUIWndCallback::void_function( this, &CUIActorMenu::OnMesBoxYes );
	HUD().GetUI()->StartStopMenu( m_message_box_yes_no, false );
}

void CUIActorMenu::CallMessageBoxOK( LPCSTR text )
{
	m_message_box_ok->SetText( text );
	HUD().GetUI()->StartStopMenu( m_message_box_ok, false );
}

void CUIActorMenu::OnMesBoxYes( CUIWindow*, void* )
{
	switch( m_currMenuMode )
	{
	case mmUndefined:
		break;
	case mmInventory:
		break;
	case mmTrade:
		break;
	case mmUpgrade:
		if ( m_repair_mode )
		{
			RepairEffect_CurItem();
			m_repair_mode = false;
		}
		else
		{
			m_pUpgradeWnd->OnMesBoxYes();
		}
		break;
	case mmDeadBodySearch:
		break;
	default:
		R_ASSERT(0);
		break;
	}
	UpdateItemsPlace();
}

void CUIActorMenu::OnBtnExitClicked(CUIWindow* w, void* d)
{
	GetHolder()->StartStopMenu			(this,true);
}

void CUIActorMenu::ResetMode()
{
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	m_UIPropertiesBox->Hide		();
	SetCurrentItem				(NULL);
}

void CUIActorMenu::UpdateActorMP()
{
	if ( !Level().game || !Game().local_player || !m_pActorInvOwner || IsGameTypeSingle() )
	{
		m_ActorCharacterInfo->ClearInfo();
		m_ActorMoney->SetText( "" );
		return;
	}

	int money = Game().local_player->money_for_round;

	string64 buf;
	sprintf_s( buf, "%d RU", money );
	m_ActorMoney->SetText( buf );

	m_ActorCharacterInfo->InitCharacterMP( Game().local_player->name, "ui_npc_u_nebo_1" );

}
