#include "stdafx.h"
#include "weaponmagazinedwgrenade.h"
#include "HUDManager.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "GrenadeLauncher.h"
#include "xrserver_objects_alife_items.h"
#include "ExplosiveRocket.h"
#include "Actor.h"
#include "xr_level_controller.h"
#include "Level.h"
#include "object_broker.h"
#include "game_base_space.h"
#include "MathUtils.h"
#include "player_hud.h"

#ifdef DEBUG
#	include "phdebug.h"
#endif

CWeaponMagazinedWGrenade::CWeaponMagazinedWGrenade(ESoundTypes eSoundType) : CWeaponMagazined(eSoundType)
{
	m_ammoType2 = 0;
    m_bGrenadeMode = false;
}

CWeaponMagazinedWGrenade::~CWeaponMagazinedWGrenade()
{
}

void CWeaponMagazinedWGrenade::Load	(LPCSTR section)
{
	inherited::Load			(section);
	CRocketLauncher::Load	(section);
	
	
	//// Sounds
	m_sounds.LoadSound(section,"snd_shoot_grenade"	, "sndShotG"		, m_eSoundShot);
	m_sounds.LoadSound(section,"snd_reload_grenade"	, "sndReloadG"	, m_eSoundReload);
	m_sounds.LoadSound(section,"snd_switch"			, "sndSwitch"		, m_eSoundReload);
	

	m_sFlameParticles2 = pSettings->r_string(section, "grenade_flame_particles");

	
	if(m_eGrenadeLauncherStatus == ALife::eAddonPermanent)
	{
		CRocketLauncher::m_fLaunchSpeed = pSettings->r_float(section, "grenade_vel");
	}

	// load ammo classes SECOND (grenade_class)
	m_ammoTypes2.clear	(); 
	LPCSTR				S = pSettings->r_string(section,"grenade_class");
	if (S && S[0]) 
	{
		string128		_ammoItem;
		int				count		= _GetItemCount	(S);
		for (int it=0; it<count; ++it)	
		{
			_GetItem				(S,it,_ammoItem);
			m_ammoTypes2.push_back	(_ammoItem);
		}
		m_ammoName2 = pSettings->r_string(*m_ammoTypes2[0],"inv_name_short");
	}
	else
		m_ammoName2 = 0;

	iMagazineSize2 = iMagazineSize;
}

void CWeaponMagazinedWGrenade::net_Destroy()
{
	inherited::net_Destroy();
}


BOOL CWeaponMagazinedWGrenade::net_Spawn(CSE_Abstract* DC) 
{
	CSE_ALifeItemWeapon* const weapon		= smart_cast<CSE_ALifeItemWeapon*>(DC);
	R_ASSERT								(weapon);
	if ( IsGameTypeSingle() )
	{
		inherited::net_Spawn_install_upgrades	(weapon->m_upgrades);
	}

	BOOL l_res = inherited::net_Spawn(DC);
	 
	UpdateGrenadeVisibility(!!iAmmoElapsed);
	SetPending			(FALSE);

	iAmmoElapsed2	= weapon->a_elapsed_grenades.grenades_count;
	m_ammoType2		= weapon->a_elapsed_grenades.grenades_type;

	m_DefaultCartridge2.Load(*m_ammoTypes2[m_ammoType2], u8(m_ammoType2));
	
	if (!IsGameTypeSingle())
	{
		if (!m_bGrenadeMode && IsGrenadeLauncherAttached() && !getRocketCount() && iAmmoElapsed2)
		{
			m_magazine2.push_back(m_DefaultCartridge2);

			shared_str grenade_name = m_DefaultCartridge2.m_ammoSect;
			shared_str fake_grenade_name = pSettings->r_string(grenade_name, "fake_grenade_name");

			CRocketLauncher::SpawnRocket(*fake_grenade_name, this);
		}
	}else
	{
		xr_vector<CCartridge>* pM = NULL;
		bool b_if_grenade_mode	= (m_bGrenadeMode && iAmmoElapsed && !getRocketCount());
		if(b_if_grenade_mode)
			pM = &m_magazine;
			
		bool b_if_simple_mode	= (!m_bGrenadeMode && m_magazine2.size() && !getRocketCount());
		if(b_if_simple_mode)
			pM = &m_magazine2;

		if(b_if_grenade_mode || b_if_simple_mode) 
		{
			shared_str fake_grenade_name = pSettings->r_string(pM->back().m_ammoSect, "fake_grenade_name");
			
			CRocketLauncher::SpawnRocket(*fake_grenade_name, this);
		}
	}
	return l_res;
}

