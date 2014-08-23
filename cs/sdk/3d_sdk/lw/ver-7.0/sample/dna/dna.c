/*
======================================================================
dna.c

Ernie Wright  24 Jan 00

A Modeler plug-in that creates classic Watson-Crick DNA molecules.
This uses LWPanels to construct the interface and the ModLib library
of Modeler functions to issue commands and perform mesh edits.
====================================================================== */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <lwpanel.h>
#include "lwmodlib.h"
#include "geometry.h"


#define ATYPE_POINT    0
#define ATYPE_SPHERE   1
#define ATYPE_DODEC    2

#define BTYPE_LINE     0
#define BTYPE_CYLINDER 1


static char sequence[ 128 ] = "acg tac";

static char
   layer1[ 8 ] = "1",
   layer2[ 8 ] = "2";

static int
   seq[ 128 ],
   seqlen,
   userabort      = 0,
   atom_type      = ATYPE_POINT,
   atom_nsides    = 12,
   atom_nsegments = 6,
   bond_type      = BTYPE_LINE,
   bond_nsides    = 12,
   bond_nsegments = 1,
   do_plates      = 1;

static double
   atom_radius[ 5 ] = { 0.5, 0.3, 0.5, 0.5, 0.7 },
   bond_radius = 0.15;


/*
======================================================================
dodec()

Create a dodecahedron.

INPUTS
   center      dodec center
   r           dodec radius
   surf        surface name

RESULTS
   Creates a dodecahedron of the specified size and position in the
   current layer.

Called by meshedit().
====================================================================== */

static void dodec( double center[ 3 ], double r, char *surf )
{
   static float vert[ 20 ][ 3 ] = {
      0.618034f,  0.618034f,  0.618034f,
      1.000000f,       0.0f,  0.381966f,
      0.618034f, -0.618034f,  0.618034f,
           0.0f, -0.381966f,  1.000000f,
           0.0f,  0.381966f,  1.000000f,
      1.000000f,       0.0f, -0.381966f,
      0.618034f,  0.618034f, -0.618034f,
      0.381966f,  1.000000f,       0.0f,
     -0.381966f,  1.000000f,       0.0f,
     -0.618034f,  0.618034f, -0.618034f,
           0.0f,  0.381966f, -1.000000f,
      0.618034f, -0.618034f, -0.618034f,
      0.381966f, -1.000000f,       0.0f,
     -0.381966f, -1.000000f,       0.0f,
     -1.000000f,       0.0f,  0.381966f,
     -0.618034f,  0.618034f,  0.618034f,
     -0.618034f, -0.618034f, -0.618034f,
           0.0f, -0.381966f, -1.000000f,
     -0.618034f, -0.618034f,  0.618034f,
     -1.000000f,       0.0f, -0.381966f
   };

   static short face[ 12 ][ 5 ] = {
        7,  8, 15,  4,  0,
       10,  9,  8,  7,  6,
        5,  6,  7,  0,  1,
       12, 11,  5,  1,  2,
       13, 16, 17, 11, 12,
        3, 18, 13, 12,  2,
        0,  4,  3,  2,  1,
        4, 15, 14, 18,  3,
       11, 17, 10,  6,  5,
       16, 19,  9, 10, 17,
        8,  9, 19, 14, 15,
       14, 19, 16, 13, 18
   };

   double pt[ 3 ];
   LWPntID id[ 20 ], idf[ 5 ];
   int i, j;


   for ( i = 0; i < 20; i++ ) {
      pt[ 0 ] = r * vert[ i ][ 0 ] + center[ 0 ];
      pt[ 1 ] = r * vert[ i ][ 1 ] + center[ 1 ];
      pt[ 2 ] = r * vert[ i ][ 2 ] + center[ 2 ];
      id[ i ] = meAddPoint( pt );
   }

   for ( i = 0; i < 12; i++ ) {
      for ( j = 0; j < 5; j++ )
         idf[ j ] = id[ face[ i ][ j ]];
      meAddPoly( LWPOLTYPE_FACE, NULL, surf, 5, idf );
   }
}


