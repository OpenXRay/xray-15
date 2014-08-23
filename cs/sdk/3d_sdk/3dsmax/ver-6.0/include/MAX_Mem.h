/**********************************************************************
 *<
	FILE: MAX_Mem.h

	DESCRIPTION: 3ds max memory wrapper
	Redirects calls to new, delete, malloc(), free(), and heap debug routines
	into implementations within MAX.
	Plug-ins can use this if they are developed with a compiler that is not
	memory compatible with MAX, forcing all memory allocation to occur in a
	compatible fashion, on the same heap.

	CREATED BY: Michaelson Britt

	HISTORY:

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/


#ifndef __MAX_MEM_H
#define __MAX_MEM_H

#if (_MSC_VER >= 1300)  // Visual Studio .NET


#include <malloc.h>
#include <new.h>
//Notes:
//- No handling for new/delete "placement form" operators
//- The Microsoft documentation claims that the functions _heapadd(), _heapchk() etc.
//  are supported only under WinNT


//FIXME? should we wrap the header files for these functions?


//CodeExport void *__cdecl MAX_new_placement(size_t,void*);
//CodeExport void __cdecl MAX_delete_placement(void*,void*);

//__forceinline void* operator new(size_t size, void* _P)
//{
//	return MAX_new_placement(size,_P);
//}

//__forceinline void operator delete(void* memblock, void* _P)
//{
//	MAX_delete_placement(memblock,_P);
//}

//Allocate block of memory from heap 
//_CRTIMP void * __cdecl malloc(size_t);
CoreExport void *	(__cdecl *MAX_malloc)(size_t);
#define malloc(size)	(*MAX_malloc)(size)

//Allocate storage for array, initializing every byte in allocated block to 0 
//_CRTIMP void * __cdecl calloc(size_t, size_t);
CoreExport void *	(__cdecl *MAX_calloc)(size_t, size_t);
#define calloc(num,size)	(*MAX_calloc)(num,size)

//Reallocate block to new size 
//_CRTIMP void * __cdecl realloc(void *, size_t);
CoreExport void *	(__cdecl *MAX_realloc)(void *, size_t);
#define realloc(memblock,size)	(*MAX_realloc)(memblock,size)

//Expand or shrink block of memory without moving it 
//_CRTIMP void *  __cdecl _expand(void *, size_t);
CoreExport void *	(__cdecl *MAX_expand)(void *, size_t);
#define _expand(memblock,size)	(*MAX_expand)(memblock,size)

//Free allocated block 
//_CRTIMP void   __cdecl free(void *);
CoreExport void	(__cdecl *MAX_free)(void *);
#define free(memblock)	(*MAX_free)(memblock)

//Return size of allocated block 
//_CRTIMP size_t  __cdecl _msize(void *);
CoreExport size_t	(__cdecl *MAX_msize)(void *);
#define _msize(memblock)	(*MAX_msize)(memblock)

// Set hook function 
//_CRTIMP _HEAPHOOK __cdecl _setheaphook(_HEAPHOOK);
//CoreExport _HEAPHOOK (__cdecl *MAX_setheaphook)(_HEAPHOOK);
//Warning! Disabled because HEAPHOOK does not seem to be enabled in practice
//#define MAX_setheaphook(_HEAPHOOK)	(*MAX_setheaphook)(_HEAPHOOK)

//Add memory to heap 
//_CRTIMP int     __cdecl _heapadd(void *, size_t);
CoreExport int	(__cdecl *MAX_heapadd)(void *, size_t);
#define _heapadd(memblock,size)	(*MAX_heapadd)(memblock,size)

//Check heap for consistency 
//_CRTIMP int     __cdecl _heapchk(void);
CoreExport int	(__cdecl *MAX_heapchk)(void);
#define _heapchk()	(*MAX_heapchk)()

//Release unused memory in heap 
//_CRTIMP int     __cdecl _heapmin(void);
CoreExport int	(__cdecl *MAX_heapmin)(void);
#define _heapmin()	(*MAX_heapmin)()

//Fill free heap entries with specified value 
//_CRTIMP int     __cdecl _heapset(unsigned int);
CoreExport int	(__cdecl *MAX_heapset)(unsigned int);
#define _heapset(fill)	(*MAX_heapset)(fill)

//Return information about each entry in heap 
//_CRTIMP int     __cdecl _heapwalk(_HEAPINFO *);
CoreExport int	(__cdecl *MAX_heapwalk)(_HEAPINFO *);
#define _heapwalk(entryinfo)	(*MAX_heapwalk)(entryinfo)

//Return address of current new handler routine as set by _set_new_handler 
//_CRTIMP _PNH __cdecl _query_new_handler( void );
CoreExport _PNH	(__cdecl *MAX_query_new_handler)( void );
#define _query_new_handler()	(*MAX_query_new_handler)()

//Enable error-handling mechanism when new operator fails (to allocate memory) and enable compilation of Standard Template Libraries (STL) 
//_CRTIMP _PNH __cdecl _set_new_handler( _PNH );
CoreExport _PNH	(__cdecl *MAX_set_new_handler)( _PNH );
#define _set_new_handler(pNewHandler)	(*MAX_set_new_handler)(pNewHandler)

//Return integer indicating new handler mode set by _set_new_mode for malloc 
//_CRTIMP int __cdecl _query_new_mode( void );
CoreExport int	(__cdecl *MAX_query_new_mode)( void );
#define _query_new_mode()	(*MAX_query_new_mode)()

//Set new handler mode for malloc 
//_CRTIMP int __cdecl _set_new_mode( int );
CoreExport int	(__cdecl *MAX_set_new_mode)( int );
#define _set_new_mode(newhandlermode)	(*MAX_set_new_mode)(newhandlermode)

//Get/Set the upper limit for the size of a memory allocation that will be supported by the small-block heap 
//_CRTIMP size_t  __cdecl _get_sbh_threshold(void);
//_CRTIMP int     __cdecl _set_sbh_threshold(size_t);
CoreExport size_t	(__cdecl *MAX_get_sbh_threshold)(void);
#define _get_sbh_threshold()	(*MAX_get_sbh_threshold)()

CoreExport int	(__cdecl *MAX_set_sbh_threshold)(size_t);
#define _set_sbh_threshold(size)	(*MAX_set_sbh_threshold)(size)


//Allocate memory from stack
//NOTE: no implementation needed.  Only heap allocation causes a problem
//void *          __cdecl _alloca(size_t);
//CoreExport void *          __cdecl MAX_alloca(size_t);
//#define _alloca(size)	MAX_alloca(size)



/***
*
* MAX_Mem wrapper for Microsoft crtdbg.h geader file
*
****/


