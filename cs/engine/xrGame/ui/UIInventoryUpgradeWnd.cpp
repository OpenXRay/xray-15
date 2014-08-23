////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInventoryUpgradeWnd.cpp
//	Created 	: 06.10.2007
//	Author		: Evgeniy Sokolov
//	Description : inventory upgrade UI window class implementation
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "object_broker.h"
#include "UIInventoryUpgradeWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "../string_table.h"

#include "../actor.h"
#include "../../xrServerEntities/script_process.h"
#include "../inventory.h"

#include "ai_space.h"
#include "alife_simulator.h"
#include "inventory_upgrade_manager.h"
#include "inventory_upgrade.h"
#include "inventory_upgrade_property.h"

#include "UIActorMenu.h"
#include "UIItemInfo.h"
#include "UIFrameLineWnd.h"
#include "UI3tButton.h"
#include "UIHelper.h"

// -----

const LPCSTR g_inventory_upgrade_xml = "inventory_upgrade.xml";

CUIInventoryUpgradeWnd::Scheme::Scheme()
{
}

CUIInventoryUpgradeWnd::Scheme::~Scheme()
{
	delete_data( cells );
}

// =============================================================================================

CUIInventoryUpgradeWnd::CUIInventoryUpgradeWnd()
{
	m_inv_item       = NULL;
	m_cur_upgrade_id = NULL;
	m_current_scheme = NULL;
	m_btn_repair     = NULL;
}

CUIInventoryUpgradeWnd::~CUIInventoryUpgradeWnd()
{
	delete_data( m_schemes );
}

void CUIInventoryUpgradeWnd::Init()
{
	CUIXml uiXml;
	uiXml.Load( CONFIG_PATH, UI_PATH, g_inventory_upgrade_xml );
	
	CUIXmlInit xml_init;
	xml_init.InitWindow( uiXml, "main", 0, this );
	m_border_texture = uiXml.ReadAttrib( "border", 0, "texture" );
	m_ink_texture    = uiXml.ReadAttrib( "inking",  0, "texture" );
	VERIFY( m_border_texture.size() );
	VERIFY( m_ink_texture.size() );

	xml_init.InitWindow( uiXml, "main", 0, this );

	m_background = UIHelper::CreateStatic( uiXml, "background", this );
//	m_delimiter  = UIHelper::CreateFrameLine( uiXml, "delimiter", this );

	m_scheme_wnd = xr_new<CUIWindow>();
	m_scheme_wnd->SetAutoDelete( true );
	AttachChild( m_scheme_wnd );
	xml_init.InitWindow( uiXml, "scheme", 0, m_scheme_wnd );

	m_item_info = xr_new<CUIItemInfo>();
	m_item_info->SetAutoDelete( true );
	AttachChild( m_item_info );
	m_item_info->InitItemInfo( "inventory_upgrade_info.xml" );
//	m_info_orig_pos.set( m_item_info->GetWndPos() );
	
	m_btn_repair = UIHelper::Create3tButtonEx( uiXml, "repair_button", this );
	CUIActorMenu* parent_wnd = smart_cast<CUIActorMenu*>(m_pParentWnd);
	if ( parent_wnd )
	{
		m_btn_repair->set_hint_wnd( parent_wnd->get_hint_wnd() );
	}

	LoadCellsBacks( uiXml );
	LoadSchemes( uiXml );
}

void CUIInventoryUpgradeWnd::InitInventory( CInventoryItem* item, bool can_upgrade )
{
	m_inv_item = item;
	m_item_info->InitItem( item );
	
	m_scheme_wnd->DetachAll();
	m_scheme_wnd->Show( false );
	m_btn_repair->Enable( false );
	
	if ( ai().get_alife() && m_inv_item )
	{
		if ( install_item( *m_inv_item, can_upgrade ) )
		{
			UpdateAllUpgrades();
		}
	}
}

// ------------------------------------------------------------------------------------------

void CUIInventoryUpgradeWnd::Show( bool status )
{ 
	inherited::Show( status );
	UpdateAllUpgrades();
}

void CUIInventoryUpgradeWnd::Update()
{
	inherited::Update();
//	UpdateAllUpgrades();
}

