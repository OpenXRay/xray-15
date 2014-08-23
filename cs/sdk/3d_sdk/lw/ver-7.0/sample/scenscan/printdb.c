/*
======================================================================
printdb.c

Functions for printing an object database to a text file.
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "objectdb.h"
#include "vecmat.h"


/*
======================================================================
printObjectDB()

Print the information in an object database to a file.  This is mostly
for debugging and to illustrate how to reference the items in the
database.  The c argument is 0 for initial normals and point positions
and 1 for final.
====================================================================== */

int printObjectDB( ObjectDB *odb, FILE *fp, int c )
{
   DBVMap *vmap;
   char *tag;
   int i, j, k, n;

   fprintf( fp, "%08.8x %s\n\n", odb->id, odb->filename );

   fprintf( fp, "Points (%d)\n\n", odb->npoints );
   for ( i = 0; i < odb->npoints; i++ ) {
      fprintf( fp, "%08.8x  pos (%g, %g, %g)  npols %d:",
         odb->pt[ i ].id,
         odb->pt[ i ].pos[ c ][ 0 ],
         odb->pt[ i ].pos[ c ][ 1 ],
         odb->pt[ i ].pos[ c ][ 2 ],
         odb->pt[ i ].npols );
      for ( j = 0; j < odb->pt[ i ].npols; j++ )
         fprintf( fp, " %d", odb->pt[ i ].pol[ j ] );
      fprintf( fp, "\n" );

      for ( j = 0; j < odb->pt[ i ].nvmaps; j++ ) {
         vmap = odb->pt[ i ].vm[ j ].vmap;
         switch ( vmap->type ) {
            case LWVMAP_PICK:  tag = "PICK";  break;
            case LWVMAP_WGHT:  tag = "WGHT";  break;
            case LWVMAP_MNVW:  tag = "MNVW";  break;
            case LWVMAP_TXUV:  tag = "TXUV";  break;
            case LWVMAP_MORF:  tag = "MORF";  break;
            case LWVMAP_SPOT:  tag = "SPOT";  break;
            default:           tag = "Unknown type";  break;
         }
         fprintf( fp, "  %s %s:", tag, vmap->name );
         for ( k = 0; k < vmap->dim; k++ ) {
            n = odb->pt[ i ].vm[ j ].index;
            fprintf( fp, " %g", vmap->val[ k ][ n ] );
         }
         fprintf( fp, "\n" );
      }
   }

   /* test the point search */

   /* If you uncommented the initPointSearch() call in getObjectDB(),
      don't call it here.  Either way, uncomment the rest of this to
      test the point search.  Note that the point search will "fail"
      if your object has two or more points at the same position.

   if ( initPointSearch( odb, c ))
      for ( i = 0; i < odb->npoints; i++ )
         if ( i != pointSearch( odb, odb->pt[ i ].pos[ c ] ))
            fprintf( fp, "Point search failed for point %d.\n", i );
   */

   fprintf( fp, "\n\nPolygons (%d)\n\n", odb->npolygons );
   for ( i = 0; i < odb->npolygons; i++ ) {
      fprintf( fp, "%08.8x  surf %d  norm (%g, %g, %g)  nverts %d:\n",
         odb->pol[ i ].id,
         odb->pol[ i ].sindex,
         odb->pol[ i ].norm[ c ][ 0 ],
         odb->pol[ i ].norm[ c ][ 1 ],
         odb->pol[ i ].norm[ c ][ 2 ],
         odb->pol[ i ].nverts );
      for ( j = 0; j < odb->pol[ i ].nverts; j++ ) {
         fprintf( fp, "  vert %d  vnorm (%g, %g, %g)\n",
            odb->pol[ i ].v[ j ].index,
            odb->pol[ i ].v[ j ].norm[ c ][ 0 ],
            odb->pol[ i ].v[ j ].norm[ c ][ 1 ],
            odb->pol[ i ].v[ j ].norm[ c ][ 2 ] );
      }
   }

   fprintf( fp, "\n\nSurfaces (%d)\n", odb->nsurfaces );
   for ( i = 0; i < odb->nsurfaces; i++ ) {
      fprintf( fp, "\n%08.8X  \"%s\"\n",
         odb->surf[ i ].id,
         odb->surf[ i ].name );
      fprintf( fp, "  " SURF_COLR "  %g %g %g\n",
         odb->surf[ i ].colr[ 0 ],
         odb->surf[ i ].colr[ 1 ],
         odb->surf[ i ].colr[ 2 ] );
      fprintf( fp, "  " SURF_LUMI "  %g\n", odb->surf[ i ].lumi );
      fprintf( fp, "  " SURF_DIFF "  %g\n", odb->surf[ i ].diff );
      fprintf( fp, "  " SURF_SPEC "  %g\n", odb->surf[ i ].spec );
      fprintf( fp, "  " SURF_REFL "  %g\n", odb->surf[ i ].refl );
      fprintf( fp, "  " SURF_RFOP "  0x%X\n", odb->surf[ i ].rfop );
      fprintf( fp, "  " SURF_TRAN "  %g\n", odb->surf[ i ].tran );
      fprintf( fp, "  " SURF_TROP "  0x%X\n", odb->surf[ i ].trop );
      fprintf( fp, "  " SURF_TRNL "  %g\n", odb->surf[ i ].trnl );
      fprintf( fp, "  " SURF_RIND "  %g\n", odb->surf[ i ].rind );
      fprintf( fp, "  " SURF_BUMP "  %g\n", odb->surf[ i ].bump );
      fprintf( fp, "  " SURF_GLOS "  %g\n", odb->surf[ i ].glos );
      fprintf( fp, "  " SURF_SHRP "  %g\n", odb->surf[ i ].shrp );
      fprintf( fp, "  " SURF_SMAN "  %g\n", odb->surf[ i ].sman );
      fprintf( fp, "  " SURF_RSAN "  %g\n", odb->surf[ i ].rsan );
      fprintf( fp, "  " SURF_TSAN "  %g\n", odb->surf[ i ].tsan );
      fprintf( fp, "  " SURF_CLRF "  %g\n", odb->surf[ i ].clrf );
      fprintf( fp, "  " SURF_CLRH "  %g\n", odb->surf[ i ].clrh );
      fprintf( fp, "  " SURF_ADTR "  %g\n", odb->surf[ i ].adtr );
      fprintf( fp, "  " SURF_ALPH "  0x%X\n", odb->surf[ i ].alph );
      fprintf( fp, "  " SURF_AVAL "  %g\n", odb->surf[ i ].aval );
      fprintf( fp, "  " SURF_GLOW "  0x%X\n", odb->surf[ i ].glow );
      fprintf( fp, "  " SURF_GVAL "  %g\n", odb->surf[ i ].gval );
      fprintf( fp, "  " SURF_LINE "  0x%X\n", odb->surf[ i ].line );
      fprintf( fp, "  " SURF_LSIZ "  %g\n", odb->surf[ i ].lsiz );
      fprintf( fp, "  " SURF_LCOL "  %g %g %g\n",
         odb->surf[ i ].lcol[ 0 ],
         odb->surf[ i ].lcol[ 1 ],
         odb->surf[ i ].lcol[ 2 ] );
      fprintf( fp, "  " SURF_SIDE "  0x%X\n", odb->surf[ i ].side );
      fprintf( fp, "  " SURF_RIMG "  %08.8X\n", odb->surf[ i ].rimg );
      fprintf( fp, "  " SURF_TIMG "  %08.8X\n", odb->surf[ i ].timg );
   }

   fprintf( fp, "\n\nVertex Maps (%d)\n\n", odb->nvertmaps );
   for ( i = 0; i < odb->nvertmaps; i++ ) {
      tag = ( char * ) &odb->vmap[ i ].type;
      fprintf( fp, "%c%c%c%c \"%s\"  dim %d  nverts %d\n",
#ifdef _WIN32
         tag[ 3 ], tag[ 2 ], tag[ 1 ], tag[ 0 ],
#else
         tag[ 0 ], tag[ 1 ], tag[ 2 ], tag[ 3 ],
#endif
         odb->vmap[ i ].name,
         odb->vmap[ i ].dim,
         odb->vmap[ i ].nverts );
      for ( j = 0; j < odb->vmap[ i ].nverts; j++ ) {
         fprintf( fp, "  %d ", odb->vmap[ i ].vindex[ j ] );
         for ( k = 0; k < odb->vmap[ i ].dim; k++ )
            fprintf( fp, " %g", odb->vmap[ i ].val[ k ][ j ] );
         fprintf( fp, "\n" );
      }
   }

   return 1;
}
