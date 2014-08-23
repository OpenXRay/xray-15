/*
======================================================================
zbcomp6.c

Composite an image sequence using a z-buffer.  This version is for
LW 6 and later.

Ernie Wright  22 Jul 00
Chris "Xenon" Hanson  7 Aug 96
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwserver.h>
#include <lwfilter.h>
#include <lwpanel.h>
#include <lwimage.h>
#include "zbuf6.h"


/* the instance data structure */

typedef struct {
   LWImageID   id;
   int         resample;
   float       dx, dy;
} ZBImage;

typedef struct {
   GlobalFunc *global;
   ZBImage     zimage;
   ZBImage     cimage;
   char        desc[ 260 ];
} ZBCompInst;


/* a global we'll use a lot */

LWImageList *imglist;


XCALL_( static ZBCompInst * )
Create( void *priv, void *context, LWError *err )
{
   ZBCompInst *zbc;

   zbc = calloc( 1, sizeof( ZBCompInst ));
   if ( !zbc ) {
      *err = ZCOMP_NAME ":  Couldn't allocate instance data.";
      return NULL;
   }

   zbc->global = ( GlobalFunc * ) priv;
   return zbc;
}


XCALL_( static void )
Destroy( ZBCompInst *zbc )
{
   if ( zbc ) free( zbc );
}


XCALL_( static LWError )
Copy( ZBCompInst *to, ZBCompInst *from )
{
   *to = *from;
   return NULL;
}


XCALL_( static LWError )
Load( ZBCompInst *zbc, const LWLoadState *ls )
{
   zbc->zimage.id = imglist->sceneLoad( ls );
   zbc->cimage.id = imglist->sceneLoad( ls );
   return NULL;
}


XCALL_( static LWError )
Save( ZBCompInst *zbc, const LWSaveState *ls )
{
   if ( zbc->zimage.id && zbc->cimage.id ) {
      imglist->sceneSave( ls, zbc->zimage.id );
      imglist->sceneSave( ls, zbc->cimage.id );
   }
   return NULL;
}


XCALL_( static const char * )
DescLn( ZBCompInst *zbc )
{
   sprintf( zbc->desc, ZCOMP_NAME ":  %s, %s",
      zbc->zimage.id ? imglist->name( zbc->zimage.id ) : "(none)",
      zbc->cimage.id ? imglist->name( zbc->cimage.id ) : "(none)" );

   return zbc->desc;
}


/*
======================================================================
Init()
====================================================================== */

XCALL_( static LWError )
Init( ZBCompInst *zbc, int mode )
{
   LWSceneInfo *lws;
   int w, h;

   /* do we have the z-buffer and the image? */

   if ( !zbc->zimage.id )
      return ZCOMP_NAME ":  No depth buffer file specified.";

   if ( !zbc->cimage.id )
      return ZCOMP_NAME ":  No image to composite.";

   /* calculate scale factors if the image sizes don't match the frame */

   if ( lws = zbc->global( LWSCENEINFO_GLOBAL, GFUSE_TRANSIENT )) {
      imglist->size( zbc->zimage.id, &w, &h );
      if ( w != lws->frameWidth || h != lws->frameHeight ) {
         zbc->zimage.resample = 1;
         zbc->zimage.dx = ( float ) w / lws->frameWidth;
         zbc->zimage.dy = ( float ) h / lws->frameHeight;
      }
      else
         zbc->zimage.resample = 0;

      imglist->size( zbc->cimage.id, &w, &h );
      if ( w != lws->frameWidth || h != lws->frameHeight ) {
         zbc->cimage.resample = 1;
         zbc->cimage.dx = ( float ) w / lws->frameWidth;
         zbc->cimage.dy = ( float ) h / lws->frameHeight;
      }
      else
         zbc->cimage.resample = 0;
   }

   return NULL;
}


XCALL_( static void )
Cleanup( ZBCompInst *zbc )
{
}


XCALL_( static LWError )
NewTime( ZBCompInst *zbc, LWFrame f, LWTime t )
{
   return NULL;
}


XCALL_( static int )
Flags( ZBCompInst *zbc )
{
   return 1 << LWBUF_DEPTH;
}


/*
======================================================================
Evaluate()
====================================================================== */

