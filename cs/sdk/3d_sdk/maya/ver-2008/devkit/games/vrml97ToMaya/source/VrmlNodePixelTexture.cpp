//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodePixelTexture.cpp

#include "VrmlNodePixelTexture.h"

#include "VrmlNodeType.h"
#include "VrmlScene.h"


static VrmlNode *creator( VrmlScene *s ) {
  return new VrmlNodePixelTexture(s); 
}

VrmlNodePixelTexture* VrmlNodePixelTexture::toPixelTexture() const
{ return (VrmlNodePixelTexture*) this; }


// Define the built in VrmlNodeType:: "PixelTexture" fields

VrmlNodeType *VrmlNodePixelTexture::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("PixelTexture", creator);
    }

  VrmlNodeTexture::defineType(t);	// Parent class

  t->addExposedField("image", VrmlField::SFIMAGE);
  t->addField("repeatS", VrmlField::SFBOOL);
  t->addField("repeatT", VrmlField::SFBOOL);
  return t;
}

VrmlNodeType *VrmlNodePixelTexture::nodeType() const { return defineType(0); }


VrmlNodePixelTexture::VrmlNodePixelTexture(VrmlScene *scene) :
  VrmlNodeTexture(scene),
  d_repeatS(true),
  d_repeatT(true),
  d_texObject(0)
{
}

VrmlNodePixelTexture::~VrmlNodePixelTexture()
{
  // viewer->removeTextureObject( d_texObject ); ...
}


VrmlNode *VrmlNodePixelTexture::cloneMe() const
{
  return new VrmlNodePixelTexture(*this);
}


ostream& VrmlNodePixelTexture::printFields(ostream& os, int indent)
{
  if (! d_repeatS.get()) PRINT_FIELD(repeatS);
  if (! d_repeatT.get()) PRINT_FIELD(repeatT);
  if (d_image.width() > 0 &&
      d_image.height() > 0 &&
      d_image.nComponents() > 0 &&
      d_image.pixels() != 0)
    PRINT_FIELD(image);
  return os;
}

      
void VrmlNodePixelTexture::render(Viewer *viewer)
{
  unsigned char *pixels = d_image.pixels();

  if ( isModified() )
    {
      if (d_texObject)
	{
	  viewer->removeTextureObject(d_texObject);
	  d_texObject = 0;
	}
    }

  if (pixels)
    {
      if (d_texObject)
	{
	  viewer->insertTextureReference(d_texObject, d_image.nComponents());
	}
      else
	{
	  // Ensure the image dimensions are powers of two
	  const int sizes[] = { 2, 4, 8, 16, 32, 64, 128, 256 };
	  const int nSizes = sizeof(sizes) / sizeof(int);
	  int w = d_image.width();
	  int h = d_image.height();
	  int i, j;
	  for (i=0; i<nSizes; ++i)
	    if (w < sizes[i]) break;
	  for (j=0; j<nSizes; ++j)
	    if (h < sizes[j]) break;

	  if (i > 0 && j > 0)
	    {
	      // Always scale images down in size and reuse the same pixel memory.
	      if (w != sizes[i-1] || h != sizes[j-1])
		{
		  viewer->scaleTexture( w, h, sizes[i-1], sizes[j-1],
					d_image.nComponents(), pixels );
		  d_image.setSize( sizes[i-1], sizes[j-1] );
		}

	      d_texObject = viewer->insertTexture(d_image.width(),
						  d_image.height(),
						  d_image.nComponents(),
						  d_repeatS.get(),
						  d_repeatT.get(),
						  pixels,
						  true);
	    }
	}
    }

  clearModified();
}


int VrmlNodePixelTexture::nComponents()
{
  return d_image.nComponents();
}

int VrmlNodePixelTexture::width()
{
  return d_image.width();
}

int VrmlNodePixelTexture::height()
{
  return d_image.height();
}

int VrmlNodePixelTexture::nFrames()
{
  return 0;
}

unsigned char* VrmlNodePixelTexture::pixels()
{
  return d_image.pixels();
}


// Set the value of one of the node fields.

void VrmlNodePixelTexture::setField(const char *fieldName,
				    const VrmlField &fieldValue)
{
  if TRY_FIELD(image, SFImage)
  else if TRY_FIELD(repeatS, SFBool)
  else if TRY_FIELD(repeatT, SFBool)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