/***
*crtdbg.h - Supports debugging features of the C runtime library.
*
*       Copyright (c) 1994-2001, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Support CRT debugging features.
*
*       [Public]
*
****/

//#if (_MSC_VER >= 1300)  // Visual Studio .NET


//#if     _MSC_VER > 1000
//#pragma once
//#endif

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif



CoreExport void *__cdecl MAX_new(size_t size);
CoreExport void __cdecl MAX_delete(void* mem);

__forceinline void* operator new(size_t size)
{
	return MAX_new(size);
}

__forceinline void operator delete(void* memblock)
{
	MAX_delete(memblock);
}



 /****************************************************************************
 *
 * CRTDBG.H Support
 *
 ***************************************************************************/


#ifdef  __cplusplus
//extern "C" {
#endif  /* __cplusplus */

 /****************************************************************************
 *
 * Constants and types
 *
 ***************************************************************************/

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

/* Define NULL pointer value */

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

 /****************************************************************************
 *
 * Debug Reporting
 *
 ***************************************************************************/

typedef void *_HFILE; /* file handle pointer */

#define _CRT_WARN           0
#define _CRT_ERROR          1
#define _CRT_ASSERT         2
#define _CRT_ERRCNT         3

#define _CRTDBG_MODE_FILE      0x1
#define _CRTDBG_MODE_DEBUG     0x2
#define _CRTDBG_MODE_WNDW      0x4
#define _CRTDBG_REPORT_MODE    -1

#define _CRTDBG_INVALID_HFILE ((_HFILE)-1)
#define _CRTDBG_HFILE_ERROR   ((_HFILE)-2)
#define _CRTDBG_FILE_STDOUT   ((_HFILE)-4)
#define _CRTDBG_FILE_STDERR   ((_HFILE)-5)
#define _CRTDBG_REPORT_FILE   ((_HFILE)-6)

typedef int (__cdecl * _CRT_REPORT_HOOK)(int, char *, int *);

