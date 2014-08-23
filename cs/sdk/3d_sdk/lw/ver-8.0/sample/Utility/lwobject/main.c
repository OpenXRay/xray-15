/*
======================================================================
main.c

Test the LightWave object loader.

Ernie Wright  16 Nov 00

This is a command-line program that takes an object filename as its
argument, loads the file, and displays some statistics.  The filename
can include wildcards, and if it's "*.*", the program will also look
in subdirectories for objects to load.

We have to use platform-specific code, since C has no native method
for traversing a file system.  This version uses the MSVC runtime.

The print_vmaps functions demonstrate how to access vmap information
in three different ways, by looping through the vmap list, the point
array and the polygon array.
====================================================================== */

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <lwo2.h>


int nobjects = 0, nlayers = 0, nsurfs = 0, nenvs = 0, nclips = 0,
   npoints = 0, npolygons = 0;


/*
======================================================================
print_vmaps1()

Print vmap values for a layer, looping through the vmaps.

   for each vmap
      print vmap statistics
      for each mapped point
         print point index and position
         if vmad, print polygon index and vertex number
         print vmap values
====================================================================== */

static void print_vmaps1( FILE *fp, lwLayer *layer )
{
   lwPoint *pt;
   lwVMap *vmap;
   char *tag;
   int i, j, k, n;

   fprintf( fp, "\n\nVertex Maps (%d)\n\n", layer->nvmaps );

   vmap = layer->vmap;
   for ( i = 0; i < layer->nvmaps; i++ ) {
      tag = ( char * ) &vmap->type;

      fprintf( fp, "%c%c%c%c \"%s\"  dim %d  nverts %d  vmad (%s)\n",
         tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ],
         vmap->name,
         vmap->dim,
         vmap->nverts,
         vmap->perpoly ? "yes" : "no" );

      printf( "%c%c%c%c \"%s\"  dim %d  nverts %d  vmad (%s)\n",
         tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ],
         vmap->name,
         vmap->dim,
         vmap->nverts,
         vmap->perpoly ? "yes" : "no" );

      for ( j = 0; j < vmap->nverts; j++ ) {
         /* point index */
         fprintf( fp, "  point %d ", vmap->vindex[ j ] );

         /* if vmad */
         if ( vmap->perpoly ) {
            lwPolygon *pol;

            /* polygon index */
            k = vmap->pindex[ j ];
            fprintf( fp, " poly %d ", k );

            /* vertex index */
            pol = &layer->polygon.pol[ k ];
            for ( n = 0; n < pol->nverts; n++ )
               if ( pol->v[ n ].index == vmap->vindex[ j ] ) break;
            fprintf( fp, " vert %d ", n );
         }

         /* point coords */
         pt = &layer->point.pt[ vmap->vindex[ j ]];
         fprintf( fp, " (%g, %g, %g) ", pt->pos[ 0 ], pt->pos[ 1 ], pt->pos[ 2 ] );

         /* vmap values */
         for ( k = 0; k < vmap->dim; k++ )
            fprintf( fp, " %g", vmap->val[ j ][ k ] );

         /* done with this point */
         fprintf( fp, "\n" );
      }
      /* done with this vmap */
      fprintf( fp, "\n" );
      vmap = vmap->next;
   }
   /* done with this layer */
   fprintf( fp, "\n\n" );
}


/*
======================================================================
print_vmaps2()

Print vmap values for a layer, looping through the points.

   for each point
      print point index, position, number of vmaps, polygon indexes
      for each vmap on the point
         print vmap name, type and values
====================================================================== */

static void print_vmaps2( FILE *fp, lwLayer *layer )
{
   lwPoint *pt;
   lwVMap *vmap;
   char *tag;
   int i, j, k, n;

   fprintf( fp, "\n\nPoints (%d)\n\n", layer->point.count );

   for ( i = 0; i < layer->point.count; i++ ) {
      pt = &layer->point.pt[ i ];

      /* point index and position */
      fprintf( fp, "%d (%g, %g, %g)", i, pt->pos[ 0 ], pt->pos[ 1 ], pt->pos[ 2 ] );

      /* number of vmaps and polygons */
      fprintf( fp, "  nvmaps %d  npolygons %d", pt->nvmaps, pt->npols );

      /* polygon indexes */
      fprintf( fp, " [" );
      for ( j = 0; j < pt->npols; j++ )
         fprintf( fp, " %d", pt->pol[ j ] );
      fprintf( fp, "]\n" );

      /* vmaps for this point */
      for ( j = 0; j < pt->nvmaps; j++ ) {
         vmap = pt->vm[ j ].vmap;
         n = pt->vm[ j ].index;

         tag = ( char * ) &vmap->type;

         fprintf( fp, "  %c%c%c%c \"%s\" vmad (%s)",
            tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], vmap->name,
            vmap->perpoly ? "yes" : "no" );

         /* vmap values */
         for ( k = 0; k < vmap->dim; k++ )
            fprintf( fp, " %g", vmap->val[ n ][ k ] );

         /* done with this vmap */
         fprintf( fp, "\n" );
      }
      /* done with this point */
      fprintf( fp, "\n" );
   }
   /* done with this layer */
   fprintf( fp, "\n\n" );
}


