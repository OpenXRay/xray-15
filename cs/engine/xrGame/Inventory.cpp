#include "pch_script.h"
#include "inventory.h"
#include "actor.h"
#include "CustomOutfit.h"
#include "trade.h"
#include "weapon.h"

#include "ui/UIInventoryUtilities.h"

#include "eatable_item.h"
#include "script_engine.h"
#include "xrmessages.h"
//#include "game_cl_base.h"
#include "xr_level_controller.h"
#include "level.h"
#include "ai_space.h"
#include "entitycondition.h"
#include "game_base_space.h"
#include "hudmanager.h"
#include "uigamecustom.h"
#include "clsid_game.h"
#include "static_cast_checked.hpp"
#include "player_hud.h"

using namespace InventoryUtilities;

// what to block
u32	INV_STATE_LADDER		= (1<<RIFLE_SLOT | 1<<APPARATUS_SLOT);
u32	INV_STATE_CAR			= INV_STATE_LADDER;
u32	INV_STATE_BLOCK_ALL		= 0xffffffff;
u32	INV_STATE_INV_WND		= INV_STATE_BLOCK_ALL;
u32	INV_STATE_BUY_MENU		= INV_STATE_BLOCK_ALL;

CInventorySlot::CInventorySlot() 
{
	m_pIItem				= NULL;
	m_bAct					= true;
	m_bPersistent			= false;
	m_blockCounter			= 0;
}

CInventorySlot::~CInventorySlot() 
{
}

bool CInventorySlot::CanBeActivated() const 
{
	return (m_bAct && !IsBlocked());
};

bool CInventorySlot::IsBlocked() const 
{
	return (m_blockCounter>0);
}

CInventory::CInventory() 
{
	m_fMaxWeight								= pSettings->r_float	("inventory","max_weight");
//m_iMaxBelt									= pSettings->r_s32		("inventory","max_belt");
	
	u32 sz										= pSettings->r_s32		("inventory","slots_count");
	m_slots.resize								(sz);
	
	m_iActiveSlot								= NO_ACTIVE_SLOT;
	m_iNextActiveSlot							= NO_ACTIVE_SLOT;
	m_iPrevActiveSlot							= NO_ACTIVE_SLOT;
	//m_iLoadActiveSlot							= NO_ACTIVE_SLOT;

	string256 temp;
	for(u32 i=0; i<m_slots.size(); ++i ) 
	{
		sprintf_s(temp, "slot_persistent_%d", i+1);
		m_slots[i].m_bPersistent = !!pSettings->r_bool("inventory",temp);

		sprintf_s			(temp, "slot_active_%d", i+1);
		m_slots[i].m_bAct	= !!pSettings->r_bool("inventory",temp);
	};

	m_bSlotsUseful								= true;
	m_bBeltUseful								= false;

	m_fTotalWeight								= -1.f;
	m_dwModifyFrame								= 0;
	m_drop_last_frame							= false;
	//m_iLoadActiveSlotFrame						= u32(-1);
}


CInventory::~CInventory() 
{
}

void CInventory::Clear()
{
	m_all.clear							();
	m_ruck.clear						();
	m_belt.clear						();
	
	for(u32 i=0; i<m_slots.size(); i++)
	{
		m_slots[i].m_pIItem				= NULL;
	}
	

	m_pOwner							= NULL;

	CalcTotalWeight						();
	InvalidateState						();
}

void CInventory::Take(CGameObject *pObj, bool bNotActivate, bool strict_placement)
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	VERIFY								(pIItem->m_pInventory==NULL);
	VERIFY								(CanTakeItem(pIItem));
	
	pIItem->m_pInventory				= this;
	pIItem->SetDropManual				(FALSE);
	pIItem->AllowTrade					();
	//if net_Import for pObj arrived then the pObj will pushed to CrPr list (correction prediction)
	//usually net_Import arrived for objects that not has a parent object..
	//for unknown reason net_Import arrived for object that has a parent, so correction prediction schema will crash
	Level().RemoveObject_From_4CrPr		(pObj);

	m_all.push_back						(pIItem);

	if(!strict_placement)
		pIItem->m_eItemCurrPlace			= eItemPlaceUndefined;

	bool result							= false;
	switch(pIItem->m_eItemCurrPlace)
	{
	case eItemPlaceBelt:
		result							= Belt(pIItem, strict_placement); 
		if(!result)
			pIItem->m_eItemCurrPlace	= eItemPlaceUndefined;
#ifdef DEBUG
		if(!result) 
			Msg("cant put in belt item %s", *pIItem->object().cName());
#endif

		break;
	case eItemPlaceRuck:
		result							= Ruck(pIItem, strict_placement);
		if(!result)
			pIItem->m_eItemCurrPlace	= eItemPlaceUndefined;
#ifdef DEBUG
		if(!result) 
			Msg("cant put in ruck item %s", *pIItem->object().cName());
#endif

		break;
	case eItemPlaceSlot:
		result							= Slot(pIItem, bNotActivate, strict_placement); 
		if(!result)
			pIItem->m_eItemCurrPlace	= eItemPlaceUndefined;
#ifdef DEBUG
		if(!result) 
			Msg("cant slot in slot item %s", *pIItem->object().cName());
#endif
		break;
	}

	if(pIItem->m_eItemCurrPlace==eItemPlaceUndefined)
	{
		if( !pIItem->RuckDefault() )
		{
			if( CanPutInSlot(pIItem) )
			{
				result						= Slot(pIItem, bNotActivate,strict_placement); VERIFY(result);
			}else
				if (CanPutInBelt(pIItem))
				{
					result					= Belt(pIItem,strict_placement); VERIFY(result);
				}else
				{
					result					= Ruck(pIItem,strict_placement); VERIFY(result);
				}
		}else
		{
			result						= Ruck(pIItem,strict_placement); VERIFY(result);
		}
	}
	
	m_pOwner->OnItemTake				(pIItem);

	CalcTotalWeight						();
	InvalidateState						();

	pIItem->object().processing_deactivate();
	VERIFY								(pIItem->m_eItemCurrPlace != eItemPlaceUndefined);


	CUI* ui				= HUD().GetUI();
	if( ui && ui->UIGame() )
	{
		CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);

		if (Level().CurrentViewEntity() == pActor_owner)
			ui->UIGame()->OnInventoryAction(pIItem, GE_OWNERSHIP_TAKE);
	};
}

