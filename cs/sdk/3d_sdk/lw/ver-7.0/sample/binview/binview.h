/*
======================================================================
binview.h

Shared definitions and declarations for the LightWave SDK BinView
sample plug-in.

Ernie Wright  3 Nov 99
====================================================================== */

#define ROWBYTES 16     /* maximum number of bytes displayed per line */
#define NROWS 32        /* maximum number of lines */

#define TXWIDTH 6       /* width of monospace font in pixels */
#define TXHEIGHT 12     /* height of the font, see glyph.h */
#define DX 3            /* border width (offset to first text cell) */
#define DY 3            /* border height */

#define PLUGIN_NAME  "BinView"
#define PANEL_TITLE  PLUGIN_NAME

#ifdef LWSDK_SERVER_H
int  open_window( GlobalFunc *global );
#endif

#ifdef LWPANEL_H
LWRasterID create_char( int fg, int bg, LWRasterFuncs *rasf );
void draw_char( int c, int x, int y, LWRasterID ras, LWRasterFuncs *rasf,
        LWPanelID panel );
void free_char( LWRasterID ras, LWRasterFuncs *rasf );
void draw_icon( int x, int y, LWRasterFuncs *rasf, LWPanelID panel );
void free_icon( LWRasterFuncs *rasf );
#endif

void show_byt( long pos, int bufsize, int w );
void show_prt( char *buf, int bufsize, int w );
void show_num( char *buf, int bufsize, int type, int unsign, int rev,
        int rowbytes );
void print_buf( char *buf, int bufsize, int type, int unsign, int rev,
        int rowbytes, long pos, int flags, FILE *fp );
int  native_order( void );
void reverse_bytes( void *p, int elsize, int bytecount );

void free_window( void );
void outtext( int row, int col, char *a );
void clrtext( int r1, int c1, int r2, int c2 );

int  search( FILE *fp, int *pos, char *userstr, int searchtype,
        int datatype, int unsign, int byteorder );
int  parse( char *a, char *b, int type, int unsign, int byteorder );