#define _CRT_RPTHOOK_INSTALL  0
#define _CRT_RPTHOOK_REMOVE   1

 /****************************************************************************
 *
 * Heap
 *
 ***************************************************************************/



 /****************************************************************************
 *
 * Heap
 *
 ***************************************************************************/

 /****************************************************************************
 *
 * Client-defined allocation hook
 *
 ***************************************************************************/

#define _HOOK_ALLOC     1
#define _HOOK_REALLOC   2
#define _HOOK_FREE      3

typedef int (__cdecl * _CRT_ALLOC_HOOK)(int, void *, size_t, int, long, const unsigned char *, int);

 /****************************************************************************
 *
 * Memory management
 *
 ***************************************************************************/

/*
 * Bit values for _crtDbgFlag flag:
 *
 * These bitflags control debug heap behavior.
 */

#define _CRTDBG_ALLOC_MEM_DF        0x01  /* Turn on debug allocation */
#define _CRTDBG_DELAY_FREE_MEM_DF   0x02  /* Don't actually free memory */
#define _CRTDBG_CHECK_ALWAYS_DF     0x04  /* Check heap every alloc/dealloc */
#define _CRTDBG_RESERVED_DF         0x08  /* Reserved - do not use */
#define _CRTDBG_CHECK_CRT_DF        0x10  /* Leak check/diff CRT blocks */
#define _CRTDBG_LEAK_CHECK_DF       0x20  /* Leak check at program exit */

/*
 * Some bit values for _crtDbgFlag which correspond to frequencies for checking
 * the the heap.
 */
#define _CRTDBG_CHECK_EVERY_16_DF   0x00100000  /* check heap every 16 heap ops */
#define _CRTDBG_CHECK_EVERY_128_DF  0x00800000  /* check heap every 128 heap ops */
#define _CRTDBG_CHECK_EVERY_1024_DF 0x04000000  /* check heap every 1024 heap ops */
#define _CRTDBG_CHECK_DEFAULT_DF    _CRTDBG_CHECK_EVERY_1024_DF

#define _CRTDBG_REPORT_FLAG         -1    /* Query bitflag status */

#define _BLOCK_TYPE(block)          (block & 0xFFFF)
#define _BLOCK_SUBTYPE(block)       (block >> 16 & 0xFFFF)


 /****************************************************************************
 *
 * Memory state
 *
 ***************************************************************************/

/* Memory block identification */
#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

typedef void (__cdecl * _CRT_DUMP_CLIENT)(void *, size_t);

struct _CrtMemBlockHeader;
typedef struct _CrtMemState
{
        struct _CrtMemBlockHeader * pBlockHeader;
        size_t lCounts[_MAX_BLOCKS];
        size_t lSizes[_MAX_BLOCKS];
        size_t lHighWaterCount;
        size_t lTotalCount;
} _CrtMemState;


 /****************************************************************************
 *
 * Declarations, prototype and function-like macros
 *
 ***************************************************************************/


#ifndef _DEBUG

 /****************************************************************************
 *
 * Debug OFF
 * Debug OFF
 * Debug OFF
 *
 ***************************************************************************/

#define _ASSERT(expr) ((void)0)

#define _ASSERTE(expr) ((void)0)


#define _RPT0(rptno, msg)

#define _RPT1(rptno, msg, arg1)

#define _RPT2(rptno, msg, arg1, arg2)

#define _RPT3(rptno, msg, arg1, arg2, arg3)

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4)


#define _RPTF0(rptno, msg)

#define _RPTF1(rptno, msg, arg1)

#define _RPTF2(rptno, msg, arg1, arg2)

#define _RPTF3(rptno, msg, arg1, arg2, arg3)

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4)

#define _malloc_dbg(s, t, f, l)         MAX_malloc(s)
#define _calloc_dbg(c, s, t, f, l)      MAX_calloc(c, s)
#define _realloc_dbg(p, s, t, f, l)     MAX_realloc(p, s)
#define _expand_dbg(p, s, t, f, l)      MAX_expand(p, s)
#define _free_dbg(p, t)                 MAX_free(p)
#define _msize_dbg(p, t)                MAX_msize(p)

/* WARNING! There are no VC6 equivalents for these
#define _aligned_malloc_dbg(s, a, f, l)     _aligned_malloc(s, a)
#define _aligned_realloc_dbg(p, s, a, f, l) _aligned_realloc(p, s, a)
#define _aligned_free_dbg(p)                _aligned_free(p)
#define _aligned_offset_malloc_dbg(s, a, o, f, l)       _aligned_offset_malloc(s, a, o)
#define _aligned_offset_realloc_dbg(p, s, a, o, f, l)   _aligned_offset_realloc(p, s, a, o)
*/

