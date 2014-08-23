//************************************************************************** 
//* Main.cpp - 
//* [SaveVerification]	A system to validate that scene files are saved correctly.
//* 
//* Christer Janson
//* Discreet, A division of Autodesk, Inc.
//*
//* October 28, 2002, CCJ - Initial coding.
//*
//* Copyright (c) 2002, All Rights Reserved. 
//***************************************************************************

#include "SaveVerification.h"

HINSTANCE hInstance;

#define SAVEVERIFICATION_CLASS_ID	Class_ID(0xc8eb2a61, 0x8ac3c685)

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			InitCustomControls(hInstance);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return GetString(IDS_LIBDESCRIPTION);
}

__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
		case 0: return GetSaveVerificationDesc();
		default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}

class SaveVerificationClassDesc:public ClassDesc {
public:
	int				IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new SaveVerificationGUP; } 
	const TCHAR*	ClassName() { return GetString(IDS_SAVEVERIFICATION); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; } 
	Class_ID		ClassID() { return SAVEVERIFICATION_CLASS_ID; }
	const TCHAR*	Category() { return GetString(IDS_CATEGORY); }
};

static SaveVerificationClassDesc SaveVerificationDesc;

ClassDesc* GetSaveVerificationDesc()
{
	return &SaveVerificationDesc;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}