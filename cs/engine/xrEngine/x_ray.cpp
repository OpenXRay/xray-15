//-----------------------------------------------------------------------------
// File: x_ray.cpp
//
// Programmers:
//	Oles		- Oles Shishkovtsov
//	AlexMX		- Alexander Maksimchuk
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "IGame_Level.h"
#include "IGame_Persistent.h"

#include "dedicated_server_only.h"
#include "no_single.h"

#include "xr_input.h"
#include "xr_ioconsole.h"
#include "xr_ioc_cmd.h"
#include "x_ray.h"
#include "std_classes.h"
#include "GameFont.h"
#include "resource.h"
#include "LightAnimLibrary.h"
#include "ISpatial.h"
#include "CopyProtection.h"
#include "Text_Console.h"
#include <process.h>
// OpenAutomate
#include "xrSash.h"
// Приблудина для SecuROM-а
#include "securom_api.h"

#ifndef DEDICATED_SERVER
static HWND logoWindow = NULL;
#endif

#ifdef MASTER_GOLD
#define NO_MULTI_INSTANCES
#endif

#ifdef NO_MULTI_INSTANCES
#define STALKER_PRESENCE_MUTEX "STALKER-SoC"
#endif

#ifdef DEDICATED_SERVER
ENGINE_API bool g_dedicated_server = true;
#else
ENGINE_API bool g_dedicated_server = false;
#endif

ENGINE_API BOOL g_appLoaded = FALSE;
ENGINE_API CInifile* pGameIni = nullptr; // declared in stdafx.h
ENGINE_API CApplication* pApp = nullptr;
extern CRenderDevice Device;
ENGINE_API bool g_bBenchmark = false; // declared in device.h
string512 g_sBenchmarkName;
ENGINE_API string512 g_sLaunchOnExit_params;
ENGINE_API string512 g_sLaunchOnExit_app;
ENGINE_API string_path g_sLaunchWorkingFolder;
static CTimer PhaseTimer;
static bool IntroFinished = false;

void doBenchmark(const char* name);

// Фунция для тупых требований THQ и тупых американских пользователей
bool IsOutOfVirtualMemory()
{
    SECUROM_MARKER_HIGH_SECURITY_ON(1)
        MEMORYSTATUSEX memStatus = { 0 };
    memStatus.dwLength = sizeof(memStatus);
    if (!GlobalMemoryStatusEx(&memStatus))
        return false;
    DWORD pageFileSize = (DWORD)(memStatus.ullTotalPageFile / (1024 * 1024));
    DWORD physMemSize = (DWORD)(memStatus.ullTotalPhys / (1024 * 1024));
    // Довольно отфонарное условие
    if (physMemSize > 500 && pageFileSize + physMemSize > 2500)
        return false;
    HINSTANCE hApp = GetModuleHandle(NULL);
    char error[256];
    char message[512];
    if (!LoadString(hApp, RC_VIRT_MEM_ERROR, error, sizeof(error)))
        return false;
    if (!LoadString(hApp, RC_VIRT_MEM_TEXT, message, sizeof(message)))
        return false;
    MessageBox(NULL, message, error, MB_OK | MB_ICONHAND);
    SECUROM_MARKER_HIGH_SECURITY_OFF(1)
        return true;
}

class _SoundProcessor : public pureFrame
{
public:
    virtual void OnFrame()
    {
        if (false)
            Msg("* sound: %d [%3.2f,%3.2f,%3.2f]", u32(Device.dwFrame), VPUSH(Device.vCameraPosition));
        Device.Statistic->Sound.Begin();
        ::Sound->update(Device.vCameraPosition, Device.vCameraDirection, Device.vCameraTop);
        Device.Statistic->Sound.End();
    }
} SoundProcessor;

