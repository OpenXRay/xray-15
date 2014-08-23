//-----------------------------------------------------------------------------
// Notify.h : main header file for the NOTIFY application
//

#ifndef AFX_NOTIFY_H__25CB49C9_INCLUDED_
#define AFX_NOTIFY_H__25CB49C9_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include <alerts.h>
#include <mmsystem.h>

//-----------------------------------------------------------------------------
// INI File Constants

#define iniMAILTARGETS		"Sounds"
#define iniFAILURE			"Failure"
#define iniPROGRESS			"Progress"
#define iniCOMPLETION		"Completion"

//-----------------------------------------------------------------------------
// CNotifyApp:
// See Notify.cpp for the implementation of this class
//

class CNotifyApp : public CWinApp {

public:

			CNotifyApp		( );
			~CNotifyApp		( );
	
	CString	m_sINIname;

	CString failureTarget;
	CString progressTarget;
	CString completionTarget;
	
	DWORD	alertFlags;

	//{{AFX_VIRTUAL(CNotifyApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CNotifyApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif
