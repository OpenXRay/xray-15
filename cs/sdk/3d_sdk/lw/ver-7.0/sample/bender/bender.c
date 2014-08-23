/*
======================================================================
bender.c

Demo interactive mesh-edit tool.  Eventually this could be something
really useful, but for now it only demonstrates some of the handler
interaction methods.

Copyright 1999, NewTek, Inc.
written by Stuart Ferguson
last revision  8/30/99

[EW] cosmetic changes  Dec 99
====================================================================== */

#include <lwserver.h>
#include <lwmodtool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


typedef struct st_BenderTool {
    LWFVector v[ 2 ];
    int       active;
    int       dirty;
} BenderTool;


XCALL_( static void )
Draw( BenderTool *tool, LWWireDrawAccess *draw )
{
   if ( !tool->active ) return;

   draw->moveTo( draw->data, tool->v[ 0 ], 0 );
   draw->lineTo( draw->data, tool->v[ 1 ], 0 );
   tool->dirty = 0;
}


XCALL_( static int )
Dirty( BenderTool *tool )
{
   return tool->dirty ? LWT_DIRTY_WIREFRAME : 0;
}


XCALL_( static int )
Count( BenderTool *tool, LWToolEvent *event )
{
   return tool->active ? 2 : 0;
}


XCALL_( static int )
Handle( BenderTool *tool, LWToolEvent *event, int i, LWDVector pos )
{
   pos[ 0 ] = tool->v[ i ][ 0 ];
   pos[ 1 ] = tool->v[ i ][ 1 ];
   pos[ 2 ] = tool->v[ i ][ 2 ];
   return 1;
}


XCALL_( static int )
Adjust( BenderTool *tool, LWToolEvent *event, int i )
{
   tool->v[ i ][ 0 ] = event->posSnap[ 0 ];
   tool->v[ i ][ 1 ] = event->posSnap[ 1 ];
   tool->v[ i ][ 2 ] = event->posSnap[ 2 ];
   tool->dirty = 1;
   return i;
}


XCALL_( static void )
Destroy( BenderTool *tool )
{
   free( tool );
}


/*
======================================================================
Activate()

Activation function.  Check the version, then fill in the fields of
the local structure.
====================================================================== */

XCALL_( int )
Activate( long version, GlobalFunc *global, LWMeshEditTool *local,
   void *serverData )
{
   BenderTool *tool;

   if ( version != LWMESHEDITTOOL_VERSION )
      return AFUNC_BADVERSION;

   tool = calloc( 1, sizeof( BenderTool ));
   if ( !tool ) return AFUNC_OK;

   tool->active = 1;
   tool->v[ 0 ][ 0 ] = -1.0;
   tool->v[ 0 ][ 1 ] = -1.0;
   tool->v[ 0 ][ 2 ] = -1.0;
   tool->v[ 1 ][ 0 ] =  1.0;
   tool->v[ 1 ][ 1 ] =  1.0;
   tool->v[ 1 ][ 2 ] =  1.0;

   local->instance     = tool;

   local->tool->done   = Destroy;
   local->tool->draw   = Draw;
   local->tool->count  = Count;
   local->tool->handle = Handle;
   local->tool->adjust = Adjust;
   local->tool->dirty  = Dirty;

   return AFUNC_OK;
}


/*
======================================================================
Server description.  The convention is to stick this at the end of the
source file containing the activation functions.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWMESHEDITTOOL_CLASS, "LW_BenderTool", Activate },
   { NULL }
};
