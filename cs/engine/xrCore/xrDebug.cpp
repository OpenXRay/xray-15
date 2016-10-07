#include "stdafx.h"
#pragma hdrstop

#include "xrDebug.h"
#include "os_clipboard.h"
#include "dxerr.h"
#include "Debug/StackTrace.h"
#include "Debug/MiniDump.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#include <direct.h>
#pragma warning(pop)

extern bool shared_str_initialized;

#define USE_BUG_TRAP

#ifdef __BORLANDC__
#include "d3d9.h"
#include "d3dx9.h"
#include "D3DX_Wrapper.h"
#pragma comment(lib, "EToolsB.lib")
#endif

#ifndef USE_BUG_TRAP
#include <exception>
#endif

#ifndef __BORLANDC__
#pragma comment(lib, "dxerr.lib")
#endif

#include <dbghelp.h> // MiniDump flags

#ifdef USE_BUG_TRAP
#include <bugtrap.h> // for BugTrap functionality
#ifndef __BORLANDC__
#pragma comment(lib, "BugTrap.lib")
#else
#pragma comment(lib, "BugTrapB.lib")
#endif
#endif

#include <new.h> // for _set_new_mode
#include <signal.h> // for signals

#ifdef DEBUG
#define USE_OWN_ERROR_MESSAGE_WINDOW
#else
#define USE_OWN_MINI_DUMP
#endif

using namespace XRay;

intptr __declspec(naked, noinline) __cdecl GetInstructionPtr()
{
#ifdef _WIN64
    _asm mov rax, [rsp]
    _asm retn
#else
    _asm mov eax, [esp]
    _asm retn
#endif
}

char xrDebug::StackTraceInfo::Frames[Capacity*LineCapacity];
int xrDebug::StackTraceInfo::Count;
xrDebug::StackTraceInfo xrDebug::StackTrace;
xrDebug::UnhandledExceptionFilterType* xrDebug::PrevFilter = nullptr;
xrDebug::OnCrashHandler* xrDebug::OnCrash = nullptr;
xrDebug::OnDialogHandler* xrDebug::OnDialog = nullptr;
string_path xrDebug::BugReportFile;
bool xrDebug::ErrorAfterDialog = false;

void xrDebug::SetBugReportFile(const char* fileName)
{
    strcpy_s(BugReportFile, fileName);
}

void xrDebug::LogStackTrace(const char* header)
{
    if (!shared_str_initialized)
        return;
    StackTrace.Count = BuildStackTrace(StackTrace.Frames, StackTrace.Capacity, StackTrace.LineCapacity);
    Msg("%s", header);
    for (int i = 1; i < StackTrace.Count; i++)
        Msg("%s", StackTrace.GetFrame(i));
}

int xrDebug::BuildStackTrace(char* buffer, int capacity, int lineCapacity)
{
    CONTEXT context;
    EXCEPTION_POINTERS ex_ptrs;
    intptr ebp;
    context.ContextFlags = CONTEXT_FULL;
    if (GetThreadContext(GetCurrentThread(), &context))
    {
        context.Eip = GetInstructionPtr();
        context.Ebp = (intptr)&ebp;
        context.Esp = (intptr)&context;
        ex_ptrs.ContextRecord = &context;
        ex_ptrs.ExceptionRecord = 0;
        return BuildStackTrace(&ex_ptrs, buffer, capacity, lineCapacity);
    }
    return 0;
}

int xrDebug::BuildStackTrace(EXCEPTION_POINTERS* exPtrs, char* buffer, int capacity, int lineCapacity)
{
    memset(buffer, capacity*lineCapacity, 0);
    auto flags = GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
    auto traceDump = GetFirstStackTraceString(flags, exPtrs);
    int frameCount = 0;
    while (traceDump)
    {
        lstrcpy(buffer + frameCount*lineCapacity, traceDump);
        frameCount++;
        traceDump = GetNextStackTraceString(flags, exPtrs);
    }
    return frameCount;
}

