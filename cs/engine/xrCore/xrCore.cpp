// xrCore.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#pragma hdrstop

#include <mmsystem.h>
#include <objbase.h>
#include "xrCore.h"
#include "CPUID.hpp"
 
#pragma comment(lib,"winmm.lib")

#ifdef DEBUG
#	include	<malloc.h>
#endif // DEBUG

XRCORE_API		xrCore	Core;
static const char* BuildDate;
static u32 BuildId;
static u32	init_counter	= 0;

//. extern xr_vector<shared_str>*	LogFile;

static void ComputeBuildId()
{
    static const char* monthId[12] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static int daysInMonth[12] =
    {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    static const int startDay = 31, startMonth = 1, startYear = 1999;
    BuildDate = __DATE__;
    int days, years;
    string16 month;
    sscanf(BuildDate, "%s %d %d", month, &days, &years);
    int months = 0;
    for (int i = 0; i < 12; i++)
    {
        if (stricmp(monthId[i], month))
            continue;
        months = i;
        break;
    }
    u32 buildId = (years - startYear) * 365 + days - startDay;
    for (int i = 0; i < months; ++i)
        buildId += daysInMonth[i];
    for (int i = 0; i < startMonth - 1; ++i)
        buildId -= daysInMonth[i];
    BuildId = buildId;
}

void xrCore::_initialize	(LPCSTR _ApplicationName, LogCallback cb, BOOL init_fs, LPCSTR fs_fname)
{
	strcpy_s					(ApplicationName,_ApplicationName);
	if (0==init_counter) {
#ifdef XRCORE_STATIC	
		_clear87	();
		_control87	( _PC_53,   MCW_PC );
		_control87	( _RC_CHOP, MCW_RC );
		_control87	( _RC_NEAR, MCW_RC );
		_control87	( _MCW_EM,  MCW_EM );
#endif
        ComputeBuildId();
        Params = xr_strdup(GetCommandLine());
        strlwr(Params);
		// Init COM so we can use CoCreateInstance
//		HRESULT co_res = 
		if (!strstr(Params, "-editor"))
			CoInitializeEx	(NULL, COINIT_MULTITHREADED);

		string_path		fn,dr,di;

		// application path
        GetModuleFileName(GetModuleHandle(MODULE_NAME),fn,sizeof(fn));
        _splitpath		(fn,dr,di,0,0);
        strconcat		(sizeof(ApplicationPath),ApplicationPath,dr,di);

#ifdef _EDITOR
		// working path
        if( strstr(Params,"-wf") )
        {
            string_path				c_name;
            sscanf					(strstr(Core.Params,"-wf ")+4,"%[^ ] ",c_name);
            SetCurrentDirectory     (c_name);
        }
#endif

		GetCurrentDirectory(sizeof(WorkingPath),WorkingPath);

		// User/Comp Name
		DWORD	sz_user		= sizeof(UserName);
		GetUserName			(UserName,&sz_user);

		DWORD	sz_comp		= sizeof(CompName);
		GetComputerName		(CompName,&sz_comp);
		
		// Mathematics & PSI detection
        if (!XRay::CPUID::Detect())
        {
            abort();
        }
        CPU::Initialize();
		Memory._initialize	(strstr(Params,"-mem_debug") ? TRUE : FALSE);

		DUMP_PHASE;

		InitLog				();
		_initialize_cpu		();

//		xrDebug::Initialize	();

		rtc_initialize		();

		xr_FS				= new CLocatorAPI();

		xr_EFS				= new EFS_Utils();
//.		R_ASSERT			(co_res==S_OK);
	}
	if (init_fs){
		u32 flags			= 0;
		if (0!=strstr(Params,"-build"))	 flags |= CLocatorAPI::flBuildCopy;
		if (0!=strstr(Params,"-ebuild")) flags |= CLocatorAPI::flBuildCopy|CLocatorAPI::flEBuildCopy;
#ifdef DEBUG
		if (strstr(Params,"-cache"))  flags |= CLocatorAPI::flCacheFiles;
		else flags &= ~CLocatorAPI::flCacheFiles;
#endif // DEBUG
#ifdef _EDITOR // for EDITORS - no cache
		flags 				&=~ CLocatorAPI::flCacheFiles;
#endif // _EDITOR
		flags |= CLocatorAPI::flScanAppRoot;

#ifndef	_EDITOR
	#ifndef ELocatorAPIH
		if (0!=strstr(Params,"-file_activity"))	 flags |= CLocatorAPI::flDumpFileActivity;
	#endif
#endif
		FS._initialize		(flags,0,fs_fname);
		Msg					("'%s' build %d, %s\n", "xrCore", BuildId, BuildDate);
		EFS._initialize		();
#ifdef DEBUG
    #ifndef	_EDITOR
		Msg					("Process heap 0x%08x",GetProcessHeap());
    #endif
#endif // DEBUG
	}
	SetLogCB				(cb);
	init_counter++;
}

#ifndef	_EDITOR
#include "compression_ppmd_stream.h"
extern compression::ppmd::stream	*trained_model;
#endif
void xrCore::_destroy		()
{
	--init_counter;
	if (0==init_counter){
		FS._destroy			();
		EFS._destroy		();
		xr_delete			(xr_FS);
		xr_delete			(xr_EFS);

#ifndef	_EDITOR
		if (trained_model) {
			void			*buffer = trained_model->buffer();
			xr_free			(buffer);
			xr_delete		(trained_model);
		}
#endif
        xr_free(Params);
		Memory._destroy		();
	}
}

#ifndef XRCORE_STATIC

//. why ??? 
#ifdef _EDITOR
	BOOL WINAPI DllEntryPoint(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
#else
	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
#endif
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			_clear87		();
			_control87		( _PC_53,   MCW_PC );
			_control87		( _RC_CHOP, MCW_RC );
			_control87		( _RC_NEAR, MCW_RC );
			_control87		( _MCW_EM,  MCW_EM );
		}
//.		LogFile.reserve		(256);
		break;
	case DLL_THREAD_ATTACH:
		if (!strstr(GetCommandLine(),"-editor"))
			CoInitializeEx	(NULL, COINIT_MULTITHREADED);
		timeBeginPeriod	(1);
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef USE_MEMORY_MONITOR
		memory_monitor::flush_each_time	(true);
#endif // USE_MEMORY_MONITOR
		break;
	}
    return TRUE;
}
#endif // XRCORE_STATIC