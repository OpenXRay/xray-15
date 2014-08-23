#include "stdafx.h"
#include "UIActorMenu.h"
#include "../inventory.h"
#include "../inventoryOwner.h"
#include "UIInventoryUtilities.h"
#include "UIItemInfo.h"
#include "../Level.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UICellCustomItems.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UIPropertiesBox.h"
#include "UIListBoxItem.h"
#include "UIMainIngameWnd.h"

#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../Artefact.h"
#include "../eatable_item.h"
#include "../BottleItem.h"
#include "../WeaponMagazined.h"
#include "../Medkit.h"
#include "../Antirad.h"
#include "../CustomOutfit.h"
#include "../UICursor.h"
#include "../MPPlayersBag.h"
#include "../HUDManager.h"
#include "../player_hud.h"


void move_item_from_to(u16 from_id, u16 to_id, u16 what_id);

void CUIActorMenu::InitInventoryMode()
{
	m_pInventoryBagList->Show			(true);
	m_pInventoryBeltList->Show			(true);
	m_pInventoryOutfitList->Show		(true);
	m_pInventoryDetectorList->Show		(true);
	m_pInventoryPistolList->Show		(true);
	m_pInventoryAutomaticList->Show		(true);
	
	m_RightDelimiter->Show				(false);
	m_clock_value->Show					(true);

	InitInventoryContents				(m_pInventoryBagList);

	VERIFY( HUD().GetUI() && HUD().GetUI()->UIMainIngameWnd );
	HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(true);
}

void CUIActorMenu::DeInitInventoryMode()
{
	m_clock_value->Show					(false);
}

void CUIActorMenu::SendEvent_ActivateSlot(u32 slot, u16 recipient)
{
	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ACTIVATE_SLOT, recipient);
	P.w_u32							(slot);
	CGameObject::u_EventSend		(P);
}

void CUIActorMenu::SendEvent_Item2Slot(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2SLOT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToSlot);
};

void CUIActorMenu::SendEvent_Item2Belt(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2BELT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToBelt);
};

void CUIActorMenu::SendEvent_Item2Ruck(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM2RUCK, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);

	PlaySnd							(eItemToRuck);
};

void CUIActorMenu::SendEvent_Item_Eat(PIItem pItem, u16 recipient)
{
	if(pItem->parent_id()!=recipient)
		move_item_from_to			(pItem->parent_id(), recipient, pItem->object_id());

	NET_Packet						P;
	CGameObject::u_EventGen			(P, GEG_PLAYER_ITEM_EAT, recipient);
	P.w_u16							(pItem->object().ID());
	CGameObject::u_EventSend		(P);
};

void CUIActorMenu::SendEvent_Item_Drop(PIItem pItem, u16 recipient)
{
	R_ASSERT(pItem->parent_id()==recipient);
	if (!IsGameTypeSingle())
		pItem->DenyTrade();
	//pItem->SetDropManual			(TRUE);
	NET_Packet					P;
	pItem->object().u_EventGen	(P,GE_OWNERSHIP_REJECT,pItem->parent_id());
	P.w_u16						(pItem->object().ID());
	pItem->object().u_EventSend	(P);
	PlaySnd						(eDropItem);
}

void CUIActorMenu::DropAllCurrentItem()
{
	if ( CurrentIItem() && !CurrentIItem()->IsQuestItem() )
	{
		u32 const cnt = CurrentItem()->ChildsCount();
		for( u32 i = 0; i < cnt; ++i )
		{
			CUICellItem*	itm  = CurrentItem()->PopChild(NULL);
			PIItem			iitm = (PIItem)itm->m_pData;
			SendEvent_Item_Drop( iitm, m_pActorInvOwner->object_id() );
		}

		SendEvent_Item_Drop( CurrentIItem(), m_pActorInvOwner->object_id() );
	}
	SetCurrentItem								(NULL);
}

