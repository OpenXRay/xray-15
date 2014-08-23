////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_ALife.cpp
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Oles Shyshkovtsov, Alexander Maksimchuk, Victor Reutskiy and Dmitriy Iassenev
//	Description : Server objects for ALife simulator
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "game_base_space.h"
#include "object_broker.h"
#include "restriction_space.h"

#ifndef AI_COMPILER
#	include "character_info.h"
#endif // AI_COMPILER

#ifndef XRGAME_EXPORTS
#	include "bone.h"
#	include "defines.h"
	LPCSTR GAME_CONFIG = "game.ltx";
#else // XRGAME_EXPORTS
#	include "../xrEngine/bone.h"
#	include "../xrEngine/render.h"
#endif // XRGAME_EXPORTS

#ifdef XRSE_FACTORY_EXPORTS
#	include "ai_space.h"
#	include "script_engine.h"
#	pragma warning(push)
#	pragma warning(disable:4995)
#		include <luabind/luabind.hpp>
#		include <shlwapi.h>
#	pragma warning(pop)

#pragma comment(lib, "shlwapi.lib")

struct logical_string_predicate {
	static HRESULT AnsiToUnicode						(LPCSTR pszA, LPVOID buffer, u32 const& buffer_size)
	{
		VERIFY			(pszA);
		VERIFY			(buffer);
		VERIFY			(buffer_size);

		u32				cCharacters =  xr_strlen(pszA)+1;
		VERIFY			(cCharacters*2 <= buffer_size);

		if (MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters, (LPOLESTR)buffer, cCharacters))
			return		(NOERROR);

		return			(HRESULT_FROM_WIN32(GetLastError()));
	}

	bool operator()			(LPCSTR const& first, LPCSTR const& second) const
	{
		u32				buffer_size0 = (xr_strlen(first) + 1)*2;
		LPCWSTR			buffer0 = (LPCWSTR)_alloca(buffer_size0);
		AnsiToUnicode	(first, (LPVOID)buffer0, buffer_size0);

		u32				buffer_size1 = (xr_strlen(second) + 1)*2;
		LPCWSTR			buffer1 = (LPCWSTR)_alloca(buffer_size1);
		AnsiToUnicode	(second, (LPVOID)buffer1, buffer_size1);

		return			(StrCmpLogicalW(buffer0, buffer1) < 0);
	}

	bool operator()			(shared_str const& first, shared_str const& second) const
	{
		u32				buffer_size0 = (first.size() + 1)*2;
		LPCWSTR			buffer0 = (LPCWSTR)_alloca(buffer_size0);
		AnsiToUnicode	(first.c_str(), (LPVOID)buffer0, buffer_size0);

		u32				buffer_size1 = (second.size() + 1)*2;
		LPCWSTR			buffer1 = (LPCWSTR)_alloca(buffer_size1);
		AnsiToUnicode	(second.c_str(), (LPVOID)buffer1, buffer_size1);

		return			(StrCmpLogicalW(buffer0, buffer1) < 0);
	}
}; // struct logical_string_predicate

#endif // XRSE_FACTORY_EXPORTS

bool SortStringsByAlphabetPred (const shared_str& s1, const shared_str& s2)
{
	R_ASSERT(s1.size());
	R_ASSERT(s2.size());

	return (xr_strcmp(s1,s2)<0);
};

struct story_name_predicate {
	IC	bool	operator()	(const xr_rtoken &_1, const xr_rtoken &_2) const
	{
		VERIFY	(_1.name.size());
		VERIFY	(_2.name.size());

		return	(xr_strcmp(_1.name,_2.name) < 0);
	}
};

#ifdef XRSE_FACTORY_EXPORTS
struct SFillPropData{
    RTokenVec 	locations[4];
    RStringVec	level_ids;
	RTokenVec 	story_names;
	RTokenVec 	spawn_story_names;
	RStringVec	character_profiles;
    RStringVec	smart_covers;

    u32			counter;
                SFillPropData	()
    {
        counter = 0;
    }
                ~SFillPropData	()
    {
    	VERIFY	(0==counter);
    }
    void		load			()
    {
        // create ini
#ifdef XRGAME_EXPORTS
        CInifile				*Ini = 	pGameIni;
#else // XRGAME_EXPORTS
        CInifile				*Ini = 0;
        string_path				gm_name;
        FS.update_path			(gm_name,"$game_config$",GAME_CONFIG);
        R_ASSERT3				(FS.exist(gm_name),"Couldn't find file",gm_name);
        Ini						= xr_new<CInifile>(gm_name);
#endif // XRGAME_EXPORTS

        // location type
        LPCSTR					N,V;
        u32 					k;
		for (int i=0; i<GameGraph::LOCATION_TYPE_COUNT; ++i){
            VERIFY				(locations[i].empty());
            string256			caSection, T;
            strconcat			(sizeof(caSection),caSection,SECTION_HEADER,itoa(i,T,10));
            R_ASSERT			(Ini->section_exist(caSection));
            for (k = 0; Ini->r_line(caSection,k,&N,&V); ++k)
                locations[i].push_back	(xr_rtoken(V,atoi(N)));
        }
        
		// level names/ids
        VERIFY					(level_ids.empty());
        for (k = 0; Ini->r_line("levels",k,&N,&V); ++k)
            level_ids.push_back	(Ini->r_string_wb(N,"caption"));

        // story names
		{
			VERIFY					(story_names.empty());
			LPCSTR section 			= "story_ids";
			R_ASSERT				(Ini->section_exist(section));
			for (k = 0; Ini->r_line(section,k,&N,&V); ++k)
				story_names.push_back	(xr_rtoken(V,atoi(N)));

			std::sort				(story_names.begin(),story_names.end(),story_name_predicate());
			story_names.insert		(story_names.begin(),xr_rtoken("NO STORY ID",ALife::_STORY_ID(-1)));
		}

        // spawn story names
		{
			VERIFY					(spawn_story_names.empty());
			LPCSTR section 			= "spawn_story_ids";
			R_ASSERT				(Ini->section_exist(section));
			for (k = 0; Ini->r_line(section,k,&N,&V); ++k)
				spawn_story_names.push_back	(xr_rtoken(V,atoi(N)));

			std::sort				(spawn_story_names.begin(),spawn_story_names.end(),story_name_predicate());
			spawn_story_names.insert(spawn_story_names.begin(),xr_rtoken("NO SPAWN STORY ID",ALife::_SPAWN_STORY_ID(-1)));
		}

#ifndef AI_COMPILER
		//character profiles indexes
		VERIFY					(character_profiles.empty());
		for(int i = 0; i<=CCharacterInfo::GetMaxIndex(); i++)
		{
			character_profiles.push_back(CCharacterInfo::IndexToId(i));
		}

		std::sort(character_profiles.begin(), character_profiles.end(), SortStringsByAlphabetPred);
#endif // AI_COMPILER
		
        // destroy ini
#ifndef XRGAME_EXPORTS
		xr_delete				(Ini);
#endif // XRGAME_EXPORTS

		luabind::object			table;
		R_ASSERT				(ai().script_engine().function_object("smart_covers.descriptions", table, LUA_TTABLE));
		luabind::object::iterator	I = table.begin();
		luabind::object::iterator	E = table.end();
		for ( ; I != E; ++I)
			smart_covers.push_back	(luabind::object_cast<LPCSTR>(I.key()));

		std::sort				(smart_covers.begin(), smart_covers.end(), logical_string_predicate());
    }

    void		unload			()
    {
        for (int i=0; i<GameGraph::LOCATION_TYPE_COUNT; ++i)
            locations[i].clear	();
        level_ids.clear			();
        story_names.clear		();
        spawn_story_names.clear	();
		character_profiles.clear();
		smart_covers.clear		();
    }

    void 		dec				()
    {
        VERIFY					(counter > 0);
        --counter;

        if (!counter)
            unload				();
    }

    void 		inc				()
    {
        VERIFY					(counter < 0xffffffff);

        if (!counter)
            load				();

        ++counter;
    }
};
static SFillPropData			fp_data;
#endif // #ifdef XRSE_FACTORY_EXPORTS

#ifndef XRGAME_EXPORTS
void CSE_ALifeTraderAbstract::FillProps	(LPCSTR pref, PropItemVec& items)
{
#	ifdef XRSE_FACTORY_EXPORTS
	PHelper().CreateU32			(items, PrepareKey(pref,*base()->s_name,"Money"), 	&m_dwMoney,	0, u32(-1));
	PHelper().CreateFlag32		(items,	PrepareKey(pref,*base()->s_name,"Trader\\Infinite ammo"),&m_trader_flags, eTraderFlagInfiniteAmmo);
	RListValue *value		= PHelper().CreateRList	(items,	PrepareKey(pref,*base()->s_name,"npc profile"),	 
		&m_sCharacterProfile, 
		&*fp_data.character_profiles.begin(), fp_data.character_profiles.size());
	
	value->OnChangeEvent.bind	(this,&CSE_ALifeTraderAbstract::OnChangeProfile);
#	endif // #ifdef XRSE_FACTORY_EXPORTS
}
#endif // #ifndef XRGAME_EXPORTS
////////////////////////////////////////////////////////////////////////////
// CSE_ALifeGraphPoint
////////////////////////////////////////////////////////////////////////////
CSE_ALifeGraphPoint::CSE_ALifeGraphPoint	(LPCSTR caSection) : CSE_Abstract(caSection)
{
//.	s_gameid					= GAME_DUMMY;
	m_tLocations[0]				= 0;
	m_tLocations[1]				= 0;
	m_tLocations[2]				= 0;
	m_tLocations[3]				= 0;

#ifdef XRSE_FACTORY_EXPORTS
	fp_data.inc					();
#endif // XRSE_FACTORY_EXPORTS
}

CSE_ALifeGraphPoint::~CSE_ALifeGraphPoint	()
{
#ifdef XRSE_FACTORY_EXPORTS
    fp_data.dec					();
#endif // XRSE_FACTORY_EXPORTS
}

void CSE_ALifeGraphPoint::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	tNetPacket.r_stringZ		(m_caConnectionPointName);
	if (m_wVersion < 33)
		tNetPacket.r_u32		();
	else
		tNetPacket.r_stringZ	(m_caConnectionLevelName);
	tNetPacket.r_u8				(m_tLocations[0]);
	tNetPacket.r_u8				(m_tLocations[1]);
	tNetPacket.r_u8				(m_tLocations[2]);
	tNetPacket.r_u8				(m_tLocations[3]);
};

void CSE_ALifeGraphPoint::STATE_Write		(NET_Packet	&tNetPacket)
{
	tNetPacket.w_stringZ		(m_caConnectionPointName);
	tNetPacket.w_stringZ		(m_caConnectionLevelName);
	tNetPacket.w_u8				(m_tLocations[0]);
	tNetPacket.w_u8				(m_tLocations[1]);
	tNetPacket.w_u8				(m_tLocations[2]);
	tNetPacket.w_u8				(m_tLocations[3]);
};
void CSE_ALifeGraphPoint::UPDATE_Read		(NET_Packet	&tNetPacket)
{
}