class KeyFilter
{
private:
    BOOL screensaverState;
    STICKYKEYS stickyKeys;
    FILTERKEYS filterKeys;
    TOGGLEKEYS toggleKeys;
    DWORD stickyKeysFlags;
    DWORD filterKeysFlags;
    DWORD toggleKeysFlags;

public:
    KeyFilter()
    {
        screensaverState = FALSE;
        // Save current state and disable screensaver
        SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &screensaverState, 0);
        if (screensaverState)
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);
        stickyKeysFlags = 0;
        filterKeysFlags = 0;
        toggleKeysFlags = 0;
        stickyKeys = { 0 };
        filterKeys = { 0 };
        toggleKeys = { 0 };
        stickyKeys.cbSize = sizeof(stickyKeys);
        filterKeys.cbSize = sizeof(filterKeys);
        toggleKeys.cbSize = sizeof(toggleKeys);
        SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(stickyKeys), &stickyKeys, 0);
        SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(filterKeys), &filterKeys, 0);
        SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(toggleKeys), &toggleKeys, 0);
        if (stickyKeys.dwFlags & SKF_AVAILABLE)
        {
            stickyKeysFlags = stickyKeys.dwFlags;
            stickyKeys.dwFlags = 0;
            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(stickyKeys), &stickyKeys, 0);
        }
        if (filterKeys.dwFlags & FKF_AVAILABLE)
        {
            filterKeysFlags = filterKeys.dwFlags;
            filterKeys.dwFlags = 0;
            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(filterKeys), &filterKeys, 0);
        }
        if (toggleKeys.dwFlags & TKF_AVAILABLE)
        {
            toggleKeysFlags = toggleKeys.dwFlags;
            toggleKeys.dwFlags = 0;
            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(toggleKeys), &toggleKeys, 0);
        }
    }

    ~KeyFilter()
    {
        // Restore original state
        if (screensaverState)
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, 0);
        if (stickyKeysFlags != 0)
        {
            stickyKeys.dwFlags = stickyKeysFlags;
            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(stickyKeys), &stickyKeys, 0);
        }
        if (filterKeysFlags != 0)
        {
            filterKeys.dwFlags = filterKeysFlags;
            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(filterKeys), &filterKeys, 0);
        }
        if (toggleKeysFlags != 0)
        {
            toggleKeys.dwFlags = toggleKeysFlags;
            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(toggleKeys), &toggleKeys, 0);
        }
    }
};

void InitEngine()
{
    Engine.Initialize();
    while (!IntroFinished)
        Sleep(100);
    Device.Initialize();
    CheckCopyProtection();
}

PROTECT_API void InitSettings()
{
    string_path fname;
    FS.update_path(fname, "$game_config$", "system.ltx");
#ifdef DEBUG
    Msg("Updated path to system.ltx is %s", fname);
#endif // DEBUG
    pSettings = new CInifile(fname, TRUE);
    CHECK_OR_EXIT(!pSettings->sections().empty(),
        make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));
    FS.update_path(fname, "$game_config$", "game.ltx");
    pGameIni = new CInifile(fname, TRUE);
    CHECK_OR_EXIT(!pGameIni->sections().empty(),
        make_string("Cannot find file %s.\nReinstalling application may fix this problem.", fname));
}

PROTECT_API void InitConsole()
{
#ifdef DEDICATED_SERVER
    Console = new CTextConsole();
#else
    Console = new CConsole();
#endif
    Console->Initialize();
    strcpy_s(Console->ConfigFile, "user.ltx");
	const char* ltx = "-ltx ";
    if (char* ltxArg = strstr(Core.Params, ltx))
    {
        ltxArg += strlen(ltx);
        string64 ltxName;
        sscanf(ltxArg, "%[^ ] ", ltxName);
        strcpy_s(Console->ConfigFile, ltxName);
    }
}

PROTECT_API void InitInput()
{
    bool captureInput = !strstr(Core.Params, "-i");
    pInput = new CInput(captureInput);
}

void destroyInput()
{
    xr_delete(pInput);
}

PROTECT_API void InitSound1()
{
    CSound_manager_interface::_create(0);
}

PROTECT_API void InitSound2()
{
    CSound_manager_interface::_create(1);
}

void destroySound()
{
    CSound_manager_interface::_destroy();
}

void destroySettings()
{
    xr_delete(pSettings);
    xr_delete(pGameIni);
}

void destroyConsole()
{
    Console->Execute("cfg_save");
    Console->Destroy();
    xr_delete(Console);
}

void destroyEngine()
{
    Device.Destroy();
    Engine.Destroy();
}

void execUserScript()
{
    Console->Execute("unbindall");
    Console->ExecuteScript(Console->ConfigFile);
}

