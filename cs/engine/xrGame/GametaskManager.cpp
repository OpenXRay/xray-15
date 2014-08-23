#include "pch_script.h"
#include "GameTaskManager.h"
#include "alife_registry_wrappers.h"
#include "ui/xrUIXmlParser.h"
#include "GameTask.h"
#include "Level.h"
#include "map_manager.h"
#include "map_location.h"
#include "HUDManager.h"
#include "actor.h"
#include "UIGameSP.h"
#include "ui/UIPDAWnd.h"
#include "encyclopedia_article.h"
#include "ui/UIMapWnd.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

shared_str	g_active_task_id	[eTaskTypeCount];

struct FindTaskByID{
	shared_str	id;
	bool		b_only_inprocess;
	FindTaskByID(const shared_str& s, bool search_only_inprocess):id(s),b_only_inprocess(search_only_inprocess){}
	bool operator () (const SGameTaskKey& key)
		{
			if(b_only_inprocess)
				return (id==key.task_id && key.game_task->GetTaskState()==eTaskStateInProgress);
			else
				return (id==key.task_id);
		}
};

bool task_prio_pred(const SGameTaskKey& k1, const SGameTaskKey& k2)
{
	return k1.game_task->m_priority > k2.game_task->m_priority;
}

CGameTaskManager::CGameTaskManager()
{
	m_gametasks_wrapper			= xr_new<CGameTaskWrapper>();
	m_gametasks_wrapper->registry().init(0);// actor's id
	m_flags.zero				();
	m_flags.set					(eChanged, TRUE);
	m_gametasks					= NULL;

	for(int i=0; i<eTaskTypeCount; ++i)
		if( g_active_task_id[i].size() )
		{
			CGameTask* t = HasGameTask( g_active_task_id[i], true );
			if ( t )
			{
				VERIFY( t->GetTaskType() == (ETaskType)i );
				SetActiveTask( t );
			}
		}
}

CGameTaskManager::~CGameTaskManager()
{
	delete_data					(m_gametasks_wrapper);
	for ( int i=0; i<eTaskTypeCount; ++i )
	{
		g_active_task_id[i] = NULL;
	}	
}

vGameTasks&	CGameTaskManager::GetGameTasks	() 
{
	if(!m_gametasks)
	{
		m_gametasks = &m_gametasks_wrapper->registry().objects();
#ifdef DEBUG
		Msg("m_gametasks size=%d",m_gametasks->size());
#endif // #ifdef DEBUG
	}

	return *m_gametasks;
}

CGameTask* CGameTaskManager::HasGameTask(const shared_str& id, bool only_inprocess)
{
	FindTaskByID key(id, only_inprocess);
	vGameTasks_it it = std::find_if(GetGameTasks().begin(),GetGameTasks().end(),key);
	if( it!=GetGameTasks().end() )
		return (*it).game_task;
	
	return 0;
}

CGameTask*	CGameTaskManager::GiveGameTaskToActor(CGameTask* t, u32 timeToComplete, bool bCheckExisting, u32 timer_ttl)
{
	t->CommitScriptHelperContents	();
	if(/* bCheckExisting &&*/ HasGameTask(t->m_ID, true) ) 
	{
 		Msg("! task [%s] already inprocess",t->m_ID.c_str());
		VERIFY2( 0, make_string( "give_task : Task [%s] already inprocess!", t->m_ID.c_str()) );
		return NULL;
	}

	m_flags.set						(eChanged, TRUE);

	GetGameTasks().push_back		(SGameTaskKey(t->m_ID) );
	GetGameTasks().back().game_task	= t;
	t->m_ReceiveTime				= Level().GetGameTime();
	t->m_TimeToComplete				= t->m_ReceiveTime + timeToComplete * 1000; //ms
	t->m_timer_finish				= t->m_ReceiveTime + timer_ttl      * 1000; //ms

	std::stable_sort				(GetGameTasks().begin(), GetGameTasks().end(), task_prio_pred);

	t->OnArrived					();

	ETaskType const	task_type		= t->GetTaskType();
	CGameTask* _at					= ActiveTask(t->GetTaskType());
	//if ( t->GetTaskType() != eTaskTypeInsignificant )
	if ( task_type == eTaskTypeStoryline || task_type == eTaskTypeAdditional )
	{
		if ( (_at == NULL) || (_at->m_priority < t->m_priority) )
		{
			SetActiveTask( t );
		}
	}


	//установить флажок необходимости прочтения тасков в PDA
	if ( HUD().GetUI() )
	{
		HUD().GetUI()->UpdatePda();
	}
	if(true)
		t->ChangeStateCallback();

	return t;
}