#define _CrtSetReportHook(f)                ((_CRT_REPORT_HOOK)0)
#define _CrtSetReportHook2(t, f)            ((int)0)
#define _CrtSetReportMode(t, f)             ((int)0)
#define _CrtSetReportFile(t, f)             ((_HFILE)0)

#define _CrtDbgBreak()                      ((void)0)

#define _CrtSetBreakAlloc(a)                ((long)0)

#define _CrtSetAllocHook(f)                 ((_CRT_ALLOC_HOOK)0)

#define _CrtCheckMemory()                   ((int)1)
#define _CrtSetDbgFlag(f)                   ((int)0)
#define _CrtDoForAllClientObjects(f, c)     ((void)0)
#define _CrtIsValidPointer(p, n, r)         ((int)1)
#define _CrtIsValidHeapPointer(p)           ((int)1)
#define _CrtIsMemoryBlock(p, t, r, f, l)    ((int)1)
#define _CrtReportBlockType(p)              ((int)-1)

#define _CrtSetDumpClient(f)                ((_CRT_DUMP_CLIENT)0)

#define _CrtMemCheckpoint(s)                ((void)0)
#define _CrtMemDifference(s1, s2, s3)       ((int)0)
#define _CrtMemDumpAllObjectsSince(s)       ((void)0)
#define _CrtMemDumpStatistics(s)            ((void)0)
#define _CrtDumpMemoryLeaks()               ((int)0)


#else   /* _DEBUG */


 /****************************************************************************
 *
 * Debug ON
 * Debug ON
 * Debug ON
 *
 ***************************************************************************/


/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* _CRTIMP */

 /****************************************************************************
 *
 * Debug Reporting
 *
 ***************************************************************************/

//_CRTIMP extern long _crtAssertBusy;
CoreExport extern long& MAX_crtAssertBusy;
#define _crtAssertBusy (MAX_crtAssertBusy)

//Install a client-defined reporting function by hooking it into the C run-time debug reporting process
//_CRTIMP _CRT_REPORT_HOOK __cdecl _CrtSetReportHook(
//        _CRT_REPORT_HOOK
//        );
CoreExport _CRT_REPORT_HOOK	(__cdecl *MAX_CrtSetReportHook)(_CRT_REPORT_HOOK);
#define _CrtSetReportHook(reportHook)	(*MAX_CrtSetReportHook)(reportHook)



/* WARNING! There is no VC6 equivalent
_CRTIMP int __cdecl _CrtSetReportHook2(
        int,
        _CRT_REPORT_HOOK
        );
*/

//Install a client-defined reporting function by hooking it into the C run-time debug reporting process
//_CRTIMP int __cdecl _CrtSetReportMode(
//        int,
//        int
//        );
CoreExport int	(__cdecl *MAX_CrtSetReportMode)(int,int);
#define _CrtSetReportMode(reportType,reportMode)	(*MAX_CrtSetReportMode)(reportType,reportMode)


//Identify the file or stream to be used as a destination for a specific report type by _CrtDbgReport
//_CRTIMP _HFILE __cdecl _CrtSetReportFile(
//        int,
//        _HFILE
//        );
CoreExport _HFILE	(__cdecl *MAX_CrtSetReportFile)(int,_HFILE);
#define _CrtSetReportFile(reportType,reportFile)	(*MAX_CrtSetReportFile)(reportType,reportFile)

//Generate a debug report with a user message and send the report to three possible destinations
//_CRTIMP int __cdecl _CrtDbgReport(
//        int,
//        const char *,
//        int,
//        const char *,
//        const char *,
//        ...);
CoreExport int	(__cdecl *MAX_CrtDbgReport)(int,const char *,int,const char *,const char *,...);
#define _CrtDbgReport	(*MAX_CrtDbgReport) //NOTE: using a standard define, rather than a macro


/* Asserts */

#if     _MSC_VER >= 1300 || !defined(_M_IX86) || defined(_CRT_PORTABLE)
#define _ASSERT_BASE(expr, msg) \
        (void) ((expr) || \
                (1 != MAX_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, msg)) || \
                (MAX_CrtDbgBreak(), 0))
