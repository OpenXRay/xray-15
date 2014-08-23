#include "pch_script.h"
#include "UIGameCustom.h"
#include "ui.h"
#include "level.h"
#include "hudmanager.h"
#include "ui/UIMultiTextStatic.h"
#include "ui/UIXmlInit.h"
#include "object_broker.h"
#include "string_table.h"

#include "InventoryOwner.h"
#include "ui/UIActorMenu.h"
#include "ui/UIPdaWnd.h"

#include "../xrEngine/x_ray.h"
EGameIDs ParseStringToGameType(LPCSTR str);
struct predicate_remove_stat {
	bool	operator() (SDrawStaticStruct& s) {
		return ( !s.IsActual() );
	}
};

CUIGameCustom::CUIGameCustom()
{
	uFlags					= 0;
	shedule.t_min			= 5;
	shedule.t_max			= 20;
	shedule_register		();
	m_pgameCaptions			= xr_new<CUICaption>();
	m_msgs_xml				= xr_new<CUIXml>();
	m_msgs_xml->Load		(CONFIG_PATH, UI_PATH, "ui_custom_msgs.xml");

	m_ActorMenu		= xr_new<CUIActorMenu>		();
	m_PdaMenu		= xr_new<CUIPdaWnd>			();
}

CUIGameCustom::~CUIGameCustom()
{
	delete_data				(m_pgameCaptions);
	shedule_unregister		();
	delete_data				(m_custom_statics);
	delete_data				(m_msgs_xml);
	
	delete_data				(m_ActorMenu);	
	delete_data				(m_PdaMenu);	
}


float CUIGameCustom::shedule_Scale		() 
{
	return 0.5f;
};

void CUIGameCustom::shedule_Update		(u32 dt)
{
	inherited::shedule_Update(dt);
}

bool g_b_ClearGameCaptions = false;

void CUIGameCustom::OnFrame() 
{
	st_vec::iterator it = m_custom_statics.begin();
	for(;it!=m_custom_statics.end();++it)
		(*it).Update();

	m_custom_statics.erase(
		std::remove_if(
			m_custom_statics.begin(),
			m_custom_statics.end(),
			predicate_remove_stat()
		),
		m_custom_statics.end()
	);
	
	if(g_b_ClearGameCaptions)
	{
		delete_data				(m_custom_statics);
		g_b_ClearGameCaptions	= false;
	}
}

void CUIGameCustom::Render()
{
	GameCaptions()->Draw();
	st_vec::iterator it = m_custom_statics.begin();
	for(;it!=m_custom_statics.end();++it)
		(*it).Draw();

}

bool CUIGameCustom::IR_OnKeyboardPress(int dik) 
{
	return false;
}

bool CUIGameCustom::IR_OnKeyboardRelease(int dik) 
{
	return false;
}

bool CUIGameCustom::IR_OnMouseMove(int dx,int dy)
{
	return false;
}
bool CUIGameCustom::IR_OnMouseWheel			(int direction)
{
	return false;
}

void CUIGameCustom::AddDialogToRender(CUIWindow* pDialog)
{
	HUD().GetUI()->AddDialogToRender(pDialog);

}

void CUIGameCustom::RemoveDialogToRender(CUIWindow* pDialog)
{
	HUD().GetUI()->RemoveDialogToRender(pDialog);
}

CUIDialogWnd* CUIGameCustom::MainInputReceiver	()
{ 
	return HUD().GetUI()->MainInputReceiver();
};

void CUIGameCustom::AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color/* LPCSTR def_text*/ )
{
	GameCaptions()->addCustomMessage(id,x,y,font_size,pFont,(CGameFont::EAligment)alignment,color,"");
}

void CUIGameCustom::AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color, /*LPCSTR def_text,*/ float flicker )
{
	AddCustomMessage(id,x,y,font_size, pFont, alignment, color);
	GameCaptions()->customizeMessage(id, CUITextBanner::tbsFlicker)->fPeriod = flicker;
}

void CUIGameCustom::CustomMessageOut(LPCSTR id, LPCSTR msg, u32 color)
{
	GameCaptions()->setCaption(id,msg,color,true);
}

void CUIGameCustom::RemoveCustomMessage		(LPCSTR id)
{
	GameCaptions()->removeCustomMessage(id);
}

SDrawStaticStruct* CUIGameCustom::AddCustomStatic			(LPCSTR id, bool bSingleInstance)
{
	if(bSingleInstance){
		st_vec::iterator it = std::find(m_custom_statics.begin(),m_custom_statics.end(), id);
		if(it!=m_custom_statics.end())
			return &(*it);
	}
	
	CUIXmlInit xml_init;
	m_custom_statics.push_back		(SDrawStaticStruct());
	SDrawStaticStruct& sss			= m_custom_statics.back();

	sss.m_static					= xr_new<CUIStatic>();
	sss.m_name						= id;
	xml_init.InitStatic				(*m_msgs_xml, id, 0, sss.m_static);
	float ttl						= m_msgs_xml->ReadAttribFlt(id, 0, "ttl", -1);
	if(ttl>0.0f)
		sss.m_endTime				= Device.fTimeGlobal + ttl;

	return &sss;
}

