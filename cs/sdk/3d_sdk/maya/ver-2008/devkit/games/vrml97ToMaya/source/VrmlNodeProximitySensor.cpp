//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeProximitySensor.cpp

#include "config.h"
#include "VrmlNodeProximitySensor.h"
#include "VrmlNodeType.h"

#include "MathUtils.h"
#include "System.h"
#include "Viewer.h"
#include "VrmlScene.h"

// ProximitySensor factory. 

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeProximitySensor(scene);
}


// Define the built in VrmlNodeType:: "ProximitySensor" fields

VrmlNodeType *VrmlNodeProximitySensor::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("ProximitySensor", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("center", VrmlField::SFVEC3F);
  t->addExposedField("enabled", VrmlField::SFBOOL);
  t->addExposedField("size", VrmlField::SFVEC3F);
  t->addEventOut("enterTime", VrmlField::SFTIME);
  t->addEventOut("exitTime", VrmlField::SFTIME);
  t->addEventOut("isActive", VrmlField::SFBOOL);
  t->addEventOut("orientation_changed", VrmlField::SFROTATION);
  t->addEventOut("position_changed", VrmlField::SFVEC3F);

  return t;
}

VrmlNodeType *VrmlNodeProximitySensor::nodeType() const { return defineType(0); }


VrmlNodeProximitySensor::VrmlNodeProximitySensor( VrmlScene *scene ) :
  VrmlNodeChild(scene),
  d_center(0.0, 0.0, 0.0),
  d_enabled(true),
  d_size(0.0, 0.0, 0.0),
  d_isActive(false),
  d_position(0.0, 0.0, 0.0),
  d_enterTime(0.0),
  d_exitTime(0.0)
{
  setModified();
}

VrmlNodeProximitySensor::~VrmlNodeProximitySensor()
{
}

VrmlNode *VrmlNodeProximitySensor::cloneMe() const
{
  return new VrmlNodeProximitySensor(*this);
}


ostream& VrmlNodeProximitySensor::printFields(ostream& os, int indent)
{
  if (! FPZERO(d_center.x()) ||
      ! FPZERO(d_center.y()) ||
      ! FPZERO(d_center.z()) )
    PRINT_FIELD(center);
  if (! d_enabled.get())
    PRINT_FIELD(enabled);
  if (! FPZERO(d_size.x()) ||
      ! FPZERO(d_size.y()) ||
      ! FPZERO(d_size.z()) )
    PRINT_FIELD(size);
      
  return os;
}

//
// Generate proximity events. If necessary, events prior to the current
// time are generated due to interpolation of enterTimes and exitTimes. 
// The timestamp should never be increased.
//
// This is in a render() method since the it needs the viewer position
// with respect to the local coordinate system.
// Could do this with VrmlNode::inverseTransform(double [4][4]) now...
//
// The positions and times are not interpolated to report the exact
// place and time of entries and exits from the sensor regions as
// required by the spec, since that would require the last viewer position
// to be stored in the node, which is problematic in the presence of
// DEF/USEd nodes...
// I suppose the scene could keep the last viewer position in the global
// coordinate system and it could be transformed all the way down the
// scenegraph, but that sounds painful.

void VrmlNodeProximitySensor::render(Viewer *viewer)
{
  if (d_enabled.get() &&
      d_size.x() > 0.0 &&
      d_size.y() > 0.0 &&
      d_size.z() > 0.0 &&
      viewer->getRenderMode() == Viewer::RENDER_MODE_DRAW)
    {
      VrmlSFTime timeNow( theSystem->time() );
      float x, y, z;

      // Is viewer inside the box?
      viewer->getPosition( &x, &y, &z );
      bool inside = (fabs(double(x - d_center.x())) <= 0.5 * d_size.x() &&
 		     fabs(double(y - d_center.y())) <= 0.5 * d_size.y() &&
 		     fabs(double(z - d_center.z())) <= 0.5 * d_size.z());
      bool wasIn = d_isActive.get();

      // Check if viewer has entered the box
      if (inside && ! wasIn)
	{
	  theSystem->debug("PS.%s::render ENTER %g %g %g\n", name(), x, y, z);

	  d_isActive.set(true);
	  eventOut(timeNow.get(), "isActive", d_isActive);

	  d_enterTime = timeNow;
	  eventOut(timeNow.get(), "enterTime", d_enterTime);
	}

      // Check if viewer has left the box
      else if (wasIn && ! inside)
	{
	  theSystem->debug("PS.%s::render EXIT %g %g %g\n", name(), x, y, z);

	  d_isActive.set(false);
	  eventOut(timeNow.get(), "isActive", d_isActive );

	  d_exitTime = timeNow;
	  eventOut(timeNow.get(), "exitTime", d_exitTime);
	}

      // Check for movement within the box
      if (wasIn || inside)
	{
	  if ( ! FPEQUAL(d_position.x(), x) ||
	       ! FPEQUAL(d_position.y(), y) ||
	       ! FPEQUAL(d_position.z(), z) )
	    {
	      d_position.set( x, y, z );
	      eventOut(timeNow.get(), "position_changed", d_position);
	    }

	  float xyzr[4];
	  viewer->getOrientation( xyzr );
	  if ( ! FPEQUAL(d_orientation.x(), xyzr[0]) ||
	       ! FPEQUAL(d_orientation.y(), xyzr[1]) ||
	       ! FPEQUAL(d_orientation.z(), xyzr[2]) ||
	       ! FPEQUAL(d_orientation.r(), xyzr[3]) )
	    {
	      d_orientation.set( xyzr[0], xyzr[1], xyzr[2], xyzr[3] );
	      eventOut(timeNow.get(), "orientation_changed", d_orientation);
	    }
	}
    }

  else
    clearModified();
}


// Set the value of one of the node fields.

void VrmlNodeProximitySensor::setField(const char *fieldName,
				       const VrmlField &fieldValue)
{
  if TRY_FIELD(center, SFVec3f)
  else if TRY_FIELD(enabled, SFBool)
  else if TRY_FIELD(size, SFVec3f)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

