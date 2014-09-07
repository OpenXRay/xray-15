#include "stdafx.h"

#include <functional>
#include "UIMainIngameWnd.h"
#include "UIMessagesWindow.h"
#include "UIZoneMap.h"

#include <dinput.h>
#include "Actor.h"
#include "ActorCondition.h"
#include "CustomOutfit.h"
#include "HUDManager.h"
#include "PDA.h"
#include "xrServerEntities/character_info.h"
#include "inventory.h"
#include "UIGameSP.h"
#include "weaponmagazined.h"
#include "missile.h"
#include "Grenade.h"
#include "xrServerEntities/xrServer_objects_ALife.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "game_cl_base.h"
#include "Level.h"
#include "seniority_hierarchy_holder.h"
#include "date_time.h"
#include "xrServerEntities/xrServer_Objects_ALife_Monsters.h"
#include "xrEngine/LightAnimLibrary.h"
#include "UIInventoryUtilities.h"
#include "UIXmlInit.h"
#include "UIPdaMsgListItem.h"
#include "alife_registry_wrappers.h"
#include "actorcondition.h"
#include "string_table.h"

#ifdef DEBUG
#	include "attachable_item.h"
#	include "xrEngine/xr_input.h"
#endif

#include "UIScrollView.h"
#include "map_hint.h"
#include "UIColorAnimatorWrapper.h"
#include "game_news.h"

#include "static_cast_checked.hpp"
#include "game_cl_capture_the_artefact.h"
#include "UIHudStatesWnd.h"
#include "UIActorMenu.h"

void test_draw	();
void test_key	(int dik);

#include "Include/xrRender/Kinematics.h"


using namespace InventoryUtilities;
//BOOL		g_old_style_ui_hud			= FALSE;
const u32	g_clWhite					= 0xffffffff;

#define		DEFAULT_MAP_SCALE			1.f

#define		C_SIZE						0.025f
#define		NEAR_LIM					0.5f

#define		SHOW_INFO_SPEED				0.5f
#define		HIDE_INFO_SPEED				10.f
#define		C_ON_ENEMY					D3DCOLOR_XRGB(0xff,0,0)
#define		C_DEFAULT					D3DCOLOR_XRGB(0xff,0xff,0xff)

#define				MAININGAME_XML				"maingame.xml"

CUIMainIngameWnd::CUIMainIngameWnd()
{
//	m_pWeapon					= NULL;
	m_pGrenade					= NULL;
	m_pItem						= NULL;
	UIZoneMap					= new CUIZoneMap();
	m_pPickUpItem				= NULL;
	m_pMPChatWnd				= NULL;
	m_pMPLogWnd					= NULL;	
}

#include "UIProgressShape.h"
extern CUIProgressShape* g_MissileForceShape;

CUIMainIngameWnd::~CUIMainIngameWnd()
{
	DestroyFlashingIcons		();
	xr_delete					(UIZoneMap);
	HUD_SOUND_ITEM::DestroySound(m_contactSnd);
	xr_delete					(g_MissileForceShape);
}

