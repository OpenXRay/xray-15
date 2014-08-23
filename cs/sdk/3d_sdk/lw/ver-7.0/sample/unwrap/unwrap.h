/*
======================================================================
unwrap.h

Typedefs and prototypes for the Unwrap plug-in.

Ernie Wright  21 Aug 01
====================================================================== */

#ifndef UNWRAP_H
#define UNWRAP_H

typedef struct st_UnwrapParams {
   LWItemID       object;
   LWSurfaceID    surface;
   LWTextureID    texture;
   LWTLayerID     tlayer;
   LWImageID      image;
   int            proj;
   void *         vmap;
   int            width, height;
   char           filename[ 260 ];
   int            saver;
   unsigned char  fgcolor[ 4 ];
   int            fgoptions;
   unsigned char  bgcolor[ 4 ];
   int            bgoptions;
} UnwrapParams;

#define FG_COLOR   0
#define FG_INVERT  1
#define FG_LIGHTEN 2
#define FG_DARKEN  3

#define BG_COLOR   0
#define BG_IMAGE   1


int get_user( UnwrapParams *uwp );

#endif