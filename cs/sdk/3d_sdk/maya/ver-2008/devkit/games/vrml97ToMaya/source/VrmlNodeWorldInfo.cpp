//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeWorldInfo.cpp

#include "VrmlNodeWorldInfo.h"

#include "VrmlNodeType.h"
#include "VrmlScene.h"


//  WorldInfo factory.

static VrmlNode *creator(VrmlScene *s)
{
  return new VrmlNodeWorldInfo(s);
}

// Define the built in VrmlNodeType:: "WorldInfo" fields

VrmlNodeType *VrmlNodeWorldInfo::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("WorldInfo", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addField("info", VrmlField::MFSTRING);
  t->addField("title", VrmlField::SFSTRING);

  return t;
}

VrmlNodeType *VrmlNodeWorldInfo::nodeType() const { return defineType(0); }


VrmlNodeWorldInfo::VrmlNodeWorldInfo(VrmlScene *scene) :
  VrmlNodeChild(scene)
{
}

VrmlNodeWorldInfo::~VrmlNodeWorldInfo()
{
}

VrmlNode *VrmlNodeWorldInfo::cloneMe() const
{
  return new VrmlNodeWorldInfo(*this);
}


ostream& VrmlNodeWorldInfo::printFields(ostream& os, int indent)
{
  if (d_title.get()) PRINT_FIELD(title);
  if (d_info.size() > 0) PRINT_FIELD(info);

  return os;
}

// Set the value of one of the node fields.

void VrmlNodeWorldInfo::setField(const char *fieldName,
				 const VrmlField &fieldValue)
{
  if TRY_FIELD(info, MFString)
  else if TRY_FIELD(title, SFString)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