/*
======================================================================
transform_point()

Move and rotate a point on a molecule component into position.

INPUTS
   dy          amount to move along Y
   rot         amount to rotate around Y
   pt          the pt to transform

RESULTS
   Transforms pt[] in place.

Called by meshedit() and cmdedit().
====================================================================== */

static void transform_point( double dy, double rot, double pt[ 3 ] )
{
   double r, theta;

   pt[ 1 ] += dy;
   if ( pt[ 0 ] == 0.0 && pt[ 2 ] == 0.0 ) return;

   r = sqrt( pt[ 0 ] * pt[ 0 ] + pt[ 2 ] * pt[ 2 ] );
   theta = PI / 2.0 - atan2( pt[ 2 ], pt[ 0 ] ) - degrad( rot );
   pt[ 0 ] = r * sin( theta );
   pt[ 2 ] = r * cos( theta );
}


/*
======================================================================
meshedit()

Perform all of the mesh editing operations.  Creates the parts of the
molecule comprising points, lines, and polygons.

Called by BuildDNA().
====================================================================== */

static void meshedit( void )
{
   double pt[ 3 ];
   LWPntID id[ 57 ], vid[ 2 ];
   int i, j, k, n, v[ 9 ], snum;


   csMeshBegin( 0, 0, OPSEL_GLOBAL );

   if ( bond_type == BTYPE_LINE ) {
      mgMonitorBegin( "Line Bonds", NULL, seqlen );

      for ( j = 0; j < seqlen; j++ ) {
         n = point_count( seq[ j ] );
         for ( i = 0; i < n; i++ ) {
            vert_coords( seq[ j ], i, pt );
            transform_point( j * 3.4, j * 36.0, pt );
            id[ i ] = meAddPoint( pt );
         }
         n = bond_count( seq[ j ] );
         for ( i = 0; i < n; i++ ) {
            bond_info( seq[ j ], i, &v[ 0 ], &v[ 1 ], &snum );
            vid[ 0 ] = id[ v[ 0 ]];
            vid[ 1 ] = id[ v[ 1 ]];
            meAddPoly( LWPOLTYPE_FACE, NULL, surface_name( snum ), 2, vid );
         }
         if ( userabort = mgMonitorStep( 1 ))
            break;
      }
      mgMonitorDone();
   }

   if ( atom_type == ATYPE_POINT && !userabort ) {
      mgMonitorBegin( "Point Atoms", NULL, seqlen );

      for ( j = 0; j < seqlen; j++ ) {
         n = atom_count( seq[ j ] );
         for ( i = 0; i < n; i++ ) {
            atom_info( seq[ j ], i, &v[ 0 ], &snum );
            vert_coords( seq[ j ], v[ 0 ], pt );
            transform_point( j * 3.4, j * 36.0, pt );
            id[ 0 ] = meAddPoint(  pt );
            meAddPoly( LWPOLTYPE_FACE, NULL, surface_name( snum ), 1, &id[ 0 ] );
         }
         if ( userabort = mgMonitorStep( 1 ))
            break;
      }
      mgMonitorDone();
   }

   else if ( atom_type == ATYPE_DODEC && !userabort ) {
      mgMonitorBegin( "Dodecahedron Atoms", NULL, seqlen );

      for ( j = 0; j < seqlen; j++ ) {
         n = atom_count( seq[ j ] );
         for ( i = 0; i < n; i++ ) {
            atom_info( seq[ j ], i, &v[ 0 ], &snum );
            vert_coords( seq[ j ], v[ 0 ], pt );
            transform_point( j * 3.4, j * 36.0, pt );
            dodec( pt, atom_radius[ snum ], surface_name( snum ));
         }
         if ( userabort = mgMonitorStep( 1 ))
            break;
      }
      mgMonitorDone();
   }

   if ( do_plates && !userabort ) {
      mgMonitorBegin( "Base Plates", NULL, seqlen );

      for ( j = 0; j < seqlen; j++ ) {
         for ( i = 0; i < 2; i++ ) {
            plate_info( seq[ j ], i, &n, v, &snum );
            for ( k = 0; k < n; k++ ) {
               vert_coords( seq[ j ], v[ k ], pt );
               transform_point( j * 3.4, j * 36.0, pt );
               id[ k ] = meAddPoint( pt );
            }
            meAddPoly( LWPOLTYPE_FACE, NULL, surface_name( snum ), n, id );
         }
         if ( userabort = mgMonitorStep( 1 ))
            break;
      }
      mgMonitorDone();
   }

   csMeshDone( EDERR_NONE, 0 );
}