bool CUIActorMenu::DropAllItemsFromRuck( bool quest_force )
{
	if ( !IsShown() || !m_pInventoryBagList || m_currMenuMode != mmInventory )
	{
		return false;
	}

	u32 const ci_count = m_pInventoryBagList->ItemsCount();
	for ( u32 i = 0; i < ci_count; ++i )
	{
		CUICellItem* ci = m_pInventoryBagList->GetItemIdx( i );
		VERIFY( ci );
		PIItem item = (PIItem)ci->m_pData;
		VERIFY( item );

		if ( !quest_force && item->IsQuestItem() )
		{
			continue;
		}
	
		u32 const cnt = ci->ChildsCount();
		for( u32 j = 0; j < cnt; ++j )
		{
			CUICellItem*	child_ci   = ci->PopChild(NULL);
			PIItem			child_item = (PIItem)child_ci->m_pData;
			SendEvent_Item_Drop( child_item, m_pActorInvOwner->object_id() );
		}
		SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
	}
	
	SetCurrentItem( NULL );
	return true;
}

bool FindItemInList(CUIDragDropListEx* lst, PIItem pItem, CUICellItem*& ci_res)
{
	u32 count = lst->ItemsCount();
	for (u32 i=0; i<count; ++i)
	{
		CUICellItem* ci				= lst->GetItemIdx(i);
		for(u32 j=0; j<ci->ChildsCount(); ++j)
		{
			CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(ci->Child(j));
			if(ici->object()==pItem)
			{
				ci_res = ici;
				//lst->RemoveItem(ci,false);
				return true;
			}
		}

		CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(ci);
		if(ici->object()==pItem)
		{
			ci_res = ci;
			//lst->RemoveItem(ci,false);
			return true;
		}
	}
	return false;
}

bool RemoveItemFromList(CUIDragDropListEx* lst, PIItem pItem)
{// fixme
	CUICellItem*	ci	= NULL;
	if(FindItemInList(lst, pItem, ci))
	{
		R_ASSERT		(ci);
		
		CUICellItem* dying_cell = lst->RemoveItem	(ci, false);
		xr_delete(dying_cell);

		return			true;
	}else
		return			false;
}

void CUIActorMenu::OnInventoryAction(PIItem pItem, u16 action_type)
{
	CUIDragDropListEx* all_lists[] =
	{
		m_pInventoryBeltList, 
		m_pInventoryPistolList, 
		m_pInventoryAutomaticList, 
		m_pInventoryOutfitList, 
		m_pInventoryDetectorList, 
		m_pInventoryBagList,
		m_pTradeActorBagList,
		m_pTradeActorList,
		NULL
	};

	switch (action_type)
	{
		case GE_TRADE_BUY :
		case GE_OWNERSHIP_TAKE : 
			{
				u32 i			= 0;
				bool b_already	= false;

				CUIDragDropListEx* lst_to_add		= NULL;
				EItemPlace pl						= pItem->m_eItemCurrPlace;
				if ( pItem->GetSlot() == GRENADE_SLOT )
				{
					pl = eItemPlaceRuck;
				}
#ifndef MASTER_GOLD
				Msg("item place [%d]", pl);
#endif // #ifndef MASTER_GOLD

				if(pl==eItemPlaceSlot)
					lst_to_add						= GetSlotList(pItem->GetSlot());
				else if(pl==eItemPlaceRuck)
					lst_to_add						= GetListByType(iActorBag);
				else if(pl==eItemPlaceBelt)
					lst_to_add						= GetListByType(iActorBelt);


				while ( all_lists[i] )
				{
					CUIDragDropListEx*	curr = all_lists[i];
					CUICellItem*		ci   = NULL;

					if ( FindItemInList(curr, pItem, ci) )
					{
						if ( lst_to_add != curr )
						{
							RemoveItemFromList(curr, pItem);
						}
						else
						{
							b_already = true;
						}
						//break;
					}
					++i;
				}
				if ( !b_already )
				{
					if ( lst_to_add )
					{
						CUICellItem* itm	= create_cell_item(pItem);
						lst_to_add->SetItem	(itm);
					}
				}
			}break;
		case GE_TRADE_SELL :
		case GE_OWNERSHIP_REJECT : 
			{
				if(CUIDragDropListEx::m_drag_item)
				{
					CUIInventoryCellItem* ici = smart_cast<CUIInventoryCellItem*>(CUIDragDropListEx::m_drag_item->ParentItem());
					R_ASSERT(ici);
					if(ici->object()==pItem)
					{
						CUIDragDropListEx*	_drag_owner		= ici->OwnerList();
						_drag_owner->DestroyDragItem		();
					}
				}
				
				u32 i = 0;
				while(all_lists[i])
				{
					CUIDragDropListEx* curr = all_lists[i];
					if(RemoveItemFromList(curr, pItem))
					{
#ifndef MASTER_GOLD
						Msg("all ok. item [%d] removed from list", pItem->object_id());
#endif // #ifndef MASTER_GOLD
						break;
					}
					++i;
				}
			}break;
	}
	UpdateItemsPlace();
}
void CUIActorMenu::AttachAddon(PIItem item_to_upgrade)
{
	PlaySnd										(eAttachAddon);
	R_ASSERT									(item_to_upgrade);
	if (OnClient())
	{
		NET_Packet								P;
		CGameObject::u_EventGen					(P, GE_ADDON_ATTACH, item_to_upgrade->object().ID());
		P.w_u16									(CurrentIItem()->object().ID());
		CGameObject::u_EventSend				(P);
	};

	item_to_upgrade->Attach						(CurrentIItem(), true);

	SetCurrentItem								(NULL);
}