void CWeaponMagazinedWGrenade::switch2_Reload()
{
	VERIFY(GetState()==eReload);
	if(m_bGrenadeMode) 
	{
		PlaySound("sndReloadG", get_LastFP2());

		PlayHUDMotion("anm_reload_g", FALSE, this, GetState());
		SetPending			(TRUE);
	}
	else 
	     inherited::switch2_Reload();
}

void CWeaponMagazinedWGrenade::OnShot		()
{
	if(m_bGrenadeMode)
	{
		PlayAnimShoot		();
		PlaySound			("sndShotG", get_LastFP2());
		AddShotEffector		();
		StartFlameParticles2();
	} 
	else 
		inherited::OnShot	();
}

bool CWeaponMagazinedWGrenade::SwitchMode() 
{
	bool bUsefulStateToSwitch = ((eIdle==GetState())||(eHidden==GetState())||(eMisfire==GetState())||(eMagEmpty==GetState())) && (!IsPending());

	if(!bUsefulStateToSwitch)
		return false;

	if(!IsGrenadeLauncherAttached()) 
		return false;

	SetPending				(TRUE);

	PerformSwitchGL			();
	
	PlaySound				("sndSwitch", get_LastFP());

	PlayAnimModeSwitch		();

	m_dwAmmoCurrentCalcFrame = 0;

	return					true;
}

void  CWeaponMagazinedWGrenade::PerformSwitchGL()
{
	m_bGrenadeMode		= !m_bGrenadeMode;

	iMagazineSize		= m_bGrenadeMode?1:iMagazineSize2;

	m_ammoTypes.swap	(m_ammoTypes2);

	swap				(m_ammoType,m_ammoType2);
	swap				(m_ammoName,m_ammoName2);
	
	swap				(m_DefaultCartridge, m_DefaultCartridge2);

	xr_vector<CCartridge> l_magazine;
	while(m_magazine.size()) { l_magazine.push_back(m_magazine.back()); m_magazine.pop_back(); }
	while(m_magazine2.size()) { m_magazine.push_back(m_magazine2.back()); m_magazine2.pop_back(); }
	while(l_magazine.size()) { m_magazine2.push_back(l_magazine.back()); l_magazine.pop_back(); }
	iAmmoElapsed = (int)m_magazine.size();

}

bool CWeaponMagazinedWGrenade::Action(s32 cmd, u32 flags) 
{
	if(m_bGrenadeMode && cmd==kWPN_FIRE)
	{
		if(IsPending())		
			return				false;

		if(flags&CMD_START)
		{
			if(iAmmoElapsed)
				LaunchGrenade		();
			else
				Reload				();

			if(GetState() == eIdle) 
				OnEmptyClick			();
		}
		return					true;
	}
	if(inherited::Action(cmd, flags))
		return true;
	
	switch(cmd) 
	{
	case kWPN_FUNC: 
		{
            if(flags&CMD_START && !IsPending()) 
				SwitchState(eSwitch);
			return true;
		}
	}
	return false;
}

