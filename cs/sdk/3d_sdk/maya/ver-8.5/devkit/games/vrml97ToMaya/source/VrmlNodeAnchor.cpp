//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeAnchor.cpp
//

#include "VrmlNodeAnchor.h"
#include "VrmlNodeType.h"

#include "VrmlScene.h"

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeAnchor(scene);
}


// Define the built in VrmlNodeType:: "Anchor" fields

VrmlNodeType *VrmlNodeAnchor::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;
  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Anchor", creator);
    }

  VrmlNodeGroup::defineType(t);	// Parent class
  t->addExposedField("description", VrmlField::SFSTRING);
  t->addExposedField("parameter", VrmlField::MFSTRING);
  t->addExposedField("url", VrmlField::MFSTRING);

  return t;
}

VrmlNodeType *VrmlNodeAnchor::nodeType() const { return defineType(0); }


VrmlNodeAnchor::VrmlNodeAnchor(VrmlScene *scene) : VrmlNodeGroup(scene)
{
}

VrmlNodeAnchor::VrmlNodeAnchor(const VrmlNodeAnchor &n) : VrmlNodeGroup(n)
{
  d_description = n.d_description;
  d_parameter = n.d_parameter;
  d_url = n.d_url;
}

VrmlNodeAnchor::~VrmlNodeAnchor()
{
}

VrmlNode *VrmlNodeAnchor::cloneMe() const
{
  return new VrmlNodeAnchor(*this);
}


VrmlNodeAnchor* VrmlNodeAnchor::toAnchor() const
{ return (VrmlNodeAnchor*)this; }


ostream& VrmlNodeAnchor::printFields(ostream& os, int indent)
{
  VrmlNodeGroup::printFields(os, indent);
  if (d_description.get()) PRINT_FIELD(description);
  if (d_parameter.get()) PRINT_FIELD(parameter);
  if (d_url.get()) PRINT_FIELD(url);

  return os;
}


void VrmlNodeAnchor::render(Viewer *viewer)
{
  viewer->setSensitive( this );

  // Render children
  VrmlNodeGroup::render(viewer);

  viewer->setSensitive( 0 );
}


// Handle a click by loading the url

void VrmlNodeAnchor::activate()
{
  if (d_scene && d_url.size() > 0)
    {
      if (! d_scene->loadUrl( &d_url, &d_parameter ))
	theSystem->warn("Couldn't load URL %s\n", d_url[0]);
    }
}


// Set the value of one of the node fields.
// Need to delete current values ...

void VrmlNodeAnchor::setField(const char *fieldName,
			      const VrmlField &fieldValue)
{
  if TRY_FIELD(description, SFString)
  else if TRY_FIELD(parameter, MFString)
  else if TRY_FIELD(url, MFString)
  else
    VrmlNodeGroup::setField(fieldName, fieldValue);
}