void CUIActorMenu::DetachAddon(LPCSTR addon_name)
{
	PlaySnd										(eDetachAddon);
	if (OnClient())
	{
		NET_Packet								P;
		CGameObject::u_EventGen					(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		P.w_stringZ								(addon_name);
		CGameObject::u_EventSend				(P);
	};
	CurrentIItem()->Detach						(addon_name, true);
}

void CUIActorMenu::InitCellForSlot( u32 slot_idx ) 
{
	VERIFY( KNIFE_SLOT <= slot_idx && slot_idx <= DETECTOR_SLOT );
	PIItem item	= m_pActorInvOwner->inventory().m_slots[slot_idx].m_pIItem;
	if ( !item )
	{
		return;
	}
	
	CUIDragDropListEx* curr_list	= GetSlotList( slot_idx );
	CUICellItem* cell_item			= create_cell_item( item );
	curr_list->SetItem( cell_item );
	if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
	{
		ColorizeItem( cell_item, !CanMoveToPartner( item ) );
	}
}

void CUIActorMenu::InitInventoryContents(CUIDragDropListEx* pBagList) 
{
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	m_UIPropertiesBox->Hide		();
	SetCurrentItem				(NULL);

	CUIDragDropListEx*			curr_list = NULL;
	//Slots
	InitCellForSlot				(PISTOL_SLOT);
	InitCellForSlot				(RIFLE_SLOT);
	InitCellForSlot				(OUTFIT_SLOT);
	InitCellForSlot				(DETECTOR_SLOT);
	InitCellForSlot				(GRENADE_SLOT);

	curr_list					= m_pInventoryBeltList;
	TIItemContainer::iterator itb = m_pActorInvOwner->inventory().m_belt.begin();
	TIItemContainer::iterator ite = m_pActorInvOwner->inventory().m_belt.end();
	for ( ; itb != ite; ++itb )
	{
		CUICellItem* itm		= create_cell_item(*itb);
		curr_list->SetItem		(itm);
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
		}
	}

	TIItemContainer				ruck_list;
	ruck_list					= m_pActorInvOwner->inventory().m_ruck;
	std::sort					( ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck );

	curr_list					= pBagList;

	itb = ruck_list.begin();
	ite = ruck_list.end();
	for ( ; itb != ite; ++itb )
	{
		CMPPlayersBag* bag = smart_cast<CMPPlayersBag*>( &(*itb)->object() );
		if ( bag )
		{
			continue;
		}
		CUICellItem* itm = create_cell_item( *itb );
		curr_list->SetItem( itm );
		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( *itb ) );
		}
	}

}

