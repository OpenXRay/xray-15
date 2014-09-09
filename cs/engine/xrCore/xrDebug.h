#pragma once
#include "_types.h"

class XRCORE_API xrDebug
{
private:
    struct StackTraceInfo
    {
        static const int Capacity = 100;
        static const int LineCapacity = 256;
        static char Frames[Capacity*LineCapacity];
        static int Count;

        static IC char* GetFrame(int i) { return Frames + i*LineCapacity; }
    };
    static StackTraceInfo StackTrace;
    typedef	void OnCrashHandler();
    typedef	void OnDialogHandler(bool);
    typedef LONG WINAPI UnhandledExceptionFilterType(EXCEPTION_POINTERS* exPtrs);
    static UnhandledExceptionFilterType* PrevFilter;
    static OnCrashHandler* OnCrash;
    static OnDialogHandler* OnDialog;
    static string_path BugReportFile;
    static bool	ErrorAfterDialog;
    
public:
    xrDebug() = delete;
	static void Initialize(const bool& dedicated);
    static void Destroy();
    static void OnThreadSpawn();
    static OnCrashHandler* GetCrashHandler() { return OnCrash; }
    static void SetCrashHandler(OnCrashHandler* handler) { OnCrash = handler; }
    static OnDialogHandler* GetDialogHandler() { return OnDialog; }
    static void SetDialogHandler(OnDialogHandler* handler) { OnDialog = handler; }
    static const char* ErrorToString(long code);
    static void SetBugReportFile(const char* fileName);
    static void LogStackTrace(const char* header);
    static int BuildStackTrace(char* buffer, int capacity, int lineCapacity);
    static void GatherInfo(const char* expression, const char* description, const char* arg0, const char* arg1,
        const char* file, int line, const char* function, char* assertionInfo);
    static void Fail(const char* e1, const char* file, int line, const char* function, bool& ignoreAlways);
    static void Fail(const char* e1, const std::string& e2, const char* file, int line, const char* function,
        bool& ignoreAlways);
    static void Fail(const char* e1, const char* e2, const char* file, int line, const char* function,
        bool& ignoreAlways);
    static void Fail(const char* e1, const char* e2, const char* e3, const char* file, int line, const char* function,
        bool& ignoreAlways);
    static void Fail(const char* e1, const char* e2, const char* e3, const char* e4, const char* file, int line,
        const char* function, bool &ignoreAlways);
    static void Error(long code, const char* e1, const char* file, int line, const char* function,
        bool& ignoreAlways);
    static void Error(long code, const char* e1, const char* e2, const char* file, int line, const char* function,
        bool& ignoreAlways);
    static void Fatal(const char* file, int line, const char* function, const char* format, ...);
    static void Backend(const char* reason, const char* expression, const char* arg0, const char* arg1,
        const char* file, int line, const char* function, bool& ignoreAlways);
    static void DoExit(const std::string& message);

private:
    static void FormatLastError(char* buffer, const size_t& bufferSize);
    static int BuildStackTrace(EXCEPTION_POINTERS* exPtrs, char* buffer, int capacity, int lineCapacity);
    static void SetupExceptionHandler(const bool& dedicated);
    static LONG WINAPI UnhandledFilter(EXCEPTION_POINTERS* exPtrs);
    static void WINAPI PreErrorHandler(INT_PTR);
    static void SaveMiniDump(EXCEPTION_POINTERS* exPtrs);
};

// warning
// this function can be used for debug purposes only
IC std::string __cdecl make_string(LPCSTR format, ...)
{
    va_list args;
    va_start(args, format);
    string4096 temp;
    vsprintf(temp, format, args);
    return std::string(temp);
}

#include "xrDebug_macros.h"
