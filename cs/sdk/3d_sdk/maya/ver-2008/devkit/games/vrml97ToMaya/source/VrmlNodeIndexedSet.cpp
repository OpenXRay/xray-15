//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeIndexedSet.cpp

#include "VrmlNodeIndexedSet.h"

#include "VrmlNodeType.h"


// Define the built in VrmlNodeType:: "Indexed*Set" fields

VrmlNodeType *VrmlNodeIndexedSet::defineType(VrmlNodeType *t)
{
  VrmlNodeGeometry::defineType(t);	// Parent class

  t->addEventIn("set_colorIndex", VrmlField::MFINT32);
  t->addEventIn("set_coordIndex", VrmlField::MFINT32);
  t->addExposedField("color", VrmlField::SFNODE);
  t->addExposedField("coord", VrmlField::SFNODE);
  t->addField("colorIndex", VrmlField::MFINT32);
  t->addField("colorPerVertex", VrmlField::SFBOOL);
  t->addField("coordIndex", VrmlField::MFINT32);

  return t;
}

VrmlNodeIndexedSet::VrmlNodeIndexedSet(VrmlScene *scene) :
  VrmlNodeGeometry(scene),
  d_colorPerVertex(true)
{
}

VrmlNodeIndexedSet::~VrmlNodeIndexedSet()
{
}


bool VrmlNodeIndexedSet::isModified() const
{
  return ( d_modified ||
	   (d_color.get() && d_color.get()->isModified()) ||
	   (d_coord.get() && d_coord.get()->isModified()) );
}

void VrmlNodeIndexedSet::clearFlags()
{
  VrmlNode::clearFlags();
  if (d_color.get()) d_color.get()->clearFlags();
  if (d_coord.get()) d_coord.get()->clearFlags();
}

void VrmlNodeIndexedSet::addToScene( VrmlScene *s, const char *rel )
{
  d_scene = s;
  if (d_color.get()) d_color.get()->addToScene(s, rel);
  if (d_coord.get()) d_coord.get()->addToScene(s, rel);
}

void VrmlNodeIndexedSet::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);
  if (d_color.get()) d_color.get()->copyRoutes(ns);
  if (d_coord.get()) d_coord.get()->copyRoutes(ns);
}

ostream& VrmlNodeIndexedSet::printFields(ostream& os, int indent)
{
  if (d_color.get()) PRINT_FIELD(color);
  if (d_colorIndex.size() > 0) PRINT_FIELD(colorIndex);
  if (! d_colorPerVertex.get()) PRINT_FIELD(colorPerVertex);
  if (d_coord.get()) PRINT_FIELD(coord);
  if (d_coordIndex.size() > 0) PRINT_FIELD(coordIndex);
  return os;
}

VrmlNodeColor *VrmlNodeIndexedSet::color()
{
  return d_color.get() ? d_color.get()->toColor() : 0;
}


VrmlNode* VrmlNodeIndexedSet::getCoordinate()
{   return d_coord.get(); }

const VrmlMFInt& VrmlNodeIndexedSet::getCoordIndex() const
{   return d_coordIndex; }

const VrmlMFInt& VrmlNodeIndexedSet::getColorIndex() const   // LarryD Feb 18/99
{   return d_colorIndex; }


// Set the value of one of the node fields.

void VrmlNodeIndexedSet::setField(const char *fieldName,
				  const VrmlField &fieldValue)
{
  if TRY_SFNODE_FIELD(color, Color)
  else if TRY_FIELD(colorIndex, MFInt)
  else if TRY_FIELD(colorPerVertex, SFBool)
  else if TRY_SFNODE_FIELD(coord, Coordinate)
  else if TRY_FIELD(coordIndex, MFInt)
  else
    VrmlNodeGeometry::setField(fieldName, fieldValue);
}