bool CUIActorMenu::TryActiveSlot(CUICellItem* itm)
{
	PIItem	iitem	= (PIItem)itm->m_pData;
	u32 slot		= iitem->GetSlot();

	if ( slot == GRENADE_SLOT )
	{
		PIItem	prev_iitem = m_pActorInvOwner->inventory().m_slots[slot].m_pIItem;
		if ( prev_iitem && (prev_iitem->object().cNameSect() != iitem->object().cNameSect()) )
		{
			SendEvent_Item2Ruck( prev_iitem, m_pActorInvOwner->object_id() );
			SendEvent_Item2Slot( iitem, m_pActorInvOwner->object_id() );
		}
		SendEvent_ActivateSlot( slot, m_pActorInvOwner->object_id() );
		return true;
	}
	if ( slot == DETECTOR_SLOT )
	{

	}
	return false;
}

bool CUIActorMenu::ToSlot(CUICellItem* itm, bool force_place)
{
	CUIDragDropListEx*	old_owner			= itm->OwnerList();
	PIItem	iitem							= (PIItem)itm->m_pData;
	u32 _slot								= iitem->GetSlot();

	bool b_own_item							= (iitem->parent_id()==m_pActorInvOwner->object_id());

	if(m_pActorInvOwner->inventory().CanPutInSlot(iitem))
	{
		CUIDragDropListEx* new_owner		= GetSlotList(_slot);
		
		if ( _slot == GRENADE_SLOT || !new_owner )
		{
			return true; //fake, sorry (((
		}

		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Slot(iitem);
		VERIFY								(result);

		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		new_owner->SetItem					(i);
	
//.		if(!b_own_item)
		SendEvent_Item2Slot					(iitem, m_pActorInvOwner->object_id());

		SendEvent_ActivateSlot				(_slot, m_pActorInvOwner->object_id());
		
		//ColorizeItem						( itm, false );
		if ( _slot == OUTFIT_SLOT )
		{
			MoveArtefactsToBag();
		}
		return								true;
	}
	else
	{ // in case slot is busy
		if ( !force_place || _slot == NO_ACTIVE_SLOT ) return false;
		if ( m_pActorInvOwner->inventory().m_slots[_slot].m_bPersistent && _slot != DETECTOR_SLOT  )
		{
			return false;
		}

		PIItem	_iitem						= m_pActorInvOwner->inventory().m_slots[_slot].m_pIItem;
		CUIDragDropListEx* slot_list		= GetSlotList(_slot);
		VERIFY								(slot_list->ItemsCount()==1);

		CUICellItem* slot_cell				= slot_list->GetItemIdx(0);
		VERIFY								(slot_cell && ((PIItem)slot_cell->m_pData)==_iitem);

		bool result							= ToBag(slot_cell, false);
		VERIFY								(result);

		result								= ToSlot(itm, false);
		if(b_own_item && result && _slot==DETECTOR_SLOT)
		{
			CCustomDetector* det			= smart_cast<CCustomDetector*>(iitem);
			det->ToggleDetector				(g_player_hud->attached_item(0)!=NULL);
		}
		return result;
	}
}


bool CUIActorMenu::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());
	
	bool b_already						= m_pActorInvOwner->inventory().InRuck(iitem);

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= NULL;
	if(b_use_cursor_pos)
	{
			new_owner					= CUIDragDropListEx::m_drag_item->BackList();
			VERIFY						(GetListType(new_owner)==iActorBag);
	}else
			new_owner					= GetListByType(iActorBag);

	if(m_pActorInvOwner->inventory().CanPutInRuck(iitem) || (b_already && (new_owner!=old_owner)) )
	{
		bool result							= b_already || (!b_own_item || m_pActorInvOwner->inventory().Ruck(iitem) );
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		if(!b_already || !b_own_item)
			SendEvent_Item2Ruck					(iitem, m_pActorInvOwner->object_id());

		if ( m_currMenuMode == mmTrade && m_pPartnerInvOwner )
		{
			ColorizeItem( itm, !CanMoveToPartner( iitem ) );
		}
		return true;
	}
	return false;
}

