#include "pch_script.h"
#include "uigamesp.h"
#include "actor.h"
#include "level.h"

#include "game_cl_Single.h"
#include "xr_level_controller.h"
#include "actorcondition.h"
#include "../xrEngine/xr_ioconsole.h"
#include "object_broker.h"
#include "GameTaskManager.h"
#include "GameTask.h"

#include "ui/UIActorMenu.h"
#include "ui/UITradeWnd.h"
#include "ui/UIPdaWnd.h"
#include "ui/UITalkWnd.h"
#include "ui/UIMessageBox.h"
#include "UIDialogHolder.h"

CUIGameSP::CUIGameSP()
{
	m_game			= NULL;
	
	TalkMenu		= xr_new<CUITalkWnd>		();
	UIChangeLevelWnd= xr_new<CChangeLevelWnd>	();
}

CUIGameSP::~CUIGameSP() 
{
	delete_data(TalkMenu);
	delete_data(UIChangeLevelWnd);
}

void CUIGameSP::shedule_Update(u32 dt)
{
	inherited::shedule_Update			(dt);
	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)							return;
	if(pActor->g_Alive())				return;

	HideShownDialogs						();
}

void CUIGameSP::HideShownDialogs()
{
	CUIDialogWnd* mir = MainInputReceiver();
	if ( mir && mir == TalkMenu )
	{
		mir->GetHolder()->StartStopMenu( mir, true );
	}

	HideActorMenu();
	HidePdaMenu();
}

void CUIGameSP::SetClGame (game_cl_GameState* g)
{
	inherited::SetClGame				(g);
	m_game = smart_cast<game_cl_Single*>(g);
	R_ASSERT							(m_game);
}


bool CUIGameSP::IR_OnKeyboardPress(int dik) 
{
	if(inherited::IR_OnKeyboardPress(dik)) return true;

	if( Device.Paused()		) return false;

	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>( Level().CurrentEntity() );
	if ( !pInvOwner )				return false;
	CEntityAlive* EA			= smart_cast<CEntityAlive*>(Level().CurrentEntity());
	if (!EA || !EA->g_Alive() )	return false;

	switch ( get_binded_action(dik) )
	{
	case kACTIVE_JOBS:
		{
			ShowPdaMenu();
			break;
		}

	case kINVENTORY:
		{
			ShowActorMenu();
			break;
		}

	case kSCORES:
		{
			CActor *pActor = smart_cast<CActor*>(pInvOwner);
			if( !pActor ) return false;
			if( !pActor->g_Alive() )	return false;


			SDrawStaticStruct* sm	= AddCustomStatic("main_task", true);
			CGameTask* t1			= Level().GameTaskManager().ActiveTask(eTaskTypeStoryline);
			CGameTask* t2			= Level().GameTaskManager().ActiveTask(eTaskTypeAdditional);
			if(t1 && t2)
			{
				sm->m_static->SetTextST		(t1->m_Title.c_str());
				SDrawStaticStruct* sm2		= AddCustomStatic("secondary_task", true);
				sm2->m_static->SetTextST	(t2->m_Title.c_str());
			}else
			if(t1 || t2)
			{
				CGameTask* t				= (t1)?t1:t2;
				sm->m_static->SetTextST		(t->m_Title.c_str());
			}else
				sm->m_static->SetTextST	("st_no_active_task");

		}break;
	}
	return false;
}

bool CUIGameSP::IR_OnKeyboardRelease(int dik) 
{
	if(inherited::IR_OnKeyboardRelease(dik)) return true;

	if( is_binded(kSCORES, dik))
	{
			RemoveCustomStatic		("main_task");
			RemoveCustomStatic		("secondary_task");
	}

	return false;
}

void  CUIGameSP::StartTrade(CInventoryOwner* pActorInv, CInventoryOwner* pOtherOwner)
{
	if( MainInputReceiver() )	return;

	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetPartner		(pOtherOwner);

	m_ActorMenu->SetMenuMode	(mmTrade);
	m_game->StartStopMenu		(m_ActorMenu,true);
}

void  CUIGameSP::StartUpgrade(CInventoryOwner* pActorInv, CInventoryOwner* pMech)
{
	if( MainInputReceiver() )	return;

	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetPartner		(pMech);

	m_ActorMenu->SetMenuMode	(mmUpgrade);
	m_game->StartStopMenu		(m_ActorMenu,true);
}

void CUIGameSP::StartTalk(bool disable_break)
{
	TalkMenu->b_disable_break = disable_break;
	m_game->StartStopMenu(TalkMenu, true);
}


