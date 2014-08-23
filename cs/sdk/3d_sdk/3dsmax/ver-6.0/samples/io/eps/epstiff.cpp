/******************************************************************************
  
  EPSTIFF.C - Part of the EPS file saver (NOT loader)
  Written by Autodesk for 3D Studio MAX
  
  (C) Copyright 1995 by Autodesk, Inc.

  This program is copyrighted by Autodesk, Inc. and is licensed to you under
  the following conditions.  You may not distribute or publish the source
  code of this program in any form.  You may incorporate this code in object
  form in derivative works provided such derivative works are (i.) are de-
  signed and intended to work solely with Autodesk, Inc. products, and (ii.)
  contain Autodesk's copyright notice "(C) Copyright 1994 by Autodesk, Inc."

  AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.  AUTODESK SPE-
  CIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
  A PARTICULAR USE.  AUTODESK, INC.  DOES NOT WARRANT THAT THE OPERATION OF
  THE PROGRAM WILL BE UNINTERRUPTED OR ERROR FREE.

  This files contains the portion of the code for writing a simple TIFF
  image that is part of the EPS file.
  
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "max.h"
#include "bmmlib.h"
#include "eps.h"

#define TIFF_SHORT  3
#define TIFF_LONG  4
#define TIFF_RATIONAL  5

#define NUM_TAG_ENTRIES 15
#define TIFF_MAGIC 42

#define EPSF_HEADER_SIZE 30

struct TiffHeader
{
    short byteOrder;          /* 0x4949 for little-endian */
    short magic;              /* TIFF magic number of 42   */
    long IFDOffset;           /* offset in bytes to first IFD */
};

/* For TIFF tags */
struct IFDLongEntry
{
    short  id;              /* Tag ID */
    short  type;            /* short, long, etc */
    long   numValues;       /* Number of values */
    long   valueOrOffset;   /* Sometime offset, sometimes value */
};

struct IFDShortEntry
{
    short  id;              /* Tag ID */
    short  type;            /* short, long, etc */
    long   numValues;       /* Number of values */
    short  valueOrOffset;   /* Sometime offset, sometimes value */
    short  dummy;           /* Fills out the structure */
};

struct TiffDirectory
{
    struct IFDLongEntry NewSubfileType;
    struct IFDLongEntry ImageWidth;
    struct IFDLongEntry ImageLength;
    struct IFDShortEntry BitsPerSample;
    struct IFDShortEntry Compression;       /* 1 for none */
    struct IFDShortEntry Photometric;       /* 1 for 0 is black, 2 for RGB */
    struct IFDLongEntry StripOffsets;
    struct IFDShortEntry Orientation;       /* 1 always */
    struct IFDShortEntry SamplesPerPixel;   /* 1 for gray, 3 for RGB */
    struct IFDShortEntry RowsPerStrip;      /* We put all in a single strip */
    struct IFDLongEntry StripByteCounts;    /* Size of entire image for us */
    struct IFDLongEntry XResolution;        /* Pixels per res unit of width */
    struct IFDLongEntry YResolution;        /* Pixels per res unit of height */
    struct IFDShortEntry PlanarConfiguration; /* 1 for rgbrgbrgb... */
    struct IFDShortEntry ResolutionUnit;    /* 2 for inch */
    long offsetNextIFD;                     /* offset to next IFD (0) */
    long x_num, x_denom, y_num, y_denom;    /* resolution rationals */
    short red, green, blue, dummy;          /* Bits per sample (8) */
};

static struct TiffHeader tiffHeader = 
{
    0X4949, TIFF_MAGIC, 8L
};

static short numTagEntries = NUM_TAG_ENTRIES;

static struct TiffDirectory tiffDirectory =
{
    254, TIFF_LONG,     1L, 0L,       /* SubFileType */
    256, TIFF_LONG,     1L, 0L,       /* ImageWidth */
    257, TIFF_LONG,     1L, 0L,       /* ImageLength */
    258, TIFF_SHORT,    3L, 0, 0,     /* 8 bits per sample */
    259, TIFF_SHORT,    1L, 1, 0,     /* no compression */
    262, TIFF_SHORT,    1L, 2, 0,     /* RGB */
    273, TIFF_LONG,     1L, 0L,       /* Strip offsets */
    274, TIFF_SHORT,    1L, 1, 0,     /* Orientation, 1 always */
    277, TIFF_SHORT,    1L, 3, 0,     /* Samples per pixel */
    278, TIFF_SHORT,    1L, 0, 0,     /* Rows per strip */
    279, TIFF_LONG,     1L, 0L,       /* Strip byte count */
    282, TIFF_RATIONAL, 1L, 0L,       /* Resolution of width */
    283, TIFF_RATIONAL, 1L, 0L,       /* Resolution of height */
    284, TIFF_SHORT,    1L, 1, 0,     /* Planar configuration = rgbrgb... */
    296, TIFF_SHORT,    1L, 1, 0,     /* Units in inches */
    0L, 	                      /* No next IFD */
    0L, 0L, 0L, 0L,		      /* Resolutions as fractions */
    8, 8, 8, 0                        /* Bits per sample */
};