void xrDebug::GatherInfo(const char* expression, const char* description, const char* arg0, const char* arg1,
    const char* file, int line, const char* function, char* assertionInfo)
{
    char* buffer = assertionInfo;
    bool extendedDesc = description && strchr(description, '\n');
    char* prefix = "[error] ";
    buffer += sprintf(buffer, "\nFATAL ERROR\n\n");
    buffer += sprintf(buffer, "%sExpression    : %s\n", prefix, expression);
    buffer += sprintf(buffer, "%sFunction      : %s\n", prefix, function);
    buffer += sprintf(buffer, "%sFile          : %s\n", prefix, file);
    buffer += sprintf(buffer, "%sLine          : %d\n", prefix, line);
    if (extendedDesc)
    {
        buffer += sprintf(buffer, "\n%s\n", description);
        if (arg0)
        {
            buffer += sprintf(buffer, "%s\n", arg0);
            if (arg1)
            {
                buffer += sprintf(buffer, "%s\n", arg1);
            }
        }
    }
    else
    {
        buffer += sprintf(buffer, "%sDescription   : %s\n", prefix, description);
        if (arg0)
        {
            if (arg1)
            {
                buffer += sprintf(buffer, "%sArgument 0    : %s\n", prefix, arg0);
                buffer += sprintf(buffer, "%sArgument 1    : %s\n", prefix, arg1);
            }
            else
                buffer += sprintf(buffer, "%sArguments     : %s\n", prefix, arg0);
        }
    }
    buffer += sprintf(buffer, "\n");
    if (shared_str_initialized)
    {
        Log(assertionInfo);
        FlushLog();
    }
    buffer = assertionInfo;
#ifdef USE_MEMORY_MONITOR
    memory_monitor::flush_each_time(true);
    memory_monitor::flush_each_time(false);
#endif // USE_MEMORY_MONITOR
    if (IsDebuggerPresent() || !strstr(GetCommandLine(), "-no_call_stack_assert"))
        return;
    if (shared_str_initialized)
        Log("stack trace:\n");
#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
    buffer += sprintf(buffer, "stack trace:\n\n");
#endif // USE_OWN_ERROR_MESSAGE_WINDOW
    BuildStackTrace(StackTrace.Frames, StackTrace.Capacity, StackTrace.LineCapacity);
	for (int i = 2; i < StackTrace.Count; i++)
    {
		if (shared_str_initialized)
			Log(StackTrace.GetFrame(i));
    #ifdef USE_OWN_ERROR_MESSAGE_WINDOW
		buffer += sprintf(buffer, "%s\n", StackTrace.GetFrame(i));
    #endif // USE_OWN_ERROR_MESSAGE_WINDOW
    }
    if (shared_str_initialized)
        FlushLog();
    os_clipboard::copy_to_clipboard(assertionInfo);
}

