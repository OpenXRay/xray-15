/*
======================================================================
framebuf.c

Frame buffer handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwserver.h>
#include <lwframbuf.h>
#include <stdio.h>
#include "pipeline.h"


typedef struct {
   int frame;
   int y;
} FrameBufInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   FrameBufInst *dat;

   dat = calloc( 1, sizeof( FrameBufInst ));
   trace( "Create", FBUF_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( FrameBufInst *dat )
{
   if ( dat ) {
      trace( "Destroy", FBUF_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( FrameBufInst *to, FrameBufInst *from )
{
   trace( "Copy", FBUF_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( FrameBufInst *dat, const LWLoadState *ls )
{
   trace( "Load", FBUF_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( FrameBufInst *dat, const LWSaveState *ls )
{
   trace( "Save", FBUF_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( FrameBufInst *dat )
{
   trace( "DescLn", FBUF_PNAME, NULL );
   return FBUF_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( FrameBufInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", FBUF_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( FrameBufInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", FBUF_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Open( FrameBufInst *dat, int w, int h )
{
   dat->frame = 0;
   trace( "Open", FBUF_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Close( FrameBufInst *dat )
{
   trace( "Close", FBUF_PNAME, NULL );
}


XCALL_( static LWError )
Begin( FrameBufInst *dat )
{
   ++dat->frame;
   dat->y = 0;

   trace( "Begin", FBUF_PNAME, "%d", dat->frame );

   return NULL;
}


XCALL_( static LWError )
Write( FrameBufInst *dat, const void *r, const void *g, const void *b,
   const void *a )
{
   if ( dat->y == 0 )
      trace( "Write", FBUF_PNAME, NULL );
   ++dat->y;
   return NULL;
}


XCALL_( static void )
Pause( FrameBufInst *dat )
{
   trace( "Pause", FBUF_PNAME, NULL );
}


XCALL_( int )
FrameBuffer( long version, GlobalFunc *global, LWFrameBufferHandler *local,
   void *serverData )
{
   if ( version != LWFRAMEBUFFER_VERSION ) return AFUNC_BADVERSION;

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
   local->type           = LWFBT_FLOAT;
   local->open           = Open;
   local->close          = Close;
   local->begin          = Begin;
   local->write          = Write;
   local->pause          = Pause;

   trace( "Activate", FBUF_PNAME, NULL );

   return AFUNC_OK;
}
