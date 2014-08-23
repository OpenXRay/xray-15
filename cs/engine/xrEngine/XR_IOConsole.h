// XR_IOConsole.h: interface for the CConsole class.
//
//////////////////////////////////////////////////////////////////////
#ifndef XR_IOCONSOLE_H_INCLUDED
#define XR_IOCONSOLE_H_INCLUDED

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/ConsoleRender.h"

//refs
class ENGINE_API CGameFont;
class ENGINE_API IConsole_Command;

namespace text_editor
{
class line_editor;
class line_edit_control;
};

class ENGINE_API CConsole :
	public pureRender,
	public pureFrame
{
public:
	struct str_pred : public std::binary_function<char*, char*, bool>
	{	
		IC bool operator()(const char* x, const char* y) const
		{
			return (xr_strcmp( x, y ) < 0);
		}
	};
	typedef  xr_map<LPCSTR,IConsole_Command*,str_pred>	vecCMD;
	typedef  vecCMD::iterator							vecCMD_IT;
	typedef  fastdelegate::FastDelegate0<void>		Callback;
	enum			{ CONSOLE_BUF_SIZE = 1024 };

protected:
	int				scroll_delta;

	CGameFont*		pFont;
	CGameFont*		pFont2;
	IConsoleRender*	m_pRender;

	POINT			m_mouse_pos;

private:
	xr_vector<shared_str>	m_cmd_history;
	u32						m_cmd_history_max;
	int						m_cmd_history_idx;
	shared_str				m_last_cmd;

public:
					CConsole			();
	virtual			~CConsole			();
	virtual	void	Initialize			();
	virtual void	Destroy				();

	virtual void	OnRender			();
	virtual void	OnFrame				();
	
	string64		ConfigFile;
	bool			bVisible;
	vecCMD			Commands;

	void			AddCommand			( IConsole_Command* cc );
	void			RemoveCommand		( IConsole_Command* cc );

	void			Show				();
	void			Hide				();

//	void			Save				();
	void			Execute				( LPCSTR cmd );
	void			ExecuteScript		( LPCSTR str );
	void			ExecuteCommand		( LPCSTR cmd, bool record_cmd = true );
	void			SelectCommand		();

	bool			GetBool				( LPCSTR cmd );
	float			GetFloat			( LPCSTR cmd, float& min, float& max);
	int				GetInteger			( LPCSTR cmd, int& min, int& max);
	LPCSTR			GetString			( LPCSTR cmd );
	LPCSTR			GetToken			( LPCSTR cmd );
	xr_token*		GetXRToken			( LPCSTR cmd );
	Fvector			GetFVector			( LPCSTR cmd );
	Fvector*		GetFVectorPtr		( LPCSTR cmd );

protected:
	text_editor::line_editor*			m_editor;
	text_editor::line_edit_control&		ec();

	enum Console_mark // (int)=char
	{
		no_mark = ' ',
		mark0  = '~',
		mark1  = '!', // error
		mark2  = '@', // console cmd
		mark3  = '#',
		mark4  = '$',
		mark5  = '%',
		mark6  = '^',
		mark7  = '&',
		mark8  = '*',
		mark9  = '-', // green = ok
		mark10 = '+',
		mark11 = '=',
		mark12 = '/'
	};
	
	bool	is_mark				( Console_mark type );
	u32		get_mark_color		( Console_mark type );

	void	OutFont				( LPCSTR text, float& pos_y );
	void	Register_callbacks	();
	
protected:
	void xr_stdcall Prev_log	();
	void xr_stdcall Next_log	();
	void xr_stdcall Begin_log	();
	void xr_stdcall End_log		();
	
	void xr_stdcall Find_cmd	();
	void xr_stdcall Find_cmd_back();
	void xr_stdcall Prev_cmd	();
	void xr_stdcall Next_cmd	();
	
	void xr_stdcall Execute_cmd	();
	void xr_stdcall Show_cmd	();
	void xr_stdcall Hide_cmd	();
	void xr_stdcall GamePause	();
	void xr_stdcall SwitchKL	();

protected:
	void	add_cmd_history		( shared_str const& str );
	void	next_cmd_history_idx();
	void	prev_cmd_history_idx();
	void	reset_cmd_history_idx();

}; // class CConsole

ENGINE_API extern CConsole* Console;

#endif // XR_IOCONSOLE_H_INCLUDED