/*
======================================================================
cmdedit()

Perform all of the command sequence operations.  Creates the parts of
the molecule comprising Modeler primitives (spheres and cylinders).

Called by BuildDNA().
====================================================================== */

static void cmdedit( void )
{
   double pt[ 3 ], r[ 3 ], c[ 3 ], h, xrot, yrot, rot;
   int i, j, n, vi, snum;


   csSetLayer( layer1 );
   csMergePoints( 0 );

   if ( atom_type == ATYPE_SPHERE ) {
      mgMonitorBegin( "Sphere Atoms", NULL, seqlen );

      csSetLayer( layer1 );
      for ( j = 0; j < seqlen; j++ ) {
         n = atom_count( seq[ j ] );
         for ( i = 0; i < n; i++ ) {
            atom_info( seq[ j ], i, &vi, &snum );
            vert_coords( seq[ j ], vi, pt );
            transform_point( j * 3.4, j * 36.0, pt );
            r[ 0 ] = r[ 1 ] = r[ 2 ] = atom_radius[ snum ];
            csSetDefaultSurface( surface_name( snum ));
            csMakeBall( r, atom_nsides, atom_nsegments, pt );
         }
         if ( userabort = mgMonitorStep( 1 ))
            break;
      }
      mgMonitorDone();
   }

   if ( bond_type == BTYPE_CYLINDER && !userabort ) {
      mgMonitorBegin( "Cylinder Bonds", NULL, seqlen );

      for ( j = 0; j < seqlen; j++ ) {
         n = bond_count( seq[ j ] );
         for ( i = 0; i < n; i++ ) {
            bond_coords( seq[ j ], i, pt, &h, &xrot, &yrot, &snum );
            pt[ 1 ] += 3.4 * j;
            csSetLayer( layer2 );
            csSetDefaultSurface( surface_name( snum ));
            r[ 0 ] = r[ 1 ] = r[ 2 ] = bond_radius;
            c[ 0 ] = c[ 2 ] = 0.0;
            c[ 1 ] = h / 2.0;
            csMakeDisc( r, h, 0, "Y", bond_nsides, bond_nsegments, c );
            csRotate( xrot, "X", NULL );
            csRotate( yrot, "Y", NULL );
            csMove( pt );
            rot = 36 * j;
            csRotate( rot, "Y", NULL );
            csCut();
            csSetLayer( layer1 );
            csPaste();
         }
         if ( userabort = mgMonitorStep( 1 ))
            break;
      }
      mgMonitorDone();
   }
}


/*
======================================================================
getlayers()

Find the two layers in which editing will take place.  We just pick
the primary layer (where the molecule will end up) and the first empty
layer in the list other than the primary (used as a scratch layer).

Called by BuildDNA().
====================================================================== */