void CGameTaskManager::SetTaskState(CGameTask* t, ETaskState state)
{
	m_flags.set						(eChanged, TRUE);

	t->SetTaskState					(state);
	
	if ( ActiveTask(t->GetTaskType()) == t )
	{
		//SetActiveTask	("", t->GetTaskType());
		g_active_task_id[ t->GetTaskType() ] = "";
	}

	if ( HUD().GetUI() )
	{
		HUD().GetUI()->UpdatePda();
	}
}

void CGameTaskManager::SetTaskState(const shared_str& id, ETaskState state)
{
	CGameTask* t				= HasGameTask(id, true);
	if (NULL==t)				{Msg("actor does not has task [%s] or it is completed", *id);	return;}
	SetTaskState				(t, state);
}

void CGameTaskManager::UpdateTasks						()
{
	Level().MapManager().DisableAllPointers();

	u32					task_count = GetGameTasks().size();
	if(0==task_count)	return;

	{
		typedef buffer_vector<SGameTaskKey>	Tasks;
		Tasks tasks				(
			_alloca(task_count*sizeof(SGameTaskKey)),
			task_count,
			GetGameTasks().begin(),
			GetGameTasks().end()
		);

		Tasks::const_iterator	I = tasks.begin();
		Tasks::const_iterator	E = tasks.end();
		for ( ; I != E; ++I) {
			CGameTask* const	t = (*I).game_task;
			if (t->GetTaskState()!=eTaskStateInProgress)
				continue;

			ETaskState const	state = t->UpdateState();

			if ( (state == eTaskStateFail) || (state == eTaskStateCompleted) )
				SetTaskState	(t, state);
		}

	}

	for(int i=0; i<eTaskTypeCount; ++i)
	{
		CGameTask		*t		= ActiveTask((ETaskType)i);
		if(t)
		{
			CMapLocation* ml = t->LinkedMapLocation();
			if(ml && !ml->PointerEnabled())
				ml->EnablePointer();
		}
	}
	if(	m_flags.test(eChanged) )
		UpdateActiveTask	();
}


void CGameTaskManager::UpdateActiveTask()
{
	std::stable_sort			(GetGameTasks().begin(), GetGameTasks().end(), task_prio_pred);

	for(u32 i=eTaskTypeStoryline; i<eTaskTypeCount; ++i)
	{
		CGameTask* _at				= ActiveTask( (ETaskType)i );
		if( NULL==_at )
		{
			CGameTask* _front		= IterateGet(NULL, eTaskStateInProgress, (ETaskType)i, true);
			if(_front)
				SetActiveTask	(_front);
		}
	}

	m_flags.set					(eChanged, FALSE);
	m_actual_frame				= Device.dwFrame;
}

CGameTask* CGameTaskManager::ActiveTask(ETaskType type)
{
	const shared_str&	t_id	= g_active_task_id[type];
	if(!t_id.size())			return NULL;
	return						HasGameTask( t_id, true );
}
/*
void CGameTaskManager::SetActiveTask(const shared_str& id, ETaskType type)
{
	g_active_task_id[type]			= id;
	m_flags.set						(eChanged, TRUE);
	m_read							= true;
}*/

