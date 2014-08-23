#include "stdafx.h"
#include "UITaskWnd.h"
#include "UIMapWnd.h"
#include "object_broker.h"
#include "UIXmlInit.h"
#include "UIStatic.h"
#include "UI3tButton.h"
#include "UIFrameLineWnd.h"
#include "UISecondTaskWnd.h"
#include "UIMapLegend.h"
#include "UIHelper.h"
#include "UIHint.h"

#include "../gametask.h"
#include "../map_location.h"
#include "UIInventoryUtilities.h"
#include "../string_table.h"
#include "../level.h"
#include "../gametaskmanager.h"
#include "../actor.h"


CUITaskWnd::CUITaskWnd()
{
	hint_wnd = NULL;
}

CUITaskWnd::~CUITaskWnd()
{
	delete_data						(m_pMapWnd);
}

void CUITaskWnd::Init()
{
	CUIXml							xml;
	xml.Load						(CONFIG_PATH, UI_PATH, PDA_TASK_XML);
	VERIFY							(hint_wnd);

	CUIXmlInit::InitWindow			(xml, "main_wnd", 0, this);

	m_background					= UIHelper::CreateFrameLine( xml, "background", this );
	m_task_split					= UIHelper::CreateFrameLine( xml, "task_split", this );

	m_pMapWnd						= xr_new<CUIMapWnd>(); 
	m_pMapWnd->SetAutoDelete		(false);
	m_pMapWnd->hint_wnd				= hint_wnd;
	m_pMapWnd->Init					(PDA_TASK_XML,"map_wnd");
	AttachChild						(m_pMapWnd);

	m_center_background				= UIHelper::CreateStatic( xml, "center_background", this );
//	m_right_bottom_background		= UIHelper::CreateStatic( xml, "right_bottom_background", this );

	m_pStoryLineTaskItem			= xr_new<CUITaskItem>();
	m_pStoryLineTaskItem->Init		(xml,"storyline_task_item");
	AttachChild						(m_pStoryLineTaskItem);
	m_pStoryLineTaskItem->SetAutoDelete(true);
	AddCallback						(m_pStoryLineTaskItem->WindowName(), WINDOW_LBUTTON_DB_CLICK,   CUIWndCallback::void_function(this,&CUITaskWnd::OnTask1DbClicked));
	
	m_pSecondaryTaskItem			= xr_new<CUITaskItem>();
	m_pSecondaryTaskItem->Init		(xml,"secondary_task_item");
	AttachChild						(m_pSecondaryTaskItem);
	m_pSecondaryTaskItem->SetAutoDelete(true);
	AddCallback						(m_pSecondaryTaskItem->WindowName(), WINDOW_LBUTTON_DB_CLICK,   CUIWndCallback::void_function(this,&CUITaskWnd::OnTask2DbClicked));
	

	m_btn_focus		= UIHelper::Create3tButtonEx( xml, "btn_task_focus", this );
	m_btn_focus2	= UIHelper::Create3tButtonEx( xml, "btn_task_focus2", this );
	Register		(m_btn_focus);
	Register		(m_btn_focus2);
	AddCallback		(m_btn_focus->WindowName(),  BUTTON_DOWN, CUIWndCallback::void_function(this,&CUITaskWnd::OnTask1DbClicked));
	AddCallback		(m_btn_focus2->WindowName(), BUTTON_DOWN, CUIWndCallback::void_function(this,&CUITaskWnd::OnTask2DbClicked));
	m_btn_focus->set_hint_wnd(  hint_wnd );
	m_btn_focus2->set_hint_wnd( hint_wnd );

//	m_pBtnNextTask					= UIHelper::Create3tButton( xml, "btn_next_task", this );
//	AddCallback						(m_pBtnNextTask->WindowName(),BUTTON_CLICKED,CUIWndCallback::void_function(this,&CUITaskWnd::OnNextTaskClicked));

	m_BtnSecondaryTaskWnd	= UIHelper::Create3tButtonEx( xml, "btn_second_task", this );
	AddCallback				(m_BtnSecondaryTaskWnd->WindowName(), BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowSecondTaskWnd));
	m_BtnSecondaryTaskWnd->set_hint_wnd( hint_wnd );

	m_second_task_index		= UIHelper::CreateStatic( xml, "second_task_index", this );

	m_second_task_wnd					= xr_new<UISecondTaskWnd>(); 
	m_second_task_wnd->SetAutoDelete	(true);
	m_second_task_wnd->hint_wnd			= hint_wnd;
	m_second_task_wnd->init_from_xml	(xml, "second_task_wnd");
	m_pMapWnd->AttachChild				(m_second_task_wnd);
	m_second_task_wnd->SetMessageTarget	(this);
	m_second_task_wnd->Show				(false);
	m_second_task_wnd_show				= false;

	m_map_legend_wnd					= xr_new<UIMapLegend>(); 
	m_map_legend_wnd->SetAutoDelete		(true);
	m_map_legend_wnd->init_from_xml		(xml, "map_legend_wnd");
	m_pMapWnd->AttachChild				(m_map_legend_wnd);
	m_map_legend_wnd->SetMessageTarget	(this);
	m_map_legend_wnd->Show				(false);

}

