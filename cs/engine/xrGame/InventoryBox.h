#pragma once
#include "inventory_space.h"
#include "GameObject.h"

class CInventoryBox :public CGameObject
{
	typedef CGameObject									inherited;
public:
	xr_vector<u16>										m_items;
				bool	m_in_use;
						CInventoryBox					();
	virtual		void	OnEvent							(NET_Packet& P, u16 type);
	virtual		BOOL	net_Spawn						(CSE_Abstract* DC);
	virtual		void	net_Destroy						();
	virtual		void	net_Relcase						(CObject* O	);
				void	AddAvailableItems				(TIItemContainer& items_container) const;
				bool	IsEmpty							() {return m_items.empty();}
	virtual		void	UpdateCL						();
};