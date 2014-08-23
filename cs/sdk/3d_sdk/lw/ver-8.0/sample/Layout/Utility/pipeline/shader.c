/*
======================================================================
shader.c

Shader handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Jul 01
====================================================================== */

#include <lwshader.h>
#include <lwsurf.h>
#include "pipeline.h"


typedef struct {
   LWSurfaceID surface;
   int         newtime;
} ShadInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   ShadInst *dat;
   GlobalFunc *global;
   LWSurfaceFuncs *surff;
   const char *surfname = NULL;

   dat = calloc( 1, sizeof( ShadInst ));
   if ( dat ) {
      dat->surface = ( LWSurfaceID ) context;
      global = ( GlobalFunc * ) priv;
      surff = global( LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT );
      surfname = surff->name( dat->surface );
   }
   trace( "Create", SHAD_PNAME, "%s", surfname );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( ShadInst *dat )
{
   if ( dat ) {
      trace( "Destroy", SHAD_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( ShadInst *to, ShadInst *from )
{
   trace( "Copy", SHAD_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( ShadInst *dat, const LWLoadState *ls )
{
   trace( "Load", SHAD_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( ShadInst *dat, const LWSaveState *ls )
{
   trace( "Save", SHAD_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( ShadInst *dat )
{
   trace( "DescLn", SHAD_PNAME, NULL );
   return SHAD_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( ShadInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", SHAD_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( ShadInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", SHAD_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Init( ShadInst *dat, int mode )
{
   trace( "Init", SHAD_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( ShadInst *dat )
{
   trace( "Cleanup", SHAD_PNAME, NULL );
}


XCALL_( static LWError )
NewTime( ShadInst *dat, LWFrame fr, LWTime t )
{
   dat->newtime = 1;
   trace( "NewTime", SHAD_PNAME, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags( ShadInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", SHAD_PNAME, NULL );
   return LWSHF_COLOR;
}


XCALL_( static void )
Evaluate( ShadInst *dat, LWShaderAccess *sa )
{
   if ( dat->newtime ) {
      trace( "Evaluate", SHAD_PNAME, NULL );
      dat->newtime = 0;
   }
}


XCALL_( int )
Shader( long version, GlobalFunc *global, LWShaderHandler *local,
   void *serverData )
{
   if ( version != LWSHADER_VERSION ) return AFUNC_BADVERSION;

   local->inst->priv     = global;
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

   trace( "Activate", SHAD_PNAME, NULL );

   return AFUNC_OK;
}
