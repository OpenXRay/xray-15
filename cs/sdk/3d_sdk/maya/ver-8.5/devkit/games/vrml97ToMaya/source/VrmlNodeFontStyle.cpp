//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeFontStyle.cpp

#include "VrmlNodeFontStyle.h"
#include "VrmlNodeType.h"
#include "MathUtils.h"

static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeFontStyle(s); }


// Define the built in VrmlNodeType:: "FontStyle" fields

VrmlNodeType *VrmlNodeFontStyle::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("FontStyle", creator);
    }

  VrmlNode::defineType(t);	// Parent class
  t->addField("family", VrmlField::MFSTRING);
  t->addField("horizontal", VrmlField::SFBOOL);
  t->addField("justify", VrmlField::MFSTRING);
  t->addField("language", VrmlField::SFSTRING);
  t->addField("leftToRight", VrmlField::SFBOOL);
  t->addField("size", VrmlField::SFFLOAT);
  t->addField("spacing", VrmlField::SFFLOAT);
  t->addField("style", VrmlField::SFSTRING);
  t->addField("topToBottom", VrmlField::SFBOOL);

  return t;
}


VrmlNodeType *VrmlNodeFontStyle::nodeType() const { return defineType(0); }


VrmlNodeFontStyle::VrmlNodeFontStyle(VrmlScene *scene) :
  VrmlNode(scene),
  d_family("SERIF"),
  d_horizontal(true),
  d_justify("BEGIN"),
  d_leftToRight(true),
  d_size(1.0),
  d_spacing(1.0),
  d_style("PLAIN"),
  d_topToBottom(true)
{
}

VrmlNodeFontStyle::~VrmlNodeFontStyle()
{
}


VrmlNode *VrmlNodeFontStyle::cloneMe() const
{
  return new VrmlNodeFontStyle(*this);
}


VrmlNodeFontStyle* VrmlNodeFontStyle::toFontStyle() const
{ return (VrmlNodeFontStyle*) this; }


ostream& VrmlNodeFontStyle::printFields(ostream& os, int indent)
{
  if (d_family.size() > 1 ||
      (d_family.size() == 1 && strcmp(d_family[0],"SERIF")) )
    PRINT_FIELD(family);
  if (! d_horizontal.get()) PRINT_FIELD(horizontal);
  if (d_justify.size() > 1 ||
      (d_justify.size() == 1 && strcmp(d_justify[0],"BEGIN")) )
    PRINT_FIELD(justify);
  if (d_language.get() && strcmp(d_language.get(), ""))
    PRINT_FIELD(language);
  if (! d_leftToRight.get()) PRINT_FIELD(leftToRight);
  if (! FPEQUAL(d_size.get(), 1.0)) PRINT_FIELD(size);
  if (! FPEQUAL(d_spacing.get(), 1.0)) PRINT_FIELD(spacing);
  if (d_style.get() && strcmp(d_style.get(), "PLAIN"))
    PRINT_FIELD(style);
  if (! d_topToBottom.get()) PRINT_FIELD(topToBottom);

  return os;
}


// Set the value of one of the node fields.

void VrmlNodeFontStyle::setField(const char *fieldName,
				 const VrmlField &fieldValue)
{
  if TRY_FIELD(family, MFString)
  else if TRY_FIELD(horizontal, SFBool)
  else if TRY_FIELD(justify, MFString)		
  else if TRY_FIELD(language, SFString)		
  else if TRY_FIELD(leftToRight, SFBool)
  else if TRY_FIELD(size, SFFloat)
  else if TRY_FIELD(spacing, SFFloat)
  else if TRY_FIELD(style, SFString)		
  else if TRY_FIELD(topToBottom, SFBool)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

