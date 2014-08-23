/*
======================================================================
environ.c

Environment handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwenviron.h>
#include "pipeline.h"


typedef struct {
   int newtime;
} EnvironInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   EnvironInst *dat;

   dat = calloc( 1, sizeof( EnvironInst ));
   trace( "Create", ENVI_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( EnvironInst *dat )
{
   if ( dat ) {
      trace( "Destroy", ENVI_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( EnvironInst *to, EnvironInst *from )
{
   trace( "Copy", ENVI_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( EnvironInst *dat, const LWLoadState *ls )
{
   trace( "Load", ENVI_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( EnvironInst *dat, const LWSaveState *ls )
{
   trace( "Save", ENVI_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( EnvironInst *dat )
{
   trace( "DescLn", ENVI_PNAME, NULL );
   return ENVI_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( EnvironInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", ENVI_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( EnvironInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", ENVI_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Init( EnvironInst *dat, int mode )
{
   trace( "Init", ENVI_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( EnvironInst *dat )
{
   trace( "Cleanup", ENVI_PNAME, NULL );
}


XCALL_( static LWError )
NewTime( EnvironInst *dat, LWFrame fr, LWTime t )
{
   dat->newtime = 1;
   trace( "NewTime", ENVI_PNAME, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags( EnvironInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", ENVI_PNAME, NULL );
   return 0;
}


XCALL_( static LWError )
Evaluate( EnvironInst *dat, LWEnvironmentAccess *ea )
{
   if ( dat->newtime ) {
      trace( "Evaluate", ENVI_PNAME, NULL );
      dat->newtime = 0;
   }
   return NULL;
}


XCALL_( int )
Environment( long version, GlobalFunc *global, LWEnvironmentHandler *local,
   void *serverData )
{
   if ( version != LWENVIRONMENT_VERSION ) return AFUNC_BADVERSION;

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

   trace( "Activate", ENVI_PNAME, NULL );

   return AFUNC_OK;
}