void CUIMainIngameWnd::Init()
{
	CUIXml						uiXml;
	uiXml.Load					(CONFIG_PATH, UI_PATH, MAININGAME_XML);
	
	CUIXmlInit					xml_init;
	xml_init.InitWindow			(uiXml,"main",0,this);

	Enable(false);

//	AttachChild					(&UIStaticHealth);	xml_init.InitStatic			(uiXml, "static_health", 0, &UIStaticHealth);
//	AttachChild					(&UIStaticArmor);	xml_init.InitStatic			(uiXml, "static_armor", 0, &UIStaticArmor);
//	AttachChild					(&UIWeaponBack);
//	xml_init.InitStatic			(uiXml, "static_weapon", 0, &UIWeaponBack);

/*	UIWeaponBack.AttachChild	(&UIWeaponSignAmmo);
	xml_init.InitStatic			(uiXml, "static_ammo", 0, &UIWeaponSignAmmo);
	UIWeaponSignAmmo.SetEllipsis	(CUIStatic::eepEnd, 2);

	UIWeaponBack.AttachChild	(&UIWeaponIcon);
	xml_init.InitStatic			(uiXml, "static_wpn_icon", 0, &UIWeaponIcon);
	UIWeaponIcon.SetShader		(GetEquipmentIconsShader());
	UIWeaponIcon_rect			= UIWeaponIcon.GetWndRect();
*/	//---------------------------------------------------------
	AttachChild					(&UIPickUpItemIcon);
	xml_init.InitStatic			(uiXml, "pick_up_item", 0, &UIPickUpItemIcon);
	UIPickUpItemIcon.SetShader	(GetEquipmentIconsShader());
	UIPickUpItemIcon.ClipperOn	();

	m_iPickUpItemIconWidth		= UIPickUpItemIcon.GetWidth();
	m_iPickUpItemIconHeight		= UIPickUpItemIcon.GetHeight();
	m_iPickUpItemIconX			= UIPickUpItemIcon.GetWndRect().left;
	m_iPickUpItemIconY			= UIPickUpItemIcon.GetWndRect().top;
	//---------------------------------------------------------

	//индикаторы 
	UIZoneMap->Init				();

	// Подсказки, которые возникают при наведении прицела на объект
	AttachChild					(&UIStaticQuickHelp);
	xml_init.InitStatic			(uiXml, "quick_info", 0, &UIStaticQuickHelp);

	uiXml.SetLocalRoot			(uiXml.GetRoot());

	m_UIIcons					= new CUIScrollView(); m_UIIcons->SetAutoDelete(true);
	xml_init.InitScrollView		(uiXml, "icons_scroll_view", 0, m_UIIcons);
	AttachChild					(m_UIIcons);

	// Загружаем иконки 
/*	if ( IsGameTypeSingle() )
	{
		xml_init.InitStatic		(uiXml, "starvation_static", 0, &UIStarvationIcon);
		UIStarvationIcon.Show	(false);

//		xml_init.InitStatic		(uiXml, "psy_health_static", 0, &UIPsyHealthIcon);
//		UIPsyHealthIcon.Show	(false);
	}
*/
	xml_init.InitStatic			(uiXml, "weapon_jammed_static", 0, &UIWeaponJammedIcon);
	UIWeaponJammedIcon.Show		(false);

//	xml_init.InitStatic			(uiXml, "radiation_static", 0, &UIRadiaitionIcon);
//	UIRadiaitionIcon.Show		(false);

//	xml_init.InitStatic			(uiXml, "wound_static", 0, &UIWoundIcon);
//	UIWoundIcon.Show			(false);

	xml_init.InitStatic			(uiXml, "invincible_static", 0, &UIInvincibleIcon);
	UIInvincibleIcon.Show		(false);


	if ( (GameID() == eGameIDArtefactHunt) || (GameID() == eGameIDCaptureTheArtefact) )
	{
		xml_init.InitStatic		(uiXml, "artefact_static", 0, &UIArtefactIcon);
		UIArtefactIcon.Show		(false);
	}
	
	shared_str warningStrings[7] = 
	{	
		"jammed",
		"radiation",
		"wounds",
		"starvation",
		"fatigue",
		"invincible"
		"artefact"
	};

	// Загружаем пороговые значения для индикаторов
	EWarningIcons j = ewiWeaponJammed;
	while (j < ewiInvincible)
	{
		// Читаем данные порогов для каждого индикатора
		shared_str cfgRecord = pSettings->r_string("main_ingame_indicators_thresholds", *warningStrings[static_cast<int>(j) - 1]);
		u32 count = _GetItemCount(*cfgRecord);

		char	singleThreshold[8];
		float	f = 0;
		for (u32 k = 0; k < count; ++k)
		{
			_GetItem(*cfgRecord, k, singleThreshold);
			sscanf(singleThreshold, "%f", &f);

			m_Thresholds[j].push_back(f);
		}

		j = static_cast<EWarningIcons>(j + 1);
	}


	// Flashing icons initialize
	uiXml.SetLocalRoot						(uiXml.NavigateToNode("flashing_icons"));
	InitFlashingIcons						(&uiXml);

	uiXml.SetLocalRoot						(uiXml.GetRoot());
	
//	AttachChild								(&UICarPanel);	xml_init.InitWindow		(uiXml, "car_panel", 0, &UICarPanel);

	AttachChild								(&UIMotionIcon);
	UIMotionIcon.Init						();

	AttachChild								(&UIStaticDiskIO);
	xml_init.InitStatic						(uiXml, "disk_io", 0, &UIStaticDiskIO);

	m_ui_hud_states							= new CUIHudStatesWnd();
	m_ui_hud_states->SetAutoDelete			(true);
	AttachChild								(m_ui_hud_states);
	m_ui_hud_states->InitFromXml			(uiXml, "hud_states");

	HUD_SOUND_ITEM::LoadSound					("maingame_ui", "snd_new_contact", m_contactSnd, SOUND_TYPE_IDLE);
}