bool CInventory::DropItem(CGameObject *pObj, bool just_before_destroy) 
{
	CInventoryItem *pIItem				= smart_cast<CInventoryItem*>(pObj);
	VERIFY								(pIItem);
	VERIFY								(pIItem->m_pInventory);
	VERIFY								(pIItem->m_pInventory==this);
	VERIFY								(pIItem->m_eItemCurrPlace!=eItemPlaceUndefined);
	
	pIItem->object().processing_activate(); 
	
	switch(pIItem->m_eItemCurrPlace)
	{
	case eItemPlaceBelt:{
			VERIFY(InBelt(pIItem));
			TIItemContainer::iterator temp_iter = std::find(m_belt.begin(), m_belt.end(), pIItem);
			if (temp_iter != m_belt.end())
			{
				m_belt.erase(temp_iter);
			} else
			{
				Msg("! ERROR: CInventory::Drop item not found in belt...");
			}
			pIItem->object().processing_deactivate();
		}break;
	case eItemPlaceRuck:{
			VERIFY(InRuck(pIItem));
			TIItemContainer::iterator temp_iter = std::find(m_ruck.begin(), m_ruck.end(), pIItem);
			if (temp_iter != m_ruck.end())
			{
				m_ruck.erase(temp_iter);
			} else
			{
				Msg("! ERROR: CInventory::Drop item not found in ruck...");
			}
		}break;
	case eItemPlaceSlot:{
			VERIFY			(InSlot(pIItem));
			if(m_iActiveSlot == pIItem->GetSlot())
			{
				CActor* pActor	= smart_cast<CActor*>(m_pOwner);
				if (!pActor || pActor->g_Alive())
				{
					if (just_before_destroy)
					{
#ifdef DEBUG
						Msg("---DropItem activating slot [-1], forced, Frame[%d]", Device.dwFrame);
#endif // #ifdef DEBUG
						Activate		(NO_ACTIVE_SLOT, true);
					} else 
					{
#ifdef DEBUG
						Msg("---DropItem activating slot [-1], Frame[%d]", Device.dwFrame);
#endif // #ifdef DEBUG
						Activate		(NO_ACTIVE_SLOT);
					}
				}
			}
			m_slots[pIItem->GetSlot()].m_pIItem = NULL;							
			pIItem->object().processing_deactivate();
		}break;
	default:
		NODEFAULT;
	};
	TIItemContainer::iterator it = std::find(m_all.begin(), m_all.end(), pIItem);
	if(it!=m_all.end())
		m_all.erase(std::find(m_all.begin(), m_all.end(), pIItem));
	else
		Msg("! CInventory::Drop item not found in inventory!!!");

	pIItem->m_pInventory = NULL;


	m_pOwner->OnItemDrop	(smart_cast<CInventoryItem*>(pObj));

	CalcTotalWeight					();
	InvalidateState					();
	m_drop_last_frame				= true;

	CUI* ui				= HUD().GetUI();
	if( ui && ui->UIGame() )
	{
		CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);

		if (Level().CurrentViewEntity() == pActor_owner)
			ui->UIGame()->OnInventoryAction(pIItem, GE_OWNERSHIP_REJECT);
	};
	pObj->H_SetParent(0, just_before_destroy);
	return							true;
}

//положить вещь в слот
bool CInventory::Slot(PIItem pIItem, bool bNotActivate, bool strict_placement) 
{
	VERIFY(pIItem);
	
	if(m_slots[pIItem->GetSlot()].m_pIItem == pIItem)
		return false;

	if (!IsGameTypeSingle())
	{
		u16 real_parent = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1);
		if (GetOwner()->object_id() != real_parent)
		{
			Msg("! WARNING: CL: actor [%d] tries to place to slot not own item [%d], that has parent [%d]",
				GetOwner()->object_id(), pIItem->object_id(), real_parent);
			return false;
		}
	}