void CUITaskWnd::Update()
{
	if(Level().GameTaskManager().ActualFrame() != m_actual_frame)
	{
		ReloadTaskInfo();
	}

	if ( m_pStoryLineTaskItem->show_hint && m_pStoryLineTaskItem->OwnerTask() )
	{
		m_pMapWnd->ShowHintTask( m_pStoryLineTaskItem->OwnerTask(), m_pStoryLineTaskItem );
		//ReloadTaskInfo();
	}
	else if ( m_pSecondaryTaskItem->show_hint && m_pSecondaryTaskItem->OwnerTask() )
	{
		m_pStoryLineTaskItem->show_hint = false;
		m_pMapWnd->ShowHintTask( m_pSecondaryTaskItem->OwnerTask(), m_pSecondaryTaskItem );
		//ReloadTaskInfo();
	}
	else
	{
		m_pMapWnd->HideCurHint();
	}
	inherited::Update				();
}

void CUITaskWnd::Draw()
{
	inherited::Draw					();
}

void CUITaskWnd::DrawHint()
{
	m_pMapWnd->DrawHint();
}


void CUITaskWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	/*if ( pWnd == m_pSecondaryTaskItem )
	{
		if ( msg == WINDOW_MOUSE_WHEEL_UP )
		{
			OnNextTaskClicked();
			return;
		}
		if ( msg == WINDOW_MOUSE_WHEEL_DOWN )
		{
			OnPrevTaskClicked();
			return;
		}
	}*/
	
	if ( msg == PDA_TASK_SET_TARGET_MAP && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		TaskSetTargetMap( task );
		return;
	}
	if ( msg == PDA_TASK_SHOW_MAP_SPOT && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		TaskShowMapSpot( task, true );
		return;
	}
	if ( msg == PDA_TASK_HIDE_MAP_SPOT && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		TaskShowMapSpot( task, false );
		return;
	}
	
	if ( msg == PDA_TASK_SHOW_HINT && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		m_pMapWnd->ShowHintTask( task, pWnd );
		return;
	}
	if ( msg == PDA_TASK_HIDE_HINT )
	{
		m_pMapWnd->HideCurHint();
		return;
	}

	inherited::SendMessage(  pWnd, msg, pData );
	CUIWndCallback::OnEvent( pWnd, msg, pData );
}

void CUITaskWnd::ReloadTaskInfo()
{
	CGameTask* t					= Level().GameTaskManager().ActiveTask(eTaskTypeStoryline);
	m_pStoryLineTaskItem->InitTask	(t);

	t								= Level().GameTaskManager().ActiveTask(eTaskTypeAdditional);
	m_pSecondaryTaskItem->InitTask	(t);
	m_actual_frame					= Level().GameTaskManager().ActualFrame();
	
	u32 task2_count					= Level().GameTaskManager().GetTaskCount( eTaskStateInProgress, eTaskTypeAdditional );
	
	if ( task2_count )
	{
		u32 task2_index				= Level().GameTaskManager().GetTaskIndex( t, eTaskStateInProgress, eTaskTypeAdditional );
		string32 buf;
		sprintf_s( buf, sizeof(buf), "%d / %d", task2_index, task2_count );

		m_second_task_index->SetVisible( true );
		m_second_task_index->SetText( buf );
	}
	else
	{
		m_second_task_index->SetVisible( false );
		m_second_task_index->SetText( "" );
	}
	m_second_task_wnd->UpdateList();
}