void slowdownthread(void*)
{
    while (true)
    {
        if (Device.Statistic->fFPS < 30)
            Sleep(1);
        if (Device.mt_bMustExit)
            return;
        if (pSettings == nullptr)
            return;
        if (Console == nullptr)
            return;
        if (pInput == nullptr)
            return;
        if (pApp == nullptr)
            return;
    }
}

void CheckPrivilegySlowdown()
{
#ifdef DEBUG
    if (strstr(Core.Params, "-slowdown"))
    {
        thread_spawn(slowdownthread, "slowdown", 0, 0);
    }
    if (strstr(Core.Params, "-slowdown2x"))
    {
        thread_spawn(slowdownthread, "slowdown", 0, 0);
        thread_spawn(slowdownthread, "slowdown", 0, 0);
    }
#endif // DEBUG
}

void Startup()
{
    InitSound1();
    execUserScript();
    InitSound2();
    // ...command line for auto start
    char* startupCmd = strstr(Core.Params, "-start ");
    if (startupCmd)
        Console->Execute(startupCmd + 1);
    startupCmd = strstr(Core.Params, "-load ");
    if (startupCmd)
        Console->Execute(startupCmd + 1);
    // Initialize APP
    ShowWindow(Device.m_hWnd, SW_SHOWNORMAL);
    Device.Create();
    LALib.OnCreate();
    pApp = new CApplication();
    g_pGamePersistent = (IGame_Persistent*)Engine.External.pCreate(CLSID_GAME_PERSISTANT);
    g_SpatialSpace = new ISpatial_DB();
    g_SpatialSpacePhysic = new ISpatial_DB();
    // Destroy LOGO
#ifndef DEDICATED_SERVER
    DestroyWindow(logoWindow);
    logoWindow = NULL;
#endif
    // Main cycle
    CheckCopyProtection();
    Memory.mem_usage();
    Device.Run();
    // Destroy APP
    xr_delete(g_SpatialSpacePhysic);
    xr_delete(g_SpatialSpace);
    Engine.External.pDestroy(g_pGamePersistent);
    g_pGamePersistent = nullptr;
    xr_delete(pApp);
    Engine.Event.Dump();
    destroyInput();
    if (!g_bBenchmark && !g_SASH.IsRunning())
        destroySettings();
    LALib.OnDestroy();
    if (!g_bBenchmark && !g_SASH.IsRunning())
        destroyConsole();
    else
        Console->Destroy();
    destroySound();
    destroyEngine();
}

#ifndef DEDICATED_SERVER
static BOOL CALLBACK LogoWndProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY:
        break;
    case WM_CLOSE:
        DestroyWindow(hw);
        break;
    case WM_COMMAND:
        if (LOWORD(wp) == IDCANCEL)
            DestroyWindow(hw);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
#endif

int APIENTRY WinMain_impl(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow)
{
    xrDebug::Initialize(g_dedicated_server);
    if (!IsDebuggerPresent())
    {
        // Starting with Windows Vista, the LFH is enabled by default but this call does not cause an error.
        ULONG heapInfo = 2;
        BOOL result = HeapSetInformation(GetProcessHeap(), HeapCompatibilityInformation, &heapInfo, sizeof(heapInfo));
        VERIFY2(result, "can't set process heap low fragmentation");
    }
#ifndef DEDICATED_SERVER
    if (!strstr(lpCmdLine, "--skipmemcheck") && IsOutOfVirtualMemory())
        return 0;
#endif // !DEDICATED_SERVER
#if !defined(DEDICATED_SERVER) && defined(NO_MULTI_INSTANCES)
    HANDLE presenceMutex = OpenMutex(READ_CONTROL, FALSE, STALKER_PRESENCE_MUTEX);
    if (presenceMutex == NULL)
    {
        presenceMutex = CreateMutex(NULL, FALSE, STALKER_PRESENCE_MUTEX);
        if (presenceMutex == NULL)
            return 2;
    }
    else
    {
        CloseHandle(presenceMutex);
        return 0;
    }
#endif // !DEDICATED_SERVER && NO_MULTI_INSTANCES
    SetThreadAffinityMask(GetCurrentThread(), 1);
#ifndef DEDICATED_SERVER
    logoWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STARTUP), 0, LogoWndProc);
    HWND logoPicture = GetDlgItem(logoWindow, IDC_STATIC_LOGO);
    RECT logoRect;
    GetWindowRect(logoPicture, &logoRect);