#include "inventory.h"
#include "inventoryOwner.h"
void CWeaponMagazinedWGrenade::state_Fire(float dt) 
{
	VERIFY(fOneShotTime>0.f);

	//����� �������� �������������
	if(m_bGrenadeMode)
	{
		/*
		fTime					-=dt;
		while (fTime<=0 && (iAmmoElapsed>0) && (IsWorking() || m_bFireSingleShot))
		{
			++m_iShotNum;
			OnShot			();
			
			// Ammo
			if(Local()) 
			{
				VERIFY				(m_magazine.size());
				m_magazine.pop_back	();
				--iAmmoElapsed;
			
				VERIFY((u32)iAmmoElapsed == m_magazine.size());
			}
		}
		UpdateSounds				();
		if(m_iShotNum == m_iQueueSize) 
			FireEnd();
		*/
	} 
	//����� �������� ���������
	else 
		inherited::state_Fire(dt);
}


void CWeaponMagazinedWGrenade::OnEvent(NET_Packet& P, u16 type) 
{
	inherited::OnEvent(P,type);
	u16 id;
	switch (type) 
	{
		case GE_OWNERSHIP_TAKE: 
			{
				P.r_u16(id);
				CRocketLauncher::AttachRocket(id, this);
			}
			break;
		case GE_OWNERSHIP_REJECT :
		case GE_LAUNCH_ROCKET : 
			{
				bool bLaunch	= (type==GE_LAUNCH_ROCKET);
				P.r_u16			(id);
				CRocketLauncher::DetachRocket(id, bLaunch);
				if(bLaunch)
				{
					PlayAnimShoot		();
					PlaySound			("sndShotG", get_LastFP2());
					AddShotEffector		();
					StartFlameParticles2();
				}
				break;
			}
	}
}

void  CWeaponMagazinedWGrenade::LaunchGrenade()
{
	if(!getRocketCount())	return;
	R_ASSERT				(m_bGrenadeMode);
	{
		Fvector						p1, d; 
		p1.set						(get_LastFP2());
		d.set						(get_LastFD());
		CEntity*					E = smart_cast<CEntity*>(H_Parent());

		if (E){
			CInventoryOwner* io		= smart_cast<CInventoryOwner*>(H_Parent());
			if(NULL == io->inventory().ActiveItem())
			{
				Log("current_state", GetState() );
				Log("next_state", GetNextState());
				Log("item_sect", cNameSect().c_str());
				Log("H_Parent", H_Parent()->cNameSect().c_str());
			}
			E->g_fireParams		(this, p1,d);
		}
		if (IsGameTypeSingle())
			p1.set						(get_LastFP2());
		
		Fmatrix							launch_matrix;
		launch_matrix.identity			();
		launch_matrix.k.set				(d);
		Fvector::generate_orthonormal_basis(launch_matrix.k,
											launch_matrix.j, 
											launch_matrix.i);

		launch_matrix.c.set				(p1);

		if(IsZoomed() && smart_cast<CActor*>(H_Parent()))
		{
			H_Parent()->setEnabled		(FALSE);
			setEnabled					(FALSE);

			collide::rq_result			RQ;
			BOOL HasPick				= Level().ObjectSpace.RayPick(p1, d, 300.0f, collide::rqtStatic, RQ, this);

			setEnabled					(TRUE);
			H_Parent()->setEnabled		(TRUE);

			if(HasPick)
			{
				Fvector					Transference;
				Transference.mul		(d, RQ.range);
				Fvector					res[2];
#ifdef		DEBUG
//.				DBG_OpenCashedDraw();
//.				DBG_DrawLine(p1,Fvector().add(p1,d),color_xrgb(255,0,0));
#endif
				u8 canfire0 = TransferenceAndThrowVelToThrowDir(Transference, 
																CRocketLauncher::m_fLaunchSpeed, 
																EffectiveGravity(), 
																res);
#ifdef DEBUG
//.				if(canfire0>0)DBG_DrawLine(p1,Fvector().add(p1,res[0]),color_xrgb(0,255,0));
//.				if(canfire0>1)DBG_DrawLine(p1,Fvector().add(p1,res[1]),color_xrgb(0,0,255));
//.				DBG_ClosedCashedDraw(30000);
#endif
				
				if (canfire0 != 0)
				{
					d = res[0];
				};
			}
		};
		
		d.normalize						();
		d.mul							(CRocketLauncher::m_fLaunchSpeed);
		VERIFY2							(_valid(launch_matrix),"CWeaponMagazinedWGrenade::SwitchState. Invalid launch_matrix!");
		CRocketLauncher::LaunchRocket	(launch_matrix, d, zero_vel);

		CExplosiveRocket* pGrenade		= smart_cast<CExplosiveRocket*>(getCurrentRocket());
		VERIFY							(pGrenade);
		pGrenade->SetInitiator			(H_Parent()->ID());

		
		if (Local() && OnServer())
		{
			VERIFY				(m_magazine.size());
			m_magazine.pop_back	();
			--iAmmoElapsed;
			VERIFY((u32)iAmmoElapsed == m_magazine.size());

			NET_Packet					P;
			u_EventGen					(P,GE_LAUNCH_ROCKET,ID());
			P.w_u16						(getCurrentRocket()->ID());
			u_EventSend					(P);
		};
	}
}