void CUITaskWnd::Show(bool status)
{
	inherited::Show			(status);
	m_pMapWnd->Show			(status);
	m_pMapWnd->HideCurHint	();
	m_map_legend_wnd->Show	(false);
	ReloadTaskInfo();
	
	if ( status )
	{
		m_second_task_wnd->Show( m_second_task_wnd_show );
	}
	else
	{
		m_second_task_wnd_show = false;
		m_second_task_wnd->Show(false);
	}
}

void CUITaskWnd::Reset()
{
	inherited::Reset	();
}

void CUITaskWnd::OnNextTaskClicked()
{
	/*CGameTask* t = Level().GameTaskManager().IterateGet( m_pSecondaryTaskItem->OwnerTask(), eTaskStateInProgress, eTaskTypeAdditional, true);
	if ( t )
	{
		Level().GameTaskManager().SetActiveTask(t);
	}*/
}

void CUITaskWnd::OnPrevTaskClicked()
{
	/*CGameTask* t = Level().GameTaskManager().IterateGet( m_pSecondaryTaskItem->OwnerTask(), eTaskStateInProgress, eTaskTypeAdditional, false);
	if ( t )
	{
		Level().GameTaskManager().SetActiveTask(t);
	}*/
}

void CUITaskWnd::OnShowSecondTaskWnd( CUIWindow* w, void* d )
{
	m_second_task_wnd_show = false;
	m_second_task_wnd->Show( !m_second_task_wnd->IsShown() );
}

void CUITaskWnd::Show_SecondTasksWnd(bool status)
{
	m_second_task_wnd->Show( status );
	m_second_task_wnd_show = status;
}

void CUITaskWnd::TaskSetTargetMap( CGameTask* task )
{
	if ( !task )
	{
		return;
	}
	
	TaskShowMapSpot( task, true );
	CMapLocation* ml = task->LinkedMapLocation();
	if ( ml && ml->SpotEnabled() )
	{
		ml->CalcPosition();
		m_pMapWnd->SetTargetMap( ml->GetLevelName(), ml->GetPosition(), true );
	}
}

void CUITaskWnd::TaskShowMapSpot( CGameTask* task, bool show )
{
	if ( !task )
	{
		return;
	}

	CMapLocation* ml = task->LinkedMapLocation();
	if ( ml )
	{
		if ( show )
		{
			ml->EnableSpot();
			ml->CalcPosition();
			m_pMapWnd->SetTargetMap( ml->GetLevelName(), ml->GetPosition(), true );
		}
		else
		{
			ml->DisableSpot();
		}
	}
}

void CUITaskWnd::OnTask1DbClicked( CUIWindow* ui, void* d )
{
//	CUITaskItem* task_item = smart_cast<CUITaskItem*>( ui );
//	if ( task_item && task_item == m_pStoryLineTaskItem )
	{
		CGameTask* task = Level().GameTaskManager().ActiveTask( eTaskTypeStoryline );
		TaskSetTargetMap( task );
	}
}

void CUITaskWnd::OnTask2DbClicked( CUIWindow* ui, void* d )
{
//	CUITaskItem* task_item = smart_cast<CUITaskItem*>( ui );
//	if ( task_item && task_item == m_pSecondaryTaskItem )
	{
		CGameTask* task = Level().GameTaskManager().ActiveTask( eTaskTypeAdditional );
		TaskSetTargetMap( task );
	}
}

void CUITaskWnd::ShowMapLegend( bool status )
{
	m_map_legend_wnd->Show( status );
}

void CUITaskWnd::Switch_ShowMapLegend()
{
	m_map_legend_wnd->Show( !m_map_legend_wnd->IsShown() );
}

// --------------------------------------------------------------------------------------------------
CUITaskItem::CUITaskItem() :
	m_owner(NULL),
	m_hint_wt(500),
	show_hint(false),
	show_hint_can(false)
{}

CUITaskItem::~CUITaskItem()
{}

CUIStatic* init_static_field(CUIXml& uiXml, LPCSTR path, LPCSTR path2);

