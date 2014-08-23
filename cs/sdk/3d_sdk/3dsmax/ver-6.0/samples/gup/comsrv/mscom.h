//-----------------------------------------------------------------------------
// ----------------
// File ....: mscom.h
// ----------------
// Author...: Gus J Grubba
// Date ....: October 1995
// Descr....: MSCOM File I/O Module
//
// History .: Oct, 26 1995 - Started
//            
//
//
//-----------------------------------------------------------------------------
        
#ifndef _MSCOMCLASS_
#define _MSCOMCLASS_


#include <Max.h>
#include <bmmlib.h>
#include <guplib.h>

#define DLLEXPORT __declspec(dllexport)

#define _REGISTERCOM	_T("RegisterMAXRenderer")
#define _UNREGISTERCOM	_T("UnregisterMAXRenderer")

//-----------------------------------------------------------------------------
//-- Class Definition ---------------------------------------------------------
//


class GUP_MSCOM : public GUP {
    
	public:
     
		//-- Constructors/Destructors
        
				GUP_MSCOM		( );
				~GUP_MSCOM		( );
	
		//-- GUP Methods

		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );		
		void	DeleteThis		( );

		//JH 11/13/02 the material collections needs persistence for the scratch materials
		IOResult		Save		( ISave *isave );
		IOResult		Load		( ILoad *iload );

		//-- Private Control Methods (For Utility PlugIn Access)

		bool	IsCOMRegistered	( );
		bool	RegisterCOM		( );
		bool	UnRegisterCOM	( );

		//JH 9/15/02
		static IUnknown *GetAppObject	( );

private:
	unsigned long dwActiveCookie;
	static CComPtr<IUnknown> spOMAppUnk;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// MSCOM Class Description

class mClassDesc:public ClassDesc {
	
	public:
												
		int             IsPublic     ( );
		void           *Create       ( BOOL );
		const TCHAR    *ClassName    ( );
		SClass_ID       SuperClassID ( );
		Class_ID        ClassID      ( );
		const TCHAR    *Category     ( );

		//access to the single instance
		static GUP_MSCOM*	Instance();
		static void RevokeInstance();//Called from GUP_MSCOM::DeleteThis
	private:
		static GUP_MSCOM*	_instance;
		static ULONG mcRef;

};

#endif

//-- EOF: mscom.h -------------------------------------------------------------