void CWeaponMagazinedWGrenade::FireEnd() 
{
	if(m_bGrenadeMode)
	{
		CWeapon::FireEnd();
	}else
		inherited::FireEnd();
}

void CWeaponMagazinedWGrenade::OnMagazineEmpty() 
{
	if(GetState() == eIdle) 
	{
		OnEmptyClick			();
	}
}

void CWeaponMagazinedWGrenade::ReloadMagazine() 
{
	inherited::ReloadMagazine();

	//����������� ������������� �����������
	if(iAmmoElapsed && !getRocketCount() && m_bGrenadeMode) 
	{
		shared_str fake_grenade_name = pSettings->r_string(m_ammoTypes[m_ammoType].c_str(), "fake_grenade_name");
		
		CRocketLauncher::SpawnRocket(*fake_grenade_name, this);
	}
}


void CWeaponMagazinedWGrenade::OnStateSwitch(u32 S) 
{

	switch (S)
	{
	case eSwitch:
		{
			if( !SwitchMode() ){
				SwitchState(eIdle);
				return;
			}
		}break;
	}
	
	inherited::OnStateSwitch(S);
	UpdateGrenadeVisibility(!!iAmmoElapsed || S == eReload);
}


void CWeaponMagazinedWGrenade::OnAnimationEnd(u32 state)
{
	switch (state)
	{
	case eSwitch:
		{
			SwitchState(eIdle);
		}break;
	case eFire:
		{
			if(m_bGrenadeMode)
				Reload();
		}break;
	}
	inherited::OnAnimationEnd(state);
}


void CWeaponMagazinedWGrenade::OnH_B_Independent(bool just_before_destroy)
{
	inherited::OnH_B_Independent(just_before_destroy);

	SetPending			(FALSE);
	if (m_bGrenadeMode) {
		SetState		( eIdle );
		SetPending		(FALSE);
	}
}

bool CWeaponMagazinedWGrenade::CanAttach(PIItem pIItem)
{
	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);
	
	if(pGrenadeLauncher &&
	   ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
	   0 == (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
	   !xr_strcmp(*m_sGrenadeLauncherName, pIItem->object().cNameSect()))
       return true;
	else
		return inherited::CanAttach(pIItem);
}

bool CWeaponMagazinedWGrenade::CanDetach(LPCSTR item_section_name)
{
	if(ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
	   0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
	   !xr_strcmp(*m_sGrenadeLauncherName, item_section_name))
	   return true;
	else
	   return inherited::CanDetach(item_section_name);
}

