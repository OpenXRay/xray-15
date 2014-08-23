//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeInline.cpp
//

#include "config.h"
#include "VrmlNodeInline.h"

#include <errno.h>

#include "VrmlNamespace.h"
#include "VrmlNodeType.h"
#include "Doc.h"
#include "MathUtils.h"
#include "VrmlScene.h"

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeInline(scene);
}


// Define the built in VrmlNodeType:: "Inline" fields

VrmlNodeType *VrmlNodeInline::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;
  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Inline", creator);
    }

  // Having Inline a subclass of Group is not right since
  // Groups have an exposedField "children" and eventIns
  // addChildren/deleteChildren that Inlines don't support...
  VrmlNodeGroup::defineType(t);	// Parent class
  t->addExposedField("url", VrmlField::MFSTRING);

  return t;
}

VrmlNodeType *VrmlNodeInline::nodeType() const { return defineType(0); }


VrmlNodeInline::VrmlNodeInline(VrmlScene *scene) :
  VrmlNodeGroup(scene),
  d_namespace(0),
  d_hasLoaded(false)
{
}

VrmlNodeInline::~VrmlNodeInline()
{
  delete d_namespace;
}


VrmlNode *VrmlNodeInline::cloneMe() const
{
  return new VrmlNodeInline(*this);
}


VrmlNodeInline* VrmlNodeInline::toInline() const
{ return (VrmlNodeInline*) this; }


// Inlines are loaded during addToScene traversal

void VrmlNodeInline::addToScene(VrmlScene *s, const char *relativeUrl)
{
  d_scene = s;
  load(relativeUrl);
  VrmlNodeGroup::addToScene(s, relativeUrl);
}


ostream& VrmlNodeInline::printFields(ostream& os, int indent)
{
  if ( !FPZERO(d_bboxCenter.x()) ||
       !FPZERO(d_bboxCenter.y()) ||
       !FPZERO(d_bboxCenter.z()) )
    PRINT_FIELD(bboxCenter);

  if ( !FPEQUAL(d_bboxSize.x(), -1) ||
       !FPEQUAL(d_bboxSize.y(), -1) ||
       !FPEQUAL(d_bboxSize.z(), -1) )
    PRINT_FIELD(bboxCenter);

  if (d_url.get()) PRINT_FIELD(url);

  return os;
}


// Set the value of one of the node fields.

void VrmlNodeInline::setField(const char *fieldName,
			      const VrmlField &fieldValue)
{
  if TRY_FIELD(url, MFString)
  else
    VrmlNodeGroup::setField(fieldName, fieldValue);
}

//  Load the children from the URL

void VrmlNodeInline::load(const char *relativeUrl)
{
  // Already loaded? Need to check whether Url has been modified...
  if (d_hasLoaded) return;

  d_hasLoaded = true;		// although perhaps not successfully

  if (d_url.size() > 0)
    {
      VrmlNamespace *ns = new VrmlNamespace();
      VrmlMFNode *kids = 0;
      Doc url;
      int i, n = d_url.size();
      for (i=0; i<n; ++i)
	{
	  Doc relDoc( relativeUrl );
	  theSystem->debug("Trying to read url '%s' (relative %s)\n",
			   d_url.get(i), d_relative.get() ? d_relative.get() : "<null>");
	  url.seturl( d_url.get(i), &relDoc );

	  kids = VrmlScene::readWrl( &url, ns );
	  if ( kids )
	    break;
	  else if (i < n-1 && strncmp(d_url.get(i),"urn:",4))
	    theSystem->warn("Couldn't read url '%s': %s\n",
			    d_url.get(i), strerror( errno));
	}

      if ( kids )
	{
	  delete d_namespace;
	  d_namespace = ns;
	  d_relative.set(url.url()); // children will be relative to this url

	  removeChildren();
	  addChildren( *kids ); 	// check for nested Inlines
	  delete kids;
	}
      else
	{
	  theSystem->warn("couldn't load Inline %s (relative %s)\n",
			  d_url[0],
			  d_relative.get() ? d_relative.get() : "<null>");
	  delete ns;
	}
    }
}
