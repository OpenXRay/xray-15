//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeViewpoint.cpp

#include "VrmlNodeViewpoint.h"
#include "MathUtils.h"
#include "VrmlNodeType.h"
#include "VrmlScene.h"
#include "Viewer.h"

#if defined AW_NEW_IOSTREAMS
#  include <iostream>
#else
#  include <iostream.h>
#endif


//  Viewpoint factory.

static VrmlNode *creator(VrmlScene *scene)
{
  return new VrmlNodeViewpoint(scene);
}


// Define the built in VrmlNodeType:: "Viewpoint" fields

VrmlNodeType *VrmlNodeViewpoint::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Viewpoint", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addEventIn("set_bind", VrmlField::SFBOOL);
  t->addExposedField("fieldOfView", VrmlField::SFFLOAT);
  t->addExposedField("jump", VrmlField::SFBOOL);
  t->addExposedField("orientation", VrmlField::SFROTATION);
  t->addExposedField("position", VrmlField::SFVEC3F);
  t->addField("description", VrmlField::SFSTRING);
  t->addEventOut("bindTime", VrmlField::SFTIME);
  t->addEventOut("isBound", VrmlField::SFBOOL);

  return t;
}

VrmlNodeType *VrmlNodeViewpoint::nodeType() const { return defineType(0); }


static const float DEFAULT_FIELD_OF_VIEW = 0.785398;

VrmlNodeViewpoint::VrmlNodeViewpoint(VrmlScene *scene) :
  VrmlNodeChild(scene),
  d_fieldOfView(DEFAULT_FIELD_OF_VIEW),
  d_jump(true),
  d_orientation(0.0, 0.0, 1.0, 0.0),
  d_position(0.0, 0.0, 10.0),
  d_parentTransform(0)
{
  if (d_scene) d_scene->addViewpoint(this);
}

// need copy constructor for d_parentTransform ...

VrmlNodeViewpoint::~VrmlNodeViewpoint()
{
  if (d_scene) d_scene->removeViewpoint(this);
}


VrmlNode *VrmlNodeViewpoint::cloneMe() const
{
  return new VrmlNodeViewpoint(*this);
}


VrmlNodeViewpoint* VrmlNodeViewpoint::toViewpoint() const
{ return (VrmlNodeViewpoint*) this; }

void VrmlNodeViewpoint::addToScene(VrmlScene *s, const char *)
{ if (d_scene != s && (d_scene = s) != 0) d_scene->addViewpoint(this); }


ostream& VrmlNodeViewpoint::printFields(ostream& os, int indent)
{
  if (! FPEQUAL( d_fieldOfView.get(), DEFAULT_FIELD_OF_VIEW))
    PRINT_FIELD(fieldOfView);
  if (! d_jump.get()) PRINT_FIELD(jump);
  if (! FPZERO(d_orientation.x()) ||
      ! FPZERO(d_orientation.y()) ||
      ! FPEQUAL(d_orientation.z(), 1.0) ||
      ! FPZERO(d_orientation.r()) )
    PRINT_FIELD(orientation);
  if (! FPZERO(d_position.x()) ||
      ! FPZERO(d_position.y()) ||
      ! FPEQUAL(d_position.z(), 10.0) )
    PRINT_FIELD(position);
  if (d_description.get()) PRINT_FIELD(description);

  return os;
}

// Cache a pointer to (one of the) parent transforms for proper
// rendering of bindables.

void VrmlNodeViewpoint::accumulateTransform( VrmlNode *parent )
{
  d_parentTransform = parent;
}

VrmlNode* VrmlNodeViewpoint::getParentTransform() { return d_parentTransform; }


void VrmlNodeViewpoint::eventIn(double timeStamp,
				const char *eventName,
				const VrmlField *fieldValue)
{
  if (strcmp(eventName, "set_bind") == 0)
    {
      VrmlNodeViewpoint *current = d_scene->bindableViewpointTop();
      const VrmlSFBool *b = fieldValue->toSFBool();
      
      if (! b)
	{
	  cerr << "Error: invalid value for Viewpoint::set_bind eventIn "
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
#ifdef VP_TESTING
	      const char *n = name();
	      const char *d = d_description.get();
	      if ( *n && d && *d )
		theSystem->inform("%s: %s", n, d);
	      else if ( d && *d )
		theSystem->inform("%s", d);
	      else if ( *n )
		theSystem->inform("%s", n);
#endif
	  }
	}
      else			// set_bind FALSE
	{
	  d_scene->bindableRemove( this );
	  if (this == current)
	    {
	      eventOut( timeStamp, "isBound", VrmlSFBool(false));
	      current = d_scene->bindableViewpointTop();
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

void VrmlNodeViewpoint::setField(const char *fieldName,
				 const VrmlField &fieldValue)
{
  if TRY_FIELD(fieldOfView, SFFloat)
  else if TRY_FIELD(jump, SFBool)
  else if TRY_FIELD(orientation, SFRotation)
  else if TRY_FIELD(position, SFVec3f)
  else if TRY_FIELD(description, SFString)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

