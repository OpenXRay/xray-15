/*
======================================================================
write.c

Functions for writing an FPBM image file.

Ernie Wright  10 Dec 00
====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "flex.h"


/*
======================================================================
flexWriteBegin()

Open a file and prepare it to receive FPBM frames.  Returns true if
all goes well, otherwise false.
====================================================================== */

int flexWriteBegin( Flex *flex, char *filename )
{
   flex->fp = fopen( filename, "wb" );
   if ( !flex->fp ) return 0;

   set_flen( 0 );

   putU4( flex->fp, ID_FORM );
   putU4( flex->fp, 0 );
   putU4( flex->fp, ID_FPBM );

   putU4( flex->fp, ID_FPHD );
   putU4( flex->fp, 28 );
   putI2( flex->fp, flex->hdr.width );
   putI2( flex->fp, flex->hdr.height );
   putI2( flex->fp, flex->hdr.numLayers );
   putI2( flex->fp, flex->hdr.numFrames );
   putI2( flex->fp, flex->hdr.numBuffers );
   putI2( flex->fp, flex->hdr.flags );
   putI2( flex->fp, flex->hdr.srcLayerDepth );
   putI2( flex->fp, 0 );
   putF4( flex->fp, flex->hdr.pixelAspect );
   putF4( flex->fp, flex->hdr.pixelWidth );
   putF4( flex->fp, flex->hdr.framesPerSecond );

   return ( 48 == get_flen() );
}


/*
======================================================================
flexWriteFrame()

Write a FLEX structure to start a frame.
====================================================================== */

int flexWriteFrame( Flex *flex, int nlayers )
{
   set_flen( 0 );
   putU4( flex->fp, ID_FLEX );
   putU4( flex->fp, 2 );
   putI2( flex->fp, ( short ) nlayers );
   return ( 10 == get_flen() );
}


/*
======================================================================
packRLE()

Run-length encode bytes.
====================================================================== */

#define DUMP    0
#define RUN     1
#define MINRUN  3
#define MAXRUN  128
#define MAXDUMP 128

static int packRLE( char *src, char *dst, int size, int step )
{
   char c, lastc;
   int
      mode = DUMP,
      rstart = 0,
      putsize = 0,
      sp = 1,
      i;

   lastc = *src;
   size--;

   while ( size > 0 ) {
      c = *( src + sp * step );
      sp++;
      size--;

      switch ( mode ) {
         case DUMP:
            if ( sp > MAXDUMP ) {
               *dst++ = sp - 2;
               for ( i = 0; i < sp - 1; i++ )
                  *dst++ = *( src + i * step );
               putsize += sp;
               src += ( sp - 1 ) * step;
               sp = 1;
               rstart = 0;
               break;
            }

            if ( c == lastc ) {
               if (( sp - rstart ) >= MINRUN ) {
                  if ( rstart > 0 ) {
                     *dst++ = rstart - 1;
                     for ( i = 0; i < rstart; i++ )
                        *dst++ = *( src + i * step );
                     putsize += rstart + 1;
                  }
                  mode = RUN;
               }
               else if ( rstart == 0 ) mode = RUN;
            }
            else rstart = sp - 1;
            break;

         case RUN:
            if (( c != lastc ) || ( sp - rstart > MAXRUN )) {
               *dst++ = rstart + 2 - sp;
               *dst++ = lastc;
               putsize += 2;
               src += ( sp - 1 ) * step;
               sp = 1;
               rstart = 0;
               mode = DUMP;
            }
      }
      lastc = c;
   }

   switch ( mode ) {
      case DUMP:
         *dst++ = sp - 1;
         for ( i = 0; i < sp; i++ )
            *dst++ = *( src + i * step );
         putsize += sp + 1;
         break;

      case RUN:
         *dst++ = rstart + 1 - sp;
         *dst   = lastc;
         putsize += 2;
   }

   return putsize;
}


/*
======================================================================
flexWriteLayer()

Write the LayerHeader and the layer contents.
====================================================================== */