#ifndef DEBUG
    HWND insertAfter = HWND_TOPMOST;
#else
    HWND insertAfter = HWND_NOTOPMOST;
#endif // NDEBUG
    SetWindowPos(logoWindow, insertAfter, 0, 0, logoRect.right - logoRect.left,
        logoRect.bottom - logoRect.top, SWP_NOMOVE | SWP_SHOWWINDOW);
    UpdateWindow(logoWindow);
#endif
    IntroFinished = true;
    const char* fsltx = "-fsltx ";
    string_path fsgame = "";
    if (char* fsltxPath = strstr(lpCmdLine, fsltx))
    {
        fsltxPath += strlen(fsltx);
        sscanf(fsltxPath, "%[^ ] ", fsgame);
    }
    Core._initialize("xray", NULL, TRUE, strlen(fsgame) > 0 ? fsgame : nullptr);
    InitSettings();
    // Adjust player & computer name for Asian
    if (pSettings->line_exist("string_table", "no_native_input"))
    {
        strcpy_s(Core.UserName, sizeof(Core.UserName), "Player");
        strcpy_s(Core.CompName, sizeof(Core.CompName), "Computer");
    }
#ifndef DEDICATED_SERVER
    KeyFilter filter;
    (void)filter;
#endif // !DEDICATED_SERVER
    FPU::m24r();
    InitEngine();
    InitInput();
    InitConsole();
    Engine.External.CreateRendererList();
    const char* benchmark = "-batch_benchmark ";
    if (char* cmdBenchmark = strstr(lpCmdLine, benchmark))
    {
        cmdBenchmark += strlen(benchmark);
        string64 benchmarkName;
        sscanf(cmdBenchmark, "%[^ ] ", benchmarkName);
        doBenchmark(benchmarkName);
        return 0;
    }
    Msg("command line %s", lpCmdLine);
    const char* sash = "-openautomate ";
    if (char* cmdSash = strstr(lpCmdLine, sash))
    {
        cmdSash += strlen(sash);
        string512 sashArgs;
        sscanf(cmdSash, "%[^ ] ", sashArgs);
        g_SASH.Init(sashArgs);
        g_SASH.MainLoop();
        return 0;
    }
#ifndef DEDICATED_SERVER
    if (strstr(Core.Params, "-r2a"))
    {
        Console->Execute("renderer renderer_r2a");
    }
    else if (strstr(Core.Params, "-r2"))
    {
        Console->Execute("renderer renderer_r2");
    }
    else
    {
        // XXX nitrocaster: construct CCC_LoadCFG_custom on stack?
        auto cmd = new CCC_LoadCFG_custom("renderer ");
        cmd->Execute(Console->ConfigFile);
        xr_delete(cmd);
    }
#else
    Console->Execute("renderer renderer_r1");
#endif
    Engine.External.Initialize();
    Console->Execute("stat_memory");
    Startup();
    Core._destroy();
#if !defined(DEDICATED_SERVER) && defined(NO_MULTI_INSTANCES)
    CloseHandle(presenceMutex);
#endif // !DEDICATED_SERVER && NO_MULTI_INSTANCES
    return 0;
}

