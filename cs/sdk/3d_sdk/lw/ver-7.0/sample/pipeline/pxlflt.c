/*
======================================================================
pxlfilt.c

Pixel filter handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwfilter.h>
#include "pipeline.h"


typedef struct {
   int newtime;
   int context;
} PFiltInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   PFiltInst *dat;

   dat = calloc( 1, sizeof( PFiltInst ));
   if ( dat )
      dat->context = ( int ) context;
   trace( "Create", PFLT_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( PFiltInst *dat )
{
   if ( dat ) {
      trace( "Destroy", PFLT_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( PFiltInst *to, PFiltInst *from )
{
   trace( "Copy", PFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( PFiltInst *dat, const LWLoadState *ls )
{
   trace( "Load", PFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( PFiltInst *dat, const LWSaveState *ls )
{
   trace( "Save", PFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( PFiltInst *dat )
{
   trace( "DescLn", PFLT_PNAME, NULL );
   return PFLT_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( PFiltInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", PFLT_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( PFiltInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", PFLT_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Init( PFiltInst *dat, int mode )
{
   trace( "Init", PFLT_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( PFiltInst *dat )
{
   trace( "Cleanup", PFLT_PNAME, NULL );
}


XCALL_( static LWError )
NewTime( PFiltInst *dat, LWFrame fr, LWTime t )
{
   dat->newtime = 1;
   trace( "NewTime", PFLT_PNAME, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags( PFiltInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", PFLT_PNAME, NULL );
   return 0;
}


XCALL_( static void )
Evaluate( PFiltInst *dat, const LWPixelAccess *pa )
{
   float out[ 4 ];

   pa->getVal( LWBUF_RED, 4, out );
   pa->setRGBA( out );

   if ( dat->newtime ) {
      trace( "Evaluate", PFLT_PNAME, NULL );
      dat->newtime = 0;
   }
}


XCALL_( int )
PixelFilter( long version, GlobalFunc *global, LWPixelFilterHandler *local,
   void *serverData )
{
   if ( version != LWPIXELFILTER_VERSION ) return AFUNC_BADVERSION;

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
   local->rend->init     = Init;
   local->rend->cleanup  = Cleanup;
   local->rend->newTime  = NewTime;
   local->evaluate       = Evaluate;
   local->flags          = Flags;

   trace( "Activate", PFLT_PNAME, NULL );

   return AFUNC_OK;
}