//.	Msg("To Slot %s[%d]", *pIItem->object().cName(), pIItem->object().ID());

	if(!strict_placement && !CanPutInSlot(pIItem)) 
	{
#ifdef _DEBUG
		Msg("there is item %s[%d,%x] in slot %d[%d,%x]", 
				*m_slots[pIItem->GetSlot()].m_pIItem->object().cName(), 
				m_slots[pIItem->GetSlot()].m_pIItem->object().ID(), 
				m_slots[pIItem->GetSlot()].m_pIItem, 
				pIItem->GetSlot(), 
				pIItem->object().ID(),
				pIItem);
#endif
//.		if(m_slots[pIItem->GetSlot()].m_pIItem == pIItem && !bNotActivate )
//.			Activate(pIItem->GetSlot());

		return false;
	}


	m_slots[pIItem->GetSlot()].m_pIItem = pIItem;

	//удалить из рюкзака или пояса
	TIItemContainer::iterator it_ruck = std::find(m_ruck.begin(), m_ruck.end(), pIItem);
	TIItemContainer::iterator it_belt = std::find(m_belt.begin(), m_belt.end(), pIItem);
	if (!IsGameTypeSingle())
	{
		if (it_ruck != m_ruck.end())
		{
			m_ruck.erase(it_ruck);
			R_ASSERT(it_belt == m_belt.end());
		} else if(it_belt != m_belt.end())
		{
			m_belt.erase(it_belt);
			R_ASSERT(it_ruck == m_ruck.end());
		} else
		{
			u16 real_parent = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1);
			R_ASSERT2(GetOwner()->object_id() == real_parent,
				make_string("! ERROR: CL: actor [%d] doesn't contain [%d], real parent is [%d]", 
					GetOwner()->object_id(), pIItem->object_id(), real_parent).c_str()
			);
		}
#ifdef MP_LOGGING
		Msg("--- Actor [%d] places to slot item [%d]", GetOwner()->object_id(), pIItem->object_id());
#endif //#ifdef MP_LOGGING
	} else
	{
		if (it_ruck != m_ruck.end())
			m_ruck.erase(it_ruck);
		if (it_belt != m_belt.end())
			m_belt.erase(it_belt);
	}

	if (((m_iActiveSlot==pIItem->GetSlot()) ||(m_iActiveSlot==NO_ACTIVE_SLOT) && m_iNextActiveSlot==NO_ACTIVE_SLOT) && (!bNotActivate))
	{
#ifdef DEBUG
		Msg("---To Slot: activating slot [%d], Frame[%d]", pIItem->GetSlot(), Device.dwFrame);
#endif // #ifdef DEBUG
		Activate				(pIItem->GetSlot());
	}
	
	m_pOwner->OnItemSlot		(pIItem, pIItem->m_eItemCurrPlace);
	pIItem->m_eItemCurrPlace	= eItemPlaceSlot;
	pIItem->OnMoveToSlot		();
	
	pIItem->object().processing_activate();

	return						true;
}

bool CInventory::Belt(PIItem pIItem, bool strict_placement) 
{
	if(!strict_placement && !CanPutInBelt(pIItem))	return false;
	
	//вещь была в слоте
	bool in_slot = InSlot(pIItem);
	if(in_slot) 
	{
		if(m_iActiveSlot == pIItem->GetSlot()) Activate(NO_ACTIVE_SLOT);
		m_slots[pIItem->GetSlot()].m_pIItem = NULL;
	}
	
	m_belt.insert(m_belt.end(), pIItem); 

	if(!in_slot)
	{
		TIItemContainer::iterator it = std::find(m_ruck.begin(), m_ruck.end(), pIItem); 
		if(m_ruck.end() != it) m_ruck.erase(it);
	}

	CalcTotalWeight();
	InvalidateState();

	EItemPlace p = pIItem->m_eItemCurrPlace;
	pIItem->m_eItemCurrPlace = eItemPlaceBelt;
	m_pOwner->OnItemBelt(pIItem, p);
	pIItem->OnMoveToBelt();

	if(in_slot)
		pIItem->object().processing_deactivate();

	pIItem->object().processing_activate();

	return true;
}

bool CInventory::Ruck(PIItem pIItem, bool strict_placement) 
{
	if(!strict_placement && !CanPutInRuck(pIItem)) return false;

	if (!IsGameTypeSingle())
	{
		u16 real_parent = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1);
		if (GetOwner()->object_id() != real_parent)
		{
			Msg("! WARNING: CL: actor [%d] tries to place to ruck not own item [%d], that has parent [%d]",
				GetOwner()->object_id(), pIItem->object_id(), real_parent);
			return false;
		}
	}
	
	bool in_slot = InSlot(pIItem);
	//вещь была в слоте
	if(in_slot) 
	{
		if(m_iActiveSlot == pIItem->GetSlot()) Activate(NO_ACTIVE_SLOT);
		m_slots[pIItem->GetSlot()].m_pIItem = NULL;
	}
	else
	{
		//вещь была на поясе или вообще только поднята с земли
		TIItemContainer::iterator it = std::find(m_belt.begin(), m_belt.end(), pIItem); 
		if(m_belt.end() != it) m_belt.erase(it);

		if (!IsGameTypeSingle())
		{
			u16 item_parent_id = pIItem->object().H_Parent() ? pIItem->object().H_Parent()->ID() : u16(-1) ;
			u16 inventory_owner_id = GetOwner()->object_id();
			R_ASSERT2(item_parent_id == inventory_owner_id,
				make_string("! ERROR: CL: Actor[%d] tries to place to ruck not own item [%d], real item owner is [%d]",
				inventory_owner_id, pIItem->object_id(), item_parent_id).c_str()
			);
#ifdef MP_LOGGING
			Msg("--- Actor [%d] place to ruck item [%d]", inventory_owner_id, pIItem->object_id());
#endif
		}
	}
	
	m_ruck.insert									(m_ruck.end(), pIItem); 
	
	CalcTotalWeight									();
	InvalidateState									();

	m_pOwner->OnItemRuck							(pIItem, pIItem->m_eItemCurrPlace);
	EItemPlace prev_place							= pIItem->m_eItemCurrPlace;
	pIItem->m_eItemCurrPlace						= eItemPlaceRuck;
	pIItem->OnMoveToRuck							(prev_place);

	if(in_slot)
		pIItem->object().processing_deactivate();

	return true;
}
/*
void CInventory::Activate_deffered	(u32 slot, u32 _frame)
{
	 m_iLoadActiveSlot			= slot;
	 m_iLoadActiveSlotFrame		= _frame;
}*/