bool CUIActorMenu::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;
	bool b_own_item						= (iitem->parent_id()==m_pActorInvOwner->object_id());

	if(m_pActorInvOwner->inventory().CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pInventoryBeltList);
		}else
				new_owner					= m_pInventoryBeltList;

		bool result							= (!b_own_item) || m_pActorInvOwner->inventory().Belt(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		if(!b_own_item)
			SendEvent_Item2Belt				(iitem, m_pActorInvOwner->object_id());

		//ColorizeItem						(itm, false);
		return								true;
	}
	return									false;
}
CUIDragDropListEx* CUIActorMenu::GetSlotList(u32 slot_idx)
{
	if ( slot_idx == NO_ACTIVE_SLOT /*|| m_pActorInvOwner->inventory().m_slots[slot_idx].m_bPersistent*/ )
	{
		return NULL;
	}
	switch ( slot_idx )
	{
		case PISTOL_SLOT:
			return m_pInventoryPistolList;
			break;

		case RIFLE_SLOT:
			return m_pInventoryAutomaticList;
			break;

		case OUTFIT_SLOT:
			return m_pInventoryOutfitList;
			break;

		case DETECTOR_SLOT:
			return m_pInventoryDetectorList;
			break;

		case GRENADE_SLOT://fake
			if ( m_currMenuMode == mmTrade )
			{
				return m_pTradeActorBagList;
			}
			return m_pInventoryBagList;
			break;
	};
	return NULL;
}

bool CUIActorMenu::TryUseItem( CUICellItem* cell_itm )
{
	if ( !cell_itm )
	{
		return false;
	}
	PIItem item	= (PIItem)cell_itm->m_pData;

	CBottleItem*	pBottleItem		= smart_cast<CBottleItem*>	(item);
	CMedkit*		pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad		= smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem	= smart_cast<CEatableItem*>	(item);

	if ( !(pMedkit || pAntirad || pEatableItem || pBottleItem) )
	{
		return false;
	}
	if ( !item->Useful() )
	{
		return false;
	}
	u16 recipient = m_pActorInvOwner->object_id();
	if ( item->parent_id() != recipient )
	{
		//move_item_from_to	(itm->parent_id(), recipient, itm->object_id());
		cell_itm->OwnerList()->RemoveItem( cell_itm, false );
	}

	SendEvent_Item_Eat		( item, recipient );
	PlaySnd					( eItemUse );
	SetCurrentItem			( NULL );
	return true;
}

bool CUIActorMenu::OnItemDropped(PIItem itm, CUIDragDropListEx* new_owner, CUIDragDropListEx* old_owner)
{
	CUICellItem*	_citem	= (new_owner->ItemsCount()==1) ? new_owner->GetItemIdx(0) : NULL;
	PIItem _iitem	= _citem ? (PIItem)_citem->m_pData : NULL;

	if(!_iitem)						return	false;
	if(!_iitem->CanAttach(itm))		return	false;
	
	if(old_owner != m_pInventoryBagList)	return	false;
	
	AttachAddon						(_iitem);

	return							true;
}

void CUIActorMenu::TryHidePropertiesBox()
{
	if ( m_UIPropertiesBox->IsShown() )
	{
		m_UIPropertiesBox->Hide();
	}
}