void CUIGameSP::StartCarBody(CInventoryOwner* pActorInv, CInventoryOwner* pOtherOwner) //Deadbody search
{
	if( MainInputReceiver() )		return;

	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetPartner		(pOtherOwner);

	m_ActorMenu->SetMenuMode	(mmDeadBodySearch);
	m_game->StartStopMenu		(m_ActorMenu,true);
}

void CUIGameSP::StartCarBody(CInventoryOwner* pActorInv, CInventoryBox* pBox) //Deadbody search
{
	if( MainInputReceiver() )		return;
	
	m_ActorMenu->SetActor		(pActorInv);
	m_ActorMenu->SetInvBox		(pBox);
	VERIFY( pBox );

	m_ActorMenu->SetMenuMode	(mmDeadBodySearch);
	m_game->StartStopMenu		(m_ActorMenu,true);
}


extern ENGINE_API BOOL bShowPauseString;
void CUIGameSP::ChangeLevel(	GameGraph::_GRAPH_ID game_vert_id, 
								u32 level_vert_id, 
								Fvector pos, 
								Fvector ang, 
								Fvector pos2, 
								Fvector ang2, 
								bool b_use_position_cancel,
								const shared_str& message_str,
								bool b_allow_change_level)
{
	if( !MainInputReceiver() || MainInputReceiver()!=UIChangeLevelWnd)
	{
		UIChangeLevelWnd->m_game_vertex_id		= game_vert_id;
		UIChangeLevelWnd->m_level_vertex_id		= level_vert_id;
		UIChangeLevelWnd->m_position			= pos;
		UIChangeLevelWnd->m_angles				= ang;
		UIChangeLevelWnd->m_position_cancel		= pos2;
		UIChangeLevelWnd->m_angles_cancel		= ang2;
		UIChangeLevelWnd->m_b_position_cancel	= b_use_position_cancel;
		UIChangeLevelWnd->m_b_allow_change_level=b_allow_change_level;
		UIChangeLevelWnd->m_message_str			= message_str;

		m_game->StartStopMenu					(UIChangeLevelWnd,true);
	}
}

void CUIGameSP::reset_ui()
{
	inherited::reset_ui				();
	TalkMenu->Reset					();
	UIChangeLevelWnd->Reset			();
}

CChangeLevelWnd::CChangeLevelWnd		()
{
	m_messageBox			= xr_new<CUIMessageBox>();	
	m_messageBox->SetAutoDelete(true);
	AttachChild				(m_messageBox);
}

void CChangeLevelWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd==m_messageBox){
		if(msg==MESSAGE_BOX_YES_CLICKED){
			OnOk									();
		}else
		if(msg==MESSAGE_BOX_NO_CLICKED || msg==MESSAGE_BOX_OK_CLICKED)
		{
			OnCancel								();
		}
	}else
		inherited::SendMessage(pWnd, msg, pData);
}

void CChangeLevelWnd::OnOk()
{
	Game().StartStopMenu					(this, true);
	NET_Packet								p;
	p.w_begin								(M_CHANGE_LEVEL);
	p.w										(&m_game_vertex_id,sizeof(m_game_vertex_id));
	p.w										(&m_level_vertex_id,sizeof(m_level_vertex_id));
	p.w_vec3								(m_position);
	p.w_vec3								(m_angles);

	Level().Send							(p,net_flags(TRUE));
}

void CChangeLevelWnd::OnCancel()
{
	Game().StartStopMenu					(this, true);
	if(m_b_position_cancel){
		Actor()->MoveActor(m_position_cancel, m_angles_cancel);
	}
}

bool CChangeLevelWnd::OnKeyboard(int dik, EUIMessages keyboard_action)
{
	if(keyboard_action==WINDOW_KEY_PRESSED)
	{
		if(is_binded(kQUIT, dik) )
			OnCancel		();
		return true;
	}
	return inherited::OnKeyboard(dik, keyboard_action);
}

bool g_block_pause	= false;
void CChangeLevelWnd::Show()
{
	m_messageBox->InitMessageBox(m_b_allow_change_level?"message_box_change_level":"message_box_change_level_disabled");
	SetWndPos				(m_messageBox->GetWndPos());
	m_messageBox->SetWndPos	(Fvector2().set(0.0f,0.0f));
	SetWndSize				(m_messageBox->GetWndSize());

	m_messageBox->SetText	(m_message_str.c_str());
	

	g_block_pause							= true;
	Device.Pause							(TRUE, TRUE, TRUE, "CChangeLevelWnd_show");
	bShowPauseString						= FALSE;
}

void CChangeLevelWnd::Hide()
{
	g_block_pause							= false;
	Device.Pause							(FALSE, TRUE, TRUE, "CChangeLevelWnd_hide");
}