PIItem CInventory::GetNextItemInActiveSlot( bool first_call )
{
	PIItem current_item	= m_slots[m_iActiveSlot].m_pIItem;
	PIItem new_item		= NULL;
	bool b				= (current_item == NULL);
	bool found			= false;

	TIItemContainer::const_iterator it		= m_all.begin();
	TIItemContainer::const_iterator it_e	= m_all.end();
	for ( ; it != it_e; ++it )
	{
		PIItem item = *it;
		if ( item == current_item )
		{
			b = true;
			if ( new_item )
			{
				return new_item;
			}
			continue;
		}

		if ( item->GetSlot() == m_iActiveSlot )
		{
			found = true;
			if ( std::find( m_activ_last_items.begin(), m_activ_last_items.end(), item ) == m_activ_last_items.end() )
			{
				new_item = item;
				if ( b )
				{
					return new_item;
				}
			}
		}
	}
	
	m_activ_last_items.clear_not_free();

	if ( first_call && found )
	{
		return GetNextItemInActiveSlot( false ); //m_activ_last_items is full
	}
	return NULL; //only 1 item for this slot
}

bool CInventory::ActivateNextItemInActiveSlot()
{
	if ( m_iActiveSlot == NO_ACTIVE_SLOT )
	{
		return false;
	}
	
	CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);
	if ( Level().CurrentViewEntity() != pActor_owner )
	{
		return false;
	}

	PIItem new_item = GetNextItemInActiveSlot( true );

	if ( new_item == NULL )
	{
		return false; //only 1 item for this slot
	}

	m_activ_last_items.push_back		( new_item );
	PIItem current_item					= m_slots[m_iActiveSlot].m_pIItem;
	
	NET_Packet	P;
	bool		res;
	if ( current_item )
	{
		res = Ruck							(current_item);
		R_ASSERT							(res);
		current_item->object().u_EventGen	(P, GEG_PLAYER_ITEM2RUCK, current_item->object().H_Parent()->ID());
		P.w_u16								(current_item->object().ID());
		current_item->object().u_EventSend	(P);
	}

	res = Slot							(new_item);
	R_ASSERT							(res);
	new_item->object().u_EventGen		(P, GEG_PLAYER_ITEM2SLOT, new_item->object().H_Parent()->ID());
	P.w_u16								(new_item->object().ID());
	new_item->object().u_EventSend		(P);

	//activate
	new_item->object().u_EventGen		(P, GEG_PLAYER_ACTIVATE_SLOT, new_item->object().H_Parent()->ID());
	P.w_u32								(new_item->GetSlot());
	new_item->object().u_EventSend		(P);

//	Msg( "Weapon change" );
	return true;
}

void CInventory::Activate(u32 slot, /*EActivationReason reason, */bool bForce) 
{	
	if(!OnServer())
	{
		return;
	}
	if (m_iActiveSlot==slot || (m_iNextActiveSlot==slot && !bForce))
	{
		m_iNextActiveSlot=slot;
//		Msg("--- There's no need to activate slot [%d], next active slot is [%d]", slot, m_iNextActiveSlot);
		return;
	}
#ifdef DEBUG
	Msg("--- Activating slot [%d], inventory owner: [%s], Frame[%d]", slot, m_pOwner->Name(), Device.dwFrame);
#endif // #ifdef DEBUG

	/*if(Device.dwFrame == m_iLoadActiveSlotFrame) 
	{
		 if( (m_iLoadActiveSlot == slot) && m_slots[slot].m_pIItem )
			m_iLoadActiveSlotFrame = u32(-1);
		 else
			{
			 res = false;
			 goto _finish;
			}

	}*/

	if ( (slot != NO_ACTIVE_SLOT) &&
		(m_slots[slot].IsBlocked()) && 
		(!bForce) )
	{
		return;
	}

	R_ASSERT2(slot == NO_ACTIVE_SLOT || slot<m_slots.size(), "wrong slot number");

	if (slot != NO_ACTIVE_SLOT && !m_slots[slot].m_bAct) 
	{
		return;
	}

	/*if(	m_iActiveSlot == slot || 
		(m_iNextActiveSlot == slot &&
		 m_iActiveSlot != NO_ACTIVE_SLOT &&
		 m_slots[m_iActiveSlot].m_pIItem &&
		 m_slots[m_iActiveSlot].m_pIItem->cast_hud_item() &&
		 m_slots[m_iActiveSlot].m_pIItem->cast_hud_item()->IsHiding()
		 )
	   )
	{
		res = false;
		goto _finish;
	}*/

	//активный слот не выбран
	if (m_iActiveSlot == NO_ACTIVE_SLOT)
	{
		if (m_slots[slot].m_pIItem)
		{
			m_iNextActiveSlot		= slot;
		}
		else 
		{
			if(slot==GRENADE_SLOT)//fake for grenade
			{
				PIItem gr = SameSlot(GRENADE_SLOT, NULL, true);
				if(gr)
				{
					Slot(gr);
				}
			}
		}
	}
	//активный слот задействован
	else if (slot == NO_ACTIVE_SLOT || m_slots[slot].m_pIItem)
	{
		PIItem active_item = m_slots[m_iActiveSlot].m_pIItem;
		if(active_item && !bForce)
		{
			CHudItem* tempItem = active_item->cast_hud_item();
			R_ASSERT2(tempItem, active_item->object().cNameSect().c_str());
			
			tempItem->SendDeactivateItem();

#ifdef DEBUG
			Msg("--- Inventory owner [%s]: send deactivate item [%s]", m_pOwner->Name(), active_item->NameItem());
#endif // #ifdef DEBUG
		} else //in case where weapon is going to destroy
		{
			if ( (slot != NO_ACTIVE_SLOT) && m_slots[slot].m_pIItem )
			{
				m_slots[slot].m_pIItem->ActivateItem();
			}
			m_iActiveSlot			= slot;
		}
		m_iNextActiveSlot		= slot;

	}
}