static void getlayers( void )
{
   const char *str;
   char buf[ 128 ], *tok;
   int i, j;

   str = mgGetLayerList( OPLYR_PRIMARY, NULL );
   i = atoi( str );
   itoa( i, layer1, 10 );

   str = mgGetLayerList( OPLYR_EMPTY, NULL );
   strcpy( buf, str );

   tok = strtok( buf, " " );
   j = atoi( tok );

   while ( tok && ( i == j )) {
      tok = strtok( NULL, " " );
      j = atoi( tok );
   }

   itoa( j, layer2, 10 );
}


/*
======================================================================
setsurf()

Give Modeler the surface descriptions for the DNA surfaces.

Called by BuildDNA().
====================================================================== */

static void setsurf( void )
{
   DNA_SURFACE *sdata;
   LWSurfaceID sid;
   double color[ 3 ];
   const char *objname;
   int i;

   objname = mgGetCurrentObject();
   if ( !objname ) return;

   for ( i = 0; i < 11; i++ ) {
      sdata = surface_data( i );
      sid = mgCreateSurface( objname, sdata->name );
      csSetSurfEdSurface( sdata->name, objname );
      color[ 0 ] = ( double ) sdata->colr[ 0 ];
      color[ 1 ] = ( double ) sdata->colr[ 1 ];
      color[ 2 ] = ( double ) sdata->colr[ 2 ];
      csSetSurfaceColor( SURF_COLR, color );

      csSetSurfaceFlt( SURF_DIFF, sdata->diff );
      csSetSurfaceFlt( SURF_SPEC, sdata->spec );
      csSetSurfaceFlt( SURF_GLOS, sdata->glos );

      if ( sdata->sman > 0.0 )
         csSetSurfaceFlt( SURF_SMAN, sdata->sman );

      if ( sdata->tran > 0.0 ) {
         csSetSurfaceFlt( SURF_TRAN, sdata->tran );
         csSetSurfaceFlt( SURF_RIND, sdata->rind );
      }

      csSetSurfaceInt( SURF_SIDE, sdata->side );
   }

}


/*
======================================================================
getseq()

Convert the sequence string to an array of integers and return the
sequence length.
====================================================================== */

static int getseq( void )
{
   int len, i;

   len = strlen( sequence );
   for ( i = 0, seqlen = 0; i < len; i++ )
      switch ( sequence[ i ] ) {
         case 'A': case 'a':  seq[ seqlen++ ] = 0;  break;
         case 'C': case 'c':  seq[ seqlen++ ] = 1;  break;
         case 'G': case 'g':  seq[ seqlen++ ] = 2;  break;
         case 'T': case 't':  seq[ seqlen++ ] = 3;  break;
         default:             break;
      }

   return seqlen;
}


/*
======================================================================
elcount()

Calculates the number of points and polygons that would be created,
based on the current settings.
====================================================================== */

static void elcount( int *npnts, int *npols )
{
   int i, apts, apls, bpts, bpls, ppts, ppls;

   getseq();

   for ( i = 0, bpls = 0; i < seqlen; i++ )
      bpls += bond_count( seq[ i ] );

   if ( bond_type == BTYPE_LINE ) {
      for ( i = 0, bpts = 0; i < seqlen; i++ )
         bpts += point_count( seq[ i ] ) - 4;
      bpts += 2;
   }
   else {
      bpts = bpls * bond_nsides * ( 1 + bond_nsegments );
      bpls *= bond_nsides * bond_nsegments + 2;
   }

   for ( i = 0, apls = 0; i < seqlen; i++ )
      apls += atom_count( seq[ i ] );

   if ( atom_type == ATYPE_POINT )
      apts = ( bond_type == BTYPE_LINE ) ? 0 : apls;
   else if ( atom_type == ATYPE_SPHERE ) {
      apts = apls * ( atom_nsides * ( atom_nsegments - 1 ) + 2 );
      apls *= atom_nsides * atom_nsegments;
   }
   else {
      apts = 20 * apls;
      apls *= 12;
   }

   if ( do_plates ) {
      ppts = ( atom_type == ATYPE_POINT || bond_type == BTYPE_LINE ) ?
         0 : 15 * seqlen;
      ppls = 2 * seqlen;
   }
   else ppts = ppls = 0;

   if ( npnts ) *npnts = apts + bpts + ppts;
   if ( npols ) *npols = apls + bpls + ppls;
}