bool CWeaponMagazinedWGrenade::Attach(PIItem pIItem, bool b_send_event)
{
	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(pIItem);
	
	if(pGrenadeLauncher &&
	   ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
	   0 == (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
	   !xr_strcmp(*m_sGrenadeLauncherName, pIItem->object().cNameSect()))
	{
		m_flagsAddOnState |= CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;

		CRocketLauncher::m_fLaunchSpeed = pGrenadeLauncher->GetGrenadeVel();

 		//���������� ������������ �� ���������
		if(b_send_event)
		{
			if (OnServer()) 
				pIItem->object().DestroyObject	();
		}
		InitAddons				();
		UpdateAddonsVisibility	();

		if(GetState()==eIdle)
			PlayAnimIdle		();

		return					true;
	}
	else
        return inherited::Attach(pIItem, b_send_event);
}

bool CWeaponMagazinedWGrenade::Detach(LPCSTR item_section_name, bool b_spawn_item)
{
	if (ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
	   0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher) &&
	   !xr_strcmp(*m_sGrenadeLauncherName, item_section_name))
	{
		m_flagsAddOnState &= ~CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher;
		if(m_bGrenadeMode)
		{
			UnloadMagazine();
			PerformSwitchGL();
		}

		UpdateAddonsVisibility();

		if(GetState()==eIdle)
			PlayAnimIdle		();

		return CInventoryItemObject::Detach(item_section_name, b_spawn_item);
	}
	else
		return inherited::Detach(item_section_name, b_spawn_item);
}




void CWeaponMagazinedWGrenade::InitAddons()
{	
	inherited::InitAddons();

	if(GrenadeLauncherAttachable())
	{
		if(IsGrenadeLauncherAttached())
		{
			CRocketLauncher::m_fLaunchSpeed = pSettings->r_float(*m_sGrenadeLauncherName,"grenade_vel");
		}
	}
}

bool	CWeaponMagazinedWGrenade::UseScopeTexture()
{
	if (IsGrenadeLauncherAttached() && m_bGrenadeMode) return false;
	
	return true;
};

float	CWeaponMagazinedWGrenade::CurrentZoomFactor	()
{
	if (IsGrenadeLauncherAttached() && m_bGrenadeMode) return m_zoom_params.m_fIronSightZoomFactor;
	return inherited::CurrentZoomFactor();
}

//����������� ������� ��� ������������ �������� HUD
void CWeaponMagazinedWGrenade::PlayAnimShow()
{
	VERIFY(GetState()==eShowing);
	if(IsGrenadeLauncherAttached())
	{
		if(!m_bGrenadeMode)
			PlayHUDMotion("anm_show_w_gl", FALSE, this, GetState());
		else
			PlayHUDMotion("anm_show_g", FALSE, this, GetState());
	}	
	else
		PlayHUDMotion("anm_show", FALSE, this, GetState());
}

void CWeaponMagazinedWGrenade::PlayAnimHide()
{
	VERIFY(GetState()==eHiding);
	
	if(IsGrenadeLauncherAttached())
		if(!m_bGrenadeMode)
			PlayHUDMotion("anm_hide_w_gl", TRUE, this, GetState());
		else
			PlayHUDMotion("anm_hide_g", TRUE, this, GetState());

	else
		PlayHUDMotion("anm_hide", TRUE, this, GetState());
}

void CWeaponMagazinedWGrenade::PlayAnimReload()
{
	VERIFY(GetState()==eReload);

	if(IsGrenadeLauncherAttached())
		PlayHUDMotion("anm_reload_w_gl", TRUE, this, GetState());
	else
		inherited::PlayAnimReload();
}

void CWeaponMagazinedWGrenade::PlayAnimIdle()
{
	if(IsGrenadeLauncherAttached())
	{
		if(IsZoomed())
		{
			if(m_bGrenadeMode)
				PlayHUDMotion("anm_idle_g_aim", FALSE, NULL, GetState());
			else
				PlayHUDMotion("anm_idle_w_gl_aim", TRUE, NULL, GetState());
		}else
		{
			int act_state = 0;
			CActor* pActor = smart_cast<CActor*>(H_Parent());
			if(pActor)
			{
				CEntity::SEntityState st;
				pActor->g_State(st);
				if(st.bSprint)
				{
					act_state = 1;
					PlayHUDMotion("anm_idle_sprint", TRUE, NULL,GetState());
				}else
				if(pActor->AnyMove())
				{
					act_state = 2;
					PlayHUDMotion("anm_idle_moving", TRUE, NULL, GetState());
				}
			}

			if(m_bGrenadeMode)
			{
				if(act_state==0)
					PlayHUDMotion("anm_idle_g", FALSE, NULL, GetState());
				else
				if(act_state==1)
					PlayHUDMotion("anm_idle_sprint_g", TRUE, NULL,GetState());
				else
				if(act_state==2)
					PlayHUDMotion("anm_idle_moving_g", TRUE, NULL,GetState());

			}else
			{
				if(act_state==0)
					PlayHUDMotion("anm_idle_w_gl", FALSE, NULL, GetState());
				else
				if(act_state==1)
					PlayHUDMotion("anm_idle_sprint_w_gl", TRUE, NULL,GetState());
				else
				if(act_state==2)
					PlayHUDMotion("anm_idle_moving_w_gl", TRUE, NULL,GetState());
			}
		
		}
	}
	else
		inherited::PlayAnimIdle();
}

