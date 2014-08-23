// MaxClient.h : main header file for the MAXCLIENT application
//

#if !defined(AFX_MAXCLIENT_H__4E380C9F_5DB1_11D2_91CB_0060081C257E__INCLUDED_)
#define AFX_MAXCLIENT_H__4E380C9F_5DB1_11D2_91CB_0060081C257E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "..\..\..\..\include\buildver.h"
#include "resource.h"		// main symbols
#include "resourceOverride.h"

/////////////////////////////////////////////////////////////////////////////
// CMaxClientApp:
// See MaxClient.cpp for the implementation of this class
//

class CMaxClientApp : public CWinApp
{
public:
	CMaxClientApp();
	bool	IsTypeLibRegistered		( );
	bool	RegisterTypeLibrary		( );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaxClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMaxClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAXCLIENT_H__4E380C9F_5DB1_11D2_91CB_0060081C257E__INCLUDED_)