void CSE_ALifeGraphPoint::UPDATE_Write		(NET_Packet	&tNetPacket)
{
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeGraphPoint::FillProps			(LPCSTR pref, PropItemVec& items)
{
#	ifdef XRSE_FACTORY_EXPORTS
	PHelper().CreateRToken8		(items,	PrepareKey(pref,*s_name,"Location\\1"),				&m_tLocations[0],			&*fp_data.locations[0].begin(), fp_data.locations[0].size());
	PHelper().CreateRToken8		(items,	PrepareKey(pref,*s_name,"Location\\2"),				&m_tLocations[1],			&*fp_data.locations[1].begin(), fp_data.locations[1].size());
	PHelper().CreateRToken8		(items,	PrepareKey(pref,*s_name,"Location\\3"),				&m_tLocations[2],			&*fp_data.locations[2].begin(), fp_data.locations[2].size());
	PHelper().CreateRToken8		(items,	PrepareKey(pref,*s_name,"Location\\4"),				&m_tLocations[3],			&*fp_data.locations[3].begin(), fp_data.locations[3].size());
	PHelper().CreateRList	 	(items,	PrepareKey(pref,*s_name,"Connection\\Level name"),	&m_caConnectionLevelName,	&*fp_data.level_ids.begin(),	fp_data.level_ids.size());
	PHelper().CreateRText	 	(items,	PrepareKey(pref,*s_name,"Connection\\Point name"),	&m_caConnectionPointName);
#	endif // #ifdef XRSE_FACTORY_EXPORTS
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_SmartCover
////////////////////////////////////////////////////////////////////////////
#ifdef XRSE_FACTORY_EXPORTS
bool parse_bool		(luabind::object const &table, LPCSTR identifier)
{
	VERIFY2						(table.type() == LUA_TTABLE, "invalid loophole description passed");
	luabind::object				result = table[identifier];
	VERIFY2						(result.type() != LUA_TNIL, make_string("cannot read boolean value %s", identifier));
	VERIFY2						(result.type() == LUA_TBOOLEAN, make_string("cannot read boolean value %s", identifier));
	return						(luabind::object_cast<bool>(result));
}

BOOL is_combat_cover			(shared_str const &table_id)
{
	if (table_id.size() == 0)
		return					(FALSE);

	string256					temp;
	strcpy_s					(temp, "smart_covers.descriptions.");
	strcat_s					(temp, *table_id);

	luabind::object				table, value;
	bool						result = 
		ai().script_engine().function_object(
			temp,
			table,
			LUA_TTABLE
		);

	VERIFY2						(result, make_string("bad or missing description in smart_cover [%s]", table_id.c_str()));
	if (table.type() != LUA_TTABLE) {
		VERIFY					(table.type() != LUA_TNIL);
		return					(TRUE);
	}

	value						= table["is_combat_cover"];
	if (value.type() == LUA_TNIL) {
		Msg						("! is_combat_cover flag not found for smart_cover [%s], forcing to \"true\"", table_id.c_str());
		return					(TRUE);
	}

	return						(parse_bool(table, "is_combat_cover") ? TRUE : FALSE);
}
#endif // XRSE_FACTORY_EXPORTS

CSE_SmartCover::CSE_SmartCover	(LPCSTR section) : CSE_ALifeDynamicObject(section)
{
#ifdef XRSE_FACTORY_EXPORTS
	fp_data.inc					();
#endif // XRSE_FACTORY_EXPORTS

	m_enter_min_enemy_distance	= pSettings->r_float(section, "enter_min_enemy_distance");
	m_exit_min_enemy_distance	= pSettings->r_float(section, "exit_min_enemy_distance");
	m_is_combat_cover			= pSettings->r_bool(section, "is_combat_cover");
}

CSE_SmartCover::~CSE_SmartCover	()
{
#ifdef XRSE_FACTORY_EXPORTS
	fp_data.dec					();
#endif // XRSE_FACTORY_EXPORTS
}

ISE_Shape* CSE_SmartCover::shape()
{
	return						(this);
}

bool CSE_SmartCover::used_ai_locations	() const
{
	return						(true);
}

bool CSE_SmartCover::can_save			() const
{
	return						(true);
}

bool CSE_SmartCover::can_switch_online	() const
{
	return						(true);
}

bool CSE_SmartCover::can_switch_offline	() const
{
	return						(false);
}

bool CSE_SmartCover::interactive		() const
{
	return						(false);
}

void CSE_SmartCover::STATE_Read	(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket, size);
	cform_read					(tNetPacket);
	tNetPacket.r_stringZ		(m_description);
	m_hold_position_time		= tNetPacket.r_float();
	if (m_wVersion >= 120) {
		m_enter_min_enemy_distance	= tNetPacket.r_float();
		m_exit_min_enemy_distance	= tNetPacket.r_float();
	}
	if (m_wVersion >= 122)
		m_is_combat_cover		= tNetPacket.r_u8();
}

void CSE_SmartCover::STATE_Write(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	cform_write					(tNetPacket);
	tNetPacket.w_stringZ		(m_description);
	tNetPacket.w_float			(m_hold_position_time);
	tNetPacket.w_float			(m_enter_min_enemy_distance);
	tNetPacket.w_float			(m_exit_min_enemy_distance);
	tNetPacket.w_u8				((u8)m_is_combat_cover);
}

void CSE_SmartCover::UPDATE_Read(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
}

void CSE_SmartCover::UPDATE_Write(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_SmartCover::FillProps	(LPCSTR pref, PropItemVec& items)
{
#	ifdef XRSE_FACTORY_EXPORTS
	PHelper().CreateFloat		(items, PrepareKey(pref,*s_name,"hold position time"), 	&m_hold_position_time,	0.f, 60.f);
	RListValue *value			= PHelper().CreateRList	 	(items,	PrepareKey(pref,*s_name,"description"),			&m_description,			&*fp_data.smart_covers.begin(),	fp_data.smart_covers.size());
	value->OnChangeEvent.bind	(this,&CSE_SmartCover::OnChangeDescription);
 
	PHelper().CreateFloat		(items, PrepareKey(pref,*s_name,"enter min enemy distance"),&m_enter_min_enemy_distance,	0.f, 100.f);
	PHelper().CreateFloat		(items, PrepareKey(pref,*s_name,"exit min enemy distance"),	&m_exit_min_enemy_distance,		0.f, 100.f);

	if (is_combat_cover(m_description))
		PHelper().CreateBOOL	(items, PrepareKey(pref, *s_name, "is combat cover"), &m_is_combat_cover);
#	endif // #ifdef XRSE_FACTORY_EXPORTS
}
#endif // #ifndef XRGAME_EXPORTS

#ifdef XRSE_FACTORY_EXPORTS
void CSE_SmartCover::OnChangeDescription(PropValue* sender)
{
	set_editor_flag	(flVisualChange);
	load_draw_data	();
}

LPCSTR parse_string(luabind::object const &table, LPCSTR identifier)
{
	VERIFY2			(table.type() == LUA_TTABLE, "invalid loophole description passed");
	luabind::object	result = table[identifier];
	VERIFY2			(result.type() != LUA_TNIL, make_string("cannot read string value %s", identifier));
	VERIFY2			(result.type() == LUA_TSTRING, make_string("cannot read string value %s", identifier));
	return			(luabind::object_cast<LPCSTR>(result));
}

Fvector parse_fvector (luabind::object const &table, LPCSTR identifier)
{
	VERIFY2			(table.type() == LUA_TTABLE, "invalid loophole description passed");
	luabind::object	result = table[identifier];
	VERIFY2			(result.type() != LUA_TNIL, make_string("cannot read vector value %s", identifier));
	return			(luabind::object_cast<Fvector>(result));
}

float parse_float	(
		luabind::object const &table,
		LPCSTR identifier,
		float const &min_threshold = flt_min,
		float const &max_threshold = flt_max
	)
{
	VERIFY2			(table.type() == LUA_TTABLE, "invalid loophole description passed");
	luabind::object	lua_result = table[identifier];
	VERIFY2			(lua_result.type() != LUA_TNIL, make_string("cannot read number value %s", identifier));
	VERIFY2			(lua_result.type() == LUA_TNUMBER, make_string("cannot read number value %s", identifier));
	float			result = luabind::object_cast<float>(lua_result);
	VERIFY2			(result >= min_threshold, make_string("invalid read number value %s", identifier));
	VERIFY2			(result <= max_threshold, make_string("invalid number value %s", identifier));
	return			(result);
}

void parse_table	(luabind::object const &table, LPCSTR identifier, luabind::object &result)
{
	VERIFY2			(table.type() == LUA_TTABLE, "invalid loophole description passed");
	result			= table[identifier];
	VERIFY2			(result.type() != LUA_TNIL, make_string("cannot read table value %s", identifier));
	VERIFY2			(result.type() == LUA_TTABLE, make_string("cannot read table value %s", identifier));
}

namespace smart_cover {
	static	LPCSTR				s_enter_loophole_id = "<__ENTER__>";
	static	LPCSTR				s_exit_loophole_id  = "<__EXIT__>";

	shared_str	transform_vertex(shared_str const &vertex_id, bool const &in)
	{
		if (*vertex_id.c_str())
			return				(vertex_id);

		if (in)
			return				(s_enter_loophole_id);

		return					(s_exit_loophole_id);
	}
	
	shared_str	parse_vertex	(luabind::object const &table, LPCSTR identifier, bool const &in)
	{
		return					(
			transform_vertex(
				parse_string(
					table,
					identifier
				),
				in
			)
		);
	}
} // namespace smart_cover

void CSE_SmartCover::set_enterable (shared_str const &id)
{
	class id_predicate {
		shared_str				m_id;

	public:
		IC	id_predicate		(shared_str const &id) :
			m_id				(id)
		{
		}

		IC	bool	operator()	(SSCDrawHelper const &h) const
		{
			return				(m_id._get() == h.string_identifier._get());
		}
	};

	xr_vector<SSCDrawHelper>::iterator	found = std::find_if(
									m_draw_data.begin(),
									m_draw_data.end(),
									id_predicate(id)
								);
	//VERIFY2						(found != m_draw_data.end(), id.c_str());
	if (found == m_draw_data.end())
		return;

	found->is_enterable			= true;
}

void CSE_SmartCover::check_enterable_loopholes(shared_str const &description)
{
	string256					temp;
	strcpy_s					(temp, "smart_covers.descriptions.");
	strcat_s					(temp, m_description.c_str());
	strcat_s					(temp, ".transitions");

	luabind::object				transitions;
	bool						result = 
		ai().script_engine().function_object(
			temp,
			transitions,
			LUA_TTABLE
		);
	VERIFY2						(result, make_string("bad or missing transitions table in smart_cover [%s]", temp));
	
	luabind::object::iterator	I = transitions.begin();
	luabind::object::iterator	E = transitions.end();
	for ( ; I != E; ++I) {
		luabind::object			table = *I;
		if (table.type() != LUA_TTABLE) {
			VERIFY	(table.type() != LUA_TNIL);
			continue;
		}

		shared_str				vertex_0 = smart_cover::parse_vertex(table, "vertex0", true);
		if (vertex_0 != smart_cover::transform_vertex("", true))
			continue;

		set_enterable			(smart_cover::parse_vertex(table, "vertex1", false));
	}
}

class CSE_SmartVisual : public CSE_Visual {
public:
	virtual CSE_Visual* __stdcall visual ()
	{
		return					(this);
	}
}; // class CSE_SmartVisual

void CSE_SmartCover::fill_visuals()
{
	delete_data(m_visuals);

	xr_vector<SSCDrawHelper>::iterator I = m_draw_data.begin();
	xr_vector<SSCDrawHelper>::iterator E = m_draw_data.end();
	for ( ; I != E; ++I) {
		if (!I->is_enterable)
			return;

		CSE_Visual *visual			= xr_new<CSE_SmartVisual>();
		visual->set_visual			("actors\\stalker_neutral\\stalker_neutral_1");

		if (I->animation_id.size() == 0) {
			Msg						("cover [%s] doesn't have idle_2_fire animation", I->string_identifier.c_str());
			return;
		}

		visual->startup_animation	= I->animation_id;

		visual_data					tmp;
		tmp.visual					= visual;
		tmp.matrix.rotation			(I->enter_direction, Fvector().set(0.f, 1.f, 0.f));
		tmp.matrix.c				= I->point_position;
		
		m_visuals.push_back			(tmp);
	}
}

void draw_frustum	(CDUInterface* du, float FOV, float _FAR, float A, Fvector &P, Fvector &D, Fvector &U, u32 const &CL)
{
	float YFov	= deg2rad(FOV*A);
	float XFov	= deg2rad(FOV);

	// calc window extents in camera coords
	float wR=tanf(XFov*0.5f);
	float wL=-wR;
	float wT=tanf(YFov*0.5f);
	float wB=-wT;

	// calc x-axis (viewhoriz) and store cop
	// here we are assuring that vectors are perpendicular & normalized
	Fvector			R,COP;
	D.normalize		();
	R.crossproduct	(D,U);
	R.normalize		();
	U.crossproduct	(R,D);
	U.normalize		();
	COP.set			(P);

	// calculate the corner vertices of the window
	Fvector			sPts[4];  // silhouette points (corners of window)
	Fvector			Offset,T;
	Offset.add		(D,COP);

	sPts[0].mul(R,wR);	T.mad(Offset,U,wT);	sPts[0].add(T);
	sPts[1].mul(R,wL);	T.mad(Offset,U,wT);	sPts[1].add(T);
	sPts[2].mul(R,wL);	T.mad(Offset,U,wB);	sPts[2].add(T);
	sPts[3].mul(R,wR);	T.mad(Offset,U,wB);	sPts[3].add(T);

	// find projector direction vectors (from cop through silhouette pts)
	Fvector ProjDirs[4];
	ProjDirs[0].sub(sPts[0],COP);
	ProjDirs[1].sub(sPts[1],COP);
	ProjDirs[2].sub(sPts[2],COP);
	ProjDirs[3].sub(sPts[3],COP);

	Fvector _F[4];
	_F[0].mad(COP, ProjDirs[0], _FAR); 
	_F[1].mad(COP, ProjDirs[1], _FAR); 
	_F[2].mad(COP, ProjDirs[2], _FAR); 
	_F[3].mad(COP, ProjDirs[3], _FAR); 

	du->DrawLine(COP,_F[0],CL);
	du->DrawLine(COP,_F[1],CL);
	du->DrawLine(COP,_F[2],CL);
	du->DrawLine(COP,_F[3],CL);

	du->DrawLine(_F[0],_F[1],CL);
	du->DrawLine(_F[1],_F[2],CL);
	du->DrawLine(_F[2],_F[3],CL);
	du->DrawLine(_F[3],_F[0],CL);
}

shared_str animation_id(luabind::object table)
{
	luabind::object::iterator i = table.begin();
	luabind::object::iterator e = table.end();
	for ( ; i != e; ++i ) {
		luabind::object	string = *i;
		if (string.type() != LUA_TSTRING) {
			VERIFY	(string.type() != LUA_TNIL);
			continue;
		}

		return		(luabind::object_cast<LPCSTR>(string));
	}

	return			("");
}

void CSE_SmartCover::load_draw_data () {
	string256					temp;
	strcpy_s					(temp, "smart_covers.descriptions.");
	strcat_s					(temp, m_description.c_str());
	strcat_s					(temp, ".loopholes");
	
	m_draw_data.clear			();

	luabind::object				loopholes;
	bool						result = 
		ai().script_engine().function_object(
			temp,
			loopholes,
			LUA_TTABLE
		);

	if (!result) {
		Msg						("no or invalid smart cover description (bad or missing loopholes table in smart_cover [%s])", temp);
		return;
//		VERIFY2					(result, make_string("bad or missing loopholes table in smart_cover [%s]", temp));
	}

	luabind::object::iterator	I = loopholes.begin();
	luabind::object::iterator	E = loopholes.end();
	for ( ; I != E; ++I) {
		luabind::object			table = *I;
		if (table.type() != LUA_TTABLE) {
			VERIFY	(table.type() != LUA_TNIL);
			continue;
		}
		m_draw_data.resize		(m_draw_data.size()+1);
		SSCDrawHelper& H		= m_draw_data.back();

		H.string_identifier		= parse_string(table,"id");
		H.point_position		= parse_fvector(table, "fov_position");
		H.is_enterable			= false;
		H.fov_direction			= parse_fvector(table, "fov_direction");

		if (H.fov_direction.square_magnitude() < EPS_L) {
			Msg				("! fov direction for loophole %s is setup incorrectly", H.string_identifier.c_str());
			H.fov_direction.set(0.f, 0.f, 1.f);
		}
		else
			H.fov_direction.normalize	();

		H.enter_direction		= parse_fvector(table, "enter_direction");

		if (H.enter_direction.square_magnitude() < EPS_L) {
			Msg				("! enter direction for loophole %s is setup incorrectly", H.string_identifier.c_str());
			H.enter_direction.set(0.f, 0.f, 1.f);
		}
		else
			H.enter_direction.normalize	();

		H.fov					= parse_float(table, "fov", 0.f, 360.f);
		H.range					= parse_float(table, "range", 0.f);

		luabind::object	transitions;
		parse_table		(table, "transitions", transitions);
		luabind::object::iterator I = transitions.begin();
		luabind::object::iterator E = transitions.end();
		for ( ; I != E; ++I) {
			luabind::object transition = *I;
			VERIFY2				(transition.type() == LUA_TTABLE, "invalid loophole description passed");
			shared_str			action_from = smart_cover::parse_vertex(transition, "action_from", true);
			if (action_from != "idle")
				continue;
			shared_str			action_to = smart_cover::parse_vertex(transition, "action_to", false);
			if (action_to != "fire")
				continue;
			
			luabind::object		result;
			parse_table			(transition, "animations", result);

			H.animation_id		= animation_id(result);
			break;
		}
	}

	check_enterable_loopholes	(m_description);
	fill_visuals				();
}

void CSE_SmartCover::on_render	(CDUInterface* du, ISE_AbstractLEOwner* owner, bool bSelected, const Fmatrix& parent,int priority, bool strictB2F)
{
	inherited1::on_render	(du, owner, bSelected, parent, priority, strictB2F);
	if ( !((1==priority)&&(false==strictB2F)) )	
		return;

	if( (m_draw_data.empty() && m_description.size()) )
	{
		OnChangeDescription		(NULL);
	}

	if (!bSelected)
		return;

	xr_vector<SSCDrawHelper>::iterator it = m_draw_data.begin();
	xr_vector<SSCDrawHelper>::iterator it_e = m_draw_data.end();

	for( ; it!=it_e; ++it){
		SSCDrawHelper& H = *it;
		Fvector pos = H.point_position;
		parent.transform_tiny	(pos);

		du->OutText(pos, H.string_identifier.c_str(), color_rgba(255,255,255,255));

		//du->DrawBox(H.point_position,Fvector().set(0.2f,0.2f,0.2f),TRUE,TRUE,color_rgba(255,0,0,80),color_rgba(0,255,0,255));
		//du->DrawFlag(H.point_position, 0, 1.0f, 1, 1, color_rgba(0,255,0,255), FALSE);
		//du->DrawCylinder(Fidentity, pos, Fvector().set(0.f, 1.f, 0.f), 1.f, .05f, color_rgba(0,255,0,255), color_rgba(0,255,0,255), TRUE, FALSE);
		
		Fvector dir = H.fov_direction;
		parent.transform_dir(dir);
		Fvector up = parent.j;
		draw_frustum(du, H.fov, H.range, 1.f, pos, dir, up, color_rgba(255,0,0,255));
	}
}
#endif // #ifdef XRSE_FACTORY_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeObject
////////////////////////////////////////////////////////////////////////////
CSE_ALifeObject::CSE_ALifeObject			(LPCSTR caSection) : CSE_Abstract(caSection)
{
	m_bOnline					= false;
	m_fDistance					= 0.0f;
	ID							= ALife::_OBJECT_ID(-1);
	m_tGraphID					= GameGraph::_GRAPH_ID(-1);
	m_tSpawnID					= ALife::_SPAWN_ID(-1);
	m_bDirectControl			= true;
	m_bALifeControl				= true;
	m_tNodeID					= u32(-1);
	m_flags.one					();
	m_story_id					= INVALID_STORY_ID;
	m_spawn_story_id			= INVALID_SPAWN_STORY_ID;
#ifdef XRGAME_EXPORTS
	m_alife_simulator			= 0;
#endif
#ifdef XRSE_FACTORY_EXPORTS
    fp_data.inc					();
#endif // XRSE_FACTORY_EXPORTS
	m_flags.set					(flOfflineNoMove,FALSE);
	seed						(u32(CPU::QPC() & 0xffffffff));
}

#ifdef XRGAME_EXPORTS
CALifeSimulator	&CSE_ALifeObject::alife	() const
{
	VERIFY						(m_alife_simulator);
	return						(*m_alife_simulator);
}

Fvector CSE_ALifeObject::draw_level_position	() const
{
	return						(Position());
}
#endif

CSE_ALifeObject::~CSE_ALifeObject			()
{
#ifdef XRSE_FACTORY_EXPORTS
    fp_data.dec					();
#endif // XRSE_FACTORY_EXPORTS
}

bool CSE_ALifeObject::move_offline			() const
{
	return						(!m_flags.test(flOfflineNoMove));
}

void CSE_ALifeObject::move_offline			(bool value)
{
	m_flags.set					(flOfflineNoMove,!value ? TRUE : FALSE);
}

bool CSE_ALifeObject::visible_for_map		() const
{
	return						(!!m_flags.test(flVisibleForMap));
}

void CSE_ALifeObject::visible_for_map		(bool value)
{
	m_flags.set					(flVisibleForMap,value ? TRUE : FALSE);
}

void CSE_ALifeObject::STATE_Write			(NET_Packet &tNetPacket)
{
	tNetPacket.w_u16			(m_tGraphID);
	tNetPacket.w_float			(m_fDistance);
	tNetPacket.w_u32			(m_bDirectControl);
	tNetPacket.w_u32			(m_tNodeID);
	tNetPacket.w_u32			(m_flags.get());
	tNetPacket.w_stringZ		(m_ini_string);
	tNetPacket.w_u32			(m_story_id);
	tNetPacket.w_u32			(m_spawn_story_id);
}

void CSE_ALifeObject::STATE_Read			(NET_Packet &tNetPacket, u16 size)
{
	if (m_wVersion >= 1) {
		if (m_wVersion > 24) {
			if (m_wVersion < 83) {
				tNetPacket.r_float	();//m_spawn_probability);
			}
		}
		else {
			tNetPacket.r_u8		();
			/**
			u8					l_ucTemp;
			tNetPacket.r_u8		(l_ucTemp);
			m_spawn_probability	= (float)l_ucTemp;
			/**/
		}
		if (m_wVersion < 83) {
			tNetPacket.r_u32	();
		}
		if (m_wVersion < 4) {
			u16					wDummy;
			tNetPacket.r_u16	(wDummy);
		}
		tNetPacket.r_u16		(m_tGraphID);
		tNetPacket.r_float		(m_fDistance);
	}
	if (m_wVersion >= 4) {
		u32						dwDummy;
		tNetPacket.r_u32		(dwDummy);
		m_bDirectControl		= !!dwDummy;
	}
	
	if (m_wVersion >= 8)
		tNetPacket.r_u32		(m_tNodeID);
	
	if ((m_wVersion > 22) && (m_wVersion <= 79))
		tNetPacket.r_u16		(m_tSpawnID);
	
	if ((m_wVersion > 23) && (m_wVersion < 84)) {
		shared_str				temp;
		tNetPacket.r_stringZ	(temp);//m_spawn_control);
	}

	if (m_wVersion > 49) {
		tNetPacket.r_u32		(m_flags.flags);
	}
	
	if (m_wVersion > 57) {
		if (m_ini_file)
			xr_delete			(m_ini_file);
		tNetPacket.r_stringZ	(m_ini_string);
	}

	if (m_wVersion > 61)
		tNetPacket.r_u32		(m_story_id);

	if (m_wVersion > 111)
		tNetPacket.r_u32		(m_spawn_story_id);
}

void CSE_ALifeObject::UPDATE_Write			(NET_Packet &tNetPacket)
{
}

void CSE_ALifeObject::UPDATE_Read			(NET_Packet &tNetPacket)
{
};

#ifndef XRGAME_EXPORTS
void CSE_ALifeObject::FillProps				(LPCSTR pref, PropItemVec& items)
{
#	ifdef XRSE_FACTORY_EXPORTS
	inherited::FillProps		(pref, 	items);
	PHelper().CreateRText		(items,	PrepareKey(pref,*s_name,"Custom data"),&m_ini_string);
	if (m_flags.is(flUseSwitches)) {
		PHelper().CreateFlag32	(items,	PrepareKey(pref,*s_name,"ALife\\Can switch online"),	&m_flags,			flSwitchOnline);
		PHelper().CreateFlag32	(items,	PrepareKey(pref,*s_name,"ALife\\Can switch offline"),	&m_flags,			flSwitchOffline);
	}                            
	PHelper().CreateFlag32		(items,	PrepareKey(pref,*s_name,"ALife\\Interactive"),			&m_flags,			flInteractive);
	PHelper().CreateFlag32		(items,	PrepareKey(pref,*s_name,"ALife\\Used AI locations"),	&m_flags,			flUsedAI_Locations);
	PHelper().CreateRToken32	(items,	PrepareKey(pref,*s_name,"ALife\\Story ID"),				&m_story_id,		&*fp_data.story_names.begin(), fp_data.story_names.size());
	PHelper().CreateRToken32	(items,	PrepareKey(pref,*s_name,"ALife\\Spawn Story ID"),		&m_spawn_story_id,	&*fp_data.spawn_story_names.begin(), fp_data.spawn_story_names.size());
#	endif // #ifdef XRSE_FACTORY_EXPORTS
}
#endif // #ifndef XRGAME_EXPORTS

u32	CSE_ALifeObject::ef_equipment_type		() const
{
	string16					temp; CLSID2TEXT(m_tClassID,temp);
	R_ASSERT3	(false,"Invalid alife equipment type request, virtual function is not properly overloaded!",temp);
	return		(u32(-1));
//	return		(6);
}

u32	 CSE_ALifeObject::ef_main_weapon_type	() const
{
	string16					temp; CLSID2TEXT(m_tClassID,temp);
	R_ASSERT3	(false,"Invalid alife main weapon type request, virtual function is not properly overloaded!",temp);
	return		(u32(-1));
//	return		(5);
}

u32	 CSE_ALifeObject::ef_weapon_type		() const
{
//	string16					temp; CLSID2TEXT(m_tClassID,temp);
//	R_ASSERT3	(false,"Invalid alife weapon type request, virtual function is not properly overloaded!",temp);
//	return		(u32(-1));
	return		(0);
}

u32	CSE_ALifeObject::ef_detector_type		() const
{
	string16					temp; CLSID2TEXT(m_tClassID,temp);
	R_ASSERT3	(false,"Invalid alife detector type request, virtual function is not properly overloaded!",temp);
	return		(u32(-1));
}

bool CSE_ALifeObject::used_ai_locations		() const
{
	return						(!!m_flags.is(flUsedAI_Locations));
}

bool CSE_ALifeObject::can_switch_online		() const
{
	return						(match_configuration() && !!m_flags.is(flSwitchOnline));
}

bool CSE_ALifeObject::can_switch_offline	() const
{
	return						(!match_configuration() || !!m_flags.is(flSwitchOffline));
}

bool CSE_ALifeObject::can_save				() const
{
	return						(!!m_flags.is(flCanSave));
}

bool CSE_ALifeObject::interactive			() const
{
	return						(
		!!m_flags.is(flInteractive) &&
		!!m_flags.is(flVisibleForAI) &&
		!!m_flags.is(flUsefulForAI)
	);
}

void CSE_ALifeObject::can_switch_online		(bool value)
{
	m_flags.set					(flSwitchOnline,BOOL(value));
}

void CSE_ALifeObject::can_switch_offline	(bool value)
{
	m_flags.set					(flSwitchOffline,BOOL(value));
}

void CSE_ALifeObject::interactive			(bool value)
{
	m_flags.set					(flInteractive,BOOL(value));
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeGroupAbstract
////////////////////////////////////////////////////////////////////////////
CSE_ALifeGroupAbstract::CSE_ALifeGroupAbstract(LPCSTR caSection)
{
	m_tpMembers.clear			();
	m_bCreateSpawnPositions		= true;
	m_wCount					= 1;
	m_tNextBirthTime			= 0;
}

CSE_Abstract *CSE_ALifeGroupAbstract::init	()
{
	return						(base());
}

CSE_ALifeGroupAbstract::~CSE_ALifeGroupAbstract()
{
}

void CSE_ALifeGroupAbstract::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	u16 m_wVersion = base()->m_wVersion;
	u32							dwDummy;
	tNetPacket.r_u32			(dwDummy);
	m_bCreateSpawnPositions		= !!dwDummy;
	tNetPacket.r_u16			(m_wCount);
	if (m_wVersion > 19)
		load_data				(m_tpMembers,tNetPacket);
};

void CSE_ALifeGroupAbstract::STATE_Write	(NET_Packet	&tNetPacket)
{
	tNetPacket.w_u32			(m_bCreateSpawnPositions);
	tNetPacket.w_u16			(m_wCount);
	save_data					(m_tpMembers,tNetPacket);
};

void CSE_ALifeGroupAbstract::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	u32							dwDummy;
	tNetPacket.r_u32			(dwDummy);
	m_bCreateSpawnPositions		= !!dwDummy;
};

void CSE_ALifeGroupAbstract::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	tNetPacket.w_u32			(m_bCreateSpawnPositions);
};