/*
======================================================================
print_vmaps3()

Print vmap values for a layer, looping through the polygons.

   for each polygon
      print polygon index, number of points
      for each vertex
         print point index, position, number of vmaps
         for each vmap on the point
            print vmap name, type and values
====================================================================== */

static void print_vmaps3( FILE *fp, lwLayer *layer )
{
   lwPoint *pt;
   lwPolygon *pol;
   lwVMap *vmap;
   char *tag;
   int i, j, k, m, n;

   fprintf( fp, "\n\nPolygons (%d)\n\n", layer->polygon.count );

   for ( i = 0; i < layer->polygon.count; i++ ) {
      pol = &layer->polygon.pol[ i ];

      /* polygon index, type, number of vertices */
      tag = ( char * ) &pol->type;
      fprintf( fp, "%d %c%c%c%c  nverts %d\n", i,
         tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], pol->nverts );

      for ( k = 0; k < pol->nverts; k++ ) {
         /* point index, position, number of vmads and vmaps */
         n = pol->v[ k ].index;
         pt = &layer->point.pt[ n ];
         fprintf( fp, "%d (%g, %g, %g)  nvmads %d  nvmaps %d\n", n,
            pt->pos[ 0 ], pt->pos[ 1 ], pt->pos[ 2 ],
            pol->v[ k ].nvmaps, pt->nvmaps - pol->v[ k ].nvmaps );

         /* vmads for this vertex */
         for ( j = 0; j < pol->v[ k ].nvmaps; j++ ) {
            vmap = pol->v[ k ].vm[ j ].vmap;
            n = pol->v[ k ].vm[ j ].index;

            tag = ( char * ) &vmap->type;
            fprintf( fp, "  %c%c%c%c vmad \"%s\"",
               tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], vmap->name );

            /* vmap values */
            for ( m = 0; m < vmap->dim; m++ )
               fprintf( fp, " %g", vmap->val[ n ][ m ] );

            /* done with this vmad */
            fprintf( fp, "\n" );
         }

         /* vmaps for this vertex */
         for ( j = 0; j < pt->nvmaps; j++ ) {
            vmap = pt->vm[ j ].vmap;
            if ( vmap->perpoly ) continue;
            n = pt->vm[ j ].index;

            tag = ( char * ) &vmap->type;
            fprintf( fp, "  %c%c%c%c vmap \"%s\"",
               tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ], vmap->name );

            /* vmap values */
            for ( m = 0; m < vmap->dim; m++ )
               fprintf( fp, " %g", vmap->val[ n ][ m ] );

            /* done with this vmap */
            fprintf( fp, "\n" );
         }

         /* done with this vertex */
         if ( pt->nvmaps )
            fprintf( fp, "\n" );
      }
      /* done with this polygon */
      fprintf( fp, "\n" );
   }
   /* done with this layer */
   fprintf( fp, "\n\n" );
}


/*
======================================================================
print_vmaps()

Print vmap values for a layer.

Calls print_vmaps1(), print_vmaps2() and print_vmaps3().
====================================================================== */

