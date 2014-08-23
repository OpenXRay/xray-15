/****************************************************************************
 MAX File Finder
 Christer Janson
 September 19, 1998
 App.cpp - Main Application and Search Methods
 ***************************************************************************/
#include "pch.h"

#include "app.h"
#include "resource.h"
#include "..\..\..\include\buildver.h"

LRESULT	CALLBACK AppWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT	CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK statPanDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
// WIN64 Cleanup: Martell
BOOL BrowseFolder(HINSTANCE hInstance, HWND hParent);

static char* useFolder;

App::App(HINSTANCE hInst, int cshow)
{
	hInstance = hInst;
	cmdShow = cshow;
	hPropDialog = NULL;
	InitializeCriticalSection(&cs);
	bSearchActive = FALSE;
	hThreadExitRequest = CreateEvent(NULL, TRUE, FALSE, NULL);
}

App::~App()
	{
	DeleteCriticalSection(&cs);
	CloseHandle(hThreadExitRequest);
	if (hFont) DeleteObject(hFont);
	}

BOOL App::Init()
	{
	WNDCLASSEX	wc;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = AppWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
//	wc.hIconSm		 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINWND));
//	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINWND));
	wc.hIconSm		 = NULL;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL; //(HBRUSH)(COLOR_MENU+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = THECLASSNAME;

	if (!RegisterClassEx(&wc))
		return FALSE;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = StaticWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIconSm		 = NULL;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_MENU+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "CJSTATIC";

	if (!RegisterClassEx(&wc))
		return FALSE;

	LOGFONT lf;
	GetObject(GetStockObject(SYSTEM_FONT), sizeof(lf), &lf);
	lf.lfWeight = 400;
	lf.lfHeight = 14;
	lf.lfWidth = 0;
	strcpy((char *)&lf.lfFaceName, GetString(IDS_APP_FONT)); // mjm - 2.10.99 - for localization
//	strcpy((char *)&lf.lfFaceName, "MS Sans Serif");
	hFont = CreateFontIndirect(&lf);

	GetRegSettings();

	return TRUE;
	}

void App::GetRegSettings()
	{
	HKEY	hKey;

	regsettings.x = CW_USEDEFAULT;
	regsettings.y = CW_USEDEFAULT;
	regsettings.w = 500;
	regsettings.h = 500;
	regsettings.filespec = 0;
	regsettings.propdlgx = 100;
	regsettings.propdlgy = 100;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitX", 0, 0, (LPBYTE)&regsettings.x, &nSize);
		nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitY", 0, 0, (LPBYTE)&regsettings.y, &nSize);
		nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitW", 0, 0, (LPBYTE)&regsettings.w, &nSize);
		nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitH", 0, 0, (LPBYTE)&regsettings.h, &nSize);
		nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitFileSpec", 0, 0, (LPBYTE)&regsettings.filespec, &nSize);
		nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitPropDlgX", 0, 0, (LPBYTE)&regsettings.propdlgx, &nSize);
		nSize = sizeof(DWORD);
		RegQueryValueEx(hKey, "InitPropDlgY", 0, 0, (LPBYTE)&regsettings.propdlgy, &nSize);
		}
	}

void App::SetRegSettings()
	{
	HKEY	hKey;
	DWORD	disp;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp) == ERROR_SUCCESS) {
		RegSetValueEx(hKey, "InitX", 0, REG_DWORD, (LPBYTE)&regsettings.x, sizeof(DWORD));
		RegSetValueEx(hKey, "InitY", 0, REG_DWORD, (LPBYTE)&regsettings.y, sizeof(DWORD));
		RegSetValueEx(hKey, "InitW", 0, REG_DWORD, (LPBYTE)&regsettings.w, sizeof(DWORD));
		RegSetValueEx(hKey, "InitH", 0, REG_DWORD, (LPBYTE)&regsettings.h, sizeof(DWORD));
		RegSetValueEx(hKey, "InitFileSpec", 0, REG_DWORD, (LPBYTE)&regsettings.filespec, sizeof(DWORD));
		RegSetValueEx(hKey, "InitPropDlgX", 0, REG_DWORD, (LPBYTE)&regsettings.propdlgx, sizeof(DWORD));
		RegSetValueEx(hKey, "InitPropDlgY", 0, REG_DWORD, (LPBYTE)&regsettings.propdlgy, sizeof(DWORD));
		}
	}