#ifndef XRGAME_EXPORTS
void CSE_ALifeGroupAbstract::FillProps		(LPCSTR pref, PropItemVec& items)
{
	PHelper().CreateU16			(items,	PrepareKey(pref, "ALife\\Count"),			&m_wCount,			0,0xff);
};	
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeDynamicObject
////////////////////////////////////////////////////////////////////////////

CSE_ALifeDynamicObject::CSE_ALifeDynamicObject(LPCSTR caSection) : CSE_ALifeObject(caSection)
{
	m_tTimeID					= 0;
	m_switch_counter			= u64(-1);
}

CSE_ALifeDynamicObject::~CSE_ALifeDynamicObject()
{
}

void CSE_ALifeDynamicObject::STATE_Write	(NET_Packet &tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeDynamicObject::STATE_Read		(NET_Packet &tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket, size);
}

void CSE_ALifeDynamicObject::UPDATE_Write	(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
};

void CSE_ALifeDynamicObject::UPDATE_Read	(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
};

#ifndef XRGAME_EXPORTS
void CSE_ALifeDynamicObject::FillProps	(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,values);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeDynamicObjectVisual
////////////////////////////////////////////////////////////////////////////
CSE_ALifeDynamicObjectVisual::CSE_ALifeDynamicObjectVisual(LPCSTR caSection) : CSE_ALifeDynamicObject(caSection), CSE_Visual()
{
	if (pSettings->line_exist(caSection,"visual"))
		set_visual				(pSettings->r_string(caSection,"visual"));
}