XCALL_( static void )
Evaluate( ZBCompInst *zbc, const LWPixelAccess *pa )
{
   float out[ 4 ], z, lwz;
   int x, y;

   /* get LW RGBA and Z */

   pa->getVal( LWBUF_RED, 4, out );
   pa->getVal( LWBUF_DEPTH, 1, &lwz );

   /* get our Z */

   if ( zbc->zimage.resample ) {
      x = ( int )( pa->sx * zbc->zimage.dx + 0.5 );
      y = ( int )( pa->sy * zbc->zimage.dy + 0.5 );
   }
   else {
      x = ( int )( pa->sx + 0.5 );
      y = ( int )( pa->sy + 0.5 );
   }

   z = imglist->luma( zbc->zimage.id, x, y );

   /* if our pixel is closer, replace the rendered pixel */

   if ( z < lwz ) {
      if ( zbc->cimage.resample ) {
         x = ( int )( pa->sx * zbc->cimage.dx + 0.5 );
         y = ( int )( pa->sy * zbc->cimage.dy + 0.5 );
      }
      else {
         x = ( int )( pa->sx + 0.5 );
         y = ( int )( pa->sy + 0.5 );
      }

      imglist->RGB( zbc->cimage.id, x, y, out );
      lwz = z;
   }

   pa->setRGBA( out );
   pa->setVal( LWBUF_DEPTH, 1, &lwz );
}


/*
======================================================================
ZComp_Handler()
====================================================================== */

XCALL_( int )
ZComp_Handler( long version, GlobalFunc *global, LWPixelFilterHandler *local,
   void *serverData )
{
   if ( version != LWPIXELFILTER_VERSION ) return AFUNC_BADVERSION;

   imglist = global( LWIMAGELIST_GLOBAL, GFUSE_TRANSIENT );
   if ( !imglist ) return AFUNC_BADGLOBAL;

   local->inst->priv = global;

   local->inst->create  = Create;
   local->inst->destroy = Destroy;
   local->inst->copy    = Copy;
   local->inst->load    = Load;
   local->inst->save    = Save;
   local->inst->descln  = DescLn;

   if ( local->rend ) {
      local->rend->init    = Init;
      local->rend->cleanup = Cleanup;
      local->rend->newTime = NewTime;
   }

   local->evaluate = Evaluate;
   local->flags    = Flags;

   return AFUNC_OK;
}


/*
======================================================================
get_user()

Display a panel that lets the user choose the comp image and the
z-buffer file.
====================================================================== */

XCALL_( static LWError )
get_user( ZBCompInst *zbc )
{
   LWPanelFuncs *panf;
   LWPanelID panel;
   LWPanControlDesc desc;
   LWValue ival = { LWT_INTEGER };
   LWControl *ctl[ 3 ];
   char *tip[] = {
      "Use the Image Editor to load",
      "both of these.",
      NULL
   };
   int i, w, w1, ok;

   panf = zbc->global( LWPANELFUNCS_GLOBAL, GFUSE_TRANSIENT );
   if ( !panf ) return ZCOMP_NAME ":  Couldn't get panel functions.";

   if( !( panel = PAN_CREATE( panf, "3D Nature's IFF-ZBUF Compositor V2.0" )))
      return ZCOMP_NAME ":  Couldn't create the panel.";

   ctl[ 0 ] = ITEM_CTL( panf, panel, "Z-Buffer", zbc->global, LWI_IMAGE );
   ctl[ 1 ] = ITEM_CTL( panf, panel, "Comp Image", zbc->global, LWI_IMAGE );
   ctl[ 2 ] = TEXT_CTL( panf, panel, "Note: ", tip );

   /* align */

   w1 = 40 + CON_LW( ctl[ 1 ] );
   for ( i = 0; i < 3; i++ ) {
      w = CON_LW( ctl[ i ] );
      ival.intv.value = w1 - w;
      ctl[ i ]->set( ctl[ i ], CTL_X, &ival );
   }

   SET_INT( ctl[ 0 ], ( int ) zbc->zimage.id );
   SET_INT( ctl[ 1 ], ( int ) zbc->cimage.id );

   ok = panf->open( panel, PANF_BLOCKING | PANF_CANCEL );

   if ( ok ) {
      ctl[ 0 ]->get( ctl[ 0 ], CTL_VALUE, &ival );
      zbc->zimage.id = ( LWItemID ) ival.intv.value;

      ctl[ 1 ]->get( ctl[ 1 ], CTL_VALUE, &ival );
      zbc->cimage.id = ( LWItemID ) ival.intv.value;
   }

   PAN_KILL( panf, panel );

   return NULL;
}


/*
======================================================================
ZComp_Interface()
====================================================================== */

XCALL_( int )
ZComp_Interface( long version, GlobalFunc *global, LWInterface *local,
   void *serverData )
{
   if ( version != LWINTERFACE_VERSION ) return AFUNC_BADVERSION;

   local->panel = NULL;
   local->options = get_user;
   local->command = NULL;

   return AFUNC_OK;
}


ServerRecord ServerDesc[] = {
   { LWPIXELFILTER_HCLASS, ZCOMP_NAME, ZComp_Handler },
   { LWPIXELFILTER_ICLASS, ZCOMP_NAME, ZComp_Interface },
   { LWIMAGEFILTER_HCLASS, ZSAVE_NAME, ZSave_Handler },
   { LWIMAGEFILTER_ICLASS, ZSAVE_NAME, ZSave_Interface },
   { LWIMAGELOADER_CLASS,  ZLOAD_NAME, ZLoad },
   { NULL }
};
