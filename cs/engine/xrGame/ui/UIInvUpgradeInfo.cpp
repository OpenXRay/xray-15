	////////////////////////////////////////////////////////////////////////////
//	Module 		: UIInvUpgradeInfo.cpp
//	Created 	: 21.11.2007
//  Modified 	: 27.11.2007
//	Author		: Evgeniy Sokolov
//	Description : inventory upgrade UI info window class implementation
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "UIInvUpgradeInfo.h"
#include "../string_table.h"
#include "../Actor.h"

#include "UIStatic.h"
#include "UIXmlInit.h"
#include "UIFrameWindow.h"

#include "UIInvUpgradeProperty.h"

#include "inventory_upgrade.h"
#include "inventory_upgrade_property.h"


UIInvUpgradeInfo::UIInvUpgradeInfo()
{
	m_upgrade    = NULL;
	m_background = NULL;
	m_name       = NULL;
	m_desc       = NULL;
	m_prereq     = NULL;
	m_properties_wnd = NULL;
}

UIInvUpgradeInfo::~UIInvUpgradeInfo()
{
}

void UIInvUpgradeInfo::init_from_xml( LPCSTR xml_name )
{
	CUIXml ui_xml;
	ui_xml.Load( CONFIG_PATH, UI_PATH, xml_name );
	CUIXmlInit xml_init;
	
	XML_NODE* stored_root = ui_xml.GetLocalRoot();
	XML_NODE* node = ui_xml.NavigateToNode( "upgrade_info", 0 );
	ui_xml.SetLocalRoot( node );

	xml_init.InitWindow( ui_xml, "main_frame", 0, this );
	
	m_background = xr_new<CUIFrameWindow>();
	AttachChild( m_background );
	m_background->SetAutoDelete( true );
	xml_init.InitFrameWindow( ui_xml, "background_frame", 0, m_background );

	m_name = xr_new<CUIStatic>();	 
	AttachChild( m_name );		
	m_name->SetAutoDelete( true );
	xml_init.InitStatic( ui_xml, "info_name", 0, m_name );

	m_desc = xr_new<CUIStatic>();	 
	AttachChild( m_desc );
	m_desc->SetAutoDelete( true );
	xml_init.InitStatic( ui_xml, "info_desc", 0, m_desc );

	m_prereq = xr_new<CUIStatic>();	 
	AttachChild( m_prereq );
	m_prereq->SetAutoDelete( true );
	xml_init.InitStatic( ui_xml, "info_prerequisites", 0, m_prereq );

	m_properties_wnd = xr_new<UIInvUpgPropertiesWnd>();	 
	AttachChild( m_properties_wnd );
	m_properties_wnd->SetAutoDelete( true );
	m_properties_wnd->init_from_xml( xml_name );
	
	m_properties_wnd->Show( false );
	ui_xml.SetLocalRoot( stored_root );
}

bool UIInvUpgradeInfo::init_upgrade( Upgrade_type* upgr, CInventoryItem* inv_item )
{
	if ( !upgr || !inv_item )
	{
		m_upgrade = NULL;
		Show( false );
		return false;
	}
	
	if ( m_upgrade == upgr )
	{
		return false;
	}
	m_upgrade = upgr;

	Show( true );
	m_name->Show( true );
	m_desc->Show( true );

	m_name->SetText( m_upgrade->name() );
	if ( m_upgrade->is_known() )
	{
		m_desc->SetText( m_upgrade->description_text() );
		m_prereq->Show( true );

		inventory::upgrade::UpgradeStateResult upg_res = m_upgrade->can_install( *inv_item, false );
		if ( upg_res == inventory::upgrade::result_ok || upg_res == inventory::upgrade::result_e_precondition_money
			|| upg_res == inventory::upgrade::result_e_precondition_quest )
		{
			m_prereq->SetText( m_upgrade->get_prerequisites() );
		}
		else
		{
			string32 str_res;
			switch( upg_res )
			{
			case inventory::upgrade::result_e_unknown:		strcpy_s( str_res, sizeof(str_res), "st_upgr_unknown" );		break;
			case inventory::upgrade::result_e_installed:	strcpy_s( str_res, sizeof(str_res), "st_upgr_installed" );		break;
			case inventory::upgrade::result_e_parents:		strcpy_s( str_res, sizeof(str_res), "st_upgr_parents" );		break;
			case inventory::upgrade::result_e_group:		strcpy_s( str_res, sizeof(str_res), "st_upgr_group" );			break;
			//result_e_precondition:
			default:										strcpy_s( str_res, sizeof(str_res), "st_upgr_unknown" );		break;
			}
			m_prereq->SetTextST( str_res );
		}
		m_properties_wnd->Show( true );
	}
	else
	{
		m_desc->SetTextST( "st_desc_unknown" );
		m_prereq->Show( false );
		m_prereq->SetText( "" );
		m_properties_wnd->Show( false );
	}
	m_desc->AdjustHeightToText();
	m_prereq->AdjustHeightToText();

	Fvector2 new_pos;
	new_pos.x = m_prereq->GetWndPos().x;
	new_pos.y = m_desc->GetWndPos().y + m_desc->GetWndSize().y + 5.0f;
	m_prereq->SetWndPos( new_pos );

	new_pos.x = m_properties_wnd->GetWndPos().x;
	new_pos.y = m_prereq->GetWndPos().y + m_prereq->GetWndSize().y + 5.0f;
	m_properties_wnd->SetWndPos( new_pos );
	
	m_properties_wnd->set_upgrade_info( *m_upgrade );

	// this wnd
	Fvector2 new_size;
	new_size.x = GetWndSize().x;
	new_size.y = m_properties_wnd->GetWndPos().y + m_properties_wnd->GetWndSize().y + 10.0f;
	SetWndSize( new_size );
	m_background->InitFrameWindow( m_background->GetWndPos(), new_size );
	
	return true;
}

void UIInvUpgradeInfo::Draw()
{
	if ( m_upgrade )
	{
		inherited::Draw();
	}
}
