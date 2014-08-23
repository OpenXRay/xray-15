/*===========================================================================*\
 | 
 |  FILE:	Plugin Management Tool
 |			3D Studio MAX R4.0
 | 
 |  AUTH:   Harry Denholm
 |			Ravi Karra
 |			All Rights Reserved
 |
 |  HIST:	Started 19-10-99
 | 
\*===========================================================================*/

#include "Plugman.h"
#include "iparamm2.h"
#include "notify.h"

// Uncomment to enable splash screen
// #define DO_SPLASHSCREEN

GetModuleInformation getModuleInformation;

HMODULE PlugMgrUtility::hPsapi =0;

/*===========================================================================*\
 |	Function Descriptor
\*===========================================================================*/

PlugMgrUtility sPlugMgrUtility(
	PLUGINMANAGER_FN_INTERFACE, _T("pluginManager"), 0, NULL, FP_CORE,
	PlugMgrUtility::pm_loadClass, _T("loadClass"), 0, TYPE_VOID, 0, 1,
		_T("class"), 0, TYPE_CLASS,
	properties,
		PlugMgrUtility::pm_isVisible, PlugMgrUtility::pm_setVisible, _T("visible"), 0, TYPE_BOOL,
	end);

/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class PlugMgrClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic()					{ return TRUE; }
	void*			Create( BOOL loading )		{ return &sPlugMgrUtility; }
	const TCHAR*	ClassName()					{ return GetString(IDS_CLASSNAME); }
	SClass_ID		SuperClassID()				{ return GUP_CLASS_ID; }
	Class_ID 		ClassID()					{ return PLUGIN_MANAGER_CLASSID; }
	const TCHAR* 	Category()					{ return _T("");  }	
	HINSTANCE		HInstance()					{ return hInstance; }
};

static PlugMgrClassDesc PlugMgrCD;
ClassDesc* GetPlugMgrDesc() {return &PlugMgrCD;}

/*===========================================================================*\
 |	Action Items Descriptor
\*===========================================================================*/

class PluginManagerActions: public FPStaticInterface {
	public:		
		FPStatus ShowPluginManager() { sPlugMgrUtility.SetVisible(); return FPS_OK; }
		
		// action dispatch
		DECLARE_DESCRIPTOR(PluginManagerActions)
		enum ActionID { pm_show };
		BEGIN_FUNCTION_MAP
			FN_ACTION(pm_show, ShowPluginManager);
		END_FUNCTION_MAP
};

static PluginManagerActions sPluginManagerActions(
	PLUGINMANAGER_ACT_INTERFACE, _T("PluginMgrAction"), IDS_CLASSNAME, &PlugMgrCD, FP_ACTIONS, kActionMainUIContext, 
		PluginManagerActions::pm_show, _T("show"), IDS_CLASSNAME, 0,
		end,
	end);

#ifdef DO_SPLASHSCREEN
/*===========================================================================*\
 |	Splash Screen
\*===========================================================================*/

BOOL CALLBACK SplashProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

		case WM_INITDIALOG:
			SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOREPOSITION);
			SetTimer(hWnd,34,2400,NULL);
			InvalidateRect(hWnd,NULL,TRUE);
			break;

		case WM_TIMER:
			if(wParam==34) EndDialog(hWnd,0);
			break;


		default:
			return FALSE;
	}
	return TRUE;

}

/*===========================================================================*\
 |	Global Utility interface (start/stop/control)
\*===========================================================================*/

DWORD WINAPI ShowSplash( LPVOID ptr )
{
	Sleep(800);

	DialogBox( (HINSTANCE)ptr, 
	MAKEINTRESOURCE(IDD_PMSPLASH), 
	GetDesktopWindow(), 
	(DLGPROC) SplashProc 
	); 

	ExitThread(0);
	return 1;
}
#endif

void newPluginLoadedProc(void *param, NotifyInfo *info)
{
	if (info->intcode==NOTIFY_PLUGIN_LOADED)
	{
		PlugMgrUtility* pmu = (PlugMgrUtility*)param;
		DllDesc *dsc = (DllDesc*)info->callParam;
		pmu->newlyLoaded.Append(1,&dsc);
		rebuildCache(pmu->ip, pmu, pmu->popup1);
		DeepRefreshUI(pmu->ip);
		PVRefresh(pmu->popup1,pmu,TRUE);
	}
}

DWORD PlugMgrUtility::Start( ) 
{
#ifdef DO_SPLASHSCREEN
	// splash screen
	static DWORD tID;
	static HANDLE tH;
	tH = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ShowSplash,(LPVOID)hInstance,0,&tID);
#endif
	
	hPsapi = LoadLibrary("psapi.dll");
	if (hPsapi) {
		getModuleInformation = (GetModuleInformation)GetProcAddress(hPsapi,"GetModuleInformation");
		if (!getModuleInformation) {
			FreeLibrary(hPsapi);
			hPsapi = 0;
		}
	}
	popup1 = NULL;
	ip = Max();

	recentArray.SetSize(0);

	UI_active = FALSE;
	columnActive = 1;

		RegisterNotification(newPluginLoadedProc, (void*)this, NOTIFY_PLUGIN_LOADED);

	return GUPRESULT_KEEP;
}

void PlugMgrUtility::Stop( ) {
	if (hPsapi)
		FreeLibrary(hPsapi);
}

DWORD PlugMgrUtility::Control( DWORD parameter ) {
	if (parameter==1) SetVisible(TRUE);
	else if(parameter==0) SetVisible(FALSE);
	return 0;
}

void PlugMgrUtility::SetVisible(BOOL show)
{
	if (show && !popup1)	// make visible
	{
		popup1 = CreateDialogParam(
							hInstance,
							MAKEINTRESOURCE(IDD_MAIN),
							GetCOREInterface()->GetMAXHWnd(),
							PMDefaultDlgProc,
							(LPARAM)this);
		SetWindowLong(popup1,GWL_USERDATA,(LPARAM)this);
	}
	else if(!show && popup1) // hide it
	{
		DestroyWindow(popup1);
	}
}

BOOL PlugMgrUtility::IsVisible()
{
	return (popup1) ? TRUE : FALSE;
}

void PlugMgrUtility::LoadClass(ClassDesc* cd)			// ensure deferred plugin class is loaded - JBW 7.4.00
{
	ClassDirectory& cdir = GetCOREInterface()->GetDllDir().ClassDir();
	cdir.FindClassEntry(cd->SuperClassID(), cd->ClassID())->FullCD();
}
