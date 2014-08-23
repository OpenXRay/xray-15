//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeDirLight.cpp

#include "VrmlNodeDirLight.h"
#include "VrmlNodeType.h"

#include "MathUtils.h"
#include "Viewer.h"

// Return a new VrmlNodeDirLight
static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeDirLight(s); }


// Define the built in VrmlNodeType:: "DirLight" fields

VrmlNodeType *VrmlNodeDirLight::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("DirectionalLight", creator);
    }

  VrmlNodeLight::defineType(t);	// Parent class
  t->addExposedField("direction", VrmlField::SFVEC3F);

  return t;
}

VrmlNodeType *VrmlNodeDirLight::nodeType() const { return defineType(0); }


VrmlNodeDirLight::VrmlNodeDirLight(VrmlScene *scene) :
  VrmlNodeLight(scene),
  d_direction(0.0, 0.0, -1.0)
{
}

VrmlNodeDirLight::~VrmlNodeDirLight()
{
}


VrmlNode *VrmlNodeDirLight::cloneMe() const
{
  return new VrmlNodeDirLight(*this);
}


ostream& VrmlNodeDirLight::printFields(ostream& os, int indent)
{
  VrmlNodeLight::printFields(os, indent);
  if (! FPZERO(d_direction.x()) ||
      ! FPZERO(d_direction.y()) ||
      ! FPEQUAL(d_direction.z(), -1.0) )
    PRINT_FIELD(direction);

  return os;
}

// This should be called before rendering any sibling nodes.

void VrmlNodeDirLight::render(Viewer *viewer)
{
  if ( d_on.get() )
    viewer->insertDirLight( d_ambientIntensity.get(),
			    d_intensity.get(),
			    d_color.get(),
			    d_direction.get() );
  clearModified();
}

// Set the value of one of the node fields.

void VrmlNodeDirLight::setField(const char *fieldName,
				const VrmlField &fieldValue)
{
  if TRY_FIELD(direction, SFVec3f)
  else 
    VrmlNodeLight::setField(fieldName, fieldValue);
}

const VrmlSFVec3f& VrmlNodeDirLight::getDirection() const   // LarryD Mar 04/99
{  return d_direction; }

VrmlNodeDirLight *VrmlNodeDirLight::toDirLight() const // LarryD Mar 04/99
{ return (VrmlNodeDirLight*) this; }
