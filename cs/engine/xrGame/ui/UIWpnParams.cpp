#include "pch_script.h"
#include "UIWpnParams.h"
#include "UIXmlInit.h"
#include "../level.h"
#include "game_base_space.h"
#include "../ai_space.h"
#include "../../xrServerEntities/script_engine.h"
#include "inventory_item_object.h"


struct SLuaWpnParams
{
	luabind::functor<float>		m_functorRPM;
	luabind::functor<float>		m_functorAccuracy;
	luabind::functor<float>		m_functorDamage;
	luabind::functor<float>		m_functorDamageMP;
	luabind::functor<float>		m_functorHandling;
	SLuaWpnParams();
	~SLuaWpnParams();
};

SLuaWpnParams::SLuaWpnParams()
{
	bool	functor_exists;
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetRPM" ,		m_functorRPM);		VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetDamage" ,	m_functorDamage);	VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetDamageMP" ,m_functorDamageMP);	VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetHandling" ,m_functorHandling);	VERIFY(functor_exists);
	functor_exists	= ai().script_engine().functor("ui_wpn_params.GetAccuracy" ,m_functorAccuracy);	VERIFY(functor_exists);
}

SLuaWpnParams::~SLuaWpnParams()
{
}

SLuaWpnParams* g_lua_wpn_params = NULL;

void destroy_lua_wpn_params()
{
	if(g_lua_wpn_params)
		xr_delete(g_lua_wpn_params);
}

// =====================================================================

CUIWpnParams::CUIWpnParams()
{
	AttachChild(&m_textAccuracy);
	AttachChild(&m_textDamage);
	AttachChild(&m_textHandling);
	AttachChild(&m_textRPM);

	AttachChild(&m_progressAccuracy);
	AttachChild(&m_progressDamage);
	AttachChild(&m_progressHandling);
	AttachChild(&m_progressRPM);
}

CUIWpnParams::~CUIWpnParams()
{
}

void CUIWpnParams::InitFromXml(CUIXml& xml_doc)
{
	if (!xml_doc.NavigateToNode("wpn_params", 0))	return;
	CUIXmlInit::InitWindow			(xml_doc, "wpn_params", 0, this);

	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_accuracy",		0, &m_textAccuracy);
	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_damage",			0, &m_textDamage);
	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_handling",		0, &m_textHandling);
	CUIXmlInit::InitStatic			(xml_doc, "wpn_params:cap_rpm",				0, &m_textRPM);

	m_progressAccuracy.InitFromXml	( xml_doc, "wpn_params:progress_accuracy" );
	m_progressDamage.InitFromXml	( xml_doc, "wpn_params:progress_damage" );
	m_progressHandling.InitFromXml	( xml_doc, "wpn_params:progress_handling" );
	m_progressRPM.InitFromXml		( xml_doc, "wpn_params:progress_rpm" );
}

void CUIWpnParams::SetInfo( CInventoryItem const* slot_wpn, CInventoryItem const& cur_wpn )
{
	if ( !g_lua_wpn_params )
	{
		g_lua_wpn_params = xr_new<SLuaWpnParams>();
	}
	
	LPCSTR cur_section  = cur_wpn.object().cNameSect().c_str();
	string2048 str_upgrades;
	str_upgrades[0] = 0;
	cur_wpn.get_upgrades_str( str_upgrades );

	float cur_rpm    = g_lua_wpn_params->m_functorRPM( cur_section, str_upgrades );
	float cur_accur  = g_lua_wpn_params->m_functorAccuracy( cur_section, str_upgrades );
	float cur_hand   = g_lua_wpn_params->m_functorHandling( cur_section, str_upgrades );
	float cur_damage = ( GameID() == eGameIDSingle ) ?
		g_lua_wpn_params->m_functorDamage( cur_section, str_upgrades )
		: g_lua_wpn_params->m_functorDamageMP( cur_section, str_upgrades );

	float slot_rpm    = cur_rpm;
	float slot_accur  = cur_accur;
	float slot_hand   = cur_hand;
	float slot_damage = cur_damage;

	if ( slot_wpn && (slot_wpn != &cur_wpn) )
	{
		LPCSTR slot_section  = slot_wpn->object().cNameSect().c_str();
		str_upgrades[0] = 0;
		slot_wpn->get_upgrades_str( str_upgrades );

		slot_rpm    = g_lua_wpn_params->m_functorRPM( slot_section, str_upgrades );
		slot_accur  = g_lua_wpn_params->m_functorAccuracy( slot_section, str_upgrades );
		slot_hand   = g_lua_wpn_params->m_functorHandling( slot_section, str_upgrades );
		slot_damage = ( GameID() == eGameIDSingle ) ?
			g_lua_wpn_params->m_functorDamage( slot_section, str_upgrades )
			: g_lua_wpn_params->m_functorDamageMP( slot_section, str_upgrades );
	}
	
	m_progressAccuracy.SetTwoPos( cur_accur,  slot_accur );
	m_progressDamage.SetTwoPos(   cur_damage, slot_damage );
	m_progressHandling.SetTwoPos( cur_hand,   slot_hand );
	m_progressRPM.SetTwoPos(      cur_rpm,    slot_rpm );
}

bool CUIWpnParams::Check(const shared_str& wpn_section)
{
	if (pSettings->line_exist(wpn_section, "fire_dispersion_base"))
	{
        if (0==xr_strcmp(wpn_section, "wpn_addon_silencer"))
            return false;
        if (0==xr_strcmp(wpn_section, "wpn_binoc"))
            return false;
        if (0==xr_strcmp(wpn_section, "mp_wpn_binoc"))
            return false;

        return true;		
	}
	return false;
}

// -------------------------------------------------------------------------------------------------

CUIConditionParams::CUIConditionParams()
{
	AttachChild( &m_progress );
	AttachChild( &m_text );
}

CUIConditionParams::~CUIConditionParams()
{
}

void CUIConditionParams::InitFromXml(CUIXml& xml_doc)
{
	if (!xml_doc.NavigateToNode("condition_params", 0))	return;
	CUIXmlInit::InitWindow	(xml_doc, "condition_params", 0, this);
	CUIXmlInit::InitStatic	( xml_doc, "condition_params:caption", 0, &m_text );
	m_progress.InitFromXml	( xml_doc, "condition_params:progress_state" );
}

void CUIConditionParams::SetInfo( CInventoryItem const* slot_item, CInventoryItem const& cur_item )
{
	float cur_value  = cur_item.GetConditionToShow() * 100.0f + 1.0f - EPS;
	float slot_value = cur_value;

	if ( slot_item && (slot_item != &cur_item) /*&& (cur_item.object().cNameSect()._get() == slot_item->object().cNameSect()._get())*/ )
	{
		slot_value = slot_item->GetConditionToShow() * 100.0f + 1.0f - EPS;
	}
	m_progress.SetTwoPos( cur_value, slot_value );
}