void CGameTaskManager::SetActiveTask(CGameTask* task)
{
	VERIFY( task );
	if ( task )
	{
		//SetActiveTask(task->m_ID, task->GetTaskType());
		g_active_task_id[ task->GetTaskType() ] = task->m_ID;
		m_flags.set								(eChanged, TRUE);
		task->m_read							= true;
	}
}

CUIMapWnd* GetMapWnd();

void CGameTaskManager::MapLocationRelcase(CMapLocation* ml)
{
	CUIMapWnd* mwnd = GetMapWnd();
	if(mwnd)
		mwnd->MapLocationRelcase(ml);

	CGameTask* gt = HasGameTask(ml, false);
	if(gt)
		gt->RemoveMapLocations(true);
}

CGameTask* CGameTaskManager::HasGameTask(const CMapLocation* ml, bool only_inprocess)
{
	vGameTasks_it it		= GetGameTasks().begin();
	vGameTasks_it it_e		= GetGameTasks().end();

	for(; it!=it_e; ++it)
	{
		CGameTask* gt = (*it).game_task;
		if(gt->LinkedMapLocation()==ml)
		{
			if(only_inprocess && gt->GetTaskState()!=eTaskStateInProgress)
				continue;

			return gt;
		}
	}
	return NULL;
}

CGameTask* CGameTaskManager::IterateGet(CGameTask* t, ETaskState state, ETaskType type, bool bForward)
{
	vGameTasks& v		= GetGameTasks();
	u32 cnt				= v.size();
	for(u32 i=0; i<cnt; ++i)
	{
		CGameTask* gt	= v[i].game_task;
		if(gt==t || NULL==t)
		{
			bool			allow;
			if(bForward)	
			{
				if(t)		++i;
				allow		= i < cnt;
			}else
			{
				allow		= (i>0) && (--i >= 0);
			}
			if(allow)
			{
				CGameTask* found		= v[i].game_task;
				if(found->GetTaskState()==state && found->GetTaskType()==type)
					return found;
				else
					return IterateGet(found, state, type, bForward);
			}else
				return NULL;
		}
	}
	return NULL;
}

u32 CGameTaskManager::GetTaskIndex( CGameTask* t, ETaskState state, ETaskType type )
{
	if ( !t )
	{
		return 0;
	}

	vGameTasks& v	= GetGameTasks();
	u32 cnt			= v.size();
	u32 res			= 0;
	for ( u32 i = 0; i < cnt; ++i )
	{
		CGameTask* gt = v[i].game_task;
		if ( gt->GetTaskType() == type && gt->GetTaskState() == state )
		{
			++res;
			if ( gt == t )
			{
				return res;
			}
		}
	}
	return 0;
}

u32 CGameTaskManager::GetTaskCount( ETaskState state, ETaskType type )
{
	vGameTasks& v	= GetGameTasks();
	u32 cnt			= v.size();
	u32 res			= 0;
	for ( u32 i = 0; i < cnt; ++i )
	{
		CGameTask* gt = v[i].game_task;
		if ( gt->GetTaskType() == type && gt->GetTaskState()==state )
		{
			++res;
		}
	}
	return res;
}

char* sTaskStates[]=
{
	"eTaskStateFail",
	"TaskStateInProgress",
	"TaskStateCompleted",
	"TaskStateDummy"
};
char* sTaskTypes[] =
{
	"TaskTypeStoryline",
	"TaskTypeAdditional",
	"TaskTypeInsignificant",
};

void CGameTaskManager::DumpTasks()
{
	vGameTasks_it it			= GetGameTasks().begin();
	vGameTasks_it it_e			= GetGameTasks().end();
	for(; it!=it_e; ++it)
	{
		const CGameTask* gt = (*it).game_task;
		Msg("ID=[%s] type=[%s] state=[%s] prio=[%d]",	gt->m_ID.c_str(),
														sTaskTypes[gt->GetTaskType()], 
														sTaskStates[gt->GetTaskState()],
														gt->m_priority);
	}
}