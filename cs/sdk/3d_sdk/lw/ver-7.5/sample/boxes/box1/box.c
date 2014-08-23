/*
======================================================================
box.c

A Modeler plug-in that makes a box.

Ernie Wright  28 May 01
====================================================================== */

#include <lwserver.h>
#include <lwcmdseq.h>
#include <stdio.h>


XCALL_( int )
Activate( long version, GlobalFunc *global, LWModCommand *local,
   void *serverData )
{
   char cmd[ 128 ];

   if ( version != LWMODCOMMAND_VERSION )
      return AFUNC_BADVERSION;

   sprintf( cmd, "MAKEBOX <%g %g %g> <%g %g %g> <%d %d %d>",
      -0.5, -0.5, -0.5,  0.5, 0.5, 0.5,  1, 1, 1 );
   local->evaluate( local->data, cmd );

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWMODCOMMAND_CLASS, "Tutorial_Box1", Activate },
   { NULL }
};
