#include "stdafx.h"
#include "level.h"
#include "Level_Bullet_Manager.h"
#include "xrserver.h"
#include "game_cl_base.h"
#include "xrmessages.h"
#include "xrGameSpyServer.h"
#include "../xrEngine/x_ray.h"
#include "../xrEngine/device.h"
#include "../xrEngine/IGame_Persistent.h"
#include "../xrEngine/xr_ioconsole.h"
#include "MainMenu.h"
#include "string_table.h"

extern	void	GetPlayerName_FromRegistry	(char* name, u32 const name_size);

#define DEMO_PLAY_OPT "mpdemoplay:"
#define DEMO_SAVE_KEY "-mpdemosave"

BOOL CLevel::net_Start	( LPCSTR op_server, LPCSTR op_client )
{
	net_start_result_total				= TRUE;

	pApp->LoadBegin				();

	string64	player_name;
	GetPlayerName_FromRegistry( player_name, sizeof(player_name) );

	if ( xr_strlen(player_name) == 0 )
	{
		strcpy_s( player_name, xr_strlen(Core.UserName) ? Core.UserName : Core.CompName );
	}
	VERIFY( xr_strlen(player_name) );

	//make Client Name if options doesn't have it
	LPCSTR	NameStart	= strstr(op_client,"/name=");
	if (!NameStart)
	{
		string512 tmp;
		strcpy_s(tmp, op_client);
		strcat_s(tmp, "/name=");
		strcat_s(tmp, player_name);
		m_caClientOptions			= tmp;
	} else {
		string1024	ret="";
		LPCSTR		begin	= NameStart + xr_strlen("/name="); 
		sscanf			(begin, "%[^/]",ret);
		if (!xr_strlen(ret))
		{
			string1024 tmpstr;
			strcpy_s(tmpstr, op_client);
			*(strstr(tmpstr, "name=")+5) = 0;
			strcat_s(tmpstr, player_name);
			const char* ptmp = strstr(strstr(op_client, "name="), "/");
			if (ptmp)
				strcat_s(tmpstr, ptmp);
			m_caClientOptions = tmpstr;
		}
		else
		{
			m_caClientOptions			= op_client;
		};		
	};
	m_caServerOptions			    = op_server;
	//---------------------------------------------------------------------
	const char* pdemok = NULL;
	if (op_server)
		pdemok = strstr(op_server, DEMO_PLAY_OPT);

	if (pdemok != NULL)
	{
		string_path	f_name;
		
		sscanf(pdemok + sizeof(DEMO_PLAY_OPT) - 1,
			"%[^ ] ",f_name
		);
		PrepareToPlayDemo(f_name);
		m_caServerOptions = m_demo_header.m_server_options;
	}
	else
	{
		pdemok = strstr(Core.Params, DEMO_SAVE_KEY);
		bool is_single = m_caServerOptions.size() != 0 ? 
			(strstr(m_caServerOptions.c_str(), "single") != NULL) :
			false;
		
		if ((pdemok != NULL) && !is_single)
		{
			PrepareToSaveDemo();
		}
	}
	//---------------------------------------------------------------------------
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start1));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start2));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start3));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start4));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start5));
	g_loading_events.push_back	(LOADING_EVENT(this,&CLevel::net_start6));
	
	return net_start_result_total;
}

shared_str level_version(const shared_str &server_options);
shared_str level_name(const shared_str &server_options);
bool CLevel::net_start1				()
{
	// Start client and server if need it
	if (m_caServerOptions.size())
	{
		g_pGamePersistent->LoadTitle		("st_server_starting");

		typedef IGame_Persistent::params params;
		params							&p = g_pGamePersistent->m_game_params;
		// Connect
		if (!xr_strcmp(p.m_game_type,"single"))
			Server					= xr_new<xrServer>();		
		else
			Server					= xr_new<xrGameSpyServer>();
		
		if (xr_strcmp(p.m_alife,"alife"))
		{
			shared_str l_ver			= game_sv_GameState::parse_level_version(m_caServerOptions);
			
			map_data.m_name				= game_sv_GameState::parse_level_name(m_caServerOptions);

			int							id = pApp->Level_ID(map_data.m_name.c_str(), l_ver.c_str(), true);

			if (id<0) {
				Log						("Can't find level: ",map_data.m_name.c_str());
				net_start_result_total	= FALSE;
				return true;
			}
		}
	}
	return true;
}

bool CLevel::net_start2				()
{
	if (net_start_result_total && m_caServerOptions.size())
	{
		GameDescriptionData game_descr;
		if ((m_connect_server_err=Server->Connect(m_caServerOptions, game_descr))!=xrServer::ErrNoError)
		{
			net_start_result_total = false;
			Msg				("! Failed to start server.");
//			Console->Execute("main_menu on");
			return true;
		}
		Server->SLS_Default		();
		map_data.m_name			= Server->level_name(m_caServerOptions);
	}
	return true;
}

