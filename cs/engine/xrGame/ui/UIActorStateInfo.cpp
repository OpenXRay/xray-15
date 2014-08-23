////////////////////////////////////////////////////////////////////////////
//	Module 		: UIActorStateInfo.cpp
//	Created 	: 15.02.2008
//	Author		: Evgeniy Sokolov
//	Description : UI actor state window class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIActorStateInfo.h"
#include "UIProgressBar.h"
#include "UIProgressShape.h"
#include "UIScrollView.h"
#include "UIFrameWindow.h"
#include "UIStatic.h"
#include "UIXmlInit.h"
#include "object_broker.h"

#include "UIHelper.h"
#include "ui_arrow.h"
#include "UIHudStatesWnd.h"

#include "../Level.h"
#include "../location_manager.h"
#include "../player_hud.h"
#include "../hudmanager.h"
#include "UIMainIngameWnd.h"

#include "../Actor.h"
#include "../ActorCondition.h"
#include "../CustomOutfit.h"
#include "../string_table.h"

ui_actor_state_wnd::ui_actor_state_wnd()
{
}

ui_actor_state_wnd::~ui_actor_state_wnd()
{
	delete_data( m_hint_wnd );
}

void ui_actor_state_wnd::init_from_xml( CUIXml& xml, LPCSTR path )
{
	XML_NODE* stored_root = xml.GetLocalRoot();
	CUIXmlInit::InitWindow( xml, path, 0, this );

	XML_NODE* new_root = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( new_root );

	m_hint_wnd = UIHelper::CreateHint( xml, "hint_wnd" );

	for ( int i = 0; i < stt_count; ++i )
	{
		m_state[i] = xr_new<ui_actor_state_item>();
		m_state[i]->SetAutoDelete( true );
		AttachChild( m_state[i] );
		m_state[i]->set_hint_wnd( m_hint_wnd );
	}
	m_state[stt_stamina]->init_from_xml( xml, "stamina_state" );
	m_state[stt_health ]->init_from_xml( xml, "health_state"  );
	m_state[stt_armor  ]->init_from_xml( xml, "armor_state"   );

	m_state[stt_main ]->init_from_xml( xml, "main_sensor"  );
	m_state[stt_fire ]->init_from_xml( xml, "fire_sensor"  );
	m_state[stt_radia]->init_from_xml( xml, "radia_sensor" );
	m_state[stt_acid ]->init_from_xml( xml, "acid_sensor"  );
	m_state[stt_psi  ]->init_from_xml( xml, "psi_sensor"   );

	xml.SetLocalRoot( stored_root );
}

void ui_actor_state_wnd::UpdateActorInfo( CInventoryOwner* owner )
{
	CActor* actor = smart_cast<CActor*>( owner );
	if ( !actor )
	{
		return;
	}

	float value = 0.0f;
	
	value = actor->conditions().GetPower();							m_state[stt_stamina]->set_progress( value );
	value = actor->GetRestoreSpeed( ALife::ePowerRestoreSpeed );	m_state[stt_stamina]->set_text( value ); // 0..0.99

	value = actor->conditions().GetHealth();						m_state[stt_health]->set_progress( value );
	value = actor->conditions().BleedingSpeed();					m_state[stt_health]->show_static( (value > 0.01f) );

	CCustomOutfit* outfit = actor->GetOutfit();
	if ( outfit )
	{
		value = outfit->GetCondition();								m_state[stt_armor]->set_progress( value );

		IKinematics* ikv = smart_cast<IKinematics*>( actor->Visual() );
		VERIFY( ikv );
		u16 spine_bone = ikv->LL_BoneID( "bip01_spine" );
		value = outfit->GetBoneArmor( spine_bone );					m_state[stt_armor]->set_text( value );
	}
	else
	{
		m_state[stt_armor]->set_progress( 0.0f );
		m_state[stt_armor]->set_text( 0.0f );
	}

	// -----------------------------------------------------------------------------------
	m_state[stt_main]->set_progress_shape( actor->conditions().GetRadiation() );

	update_round_states( actor, ALife::eHitTypeBurn, stt_fire );
	update_round_states( actor, ALife::eHitTypeRadiation, stt_radia );
	update_round_states( actor, ALife::eHitTypeChemicalBurn, stt_acid );
	update_round_states( actor, ALife::eHitTypeTelepatic, stt_psi );

	UpdateHitZone();
}

void ui_actor_state_wnd::update_round_states( CActor* actor, ALife::EHitType hit_type, EStateType stt_type )
{
	CCustomOutfit* outfit = actor->GetOutfit();
	float value = (outfit)? outfit->GetDefHitTypeProtection( hit_type ) : 0.0f;
	value += actor->GetProtection_ArtefactsOnBelt( hit_type );
	
	float max_power = actor->conditions().GetZoneMaxPower( hit_type );
	value = value / max_power; //  = 0..1
	//	m_state[stt_type]->set_progress_shape( value );
	m_state[stt_type]->set_arrow( value );//0..1
	m_state[stt_type]->set_text( value );//0..1
}

