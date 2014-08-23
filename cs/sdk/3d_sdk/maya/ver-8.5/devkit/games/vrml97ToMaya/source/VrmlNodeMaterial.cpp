//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeMaterial.cpp

#include "VrmlNodeMaterial.h"

#include "MathUtils.h"
#include "VrmlNodeType.h"
#include "Viewer.h"



static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeMaterial(s); }


// Define the built in VrmlNodeType:: "Material" fields

VrmlNodeType *VrmlNodeMaterial::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Material", creator);
    }

  VrmlNode::defineType(t);	// Parent class
  t->addExposedField("ambientIntensity", VrmlField::SFFLOAT);
  t->addExposedField("diffuseColor", VrmlField::SFCOLOR);
  t->addExposedField("emissiveColor", VrmlField::SFCOLOR);
  t->addExposedField("shininess", VrmlField::SFFLOAT);
  t->addExposedField("specularColor", VrmlField::SFCOLOR);
  t->addExposedField("transparency", VrmlField::SFFLOAT);

  return t;
}

VrmlNodeType *VrmlNodeMaterial::nodeType() const { return defineType(0); }


VrmlNodeMaterial::VrmlNodeMaterial(VrmlScene *scene) :
  VrmlNode(scene),
  d_ambientIntensity(0.2),
  d_diffuseColor(0.8, 0.8, 0.8),
  d_emissiveColor(0.0, 0.0, 0.0),
  d_shininess(0.2),
  d_specularColor(0.0, 0.0, 0.0),
  d_transparency(0.0)
{
}

VrmlNodeMaterial::~VrmlNodeMaterial()
{
}


VrmlNode *VrmlNodeMaterial::cloneMe() const
{
  return new VrmlNodeMaterial(*this);
}


VrmlNodeMaterial* VrmlNodeMaterial::toMaterial() const
{ return (VrmlNodeMaterial*) this; }


ostream& VrmlNodeMaterial::printFields(ostream& os, int indent)
{
  if (! FPEQUAL(d_ambientIntensity.get(), 0.2))
    PRINT_FIELD(ambientIntensity);

  if (! FPEQUAL(d_diffuseColor.r(), 0.8) ||
      ! FPEQUAL(d_diffuseColor.g(), 0.8) ||
      ! FPEQUAL(d_diffuseColor.b(), 0.8) )
    PRINT_FIELD(diffuseColor);

  if (! FPZERO(d_emissiveColor.r()) ||
      ! FPZERO(d_emissiveColor.g()) ||
      ! FPZERO(d_emissiveColor.b()) )
    PRINT_FIELD(emissiveColor);

  if (! FPEQUAL(d_shininess.get(), 0.2))
    PRINT_FIELD(shininess);

  if (! FPZERO(d_specularColor.r()) ||
      ! FPZERO(d_specularColor.g()) ||
      ! FPZERO(d_specularColor.b()) )
    PRINT_FIELD(specularColor);

  if (! FPZERO(d_transparency.get()) )
      PRINT_FIELD(transparency);

  return os;
}

// This currently isn't used - see VrmlNodeAppearance.cpp.

void VrmlNodeMaterial::render(Viewer *viewer)
{
  viewer->setMaterial(d_ambientIntensity.get(),
		      d_diffuseColor.get(),
		      d_emissiveColor.get(),
		      d_shininess.get(),
		      d_specularColor.get(),
		      d_transparency.get());
  clearModified();
}


// Set the value of one of the node fields.

void VrmlNodeMaterial::setField(const char *fieldName,
				const VrmlField &fieldValue)
{
  if TRY_FIELD(ambientIntensity, SFFloat)
  else if TRY_FIELD(diffuseColor, SFColor)
  else if TRY_FIELD(emissiveColor, SFColor)
  else if TRY_FIELD(shininess, SFFloat)
  else if TRY_FIELD(specularColor, SFColor)
  else if TRY_FIELD(transparency, SFFloat)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

