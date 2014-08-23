//-----------------------------------------------------------------------------
// -------------------
// File ....: comsrv.h
// -------------------
// Author...: Gus J Grubba
// Date ....: March 1997
// Descr....: IFL Manager Utility
//
// History .: Mar, 18 1997 - Started
//            
//-----------------------------------------------------------------------------

#ifndef __COMSRV__H
#define __COMSRV__H

#include <Max.h>
#include <utilapi.h>
#include <bmmlib.h>
#include <guplib.h>
#include "resource.h"

#define DLLEXPORT __declspec(dllexport)

#define ISCOMREGISTERED	comgup->Control(0)
#define REGISTERCOM		comgup->Control(1)
#define UNREGISTERCOM	comgup->Control(2)

//-----------------------------------------------------------------------------
//-- The Utility Class

class COMsrv : public UtilityObj {

	public:

		GUP*		comgup;

		IUtil*		iu;
		Interface*	ip;
		HWND		hPanel;		

				COMsrv();

		//--	Class Implementation
		
		void	BeginEditParams	( Interface *ip, IUtil *iu );
		void	EndEditParams	( Interface *ip, IUtil *iu );
		void	DeleteThis		( ) { ; }

		//--	Local Defined
		
		void	UpdateButton	( HWND hWnd );
		void	Init			( HWND hWnd );
		void	Destroy			( HWND hWnd );
		void	Register		( HWND hWnd );

};

#endif

//-- EOF: comsrv.h ------------------------------------------------------------