void App::PostInit()
	{
	// russom - 06/21/01
	// we don't have access to resmgr - ugh.
	// Product specific code here:
#if defined(GAME_VER)
	// Add .gmax for both free and dev
	SendMessage(hFileSpec, CB_ADDSTRING, 0, (LPARAM)"*.gmax");
	// If this is dev (not free), also add max
  #if !defined(GAME_FREE_VER)
	SendMessage(hFileSpec, CB_ADDSTRING, 0, (LPARAM)"*.max");
  #endif
#else
	SendMessage(hFileSpec, CB_ADDSTRING, 0, (LPARAM)"*.max");
#endif
	SendMessage(hFileSpec, CB_SETCURSEL, regsettings.filespec, 0);
	PostMessage(hFileSpec, CB_SETEDITSEL, 0, MAKELPARAM(-1, 0));

	int idx;
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_ALL));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)ALL_PROPERTIES);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_TITLE));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)TITLE_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_SUBJECT));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)SUBJECT_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_AUTHOR));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)AUTHOR_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_MANAGER));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)MANAGER_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_COMPANY));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)COMPANY_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_CATEGORY));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)CATEGORY_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_KEYWORDS));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)KEYWORDS_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_COMMENTS));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)COMMENTS_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_EXTDEP));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)EXT_DEPEND_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_PLUGINS));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)PLUGINS_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_OBJECTS));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)OBJECTS_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_MATERIALS));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)MATERIALS_PROP);
	idx = SendMessage(hProperty, CB_ADDSTRING, 0, (LPARAM)GetString(IDS_PROP_CUSTOM));
		SendMessage(hProperty, CB_SETITEMDATA, idx, (LPARAM)USER_PROP);
	
	SendMessage(hProperty, CB_SETCURSEL, 0, 0);

	SendMessage(hCheckSubdirs, BM_SETCHECK, BST_CHECKED, 0);

	SetFocus(hSearchText);
	DoStatusDirectory();
	}

void App::CreateControls()
	{
	hControlPane = CreateDialogParam(hInstance,
				MAKEINTRESOURCE(IDD_PANEL),
				hMainWnd,
				statPanDlgProc,
				(LPARAM)this);

	hTxtSearch		= GetDlgItem(hControlPane, IDC_TXTSEARCH);
	hTxtFileSpec	= GetDlgItem(hControlPane, IDC_TXTFILESPEC);
	hTxtProperty	= GetDlgItem(hControlPane, IDC_TXTPROPERTY);
	hFindButton		= GetDlgItem(hControlPane, IDC_START);
	hCDButton		= GetDlgItem(hControlPane, IDC_BROWSE);
	hFileSpec		= GetDlgItem(hControlPane, IDC_FILESPEC);
	hProperty		= GetDlgItem(hControlPane, IDC_PROPERTY);
	hSearchText		= GetDlgItem(hControlPane, IDC_SEARCHEDIT);
	hCheckSubdirs	= GetDlgItem(hControlPane, IDC_SUBDIRS);
	hStatusPanel	= GetDlgItem(hControlPane, IDC_STATUSPANEL);
	hDlgFrame		= GetDlgItem(hControlPane, IDC_BORDER);

	hListBox = CreateWindow("LISTBOX",
				 "",
				 WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP | LBS_NOTIFY,
				 0, 0,
				 0, 0,
				 hMainWnd,
				 (HMENU)IDC_LISTBOX,
				 hInstance,
				 NULL);

	SendMessage(hListBox, WM_SETFONT, (WPARAM)GetAppFont(), 0);

	}

void App::Move()
	{
	RECT	wr;
	GetWindowRect(hMainWnd, &wr);

	regsettings.x = wr.left;
	regsettings.y = wr.top;
	regsettings.w = wr.right-wr.left;
	regsettings.h = wr.bottom-wr.top;
	}

