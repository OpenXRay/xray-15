/*
======================================================================
master.c

Master handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwmaster.h>
#include "pipeline.h"


typedef struct {
   int foo;
} MastInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   MastInst *dat;

   dat = calloc( 1, sizeof( MastInst ));
   trace( "Create", MAST_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( MastInst *dat )
{
   if ( dat ) {
      trace( "Destroy", MAST_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( MastInst *to, MastInst *from )
{
   trace( "Copy", MAST_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( MastInst *dat, const LWLoadState *ls )
{
   trace( "Load", MAST_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( MastInst *dat, const LWSaveState *ls )
{
   trace( "Save", MAST_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( MastInst *dat )
{
   trace( "DescLn", MAST_PNAME, NULL );
   return MAST_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( MastInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", MAST_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( MastInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", MAST_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static int )
Flags( MastInst *dat )
{
   trace( "Flags", MAST_PNAME, NULL );
   return 0;
}


XCALL_( static double )
Event( MastInst *dat, const LWMasterAccess *ma )
{
   char *msg;

   switch ( ma->eventCode ) {
      case LWEVNT_NOTHING:  msg = "LWEVNT_NOTHING";  break;
      case LWEVNT_TIME:     msg = "LWEVNT_TIME";     break;
      case LWEVNT_COMMAND:  msg = ma->eventData;     break;
      default:              msg = "";                break;
   }

   trace( "Event", MAST_PNAME, "%s", msg );
   return 0.0;
}


XCALL_( int )
Master( long version, GlobalFunc *global, LWMasterHandler *local,
   void *serverData )
{
   if ( version != LWMASTER_VERSION ) return AFUNC_BADVERSION;

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
   local->type           = LWMAST_SCENE;
   local->event          = Event;
   local->flags          = Flags;

   trace( "Activate", MAST_PNAME, NULL );

   return AFUNC_OK;
}