void ui_actor_state_wnd::UpdateHitZone()
{
	CUIHudStatesWnd* wnd = HUD().GetUI()->UIMainIngameWnd->get_hud_states(); //некрасиво слишком
	VERIFY( wnd );
	if ( !wnd )
	{
		return;
	}
	wnd->UpdateZones();
	m_state[stt_main]->set_arrow(  wnd->get_main_sensor_value() );

/*	m_state[stt_fire]->set_arrow(  wnd->get_zone_cur_power( ALife::eHitTypeBurn ) );
	m_state[stt_radia]->set_arrow( wnd->get_zone_cur_power( ALife::eHitTypeRadiation ) );
	m_state[stt_acid]->set_arrow(  wnd->get_zone_cur_power( ALife::eHitTypeChemicalBurn ) );
	m_state[stt_psi]->set_arrow(   wnd->get_zone_cur_power( ALife::eHitTypeTelepatic ) );
	*/
}
/*
void ui_actor_state_wnd::Update()
{
	inherited::Update();
}
*/
void ui_actor_state_wnd::Draw()
{
	inherited::Draw();
	m_hint_wnd->Draw();
}

void ui_actor_state_wnd::Show( bool status )
{
	inherited::Show( status );
	ShowChildren( status );
}

/// =============================================================================================
ui_actor_state_item::ui_actor_state_item()
{
	m_static		= NULL;
	m_progress		= NULL;
	m_sensor		= NULL;
	m_arrow			= NULL;
	m_arrow_shadow	= NULL;
	m_magnitude		= 1.0f;
}

ui_actor_state_item::~ui_actor_state_item()
{
}

void ui_actor_state_item::init_from_xml( CUIXml& xml, LPCSTR path )
{
	CUIXmlInit::InitWindow( xml, path, 0, this);

	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* new_root = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( new_root );

	LPCSTR hint_text = xml.Read( "hint_text", 0, "no hint" );
	set_hint_text_ST( hint_text );
	
	set_hint_delay( (u32)xml.ReadAttribInt( "hint_text", 0, "delay" ) );

	if ( xml.NavigateToNode( "state_progress", 0 ) )	
	{
		m_progress = UIHelper::CreateProgressBar( xml, "state_progress", this );
	}
	if ( xml.NavigateToNode( "progress_shape", 0 ) )	
	{
		m_sensor = xr_new<CUIProgressShape>();
		AttachChild( m_sensor );
		m_sensor->SetAutoDelete( true );
		CUIXmlInit::InitProgressShape( xml, "progress_shape", 0, m_sensor );
	}
	if ( xml.NavigateToNode( "arrow", 0 ) )	
	{
		m_arrow = xr_new<UI_Arrow>();
		m_arrow->init_from_xml( xml, "arrow", this );
	}
	if ( xml.NavigateToNode( "arrow_shadow", 0 ) )	
	{
		m_arrow_shadow = xr_new<UI_Arrow>();
		m_arrow_shadow->init_from_xml( xml, "arrow_shadow", this );
	}
	if ( xml.NavigateToNode( "icon", 0 ) )	
	{
		m_static = UIHelper::CreateStatic( xml, "icon", this );
		m_static->SetText( "" );
		m_magnitude = xml.ReadAttribFlt( "icon", 0, "magnitude", 1.0f );
	}

	set_arrow( 0.0f );
	xml.SetLocalRoot( stored_root );
}

bool ui_actor_state_item::OnMouse( float x, float y, EUIMessages mouse_action )
{
	if( CUIWindow::OnMouse( x, y, mouse_action ) )
	{
		return true;
	}

	if ( ( mouse_action == WINDOW_LBUTTON_DOWN || mouse_action == WINDOW_LBUTTON_UP ||
		mouse_action == WINDOW_RBUTTON_DOWN || mouse_action == WINDOW_RBUTTON_UP ) &&
		HasChildMouseHandler() )
	{
		return false;
	}

	/*if ( m_bCursorOverWindow )
	{
		if ( mouse_action == WINDOW_LBUTTON_DOWN )
		{
			OnClick();
			return true;
		}
	}*/
	return false;
}

void ui_actor_state_item::set_text( float value )
{
	if ( !m_static )
	{
		return;
	}
	int v = (int)( value * m_magnitude + 0.49f );// m_magnitude=100
	clamp( v, 0, 99 );
	string32 text_res;
	sprintf_s( text_res, sizeof(text_res), "%d", v );
	m_static->SetText( text_res );
}

void ui_actor_state_item::set_progress( float value )
{
	if ( !m_progress )
	{
		return;
	}
	m_progress->SetProgressPos( value );
}

void ui_actor_state_item::set_progress_shape( float value )
{
	if ( !m_sensor )
	{
		return;
	}
	m_sensor->SetPos( value );
}

void ui_actor_state_item::set_arrow( float value )
{
	if ( !m_arrow )
	{
		return;	
	}
	m_arrow->SetNewValue( value );
	if ( !m_arrow_shadow )
	{
		return;
	}
	m_arrow_shadow->SetPos( m_arrow->GetPos() );
}


void ui_actor_state_item::show_static( bool status )
{
	if ( !m_static )
	{	
		return;
	}
	m_static->Show( status );
}
