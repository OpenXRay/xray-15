// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__F088EA6A_2E87_11D3_B1F3_00C0F03C37D3__INCLUDED_)
#define AFX_STDAFX_H__F088EA6A_2E87_11D3_B1F3_00C0F03C37D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

// Don't include the debug version of the CRT
#include <assert.h>
#define _ATL_NO_DEBUG_CRT
#define _ASSERT(x) assert((x))
#define ATLASSERT(x) assert((x))
#define _ASSERTE(x) assert((x))

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__F088EA6A_2E87_11D3_B1F3_00C0F03C37D3__INCLUDED)