void CWeaponMagazinedWGrenade::PlayAnimShoot()
{
	if(m_bGrenadeMode)
	{
		PlayHUDMotion("anm_shots_g" ,FALSE, this, eFire);
	}
	else
	{
		VERIFY(GetState()==eFire);
		if(IsGrenadeLauncherAttached())
			PlayHUDMotion("anm_shots_w_gl" ,FALSE, this, GetState());
		else
			inherited::PlayAnimShoot();
	}
}

void  CWeaponMagazinedWGrenade::PlayAnimModeSwitch()
{
	if(m_bGrenadeMode)
		PlayHUDMotion("anm_switch_g" , FALSE, this, eSwitch);
	else 
		PlayHUDMotion("anm_switch" , FALSE, this, eSwitch);
}

void CWeaponMagazinedWGrenade::PlayAnimBore()
{
	if(IsGrenadeLauncherAttached())
	{
		if(m_bGrenadeMode)
			PlayHUDMotion	("anm_bore_g", TRUE, this, GetState());
		else
			PlayHUDMotion	("anm_bore_w_gl", TRUE, this, GetState());
	}else
		inherited::PlayAnimBore();
}

void CWeaponMagazinedWGrenade::UpdateSounds	()
{
	inherited::UpdateSounds			();
	Fvector P						= get_LastFP();
	m_sounds.SetPosition("sndShotG", P);
	m_sounds.SetPosition("sndReloadG", P);
	m_sounds.SetPosition("sndSwitch", P);
}

void CWeaponMagazinedWGrenade::UpdateGrenadeVisibility(bool visibility)
{
	if(!GetHUDmode())							return;
	HudItemData()->set_bone_visible				("grenade", visibility, TRUE);
}

void CWeaponMagazinedWGrenade::save(NET_Packet &output_packet)
{
	inherited::save								(output_packet);
	save_data									(m_bGrenadeMode, output_packet);
	save_data									(m_magazine2.size(), output_packet);

}

void CWeaponMagazinedWGrenade::load(IReader &input_packet)
{
	inherited::load				(input_packet);
	bool b;
	load_data					(b, input_packet);
	if(b!=m_bGrenadeMode)		
		SwitchMode				();

	u32 sz;
	load_data					(sz, input_packet);

	CCartridge					l_cartridge; 
	l_cartridge.Load			(*m_ammoTypes2[m_ammoType2], u8(m_ammoType2));

	while (sz > m_magazine2.size())
		m_magazine2.push_back(l_cartridge);
}

void CWeaponMagazinedWGrenade::net_Export	(NET_Packet& P)
{
	P.w_u8						(m_bGrenadeMode ? 1 : 0);

	inherited::net_Export		(P);
}

void CWeaponMagazinedWGrenade::net_Import	(NET_Packet& P)
{
	bool NewMode				= FALSE;
	NewMode						= !!P.r_u8();	
	if (NewMode != m_bGrenadeMode)
		SwitchMode				();

	inherited::net_Import		(P);
}

