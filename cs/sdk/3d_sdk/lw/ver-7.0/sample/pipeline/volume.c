/*
======================================================================
volume.c

Volumetric handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Jul 01
====================================================================== */

#include <lwvolume.h>
#include "pipeline.h"


typedef struct {
   int newtime;
} VolInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   VolInst *dat;

   dat = calloc( 1, sizeof( VolInst ));
   trace( "Create", VOLU_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( VolInst *dat )
{
   if ( dat ) {
      trace( "Destroy", VOLU_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( VolInst *to, VolInst *from )
{
   trace( "Copy", VOLU_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( VolInst *dat, const LWLoadState *ls )
{
   trace( "Load", VOLU_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( VolInst *dat, const LWSaveState *ls )
{
   trace( "Save", VOLU_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( VolInst *dat )
{
   trace( "DescLn", VOLU_PNAME, NULL );
   return VOLU_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( VolInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", VOLU_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( VolInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", VOLU_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Init( VolInst *dat, int mode )
{
   trace( "Init", VOLU_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( VolInst *dat )
{
   trace( "Cleanup", VOLU_PNAME, NULL );
}


XCALL_( static LWError )
NewTime( VolInst *dat, LWFrame fr, LWTime t )
{
   dat->newtime = 1;
   trace( "NewTime", VOLU_PNAME, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags( VolInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", VOLU_PNAME, NULL );
   return 0;
}


XCALL_( static double )
Evaluate( VolInst *dat, LWVolumeAccess *va )
{
   if ( dat->newtime ) {
      trace( "Evaluate", VOLU_PNAME, NULL );
      dat->newtime = 0;
   }
   return 1.0;
}


XCALL_( int )
Volume( long version, GlobalFunc *global, LWVolumetricHandler *local,
   void *serverData )
{
   if ( version != LWVOLUMETRIC_VERSION ) return AFUNC_BADVERSION;

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

   trace( "Activate", VOLU_PNAME, NULL );

   return AFUNC_OK;
}