void CUIActorMenu::ActivatePropertiesBox()
{
	TryHidePropertiesBox();
	if ( !(m_currMenuMode == mmInventory || m_currMenuMode == mmDeadBodySearch || m_currMenuMode == mmUpgrade) )
	{
		return;
	}
	
	PIItem item = CurrentIItem();
	if ( !item ) 
	{
		return;
	}

	CUICellItem* cell_item = CurrentItem();
	m_UIPropertiesBox->RemoveAll();
	bool b_show = false;

	if ( m_currMenuMode == mmInventory )
	{
		PropertiesBoxForSlots( item, b_show );
		PropertiesBoxForWeapon( cell_item, item, b_show );
		PropertiesBoxForAddon( item, b_show );
		PropertiesBoxForUsing( item, b_show );
		PropertiesBoxForDrop( cell_item, item, b_show );
	}
	else if ( m_currMenuMode == mmDeadBodySearch )
	{
		PropertiesBoxForUsing( item, b_show );
	}
	else if ( m_currMenuMode == mmUpgrade )
	{
		PropertiesBoxForRepair( item, b_show );
	}
	
	if ( b_show )
	{
		m_UIPropertiesBox->AutoUpdateSize();
		m_UIPropertiesBox->BringAllToTop();

		Fvector2 cursor_pos;
		Frect    vis_rect;
		GetAbsoluteRect				( vis_rect );

		cursor_pos					= GetUICursor()->GetCursorPosition();
		cursor_pos.sub				( vis_rect.lt );
		m_UIPropertiesBox->Show		( vis_rect, cursor_pos );
		PlaySnd						( eProperties );
	}
}

void CUIActorMenu::PropertiesBoxForSlots( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>( item );
	CInventory*  inv = &m_pActorInvOwner->inventory();

	// Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
	bool bAlreadyDressed = false;
	u32 const cur_slot = item->GetSlot();

	if ( !pOutfit && cur_slot != NO_ACTIVE_SLOT
		&& !inv->m_slots[cur_slot].m_bPersistent && inv->CanPutInSlot(item) )
	{
		m_UIPropertiesBox->AddItem( "st_move_to_slot",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show = true;
	}
	if ( item->Belt() && inv->CanPutInBelt( item ) )
	{
		m_UIPropertiesBox->AddItem( "st_move_on_belt",  NULL, INVENTORY_TO_BELT_ACTION );
		b_show = true;
	}

	if ( item->Ruck() && inv->CanPutInRuck( item )
		&& ( cur_slot == (u32)(-1) || !inv->m_slots[cur_slot].m_bPersistent ) )
	{
		if( !pOutfit )
		{
			m_UIPropertiesBox->AddItem( "st_move_to_bag",  NULL, INVENTORY_TO_BAG_ACTION );
		}
		else
		{
			m_UIPropertiesBox->AddItem( "st_undress_outfit",  NULL, INVENTORY_TO_BAG_ACTION );
		}
		bAlreadyDressed = true;
		b_show			= true;
	}
	if ( pOutfit && !bAlreadyDressed )
	{
		m_UIPropertiesBox->AddItem( "st_dress_outfit",  NULL, INVENTORY_TO_SLOT_ACTION );
		b_show			= true;
	}
}

void CUIActorMenu::PropertiesBoxForWeapon( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	//отсоединение аддонов от вещи
	CWeapon*	pWeapon = smart_cast<CWeapon*>( item );
	if ( !pWeapon )
	{
		return;
	}

	if ( pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached() )
	{
		m_UIPropertiesBox->AddItem( "st_detach_gl",  NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON );
		b_show			= true;
	}
	if ( pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached() )
	{
		m_UIPropertiesBox->AddItem( "st_detach_scope",  NULL, INVENTORY_DETACH_SCOPE_ADDON );
		b_show			= true;
	}
	if ( pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached() )
	{
		m_UIPropertiesBox->AddItem( "st_detach_silencer",  NULL, INVENTORY_DETACH_SILENCER_ADDON );
		b_show			= true;
	}
	if ( smart_cast<CWeaponMagazined*>(pWeapon) && IsGameTypeSingle() )
	{
		bool b = ( pWeapon->GetAmmoElapsed() !=0 );
		if ( !b )
		{
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->Child(i)->m_pData );
				if ( weap_mag && weap_mag->GetAmmoElapsed() )
				{
					b = true;
					break; // for
				}
			}
		}
		if ( b )
		{
			m_UIPropertiesBox->AddItem( "st_unload_magazine",  NULL, INVENTORY_UNLOAD_MAGAZINE );
			b_show = true;
		}
	}
}