PIItem CInventory::ItemFromSlot(u32 slot) const
{
	VERIFY(NO_ACTIVE_SLOT != slot);
	return m_slots[slot].m_pIItem;
}

void CInventory::SendActionEvent(s32 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	if (!pActor) return;

	NET_Packet		P;
	pActor->u_EventGen		(P,GE_INV_ACTION, pActor->ID());
	P.w_s32					(cmd);
	P.w_u32					(flags);
	P.w_s32					(pActor->GetZoomRndSeed());
	P.w_s32					(pActor->GetShotRndSeed());
	pActor->u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
};

bool CInventory::Action(s32 cmd, u32 flags) 
{
	CActor *pActor = smart_cast<CActor*>(m_pOwner);
	
	if (pActor)
	{
		switch(cmd)
		{
			case kWPN_FIRE:
			{
				pActor->SetShotRndSeed();
			}break;
			case kWPN_ZOOM : 
			{
				pActor->SetZoomRndSeed();
			}break;
		};
	};

	if (g_pGameLevel && OnClient() && pActor) 
	{
		switch(cmd)
		{
		case kUSE:		break;
		
		case kDROP:		
			{
				if ((flags & CMD_STOP) && !IsGameTypeSingle())
				{
					PIItem tmp_item = ActiveItem();
					if (tmp_item)
					{
						tmp_item->DenyTrade();
					}
				}
				SendActionEvent	(cmd, flags);
				return			true;
			}break;

		case kWPN_NEXT:
		case kWPN_RELOAD:
		case kWPN_FIRE:
		case kWPN_FUNC:
		case kWPN_FIREMODE_NEXT:
		case kWPN_FIREMODE_PREV:
		case kWPN_ZOOM : 
		case kTORCH:
		case kNIGHT_VISION:
			{
				SendActionEvent(cmd, flags);
			}break;
		}
	}


	if (m_iActiveSlot < m_slots.size() && 
			m_slots[m_iActiveSlot].m_pIItem && 
			!m_slots[m_iActiveSlot].m_pIItem->object().getDestroy() &&
			m_slots[m_iActiveSlot].m_pIItem->Action(cmd, flags)) 
											return true;
	bool b_send_event = false;
	switch(cmd) 
	{
	case kWPN_1:
	case kWPN_2:
	case kWPN_3:
	case kWPN_4:
	case kWPN_5:
	case kWPN_6:
		{
			b_send_event = true;
			if (cmd == kWPN_6 && !IsGameTypeSingle()) return false;
			
			u32 slot = cmd - kWPN_1;
			if ( flags & CMD_START )
			{
				ActiveWeapon( slot );
			}
		}break;
	case kARTEFACT:
		{
		    b_send_event = true;
			if(flags&CMD_START)
			{
                if((int)m_iActiveSlot == ARTEFACT_SLOT &&
					m_slots[m_iActiveSlot].m_pIItem /*&& IsGameTypeSingle()*/)
				{
					Activate(NO_ACTIVE_SLOT);
				}else {
					Activate(ARTEFACT_SLOT);
				}
			}
		}break;
	}

	if(b_send_event && g_pGameLevel && OnClient() && pActor)
			SendActionEvent(cmd, flags);

	return false;
}

void CInventory::ActiveWeapon( u32 slot )
{
	// weapon is in active slot
	if ( m_iActiveSlot == slot && m_slots[m_iActiveSlot].m_pIItem )
	{
		if ( IsGameTypeSingle() )
		{
			Activate(NO_ACTIVE_SLOT);
		}
		else
		{
			ActivateNextItemInActiveSlot();
		}
		return;
	}

	if ( IsGameTypeSingle() )
	{
		Activate(slot);
		return;
	}
	if ( m_iActiveSlot == slot )
	{
		return;
	}

	Activate(slot);
	if ( slot != NO_ACTIVE_SLOT && m_slots[slot].m_pIItem == NULL )
	{
		u32 prev_activ = m_iActiveSlot;
		m_iActiveSlot  = slot;
		if ( !ActivateNextItemInActiveSlot() )
		{
			m_iActiveSlot = prev_activ;
		}
	}

}