void CUITaskItem::Init(CUIXml& uiXml, LPCSTR path)
{
	CUIXmlInit::InitWindow			(uiXml,path,0,this);
	m_hint_wt						= uiXml.ReadAttribInt(path, 0, "hint_wt", 500);

	string256		buff;
	CUIStatic* S					= NULL;

	strconcat( sizeof(buff), buff, path, ":t_icon" );
	if ( uiXml.NavigateToNode( buff ) )
	{
		S = init_static_field		(uiXml, path, "t_icon");
		AttachChild					(S);
	}
	m_info["t_icon"]				= S;
	
	strconcat( sizeof(buff), buff, path, ":t_icon_over" );
	if ( uiXml.NavigateToNode( buff ) )
	{
		S = init_static_field		(uiXml, path, "t_icon_over");
		AttachChild					(S);
	}
	m_info["t_icon_over"]			= S;
	
	S = init_static_field			(uiXml, path, "t_caption");
	AttachChild						(S);
	m_info["t_caption"]				= S;

/*	S = init_static_field			(uiXml, path, "t_time");
	AttachChild						(S);
	m_info["t_time"]				= S;

	S = init_static_field			(uiXml, path, "t_time_rem");
	AttachChild						(S);
	m_info["t_time_rem"]			= S;
/*
	S = init_static_field			(uiXml, path, "t_hint_text");
	AttachChild						(S);
	m_info["t_hint_text"]			= S;
*/
	show_hint_can = false;
	show_hint     = false;
}

void CUITaskItem::InitTask(CGameTask* task)
{
	m_owner							= task;
	CUIStatic* S					= m_info["t_icon"];
	if ( S )
	{
		if ( task )
		{
			S->InitTexture			(task->m_icon_texture_name.c_str());
			S->SetStretchTexture	(true);
			m_info["t_icon_over"]->Show(true);
		}
		else
		{
			S->TextureOff			();
			m_info["t_icon_over"]->Show(false);
		}
	}

	S								= m_info["t_caption"];
	S->SetTextST					((task) ? task->m_Title.c_str() : "");

	/*S								= m_info["t_time"];
	xr_string	txt					="";
	if(task)
	{
		txt								+= *(InventoryUtilities::GetDateAsString(task->m_ReceiveTime, InventoryUtilities::edpDateToDay));
		txt								+= " ";
		txt								+= *(InventoryUtilities::GetTimeAsString(task->m_ReceiveTime, InventoryUtilities::etpTimeToMinutes));
	}
	S->SetText						(txt.c_str());

	S								= m_info["t_time_rem"];
	bool b_rem						= task && (task->m_ReceiveTime!=task->m_TimeToComplete);
	S->Show							(b_rem);
	if(b_rem)
	{
		string512									buff, buff2;
		InventoryUtilities::GetTimePeriodAsString	(buff, sizeof(buff), Level().GetGameTime(), task->m_TimeToComplete);
		sprintf_s									(buff2,"%s %s", *CStringTable().translate("ui_st_time_remains"), buff);
		S->SetText					(buff2);
	}*/
/*
	S								= m_info["t_hint_text"];
	S->SetTextST					(task->m_Description.c_str());
*/
}

void CUITaskItem::OnFocusReceive()
{
	inherited::OnFocusReceive();
	show_hint_can = true;
	show_hint     = false;
}

void CUITaskItem::OnFocusLost()
{
	inherited::OnFocusLost();
	show_hint_can = false;
	show_hint     = false;
}

void CUITaskItem::Update()
{
	inherited::Update();
	if ( m_owner && m_bCursorOverWindow && show_hint_can )
	{
		if ( Device.dwTimeGlobal > ( m_dwFocusReceiveTime + m_hint_wt ) )
		{
			show_hint = true;
			return;
		}
	}
	//show_hint = false;
}

void CUITaskItem::OnMouseScroll( float iDirection )
{
	/*if ( iDirection == WINDOW_MOUSE_WHEEL_UP )
	{
		GetMessageTarget()->SendMessage(this, WINDOW_MOUSE_WHEEL_UP, NULL);
		show_hint     = false;
	}
	else if ( iDirection == WINDOW_MOUSE_WHEEL_DOWN )
	{
		GetMessageTarget()->SendMessage(this, WINDOW_MOUSE_WHEEL_DOWN, NULL);
		show_hint     = false;
	}*/
}

bool CUITaskItem::OnMouse( float x, float y, EUIMessages mouse_action )
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
		show_hint_can = false;
		show_hint     = false;
		break;
	}//switch

	return true;
}

void CUITaskItem::SendMessage( CUIWindow* pWnd, s16 msg, void* pData )
{
	/*if ( pWnd == btn_focus )
	{
		if ( msg == BUTTON_DOWN )
		{
			GetMessageTarget()->SendMessage( this, PDA_TASK_SET_TARGET_MAP, (void*)m_owner );
			return;
		}
	}*/
	inherited::SendMessage( pWnd, msg, pData );
}
