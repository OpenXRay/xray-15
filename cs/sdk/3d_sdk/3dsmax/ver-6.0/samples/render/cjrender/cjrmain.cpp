//***************************************************************************
// CJRender - [cjrmain.cpp] Sample Plugin Renderer for 3D Studio MAX.
// 
// By Christer Janson - Kinetix
//
// Description:
// Plugin initialization functions
//
//***************************************************************************

#include "maxincl.h"
#include "cjrender.h"
#include "resource.h"

HINSTANCE hInstance;
int controlsInit = FALSE;
extern ClassDesc* GetCJRenderDesc();

//***************************************************************************
// DllMain.
// Grab instance handle and initialize controls
//***************************************************************************

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
			
	return (TRUE);
}


//***************************************************************************
// This is the interface to MAX:
//***************************************************************************

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString (IDS_LIB_DESCRIPTION); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetCJRenderDesc();
		default: return 0;
	}

}


//***************************************************************************
// Return version so can detect obsolete DLLs
//***************************************************************************

__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }


//***************************************************************************
// Class descriptor
//***************************************************************************

class CJRenderClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) {return new CJRenderer;}
	const TCHAR *	ClassName() {return _T(RENDERNAME);}
	SClass_ID		SuperClassID() {return RENDERER_CLASS_ID;}
	Class_ID		ClassID() {return CCJREND_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
};

static CJRenderClassDesc CJRenderDesc;
ClassDesc* GetCJRenderDesc() {return &CJRenderDesc;}


//***************************************************************************
// String resource utility function
//***************************************************************************

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


