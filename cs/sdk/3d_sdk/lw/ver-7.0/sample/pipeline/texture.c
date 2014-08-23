/*
======================================================================
texture.c

Procedural texture handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  16 Jul 01
====================================================================== */

#include <lwtexture.h>
#include <lwtxtr.h>
#include "pipeline.h"


typedef struct {
   LWTextureID texture;
   int         newtime;
   double     (*noise)( double p[ 3 ] );
} TxtrInst;


XCALL_( static LWInstance )
Create( void *priv, void *context, LWError *err )
{
   TxtrInst *dat;
   GlobalFunc *global;
   LWTextureFuncs *txtrf;

   dat = calloc( 1, sizeof( TxtrInst ));
   if ( dat ) {
      dat->texture = ( LWTextureID ) context;
      global = ( GlobalFunc * ) priv;
      txtrf = global( LWTEXTUREFUNCS_GLOBAL, GFUSE_TRANSIENT );
      dat->noise = txtrf->noise;
   }
   trace( "Create", TXTR_PNAME, NULL );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( TxtrInst *dat )
{
   if ( dat ) {
      trace( "Destroy", TXTR_PNAME, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( TxtrInst *to, TxtrInst *from )
{
   trace( "Copy", TXTR_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( TxtrInst *dat, const LWLoadState *ls )
{
   trace( "Load", TXTR_PNAME, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( TxtrInst *dat, const LWSaveState *ls )
{
   trace( "Save", TXTR_PNAME, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( TxtrInst *dat )
{
   trace( "DescLn", TXTR_PNAME, NULL );
   return TXTR_PNAME;
}


XCALL_( static const LWItemID * )
UseItems( TxtrInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", TXTR_PNAME, NULL );
   return id;
}


XCALL_( static void )
ChangeID( TxtrInst *dat, const LWItemID *idlist )
{
   trace( "ChangeID", TXTR_PNAME, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static LWError )
Init( TxtrInst *dat, int mode )
{
   trace( "Init", TXTR_PNAME, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( TxtrInst *dat )
{
   trace( "Cleanup", TXTR_PNAME, NULL );
}


XCALL_( static LWError )
NewTime( TxtrInst *dat, LWFrame fr, LWTime t )
{
   dat->newtime = 1;
   trace( "NewTime", TXTR_PNAME, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags( TxtrInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", TXTR_PNAME, NULL );
   return 0;
}


XCALL_( static double )
Evaluate( TxtrInst *dat, LWTextureAccess *ta )
{
   if ( dat->newtime ) {
      trace( "Evaluate", TXTR_PNAME, NULL );
      dat->newtime = 0;
   }
   return dat->noise( ta->tPos );
}


XCALL_( int )
Texture( long version, GlobalFunc *global, LWTextureHandler *local,
   void *serverData )
{
   if ( version != LWTEXTURE_VERSION ) return AFUNC_BADVERSION;

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

   trace( "Activate", TXTR_PNAME, NULL );

   return AFUNC_OK;
}
