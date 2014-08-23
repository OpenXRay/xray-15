/*
=====================================================================
zbuf6.h

Definitions and typedefs for the WCS z-buffer file format.

Chris "Xenon" Hanson  28 Nov 95
Ernie Wright  21 Jul 00
===================================================================== */

/* file header */

typedef struct {
   unsigned long  w, h;
   unsigned short datatype;
   unsigned short compression;
   unsigned short sorting;
   unsigned short units;
   float          min, max, background;
   float          scale, offset;
} ZBufferHeader;

/* data types */

#define ZBVAR_BYTE   0
#define ZBVAR_UBYTE  1
#define ZBVAR_SHORT  2
#define ZBVAR_USHORT 3
#define ZBVAR_LONG   4
#define ZBVAR_ULONG  5
#define ZBVAR_FLOAT  6
#define ZBVAR_DOUBLE 7

/* compression types */

#define ZBCOMP_NONE     0
#define ZBCOMP_BYTERUN1 1     /* UBYTE only */

/* sorting */

#define ZBSORT_NEARTOFAR 0
#define ZBSORT_FARTONEAR 1

/* units */

#define ZBUNIT_NONE  0        /* e.g. grayscale */
#define ZBUNIT_MM    1
#define ZBUNIT_M     2
#define ZBUNIT_KM    3
#define ZBUNIT_IN    4
#define ZBUNIT_FT    5
#define ZBUNIT_YD    6
#define ZBUNIT_MI    7
#define ZBUNIT_LY    8        /* light-years */
#define ZBUNIT_UNDEF 100

/* IFF IDs */

#ifdef _WIN32
#define MAKE_ID(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))
#else
#define MAKE_ID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))
#endif

#define ID_FORM MAKE_ID('F','O','R','M')
#define ID_ILBM MAKE_ID('I','L','B','M')
#define ID_ZBUF MAKE_ID('Z','B','U','F')
#define ID_ZBOD MAKE_ID('Z','B','O','D')

/* byte swap function */

#ifdef _WIN32
void revbytes( void *bp, int elsize, int elcount );
#else
#define revbytes( b, s, c )
#endif

/* plug-in names */

#define ZLOAD_NAME "3DN_ZLoad"
#define ZSAVE_NAME "3DN_ZSave"
#define ZCOMP_NAME "3DN_ZComp"

/* activation functions */

int ZSave_Handler( long, GlobalFunc *, void *, void * );
int ZSave_Interface( long, GlobalFunc *, void *, void * );
int ZComp_Handler( long, GlobalFunc *, void *, void * );
int ZComp_Interface( long, GlobalFunc *, void *, void * );
int ZLoad( long, GlobalFunc *, void *, void * );
