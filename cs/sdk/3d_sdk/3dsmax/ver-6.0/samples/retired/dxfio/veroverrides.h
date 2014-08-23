// The to be #defined in a .h file included by a .rc file before maxversion.r


#ifdef DXF_IMPORT_RESOURCES

#define MAXVER_INTERNALNAME "DXFImp\0"//should  be overidden on a per-dll level
#define MAXVER_ORIGINALFILENAME "dxfimp.dli\0"//should  be overidden on a per-dll level
#define MAXVER_FILEDESCRIPTION "AutoCAD DXF file importer (plugin)\0"//should  be overidden on a per-dll level
#define MAXVER_COMMENTS "TECH: stewart.sabadell\0"//should  be overidden on a per-dll level

#endif

#ifdef DXF_EXPORT_RESOURCES

#define MAXVER_INTERNALNAME "DXFExp\0"//should  be overidden on a per-dll level
#define MAXVER_ORIGINALFILENAME "dxfexp.dle\0"//should  be overidden on a per-dll level
#define MAXVER_FILEDESCRIPTION "AutoCAD DXF file exporter (plugin)\0"//should  be overidden on a per-dll level
#define MAXVER_COMMENTS "TECH: stewart.sabadell\0"//should  be overidden on a per-dll level

#endif

// #define MAXVER_PRODUCTNAME //generally not overridden at the maxversion.r level
// #define MAXVER_COPYRIGHT //only in exceptions should this be overridden
// #define MAXVER_LEGALTRADEMARKS //only in exceptions should this be overridden
// #define MAXVER_COMPANYNAME //only in exceptions should this be overridden
// #define MAX_VERSION_MAJOR //only in exceptions should this be overridden
// #define MAX_VERSION_MINOR //only in exceptions should this be overridden
// #define MAX_VERSION_POINT //only in exceptions should this be overridden

