/*
======================================================================
objectdb.c

Functions for creating an object database.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "objectdb.h"
#include "vecmat.h"


/*
======================================================================
pntScan()

Point scan callback.  Insert the point ID into the point array in
ascending numerical order.  This takes only linear time if point IDs
arrive in order.
====================================================================== */

static int pntScan( ObjectDB *odb, LWPntID id )
{
   int j;

   j = odb->npoints;

   if ( j == 0 ) {
      odb->pt[ 0 ].id = id;
      ++odb->npoints;
      return 0;
   }

   while ( odb->pt[ j - 1 ].id > id ) {
      odb->pt[ j ].id = odb->pt[ j - 1 ].id;
      --j;
      if ( j == 0 ) break;
   }

   odb->pt[ j ].id = id;
   ++odb->npoints;

   return 0;
}


/*
======================================================================
polScan()

Polygon scan callback.  Just store the ID.
====================================================================== */

static int polScan( ObjectDB *odb, LWPolID id )
{
   odb->pol[ odb->npolygons ].id = id;
   ++odb->npolygons;
   return 0;
}


/*
======================================================================
findVert()

Binary search the point array and return the array index for the given
point ID.
====================================================================== */

int findVert( ObjectDB *odb, LWPntID id )
{
   int lt = 0, rt = odb->npoints - 1, x;

   while ( rt >= lt ) {
      x = ( lt + rt ) / 2;
      if ( id < odb->pt[ x ].id ) rt = x - 1; else lt = x + 1;
      if ( id == odb->pt[ x ].id ) return x;
   }
   return -1;
}


/*
======================================================================
getPolyNormals()

Calculate the polygon normals.  By convention, LW's polygon normals
are based on the first, second and last points in the vertex list.
The normal is the cross product of two vectors formed from these
points.  It's undefined for one- and two-point polygons.
====================================================================== */

void getPolyNormals( ObjectDB *odb, int i )
{
   int j, k;
   LWFVector p1, p2, pn, v1, v2;

   for ( j = 0; j < odb->npolygons; j++ ) {
      if ( odb->pol[ j ].nverts < 3 ) continue;
      for ( k = 0; k < 3; k++ ) {
         p1[ k ] = odb->pt[ odb->pol[ j ].v[ 0 ].index ].pos[ i ][ k ];
         p2[ k ] = odb->pt[ odb->pol[ j ].v[ 1 ].index ].pos[ i ][ k ];
         pn[ k ] = odb->pt[ odb->pol[ j ].v[
            odb->pol[ j ].nverts - 1 ].index ].pos[ i ][ k ];
      }

      for ( k = 0; k < 3; k++ ) {
         v1[ k ] = p2[ k ] - p1[ k ];
         v2[ k ] = pn[ k ] - p1[ k ];
      }

      cross( v1, v2, odb->pol[ j ].norm[ i ] );
      normalize( odb->pol[ j ].norm[ i ] );
   }
}


/*
======================================================================
getVertNormals()

Calculate the vertex normals.  For each polygon vertex, sum the
normals of the polygons that share the point.  If the normals of the
current and adjacent polygons form an angle greater than the max
smoothing angle for the current polygon's surface, the normal of the
adjacent polygon is excluded from the sum.
====================================================================== */

void getVertNormals( ObjectDB *odb, int i )
{
   int j, k, n, g, h, p;
   float a;

   for ( j = 0; j < odb->npolygons; j++ ) {
      for ( n = 0; n < odb->pol[ j ].nverts; n++ ) {
         for ( k = 0; k < 3; k++ )
            odb->pol[ j ].v[ n ].norm[ i ][ k ]
               = odb->pol[ j ].norm[ i ][ k ];

         if ( odb->surf[ odb->pol[ j ].sindex ].sman <= 0 ) continue;

         p = odb->pol[ j ].v[ n ].index;

         for ( g = 0; g < odb->pt[ p ].npols; g++ ) {
            h = odb->pt[ p ].pol[ g ];
            if ( h == j ) continue;

            a = vecangle( odb->pol[ j ].norm[ i ], odb->pol[ h ].norm[ i ] );
            if ( a > odb->surf[ odb->pol[ j ].sindex ].sman ) continue;

            for ( k = 0; k < 3; k++ )
               odb->pol[ j ].v[ n ].norm[ i ][ k ]
                  += odb->pol[ h ].norm[ i ][ k ];
         }

         normalize( odb->pol[ j ].v[ n ].norm[ i ] );
      }
   }
}


/*
======================================================================
freeObjectDB()

Free an ObjectDB created by getObjectDB().
====================================================================== */