int flexWriteLayer( Flex *flex, Layer *layer, void *buf )
{
   char *src, *dst;
   int ok, wlen, pos, y, rowbytes;

   set_flen( 0 );
   putU4( flex->fp, ID_LYHD );
   putU4( flex->fp, 20ul );
   putI2( flex->fp, layer->hdr.flags );
   putI2( flex->fp, layer->hdr.layerType );
   putI2( flex->fp, layer->hdr.layerDepth );
   putI2( flex->fp, layer->hdr.compression );
   putF4( flex->fp, layer->hdr.blackPoint );
   putF4( flex->fp, layer->hdr.whitePoint );
   putF4( flex->fp, layer->hdr.gamma );

   wlen = get_flen();
   if ( wlen != 28 ) return 0;

   src = ( char * ) buf;
   putU4( flex->fp, ID_LAYR );

   if ( layer->hdr.compression == NoCompression ) {
      wlen = layer->hdr.layerDepth * layer->w * layer->h;
      putU4( flex->fp, wlen );
      if ( layer->hdr.layerDepth > 1 )
         revbytes( src, layer->hdr.layerDepth, layer->w * layer->h );
      ok = fwrite( src, wlen, 1, flex->fp );
      if ( ok && ( wlen & 1 ))
         fputc( 0, flex->fp );
      if ( layer->hdr.layerDepth > 1 )
         revbytes( src, layer->hdr.layerDepth, layer->w * layer->h );
      return ok;
   }

   rowbytes = layer->hdr.layerDepth * layer->w;
   layer->offset = ftell( flex->fp );
   putU4( flex->fp, 0 );
   if ( get_flen() < 0 ) return 0;
   set_flen( 0 );

   if ( layer->hdr.layerDepth > 1 )
      revbytes( buf, layer->hdr.layerDepth, layer->w * layer->h );

   switch ( layer->hdr.compression ) {
      case HorizontalRLE:
         dst = calloc( 2, rowbytes );
         if ( !dst ) return 0;
         for ( y = 0; y < layer->h; y++ ) {
            wlen = packRLE( src, dst, rowbytes, 1 );
            putbytes( flex->fp, dst, wlen );
            src += rowbytes;
         }
         free( dst );
         break;

      case VerticalRLE:
         dst = calloc( 2, layer->h );
         if ( !dst ) return 0;
         for ( y = 0; y < rowbytes; y++ ) {
            wlen = packRLE( src, dst, layer->h, rowbytes );
            putbytes( flex->fp, dst, wlen );
            src++;
         }
         free( dst );
         break;

      /* no delta encoding yet */

      default:
         return 0;
   }

   if ( layer->hdr.layerDepth > 1 )
      revbytes( buf, layer->hdr.layerDepth, layer->w * layer->h );

   wlen = get_flen();
   if ( wlen < 0 ) return 0;
   pos = ftell( flex->fp );
   fseek( flex->fp, layer->offset, SEEK_SET );
   putU4( flex->fp, wlen );
   fseek( flex->fp, pos, SEEK_SET );

   return 1;
}


/*
======================================================================
flexWriteDone()

Fill in the FORM size and close the file.
====================================================================== */

void flexWriteDone( Flex *flex )
{
   int pos;

   pos = ftell( flex->fp );
   fseek( flex->fp, 4, SEEK_SET );
   putU4( flex->fp, pos - 8 );
   fclose( flex->fp );
}


/*
======================================================================
flexByteToLayer()

Convert byte levels to layer data.  Useful when you need byte-valued
channels to be represented as floats.

INPUTS
   layer       the layer to convert
   src         the byte values
   dst         buffer to receive converted layer data
   drowbytes   number of bytes in one scanline of src
   dstep       bytes between successive channel values in src

For example, a 24-bit Windows DIB stores pixels as

   B G R B G R B G R ...

The raster is longword-aligned, so drowbytes would be 3 * w, rounded
up to the next multiple of 4.  dstep would be 3, since each B (blue)
byte, for example, is offset from the previous one by 3 bytes.  The
src pointer would be offset by 1 for the G (green) layer and by 2 for
the red layer.

A final complication is that DIBs are "upside-down," storing the
bottom scanline first.  So set src to the start of the last (top)
scanline (+1 for G, +2 for R), and make drowbytes negative.

When you don't have to worry about scanline alignment, upside-down
scanlines, or interleaved color channels, the conversion is quite
straightforward.  Set drowbytes = layer->w, and dstep = 1.

When the layer is float-valued, the black and white points are used to
define the range.  The source is assumed to have been gamma modified
if the layer gamma isn't 1.0.
====================================================================== */

