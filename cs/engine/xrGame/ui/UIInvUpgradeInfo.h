////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInvUpgradeInfo.h
//	Created 	: 21.11.2007
//  Modified 	: 27.11.2007
//	Author		: Evgeniy Sokolov
//	Description : inventory upgrade UI info window class
////////////////////////////////////////////////////////////////////////////

#ifndef UI_INVENTORY_UPGRADE_INFO_H_INCLUDED
#define UI_INVENTORY_UPGRADE_INFO_H_INCLUDED

#include "UIWindow.h"
#include "xrUIXmlParser.h"


namespace inventory { namespace upgrade {
	class Upgrade;
} } // namespace upgrade, inventory

class CUIStatic;
class CUIFrameWindow;
class UIInvUpgPropertiesWnd;
class CInventoryItem;

class UIInvUpgradeInfo: public CUIWindow
{
private:
	typedef CUIWindow                    inherited;
	typedef inventory::upgrade::Upgrade  Upgrade_type;

public:
						UIInvUpgradeInfo();
	virtual				~UIInvUpgradeInfo();

			void		init_from_xml( LPCSTR xml_name );
			bool		init_upgrade( Upgrade_type* upgr, CInventoryItem* inv_item );
			bool		is_upgrade() { return (m_upgrade != NULL); }
	IC Upgrade_type const*	get_upgrade() const { return m_upgrade; }

//	virtual void		Update();
	virtual void		Draw();

protected:
	Upgrade_type*		m_upgrade;
	CUIFrameWindow*		m_background;
	
	UIInvUpgPropertiesWnd*	m_properties_wnd;

	CUIStatic*			m_name;
	CUIStatic*			m_desc;
	CUIStatic*			m_prereq;
	
}; // class UIInvUpgradeInfo

#endif // UI_INVENTORY_UPGRADE_INFO_H_INCLUDED