/* user interface stuff */

static LWPanelFuncs *panf;                // panel functions
static LWPanelID panel;                   // panel
static LWControl *ctl[ 16 ];              // panel gadgets
static const LWDisplayMetrics *dmet;      // sizes of panel elements
static LWPanControlDesc desc;             // required by macros in lwpanel.h
static LWValue
   ival = { LWT_INTEGER },
   sval = { LWT_STRING },
   fval = { LWT_FLOAT };

/*
======================================================================
ctl_get()

Gets the values of all of the controls.  Called by ctl_event() and
get_user().
====================================================================== */

static void ctl_get( void )
{
   int i;

   GET_STR( ctl[ 0 ], sequence, sizeof( sequence ));
   GET_INT( ctl[ 1 ], atom_type );
   GET_INT( ctl[ 2 ], atom_nsides );
   GET_INT( ctl[ 3 ], atom_nsegments );
   for ( i = 0; i < 5; i++ )
      GET_FLOAT( ctl[ i + 5 ], atom_radius[ i ] );
   GET_INT( ctl[ 10 ], bond_type );
   GET_INT( ctl[ 11 ], bond_nsides );
   GET_INT( ctl[ 12 ], bond_nsegments );
   GET_FLOAT( ctl[ 13 ], bond_radius );
   GET_INT( ctl[ 14 ], do_plates );
}


/*
======================================================================
ctl_set()

Sets the values of all of the controls.  Called by ctl_create().
====================================================================== */

static void ctl_set( void )
{
   int i;

   SET_STR( ctl[ 0 ], sequence, strlen( sequence ));
   SET_INT( ctl[ 1 ], atom_type );
   SET_INT( ctl[ 2 ], atom_nsides );
   SET_INT( ctl[ 3 ], atom_nsegments );
   for ( i = 0; i < 5; i++ )
      SET_FLOAT( ctl[ i + 5 ], atom_radius[ i ] );
   SET_INT( ctl[ 10 ], bond_type );
   SET_INT( ctl[ 11 ], bond_nsides );
   SET_INT( ctl[ 12 ], bond_nsegments );
   SET_FLOAT( ctl[ 13 ], bond_radius );
   SET_INT( ctl[ 14 ], do_plates );
}


/*
======================================================================
ctl_enable()

Ghosts or unghosts controls, depending on the current settings.
Called by ctl_event().
====================================================================== */

static void ctl_enable( void )
{
   int i;

   switch ( atom_type ) {
      case ATYPE_POINT:
         for ( i = 2; i < 10; i++ )
            GHOST_CON( ctl[ i ] );
         break;
      case ATYPE_SPHERE:
         for ( i = 2; i < 10; i++ )
            RENDER_CON( ctl[ i ] );
         break;
      case ATYPE_DODEC:
         for ( i = 2; i < 4; i++ )
            GHOST_CON( ctl[ i ] );
         for ( i = 4; i < 10; i++ )
            RENDER_CON( ctl[ i ] );
         break;
   }

   switch ( bond_type ) {
      case BTYPE_LINE:
         for ( i = 11; i < 14; i++ )
            GHOST_CON( ctl[ i ] );
         break;
      case BTYPE_CYLINDER:
         for ( i = 11; i < 14; i++ )
            RENDER_CON( ctl[ i ] );
         break;
   }
}


/*
======================================================================
ctl_draw()

Control draw callback for the point and polygon count.  This simulates
LightWave's info boxes.  Called by LWPanels.
====================================================================== */

