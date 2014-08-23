//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeTransform.cpp

#include "VrmlNodeTransform.h"
#include "MathUtils.h"
#include "VrmlNodeType.h"


static VrmlNode *creator(VrmlScene *s) { return new VrmlNodeTransform(s); }


// Define the built in VrmlNodeType:: "Transform" fields

VrmlNodeType *VrmlNodeTransform::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Transform", creator);
    }

  VrmlNodeGroup::defineType(t);	// Parent class
  t->addExposedField("center", VrmlField::SFVEC3F);
  t->addExposedField("rotation", VrmlField::SFROTATION);
  t->addExposedField("scale", VrmlField::SFVEC3F);
  t->addExposedField("scaleOrientation", VrmlField::SFROTATION);
  t->addExposedField("translation", VrmlField::SFVEC3F);

  return t;
}

VrmlNodeType *VrmlNodeTransform::nodeType() const { return defineType(0); }


VrmlNodeTransform::VrmlNodeTransform(VrmlScene *scene) :
  VrmlNodeGroup(scene),
  d_center(0.0, 0.0, 0.0),
  d_rotation(0.0, 0.0, 1.0, 0.0),
  d_scale(1.0, 1.0, 1.0),
  d_scaleOrientation(0.0, 0.0, 1.0, 0.0),
  d_translation(0.0, 0.0, 0.0),
  d_xformObject(0)
{
}

VrmlNodeTransform::~VrmlNodeTransform()
{
  // delete d_xformObject...
}

VrmlNodeTransform* VrmlNodeTransform::toTransform() const //LarryD Feb24/99
{ return (VrmlNodeTransform*) this; }

const VrmlSFVec3f& VrmlNodeTransform::getCenter() const   // LarryD Feb 18/99
{  return d_center; }

const VrmlSFRotation& VrmlNodeTransform::getRotation() const  //LarryD Feb 24/99
{ return d_rotation; }

const VrmlSFVec3f& VrmlNodeTransform::getScale() const //LarryD Feb 24/99
{ return d_scale ;}

const VrmlSFRotation& VrmlNodeTransform::getScaleOrientation() const //LarryD Feb 24/99
{ return d_scaleOrientation; }

const VrmlSFVec3f& VrmlNodeTransform::getTranslation() const  //LarryD Feb 24/99
{ return d_translation; }

VrmlNode *VrmlNodeTransform::cloneMe() const
{
  return new VrmlNodeTransform(*this);
}


ostream& VrmlNodeTransform::printFields(ostream& os, int indent)
{
  if (! FPZERO(d_center.x()) ||
      ! FPZERO(d_center.y()) ||
      ! FPZERO(d_center.z()))
    PRINT_FIELD(center);
  if (! FPZERO(d_rotation.x()) ||
      ! FPZERO(d_rotation.y()) ||
      ! FPEQUAL(d_rotation.z(), 1.0) ||
      ! FPZERO(d_rotation.r()))
    PRINT_FIELD(rotation);
  if (! FPEQUAL(d_scale.x(), 1.0) ||
      ! FPEQUAL(d_scale.y(), 1.0) ||
      ! FPEQUAL(d_scale.z(), 1.0))
    PRINT_FIELD(scale);
  if (! FPZERO(d_scaleOrientation.x()) ||
      ! FPZERO(d_scaleOrientation.y()) ||
      ! FPEQUAL(d_scaleOrientation.z(), 1.0) ||
      ! FPZERO(d_scaleOrientation.r()))
    PRINT_FIELD(scaleOrientation);
  if (! FPZERO(d_translation.x()) ||
      ! FPZERO(d_translation.y()) ||
      ! FPZERO(d_translation.z()))
    PRINT_FIELD(translation);

  VrmlNodeGroup::printFields(os, indent);
  return os;
}


void VrmlNodeTransform::render(Viewer *viewer)
{
  if ( d_xformObject && isModified() )
    {
      viewer->removeObject(d_xformObject);
      d_xformObject = 0;
    }

  if (d_xformObject)
    viewer->insertReference(d_xformObject);

  else if (d_children.size() > 0)
    {
      d_xformObject = viewer->beginObject(name());

      // Apply transforms
      viewer->setTransform(d_center.get(),
			   d_rotation.get(),
			   d_scale.get(),
			   d_scaleOrientation.get(),
			   d_translation.get());

      // Render children
      VrmlNodeGroup::render(viewer);

      // Reverse transforms (for immediate mode/no matrix stack renderer)
      viewer->unsetTransform(d_center.get(),
			     d_rotation.get(),
			     d_scale.get(),
			     d_scaleOrientation.get(),
			     d_translation.get());
      viewer->endObject();
    }

  clearModified();
}


// Set the value of one of the node fields.

void VrmlNodeTransform::setField(const char *fieldName,
				 const VrmlField &fieldValue)
{
  if TRY_FIELD(center, SFVec3f)
  else if TRY_FIELD(rotation, SFRotation)
  else if TRY_FIELD(scale, SFVec3f)
  else if TRY_FIELD(scaleOrientation, SFRotation)
  else if TRY_FIELD(translation, SFVec3f)
  else
    VrmlNodeGroup::setField(fieldName, fieldValue);
}


// Cache a pointer to (one of the) parent transforms for proper
// rendering of bindables.

void VrmlNodeTransform::accumulateTransform( VrmlNode *parent )
{
  d_parentTransform = parent;

  int i, n = d_children.size();

  for (i = 0; i<n; ++i)
    {
      VrmlNode *kid = d_children[i];
      kid->accumulateTransform( this );
    }
}


void VrmlNodeTransform::inverseTransform(Viewer *viewer)
{
  VrmlNode *parentTransform = getParentTransform();
  if (parentTransform)
    parentTransform->inverseTransform(viewer);

  // Build the inverse
  float trans[3] = { - d_translation.x(),
		     - d_translation.y(),
		     - d_translation.z() };
  float rot[4] = { d_rotation.x(),
		   d_rotation.y(),
		   d_rotation.z(),
		   -  d_rotation.r() };

  // Invert scale (1/x)
  float scale[3] = { d_scale.x(), d_scale.y(), d_scale.z() };
  if (! FPZERO(scale[0])) scale[0] = 1.0f / scale[0];
  if (! FPZERO(scale[1])) scale[1] = 1.0f / scale[1];
  if (! FPZERO(scale[2])) scale[2] = 1.0f / scale[2];

  // Apply transforms (this may need to be broken into separate
  // calls to perform the ops in reverse order...)
  viewer->setTransform( d_center.get(),
			rot,
			scale,
			d_scaleOrientation.get(),
			trans );
}

void VrmlNodeTransform::inverseTransform(double m[4][4])
{
  VrmlNode *parentTransform = getParentTransform();
  if (parentTransform)
    parentTransform->inverseTransform(m);
  else
    Midentity(m);

  // Invert this transform
  float rot[4] = { d_rotation.x(),
		   d_rotation.y(),
		   d_rotation.z(),
		   - d_rotation.r() };
  double M[4][4];
  Mrotation( M, rot );
  MM( m, M );

  // Invert scale (1/x)
  float scale[3] = { d_scale.x(), d_scale.y(), d_scale.z() };
  if (! FPZERO(scale[0])) scale[0] = 1.0f / scale[0];
  if (! FPZERO(scale[1])) scale[1] = 1.0f / scale[1];
  if (! FPZERO(scale[2])) scale[2] = 1.0f / scale[2];

  Mscale( M, scale );
  MM( m, M );
}