#else
#define _ASSERT_BASE(expr, msg) \
        do { if (!(expr) && \
                (1 == MAX_CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, msg))) \
             MAX_CrtDbgBreak(); } while (0)
#endif

#define _ASSERT(expr)   _ASSERT_BASE((expr), NULL)

#define _ASSERTE(expr)  _ASSERT_BASE((expr), #expr)

/* Reports with no file/line info */

#if     _MSC_VER >= 1300 || !defined(_M_IX86) || defined(_CRT_PORTABLE)
#define _RPT_BASE(args) \
        (void) ((1 != MAX_CrtDbgReport args) || \
                (MAX_CrtDbgBreak(), 0))
#else
#define _RPT_BASE(args) \
        do { if ((1 == MAX_CrtDbgReport args)) \
                MAX_CrtDbgBreak(); } while (0)
#endif

#define _RPT0(rptno, msg) \
        _RPT_BASE((rptno, NULL, 0, NULL, "%s", msg))

#define _RPT1(rptno, msg, arg1) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1))

#define _RPT2(rptno, msg, arg1, arg2) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1, arg2))

#define _RPT3(rptno, msg, arg1, arg2, arg3) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1, arg2, arg3))

#define _RPT4(rptno, msg, arg1, arg2, arg3, arg4) \
        _RPT_BASE((rptno, NULL, 0, NULL, msg, arg1, arg2, arg3, arg4))


/* Reports with file/line info */

#define _RPTF0(rptno, msg) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, "%s", msg))

#define _RPTF1(rptno, msg, arg1) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1))

#define _RPTF2(rptno, msg, arg1, arg2) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2))

#define _RPTF3(rptno, msg, arg1, arg2, arg3) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3))

#define _RPTF4(rptno, msg, arg1, arg2, arg3, arg4) \
        _RPT_BASE((rptno, __FILE__, __LINE__, NULL, msg, arg1, arg2, arg3, arg4))


//_CRTIMP void __cdecl _CrtDbgBreak(void);
CoreExport void	__cdecl MAX_CrtDbgBreak(void);

#define _CrtDbgBreak()	MAX_CrtDbgBreak()

/*
#if     _MSC_VER >= 1300 && !defined(_CRT_PORTABLE)
#define _CrtDbgBreak() __debugbreak()
#elif   defined(_M_IX86) && !defined(_CRT_PORTABLE)
#define _CrtDbgBreak() __asm { int 3 }
#elif   defined(_M_ALPHA) && !defined(_CRT_PORTABLE)
void _BPT();
#pragma intrinsic(_BPT)
#define _CrtDbgBreak() _BPT()
#elif   defined(_M_IA64) && !defined(_CRT_PORTABLE)
void __break(int);
#pragma intrinsic (__break)
#define _CrtDbgBreak() __break(0x80016)
#else
_CRTIMP void __cdecl _CrtDbgBreak(
        void
        );
#endif
*/

 /****************************************************************************
 *
 * Heap routines
 *
 ***************************************************************************/

#ifdef  _CRTDBG_MAP_ALLOC

#define   malloc(s)         MAX_malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   calloc(c, s)      MAX_calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     MAX_realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   _expand(p, s)     MAX_expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   free(p)           MAX_free_dbg(p, _NORMAL_BLOCK)
#define   _msize(p)         MAX_msize_dbg(p, _NORMAL_BLOCK)
/* WARNING! No VC6 equivalent
#define   _aligned_malloc(s, a)                 _aligned_malloc_dbg(s, a, __FILE__, __LINE__)
#define   _aligned_realloc(p, s, a)             _aligned_realloc_dbg(p, s, a, __FILE__, __LINE__)
#define   _aligned_offset_malloc(s, a, o)       _aligned_offset_malloc_dbg(s, a, o, __FILE__, __LINE__)
#define   _aligned_offset_realloc(p, s, a, o)   _aligned_offset_realloc_dbg(p, s, a, o, __FILE__, __LINE__)
#define   _aligned_free(p)  _aligned_free_dbg(p)
*/
#endif  /* _CRTDBG_MAP_ALLOC */

//_CRTIMP extern long _crtBreakAlloc;      /* Break on this allocation */
CoreExport extern long& MAX_crtBreakAlloc;
#define _crtBreakAlloc (MAX_crtBreakAlloc)

