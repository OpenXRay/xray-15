/*
======================================================================
motion.c

Item motion handler for the pipeline tracer.  See pipeline.c

Ernie Wright  15 Jul 01
====================================================================== */

#include <lwmotion.h>
#include "pipeline.h"


typedef struct {
   LWItemID item;
   char *   pname;
} MotInst;


XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   MotInst *dat;

   dat = calloc( 1, sizeof( MotInst ));
   if ( dat ) {
      dat->item = item;
      dat->pname = priv;
   }
   trace( "Create", dat->pname, "item = %x", item );
   return ( LWInstance ) dat;
}


XCALL_( static void )
Destroy( MotInst *dat )
{
   if ( dat ) {
      trace( "Destroy", dat->pname, NULL );
      free( dat );
   }
}


XCALL_( static LWError )
Copy( MotInst *to, MotInst *from )
{
   trace( "Copy", to->pname, NULL );
   return NULL;
}


XCALL_( static LWError )
Load( MotInst *dat, const LWLoadState *ls )
{
   trace( "Load", dat->pname, NULL );
   return NULL;
}


XCALL_( static LWError )
Save( MotInst *dat, const LWSaveState *ls )
{
   trace( "Save", dat->pname, NULL );
   return NULL;
}


XCALL_( static const char * )
DescLn( MotInst *dat )
{
   trace( "DescLn", dat->pname, NULL );
   return dat->pname;
}


XCALL_( static const LWItemID * )
UseItems( MotInst *dat )
{
   static LWItemID id[ 2 ] = { LWITEM_ALL, LWITEM_NULL };

   trace( "UseItems", dat->pname, NULL );
   return id;
}


XCALL_( static void )
ChangeID( MotInst *dat, const LWItemID *idlist )
{
   int i;

   for ( i = 0; ; i += 2 ) {
      if ( !idlist[ i ] && !idlist[ i + 1 ] ) break;
      if ( idlist[ i ] == dat->item ) {
         dat->item = idlist[ i + 1 ];
         break;
      }
   }
   trace( "ChangeID", dat->pname, "%x %x", idlist[ 0 ], idlist[ 1 ] );
}


XCALL_( static int )
Flags1( MotInst *dat )
{
   trace( "Flags", dat->pname, NULL );
   return 0;
}


XCALL_( static int )
Flags2( MotInst *dat )
{
   trace( "Flags", dat->pname, NULL );
   return LWIMF_AFTERIK;
}


XCALL_( static void )
Evaluate( MotInst *dat, const LWItemMotionAccess *ia )
{
   trace( "Evaluate", dat->pname, "frame = %d  time = %g", ia->frame, ia->time );
}


static int set_callbacks( LWItemMotionHandler *local, int version,
   char *pname, int (*flags)(LWInstance))
{
   if ( version != LWITEMMOTION_VERSION ) return AFUNC_BADVERSION;

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
   local->evaluate       = Evaluate;
   local->flags          = flags;

   trace( "Activate", pname, NULL );

   return AFUNC_OK;
}


XCALL_( int )
Motion1( long version, GlobalFunc *global, LWItemMotionHandler *local,
   void *serverData )
{
   return set_callbacks( local, version, MOTN_PNAME1, Flags1 );
}


XCALL_( int )
Motion2( long version, GlobalFunc *global, LWItemMotionHandler *local,
   void *serverData )
{
   return set_callbacks( local, version, MOTN_PNAME2, Flags2 );
}
