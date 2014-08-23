/*
======================================================================
iff.c

Ernie Wright  22 Mar 00

LightWave IFF-ILBM loader/saver plug-in.
====================================================================== */

#include <lwserver.h>
#include <lwimageio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iff.h"


/*
======================================================================
load_24()

Send LW the 24-bit RGB image in pic.  Called by ImageLoad().
====================================================================== */

static void load_24( LWImageLoaderLocal *local, PicInfo *pic )
{
   LWImageProtocolID ip;
   unsigned char *p, *scanline;
   int x, y;


   scanline = malloc( pic->bmhd.w * 3 );
   if ( !scanline ) {
      local->result = IPSTAT_FAILED;
      return;
   }

   ip = local->begin( local->priv_data, LWIMTYP_RGB24 );
   if ( !ip ) {
      free( scanline );
      local->result = IPSTAT_FAILED;
      return;
   }

   LWIP_SETSIZE( ip, pic->bmhd.w, pic->bmhd.h );
   LWIP_ASPECT( ip, ( float ) pic->bmhd.xAspect / pic->bmhd.yAspect );

   for ( y = 0; y < pic->bmhd.h; y++ ) {
      if ( !GetRowILBM( pic )) break;
      p = scanline;
      for ( x = 0; x < pic->bmhd.w; x++ ) {
         *p++ = pic->rgb[ 0 ][ x ];
         *p++ = pic->rgb[ 1 ][ x ];
         *p++ = pic->rgb[ 2 ][ x ];
      }
      if ( LWIP_SENDLINE( ip, y, scanline )) break;
   }

   free( scanline );

   local->result = LWIP_DONE( ip, y < pic->bmhd.h ? IPSTAT_FAILED : IPSTAT_OK );
   local->done( local->priv_data, ip );
}


/*
======================================================================
load_gray()

Send LW the grayscale image in pic.  Called by ImageLoad().
====================================================================== */

static void load_gray( LWImageLoaderLocal *local, PicInfo *pic )
{
   LWImageProtocolID ip = NULL;
   int y;

   ip = local->begin( local->priv_data, LWIMTYP_GREY8 );
   if ( !ip ) {
      local->result = IPSTAT_FAILED;
      return;
   }

   LWIP_SETSIZE( ip, pic->bmhd.w, pic->bmhd.h );
   LWIP_ASPECT( ip, ( float ) pic->bmhd.xAspect / pic->bmhd.yAspect );

   for ( y = 0; y < pic->bmhd.h; y++ ) {
      if ( !GetRowILBM( pic )) break;
      if ( LWIP_SENDLINE( ip, y, pic->buf )) break;
   }

   local->result = LWIP_DONE( ip, y < pic->bmhd.h ? IPSTAT_FAILED : IPSTAT_OK );
   local->done( local->priv_data, ip );
}


/*
======================================================================
load_indexed()

Send LW the indexed color image in pic.  Called by ImageLoad().
====================================================================== */

static void load_indexed( LWImageLoaderLocal *local, PicInfo *pic )
{
   LWImageProtocolID ip = NULL;
   int y, i;

   ip = local->begin( local->priv_data, LWIMTYP_INDEX8 );
   if ( !ip ) {
      local->result = IPSTAT_FAILED;
      return;
   }

   LWIP_SETSIZE( ip, pic->bmhd.w, pic->bmhd.h );
   LWIP_ASPECT( ip, ( float ) pic->bmhd.xAspect / pic->bmhd.yAspect );
   LWIP_NUMCOLORS( ip, pic->ncolors );
   for ( i = 0; i < pic->ncolors; i++ )
      LWIP_SETMAP( ip, i, &pic->cmap[ i ].red );

   for ( y = 0; y < pic->bmhd.h; y++ ) {
      if ( !GetRowILBM( pic )) break;
      if ( LWIP_SENDLINE( ip, y, pic->buf )) break;
   }

   local->result = LWIP_DONE( ip, y < pic->bmhd.h ? IPSTAT_FAILED : IPSTAT_OK );
   local->done( local->priv_data, ip );
}


/*
======================================================================
ImageLoad()

Handle request to load image.  Called by LW.
====================================================================== */

