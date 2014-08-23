/****************************************************************************
 MAX File Finder
 Christer Janson
 September 19, 1998
 MaxFind.cpp - Main Window Proc Implementation
 ***************************************************************************/
#include "pch.h"

#include "app.h"
#include "..\..\..\include\buildver.h"
#include "resource.h"
#include "resourceOverride.h"

App* pApp = NULL;
#define	WM_INITAFTERCREATE	(WM_USER+42)

BOOL CommandHandler(HWND hWnd, int cmd, int code, HWND hControl);	// Handle WM_COMMAND messages

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpszCmdLine, int nCmdShow)
{
    MSG 	msg;
	BOOL	bRetVal = FALSE;
	HWND	hWnd;
	BOOL	done = FALSE;	

	pApp = new App(hInstance, nCmdShow);
	pApp->Init();

    hWnd = CreateWindowEx(
					WS_EX_CONTROLPARENT,
					THECLASSNAME,
					pApp->GetString(IDS_APPTITLE),
					WS_OVERLAPPEDWINDOW,
					pApp->regsettings.x, pApp->regsettings.y,
					pApp->regsettings.w, pApp->regsettings.h,
					NULL, NULL, hInstance, NULL);

	if (!hWnd) {
		char msg[128];
		strcpy(msg, pApp->GetString(IDS_FATAL_ERROR));
		MessageBox(NULL, pApp->GetString(IDS_NOWIN_ERROR), msg, MB_OK);
		return 0;
	}

	// My own post-creation initializations
	SendMessage(hWnd, WM_INITAFTERCREATE, 0, 0);

    ShowWindow(hWnd, nCmdShow);
    if (nCmdShow != SW_HIDE)
        UpdateWindow(hWnd);

	while (!done) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {
			if (GetMessage(&msg, NULL, 0,0 )) {
				if (!IsDialogMessage(pApp->GetPanel(), &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					}
			}
			else {
				done = TRUE;
			}
		}
		pApp->AppIsIdle(); 
	}

	pApp->SetRegSettings();

	return bRetVal;
}

LRESULT CALLBACK AppWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	char		buf[256];

//	sprintf(buf, "AppMsg: %08x\n", message);
//	OutputDebugString(buf);

    switch (message) {
    case WM_CREATE: {
        pApp->SetHWnd(hWnd);
        pApp->SetHMenu(GetMenu(hWnd));
		pApp->SetDC(GetDC(hWnd));
		pApp->CreateControls();
		DragAcceptFiles(hWnd, TRUE);
        HICON hIcon= LoadIcon(pApp->GetInstance(), MAKEINTRESOURCE(IDI_MAINWND));
        SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR) hIcon);
        
		return FALSE; }

    case WM_INITAFTERCREATE:
		pApp->PostInit();
		break;

	case WM_ENABLE:
		break;

	case WM_WINDOWPOSCHANGING: {
		WINDOWPOS *wp = (WINDOWPOS*)lParam;
		if (wp->cx<310) {
			wp->cx=310;
		}
		if (wp->cy<170) {
			wp->cy=170;
		}
		break;
		}
	case WM_MOVE:
		pApp->Move();
		break;

	case WM_SIZE:
		pApp->Resize(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

    case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, 0, buf, 256);
		DragFinish((HDROP)wParam);
		return 0;

    case WM_COMMAND:
		if (!CommandHandler(hWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam)) {
			return FALSE;
		}
		break;

    case WM_TIMER:
		break;

	case WM_LBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		break;

	case WM_MBUTTONDOWN:
		break;
	case WM_MBUTTONUP:
		break;

	case WM_RBUTTONDOWN:
		break;
	case WM_RBUTTONUP:
		break;

	case WM_MOUSEMOVE:
		break;

	case WM_CHAR:
		if((TCHAR)wParam != VK_ESCAPE)
			break;
		//pApp->statusPanel->setCancel(1);
		break;
	
	case WM_QUERYENDSESSION:
		return FALSE;

	case WM_ERASEBKGND:
		break;

	case WM_PAINT:
		break;

    case WM_CLOSE:
		pApp->AppIsClosing();
        DestroyWindow(hWnd);
        return TRUE;

    case WM_DESTROY:
		pApp->AppIsClosing();
        DragAcceptFiles(hWnd, 0);
        PostQuitMessage(0);
        return FALSE;

    default:
		break;
    }
    return (DefWindowProc(hWnd, message, wParam, lParam));
}

BOOL CommandHandler(HWND hWnd, int cmd, int code, HWND hControl)
{
	switch(cmd) {
		case ID_FILE_RESET:
			pApp->Reset();
			break;
		case ID_FILE_EXIT:
			pApp->AppIsClosing();
		    DragAcceptFiles(hWnd, 0);
		    PostQuitMessage(0);
			return FALSE;

		case ID_HELP_ABOUT:
			pApp->DoAboutBox();
			break;

		case IDC_START:
			pApp->DoFind();
			break;
		case IDC_BROWSE:
			pApp->DoCD();
			break;
		case IDC_FILESPEC:
			if (code == CBN_SELENDOK) {
				int idx = SendMessage(hControl, CB_GETCURSEL, 0, 0);
				pApp->regsettings.filespec = idx;
				}
			break;
		case IDC_LISTBOX:
			if (code == LBN_DBLCLK) {
				pApp->ViewFile();
				}
			break;
	}
	return TRUE;
}



LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	PAINTSTRUCT	ps;
//	char buf[256];

//	sprintf(buf, "StaticMsg: %08x\n", message);
//	OutputDebugString(buf);

    switch (message) {
	case WM_COMMAND:
		if (!CommandHandler(hWnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam)) {
			return FALSE;
		}
		break;
	case WM_PAINT: {
		RECT rect;

		BeginPaint(hWnd, &ps);

		HDC hDC = ps.hdc;
		GetClientRect(hWnd, &rect);
		rect.bottom -= 2;

		SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW)));
		MoveToEx(hDC,rect.left,rect.bottom,NULL);
		LineTo(hDC,rect.left,rect.top);
		LineTo(hDC,rect.right,rect.top);

		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHIGHLIGHT))));
		MoveToEx(hDC,rect.left+1,rect.bottom,NULL);
		LineTo(hDC,rect.left+1,rect.top+1);
		LineTo(hDC,rect.right,rect.top+1);

		SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHIGHLIGHT)));
		MoveToEx(hDC,rect.left+1,rect.bottom,NULL);
		LineTo(hDC,rect.right,rect.bottom);
		LineTo(hDC,rect.right,rect.top);

		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW))));
		MoveToEx(hDC,rect.left+1,rect.bottom-1,NULL);
		LineTo(hDC,rect.right-1,rect.bottom-1);
		LineTo(hDC,rect.right-1,rect.top-1);

		DeleteObject(SelectObject(hDC, GetStockObject(BLACK_PEN)));
		EndPaint(hWnd, &ps);
		}
		break;

    default:
		break;
    }
    return (DefWindowProc(hWnd, message, wParam, lParam));
}

INT_PTR CALLBACK statPanDlgProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
// WIN64 Cleanup: Martell
	{
//	char buf[256];
//	sprintf(buf, "StatPanMsg: %08x\n", message);
//	OutputDebugString(buf);

	switch (message) {
	case WM_COMMAND:
		if (CommandHandler(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam)) {
			return FALSE;
			}
		break;
		}
	return FALSE;
	}

TCHAR *App::GetString(int id)
{
	static TCHAR buf[256];

	if (GetInstance())
		return LoadString(GetInstance(), id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

