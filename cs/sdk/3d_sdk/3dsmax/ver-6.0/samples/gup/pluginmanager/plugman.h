/*===========================================================================*\
 | 
 |  FILE:	PlugMan.h
 |			Plugin Manager Global Utility Plugin
 |			3D Studio MAX R4.0
 | 
 |  AUTH:   Harry Denholm
 |			Ravi Karra
 |			Developer Consulting Group
 |			Copyright(c) Discreet 1999
 |
 |  HIST:	Started 21-1-99
 | 
 |	NOTE:	The plugin manager also has minimal support for unloading plugins. 
 |			You can enable it by uncommenting the following lines in from Plugman.rc
 |			MENUITEM "Unload",                      ID_DLLMENU_UNLOAD
 |			MENUITEM "Unload",                      ID_TAG_UNLOAD
 |
 |			Unloading plugins is an untested feature and might lead to crashes,
 |			if you try to unload a plugin that is currently being used. 
 |			USE IT AT YOUR OWN RISK
 |			
 |
\*===========================================================================*/

#ifndef __GUPSKEL__H
#define __GUPSKEL__H


#include <Max.h>
#include <bmmlib.h>
#include <guplib.h>

#include "resource.h"


#define UI_MAKEBUSY			SetCursor(LoadCursor(NULL, IDC_WAIT));
#define UI_MAKEFREE			SetCursor(LoadCursor(NULL, IDC_ARROW));


// IMPORTANT:
// The ClassID must be changed whenever a new project
// is created using this skeleton
#define	PLUGIN_MANAGER_CLASSID		Class_ID(0x61c2789d, 0x6dc3025f)


TCHAR *GetString(int id);
extern ClassDesc* GetPlugMgrDesc();

class PlugMgrUtility;

/*===========================================================================*\
 |	DLL Management functions
\*===========================================================================*/

// recalc the class usage in the scene
void ComputeClassUse(Interface *ip);

// find a usage DllDesc from a plugin filename
DllDesc *GetDescFromPlugname(char *s);

// unload specified dll and set to unloaded state
bool UnloadDLL( char *s, HWND hWnd, Interface *ip);

// open file selector and load in chosen plugin
bool LoadNewDLL( HWND hWnd, Interface *ip);

// load in given plugin
bool LoadExistingDLL( char *s, HWND hWnd, Interface *ip);

// check to see if a plugin has entries in the class lists (ie, deferred)
BOOL pluginHasClassesRegistered( int masterCode, Interface *ip );

// refresh subsections of the UI after a DLL addition
void DeepRefreshUI(Interface *ip);


// fill the listview
void PVRefresh( HWND hWnd, PlugMgrUtility *pmu, BOOL rebuild );

// enumerate the selection and call BAC's proc
void PVSelectionEnumerator( HWND hWnd, BitArrayCallback &bac);

// rebuild our internal dll dir representation
void rebuildCache( Interface *ip, PlugMgrUtility *pmu, HWND hWnd );



struct plugCache
{
	DllDesc* ddesc;
	int realIdx;
	int bytesize;
};

BOOL CALLBACK PMDefaultDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class IPluginManager : public FPStaticInterface {
	public:
		virtual void	SetVisible(BOOL show=TRUE)=0;
		virtual	BOOL	IsVisible()=0;
		virtual void	LoadClass(ClassDesc* cd)=0;  // ensure class is fully loaded - jbw 7.4.00
};

#define PLUGINMANAGER_FN_INTERFACE	Interface_ID(0x5e3b07f6, 0x273f7de7)
#define PLUGINMANAGER_ACT_INTERFACE Interface_ID(0x5e3b07f6, 0x273f7de8)
inline IPluginManager* GetPluginManager() { return (IPluginManager*)GetCOREInterface(PLUGINMANAGER_FN_INTERFACE); }

class PlugMgrUtility : public GUP, public IPluginManager {
	public:
	
		HWND			popup1;
		BitArray		recentArray;
		Tab<plugCache>	ddList;
		Tab<DllDesc*>	newlyLoaded;
		Interface		*ip;
		HIMAGELIST		pmImages;
		BOOL			UI_active;

		int				columnActive;
		static HMODULE	hPsapi;
		BOOL*			pdrec;

				~PlugMgrUtility	( ) {}
		// GUP Methods
		void	DeleteThis		( )	{ }
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		

		void	Init(HWND hWnd);
		void	Destroy(HWND hWnd);

		void	SetVisible(BOOL show=TRUE);
		BOOL	IsVisible();
		void	LoadClass(ClassDesc* cd);

		// function IDs 
		enum { pm_setVisible, pm_isVisible, pm_loadClass,  }; 

		DECLARE_DESCRIPTOR(PlugMgrUtility)
		// dispatch map
		BEGIN_FUNCTION_MAP
			VFN_1(pm_loadClass,    LoadClass, TYPE_CLASS);
			PROP_FNS(pm_isVisible, IsVisible, pm_setVisible, SetVisible, TYPE_BOOL);
		END_FUNCTION_MAP 
};


//-----------------------------------------------------------------------------
// Structure for GetModuleInformation()
//
// This is from psapi.h which is only included in the Platform SDK

typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

//-- This is in psapi.dll (NT4 and W2k only)
typedef BOOL (WINAPI* GetModuleInformation)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

#endif