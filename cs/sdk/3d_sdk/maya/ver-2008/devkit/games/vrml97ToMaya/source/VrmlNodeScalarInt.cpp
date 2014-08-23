//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeScalarInt.cpp

#include "VrmlNodeScalarInt.h"
#include "VrmlNodeType.h"


#include "VrmlScene.h"

// ScalarInt factory.

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeScalarInt(scene);
}


// Define the built in VrmlNodeType:: "ScalarInterpolator" fields

VrmlNodeType *VrmlNodeScalarInt::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("ScalarInterpolator", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addEventIn("set_fraction", VrmlField::SFFLOAT);
  t->addExposedField("key", VrmlField::MFFLOAT);
  t->addExposedField("keyValue", VrmlField::MFFLOAT);
  t->addEventOut("value_changed", VrmlField::SFFLOAT);

  return t;
}

VrmlNodeType *VrmlNodeScalarInt::nodeType() const { return defineType(0); }


VrmlNodeScalarInt::VrmlNodeScalarInt( VrmlScene *scene ) :
  VrmlNodeChild(scene)
{
}

VrmlNodeScalarInt::~VrmlNodeScalarInt()
{
}


VrmlNode *VrmlNodeScalarInt::cloneMe() const
{
  return new VrmlNodeScalarInt(*this);
}


ostream& VrmlNodeScalarInt::printFields(ostream& os, int indent)
{
  if (d_key.size() > 0) PRINT_FIELD(key);
  if (d_keyValue.size() > 0) PRINT_FIELD(keyValue);

  return os;
}

void VrmlNodeScalarInt::eventIn(double timeStamp,
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

      int n = d_key.size() - 1;
      if (f < d_key[0])
	d_value.set( d_keyValue[0] );
      else if (f > d_key[n])
	d_value.set( d_keyValue[n] );
      else
	{
	  for (int i=0; i<n; ++i)
	    if (d_key[i] <= f && f <= d_key[i+1])
	      {
		float v1 = d_keyValue[i];
		float v2 = d_keyValue[i+1];

		f = (f - d_key[i]) / (d_key[i+1] - d_key[i]);
		d_value.set( v1 + f * (v2 - v1) );
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

void VrmlNodeScalarInt::setField(const char *fieldName,
				 const VrmlField &fieldValue)
{
  if TRY_FIELD(key, MFFloat)
  else if TRY_FIELD(keyValue, MFFloat)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

VrmlNodeScalarInt* VrmlNodeScalarInt::toScalarInt() const
{ return (VrmlNodeScalarInt*) this; }

const VrmlMFFloat& VrmlNodeScalarInt::getKey() const 
{   return d_key; }

const VrmlMFFloat& VrmlNodeScalarInt::getKeyValue() const 
{ return d_keyValue; }
