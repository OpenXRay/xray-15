//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeText.h

#ifndef  _VRMLNODETEXT_
#define  _VRMLNODETEXT_

#include "VrmlNodeGeometry.h"
#include "VrmlMFString.h"
#include "VrmlMFFloat.h"
#include "VrmlSFFloat.h"
#include "VrmlSFNode.h"

class VrmlScene;

class VrmlNodeText : public VrmlNodeGeometry {

public:

  // Define the fields of text nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeText(VrmlScene *);
  virtual ~VrmlNodeText();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace*);

  virtual bool isModified() const;

  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

protected:

  VrmlMFString d_string;
  VrmlSFNode d_fontStyle;
  VrmlMFFloat d_length;
  VrmlSFFloat d_maxExtent;

};

#endif // _VRMLNODETEXT_
