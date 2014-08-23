////////////////////////////////////////////////////////////////////////////
//	Module 		: UISecondTaskWnd.cpp
//	Created 	: 30.05.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Secondary Task Wnd class impl
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UISecondTaskWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIHelper.h"

#include "UIFrameWindow.h"
#include "UIScrollView.h"
#include "UIStatic.h"
#include "UI3tButton.h"
#include "UICheckButton.h"
#include "UIFrameLineWnd.h"
#include "UIHint.h"

#include "../GameTaskDefs.h"
#include "../gametask.h"
#include "../map_location.h"
#include "UIInventoryUtilities.h"
#include "../string_table.h"
#include "../level.h"
#include "../gametaskmanager.h"
#include "../actor.h"


UISecondTaskWnd::UISecondTaskWnd()
{
	hint_wnd = NULL;
}

UISecondTaskWnd::~UISecondTaskWnd()
{
}

void UISecondTaskWnd::init_from_xml( CUIXml& xml, LPCSTR path )
{
	VERIFY( hint_wnd );
	CUIXmlInit::InitWindow( xml, path, 0, this );

	XML_NODE*  stored_root = xml.GetLocalRoot();
	XML_NODE*  tmpl_root   = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( tmpl_root );
	
	m_background = UIHelper::CreateFrameWindow( xml, "background_frame", this );
	m_caption    = UIHelper::CreateStatic( xml, "t_caption", this );
//	m_counter    = UIHelper::CreateStatic( xml, "t_counter", this );
	m_bt_close   = UIHelper::Create3tButtonEx( xml, "btn_close", this );

	Register( m_bt_close );
	AddCallback( m_bt_close->WindowName(), BUTTON_DOWN, CUIWndCallback::void_function( this, &UISecondTaskWnd::OnBtnClose ) );

	m_list = xr_new<CUIScrollView>();
	m_list->SetAutoDelete( true );
	AttachChild( m_list );
	CUIXmlInit::InitScrollView( xml, "task_list", 0, m_list );
	m_orig_h = GetHeight();

	m_list->SetWindowName("---second_task_list");
	m_list->m_sort_function = fastdelegate::MakeDelegate( this, &UISecondTaskWnd::SortingLessFunction );

	xml.SetLocalRoot( stored_root );
}

bool UISecondTaskWnd::OnMouse( float x, float y, EUIMessages mouse_action )
{
	if ( inherited::OnMouse( x, y, mouse_action ) )
	{
		return true;
	}
	return true;
}

void UISecondTaskWnd::Show( bool status )
{
	inherited::Show( status );
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
	UpdateList();
}

void UISecondTaskWnd::OnFocusReceive()
{
	inherited::OnFocusReceive();
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
}

void UISecondTaskWnd::OnFocusLost()
{
	inherited::OnFocusLost();
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
}

void UISecondTaskWnd::Update()
{
	inherited::Update();
//	UpdateCounter();
}

void UISecondTaskWnd::SendMessage( CUIWindow* pWnd, s16 msg, void* pData )
{
	GetMessageTarget()->SendMessage( pWnd, msg, pData );
	inherited::SendMessage( pWnd, msg, pData );
	CUIWndCallback::OnEvent( pWnd, msg, pData );
}

void UISecondTaskWnd::OnBtnClose( CUIWindow* w, void* d )
{
	Show( false );
}

void UISecondTaskWnd::UpdateList()
{
	m_list->Clear(); // delete[] UISecondTaskItem*
	
	u32 count_for_check = 0;
	vGameTasks& tasks = Level().GameTaskManager().GetGameTasks();
	vGameTasks::iterator itb = tasks.begin();
	vGameTasks::iterator ite = tasks.end();
	for ( ; itb != ite; ++itb )
	{
		CGameTask* task = (*itb).game_task;
		if ( task && task->GetTaskType() == eTaskTypeAdditional && task->GetTaskState() == eTaskStateInProgress )
		{
			UISecondTaskItem* item = xr_new<UISecondTaskItem>();
			if ( item->init_task( task, this ) )
			{
				m_list->AddWindow( item, true );
				++count_for_check;
			}
		}
	}// for
	//float h1 = m_list->GetWndPos().y + m_list->GetHeight() + 15.0f;
	//h1 = _max( h1, 120.0f );
	//h1 = _min( h1, m_orig_h );//_min
	//SetHeight( m_orig_h );
	//m_background->SetHeight( m_orig_h );
}

bool UISecondTaskWnd::SortingLessFunction( CUIWindow* left, CUIWindow* right )
{
	UISecondTaskItem* lpi = smart_cast<UISecondTaskItem*>(left);
	UISecondTaskItem* rpi = smart_cast<UISecondTaskItem*>(right);
	VERIFY( lpi && rpi );
	return ( lpi->get_priority_task() > rpi->get_priority_task() );
}

/*
void UISecondTaskWnd::UpdateCounter()
{
	u32  m_progress_task_count = Level().GameTaskManager().GetTaskCount( eTaskStateInProgress, eTaskTypeAdditional );
	CGameTask* act_task = Level().GameTaskManager().ActiveTask( eTaskTypeAdditional );
	u32 task2_index     = Level().GameTaskManager().GetTaskIndex( act_task, eTaskStateInProgress, eTaskTypeAdditional );

	string32 buf;
	sprintf_s( buf, sizeof(buf), "%d / %d", task2_index, m_progress_task_count );
	m_counter->SetText( buf );
}
*/
// - -----------------------------------------------------------------------------------------------