float UIStaticDiskIO_start_time = 0.0f;
void CUIMainIngameWnd::Draw()
{
	CActor* m_pActor		= smart_cast<CActor*>(Level().CurrentViewEntity());
#ifdef DEBUG
	test_draw				();
#endif
	// show IO icon
	bool IOActive	= (FS.dwOpenCounter>0);
	if	(IOActive)	UIStaticDiskIO_start_time = Device.fTimeGlobal;

	if ((UIStaticDiskIO_start_time+1.0f) < Device.fTimeGlobal)	UIStaticDiskIO.Show(false); 
	else {
		u32		alpha			= clampr(iFloor(255.f*(1.f-(Device.fTimeGlobal-UIStaticDiskIO_start_time)/1.f)),0,255);
		UIStaticDiskIO.Show		( true  ); 
		UIStaticDiskIO.SetColor	(color_rgba(255,255,255,alpha));
	}
	FS.dwOpenCounter = 0;

	if(!IsGameTypeSingle())
	{
		float		luminocity = smart_cast<CGameObject*>(Level().CurrentEntity())->ROS()->get_luminocity();
		float		power = log(luminocity > .001f ? luminocity : .001f)*(1.f/*luminocity_factor*/);
		luminocity	= exp(power);

		static float cur_lum = luminocity;
		cur_lum = luminocity*0.01f + cur_lum*0.99f;
		UIMotionIcon.SetLuminosity((s16)iFloor(cur_lum*100.0f));
	}
	if ( !m_pActor || !m_pActor->g_Alive() ) return;

	UIMotionIcon.SetNoise((s16)(0xffff&iFloor(m_pActor->m_snd_noise*100.0f)));

	CUIWindow::Draw();

	UIZoneMap->visible = true;
	UIZoneMap->Render();

	RenderQuickInfos();		
}


void CUIMainIngameWnd::SetMPChatLog(CUIWindow* pChat, CUIWindow* pLog){
	m_pMPChatWnd = pChat;
	m_pMPLogWnd  = pLog;
}