SDrawStaticStruct* CUIGameCustom::GetCustomStatic		(LPCSTR id)
{
	st_vec::iterator it = std::find(m_custom_statics.begin(),m_custom_statics.end(), id);
	if(it!=m_custom_statics.end()){
		return &(*it);
	}
	return NULL;
}

void CUIGameCustom::RemoveCustomStatic		(LPCSTR id)
{
	st_vec::iterator it = std::find(m_custom_statics.begin(),m_custom_statics.end(), id);
	if(it!=m_custom_statics.end()){
		xr_delete((*it).m_static);
		m_custom_statics.erase(it);
	}
}

void CUIGameCustom::OnInventoryAction(PIItem item, u16 action_type)
{
	//.	if(InventoryMenu->IsShown())
	//.		InventoryMenu->InitInventory_delayed();

	if ( m_ActorMenu->IsShown() )
	{
		m_ActorMenu->OnInventoryAction( item, action_type );
	}
}

#include "ui/UIGameTutorial.h"

extern CUISequencer* g_tutorial;
extern CUISequencer* g_tutorial2;

void CUIGameCustom::reset_ui()
{
	
	if(g_tutorial2)
	{ 
		g_tutorial2->Destroy	();
		xr_delete				(g_tutorial2);
	}

	if(g_tutorial)
	{
		g_tutorial->Destroy	();
		xr_delete(g_tutorial);
	}

	m_ActorMenu->ResetAll();
	m_PdaMenu->Reset();
}

/*bool CUIGameDM::IsActorMenuShown()
{
	return m_ActorMenu->IsShown();
}
*/
bool CUIGameCustom::ShowActorMenu()
{
	if ( !MainInputReceiver() || MainInputReceiver() == m_ActorMenu )
	{
		if ( !m_ActorMenu->IsShown() )
		{
//			CInventoryOwner* pIOActor	= smart_cast<CInventoryOwner*>( Level().CurrentControlEntity() );
			CInventoryOwner* pIOActor	= smart_cast<CInventoryOwner*>( Level().CurrentViewEntity() );
			VERIFY						(pIOActor);
			m_ActorMenu->SetActor		(pIOActor);
			m_ActorMenu->SetMenuMode	(mmInventory);
		}
		HUD().GetUI()->StartStopMenu( m_ActorMenu, true );
		return true;
	}
	return false;
}

void CUIGameCustom::HideActorMenu()
{
	if ( m_ActorMenu->IsShown() )
	{
		HUD().GetUI()->StartStopMenu( m_ActorMenu, true );
	}
}

bool CUIGameCustom::ShowPdaMenu()
{
	if( !MainInputReceiver() || MainInputReceiver() == m_PdaMenu )
	{
		HUD().GetUI()->StartStopMenu( m_PdaMenu, true );
		return true;
	}
	return false;
}

void CUIGameCustom::HidePdaMenu()
{
	if ( m_PdaMenu->IsShown() )
	{
		HUD().GetUI()->StartStopMenu( m_PdaMenu, true );
	}
}

// ================================================================================================

SDrawStaticStruct::SDrawStaticStruct	()
{
	m_static	= NULL;
	m_endTime	= -1.0f;	
}

void SDrawStaticStruct::destroy()
{
	delete_data(m_static);
}

bool SDrawStaticStruct::IsActual()
{
	if(m_endTime<0) return true;
	return Device.fTimeGlobal < m_endTime;
}

void SDrawStaticStruct::Draw()
{
	if(m_static)
		m_static->Draw();
}

void SDrawStaticStruct::Update()
{
	if(!IsActual())	
		delete_data(m_static);
	else
		m_static->Update();
}

CMapListHelper	gMapListHelper;
xr_token		game_types[];

