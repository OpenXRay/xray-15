#include "StdAfx.h"
#include "UIOutfitInfo.h"
#include "UIXmlInit.h"
#include "UIStatic.h"
#include "UIDoubleProgressBar.h"

#include "..\CustomOutfit.h"
#include "..\string_table.h"
#include "..\actor.h"
#include "..\ActorCondition.h"
#include "..\player_hud.h"


LPCSTR immunity_names[]=
{
	"burn_immunity",
	"strike_immunity",
	"shock_immunity",
	"wound_immunity",		
	"radiation_immunity",
	"telepatic_immunity",
	"chemical_burn_immunity",
	"explosion_immunity",
	"fire_wound_immunity",
};

LPCSTR immunity_st_names[]=
{
	"ui_inv_outfit_burn_protection",
	"ui_inv_outfit_strike_protection",
	"ui_inv_outfit_shock_protection",
	"ui_inv_outfit_wound_protection",
	"ui_inv_outfit_radiation_protection",
	"ui_inv_outfit_telepatic_protection",
	"ui_inv_outfit_chemical_burn_protection",
	"ui_inv_outfit_explosion_protection",
	"ui_inv_outfit_fire_wound_protection",
};

CUIOutfitImmunity::CUIOutfitImmunity()
{
	AttachChild( &m_name );
	AttachChild( &m_progress );
	AttachChild( &m_value );
	m_magnitude = 1.0f;
}

CUIOutfitImmunity::~CUIOutfitImmunity()
{
}

void CUIOutfitImmunity::InitFromXml( CUIXml& xml_doc, LPCSTR base_str, u32 hit_type )
{
	CUIXmlInit::InitWindow( xml_doc, base_str, 0, this );

	string256 buf;
	strconcat( sizeof(buf), buf, base_str, ":", immunity_names[hit_type] );
	CUIXmlInit::InitWindow( xml_doc, buf, 0, this );
	CUIXmlInit::InitStatic( xml_doc, buf, 0, &m_name );
	m_name.SetTextST( immunity_st_names[hit_type] );

	strconcat( sizeof(buf), buf, base_str, ":", immunity_names[hit_type], ":progress_immunity" );
	m_progress.InitFromXml( xml_doc, buf );
	
	strconcat( sizeof(buf), buf, base_str, ":", immunity_names[hit_type], ":static_value" );
	if ( xml_doc.NavigateToNode( buf, 0 ) )
	{
		CUIXmlInit::InitStatic( xml_doc, buf, 0, &m_value );
		m_value.SetVisible( true );
	}
	else
	{
		m_value.SetVisible( false );
	}
	m_magnitude = xml_doc.ReadAttribFlt( buf, 0, "magnitude", 1.0f );
}

void CUIOutfitImmunity::SetProgressValue( float cur, float comp )
{
	cur  *= m_magnitude;
	comp *= m_magnitude;
	m_progress.SetTwoPos( cur, comp );
	string32 buf;
//	sprintf_s( buf, sizeof(buf), "%d %%", (int)cur );
	sprintf_s( buf, sizeof(buf), "%.0f", cur );
	m_value.SetText( buf );
}

// ===========================================================================================

CUIOutfitInfo::CUIOutfitInfo()
{
	for ( u32 i = 0; i < max_count; ++i )
	{
		m_items[i] = NULL;
	}
}

CUIOutfitInfo::~CUIOutfitInfo()
{
	for ( u32 i = 0; i < max_count; ++i )
	{
		xr_delete( m_items[i] );
	}
}

void CUIOutfitInfo::InitFromXml( CUIXml& xml_doc )
{
	LPCSTR base_str	= "outfit_info";

	CUIXmlInit::InitWindow( xml_doc, base_str, 0, this );
	
	m_caption = xr_new<CUIStatic>();
	AttachChild( m_caption );
	m_caption->SetAutoDelete( true );	
	string128 buf;
	strconcat( sizeof(buf), buf, base_str, ":caption" );
	CUIXmlInit::InitStatic( xml_doc, buf, 0, m_caption );

	Fvector2 pos;
	pos.set( 0.0f, m_caption->GetWndSize().y );

	for ( u32 i = 0; i < max_count; ++i )
	{
		if ( i == ALife::eHitTypeStrike || i == ALife::eHitTypeExplosion ) // +eHitTypeFireWound = 8,, eHitTypeWound(0..1)
		{
			continue;
		}
		m_items[i] = xr_new<CUIOutfitImmunity>();
		m_items[i]->InitFromXml( xml_doc, base_str, i );
		AttachChild( m_items[i] );
		m_items[i]->SetWndPos( pos );
		pos.y += m_items[i]->GetWndSize().y;
	}
	pos.x = GetWndSize().x;
	SetWndSize( pos );
}

void CUIOutfitInfo::UpdateInfo( CCustomOutfit* cur_outfit, CCustomOutfit* slot_outfit )
{
	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	if ( !actor || !cur_outfit )
	{
		return;
	}

	for ( u32 i = 0; i < max_count; ++i )
	{
		if ( i == ALife::eHitTypeStrike || i == ALife::eHitTypeExplosion || i == ALife::eHitTypeFireWound )
		{
			continue;
		}
		
		ALife::EHitType hit_type = (ALife::EHitType)i;
		float max_power = actor->conditions().GetZoneMaxPower( hit_type );

		float cur = cur_outfit->GetDefHitTypeProtection( hit_type );
		cur /= max_power; // = 0..1
		float slot = cur;
		
		if ( slot_outfit )
		{
			slot = slot_outfit->GetDefHitTypeProtection( hit_type );
			slot /= max_power; //  = 0..1
		}
		m_items[i]->SetProgressValue( cur, slot );
	}

	if ( m_items[ALife::eHitTypeFireWound] )
	{
		IKinematics* ikv = smart_cast<IKinematics*>( actor->Visual() );
		VERIFY( ikv );
		u16 spine_bone = ikv->LL_BoneID( "bip01_spine" );

		float cur = cur_outfit->GetBoneArmor( spine_bone );
		float slot = (slot_outfit)? slot_outfit->GetBoneArmor( spine_bone ) : cur;
		
		m_items[ALife::eHitTypeFireWound]->SetProgressValue( cur, slot );
	}
}
