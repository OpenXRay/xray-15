//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSound.cpp
//    contributed by Kumaran Santhanam

#include "VrmlNodeSound.h"
#include "VrmlNodeType.h"

#include "MathUtils.h"
#include "VrmlScene.h"


// Sound factory. Add each Sound to the scene for fast access.

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeSound(scene);
}


// Define the built in VrmlNodeType:: "Sound" fields

VrmlNodeType *VrmlNodeSound::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("Sound", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("direction", VrmlField::SFVEC3F);
  t->addExposedField("intensity", VrmlField::SFFLOAT);
  t->addExposedField("location", VrmlField::SFVEC3F);
  t->addExposedField("maxBack", VrmlField::SFFLOAT);
  t->addExposedField("maxFront", VrmlField::SFFLOAT);
  t->addExposedField("minBack", VrmlField::SFFLOAT);
  t->addExposedField("minFront", VrmlField::SFFLOAT);
  t->addExposedField("priority", VrmlField::SFFLOAT);
  t->addExposedField("source", VrmlField::SFNODE);
  t->addField("spatialize", VrmlField::SFBOOL);

  return t;
}

// Should subclass NodeType and have each Sound maintain its own type...

VrmlNodeType *VrmlNodeSound::nodeType() const { return defineType(0); }


VrmlNodeSound::VrmlNodeSound( VrmlScene *scene ) :
  VrmlNodeChild(scene),
  d_direction(0, 0, 1),
  d_intensity(1),
  d_maxBack(10),
  d_maxFront(10),
  d_minBack(1),
  d_minFront(1),
  d_spatialize(true)
{
}


VrmlNodeSound::~VrmlNodeSound()
{
}


VrmlNode *VrmlNodeSound::cloneMe() const
{
  return new VrmlNodeSound(*this);
}

void VrmlNodeSound::cloneChildren(VrmlNamespace *ns)
{
  if (d_source.get())
    d_source.set(d_source.get()->clone(ns));
}


VrmlNodeSound* VrmlNodeSound::toSound() const
{ return (VrmlNodeSound*) this; }


void VrmlNodeSound::clearFlags()
{
  VrmlNode::clearFlags();
  if (d_source.get()) d_source.get()->clearFlags();
}

void VrmlNodeSound::addToScene(VrmlScene *s, const char *relUrl)
{
  d_scene = s;
  if (d_source.get()) d_source.get()->addToScene(s, relUrl);
}

void VrmlNodeSound::copyRoutes(VrmlNamespace *ns) const
{
  VrmlNode::copyRoutes(ns);
  if (d_source.get()) d_source.get()->copyRoutes(ns);
}


ostream& VrmlNodeSound::printFields(ostream& os, int indent)
{
  if (! FPZERO(d_direction.x()) ||
      ! FPZERO(d_direction.y()) ||
      ! FPEQUAL(d_direction.z(), 1.0) )
    PRINT_FIELD(direction);

  // ...

  return os;
}


void VrmlNodeSound::render(Viewer *viewer)
{
    // If this clip has been modified, update the internal data
    if (d_source.get() && d_source.get()->isModified())
        d_source.get()->render (viewer);
}


// Set the value of one of the node fields/events.
// setField is public so the parser can access it.

void VrmlNodeSound::setField(const char *fieldName,
			     const VrmlField &fieldValue)
{
  if TRY_FIELD(direction, SFVec3f)
  else if TRY_FIELD(intensity, SFFloat)
  else if TRY_FIELD(location, SFVec3f)		     
  else if TRY_FIELD(maxBack, SFFloat)
  else if TRY_FIELD(maxFront, SFFloat)
  else if TRY_FIELD(minBack, SFFloat)
  else if TRY_FIELD(minFront, SFFloat)
  else if TRY_FIELD(priority, SFFloat)
  else if TRY_SFNODE_FIELD2(source, AudioClip, MovieTexture)
  else if TRY_FIELD(spatialize, SFBool)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}