void CUIMainIngameWnd::Update()
{
	CUIWindow::Update();
	CActor* m_pActor = smart_cast<CActor*>(Level().CurrentViewEntity());

	if ( m_pMPChatWnd )
	{
		m_pMPChatWnd->Update();
	}
	if ( m_pMPLogWnd )
	{
		m_pMPLogWnd->Update();
	}

	if ( !m_pActor )
	{
		m_pItem				= NULL;
//		m_pWeapon			= NULL;
		m_pGrenade			= NULL;
//y		CUIWindow::Update	();
		return;
	}

	UIZoneMap->Update();
	
//	UIHealthBar.SetProgressPos	(m_pActor->GetfHealth()*100.0f);
	UIMotionIcon.SetPower		(m_pActor->conditions().GetPower()*100.0f);
	
	UpdatePickUpItem			();

	if( Device.dwFrame % 10 )
	{
		return;
	}

	game_PlayerState* lookat_player = Game().local_player;
	if (Level().IsDemoPlayStarted())
	{
		lookat_player = Game().lookat_player();
	}
	bool b_God = ( GodMode() || ( !lookat_player ) )? true : lookat_player->testFlag(GAME_PLAYER_FLAG_INVINCIBLE);
	if ( b_God )
	{
		SetWarningIconColor( ewiInvincible, 0xffffffff );
	}
	else
	{
		SetWarningIconColor( ewiInvincible, 0x00ffffff );
	}
	
	if ( IsGameTypeSingle() )
	{
		return;
	}

	// ewiArtefact
	if ( GameID() == eGameIDArtefactHunt )
	{
		bool b_Artefact = !!( m_pActor->inventory().ItemFromSlot(ARTEFACT_SLOT) );
		if ( b_Artefact )
		{
			SetWarningIconColor( ewiArtefact, 0xffffff00 );
		}
		else
		{
			SetWarningIconColor( ewiArtefact, 0x00ffffff );
		}
	}
	else if ( GameID() == eGameIDCaptureTheArtefact )
	{
		//this is a bad style... It left for backward compatibility
		//need to move this logic into UIGameCTA class
		//bool b_Artefact = (NULL != m_pActor->inventory().ItemFromSlot(ARTEFACT_SLOT));
		game_cl_CaptureTheArtefact* cta_game = static_cast_checked<game_cl_CaptureTheArtefact*>(&Game());
		R_ASSERT(cta_game);
		R_ASSERT(lookat_player);
		
		if ( ( m_pActor->ID() == cta_game->GetGreenArtefactOwnerID() ) ||
			 ( m_pActor->ID() == cta_game->GetBlueArtefactOwnerID()  ) )
		{
			SetWarningIconColor( ewiArtefact, 0xffff0000 );
		}
		else if ( m_pActor->inventory().ItemFromSlot(ARTEFACT_SLOT) ) //own artefact
		{
			SetWarningIconColor( ewiArtefact, 0xff00ff00 );
		}
		else
		{
			SetWarningIconColor(ewiArtefact, 0x00ffffff );
		}
	}

	//	UpdateActiveItemInfo();

	EWarningIcons i	= ewiWeaponJammed;
	while ( i <= ewiWeaponJammed ) // ewiInvincible
	{
		float value = 0;
		switch (i)
		{
			//radiation
			/*			case ewiRadiation:
			value = m_pActor->conditions().GetRadiation();
			break;
			case ewiWound:
			value = m_pActor->conditions().BleedingSpeed();
			break;
			*/		
		case ewiWeaponJammed:
			{
				PIItem item = m_pActor->inventory().ActiveItem();
				if ( item )
				{
					CWeapon* pWeapon = smart_cast<CWeapon*>( item );
					if ( pWeapon )
					{
						value = _max( 0.0f, 1.0f - pWeapon->GetConditionToShow() );
					}
				}
				break;
			}
		/*case ewiStarvation:
			value =  _max( 0.0f, 1.0f - m_pActor->conditions().GetSatiety() );
			break;
		*/
		/*case ewiPsyHealth:
			value = 1 - m_pActor->conditions().GetPsyHealth();
			break;
		*/
		default:
			R_ASSERT(!"Unknown type of warning icon");
		}


		xr_vector<float>::reverse_iterator	rit;

		// Сначала проверяем на точное соответсвие
		rit  = std::find( m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), value );

		// Если его нет, то берем последнее меньшее значение ()
		if ( rit == m_Thresholds[i].rend() )
		{
			rit = std::find_if(m_Thresholds[i].rbegin(), m_Thresholds[i].rend(), std::bind2nd(std::less<float>(), value));
		}

		// Минимальное и максимальное значения границы
		float min = m_Thresholds[i].front();
		float max = m_Thresholds[i].back();

		if ( rit != m_Thresholds[i].rend() )
		{
			float v = *rit;
			SetWarningIconColor(i, color_argb(0xFF, clampr<u32>(static_cast<u32>(255 * ((v - min) / (max - min) * 2)), 0, 255), 
				clampr<u32>(static_cast<u32>(255 * (2.0f - (v - min) / (max - min) * 2)), 0, 255), 0));
		}
		else
		{
			TurnOffWarningIcon(i);
		}

		i = (EWarningIcons)(i + 1);
	}//	while ( i <= ewiWeaponJammed ) // ewiInvincible

}//update

