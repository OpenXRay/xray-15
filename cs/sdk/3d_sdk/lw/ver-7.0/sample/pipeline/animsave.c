/*
======================================================================
animsave.c

Animation saver handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Jul 01
====================================================================== */

#include <lwanimsav.h>
#include "pipeline.h"


typedef struct {
   int frame;
   int y;
} AnimSaveInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   AnimSaveInst *dat;

   dat = calloc( 1, sizeof( AnimSaveInst ));
   trace( "Create", ANSV_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( AnimSaveInst *dat )
{
   if ( dat ) {
      trace( "Destroy", ANSV_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( AnimSaveInst *to, AnimSaveInst *from )
{
   trace( "Copy", ANSV_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( AnimSaveInst *dat, const LWLoadState *ls )
{
   trace( "Load", ANSV_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( AnimSaveInst *dat, const LWSaveState *ls )
{
   trace( "Save", ANSV_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( AnimSaveInst *dat )
{
   trace( "DescLn", ANSV_PNAME, NULL );
   return ANSV_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( AnimSaveInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", ANSV_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( AnimSaveInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", ANSV_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Open( AnimSaveInst *dat, int w, int h, const char *filename )
{
   dat->frame = 0;
   trace( "Open", ANSV_PNAME, "%s", filename );
   return NULL;
}


XCALL_( static void )
Close( AnimSaveInst *dat )
{
   trace( "Close", ANSV_PNAME, NULL );
}


XCALL_( static LWError )
Begin( AnimSaveInst *dat )
{
   ++dat->frame;
   dat->y = 0;

   trace( "Begin", ANSV_PNAME, "%d", dat->frame );

   return NULL;
}


XCALL_( static LWError )
Write( AnimSaveInst *dat, const void *r, const void *g, const void *b,
   const void *a )
{
   if ( dat->y == 0 )
      trace( "Write", ANSV_PNAME, NULL );
   ++dat->y;
   return NULL;
}


XCALL_( int )
AnimSave( long version, GlobalFunc *global, LWAnimSaverHandler *local,
   void *serverData )
{
   if ( version != LWANIMSAVER_VERSION ) return AFUNC_BADVERSION;

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
   local->type           = LWAST_FLOAT;
   local->open           = Open;
   local->close          = Close;
   local->begin          = Begin;
   local->write          = Write;

   trace( "Activate", ANSV_PNAME, NULL );

   return AFUNC_OK;
}
