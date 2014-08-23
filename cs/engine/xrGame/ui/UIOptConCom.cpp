#include "StdAfx.h"
#include "../../xrEngine/xr_ioconsole.h"
#include "../../xrEngine/xr_ioc_cmd.h"
#include "UIOptConCom.h"
#include "../../xrcore/xrCore.h"
//#include "game_base_space.h"
#include "gametype_chooser.h"
#include "../RegistryFuncs.h"
#include "../../xrGameSpy/xrGameSpy_MainDefs.h"
#include "../battleye.h"
#include "player_name_modifyer.h"

extern	void	GetCDKey_FromRegistry		(char* cdkey);
extern	void	WriteCDKey_ToRegistry		(LPSTR cdkey);
extern	void	GetPlayerName_FromRegistry	(char* name, u32 const name_size);
extern	void	WritePlayerName_ToRegistry	(LPSTR name);

xr_token g_GameModes	[] = {
	{ "st_deathmatch",				eGameIDDeathmatch			},
	{ "st_team_deathmatch",			eGameIDTeamDeathmatch		},
	{ "st_artefacthunt",			eGameIDArtefactHunt			},
	{ "st_capture_the_artefact",	eGameIDCaptureTheArtefact	},
	{ 0,							0							}
};

CUIOptConCom::CUIOptConCom()
{
	strcpy_s(m_playerName, "");
}

class CCC_UserName: public CCC_String{
public:
	CCC_UserName(LPCSTR N, LPSTR V, int _size) : CCC_String(N, V, _size)  { bEmptyArgsHandled = false; };	
	virtual void Execute(LPCSTR arguments)
	{
		string512 str;
		strcpy_s(str, arguments);
		if(xr_strlen(str)>17)
			str[17] = 0;

		CCC_String::Execute(str);
		string256	new_name;
		modify_player_name(value, new_name);

		WritePlayerName_ToRegistry( new_name );
	}
	virtual void	Save	(IWriter *F)	{};
};


void CUIOptConCom::Init()
{
	ReadPlayerNameFromRegistry();
	CMD3(CCC_UserName,	"mm_net_player_name", m_playerName,	64);

	m_iMaxPlayers		= 32;
	m_curGameMode		= eGameIDDeathmatch;
	CMD4(CCC_Integer,	"mm_net_srv_maxplayers",			&m_iMaxPlayers,	2, 32);
	CMD3(CCC_Token,		"mm_net_srv_gamemode",				&m_curGameMode,	g_GameModes);
	m_uNetSrvParams.zero();
	CMD3(CCC_Mask,		"mm_mm_net_srv_dedicated",			&m_uNetSrvParams,	flNetSrvDedicated);
	CMD3(CCC_Mask,		"mm_net_con_publicserver",			&m_uNetSrvParams,	flNetConPublicServer);
	CMD3(CCC_Mask,		"mm_net_con_spectator_on",			&m_uNetSrvParams,	flNetConSpectatorOn);
#ifdef BATTLEYE
	CMD3(CCC_Mask,		"mm_net_use_battleye",				&m_uNetSrvParams,	flNetConBattlEyeOn);
//-	CMD3(CCC_Mask,		"mm_net_battleye_auto_update",		&m_uNetSrvParams,	flNetConBattlEyeAutoUpdate);
#endif // BATTLEYE

	m_iNetConSpectator	= 20;
	CMD4(CCC_Integer,	"mm_net_con_spectator",				&m_iNetConSpectator, 1, 32);

	m_iReinforcementType = 1;
	CMD4(CCC_Integer,	"mm_net_srv_reinforcement_type",	&m_iReinforcementType, 0, 2 );

	//strcpy_s			(m_sReinforcementType,"reinforcement");
	//CMD3(CCC_String,	"mm_net_srv_reinforcement_type",	m_sReinforcementType, sizeof(m_sReinforcementType));
	
	m_fNetWeatherRate = 1.0f;
	CMD4(CCC_Float,		"mm_net_weather_rateofchange",		&m_fNetWeatherRate,	0.0, 100.0f);

	strcpy_s(m_serverName, Core.CompName);
	CMD3(CCC_String,	"mm_net_srv_name",					m_serverName,	sizeof(m_serverName));

	m_uNetFilter.one	();
	CMD3(CCC_Mask,		"mm_net_filter_empty",				&m_uNetFilter,		fl_empty);
	CMD3(CCC_Mask,		"mm_net_filter_full",				&m_uNetFilter,		fl_full);
	CMD3(CCC_Mask,		"mm_net_filter_pass",				&m_uNetFilter,		fl_pass);
	CMD3(CCC_Mask,		"mm_net_filter_wo_pass",			&m_uNetFilter,		fl_wo_pass);
	CMD3(CCC_Mask,		"mm_net_filter_wo_ff",				&m_uNetFilter,		fl_wo_ff);
	CMD3(CCC_Mask,		"mm_net_filter_listen",				&m_uNetFilter,		fl_listen);

#ifdef BATTLEYE
	CMD3(CCC_Mask,		"mm_net_filter_battleye",			&m_uNetFilter,		fl_battleye);
#endif // BATTLEYE

};

void		CUIOptConCom::ReadPlayerNameFromRegistry	()
{
	GetPlayerName_FromRegistry( m_playerName, sizeof(m_playerName) );
};

void		CUIOptConCom::WritePlayerNameToRegistry		()
{
	string256 new_name;
	modify_player_name(m_playerName, new_name);
	WritePlayerName_ToRegistry( new_name );
};
