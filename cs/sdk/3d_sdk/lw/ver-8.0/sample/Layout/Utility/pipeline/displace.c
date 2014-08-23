/*
======================================================================
displace.c

Displacement handler for the pipeline tracer.  See pipeline.c.

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwdisplce.h>
#include "pipeline.h"


typedef struct {
   LWItemID    item;
   char *      pname;
   int         newtime;
} DispInst;


XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   DispInst *dat;

   dat = calloc( 1, sizeof( DispInst ));
   if ( dat ) {
      dat->item = item;
      dat->pname = priv;
   }
   trace( "Create", dat->pname, "item = %x", item );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( DispInst *dat )
{
   if ( dat ) {
      trace( "Destroy", dat->pname, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( DispInst *to, DispInst *from )
{
   trace( "Copy", to->pname, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( DispInst *dat, const LWLoadState *ls )
{
   trace( "Load", dat->pname, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( DispInst *dat, const LWSaveState *ls )
{
   trace( "Save", dat->pname, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( DispInst *dat )
{
   trace( "DescLn", dat->pname, NULL );
   return dat->pname;
}


XCALL_( static const LWItemID * )
UseItems( DispInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", dat->pname, NULL );
   return id;
}


XCALL_( static void )
ChangeID( DispInst *dat, const LWItemID *idlist )
{
   int i;

   for ( i = 0; ; i += 2 ) {
      if ( !idlist[ i ] && !idlist[ i + 1 ] ) break;
      if ( idlist[ i ] == dat->item ) {
         dat->item = idlist[ i + 1 ];
         break;
      }
   }
   trace( "ChangeID", dat->pname, "%x %x", idlist[ i ], idlist[ i + 1 ] );
}


XCALL_( static LWError )
Init( DispInst *dat, int mode )
{
   trace( "Init", dat->pname, NULL );
   return NULL;
}


XCALL_( static void )
Cleanup( DispInst *dat )
{
   trace( "Cleanup", dat->pname, NULL );
}


XCALL_( static LWError )
NewTime( DispInst *dat, LWFrame fr, LWTime t )
{
   dat->newtime = 1;
   trace( "NewTime", dat->pname, "frame = %d  time = %g", fr, t );
   return NULL;
}


XCALL_( static int )
Flags1( DispInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", dat->pname, NULL );
   return LWDMF_BEFOREBONES;
}


XCALL_( static int )
Flags2( DispInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", dat->pname, NULL );
   return 0;
}


XCALL_( static int )
Flags3( DispInst *dat )
{
   if ( dat->newtime )
      trace( "Flags", dat->pname, NULL );
   return LWDMF_WORLD;
}


XCALL_( static void )
Evaluate( DispInst *dat, LWDisplacementAccess *da )
{
   if ( dat->newtime ) {
      trace( "Evaluate", dat->pname, NULL );
      dat->newtime = 0;
   }
}


static int set_callbacks( LWDisplacementHandler *local, int version,
   char *pname, int (*flags)(LWInstance))
{
   if ( version != LWDISPLACEMENT_VERSION ) return AFUNC_BADVERSION;

   local->inst->priv     = pname;
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
   local->flags          = flags;

   trace( "Activate", pname, NULL );

   return AFUNC_OK;
}


XCALL_( int )
Displace1( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData )
{
   return set_callbacks( local, version, DISP_PNAME1, Flags1 );
}


XCALL_( int )
Displace2( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData )
{
   return set_callbacks( local, version, DISP_PNAME2, Flags2 );
}


XCALL_( int )
Displace3( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData )
{
   return set_callbacks( local, version, DISP_PNAME3, Flags3 );
}
