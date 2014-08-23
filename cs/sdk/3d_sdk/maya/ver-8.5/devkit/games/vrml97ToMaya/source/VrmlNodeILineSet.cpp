//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeILineSet.cpp

#include "VrmlNodeILineSet.h"

#include "VrmlNodeType.h"
#include "VrmlNodeColor.h"
#include "VrmlNodeCoordinate.h"
#include "VrmlNodeTextureCoordinate.h"

#include "Viewer.h"


static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeILineSet(s); }


// Define the built in VrmlNodeType:: "IndexedLineSet" fields

VrmlNodeType *VrmlNodeILineSet::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("IndexedLineSet", creator);
    }

  VrmlNodeIndexedSet::defineType(t);	// Parent class

  return t;
}

VrmlNodeType *VrmlNodeILineSet::nodeType() const { return defineType(0); }


VrmlNodeILineSet::VrmlNodeILineSet(VrmlScene *scene) :
  VrmlNodeIndexedSet(scene)
{
}

VrmlNodeILineSet::~VrmlNodeILineSet()
{
}


VrmlNode *VrmlNodeILineSet::cloneMe() const
{
  return new VrmlNodeILineSet(*this);
}

void VrmlNodeILineSet::cloneChildren(VrmlNamespace* ns)
{
  if (d_color.get())
    d_color.set(d_color.get()->clone(ns));
  if (d_coord.get())
    d_coord.set(d_coord.get()->clone(ns));
}


// TO DO colors

Viewer::Object VrmlNodeILineSet::insertGeometry(Viewer *viewer)
{
  Viewer::Object obj = 0;
  if (d_coord.get() && d_coordIndex.size() > 0)
    {
      VrmlMFVec3f &coord = d_coord.get()->toCoordinate()->coordinate();
      int nvert = coord.size();
      float *color = 0;
      int nci = 0, *ci = 0;

      // check #colors is consistent with colorPerVtx, colorIndex...
      if (d_color.get())
	{
	  VrmlMFColor &c = d_color.get()->toColor()->color();
	  color = &c[0][0];
	  nci = d_colorIndex.size();
	  if (nci) ci = d_colorIndex.get();
	}

      obj =  viewer->insertLineSet(nvert,
				   &coord[0][0],
				   d_coordIndex.size(),
				   &d_coordIndex[0],
				   d_colorPerVertex.get(),
				   color,
				   nci, ci);

    }
  
  if (d_color.get()) d_color.get()->clearModified();
  if (d_coord.get()) d_coord.get()->clearModified();

  return obj;
}


