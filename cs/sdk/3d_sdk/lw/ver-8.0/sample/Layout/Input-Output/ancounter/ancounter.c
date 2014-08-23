/*
======================================================================
ancounter.c

A LW procedural anim loader that creates an image of a time counter.

This plug-in "loads" a simple text file describing the font that will
be used to draw the text of the counter.  The file looks like this,

   Counter AnimLoader File
   Times New Roman
   40
   700

(without the indent).  The first line identifies the file type, so
that the loader will recognize it (and others hopefully won't), and it
must be exactly as it appears here.  The second line is the font name,
the third is the size, and the fourth is the weight, expressed as a
number between 100 and 700.

The evaluate() function creates image frames on the fly that are based
on the animation time.  Windows-specific code is used to rasterize the
text.  Similar code for other platforms is left as an exercise.
====================================================================== */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lwserver.h>
#include <lwanimlod.h>
#include <lwrender.h>


typedef struct st_Counter {
   int            w, h;          // image dimensions
   unsigned char *buf;           // grayscale pixels
   char           font[ 80 ];    // font name
   int            size;          // font height
   int            weight;        // font weight
   int            descent;       // pixels below the baseline
   char           descln[ 80 ];  // instance description
   double         fps;           // frames per second
} Counter;

typedef struct st_Cell {
   unsigned char *buf;           // grayscale character image
   int            size;          // size of buf
   int            span;          // buf bytes per row
   GLYPHMETRICS   gm;            // bitmap dimensions, cell size
} Cell;


GlobalFunc *gglobal;


/*
======================================================================
freeCell()

Free memory allocated by getCell.
====================================================================== */

static void freeCell( Cell *cell )
{
   if ( cell ) {
      if ( cell->buf ) free( cell->buf );
      free( cell );
   }
}


/*
======================================================================
getCell()

Create a bitmap containing the image of a single character.  This uses
the Win32 GetGlyphOutline() function, which rasterizes a character
into a grayscale pixel buffer we provide.
====================================================================== */

static Cell *getCell( HDC hdc, int c )
{
   Cell *cell;
   DWORD result;
   MAT2 mat = {{ 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 }};


   cell = calloc( 1, sizeof( Cell ));
   if ( !cell ) return NULL;

   cell->size = GetGlyphOutline( hdc, c, GGO_GRAY8_BITMAP, &cell->gm,
      0, NULL, &mat );

   if ( cell->size == 0 )
      return cell;
   else if ( cell->size < 0 ) {
      free( cell );
      return NULL;
   }

   cell->buf = malloc( cell->size );
   if ( !cell->buf ) {
      free( cell );
      return NULL;
   }

   result = GetGlyphOutline( hdc, c, GGO_GRAY8_BITMAP, &cell->gm,
      cell->size, cell->buf, &mat );

   if ( result == GDI_ERROR ) {
      freeCell( cell );
      return 0;
   }

   cell->span = (( cell->gm.gmBlackBoxX + 3 ) >> 2 ) << 2;

   return cell;
}


/*
======================================================================
getTextImage()

Create a bitmap containing the image of a text string.  We get the
character rasters as an array of Cells created by getCell() and then
"print" the characters to a grayscale image buffer.
====================================================================== */

