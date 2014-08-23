//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodePointSet.cpp

#include "VrmlNodePointSet.h"

#include "VrmlNodeCoordinate.h"
#include "VrmlNodeColor.h"

#include "VrmlNodeType.h"
#include "Viewer.h"


static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodePointSet(s); }


// Define the built in VrmlNodeType:: "PointSet" fields

VrmlNodeType *VrmlNodePointSet::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("PointSet", creator);
    }

  VrmlNodeGeometry::defineType(t);	// Parent class
  t->addExposedField("color", VrmlField::SFNODE);
  t->addExposedField("coord", VrmlField::SFNODE);

  return t;
}

VrmlNodeType *VrmlNodePointSet::nodeType() const { return defineType(0); }


VrmlNodePointSet::VrmlNodePointSet(VrmlScene *scene) :
  VrmlNodeGeometry(scene)
{
}

VrmlNodePointSet::~VrmlNodePointSet()
{
}


VrmlNode *VrmlNodePointSet::cloneMe() const
{
  return new VrmlNodePointSet(*this);
}

void VrmlNodePointSet::cloneChildren(VrmlNamespace *ns)
{
  if (d_color.get())
    d_color.set(d_color.get()->clone(ns));
  if (d_coord.get())
    d_coord.set(d_coord.get()->clone(ns));
}


bool VrmlNodePointSet::isModified() const
{
  return ( d_modified ||
	   (d_color.get() && d_color.get()->isModified()) ||
	   (d_coord.get() && d_coord.get()->isModified()) );
}


void VrmlNodePointSet::clearFlags()
{
  VrmlNode::clearFlags();
  if (d_color.get()) d_color.get()->clearFlags();
  if (d_coord.get()) d_coord.get()->clearFlags();
}

void VrmlNodePointSet::addToScene( VrmlScene *s, const char *rel )
{
  d_scene = s;
  if (d_color.get()) d_color.get()->addToScene(s, rel);
  if (d_coord.get()) d_coord.get()->addToScene(s, rel);
}

void VrmlNodePointSet::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);
  if (d_color.get()) d_color.get()->copyRoutes(ns);
  if (d_coord.get()) d_coord.get()->copyRoutes(ns);
}

ostream& VrmlNodePointSet::printFields(ostream& os, int indent)
{
  if (d_color.get()) PRINT_FIELD(color);
  if (d_coord.get()) PRINT_FIELD(coord);

  return os;
}


Viewer::Object VrmlNodePointSet::insertGeometry(Viewer *viewer)
{
  Viewer::Object obj = 0;

  if ( d_coord.get() )
    {
      float *color = 0;
      if ( d_color.get() )
	{
	  VrmlMFColor &c = d_color.get()->toColor()->color();
	  color = &c[0][0];
	}

      VrmlMFVec3f &coord = d_coord.get()->toCoordinate()->coordinate();

      obj = viewer->insertPointSet(coord.size(), &coord[0][0], color);
    }

  if (d_color.get()) d_color.get()->clearModified();
  if (d_coord.get()) d_coord.get()->clearModified();

  return obj;
}

// Set the value of one of the node fields.

void VrmlNodePointSet::setField(const char *fieldName,
				const VrmlField &fieldValue)
{
  if TRY_SFNODE_FIELD(color, Color)
  else if TRY_SFNODE_FIELD(coord, Coordinate)
  else
    VrmlNodeGeometry::setField(fieldName, fieldValue);
}
