////////////////////////////////////////////////////////////////////////////
//	Module 		: UIRankingWnd.cpp
//	Created 	: 17.01.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Ranking window class implementation
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "UIRankingWnd.h"

#include "UIXmlInit.h"
#include "UIProgressBar.h"
#include "UIFrameLineWnd.h"
#include "UIScrollView.h"
#include "UIHelper.h"
#include "UIInventoryUtilities.h"

#include "../actor.h"
#include "../ai_space.h"
#include "../alife_simulator.h"

#include "../../xrServerEntities/script_engine.h"
#include "../character_community.h"
#include "../character_reputation.h"
#include "../relation_registry.h"
#include "../string_table.h"
#include "UICharacterInfo.h"

#define  PDA_RANKING_XML		"pda_ranking.xml"

CUIRankingWnd::CUIRankingWnd()
{
	m_actor_ch_info = NULL;
	m_previous_time = Device.dwTimeGlobal;
	m_delay			= 3000;
}

CUIRankingWnd::~CUIRankingWnd()
{
}

void CUIRankingWnd::Show( bool status )
{
	if ( status )
	{
		m_actor_ch_info->InitCharacter( Actor()->object_id() );
		
		string64 buf;
		sprintf_s( buf, sizeof(buf), "%d %s", Actor()->get_money(), "RU" );
		m_money_value->SetText( buf );
		m_money_value->AdjustWidthToText();
		update_info();
		inherited::Update();
	}
	inherited::Show( status );
}

void CUIRankingWnd::Update()
{
	inherited::Update();
	if ( IsShown() )
	{
		if ( Device.dwTimeGlobal - m_previous_time > m_delay )
		{
			m_previous_time = Device.dwTimeGlobal;
			update_info();
		}
	}
}

void CUIRankingWnd::Init()
{
	Fvector2 pos;
	CUIXml xml;
	xml.Load( CONFIG_PATH, UI_PATH, PDA_RANKING_XML );

	CUIXmlInit::InitWindow( xml, "main_wnd", 0, this );
	m_delay				= (u32)xml.ReadAttribInt( "main_wnd", 0, "delay",	3000 );

	m_background		= UIHelper::CreateFrameLine( xml, "background", this );
	m_center_background	= UIHelper::CreateStatic( xml, "center_background", this );

	m_actor_ch_info = xr_new<CUICharacterInfo>();
	m_actor_ch_info->SetAutoDelete( true );
	AttachChild( m_actor_ch_info );
	m_actor_ch_info->InitCharacterInfo( &xml, "actor_ch_info" );

	m_actor_ch_info->UICommunityCaption().AdjustWidthToText();
	pos = m_actor_ch_info->UICommunity().GetWndPos();
	pos.x = m_actor_ch_info->UICommunityCaption().GetWndPos().x + m_actor_ch_info->UICommunityCaption().GetWndSize().x + 10.0f;
	m_actor_ch_info->UICommunity().SetWndPos( pos );

//	m_group_caption		= UIHelper::CreateStatic( xml, "group_caption", this );
	m_money_caption		= UIHelper::CreateStatic( xml, "money_caption", this );
	m_money_value		= UIHelper::CreateStatic( xml, "money_value", this );

//	m_group_caption->AdjustWidthToText();
//	pos.x = m_group_caption->GetWndPos().x - m_actor_ch_info->GetWndPos().x + m_group_caption->GetWndSize().x + 10.0f;
//	pos.y = m_group_caption->GetWndPos().y - m_actor_ch_info->GetWndPos().y;
//	m_actor_ch_info->UICommunity_big().SetWndPos( pos );

	m_money_caption->AdjustWidthToText();
	pos = m_money_caption->GetWndPos();
	pos.x += m_money_caption->GetWndSize().x + 10.0f;
	m_money_value->SetWndPos( pos );

//	m_faction_icon			= UIHelper::CreateStatic( xml, "fraction_icon", this );
//	m_faction_icon_over	= UIHelper::CreateStatic( xml, "fraction_icon_over", this );
//	m_rank_icon				= UIHelper::CreateStatic( xml, "rank_icon", this );
//	m_rank_icon_over		= UIHelper::CreateStatic( xml, "rank_icon_over", this );

	m_center_caption		= UIHelper::CreateStatic( xml, "center_caption", this );
	m_faction_static		= UIHelper::CreateStatic( xml, "fraction_static", this );
	m_faction_line1		= UIHelper::CreateFrameLine( xml, "fraction_line1", this );
	m_faction_line2		= UIHelper::CreateFrameLine( xml, "fraction_line2", this );


	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* node = xml.NavigateToNode( "stat_info", 0 );
	xml.SetLocalRoot( node );

	m_stat_count = (u32)xml.GetNodesNum( node, "stat" );
	u32 value_color = CUIXmlInit::GetColor( xml, "value", 0, 0xFFffffff );

	for ( u8 i = 0; i < m_stat_count; ++i )
	{
		m_stat_caption[i]		= xr_new<CUIStatic>();
		AttachChild				( m_stat_caption[i] );
		m_stat_caption[i]->SetAutoDelete( true );
		CUIXmlInit::InitStatic	( xml, "stat", i, m_stat_caption[i] );
		m_stat_caption[i]->AdjustWidthToText();

		m_stat_info[i]			= xr_new<CUIStatic>();
		AttachChild				( m_stat_info[i] );
		m_stat_info[i]->SetAutoDelete( true );
		CUIXmlInit::InitStatic	( xml, "stat", i, m_stat_info[i] );
		
		m_stat_info[i]->SetTextColor( value_color );
		
		pos.y = m_stat_caption[i]->GetWndPos().y;
		pos.x = m_stat_caption[i]->GetWndPos().x + m_stat_caption[i]->GetWndSize().x + 5.0f;
		m_stat_info[i]->SetWndPos( pos );
	}
	xml.SetLocalRoot( stored_root );

	string256 buf;
	strcpy_s( buf, sizeof(buf), m_center_caption->GetText() );
	strcat_s( buf, sizeof(buf), CStringTable().translate("ui_ranking_center_caption").c_str() );
	m_center_caption->SetText( buf );

	// pSettings->[pda_rank_communities] and XML <faction_list>
	m_factions_list = xr_new<CUIScrollView>();
	m_factions_list->SetAutoDelete( true );
	AttachChild( m_factions_list );
	CUIXmlInit::InitScrollView( xml, "fraction_list", 0, m_factions_list );
	m_factions_list->SetWindowName("---fraction_list");
	m_factions_list->m_sort_function = fastdelegate::MakeDelegate( this, &CUIRankingWnd::SortingLessFunction );

	LPCSTR fract_section = "pda_rank_communities";

	VERIFY2( pSettings->section_exist( fract_section ), make_string( "Section [%s] does not exist !", fract_section ) );
	int fract_count = pSettings->line_count( fract_section );
	VERIFY2( fract_count, make_string( "Section [%s] is empty !",       fract_section ) );
//	VERIFY2( fract_count == max_factions, make_string( "Section [%s] has not %d ids!", fract_section, max_factions ) );

	node = xml.NavigateToNode( "fraction_list", 0 );
	xml.SetLocalRoot( node );

	CInifile::Sect&		faction_section = pSettings->r_section( fract_section );
	m_factions_count = faction_section.Data.size();
	CInifile::SectIt_	ib = faction_section.Data.begin();
	CInifile::SectIt_	ie = faction_section.Data.end();
	for ( u8 i = 0; ib != ie ; ++ib, ++i )
	{
		if ( i >= max_factions ) break;
		m_faction_id[i]._set( (*ib).first );
		add_faction( xml, m_faction_id[i] );
	}
	xml.SetLocalRoot( stored_root );
}