void flexByteToLayer( Layer *layer, unsigned char *src, void *dst,
   int drowbytes, int dstep )
{
   int x, y;
   float r, *pf, f, g;
   unsigned short *pw;
   unsigned char *pb, *qb;


   switch ( layer->hdr.layerDepth ) {
      case 1:
         pb = ( unsigned char * ) dst;
         for ( y = 0; y < layer->h; y++ ) {
            qb = src + y * drowbytes;
            for ( x = 0; x < layer->w; x++, pb++, qb += dstep )
               *pb = *qb;
         }
         break;

      case 2:
         pw = ( unsigned short * ) dst;
         for ( y = 0; y < layer->h; y++ ) {
            qb = src + y * drowbytes;
            for ( x = 0; x < layer->w; x++, pw++, qb += dstep )
               *pw = *qb * 257;
         }
         break;

      case 4:
         if ( layer->hdr.flags & Layer_FP ) {
            pf = ( float * ) dst;
            r = layer->hdr.whitePoint - layer->hdr.blackPoint;
            if ( layer->hdr.gamma > 0.0f && layer->hdr.gamma != 1.0f )
               g = layer->hdr.gamma;
            else
               g = 0.0f;

            for ( y = 0; y < layer->h; y++ ) {
               qb = src + y * drowbytes;
               for ( x = 0; x < layer->w; x++, pf++, qb += dstep ) {
                  f = *qb / 255.0f;
                  if ( g ) f = ( float ) pow( f, g );
                  *pf = f * r + layer->hdr.blackPoint;
               }
            }
         }
         break;
   }
}


/*
======================================================================
flexWordToLayer()

Like flexByteToLayer, but converts from word levels.  The source is an
array of words, and the source row width and step are word counts.

Adobe Photoshop and Apple QuickDraw support 16 bits per image channel,
for example.
====================================================================== */

void flexWordToLayer( Layer *layer, unsigned short *src, void *dst,
   int drowwords, int dstep )
{
   int x, y;
   float r, *pf, f, g;
   unsigned short *pw, *qw;
   unsigned char *pb;


   switch ( layer->hdr.layerDepth ) {
      case 1:
         pb = ( unsigned char * ) dst;
         for ( y = 0; y < layer->h; y++ ) {
            qw = src + y * drowwords;
            for ( x = 0; x < layer->w; x++, pb++, qw += dstep )
               *pb = *qw >> 8;
         }
         break;

      case 2:
         pw = ( unsigned short * ) dst;
         for ( y = 0; y < layer->h; y++ ) {
            qw = src + y * drowwords;
            for ( x = 0; x < layer->w; x++, pw++, qw += dstep )
               *pw = *qw;
         }
         break;

      case 4:
         if ( layer->hdr.flags & Layer_FP ) {
            pf = ( float * ) dst;
            r = layer->hdr.whitePoint - layer->hdr.blackPoint;
            if ( layer->hdr.gamma > 0.0f && layer->hdr.gamma != 1.0f )
               g = layer->hdr.gamma;
            else
               g = 0.0f;

            for ( y = 0; y < layer->h; y++ ) {
               qw = src + y * drowwords;
               for ( x = 0; x < layer->w; x++, pf++, qw += dstep ) {
                  f = *qw / 65535.0f;
                  if ( g ) f = ( float ) pow( f, g );
                  *pf = f * r + layer->hdr.blackPoint;
               }
            }
         }
         break;
   }
}