bool CWeaponMagazinedWGrenade::IsNecessaryItem	    (const shared_str& item_sect)
{
	return (	std::find(m_ammoTypes.begin(), m_ammoTypes.end(), item_sect) != m_ammoTypes.end() ||
				std::find(m_ammoTypes2.begin(), m_ammoTypes2.end(), item_sect) != m_ammoTypes2.end() 
			);
}

u8 CWeaponMagazinedWGrenade::GetCurrentHudOffsetIdx()
{
	bool b_aiming		= 	((IsZoomed() && m_zoom_params.m_fZoomRotationFactor<=1.f) ||
							(!IsZoomed() && m_zoom_params.m_fZoomRotationFactor>0.f));
	
	if(!b_aiming)
		return		0;
	else
	if(m_bGrenadeMode)
		return		2;
	else
		return		1;
}

bool CWeaponMagazinedWGrenade::install_upgrade_ammo_class	( LPCSTR section, bool test )
{
	LPCSTR str;

	bool result = process_if_exists( section, "ammo_mag_size", &CInifile::r_s32, iMagazineSize2, test );
	iMagazineSize		= m_bGrenadeMode?1:iMagazineSize2;

	//	ammo_class = ammo_5.45x39_fmj, ammo_5.45x39_ap  // name of the ltx-section of used ammo
	bool result2 = process_if_exists_set( section, "ammo_class", &CInifile::r_string, str, test );
	if ( result2 && !test ) 
	{
		xr_vector<shared_str>& ammo_types	= m_bGrenadeMode ? m_ammoTypes2 : m_ammoTypes;
		ammo_types.clear					(); 
		for ( int i = 0, count = _GetItemCount( str ); i < count; ++i )	
		{
			string128						ammo_item;
			_GetItem						( str, i, ammo_item );
			ammo_types.push_back			( ammo_item );
		}

		shared_str& ammo_name				= m_bGrenadeMode ? m_ammoName2 : m_ammoName;
		ammo_name							= pSettings->r_string( *ammo_types[0], "inv_name_short" );		
		m_ammoType  = 0;
		m_ammoType2 = 0;
	}
	result |= result2;

	return result2;
}

bool CWeaponMagazinedWGrenade::install_upgrade_impl( LPCSTR section, bool test )
{
	LPCSTR str;
	bool result = inherited::install_upgrade_impl( section, test );
	
	//	grenade_class = ammo_vog-25, ammo_vog-25p          // name of the ltx-section of used grenades
	bool result2 = process_if_exists_set( section, "grenade_class", &CInifile::r_string, str, test );
	if ( result2 && !test )
	{
		xr_vector<shared_str>& ammo_types	= !m_bGrenadeMode ? m_ammoTypes2 : m_ammoTypes;
		ammo_types.clear					(); 
		for ( int i = 0, count = _GetItemCount( str ); i < count; ++i )	
		{
			string128						ammo_item;
			_GetItem						( str, i, ammo_item );
			ammo_types.push_back			( ammo_item );
		}

		shared_str& ammo_name				= !m_bGrenadeMode ? m_ammoName2 : m_ammoName;
		ammo_name							= pSettings->r_string( *ammo_types[0], "inv_name_short" );
		m_ammoType  = 0;
		m_ammoType2 = 0;
	}
	result |= result2;

	result |= process_if_exists( section, "launch_speed", &CInifile::r_float, m_fLaunchSpeed, test );

	result2 = process_if_exists_set( section, "snd_shoot_grenade", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_shoot_grenade", "sndShotG", m_eSoundShot );	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_reload_grenade", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_reload_grenade", "sndReloadG", m_eSoundReload );	}
	result |= result2;

	result2 = process_if_exists_set( section, "snd_switch", &CInifile::r_string, str, test );
	if ( result2 && !test ) { m_sounds.LoadSound( section, "snd_switch", "sndSwitch", m_eSoundReload );	}
	result |= result2;

	return result;
}

void CWeaponMagazinedWGrenade::net_Spawn_install_upgrades	( Upgrades_type saved_upgrades )
{
	// do not delete this
	// this is intended behaviour
}