XCALL_( static int )
ImageLoad( long version, GlobalFunc *global, LWImageLoaderLocal *local,
   void *serverData )
{
   PicInfo *pic = NULL;
   int result;

   if ( version != LWIMAGELOADER_VERSION ) return AFUNC_BADVERSION;

   pic = ReadILBM( local->filename, &result );

   if ( !pic ) {
      switch ( result ) {
         case EPNOFILE:  local->result = IPSTAT_BADFILE;  break;
         case EPNOMEM:   local->result = IPSTAT_FAILED;   break;
         default:        local->result = IPSTAT_NOREC;    break;
      }
      return AFUNC_OK;
   }

   if (( pic->bmhd.nPlanes == 24 ) || ( pic->camg & CAMG_HAM ))
      load_24( local, pic );
   else if ( pic->isgray )
      load_gray( local, pic );
   else
      load_indexed( local, pic );

   FreeILBM( pic );

   return AFUNC_OK;
}


/*
======================================================================
SetSize()

Image saver callback.  This is called to tell us the width and height
of the image to be saved.
====================================================================== */

XCALL_( static void )
SetSize( PicInfo *pic, int w, int h )
{
   if ( pic->result != IPSTAT_OK ) return;

   pic->bmhd.w = pic->bmhd.pw = ( unsigned short ) w;
   pic->bmhd.h = pic->bmhd.ph = ( unsigned short ) h;

   pic->rowsize = (( pic->bmhd.w + 15 ) >> 3 ) & 0xFFFE;
   pic->buf = calloc( pic->rowsize, 32 );
   pic->body = pic->bp = calloc( pic->rowsize, 32 );
   if ( !pic->buf || !pic->body ) {
      pic->result = IPSTAT_FAILED;
      return;
   }

   pic->rgb[ 0 ] = pic->buf + pic->rowsize * 8;
   pic->rgb[ 1 ] = pic->buf + pic->rowsize * 16;
   pic->rgb[ 2 ] = pic->buf + pic->rowsize * 24;
}


/*
======================================================================
SetParam()

Image saver callback.  This is called to tell us other information
about the image to be saved.
====================================================================== */

XCALL_( static void )
SetParam( PicInfo *pic, LWImageParam param, int ival, float fval )
{
   int aw, ah, i;

   if ( pic->result != IPSTAT_OK ) return;

   switch ( param )
   {
      case LWIMPAR_ASPECT:
         find_aspect( fval, &aw, &ah );
         pic->bmhd.xAspect = ( unsigned char ) aw;
         pic->bmhd.yAspect = ( unsigned char ) ah;
         break;

      case LWIMPAR_NUMCOLS:
         pic->ncolors = ival;
         pic->cmap = calloc( ival, sizeof( RGBTriple ));
         if ( !pic->cmap )
            pic->result = IPSTAT_FAILED;
         for ( i = 7; i >= 0; i-- )
            if ( pic->ncolors > ( 1 << i )) {
               pic->bmhd.nPlanes = i + 1;
               break;
            }
         break;

      default:
         break;
   }
}


/*
======================================================================
SetMap()

Image saver callback.  This is called to set a color table entry.
We already know the size of the table because of a SetParam() call
with an LWIMPAR_NUMCOLS image parameter.
====================================================================== */

XCALL_( static void )
SetMap( PicInfo *pic, int n, const unsigned char color[ 3 ] )
{
   if ( pic->result != IPSTAT_OK ) return;

   pic->cmap[ n ].red = color[ 0 ];
   pic->cmap[ n ].green = color[ 1 ];
   pic->cmap[ n ].blue = color[ 2 ];
}


/*
======================================================================
SendLine()

Image saver callback.  Save one scanline of the image.  The form of
the scanline data will vary, depending on the pixel type requested
when the ImageProtocol was created.  We asked for LWIMTYP_RGB24,
LWIMTYP_GREY8 or LWIMTYP_INDEX8.  For RGB24, the scanline is an array
of bytes containing r g b r g b ...

When SendLine() is called with a scanline index (y value) of 0, we
write the image header before writing the first scanline.  We know at
that point that our image parameter callbacks have been called and we
have what we need to write the header.
====================================================================== */

