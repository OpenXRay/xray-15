//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeAnchor.h

#ifndef  _VRMLNODEANCHOR_
#define  _VRMLNODEANCHOR_

#include "VrmlNodeGroup.h"
#include "VrmlMFString.h"
#include "VrmlSFString.h"

class VrmlNodeType;
class VrmlScene;

class VrmlNodeAnchor : public VrmlNodeGroup {

public:

  // Define the built in VrmlNodeType:: "Anchor"
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeAnchor(VrmlScene*);
  VrmlNodeAnchor(const VrmlNodeAnchor&);
  virtual ~VrmlNodeAnchor();

  // Copy the node.
  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeAnchor *toAnchor() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void render(Viewer *);

  void activate();

  virtual void setField(const char *fieldName,
			const VrmlField &fieldValue);

  const char *description() { return d_description.get(); }
  const char *url() { return d_url.size() > 0 ? d_url[0] : 0; }

protected:

  VrmlSFString d_description;
  VrmlMFString d_parameter;
  VrmlMFString d_url;
  
};

#endif // _VRMLNODEANCHOR_

