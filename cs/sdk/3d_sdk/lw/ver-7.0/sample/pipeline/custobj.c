/*
======================================================================
custobj.c

Custom object handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Sep 01
====================================================================== */

#include <lwcustobj.h>
#include "pipeline.h"


typedef struct {
   LWItemID item;
} CObjInst;


XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   CObjInst *dat;

   dat = calloc( 1, sizeof( CObjInst ));
   if ( dat )
      dat->item = item;
   trace( "Create", COBJ_PNAME, "item = %x", item );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( CObjInst *dat )
{
   if ( dat ) {
      trace( "Destroy", COBJ_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( CObjInst *to, CObjInst *from )
{
   trace( "Copy", COBJ_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( CObjInst *dat, const LWLoadState *ls )
{
   trace( "Load", COBJ_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( CObjInst *dat, const LWSaveState *ls )
{
   trace( "Save", COBJ_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( CObjInst *dat )
{
   trace( "DescLn", COBJ_PNAME, NULL );
   return COBJ_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( CObjInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", COBJ_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( CObjInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", COBJ_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Init( CObjInst *dat, int mode )
{
   trace( "Init", COBJ_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( CObjInst *dat )
{
   trace( "Cleanup", COBJ_PNAME, NULL );
}


XCALL_( static LWError )
NewTime( CObjInst *dat, LWFrame fr, LWTime t )
{
   trace( "NewTime", COBJ_PNAME, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags( CObjInst *dat )
{
   trace( "Flags", COBJ_PNAME, NULL );
   return 0;
}


XCALL_( static void )
Evaluate( CObjInst *dat, const LWCustomObjAccess *access )
{
   trace( "Evaluate", COBJ_PNAME, NULL );
}


XCALL_( int )
CustObject( long version, GlobalFunc *global, LWCustomObjHandler *local,
   void *serverData)
{
   if ( version != LWCUSTOMOBJ_VERSION )
      return AFUNC_BADVERSION;

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

   trace( "Activate", COBJ_PNAME, NULL );

   return AFUNC_OK;
}
