//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCylinder.cpp

#include "VrmlNodeCylinder.h"

#include "VrmlNodeType.h"
#include "Viewer.h"


//  Return a new VrmlNodeCylinder
static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeCylinder(s); }


// Define the built in VrmlNodeType:: "Cylinder" fields

VrmlNodeType *VrmlNodeCylinder::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Cylinder", creator);
    }

  VrmlNodeGeometry::defineType(t);	// Parent class
  t->addField("bottom", VrmlField::SFBOOL);
  t->addField("height", VrmlField::SFFLOAT);
  t->addField("radius", VrmlField::SFFLOAT);
  t->addField("side", VrmlField::SFBOOL);
  t->addField("top", VrmlField::SFBOOL);

  return t;
}


VrmlNodeType *VrmlNodeCylinder::nodeType() const { return defineType(0); }


VrmlNodeCylinder::VrmlNodeCylinder(VrmlScene *scene) :
  VrmlNodeGeometry(scene),
  d_bottom(true),
  d_height(2.0),
  d_radius(1.0),
  d_side(true),
  d_top(true)
{
}

VrmlNodeCylinder::~VrmlNodeCylinder()
{
  // need access to viewer to remove d_viewerObject...
}


VrmlNode *VrmlNodeCylinder::cloneMe() const
{
  return new VrmlNodeCylinder(*this);
}


ostream& VrmlNodeCylinder::printFields(ostream& os, int indent)
{
  if (! d_bottom.get()) PRINT_FIELD(bottom);
  if (! d_side.get()) PRINT_FIELD(side);
  if (! d_top.get()) PRINT_FIELD(top);
  if (d_height.get() != 2.0) PRINT_FIELD(height);
  if (d_radius.get() != 1.0) PRINT_FIELD(radius);

  return os;
}


Viewer::Object VrmlNodeCylinder::insertGeometry(Viewer *viewer)
{
  return viewer->insertCylinder(d_height.get(),
				d_radius.get(),
				d_bottom.get(),
				d_side.get(),
				d_top.get());
}

// Set the value of one of the node fields.

void VrmlNodeCylinder::setField(const char *fieldName,
				const VrmlField &fieldValue)
{
  if TRY_FIELD(bottom, SFBool)
  else if TRY_FIELD(height, SFFloat)
  else if TRY_FIELD(radius, SFFloat)
  else if TRY_FIELD(side, SFBool)
  else if TRY_FIELD(top, SFBool)
  else
    VrmlNodeGeometry::setField(fieldName, fieldValue);
}

VrmlNodeCylinder* VrmlNodeCylinder::toCylinder() const //LarryD Mar 08/99
{ return (VrmlNodeCylinder*) this; }