bool CUIMainIngameWnd::OnKeyboardPress(int dik)
{
#ifdef DEBUG
	test_key(dik);
#endif // #ifdef DEBUG
/*
	if(Level().IR_GetKeyState(DIK_LSHIFT) || Level().IR_GetKeyState(DIK_RSHIFT))
	{
//		switch(dik)
//		{
//		case DIK_NUMPADMINUS:
//			UIZoneMap->ZoomOut();
//			return true;
//			break;
//		case DIK_NUMPADPLUS:
//			UIZoneMap->ZoomIn();
//			return true;
//			break;
//		}
	}
	else
	{
		switch(dik)
		{
		case DIK_NUMPADMINUS:
			if ( HUD().GetUI()->UIGame() && !HUD().GetUI()->UIGame()->ActorMenu().IsShown() )
			{
				HUD().GetUI()->ShowGameIndicators(false);
			}
			return true;
			break;
		case DIK_NUMPADPLUS:
			if ( HUD().GetUI()->UIGame() && !HUD().GetUI()->UIGame()->ActorMenu().IsShown() )
			{
				HUD().GetUI()->ShowGameIndicators(true);
			}
			return true;
			break;
		}
	}
*/
	return false;
}


void CUIMainIngameWnd::RenderQuickInfos()
{
	CActor* m_pActor		= smart_cast<CActor*>(Level().CurrentViewEntity());
	if (!m_pActor)
		return;

	static CGameObject *pObject			= NULL;
	LPCSTR actor_action					= m_pActor->GetDefaultActionForObject();
	UIStaticQuickHelp.Show				(NULL!=actor_action);

	if(NULL!=actor_action){
		if(stricmp(actor_action,UIStaticQuickHelp.GetText()))
			UIStaticQuickHelp.SetTextST				(actor_action);
	}

	if(pObject!=m_pActor->ObjectWeLookingAt())
	{
		UIStaticQuickHelp.SetTextST				(actor_action?actor_action:" ");
		UIStaticQuickHelp.ResetClrAnimation		();
		pObject	= m_pActor->ObjectWeLookingAt	();
	}
}

void CUIMainIngameWnd::ReceiveNews(GAME_NEWS_DATA* news)
{
	VERIFY(news->texture_name.size());

//	HUD().GetUI()->m_pMessagesWnd->AddIconedPdaMessage(news->texture_name.c_str(), news->tex_rect, news->SingleLineText(), news->show_time);
	HUD().GetUI()->m_pMessagesWnd->AddIconedPdaMessage(news);
	HUD().GetUI()->UpdatePda();
}

void CUIMainIngameWnd::SetWarningIconColorUI(CUIStatic* s, const u32 cl)
{
	int bOn = ( cl >> 24 );
	bool bIsShown = s->IsShown();

	if ( bOn )
	{
		s->SetColor( cl );
	}

	if ( bOn && !bIsShown )
	{
		m_UIIcons->AddWindow	(s, false);
		s->Show					(true);
	}

	if ( !bOn && bIsShown )
	{
		m_UIIcons->RemoveWindow	(s);
		s->Show					(false);
	}
}

