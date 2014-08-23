//-----------------------------------------------------------------------------
// -------------------
// File ....: iflmgr.h
// -------------------
// Author...: Gus J Grubba
// Date ....: March 1997
// Descr....: IFL Manager Utility
//
// History .: Mar, 18 1997 - Started
//            
//-----------------------------------------------------------------------------

#ifndef __IFLMGR__H
#define __IFLMGR__H

#include <Max.h>
#include <utilapi.h>
#include <bmmlib.h>
#include "resource.h"

#define DLLEXPORT __declspec(dllexport)

//-----------------------------------------------------------------------------
//-- List of Files

typedef struct tagSortTable {
	TCHAR file[MAX_PATH];
} SortTable;

//-----------------------------------------------------------------------------
//-- The Utility Class

class IFLMgr : public UtilityObj {

		TCHAR			root[MAX_PATH];
		TCHAR			iflfile[MAX_PATH];
		TCHAR			workpath[MAX_PATH];
		
		Tab<SortTable>	table;

		void	ToggleControl	( HWND hWnd, int control, BOOL state );
		BOOL	EditIFL			( HWND hWnd, TCHAR *filename );
		void	ExtractRoot		( TCHAR *filename );
		int		BuildFileList	( HWND hWnd, const TCHAR *mask );
		void	InitSpinner		( HWND hWnd, int ed, int sp, int v, int min, int max );
		void	DestroySpinner	( HWND hWnd, int sp );
		int		GetSpinnerValue	( HWND hWnd, int sp );

	public:

		IUtil*		iu;
		Interface*	ip;
		HWND		hPanel;		

				IFLMgr();

		//--	Class Implementation
		
		void	BeginEditParams	( Interface *ip, IUtil *iu );
		void	EndEditParams	( Interface *ip, IUtil *iu );
		void	DeleteThis		( ) {}

		//--	Local Defined
		
		void	Init			( HWND hWnd );
		void	Destroy			( HWND hWnd );
		void	Create			( HWND hWnd );
		void	Select			( HWND hWnd );
		void	Edit			( HWND hWnd );

};

#endif

//-- EOF: iflmgr.h ------------------------------------------------------------
