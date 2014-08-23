//***************************************************************************
// SystemUtilities.h
// A collection of system utilities
// Christer Janson
// Discreet, A division of Autodesk, Inc.
// San Francisco, CA - March 27, 2000

UtilExport bool	IsDebugging();			// Are we running under a debugger?
UtilExport int	NumberOfProcessors();	// Number of processors in the system.
UtilExport bool	IsWindows9x();			// Are we running on Windows 9x?
UtilExport bool	IsWindows98or2000();	// Are we running on Windows 98 or 2000?
UtilExport int	GetScreenWidth();		// The width of the screen (including multiple monitors)
UtilExport int	GetScreenHeight();		// The height of the screen (including multiple monitors)

// CSIDL functions added 030110  --prs.

UtilExport HRESULT UtilGetFolderPath(HWND hwndOwner,	// just calls SFGetFolderPath()
				int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

// negative indices, refer to specCSID[] array in systemutilities.cpp
#define APP_MAP_DIR			   -1
#define APP_DOWNLOAD_DIR	   -2
#define APP_FOLIAGE_DIR		   -3
#define APP_XREF_DIR		   -4
#define APP_PLUGIN_INI_DIR	   -5
#define APP_STDPLUGS_DIR	   -6
#define APP_PLUGINS_DIR		   -7
#define APP_FILELINK_DIR	   -8
#define APP_CATALOGS_DIR	   -9
#define APP_CUI_SCRIPTS_DIR   -10	// added 030224  --prs.
// xavier robitaille | 03.02.05 | add textures dir. to bitmap paths
#ifndef TEXTURES_DIR_BMP_SEARCH_PATH
#define APP_CUI_DIR			  -11
#define APP_LAST_SPEC_DIR	  -11
#define APP_FX_DIR			  -12	
#else
#define APP_TEXTURES_DIR	  -11
#define APP_CUI_DIR			  -12
#define APP_LAST_SPEC_DIR	  -12
#define APP_FX_DIR			  -13
#endif


UtilExport bool GetSpecDir(int index, TCHAR *dirName, TCHAR *buf);	// get directory path
UtilExport bool TryCSIDLDir(int csidl, TCHAR *dirName, TCHAR *buf);	// create directory path