//Set a breakpoint on a specified object allocation order number
//_CRTIMP long __cdecl _CrtSetBreakAlloc(
//        long
//        );
CoreExport long	(__cdecl *MAX_CrtSetBreakAlloc)(long);
#define _CrtSetBreakAlloc(lBreakAlloc)	(*MAX_CrtSetBreakAlloc)(lBreakAlloc)


/*
 * Prototypes for malloc, free, realloc, etc are in malloc.h
 */

//Debug version of malloc; only available in the debug versions of the run-time libraries 
//_CRTIMP void * __cdecl _malloc_dbg(
//        size_t,
//        int,
//        const char *,
//        int
//        );
CoreExport void *	(__cdecl *MAX_malloc_dbg)(size_t,int,const char *,int);
#define _malloc_dbg(size,blockType,filename,lineNumber)	(*MAX_malloc_dbg)(size,blockType,filename,lineNumber)

//Debug version of calloc; only available in the debug versions of the run-time libraries 
//_CRTIMP void * __cdecl _calloc_dbg(
//        size_t,
//        size_t,
//        int,
//        const char *,
//        int
//        );
CoreExport void *	(__cdecl *MAX_calloc_dbg)(size_t, size_t, int, const char *, int);
#define _calloc_dbg(num,size,blockType,filename,lineNumber)	(*MAX_calloc_dbg)(num,size,blockType,filename,lineNumber)

//Debug version of realloc; only available in the debug versions of the run-time libraries 
//_CRTIMP void * __cdecl _realloc_dbg(
//        void *,
//        size_t,
//        int,
//        const char *,
//        int
//        );
CoreExport void *	(__cdecl *MAX_realloc_dbg)(void *, size_t, int, const char *, int);
#define _realloc_dbg(userData,newSize,blockType,filename,lineNumber)	(*MAX_realloc_dbg)(userData,newSize,blockType,filename,lineNumber)

//Debug version of _expand; only available in the debug versions of the run-time libraries 
//_CRTIMP void * __cdecl _expand_dbg(
//        void *,
//        size_t,
//        int,
//        const char *,
//        int
//        );
CoreExport void *	(__cdecl *MAX_expand_dbg)(void *, size_t, int, const char *, int);
#define _expand_dbg(userData,newSize,blockType,filename,lineNumber)	(*MAX_expand_dbg)(userData,newSize,blockType,filename,lineNumber)

//Debug version of free; only available in the debug versions of the run-time libraries 
//_CRTIMP void __cdecl _free_dbg(
//        void *,
//        int
//        );
CoreExport void	(__cdecl *MAX_free_dbg)(void *, int);
#define _free_dbg(userData,blockType)	(*MAX_free_dbg)(userData,blockType)

//Debug version of _msize; only available in the debug versions of the run-time libraries 
//_CRTIMP size_t __cdecl _msize_dbg (
//        void *,
//        int
//        );
CoreExport size_t	(__cdecl *MAX_msize_dbg)(void *, int);
#define _msize_dbg(userData,blockType)	(*MAX_msize_dbg)(userData,blockType)


/* WARNING! No VC6 equivalent

_CRTIMP void * __cdecl _aligned_malloc_dbg(
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void * __cdecl _aligned_realloc_dbg(
        void *,
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void * __cdecl _aligned_offset_malloc_dbg(
        size_t,
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void * __cdecl _aligned_offset_realloc_dbg(
        void *,
        size_t,
        size_t,
        size_t,
        const char *,
        int
        );

_CRTIMP void __cdecl _aligned_free_dbg(
        void *
        );
*/

 /****************************************************************************
 *
 * Client-defined allocation hook
 *
 ***************************************************************************/

//Install a client-defined allocation function by hooking it into the C run-time debug memory allocation process
//_CRTIMP _CRT_ALLOC_HOOK __cdecl _CrtSetAllocHook(
//        _CRT_ALLOC_HOOK
//        );
CoreExport _CRT_ALLOC_HOOK	(__cdecl *MAX_CrtSetAllocHook)(_CRT_ALLOC_HOOK);
#define _CrtSetAllocHook(allocHook)	(*MAX_CrtSetAllocHook)(allocHook)


 /****************************************************************************
 *
 * Memory management
 *
 ***************************************************************************/

/*
 * Bitfield flag that controls CRT heap behavior
 * Default setting is _CRTDBG_ALLOC_MEM_DF
 */

