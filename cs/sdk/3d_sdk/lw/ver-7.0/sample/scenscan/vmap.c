/*
======================================================================
vmap.c

Functions for examining vertex maps.
====================================================================== */

#include <stdlib.h>
#include <string.h>
#include "scenscan.h"
#include "objectdb.h"


/*
======================================================================
freeVertMapDB()

Frees the VertMapDB created by getVertMapDB().
====================================================================== */

void freeVertMapDB( VertMapDB *vmdb )
{
   int i;

   if ( vmdb ) {
      for ( i = 0; i < vmdb->nvmaps; i++ )
         if ( vmdb->vmap[ i ].name )
            free( vmdb->vmap[ i ].name );
      if ( vmdb->vmap )
         free( vmdb->vmap );
      free( vmdb );
   }
}


/*
======================================================================
getVertMapDB()

Fills a structure describing the vertex maps in a scene.  This is used
by getObjectVMaps(), but it can be useful for other purposes.
====================================================================== */

VertMapDB *getVertMapDB( GlobalFunc *global )
{
   VertMapDB *vmdb;
   LWObjectFuncs *objf;
   const char *name;
   int j, nvmaps;


   objf = global( LWOBJECTFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !objf ) return NULL;

   /* count vmaps of all types */

   nvmaps = objf->numVMaps( 0 );

   if ( !nvmaps ) return NULL;

   /* allocate the VertMapDB */

   vmdb = calloc( 1, sizeof( VertMapDB ));
   if ( !vmdb ) return NULL;
   vmdb->nvmaps = nvmaps;

   /* allocate the vmap array */

   vmdb->vmap = calloc( nvmaps, sizeof( DBVMapRec ));
   if ( !vmdb->vmap ) {
      free( vmdb );
      return NULL;
   }

   /* fill in the vmap array */

   for ( j = 0; j < nvmaps; j++ ) {
      vmdb->vmap[ j ].type = objf->vmapType( j );
      vmdb->vmap[ j ].dim = objf->vmapDim( 0, j );
      name = objf->vmapName( 0, j );
      vmdb->vmap[ j ].name = malloc( strlen( name ) + 1 );
      if ( !vmdb->vmap[ j ].name ) {
         freeVertMapDB( vmdb );
         return NULL;
      }
      strcpy( vmdb->vmap[ j ].name, name );
   }

   return vmdb;
}


/*
======================================================================
freeObjectVMaps()

Free memory allocated by getObjectVMaps().
====================================================================== */

void freeObjectVMaps( ObjectDB *odb )
{
   int i, j;

   if ( odb ) {
      if ( odb->vmap ) {
         for ( i = 0; i < odb->nvertmaps; i++ ) {
            if ( odb->vmap[ i ].name )
               free( odb->vmap[ i ].name );
            if ( odb->vmap[ i ].vindex )
               free( odb->vmap[ i ].vindex );
            if ( odb->vmap[ i ].val ) {
               for ( j = 0; j < odb->vmap[ i ].dim; j++ )
                  if ( odb->vmap[ i ].val[ j ] )
                     free( odb->vmap[ i ].val[ j ] );
               free( odb->vmap[ i ].val );
            }
         }
         free( odb->vmap );
         odb->vmap = NULL;
         odb->nvertmaps = 0;
      }

      for ( i = 0; i < odb->npoints; i++ ) {
         odb->pt[ i ].nvmaps = 0;
         if ( odb->pt[ i ].vm ) {
            free( odb->pt[ i ].vm );
            odb->pt[ i ].vm = NULL;
         }
      }
   }
}


/*
======================================================================
getObjectVMaps()

Allocate and fill in structures describing the vertex maps applied to
the object.  Updates the ObjectDB and returns 1 if successful.  Backs
out of any changes to the ObjectDB and returns 0 if an error occurs.

Before calling this function, the ObjectDB must be prepared to receive
vmap data.  This means that the point array has been allocated and
filled in, and the references to vmap structures are pristine--all
counts are 0 and all pointers are NULL.

We need to work within a peculiarity of the plug-in API for vmaps.
We can't ask for only the vmaps for a particular object.  In order to
find the vmaps associated with an object, we need to ask, for each
point in the object, whether there's a vmap value for that point.
Only vmaps for which at least one point has a value are added to the
ObjectDB.
====================================================================== */