void freeObjectDB( ObjectDB *odb )
{
   int i;

   if ( odb ) {
      freeObjectVMaps( odb );
      freeObjectSurfs( odb );
      freePointSearch( odb );

      if ( odb->pt ) {
         for ( i = 0; i < odb->npoints; i++ )
            if ( odb->pt[ i ].pol )
               free( odb->pt[ i ].pol );
         free( odb->pt );
      }
      if ( odb->pol ) {
         for ( i = 0; i < odb->npolygons; i++ )
            if ( odb->pol[ i ].v )
               free( odb->pol[ i ].v );
         free( odb->pol );
      }
      free( odb );
   }
}


/*
======================================================================
getObjectDB()

Create an ObjectDB for an object.
====================================================================== */

ObjectDB *getObjectDB( LWItemID id, GlobalFunc *global )
{
   LWObjectInfo *objinfo;
   LWMeshInfo *mesh;
   LWPntID ptid;
   ObjectDB *odb;
   const char *name;
   int npts, npols, nverts, i, j, k, ok = 0;

   /* get the object info global */

   objinfo = global( LWOBJECTINFO_GLOBAL, GFUSE_TRANSIENT );
   if ( !objinfo ) return NULL;

   /* get the mesh info for the object */

   mesh = objinfo->meshInfo( id, 1 );
   if ( !mesh ) return NULL;

   /* alloc the object database */

   odb = calloc( 1, sizeof( ObjectDB ));
   if ( !odb ) goto Finish;

   odb->id = id;
   name = objinfo->filename( id );
   odb->filename = malloc( strlen( name ) + 1 );
   if ( !odb->filename ) goto Finish;
   strcpy( odb->filename, name );

   /* alloc and init the points array */

   npts = mesh->numPoints( mesh );
   odb->pt = calloc( npts, sizeof( DBPoint ));
   if ( !odb->pt ) goto Finish;

   if ( mesh->scanPoints( mesh, pntScan, odb ))
      goto Finish;

   /* alloc and init the polygons array */

   npols = mesh->numPolygons( mesh );
   odb->pol = calloc( npols, sizeof( DBPolygon ));
   if ( !odb->pol ) goto Finish;

   if ( mesh->scanPolys( mesh, polScan, odb ))
      goto Finish;

   /* get the vertices of each polygon */

   for ( i = 0; i < npols; i++ ) {
      nverts = mesh->polSize( mesh, odb->pol[ i ].id );
      odb->pol[ i ].v = calloc( nverts, sizeof( DBPolVert ));
      if ( !odb->pol[ i ].v ) goto Finish;
      odb->pol[ i ].nverts = nverts;
      for ( j = 0; j < nverts; j++ ) {
         ptid = mesh->polVertex( mesh, odb->pol[ i ].id, j );
         odb->pol[ i ].v[ j ].index = findVert( odb, ptid );
      }
   }

   /* count the number of polygons per point */

   for ( i = 0; i < npols; i++ )
      for ( j = 0; j < odb->pol[ i ].nverts; j++ )
         ++odb->pt[ odb->pol[ i ].v[ j ].index ].npols;

   /* alloc per-point polygon arrays */

   for ( i = 0; i < npts; i++ ) {
      if ( odb->pt[ i ].npols == 0 ) continue;
      odb->pt[ i ].pol = calloc( odb->pt[ i ].npols, sizeof( int ));
      if ( !odb->pt[ i ].pol ) goto Finish;
      odb->pt[ i ].npols = 0;
   }

   /* fill in polygon array for each point */

   for ( i = 0; i < npols; i++ ) {
      for ( j = 0; j < odb->pol[ i ].nverts; j++ ) {
         k = odb->pol[ i ].v[ j ].index;
         odb->pt[ k ].pol[ odb->pt[ k ].npols ] = i;
         ++odb->pt[ k ].npols;
      }
   }

   /* get the position of each point */

   for ( i = 0; i < npts; i++ ) {
      mesh->pntBasePos( mesh, odb->pt[ i ].id, odb->pt[ i ].pos[ 0 ] );
      mesh->pntOtherPos( mesh, odb->pt[ i ].id, odb->pt[ i ].pos[ 1 ] );
   }

   /* init the point search array */

   /* Ordinarily, you won't need this because you can look up points
      by their IDs.  Uncomment this if you do need to search by point
      position instead.  The second argument is 0 for base position
      and 1 for final position.

      if ( !initPointSearch( odb, 0 ))
         goto Finish;
   */

   /* calculate the normal of each polygon */

   getPolyNormals( odb, 0 );
   getPolyNormals( odb, 1 );

   /* get the vmaps */

   if ( !getObjectVMaps( odb, mesh, global ))
      goto Finish;

   /* get the surfaces */

   if ( !getObjectSurfs( odb, mesh, global ))
      goto Finish;

   /* calculate vertex normals */

   getVertNormals( odb, 0 );
   getVertNormals( odb, 1 );

   /* done */

   ok = 1;

Finish:
   if ( mesh->destroy )
      mesh->destroy( mesh );

   if ( !ok ) {
      freeObjectDB( odb );
      return NULL;
   }

   return odb;
}
