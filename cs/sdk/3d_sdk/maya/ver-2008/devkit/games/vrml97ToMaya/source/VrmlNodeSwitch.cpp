//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeSwitch.cpp

#include "VrmlNodeSwitch.h"
#include "VrmlNodeType.h"

#include "Viewer.h"


// Return a new VrmlNodeSwitch
static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeSwitch(s); }


// Define the built in VrmlNodeType:: "Switch" fields

VrmlNodeType *VrmlNodeSwitch::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Switch", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("choice", VrmlField::MFNODE);
  t->addExposedField("whichChoice", VrmlField::SFINT32);

  return t;
}

VrmlNodeType *VrmlNodeSwitch::nodeType() const { return defineType(0); }


VrmlNodeSwitch::VrmlNodeSwitch(VrmlScene *scene) :
  VrmlNodeChild(scene),
  d_whichChoice(-1)
{
}

VrmlNodeSwitch::~VrmlNodeSwitch()
{
}


VrmlNode *VrmlNodeSwitch::cloneMe() const
{
  return new VrmlNodeSwitch(*this);
}

void VrmlNodeSwitch::cloneChildren(VrmlNamespace *ns)
{
  int n = d_choice.size();
  VrmlNode **kids = d_choice.get();
  for (int i = 0; i<n; ++i)
    {
      if (! kids[i]) continue;
      VrmlNode *newKid = kids[i]->clone(ns)->reference();
      kids[i]->dereference();
      kids[i] = newKid;
    }
}


bool VrmlNodeSwitch::isModified() const
{
  if (d_modified) return true;

  int w = d_whichChoice.get();

  return (w >= 0 && w < d_choice.size() && d_choice[w]->isModified());
}


void VrmlNodeSwitch::clearFlags()
{
  VrmlNode::clearFlags();
  
  int n = d_choice.size();
  for (int i = 0; i<n; ++i)
    d_choice[i]->clearFlags();
}

void VrmlNodeSwitch::addToScene( VrmlScene *s, const char *rel )
{
  d_scene = s;
  
  int n = d_choice.size();

  for (int i = 0; i<n; ++i)
    d_choice[i]->addToScene(s, rel);
}

void VrmlNodeSwitch::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);
  
  int n = d_choice.size();
  for (int i = 0; i<n; ++i)
    d_choice[i]->copyRoutes(ns);
}


ostream& VrmlNodeSwitch::printFields(ostream& os, int indent)
{
  if (d_choice.size() > 0) PRINT_FIELD(choice);
  if (d_whichChoice.get() != -1) PRINT_FIELD(whichChoice);
  return os;
}


// Render the selected child

void VrmlNodeSwitch::render(Viewer *viewer)
{
  int w = d_whichChoice.get();

  if (w >= 0 && w < d_choice.size())
    d_choice[w]->render(viewer);

  clearModified();
}


// Set the value of one of the node fields.
void VrmlNodeSwitch::setField(const char *fieldName,
			      const VrmlField &fieldValue)
{
  if TRY_FIELD(choice, MFNode)
  else if TRY_FIELD(whichChoice, SFInt)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

VrmlNodeSwitch* VrmlNodeSwitch::toSwitch() const //LarryD
{ return (VrmlNodeSwitch*) this; }
