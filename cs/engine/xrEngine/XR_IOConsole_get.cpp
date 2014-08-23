////////////////////////////////////////////////////////////////////////////
//	Module 		: XR_IOConsole_get.cpp
//	Created 	: 17.05.2008
//	Author		: Evgeniy Sokolov
//	Description : Console`s get-functions class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XR_IOConsole.h"
#include "xr_ioc_cmd.h"


bool CConsole::GetBool( LPCSTR cmd )
{
	vecCMD_IT it = Commands.find( cmd );
	if ( it == Commands.end() )
	{
		return false; // default
	}

	IConsole_Command* cc = it->second;
	CCC_Mask* cf = dynamic_cast<CCC_Mask*>(cc);
	if ( cf )
	{
		return ( cf->GetValue() != 0 );
	}

	CCC_Integer* ci = dynamic_cast<CCC_Integer*>(cc);
	if ( ci )
	{
		return ( ci->GetValue() != 0 );
	}
	return false;
}

float CConsole::GetFloat( LPCSTR cmd, float& min, float& max )
{
	min = 0.0f;
	max = 0.0f;
	vecCMD_IT it = Commands.find( cmd );
	if ( it == Commands.end() )
	{
		return 0.0f;
	}

	IConsole_Command* cc = it->second;
	CCC_Float* cf = dynamic_cast<CCC_Float*>(cc);
	if ( cf )
	{
		min = cf->GetMin();
		max = cf->GetMax();
		return cf->GetValue(); 
	}
	return 0.0f;
}

int CConsole::GetInteger( LPCSTR cmd, int& min, int& max )
{
	min = 0;
	max = 1;
	vecCMD_IT it = Commands.find( cmd );
	if ( it == Commands.end() )
	{
		return 0;	
	}

	IConsole_Command* cc = it->second;
	CCC_Integer* cf = dynamic_cast<CCC_Integer*>(cc);
	if ( cf )
	{
		min = cf->GetMin();
		max = cf->GetMax();
		return cf->GetValue();
	}
	CCC_Mask* cm = dynamic_cast<CCC_Mask*>(cc);
	if ( cm )
	{
		min = 0;
		max = 1;
		return ( cm->GetValue() )? 1 : 0;
	}
	return 0;
}

LPCSTR CConsole::GetString( LPCSTR cmd )
{
	vecCMD_IT it = Commands.find( cmd );
	if ( it == Commands.end() )
	{
		return NULL;
	}

	static IConsole_Command::TStatus stat;
	IConsole_Command* cc = it->second;
	cc->Status( stat );
	return stat;
}

LPCSTR CConsole::GetToken( LPCSTR cmd )
{
	return GetString( cmd );
}

xr_token* CConsole::GetXRToken( LPCSTR cmd )
{
	vecCMD_IT it = Commands.find( cmd );
	if ( it == Commands.end() )
	{
		return NULL;
	}

	IConsole_Command* cc = it->second;
	CCC_Token* cf = dynamic_cast<CCC_Token*>(cc);
	if ( cf )
	{
		return cf->GetToken();
	}
	return NULL;
}

Fvector* CConsole::GetFVectorPtr( LPCSTR cmd )
{
	vecCMD_IT it = Commands.find( cmd );
	if ( it == Commands.end() )
	{
		return NULL;
	}
	IConsole_Command* cc = it->second;
	CCC_Vector3* cf = dynamic_cast<CCC_Vector3*>(cc);
	if ( cf )
	{
		return cf->GetValuePtr();
	}
	return NULL;
}

Fvector CConsole::GetFVector( LPCSTR cmd )
{
	Fvector* pV = GetFVectorPtr( cmd );
	if ( pV )
	{
		return *pV;
	}
	return Fvector().set( 0.0f, 0.0f, 0.0f );
}
