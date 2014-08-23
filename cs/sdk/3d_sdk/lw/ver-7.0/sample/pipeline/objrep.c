/*
======================================================================
objrep.c

Object replacement handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwobjrep.h>
#include "pipeline.h"


typedef struct {
   LWItemID item;
} ObjRepInst;


XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   ObjRepInst *dat;

   dat = calloc( 1, sizeof( ObjRepInst ));
   if ( dat )
      dat->item = item;

   trace( "Create", OREP_PNAME, "item = %x", item );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( ObjRepInst *dat )
{
   if ( dat ) {
      trace( "Destroy", OREP_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( ObjRepInst *to, ObjRepInst *from )
{
   trace( "Copy", OREP_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( ObjRepInst *dat, const LWLoadState *ls )
{
   trace( "Load", OREP_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( ObjRepInst *dat, const LWSaveState *ls )
{
   trace( "Save", OREP_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( ObjRepInst *dat )
{
   trace( "DescLn", OREP_PNAME, NULL );
   return OREP_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( ObjRepInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", OREP_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( ObjRepInst *dat, const LWItemID *idlist )
{
   int i;

   for ( i = 0; ; i += 2 ) {
      if ( !idlist[ i ] && !idlist[ i + 1 ] ) break;
      if ( idlist[ i ] == dat->item ) {
         dat->item = idlist[ i + 1 ];
         break;
      }
   }
   trace( "ChangeID", OREP_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static void )
Evaluate( ObjRepInst *dat, LWObjReplacementAccess *oa )
{
   oa->newFilename = NULL;
   trace( "Evaluate", OREP_PNAME, NULL );
}


XCALL_( int )
ObjRep( long version, GlobalFunc *global, LWObjReplacementHandler *local,
   void *serverData )
{
   if ( version != LWOBJREPLACEMENT_VERSION ) return AFUNC_BADVERSION;

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
   local->evaluate       = Evaluate;

   trace( "Activate", OREP_PNAME, NULL );

   return AFUNC_OK;
}
