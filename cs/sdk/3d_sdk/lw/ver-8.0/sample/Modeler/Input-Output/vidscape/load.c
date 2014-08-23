/*
======================================================================
load.c

Load an object in VideoScape ASCII .geo format.

Ernie Wright  30 Mar 00
====================================================================== */

#include <lwserver.h>
#include <lwobjimp.h>
#include <lwsurf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


typedef struct {
   int   total;
   int   stepsize;
   int   nextstep;
} Progress;


/* from surf.c */

int get_surf( int index, char *name, unsigned char *buf, int bufsize );


/*
======================================================================
read_int()
read_float()

Read numeric values from a text file.
====================================================================== */

static int read_int( FILE *fp, int *val )
{
   int n;

   n = fscanf( fp, "%d", val );
   return ( n == 1 );
}


static int read_float( FILE *fp, float *val )
{
   double d;
   int n;

   n = fscanf( fp, "%lf", &d );
   *val = ( float ) d;
   return ( n == 1 );
}


/*
======================================================================
read_polygon()

Read a polygon from a VideoScape ASCII object file.

In the file, polygon vertices are recorded as indexes into the point
list.  We use the pntID array to convert from indexes to point IDs,
which is what local->polygon() wants in the vertex list.  The pntID
array was created by geoLoad() as it read the points and submitted
them to Modeler.

After all the polygons have been read and created, we'll need to send
Modeler a description of each of the surfaces used by the object.  We
keep track of which surfaces are used by incrementing the appropriate
element of the isurf array each time we read a polygon.

Detail polygons are...well, you don't want to know.  They're loaded as
ordinary polygons here.  Because of the way they're recorded in the
.geo file, this function calls itself recursively to load them.
====================================================================== */

static int read_polygon( LWObjectImport *local, FILE *fp, LWPntID *pntID,
   int *isurf, int isDetail, Progress *progress )
{
   static int vert[ 200 ];
   static LWPntID vID[ 200 ];
   LWPolID polID;
   int vscolor, nvert, ndet, i;
   char tag[ 40 ], *err = NULL;


   /* read the vertex count */

   if ( !read_int( fp, &nvert ))
      if ( feof( fp )) return 0;  else goto Fail;

   err = "Bad vertex count.";
   if ( nvert <= 0 || nvert > 200 ) goto Fail;

   /* read the vertices--these index the point list */

   err = "Couldn't read polygon.";

   for ( i = 0; i < nvert; i++ )
      if ( !read_int( fp, &vert[ i ] )) goto Fail;

   /* read the VideoScape color code */

   if ( !read_int( fp, &vscolor )) goto Fail;
   ++isurf[ vscolor ];

   /* get the surface name for this color code */

   get_surf( vscolor, tag, NULL, 0 );

   /* create the polygon */

   for ( i = 0; i < nvert; i++ )
      vID[ i ] = pntID[ vert[ i ]];

   polID = local->polygon( local->data, LWPOLTYPE_FACE, 0, nvert, vID );
   local->polTag( local->data, polID, LWPTAG_SURF, tag );

   /* update the progress monitor */

   i = ftell( fp );
   if ( i >= progress->nextstep ) {
      if ( MON_STEP( local->monitor )) {
         local->result = LWOBJIM_ABORTED;
         return 0;
      }
      progress->nextstep += progress->stepsize;
   }

   /* if this polygon has no detail polygons, we're done */

   if ( vscolor >= 0 ) return 1;

   /* read the detail polygon count--details can't have details */

   if ( isDetail || !read_int( fp, &ndet )) goto Fail;

   /* read the detail polygons recursively */

   for ( i = 0; i < ndet; i++ )
      if ( !read_polygon( local, fp, pntID, isurf, 1, progress ))
         if ( local->result == LWOBJIM_ABORTED ) return 0;
         else goto Fail;

   return 1;

Fail:
   local->result = LWOBJIM_FAILED;
   if ( !err )
      err = "Unexpected end of file.";
   if ( local->failedLen > 0 )
      strncpy( local->failedBuf, err, local->failedLen );

   return 0;
}