XCALL_( static int )
SendLine( PicInfo *pic, int y, unsigned char *pixel )
{
   int i;

   if ( pic->result != IPSTAT_OK ) return pic->result;

   if ( y == 0 )
      if ( !InitILBM( pic )) {
         pic->result = IPSTAT_FAILED;
         return IPSTAT_FAILED;
      }

   if ( pic->bmhd.nPlanes == 24 )
      for ( i = 0; i < pic->bmhd.w; i++ ) {
         pic->rgb[ 0 ][ i ] = *pixel++;
         pic->rgb[ 1 ][ i ] = *pixel++;
         pic->rgb[ 2 ][ i ] = *pixel++;
      }
   else
      memcpy( pic->rgb[ 0 ], pixel, pic->bmhd.w );

   if ( !PutRowILBM( pic ))
      pic->result = IPSTAT_FAILED;

   return pic->result;
}


/*
======================================================================
Done()

Image saver callback.  Finish saving the image.
====================================================================== */

XCALL_( static int )
Done( PicInfo *pic, int error )
{
   if ( error )
      pic->result = IPSTAT_FAILED;
   else if ( !CloseILBM( pic ))
      pic->result = IPSTAT_FAILED;

   return pic->result;
}


/*
======================================================================
ImageSave()

Image saver activation function.
====================================================================== */

XCALL_( static int )
ImageSave( long version, GlobalFunc *global, LWImageSaverLocal *local,
   void *serverData )
{
   LWImageProtocol ip;
   PicInfo *pic;

   /* check the version */

   if ( version != LWIMAGESAVER_VERSION ) return AFUNC_BADVERSION;

   /* figure out what kind of pixel data we want */

   switch ( local->type ) {
      case LWIMTYP_RGB24:
      case LWIMTYP_RGBA32:
      case LWIMTYP_RGBFP:
      case LWIMTYP_RGBAFP:  ip.type = LWIMTYP_RGB24;   break;

      case LWIMTYP_GREY8:
      case LWIMTYP_GREYFP:  ip.type = LWIMTYP_GREY8;   break;

      case LWIMTYP_INDEX8:  ip.type = LWIMTYP_INDEX8;  break;

      default:
         local->result = IPSTAT_FAILED;
         return AFUNC_OK;
   }

   /* allocate a PicInfo */

   pic = calloc( 1, sizeof( PicInfo ));
   if ( !pic ) return IPSTAT_FAILED;

   /* store the filename */

   pic->filename = malloc( strlen( local->filename ) + 1 );
   if ( !pic->filename ) {
      FreeILBM( pic );
      local->result = IPSTAT_FAILED;
      return AFUNC_OK;
   }
   strcpy( pic->filename, local->filename );

   /* open the file */

   pic->fp = fopen( local->filename, "wb" );
   if ( !pic->fp ) {
      FreeILBM( pic );
      local->result = IPSTAT_FAILED;
      return AFUNC_OK;
   }

   /* set other PicInfo fields */

   pic->isgray           = ( ip.type == LWIMTYP_GREY8 );
   pic->ncolors          = ( ip.type == LWIMTYP_INDEX8 ) ? 256 : 0;
   pic->bmhd.nPlanes     = ( ip.type == LWIMTYP_RGB24 ) ? 24 : 8;
   pic->bmhd.masking     = mskNone;
   pic->bmhd.compression = cmpByteRun1;

   /* initialize the image protocol */

   ip.priv_data = pic;
   ip.setSize   = SetSize;
   ip.setParam  = SetParam;
   ip.setMap    = SetMap;
   ip.sendLine  = SendLine;
   ip.done      = Done;

   /* Save the image.  This will cause our callbacks to be called.  It
      won't return until image saving is complete. */

   local->sendData( local->priv_data, &ip, 0 );

   /* done saving */

   local->result = pic->result;

   FreeILBM( pic );

   return AFUNC_OK;
}


/*
======================================================================
The server description.  Note that the saver includes a filename
extension in its name.  LightWave uses this to automatically append an
extension to filenames.
====================================================================== */

ServerRecord ServerDesc[] = {
   { LWIMAGELOADER_CLASS, "Demo_IFF-ILBM",       ImageLoad },
   { LWIMAGESAVER_CLASS,  "Demo_IFF-ILBM(.iff)", ImageSave },
   { NULL }
};
