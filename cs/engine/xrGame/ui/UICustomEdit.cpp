#include "stdafx.h"

#include "UICustomEdit.h"
#include "UILines.h"

#include "../../xrEngine/line_edit_control.h"
#include "../../xrEngine/xr_input.h"

CUICustomEdit::CUICustomEdit()
{
	m_editor_control = xr_new<text_editor::line_edit_control>( (u32)EDIT_BUF_SIZE );
	Init( (u32)EDIT_BUF_SIZE );

	SetVTextAlignment( valCenter );
	SetTextComplexMode( false );
	m_pLines->SetColoringMode( false );
	m_pLines->SetCutWordsMode( true );
	m_pLines->SetUseNewLineMode( false );
	
	m_out_str[0]   = NULL;
	m_dx_cur       = 0.0f;
	m_read_mode    = false;
	m_force_update = true;
	m_last_key_state_time = 0;
}	


CUICustomEdit::~CUICustomEdit()
{
	xr_delete( m_editor_control );
}

text_editor::line_edit_control& CUICustomEdit::ec()
{
	VERIFY( m_editor_control );
	return	*m_editor_control;
}

void CUICustomEdit::Register_callbacks()
{
	ec().assign_callback( DIK_ESCAPE,      text_editor::ks_free, Callback( this, &CUICustomEdit::press_escape ) );
	ec().assign_callback( DIK_RETURN,      text_editor::ks_free, Callback( this, &CUICustomEdit::press_commit ) );
	ec().assign_callback( DIK_NUMPADENTER, text_editor::ks_free, Callback( this, &CUICustomEdit::press_commit ) );
	ec().assign_callback( DIK_GRAVE,       text_editor::ks_free, Callback( this, &CUICustomEdit::nothing ) );

//	ec().assign_callback( DIK_TAB,   text_editor::ks_free,  Callback( this, &CConsole::Find_cmd      ) );
//	ec().assign_callback( DIK_TAB,   text_editor::ks_Shift, Callback( this, &CConsole::Find_cmd_back ) );
//	ec().assign_callback( DIK_UP,    text_editor::ks_free,  Callback( this, &CConsole::Prev_cmd      ) );
//	ec().assign_callback( DIK_DOWN,  text_editor::ks_free,  Callback( this, &CConsole::Next_cmd      ) );
//	ec().assign_callback( DIK_TAB,   text_editor::ks_Alt,   Callback( this, &CConsole::GamePause ) );

}

void  CUICustomEdit::Init( u32 max_char_count, bool number_only_mode, bool read_mode, bool fn_mode )
{
	if ( read_mode )
	{
		m_editor_control->init( max_char_count, text_editor::im_read_only );
		m_editor_control->set_selected_mode( true );
		m_read_mode = true;
	}
	else
	{
		if ( number_only_mode )
		{
			m_editor_control->init( max_char_count, text_editor::im_number_only );
		}
		else if ( fn_mode )
		{
			m_editor_control->init( max_char_count, text_editor::im_file_name_mode );
		}
		else
		{
			m_editor_control->init( max_char_count );
		}
		m_editor_control->set_selected_mode( false );
		m_read_mode = false;
	}
	
	Register_callbacks();
	ClearText();

	m_bInputFocus = false;
}

void CUICustomEdit::InitCustomEdit( Fvector2 pos, Fvector2 size )
{
	inherited::SetWndPos	( pos  );
	inherited::SetWndSize	( size );
}

void CUICustomEdit::SetPasswordMode( bool mode )
{
	if ( m_pLines )
	{
		m_pLines->SetPasswordMode( mode );
	}
}

void CUICustomEdit::OnFocusLost()
{
	inherited::OnFocusLost();
}

void CUICustomEdit::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
//	if(pWnd == GetParent())
//	{
		//кто-то другой захватил клавиатуру
		if ( msg == WINDOW_KEYBOARD_CAPTURE_LOST )
		{
			m_bInputFocus = false;
		}
//	}
}

bool CUICustomEdit::OnMouse(float x, float y, EUIMessages mouse_action)
{
//	if (m_bFocusByDbClick)
	{
		if(mouse_action == WINDOW_LBUTTON_DB_CLICK && !m_bInputFocus)
		{
			GetParent()->SetKeyboardCapture(this, true);
			m_bInputFocus = true;
		}
	}

	if(mouse_action == WINDOW_LBUTTON_DOWN && !m_bInputFocus)
	{
		GetParent()->SetKeyboardCapture(this, true);
		m_bInputFocus = true;
//		m_lines.MoveCursorToEnd();
	}
	return false;
}


