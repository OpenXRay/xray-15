/**********************************************************************
 *<
	FILE: PropertyTest.h

	DESCRIPTION:	Template Utility

	CREATED BY:

	HISTORY:

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#ifndef __PROPERTYTEST__H
#define __PROPERTYTEST__H

#include "Max.h"
#include "resource.h"
#include "utilapi.h"
#include "istdplug.h"

extern ClassDesc* GetPropertyTestDesc();

extern HINSTANCE hInstance;


class PropertyTest : public UtilityObj {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;

		PropertyTest();
		~PropertyTest();

		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);

		void ShowProperties();
		void GetProperties(HWND hDlg);

		void VariantToString(const PROPVARIANT* pProp, TCHAR* szString, int bufSize);
		void TypeNameFromVariant(const PROPVARIANT* pProp, TCHAR* szString, int bufSize);

		BOOL PropDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

};

// SummaryInfo properties
/* These are defined in objidl.h
#define PIDSI_TITLE				0x00000002
#define PIDSI_SUBJECT			0x00000003
#define PIDSI_AUTHOR			0x00000004
#define PIDSI_KEYWORDS			0x00000005
#define PIDSI_COMMENTS			0x00000006
*/

// Document SummaryInfo properties
#define PID_MANAGER				0x0000000E
#define PID_COMPANY				0x0000000F
#define PID_CATEGORY			0x00000002
#define PID_HEADINGPAIR			0x0000000C
#define PID_DOCPARTS			0x0000000D

#endif // __PROPERTYTEST__H
