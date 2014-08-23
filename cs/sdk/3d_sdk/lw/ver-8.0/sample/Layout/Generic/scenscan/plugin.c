/*
======================================================================
plugin.c

Activation functions and callbacks for generic and displacement
handler class plug-ins.  These demonstrate the use of the object
database functions.

Ernie Wright  1 Jun 00

24 Jul 01  Added the use of GoToFrame to step through all frames from
           within a LayoutGeneric.  The Bounding Box Threshold stuff
           requires LW 7.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <lwhost.h>
#include <lwgeneric.h>
#include <lwdisplce.h>
#include "objectdb.h"
#include "vecmat.h"

#define TEST_FILENAME "testodb.txt"    /* output file for the generic */


/*
======================================================================
lwcommand()

Issue a LayoutGeneric command.  This just saves us from using a lot of
sprintf() calls in code that issues commands.  If the command doesn't
take any arguments, "fmt" is NULL.  Otherwise, it contains a printf
formatting string, and the arguments that follow it are printf
arguments.
====================================================================== */

static int lwcommand( LWLayoutGeneric *gen, const char *cmdname,
   const char *fmt, ... )
{
   static char cmd[ 256 ], arg[ 256 ];

   if ( fmt ) {
      va_list ap;
      va_start( ap, fmt );
      vsprintf( arg, fmt, ap );
      va_end( ap );
      sprintf( cmd, "%s %s", cmdname, arg );
      return gen->evaluate( gen->data, cmd );
   }
   else
      return gen->evaluate( gen->data, cmdname );
}


/*
======================================================================
ODBTest()

Activation function for the generic.  This creates and prints an
object database for every object in the scene.  It uses the GoToFrame
command to step through each frame and get the final point positions.
====================================================================== */

