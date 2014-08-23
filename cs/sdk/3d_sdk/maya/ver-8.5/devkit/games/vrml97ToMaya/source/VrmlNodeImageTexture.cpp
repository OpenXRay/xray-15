//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeImageTexture.cpp

#include "VrmlNodeImageTexture.h"

#include "VRMLImage.h"
#include "VrmlNodeType.h"
#include "VrmlScene.h"
#include "Doc.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif

static VrmlNode *creator( VrmlScene *s ) 
{ return new VrmlNodeImageTexture(s); }

const VrmlMFString& VrmlNodeImageTexture::getUrl() const 
{   return d_url; }

VrmlNodeImageTexture* VrmlNodeImageTexture::toImageTexture() const
{ return (VrmlNodeImageTexture*) this; }

// Define the built in VrmlNodeType:: "ImageTexture" fields

VrmlNodeType *VrmlNodeImageTexture::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("ImageTexture", creator);
    }

  VrmlNodeTexture::defineType(t);	// Parent class

  t->addExposedField("url", VrmlField::MFSTRING);
  t->addField("repeatS", VrmlField::SFBOOL);
  t->addField("repeatT", VrmlField::SFBOOL);

  return t;
}

VrmlNodeImageTexture::VrmlNodeImageTexture(VrmlScene *scene) :
  VrmlNodeTexture(scene),
  d_repeatS(true),
  d_repeatT(true),
  d_image(0),
  d_texObject(0)
{
}

VrmlNodeType *VrmlNodeImageTexture::nodeType() const { return defineType(0); }

VrmlNodeImageTexture::~VrmlNodeImageTexture()
{
  delete d_image;
  // delete d_texObject...
}


VrmlNode *VrmlNodeImageTexture::cloneMe() const
{
  return new VrmlNodeImageTexture(*this);
}


ostream& VrmlNodeImageTexture::printFields(ostream& os, int indent)
{
  if (d_url.get()) PRINT_FIELD(url);
  if (! d_repeatS.get()) PRINT_FIELD(repeatS);
  if (! d_repeatT.get()) PRINT_FIELD(repeatT);
  return os;
}


void VrmlNodeImageTexture::render(Viewer *viewer)
{
  if ( isModified() )
    {
      if (d_image)
	{
	  delete d_image;		// URL is the only modifiable bit
	  d_image = 0;
	}
      if (d_texObject)
	{
	  viewer->removeTextureObject(d_texObject);
	  d_texObject = 0;
	}
    }

  // should probably read the image during addToScene...
  // should cache on url so multiple references to the same file are
  // loaded just once... of course world authors should just DEF/USE
  // them...
  if (! d_image && d_url.size() > 0)
    {
      const char *relUrl = d_relativeUrl.get() ? d_relativeUrl.get() :
	d_scene->urlDoc()->url();
      Doc relDoc(relUrl);
      d_image = new Image;
      if ( ! d_image->tryURLs( d_url.size(), d_url.get(), &relDoc ) )
	cerr << "Error: couldn't read ImageTexture from URL " << d_url << endl;
    }

  // Check texture cache
  if (d_texObject && d_image)
    {
      viewer->insertTextureReference(d_texObject, d_image->nc());
    }
  else
    {
      unsigned char *pix;

      if (d_image && (pix = d_image->pixels()))
	{
	  // Ensure the image dimensions are powers of two
	  int sizes[] = { 2, 4, 8, 16, 32, 64, 128, 256 };
	  int nSizes = sizeof(sizes) / sizeof(int);
	  int w = d_image->w();
	  int h = d_image->h();
	  int i, j;
	  for (i=0; i<nSizes; ++i)
	    if (w < sizes[i]) break;
	  for (j=0; j<nSizes; ++j)
	    if (h < sizes[j]) break;

	  if (i > 0 && j > 0)
	    {
	      // Always scale images down in size and reuse the same pixel
	      // memory. This can cause some ugliness...
	      if (w != sizes[i-1] || h != sizes[j-1])
		{
		  viewer->scaleTexture( w, h, sizes[i-1], sizes[j-1],
					d_image->nc(), pix );
		  d_image->setSize( sizes[i-1], sizes[j-1] );
		}

	      d_texObject = viewer->insertTexture(d_image->w(),
						  d_image->h(),
						  d_image->nc(),
						  d_repeatS.get(),
						  d_repeatT.get(),
						  pix,
						  true);
	    }
	}
    }

  clearModified();
}


int VrmlNodeImageTexture::nComponents()
{
  return d_image ? d_image->nc() : 0;
}

int VrmlNodeImageTexture::width()
{
  return d_image ? d_image->w() : 0;
}

int VrmlNodeImageTexture::height()
{
  return d_image ? d_image->h() : 0;
}

int VrmlNodeImageTexture::nFrames()
{
  return 0;
}

unsigned char* VrmlNodeImageTexture::pixels()
{
  return d_image ? d_image->pixels() : 0;
}



// Set the value of one of the node fields.

void VrmlNodeImageTexture::setField(const char *fieldName,
				    const VrmlField &fieldValue)
{
  if (strcmp(fieldName,"url") == 0)
    {
      delete d_image;
      d_image = 0;
    }

  if TRY_FIELD(url, MFString)
  else if TRY_FIELD(repeatS, SFBool)
  else if TRY_FIELD(repeatT, SFBool)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

