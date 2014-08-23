/*
======================================================================
save.c

Save an object in VideoScape ASCII .geo format.

Ernie Wright  30 Mar 00

The points and polygons are saved during pointScan() and polyScan()
callbacks, which is pretty easy.  The hard part is organizing the
surface references.  For each polygon, we need to match the surface
name with a VideoScape color code.

We first put all of the object's surfaces in an array, and for each of
them we decide which VideoScape color code to use.  The array gets
sorted by surface name.  Then, as each polygon is sent to our scan
callback, we can do a binary search on the surface name to look up the
VideoScape color code we should use for that polygon.
====================================================================== */

#include <lwserver.h>
#include <lwcmdseq.h>
#include <lwobjimp.h>
#include <lwmodeler.h>
#include <lwsurf.h>
#include <lwhost.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct {
   char          *name;          // surface name
   LWSurfaceID    id;            // surface ID
   int            vscode;        // closest VideoScape color code
} GeoSurf;

typedef struct {
   MeshEditOp    *edit;          // MeshDataEdit functions
   FILE          *fp;            // where the VideoScape object is written
   GeoSurf       *gsurf;         // array of surfaces
   int            nsurfs;        // number of surfaces in the array
   int            ptindex;       // tracks point index during point enumeration
} PSInfo;


/* from surf.c */

int get_colorcode( LWSurfaceFuncs *surff, LWSurfaceID id );


/*
======================================================================
free_geosurfs()

Free the surface array created by get_geosurfs().
====================================================================== */

static void free_geosurfs( GeoSurf *gsurf, int nsurfs )
{
   int i;

   if ( gsurf ) {
      for ( i = 0; i < nsurfs; i++ )
         if ( gsurf[ i ].name ) free( gsurf[ i ].name );
      free( gsurf );
   }
}


/*
======================================================================
sort_geosurfs()

Sort the surfaces in strcmp() order by surface name.  This prepares
the surface array for binary search.
====================================================================== */

static void sort_geosurfs( GeoSurf *gsurf, int nsurfs )
{
   int i, j;
   GeoSurf v;

   for ( i = 1; i < nsurfs; i++ ) {
      v = gsurf[ i ];
      j = i;
      while ( j && strcmp( gsurf[ j - 1 ].name, v.name ) > 0 ) {
         gsurf[ j ] = gsurf[ j - 1 ];
         --j;
      }
      gsurf[ j ] = v;
   }
}


/*
======================================================================
get_geosurfs()

Create an array of the object's surfaces.

A VideoScape color code is associated with each surface as the array
is filled in.  The color code is selected by the get_colorcode()
function in surf.c.

The surface entries are sorted in strcmp() order by surface name to
facilitate binary search.  Later we'll have to look up the surface by
name for every polygon in the object.
====================================================================== */

static GeoSurf *get_geosurfs( GlobalFunc *global, int *nsurfs )
{
   LWStateQueryFuncs *query;
   LWSurfaceFuncs *surff;
   LWSurfaceID *surfid, id;
   GeoSurf *gsurf;
   const char *name, *objname;
   int i, n;


   /* get some globals */

   query = global( LWSTATEQUERYFUNCS_GLOBAL, GFUSE_TRANSIENT );
   surff = global( LWSURFACEFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !query || !surff ) return NULL;

   /* get the name of the object we're saving */

   objname = query->object();

   /* get and count the surfaces used by this object */

   if ( objname ) {
      surfid = surff->byObject( objname );
      if ( !surfid ) return NULL;
      for ( n = 0; surfid[ n ]; n++ ) ;
      if ( n == 0 ) return NULL;
   }
   else {

      /* At the time of this writing, Modeler has a problem:  There's no
         way to refer to an object unless it has a filename, i.e. unless
         it's already been saved as a LW object.  If the object name is
         NULL, we can't ask for its surfaces.  The best we can do is ask
         for *all* of the surfaces and hope the surface names used by our
         object are unique. */

      n = 0;
      id = surff->first();
      while ( id ) {
         n++;
         id = surff->next( id );
      }
      if ( n == 0 ) return NULL;
      surfid = calloc( n + 1, sizeof( LWSurfaceID ));
      if ( !surfid ) return NULL;
      n = 0;
      surfid[ 0 ] = surff->first();
      while ( surfid[ n ] ) {
         n++;
         surfid[ n ] = surff->next( surfid[ n - 1 ]);
      }
   }

   /* allocate and initialize the GeoSurf array */

   gsurf = calloc( n, sizeof( GeoSurf ));
   if ( !gsurf ) return NULL;

   for ( i = 0; i < n; i++ ) {
      name = surff->name( surfid[ i ] );
      gsurf[ i ].name = malloc( strlen( name ) + 1 );
      if ( !gsurf[ i ].name ) {
         free_geosurfs( gsurf, n );
         return NULL;
      }
      strcpy( gsurf[ i ].name, name );
      gsurf[ i ].vscode = get_colorcode( surff, surfid[ i ] );
   }

   if ( surfid && !objname ) free( surfid );

   /* sort the surfaces */

   sort_geosurfs( gsurf, n );

   /* done */

   *nsurfs = n;
   return gsurf;
}


/*
======================================================================
search_geosurfs()

Find the surface entry for a given surface name and return the surface
array index.  This is a binary search.
====================================================================== */

static int search_geosurfs( const char *name, GeoSurf *gsurf, int nsurfs )
{
   int lt = 0, rt = nsurfs - 1, i, c;

   while ( rt >= lt ) {
      i = ( lt + rt ) / 2;
      c = strcmp( name, gsurf[ i ].name );
      if ( c == 0 ) return i;
      if ( c < 0 ) rt = i - 1; else lt = i + 1;
   }
   return -1;
}


