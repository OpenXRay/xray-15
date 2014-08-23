#ifndef __XR_UIGAMECUSTOM_H__
#define __XR_UIGAMECUSTOM_H__
#pragma once


#include "script_export_space.h"
#include "object_interfaces.h"
#include "inventory_space.h"
#include "ui/KillMessageStruct.h"
#include "gametype_chooser.h"
// refs
class CUI;
class CTeamBaseZone;
class game_cl_GameState;
class CUIDialogWnd;
class CUICaption;
class CUIStatic;
class CUIWindow;
class CUIXml;
class CUIActorMenu;
class CUIPdaWnd;			

struct SDrawStaticStruct :public IPureDestroyableObject{
	SDrawStaticStruct	();
	virtual	void	destroy			();
	CUIStatic*		m_static;
	float			m_endTime;
	shared_str		m_name;
	void			Draw();
	void			Update();
	CUIStatic*		wnd()		{return m_static;}
	bool			IsActual();
	bool operator ==(LPCSTR str){
		return (m_name == str);
	}
};


typedef xr_vector<SDrawStaticStruct>	st_vec;
//#include "game_base_space.h"
struct SGameTypeMaps
{
	shared_str				m_game_type_name;
	EGameIDs				m_game_type_id;
	struct SMapItm{
		shared_str	map_name;
		shared_str	map_ver;
		bool operator ==(const SMapItm& other){return map_name==other.map_name && map_ver==other.map_ver;}
	};
	xr_vector<SMapItm>		m_map_names;
};

struct SGameWeathers
{
	shared_str				m_weather_name;
	shared_str				m_start_time;
};
typedef xr_vector<SGameWeathers>					GAME_WEATHERS;
typedef xr_vector<SGameWeathers>::iterator			GAME_WEATHERS_IT;
typedef xr_vector<SGameWeathers>::const_iterator	GAME_WEATHERS_CIT;

class CMapListHelper
{
	typedef xr_vector<SGameTypeMaps>	TSTORAGE;
	typedef TSTORAGE::iterator			TSTORAGE_IT;
	typedef TSTORAGE::iterator			TSTORAGE_CIT;
	TSTORAGE							m_storage;
	GAME_WEATHERS						m_weathers;
	void						Load			();
	void						LoadMapInfo		(LPCSTR file_name, const xr_string& map_name, LPCSTR map_ver="1.0");
	SGameTypeMaps*				GetMapListInt	(const shared_str& game_type);
public:
	const SGameTypeMaps&		GetMapListFor	(const shared_str& game_type);
	const SGameTypeMaps&		GetMapListFor	(const EGameIDs game_id);
	const GAME_WEATHERS&		GetGameWeathers	();
};

extern CMapListHelper	gMapListHelper;

class CUIGameCustom :public DLL_Pure, public ISheduled
{
	typedef ISheduled inherited;
protected:
	u32					uFlags;

	void				SetFlag					(u32 mask, BOOL flag){if (flag) uFlags|=mask; else uFlags&=~mask; }
	void				InvertFlag				(u32 mask){if (uFlags&mask) uFlags&=~mask; else uFlags|=mask; }
	BOOL				GetFlag					(u32 mask){return uFlags&mask;}
	CUICaption*			GameCaptions			() {return m_pgameCaptions;}
	CUICaption*			m_pgameCaptions;
	CUIXml*				m_msgs_xml;
	st_vec				m_custom_statics;

	CUIActorMenu*		m_ActorMenu;
	CUIPdaWnd*			m_PdaMenu;

public:
	virtual void		SetClGame				(game_cl_GameState* g){};
	virtual void		OnInventoryAction		(PIItem item, u16 action_type);

	virtual				float					shedule_Scale		();
	virtual				void					shedule_Update		(u32 dt);
	
						CUIGameCustom			();
	virtual				~CUIGameCustom			();

	virtual	void		Init					()	{};
	
	virtual void		Render					();
	virtual void		OnFrame					();
	virtual	void		reset_ui				();
	
	IC CUIActorMenu&	ActorMenu				() const { return *m_ActorMenu; }
	IC CUIPdaWnd&		PdaMenu					() const { return *m_PdaMenu;   }
			bool		ShowActorMenu			();
			void		HideActorMenu			();
			bool		ShowPdaMenu				();
			void		HidePdaMenu				();

	virtual bool		IR_OnKeyboardPress		(int dik);
	virtual bool		IR_OnKeyboardRelease	(int dik);
	virtual bool		IR_OnMouseMove			(int dx, int dy);
	virtual bool		IR_OnMouseWheel			(int direction);


	void				AddDialogToRender		(CUIWindow* pDialog);
	void				RemoveDialogToRender	(CUIWindow* pDialog);
	
	CUIDialogWnd*		MainInputReceiver		();
	virtual void		HideShownDialogs		(){};

			void		AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color);
			void		AddCustomMessage		(LPCSTR id, float x, float y, float font_size, CGameFont *pFont, u16 alignment, u32 color/*, LPCSTR def_text*/, float flicker );
			void		CustomMessageOut		(LPCSTR id, LPCSTR msg, u32 color);
			void		RemoveCustomMessage		(LPCSTR id);

			SDrawStaticStruct*	AddCustomStatic		(LPCSTR id, bool bSingleInstance);
			SDrawStaticStruct*	GetCustomStatic		(LPCSTR id);
			void				RemoveCustomStatic	(LPCSTR id);

	virtual	shared_str	shedule_Name				() const		{ return shared_str("CUIGameCustom"); };
	virtual bool		shedule_Needed				()				{return true;};

	virtual void		ChangeTotalMoneyIndicator	(LPCSTR newMoneyString) {};
	virtual void		DisplayMoneyChange			(LPCSTR deltaMoney) {};
	virtual void		DisplayMoneyBonus			(KillMessageStruct bonus) {};

	DECLARE_SCRIPT_REGISTER_FUNCTION
}; // class CUIGameCustom

add_to_type_list(CUIGameCustom)
#undef script_type_list
#define script_type_list save_type_list(CUIGameCustom)

#endif // __XR_UIGAMECUSTOM_H__
