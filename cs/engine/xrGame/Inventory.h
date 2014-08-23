#pragma once
#include "inventory_item.h"

class CInventory;
class CInventoryItem;
class CHudItem;
class CInventoryOwner;

class CInventorySlot
{									
public:
							CInventorySlot		();
	virtual					~CInventorySlot		();

	bool					CanBeActivated		() const;
	bool					IsBlocked			() const;

	PIItem					m_pIItem;
	bool					m_bPersistent;
	bool					m_bAct;
	int						m_blockCounter;
};
/*
enum EActivationReason{
	eGeneral,
	eKeyAction,
	eImportUpdate,
};*/

typedef xr_vector<CInventorySlot> TISlotArr;


class CInventory
{				
public:
							CInventory			();
	virtual					~CInventory			();

	float 					TotalWeight			() const;
	float 					CalcTotalWeight		();

	void					Take				(CGameObject *pObj, bool bNotActivate, bool strict_placement);
	//if just_before_destroy is true, then activate will be forced (because deactivate message will not deliver)
	bool					DropItem			(CGameObject *pObj, bool just_before_destroy);
	void					Clear				();

	
	bool					Slot				(PIItem pIItem, bool bNotActivate = false, bool strict_placement=false);	
	bool					Belt				(PIItem pIItem, bool strict_placement=false);
	bool					Ruck				(PIItem pIItem, bool strict_placement=false);

	bool 					InSlot				(PIItem pIItem) const;
	bool 					InBelt				(PIItem pIItem) const;
	bool 					InRuck				(PIItem pIItem) const;

	bool 					CanPutInSlot		(PIItem pIItem) const;
	bool 					CanPutInBelt		(PIItem pIItem);
	bool 					CanPutInRuck		(PIItem pIItem) const;

	bool					CanTakeItem			(CInventoryItem *inventory_item) const;


	void					Activate			(u32 slot, /*EActivationReason reason=eGeneral, */bool bForce=false);
	//void					Activate_deffered	(u32 slot, u32 _frame);
	PIItem					GetNextItemInActiveSlot( bool first_call );
	bool					ActivateNextItemInActiveSlot();
	PIItem					ActiveItem			()const					{return m_iActiveSlot==NO_ACTIVE_SLOT ? NULL :m_slots[m_iActiveSlot].m_pIItem;}
	PIItem					ItemFromSlot		(u32 slot) const;

	bool					Action				(s32 cmd, u32 flags);
	void					ActiveWeapon		(u32 slot);
	void					Update				();
	// Ищет на поясе аналогичный IItem
	PIItem					Same				(const PIItem pIItem, bool bSearchRuck) const;
	// Ищет на поясе IItem для указанного слота
	PIItem					SameSlot			(const u32 slot, PIItem pIItem, bool bSearchRuck) const;
	// Ищет на поясе или в рюкзаке IItem с указанным именем (cName())
	PIItem					Get					(const char *name, bool bSearchRuck) const;
	// Ищет на поясе или в рюкзаке IItem с указанным именем (id)
	PIItem					Get					(const u16  id,	 bool bSearchRuck) const;
	// Ищет на поясе или в рюкзаке IItem с указанным CLS_ID
	PIItem					Get					(CLASS_ID cls_id,  bool bSearchRuck) const;
	PIItem					GetAny				(const char *name) const;//search both (ruck and belt)
	PIItem					item				(CLASS_ID cls_id) const;
	
	// get all the items with the same section name
	virtual u32				dwfGetSameItemCount	(LPCSTR caSection, bool SearchAll = false);	
	virtual u32				dwfGetGrenadeCount	(LPCSTR caSection, bool SearchAll);	
	// get all the items with the same object id
	virtual bool			bfCheckForObject	(ALife::_OBJECT_ID tObjectID);	
	PIItem					get_object_by_id	(ALife::_OBJECT_ID tObjectID);

	u32						dwfGetObjectCount	();
	PIItem					tpfGetObjectByIndex	(int iIndex);
	PIItem					GetItemFromInventory(LPCSTR caItemName);

	bool					Eat					(PIItem pIItem);								

	u32						GetActiveSlot		() const			{return m_iActiveSlot;}
	
	void					SetPrevActiveSlot	(u32 ActiveSlot);
	u32						GetPrevActiveSlot	() const			{return m_iPrevActiveSlot;}
	u32						GetNextActiveSlot	() const			{return m_iNextActiveSlot;}

	void					SetActiveSlot		(u32 ActiveSlot)	{m_iActiveSlot = m_iNextActiveSlot = ActiveSlot; }

	bool 					IsSlotsUseful		() const			{return m_bSlotsUseful;}	 
	void 					SetSlotsUseful		(bool slots_useful) {m_bSlotsUseful = slots_useful;}
	bool 					IsBeltUseful		() const			{return m_bBeltUseful;}
	void 					SetBeltUseful		(bool belt_useful)	{m_bBeltUseful = belt_useful;}

	void					SetSlotsBlocked		(u16 mask, bool bBlock);
	TIItemContainer			m_all;
	TIItemContainer			m_ruck, m_belt;
	TIItemContainer			m_activ_last_items;
	TISlotArr				m_slots;

	//возвращает все кроме PDA в слоте и болта
	void				AddAvailableItems			(TIItemContainer& items_container, bool for_trade) const;

	float				GetMaxWeight				() const				{return m_fMaxWeight;}
	void				SetMaxWeight				(float weight)			{m_fMaxWeight = weight;}

	u32					BeltWidth					() const;

	inline	CInventoryOwner*GetOwner				() const				{ return m_pOwner; }
	

	friend class CInventoryOwner;


	u32					ModifyFrame					() const					{ return m_dwModifyFrame; }
	void				InvalidateState				()							{ m_dwModifyFrame = Device.dwFrame; }
	void				Items_SetCurrentEntityHud	(bool current_entity);
	bool				isBeautifulForActiveSlot	(CInventoryItem *pIItem);
protected:
	void					UpdateDropTasks		();
	void					UpdateDropItem		(PIItem pIItem);

	// Активный слот и слот который станет активным после смены
    //значения совпадают в обычном состоянии (нет смены слотов)
	u32 				m_iActiveSlot;
	u32 				m_iNextActiveSlot;
	u32 				m_iPrevActiveSlot;
	//u32 				m_iLoadActiveSlot;
	//u32 				m_iLoadActiveSlotFrame;
	//EActivationReason	m_ActivationSlotReason;

	CInventoryOwner*	m_pOwner;

	//флаг, показывающий наличие пояса в инвенторе
	bool				m_bBeltUseful;
	//флаг, допускающий использование слотов
	bool				m_bSlotsUseful;

	// максимальный вес инвентаря
	float				m_fMaxWeight;
	// текущий вес в инвентаре
	float				m_fTotalWeight;

	// Максимальное кол-во объектов
	//на поясе
///	u32					m_iMaxBelt;	= outfit->get_artefact_count();

	//кадр на котором произошло последнее изменение в инвенторе
	u32					m_dwModifyFrame;

	bool				m_drop_last_frame;

	void				SendActionEvent		(s32 cmd, u32 flags);
};