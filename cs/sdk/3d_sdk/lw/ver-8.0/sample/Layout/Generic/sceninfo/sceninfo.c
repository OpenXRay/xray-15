/*
======================================================================
sceninfo.c

Ernie Wright  12 Nov 00
====================================================================== */

#include <stdio.h>
#include <string.h>
#include <lwserver.h>
#include <lwgeneric.h>
#include <lwrender.h>
#include <lwpanel.h>
#include <lwhost.h>


int show_pan( LWPanelFuncs *panf, LWSceneInfo *info )
{
   static char *title[] = {
      "Name",
      "Filename",
      "Points, Polygons",
      "Render Type",
      "Frame Start, End, Step",
      "Frames Per Second",
      "Frame Size",
      "Pixel Aspect",
      "Samples Per Pixel",
      "Limited Region",
      "Recursion Depth",
      "Camera ID"
   };

   static char *rentyp[] = {
      "Wire",
      "Quick Shade",
      "Realistic"
   };

   LWPanControlDesc desc;
   LWValue
      ival = { LWT_INTEGER },
      sval = { LWT_STRING  };
   LWPanelID panel;
   LWControl *ctl[ 20 ];
   char buf[ 256 ];
   int i, w;

   if( !( panel = PAN_CREATE( panf, "Scene Info" )))
      return 0;

   for ( i = 0; i < 12; i++ )
      ctl[ i ] = STRRO_CTL( panf, panel, title[ i ], 30 );

   /* align */

   for ( i = 0; i < 12; i++ ) {
      w = CON_LW( ctl[ i ] );
      ival.intv.value = 150 - w;
      ctl[ i ]->set( ctl[ i ], CTL_X, &ival );
   }

   strcpy( buf, info->name );
   SET_STR( ctl[ 0 ], buf, sizeof( buf ));

   strcpy( buf, info->filename );
   SET_STR( ctl[ 1 ], buf, sizeof( buf ));

   sprintf( buf, "%d  %d", info->numPoints, info->numPolygons );
   SET_STR( ctl[ 2 ], buf, sizeof( buf ));

   switch ( info->renderType ) {
      case LWRTYPE_WIRE:       SET_STR( ctl[ 3 ], rentyp[ 0 ], 20 );  break;
      case LWRTYPE_QUICK:      SET_STR( ctl[ 3 ], rentyp[ 1 ], 20 );  break;
      case LWRTYPE_REALISTIC:  SET_STR( ctl[ 3 ], rentyp[ 2 ], 20 );  break;
   }

   sprintf( buf, "%d to %d step %d", info->frameStart, info->frameEnd,
      info->frameStep );
   SET_STR( ctl[ 4 ], buf, sizeof( buf ));

   sprintf( buf, "%g", info->framesPerSecond );
   SET_STR( ctl[ 5 ], buf, sizeof( buf ));

   sprintf( buf, "%d x %d", info->frameWidth, info->frameHeight );
   SET_STR( ctl[ 6 ], buf, sizeof( buf ));

   sprintf( buf, "%g", info->pixelAspect );
   SET_STR( ctl[ 7 ], buf, sizeof( buf ));

   sprintf( buf, "min %d  max %d", info->minSamplesPerPixel,
      info->maxSamplesPerPixel );
   SET_STR( ctl[ 8 ], buf, sizeof( buf ));

   sprintf( buf, "(%d, %d) - (%d, %d)",
      info->limitedRegion[ 0 ],
      info->limitedRegion[ 1 ],
      info->limitedRegion[ 2 ],
      info->limitedRegion[ 3 ] );
   SET_STR( ctl[ 9 ], buf, sizeof( buf ));

   sprintf( buf, "%d", info->recursionDepth );
   SET_STR( ctl[ 10 ], buf, sizeof( buf ));

   i = ( int ) info->renderCamera( 0.0 );
   sprintf( buf, "%d", i & 0x00FFFFFF );
   SET_STR( ctl[ 11 ], buf, sizeof( buf ));

   panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   PAN_KILL( panf, panel );

   return 1;
}


XCALL_( int )
ShowSceneInfo( long version, GlobalFunc *global, LWLayoutGeneric *local,
   void *serverData )
{
   LWPanelFuncs *panf;
   LWSceneInfo *info;

   if ( version != LWLAYOUTGENERIC_VERSION )
      return AFUNC_BADVERSION;

   info = global( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT );
   panf = global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !info || !panf ) return AFUNC_BADGLOBAL;

   panf->globalFun = global;
   show_pan( panf, info );

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWLAYOUTGENERIC_CLASS, "SceneInfo", ShowSceneInfo },
   { NULL }
};
