//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSphereSensor.cpp

#include "config.h"
#include "VrmlNodeSphereSensor.h"
#include "VrmlNodeType.h"

#include "MathUtils.h"
#include "System.h"
#include "Viewer.h"
#include "VrmlScene.h"

// SphereSensor factory. 

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeSphereSensor(scene);
}


// Define the built in VrmlNodeType:: "SphereSensor" fields

VrmlNodeType *VrmlNodeSphereSensor::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("SphereSensor", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("autoOffset", VrmlField::SFBOOL);
  t->addExposedField("enabled", VrmlField::SFBOOL);
  t->addExposedField("offset", VrmlField::SFROTATION);
  t->addEventOut("isActive", VrmlField::SFBOOL);
  t->addEventOut("rotation_changed", VrmlField::SFROTATION);
  t->addEventOut("trackPoint_changed", VrmlField::SFVEC3F);

  return t;
}

VrmlNodeType *VrmlNodeSphereSensor::nodeType() const 
{ return defineType(0); }


VrmlNodeSphereSensor::VrmlNodeSphereSensor( VrmlScene *scene ) :
  VrmlNodeChild(scene),
  d_autoOffset(true),
  d_enabled(true),
  d_isActive(false)
{
  setModified();
}


VrmlNodeSphereSensor::~VrmlNodeSphereSensor()
{
}


VrmlNode *VrmlNodeSphereSensor::cloneMe() const
{
  return new VrmlNodeSphereSensor(*this);
}


ostream& VrmlNodeSphereSensor::printFields(ostream& os, int indent)
{
  if (! d_autoOffset.get()) PRINT_FIELD(autoOffset);
  if (! d_enabled.get())    PRINT_FIELD(enabled);

  if (! FPZERO(d_offset.x()) ||
      ! FPEQUAL(d_offset.y(), 1.0) ||
      ! FPZERO(d_offset.z()) ||
      ! FPZERO(d_offset.r()) )
    PRINT_FIELD(offset);

  return os;
}

// Set the value of one of the node fields.

void VrmlNodeSphereSensor::setField(const char *fieldName,
				   const VrmlField &fieldValue)
{
  if TRY_FIELD(autoOffset, SFBool)
  else if TRY_FIELD(enabled, SFBool)
  else if TRY_FIELD(offset, SFRotation)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