/*
======================================================================
write_point()

Point scan callback, writes a point to the .geo file.

This gets called by Modeler for each point that matches the selection
criteria for the scan.

point->userData is a 4-byte buffer allocated through the local() call
in geoSave().  Points are numbered sequentially by storing an index
in userData here and retrieving it in write_polygon().

As in LightWave, VideoScape's coordinate system is left-handed, with
Y pointing upward.
====================================================================== */

static EDError write_point( PSInfo *psi, const EDPointInfo *point )
{
   fprintf( psi->fp, "%g %g %g\n",
      point->position[ 0 ],
      point->position[ 1 ],
      point->position[ 2 ] );

   *(( int * )( point->userData )) = psi->ptindex++;

   return EDERR_NONE;
}


/*
======================================================================
write_polygon()

Polygon scan callback, writes a polygon to the .geo file.

This is called by Modeler for each polygon that matches the selection
criteria for the scan.

VideoScape .geo polygons are defined by a vertex count, a point index
list, and a color code.  Each point index refers to a position in the
file's point list.  We can look up this position using the PntIDs in
the PolygonInfo because we recorded it in the PointInfo->userData for
each point during the point scan.

The color code is a best match against the LightWave surface assigned
to the polygon.  The surface is given by name in the PolygonInfo, and
the color code is retrieved from the corresponding entry in the array
of surfaces created by get_geosurfs().
====================================================================== */

static EDError write_polygon( PSInfo *psi, const EDPolygonInfo *polygon )
{
   EDPointInfo *point;
   int i;

   if ( polygon->type != LWPOLTYPE_FACE ) return EDERR_NONE;

   fprintf( psi->fp, "%d ", polygon->numPnts );
   for ( i = 0; i < polygon->numPnts; i++ ) {
      point = psi->edit->pointInfo( psi->edit->state, polygon->points[ i ] );
      fprintf( psi->fp, "%d ", *(( int * )( point->userData )));
   }

   i = search_geosurfs( polygon->surface, psi->gsurf, psi->nsurfs );
   fprintf( psi->fp, "%d\n", i > -1 ? psi->gsurf[ i ].vscode : 15 );

   return EDERR_NONE;
}


/*
======================================================================
geoSave()

Activation function for the VideoScape object saver.

The first argument to local() requests a temporary 4-byte buffer for
each point to be saved.  This extra space is used to assign an index
that will refer to the point in the polygon definition.  The points
are numbered sequentially, starting at 0, during the point scan, and
the numbers are retrieved during the polygon scan.
====================================================================== */

XCALL_( int )
geoSave( long version, GlobalFunc *global, LWModCommand *local,
   void *serverData )
{
   static char name[ 256 ], path[ 256 ], node[ 256 ];
   LWFileActivateFunc *filereq;
   LWFileReqLocal frloc;
   PSInfo psi;
   int nvert, nface, result;


   if ( version != LWMODCOMMAND_VERSION ) return AFUNC_BADVERSION;

   if ( local->argument[ 0 ] )
      strcpy( name, local->argument );
   else {

      /* get the file request global */

      filereq = global( LWFILEACTIVATEFUNC_GLOBAL, GFUSE_TRANSIENT );
      if ( !filereq ) return AFUNC_BADGLOBAL;

      /* display a file dialog, return if the user cancels */

      frloc.reqType  = FREQ_SAVE;
      frloc.title    = "Save As VideoScape Object";
      frloc.fileType = "Objects";
      frloc.path     = path;
      frloc.baseName = node;
      frloc.fullName = name;
      frloc.bufLen   = sizeof( name );
      frloc.pickName = NULL;

      result = filereq( LWFILEREQ_VERSION, &frloc );

      if (( result != AFUNC_OK ) || !frloc.result ) return AFUNC_OK;
   }

   /* begin the mesh edit */

   psi.edit = local->editBegin( sizeof( int ), 0, OPSEL_GLOBAL );
   if ( !psi.edit ) return AFUNC_BADLOCAL;

   /* VideoScape objects are limited to 32767 points and polygons */

   nvert = psi.edit->pointCount( psi.edit->state, OPLYR_FG, EDCOUNT_ALL );
   nface = psi.edit->polyCount( psi.edit->state, OPLYR_FG, EDCOUNT_ALL );
   if ( nvert < 1 || nvert > 32767 || nface < 1 || nface > 32767 ) {
      psi.edit->done( psi.edit->state, EDERR_BADARGS, 0 );
      return AFUNC_OK;
   }

   /* get Modeler's surface list and match it with VideoScape color codes */

   psi.gsurf = get_geosurfs( global, &psi.nsurfs );
   if ( !psi.gsurf ) {
      psi.edit->done( psi.edit->state, EDERR_BADSURF, 0 );
      return AFUNC_OK;
   }

   /* write the file */

   if ( psi.fp = fopen( name, "w" )) {
      fprintf( psi.fp, "3DG1\n%d\n", nvert );
      psi.ptindex = 0;
      psi.edit->pointScan( psi.edit->state, write_point, &psi, OPLYR_FG );
      psi.edit->polyScan( psi.edit->state, write_polygon, &psi, OPLYR_FG );
      fclose( psi.fp );
   }

   /* done */

   psi.edit->done( psi.edit->state, EDERR_NONE, 0 );
   free_geosurfs( psi.gsurf, psi.nsurfs );

   return AFUNC_OK;
}


/* from load.c */

XCALL_( int ) geoLoad( long, GlobalFunc *, void *, void * );


ServerRecord ServerDesc[] = {
   { LWOBJECTIMPORT_CLASS, "GeoLoad", geoLoad },
   { LWMODCOMMAND_CLASS, "VideoScape_Export", geoSave },
   { NULL }
};