void CUIMainIngameWnd::SetWarningIconColor(EWarningIcons icon, const u32 cl)
{
	bool bMagicFlag = true;

	// Задаем цвет требуемой иконки
	switch(icon)
	{
	case ewiAll:
		bMagicFlag = false;
	case ewiWeaponJammed:
		SetWarningIconColorUI	(&UIWeaponJammedIcon, cl);
		if (bMagicFlag) break;

/*	case ewiRadiation:
		SetWarningIconColorUI	(&UIRadiaitionIcon, cl);
		if (bMagicFlag) break;
	case ewiWound:
		SetWarningIconColorUI	(&UIWoundIcon, cl);
		if (bMagicFlag) break;

	case ewiStarvation:
		SetWarningIconColorUI	(&UIStarvationIcon, cl);
		if (bMagicFlag) break;	
	case ewiPsyHealth:
		SetWarningIconColorUI	(&UIPsyHealthIcon, cl);
		if (bMagicFlag) break;
*/
	case ewiInvincible:
		SetWarningIconColorUI	(&UIInvincibleIcon, cl);
		if (bMagicFlag) break;
		break;
	case ewiArtefact:
		SetWarningIconColorUI	(&UIArtefactIcon, cl);
		break;

	default:
		R_ASSERT(!"Unknown warning icon type");
	}
}

void CUIMainIngameWnd::TurnOffWarningIcon(EWarningIcons icon)
{
	SetWarningIconColor(icon, 0x00ffffff);
}

void CUIMainIngameWnd::SetFlashIconState_(EFlashingIcons type, bool enable)
{
	// Включаем анимацию требуемой иконки
	FlashingIcons_it icon = m_FlashingIcons.find(type);
	R_ASSERT2(icon != m_FlashingIcons.end(), "Flashing icon with this type not existed");
	icon->second->Show(enable);
}

void CUIMainIngameWnd::InitFlashingIcons(CUIXml* node)
{
	const char * const flashingIconNodeName = "flashing_icon";
	int staticsCount = node->GetNodesNum("", 0, flashingIconNodeName);

	CUIXmlInit xml_init;
	CUIStatic *pIcon = NULL;
	// Пробегаемся по всем нодам и инициализируем из них статики
	for (int i = 0; i < staticsCount; ++i)
	{
		pIcon = new CUIStatic();
		xml_init.InitStatic(*node, flashingIconNodeName, i, pIcon);
		shared_str iconType = node->ReadAttrib(flashingIconNodeName, i, "type", "none");

		// Теперь запоминаем иконку и ее тип
		EFlashingIcons type = efiPdaTask;

		if		(iconType == "pda")		type = efiPdaTask;
		else if (iconType == "mail")	type = efiMail;
		else	R_ASSERT(!"Unknown type of mainingame flashing icon");

		R_ASSERT2(m_FlashingIcons.find(type) == m_FlashingIcons.end(), "Flashing icon with this type already exists");

		CUIStatic* &val	= m_FlashingIcons[type];
		val			= pIcon;

		AttachChild(pIcon);
		pIcon->Show(false);
	}
}

void CUIMainIngameWnd::DestroyFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		DetachChild(it->second);
		xr_delete(it->second);
	}

	m_FlashingIcons.clear();
}

void CUIMainIngameWnd::UpdateFlashingIcons()
{
	for (FlashingIcons_it it = m_FlashingIcons.begin(); it != m_FlashingIcons.end(); ++it)
	{
		it->second->Update();
	}
}

void CUIMainIngameWnd::AnimateContacts(bool b_snd)
{
	UIZoneMap->Counter_ResetClrAnimation();

	if(b_snd)
		HUD_SOUND_ITEM::PlaySound	(m_contactSnd, Fvector().set(0,0,0), 0, true );

}


void CUIMainIngameWnd::SetPickUpItem	(CInventoryItem* PickUpItem)
{
	m_pPickUpItem = PickUpItem;
};