static void ctl_draw( LWControl *ectl, void *edata, DrMode mode )
{
   int x, y, w, h, dx, dy, npts, npols;
   char buf[ 24 ];

   elcount( &npts, &npols );

   x = CON_HOTX( ectl );
   y = CON_HOTY( ectl );
   w = CON_HOTW( ectl );
   h = CON_HOTH( ectl );

   panf->drawFuncs->drawBox( panel, LWP_INFO_BG, x, y, w, h );

   dy = ( h - 2 * dmet->textHeight ) / 3;
   dx = w / 2 - panf->drawFuncs->textWidth( panel, "Points:" );
   sprintf( buf, "Points: %d", npts );
   panf->drawFuncs->drawText( panel, buf, LWP_INFO_IMG, x + dx, y + dy );

   dy += dy + dmet->textHeight;
   dx = w / 2 - panf->drawFuncs->textWidth( panel, "Polygons:" );
   sprintf( buf, "Polygons: %d", npols );
   panf->drawFuncs->drawText( panel, buf, LWP_INFO_IMG, x + dx, y + dy );
}


/*
======================================================================
ctl_event()

Control callback.  This updates control enable states and redraws the
point and polygon count after the user changes something.  Called by
LWPanels.
====================================================================== */

static void ctl_event( LWControl *ectl, void *edata )
{
   ctl_get();
   ctl_enable();
   RENDER_CON( ctl[ 15 ] );
}


/*
======================================================================
ctl_create()

Creates the user interface controls.  Called by get_user().
====================================================================== */

