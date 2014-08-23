/*
======================================================================
read.c

Functions for reading an FPBM image file.

Ernie Wright  4 Dec 00

  Support for both old and new formats (with long width and height in header) 9/24/02 Arnie Cachelin
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "flex.h"


/*
======================================================================
flexFree()

Free memory used by a Flex structure.
====================================================================== */

void flexFree( Flex *flex )
{
   int i;

   if ( flex ) {
      if ( flex->frame ) {
         for ( i = 0; i < flex->hdr.numFrames; i++ )
            if ( flex->frame[ i ].layer )
               free( flex->frame[ i ].layer );
         free( flex->frame );
      }
      free( flex );
   }
}


/*
======================================================================
flexReadBegin()

Opens and reads an FPBM image file.  Returns a pointer to a Flex
structure, or NULL if the file couldn't be loaded.  On failure, failID
and failpos can be used to diagnose the cause.

1.  If the file isn't an FPBM, failpos will contain 12 and failID will
    be unchanged.

2.  If an error occurs while reading, failID will contain the most
    recently read IFF chunk ID, and failpos will contain the value
    returned by ftell() at the time of the failure.

3.  If the file couldn't be opened, or an error occurs while reading
    the first 12 bytes, both failID and failpos will be unchanged.

If you don't need this information, failID and failpos can be NULL.
====================================================================== */

Flex *flexReadBegin( char *filename, unsigned int *failID, int *failpos )
{
   FILE *fp = NULL;
   Flex *flex;
   Frame *frame = NULL;
   Layer *layer = NULL;
   unsigned int id, formsize, type, cksize;
   int rlen, nframes = 0, nlayers = 0;

   /* open the file */

   fp = fopen( filename, "rb" );
   if ( !fp ) return NULL;

   /* read the first 12 bytes */

   set_flen( 0 );
   id       = getU4( fp );
   formsize = getU4( fp );
   type     = getU4( fp );
   if ( 12 != get_flen() ) {
      fclose( fp );
      return NULL;
   }

   /* is this a flex image? */

   if ( id != ID_FORM ) {
      fclose( fp );
      if ( failpos ) *failpos = 12;
      return NULL;
   }

   if ( type != ID_FPBM ) {
      fclose( fp );
      if ( failpos ) *failpos = 12;
      return NULL;
   }

   /* allocate a Flex */

   flex = calloc( 1, sizeof( Flex ));
   if ( !flex ) goto Fail;

   /* get the first chunk header */

   id = getU4( fp );
   cksize = getU4( fp );
   if ( 0 > get_flen() ) goto Fail;

   /* process chunks as they're encountered */

   while ( 1 ) {
      set_flen( 0 );

      switch ( id )
      {
         case ID_FPHD:
			if(cksize==28) // Older file version
			{
				flex->hdr.width           = getI2( fp );
				flex->hdr.height          = getI2( fp );
			}
			else // cksize==32
			{
				flex->hdr.width           = getI4( fp );
				flex->hdr.height          = getI4( fp );
			}
			flex->hdr.numLayers       = getI2( fp );
			flex->hdr.numFrames       = getI2( fp );
			flex->hdr.numBuffers      = getI2( fp );
			flex->hdr.flags           = getI2( fp );
			flex->hdr.srcLayerDepth   = getI2( fp );
			skipbytes( fp, 2 );
			flex->hdr.pixelAspect     = getF4( fp );
			flex->hdr.pixelWidth      = getF4( fp );
			flex->hdr.framesPerSecond = getF4( fp );
			frame = calloc( flex->hdr.numFrames, sizeof( Frame ));
			if ( !frame ) goto Fail;
			flex->frame = frame;
			break;

         case ID_FLEX:
            if ( !frame ) goto Fail;
            if ( nframes >= flex->hdr.numFrames ) goto Fail;
            nlayers = getI2( fp );
            layer = calloc( nlayers, sizeof( Layer ));
            if ( !layer ) goto Fail;
            frame[ nframes ].layer = layer;
            frame[ nframes ].hdr.numLayers = nlayers;
            nlayers = 0;
            ++nframes;
            break;

         case ID_LYHD:
            if ( !layer ) goto Fail;
            if ( nlayers >= frame[ nframes - 1 ].hdr.numLayers ) goto Fail;
            layer[ nlayers ].w               = flex->hdr.width;
            layer[ nlayers ].h               = flex->hdr.height;
            layer[ nlayers ].hdr.flags       = getI2( fp );
            layer[ nlayers ].hdr.layerType   = getI2( fp );
            layer[ nlayers ].hdr.layerDepth  = getI2( fp );
            layer[ nlayers ].hdr.compression = getI2( fp );
            layer[ nlayers ].hdr.blackPoint  = getF4( fp );
            layer[ nlayers ].hdr.whitePoint  = getF4( fp );
            layer[ nlayers ].hdr.gamma       = getF4( fp );
            break;

         case ID_LAYR:
            if ( !layer ) goto Fail;
            layer[ nlayers ].offset = ftell( fp );
            layer[ nlayers ].size   = cksize;
            ++nlayers;
            break;

         default:
            break;
      }

      /* error while reading current subchunk? */

      rlen = get_flen();
      if ( rlen < 0 || rlen > cksize ) goto Fail;

      /* skip unread parts of the current subchunk */

      cksize += cksize & 1;
      if ( rlen < cksize )
         fseek( fp, cksize - rlen, SEEK_CUR );

      /* end of the file? */

      if ( formsize <= ftell( fp ) - 8 ) break;

      /* get the next chunk header */

      set_flen( 0 );
      id = getU4( fp );
      cksize = getU4( fp );
      if ( 8 != get_flen() ) goto Fail;
   }

   flex->fp = fp;
   return flex;

Fail:
   if ( failID ) *failID = id;
   if ( fp ) {
      if ( failpos ) *failpos = ftell( fp );
      fclose( fp );
   }
   flexFree( flex );
   return NULL;
}