int getObjectVMaps( ObjectDB *odb, LWMeshInfo *mesh, GlobalFunc *global )
{
   VertMapDB *vmdb;
   void *vmid;
   int i, j, k, n, ismapped, nvmaps, dim, *npts, ok = 0;
   float *val = NULL;


   /* get the list of vmaps in the scene */

   vmdb = getVertMapDB( global );
   if ( !vmdb ) return 1;

   /* create an array for counting the points with each vmap */

   npts = calloc( vmdb->nvmaps, sizeof( int ));
   if ( !npts ) goto Finish;

   /* count the object's vmaps and the number of points for each vmap */

   for ( i = 0, nvmaps = 0; i < vmdb->nvmaps; i++ ) {
      vmid = mesh->pntVLookup( mesh, vmdb->vmap[ i ].type, vmdb->vmap[ i ].name );

      if ( vmid ) {
         dim = mesh->pntVSelect( mesh, vmid );
         if ( dim > 0 ) {
            val = calloc( dim, sizeof( float ));
            if ( !val ) goto Finish;
         }
         else
            val = NULL;

         for ( j = 0; j < odb->npoints; j++ ) {
            ismapped = mesh->pntVGet( mesh, odb->pt[ j ].id, val );
            if ( ismapped ) ++npts[ i ];
         }

         if ( npts[ i ] ) ++nvmaps;
         free( val );
      }
   }

   /* no vmaps for this object? */

   if ( nvmaps == 0 ) {
      ok = 1;
      goto Finish;
   }

   /* allocate the vmap array */

   odb->nvertmaps = nvmaps;
   odb->vmap = calloc( nvmaps, sizeof( DBVMap ));
   if ( !odb->vmap ) goto Finish;

   for ( i = 0; i < nvmaps; i++ ) {

      /* initialize the vmap info */

      odb->vmap[ i ].name = malloc( strlen( vmdb->vmap[ i ].name ) + 1 );
      if ( !odb->vmap[ i ].name ) goto Finish;
      strcpy( odb->vmap[ i ].name, vmdb->vmap[ i ].name );
      odb->vmap[ i ].type = vmdb->vmap[ i ].type;
      odb->vmap[ i ].dim = vmdb->vmap[ i ].dim;
      odb->vmap[ i ].nverts = npts[ i ];

      /* allocate the point index array */

      odb->vmap[ i ].vindex = calloc( npts[ i ], sizeof( int ));
      if ( !odb->vmap[ i ].vindex ) goto Finish;

      /* allocate the value arrays */

      if ( vmdb->vmap[ i ].dim > 0 ) {
         odb->vmap[ i ].val = calloc( vmdb->vmap[ i ].dim, sizeof( float * ));
         if ( !odb->vmap[ i ].val ) goto Finish;
         for ( k = 0; k < vmdb->vmap[ i ].dim; k++ ) {
            odb->vmap[ i ].val[ k ] = calloc( npts[ i ], sizeof( float ));
            if ( !odb->vmap[ i ].val[ k ] ) goto Finish;
         }
      }

      /* fill in the point index and value arrays */

      vmid = mesh->pntVLookup( mesh, vmdb->vmap[ i ].type, vmdb->vmap[ i ].name );
      if ( vmid ) {
         dim = mesh->pntVSelect( mesh, vmid );
         if ( dim > 0 ) {
            val = calloc( dim, sizeof( float ));
            if ( !val ) goto Finish;
         }
         else
            val = NULL;

         for ( j = 0, n = 0; j < odb->npoints; j++ ) {
            ismapped = mesh->pntVGet( mesh, odb->pt[ j ].id, val );
            if ( ismapped ) {
               odb->vmap[ i ].vindex[ n ] = j;
               for ( k = 0; k < dim; k++ )
                  odb->vmap[ i ].val[ k ][ n ] = val[ k ];
               ++n;
            }
         }
         free( val );
      }
   }

   /* count the number of vmap values for each point */

   for ( i = 0; i < nvmaps; i++ )
      for ( j = 0; j < odb->vmap[ i ].nverts; j++ )
         ++odb->pt[ odb->vmap[ i ].vindex[ j ]].nvmaps;

   /* allocate vmap references for each mapped point */

   for ( i = 0; i < odb->npoints; i++ ) {
      odb->pt[ i ].vm = calloc( odb->pt[ i ].nvmaps, sizeof( DBVMapPt ));
      if ( !odb->pt[ i ].vm ) goto Finish;
      odb->pt[ i ].nvmaps = 0;
   }

   /* fill in vmap references for each mapped point */

   for ( i = 0; i < nvmaps; i++ ) {
      for ( j = 0; j < odb->vmap[ i ].nverts; j++ ) {
         n = odb->vmap[ i ].vindex[ j ];
         k = odb->pt[ n ].nvmaps;
         odb->pt[ n ].vm[ k ].vmap = &odb->vmap[ i ];
         odb->pt[ n ].vm[ k ].index = j;
         ++odb->pt[ n ].nvmaps;
      }
   }

   /* success */

   ok = 1;

Finish:
   freeVertMapDB( vmdb );
   if ( npts ) free( npts );
   if ( !ok )
      freeObjectVMaps( odb );

   return ok;
}