void CMapListHelper::LoadMapInfo(LPCSTR map_cfg_fn, const xr_string& map_name, LPCSTR map_ver)
{
	CInifile	ini				(map_cfg_fn);

	shared_str _map_name		= map_name.substr(0,map_name.find('\\')).c_str();
	shared_str _map_ver			= map_ver;

	if(ini.section_exist("map_usage"))
	{
		if(ini.line_exist("map_usage","ver") && !map_ver)
			_map_ver				= ini.r_string("map_usage", "ver");

		CInifile::Sect S			= ini.r_section("map_usage");
		CInifile::SectCIt si		= S.Data.begin();
		CInifile::SectCIt si_e		= S.Data.end();
		for( ;si!=si_e; ++si)
		{
			const shared_str& game_type = (*si).first;
			
			if(game_type=="ver")		continue;

			SGameTypeMaps* M			= GetMapListInt(game_type);
			if(!M)
			{
				Msg						("--unknown game type-%s",game_type.c_str());
				m_storage.resize		(m_storage.size()+1);
				SGameTypeMaps&	Itm		= m_storage.back();
				Itm.m_game_type_name	= game_type;
				Itm.m_game_type_id		= ParseStringToGameType(game_type.c_str());
				M						= &m_storage.back();
			}
			
			SGameTypeMaps::SMapItm	Itm;
			Itm.map_name				= _map_name;
			Itm.map_ver					= _map_ver;
			
			if(M->m_map_names.end()!=std::find(M->m_map_names.begin(),M->m_map_names.end(),Itm))
			{
				Msg("! duplicate map found [%s] [%s]", _map_name.c_str(), _map_ver.c_str());
			}else
			{
#ifndef MASTER_GOLD
				Msg("added map [%s] [%s]", _map_name.c_str(), _map_ver.c_str());
#endif // #ifndef MASTER_GOLD
				M->m_map_names.push_back	(Itm);
			}
		}			
	}

}

void CMapListHelper::Load()
{
//.	pApp->LoadAllArchives		();

	string_path					fn;
	FS.update_path				(fn, "$game_config$", "mp\\map_list.ltx");
	CInifile map_list_cfg		(fn);

	//read weathers set
	CInifile::Sect w			= map_list_cfg.r_section("weather");
	CInifile::SectCIt wi		= w.Data.begin();
	CInifile::SectCIt wi_e		= w.Data.end();
	for( ;wi!=wi_e; ++wi)
	{
		m_weathers.resize		(m_weathers.size()+1);
		SGameWeathers& gw		= m_weathers.back();
		gw.m_weather_name		= (*wi).first;
		gw.m_start_time			= (*wi).second;
	}

	// scan for additional maps
	FS_FileSet			fset;
	FS.file_list		(fset,"$game_levels$",FS_ListFiles,"*level.ltx");

	FS_FileSetIt fit	= fset.begin();
	FS_FileSetIt fit_e	= fset.end();

	for( ;fit!=fit_e; ++fit)
	{
		string_path					map_cfg_fn;
		FS.update_path				(map_cfg_fn, "$game_levels$", (*fit).name.c_str());
		LoadMapInfo					(map_cfg_fn, (*fit).name);
	}
	//scan all not laoded archieves
	LPCSTR tmp_entrypoint			= "temporary_gamedata\\";
	FS_Path* game_levels			= FS.get_path("$game_levels$");
	xr_string prev_root				= game_levels->m_Root;
	game_levels->_set_root			(tmp_entrypoint);

	CLocatorAPI::archives_it it		= FS.m_archives.begin();
	CLocatorAPI::archives_it it_e	= FS.m_archives.end();

	for(;it!=it_e;++it)
	{
		CLocatorAPI::archive& A		= *it;
		if(A.hSrcFile)				continue;

		LPCSTR ln					= A.header->r_string("header", "level_name");
		LPCSTR lv					= A.header->r_string("header", "level_ver");
		FS.LoadArchive				(A, tmp_entrypoint);

		string_path					map_cfg_fn;
		FS.update_path				(map_cfg_fn, "$game_levels$", ln);

		
		strcat_s					(map_cfg_fn,"\\level.ltx");
		LoadMapInfo					(map_cfg_fn, ln, lv);
		FS.unload_archive			(A);
	}
	game_levels->_set_root			(prev_root.c_str());


	R_ASSERT2	(m_storage.size(), "unable to fill map list");
	R_ASSERT2	(m_weathers.size(), "unable to fill weathers list");
}


const SGameTypeMaps& CMapListHelper::GetMapListFor(const shared_str& game_type)
{
	if( !m_storage.size() )
		Load		();

	return *GetMapListInt(game_type);
}

SGameTypeMaps* CMapListHelper::GetMapListInt(const shared_str& game_type)
{

	TSTORAGE_CIT it		= m_storage.begin();
	TSTORAGE_CIT it_e	= m_storage.end();
	for( ;it!=it_e; ++it)
	{
		if(game_type==(*it).m_game_type_name )
			return &(*it);
	}
	return NULL;
}

const SGameTypeMaps& CMapListHelper::GetMapListFor(const EGameIDs game_id)
{
	if( !m_storage.size() )
	{
		Load		();
		R_ASSERT2	(m_storage.size(), "unable to fill map list");
	}
	TSTORAGE_CIT it		= m_storage.begin();
	TSTORAGE_CIT it_e	= m_storage.end();
	for( ;it!=it_e; ++it)
	{
		if(game_id==(*it).m_game_type_id )
			return (*it);
	}
	return m_storage[0];
}

const GAME_WEATHERS& CMapListHelper::GetGameWeathers() 
{
	if(!m_weathers.size())
		Load();

	return m_weathers;
}