/*
======================================================================
flexFindLayer()

Return the layer that matches the frame number and layer type.
====================================================================== */

Layer *flexFindLayer( Flex *flex, int fnum, int layerType )
{
   Layer *layer = NULL;
   int i;

   if ( fnum < 0 || fnum > flex->hdr.numFrames - 1 )
      return NULL;

   for ( i = 0; i < flex->frame[ fnum ].hdr.numLayers; i++ )
      if ( flex->frame[ fnum ].layer[ i ].hdr.layerType == layerType ) {
         layer = &flex->frame[ fnum ].layer[ i ];
         break;
      }

   return layer;
}


/*
======================================================================
flexLayerSize()

Return the size in bytes of the uncompressed layer data.
====================================================================== */

int flexLayerSize( Flex *flex, int fnum, int layerType )
{
   Layer *layer;

   layer = flexFindLayer( flex, fnum, layerType );
   if ( layer )
      return layer->hdr.layerDepth * layer->w * layer->h;
   else
      return 0;
}


/*
======================================================================
unpackRLE()

Decompress run-length encoded bytes.
====================================================================== */

static int unpackRLE( char **psrc, char *dst, int size, int step )
{
   int c, n;
   char *src = *psrc;

   while ( size > 0 ) {
      n = *src++;

      if ( n >= 0 ) {
         ++n;
         size -= n;
         if ( size < 0 ) return 0;
         while ( n-- ) {
            *dst = *src++;
            dst += step;
         }
      }
      else {
         n = -n + 1;
         size -= n;
         if ( size < 0 ) return 0;
         c = *src++;
         while ( n-- ) {
            *dst = c;
            dst += step;
         }
      }
   }
   *psrc = src;
   return 1;
}


/*
======================================================================
unpackDelta()

Decompress delta encoded bytes.
====================================================================== */