void CUIInventoryUpgradeWnd::Reset()
{
	SCHEMES::iterator ibw = m_schemes.begin();
	SCHEMES::iterator iew = m_schemes.end();
	for ( ; ibw != iew; ++ibw )
	{
		UI_Upgrades_type::iterator ib = (*ibw)->cells.begin();
		UI_Upgrades_type::iterator ie = (*ibw)->cells.end();
		for ( ; ib != ie; ++ib )
		{
			(*ib)->Reset();
		}
	}
	inherited::Reset();
	inherited::ResetAll();
}

void CUIInventoryUpgradeWnd::UpdateAllUpgrades()
{
	if ( !m_current_scheme || !m_inv_item )
	{
		return;
	}
	
	UI_Upgrades_type::iterator ib = m_current_scheme->cells.begin();
	UI_Upgrades_type::iterator ie = m_current_scheme->cells.end();
	for ( ; ib != ie; ++ib )
	{
		(*ib)->update_item( m_inv_item );
	}
}

void CUIInventoryUpgradeWnd::SetCurScheme( const shared_str& id )
{
	SCHEMES::iterator ib = m_schemes.begin();
	SCHEMES::iterator ie = m_schemes.end();
	for ( ; ib != ie; ++ib )
	{
		if ( (*ib)->name._get() == id._get() )
		{
			m_current_scheme = (*ib);
			return;
		}
	}
	VERIFY2( 0, make_string( "Scheme <%s> does not loaded !", id.c_str() ) );
}

bool CUIInventoryUpgradeWnd::install_item( CInventoryItem& inv_item, bool can_upgrade )
{
	m_scheme_wnd->DetachAll();
	m_btn_repair->Enable( (inv_item.GetCondition() < 0.99f) );

	if ( !can_upgrade )
	{
#ifdef DEBUG
		Msg( "Inventory item <%s> cannot upgrade - Mechanic say.", inv_item.m_section_id.c_str() );
#endif // DEBUG
		m_current_scheme = NULL;
		return false;
	}

	LPCSTR scheme_name = get_manager().get_item_scheme( inv_item );
	if ( !scheme_name )
	{
#ifdef DEBUG
		Msg( "Inventory item <%s> does not contain upgrade scheme.", inv_item.m_section_id.c_str() );
#endif // DEBUG
		m_current_scheme = NULL;
		return false;
	}

	SetCurScheme( scheme_name );
	
	UI_Upgrades_type::iterator ib = m_current_scheme->cells.begin();
	UI_Upgrades_type::iterator ie = m_current_scheme->cells.end();
	for ( ; ib != ie; ++ib )
	{
		UIUpgrade* ui_item = (*ib);
		m_scheme_wnd->AttachChild( ui_item );
		
		LPCSTR upgrade_name = get_manager().get_upgrade_by_index( inv_item, ui_item->get_scheme_index() );
		ui_item->init_upgrade( upgrade_name, inv_item );
		
		Upgrade_type* upgrade_p = get_manager().get_upgrade( upgrade_name );
		VERIFY( upgrade_p );
		Property_type* prop_p = get_manager().get_property( upgrade_p->get_property_name() );
		VERIFY( prop_p );
		
		ui_item->set_texture( UIUpgrade::LAYER_ITEM,   upgrade_p->icon_name() );
		ui_item->set_texture( UIUpgrade::LAYER_COLOR,  m_cell_textures[UIUpgrade::STATE_ENABLED].c_str() ); //default
		ui_item->set_texture( UIUpgrade::LAYER_BORDER, m_border_texture.c_str() );
		ui_item->set_texture( UIUpgrade::LAYER_INK,    m_ink_texture.c_str() );
	}
	
	m_scheme_wnd->Show( true );
	
	UpdateAllUpgrades();
	return true;
}

