//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeLOD.cpp

#include "VrmlNodeLOD.h"
#include "VrmlNodeType.h"

#include "MathUtils.h"
#include "Viewer.h"


// Return a new VrmlNodeLOD
static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeLOD(s); }


// Define the built in VrmlNodeType:: "LOD" fields

VrmlNodeType *VrmlNodeLOD::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("LOD", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("level", VrmlField::MFNODE);
  t->addField("center", VrmlField::SFVEC3F);
  t->addField("range", VrmlField::MFFLOAT);

  return t;
}


VrmlNodeType *VrmlNodeLOD::nodeType() const { return defineType(0); }


VrmlNodeLOD::VrmlNodeLOD(VrmlScene *scene) :
  VrmlNodeChild(scene)
{
}

VrmlNodeLOD::~VrmlNodeLOD()
{
}


VrmlNode *VrmlNodeLOD::cloneMe() const
{
  return new VrmlNodeLOD(*this);
}

void VrmlNodeLOD::cloneChildren(VrmlNamespace *ns)
{
  int n = d_level.size();
  VrmlNode **kids = d_level.get();
  for (int i = 0; i<n; ++i)
    {
      if (! kids[i]) continue;
      VrmlNode *newKid = kids[i]->clone(ns)->reference();
      kids[i]->dereference();
      kids[i] = newKid;
    }
}

bool VrmlNodeLOD::isModified() const
{
  if (d_modified) return true;
  
  int n = d_level.size();

  // This should really check which range is being rendered...
  for (int i = 0; i<n; ++i)
    if (d_level[i]->isModified())
      return true;

  return false;
}


void VrmlNodeLOD::clearFlags()
{
  VrmlNode::clearFlags();
  int n = d_level.size();
  for (int i = 0; i<n; ++i)
    d_level[i]->clearFlags();
}

void VrmlNodeLOD::addToScene( VrmlScene *s, const char *rel )
{
  d_scene = s;
  
  int n = d_level.size();

  for (int i = 0; i<n; ++i)
    d_level[i]->addToScene(s, rel);
}

void VrmlNodeLOD::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);
  
  int n = d_level.size();
  for (int i = 0; i<n; ++i)
    d_level[i]->copyRoutes(ns);
}


ostream& VrmlNodeLOD::printFields(ostream& os, int indent)
{
  if (d_level.size() > 0) PRINT_FIELD(level);
  if (! FPZERO(d_center.x()) ||
      ! FPZERO(d_center.y()) ||
      ! FPZERO(d_center.z()) )
    PRINT_FIELD(center);
      
  if (d_range.size() > 0) PRINT_FIELD(range);

  return os;
}


// Render one of the children

void VrmlNodeLOD::render(Viewer *viewer)
{
  clearModified();
  if (d_level.size() <= 0) return;

  float x, y, z;
  viewer->getPosition( &x, &y, &z );

  float dx = x - d_center.x();
  float dy = y - d_center.y();
  float dz = z - d_center.z();
  float d2 = dx*dx + dy*dy + dz*dz;

  int i, n = d_range.size();
  for (i=0; i<n; ++i)
    if (d2 < d_range[i] * d_range[i])
      break;

  // Should choose an "optimal" level...
  if (d_range.size() == 0) i = d_level.size() - 1;

  // Not enough levels...
  if (i >= d_level.size()) i = d_level.size() - 1;

  //printf("LOD d2 %g level %d\n", d2, i);

  d_level[i]->render(viewer);

  // Don't re-render on their accounts
  n = d_level.size();
  for (i = 0; i<n; ++i)
    d_level[i]->clearModified();
}


// Set the value of one of the node fields.
void VrmlNodeLOD::setField(const char *fieldName,
			   const VrmlField &fieldValue)
{
  if TRY_FIELD(level, MFNode)
  else if TRY_FIELD(center, SFVec3f)
  else if TRY_FIELD(range, MFFloat)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

VrmlNodeLOD* VrmlNodeLOD::toLOD() const 
{ return (VrmlNodeLOD*) this; }

const VrmlMFFloat& VrmlNodeLOD::getRange() const  
{   return d_range; }

const VrmlSFVec3f& VrmlNodeLOD::getCenter() const   
{  return d_center; }

