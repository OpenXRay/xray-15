//-----------------------------------------------------------------------------
// --------------------
// File ....: mscom.cpp
// --------------------
// Author...: Gus J Grubba
// Date ....: October 1998
// Descr....: MS COM/DCOM Server for 3D Studio MAX
//
// History .: Oct, 1 1998 - Started
//            
//-----------------------------------------------------------------------------

//-- Include files
#include "stdafx.h"
#include "mscom.h"



#include "comsrv.h" //need to create an instance
#include "MaxRenderer.h"
#ifdef RENDER_VER
	//in the render version we create a cached instance of an application object
	#include "maxapp.h"
	//and the GUP save and load methods are nontrivial, saving scratch material collections
	#include "MaxMaterialCollection.h"
#endif //RENDER_VER
#include <assert.h>

CComPtr<IUnknown> GUP_MSCOM::spOMAppUnk = NULL;


//-- Globals ------------------------------------------------------------------

HINSTANCE hInst = NULL;

extern bool StartServer	( HINSTANCE hInstance, HINSTANCE MaxhInstance, int registerCOM );
extern void StopServer	( );
extern LPCTSTR FindToken(LPCTSTR p1);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- DLL Declaration

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		 case DLL_PROCESS_ATTACH:
				if (hInst)
					return(FALSE);
				hInst = hDLLInst;
				break;
		 case DLL_PROCESS_DETACH:
				hInst  = NULL;
				break;
		 case DLL_THREAD_ATTACH:
				break;
		 case DLL_THREAD_DETACH:
				break;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
//-- Resource String Helper

TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInst)
		return LoadString(hInst, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

int             mClassDesc::IsPublic     ( )		{ return 1; }
void           *mClassDesc::Create       ( BOOL )	{ return Instance(); }
const TCHAR    *mClassDesc::ClassName    ( )		{ return _T("MaxRenderer COM Server"); }
SClass_ID       mClassDesc::SuperClassID ( )		{ return GUP_CLASS_ID; }
Class_ID        mClassDesc::ClassID      ( )		{ return Class_ID(470000002,0); }
const TCHAR    *mClassDesc::Category     ( )		{ return _T("Global Utility PlugIn"); }

GUP_MSCOM* mClassDesc::Instance()
{
	if(_instance == NULL)
	{
		_instance = new GUP_MSCOM;
		mcRef = 0;
	}
	mcRef++;
	return _instance;
}

void mClassDesc::RevokeInstance()
{
	mcRef--;//should be interlocked?
	if(mcRef == 0)
	{
		delete _instance;
		_instance = NULL;
	}
}

GUP_MSCOM* mClassDesc::_instance = NULL;
ULONG mClassDesc::mcRef = 0L;
static mClassDesc cdesc;

//-----------------------------------------------------------------------------
// Interface

DLLEXPORT const TCHAR * LibDescription ( )  { 
	return _T("MaxRenderer COM server global utility (Discreet)"); 
}

DLLEXPORT int LibNumberClasses ( ) { 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case  0: return &cdesc;	break;
		default: return 0;		break;
	}
}