static int getTextImage( Counter *counter, char *text )
{
   HDC hdc;
   HFONT hfont;
   LOGFONT lf = { 0 };
   Cell **cell;
   int i, j, k, ncells, x, y, dy, bx, by, ox, oy, gray;


   lf.lfHeight = counter->size;
   lf.lfWeight = counter->weight;
   lf.lfQuality = ANTIALIASED_QUALITY;
   strcpy( lf.lfFaceName, counter->font );

   hdc = CreateCompatibleDC( NULL );
   hfont = SelectObject( hdc, CreateFontIndirect( &lf ));

   ncells = strlen( text );
   cell = calloc( ncells, sizeof( Cell ));
   if ( !cell ) return 0;

   for ( i = 0; i < ncells; i++ ) {
      cell[ i ] = getCell( hdc, text[ i ] );
      if ( !cell[ i ] ) break;
   }

   hfont = SelectObject( hdc, hfont );
   DeleteObject( hfont );
   DeleteDC( hdc );

   if ( i < ncells ) {
      for ( j = 0; j < i; j++ ) freeCell( cell[ i ] );
      free( cell );
      return 0;
   }

   memset( counter->buf, 0, counter->w * counter->h );

   for ( k = 0, x = 0; k < ncells; k++ ) {
      if ( cell[ k ]->size ) {
         bx = cell[ k ]->gm.gmBlackBoxX;
         by = cell[ k ]->gm.gmBlackBoxY;
         ox = cell[ k ]->gm.gmptGlyphOrigin.x;
         oy = cell[ k ]->gm.gmptGlyphOrigin.y;

         for ( j = 0; j < by; j++ ) {
            y = counter->h - counter->descent - oy + j;
            dy = y * counter->w;
            for ( i = 0; i < bx; i++ ) {
               gray = cell[ k ]->buf[ j * cell[ k ]->span + i ] << 2;
               if ( gray > 255 ) gray = 255;
               counter->buf[ dy + x + ox + i ] = gray;
            }
         }
      }
      x += cell[ k ]->gm.gmCellIncX;
   }

   for ( i = 0; i < ncells; i++ )
      freeCell( cell[ i ] );
   free( cell );

   return 1;
}


/*
======================================================================
getTextImageDim()

Precalculate the width and height of the text image, as well as the
font's descent value.
====================================================================== */

static void getTextImageDim( Counter *counter, char *text )
{
   HDC hdc;
   HFONT hfont;
   LOGFONT lf = { 0 };
   SIZE size;
   TEXTMETRIC tm;
   int len;

   lf.lfHeight = counter->size;
   lf.lfWeight = counter->weight;
   lf.lfQuality = ANTIALIASED_QUALITY;
   strcpy( lf.lfFaceName, counter->font );

   hdc = CreateCompatibleDC( NULL );
   hfont = SelectObject( hdc, CreateFontIndirect( &lf ));

   len = strlen( text );
   GetTextExtentPoint32( hdc, text, len, &size );
   GetTextMetrics( hdc, &tm );

   hfont = SelectObject( hdc, hfont );
   DeleteObject( hfont );
   DeleteDC( hdc );

   counter->w = size.cx;
   counter->h = size.cy;
   counter->descent = tm.tmDescent;
}


/*
======================================================================
Create()

Standard handler create function.  Check to see whether the file is
our format, and if it's not, go no further and return NULL.  Otherwise
allocate and initialize a Counter structure as our instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, char *filename, LWError *emsg )
{
   Counter *counter;
   FILE *fp;
   char buf[ 80 ], *p;

   fp = fopen( filename, "r" );
   if ( !fp ) {
      *emsg = "Couldn't open anim file.";
      return NULL;
   }

   fgets( buf, sizeof( buf ), fp );

   if ( strncmp( buf, "Counter AnimLoader File", 23 )) {
      fclose( fp );
      return NULL;
   }

   counter = calloc( 1, sizeof( Counter ));
   if ( !counter ) {
      *emsg = "No memory for instance data.";
      fclose( fp );
      return NULL;
   }

   fgets( buf, sizeof( buf ), fp );
   p = strtok( buf, "\n" );
   strcpy( counter->font, p );

   fgets( buf, sizeof( buf ), fp );
   counter->size = atoi( buf );

   fgets( buf, sizeof( buf ), fp );
   counter->weight = atoi( buf );

   fclose( fp );

   sprintf( counter->descln, "%s %d %d",
      counter->font, counter->size, counter->weight );

   getTextImageDim( counter, "-00:00:00:00" );
   counter->buf = malloc( counter->w * counter->h );
   if ( !counter->buf ) {
      free( counter );
      *emsg = "No memory for image buffer.";
      return NULL;
   }

   return ( LWInstance ) counter;
}


/*
======================================================================
Destroy()

Standard handler function.  Free memory allocated by Create().
====================================================================== */