//_CRTIMP extern int _crtDbgFlag;
CoreExport extern int& MAX_crtDbgFlag;
#define _crtDbgFlag (MAX_crtDbgFlag)

//Confirm the integrity of the memory blocks allocated on the debug heap
//_CRTIMP int __cdecl _CrtCheckMemory(
//        void
//        );
CoreExport int	(__cdecl *MAX_CrtCheckMemory)(void);
#define _CrtCheckMemory()	(*MAX_CrtCheckMemory)()

//Retrieve or modify the state of the _crtDbgFlag flag to control the allocation behavior of the debug heap manager
//_CRTIMP int __cdecl _CrtSetDbgFlag(
//        int
//        );
CoreExport int	(__cdecl *MAX_CrtSetDbgFlag)(int);
#define _CrtSetDbgFlag(newFlag)	(*MAX_CrtSetDbgFlag)(newFlag)

//Call an application-supplied function for all _CLIENT_BLOCK types on the heap
//_CRTIMP void __cdecl _CrtDoForAllClientObjects(
//        void (*pfn)(void *, void *),
//        void *
//        );
CoreExport void	(__cdecl *MAX_CrtDoForAllClientObjects)(void (*pfn)(void *, void *),void *);
#define _CrtDoForAllClientObjects(pfn,context)	(*MAX_CrtDoForAllClientObjects)(pfn,context)

//Verify that a specified memory range is valid for reading and writing
//_CRTIMP int __cdecl _CrtIsValidPointer(
//        const void *,
//        unsigned int,
//        int
//        );
CoreExport int	(__cdecl *MAX_CrtIsValidPointer)(const void *,unsigned int,int);
#define _CrtIsValidPointer(address,size,access) (*MAX_CrtIsValidPointer)(address,size,access)

//Verify that a specified pointer is in the local heap
//_CRTIMP int __cdecl _CrtIsValidHeapPointer(
//        const void *
//        );
CoreExport int	(__cdecl *MAX_CrtIsValidHeapPointer)(const void *);
#define _CrtIsValidHeapPointer(userData)	(*MAX_CrtIsValidHeapPointer)(userData)

//Verify that a specified memory block is located within the local heap and that it has a valid debug heap block type identifier
//_CRTIMP int __cdecl _CrtIsMemoryBlock(
//        const void *,
//        unsigned int,
//        long *,
//        char **,
//        int *
//        );
CoreExport int	(__cdecl *MAX_CrtIsMemoryBlock)(const void *,unsigned int,long *,char **,int *);
#define _CrtIsMemoryBlock(userData,size,requestNumber,filename,linenumber)	(*MAX_CrtIsMemoryBlock)(userData,size,requestNumber,filename,linenumber)


/* WARNING! No VC6 equivalent
_CRTIMP int __cdecl _CrtReportBlockType(
        const void *
        );
*/


 /****************************************************************************
 *
 * Memory state
 *
 ***************************************************************************/

//Install an application-defined function that is called every time a debug dump function is called to dump _CLIENT_BLOCK type memory blocks
//_CRTIMP _CRT_DUMP_CLIENT __cdecl _CrtSetDumpClient(
//        _CRT_DUMP_CLIENT
//        );
CoreExport _CRT_DUMP_CLIENT	(__cdecl *MAX_CrtSetDumpClient)(_CRT_DUMP_CLIENT);
#define _CrtSetDumpClient(dumpClient)	(*MAX_CrtSetDumpClient)(dumpClient)

//Obtain the current state of the debug heap and store it in an application-supplied _CrtMemState structure
//_CRTIMP void __cdecl _CrtMemCheckpoint(
//        _CrtMemState *
//        );
CoreExport void	(__cdecl *MAX_CrtMemCheckpoint)(_CrtMemState *);
#define _CrtMemCheckpoint(state)	(*MAX_CrtMemCheckpoint)(state)

//Compare two memory states for significant differences and return the results
//_CRTIMP int __cdecl _CrtMemDifference(
//        _CrtMemState *,
//        const _CrtMemState *,
//        const _CrtMemState *
//        );
CoreExport int	(__cdecl *MAX_CrtMemDifference)(_CrtMemState *,const _CrtMemState *,const _CrtMemState *);
#define _CrtMemDifference(stateDiff,oldState,newState)	(*MAX_CrtMemDifference)(stateDiff,oldState,newState)

