//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCoordinate.cpp

#include "VrmlNodeCoordinate.h"
#include "VrmlNodeType.h"


static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeCoordinate(s); }


// Define the built in VrmlNodeType:: "Coordinate" fields

VrmlNodeType *VrmlNodeCoordinate::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Coordinate", creator);
    }

  VrmlNode::defineType(t);	// Parent class
  t->addExposedField("point", VrmlField::MFVEC3F);

  return t;
}


VrmlNodeType *VrmlNodeCoordinate::nodeType() const { return defineType(0); }


VrmlNodeCoordinate::VrmlNodeCoordinate(VrmlScene *scene) : VrmlNode(scene)
{
}

VrmlNodeCoordinate::~VrmlNodeCoordinate()
{
}


VrmlNode *VrmlNodeCoordinate::cloneMe() const
{
  return new VrmlNodeCoordinate(*this);
}


VrmlNodeCoordinate* VrmlNodeCoordinate::toCoordinate() const
{ return (VrmlNodeCoordinate*)this; }


ostream& VrmlNodeCoordinate::printFields(ostream& os, int indent)
{
  if (d_point.size() > 0) PRINT_FIELD(point);

  return os;
}


// Set the value of one of the node fields.

void VrmlNodeCoordinate::setField(const char *fieldName,
				  const VrmlField &fieldValue)
{
  if TRY_FIELD(point, MFVec3f)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