bool CUICustomEdit::OnKeyboard( int dik, EUIMessages keyboard_action )
{	
	if ( !m_bInputFocus )
	{
		return false;
	}

	if ( keyboard_action == WINDOW_KEY_PRESSED )
	{
		ec().on_key_press( dik );
		return true;
	}
	
	if ( keyboard_action == WINDOW_KEY_RELEASED )
	{
		ec().on_key_release( dik );
		return true;
	}
	return false;
}

bool  CUICustomEdit::OnKeyboardHold(int dik)
{
	if ( !m_bInputFocus )
	{
		return false;
	}

	ec().on_key_hold( dik );
	return true;
}

void CUICustomEdit::Update()
{
	ec().on_frame();

	if ( !ec().get_key_state( text_editor::ks_force ) )
	{
		m_last_key_state_time = Device.dwTimeGlobal;
	}
	else
	{
		if ( m_last_key_state_time + 7000 < Device.dwTimeGlobal ) // 7 sec
		{
			ec().reset_key_state();
		}
	}

	inherited::Update();
}

void  CUICustomEdit::Draw()
{
	VERIFY( m_pLines );

	Fvector2 pos, out;
	GetAbsolutePos( pos );
	CGameFont* font = m_pLines->m_pFont;
	
	if ( ec().need_update() || m_force_update )
	{
		float ui_width   = GetWidth();

		LPCSTR cursor_str   = ec().str_before_cursor();
		u32 cursor_str_size = xr_strlen( cursor_str );

		LPCSTR istr = cursor_str;
		float str_length = font->SizeOf_( istr );
		UI()->ClientToScreenScaledWidth( str_length );

		u32 ix = 0;
		while ( (str_length > ui_width) && (ix < cursor_str_size) )
		{
			istr = cursor_str + ix;
			str_length = font->SizeOf_( istr );
			UI()->ClientToScreenScaledWidth( str_length );
			++ix;
		}
		istr = cursor_str + ix;
		LPCSTR astr = ec().str_edit() + ix;
		u32 str_size = xr_strlen( ec().str_edit() );

		u32 jx = 1;
		strncpy_s( m_out_str, sizeof(m_out_str), astr, jx );

		str_length = font->SizeOf_( m_out_str );
		UI()->ClientToScreenScaledWidth( str_length );
		while ( (str_length < ui_width) && (jx < str_size-ix) )
		{
			strncpy_s( m_out_str, sizeof(m_out_str), astr, jx );
			str_length = font->SizeOf_( m_out_str );
			UI()->ClientToScreenScaledWidth( str_length );
			++jx;
		}
		strncpy_s( m_out_str, sizeof(m_out_str), astr, jx );

		inherited::SetText( m_out_str );
		m_dx_cur = font->SizeOf_( istr ); // cursor_str

		m_force_update = false;
	}

	inherited::Draw();

	if ( m_bInputFocus ) //draw cursor here
	{
		out.x = pos.x + 0.0f + GetTextX() + m_pLines->GetIndentByAlign();
		out.y = pos.y + 2.0f + GetTextY() + m_pLines->GetVIndentByAlign();
		UI()->ClientToScreenScaled( out );

		out.x += m_dx_cur; // cursor_str

		font->Out( out.x, out.y, "_" );
	}
	font->OnRender();
}

void CUICustomEdit::Show( bool status )
{
	ec().reset_key_state();
	m_force_update = true;
	inherited::Show( status );
}

void CUICustomEdit::ClearText()
{
	ec().set_edit( "" );
}

void CUICustomEdit::SetText(LPCSTR str)
{
	ec().set_edit( str );
}

LPCSTR CUICustomEdit::GetText()
{
	return ec().str_edit();
}

void CUICustomEdit::Enable(bool status)
{
	inherited::Enable( status );
	if ( !status )
	{
		SendMessage( this, WINDOW_KEYBOARD_CAPTURE_LOST );
	}
}

// =======================================================

void CUICustomEdit::nothing() {};

void CUICustomEdit::press_escape()
{
	if ( xr_strlen( ec().str_edit() ) != 0 )
	{
		if ( !m_read_mode )
		{
			ec().set_edit( "" );
		}
	}
	else
	{
		GetParent()->SetKeyboardCapture( this, false );
		m_bInputFocus = false;
	}
}

void CUICustomEdit::press_commit()
{
	m_bInputFocus = false;
	GetParent()->SetKeyboardCapture( this, false );
	GetMessageTarget()->SendMessage( this, EDIT_TEXT_COMMIT, NULL );
}