void print_vmaps( lwObject *obj )
{
   FILE *fp[ 3 ];
   char buf[ 64 ];
   lwLayer *layer;
   int i, j;

   for ( i = 0; i < 3; i++ ) {
      sprintf( buf, "vmapout%d.txt", i + 1 );
      fp[ i ] = fopen( buf, "w" );
      if ( !fp[ i ] ) {
         for ( j = i - 1; j >= 0; j-- )
            fclose( fp[ j ] );
         return;
      }
   }

   layer = obj->layer;
   for ( i = 0; i < obj->nlayers; i++ ) {
      fprintf( fp[ 0 ], "------------------------\nLayer %d\n", i );
      print_vmaps1( fp[ 0 ], layer );

      fprintf( fp[ 1 ], "------------------------\nLayer %d\n", i );
      print_vmaps2( fp[ 1 ], layer );

      fprintf( fp[ 2 ], "------------------------\nLayer %d\n", i );
      print_vmaps3( fp[ 2 ], layer );

      layer = layer->next;
   }

   for ( i = 0; i < 3; i++ )
      fclose( fp[ i ] );
}


int testload( char *filename, unsigned int *failID, int *failpos )
{
   lwObject *obj;

   obj = lwGetObject( filename, failID, failpos );

   if ( obj ) {
      printf(
         "Layers:  %d\n"
         "Surfaces:  %d\n"
         "Envelopes:  %d\n"
         "Clips:  %d\n"
         "Points (first layer):  %d\n"
         "Polygons (first layer):  %d\n\n",
         obj->nlayers, obj->nsurfs, obj->nenvs, obj->nclips,
         obj->layer->point.count, obj->layer->polygon.count );
      nobjects++;
      nlayers += obj->nlayers;
      nsurfs += obj->nsurfs;
      nenvs += obj->nenvs;
      nclips += obj->nclips;
      npoints += obj->layer->point.count;
      npolygons += obj->layer->polygon.count;

      /* uncomment this to generate vmap output */
      print_vmaps( obj );

      lwFreeObject( obj );
      return 1;
   }
   else {
      printf( "Couldn't load %s.\n", filename );
      return 0;
   }
}


int make_filename( char *spec, char *name, char *fullname )
{
   char
      drive[ _MAX_DRIVE ],
      dir[ _MAX_DIR ],
      node[ _MAX_FNAME ],
      ext[ _MAX_EXT ];

   _splitpath( spec, drive, dir, node, ext );
   _makepath( fullname, drive, dir, name, NULL );
   return 1;
}


int make_filespec( char *spec, char *subdir, char *fullname )
{
   char
      name[ _MAX_FNAME ],
      drive[ _MAX_DRIVE ],
      dir[ _MAX_DIR ],
      node[ _MAX_FNAME ],
      ext[ _MAX_EXT ];

   _splitpath( spec, drive, dir, node, ext );
   _makepath( name, drive, dir, subdir, NULL );
   _makepath( fullname, NULL, name, node, ext );
   return 1;
}


int find_files( char *filespec )
{
   long h, err;
   struct _finddata_t data;
   char *filename, *prevname;
   unsigned int failID;
   int failpos;

   filename = malloc( 520 );
   if ( !filename ) return 0;
   prevname = filename + 260;

   err = h = _findfirst( filespec, &data );
   if ( err == -1 ) {
      printf( "No files found: '%s'\n", filespec );
      return 0;
   }

   while ( err != -1 ) {
      if (( data.attrib & _A_SUBDIR ) && data.name[ 0 ] != '.' ) {
         make_filespec( filespec, data.name, filename );
         find_files( filename );
      }
      if ( !( data.attrib & _A_SUBDIR )) {
         make_filename( filespec, data.name, filename );
         if ( !strcmp( filename, prevname )) break;
         strcpy( prevname, filename );
         printf( "%s\n", filename );
         failID = failpos = 0;
         if ( !testload( filename, &failID, &failpos )) {
            printf( "%s\nLoading failed near byte %d\n\n", filename, failpos );
         }
      }
      err = _findnext( h, &data );
   }

   _findclose( h );
   free( filename );
   return 1;
}


void main( int argc, char *argv[] )
{
   float t1, t2;

   if ( argc != 2 ) {
      printf( "Usage:  %s <filespec>\n", argv[ 0 ] );
      exit( 0 );
   }

   t1 = ( float ) clock() / CLOCKS_PER_SEC;
   find_files( argv[ 1 ] );
   t2 = ( float ) clock() / CLOCKS_PER_SEC - t1;

   printf( "\n%8d objects\n", nobjects );
   printf( "%8d layers\n", nlayers );
   printf( "%8d surfaces\n", nsurfs );
   printf( "%8d envelopes\n", nenvs );
   printf( "%8d clips\n", nclips );
   printf( "%8d points\n", npoints );
   printf( "%8d polygons\n\n", npolygons );
   printf( "%g seconds\n\n", t2 );
}
