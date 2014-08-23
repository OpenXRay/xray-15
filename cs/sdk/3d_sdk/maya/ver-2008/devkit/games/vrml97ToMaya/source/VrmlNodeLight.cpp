//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeLight.cpp

#include "VrmlNodeLight.h"
#include "MathUtils.h"
#include "VrmlNodeType.h"



// Define the built in VrmlNodeType:: "Light" fields

VrmlNodeType *VrmlNodeLight::defineType(VrmlNodeType *t)
{
  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("ambientIntensity", VrmlField::SFFLOAT);
  t->addExposedField("color", VrmlField::SFCOLOR);
  t->addExposedField("intensity", VrmlField::SFFLOAT);
  t->addExposedField("on", VrmlField::SFBOOL);

  return t;
}


VrmlNodeType *VrmlNodeLight::nodeType() const { return defineType(0); }


VrmlNodeLight::VrmlNodeLight(VrmlScene *scene) :
  VrmlNodeChild(scene),
  d_ambientIntensity(0.0),
  d_color(1.0, 1.0, 1.0),
  d_intensity(1.0),
  d_on(true)
{
}

VrmlNodeLight::~VrmlNodeLight()
{
}


VrmlNodeLight* VrmlNodeLight::toLight() const
{ return (VrmlNodeLight*) this; }


void VrmlNodeLight::renderScoped(Viewer *) 
{
}


ostream& VrmlNodeLight::printFields(ostream& os, int indent)
{
  if (! FPZERO(d_ambientIntensity.get()))
    PRINT_FIELD(ambientIntensity);
  if (! FPEQUAL(d_color.r(), 1.0) ||
      ! FPEQUAL(d_color.g(), 1.0) ||
      ! FPEQUAL(d_color.b(), 1.0) )
    PRINT_FIELD(color);
  if (! FPEQUAL(d_intensity.get(), 1.0))
    PRINT_FIELD(intensity);
  if (! d_on.get() ) PRINT_FIELD(on);

  return os;
}


// Set the value of one of the node fields.

void VrmlNodeLight::setField(const char *fieldName,
			     const VrmlField &fieldValue)
{
  if TRY_FIELD(ambientIntensity, SFFloat)
  else if TRY_FIELD(color, SFColor)
  else if TRY_FIELD(intensity, SFFloat)
  else if TRY_FIELD(on, SFBool)
  else 
    VrmlNode::setField(fieldName, fieldValue);
}