static int unpackDelta( char *src, char *dst, int size, int vstep,
   int hstep )
{
   int n, nn;

   while ( size > 0 ) {
      n = *src++;
      --size;

      if ( n < 0 )
         dst += -n * vstep;
      else {
         for ( ; n >= 0; n-- ) {
            nn = *src++;
            --size;
            if ( nn < 0 )
               nn = -nn;
            else {
               ++nn;
               if ( !unpackRLE( &src, dst, nn, hstep ))
                  return 0;
            }
            dst += nn * hstep;
         }
      }
   }

   return 1;
}


/*
======================================================================
flexReadLayer()

Fill in the dst buffer with pixels from the layer matching the frame
number and layer type.  The destination buffer must be at least as
large as the number of bytes returned by flexLayerSize().
====================================================================== */

Layer *flexReadLayer( Flex *flex, int fnum, int layerType, char *dst )
{
   Layer *layer;
   char *buf, *src, *dst0;
   int i, rowbytes, ok;


   layer = flexFindLayer( flex, fnum, layerType );
   if ( !layer ) return NULL;

   if ( !flex->fp ) return NULL;
   fseek( flex->fp, layer->offset, SEEK_SET );

   if ( layer->hdr.compression == NoCompression ) {
      ok = fread( dst, layer->size, 1, flex->fp );
      if ( ok && ( layer->hdr.layerDepth > 1 ))
         revbytes( dst, layer->hdr.layerDepth, layer->w * layer->h );
      return layer;
   }

   buf = malloc( layer->size );
   if ( !buf ) return NULL;

   if ( 1 != fread( buf, layer->size, 1, flex->fp )) {
      free( buf );
      return NULL;
   }

   src = buf;
   dst0 = dst;
   rowbytes = layer->hdr.layerDepth * layer->w;

   switch ( layer->hdr.compression ) {
      case HorizontalRLE:
         for ( i = 0; i < layer->h; i++ ) {
            ok = unpackRLE( &src, dst, rowbytes, 1 );
            if ( !ok ) break;
            dst += rowbytes;
         }
         break;

      case VerticalRLE:
         for ( i = 0; i < rowbytes; i++ ) {
            ok = unpackRLE( &src, dst, layer->h, rowbytes );
            if ( !ok ) break;
            dst++;
         }
         break;

      case HorizontalDelta:
         revbytes( dst, layer->hdr.layerDepth, layer->w * layer->h );
         ok = unpackDelta( src, dst, rowbytes * layer->h, 1, rowbytes );
         break;

      case VerticalDelta:
         revbytes( dst, layer->hdr.layerDepth, layer->w * layer->h );
         ok = unpackDelta( src, dst, rowbytes * layer->h, rowbytes, 1 );
         break;

      default:
         ok = 0;
         break;
   }

   free( buf );
   if ( ok ) {
      revbytes( dst0, layer->hdr.layerDepth, layer->w * layer->h );
      return layer;
   }
   return NULL;
}


/*
======================================================================
flexReadDone()

Free resources obtained by flexReadBegin().
====================================================================== */

void flexReadDone( Flex *flex )
{
   if ( flex->fp ) fclose( flex->fp );
   flexFree( flex );
}


/*
======================================================================
flexLayerToByte()

Convert layer data to byte levels.  This is useful when you need to
display the layer.  This converts only a few common layer data types.

INPUTS
   layer       the layer to convert
   src         the layer values
   dst         buffer to receive converted layer data
   drowbytes   number of bytes in one scanline of dst
   dstep       bytes between successive layer values in dst

For example, a 24-bit Windows DIB stores pixels as

   B G R B G R B G R ...

The raster is longword-aligned, so drowbytes would be 3 * w, rounded
up to the next multiple of 4.  dstep would be 3, since each B (blue)
byte, for example, is offset from the previous one by 3 bytes.  The
dst pointer would be offset by 1 for the G (green) layer and by 2 for
the red layer.

A final complication is that DIBs are "upside-down," storing the
bottom scanline first.  So set dst to the start of the last (top)
scanline (+1 for G, +2 for R), and make drowbytes negative.

When you don't have to worry about scanline alignment, upside-down
scanlines, or interleaved color channels, the conversion is quite
straightforward.  Set drowbytes = layer->w, and dstep = 1.

When the layer is float-valued, the black and white points are used to
define the range, and the value is gamma corrected if the layer gamma
isn't 1.0.
====================================================================== */