/* This writes the TIFF preview section */
int
BitmapIO_EPS::WritePreview (int colorType, int orientation,
			    float x_resolution, float y_resolution,
			    Bitmap *bitmap)
{
    struct EPSFileHeader header;
    int status;

    /* If the resolution is less than 72.0 dpi we use the same resolution
     * else we use 72.0
     */
    if (x_resolution <= 72.0 && y_resolution <= 72.0)
        tiffDownSample = 1.0F;
    else
        tiffDownSample = (x_resolution >= y_resolution) ?
            (float) (72.0 / x_resolution) : (float) (72.0 / y_resolution);
    tiffXResolution = tiffDownSample * x_resolution;
    tiffYResolution = tiffDownSample * y_resolution;

    header.magic[0] = 0xc5;
    header.magic[1] = 0xd0;
    header.magic[2] = 0xd3;
    header.magic[3] = 0xc6;
    header.psStart = EPSF_HEADER_SIZE + TiffFileLength (colorType, bitmap);
    header.psLength = 0;
    header.metafileStart = 0;
    header.metafileLength = 0;
    header.tiffStart = EPSF_HEADER_SIZE;
    header.tiffLength = TiffFileLength (colorType, bitmap);
    header.checksum = (unsigned short) 0xffff;

    if (fwrite (&header, EPSF_HEADER_SIZE, 1, ostream) != 1)
        return BMMRES_IOERROR;

    status = WriteTiff (colorType, orientation, bitmap);

    return status;
}

/* This function writes the TIFF file */
int
BitmapIO_EPS::WriteTiff (int colorType, int orientation,
	   Bitmap *bitmap)
{
    int status;

    status = WriteTiffHeader (colorType, orientation, bitmap);

    if (status == BMMRES_SUCCESS)
	status = WriteTiffImage (colorType, orientation, bitmap);

    return status;
}

