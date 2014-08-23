//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeBox.cpp

#include "VrmlNodeBox.h"

#include "VrmlNodeType.h"
#include "MathUtils.h"
#include "Viewer.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif


// Make a VrmlNodeBox

static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeBox(s); }


// Define the built in VrmlNodeType:: "Box" fields

VrmlNodeType *VrmlNodeBox::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;	// Define type only once.
      t = st = new VrmlNodeType("Box", creator);
    }

  VrmlNodeGeometry::defineType(t);	// Parent class
  t->addField("size", VrmlField::SFVEC3F);

  return t;
}

VrmlNodeType *VrmlNodeBox::nodeType() const { return defineType(0); }


VrmlNodeBox::VrmlNodeBox(VrmlScene *scene) :
  VrmlNodeGeometry(scene),
  d_size(2.0, 2.0, 2.0)
{
}

VrmlNodeBox::~VrmlNodeBox()
{
}

VrmlNode *VrmlNodeBox::cloneMe() const
{
  return new VrmlNodeBox(*this);
}


ostream& VrmlNodeBox::printFields(ostream& os, int )
{
  if ( ! FPEQUAL(d_size.x(), 2.0) ||
       ! FPEQUAL(d_size.y(), 2.0) ||
       ! FPEQUAL(d_size.z(), 2.0) )
    os << " size " << d_size;
  return os;
}

Viewer::Object VrmlNodeBox::insertGeometry(Viewer *viewer)
{
  return viewer->insertBox(d_size.x(), d_size.y(), d_size.z());
}

// Set the value of one of the node fields.

void VrmlNodeBox::setField(const char *fieldName,
			   const VrmlField &fieldValue)
{
  if TRY_FIELD(size, SFVec3f)
  else
    VrmlNodeGeometry::setField(fieldName, fieldValue);
}

VrmlNodeBox* VrmlNodeBox::toBox() const //LarryD Mar 08/99
{ return (VrmlNodeBox*) this; }

const VrmlSFVec3f& VrmlNodeBox::getSize() const   // LarryD Mar 08/99
{  return d_size; }