void CInventory::Update() 
{
/*
	CObject*	curControl	= Level().CurrentControlEntity();
	CObject*	parentObj	= m_pOwner ? m_pOwner->cast_game_object() : NULL;
*/
/*	if ((m_iNextActiveSlot == NO_ACTIVE_SLOT) || 
		(m_iNextActiveSlot == m_iActiveSlot))
*/
/*
	if (m_iNextActiveSlot==m_iActiveSlot)
	{
		UpdateDropTasks();
		return;
	} 
*/
/*	
	if (curControl && parentObj)
	{
		CHudItem* activeItem = NULL;
		if ((m_iActiveSlot != NO_ACTIVE_SLOT) &&
			(m_slots[m_iActiveSlot].m_pIItem))
		{
			activeItem = m_slots[m_iActiveSlot].m_pIItem->cast_hud_item();
		}
		
		if ((curControl->ID() == parentObj->ID()) && activeItem && 	!activeItem->IsHidden())
		{
			UpdateDropTasks();
			return;
		}
	}
*/
	if( OnServer() )
	{
		if(m_iActiveSlot!=m_iNextActiveSlot)
		{
			CObject* pActor_owner = smart_cast<CObject*>(m_pOwner);
			if (Level().CurrentViewEntity() == pActor_owner)
			{
				if(	(m_iNextActiveSlot!=NO_ACTIVE_SLOT) && 
					 m_slots[m_iNextActiveSlot].m_pIItem &&
					 !g_player_hud->allow_activation(m_slots[m_iNextActiveSlot].m_pIItem->cast_hud_item())
				   )
				   return;
			}
			if( (m_iActiveSlot!=NO_ACTIVE_SLOT) && m_slots[m_iActiveSlot].m_pIItem )
			{
				if(!m_slots[m_iActiveSlot].m_pIItem->cast_hud_item()->IsHidden())
				{
//.					PIItem itm = m_slots[m_iActiveSlot].m_pIItem;
//.					Msg("waiting for [%s] state=%d",itm->object().cName().c_str(), itm->cast_hud_item()->GetState() );
					UpdateDropTasks	();
					return;
				}
			}

			if (GetNextActiveSlot() != NO_ACTIVE_SLOT)
			{
				PIItem tmp_next_active = ItemFromSlot(GetNextActiveSlot());
				if (tmp_next_active)
				{
					u32 tmp_slot = tmp_next_active->GetSlot();
					if ((tmp_slot != NO_ACTIVE_SLOT) && (m_slots[tmp_slot].IsBlocked()))
					{
						Activate(m_iActiveSlot);
						return;
					}			
					tmp_next_active->ActivateItem();
				}
			}
			m_iActiveSlot			= m_iNextActiveSlot;
		}
		if((m_iNextActiveSlot!=NO_ACTIVE_SLOT) && m_slots[m_iActiveSlot].m_pIItem && m_slots[m_iActiveSlot].m_pIItem->cast_hud_item()->IsHidden())
				m_slots[m_iActiveSlot].m_pIItem->ActivateItem();
	}
	UpdateDropTasks	();
}

void CInventory::UpdateDropTasks()
{
	//проверить слоты
	for(u32 i=0; i<m_slots.size(); ++i)	
	{
		if(m_slots[i].m_pIItem)
			UpdateDropItem		(m_slots[i].m_pIItem);
	}

	for(i = 0; i < 2; ++i)	
	{
		TIItemContainer &list			= i?m_ruck:m_belt;
		TIItemContainer::iterator it	= list.begin();
		TIItemContainer::iterator it_e	= list.end();
	
		for( ;it!=it_e; ++it)
		{
			UpdateDropItem		(*it);
		}
	}

	if (m_drop_last_frame)
	{
		m_drop_last_frame			= false;
		m_pOwner->OnItemDropUpdate	();
	}
}

void CInventory::UpdateDropItem(PIItem pIItem)
{
	if( pIItem->GetDropManual() )
	{
		pIItem->SetDropManual(FALSE);
		pIItem->DenyTrade();

		if ( OnServer() ) 
		{
			NET_Packet					P;
			pIItem->object().u_EventGen	(P, GE_OWNERSHIP_REJECT, pIItem->object().H_Parent()->ID());
			P.w_u16						(u16(pIItem->object().ID()));
			pIItem->object().u_EventSend(P);
		}
	}// dropManual
}

//ищем на поясе гранату такоже типа
PIItem CInventory::Same(const PIItem pIItem, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		const PIItem l_pIItem = *it;
		
		if((l_pIItem != pIItem) && 
				!xr_strcmp(l_pIItem->object().cNameSect(), 
				pIItem->object().cNameSect())) 
			return l_pIItem;
	}
	return NULL;
}

//ищем на поясе вещь для слота 

PIItem CInventory::SameSlot(const u32 slot, PIItem pIItem, bool bSearchRuck) const
{
	if(slot == NO_ACTIVE_SLOT) 	return NULL;

	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem _pIItem = *it;
		if(_pIItem != pIItem && _pIItem->GetSlot() == slot) return _pIItem;
	}

	return NULL;
}

//найти в инвенторе вещь с указанным именем
PIItem CInventory::Get(const char *name, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(!xr_strcmp(pIItem->object().cNameSect(), name) && 
								pIItem->Useful()) 
				return pIItem;
	}
	return NULL;
}

PIItem CInventory::Get(CLASS_ID cls_id, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;
	
	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem->object().CLS_ID == cls_id && 
								pIItem->Useful()) 
				return pIItem;
	}
	return NULL;
}

PIItem CInventory::Get(const u16 id, bool bSearchRuck) const
{
	const TIItemContainer &list = bSearchRuck ? m_ruck : m_belt;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem->object().ID() == id) 
			return pIItem;
	}
	return NULL;
}

//search both (ruck and belt)
PIItem CInventory::GetAny(const char *name) const
{
	PIItem itm = Get(name, false);
	if(!itm)
		itm = Get(name, true);
	return itm;
}

PIItem CInventory::item(CLASS_ID cls_id) const
{
	const TIItemContainer &list = m_all;

	for(TIItemContainer::const_iterator it = list.begin(); list.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(pIItem->object().CLS_ID == cls_id && 
			pIItem->Useful()) 
			return pIItem;
	}
	return NULL;
}

float CInventory::TotalWeight() const
{
	VERIFY(m_fTotalWeight>=0.f);
	return m_fTotalWeight;
}


