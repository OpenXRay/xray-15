/****************************************************************************
 MAX File Finder
 Christer Janson
 September 19, 1998
 App.h - Main Application header
 ***************************************************************************/
#include "..\..\..\include\buildver.h"
#define THECLASSNAME	"CJMAXFINDER_WNDCLASS"

#ifndef DESIGN_VER
#define REGISTRY_KEY "SOFTWARE\\Autodesk\\3ds max\\4.0\\max Finder"
#else
#define REGISTRY_KEY "SOFTWARE\\Autodesk\\VIZ\\4.0\\VIZ Finder"
#endif // !DESIGN_VER

#define IDC_LISTBOX				0x1000
//#define IDC_FILESPEC			0x1010

#define PROPSET_SUMINFO			0x000000ff
#define PROPSET_DOCSUMINFO		0x0000ff00
#define PROPSET_USERDEF			0x00ff0000

#define ALL_PROPERTIES			0xffffffff	// All props
#define TITLE_PROP				0x00000001	// Summary Info
#define SUBJECT_PROP			0x00000002	// Summary Info
#define AUTHOR_PROP				0x00000004	// Summary Info
#define KEYWORDS_PROP			0x00000008	// Summary Info
#define COMMENTS_PROP			0x00000010	// Summary Info
#define MANAGER_PROP			0x00000100	// Document Summary Info
#define COMPANY_PROP			0x00000200	// Document Summary Info
#define CATEGORY_PROP			0x00000400	// Document Summary Info
#define EXT_DEPEND_PROP			0x00000800	// Document Summary Info
#define PLUGINS_PROP			0x00001000	// Document Summary Info
#define OBJECTS_PROP			0x00002000	// Document Summary Info
#define MATERIALS_PROP			0x00004000	// Document Summary Info
#define USER_PROP				0x00010000	// User Defined Properties

#define PID_TITLE				0x00000002
#define PID_SUBJECT				0x00000003
#define PID_AUTHOR				0x00000004
#define PID_KEYWORDS			0x00000005
#define PID_COMMENTS			0x00000006

#define PID_MANAGER				0x0000000E
#define PID_COMPANY				0x0000000F
#define PID_CATEGORY			0x00000002
#define PID_HEADINGPAIR			0x0000000C
#define PID_DOCPARTS			0x0000000D

BOOL	CenterWindow(HWND hWndChild, HWND hWndParent);

class RegSettings {
	public:
		int x;
		int y;
		int w;
		int h;
		int filespec;
		int propdlgx;
		int propdlgy;
	};

class App {
	public:

	// Initialization
	App(HINSTANCE hInst, int cshow);
	~App();

	void		CreateControls();
	BOOL		Init();
	void		PostInit();
	void		Reset();
	void		Resize(int flags, int width, int height);
	void		Move();

	void		DoFind();
	void		DoCD();
	void		ViewFile();
	BOOL		Qualify(char* filename);
	BOOL		Compare(char* s1, char* s2);

	void		ShowProperties();
	void		GetProperties();

	// Interface
	HINSTANCE	GetInstance();
	void		SetHWnd(HWND wnd);
	HWND		GetHWnd();
	void		SetDC(HDC dc);
	HDC			GetDC();
	void		SetHMenu(HMENU menu);
	HMENU		GetHMenu();
	HFONT		GetAppFont();
	HWND		GetFileSpecDropDown() { return hFileSpec; }
	HWND		GetMainListBox() { return hListBox; }
	HWND		GetPanel() { return hControlPane; }

	void		GetRegSettings();
	void		SetRegSettings();

	// Message pump
	void		AppIsIdle();

	// Utilities
	TCHAR*		GetString(int id);
	void		DoAboutBox();
	BOOL		ChooseDir(char *title, char *dir);
	void		DoStatusDirectory();
	BOOL		ScanDirectory(char* path, char* filespec, HWND hList);
	void		EnableUI(BOOL status);

	void		AppIsClosing() { SetEvent(hThreadExitRequest); }

	BOOL		bSearchActive;
	RegSettings	regsettings;
	HWND		hListBox;
	HWND		hPropDialog;

	private:

	// Windows, DC's n' stuff
	HWND		hMainWnd;
	HMENU		hMenu;
	HINSTANCE	hInstance;
	HDC			hMainWndDC;
	HFONT		hFont;

	// Controls
	HWND		hFindButton;
	HWND		hStatusPanel;
	HWND		hCDButton;
	HWND		hSearchText;
	HWND		hFileSpec;
	HWND		hProperty;
	HWND		hControlPane;
	HWND		hCheckSubdirs;
	HWND		hDlgFrame;

	HWND		hTxtSearch;
	HWND		hTxtFileSpec;
	HWND		hTxtProperty;

	CRITICAL_SECTION	cs;
	ULONG		wWorkerThread;
	HANDLE		hThreadExitRequest;

	// Data
	int			cmdShow;
};
