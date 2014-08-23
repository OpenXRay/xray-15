//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeNavigationInfo.cpp

#include "VrmlNodeNavigationInfo.h"

#include "MathUtils.h"
#include "VrmlNodeType.h"
#include "VrmlScene.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif

//  NavigationInfo factory.
//  Since NavInfo is a bindable child node, the first one created needs
//  to notify its containing scene.

static VrmlNode *creator(VrmlScene *scene)
{
  return new VrmlNodeNavigationInfo(scene);
}


// Define the built in VrmlNodeType:: "NavigationInfo" fields

VrmlNodeType *VrmlNodeNavigationInfo::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("NavigationInfo", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addEventIn("set_bind", VrmlField::SFBOOL);
  t->addExposedField("avatarSize", VrmlField::MFFLOAT);
  t->addExposedField("headlight", VrmlField::SFBOOL);
  t->addExposedField("speed", VrmlField::SFFLOAT);
  t->addExposedField("type", VrmlField::MFSTRING);
  t->addExposedField("visibilityLimit", VrmlField::SFFLOAT);
  t->addEventOut("isBound", VrmlField::SFBOOL);

  return t;
}


VrmlNodeType *VrmlNodeNavigationInfo::nodeType() const { return defineType(0); }


VrmlNodeNavigationInfo::VrmlNodeNavigationInfo(VrmlScene *scene) :
  VrmlNodeChild(scene),
  d_headlight( true ),
  d_speed( 1.0 ),
  d_visibilityLimit( 0.0 )
{
  float avatarSize[] = { 0.25, 1.6, 0.75 };
  char *type[] = { "WALK", "ANY" };

  d_avatarSize.set( 3, avatarSize );
  d_type.set( 2, type );
  if (d_scene) d_scene->addNavigationInfo(this);
}

VrmlNodeNavigationInfo::~VrmlNodeNavigationInfo()
{
  if (d_scene) d_scene->removeNavigationInfo(this);
}


VrmlNode *VrmlNodeNavigationInfo::cloneMe() const
{
  return new VrmlNodeNavigationInfo(*this);
}


VrmlNodeNavigationInfo* VrmlNodeNavigationInfo::toNavigationInfo() const
{ return (VrmlNodeNavigationInfo*) this; }


void VrmlNodeNavigationInfo::addToScene(VrmlScene *s, const char *)
{ if (d_scene != s && (d_scene = s) != 0) d_scene->addNavigationInfo(this); }


ostream& VrmlNodeNavigationInfo::printFields(ostream& os, int indent)
{
  if (d_avatarSize.size() != 3 ||
      ! FPEQUAL(d_avatarSize[0], 0.25) ||
      ! FPEQUAL(d_avatarSize[1], 1.6) ||
      ! FPEQUAL(d_avatarSize[2], 0.75) )
    PRINT_FIELD(avatarSize);
  if (! d_headlight.get()) PRINT_FIELD(headlight);
  if (! FPEQUAL(d_speed.get(), 1.0)) PRINT_FIELD(speed);
  if (d_type.size() != 2 ||
      strcmp(d_type[0], "WALK") != 0 ||
      strcmp(d_type[1], "ANY") != 0 )
    PRINT_FIELD(type);
  if (! FPZERO(d_visibilityLimit.get())) PRINT_FIELD(visibilityLimit);

  return os;
}


void VrmlNodeNavigationInfo::eventIn(double timeStamp,
				     const char *eventName,
				     const VrmlField *fieldValue)
{
  if (strcmp(eventName, "set_bind") == 0)
    {
      VrmlNodeNavigationInfo *current = d_scene->bindableNavigationInfoTop();
      const VrmlSFBool *b = fieldValue->toSFBool();
      
      if (! b)
	{
	  cerr << "Error: invalid value for NavigationInfo::set_bind eventIn "
	       << (*fieldValue) << endl;
	  return;
	}

      if ( b->get() )		// set_bind TRUE
	{
	  if (this != current)
	    {
	      if (current)
		current->eventOut( timeStamp, "isBound", VrmlSFBool(false));
	      d_scene->bindablePush( this );
	      eventOut( timeStamp, "isBound", VrmlSFBool(true) );
	    }
	}
      else			// set_bind FALSE
	{
	  d_scene->bindableRemove( this );
	  if (this == current)
	    {
	      eventOut( timeStamp, "isBound", VrmlSFBool(false));
	      current = d_scene->bindableNavigationInfoTop();
	      if (current)
		current->eventOut( timeStamp, "isBound", VrmlSFBool(true) );
	    }
	}
    }

  else
    {
      VrmlNode::eventIn(timeStamp, eventName, fieldValue);
    }
}

// Set the value of one of the node fields.

void VrmlNodeNavigationInfo::setField(const char *fieldName,
				      const VrmlField &fieldValue)
{
  if TRY_FIELD(avatarSize, MFFloat)
  else if TRY_FIELD(headlight, SFBool)
  else if TRY_FIELD(speed, SFFloat)
  else if TRY_FIELD(type, MFString)
  else if TRY_FIELD(visibilityLimit, SFFloat)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