void App::Resize(int flags, int width, int height)
	{
	RECT	r, wr;
	GetClientRect(hMainWnd, &r);
	GetWindowRect(hMainWnd, &wr);

	regsettings.x = wr.left;
	regsettings.y = wr.top;
	regsettings.w = wr.right-wr.left;
	regsettings.h = wr.bottom-wr.top;

	int		editWidth = (r.right)/3;


	int		btnHeight = 20;
	int		editHeight = 23;
	int		txtHeight = 15;
	int		statusHeight = 16;

	int		toolbarHeight = 91;

	int		startFindBtn = r.left+5;
	int		widthFindBtn = 70;
	int		startCDBtn = startFindBtn+widthFindBtn+15;
	int		widthCDBtn = 70;
	int		startSubdir = startCDBtn+widthCDBtn+15;

	MoveWindow(hControlPane, r.left, 0, r.right, toolbarHeight, TRUE);
	MoveWindow(hDlgFrame, r.left, 0, r.right, toolbarHeight, TRUE);

	MoveWindow(hTxtSearch, r.left+5, 5, editWidth-10, txtHeight, TRUE);
	MoveWindow(hTxtFileSpec, editWidth, 5, editWidth-10, txtHeight, TRUE);
	MoveWindow(hTxtProperty, 2*editWidth, 5, editWidth-10, txtHeight, TRUE);

	MoveWindow(hSearchText, r.left+5, txtHeight+4, editWidth-10, editHeight-1, TRUE);
	MoveWindow(hFileSpec, editWidth, txtHeight+5, editWidth-10, editHeight+200, TRUE);
	MoveWindow(hProperty, 2*editWidth, txtHeight+5, editWidth-10, editHeight+200, TRUE);


	MoveWindow(hFindButton, startFindBtn, editHeight+txtHeight+10, widthFindBtn, btnHeight, TRUE);
	MoveWindow(hCDButton, startCDBtn, editHeight+txtHeight+10, widthCDBtn, btnHeight, TRUE);
	MoveWindow(hCheckSubdirs, startSubdir, editHeight+txtHeight+10, 120, 18, TRUE);
	
	MoveWindow(hStatusPanel, r.left+5, toolbarHeight-statusHeight-4, r.right-10, statusHeight, TRUE);

	MoveWindow(hListBox, r.left, r.top+toolbarHeight, r.right, r.bottom-toolbarHeight, TRUE);
	}

void App::Reset()
	{
	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	}

void App::AppIsIdle()
	{
	WaitMessage();
	}

HINSTANCE App::GetInstance()
	{
	return hInstance;
	}

void App::SetHWnd(HWND wnd)
	{
	hMainWnd = wnd;
	}

HWND App::GetHWnd()
	{
	return hMainWnd;
	}

HDC App::GetDC()
	{
	return hMainWndDC;
	}

void App::SetDC(HDC dc)
	{
	hMainWndDC = dc;
	}

void App::SetHMenu(HMENU menu)
	{
	hMenu = menu;
	}

HMENU App::GetHMenu()
	{
	return hMenu;
	}

HFONT App::GetAppFont()
	{
	return hFont;
	}


void App::DoCD()
	{
	BROWSEINFO	browseInfo;
	char		dir[_MAX_PATH];
	int			image = 0;

	browseInfo.hwndOwner = GetHWnd();
    browseInfo.pidlRoot = NULL;
    browseInfo.pszDisplayName = dir;
    browseInfo.lpszTitle = GetString(IDS_SELFOLDERTITLE);
    browseInfo.ulFlags = 0;
    browseInfo.lpfn = NULL;
    browseInfo.lParam = NULL;
    browseInfo.iImage = image;

	ITEMIDLIST* item = SHBrowseForFolder(&browseInfo);
	if (item) {
		SHGetPathFromIDList(item, dir);
		if (strcmp(dir, "")) {
			SetCurrentDirectory(dir);
			DoStatusDirectory();
			}
		}
	}