XCALL_( int )
ODBTest( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   LWMessageFuncs *msg;
   LWItemInfo *iteminfo;
   LWSceneInfo *scninfo;
   LWObjectInfo *objinfo;
   LWInterfaceInfo *ui;
   LWMeshInfo *mesh;
   LWItemID objid;
   ObjectDB *odb;
   FILE *fp;
   int i, bbt, bbt0;


   /* get some globals */

   msg = global( LWMESSAGEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   iteminfo = global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT );
   scninfo = global( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
   objinfo = global( LWOBJECTINFO_GLOBAL, GFUSE_TRANSIENT );
   ui = global( LWINTERFACEINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !msg || !iteminfo || !scninfo || !objinfo || !ui )
      return AFUNC_BADGLOBAL;

   /* make sure there are objects in the scene */

   if ( scninfo->numPoints <= 0 ) {
      msg->error( "No objects in the scene.", NULL );
      return AFUNC_OK;
   }

   /* open the output file */

   fp = fopen( TEST_FILENAME, "w" );
   if ( !fp ) {
      msg->error( "Couldn't open output file", TEST_FILENAME );
      return AFUNC_OK;
   }

   /* we probably don't need this */

   lwcommand( local, "EditObjects", NULL );

   /* We need to make sure the Bounding Box Threshold is set high enough
      to force Layout to transform all of the points of every object at
      each frame.  If we need to set it here, we'll restore it later. */

   bbt0 = ui->boxThreshold;

   objid = iteminfo->first( LWI_OBJECT, 0 );
   bbt = 0;
   while ( objid ) {
      mesh = objinfo->meshInfo( objid, 1 );
      if ( mesh ) {
         i = mesh->numPoints( mesh );
         if ( i > bbt ) bbt = i;
         if ( mesh->destroy )
            mesh->destroy( mesh );
      }
      objid = iteminfo->next( objid );
   }
   ++bbt;
   if ( bbt > bbt0 )
      lwcommand( local, "BoundingBoxThreshold", "%d", bbt );

   /* for each frame */

   for ( i = scninfo->frameStart; i <= scninfo->frameEnd; i++ ) {
      lwcommand( local, "GoToFrame", "%d", i );
      lwcommand( local, "RefreshNow", NULL );

      fprintf( fp, "\nFrame %d\n", i );

      objid = iteminfo->first( LWI_OBJECT, 0 );

      /* for each object */

      while ( objid ) {

         /* skip nulls */

         if ( 0 >= objinfo->numPolygons( objid )) {
            objid = iteminfo->next( objid );
            continue;
         }

         /* get object data */

         odb = getObjectDB( objid, global );
         if ( !odb ) {
            msg->error( "Couldn't alloc object database for",
               iteminfo->name( objid ));
            return AFUNC_OK;
         }

         /* Do something with the object data.  We could call printObjectDB(),
            but to keep the file small, we'll just print the final position of
            the first point. */

         fprintf( fp, "%s", iteminfo->name( objid ));
         if ( i == scninfo->frameStart ) {
            fprintf( fp, "\n%d points, %d polygons, %d surfaces, %d vmaps\n",
               odb->npoints, odb->npolygons, odb->nsurfaces, odb->nvertmaps );
            fprintf( fp, "first point:  (%g, %g, %g)",
               odb->pt[ 0 ].pos[ 0 ][ 0 ],
               odb->pt[ 0 ].pos[ 0 ][ 1 ],
               odb->pt[ 0 ].pos[ 0 ][ 2 ] );
         }
         fprintf( fp, "  (%g, %g, %g)\n",
            odb->pt[ 0 ].pos[ 1 ][ 0 ],
            odb->pt[ 0 ].pos[ 1 ][ 1 ],
            odb->pt[ 0 ].pos[ 1 ][ 2 ] );

         freeObjectDB( odb );
         objid = iteminfo->next( objid );
      }
   }

   fclose( fp );

   /* restore the user's Bounding Box Threshold setting */

   if ( bbt > bbt0 )
      lwcommand( local, "BoundingBoxThreshold", "%d", bbt0 );

   return AFUNC_OK;
}


/* stuff for the displacement handler */

typedef struct {
   GlobalFunc *global;
   LWItemID    objid;
   char        name[ 80 ];
   ObjectDB   *odb;
   int         index;
   LWFrame     frame;
} MyData;


/*
======================================================================
Create()

DisplacementHandler callback.  Allocate and initialize instance data.
====================================================================== */

XCALL_( static LWInstance )
Create( void *priv, LWItemID item, LWError *err )
{
   MyData *dat;
   LWItemInfo *iteminfo;

   if ( dat = calloc( 1, sizeof( MyData ))) {
      dat->objid = item;
      dat->global = ( GlobalFunc * ) priv;
      if ( iteminfo = dat->global( LWITEMINFO_GLOBAL, GFUSE_TRANSIENT ))
         strcpy( dat->name, iteminfo->name( item ));
   }

   return dat;
}


/*
======================================================================
Destroy()

Handler callback.  Free the instance.
====================================================================== */

XCALL_( static void )
Destroy( MyData *dat )
{
   if( dat ) {
      if ( dat->odb ) freeObjectDB( dat->odb );
      free( dat );
   }
}


/*
======================================================================
Copy()

Handler callback.  Copy instance data.
====================================================================== */

XCALL_( static LWError )
Copy( MyData *to, MyData *from )
{
   to->objid = from->objid;
   return NULL;
}


/*
======================================================================
Describe()

Handler callback.  Write a short, human-readable string describing
the instance data.
====================================================================== */

XCALL_( static const char * )
Describe( MyData *dat )
{
   return "Object Database Demo";
}


/*
======================================================================
ChangeID()

Handler callback.  An item ID has changed.
====================================================================== */

XCALL_( void )
ChangeID( MyData *dat, const LWItemID *id )
{
   int i;

   for ( i = 0; id[ i ]; i += 2 )
      if ( id[ i ] == dat->objid ) {
         dat->objid = id[ i + 1 ];
         break;
      }
}


/*
======================================================================
Init()

Handler callback, called at the start of rendering.
====================================================================== */

XCALL_( static LWError )
Init( MyData *dat, int mode )
{
   return NULL;
}


/*
======================================================================
Cleanup()

Handler callback, called at the end of rendering.
====================================================================== */

XCALL_( static void )
Cleanup( MyData *dat )
{
   if ( dat->odb ) {
      freeObjectDB( dat->odb );
      dat->odb = NULL;
   }
   return;
}


/*
======================================================================
NewTime()

Handler callback, called at the start of each sampling pass.  The
world coordinate positions of the points haven't been calculated yet,
so we have to wait until Evaluate() to get those.  If we allocated an
ObjectDB in a previous Evaluate(), we free it here.
====================================================================== */

XCALL_( static LWError )
NewTime( MyData *dat, LWFrame fr, LWTime t )
{
   if ( dat->odb ) {
      freeObjectDB( dat->odb );
      dat->odb = NULL;
   }

   dat->odb = getObjectDB( dat->objid, dat->global );
   if ( dat->odb ) {
      freeObjectDB( dat->odb );
      dat->odb = NULL;
   }

   dat->index = 0;
   dat->frame = fr;
   return NULL;
}


/*
======================================================================
Flags()

Handler callback.
====================================================================== */

XCALL_( static int )
Flags( MyData *dat )
{
   return LWDMF_WORLD;
}


/*
======================================================================
Evaluate()

Handler callback.  Called for each point in the object.

If this is the first call since newTime() was called, we allocate an
ObjectDB.  Then we look up the point and store its current position.
====================================================================== */

XCALL_( static void )
Evaluate( MyData *dat, LWDisplacementAccess *da )
{
   int i;

   if ( dat->index == 0 ) {
      dat->odb = getObjectDB( dat->objid, dat->global );

      if ( dat->odb ) {
         FILE *fp;
         char buf[ 80 ];
         int i, len;

         sprintf( buf, "%s%03d.txt", dat->name, dat->frame );
         len = strlen( buf );
         for ( i = 0; i < len; i++ )
            if ( buf[ i ] == ':' ) buf[ i ] = '_';
         if ( fp = fopen( buf, "w" )) {
            printObjectDB( dat->odb, fp, 0 );
            fclose( fp );
         }
      }
   }

   ++dat->index;

   if ( dat->odb ) {
      i = findVert( dat->odb, da->point );
      if ( i != -1 ) {
         dat->odb->pt[ i ].pos[ 1 ][ 0 ] = ( float ) da->source[ 0 ];
         dat->odb->pt[ i ].pos[ 1 ][ 1 ] = ( float ) da->source[ 1 ];
         dat->odb->pt[ i ].pos[ 1 ][ 2 ] = ( float ) da->source[ 2 ];
      }
   }
}


/*
======================================================================
ODBDisplace()

Handler activation function.
====================================================================== */

XCALL_( static int )
ODBDisplace( long version, GlobalFunc *global, LWDisplacementHandler *local,
   void *serverData)
{
   if ( version != LWDISPLACEMENT_VERSION ) return AFUNC_BADVERSION;

   local->inst->priv     = global;
   local->inst->create   = Create;
   local->inst->destroy  = Destroy;
   local->inst->copy     = Copy;
   local->inst->descln   = Describe;
   local->item->changeID = ChangeID;
   local->rend->init     = Init;
   local->rend->cleanup  = Cleanup;
   local->rend->newTime  = NewTime;
   local->evaluate       = Evaluate;
   local->flags          = Flags;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWLAYOUTGENERIC_CLASS, "ODBTest", ODBTest },
   { LWDISPLACEMENT_HCLASS, "Demo_Object_Database", ODBDisplace },
   { NULL }
};