bool CLevel::net_start3				()
{
	if(!net_start_result_total) return true;
	//add server port if don't have one in options
	if (!strstr(m_caClientOptions.c_str(), "port=") && Server)
	{
		string64	PortStr;
		sprintf_s(PortStr, "/port=%d", Server->GetPort());

		string4096	tmp;
		strcpy_s(tmp, m_caClientOptions.c_str());
		strcat_s(tmp, PortStr);
		
		m_caClientOptions = tmp;
	}
	//add password string to client, if don't have one
	if(m_caServerOptions.size()){
		if (strstr(m_caServerOptions.c_str(), "psw=") && !strstr(m_caClientOptions.c_str(), "psw="))
		{
			string64	PasswordStr = "";
			const char* PSW = strstr(m_caServerOptions.c_str(), "psw=") + 4;
			if (strchr(PSW, '/')) 
				strncpy(PasswordStr, PSW, strchr(PSW, '/') - PSW);
			else
				strcpy_s(PasswordStr, PSW);

			string4096	tmp;
			sprintf_s(tmp, "%s/psw=%s", m_caClientOptions.c_str(), PasswordStr);
			m_caClientOptions = tmp;
		};
	};
	//setting players GameSpy CDKey if it comes from command line
	if (strstr(m_caClientOptions.c_str(), "/cdkey="))
	{
		string64 CDKey;
		const char* start = strstr(m_caClientOptions.c_str(),"/cdkey=") +xr_strlen("/cdkey=");
		sscanf			(start, "%[^/]",CDKey);
		string128 cmd;
		sprintf_s(cmd, "cdkey %s", _strupr(CDKey));
		Console->Execute			(cmd);
	}
	return true;
}

bool CLevel::net_start4				()
{
	if(!net_start_result_total) return true;

	g_loading_events.pop_front();

	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client6));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client5));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client4));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client3));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client2));
	g_loading_events.push_front	(LOADING_EVENT(this,&CLevel::net_start_client1));

	return false;
}

bool CLevel::net_start5				()
{
	if (net_start_result_total)
	{
		NET_Packet		NP;
		NP.w_begin		(M_CLIENTREADY);
		Send			(NP,net_flags(TRUE,TRUE));

		if (OnClient() && Server)
		{
			Server->SLS_Clear();
		};
	};
	return true;
}
#include "hudmanager.h"
bool CLevel::net_start6				()
{
	//init bullet manager
	BulletManager().Clear		();
	BulletManager().Load		();

	pApp->LoadEnd				();

	if(net_start_result_total){
		if (strstr(Core.Params,"-$")) {
			string256				buf,cmd,param;
			sscanf					(strstr(Core.Params,"-$")+2,"%[^ ] %[^ ] ",cmd,param);
			strconcat				(sizeof(buf),buf,cmd," ",param);
			Console->Execute		(buf);
		}
	}else{
		Msg				("! Failed to start client. Check the connection or level existance.");

		if (m_connect_server_err==xrServer::ErrBELoad)
		{
			DEL_INSTANCE	(g_pGameLevel);
			Console->Execute("main_menu on");

			MainMenu()->OnLoadError("BattlEye/BEServer.dll");
		}
		else
		if (m_connect_server_err==xrServer::ErrConnect&&!psNET_direct_connect && !g_dedicated_server) 
		{
			DEL_INSTANCE	(g_pGameLevel);
			Console->Execute("main_menu on");

			MainMenu()->SwitchToMultiplayerMenu();
		}
		else
		if (!map_data.m_map_loaded && map_data.m_name.size() && m_bConnectResult)	//if (map_data.m_name == "") - level not loaded, see CLevel::net_start_client3
		{
			LPCSTR level_id_string = NULL;
			LPCSTR dialog_string = NULL;
			LPCSTR download_url = !!map_data.m_map_download_url ? map_data.m_map_download_url.c_str() : "";
			CStringTable	st;
			LPCSTR tmp_map_ver = !!map_data.m_map_version ? map_data.m_map_version.c_str() : "";
			
			STRCONCAT(level_id_string, st.translate("st_level"), ":",
				map_data.m_name.c_str(), "(", tmp_map_ver, "). ");
			STRCONCAT(dialog_string, level_id_string, st.translate("ui_st_map_not_found"));

			DEL_INSTANCE	(g_pGameLevel);
			Console->Execute("main_menu on");

			if	(!g_dedicated_server)
			{
				MainMenu()->SwitchToMultiplayerMenu();
				MainMenu()->Show_DownloadMPMap(dialog_string, download_url);
			}
		}
		else
		if (map_data.IsInvalidClientChecksum())
		{
			LPCSTR level_id_string = NULL;
			LPCSTR dialog_string = NULL;
			LPCSTR download_url = !!map_data.m_map_download_url ? map_data.m_map_download_url.c_str() : "";
			CStringTable	st;
			LPCSTR tmp_map_ver = !!map_data.m_map_version ? map_data.m_map_version.c_str() : "";

			STRCONCAT(level_id_string, st.translate("st_level"), ":",
				map_data.m_name.c_str(), "(", tmp_map_ver, "). ");
			STRCONCAT(dialog_string, level_id_string, st.translate("ui_st_map_data_corrupted"));

			g_pGameLevel->net_Stop();
			DEL_INSTANCE	(g_pGameLevel);
			Console->Execute("main_menu on");
			if	(!g_dedicated_server)
			{
				MainMenu()->SwitchToMultiplayerMenu();
				MainMenu()->Show_DownloadMPMap(dialog_string, download_url);
			}
		}
		else 
		{
			DEL_INSTANCE	(g_pGameLevel);
			Console->Execute("main_menu on");
		}

		return true;
	}

	if	(!g_dedicated_server)
	{
		if (g_hud)
			HUD().GetUI()->OnConnected();
	}

	return true;
}

void CLevel::InitializeClientGame	(NET_Packet& P)
{
	string256 game_type_name;
	P.r_stringZ(game_type_name);
	if(game && !xr_strcmp(game_type_name, game->type_name()) )
		return;
	
	xr_delete(game);
#ifdef DEBUG
	Msg("- Game configuring : Started ");
#endif // #ifdef DEBUG
	CLASS_ID clsid			= game_GameState::getCLASS_ID(game_type_name,false);
	game					= smart_cast<game_cl_GameState*> ( NEW_INSTANCE ( clsid ) );
	game->set_type_name		(game_type_name);
	game->Init				();
	m_bGameConfigStarted	= TRUE;

	R_ASSERT				(Load_GameSpecific_After ());
}