// From MSJ, August 94, p. 70
BOOL CenterWindow(HWND hWndChild, HWND hWndParent)
{
	RECT	rChild, rParent;
	int		wChild, hChild, wParent, hParent;
	int		wScreen, hScreen, xNew, yNew;
	HDC		hdc = GetDC(hWndChild);

	// get the display limits
	wScreen = GetDeviceCaps(hdc, HORZRES);
	hScreen = GetDeviceCaps(hdc, VERTRES);

	// Get the Height and Width of the parent window
	if(hWndParent)
		GetWindowRect(hWndParent, &rParent);
	else {
		rParent.left = 0;
		rParent.right = wScreen;
		rParent.top = 0;
		rParent.bottom = hScreen;
	}
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	// get the Height and Width of the child window
	GetWindowRect(hWndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	// calculate new X position, then adjust for screen
	xNew = rParent.left + ((wParent - wChild) / 2);
	if(xNew < 0)
		xNew = 0;
	else if ((xNew + wChild) > wScreen)
		xNew = wScreen - wChild;

	// calculate new Y position, then adjust for screen
	yNew = rParent.top + ((hParent - hChild) / 2);
	if(yNew < 0)
		yNew = 0;
	else if((yNew + hChild) > hScreen)
		yNew = hScreen - hChild;

	ReleaseDC(hWndChild, hdc);
	// set it, and return
	return SetWindowPos(hWndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void App::DoStatusDirectory()
	{
	char dir[_MAX_PATH];
	char status[_MAX_PATH+32];

	GetCurrentDirectory(_MAX_PATH, dir);
	strcpy(status, GetString(IDS_CURDIR));
	strcat(status, dir);
	SendMessage(hStatusPanel, WM_SETTEXT, 0, (LPARAM)status);
	}


void App::EnableUI(BOOL status)
	{
	EnableWindow(hCDButton, status);
	EnableWindow(hSearchText, status);
	EnableWindow(hFileSpec, status);
	EnableWindow(hProperty, status);
	EnableWindow(hCheckSubdirs, status);

	SendMessage(hFindButton, WM_SETTEXT, 0, (LPARAM)(status ? GetString(IDS_BTNSTART) : GetString(IDS_BTNCANCEL)));
	}


BOOL App::ScanDirectory(char* path, char* filespec, HWND hList)
	{
	WIN32_FIND_DATA	findData;
	char*	dirSpec = "*";
	char	buf[_MAX_PATH];
	char	filespecbuf[_MAX_PATH];
	char	curDir[_MAX_PATH+32];
	BOOL	bQuitRequest = FALSE;
	HANDLE	hFind;
	char*	pathPtr;
	char*	pathToken;


	GetCurrentDirectory(_MAX_PATH, curDir);

	strcpy(buf, GetString(IDS_SEARCHDIR));
	strcat(buf, path);

	SendMessage(hStatusPanel, WM_SETTEXT, 0, (LPARAM)buf);

	strcpy(filespecbuf, filespec);
	pathPtr = filespecbuf;

	while ((pathToken = strtok(pathPtr, ";, "))) {
		pathPtr = NULL;

		strcpy(buf, path);
		if (path[strlen(path)-1] != '\\')
			strcat(buf, "\\");

		strcat(buf, pathToken);

		hFind = FindFirstFile(buf, &findData);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (WaitForSingleObject(hThreadExitRequest, 0) == WAIT_OBJECT_0) {
					bQuitRequest = TRUE;
					break;
					}
				if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					!(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
					!(findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
					!(findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)) {

					strcpy(buf, path);
					if (path[strlen(path)-1] != '\\')
						strcat(buf, "\\");
					strcat(buf, findData.cFileName);
					if (Qualify(buf)) {
						SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buf);
						}
					}
				}while (FindNextFile(hFind, &findData));
			}

		FindClose(hFind);

		if (bQuitRequest) {
			return FALSE;
			}
		}

	if (SendMessage(hCheckSubdirs, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		strcpy(buf, path);
		if (path[strlen(path)-1] != '\\')
			strcat(buf, "\\");

		strcat(buf, dirSpec);

		hFind = FindFirstFile(buf, &findData);
		if (hFind == INVALID_HANDLE_VALUE) return TRUE;
		do {
			if (WaitForSingleObject(hThreadExitRequest, 0) == WAIT_OBJECT_0) {
				bQuitRequest = TRUE;
				break;
				}

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (strcmp(findData.cFileName, ".") &&
					strcmp(findData.cFileName, "..")) {
					strcpy(buf, path);
					if (path[strlen(path)-1] != '\\')
						strcat(buf, "\\");
					strcat(buf, findData.cFileName);
					if (!ScanDirectory(buf, filespec, hList)) {
						bQuitRequest = TRUE;
						break;
						}
					}
				}
			}while (!bQuitRequest && FindNextFile(hFind, &findData));
		FindClose(hFind);
		}

	return !bQuitRequest;
	}

DWORD WINAPI StartFinder(void* lparg)
	{
	char path[_MAX_PATH];
	char filespec[_MAX_PATH];
	App*	pApp = (App*)lparg;

	pApp->bSearchActive = TRUE;

	pApp->EnableUI(FALSE);
	pApp->Reset();
	GetCurrentDirectory(_MAX_PATH, path);
	SendMessage(pApp->GetFileSpecDropDown(), WM_GETTEXT, _MAX_PATH, (LPARAM)filespec);

	pApp->ScanDirectory(path, filespec, pApp->GetMainListBox());

	pApp->DoStatusDirectory();
	pApp->EnableUI(TRUE);
	pApp->bSearchActive = FALSE;
	return 0;
	}

void App::DoFind()
	{
	if (!bSearchActive) {
		ResetEvent(hThreadExitRequest);
		CreateThread(NULL, 0, StartFinder, this, 0, &wWorkerThread);
		}
	else {
		SetEvent(hThreadExitRequest);
		}
	}

void App::ViewFile()
	{
	ShowProperties();
	}

BOOL App::Qualify(char* filename)
	{
	LPSTORAGE				pStorage = NULL;
	IPropertySetStorage*	pPropertySetStorage = NULL;
	IPropertyStorage*		pSummaryInfoStorage = NULL;
	IPropertyStorage*		pDocumentSummaryInfoStorage = NULL;
	IPropertyStorage*		pUserDefinedPropertyStorage = NULL;
	wchar_t					wfilename[_MAX_PATH];
	char					szSearchString[_MAX_PATH];

	int						propFlags = 0;

	int idx = SendMessage(hProperty, CB_GETCURSEL, 0, 0);
	if (idx == CB_ERR) {
		return FALSE;
		}

	propFlags = SendMessage(hProperty, CB_GETITEMDATA, idx, 0);

	SendMessage(hSearchText, WM_GETTEXT, _MAX_PATH, (LPARAM)szSearchString);

	if ((propFlags == ALL_PROPERTIES) && (strcmp(szSearchString, "") == 0)) {
		// Qualify all files if we don't search for anything
		return TRUE;
		}

	MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, _MAX_PATH);
	HRESULT	res = StgOpenStorage(wfilename, (LPSTORAGE)0, STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE,	NULL,0,&pStorage);
	if (res!=S_OK) {
		return FALSE;
		}


	// Get the Storage interface
	if (S_OK != pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&pPropertySetStorage)) {
		pStorage->Release();
		return FALSE;
		}

	// Get the SummaryInfo property set interface
	if (propFlags & PROPSET_SUMINFO) {
		if (S_OK == pPropertySetStorage->Open(FMTID_SummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pSummaryInfoStorage)) {
			BOOL bFound = FALSE;

			PROPSPEC	PropSpec[5];
			PROPVARIANT	PropVar[5];

			PropSpec[0].ulKind = PRSPEC_PROPID;
			PropSpec[0].propid = PID_TITLE;

			PropSpec[1].ulKind = PRSPEC_PROPID;
			PropSpec[1].propid = PID_SUBJECT;

			PropSpec[2].ulKind = PRSPEC_PROPID;
			PropSpec[2].propid = PID_AUTHOR;

			PropSpec[3].ulKind = PRSPEC_PROPID;
			PropSpec[3].propid = PID_KEYWORDS;

			PropSpec[4].ulKind = PRSPEC_PROPID;
			PropSpec[4].propid = PID_COMMENTS;

			HRESULT hr = pSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
			if (S_OK == hr) {
				if ((propFlags & TITLE_PROP) && (PropVar[0].vt == VT_LPSTR)) {
					if (Compare(PropVar[0].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				if (!bFound && (propFlags & SUBJECT_PROP) && (PropVar[1].vt == VT_LPSTR)) {
					if (Compare(PropVar[1].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				if (!bFound && (propFlags & AUTHOR_PROP) && (PropVar[2].vt == VT_LPSTR)) {
					if (Compare(PropVar[2].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				if (!bFound && (propFlags & KEYWORDS_PROP) && (PropVar[3].vt == VT_LPSTR)) {
					if (Compare(PropVar[3].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				if (!bFound && (propFlags & COMMENTS_PROP) && (PropVar[4].vt == VT_LPSTR)) {
					if (Compare(PropVar[4].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}
				}

			FreePropVariantArray(5, PropVar);

			pSummaryInfoStorage->Release();
			if (bFound) {

				pPropertySetStorage->Release();
				pStorage->Release();
				return TRUE;
				}
			}
		}

	// Get the DocumentSummaryInfo property set interface
	if (propFlags & PROPSET_DOCSUMINFO) {
		if (S_OK == pPropertySetStorage->Open(FMTID_DocSummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pDocumentSummaryInfoStorage)) {
			BOOL bFound = FALSE;

			PROPSPEC	PropSpec[5];
			PROPVARIANT	PropVar[5];

			PropSpec[0].ulKind = PRSPEC_PROPID;
			PropSpec[0].propid = PID_MANAGER;

			PropSpec[1].ulKind = PRSPEC_PROPID;
			PropSpec[1].propid = PID_COMPANY;

			PropSpec[2].ulKind = PRSPEC_PROPID;
			PropSpec[2].propid = PID_CATEGORY;

			PropSpec[3].ulKind = PRSPEC_PROPID;
			PropSpec[3].propid = PID_HEADINGPAIR;

			PropSpec[4].ulKind = PRSPEC_PROPID;
			PropSpec[4].propid = PID_DOCPARTS;

			HRESULT hr = pDocumentSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
			if (S_OK == hr) {
				if ((propFlags & MANAGER_PROP) && (PropVar[0].vt == VT_LPSTR)) {
					if (Compare(PropVar[0].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				if (!bFound && (propFlags & COMPANY_PROP) && (PropVar[1].vt == VT_LPSTR)) {
					if (Compare(PropVar[1].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				if (!bFound && (propFlags & CATEGORY_PROP) && (PropVar[2].vt == VT_LPSTR)) {
					if (Compare(PropVar[2].pszVal, szSearchString)) {
						bFound = TRUE;
						}
					}

				// Scan through document contents
				if (!bFound && (PropVar[3].vt == (VT_VARIANT | VT_VECTOR)) && (PropVar[4].vt == (VT_LPSTR | VT_VECTOR))) {
					CAPROPVARIANT*	pHeading = &PropVar[3].capropvar;
					CALPSTR*		pDocPart = &PropVar[4].calpstr;

					// Headings:
					// =========
					// 0  - General
					// 2  - Mesh Totals
					// 4  - Scene Totals
					// 6  - External Dependencies
					// 8  - Objects
					// 10 - Materials
					// 12 - Plug-Ins
					int nDocPart = 0;
					for (UINT i=0; i<pHeading->cElems; i+=2) {
						BOOL bCompare = FALSE;

						if ((i==6) && (propFlags & EXT_DEPEND_PROP)) bCompare = TRUE;
						if ((i==8) && (propFlags & OBJECTS_PROP)) bCompare = TRUE;
						if ((i==10) && (propFlags & MATERIALS_PROP)) bCompare = TRUE;
						if ((i==12) && (propFlags & PLUGINS_PROP)) bCompare = TRUE;

						for (int j=0; j<pHeading->pElems[i+1].lVal; j++) {
							if (!bFound && bCompare) {
								if (Compare(pDocPart->pElems[nDocPart], szSearchString)) {
									bFound = TRUE;
									}
								}
							nDocPart++;
							}
						}
					}
				}

			FreePropVariantArray(5, PropVar);

			pDocumentSummaryInfoStorage->Release();
			if (bFound) {
				pPropertySetStorage->Release();
				pStorage->Release();
				return TRUE;
				}
			}
		}

	// Get the User Defined property set interface
	if (propFlags & PROPSET_USERDEF) {
		if (S_OK == pPropertySetStorage->Open(FMTID_UserDefinedProperties, STGM_READ|STGM_SHARE_EXCLUSIVE, &pUserDefinedPropertyStorage)) {
			BOOL	bFound = FALSE;
			int		numUserProps = 0;

			// First we need to count the properties
			IEnumSTATPROPSTG*	pIPropertyEnum;
			if (S_OK == pUserDefinedPropertyStorage->Enum(&pIPropertyEnum)) {
				STATPROPSTG property;
				while (pIPropertyEnum->Next(1, &property, NULL) == S_OK) {
					if (property.lpwstrName) {
						CoTaskMemFree(property.lpwstrName);
						property.lpwstrName = NULL;
						numUserProps++;
						}
					}

				PROPSPEC* pPropSpec = new PROPSPEC[numUserProps];
				PROPVARIANT* pPropVar = new PROPVARIANT[numUserProps];

				ZeroMemory(pPropVar, numUserProps*sizeof(PROPVARIANT));
				ZeroMemory(pPropSpec, numUserProps*sizeof(PROPSPEC));

				pIPropertyEnum->Reset();
				int idx = 0;
				while (pIPropertyEnum->Next(1, &property, NULL) == S_OK) {
					if (property.lpwstrName) {
						pPropSpec[idx].ulKind = PRSPEC_LPWSTR;
						pPropSpec[idx].lpwstr = (LPWSTR)CoTaskMemAlloc(sizeof(wchar_t)*(wcslen(property.lpwstrName)+1));
						wcscpy(pPropSpec[idx].lpwstr, property.lpwstrName);
						idx++;
						CoTaskMemFree(property.lpwstrName);
						property.lpwstrName = NULL;
						}
					}
				pIPropertyEnum->Release();

				HRESULT hr = pUserDefinedPropertyStorage->ReadMultiple(idx, pPropSpec, pPropVar);
				if (S_OK == hr) {
					for (int i=0; i<idx; i++) {
						if (pPropVar[i].vt == VT_LPSTR) {
							if (Compare(pPropVar[i].pszVal, szSearchString)) {
								bFound = TRUE;
								break;
								}
							}
						else if (pPropVar[i].vt == VT_LPWSTR) {
							int nSize = wcslen(pPropVar[i].pwszVal)+1;
							char* tempVal = (char*)malloc(nSize);
							if (tempVal) {
								WideCharToMultiByte(CP_ACP, 0, pPropVar[i].pwszVal, -1, tempVal, nSize, NULL, NULL);
								if (Compare(tempVal, szSearchString)) {
									bFound = TRUE;
									free(tempVal);
									break;
									}
								free(tempVal);
								}
							}
						}
					}

				for (int i=0; i<idx; i++) {
					CoTaskMemFree(pPropSpec[i].lpwstr);
					}

				FreePropVariantArray(numUserProps, pPropVar);

				delete [] pPropSpec;
				delete [] pPropVar;
				}

			pUserDefinedPropertyStorage->Release();
			if (bFound) {
				pPropertySetStorage->Release();
				pStorage->Release();
				return TRUE;
				}
			}
		}

	pPropertySetStorage->Release();
	pStorage->Release();

	return FALSE;
	}


BOOL App::Compare(char* s1, char* s2)
	{
	if (strcmp(s1, "")==0) return FALSE;

	char* lcs1 = (char*)malloc(strlen(s1)+1);
	char* lcs2 = (char*)malloc(strlen(s2)+1);

	if (!lcs1 && !lcs2) return FALSE;

	strcpy(lcs1, s1);
	strcpy(lcs2, s2);

	strlwr(lcs1);
	strlwr(lcs2);

	BOOL bFound = strstr(lcs1, lcs2) != NULL;

	free(lcs1);
	free(lcs2);

	if (bFound)
		return TRUE;

	return FALSE;
	}