static void ctl_create( void )
{
   static char *labatype[] = { "Point", "Sphere", "Dodec", NULL };
   static char *labbtype[] = { "Line", "Cylinder", NULL };
   static char *labradii[] = { "Radii", NULL };
   int x, y, w, h, ph, i;


   /* create a control */

   ctl[ 0 ] = STR_CTL( panf, panel, "Sequence", 40 );

   /* find out how much vertical space the panel wants for drawing its
      own decorations, and get some other panel metrics */

   ph = PAN_GETH( panf, panel ) - CON_H( ctl[ 0 ] );
   dmet = panf->drawFuncs->dispMetrics();

   /* create the rest of the controls */

   ctl[  1 ] = WPOPUP_CTL( panf, panel, "Atoms", labatype, 100 );
   ctl[  2 ] = INT_CTL( panf, panel, "Sides" );
   ctl[  3 ] = INT_CTL( panf, panel, "Segments" );
   ctl[  4 ] = TEXT_CTL( panf, panel, "", labradii );
   ctl[  5 ] = FLOAT_CTL( panf, panel, "Carbon" );
   ctl[  6 ] = FLOAT_CTL( panf, panel, "Hydrogen" );
   ctl[  7 ] = FLOAT_CTL( panf, panel, "Nitrogen" );
   ctl[  8 ] = FLOAT_CTL( panf, panel, "Oxygen" );
   ctl[  9 ] = FLOAT_CTL( panf, panel, "Phosphorus" );
   ctl[ 10 ] = WPOPUP_CTL( panf, panel, "Bonds", labbtype, 100 );
   ctl[ 11 ] = INT_CTL( panf, panel, "Sides" );
   ctl[ 12 ] = INT_CTL( panf, panel, "Segments" );
   ctl[ 13 ] = FLOAT_CTL( panf, panel, "Radius" );
   ctl[ 14 ] = BOOL_CTL( panf, panel, "Base Plates" );

   w = 2 * panf->drawFuncs->textWidth( panel, "Polygons:" ) + 16;
   ctl[ 15 ] = CANVAS_CTL( panf, panel, "", w, 3 * dmet->textHeight );

   /* position all of the controls */

   x = CON_X( ctl[ 0 ] );
   x += CON_W( ctl[ 1 ] );
   h = CON_HOTH( ctl[ 1 ] ) + 4;

   w = CON_W( ctl[ 1 ] );
   y = CON_Y( ctl[ 0 ] ) + 3 * h / 2;
   MOVE_CON( ctl[ 1 ], x - w, y );

   w = CON_W( ctl[ 2 ] );
   MOVE_CON( ctl[ 2 ], x - w, y + 3 * h / 2 );

   w = CON_W( ctl[ 3 ] );
   y = CON_Y( ctl[ 2 ] );
   MOVE_CON( ctl[ 3 ], x - w, y + h );

   w = CON_W( ctl[ 4 ] );
   y = CON_Y( ctl[ 3 ] );
   MOVE_CON( ctl[ 4 ], ( x - w ) / 2, y + 3 * h / 2 );

   for ( i = 5; i < 10; i++ ) {
      w = CON_W( ctl[ i ] );
      y = CON_Y( ctl[ i - 1 ] );
      MOVE_CON( ctl[ i ], x - w, y + h );
   }

   x = x * 2 + 10;
   w = CON_W( ctl[ 10 ] );
   y = CON_Y( ctl[ 1 ] );
   MOVE_CON( ctl[ 10 ], x - w, y );

   w = CON_W( ctl[ 11 ] );
   y = CON_Y( ctl[ 10 ] );
   MOVE_CON( ctl[ 11 ], x - w, y + 3 * h / 2 );

   w = CON_W( ctl[ 12 ] );
   y = CON_Y( ctl[ 11 ] );
   MOVE_CON( ctl[ 12 ], x - w, y + h );

   w = CON_W( ctl[ 13 ] );
   y = CON_Y( ctl[ 12 ] );
   MOVE_CON( ctl[ 13 ], x - w, y + 3 * h / 2 );

   w = CON_W( ctl[ 14 ] );
   y = CON_Y( ctl[ 13 ] );
   MOVE_CON( ctl[ 14 ], x - w, y + 3 * h / 2 );

   w = CON_W( ctl[ 15 ] );
   y = CON_Y( ctl[ 14 ] );
   MOVE_CON( ctl[ 15 ], x - w, y + 2 * h );

   /* now that we know how much room the controls will take up, set the
      height of the panel */

   h = CON_Y( ctl[ 9 ] );
   h += CON_HOTH( ctl[ 9 ] );
   PAN_SETH( panf, panel, h + ph - 6 );

   /* initialize the controls */

   ctl_set();

   /* set the event handlers and user data */

   for ( i = 0; i < 15; i++ )
      CON_SETEVENT( ctl[ i ], ctl_event, NULL );

   ival.ptr.ptr = ctl_draw;
   ctl[ 15 ]->set( ctl[ 15 ], CTL_USERDRAW, &ival );
}


/*
======================================================================
get_user()

Creates and displays a modal user interface.  Called by BuildDNA().
====================================================================== */

int get_user( GlobalFunc *global )
{
   int result;

   panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !panf )
      return AFUNC_BADGLOBAL;

   panf->globalFun = global;

   panel = PAN_CREATE( panf, "DNA Builder" );

   ctl_create();
   ctl_event( NULL, NULL );

   result = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );
   ctl_get();

   PAN_KILL( panf, panel );

   return result;
}


/*
======================================================================
BuildDNA()

The activation function.  Called by Modeler.
====================================================================== */

XCALL_( int )
BuildDNA( long version, GlobalFunc *global, LWModCommand *local,
   void *serverData)
{
   ModData *md;

   if ( version != LWMODCOMMAND_VERSION ) return AFUNC_BADVERSION;

   if ( !get_user( global )) return AFUNC_OK;

   userabort = 0;
   if ( md = csInit( global, local )) {
      getlayers();
      setsurf();
      meshedit();
      if ( !userabort ) cmdedit();
      csDone();
   }

   return AFUNC_OK;
}


/*
======================================================================
The module descriptor.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWMODCOMMAND_CLASS, "DNA", BuildDNA },
   { NULL }
};
