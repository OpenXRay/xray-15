//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeCoordinateInt.cpp

#include "VrmlNodeCoordinateInt.h"
#include "VrmlNodeType.h"


#include "VrmlScene.h"

// CoordinateInt factory.

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeCoordinateInt(scene);
}


// Define the built in VrmlNodeType:: "CoordinateInterpolator" fields

VrmlNodeType *VrmlNodeCoordinateInt::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("CoordinateInterpolator", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addEventIn("set_fraction", VrmlField::SFFLOAT);
  t->addExposedField("key", VrmlField::MFFLOAT);
  t->addExposedField("keyValue", VrmlField::MFVEC3F);
  t->addEventOut("value_changed", VrmlField::MFVEC3F);

  return t;
}

VrmlNodeType *VrmlNodeCoordinateInt::nodeType() const { return defineType(0); }


VrmlNodeCoordinateInt::VrmlNodeCoordinateInt( VrmlScene *scene ) :
  VrmlNodeChild(scene)
{
}

VrmlNodeCoordinateInt::~VrmlNodeCoordinateInt()
{
}

VrmlNode *VrmlNodeCoordinateInt::cloneMe() const
{
  return new VrmlNodeCoordinateInt(*this);
}


ostream& VrmlNodeCoordinateInt::printFields(ostream& os, int indent)
{
  if (d_key.size() > 0) PRINT_FIELD(key);
  if (d_keyValue.size() > 0) PRINT_FIELD(keyValue);

  return os;
}

void VrmlNodeCoordinateInt::eventIn(double timeStamp,
				    const char *eventName,
				    const VrmlField *fieldValue)
{
  if (strcmp(eventName, "set_fraction") == 0)
    {
      if (! fieldValue->toSFFloat() )
	{
	  theSystem->error
	    ("Invalid type for %s eventIn %s (expected SFFloat).\n",
	     nodeType()->getName(), eventName);
	  return;
	}
      float f = fieldValue->toSFFloat()->get();

      int nCoords = d_keyValue.size() / d_key.size();
      int n = d_key.size() - 1;

      if (f < d_key[0])
	{
	  d_value.set( nCoords, d_keyValue[0] );
	}
      else if (f > d_key[n])
	{
	  d_value.set( nCoords, d_keyValue[n*nCoords] );
	}
      else
	{
	  // Reserve enough space for the new value
	  d_value.set( nCoords, 0 );

	  for (int i=0; i<n; ++i)
	    if (d_key[i] <= f && f <= d_key[i+1])
	      {
		float *v1 = d_keyValue[i*nCoords];
		float *v2 = d_keyValue[(i+1)*nCoords];
		float *x = d_value.get();

		f = (f - d_key[i]) / (d_key[i+1] - d_key[i]);

		for (int j=0; j<nCoords; ++j)
		  {
		    *x++ = v1[0] + f * (v2[0] - v1[0]);
		    *x++ = v1[1] + f * (v2[1] - v1[1]);
		    *x++ = v1[2] + f * (v2[2] - v1[2]);
		    v1 += 3;
		    v2 += 3;
		  }

		break;
	      }
	}

      // Send the new value
      eventOut(timeStamp, "value_changed", d_value);
    }

  // Check exposedFields
  else
    {
      VrmlNode::eventIn(timeStamp, eventName, fieldValue);

      // This node is not renderable, so don't re-render on changes to it.
      clearModified();
    }
}


// Set the value of one of the node fields.

void VrmlNodeCoordinateInt::setField(const char *fieldName,
				     const VrmlField &fieldValue)
{
  if TRY_FIELD(key, MFFloat)
  else if TRY_FIELD(keyValue, MFVec3f)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

