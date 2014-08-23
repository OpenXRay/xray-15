/*===========================================================================*\
 | 
 |  FILE:	wM3_plugin.cpp
 |			Weighted Morpher for MAX R3
 |			Plugin Init
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 22-5-98
 | 
\*===========================================================================*/

#include "wM3.h"


HINSTANCE hInstance;
static int controlsInit = FALSE;


/*===========================================================================*\
 | Class Descriptor
\*===========================================================================*/

class MorphR3ClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new MorphR3();}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return MR3_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_MXCATEGORY);}

	// SView
	bool DrawRepresentation(COLORREF bkColor, HDC hDC, Rect &rect)
		{
		LoadIcons(bkColor);
		DrawMAXIcon(hDC, rect, hIcons32, hIcons16, II_MORPHER);
		return TRUE;
		}
};

static MorphR3ClassDesc MorphR3Desc;
ClassDesc* GetMorphR3Desc() {return &MorphR3Desc;}


/*===========================================================================*\
 | DLLMain and the standard plugin stuff
\*===========================================================================*/

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
			
	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

__declspec( dllexport ) int LibNumberClasses()
{
#ifndef NO_MTL_MORPHER
	return 2;
#else
	return 1;
#endif // NO_MTL_MORPHER
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetMorphR3Desc();
#ifndef NO_MTL_MORPHER
		case 1: return GetM3MatDesc();
#endif // NO_MTL_MORPHER
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
// Morpher can't because its inputType is dynamic, and based on scene state
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 0;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


HIMAGELIST hIcons32, hIcons16;
COLORREF currentBkColor;

void LoadIcons(COLORREF bkColor)
	{
	static BOOL iconsLoaded = FALSE;
	if (!iconsLoaded || bkColor != currentBkColor)
		{
		hIcons32 = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 1, 0);
		hIcons16 = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 1, 0);
		LoadMAXIcon(hInstance, MAKEINTRESOURCE(IDB_SV_MORPH32), MAKEINTRESOURCE(IDB_SV_MORPH32_MASK),
			bkColor,	hIcons32, -1);
		LoadMAXIcon(hInstance, MAKEINTRESOURCE(IDB_SV_MORPH16), MAKEINTRESOURCE(IDB_SV_MORPH16_MASK),
			bkColor,	hIcons16, -1);

		iconsLoaded = TRUE;
		currentBkColor = bkColor;
		}
	}