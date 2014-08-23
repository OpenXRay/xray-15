#ifndef	UI_MAIN_INGAME_WND_H_INCLUDED
#define UI_MAIN_INGAME_WND_H_INCLUDED

#include "UIProgressBar.h"
#include "UIGameLog.h"
#include "UICarPanel.h"
#include "UIMotionIcon.h"
#include "UIZoneMap.h"

#include "../hudsound.h"
#include "../../xrServerEntities/alife_space.h"

class	CUIPdaMsgListItem;
class	CLAItem;
class	CUIZoneMap;
class	CUIScrollView;
struct	GAME_NEWS_DATA;
class	CActor;
class	CWeapon;
class	CMissile;
class	CInventoryItem;
class CUIHudStatesWnd;

class CUIMainIngameWnd: public CUIWindow  
{
public:
			CUIMainIngameWnd();
	virtual ~CUIMainIngameWnd();

	virtual void Init();
	virtual void Draw();
	virtual void Update();

			bool OnKeyboardPress(int dik);

protected:
	
	CUIStatic			UIStaticDiskIO;
//	CUIStatic			UIStaticHealth;
//	CUIStatic			UIStaticArmor;
	CUIStatic			UIStaticQuickHelp;
//	CUIProgressBar		UIHealthBar;
//	CUIProgressBar		UIArmorBar;
//	CUICarPanel			UICarPanel;
	CUIMotionIcon		UIMotionIcon;
	CUIZoneMap*			UIZoneMap;

	//иконка, показывающая количество активных PDA
//	CUIStatic			UIPdaOnline;
	
	CUIHudStatesWnd*	m_ui_hud_states;

public:
	IC	void			ShowZoneMap( bool status ) { UIZoneMap->visible = status; }
		void			DrawZoneMap() { UIZoneMap->Render(); }
		void			UpdateZoneMap() { UIZoneMap->Update(); }
	
	CUIHudStatesWnd*	get_hud_states() { return m_ui_hud_states; } //temp
	void				OnSectorChanged			(int sector);

protected:

	// 5 статиков для отображения иконок:
	// - сломанного оружия
	// - радиации
	// - ранения
	// - голода
	// - усталости
	CUIStatic			UIWeaponJammedIcon;
//	CUIStatic			UIRadiaitionIcon;
//	CUIStatic			UIWoundIcon;
//	CUIStatic			UIStarvationIcon;
//	CUIStatic			UIPsyHealthIcon;
	CUIStatic			UIInvincibleIcon;
//	CUIStatic			UISleepIcon;
	CUIStatic			UIArtefactIcon;

	CUIScrollView*		m_UIIcons;
	CUIWindow*			m_pMPChatWnd;
	CUIWindow*			m_pMPLogWnd;

public:
	
	// Енумы соответсвующие предупреждающим иконкам 
	enum EWarningIcons
	{
		ewiAll				= 0,
		ewiWeaponJammed,
//		ewiRadiation,
//		ewiWound,
//		ewiStarvation,
//		ewiPsyHealth,
//		ewiSleep,
		ewiInvincible,
		ewiArtefact,
	};

	void				SetMPChatLog					(CUIWindow* pChat, CUIWindow* pLog);

	// Задаем цвет соответствующей иконке
	void				SetWarningIconColor				(EWarningIcons icon, const u32 cl);
	void				TurnOffWarningIcon				(EWarningIcons icon);

	// Пороги изменения цвета индикаторов, загружаемые из system.ltx
	typedef				xr_map<EWarningIcons, xr_vector<float> >	Thresholds;
	typedef				Thresholds::iterator						Thresholds_it;
	Thresholds			m_Thresholds;

	// Енум перечисления возможных мигающих иконок
	enum EFlashingIcons
	{
		efiPdaTask	= 0,
		efiMail
	};
	
	void				SetFlashIconState_				(EFlashingIcons type, bool enable);

	void				AnimateContacts					(bool b_snd);
	HUD_SOUND_ITEM		m_contactSnd;

	void				ReceiveNews						(GAME_NEWS_DATA* news);
	
protected:
	void				SetWarningIconColorUI			(CUIStatic* s, const u32 cl);
	void				InitFlashingIcons				(CUIXml* node);
	void				DestroyFlashingIcons			();
	void				UpdateFlashingIcons				();
//	void				UpdateActiveItemInfo			();

//	void				SetAmmoIcon						(const shared_str& seсt_name);

	// first - иконка, second - анимация
	DEF_MAP				(FlashingIcons, EFlashingIcons, CUIStatic*);
	FlashingIcons		m_FlashingIcons;

//	CWeapon*			m_pWeapon;
	CMissile*			m_pGrenade;
	CInventoryItem*		m_pItem;

	// Отображение подсказок при наведении прицела на объект
	void				RenderQuickInfos();

public:
//	CUICarPanel&		CarPanel							(){return UICarPanel;};
	CUIMotionIcon&		MotionIcon							(){return UIMotionIcon;}
	void				OnConnected							();
	void				reset_ui							();

protected:
	CInventoryItem*		m_pPickUpItem;
	CUIStatic			UIPickUpItemIcon;

	float				m_iPickUpItemIconX;
	float				m_iPickUpItemIconY;
	float				m_iPickUpItemIconWidth;
	float				m_iPickUpItemIconHeight;

	void				UpdatePickUpItem();
public:
	void				SetPickUpItem	(CInventoryItem* PickUpItem);
#ifdef DEBUG
	void				draw_adjust_mode					();
#endif
};

#endif // UI_MAIN_INGAME_WND_H_INCLUDED
