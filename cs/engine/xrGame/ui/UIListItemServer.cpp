#include "StdAfx.h"

#include "UIListItemServer.h"
#include "UITextureMaster.h"

CUIListItemServer::CUIListItemServer()
{
	AttachChild					(&m_icon);
	m_icon.AttachChild			(&m_iconPass);
	m_icon.AttachChild			(&m_iconDedicated);
	m_icon.AttachChild			(&m_iconPunkBuster);
	m_icon.AttachChild			(&m_iconUserPass);

	AttachChild					(&m_server);
	AttachChild					(&m_map);
	AttachChild					(&m_game);
	AttachChild					(&m_players);
	AttachChild					(&m_ping);
	AttachChild					(&m_version);
	SetAutoDelete				(false);
}

void CUIListItemServer::InitItemServer			(LIST_SRV_ITEM& params, Fvector2 pos, Fvector2 size)
{
	CUIWindow::SetWndPos(pos);
	CUIWindow::SetWndSize(size);

	SetTextColor(params.color);
	SetFont(params.font);

	float offset = 0.0f;

	m_icon.SetWndPos(Fvector2().set(offset, 0.0f));
	m_icon.SetWndSize(Fvector2().set(params.size.icon, size.y));
	offset += params.size.icon;

	m_server.SetWndPos(Fvector2().set(offset, 0.0f));
	m_server.SetWndSize(Fvector2().set(params.size.server, size.y));
	offset += params.size.server;

	m_map.SetWndPos(Fvector2().set(offset, 0.0f));
	m_map.SetWndSize(Fvector2().set(params.size.map, size.y));
	offset += params.size.map;

	m_game.SetWndPos(Fvector2().set(offset, 0.0f));
	m_game.SetWndSize(Fvector2().set(params.size.game, size.y));
	offset += params.size.game;

	m_players.SetWndPos(Fvector2().set(offset, 0.0f));
	m_players.SetWndSize(Fvector2().set(params.size.players, size.y));
	offset += params.size.players;

	m_ping.SetWndPos(Fvector2().set(offset, 0.0f));
	m_ping.SetWndSize(Fvector2().set(params.size.ping, size.y));
	offset += params.size.ping;

	m_version.SetWndPos(Fvector2().set(offset, 0.0f));
	m_version.SetWndSize(Fvector2().set(params.size.version, size.y));

	float icon_size = CUITextureMaster::GetTextureHeight("ui_icon_password");
	m_iconPass.SetWndPos(Fvector2().set(0.0f, 0.0f));
	m_iconPass.SetWndSize(Fvector2().set(icon_size,icon_size));
	m_iconPass.InitTexture("ui_icon_password");

	m_iconDedicated.SetWndPos(Fvector2().set(icon_size, 0.0f));
	m_iconDedicated.SetWndSize(Fvector2().set(icon_size,icon_size));
	m_iconDedicated.InitTexture("ui_icon_dedicated");
	
	m_iconUserPass.SetWndPos(Fvector2().set(icon_size * 2.0f, 0.0f));
	m_iconUserPass.SetWndSize(Fvector2().set(icon_size,icon_size));
	m_iconUserPass.InitTexture("ui_icon_punkbuster"); //("ui_icon_userpass");

	SetParams(params);

	m_srv_info = params;
}
#include "../string_table.h"
u32 cut_string_by_length(CGameFont* pFont, LPCSTR src, LPSTR dst, u32 dst_size, float length);

void CUIListItemServer::SetParams(LIST_SRV_ITEM& params){
	string1024				buff;

	LPCSTR _srv_name		= CStringTable().translate(params.info.server).c_str();
	cut_string_by_length	(m_map.GetFont(), _srv_name, buff, sizeof(buff), m_server.GetWidth());
	m_server.SetText		(buff);

	LPCSTR _map_name		= CStringTable().translate(params.info.map).c_str();
	cut_string_by_length	(m_map.GetFont(), _map_name, buff, sizeof(buff), m_map.GetWidth());
	m_map.SetText			(buff);

	LPCSTR _game_name		= CStringTable().translate(params.info.game).c_str();
	cut_string_by_length	(m_game.GetFont(), _game_name, buff, sizeof(buff), m_game.GetWidth());
	m_game.SetText			(buff);

	m_players.SetTextST		(*params.info.players);
	m_ping.SetTextST		(*params.info.ping);
	m_version.SetTextST		(*params.info.version);

	m_iconPass.Show			(params.info.icons.pass);
	m_iconDedicated.Show	(params.info.icons.dedicated);
	m_iconPunkBuster.Show	(params.info.icons.punkbuster);
	m_iconUserPass.Show		(params.info.icons.user_pass);

	SetValue				(params.info.Index);
}

void CUIListItemServer::Draw()
{
	CUIStatic::Draw();
	CUIWindow::Draw();
}


void CUIListItemServer::SetTextColor(u32 color){
	//m_icon.SetTextColor(color);
	m_server.SetTextColor(color);
	m_map.SetTextColor(color);
	m_game.SetTextColor(color);
	m_players.SetTextColor(color);
	m_ping.SetTextColor(color);
	m_version.SetTextColor(color);
}

void CUIListItemServer::SetFont(CGameFont* pFont){
	m_icon.SetFont		(pFont);
	m_server.SetFont	(pFont);
	m_map.SetFont		(pFont);
	m_game.SetFont		(pFont);
	m_players.SetFont	(pFont);
	m_ping.SetFont		(pFont);
	m_version.SetFont	(pFont);
}

void CUIListItemServer::CreateConsoleCommand(xr_string& command, LPCSTR player_name, LPCSTR player_pass, LPCSTR server_psw)
{
	command = "start client(";
	command += *m_srv_info.info.address;
	command += "/name=";
	command += player_name;
	command += "/pass=";
	command += player_pass;
	command += "/psw=";
	command += server_psw;
	command += ")";
}