UIUpgrade* CUIInventoryUpgradeWnd::FindUIUpgrade( Upgrade_type const* upgr )
{
	if ( !m_current_scheme )
	{
		return NULL;
	}
	UI_Upgrades_type::iterator ib = m_current_scheme->cells.begin();
	UI_Upgrades_type::iterator ie = m_current_scheme->cells.end();
	for ( ; ib != ie; ++ib )
	{
		Upgrade_type* i_upgr = (*ib)->get_upgrade();
		if ( upgr == i_upgr )
		{
			return (*ib);
		}
	}
	return NULL;
}

bool CUIInventoryUpgradeWnd::DBClickOnUIUpgrade( Upgrade_type const* upgr )
{
	UpdateAllUpgrades();
	UIUpgrade* uiupgr = FindUIUpgrade( upgr );
	if ( uiupgr )
	{
		uiupgr->OnClick();
		return true;
	}
	return false;
}

void CUIInventoryUpgradeWnd::AskUsing( LPCSTR text, LPCSTR upgrade_name )
{
	VERIFY( m_inv_item );
	VERIFY( upgrade_name );
	VERIFY( m_pParentWnd );

	UpdateAllUpgrades();

	m_cur_upgrade_id = upgrade_name;
	
	CUIActorMenu* parent_wnd = smart_cast<CUIActorMenu*>(m_pParentWnd);
	if ( parent_wnd )
	{
		parent_wnd->CallMessageBoxYesNo( text );
	}
}

void CUIInventoryUpgradeWnd::OnMesBoxYes()
{
	if ( get_manager().upgrade_install( *m_inv_item, m_cur_upgrade_id, false ) )
	{
//-		OnUpgradeItem();
//-		UpdateAllUpgrades();
		
		VERIFY( m_pParentWnd );
		CUIActorMenu* parent_wnd = smart_cast<CUIActorMenu*>( m_pParentWnd );
		if ( parent_wnd )
		{
			parent_wnd->UpdateActor();
			parent_wnd->SeparateUpgradeItem();
		}
	}
	UpdateAllUpgrades();
}

void CUIInventoryUpgradeWnd::HighlightHierarchy( shared_str const& upgrade_id )
{
	UpdateAllUpgrades();
	get_manager().highlight_hierarchy( *m_inv_item, upgrade_id );
}

void CUIInventoryUpgradeWnd::ResetHighlight()
{
	UpdateAllUpgrades();
	get_manager().reset_highlight( *m_inv_item );
}

void CUIInventoryUpgradeWnd::set_info_cur_upgrade( Upgrade_type* upgrade )
{
	UIUpgrade* uiu = FindUIUpgrade( upgrade );
	if ( uiu )
	{
		if ( Device.dwTimeGlobal < uiu->FocusReceiveTime() + m_item_info->delay )
		{
			upgrade = NULL; // visible = false
		}
	}
	else
	{
		upgrade = NULL;
	}

	CUIActorMenu* parent_wnd = smart_cast<CUIActorMenu*>(m_pParentWnd);
	if ( parent_wnd )
	{
		if ( parent_wnd->SetInfoCurUpgrade( upgrade, m_inv_item ) )
		{
			UpdateAllUpgrades();
		}
	}
}

CUIInventoryUpgradeWnd::Manager_type& CUIInventoryUpgradeWnd::get_manager()
{
	return  ai().alife().inventory_upgrade_manager();
}
/*
void CUIInventoryUpgradeWnd::PreUpgradeItem()
{
	CWeapon* weapon = smart_cast<CWeapon*>( m_inv_item );
	if ( weapon )
	{
		if ( weapon->ScopeAttachable() && weapon->IsScopeAttached() )
		{
			weapon->Detach( weapon->GetScopeName().c_str(), true );
		}
		if ( weapon->SilencerAttachable() && weapon->IsSilencerAttached() )
		{
			weapon->Detach( weapon->GetSilencerName().c_str(), true );
		}
		if ( weapon->GrenadeLauncherAttachable() && weapon->IsGrenadeLauncherAttached() )
		{
			weapon->Detach( weapon->GetGrenadeLauncherName().c_str(), true );
		}
	}
	CWeaponMagazined* wm = smart_cast<CWeaponMagazined*>( m_inv_item );
	if ( wm )
	{
		wm->UnloadMagazine();
		wm->SwitchAmmoType( CMD_START );
	}
}
*/