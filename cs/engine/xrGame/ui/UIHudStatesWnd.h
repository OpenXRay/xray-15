#ifndef	UI_HUD_STATES_WND_H_INCLUDED
#define UI_HUD_STATES_WND_H_INCLUDED

#include "UIWindow.h"
#include "..\..\xrServerEntities\alife_space.h"
#include "..\actor_defs.h"

class CUIStatic;
class CUIProgressBar;
class CUIProgressShape;
class CUIXml;
class UI_Arrow;
class CActor;

int const it_max = ALife::infl_max_count - 1;

class CUIHudStatesWnd : public CUIWindow
{
private:
	typedef CUIWindow						inherited;
//-	typedef ALife::EInfluenceType	EIndicatorType;

	CUIStatic*			m_back;
	CUIStatic*			m_back_v;
	CUIStatic*			m_back_over_arrow;
	CUIStatic*			m_static_armor;

	CUIStatic*			m_resist_back[it_max];
	CUIStatic*			m_indik[it_max];

	CUIStatic*			m_ui_weapon_sign_ammo;
	CUIStatic*			m_ui_weapon_icon;
	Frect				m_ui_weapon_icon_rect;
	CUIStatic*			m_fire_mode;

	CUIProgressBar*		m_ui_health_bar;
	CUIProgressBar*		m_ui_armor_bar;
	CUIProgressBar*		m_ui_stamina_bar;

	CUIProgressShape*	m_progress_self;
	CUIStatic*			m_radia_damage;
	UI_Arrow*			m_arrow;
	UI_Arrow*			m_arrow_shadow;
	
	CUIStatic*			m_bleeding;
	
	float				m_radia_self;
//	float				m_actor_radia_factor;
	float				m_radia_hit;
	shared_str			m_lanim_name;
	
	float				m_zone_cur_power[ALife::infl_max_count];
//--	float				m_zone_max_power[hud_it_max];//<-- CActorCondition
	float				m_zone_feel_radius[ALife::infl_max_count ];
	ALife::EHitType		m_zone_hit_type[ALife::infl_max_count ];
	float				m_zone_threshold[ALife::infl_max_count ];

	float				m_zone_feel_radius_max;
	u32					m_last_time;

	bool				m_cur_state_LA[it_max];

//	CZoneList*	m_zones_list; <----- in Level()

public:
					CUIHudStatesWnd		();
	virtual			~CUIHudStatesWnd	();

			void	InitFromXml			( CUIXml& xml, LPCSTR path );
			void	Load_section		();
	virtual void	Update				();
//	virtual void	Draw				();

			void	on_connected		();
			void	reset_ui			();
			void	UpdateHealth		( CActor* actor );
			void	SetAmmoIcon			( const shared_str& sect_name );
			void	UpdateActiveItemInfo( CActor* actor );

			void 	UpdateZones			();
			void	UpdateIndicators	( CActor* actor );

			float	get_zone_cur_power	( ALife::EHitType hit_type );
			float	get_main_sensor_value()	{ return m_radia_hit; }

protected:
	static	ALife::EInfluenceType	get_indik_type( ALife::EHitType hit_type );

			void	Load_section_type	( ALife::EInfluenceType type, LPCSTR section );
			void	UpdateIndicatorType	( CActor* actor, ALife::EInfluenceType type );
			void	SwitchLA			( bool state, ALife::EInfluenceType type );

}; // class CUIHudStatesWnd

#endif // UI_HUD_STATES_WND_H_INCLUDED
