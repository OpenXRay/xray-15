////////////////////////////////////////////////////////////////////////////
//	Module 		: UIRankingWnd.h
//	Created 	: 17.01.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Ranking window class
////////////////////////////////////////////////////////////////////////////

#pragma once
#include "UIWindow.h"
#include "UIWndCallback.h"
#include "UIRankFaction.h"

class CUIStatic;
class CUIXml;
class CUIProgressBar;
class CUIFrameLineWnd;
class CUICharacterInfo;
class CUIScrollView;

class CUIRankingWnd : public CUIWindow, public CUIWndCallback
{
private:
	typedef CUIWindow	inherited;

	CUIFrameLineWnd*	m_background;
	CUIStatic*			m_center_background;
	
	CUICharacterInfo*	m_actor_ch_info;

//	CUIStatic*			m_group_caption;
	CUIStatic*			m_money_caption;
	CUIStatic*			m_money_value;
//
//	CUIStatic*			m_faction_icon;
//	CUIStatic*			m_faction_icon_over;
//	CUIStatic*			m_rank_icon;
//	CUIStatic*			m_rank_icon_over;

	CUIStatic*			m_center_caption;
	CUIStatic*			m_faction_static;
	CUIFrameLineWnd*	m_faction_line1;
	CUIFrameLineWnd*	m_faction_line2;

	u32					m_delay;
	u32					m_previous_time;
	enum				{ max_factions = 9 };
	u32					m_factions_count;
	shared_str			m_faction_id[max_factions];
	CUIScrollView*		m_factions_list;

	enum				{ max_stat_info = 15 };
	CUIStatic*			m_stat_caption[max_stat_info];
	CUIStatic*			m_stat_info[max_stat_info];
	u32					m_stat_count;

public:
						CUIRankingWnd			();
	virtual				~CUIRankingWnd			();

//	virtual void		SendMessage				( CUIWindow* pWnd, s16 msg, void* pData );
	virtual void 		Show					( bool status );
	virtual void		Update					();

			void		Init					();
			void		update_info				();

protected:
			void		add_faction				( CUIXml& xml, shared_str const& faction_id );
			void		clear_all_factions		();
	bool	__stdcall	SortingLessFunction		( CUIWindow* left, CUIWindow* right );
			void		get_value_from_script	();

}; // class CUIRankingWnd