UISecondTaskItem::UISecondTaskItem()
{
	m_task = NULL;
	
	m_color_states[0] = (u32)(-1);
	m_color_states[1] = (u32)(-1);
	m_color_states[2] = (u32)(-1);
}

UISecondTaskItem::~UISecondTaskItem()
{
}

IC u32 UISecondTaskItem::get_priority_task() const
{
	VERIFY(m_task);
	return m_task->m_priority;
}

bool UISecondTaskItem::init_task( CGameTask* task, UISecondTaskWnd* parent )
{
	VERIFY( task );
	if ( !task )
	{
		return false;
	}
	m_task = task;
	SetMessageTarget( parent );
	
	CUIXml		xml;
	xml.Load( CONFIG_PATH, UI_PATH, PDA_TASK_XML );

	CUIXmlInit::InitWindow( xml, "second_task_wnd:task_item", 0, this );
	
	m_name     = UIHelper::Create3tButtonEx( xml, "second_task_wnd:task_item:name", this );
	m_bt_view  = UIHelper::CreateCheck(      xml, "second_task_wnd:task_item:btn_view", this );
	m_bt_focus = UIHelper::Create3tButtonEx( xml, "second_task_wnd:task_item:btn_focus", this );
	
	m_bt_view->set_hint_wnd(  parent->hint_wnd );
	m_bt_focus->set_hint_wnd( parent->hint_wnd );

	m_color_states[stt_activ ] = CUIXmlInit::GetColor( xml, "second_task_wnd:task_item:activ",  0, (u32)(-1) );
	m_color_states[stt_unread] = CUIXmlInit::GetColor( xml, "second_task_wnd:task_item:unread", 0, (u32)(-1) );
	m_color_states[stt_read  ] = CUIXmlInit::GetColor( xml, "second_task_wnd:task_item:read",   0, (u32)(-1) );
	update_view();
	return true;
}

void UISecondTaskItem::hide_hint()
{
	show_hint_can   = false;
	show_hint       = false;
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
}

void UISecondTaskItem::Update()
{
	inherited::Update();
	update_view();

	if ( m_task && m_name->CursorOverWindow() && show_hint_can )
	{
		if ( Device.dwTimeGlobal > ( m_name->FocusReceiveTime() + 700 ) )
		{
			show_hint = true;
			GetMessageTarget()->SendMessage( this, PDA_TASK_SHOW_HINT, (void*)m_task );
			return;
		}
	}
}

void UISecondTaskItem::update_view()
{
	VERIFY( m_task );
	CMapLocation* ml = m_task->LinkedMapLocation();

	if ( ml && ml->SpotEnabled() )
	{
		m_bt_view->SetCheck( false );
	}
	else
	{
		m_bt_view->SetCheck( true );
	}


	m_name->SetTextST( m_task->m_Title.c_str() );
	m_name->AdjustHeightToText();
	float h1 = m_name->GetWndPos().y + m_name->GetHeight() + 10.0f;
	h1 = _max( h1, GetHeight() );
	SetHeight( h1 );

	CGameTask* activ_task = Level().GameTaskManager().ActiveTask( eTaskTypeAdditional );

	if ( m_task == activ_task )
	{
		m_name->SetTextColor( m_color_states[stt_activ] );
	}
	else if ( m_task->m_read )
	{
		m_name->SetTextColor( m_color_states[stt_read] );
	}
	else
	{
		m_name->SetTextColor( m_color_states[stt_unread] );
	}

}

void UISecondTaskItem::SendMessage( CUIWindow* pWnd, s16 msg, void* pData )
{
	if ( pWnd == m_bt_focus )
	{
		if ( msg == BUTTON_DOWN )
		{
			GetMessageTarget()->SendMessage( this, PDA_TASK_SET_TARGET_MAP, (void*)m_task );
		}
	}
	
	if ( pWnd == m_bt_view )
	{
		if ( m_bt_view->GetCheck() && msg == BUTTON_CLICKED )
		{
			GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_MAP_SPOT, (void*)m_task );
//			Msg( " HIDE task  id = %d", m_task->m_ID );
			return;
		}
		if ( !m_bt_view->GetCheck() && msg == BUTTON_CLICKED )
		{
			GetMessageTarget()->SendMessage( this, PDA_TASK_SHOW_MAP_SPOT, (void*)m_task );
//			Msg( " show task  id = %d", m_task->m_ID );
			return;
		}
	}

	if ( pWnd == m_name )
	{
		if ( msg == BUTTON_DOWN )
		{
			Level().GameTaskManager().SetActiveTask( m_task );
			return;
		}
	}

	inherited::SendMessage( pWnd, msg, pData );
}

bool UISecondTaskItem::OnMouse( float x, float y, EUIMessages mouse_action )
{
	if ( inherited::OnMouse( x, y, mouse_action ) )
	{
		//return true;
	}

	switch ( mouse_action )
	{
	case WINDOW_LBUTTON_DOWN:
	case WINDOW_RBUTTON_DOWN:
	case BUTTON_DOWN:
		{
			hide_hint();
			break;
		}
	}//switch

	return true;
}

void UISecondTaskItem::OnFocusReceive()
{
	inherited::OnFocusReceive();
	hide_hint();
	show_hint_can = true;
}

void UISecondTaskItem::OnFocusLost()
{
	inherited::OnFocusLost();
	hide_hint();
}