/*
======================================================================
create_surfs()

Give Modeler the surface parameters for the object's surfaces.
====================================================================== */

static void create_surfs( LWObjectImport *local, int *isurf )
{
   unsigned char data[ 256 ];
   char name[ 40 ];
   int i, size;

   for ( i = 0; i < 261; i++ ) {
      if ( isurf[ i ] > 0 ) {
         size = get_surf( i, name, data, 256 );
         if ( size > 256 ) return;
         local->surface( local->data, name, NULL, size, data );
      }
   }
}


/*
======================================================================
geoLoad()

Activation function for the loader.
====================================================================== */

XCALL_( int )
geoLoad( long version, GlobalFunc *global, LWObjectImport *local,
   void *serverData )
{
   static int isurf[ 261 ];
   LWFVector pos = { 0.0f };
   LWPntID *pntID = NULL;
   Progress progress;
   FILE *fp = NULL;
   struct stat s;
   char str[ 6 ], *err = NULL;
   int npts;
   int i, j;


   /* check the activation version */

   if ( version != LWOBJECTIMPORT_VERSION ) return AFUNC_BADVERSION;

   /* get the file size */

   if ( stat( local->filename, &s )) {
      local->result = LWOBJIM_BADFILE;
      return AFUNC_OK;
   }

   progress.total = s.st_size;
   progress.stepsize = progress.total / 20;
   progress.nextstep = progress.stepsize;

   /* attempt to open the file */

   fp = fopen( local->filename, "r" );
   if ( !fp ) {
      local->result = LWOBJIM_BADFILE;
      return AFUNC_OK;
   }

   /* see whether this is a VideoScape ASCII object file;
      if not, let someone else try to load it */

   fread( str, 1, 4, fp );
   if ( strncmp( str, "3DG1", 4 )) {
      fclose( fp );
      local->result = LWOBJIM_NOREC;
      return AFUNC_OK;
   }

   /* initialize the layer */

   local->layer( local->data, 1, NULL );
   local->pivot( local->data, pos );

   /* read the point count */

   local->result = LWOBJIM_FAILED;   /* assume this until we succeed */

   err = "Bad point count.";
   if ( !read_int( fp, &npts )) goto Finish;
   if ( npts <= 0 ) goto Finish;

   /* allocate space to store point IDs */

   err = "Couldn't allocate memory for points.";
   pntID = calloc( npts, sizeof( LWPntID ));
   if ( !pntID ) goto Finish;

   /* initialize the progress monitor */

   MON_INIT( local->monitor, 20 );

   /* read the point list */

   for ( i = 0; i < npts; i++ ) {
      for ( j = 0; j < 3; j++ )
         if ( !read_float( fp, &pos[ j ] )) {
            err = "Couldn't read point.";
            goto Finish;
         }

      if ( !( pntID[ i ] = local->point( local->data, pos ))) {
         err = "Couldn't create point.";
         goto Finish;
      }

      j = ftell( fp );
      if ( j >= progress.nextstep ) {
         if ( MON_STEP( local->monitor )) {
            local->result = LWOBJIM_ABORTED;
            err = NULL;
            goto Finish;
         }
         progress.nextstep += progress.stepsize;
      }
   }

   /* read the polygon list; read_polygon() will set its own failure
      status and message */

   err = NULL;
   local->result = LWOBJIM_OK;
   memset( isurf, 0, 261 * sizeof( int ));

   while ( read_polygon( local, fp, pntID, isurf, 0, &progress )) ;

   /* create the surface descriptions */

   if ( local->result == LWOBJIM_OK )
      create_surfs( local, isurf );

   /* we're done */

   MON_DONE( local->monitor );
   local->done( local->data );

Finish:
   if ( fp ) fclose( fp );
   if ( pntID ) free( pntID );
   if (( local->result != LWOBJIM_OK ) && err && ( local->failedLen > 0 ))
      strncpy( local->failedBuf, err, local->failedLen );

   return AFUNC_OK;
}
