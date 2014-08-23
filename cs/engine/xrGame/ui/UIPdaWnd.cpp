#include "stdafx.h"
#include "UIPdaWnd.h"
#include "../Pda.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIInventoryUtilities.h"

#include "../HUDManager.h"
#include "../level.h"
#include "../game_cl_base.h"

#include "UIStatic.h"
#include "UIFrameWindow.h"
#include "UITabControl.h"
#include "UIPdaContactsWnd.h"
#include "UIMapWnd.h"
#include "UIFrameLineWnd.h"
#include "UIActorInfo.h"
#include "object_broker.h"
#include "UIMessagesWindow.h"
#include "UIMainIngameWnd.h"
#include "UITabButton.h"
#include "UIAnimatedStatic.h"

#include "UIHelper.h"
#include "UIHint.h"
#include "UIBtnHint.h"
#include "UITaskWnd.h"
#include "UIFactionWarWnd.h"
#include "UIRankingWnd.h"
#include "UILogsWnd.h"

#define PDA_XML		"pda.xml"

u32 g_pda_info_state = 0;

void RearrangeTabButtons(CUITabControl* pTab);

CUIPdaWnd::CUIPdaWnd()
{
	pUITaskWnd       = NULL;
	pUIFactionWarWnd = NULL;
	pUIRankingWnd    = NULL;
	pUILogsWnd       = NULL;
	m_hint_wnd       = NULL;
	Init();
}

CUIPdaWnd::~CUIPdaWnd()
{
	delete_data( pUITaskWnd );
	delete_data( pUIFactionWarWnd );
	delete_data( pUIRankingWnd );
	delete_data( pUILogsWnd );
	delete_data( m_hint_wnd );
	delete_data( UINoice );
}

void CUIPdaWnd::Init()
{
	CUIXml					uiXml;
	uiXml.Load				(CONFIG_PATH, UI_PATH, PDA_XML);

	m_pActiveDialog			= NULL;
	m_sActiveSection		= "";

	CUIXmlInit::InitWindow	(uiXml, "main", 0, this);

	UIMainPdaFrame			= UIHelper::CreateStatic( uiXml, "background_static", this );
	m_caption				= UIHelper::CreateStatic( uiXml, "caption_static", this );
	m_caption_const._set	( m_caption->GetText() );

	m_anim_static			= xr_new<CUIAnimatedStatic>();
	AttachChild				(m_anim_static);
	m_anim_static->SetAutoDelete(true);
	CUIXmlInit::InitAnimatedStatic(uiXml, "anim_static", 0, m_anim_static);

	m_btn_close				= UIHelper::Create3tButtonEx( uiXml, "close_button", this );
	m_hint_wnd				= UIHelper::CreateHint( uiXml, "hint_wnd" );
//	m_btn_close->set_hint_wnd( m_hint_wnd );


	if ( IsGameTypeSingle() )
	{
		pUITaskWnd					= xr_new<CUITaskWnd>();
		pUITaskWnd->hint_wnd		= m_hint_wnd;
		pUITaskWnd->Init			();

		pUIFactionWarWnd				= xr_new<CUIFactionWarWnd>();
		pUIFactionWarWnd->hint_wnd		= m_hint_wnd;
		pUIFactionWarWnd->Init			();

		pUIRankingWnd					= xr_new<CUIRankingWnd>();
		pUIRankingWnd->Init				();

		pUILogsWnd						= xr_new<CUILogsWnd>();
		pUILogsWnd->Init				();

	}

	UITabControl					= xr_new<CUITabControl>();
	UITabControl->SetAutoDelete		(true);
	AttachChild						(UITabControl);
	CUIXmlInit::InitTabControl		(uiXml, "tab", 0, UITabControl);
	UITabControl->SetMessageTarget	(this);

	UINoice					= xr_new<CUIStatic>();
	UINoice->SetAutoDelete	( true );
	CUIXmlInit::InitStatic	( uiXml, "noice_static", 0, UINoice );

	RearrangeTabButtons		(UITabControl);
}

void CUIPdaWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	switch ( msg )
	{
	case TAB_CHANGED:
		{
			if ( pWnd == UITabControl )
			{
				SetActiveSubdialog			(UITabControl->GetActiveId());
			}
			break;
		}
	case BUTTON_CLICKED:
		{
			if ( pWnd == m_btn_close )
			{
				HUD().GetUI()->StartStopMenu( this, true );
			}
			break;
		}
	default:
		{
			R_ASSERT						(m_pActiveDialog);
			m_pActiveDialog->SendMessage	(pWnd, msg, pData);
		}
	};
}

void CUIPdaWnd::Show()
{
	InventoryUtilities::SendInfoToActor	("ui_pda");
	inherited::Show						();
	
	if ( !m_pActiveDialog )
	{
		SetActiveSubdialog				("eptTasks");
	}
	m_pActiveDialog->Show				(true);
	m_btn_close->Show					(true);
}