CSE_ALifeDynamicObjectVisual::~CSE_ALifeDynamicObjectVisual()
{
}

CSE_Visual* CSE_ALifeDynamicObjectVisual::visual	()
{
	return						(this);
}

void CSE_ALifeDynamicObjectVisual::STATE_Write(NET_Packet &tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	visual_write				(tNetPacket);
}

void CSE_ALifeDynamicObjectVisual::STATE_Read(NET_Packet &tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket, size);
	if (m_wVersion > 31)
		visual_read				(tNetPacket,m_wVersion);
}

void CSE_ALifeDynamicObjectVisual::UPDATE_Write(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
};

void CSE_ALifeDynamicObjectVisual::UPDATE_Read(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
};

#ifndef XRGAME_EXPORTS
void CSE_ALifeDynamicObjectVisual::FillProps	(LPCSTR pref, PropItemVec& items)
{
	inherited1::FillProps		(pref,items);
	inherited2::FillProps		(pref,items);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifePHSkeletonObject
////////////////////////////////////////////////////////////////////////////
CSE_ALifePHSkeletonObject::CSE_ALifePHSkeletonObject(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection),CSE_PHSkeleton(caSection)
{
	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);
}

CSE_ALifePHSkeletonObject::~CSE_ALifePHSkeletonObject()
{

}


void CSE_ALifePHSkeletonObject::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read(tNetPacket,size);
	if (m_wVersion>=64)
		inherited2::STATE_Read(tNetPacket,size);

}

