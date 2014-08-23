// The to be #defined in a .h file included by a .rc file before maxversion.r


// russom - 08/20/01
#include "buildver.h"

#if defined( GAME_VER )
	#define MAXVER_INTERNALNAME "gmaxFind\0"//should  be overidden on a per-dll level
	#define MAXVER_ORIGINALFILENAME "gmaxFind.exe\0"//should  be overidden on a per-dll level
	#define MAXVER_FILEDESCRIPTION "External gmax File Finder\0"//should  be overidden on a per-dll level
#else
	#define MAXVER_INTERNALNAME "MAXFind\0"//should  be overidden on a per-dll level
	#define MAXVER_ORIGINALFILENAME "MAXFind.exe\0"//should  be overidden on a per-dll level
	#define MAXVER_FILEDESCRIPTION "External MAX File Finder\0"//should  be overidden on a per-dll level
#endif

#define MAXVER_COMMENTS "TECH: christer.janson\0"//should  be overidden on a per-dll level

// #define MAXVER_PRODUCTNAME //generally not overridden at the maxversion.r level
// #define MAXVER_COPYRIGHT //only in exceptions should this be overridden
// #define MAXVER_LEGALTRADEMARKS //only in exceptions should this be overridden
// #define MAXVER_COMPANYNAME //only in exceptions should this be overridden
// #define MAX_VERSION_MAJOR //only in exceptions should this be overridden
// #define MAX_VERSION_MINOR //only in exceptions should this be overridden
// #define MAX_VERSION_POINT //only in exceptions should this be overridden

