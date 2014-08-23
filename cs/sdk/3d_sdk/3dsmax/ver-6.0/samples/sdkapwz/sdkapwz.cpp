// SDKAPWZ.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "SDKAPWZ.h"
#include "SDKAPWZaw.h"

#ifdef _PSEUDO_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE SDKAPWZDLL = { NULL, NULL };

HINSTANCE hInst;
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;
		TRACE0("SDKAPWZ.AWX Initializing!\n");
		
		// Extension DLL one-time initialization
		AfxInitExtensionModule(SDKAPWZDLL, hInstance);

		// Insert this DLL into the resource chain
		new CDynLinkLibrary(SDKAPWZDLL);

		// Register this custom AppWizard with MFCAPWZ.DLL
		SetCustomAppWizClass(&SDKAPWZaw);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("SDKAPWZ.AWX Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(SDKAPWZDLL);
	}
	return 1;   // ok
}