void CSE_ALifePHSkeletonObject::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
}


void CSE_ALifePHSkeletonObject::load			(NET_Packet &tNetPacket)
{
	inherited1::load				(tNetPacket);
	inherited2::load				(tNetPacket);
}
void CSE_ALifePHSkeletonObject::UPDATE_Write(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	inherited2::UPDATE_Write	(tNetPacket);
};

void CSE_ALifePHSkeletonObject::UPDATE_Read(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
};

bool CSE_ALifePHSkeletonObject::can_save			() const
{
	return						CSE_PHSkeleton::need_save();
}

bool CSE_ALifePHSkeletonObject::used_ai_locations () const
{
	return false;
}

#ifndef XRGAME_EXPORTS
void CSE_ALifePHSkeletonObject::FillProps(LPCSTR pref, PropItemVec& items)
{
	inherited1::FillProps			(pref,items);
	inherited2::FillProps			(pref,items);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeSpaceRestrictor
////////////////////////////////////////////////////////////////////////////
CSE_ALifeSpaceRestrictor::CSE_ALifeSpaceRestrictor	(LPCSTR caSection) : CSE_ALifeDynamicObject(caSection)
{
	m_flags.set					(flUseSwitches,FALSE);
	m_space_restrictor_type		= RestrictionSpace::eDefaultRestrictorTypeNone;
	m_flags.set					(flUsedAI_Locations,FALSE);
	m_spawn_flags.set			(flSpawnDestroyOnSpawn,FALSE);
	m_flags.set					(flCheckForSeparator,TRUE);
}

CSE_ALifeSpaceRestrictor::~CSE_ALifeSpaceRestrictor	()
{
}

bool CSE_ALifeSpaceRestrictor::can_switch_offline	() const
{
	return						(false);
}

bool CSE_ALifeSpaceRestrictor::used_ai_locations	() const
{
	return						(false);
}

ISE_Shape* CSE_ALifeSpaceRestrictor::shape		()
{
	return						(this);
}

void CSE_ALifeSpaceRestrictor::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket,size);
	cform_read					(tNetPacket);
	if (m_wVersion > 74)
		m_space_restrictor_type = tNetPacket.r_u8();
}

void CSE_ALifeSpaceRestrictor::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	cform_write					(tNetPacket);
	tNetPacket.w_u8				(m_space_restrictor_type);
}

void CSE_ALifeSpaceRestrictor::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeSpaceRestrictor::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
}

xr_token defaul_retrictor_types[]={
	{ "NOT A restrictor",			RestrictionSpace::eRestrictorTypeNone},
	{ "NONE default restrictor",	RestrictionSpace::eDefaultRestrictorTypeNone},
	{ "OUT default restrictor",		RestrictionSpace::eDefaultRestrictorTypeOut	},
	{ "IN default restrictor",		RestrictionSpace::eDefaultRestrictorTypeIn	},
	{ 0,							0}
};

#ifndef XRGAME_EXPORTS
void CSE_ALifeSpaceRestrictor::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited1::FillProps		(pref,items);
	PHelper().CreateToken8		(items, PrepareKey(pref,*s_name,"restrictor type"),		&m_space_restrictor_type,	defaul_retrictor_types);
	PHelper().CreateFlag32		(items,	PrepareKey(pref,*s_name,"check for separator"),	&m_flags,					flCheckForSeparator);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeLevelChanger
////////////////////////////////////////////////////////////////////////////
CSE_ALifeLevelChanger::CSE_ALifeLevelChanger(LPCSTR caSection) : CSE_ALifeSpaceRestrictor(caSection)
{
	m_tNextGraphID				= GameGraph::_GRAPH_ID(-1);
	m_dwNextNodeID				= u32(-1);
	m_tNextPosition.set			(0.f,0.f,0.f);
	m_tAngles.set				(0.f,0.f,0.f);
#ifdef XRSE_FACTORY_EXPORTS
    fp_data.inc					();
#endif // XRSE_FACTORY_EXPORTS
	m_bSilentMode				= FALSE;
}

CSE_ALifeLevelChanger::~CSE_ALifeLevelChanger()\
{
#ifdef XRSE_FACTORY_EXPORTS
    fp_data.dec					();
#endif // XRSE_FACTORY_EXPORTS
}

void CSE_ALifeLevelChanger::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
	if (m_wVersion < 34) {
		tNetPacket.r_u32		();
		tNetPacket.r_u32		();
	}
	else {
		tNetPacket.r_u16		(m_tNextGraphID);
		tNetPacket.r_u32		(m_dwNextNodeID);
		tNetPacket.r_float		(m_tNextPosition.x);
		tNetPacket.r_float		(m_tNextPosition.y);
		tNetPacket.r_float		(m_tNextPosition.z);
		if (m_wVersion <= 53)
			m_tAngles.set		(0.f,tNetPacket.r_float(),0.f);
		else
			tNetPacket.r_vec3	(m_tAngles);
	}
	tNetPacket.r_stringZ		(m_caLevelToChange);
	tNetPacket.r_stringZ		(m_caLevelPointToChange);

	if (m_wVersion > 116)
		m_bSilentMode			= !!tNetPacket.r_u8();

}

void CSE_ALifeLevelChanger::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_u16			(m_tNextGraphID);
	tNetPacket.w_u32			(m_dwNextNodeID);
	tNetPacket.w_float			(m_tNextPosition.x);
	tNetPacket.w_float			(m_tNextPosition.y);
	tNetPacket.w_float			(m_tNextPosition.z);
	tNetPacket.w_vec3			(m_tAngles);
	tNetPacket.w_stringZ		(m_caLevelToChange);
	tNetPacket.w_stringZ		(m_caLevelPointToChange);
	tNetPacket.w_u8				(m_bSilentMode?1:0);
}

void CSE_ALifeLevelChanger::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeLevelChanger::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeLevelChanger::FillProps		(LPCSTR pref, PropItemVec& items)
{
#	ifdef XRSE_FACTORY_EXPORTS
	inherited::FillProps		(pref,items);
	
	PHelper().CreateRList		(items,PrepareKey(pref,*s_name,"Level to change"),		&m_caLevelToChange,		&*fp_data.level_ids.begin(), fp_data.level_ids.size());
	PHelper().CreateRText		(items,PrepareKey(pref,*s_name,"Level point to change"),	&m_caLevelPointToChange);

	PHelper().CreateBOOL		(items,PrepareKey(pref,*s_name,"Silent mode"),	&m_bSilentMode);
#	endif // #ifdef XRSE_FACTORY_EXPORTS
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeObjectPhysic
////////////////////////////////////////////////////////////////////////////
CSE_ALifeObjectPhysic::CSE_ALifeObjectPhysic(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection), CSE_PHSkeleton(caSection)
{
	type 						= epotSkeleton;
	mass 						= 10.f;

	if (pSettings->section_exist(caSection) && pSettings->line_exist(caSection,"visual"))
	{
    	set_visual				(pSettings->r_string(caSection,"visual"));
		
		if(pSettings->line_exist(caSection,"startup_animation"))
			startup_animation		= pSettings->r_string(caSection,"startup_animation");
	}
	
	if(pSettings->line_exist(caSection,"fixed_bones"))
		fixed_bones		= pSettings->r_string(caSection,"fixed_bones");

	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);
	m_flags.set					(flUsedAI_Locations,FALSE);
	
#ifdef XRGAME_EXPORTS
	m_freeze_time				= Device.dwTimeGlobal;
#	ifdef DEBUG
	m_last_update_time			= Device.dwTimeGlobal;
#	endif
#else
	m_freeze_time				= 0;
#endif
	m_relevent_random.seed		(u32(CPU::GetCLK() & u32(-1)));
}

CSE_ALifeObjectPhysic::~CSE_ALifeObjectPhysic		() 
{
}

void CSE_ALifeObjectPhysic::STATE_Read		(NET_Packet	&tNetPacket, u16 size) 
{
	if (m_wVersion >= 14)
		if (m_wVersion >= 16) {
			inherited1::STATE_Read(tNetPacket,size);
			if (m_wVersion < 32)
				visual_read		(tNetPacket,m_wVersion);
		}
		else {
			CSE_ALifeObject::STATE_Read(tNetPacket,size);
			visual_read			(tNetPacket,m_wVersion);
		}

	if (m_wVersion>=64) inherited2::STATE_Read(tNetPacket,size);
		
	tNetPacket.r_u32			(type);
	tNetPacket.r_float			(mass);
    
	if (m_wVersion > 9)
		tNetPacket.r_stringZ	(fixed_bones);

	if (m_wVersion<65&&m_wVersion > 28)
		tNetPacket.r_stringZ	(startup_animation);

	if(m_wVersion<64)
		{
		if	(m_wVersion > 39)		// > 39 		
			tNetPacket.r_u8			(_flags.flags);

		if (m_wVersion>56)
			tNetPacket.r_u16		(source_id);

		if (m_wVersion>60	&&	_flags.test(flSavedData)) {
			data_load(tNetPacket);
		}
	}
	set_editor_flag				(flVisualAnimationChange);
}

void CSE_ALifeObjectPhysic::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
	tNetPacket.w_u32			(type);
	tNetPacket.w_float			(mass);
	tNetPacket.w_stringZ			(fixed_bones);

}

static inline bool check (const u8 &mask, const u8 &test)
{
	return							(!!(mask & test));
}

const	u32		CSE_ALifeObjectPhysic::m_freeze_delta_time		= 5000;
const	u32		CSE_ALifeObjectPhysic::random_limit				= 40;		

#ifdef DEBUG
const	u32		CSE_ALifeObjectPhysic::m_update_delta_time		= 0;
#endif // #ifdef DEBUG

//if TRUE, then object sends update packet
BOOL CSE_ALifeObjectPhysic::Net_Relevant()
{
	if (!freezed)
	{
#ifdef XRGAME_EXPORTS
#ifdef DEBUG	//this block of code is only for test
		if (Device.dwTimeGlobal < (m_last_update_time + m_update_delta_time))
			return FALSE;
#endif
#endif
		return		TRUE;
	}

#ifdef XRGAME_EXPORTS
	if (Device.dwTimeGlobal >= (m_freeze_time + m_freeze_delta_time))
		return		FALSE;
#endif
	if (!prev_freezed)
	{
		prev_freezed = true;	//i.e. freezed
		return		TRUE;
	}

	if (m_relevent_random.randI(random_limit))
		return		FALSE;

	return			TRUE;
}

void CSE_ALifeObjectPhysic::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);

	if (tNetPacket.r_eof())		//backward compatibility
		return;