void xrDebug::DoExit(const std::string& message)
{
    FlushLog();
    MessageBox(NULL, message.c_str(), "Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
    TerminateProcess(GetCurrentProcess(), 1);
}

void xrDebug::Backend(const char* expression, const char* description, const char* arg0, const char* arg1,
    const char* file, int line, const char* function, bool& ignoreAlways)
{
#ifdef PROFILE_CRITICAL_SECTIONS
    static xrCriticalSection CS(MUTEX_PROFILE_ID(xrDebug::Backend));
#else
    static xrCriticalSection lock;
#endif
    lock.Enter();
    ErrorAfterDialog = true;
    string4096 assertionInfo;
    GatherInfo(expression, description, arg0, arg1, file, line, function, assertionInfo);
#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
    strcat(assertionInfo,
        "\r\n"
        "Press CANCEL to abort execution\r\n"
        "Press TRY AGAIN to continue execution\r\n"
        "Press CONTINUE to continue execution and ignore all the errors of this type\r\n"
        "\r\n");
#endif
    if (OnCrash)
        OnCrash();
    if (OnDialog)
        OnDialog(true);
#ifdef XRCORE_STATIC
    MessageBox(NULL, assertionInfo, "X-Ray error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
#else // !XRCORE_STATIC
#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
    int result = MessageBox(GetTopWindow(NULL), assertionInfo, "Fatal error",
        MB_CANCELTRYCONTINUE | MB_ICONERROR | MB_SYSTEMMODAL);
    switch (result)
    {
    case IDCANCEL:
    default:
    #ifdef USE_BUG_TRAP
        BT_SetUserMessage(assertionInfo);
    #endif
        DebugBreak();
        break;
    case IDTRYAGAIN:
        ErrorAfterDialog = false;
        break;
    case IDCONTINUE:
        ErrorAfterDialog = false;
        ignoreAlways = true;
        break;
	}
#else // !USE_OWN_ERROR_MESSAGE_WINDOW
#ifdef USE_BUG_TRAP
    BT_SetUserMessage(assertionInfo);
#endif
	DebugBreak();
#endif
#endif
    if (OnDialog)
        OnDialog(false);
    lock.Leave();
}

LPCSTR xrDebug::ErrorToString(long code)
{
    static string1024 descStorage;
    const char* errorDesc = DXGetErrorDescription(code);
    if (!errorDesc)
    {
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, code, 0, descStorage, sizeof(descStorage) - 1, 0);
        errorDesc = descStorage;
    }
    return errorDesc;
}

void xrDebug::Error(long hr, const char* expr, const char* file, int line, const char* function, bool& ignoreAlways)
{
    Backend(expr, ErrorToString(hr), nullptr, nullptr, file, line, function, ignoreAlways);
}

void xrDebug::Error(long hr, const char* expr, const char* e2, const char* file, int line, const char* function,
    bool& ignoreAlways)
{
    Backend(expr, ErrorToString(hr), e2, 0, file, line, function, ignoreAlways);
}

void xrDebug::Fail(const char* e1, const char* file, int line, const char* function, bool& ignoreAlways)
{
    Backend(e1, "assertion failed", 0, 0, file, line, function, ignoreAlways);
}

void xrDebug::Fail(const char* e1, const std::string &e2, const char* file, int line, const char* function,
    bool& ignoreAlways)
{
    Backend(e1, e2.c_str(), 0, 0, file, line, function, ignoreAlways);
}

void xrDebug::Fail(const char* e1, const char* e2, const char* file, int line, const char* function,
    bool& ignoreAlways)
{
    Backend(e1, e2, 0, 0, file, line, function, ignoreAlways);
}

void xrDebug::Fail(const char* e1, const char* e2, const char* e3, const char* file, int line, const char* function,
    bool& ignoreAlways)
{
    Backend(e1, e2, e3, 0, file, line, function, ignoreAlways);
}

void xrDebug::Fail(const char* e1, const char* e2, const char* e3, const char* e4, const char* file, int line,
    const char* function, bool& ignoreAlways)
{
    Backend(e1, e2, e3, e4, file, line, function, ignoreAlways);
}

void xrDebug::Fatal(const char* file, int line, const char* function, const char* format, ...)
{
    string1024 buffer;
    va_list p;
    va_start(p, format);
    vsprintf(buffer, format, p);
    va_end(p);
    bool ignoreAlways = true;
    Backend("<no expression>", "fatal error", buffer, 0, file, line, function, ignoreAlways);
}

static int out_of_memory_handler(size_t size)
{
    Memory.mem_compact();
    uint processHeap = Memory.mem_usage();
    uint ecoStrings = g_pStringContainer->stat_economy();
    uint ecoSmem = g_pSharedMemoryContainer->stat_economy();
    Msg("* [x-ray]: process heap[%d K]", processHeap / 1024);
    Msg("* [x-ray]: economy: strings[%d K], smem[%d K]", ecoStrings / 1024, ecoSmem);
    xrDebug::Fatal(DEBUG_INFO, "Out of memory. Memory request: %d K", size / 1024);
    return 1;
}

extern LPCSTR log_name();

#ifdef USE_BUG_TRAP
void WINAPI xrDebug::PreErrorHandler(INT_PTR)
{
    if (!xr_FS || !FS.m_Flags.test(CLocatorAPI::flReady))
        return;
    string_path logDir;
    __try
    {
        FS.update_path(logDir, "$logs$", "");
        if (logDir[0] != '\\' && logDir[1] != ':')
        {
            string256 currentDir;
            _getcwd(currentDir, sizeof(currentDir));
            string256 relDir;
            strcpy_s(relDir, logDir);
            strconcat(sizeof(logDir), logDir, currentDir, "\\", relDir);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        strcpy_s(logDir, "logs");
    }
    string_path temp;
    strconcat(sizeof(temp), temp, logDir, log_name());
    BT_AddLogFile(temp);
    if (BugReportFile[0] != 0)
        BT_AddLogFile(BugReportFile);
    BT_SaveSnapshot(nullptr);
}

void xrDebug::SetupExceptionHandler(const bool& dedicated)
{
    BT_InstallSehFilter();
    if (!dedicated && !strstr(GetCommandLine(), "-silent_error_mode"))
        BT_SetActivityType(BTA_SHOWUI);
    else
        BT_SetActivityType(BTA_SAVEREPORT);
    BT_SetDialogMessage(BTDM_INTRO2,
        "This is X-Ray Engine v1.5 crash reporting client. "
        "To help the development process, "
        "please Submit Bug or save report and email it manually (button More...)."
        "\r\n"
        "Many thanks in advance and sorry for the inconvenience.");
    BT_SetPreErrHandler(PreErrorHandler, 0);
    BT_SetAppName("X-Ray Engine");
    BT_SetReportFormat(BTRF_TEXT);
    BT_SetFlags(BTF_DETAILEDMODE | BTF_ATTACHREPORT);
#ifdef MASTER_GOLD
#ifdef _EDITOR // MASTER_GOLD && EDITOR
    auto minidumpFlags = !dedicated ? MiniDumpNoDump : MiniDumpWithDataSegs;
#else // MASTER_GOLD && !EDITOR
    auto minidumpFlags = !dedicated ? MiniDumpNoDump : MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory;
#endif
#else
#ifdef EDITOR // !MASTER_GOLD && EDITOR
    auto minidumpFlags = MiniDumpWithDataSegs;
#else // !MASTER_GOLD && !EDITOR
    auto minidumpFlags = MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory;
#endif
#endif
    BT_SetDumpType(minidumpFlags);
    // XXX nitrocaster: use some other email
    BT_SetSupportEMail("cs-crash-report@stalker-game.com");
}
#endif // USE_BUG_TRAP

#ifdef USE_OWN_MINI_DUMP
void xrDebug::SaveMiniDump(EXCEPTION_POINTERS* exPtrs)
{
    string64 dateStr;
    timestamp(dateStr);
    string_path	dumpPath;
    sprintf(dumpPath, "%s_%s_%s.mdmp", Core.ApplicationName, Core.UserName, dateStr);
    __try
    {
        if (FS.path_exist("$logs$"))
            FS.update_path(dumpPath, "$logs$", dumpPath);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        string_path	temp;
        strcpy_s(temp, dumpPath);
        sprintf(dumpPath, "logs/%s", temp);
    }
    WriteMiniDump(MINIDUMP_TYPE(MiniDumpFilterMemory | MiniDumpScanMemory),
        dumpPath, GetCurrentThreadId(), exPtrs);
}
#endif

void xrDebug::FormatLastError(char* buffer, const size_t& bufferSize)
{
    int lastErr = GetLastError();
    if (lastErr == ERROR_SUCCESS)
    {
        *buffer = 0;
        return;
    }
    void* msg = nullptr;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        lastErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msg,
        0,
        nullptr);
    // XXX nitrocaster: check buffer overflow
    sprintf(buffer, "[error][%8d]: %s", lastErr, msg);
    LocalFree(msg);
}

LONG WINAPI xrDebug::UnhandledFilter(EXCEPTION_POINTERS* exPtrs)
{
    string256 errMsg;
    FormatLastError(errMsg, sizeof(errMsg));
	if (!ErrorAfterDialog && !strstr(GetCommandLine(),"-no_call_stack_assert"))
    {
        CONTEXT save = *exPtrs->ContextRecord;
        StackTrace.Count = BuildStackTrace(exPtrs, StackTrace.Frames, StackTrace.Capacity, StackTrace.LineCapacity);
        *exPtrs->ContextRecord = save;
        if (shared_str_initialized)
            Msg("stack trace:\n");
        if (!IsDebuggerPresent())
            os_clipboard::copy_to_clipboard("stack trace:\r\n\r\n");
        string4096 buffer;
        for (int i = 0; i < StackTrace.Count; i++)
        {
            if (shared_str_initialized)
                Log(StackTrace.GetFrame(i));
            sprintf(buffer, "%s\r\n", StackTrace.GetFrame(i));
#ifdef DEBUG
			if (!IsDebuggerPresent())
				os_clipboard::update_clipboard(buffer);
#endif
		}
        if (errMsg[0] != 0)
        {
			if (shared_str_initialized)
                Msg("\n%s", errMsg);
            strcat(errMsg, "\r\n");
#ifdef DEBUG
			if (!IsDebuggerPresent())
				os_clipboard::update_clipboard(buffer);
#endif
		}
	}
	if (shared_str_initialized)
		FlushLog();
#ifndef USE_OWN_ERROR_MESSAGE_WINDOW
#ifdef USE_OWN_MINI_DUMP
    SaveMiniDump(exPtrs);
#endif
#else
    if (!ErrorAfterDialog)
    {
        if (OnDialog)
            OnDialog(true);
        MessageBox(NULL, "Fatal error occured\n\n"
            "Press OK to abort program execution", "Fatal error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
    }
#endif
    if (PrevFilter)
        PrevFilter(exPtrs);
#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
    if (OnDialog)
        OnDialog(false);
#endif
    return EXCEPTION_CONTINUE_SEARCH;
}

#ifdef M_BORLAND
namespace std
{
extern new_handler _RTLENTRY _EXPFUNC set_new_handler(new_handler new_p);
};

static void __cdecl def_new_handler()
{
    FATAL("Out of memory.");
}

void xrDebug::Initialize(const bool& dedicated)
{
    OnCrash = 0;
    OnDialog = 0;
    std::set_new_handler(def_new_handler); // exception-handler for 'out of memory' condition
    //::SetUnhandledExceptionFilter	(UnhandledFilter);	// exception handler to all "unhandled" exceptions
}
#else // !M_BORLAND

#ifndef USE_BUG_TRAP
void _terminate()
{
    if (strstr(GetCommandLine(), "-silent_error_mode"))
        exit(-1);
    string4096 assertionInfo;
    xrDebug::GatherInfo("<no expression>", "Unexpected application termination", nullptr, nullptr,
        DEBUG_INFO, assertionInfo);
    strcat(assertionInfo, "Press OK to abort execution\r\n");
    MessageBox(GetTopWindow(NULL), assertionInfo, "Fatal Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
    exit(-1);
}
#endif // USE_BUG_TRAP

static void handler_base(const char* reason)
{
    bool ignoreAlways = false;
    xrDebug::Backend("<no expression>", reason, nullptr, nullptr, DEBUG_INFO, ignoreAlways);
}

static void invalid_parameter_handler(const wchar_t* expression, const wchar_t* function,
    const wchar_t* file, unsigned int line, uintptr_t reserved)
{
    bool ignoreAlways = false;
    string4096 mbExpression;
    string4096 mbFunction;
    string4096 mbFile;
    size_t convertedChars = 0;
    if (expression)
        wcstombs_s(&convertedChars, mbExpression, sizeof(mbExpression), expression, (wcslen(expression) + 1) * 2);
    else
        strcpy_s(mbExpression, "");
    if (function)
        wcstombs_s(&convertedChars, mbFunction, sizeof(mbFunction), function, (wcslen(function) + 1) * 2);
    else
        strcpy_s(mbFunction, __FUNCTION__);
    if (file)
        wcstombs_s(&convertedChars, mbFile, sizeof(mbFile), file, (wcslen(file) + 1) * 2);
    else
    {
        line = __LINE__;
        strcpy_s(mbFile, __FILE__);
    }
    xrDebug::Backend(mbExpression, "invalid parameter", nullptr, nullptr, mbFile, line, mbFunction, ignoreAlways);
}

static void pure_call_handler()
{
    handler_base("pure virtual function call");
}

static void abort_handler(int signal)
{
    handler_base("application is aborting");
}

static void floating_point_handler(int signal)
{
    handler_base("floating point error");
}

static void illegal_instruction_handler(int signal)
{
    handler_base("illegal instruction");
}

static void termination_handler(int signal)
{
    handler_base("termination with exit code 3");
}

void xrDebug::OnThreadSpawn()
{
#ifdef USE_BUG_TRAP
    BT_SetTerminate();
#else // !USE_BUG_TRAP
    // std::set_terminate(_terminate);
#endif
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    signal(SIGABRT, abort_handler);
    signal(SIGABRT_COMPAT, abort_handler);
    signal(SIGFPE, floating_point_handler);
    signal(SIGILL, illegal_instruction_handler);
    signal(SIGINT, 0);
    signal(SIGTERM, termination_handler);
    _set_invalid_parameter_handler(&invalid_parameter_handler);
    _set_new_mode(1);
    _set_new_handler(&out_of_memory_handler);
    _set_purecall_handler(&pure_call_handler);
#if 0// should be if we use exceptions
    std::set_unexpected(_terminate);
#endif
}

void xrDebug::Initialize(const bool& dedicated)
{
    BugReportFile[0] = 0;
    OnThreadSpawn();
    SetupExceptionHandler(dedicated);
    // exception handler to all "unhandled" exceptions
    PrevFilter = ::SetUnhandledExceptionFilter(UnhandledFilter);
}
#endif