//Dump information about objects on the heap since a specified checkpoint was taken or from the start of program execution
//_CRTIMP void __cdecl _CrtMemDumpAllObjectsSince(
//        const _CrtMemState *
//        );
CoreExport void	(__cdecl *MAX_CrtMemDumpAllObjectsSince)(const _CrtMemState *);
#define _CrtMemDumpAllObjectsSince(state)	(*MAX_CrtMemDumpAllObjectsSince)(state)

//Dump the debug header information for a specified memory state in a user-readable form
//_CRTIMP void __cdecl _CrtMemDumpStatistics(
//        const _CrtMemState *
//        );
CoreExport void	(__cdecl *MAX_CrtMemDumpStatistics)(const _CrtMemState *);
#define _CrtMemDumpStatistics(state)	(*MAX_CrtMemDumpStatistics)(state)

//Dump all of the memory blocks on the debug heap when a significant memory leak has occurred
//_CRTIMP int __cdecl _CrtDumpMemoryLeaks(
//        void
//        );
CoreExport int	(__cdecl *MAX_CrtDumpMemoryLeaks)(void);
#define _CrtDumpMemoryLeaks()	(*MAX_CrtDumpMemoryLeaks)()


#endif  /* _DEBUG */

#ifdef  __cplusplus
//}

#ifndef _MFC_OVERRIDES_NEW

//extern "C++" {

#pragma warning(disable: 4507)  /* Ignore faulty warning */

#ifndef _DEBUG

 /****************************************************************************
 *
 * Debug OFF
 * Debug OFF
 * Debug OFF
 *
 ***************************************************************************/

CoreExport void *__cdecl MAX_new_array(size_t size);
CoreExport void __cdecl MAX_delete_array(void* mem);

//void * __cdecl operator new[](size_t);
__forceinline void* _cdecl operator new[](size_t size)
{
	return MAX_new_array(size);
}


//FIXME: implement this
//inline void * __cdecl operator new(size_t s, int, const char *, int)
//        { return ::operator new(s); }

//FIXME: implement this
//inline void* __cdecl operator new[](size_t s, int, const char *, int)
//        { return ::operator new[](s); }

#if     _MSC_VER >= 1200


//void __cdecl operator delete[](void *);
__forceinline void _cdecl operator delete[](void* memblock)
{
	MAX_delete_array(memblock);
}

//FIXME: implement this
//inline void __cdecl operator delete(void * _P, int, const char *, int)
//        { ::operator delete(_P); }

//FIXME: implement this
//inline void __cdecl operator delete[](void * _P, int, const char *, int)
//        { ::operator delete[](_P); }
#endif
#else /* _DEBUG */

 /****************************************************************************
 *
 * Debug ON
 * Debug ON
 * Debug ON
 *
 ***************************************************************************/
 
CoreExport void *__cdecl MAX_new_array(size_t size);
CoreExport void __cdecl MAX_delete_array(void* mem);

//void * __cdecl operator new[](size_t);
__forceinline void* _cdecl operator new[](size_t size)
{
	return MAX_new_array(size);
}

//FIXME: implement this
//void * __cdecl operator new(
//        size_t,
//        int,
//        const char *,
//        int
//        );

//FIXME: implement this
//void * __cdecl operator new[](
//        size_t,
//        int,
//        const char *,
//        int
//        );

#if     _MSC_VER >= 1200
//void __cdecl operator delete[](void *);
__forceinline void _cdecl operator delete[](void* memblock)
{
	MAX_delete_array(memblock);
}

//FIXME: implement this
//inline void __cdecl operator delete(void * _P, int, const char *, int)
//        { ::operator delete(_P); }

//FIXME: implement this
//inline void __cdecl operator delete[](void * _P, int, const char *, int)
//        { ::operator delete[](_P); }
#endif

#ifdef _CRTDBG_MAP_ALLOC

//FIXME: implement this
//inline void * __cdecl operator new(size_t s)
//        { return ::operator new(s, _NORMAL_BLOCK, __FILE__, __LINE__); }

//FIXME: implement this
//inline void* __cdecl operator new[](size_t s)
//        { return ::operator new[](s, _NORMAL_BLOCK, __FILE__, __LINE__); }

#endif  /* _CRTDBG_MAP_ALLOC */

#endif  /* _DEBUG */

//}

#endif  /* _MFC_OVERRIDES_NEW */

#endif  /* __cplusplus */



#endif	//_MSC_VER > 1300

#endif  /* MAX_MEM */

