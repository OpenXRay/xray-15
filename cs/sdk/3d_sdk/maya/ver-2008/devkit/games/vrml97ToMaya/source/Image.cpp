//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  Image.cpp

#include "config.h"
#include "VRMLImage.h"
#include "Doc.h"
#include "System.h"

#include <stdlib.h>		// free()
#include <string.h>

// gif support is builtin
# include "gifread.h"

#if HAVE_LIBJPEG
#include "jpgread.h"
#endif
#if HAVE_LIBPNG
#include "pngread.h"
#endif

typedef enum {
  ImageFile_UNKNOWN,

  ImageFile_GIF,
  ImageFile_JPG,
  ImageFile_PNG

} ImageFileType;


static ImageFileType imageFileType(const char *, FILE *);


Image::Image(const char *url, Doc *relative) :
  d_url(0),
  d_w(0), d_h(0), d_nc(0), d_pixels(0), d_frame(0)
{
  if (url) (void) setURL(url, relative);
}


Image::~Image()
{
  delete d_url;
  if (d_pixels) free(d_pixels); // assumes file readers use malloc...
  if (d_frame) free(d_frame);
}


bool Image::setURL(const char *url, Doc *relative)
{
  if (d_url) delete d_url;
  if (d_pixels) free(d_pixels); // assumes file readers use malloc...
  if (d_frame) free(d_frame);
  d_pixels = 0;
  d_frame = 0;
  d_w = d_h = d_nc = d_nFrames = 0;
  if (! url) return true;

  d_url = new Doc(url, relative);

  //  theSystem->debug("Image: trying to create Doc(%s, %s)\n",
  //		   url, relative ? relative->url() : "");
  
  FILE *fp = d_url->fopen("rb");

  if (fp)
    {
      switch (imageFileType(url, fp))
	{
	case ImageFile_GIF:
	  d_pixels = gifread(fp, &d_w, &d_h, &d_nc, &d_nFrames, &d_frame);
	  break;

#if HAVE_LIBJPEG
	case ImageFile_JPG:
	  d_pixels = jpgread(fp, &d_w, &d_h, &d_nc);
	  break;
#endif
#if HAVE_LIBPNG
	case ImageFile_PNG:
	  d_pixels = pngread(fp, &d_w, &d_h, &d_nc);
	  break;
#endif

	default:
	  fprintf(stderr,"Error: unrecognized image file format (%s).\n", url);
	  break;
	}

      if (! d_pixels)
	fprintf(stderr,"Error: unable to read image file (%s).\n", url);
	
      d_url->fclose();
    }

  return (d_pixels != 0);
}


bool Image::tryURLs(int nUrls, char **urls, Doc *relative)
{
  int i;
  for (i=0; i<nUrls; ++i)	// Try each url until one succeeds
    if (urls[i] && setURL(urls[i], relative))
      break;

  return i < nUrls;
}


const char *Image::url() { return d_url ? d_url->url() : 0; }


// Could peek at file header...

static ImageFileType imageFileType(const char *url, FILE *)
{
  const char *suffix = strrchr(url, '.');
  if (suffix) ++suffix;

  if (strcmp(suffix,"gif") == 0 ||
      strcmp(suffix,"GIF") == 0)
    return ImageFile_GIF;

  else if (strcmp(suffix,"jpg") == 0 ||
	   strcmp(suffix,"JPG") == 0 ||
	   strcmp(suffix,"jpeg") == 0 ||
	   strcmp(suffix,"JPEG") == 0)
    return ImageFile_JPG;

  else if (strcmp(suffix,"png") == 0 ||
	   strcmp(suffix,"PNG") == 0)
    return ImageFile_PNG;

  else
    return ImageFile_UNKNOWN;
}


unsigned char *Image::pixels(int frame)
{
  return (frame < d_nFrames) ? d_frame[ frame ] : 0;
}

