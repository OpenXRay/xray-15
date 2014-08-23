/*
=====================================================================
save.c

Save the LightWave depth buffer in 3D Nature's ZBUF format.  This
version is for LW 6 and later.

Chris "Xenon" Hanson  28 Nov 95
Ernie Wright  21 Jul 00
===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwserver.h>
#include <lwhost.h>
#include <lwfilter.h>
#include <lwpanel.h>
#include "zbuf6.h"


/* plug-in instance data */

typedef struct {
   GlobalFunc    *global;
   char           name[ 260 ];
   char           path[ 260 ];
   char           node[ 260 ];
   char           desc[ 128 ];
   ZBufferHeader  zhd;
} ZBufThing;


XCALL_( static ZBufThing * )
Create( void *priv, void *context, LWError *err )
{
   ZBufThing *zbt;

   zbt = calloc( 1, sizeof( ZBufThing ));
   if ( !zbt ) {
      *err = "Couldn't allocate instance data.";
      return NULL;
   }

   /* priv was set in the activation function */

   zbt->global = ( GlobalFunc * ) priv;

   strcpy( zbt->node, "zbuf" );
   strcpy( zbt->name, "zbuf" );

   zbt->zhd.datatype    = ZBVAR_FLOAT;
   zbt->zhd.compression = ZBCOMP_NONE;
   zbt->zhd.sorting     = ZBSORT_NEARTOFAR;
   zbt->zhd.units       = ZBUNIT_M;
   zbt->zhd.scale       = 1.0f;
   zbt->zhd.offset      = 0.0f;

   return zbt;
}


XCALL_( static void )
Destroy( ZBufThing *zbt )
{
   if ( zbt ) free( zbt );
}


XCALL_( static LWError )
Copy( ZBufThing *to, ZBufThing *from )
{
   *to = *from;
   return NULL;
}


XCALL_( static LWError )
Load( ZBufThing *zbt, const LWLoadState *ls )
{
   LWLOAD_STR( ls, zbt->name, sizeof( zbt->name ));

   LWLOAD_U2( ls, &zbt->zhd.datatype, 1 );
   LWLOAD_U2( ls, &zbt->zhd.compression, 1 );
   LWLOAD_U2( ls, &zbt->zhd.sorting, 1 );
   LWLOAD_U2( ls, &zbt->zhd.units, 1 );

   return NULL;
}


XCALL_( static LWError )
Save( ZBufThing *zbt, const LWSaveState *ss )
{
   LWSAVE_STR( ss, zbt->name );

   LWSAVE_U2( ss, &zbt->zhd.datatype, 1 );
   LWSAVE_U2( ss, &zbt->zhd.compression, 1 );
   LWSAVE_U2( ss, &zbt->zhd.sorting, 1 );
   LWSAVE_U2( ss, &zbt->zhd.units, 1 );

   return NULL;
}


XCALL_( static const char * )
DescLn( ZBufThing *zbt )
{
   sprintf( zbt->desc, ZSAVE_NAME ":  %s####.zb", zbt->node );
   return zbt->desc;
}


XCALL_( static unsigned int )
Flags( ZBufThing *zbt )
{
   return 1 << LWBUF_DEPTH;
}


#ifdef _WIN32
/*
=====================================================================
revbytes()

Reverses byte order in place.

INPUTS
   bp       bytes to reverse
   elsize   size of the underlying data type
   elcount  number of elements to swap

RESULTS
   Reverses the byte order in each of elcount elements.
===================================================================== */

void revbytes( void *bp, int elsize, int elcount )
{
   register unsigned char *p, *q;

   p = ( unsigned char * ) bp;

   if ( elsize == 2 ) {
      q = p + 1;
      while ( elcount-- ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         p += 2;
         q += 2;
      }
      return;
   }

   while ( elcount-- ) {
      q = p + elsize - 1;
      while ( p < q ) {
         *p ^= *q;
         *q ^= *p;
         *p ^= *q;
         ++p;
         --q;
      }
      p += elsize >> 1;
   }
}
#endif


static int write_header( ZBufThing *zbt, FILE *fp )
{
   unsigned long bsize, ck[ 3 ] = { ID_FORM, 0, ID_ILBM };
   ZBufferHeader zhd;

   bsize = zbt->zhd.w * zbt->zhd.h * sizeof( float );

   ck[ 1 ] = 56 + bsize;
   revbytes( &ck[ 1 ], 4, 1 );
   if ( 1 != fwrite( ck, 12, 1, fp )) return 0;

   ck[ 0 ] = ID_ZBUF;
   ck[ 1 ] = 36;
   revbytes( &ck[ 1 ], 4, 1 );
   if ( 1 != fwrite( ck, 8, 1, fp )) return 0;

   zhd = zbt->zhd;
   revbytes( &zhd.w, 4, 2 );
   revbytes( &zhd.datatype, 2, 4 );
   revbytes( &zhd.min, 4, 5 );
   if ( 1 != fwrite( &zhd, 36, 1, fp )) return 0;

   ck[ 0 ] = ID_ZBOD;
   ck[ 1 ] = bsize;
   revbytes( &ck[ 1 ], 4, 1 );
   if ( 1 != fwrite( ck, 8, 1, fp )) return 0;

   return 1;
}