XCALL_( static void )
Destroy( Counter *counter )
{
   if ( counter ) {
      if ( counter->buf ) free( counter->buf );
      free( counter );
   }
}


/*
======================================================================
Copy(), Load(), Save(), DescLn()

We don't make much use of these, but they're here in case we want to
add something later.
====================================================================== */

XCALL_( static LWError )
Copy( Counter *to, Counter *from ) { return NULL; }

XCALL_( static LWError )
Load( Counter *counter, const LWLoadState *ls ) { return NULL; }

XCALL_( static LWError )
Save( Counter *counter, const LWSaveState *ss ) { return NULL; }

XCALL_( static const char * )
DescLn( Counter *counter ) { return counter->descln; }


/*
======================================================================
FrameCount()

LW calls this just after Create() to get the anim file's frame count.
Since we're making up the anim frames as we go, we don't really have a
frame count, so we return a frame count based on scene settings.
====================================================================== */

XCALL_( static int )
FrameCount( Counter *counter )
{
   LWSceneInfo *lws;

   lws = gglobal( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !lws ) return 60;

   return lws->frameEnd - lws->frameStart + 1;
}


/*
======================================================================
FrameRate()

LW calls this just after Create() to get the anim file's frame rate,
or frames per second.
====================================================================== */

XCALL_( static double )
FrameRate( Counter *counter )
{
   LWSceneInfo *lws;

   lws = gglobal( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( lws )
      counter->fps = lws->framesPerSecond;
   else
      counter->fps = 30.0;

   return counter->fps;
}


/*
======================================================================
Aspect()

LW calls this just after Create() to get the anim file's image
dimensions.
====================================================================== */

XCALL_( static double )
Aspect( Counter *counter, int *w, int *h, double *pixaspect )
{
   *w = counter->w;
   *h = counter->h;
   *pixaspect = 1.0;
   return ( double ) counter->w / counter->h;
}


/*
======================================================================
Evaluate()

This is the fun part.  LW calls this at every new frame.  We construct
a string based on the time argument, convert it into a bitmap, and
send the bitmap as the current "frame" in our animation.
====================================================================== */

XCALL_( static void )
Evaluate( Counter *counter, double t, LWAnimFrameAccess *access )
{
   LWImageProtocolID ip;
   char text[ 32 ];
   int h, m, s, f, n = ' ', i, result;


   if ( t < 0 ) { n = '-'; t = -t; }
   h = ( int ) t / 3600;
   m = (( int ) t % 3600 ) / 60;
   s = ( int ) t % 60;
   f = ( int )(( t - floor( t )) * counter->fps );
   sprintf( text, "%c%02.2d:%02.2d:%02.2d:%02.2d", n, h, m, s, f );

   if ( !getTextImage( counter, text )) return;

   ip = access->begin( access->priv_data, LWIMTYP_GREY8 );
   if ( !ip ) return;

   LWIP_SETSIZE( ip, counter->w, counter->h );
   LWIP_SETPARAM( ip, LWIMPAR_ASPECT, 0, 1.0f );

   for ( i = 0; i < counter->h; i++ ) {
      result = LWIP_SENDLINE( ip, i, counter->buf + i * counter->w );
      if ( result != IPSTAT_OK ) break;
   }

   LWIP_DONE( ip, result );
   access->done( access->priv_data, ip );
}


/*
======================================================================
CounterLoader()

The activation function.
====================================================================== */

XCALL_( int )
CounterLoader( long version, GlobalFunc *global, LWAnimLoaderHandler *local,
   void *serverData )
{
   if ( version != LWANIMLOADER_VERSION ) return AFUNC_BADVERSION;

   gglobal = global;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->descln  = DescLn;
   local->frameCount    = FrameCount;
   local->frameRate     = FrameRate;
   local->aspect        = Aspect;
   local->evaluate      = Evaluate;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWANIMLOADER_HCLASS, "CounterAnimLoader", CounterLoader },
   { NULL }
};
