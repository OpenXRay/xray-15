/*
======================================================================
init.c

Ernie Wright  7 Jan 00

This module, part of the ModLib library, contains the functions used
to begin and end a Modeler plug-in session.
====================================================================== */

#include <stdlib.h>
#include <assert.h>
#include "lwmodlib.h"


static ModData md;


static int ginit( GlobalFunc *global )
{
   unsigned long info;
   int major, minor;

   md.global = global;

   info = ( unsigned long ) global( LWPRODUCTINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !info ) return 0;

   md.build = LWINF_GETBUILD( info );
   md.product = info & LWINF_PRODUCT;

   major = LWINF_GETMAJOR( info );
   minor = LWINF_GETMINOR( info );
   md.version = ( float )( major + minor / 10.0 );

   info = ( unsigned long ) global( LWSYSTEMID_GLOBAL, GFUSE_TRANSIENT );
   md.serialno = info & LWSYS_SERIALBITS;

   md.locale = ( unsigned long ) global( LWLOCALEINFO_GLOBAL, GFUSE_TRANSIENT );

   md.monf    = global( LWDYNAMONITORFUNCS_GLOBAL, GFUSE_TRANSIENT );
   md.query   = global( LWSTATEQUERYFUNCS_GLOBAL, GFUSE_TRANSIENT );
   md.surface = global( LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   md.font    = global( LWFONTLISTFUNCS_GLOBAL, GFUSE_TRANSIENT );
   md.hdi     = global( LWHOSTDISPLAYINFO_GLOBAL, GFUSE_TRANSIENT );
   md.filereq = global( LWFILEREQFUNC_GLOBAL, GFUSE_TRANSIENT );
   md.message = global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   md.dirinfo = global( LWDIRINFOFUNC_GLOBAL, GFUSE_TRANSIENT );

   return md.monf && md.query && md.surface && md.font && md.hdi && md.filereq
      && md.message && md.dirinfo;
}


ModData *csInit( GlobalFunc *global, LWModCommand *local )
{
   if ( !ginit( global )) return NULL;

   md.local = local;
   md.cmderror = CSERR_NONE;
   md.opsel = OPSEL_GLOBAL;

   return &md;
}


void csDone( void )
{
   return;
}


int csMeshBegin( int pntbufsize, int polbufsize, EltOpSelect select )
{
   assert( md.edit == NULL );
   md.edit = md.local->editBegin( pntbufsize, polbufsize, select );
   return md.edit != NULL;
}


void csMeshDone( EDError ederror, int selm )
{
   if ( md.edit ) {
      md.edit->done( md.edit->state, ederror, selm );
      md.edit = NULL;
   }
}


ModData *meInit( GlobalFunc *global, MeshEditBegin *local, int pntbufsize,
   int polbufsize, EltOpSelect select )
{
   if ( !ginit( global )) return NULL;

   assert( md.edit == NULL );
   md.edit = local( pntbufsize, polbufsize, select );

   return md.edit ? &md : NULL;
}


void meDone( EDError ederror, int selm )
{
   if ( md.edit ) {
      md.edit->done( md.edit->state, ederror, selm );
      md.edit = NULL;
   }
}


ModData *getModData( void )
{
   return &md;
}