void CUIPdaWnd::Hide()
{
	inherited::Hide						();
	InventoryUtilities::SendInfoToActor	("ui_pda_hide");
	HUD().GetUI()->UIMainIngameWnd->SetFlashIconState_(CUIMainIngameWnd::efiPdaTask, false);
	m_pActiveDialog->Show				(false);
	m_btn_close->Show					(false);
	g_btnHint->Discard					();
}

void CUIPdaWnd::Update()
{
	inherited::Update();
	m_pActiveDialog->Update();

	Device.seqParallel.push_back	(fastdelegate::FastDelegate0<>(pUILogsWnd,&CUILogsWnd::PerformWork));
}

void CUIPdaWnd::SetActiveSubdialog(const shared_str& section)
{
	if ( m_sActiveSection == section ) return;

	if ( m_pActiveDialog )
	{
		UIMainPdaFrame->DetachChild( m_pActiveDialog );
		m_pActiveDialog->Show( false );
	}

	if ( section == "eptTasks" )
	{
		m_pActiveDialog = pUITaskWnd;
	}
	else if ( section == "eptFractionWar" )
	{
		m_pActiveDialog = pUIFactionWarWnd;
	}
	else if ( section == "eptRanking" )
	{
		m_pActiveDialog = pUIRankingWnd;
	}
	else if ( section == "eptLogs" )
	{
		m_pActiveDialog = pUILogsWnd;
	}

	R_ASSERT						(m_pActiveDialog);
	UIMainPdaFrame->AttachChild		(m_pActiveDialog);
	m_pActiveDialog->Show			(true);

	if ( UITabControl->GetActiveId() != section )
	{
		UITabControl->SetActiveTab( section );
	}
	m_sActiveSection = section;
	SetActiveCaption();
}

void CUIPdaWnd::SetActiveCaption()
{
	TABS_VECTOR*	btn_vec		= UITabControl->GetButtonsVector();
	TABS_VECTOR::iterator it_b	= btn_vec->begin();
	TABS_VECTOR::iterator it_e	= btn_vec->end();
	for ( ; it_b != it_e; ++it_b )
	{
		if ( (*it_b)->m_btn_id._get() == m_sActiveSection._get() )
		{
			LPCSTR cur = (*it_b)->GetText();
			string256 buf;
			strconcat( sizeof(buf), buf, m_caption_const.c_str(), cur );
			SetCaption( buf );
			return;
		}
	}
}

void CUIPdaWnd::Show_SecondTaskWnd( bool status )
{
	if ( status )
	{
		SetActiveSubdialog( "eptTasks" );
	}
	pUITaskWnd->Show_SecondTasksWnd( status );
}

void CUIPdaWnd::Show_MapLegendWnd( bool status )
{
	if ( status )
	{
		SetActiveSubdialog( "eptTasks" );
	}
	pUITaskWnd->ShowMapLegend( status );
}

void CUIPdaWnd::Draw()
{
	inherited::Draw();
//.	DrawUpdatedSections();
	DrawHint();
	UINoice->Draw(); // over all
}

void CUIPdaWnd::DrawHint()
{
	if ( m_pActiveDialog == pUITaskWnd )
	{
		pUITaskWnd->DrawHint();
	}
	else if ( m_pActiveDialog == pUIFactionWarWnd )
	{
//		m_hint_wnd->Draw();
	}
	else if ( m_pActiveDialog == pUIRankingWnd )
	{

	}
	else if ( m_pActiveDialog == pUILogsWnd )
	{

	}
	m_hint_wnd->Draw();
}

void CUIPdaWnd::UpdatePda()
{
	pUILogsWnd->UpdateNews();

	if ( m_pActiveDialog == pUITaskWnd )
	{
		pUITaskWnd->ReloadTaskInfo();
	}
}

void CUIPdaWnd::Reset()
{
	inherited::ResetAll		();

	if ( pUITaskWnd )		pUITaskWnd->ResetAll();
	if ( pUIFactionWarWnd )	pUITaskWnd->ResetAll();
	if ( pUIRankingWnd )	pUITaskWnd->ResetAll();
	if ( pUILogsWnd )		pUITaskWnd->ResetAll();
}

void CUIPdaWnd::SetCaption( LPCSTR text )
{
	m_caption->SetText( text );
}

void RearrangeTabButtons(CUITabControl* pTab)
{
	TABS_VECTOR *	btn_vec		= pTab->GetButtonsVector();
	TABS_VECTOR::iterator it	= btn_vec->begin();
	TABS_VECTOR::iterator it_e	= btn_vec->end();

	Fvector2					pos;
	pos.set						((*it)->GetWndPos());
	float						size_x;

	for ( ; it != it_e; ++it )
	{
		(*it)->SetWndPos		(pos);
		(*it)->AdjustWidthToText();
		size_x					= (*it)->GetWndSize().x + 30.0f;
		(*it)->SetWidth			(size_x);
		pos.x					+= size_x - 6.0f;
	}
	
	pTab->SetWidth( pos.x + 5.0f );
	pos.x = pTab->GetWndPos().x - pos.x;
	pos.y = pTab->GetWndPos().y;
	pTab->SetWndPos( pos );
}
