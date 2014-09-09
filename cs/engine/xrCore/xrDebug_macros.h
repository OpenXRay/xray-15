#pragma once

#ifndef __BORLANDC__
#ifndef ANONYMOUS_BUILD
#define DEBUG_INFO __FILE__, __LINE__, __FUNCTION__
#else
#define DEBUG_INFO "", __LINE__, ""
#endif
#else
#define DEBUG_INFO __FILE__, __LINE__, __FILE__
#endif

#ifdef ANONYMOUS_BUILD
#define _TRE(arg)	""
#else
#define _TRE(arg)	arg
#endif

#define CHECK_OR_EXIT(expr, message) \
    do \
    { \
        if (!(expr)) \
            xrDebug::DoExit(message); \
    } while (false)
#define R_ASSERT(expr) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(_TRE(#expr), DEBUG_INFO, ignoreAlways); \
    } while (false)
#define R_ASSERT2(expr, e2) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(_TRE(#expr), _TRE(e2), DEBUG_INFO, ignoreAlways); \
    } while (false)
#define R_ASSERT3(expr, e2, e3) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(_TRE(#expr), _TRE(e2), _TRE(e3), DEBUG_INFO, ignoreAlways); \
    } while (false)
#define R_ASSERT4(expr, e2, e3, e4) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(_TRE(#expr), _TRE(e2), _TRE(e3), _TRE(e4), DEBUG_INFO, ignoreAlways); \
    } while (false)
#define R_CHK(expr) \
    do \
    { \
        static bool ignoreAlways = false; \
        HRESULT hr = (expr); \
        if (!ignoreAlways && FAILED(hr)) \
            xrDebug::Error(hr, _TRE(#expr), DEBUG_INFO, ignoreAlways); \
    } while (false)
#define R_CHK2(expr, e2) \
    do \
    { \
        static bool ignoreAlways = false; \
        HRESULT hr = (expr); \
        if (!ignoreAlways && FAILED(hr)) \
            xrDebug::Error(hr, _TRE(#expr), _TRE(e2), DEBUG_INFO, ignoreAlways); \
    } while (false)
#define FATAL(description) xrDebug::Fatal(DEBUG_INFO, description)

#ifdef VERIFY
#undef VERIFY
#endif

#ifdef DEBUG
#define NODEFAULT FATAL("nodefault reached")
#define VERIFY(expr) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(#expr, DEBUG_INFO, ignoreAlways); \
    } while (false)
#define VERIFY2(expr, e2) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(#expr, e2, DEBUG_INFO, ignoreAlways); \
    } while (false)
#define VERIFY3(expr, e2, e3) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(#expr, e2, e3, DEBUG_INFO, ignoreAlways); \
    } while (false)
#define VERIFY4(expr, e2, e3, e4) \
    do \
    { \
        static bool ignoreAlways = false; \
        if (!ignoreAlways && !(expr)) \
            xrDebug::Fail(#expr, e2, e3, e4, DEBUG_INFO, ignoreAlways); \
    } while (false)
#define CHK_DX(expr) \
    do \
    { \
        static bool ignoreAlways = false; \
        HRESULT hr = (expr); \
        if (!ignoreAlways && FAILED(hr)) \
            xrDebug::Error(hr, #expr, DEBUG_INFO, ignoreAlways); \
    } while (false)
#else // DEBUG
#ifdef __BORLANDC__
#define NODEFAULT
#else
#define NODEFAULT __assume(0)
#endif
#define VERIFY(expr) do {} while (0)
#define VERIFY2(expr, e2) do {} while (0)
#define VERIFY3(expr, e2, e3) do {} while (0)
#define VERIFY4(expr, e2, e3, e4) do {} while (0)
#define CHK_DX(a) a
#endif // DEBUG
//---------------------------------------------------------------------------------------------
// FIXMEs / TODOs / NOTE macros
//---------------------------------------------------------------------------------------------
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define NOTE( x )  message( x )
#define FILE_LINE  message( __FILE__LINE__ )

#define TODO( x )  message( __FILE__LINE__"\n"           \
	" ------------------------------------------------\n" \
	"|  TODO :   " #x "\n" \
	" -------------------------------------------------\n" )
#define FIXME( x )  message(  __FILE__LINE__"\n"           \
	" ------------------------------------------------\n" \
	"|  FIXME :  " #x "\n" \
	" -------------------------------------------------\n" )
#define todo(x) message( __FILE__LINE__" TODO :   " #x "\n" ) 
#define fixme(x) message( __FILE__LINE__" FIXME:   " #x "\n" ) 

//--------- static assertion
// XXX nitrocaster: use static_assert
template<bool>	struct CompileTimeError;
template<>		struct CompileTimeError<true>	{};
#define STATIC_CHECK(expr, msg) \
{ \
	CompileTimeError<((expr) != 0)> ERROR_##msg; \
	(void)ERROR_##msg; \
}
