////////////////////////////////////////////////////////////////////////////
//	Module 		: XR_IOConsole_callback.cpp
//	Created 	: 17.05.2008
//	Author		: Evgeniy Sokolov
//	Description : Console`s callback functions class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XR_IOConsole.h"

#include "line_editor.h"
#include "xr_input.h"
#include "xr_ioc_cmd.h"


void CConsole::Register_callbacks()
{
	ec().assign_callback( DIK_PRIOR, text_editor::ks_free,  Callback( this, &CConsole::Prev_log      ) );
	ec().assign_callback( DIK_NEXT,  text_editor::ks_free,  Callback( this, &CConsole::Next_log      ) );
	ec().assign_callback( DIK_PRIOR, text_editor::ks_Ctrl,  Callback( this, &CConsole::Begin_log     ) );
	ec().assign_callback( DIK_NEXT,  text_editor::ks_Ctrl,  Callback( this, &CConsole::End_log       ) );

	ec().assign_callback( DIK_TAB,   text_editor::ks_free,  Callback( this, &CConsole::Find_cmd      ) );
	ec().assign_callback( DIK_TAB,   text_editor::ks_Shift, Callback( this, &CConsole::Find_cmd_back ) );
	ec().assign_callback( DIK_UP,    text_editor::ks_free,  Callback( this, &CConsole::Prev_cmd      ) );
	ec().assign_callback( DIK_DOWN,  text_editor::ks_free,  Callback( this, &CConsole::Next_cmd      ) );
	ec().assign_callback( DIK_TAB,   text_editor::ks_Alt,   Callback( this, &CConsole::GamePause ) );

	ec().assign_callback( DIK_RETURN,      text_editor::ks_free, Callback( this, &CConsole::Execute_cmd ) );
	ec().assign_callback( DIK_NUMPADENTER, text_editor::ks_free, Callback( this, &CConsole::Execute_cmd ) );
	
	ec().assign_callback( DIK_ESCAPE, text_editor::ks_free, Callback( this, &CConsole::Hide_cmd ) );
	ec().assign_callback( DIK_GRAVE,  text_editor::ks_free, Callback( this, &CConsole::Hide_cmd ) );
	
	ec().assign_callback( DIK_LSHIFT, text_editor::ks_Ctrl, Callback( this, &CConsole::SwitchKL ) );
}

void CConsole::Prev_log() // DIK_PRIOR=PAGE_UP
{
	scroll_delta++;
	if ( scroll_delta > int(LogFile->size())-1 )
	{
		scroll_delta = LogFile->size()-1;
	}
}

void CConsole::Next_log() // DIK_NEXT=PAGE_DOWN
{
	scroll_delta--;
	if ( scroll_delta < 0 )
	{
		scroll_delta = 0;
	}
}

void CConsole::Begin_log() // PAGE_UP+Ctrl
{
	scroll_delta = LogFile->size()-1;
}

void CConsole::End_log() // PAGE_DOWN+Ctrl
{
	scroll_delta = 0;
}

void CConsole::Find_cmd() // DIK_TAB
{
	LPCSTR edt      = ec().str_edit();
	LPCSTR radmin_cmd_name = "ra ";
	bool b_ra  = (edt == strstr( edt, radmin_cmd_name ) );
	u32 offset = (b_ra)? xr_strlen( radmin_cmd_name ) : 0;

	vecCMD_IT it = Commands.lower_bound( edt + offset );
	if ( it != Commands.end() )
	{
		IConsole_Command& cc = *(it->second);
		LPCSTR name_cmd      = cc.Name();
		u32    name_cmd_size = xr_strlen( name_cmd );
		PSTR   new_str  = (PSTR)_alloca( (offset + name_cmd_size + 2) * sizeof(char) );

		strcpy_s( new_str, offset + name_cmd_size + 2, (b_ra)? radmin_cmd_name : "" );
		strcat_s( new_str, offset + name_cmd_size + 2, name_cmd );
		strcat_s( new_str, offset + name_cmd_size + 2, " " );
		ec().set_edit( new_str );
	}
}

void CConsole::Find_cmd_back() // DIK_TAB+shift
{
	LPCSTR edt      = ec().str_edit();
	LPCSTR radmin_cmd_name = "ra ";
	bool b_ra  = (edt == strstr( edt, radmin_cmd_name ) );
	u32 offset = (b_ra)? xr_strlen( radmin_cmd_name ) : 0;

	vecCMD_IT it = Commands.lower_bound( edt + offset );
	if ( it != Commands.begin() )
	{
		--it;
		if ( it != Commands.begin() )
		{
			--it;
			IConsole_Command& cc = *(it->second);
			LPCSTR name_cmd      = cc.Name();
			u32    name_cmd_size = xr_strlen( name_cmd );
			PSTR   new_str  = (PSTR)_alloca( (offset + name_cmd_size + 2) * sizeof(char) );

			strcpy_s( new_str, offset + name_cmd_size + 2, (b_ra)? radmin_cmd_name : "" );
			strcat_s( new_str, offset + name_cmd_size + 2, name_cmd );
			strcat_s( new_str, offset + name_cmd_size + 2, " " );
			ec().set_edit( new_str );
		}
	}
}

void CConsole::Prev_cmd() // DIK_UP
{
	prev_cmd_history_idx();
	SelectCommand();
}

void CConsole::Next_cmd() // DIK_DOWN
{
	next_cmd_history_idx();
	SelectCommand();
}

void CConsole::Execute_cmd() // DIK_RETURN, DIK_NUMPADENTER
{
	ExecuteCommand( ec().str_edit() );
}

void CConsole::Show_cmd()
{
	Show();
}

void CConsole::Hide_cmd()
{
	Hide();
}

void CConsole::GamePause()
{

}

void CConsole::SwitchKL()
{
	ActivateKeyboardLayout( 0, 0 );
}