DLLEXPORT ULONG LibVersion ( )  { 
	return ( VERSION_3DSMAX ); 
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::GUP_MSCOM()

GUP_MSCOM::GUP_MSCOM( ) { 

}

GUP_MSCOM::~GUP_MSCOM ( ) {

}

void GUP_MSCOM::DeleteThis()
{
	//delete this;  // RK: 06/28/00
	// JH 9/3/03 let the class descriptor manage the singleton.
	mClassDesc::RevokeInstance();
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::Start()

DWORD GUP_MSCOM::Start( ) {
	
	bool toregister		= false;
	bool tounregister	= false;


	LPTSTR lpCmdLine;
	lpCmdLine = GetCommandLine();

	LPCTSTR lpszToken = FindToken(lpCmdLine);
	while (lpszToken != NULL) {
		if (lstrcmpi(lpszToken,_REGISTERCOM)==0) {
			toregister = true;
			break;
		}
		if (lstrcmpi(lpszToken,_UNREGISTERCOM)==0) {
			tounregister = true;
			break;
		}
		if (lstrcmpi(lpszToken,_T("Embedding"))==0) {
			_Module.m_bOLEStart = true;
			break;
		}
		lpszToken = FindToken(lpszToken);
	}

	//-----------------------------------------------------
	//-- If not registered

	if (!IsCOMRegistered()) {

		//-- We are registering ourselves
		
		if (toregister) {
			StartServer(hInst,MaxInst(),2);
			return GUPRESULT_ABORT;
		}
		
		//-- Not registered and not registering, no need to
		//   stay around.


		return GUPRESULT_NOKEEP;
	
	//-----------------------------------------------------
	//-- If registered
	
	} else {

		//-- Unregistering COM/DCOM server

		if (tounregister) {
			StartServer(hInst,MaxInst(),1);
			return GUPRESULT_ABORT;
		}
		else if(toregister)
			return GUPRESULT_ABORT;

	}

	//-----------------------------------------------------
	//-- Normal Operation
	StartServer(hInst,MaxInst(),0);	

#ifdef RENDER_VER
	//TO DO JH 7/7/02 asap
	//Migrate this code into app 
	if(!_Module.m_bOLEStart)
	{
	//when the app has been started by a user,
	//create a document

		dwActiveCookie = 0;
		CComPtr<IMaxDocument> spDoc;
		CComPtr<IMaxApp> spApp;
		CComPtr<IClassFactory> spCF;


		HRESULT hr = _Module.GetClassObject(CLSID_MaxDocument, IID_IClassFactory, (void **) &spCF);
		if(SUCCEEDED(hr))
		{
			hr = spCF->CreateInstance(NULL, IID_IMaxDocument, (void **) &spDoc);
			ATLASSERT(SUCCEEDED(hr));
		}



		if(SUCCEEDED(hr))
		{
			spDoc->Show();
		}
		
		//Now revoke the class objects, we don't want them out there when started by
		//the user
		hr = _Module.RevokeClassObjects();

	}
#endif //RENDER_VER



	return GUPRESULT_KEEP;
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::Stop()

void GUP_MSCOM::Stop( )
{
	if (IsCOMRegistered())
	{
#ifdef RENDER_VER				
		//destroy the instance of the app object
		if(spOMAppUnk)
		{
			CComQIPtr<IMaxApp> spApp = spOMAppUnk;
			if(spApp)
				spApp->put_Visible(VARIANT_FALSE);

			CoLockObjectExternal(spOMAppUnk, false, true);
			HRESULT hr = CoDisconnectObject(spOMAppUnk, 0);

			spOMAppUnk.p->Release();
			spOMAppUnk.Detach();
		}
#endif //RENDER_VER

		StopServer();
	}
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::Control()

DWORD GUP_MSCOM::Control( DWORD parameter ) {
	switch (parameter) {
		case 0:	return (DWORD)IsCOMRegistered();
		case 1:	return (DWORD)RegisterCOM();
		case 2:	return (DWORD)UnRegisterCOM();
		case 4: return (DWORD)GetAppObject();
	}
	return 0;
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::IsCOMRegistered()

bool GUP_MSCOM::IsCOMRegistered( ) {
	TCHAR szKeyName[256];
	TCHAR clsidApp[] = {"{4AD72E6E-5A4B-11D2-91CB-0060081C257E}"};
	wsprintf(szKeyName,"CLSID\\%s",clsidApp);
	HKEY	hKey;
	long retVal = RegOpenKeyEx(HKEY_CLASSES_ROOT,szKeyName,0,KEY_READ,&hKey);
	if (retVal == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::RegisterCOM()

bool GUP_MSCOM::RegisterCOM( ) {
	if (IsCOMRegistered())
		return false;
	bool res = StartServer(hInst,MaxInst(),2);
	return res;
}

//-----------------------------------------------------------------------------
// #> GUP_MSCOM::UnRegisterCOM()

bool GUP_MSCOM::UnRegisterCOM( ) {
	bool res = StartServer(hInst,MaxInst(),1);
	return res;
}

//we don't addref this point, clients should
IUnknown *GUP_MSCOM::GetAppObject	( )
{
#ifdef RENDER_VER
	if(spOMAppUnk == NULL)
	{
		typedef CComObjectCached<CMaxApp> _apptype;
		HRESULT hr = CComCreator<_apptype>::CreateInstance(NULL, IID_IUnknown, (void **)&spOMAppUnk.p);
//		long test = spOMAppUnk.p->AddRef();
		ATLASSERT(SUCCEEDED(hr));
	}
//	spOMAppUnk.p->AddRef();
	return spOMAppUnk.p;
#else 
	return NULL;
#endif //RENDER_VER
}

static const USHORT kMaxMaterialChunk = 0xaf40;

IOResult		GUP_MSCOM::Save( ISave *isave )
{
	IOResult res = IO_OK;

	return res;
}

IOResult		GUP_MSCOM::Load		( ILoad *iload )
{
	IOResult res = IO_OK;


	return IO_OK;
}


//-- EOF: mscom.cpp -----------------------------------------------------------