int StackOverflowFilter(int exceptionCode)
{
    if (exceptionCode == EXCEPTION_STACK_OVERFLOW)
    {
        // Do not call _resetstkoflw here, because
        // at this point, the stack is not yet unwound.
        // Instead, signal that the handler (the __except block)
        // is to be executed.
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow)
{
    __try
    {
        WinMain_impl(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    }
    __except (StackOverflowFilter(GetExceptionCode()))
    {
        _resetstkoflw();
        FATAL("stack overflow");
    }
    return 0;
}

LPCSTR _GetFontTexName(LPCSTR section)
{
    static char* textureNames[] = { "texture800", "texture", "texture1600" };
    int defaultIndex = 1; // default 1024x768
    int selectedIndex = defaultIndex;
    u32 h = Device.dwHeight;
    if (h <= 600)
        selectedIndex = 0;
    else if (h < 1024)
        selectedIndex = 1;
    else
        selectedIndex = 2;
    while (selectedIndex >= 0)
    {
        if (pSettings->line_exist(section, textureNames[selectedIndex]))
            return pSettings->r_string(section, textureNames[selectedIndex]);
        selectedIndex--;
    }
    return pSettings->r_string(section, textureNames[defaultIndex]);
}

void _InitializeFont(CGameFont*& F, LPCSTR section, u32 flags)
{
    auto fontTextureName = _GetFontTexName(section);
    R_ASSERT(fontTextureName);
    if (!F)
    {
        F = new CGameFont("font", fontTextureName, flags);
        Device.seqRender.Add(F, REG_PRIORITY_LOW - 1000);
    }
    else
    {
        F->Initialize("font", fontTextureName);
    }
    if (pSettings->line_exist(section, "size"))
    {
        float sz = pSettings->r_float(section, "size");
        if (flags & CGameFont::fsDeviceIndependent)
            F->SetHeightI(sz);
        else
            F->SetHeight(sz);
    }
    if (pSettings->line_exist(section, "interval"))
        F->SetInterval(pSettings->r_fvector2(section, "interval"));
}

CApplication::CApplication()
{
    ll_dwReference = 0;
    max_load_stage = 0;
    // events
    eQuit = Engine.Event.Handler_Attach("KERNEL:quit", this);
    eStart = Engine.Event.Handler_Attach("KERNEL:start", this);
    eStartLoad = Engine.Event.Handler_Attach("KERNEL:load", this);
    eDisconnect = Engine.Event.Handler_Attach("KERNEL:disconnect", this);
    eConsole = Engine.Event.Handler_Attach("KERNEL:console", this);
    // levels
    Level_Current = 0;
    Level_Scan();
    // Font
    pFontSystem = nullptr;
    // Register us
    Device.seqFrame.Add(this, REG_PRIORITY_HIGH + 1000);
    if (psDeviceFlags.test(mtSound))
        Device.seqFrameMT.Add(&SoundProcessor);
    else
        Device.seqFrame.Add(&SoundProcessor);
    Console->Show();
    app_title[0] = '\0';
}

CApplication::~CApplication()
{
    Console->Hide();
    // font
    Device.seqRender.Remove(pFontSystem);
    xr_delete(pFontSystem);
    Device.seqFrameMT.Remove(&SoundProcessor);
    Device.seqFrame.Remove(&SoundProcessor);
    Device.seqFrame.Remove(this);
    // events
    Engine.Event.Handler_Detach(eConsole, this);
    Engine.Event.Handler_Detach(eDisconnect, this);
    Engine.Event.Handler_Detach(eStartLoad, this);
    Engine.Event.Handler_Detach(eStart, this);
    Engine.Event.Handler_Detach(eQuit, this);
}

void CApplication::OnEvent(EVENT E, u64 P1, u64 P2)
{
    if (E == eQuit)
    {
        g_SASH.EndBenchmark();
        PostQuitMessage(0);
        for (sLevelInfo& level : Levels)
        {
            xr_free(level.folder);
            xr_free(level.name);
        }
    }
    else if (E == eStart)
    {
        char* opServer = (char*)P1;
        char* opClient = (char*)P2;
        R_ASSERT(!g_pGameLevel);
        R_ASSERT(g_pGamePersistent);
    #ifdef NO_SINGLE
        Console->Execute("main_menu on");
        if (!opServer || !strlen(opServer) ||
            ((strstr(opServer, "/dm") || strstr(opServer, "/deathmatch") ||
            strstr(opServer, "/tdm") || strstr(opServer, "/teamdeathmatch") ||
            strstr(opServer, "/ah") || strstr(opServer, "/artefacthunt") ||
            strstr(opServer, "/cta") || strstr(opServer, "/capturetheartefact")) &&
            !strstr(opServer, "/alife")))
    #endif // #ifdef NO_SINGLE
        {
            Console->Execute("main_menu off");
            Console->Hide();
            Device.Reset(false);
            g_pGamePersistent->PreStart(opServer);
            g_pGameLevel = (IGame_Level*)Engine.External.pCreate(CLSID_GAME_LEVEL);
            pApp->LoadBegin();
            g_pGamePersistent->Start(opServer);
            g_pGameLevel->net_Start(opServer, opClient);
            pApp->LoadEnd();
        }
        xr_free(opServer);
        xr_free(opClient);
    }
    else if (E == eDisconnect)
    {
        if (g_pGameLevel)
        {
            Console->Hide();
            g_pGameLevel->net_Stop();
            Engine.External.pDestroy(g_pGameLevel);
            g_pGameLevel = nullptr;
            Console->Show();
            if (!Engine.Event.Peek("KERNEL:quit") && !Engine.Event.Peek("KERNEL:start"))
            {
                Console->Execute("main_menu off");
                Console->Execute("main_menu on");
            }
        }
        R_ASSERT(g_pGamePersistent);
        g_pGamePersistent->Disconnect();
    }
    else if (E == eConsole)
    {
        LPSTR command = (LPSTR)P1;
        Console->ExecuteCommand(command, false);
        xr_free(command);
    }
}

void CApplication::LoadBegin()
{
    ll_dwReference++;
    if (ll_dwReference == 1)
    {
        g_appLoaded = FALSE;
    #ifndef DEDICATED_SERVER
        _InitializeFont(pFontSystem, "ui_font_graffiti19_russian", 0);
        m_pRender->LoadBegin();
    #endif
        PhaseTimer.Start();
        load_stage = 0;
        CheckCopyProtection();
    }
}

void CApplication::LoadEnd()
{
    ll_dwReference--;
    if (ll_dwReference == 0)
    {
        Msg("* phase time: %d ms", PhaseTimer.GetElapsed_ms());
        Msg("* phase cmem: %d K", Memory.mem_usage() / 1024);
        Console->Execute("stat_memory");
        g_appLoaded = TRUE;
        if (false) // enable to dump memory manager statistics
            DUMP_PHASE;
    }
}

void CApplication::destroy_loading_shaders()
{
    m_pRender->destroy_loading_shaders();
}

PROTECT_API void CApplication::LoadDraw()
{
    if (g_appLoaded)
        return;
    Device.dwFrame++;
    if (!Device.Begin())
        return;
    if (g_dedicated_server)
        Console->OnRender();
    else
        load_draw_internal();
    Device.End();
    CheckCopyProtection();
}

void CApplication::LoadTitleInt(LPCSTR str)
{
    load_stage++;
    VERIFY(ll_dwReference);
    VERIFY(str && strlen(str) < 256);
    strcpy_s(app_title, str);
    Msg("* phase time: %d ms", PhaseTimer.GetElapsed_ms());
    PhaseTimer.Start();
    Msg("* phase cmem: %d K", Memory.mem_usage() / 1024);
    Log(app_title);
    if (g_pGamePersistent->GameType() == EGameIDs::eGameIDSingle &&
        strstr(Core.Params, "alife"))
    {
        max_load_stage = 17;
    }
    else
    {
        max_load_stage = 14;
    }
    LoadDraw();
}

void CApplication::LoadSwitch()
{
}

void CApplication::OnFrame()
{
    Engine.Event.OnFrame();
    g_SpatialSpace->update();
    g_SpatialSpacePhysic->update();
    if (g_pGameLevel)
        g_pGameLevel->SoundEvent_Dispatch();
}

void CApplication::Level_Append(LPCSTR folder)
{
    string_path	level, ltx, geom, cform;
    strconcat(sizeof(level), level, folder, "level");
    strconcat(sizeof(ltx), ltx, folder, "level.ltx");
    strconcat(sizeof(geom), geom, folder, "level.geom");
    strconcat(sizeof(cform), cform, folder, "level.cform");
    if (FS.exist("$game_levels$", level) &&
        FS.exist("$game_levels$", ltx) &&
        FS.exist("$game_levels$", geom) &&
        FS.exist("$game_levels$", cform))
    {
        sLevelInfo levelInfo;
        levelInfo.folder = xr_strdup(folder);
        levelInfo.name = nullptr;
        Levels.push_back(levelInfo);
    }
}

void CApplication::Level_Scan()
{
    for (u32 i = 0; i < Levels.size(); i++)
    {
        xr_free(Levels[i].folder);
        xr_free(Levels[i].name);
    }
    Levels.clear();
    auto folder = FS.file_list_open("$game_levels$", FS_ListFolders | FS_RootOnly);
    if (false)
        R_ASSERT(folder && folder->size());
    for (u32 i = 0; i < folder->size(); i++)
        Level_Append((*folder)[i]);
    FS.file_list_close(folder);
}

void CApplication::Level_Set(u32 L)
{
    if (L >= Levels.size())
        return;
    Level_Current = L;
    FS.get_path("$level$")->_set(Levels[L].folder);
    string_path temp, temp2;
    strconcat(sizeof(temp), temp, "intro\\intro_", Levels[L].folder);
    temp[xr_strlen(temp) - 1] = 0;
    if (FS.exist(temp2, "$game_textures$", temp, ".dds") || FS.exist(temp2, "$level$", temp, ".dds"))
        m_pRender->setLevelLogo(temp);
    else
        m_pRender->setLevelLogo("intro\\intro_no_start_picture");
    CheckCopyProtection();
}

int CApplication::Level_ID(LPCSTR name, LPCSTR ver, bool bSet)
{
    int result = -1;
    bool arch_res = false;
    for (auto& archive : FS.m_archives)
    {
        if (archive.hSrcFile == NULL)
        {
            LPCSTR ln = archive.header->r_string("header", "level_name");
            LPCSTR lv = archive.header->r_string("header", "level_ver");
            if (stricmp(ln, name) == 0 && stricmp(lv, ver) == 0)
            {
                FS.LoadArchive(archive);
                arch_res = true;
            }
        }
    }
    if (arch_res)
        Level_Scan();
    string256 buffer;
    strconcat(sizeof(buffer), buffer, name, "\\");
    for (u32 I = 0; I < Levels.size(); ++I)
    {
        if (0 == stricmp(buffer, Levels[I].folder))
        {
            result = int(I);
            break;
        }
    }
    if (bSet && result != -1)
        Level_Set(result);
    if (arch_res)
        g_pGamePersistent->OnAssetsChanged();
    return result;
}

CInifile* CApplication::GetArchiveHeader(LPCSTR name, LPCSTR ver)
{
    for (CLocatorAPI::archive& archive : FS.m_archives)
    {
        LPCSTR ln = archive.header->r_string("header", "level_name");
        LPCSTR lv = archive.header->r_string("header", "level_ver");
        if (stricmp(ln, name) == 0 && stricmp(lv, ver) == 0)
        {
            return archive.header;
        }
    }
    return nullptr;
}

void CApplication::LoadAllArchives()
{
    if (FS.load_all_unloaded_archives())
    {
        Level_Scan();
        g_pGamePersistent->OnAssetsChanged();
    }
}

void doBenchmark(const char* name)
{
    g_bBenchmark = true;
    string_path configPath;
    FS.update_path(configPath, "$app_data_root$", name);
    CInifile ini(configPath);
    int testCount = ini.line_count("benchmark");
    LPCSTR test_name, t;
    for (int i = 0; i < testCount; i++)
    {
        ini.r_line("benchmark", i, &test_name, &t);
        strcpy_s(g_sBenchmarkName, test_name);
        shared_str test_command = ini.r_string_wb("benchmark", test_name);
        Core.Params = (char*)xr_realloc(Core.Params, test_command.size() + 1);
        strcpy(Core.Params, *test_command);
        strlwr(Core.Params);
        InitInput();
        if (i != 0)
        {
            // XXX nitrocaster: why?
            //ZeroMemory(&HW,sizeof(CHW));
            //	TODO: KILL HW here!
            //  pApp->m_pRender->KillHW();
            InitEngine();
        }
        Engine.External.Initialize();
        strcpy_s(Console->ConfigFile, "user.ltx");
        const char* ltx = "-ltx ";
        if (char* ltxArg = strstr(Core.Params, ltx))
        {
            ltxArg += strlen(ltx);
            string64 ltxName;
            sscanf(ltxArg, "%[^ ] ", ltxName);
            strcpy_s(Console->ConfigFile, ltxName);
        }
        Startup();
    }
}

#pragma optimize("g", off)
void CApplication::load_draw_internal()
{
    m_pRender->load_draw_internal(*this);
}