void CUIRankingWnd::add_faction( CUIXml& xml, shared_str const& faction_id )
{
	CUIRankFaction* faction = xr_new<CUIRankFaction>( faction_id );
	faction->init_from_xml( xml );
	faction->SetWindowName( "fraction_item" );
	m_factions_list->AddWindow( faction, true );
	Register( faction );
}

void CUIRankingWnd::clear_all_factions()
{
	m_factions_list->Clear();
}

bool CUIRankingWnd::SortingLessFunction( CUIWindow* left, CUIWindow* right )
{
	CUIRankFaction* lpi = smart_cast<CUIRankFaction*>(left);
	CUIRankFaction* rpi = smart_cast<CUIRankFaction*>(right);
	VERIFY( lpi && rpi );
	return ( lpi->get_faction_power() > rpi->get_faction_power() );
}

void CUIRankingWnd::update_info()
{
	bool force_rating = false;
	for ( u8 i = 0; i < m_factions_list->GetSize() && i < max_factions; ++i )
	{
		CUIRankFaction* ui_faction = smart_cast<CUIRankFaction*>( m_factions_list->GetItem(i) );
		if ( ui_faction )
		{
			if ( ui_faction->get_cur_sn() != i+1 )
			{
				force_rating = true;
				break;
			}
		}
	}

	for ( u8 i = 0; i < m_factions_list->GetSize() && i < max_factions; ++i )
	{
		CUIRankFaction* ui_faction = smart_cast<CUIRankFaction*>( m_factions_list->GetItem(i) );
		if ( ui_faction )
		{
			ui_faction->update_info( i+1 );
			ui_faction->rating( i+1, force_rating );
		}
	}
	m_factions_list->ForceUpdate();
	get_value_from_script();
}

void CUIRankingWnd::get_value_from_script()
{
	string128 buf;
	InventoryUtilities::GetTimePeriodAsString( buf, sizeof(buf), Level().GetStartGameTime(), Level().GetGameTime() );
	m_stat_info[0]->SetText( buf );

	for ( u8 i = 1; i < m_stat_count; ++i )
	{
		luabind::functor<LPCSTR>	funct;
		R_ASSERT( ai().script_engine().functor( "pda.get_stat", funct ) );
		LPCSTR str = funct( i );
		m_stat_info[i]->SetTextST( str );
	}
}