float CInventory::CalcTotalWeight()
{
	float weight = 0;
	for(TIItemContainer::const_iterator it = m_all.begin(); m_all.end() != it; ++it) 
		weight += (*it)->Weight();

	m_fTotalWeight = weight;
	return m_fTotalWeight;
}


u32 CInventory::dwfGetSameItemCount(LPCSTR caSection, bool SearchAll)
{
	u32			l_dwCount = 0;
	TIItemContainer	&l_list = SearchAll ? m_all : m_ruck;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (!xr_strcmp(l_pIItem->object().cNameSect(), caSection))
            ++l_dwCount;
	}
	
	return		(l_dwCount);
}
u32		CInventory::dwfGetGrenadeCount(LPCSTR caSection, bool SearchAll)
{
	u32			l_dwCount = 0;
	TIItemContainer	&l_list = SearchAll ? m_all : m_ruck;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem->object().CLS_ID == CLSID_GRENADE_F1 || l_pIItem->object().CLS_ID == CLSID_GRENADE_RGD5)
			++l_dwCount;
	}

	return		(l_dwCount);
}

bool CInventory::bfCheckForObject(ALife::_OBJECT_ID tObjectID)
{
	TIItemContainer	&l_list = m_all;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem->object().ID() == tObjectID)
			return(true);
	}
	return		(false);
}

CInventoryItem *CInventory::get_object_by_id(ALife::_OBJECT_ID tObjectID)
{
	TIItemContainer	&l_list = m_all;
	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it) 
	{
		PIItem	l_pIItem = *l_it;
		if (l_pIItem->object().ID() == tObjectID)
			return	(l_pIItem);
	}
	return		(0);
}

//скушать предмет 
#include "game_object_space.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
bool CInventory::Eat(PIItem pIItem)
{
	//устанаовить съедобна ли вещь
	CEatableItem* pItemToEat = smart_cast<CEatableItem*>(pIItem);
	if ( !pItemToEat )			return false;

	CEntityAlive *entity_alive = smart_cast<CEntityAlive*>(m_pOwner);
	if ( !entity_alive )		return false;

	CInventoryOwner* IO	= smart_cast<CInventoryOwner*>(entity_alive);
	if ( !IO )					return false;
	
	CInventory* pInventory = pItemToEat->m_pInventory;
	if ( !pInventory || pInventory != this )	return false;
	if ( pInventory != IO->m_inventory )		return false;
	if ( pItemToEat->object().H_Parent()->ID() != entity_alive->ID() )		return false;
	
	pItemToEat->UseBy			(entity_alive);

#ifdef MP_LOGGING
	Msg( "--- Actor [%d] use or eat [%d][%s]", entity_alive->ID(), pItemToEat->object().ID(), pItemToEat->object().cNameSect().c_str() );
#endif // MP_LOGGING

	if(IsGameTypeSingle() && Actor()->m_inventory == this)
		Actor()->callback(GameObject::eUseObject)((smart_cast<CGameObject*>(pIItem))->lua_game_object());

	if(pItemToEat->Empty())
	{
		pIItem->SetDropManual(TRUE);
		return		false;
	}
	return			true;
}

bool CInventory::InSlot(PIItem pIItem) const
{
	if(pIItem->GetSlot() < m_slots.size() && 
		m_slots[pIItem->GetSlot()].m_pIItem == pIItem)
		return true;
	return false;
}
bool CInventory::InBelt(PIItem pIItem) const
{
	if(Get(pIItem->object().ID(), false)) return true;
	return false;
}
bool CInventory::InRuck(PIItem pIItem) const
{
	if(Get(pIItem->object().ID(), true)) return true;
	return false;
}


bool CInventory::CanPutInSlot(PIItem pIItem) const
{
	if(!m_bSlotsUseful) return false;

	if( !GetOwner()->CanPutInSlot(pIItem, pIItem->GetSlot() ) ) return false;

	if(pIItem->GetSlot() < m_slots.size() && 
		m_slots[pIItem->GetSlot()].m_pIItem == NULL )
		return true;
	
	return false;
}
//проверяет можем ли поместить вещь на пояс,
//при этом реально ничего не меняется
bool CInventory::CanPutInBelt(PIItem pIItem)
{
	if(InBelt(pIItem))					return false;
	if(!m_bBeltUseful)					return false;
	if(!pIItem || !pIItem->Belt())		return false;
	if(m_belt.size() >= BeltWidth())	return false;

	return FreeRoom_inBelt(m_belt, pIItem, BeltWidth(), 1);
}
//проверяет можем ли поместить вещь в рюкзак,
//при этом реально ничего не меняется
bool CInventory::CanPutInRuck(PIItem pIItem) const
{
	if(InRuck(pIItem)) return false;
	return true;
}

u32	CInventory::dwfGetObjectCount()
{
	return		(m_all.size());
}

CInventoryItem	*CInventory::tpfGetObjectByIndex(int iIndex)
{
	if ((iIndex >= 0) && (iIndex < (int)m_all.size())) {
		TIItemContainer	&l_list = m_all;
		int			i = 0;
		for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it, ++i) 
			if (i == iIndex)
                return	(*l_it);
	}
	else {
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"invalid inventory index!");
		return	(0);
	}
	R_ASSERT	(false);
	return		(0);
}

CInventoryItem	*CInventory::GetItemFromInventory(LPCSTR caItemName)
{
	TIItemContainer	&l_list = m_all;

	u32 crc = crc32(caItemName, xr_strlen(caItemName));

	for(TIItemContainer::iterator l_it = l_list.begin(); l_list.end() != l_it; ++l_it)
		if ((*l_it)->object().cNameSect()._get()->dwCRC == crc){
			VERIFY(	0 == xr_strcmp( (*l_it)->object().cNameSect().c_str(), caItemName)  );
			return	(*l_it);
		}
	return	(0);
}