void CUIActorMenu::PropertiesBoxForAddon( PIItem item, bool& b_show )
{
	//присоединение аддонов к активному слоту (2 или 3)

	CScope*				pScope				= smart_cast<CScope*>			(item);
	CSilencer*			pSilencer			= smart_cast<CSilencer*>		(item);
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(item);
	CInventory*			inv					= &m_pActorInvOwner->inventory();

	if ( pScope )
	{
		if ( inv->m_slots[PISTOL_SLOT].m_pIItem && inv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pScope) )
		{
			PIItem tgt = inv->m_slots[PISTOL_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_scope_to_pistol",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pScope) )
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_scope_to_rifle",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		return;
	}
	
	if ( pSilencer )
	{
		if ( inv->m_slots[PISTOL_SLOT].m_pIItem && inv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pSilencer) )
		{
			PIItem tgt = inv->m_slots[PISTOL_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_silencer_to_pistol",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		if ( inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pSilencer) )
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_silencer_to_rifle",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
		return;
	}
	
	if ( pGrenadeLauncher )
	{
		if ( inv->m_slots[RIFLE_SLOT].m_pIItem && inv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pGrenadeLauncher) )
		{
			PIItem tgt = inv->m_slots[RIFLE_SLOT].m_pIItem;
			m_UIPropertiesBox->AddItem( "st_attach_gl_to_rifle",  (void*)tgt, INVENTORY_ATTACH_ADDON );
			b_show			= true;
		}
	}
}

void CUIActorMenu::PropertiesBoxForUsing( PIItem item, bool& b_show )
{
	CMedkit*		pMedkit			= smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad		= smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem	= smart_cast<CEatableItem*>	(item);
	CBottleItem*	pBottleItem		= smart_cast<CBottleItem*>	(item);

	LPCSTR act_str = NULL;
	if ( pMedkit || pAntirad )
	{
		act_str = "st_use";
	}
	else if ( pEatableItem )
	{
		if ( pBottleItem )
		{
			act_str = "st_drink";
		}
		else
		{
			act_str = "st_eat";
		}
	}
	if ( act_str )
	{
		m_UIPropertiesBox->AddItem( act_str,  NULL, INVENTORY_EAT_ACTION );
		b_show			= true;
	}
}

void CUIActorMenu::PropertiesBoxForDrop( CUICellItem* cell_item, PIItem item, bool& b_show )
{
	if ( !item->IsQuestItem() )
	{
		m_UIPropertiesBox->AddItem( "st_drop", NULL, INVENTORY_DROP_ACTION );
		b_show = true;

		if ( cell_item->ChildsCount() )
		{
			m_UIPropertiesBox->AddItem( "st_drop_all", (void*)33, INVENTORY_DROP_ACTION );
		}
	}
}

void CUIActorMenu::PropertiesBoxForRepair( PIItem item, bool& b_show )
{
	CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>( item );
	CWeapon*       pWeapon = smart_cast<CWeapon*>( item );

	if ( (pOutfit || pWeapon) && item->GetCondition() < 0.99f )
	{
		m_UIPropertiesBox->AddItem( "ui_inv_repair", NULL, INVENTORY_REPAIR );
		b_show = true;
	}
}

void CUIActorMenu::ProcessPropertiesBoxClicked( CUIWindow* w, void* d )
{
	PIItem			item		= CurrentIItem();
	CUICellItem*	cell_item	= CurrentItem();
	if ( !m_UIPropertiesBox->GetClickedItem() || !item || !cell_item || !cell_item->OwnerList() )
	{
		return;
	}
	CWeapon* weapon = smart_cast<CWeapon*>( item );
	
	switch ( m_UIPropertiesBox->GetClickedItem()->GetTAG() )
	{
	case INVENTORY_TO_SLOT_ACTION:	ToSlot( cell_item, true  );		break;
	case INVENTORY_TO_BELT_ACTION:	ToBelt( cell_item, false );		break;
	case INVENTORY_TO_BAG_ACTION:	ToBag ( cell_item, false );		break;
	case INVENTORY_EAT_ACTION:		TryUseItem( cell_item );		break;
	case INVENTORY_DROP_ACTION:
		{
			void* d = m_UIPropertiesBox->GetClickedItem()->GetData();
			if ( d == (void*)33 )
			{
				DropAllCurrentItem();
			}
			else
			{
				SendEvent_Item_Drop( item, m_pActorInvOwner->object_id() );
			}
			break;
		}
	case INVENTORY_ATTACH_ADDON:
		AttachAddon( (PIItem)(m_UIPropertiesBox->GetClickedItem()->GetData()) );
		break;
	case INVENTORY_DETACH_SCOPE_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetScopeName().c_str() );
		}
		break;
	case INVENTORY_DETACH_SILENCER_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetSilencerName().c_str() );
		}
		break;
	case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
		if ( weapon )
		{
			DetachAddon( weapon->GetGrenadeLauncherName().c_str() );
		}
		break;
	case INVENTORY_RELOAD_MAGAZINE:
		if ( weapon )
		{
			weapon->Action( kWPN_RELOAD, CMD_START );
		}
		break;
	case INVENTORY_UNLOAD_MAGAZINE:
		{
			CWeaponMagazined* weap_mag = smart_cast<CWeaponMagazined*>( (CWeapon*)cell_item->m_pData );
			if ( !weap_mag )
			{
				break;
			}
			weap_mag->UnloadMagazine();
			for ( u32 i = 0; i < cell_item->ChildsCount(); ++i )
			{
				CUICellItem*		child_itm		= cell_item->Child(i);
				CWeaponMagazined*	child_weap_mag	= smart_cast<CWeaponMagazined*>( (CWeapon*)child_itm->m_pData );
				if ( child_weap_mag )
				{
					child_weap_mag->UnloadMagazine();
				}
			}
			break;
		}
	case INVENTORY_REPAIR:
		{
			TryRepairItem(this,0);
			return;
			break;
		}
	}//switch
	
	SetCurrentItem( NULL );
	UpdateItemsPlace();
}//ProcessPropertiesBoxClicked

