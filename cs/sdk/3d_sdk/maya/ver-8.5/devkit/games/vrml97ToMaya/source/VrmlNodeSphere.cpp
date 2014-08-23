//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSphere.cpp

#include "VrmlNodeSphere.h"

#include "VrmlNodeType.h"
#include "Viewer.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif


static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeSphere(s); }


// Define the built in VrmlNodeType:: "Sphere" fields

VrmlNodeType *VrmlNodeSphere::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Sphere", creator);
    }

  VrmlNodeGeometry::defineType(t);	// Parent class
  t->addField("radius", VrmlField::SFFLOAT);

  return t;
}

VrmlNodeType *VrmlNodeSphere::nodeType() const { return defineType(0); }


VrmlNodeSphere::VrmlNodeSphere(VrmlScene *scene) :
  VrmlNodeGeometry(scene),
  d_radius(1.0)
{
}

VrmlNodeSphere::~VrmlNodeSphere()
{
}

VrmlNode *VrmlNodeSphere::cloneMe() const
{
  return new VrmlNodeSphere(*this);
}


ostream& VrmlNodeSphere::printFields(ostream& os, int )
{
  if (d_radius.get() != 1.0)
    os << "radius " << d_radius;
  return os;
}

Viewer::Object VrmlNodeSphere::insertGeometry(Viewer *viewer)
{
  return viewer->insertSphere(d_radius.get());
}

// Set the value of one of the node fields.

void VrmlNodeSphere::setField(const char *fieldName,
			      const VrmlField &fieldValue)
{
  if TRY_FIELD(radius, SFFloat)
  else
    VrmlNodeGeometry::setField(fieldName, fieldValue);
}

VrmlNodeSphere* VrmlNodeSphere::toSphere() const //LarryD Mar 08/99
{ return (VrmlNodeSphere*) this; }
