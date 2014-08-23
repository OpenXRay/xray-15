/*
======================================================================
channel.c

Channel handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Jul 01
====================================================================== */

#include <lwchannel.h>
#include "pipeline.h"


typedef struct {
   LWChannelID channel;
} ChanInst;


XCALL_( static LWInstance )
Create( void *priv, LWChannelID chan, LWError *err )
{
   ChanInst *dat;

   dat = calloc( 1, sizeof( ChanInst ));
   if ( dat )
      dat->channel = chan;
   trace( "Create", CHAN_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( ChanInst *dat )
{
   if ( dat ) {
      trace( "Destroy", CHAN_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( ChanInst *to, ChanInst *from )
{
   trace( "Copy", CHAN_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( ChanInst *dat, const LWLoadState *ls )
{
   trace( "Load", CHAN_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( ChanInst *dat, const LWSaveState *ls )
{
   trace( "Save", CHAN_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( ChanInst *dat )
{
   trace( "DescLn", CHAN_PNAME, NULL );
   return CHAN_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( ChanInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", CHAN_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( ChanInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", CHAN_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static int )
Flags( ChanInst *dat )
{
   trace( "Flags", CHAN_PNAME, NULL );
   return 0;
}


XCALL_( static void )
Evaluate( ChanInst *dat, const LWChannelAccess *ca )
{
   trace( "Evaluate", CHAN_PNAME, "frame = %d  time = %g",
      ca->frame, ca->time );
}


XCALL_( int )
Channel( long version, GlobalFunc *global, LWChannelHandler *local,
   void *serverData )
{
   if ( version != LWCHANNEL_VERSION ) return AFUNC_BADVERSION;

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
   local->flags          = Flags;

   trace( "Activate", CHAN_PNAME, NULL );

   return AFUNC_OK;
}