/* This function writes the TIFF header portion of the TIFF file */
int
BitmapIO_EPS::WriteTiffHeader (int colorType, int orientation,
		 Bitmap *bitmap)
{
    /* Fill in the fields that need to be */
    if (colorType == RGBIMAGE) {
        tiffDirectory.BitsPerSample.numValues = 3;
        tiffDirectory.BitsPerSample.valueOrOffset = 30 + NUM_TAG_ENTRIES * 12;
        tiffDirectory.Photometric.valueOrOffset = 2;
        tiffDirectory.SamplesPerPixel.valueOrOffset = 3;
        tiffDirectory.StripByteCounts.valueOrOffset =
            3 * TiffWidth (bitmap) * TiffLength (bitmap);
    }
    else {
        tiffDirectory.BitsPerSample.numValues = 1;
        tiffDirectory.BitsPerSample.valueOrOffset = 8;
        tiffDirectory.Photometric.valueOrOffset = 1;
        tiffDirectory.SamplesPerPixel.valueOrOffset = 1;
        tiffDirectory.StripByteCounts.valueOrOffset =
            TiffWidth (bitmap) * TiffLength (bitmap);
    }
    tiffDirectory.ImageWidth.valueOrOffset = (orientation == PORTRAIT) ?
        TiffWidth (bitmap) : TiffLength (bitmap);
    tiffDirectory.ImageLength.valueOrOffset = (orientation == PORTRAIT) ?
        TiffLength (bitmap) : TiffWidth (bitmap);
    tiffDirectory.StripOffsets.valueOrOffset = 10 + sizeof(struct TiffDirectory);
    tiffDirectory.RowsPerStrip.valueOrOffset = (orientation == PORTRAIT) ?
        (short) TiffLength (bitmap) : (short) TiffWidth (bitmap);
    tiffDirectory.XResolution.valueOrOffset = 14 + NUM_TAG_ENTRIES * 12;
    tiffDirectory.YResolution.valueOrOffset = 22 + NUM_TAG_ENTRIES * 12;
    tiffDirectory.x_num = (long) (10000 * tiffXResolution);
    tiffDirectory.x_denom = 10000L;
    tiffDirectory.y_num = (long) (10000 * tiffYResolution);
    tiffDirectory.y_denom = 10000L;    

    /* Write out this way to avoid any packing problems in structure */
    if (fwrite (&tiffHeader, sizeof(struct TiffHeader), 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&numTagEntries, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    WriteIFDLong (&tiffDirectory.NewSubfileType);
    WriteIFDLong (&tiffDirectory.ImageWidth);
    WriteIFDLong (&tiffDirectory.ImageLength);
    WriteIFDShort (&tiffDirectory.BitsPerSample);
    WriteIFDShort (&tiffDirectory.Compression);
    WriteIFDShort (&tiffDirectory.Photometric);
    WriteIFDLong (&tiffDirectory.StripOffsets);
    WriteIFDShort (&tiffDirectory.Orientation);
    WriteIFDShort (&tiffDirectory.SamplesPerPixel);
    WriteIFDShort (&tiffDirectory.RowsPerStrip);
    WriteIFDLong (&tiffDirectory.StripByteCounts);
    WriteIFDLong (&tiffDirectory.XResolution);
    WriteIFDLong (&tiffDirectory.YResolution);
    WriteIFDShort (&tiffDirectory.PlanarConfiguration);
    WriteIFDShort (&tiffDirectory.ResolutionUnit);

    fwrite (&tiffDirectory.offsetNextIFD, 4, 1, ostream);
    fwrite (&tiffDirectory.x_num, 4, 1, ostream);
    fwrite (&tiffDirectory.x_denom, 4, 1, ostream);
    fwrite (&tiffDirectory.y_num, 4, 1, ostream);
    fwrite (&tiffDirectory.y_denom, 4, 1, ostream);
    fwrite (&tiffDirectory.red, 2, 1, ostream);
    fwrite (&tiffDirectory.green, 2, 1, ostream);
    fwrite (&tiffDirectory.blue, 2, 1, ostream);
    fwrite (&tiffDirectory.dummy, 2, 1, ostream);

    return BMMRES_SUCCESS;
}

/* This writes the TIFF image data itself */
int
BitmapIO_EPS::WriteTiffImage (int colorType, int orientation, Bitmap *bitmap)
{
    BMM_Color_64 *fullColor = NULL;
    long tiffLength, tiffWidth, numBytes;
    unsigned char *buffer = NULL;
    int status = BMMRES_SUCCESS;
    int row, column, sub, i, j, m;

    tiffLength = TiffLength (bitmap);
    tiffWidth = TiffWidth (bitmap);
    if (colorType == RGBIMAGE && orientation == PORTRAIT)
        numBytes = 3 * tiffWidth;
    else if (colorType == GRAYIMAGE && orientation == PORTRAIT)
        numBytes = tiffWidth;
    else if (colorType == RGBIMAGE)
        numBytes =  3 * tiffLength;
    else
        numBytes = tiffLength;
    buffer = (unsigned char *) malloc (numBytes);
    if (! buffer) {
        status = BMMRES_MEMORYERROR;
	goto cleanup;
    }

    /* Now write the image data, we simply do point sampling
     * Some programs can not handle TIFFS that don't have the default
     * orientation, so we always output the standard orientation by
     * outputting the image data in the required order.
     */
    if (orientation == PORTRAIT) {
	fullColor = (BMM_Color_64 *) malloc (bitmap->Width() *
					     sizeof(BMM_Color_64));
	if (! fullColor) {
	    status = BMMRES_MEMORYERROR;
	    goto cleanup;
	}

        for (i = 0; i < tiffLength; i++) {
            row = (i * bitmap->Height()) / tiffLength;
	    if (GetOutputPixels (0, i, bitmap->Width(), fullColor) 
		!= 1) {
		status = BMMRES_IOERROR;
		goto cleanup;
	    }
            for (j = 0, m = 0; j < tiffWidth; j++) {
                sub = (j * bitmap->Width()) / tiffWidth;
                if (colorType == RGBIMAGE) {
                    buffer[m++] = fullColor[sub].r >> 8;
                    buffer[m++] = fullColor[sub].g >> 8;
                    buffer[m++] = fullColor[sub].b >> 8;
                }
                else
                    buffer[m++] = (unsigned char) 
			(0.3 * (fullColor[sub].r >> 8) +
			 0.59 * (fullColor[sub].g >> 8) +
			 0.11 * (fullColor[sub].b >> 8) + 0.5);
            }
            if (fwrite (buffer, numBytes, 1, ostream) != 1) {
                status = BMMRES_IOERROR;
		goto cleanup;
            }
        }
    }
    else {
	// What we can do is read in full rows, but read in only those
	// rows that are actually used.  For high resolution images and
	// a down sampled image that should significantly reduce the 
	// amount of memory used.
	fullColor = (BMM_Color_64 *) malloc (bitmap->Width()*bitmap->Height()*
					     sizeof(BMM_Color_64));
	if (! fullColor) {
	    status = BMMRES_MEMORYERROR;
	    goto cleanup;
	}

	for (i = 0; i < bitmap->Height(); i++) {
	    if (GetOutputPixels (0, i, bitmap->Width(), 
				 &fullColor[i*bitmap->Width()])
		!= 1) {
		status = BMMRES_IOERROR;
		goto cleanup;
	    }
	}

        for (i = 0; i < tiffWidth; i++) {
            column = bitmap->Width() - 1 - ((i * bitmap->Width()) / tiffWidth);
            for (j = 0, m = 0; j < tiffLength; j++) {
                sub = column +
                    ((j * bitmap->Height()) / tiffLength) * bitmap->Width();
                if (colorType == RGBIMAGE) {
                    buffer[m++] = fullColor[sub].r >> 8;
                    buffer[m++] = fullColor[sub].g >> 8;
                    buffer[m++] = fullColor[sub].b >> 8;
                }
                else
                    buffer[m++] = (unsigned char) 
			(0.3 * (fullColor[sub].r >> 8) +
			 0.59 * (fullColor[sub].g >> 8) +
			 0.11 * (fullColor[sub].b >> 8) + 0.5);
            }
            if (fwrite (buffer, numBytes, 1, ostream) != 1) {
                status = BMMRES_IOERROR;
		goto cleanup;
            }
        }
    }

 cleanup:
    free (fullColor);
    free (buffer);
    return status;
}

/* We only know the length of the PostScript after we are all done
 * by finding out how long the file is, so we go back and fill in this
 * data in the binary DOS EPS header.
 */
int
BitmapIO_EPS::WritePSLength (int colorType, Bitmap *bitmap)
{
    long totalLength, psLength;

    totalLength = ftell (ostream);
    psLength = totalLength - EPSF_HEADER_SIZE -
        TiffFileLength (colorType, bitmap);

    /* Reposition to the EPSF header and write out the length of the PostScript
     * file
     */
    if (fseek (ostream, 8, SEEK_SET))
        return BMMRES_IOERROR;
    if (fwrite (&psLength, 4, 1, ostream) != 1)
        return BMMRES_IOERROR;
        
    return BMMRES_SUCCESS;
}

/* Length of TIFF image.  Must be positive */
long
BitmapIO_EPS::TiffLength (Bitmap *bitmap)
{
    long length = (long) (tiffDownSample * bitmap->Height());
    if (length <= 0)
        length = 1;
    return length;
}

/* Width of TIFF image.  Must be positive */
long
BitmapIO_EPS::TiffWidth (Bitmap *bitmap)
{
    long width = (long) (tiffDownSample * bitmap->Width());
    if (width <= 0)
        width = 1;
    return width;
}

/* Computes length of TIFF file in bytes */
long
BitmapIO_EPS::TiffFileLength (int colorType, Bitmap *bitmap)
{
    long length;

    if (colorType == RGBIMAGE)
        length =  10 + sizeof (struct TiffDirectory) + 
            3 * TiffWidth (bitmap) *  TiffLength (bitmap);
    else
        length = 10 + sizeof (struct TiffDirectory) +
            TiffWidth (bitmap) * TiffLength (bitmap);

    return length;
}

/* Writes  out an IFD entry that contains a long */
int
BitmapIO_EPS::WriteIFDLong (struct IFDLongEntry *entry)
{
    if (fwrite (&entry->id, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->type, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->numValues, 4, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->valueOrOffset, 4, 1, ostream) != 1)
        return BMMRES_IOERROR;
    return BMMRES_SUCCESS;
}

/* Writes  out an IFD entry that contains a short */
int
BitmapIO_EPS::WriteIFDShort (struct IFDShortEntry *entry)
{
    if (fwrite (&entry->id, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->type, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->numValues, 4, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->valueOrOffset, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    if (fwrite (&entry->dummy, 2, 1, ostream) != 1)
        return BMMRES_IOERROR;
    return BMMRES_SUCCESS;
}