void CUIMainIngameWnd::UpdatePickUpItem	()
{
	if (!m_pPickUpItem || !Level().CurrentViewEntity() || !smart_cast<CActor*>(Level().CurrentViewEntity())) 
	{
		UIPickUpItemIcon.Show(false);
		return;
	};


	shared_str sect_name	= m_pPickUpItem->object().cNameSect();

	//properties used by inventory menu
	int m_iGridWidth	= pSettings->r_u32(sect_name, "inv_grid_width");
	int m_iGridHeight	= pSettings->r_u32(sect_name, "inv_grid_height");

	int m_iXPos			= pSettings->r_u32(sect_name, "inv_grid_x");
	int m_iYPos			= pSettings->r_u32(sect_name, "inv_grid_y");

	float scale_x = m_iPickUpItemIconWidth/
		float(m_iGridWidth*INV_GRID_WIDTH);

	float scale_y = m_iPickUpItemIconHeight/
		float(m_iGridHeight*INV_GRID_HEIGHT);

	scale_x = (scale_x>1) ? 1.0f : scale_x;
	scale_y = (scale_y>1) ? 1.0f : scale_y;

	float scale = scale_x<scale_y?scale_x:scale_y;

	UIPickUpItemIcon.GetUIStaticItem().SetOriginalRect(
		float(m_iXPos * INV_GRID_WIDTH),
		float(m_iYPos * INV_GRID_HEIGHT),
		float(m_iGridWidth * INV_GRID_WIDTH),
		float(m_iGridHeight * INV_GRID_HEIGHT));

	UIPickUpItemIcon.SetStretchTexture(true);

	UIPickUpItemIcon.SetWidth(m_iGridWidth*INV_GRID_WIDTH*scale);
	UIPickUpItemIcon.SetHeight(m_iGridHeight*INV_GRID_HEIGHT*scale);

	UIPickUpItemIcon.SetWndPos(Fvector2().set(	m_iPickUpItemIconX+(m_iPickUpItemIconWidth-UIPickUpItemIcon.GetWidth())/2.0f,
												m_iPickUpItemIconY+(m_iPickUpItemIconHeight-UIPickUpItemIcon.GetHeight())/2.0f) );

	UIPickUpItemIcon.SetColor(color_rgba(255,255,255,192));
	UIPickUpItemIcon.Show(true);
};

void CUIMainIngameWnd::OnConnected()
{
	UIZoneMap->SetupCurrentMap();
	if ( m_ui_hud_states )
	{
		m_ui_hud_states->on_connected();
	}
}

void CUIMainIngameWnd::OnSectorChanged(int sector)
{
	UIZoneMap->OnSectorChanged(sector);
}

void CUIMainIngameWnd::reset_ui()
{
//	m_pWeapon						= NULL;
	m_pGrenade						= NULL;
	m_pItem							= NULL;
	m_pPickUpItem					= NULL;
	UIMotionIcon.ResetVisibility	();
	if ( m_ui_hud_states )
	{
		m_ui_hud_states->reset_ui();
	}
}


#include "xrEngine/xr_input.h"
#include "GamePersistent.h"

void hud_adjust_mode_keyb(int dik);
void hud_draw_adjust_mode();

#ifdef DEBUG
	void attach_adjust_mode_keyb(int dik);
	void attach_draw_adjust_mode();
#endif

struct TS{
	ref_sound test_sound;
};
TS* pTS = NULL;
void test_key(int dik)
{
	hud_adjust_mode_keyb	(dik);
#ifdef DEBUG
	attach_adjust_mode_keyb	(dik);
#endif
/*
	if(dik==DIK_V)
	{
		if (!pTS)
		{
			pTS = new TS();
			Msg("created");
		}else
		{
			xr_delete(pTS);
			Msg("destroyed");
		}
	}
	if(dik==DIK_B && pTS)
	{
		pTS->test_sound.create("music\\combat\\theme1_intro", st_Effect, 0);
		pTS->test_sound.play_at_pos(Actor(), Fvector().set(0,0,0), sm_2D);
		pTS->test_sound.attach_tail("music\\combat\\theme1_combat_2");
	}
	if(dik==DIK_N && pTS)
	{
		pTS->test_sound.attach_tail("music\\combat\\theme1_combat_2");
	}
	if(dik==DIK_M && pTS)
	{
		pTS->test_sound.attach_tail("music\\combat\\theme1_final");
	}
*/
}

void test_draw()
{
	hud_draw_adjust_mode();
#ifdef DEBUG
	attach_draw_adjust_mode();
#endif
}