#define MSG( a, b )  if ( msgf ) msgf->error( ZSAVE_NAME ":  " a, b )

XCALL_( static void )
Process( ZBufThing *zbt, const LWFilterAccess *fa )
{
   LWMessageFuncs *msgf;
   FILE *fp = NULL;
   float *r, *g, *b, *a, *z, *zbuf = NULL, out[ 3 ];
   char filename[ 260 ];
   int x, y;


   msgf = zbt->global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );

   if ( !zbt->name[ 0 ] ) {
      MSG( "No filename selected.", NULL );
      return;
   }

   /* pass the RGBA buffers through */

   for ( y = 0; y < fa->height; y++ ) {
      r = fa->getLine( LWBUF_RED, y );
      g = fa->getLine( LWBUF_GREEN, y );
      b = fa->getLine( LWBUF_BLUE, y );
      a = fa->getLine( LWBUF_ALPHA, y );

      for ( x = 0; x < fa->width; x++ ) {
         out[ 0 ] = r[ x ];
         out[ 1 ] = g[ x ];
         out[ 2 ] = b[ x ];
         fa->setRGB( x, y, out );
         if ( a ) fa->setAlpha( x, y, a[ x ] );
      }
   }

   /* find min, max and background */

   zbt->zhd.min = 1e20f;
   zbt->zhd.max = zbt->zhd.background = -1e20f;

   for ( y = 0; y < fa->height; y++ ) {
      z = fa->getLine( LWBUF_DEPTH, y );
      if ( !z ) {
         MSG( "Couldn't get depth buffer pointer.", NULL );
         goto Finish;
      }
      for ( x = 0; x < fa->width; x++ ) {
         if ( z[ x ] < zbt->zhd.min ) zbt->zhd.min = z[ x ];
         if ( z[ x ] > zbt->zhd.background ) zbt->zhd.background = z[ x ];
         if ( z[ x ] > zbt->zhd.max && z[ x ] < zbt->zhd.background )
            zbt->zhd.max = z[ x ];
      }
   }

   /* get memory for a local copy of one scanline */

   zbuf = malloc( fa->width * sizeof( float ));
   if ( !zbuf ) {
      MSG( "Couldn't allocate depth buffer scanline.", NULL );
      return;
   }

   /* open the file, write the header */

   sprintf( filename, "%s%04.4d.zb", zbt->name, fa->frame );

   fp = fopen( filename, "wb" );
   if ( !fp ) {
      MSG( "Couldn't open output file", filename );
      goto Finish;
   }

   zbt->zhd.w = fa->width;
   zbt->zhd.h = fa->height;
   if ( !write_header( zbt, fp )) {
      MSG( "Couldn't write headers", filename );
      goto Finish;
   }

   /* write the ZBOD data */

   for ( y = 0; y < fa->height; y++ ) {
      z = fa->getLine( LWBUF_DEPTH, y );
      if ( !z ) {
         MSG( "Couldn't get depth buffer pointer.", NULL );
         goto Finish;
      }
      memcpy( zbuf, z, fa->width * 4 );
      revbytes( zbuf, 4, fa->width );
      if ( 4 != fwrite( zbuf, fa->width, 4, fp )) {
         MSG( "Error while writing", filename );
         goto Finish;
      }
   }

Finish:
   if ( fp ) fclose( fp );
   if ( zbuf ) free( zbuf );
}


XCALL_( int )
ZSave_Handler( long version, GlobalFunc *global, LWImageFilterHandler *local,
   void *serverData )
{
   if ( version != LWIMAGEFILTER_VERSION )
      return AFUNC_BADVERSION;

   local->inst->priv    = global;
   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->descln  = DescLn;

   if ( local->item ) {
      local->item->useItems = NULL;
      local->item->changeID = NULL;
   }

   local->process = Process;
   local->flags   = Flags;

   return AFUNC_OK;
}


XCALL_( static LWError )
get_filename( ZBufThing *zbt )
{
   LWFileReqLocal frloc;
   LWFileActivateFunc *filereq;

   filereq = zbt->global( LWFILEACTIVATEFUNC_GLOBAL, GFUSE_TRANSIENT );
   if ( !filereq ) return "Couldn't open the file dialog.";

   frloc.reqType  = FREQ_SAVE;
   frloc.title    = "Z Buffer Base Name";
   frloc.bufLen   = sizeof( zbt->name );
   frloc.pickName = NULL;
   frloc.fileType = NULL;
   frloc.path     = zbt->path;
   frloc.baseName = zbt->node;
   frloc.fullName = zbt->name;

   filereq( LWFILEREQ_VERSION, &frloc );
   return NULL;
}


XCALL_( int )
ZSave_Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   local->panel = NULL;
   local->options = get_filename;
   local->command = NULL;

   return AFUNC_OK;
}
