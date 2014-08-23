//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeText.cpp

#include "VrmlNodeText.h"

#include "VrmlNodeType.h"
#include "VrmlNodeFontStyle.h"
#include "MathUtils.h"
#include "Viewer.h"


// Make a VrmlNodeText

static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeText(s); }


// Define the built in VrmlNodeType:: "Text" fields

VrmlNodeType *VrmlNodeText::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;	// Define type only once.
      t = st = new VrmlNodeType("Text", creator);
    }

  VrmlNodeGeometry::defineType(t);	// Parent class
  t->addExposedField("string", VrmlField::MFSTRING);
  t->addExposedField("fontStyle", VrmlField::SFNODE);
  t->addExposedField("length", VrmlField::MFFLOAT);
  t->addExposedField("maxExtent", VrmlField::SFFLOAT);

  return t;
}

VrmlNodeType *VrmlNodeText::nodeType() const { return defineType(0); }


VrmlNodeText::VrmlNodeText(VrmlScene *scene) :
  VrmlNodeGeometry(scene)
{
}

VrmlNodeText::~VrmlNodeText()
{
}


VrmlNode *VrmlNodeText::cloneMe() const
{
  return new VrmlNodeText(*this);
}

void VrmlNodeText::cloneChildren(VrmlNamespace *ns)
{
  if (d_fontStyle.get())
    d_fontStyle.set(d_fontStyle.get()->clone(ns));
}

bool VrmlNodeText::isModified() const
{
  return (VrmlNode::isModified() ||
	  (d_fontStyle.get() && d_fontStyle.get()->isModified()));
}

void VrmlNodeText::clearFlags()
{
  VrmlNode::clearFlags();
  if (d_fontStyle.get()) d_fontStyle.get()->clearFlags();
}

void VrmlNodeText::addToScene( VrmlScene *s, const char *relUrl )
{
  d_scene = s;
  if (d_fontStyle.get()) d_fontStyle.get()->addToScene(s, relUrl);
}

void VrmlNodeText::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);
  if (d_fontStyle.get()) d_fontStyle.get()->copyRoutes(ns);
}

ostream& VrmlNodeText::printFields(ostream& os, int indent)
{
  if (d_string.size() > 0) PRINT_FIELD(string);
  if (d_fontStyle.get()) PRINT_FIELD(fontStyle);
  if (d_length.size() > 0) PRINT_FIELD(length);
  if (! FPZERO(d_maxExtent.get()) ) PRINT_FIELD(maxExtent);
  return os;
}


Viewer::Object VrmlNodeText::insertGeometry(Viewer *viewer)
{
  char **s = d_string.get();

  if (s)
    {
      int justify[2] = { 1, 1 };
      float size = 1.0;
      VrmlNodeFontStyle *f = 0;
      if (d_fontStyle.get())
	f = d_fontStyle.get()->toFontStyle();

      if (f)
	{
	  VrmlMFString &j = f->justify();

	  for (int i=0; i<j.size(); ++i)
	    {
	      if (strcmp(j[i], "END") == 0)
		justify[i] = -1;
	      else if (strcmp(j[i], "MIDDLE") == 0)
		justify[i] = 0;
	    }
	  size = f->size();
	} 
    
      return viewer->insertText(justify, size, d_string.size(), s);
    }

  return 0;
}


// Set the value of one of the node fields.

void VrmlNodeText::setField(const char *fieldName,
			    const VrmlField &fieldValue)
{
  if TRY_FIELD(string, MFString)
  else if TRY_SFNODE_FIELD(fontStyle, FontStyle)
  else if TRY_FIELD(length, MFFloat)
  else if TRY_FIELD(maxExtent, SFFloat)
  else
    VrmlNodeGeometry::setField(fieldName, fieldValue);
}