void flexLayerToByte( Layer *layer, void *src, unsigned char *dst,
   int drowbytes, int dstep )
{
   int x, y, c;
   float r, *pf, f, g;
   unsigned short *pw;
   unsigned char *pb, *qb;


   switch ( layer->hdr.layerDepth ) {
      case 1:
         pb = ( unsigned char * ) src;
         for ( y = 0; y < layer->h; y++ ) {
            qb = dst + y * drowbytes;
            for ( x = 0; x < layer->w; x++, pb++, qb += dstep )
               *qb = *pb;
         }
         break;

      case 2:
         pw = ( unsigned short * ) src;
         for ( y = 0; y < layer->h; y++ ) {
            qb = dst + y * drowbytes;
            for ( x = 0; x < layer->w; x++, pw++, qb += dstep )
               *qb = *pw >> 8;
         }
         break;

      case 4:
         if ( layer->hdr.flags & Layer_FP ) {
            pf = ( float * ) src;
            r = layer->hdr.whitePoint - layer->hdr.blackPoint;
            if ( layer->hdr.gamma > 0.0f && layer->hdr.gamma != 1.0f )
               g = 1.0f / layer->hdr.gamma;
            else
               g = 0.0f;

            for ( y = 0; y < layer->h; y++ ) {
               qb = dst + y * drowbytes;
               for ( x = 0; x < layer->w; x++, pf++, qb += dstep ) {
                  f = ( *pf - layer->hdr.blackPoint ) / r;
                  if ( g != 0.0f ) f = ( float ) pow( f, g );
                  c = ( int )( 255.0f * f );
                  if ( c > 255 ) c = 255;
                  if ( c < 0 ) c = 0;
                  *qb = c;
               }
            }
         }
         break;
   }
}


/*
======================================================================
flexLayerToWord()

Like flexLayerToByte, but converts to word levels.  The destination is
an array of words, and the destination row width and step are word
counts.

Adobe Photoshop and Apple QuickDraw support 16 bits per image channel,
for example.
====================================================================== */

void flexLayerToWord( Layer *layer, void *src, unsigned short *dst,
   int drowwords, int dstep )
{
   int x, y, c;
   float r, *pf, f, g;
   unsigned short *pw, *qw;
   unsigned char *pb;


   switch ( layer->hdr.layerDepth ) {
      case 1:
         pb = ( unsigned char * ) src;
         for ( y = 0; y < layer->h; y++ ) {
            qw = dst + y * drowwords;
            for ( x = 0; x < layer->w; x++, pb++, qw += dstep )
               *qw = *pb * 257;
         }
         break;

      case 2:
         pw = ( unsigned short * ) src;
         for ( y = 0; y < layer->h; y++ ) {
            qw = dst + y * drowwords;
            for ( x = 0; x < layer->w; x++, pw++, qw += dstep )
               *qw = *pw;
         }
         break;

      case 4:
         if ( layer->hdr.flags & Layer_FP ) {
            pf = ( float * ) src;
            r = layer->hdr.whitePoint - layer->hdr.blackPoint;
            if ( layer->hdr.gamma > 0.0f && layer->hdr.gamma != 1.0f )
               g = 1.0f / layer->hdr.gamma;
            else
               g = 0.0f;

            for ( y = 0; y < layer->h; y++ ) {
               qw = dst + y * drowwords;
               for ( x = 0; x < layer->w; x++, pf++, qw += dstep ) {
                  f = ( *pf - layer->hdr.blackPoint ) / r;
                  if ( g != 0.0f ) f = ( float ) pow( f, g );
                  c = ( int )( 65535.0f * f );
                  if ( c > 65535 ) c = 65535;
                  if ( c < 0 ) c = 0;
                  *qw = c;
               }
            }
         }
         break;
   }
}
