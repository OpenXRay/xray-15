
#include "stdafx.h"
#include "UICDkey.h"
#include "UILines.h"
#include "../../xrEngine/line_edit_control.h"
#include "../MainMenu.h"

#include "UIColorAnimatorWrapper.h"
#include "../../xrEngine/xr_IOConsole.h"
#include "../RegistryFuncs.h"
#include "../../xrGameSpy/xrGameSpy_MainDefs.h"
#include "player_name_modifyer.h"

extern string64	gsCDKey;

CUICDkey::CUICDkey()
{
	m_view_access = false;
	CreateCDKeyEntry();
	SetCurrentValue();
}

void CUICDkey::Show( bool status )
{
	inherited::Show( status );
	SetCurrentValue();
}

void CUICDkey::OnFocusLost()
{
	inherited::OnFocusLost();
	if(m_bInputFocus)
	{
		m_bInputFocus = false;
		GetMessageTarget()->SendMessage(this,EDIT_TEXT_COMMIT,NULL);
	}
	SaveValue();
}

void CUICDkey::Draw()
{
	LPCSTR  edt_str = ec().str_edit();
	u32    edt_size = xr_strlen( edt_str );

	if ( edt_size == 0 )
	{
		m_view_access = true;
	}
	
	//inherited::Draw();
	Frect						rect;
	GetAbsoluteRect				(rect);
	Fvector2					out;

	out.y						= (m_wndSize.y - m_pLines->m_pFont->CurrentHeight_())/2.0f;
	out.x						= 0.0f;
	m_pLines->m_pFont->SetColor	(m_pLines->GetTextColor());

	Fvector2					pos;
	pos.set						(rect.left+out.x, rect.top+out.y);
	UI()->ClientToScreenScaled	(pos);

	string64 xx_str = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	edt_size = xr_strlen( edt_str );
	if ( edt_size > 63 ) { edt_size = 63; }
	xx_str[edt_size] = 0;

	string64 xx_str1 = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	LPCSTR  edt_str1 = ec().str_before_cursor();
	u32    edt_size1 = xr_strlen( edt_str1 );
	if ( edt_size1 > 63 ) { edt_size1 = 63; }
	xx_str1[edt_size1] = 0;

	if ( m_bInputFocus )
	{		
		LPCSTR res  = ( m_view_access )? edt_str  : xx_str;
		LPCSTR res1 = ( m_view_access )? edt_str1 : xx_str1;

		m_pLines->m_pFont->Out	( pos.x, pos.y, "%s", CMainMenu::AddHyphens( res ) );
		
		float _h				= m_pLines->m_pFont->CurrentHeight_();
		UI()->ClientToScreenScaledHeight(_h);
		
		out.y					= rect.top + (m_wndSize.y - _h)/2.0f;
		
		float	w_tmp			= 0.0f;
		int i					= (int)xr_strlen( res1 );
		w_tmp					= m_pLines->m_pFont->SizeOf_( res1 );
		UI()->ClientToScreenScaledWidth( w_tmp );
		out.x					= rect.left + w_tmp;
		
		w_tmp					= m_pLines->m_pFont->SizeOf_("-");
		UI()->ClientToScreenScaledWidth( w_tmp );
		
		if(i>3)
			out.x	+= w_tmp;
		if(i>7)
			out.x	+= w_tmp;
		if(i>11)
			out.x	+= w_tmp;

		UI()->ClientToScreenScaled	(out);
		m_pLines->m_pFont->Out		(out.x, out.y, "_");
	}
	else
	{
		m_pLines->m_pFont->Out(pos.x, pos.y, "%s" , CMainMenu::AddHyphens(xx_str) );
	}
	m_pLines->m_pFont->OnRender();
}

LPCSTR CUICDkey::GetText()
{
	return CMainMenu::AddHyphens(inherited::GetText());
}

void CUICDkey::SetCurrentValue()
{
	string512	CDKeyStr;
	CDKeyStr[0] = 0;
	GetCDKey_FromRegistry(CDKeyStr);
	inherited::SetText( CMainMenu::DelHyphens(CDKeyStr) );
}

void CUICDkey::SaveValue()
{
	CUIOptionsItem::SaveValue();

	strcpy_s( gsCDKey, sizeof(gsCDKey), CMainMenu::AddHyphens(inherited::GetText()) );
	WriteCDKey_ToRegistry( gsCDKey );

	if ( MainMenu()->IsCDKeyIsValid() )
	{
		m_view_access = false;
	}
}

bool CUICDkey::IsChanged()
{
	string512	tmpCDKeyStr;
	GetCDKey_FromRegistry(tmpCDKeyStr);
	return 0 != xr_strcmp(tmpCDKeyStr, inherited::GetText());
}

void CUICDkey::CreateCDKeyEntry()
{
}

 //=================================================================

void CUIMPPlayerName::OnFocusLost()
{
	inherited::OnFocusLost();
	if ( m_bInputFocus )
	{
		m_bInputFocus = false;
		GetMessageTarget()->SendMessage(this, EDIT_TEXT_COMMIT, NULL);
	}
	string64 name;
	strcpy_s( name, GetText() );
	string256 new_name;
	modify_player_name(name, new_name);
	WritePlayerName_ToRegistry( new_name );
}

// -------------------------------------------------------------------------------------------------

void GetCDKey_FromRegistry(char* cdkey)
{
	ReadRegistry_StrValue(REGISTRY_VALUE_GSCDKEY, cdkey);
	if ( xr_strlen(cdkey) > 64 )
	{
		cdkey[64] = 0;
	}
}

void WriteCDKey_ToRegistry(LPSTR cdkey)
{
	if ( xr_strlen(cdkey) > 64 )
	{
		cdkey[64] = 0;
	}
	WriteRegistry_StrValue(REGISTRY_VALUE_GSCDKEY, cdkey);
}

void GetPlayerName_FromRegistry(char* name, u32 const name_size)
{
	string256	new_name;
	if (!ReadRegistry_StrValue(REGISTRY_VALUE_USERNAME, name))
	{
		name[0] = 0;
		Msg( "! Player name registry key (%s) not found !", REGISTRY_VALUE_USERNAME );
		return;
	}
	if ( xr_strlen(name) > 17 )
	{
		name[17] = 0;
	}
	if ( xr_strlen(name) == 0 )
	{
		Msg( "! Player name in registry is empty! (%s)", REGISTRY_VALUE_USERNAME );
	}
	modify_player_name(name, new_name);
	strncpy_s(name, name_size, new_name, 17);
}

void WritePlayerName_ToRegistry(LPSTR name)
{
	if ( xr_strlen(name) > 17 )
	{
		name[17] = 0;
	}
	WriteRegistry_StrValue(REGISTRY_VALUE_USERNAME, name);
}