bool CInventory::CanTakeItem(CInventoryItem *inventory_item) const
{
	VERIFY			(inventory_item);
	VERIFY			(m_pOwner);

	if (inventory_item->object().getDestroy()) return false;

	if(!inventory_item->CanTake()) return false;

	for(TIItemContainer::const_iterator it = m_all.begin(); it != m_all.end(); it++)
		if((*it)->object().ID() == inventory_item->object().ID()) break;
	VERIFY3(it == m_all.end(), "item already exists in inventory",*inventory_item->object().cName());

	CActor* pActor = smart_cast<CActor*>(m_pOwner);
	//актер всегда может взять вещь
	if(!pActor && (TotalWeight() + inventory_item->Weight() > m_pOwner->MaxCarryWeight()))
		return	false;

	return	true;
}


u32  CInventory::BeltWidth() const
{
	CActor* pActor = smart_cast<CActor*>( m_pOwner );
	if ( pActor )
	{
		CCustomOutfit* outfit = pActor->GetOutfit();
		if ( outfit )
		{
			return outfit->get_artefact_count();
		}
	}
	return 0; //m_iMaxBelt;
}

void  CInventory::AddAvailableItems(TIItemContainer& items_container, bool for_trade) const
{
	for(TIItemContainer::const_iterator it = m_ruck.begin(); m_ruck.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		if(!for_trade || pIItem->CanTrade())
			items_container.push_back(pIItem);
	}

	if(m_bBeltUseful)
	{
		for(TIItemContainer::const_iterator it = m_belt.begin(); m_belt.end() != it; ++it) 
		{
			PIItem pIItem = *it;
			if(!for_trade || pIItem->CanTrade())
				items_container.push_back(pIItem);
		}
	}
	
	if(m_bSlotsUseful)
	{
		TISlotArr::const_iterator slot_it			= m_slots.begin();
		TISlotArr::const_iterator slot_it_e			= m_slots.end();
		for(;slot_it!=slot_it_e;++slot_it)
		{
			const CInventorySlot& S = *slot_it;
			if(S.m_pIItem && (!for_trade || S.m_pIItem->CanTrade())  )
			{
				if(!S.m_bPersistent || S.m_pIItem->GetSlot()==GRENADE_SLOT )
					items_container.push_back(S.m_pIItem);
			}
		}
	}		
}

bool CInventory::isBeautifulForActiveSlot	(CInventoryItem *pIItem)
{
	if (!IsGameTypeSingle()) return (true);
	TISlotArr::iterator it =  m_slots.begin();
	for( ; it!=m_slots.end(); ++it) {
		if ((*it).m_pIItem && (*it).m_pIItem->IsNecessaryItem(pIItem))
			return		(true);
	}
	return				(false);
}

//.#include "WeaponHUD.h"
void CInventory::Items_SetCurrentEntityHud(bool current_entity)
{
	TIItemContainer::iterator it;
	for(it = m_all.begin(); m_all.end() != it; ++it) 
	{
		PIItem pIItem = *it;
		CWeapon* pWeapon = smart_cast<CWeapon*>(pIItem);
		if (pWeapon)
		{
			pWeapon->InitAddons();
			pWeapon->UpdateAddonsVisibility();
		}
	}
};

void  CInventory::SetPrevActiveSlot(u32 ActiveSlot)	
{
	m_iPrevActiveSlot = ActiveSlot;
}

//call this only via Actor()->SetWeaponHideState()
void CInventory::SetSlotsBlocked(u16 mask, bool bBlock)
{
	R_ASSERT(OnServer() || Level().IsDemoPlayStarted());

	bool bChanged = false;
	for(u32 i =0; i<m_slots.size(); ++i)
	{
		if(mask & (1<<i))
		{
			bool bCanBeActivated = m_slots[i].CanBeActivated();
			if(bBlock){
				++m_slots[i].m_blockCounter;
				VERIFY2(m_slots[i].m_blockCounter< 5,"block slots overflow");
			}else{
				--m_slots[i].m_blockCounter;
				VERIFY2(m_slots[i].m_blockCounter>-5,"block slots underflow");
			}
			if(bCanBeActivated != m_slots[i].CanBeActivated())
				bChanged = true;
		}
	}
	if(bChanged)
	{
		u32 ActiveSlot		= GetActiveSlot();
		u32 PrevActiveSlot	= GetPrevActiveSlot();
		if(ActiveSlot==NO_ACTIVE_SLOT)
		{//try to restore hidden weapon
			if(PrevActiveSlot!=NO_ACTIVE_SLOT && m_slots[PrevActiveSlot].CanBeActivated())
			{
#ifndef MASTER_GOLD
				Msg("Set slots blocked: activating prev slot [%d], Frame[%d]", PrevActiveSlot, Device.dwFrame);
#endif // #ifndef MASTER_GOLD
				Activate(PrevActiveSlot);
				SetPrevActiveSlot(NO_ACTIVE_SLOT);
			}
		}else
		{//try to hide active weapon
			if(!m_slots[ActiveSlot].CanBeActivated())
			{
#ifndef MASTER_GOLD
				Msg("Set slots blocked: activating slot [-1], Frame[%d]", Device.dwFrame);
#endif // #ifndef MASTER_GOLD
				Activate(NO_ACTIVE_SLOT);
				SetPrevActiveSlot(ActiveSlot);
			}
		}
	}
}
