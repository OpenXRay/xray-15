/*
======================================================================
imgfilt.c

Image filter handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwfilter.h>
#include "pipeline.h"


typedef struct {
   int context;
} IFiltInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   IFiltInst *dat;

   dat = calloc( 1, sizeof( IFiltInst ));
   if ( dat )
      dat->context = ( int ) context;
   trace( "Create", IFLT_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( IFiltInst *dat )
{
   if ( dat ) {
      trace( "Destroy", IFLT_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( IFiltInst *to, IFiltInst *from )
{
   trace( "Copy", IFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( IFiltInst *dat, const LWLoadState *ls )
{
   trace( "Load", IFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( IFiltInst *dat, const LWSaveState *ls )
{
   trace( "Save", IFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( IFiltInst *dat )
{
   trace( "DescLn", IFLT_PNAME, NULL );
   return IFLT_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( IFiltInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", IFLT_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( IFiltInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", IFLT_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static int )
Flags( IFiltInst *dat )
{
   trace( "Flags", IFLT_PNAME, NULL );
   return 0;
}


XCALL_( static void )
Process( IFiltInst *dat, const LWFilterAccess *fa )
{
   LWFVector out;
   float *r, *g, *b, *a;
   int x, y;

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
         fa->setAlpha( x, y, a[ x ] );
      }
   }

   trace( "Process", IFLT_PNAME, NULL );
}


XCALL_( int )
ImageFilter( long version, GlobalFunc *global, LWImageFilterHandler *local,
   void *serverData )
{
   if ( version != LWIMAGEFILTER_VERSION ) return AFUNC_BADVERSION;

   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->copy     = Copy;
   local->inst->load     = Load;
   local->inst->save     = Save;
   local->inst->descln   = DescLn;
   if ( local->item ) {
      local->item->useItems = UseItems;
      local->item->changeID = ChangeID;
   }
   local->process        = Process;
   local->flags          = Flags;

   trace( "Activate", IFLT_PNAME, NULL );

   return AFUNC_OK;
}