//////////////////////////////////////////////////////////////////////////
	tNetPacket.r_u8					(m_u8NumItems);
	if (!m_u8NumItems) {
		return;
	}

	mask_num_items					num_items;
	num_items.common				= m_u8NumItems;
	m_u8NumItems					= num_items.num_items;

	R_ASSERT2						(
		m_u8NumItems < (u8(1) << 5),
		make_string("%d",m_u8NumItems)
		);
	
	/*if (check(num_items.mask,animated))
	{
		tNetPacket.r_float(m_blend_timeCurrent);
		anim_use=true;
	}
	else
	{
	anim_use=false;
	}*/

	{
		tNetPacket.r_vec3				(State.force);
		tNetPacket.r_vec3				(State.torque);

		tNetPacket.r_vec3				(State.position);

		tNetPacket.r_float			(State.quaternion.x);
		tNetPacket.r_float			(State.quaternion.y);
		tNetPacket.r_float			(State.quaternion.z);
		tNetPacket.r_float			(State.quaternion.w);	

		State.enabled					= check(num_items.mask,inventory_item_state_enabled);

		if (!check(num_items.mask,inventory_item_angular_null)) {
			tNetPacket.r_float		(State.angular_vel.x);
			tNetPacket.r_float		(State.angular_vel.y);
			tNetPacket.r_float		(State.angular_vel.z);
		}
		else
			State.angular_vel.set		(0.f,0.f,0.f);

		if (!check(num_items.mask,inventory_item_linear_null)) {
			tNetPacket.r_float		(State.linear_vel.x);
			tNetPacket.r_float		(State.linear_vel.y);
			tNetPacket.r_float		(State.linear_vel.z);
		}
		else
			State.linear_vel.set		(0.f,0.f,0.f);

		/*if (check(num_items.mask,animated))
		{
			anim_use=true;
		}*/
	}
	prev_freezed = freezed;
	if (tNetPacket.r_eof())		// in case spawn + update 
	{
		freezed = false;
		return;
	}
	if (tNetPacket.r_u8())
	{
		freezed = false;
	}
	else {
		if (!freezed)
#ifdef XRGAME_EXPORTS
			m_freeze_time	= Device.dwTimeGlobal;
#else
			m_freeze_time	= 0;
#endif
		freezed = true;
	}
}

void CSE_ALifeObjectPhysic::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write		(tNetPacket);
	inherited2::UPDATE_Write		(tNetPacket);
//////////////////////////////////////////////////////////////////////////
	if (!m_u8NumItems) {
		tNetPacket.w_u8				(0);
		return;
	}

	mask_num_items					num_items;
	num_items.mask					= 0;
	num_items.num_items				= m_u8NumItems;

	R_ASSERT2						(
		num_items.num_items < (u8(1) << 5),
		make_string("%d",num_items.num_items)
		);

	if (State.enabled)									num_items.mask |= inventory_item_state_enabled;
	if (fis_zero(State.angular_vel.square_magnitude()))	num_items.mask |= inventory_item_angular_null;
	if (fis_zero(State.linear_vel.square_magnitude()))	num_items.mask |= inventory_item_linear_null;
	//if (anim_use)										num_items.mask |= animated;

	tNetPacket.w_u8					(num_items.common);

	/*if(check(num_items.mask,animated))
	{
		tNetPacket.w_float				(m_blend_timeCurrent);
	}*/

	{
		tNetPacket.w_vec3				(State.force);
		tNetPacket.w_vec3				(State.torque);

		tNetPacket.w_vec3				(State.position);

		tNetPacket.w_float			(State.quaternion.x);
		tNetPacket.w_float			(State.quaternion.y);
		tNetPacket.w_float			(State.quaternion.z);
		tNetPacket.w_float			(State.quaternion.w);	

		if (!check(num_items.mask,inventory_item_angular_null)) {
			tNetPacket.w_float		(State.angular_vel.x);
			tNetPacket.w_float		(State.angular_vel.y);
			tNetPacket.w_float		(State.angular_vel.z);
		}

		if (!check(num_items.mask,inventory_item_linear_null)) {
			tNetPacket.w_float		(State.linear_vel.x);
			tNetPacket.w_float		(State.linear_vel.y);
			tNetPacket.w_float		(State.linear_vel.z);
		}

	}
//.	Msg("--- Sync PH [%d].", ID);
	tNetPacket.w_u8(1);	//not freezed - doesn't mean anything..

#ifdef XRGAME_EXPORTS
#ifdef DEBUG
	m_last_update_time			= Device.dwTimeGlobal;
#endif
#endif
}




void CSE_ALifeObjectPhysic::load(NET_Packet &tNetPacket)
{
	inherited1::load(tNetPacket);
	inherited2::load(tNetPacket);
}


xr_token po_types[]={
	{ "Box",			epotBox			},
	{ "Fixed chain",	epotFixedChain	},
	{ "Free chain",		epotFreeChain	},
	{ "Skeleton",		epotSkeleton	},
	{ 0,				0				}
};

#ifndef XRGAME_EXPORTS
void CSE_ALifeObjectPhysic::FillProps		(LPCSTR pref, PropItemVec& values) 
{
	inherited1::FillProps		(pref,	 values);
	inherited2::FillProps		(pref,	 values);

	PHelper().CreateToken32		(values, PrepareKey(pref,*s_name,"Type"), &type,	po_types);
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Mass"), &mass, 0.1f, 10000.f);
    PHelper().CreateFlag8		(values, PrepareKey(pref,*s_name,"Active"), &_flags, flActive);

    // motions & bones
	PHelper().CreateChoose		(values, 	PrepareKey(pref,*s_name,"Model\\Fixed bones"),	&fixed_bones,		smSkeletonBones,0,(void*)visual()->get_visual(),8);
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_ALifeObjectPhysic::used_ai_locations	() const
{
	return						(false);
}

bool CSE_ALifeObjectPhysic::can_save			() const
{
		return						CSE_PHSkeleton::need_save();
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeObjectHangingLamp
////////////////////////////////////////////////////////////////////////////
CSE_ALifeObjectHangingLamp::CSE_ALifeObjectHangingLamp(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection),CSE_PHSkeleton(caSection)
{
	flags.assign				(flTypeSpot|flR1|flR2);

	range						= 10.f;
	color						= 0xffffffff;
    brightness					= 1.f;
	m_health					= 100.f;
	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);

	m_virtual_size				= 0.1f;
	m_ambient_radius			= 10.f;
    m_ambient_power				= 0.1f;
    spot_cone_angle				= deg2rad(120.f);
    glow_radius					= 0.7f;
	m_volumetric_quality		= 1.0f;
	m_volumetric_intensity		= 1.0f;
	m_volumetric_distance		= 1.0f;
}

CSE_ALifeObjectHangingLamp::~CSE_ALifeObjectHangingLamp()
{
}

void CSE_ALifeObjectHangingLamp::STATE_Read	(NET_Packet	&tNetPacket, u16 size)
{
	if (m_wVersion > 20)
		inherited1::STATE_Read	(tNetPacket,size);
	if (m_wVersion>=69)
		inherited2::STATE_Read	(tNetPacket,size);
	if (m_wVersion < 32)
		visual_read				(tNetPacket,m_wVersion);

	if (m_wVersion < 49){
		shared_str s_tmp;
		float	f_tmp;
		// model
		tNetPacket.r_u32			(color);
		tNetPacket.r_stringZ		(color_animator);
		tNetPacket.r_stringZ		(s_tmp);
		tNetPacket.r_stringZ		(s_tmp);
		tNetPacket.r_float			(range);
		tNetPacket.r_angle8			(f_tmp);
		if (m_wVersion>10)
			tNetPacket.r_float		(brightness);
		if (m_wVersion>11)
			tNetPacket.r_u16		(flags.flags);
		if (m_wVersion>12)
			tNetPacket.r_float		(f_tmp);
		if (m_wVersion>17)
			tNetPacket.r_stringZ	(startup_animation);

		set_editor_flag				(flVisualAnimationChange);

		if (m_wVersion > 42) {
			tNetPacket.r_stringZ	(s_tmp);
			tNetPacket.r_float		(f_tmp);
		}

		if (m_wVersion > 43){
			tNetPacket.r_stringZ	(fixed_bones);
		}

		if (m_wVersion > 44){
			tNetPacket.r_float		(m_health);
		}
	}else{
		// model
		tNetPacket.r_u32			(color);
		tNetPacket.r_float			(brightness);
		tNetPacket.r_stringZ		(color_animator);
		tNetPacket.r_float			(range);
    	tNetPacket.r_u16			(flags.flags);
		tNetPacket.r_stringZ		(startup_animation);
		set_editor_flag				(flVisualAnimationChange);
		tNetPacket.r_stringZ		(fixed_bones);
		tNetPacket.r_float			(m_health);
	}
	if (m_wVersion > 55){
		tNetPacket.r_float			(m_virtual_size);
	    tNetPacket.r_float			(m_ambient_radius);
    	tNetPacket.r_float			(m_ambient_power);
	    tNetPacket.r_stringZ		(m_ambient_texture);
        tNetPacket.r_stringZ		(light_texture);
        tNetPacket.r_stringZ		(light_main_bone);
        tNetPacket.r_float			(spot_cone_angle);
        tNetPacket.r_stringZ		(glow_texture);
        tNetPacket.r_float			(glow_radius);
	}
	if (m_wVersion > 96){
		tNetPacket.r_stringZ		(light_ambient_bone);
	}else{
		light_ambient_bone			= light_main_bone;
	}

	if (m_wVersion>118)
	{
		tNetPacket.r_float			(m_volumetric_quality);
		tNetPacket.r_float			(m_volumetric_intensity);
		tNetPacket.r_float			(m_volumetric_distance);
	}
}

void CSE_ALifeObjectHangingLamp::STATE_Write(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);

	// model
	tNetPacket.w_u32			(color);
	tNetPacket.w_float			(brightness);
	tNetPacket.w_stringZ		(color_animator);
	tNetPacket.w_float			(range);
   	tNetPacket.w_u16			(flags.flags);
	tNetPacket.w_stringZ		(startup_animation);
    tNetPacket.w_stringZ		(fixed_bones);
	tNetPacket.w_float			(m_health);
	tNetPacket.w_float			(m_virtual_size);
    tNetPacket.w_float			(m_ambient_radius);
    tNetPacket.w_float			(m_ambient_power);
    tNetPacket.w_stringZ		(m_ambient_texture);

    tNetPacket.w_stringZ		(light_texture);
    tNetPacket.w_stringZ		(light_main_bone);
    tNetPacket.w_float			(spot_cone_angle);
    tNetPacket.w_stringZ		(glow_texture);
    tNetPacket.w_float			(glow_radius);
    
	tNetPacket.w_stringZ		(light_ambient_bone);

	tNetPacket.w_float			(m_volumetric_quality);
	tNetPacket.w_float			(m_volumetric_intensity);
	tNetPacket.w_float			(m_volumetric_distance);
}


