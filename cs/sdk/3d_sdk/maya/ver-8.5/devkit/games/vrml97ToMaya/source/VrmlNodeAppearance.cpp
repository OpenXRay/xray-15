//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeAppearance.cpp
//

#include "VrmlNodeAppearance.h"
#include "VrmlNodeType.h"

#include "Viewer.h"
#include "VrmlNodeMaterial.h"
#include "VrmlNodeTexture.h"

static VrmlNode *creator( VrmlScene *scene ) 
{ return new VrmlNodeAppearance(scene); }


// Define the built in VrmlNodeType:: "Appearance" fields

VrmlNodeType *VrmlNodeAppearance::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("Appearance", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("material", VrmlField::SFNODE);
  t->addExposedField("texture", VrmlField::SFNODE);
  t->addExposedField("textureTransform", VrmlField::SFNODE);

  return t;
}

VrmlNodeType *VrmlNodeAppearance::nodeType() const { return defineType(0); }


VrmlNodeAppearance::VrmlNodeAppearance(VrmlScene *scene) :
  VrmlNodeChild(scene)
{
}

VrmlNodeAppearance::~VrmlNodeAppearance()
{
}

VrmlNode *VrmlNodeAppearance::cloneMe() const
{
  return new VrmlNodeAppearance(*this);
}

void VrmlNodeAppearance::cloneChildren(VrmlNamespace *ns)
{
  // Replace references with clones
  if (d_material.get())
    d_material.set(d_material.get()->clone(ns));
  if (d_texture.get())
    d_texture.set(d_texture.get()->clone(ns));
  if (d_textureTransform.get())
    d_textureTransform.set(d_textureTransform.get()->clone(ns));
}


VrmlNodeAppearance* VrmlNodeAppearance::toAppearance() const
{
  return (VrmlNodeAppearance*) this;
}


bool VrmlNodeAppearance::isModified() const
{
  return ( d_modified ||
	   (d_material.get() && d_material.get()->isModified()) ||
	   (d_texture.get() && d_texture.get()->isModified()) ||
	   (d_textureTransform.get() &&
	    d_textureTransform.get()->isModified()) );
}

void VrmlNodeAppearance::clearFlags()
{
  VrmlNode::clearFlags();
  if (d_material.get()) d_material.get()->clearFlags();
  if (d_texture.get()) d_texture.get()->clearFlags();
  if (d_textureTransform.get()) d_textureTransform.get()->clearFlags();
}

void VrmlNodeAppearance::addToScene( VrmlScene *s, const char *rel )
{
  d_scene = s;
  if (d_material.get()) d_material.get()->addToScene(s, rel);
  if (d_texture.get()) d_texture.get()->addToScene(s, rel);
  if (d_textureTransform.get())
    d_textureTransform.get()->addToScene(s,rel);
}

// Copy the routes to nodes in the given namespace.

void VrmlNodeAppearance::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);  // Copy my routes

  // Copy subnode routes
  if (d_material.get()) d_material.get()->copyRoutes(ns);
  if (d_texture.get()) d_texture.get()->copyRoutes(ns);
  if (d_textureTransform.get())
    d_textureTransform.get()->copyRoutes(ns);
}


ostream& VrmlNodeAppearance::printFields(ostream& os, int indent)
{
  if (d_material.get()) PRINT_FIELD(material);
  if (d_texture.get()) PRINT_FIELD(texture);
  if (d_textureTransform.get()) PRINT_FIELD(textureTransform);

  return os;
}


void VrmlNodeAppearance::render(Viewer *viewer)
{
  VrmlNodeMaterial *m = d_material.get() ? d_material.get()->toMaterial() : 0;
  VrmlNodeTexture *t = d_texture.get() ? d_texture.get()->toTexture() : 0;

  if (m)
    {
      float trans = m->transparency();
      float *diff = m->diffuseColor();
      float diffuse[3] = { diff[0], diff[1], diff[2] };
      int nTexComponents = t ? t->nComponents() : 0;
      if (nTexComponents == 2 || nTexComponents == 4)
	trans = 0.0;
      if (nTexComponents >= 3)
	diffuse[0] = diffuse[1] = diffuse[2] = 1.0;

      viewer->setMaterial(m->ambientIntensity(),
			  diffuse,
			  m->emissiveColor(),
			  m->shininess(),
			  m->specularColor(),
			  trans);

      m->clearModified();
    }
  else
    {
      viewer->setColor(1.0, 1.0, 1.0); // default color
      viewer->enableLighting(false);   // turn lighting off for this object
    }

  if (t)
    {
      if (d_textureTransform.get())
	d_textureTransform.get()->render(viewer);
      else
	viewer->setTextureTransform( 0, 0, 0, 0 );

      t->render(viewer);
    }

  clearModified();
}

// Set the value of one of the node fields.

void VrmlNodeAppearance::setField(const char *fieldName,
				  const VrmlField &fieldValue)
{
  if TRY_SFNODE_FIELD(material, Material)
  else if TRY_SFNODE_FIELD(texture, Texture)
  else if TRY_SFNODE_FIELD(textureTransform, TextureTransform)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