void CUIActorMenu::UpdateOutfit()
{
	for ( u8 i = 0; i < e_af_count ; ++i )
	{
		m_belt_list_over[i]->SetVisible( true );
	}
	u32 af_count = m_pActorInvOwner->inventory().BeltWidth();
	VERIFY( 0 <= af_count && af_count <= 5 );

	VERIFY( m_pInventoryBeltList );
	PIItem         ii_outfit = m_pActorInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem;
	CCustomOutfit* outfit    = smart_cast<CCustomOutfit*>( ii_outfit );
	if ( !ii_outfit || !outfit )
	{
		MoveArtefactsToBag();
		return;
	}

	Ivector2 afc;
	afc.x = 1; // m_pInventoryBeltList->GetCellsCapacity().x;
	afc.y = af_count;
	
	m_pInventoryBeltList->SetCellsCapacity( afc );
	for ( u8 i = 0; i < af_count ; ++i )
	{
		m_belt_list_over[i]->SetVisible( false );
	}
}

void CUIActorMenu::MoveArtefactsToBag()
{
	while ( m_pInventoryBeltList->ItemsCount() )
	{
		CUICellItem* ci = m_pInventoryBeltList->GetItemIdx(0);
		VERIFY( ci && ci->m_pData );
		ToBag( ci, false );
	}//for i
	m_pInventoryBeltList->ClearAll( true );
}