void CSE_ALifeObjectHangingLamp::UPDATE_Read(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeObjectHangingLamp::UPDATE_Write(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write		(tNetPacket);
	inherited2::UPDATE_Write		(tNetPacket);

}

void CSE_ALifeObjectHangingLamp::load(NET_Packet &tNetPacket)
{
	inherited1::load(tNetPacket);
	inherited2::load(tNetPacket);
}

void CSE_ALifeObjectHangingLamp::OnChangeFlag(PropValue* sender)
{
	set_editor_flag				(flUpdateProperties);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeObjectHangingLamp::FillProps	(LPCSTR pref, PropItemVec& values)
{
	inherited1::FillProps		(pref,values);
	inherited2::FillProps		(pref,values);

    PropValue* P				= 0;
	PHelper().CreateFlag16		(values, PrepareKey(pref,*s_name,"Flags\\Physic"),		&flags,			flPhysic);
	PHelper().CreateFlag16		(values, PrepareKey(pref,*s_name,"Flags\\Cast Shadow"),	&flags,			flCastShadow);
	PHelper().CreateFlag16		(values, PrepareKey(pref,*s_name,"Flags\\Allow R1"),	&flags,			flR1);
	PHelper().CreateFlag16		(values, PrepareKey(pref,*s_name,"Flags\\Allow R2"),	&flags,			flR2);
	P=PHelper().CreateFlag16	(values, PrepareKey(pref,*s_name,"Flags\\Allow Ambient"),&flags,			flPointAmbient);
    P->OnChangeEvent.bind		(this,&CSE_ALifeObjectHangingLamp::OnChangeFlag);
	// 
	P=PHelper().CreateFlag16	(values, PrepareKey(pref,*s_name,"Light\\Type"), 		&flags,				flTypeSpot, "Point", "Spot");
    P->OnChangeEvent.bind		(this,&CSE_ALifeObjectHangingLamp::OnChangeFlag);
	PHelper().CreateColor		(values, PrepareKey(pref,*s_name,"Light\\Main\\Color"),			&color);
    PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Light\\Main\\Brightness"),	&brightness,		0.1f, 5.f);
	PHelper().CreateChoose		(values, PrepareKey(pref,*s_name,"Light\\Main\\Color Animator"),&color_animator, 	smLAnim);
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Light\\Main\\Range"),			&range,				0.1f, 1000.f);
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Light\\Main\\Virtual Size"),	&m_virtual_size,	0.f, 100.f);
	PHelper().CreateChoose		(values, PrepareKey(pref,*s_name,"Light\\Main\\Texture"),	    &light_texture, 	smTexture, "lights");
	PHelper().CreateChoose		(values, PrepareKey(pref,*s_name,"Light\\Main\\Bone"),			&light_main_bone,	smSkeletonBones,0,(void*)visual()->get_visual());
	if (flags.is(flTypeSpot))
	{
		PHelper().CreateAngle	(values, PrepareKey(pref,*s_name,"Light\\Main\\Cone Angle"),	&spot_cone_angle,	deg2rad(1.f), deg2rad(120.f));
//		PHelper().CreateFlag16	(values, PrepareKey(pref,*s_name,"Light\\Main\\Volumetric"),	&flags,			flVolumetric);
		P=PHelper().CreateFlag16	(values, PrepareKey(pref,*s_name,"Flags\\Volumetric"),	&flags,			flVolumetric);
		P->OnChangeEvent.bind	(this,&CSE_ALifeObjectHangingLamp::OnChangeFlag);
	}

	if (flags.is(flPointAmbient)){
		PHelper().CreateFloat	(values, PrepareKey(pref,*s_name,"Light\\Ambient\\Radius"),		&m_ambient_radius,	0.f, 1000.f);
		PHelper().CreateFloat	(values, PrepareKey(pref,*s_name,"Light\\Ambient\\Power"),		&m_ambient_power);
		PHelper().CreateChoose	(values, PrepareKey(pref,*s_name,"Light\\Ambient\\Texture"),	&m_ambient_texture,	smTexture, 	"lights");
		PHelper().CreateChoose	(values, PrepareKey(pref,*s_name,"Light\\Ambient\\Bone"),		&light_ambient_bone,smSkeletonBones,0,(void*)visual()->get_visual());
	}

	if (flags.is(flVolumetric))
	{
		PHelper().CreateFloat	(values, PrepareKey(pref,*s_name,"Light\\Volumetric\\Quality"),		&m_volumetric_quality,	0.f, 1.f);
		PHelper().CreateFloat	(values, PrepareKey(pref,*s_name,"Light\\Volumetric\\Intensity"),	&m_volumetric_intensity,0.f, 10.f);
		PHelper().CreateFloat	(values, PrepareKey(pref,*s_name,"Light\\Volumetric\\Distance"),	&m_volumetric_distance,	0.f, 1.f);
	}

	// fixed bones
    PHelper().CreateChoose		(values, PrepareKey(pref,*s_name,"Model\\Fixed bones"),	&fixed_bones,		smSkeletonBones,0,(void*)visual()->get_visual(),8);
    // glow
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Glow\\Radius"),	    &glow_radius,		0.01f, 100.f);
	PHelper().CreateChoose		(values, PrepareKey(pref,*s_name,"Glow\\Texture"),	    &glow_texture, 		smTexture,	"glow");
	// game
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Game\\Health"),		&m_health,			0.f, 100.f);
}

#define VIS_RADIUS 		0.25f
void CSE_ALifeObjectHangingLamp::on_render(CDUInterface* du, ISE_AbstractLEOwner* owner, bool bSelected, const Fmatrix& parent,int priority, bool strictB2F)
{
	inherited1::on_render		(du,owner,bSelected,parent,priority,strictB2F);
	if ((1==priority)&&(false==strictB2F)){
		u32 clr					= bSelected?0x00FFFFFF:0x00FFFF00;
		Fmatrix main_xform, ambient_xform;
		owner->get_bone_xform		(*light_main_bone,main_xform);
		main_xform.mulA_43			(parent);
		if(flags.is(flPointAmbient) ){
			owner->get_bone_xform	(*light_ambient_bone,ambient_xform);
			ambient_xform.mulA_43	(parent);
		}
		if (bSelected){
			if (flags.is(flTypeSpot)){
				du->DrawSpotLight	(main_xform.c, main_xform.k, range, spot_cone_angle, clr);
			}else{
				du->DrawLineSphere	(main_xform.c, range, clr, true);
			}
			if(flags.is(flPointAmbient) )
				du->DrawLineSphere	(ambient_xform.c, m_ambient_radius, clr, true);
		}
		du->DrawPointLight		(main_xform.c,VIS_RADIUS, clr);
		if(flags.is(flPointAmbient) )
			du->DrawPointLight	(ambient_xform.c,VIS_RADIUS, clr);
	}
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_ALifeObjectHangingLamp::used_ai_locations	() const
{
	return						(false);
}

bool CSE_ALifeObjectHangingLamp::validate			()
{
	if (flags.test(flR1) || flags.test(flR2))
		return					(true);

	Msg							("! Render type is not set properly!");
	return						(false);
}

bool CSE_ALifeObjectHangingLamp::match_configuration() const
{
	R_ASSERT3(flags.test(flR1) || flags.test(flR2),"no renderer type set for hanging-lamp ",name_replace());
#ifdef XRGAME_EXPORTS
	return						(
		(flags.test(flR1) && (::Render->get_generation() == IRender_interface::GENERATION_R1)) ||
		(flags.test(flR2) && (::Render->get_generation() == IRender_interface::GENERATION_R2))
	);
#else
	return						(true);
#endif
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeObjectSearchlight
////////////////////////////////////////////////////////////////////////////

CSE_ALifeObjectProjector::CSE_ALifeObjectProjector(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection)
{
	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);
}

CSE_ALifeObjectProjector::~CSE_ALifeObjectProjector()
{
}

void CSE_ALifeObjectProjector::STATE_Read	(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read	(tNetPacket,size);
}

void CSE_ALifeObjectProjector::STATE_Write(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeObjectProjector::UPDATE_Read(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeObjectProjector::UPDATE_Write(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeObjectProjector::FillProps			(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,	 values);
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_ALifeObjectProjector::used_ai_locations() const
{
	return						(false);
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeSchedulable
////////////////////////////////////////////////////////////////////////////

CSE_ALifeSchedulable::CSE_ALifeSchedulable	(LPCSTR caSection)
{
	m_tpCurrentBestWeapon		= 0;
	m_tpBestDetector			= 0;
	m_schedule_counter			= u64(-1);
}

CSE_ALifeSchedulable::~CSE_ALifeSchedulable	()
{
}

bool CSE_ALifeSchedulable::need_update		(CSE_ALifeDynamicObject *object)
{
	return						(!object || (object->m_bDirectControl && /**object->interactive() && /**/object->used_ai_locations() && !object->m_bOnline));
}

CSE_Abstract *CSE_ALifeSchedulable::init	()
{
	return						(base());
}

u32	CSE_ALifeSchedulable::ef_creature_type	() const
{
	string16					temp; CLSID2TEXT(base()->m_tClassID,temp);
	R_ASSERT3					(false,"Invalid alife creature type request, virtual function is not properly overloaded!",temp);
	return						(u32(-1));
}

u32	 CSE_ALifeSchedulable::ef_anomaly_type	() const
{
	string16					temp; CLSID2TEXT(base()->m_tClassID,temp);
	R_ASSERT3					(false,"Invalid alife anomaly type request, virtual function is not properly overloaded!",temp);
	return						(u32(-1));
}

u32	 CSE_ALifeSchedulable::ef_weapon_type	() const
{
	string16					temp; CLSID2TEXT(base()->m_tClassID,temp);
	R_ASSERT3					(false,"Invalid alife weapon type request, virtual function is not properly overloaded!",temp);
	return						(u32(-1));
}

u32	 CSE_ALifeSchedulable::ef_detector_type	() const
{
	string16					temp; CLSID2TEXT(base()->m_tClassID,temp);
	R_ASSERT3					(false,"Invalid alife detector type request, virtual function is not properly overloaded!",temp);
	return						(u32(-1));
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeHelicopter
////////////////////////////////////////////////////////////////////////////

CSE_ALifeHelicopter::CSE_ALifeHelicopter	(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection), CSE_Motion(),CSE_PHSkeleton(caSection)
{
	m_flags.set					(flUseSwitches,		FALSE);
	m_flags.set					(flSwitchOffline,	FALSE);
	m_flags.set					(flInteractive,		FALSE);
}

CSE_ALifeHelicopter::~CSE_ALifeHelicopter	()
{
}

CSE_Motion* CSE_ALifeHelicopter::motion		()
{
	return						(this);
}

void CSE_ALifeHelicopter::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket,size);
    CSE_Motion::motion_read		(tNetPacket);
	if(m_wVersion>=69)
		inherited3::STATE_Read		(tNetPacket,size);
    tNetPacket.r_stringZ		(startup_animation);
	tNetPacket.r_stringZ		(engine_sound);

	set_editor_flag				(flVisualAnimationChange | flMotionChange);
}

void CSE_ALifeHelicopter::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
    CSE_Motion::motion_write	(tNetPacket);
	inherited3::STATE_Write		(tNetPacket);
    tNetPacket.w_stringZ			(startup_animation);
    tNetPacket.w_stringZ			(engine_sound);
}

void CSE_ALifeHelicopter::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited3::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeHelicopter::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write		(tNetPacket);
	inherited3::UPDATE_Write		(tNetPacket);
}

void CSE_ALifeHelicopter::load		(NET_Packet &tNetPacket)
{
	inherited1::load(tNetPacket);
	inherited3::load(tNetPacket);
}
bool CSE_ALifeHelicopter::can_save() const
{
	return						CSE_PHSkeleton::need_save();
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeHelicopter::FillProps(LPCSTR pref, PropItemVec& values)
{
	inherited1::FillProps		(pref,	values);
	inherited2::FillProps		(pref,	values);
	inherited3::FillProps		(pref,	values);

    PHelper().CreateChoose		(values,	PrepareKey(pref,*s_name,"Engine Sound"), &engine_sound, smSoundSource);
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_ALifeHelicopter::used_ai_locations	() const
{
	return						(false);
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeCar
////////////////////////////////////////////////////////////////////////////
CSE_ALifeCar::CSE_ALifeCar				(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection),CSE_PHSkeleton(caSection)
{

	if (pSettings->section_exist(caSection) && pSettings->line_exist(caSection,"visual"))
    	set_visual				(pSettings->r_string(caSection,"visual"));
	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);
	health						=1.0f;
}

CSE_ALifeCar::~CSE_ALifeCar				()
{
}

void CSE_ALifeCar::STATE_Read			(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket,size);

	if(m_wVersion>65)
		inherited2::STATE_Read		(tNetPacket,size);
		if ((m_wVersion > 52) && (m_wVersion < 55))
		tNetPacket.r_float		();
	if(m_wVersion>92)
			health=tNetPacket.r_float();
	if(health>1.0f) health/=100.0f;
}

void CSE_ALifeCar::STATE_Write			(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
	tNetPacket.w_float(health);
}

void CSE_ALifeCar::UPDATE_Read			(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeCar::UPDATE_Write			(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write		(tNetPacket);
	inherited2::UPDATE_Write		(tNetPacket);
}

bool CSE_ALifeCar::used_ai_locations() const
{
	return						(false);
}

bool CSE_ALifeCar::can_save() const
{
	return						CSE_PHSkeleton::need_save();
}

void CSE_ALifeCar::load(NET_Packet &tNetPacket)
{
	inherited1::load(tNetPacket);
	inherited2::load(tNetPacket);

}

void CSE_ALifeCar::data_load(NET_Packet	&tNetPacket)
{
	//inherited1::data_load(tNetPacket);
	inherited2::data_load(tNetPacket);
	//VERIFY(door_states.empty());

	tNetPacket.r_vec3(o_Position);
	tNetPacket.r_vec3(o_Angle);
	door_states.clear();
	u16 doors_number=tNetPacket.r_u16();
	for(u16 i=0;i<doors_number;++i)
	{
		SDoorState ds;ds.read(tNetPacket);
		door_states.push_back(ds);
	}

	wheel_states.clear();
	u16 wheels_number=tNetPacket.r_u16();
	for(u16 i=0;i<wheels_number;++i)
	{
		SWheelState ws;ws.read(tNetPacket);
		wheel_states.push_back(ws);
	}
	health=tNetPacket.r_float();
}
void CSE_ALifeCar::data_save(NET_Packet &tNetPacket)
{
	//inherited1::data_save(tNetPacket);
	inherited2::data_save(tNetPacket);
	tNetPacket.w_vec3(o_Position);
	tNetPacket.w_vec3(o_Angle);
	{
		tNetPacket.w_u16(u16(door_states.size()));
		xr_vector<SDoorState>::iterator i=door_states.begin(),e=door_states.end();
		for(;e!=i;++i)
		{
			i->write(tNetPacket);
		}
		//door_states.clear();
	}
	{
		tNetPacket.w_u16(u16(wheel_states.size()));
		xr_vector<SWheelState>::iterator i=wheel_states.begin(),e=wheel_states.end();
		for(;e!=i;++i)
		{
			i->write(tNetPacket);
		}
		//wheel_states.clear();
	}
	tNetPacket.w_float(health);
}
void CSE_ALifeCar::SDoorState::read(NET_Packet& P)
{
	open_state=P.r_u8();health=P.r_float();
}
void CSE_ALifeCar::SDoorState::write(NET_Packet& P)
{
	P.w_u8(open_state);P.w_float(health); 
}

void CSE_ALifeCar::SWheelState::read(NET_Packet& P)
{
	health=P.r_float();
}
void CSE_ALifeCar::SWheelState::write(NET_Packet& P)
{
	P.w_float(health);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeCar::FillProps				(LPCSTR pref, PropItemVec& values)
{
  	inherited1::FillProps			(pref,values);
	inherited2::FillProps			(pref,values);
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Health"),			&health,			0.f, 1.0f);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeObjectBreakable
////////////////////////////////////////////////////////////////////////////
CSE_ALifeObjectBreakable::CSE_ALifeObjectBreakable	(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection)
{
	m_health					= 1.f;
	m_flags.set					(flUseSwitches,FALSE);
	m_flags.set					(flSwitchOffline,FALSE);
}

CSE_ALifeObjectBreakable::~CSE_ALifeObjectBreakable	()
{
}

void CSE_ALifeObjectBreakable::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
	tNetPacket.r_float			(m_health);
}

void CSE_ALifeObjectBreakable::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_float			(m_health);
}

void CSE_ALifeObjectBreakable::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeObjectBreakable::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeObjectBreakable::FillProps		(LPCSTR pref, PropItemVec& values)
{
  	inherited::FillProps			(pref,values);
	PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Health"),			&m_health,			0.f, 100.f);
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_ALifeObjectBreakable::used_ai_locations	() const
{
	return						(false);
}

bool CSE_ALifeObjectBreakable::can_switch_offline	() const
{
	return						(false);
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeObjectClimable
////////////////////////////////////////////////////////////////////////////
CSE_ALifeObjectClimable::CSE_ALifeObjectClimable	(LPCSTR caSection) : CSE_Shape(), CSE_ALifeDynamicObject(caSection)
{
	//m_health					= 100.f;
	//m_flags.set					(flUseSwitches,FALSE);
	//m_flags.set					(flSwitchOffline,FALSE);
}

CSE_ALifeObjectClimable::~CSE_ALifeObjectClimable	()
{
}

ISE_Shape* CSE_ALifeObjectClimable::shape					()
{
	return						(this);
}

void CSE_ALifeObjectClimable::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	//inherited1::STATE_Read		(tNetPacket,size);
	if(m_wVersion==99)
		CSE_ALifeObject::STATE_Read		(tNetPacket,size);
	if(m_wVersion>99)
		inherited2::STATE_Read		(tNetPacket,size);
	cform_read(tNetPacket);
}

void CSE_ALifeObjectClimable::STATE_Write	(NET_Packet	&tNetPacket)
{
	//inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
	cform_write(tNetPacket);
}

void CSE_ALifeObjectClimable::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	//inherited1::UPDATE_Read		(tNetPacket);
	//inherited2::UPDATE_Read		(tNetPacket);
	
}

void CSE_ALifeObjectClimable::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	//inherited1::UPDATE_Write		(tNetPacket);
	//inherited2::UPDATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeObjectClimable::FillProps		(LPCSTR pref, PropItemVec& values)
{
	//inherited1::FillProps			(pref,values);
	inherited2::FillProps			(pref,values);
	//PHelper().CreateFloat		(values, PrepareKey(pref,*s_name,"Health"),			&m_health,			0.f, 100.f);
}
#endif // #ifndef XRGAME_EXPORTS

bool CSE_ALifeObjectClimable::used_ai_locations	() const
{
	return						(false);
}

bool CSE_ALifeObjectClimable::can_switch_offline	() const
{
	return						(false);
}

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeMountedWeapon
////////////////////////////////////////////////////////////////////////////
CSE_ALifeMountedWeapon::CSE_ALifeMountedWeapon	(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection)
{             
}

CSE_ALifeMountedWeapon::~CSE_ALifeMountedWeapon	()
{
}

void CSE_ALifeMountedWeapon::STATE_Read			(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeMountedWeapon::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeMountedWeapon::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeMountedWeapon::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeMountedWeapon::FillProps			(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,values);
}
#endif // #ifndef XRGAME_EXPORTS

CSE_ALifeStationaryMgun::CSE_ALifeStationaryMgun	(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection)
{}

CSE_ALifeStationaryMgun::~CSE_ALifeStationaryMgun	()
{}

void CSE_ALifeStationaryMgun::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read			(tNetPacket);
	m_bWorking = !!tNetPacket.r_u8	();
	load_data						(m_destEnemyDir, tNetPacket);
}

void CSE_ALifeStationaryMgun::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write			(tNetPacket);
	tNetPacket.w_u8					(m_bWorking? 1:0);
	save_data						(m_destEnemyDir, tNetPacket);
}


void CSE_ALifeStationaryMgun::STATE_Read			(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeStationaryMgun::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeStationaryMgun::FillProps			(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProps			(pref,values);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeTeamBaseZone
////////////////////////////////////////////////////////////////////////////
CSE_ALifeTeamBaseZone::CSE_ALifeTeamBaseZone(LPCSTR caSection) : CSE_ALifeSpaceRestrictor(caSection)
{
	m_team						= 0;
}

CSE_ALifeTeamBaseZone::~CSE_ALifeTeamBaseZone()
{
}

void CSE_ALifeTeamBaseZone::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
	tNetPacket.r_u8				(m_team);
}

void CSE_ALifeTeamBaseZone::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_u8				(m_team);
}

void CSE_ALifeTeamBaseZone::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeTeamBaseZone::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeTeamBaseZone::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited::FillProps		(pref,items);
	PHelper().CreateU8			(items, PrepareKey(pref,*s_name,"team"),			&m_team,			0, 16);
}
#endif // #ifndef XRGAME_EXPORTS

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeSmartZone
////////////////////////////////////////////////////////////////////////////

CSE_ALifeSmartZone::CSE_ALifeSmartZone	(LPCSTR caSection) : CSE_ALifeSpaceRestrictor(caSection), CSE_ALifeSchedulable(caSection)
{
}

CSE_ALifeSmartZone::~CSE_ALifeSmartZone	()
{
}

CSE_Abstract *CSE_ALifeSmartZone::base		()
{
	return						(this);
}

const CSE_Abstract *CSE_ALifeSmartZone::base	() const
{
	return						(this);
}

CSE_Abstract *CSE_ALifeSmartZone::init		()
{
	inherited1::init			();
	inherited2::init			();
	return						(this);
}

void CSE_ALifeSmartZone::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeSmartZone::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
}

void CSE_ALifeSmartZone::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeSmartZone::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
}

#ifndef XRGAME_EXPORTS
void CSE_ALifeSmartZone::FillProps		(LPCSTR pref, PropItemVec& items)
{
	inherited1::FillProps		(pref,items);
}
#endif // #ifndef XRGAME_EXPORTS

void CSE_ALifeSmartZone::update			()
{
}

float CSE_ALifeSmartZone::detect_probability()
{
	return						(0.f);
}

void CSE_ALifeSmartZone::smart_touch	(CSE_ALifeMonsterAbstract *monster)
{
}
