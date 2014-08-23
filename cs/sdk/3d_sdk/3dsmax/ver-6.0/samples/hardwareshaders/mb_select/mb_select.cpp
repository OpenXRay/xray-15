/**********************************************************************
 *<
	FILE: mb_select.cpp

	DESCRIPTION:	Simple MetalBump selector - this allows 
	support for both Dx9 and Dx8 versions of the plugin.  The correct 
	version is loaded depending on the GFX in use.
	
	This has been extended to either load DX compiled plugins or not.
	The reason here is that we don't want to force people to use DX9, 
	but still give some functionality


	CREATED BY: Neil Hazzard

	HISTORY:	01|24|03
				05|27|03 - DX9 additions and MaxClass loading

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "mb_select.h"
#include "maxscrpt/MAXScrpt.h"
#include "maxscrpt/MAXObj.h" // MAXClass 

#define MB_SELECT_CLASS_ID		Class_ID(0x14907f3d, 0x506928eb)

#define DXMATERIAL_CLASS_ID		Class_ID(0xed995e4, 0x6133daf2)
#define	MAX_SHADER_CLASS_ID		Class_ID(0x20542fe5, 0x8f74721f)
#define DDS_CLASS_ID			Class_ID(0xe3061ca, 0xd2120df)


class Mb_select : public GUP {
	public:


		static HWND hParams;



		// GUP Methods
		DWORD	Start			( );
		void	Stop			( );
		DWORD	Control			( DWORD parameter );
		
		// Loading/Saving
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		
		//Constructor/Destructor

		Mb_select();
		~Mb_select();		

};


class Mb_selectClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new Mb_select(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return GUP_CLASS_ID; }
	Class_ID		ClassID() { return MB_SELECT_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("Mb_select"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};



static Mb_selectClassDesc Mb_selectDesc;
ClassDesc2* GetMb_selectDesc() { return &Mb_selectDesc; }




Mb_select::Mb_select()
{

}

Mb_select::~Mb_select()
{

}

bool CheckDX9()
{
	Interface * ip = GetCOREInterface();

    TCHAR filename[MAX_PATH];
	
	_tcscpy(filename,ip->GetDir(APP_MAXROOT_DIR));
	int len = _tcslen(filename);
	if (len) {
		if (_tcscmp(&filename[len-1],_T("\\")))
			_tcscat(filename,_T("\\"));
	}

	_tcscat(filename,_T("d3dgfx.drv")); 
	//lets find whether the Dx9 GFX was loaded.
	HMODULE dxDriver = GetModuleHandle(filename);
	if(dxDriver != NULL)
		return true;
	else
		return false;
	
}

//-----------------------------------------------------------------------------
// Name: GetFileVersion()
// Desc: Returns ULARGE_INTEGER with a file version of a file, or a failure code.
//-----------------------------------------------------------------------------
HRESULT GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion )
{   
	if( szPath == NULL || pllFileVersion == NULL )
		return E_INVALIDARG;

	DWORD dwHandle;
	UINT  cb;
	cb = GetFileVersionInfoSize( szPath, &dwHandle );
	if (cb > 0)
	{
		BYTE* pFileVersionBuffer = new BYTE[cb];
		if( pFileVersionBuffer == NULL )
			return E_OUTOFMEMORY;

		if (GetFileVersionInfo( szPath, 0, cb, pFileVersionBuffer))
		{
			VS_FIXEDFILEINFO* pVersion = NULL;
			if (VerQueryValue(pFileVersionBuffer, TEXT("\\"), (VOID**)&pVersion, &cb) && 
				pVersion != NULL) 
			{
				pllFileVersion->HighPart = pVersion->dwFileVersionMS;
				pllFileVersion->LowPart  = pVersion->dwFileVersionLS;
				delete[] pFileVersionBuffer;
				return S_OK;
			}
		}

		delete[] pFileVersionBuffer;
	}

	return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: MakeInt64()
// Desc: Returns a ULARGE_INTEGER where a<<48|b<<32|c<<16|d<<0
//-----------------------------------------------------------------------------
ULARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d )
{
	ULARGE_INTEGER ull;
	ull.HighPart = MAKELONG(b,a);
	ull.LowPart = MAKELONG(d,c);
	return ull;
}

// Activate and Stay Resident
DWORD Mb_select::Start( ) 
{
	Interface * ip = GetCOREInterface();

    TCHAR filename[MAX_PATH];
	TCHAR dxDDSName[MAX_PATH];
	_tcscpy(filename,ip->GetDir(APP_MAXROOT_DIR));

	//load MetalBump
	if(CheckDX9())
		_tcscat(filename,GetString(IDS_METALBUMP_DX9));

	else
		_tcscat(filename,GetString(IDS_METALBUMP_DX8));


	//hook up maxscript.
	lookup_MAXClass(&MAX_SHADER_CLASS_ID,REF_TARGET_CLASS_ID,true);


	//load DxMaterial and DDS plugins
	DllDir *dd = ip->GetDllDirectory();
	dd->LoadADll(filename,TRUE);

	ULARGE_INTEGER llFileVersion;  
	TCHAR szPath[512];
	TCHAR szFile[512];
	BOOL bFound = false;

	_tcscpy(filename,ip->GetDir(APP_MAXROOT_DIR));
	_tcscpy(dxDDSName,ip->GetDir(APP_MAXROOT_DIR));

	if( GetSystemDirectory( szPath, MAX_PATH ) != 0 )
	{
		_tcscpy( szFile, szPath );
		_tcscat( szFile, TEXT("\\d3d9.dll"));
		if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
		{
			// File exists - we can go ahead and load  the uber version!!
			_tcscat(filename,GetString(IDS_DX_MATERIAL));
			dd->LoadADll(filename,TRUE);

			_tcscat(dxDDSName,GetString(IDS_DX_DDS));
			dd->LoadADll(dxDDSName,TRUE);
		}
		else
		{
			//basic support versions
			_tcscat(dxDDSName,GetString(IDS_DDS));
			dd->LoadADll(dxDDSName,TRUE);
			_tcscat(filename,GetString(IDS_STUB_DX_MATERIAL));
			dd->LoadADll(filename,TRUE);

		}
		lookup_MAXClass(&DDS_CLASS_ID,BMM_IO_CLASS_ID,true);
		lookup_MAXClass(&DXMATERIAL_CLASS_ID,MATERIAL_CLASS_ID,true);

	}
	return GUPRESULT_NOKEEP;
}

void Mb_select::Stop( ) {
	// TODO: Do plugin un-initialization here
}

DWORD Mb_select::Control( DWORD parameter ) {
	return 0;
}

IOResult Mb_select::Save(ISave *isave)
{
	return IO_OK;
}

IOResult Mb_select::Load(ILoad *iload)
{
	return IO_OK;
}

