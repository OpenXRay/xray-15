//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  %W% %G%
//  VrmlNodeInline.h

#ifndef  _VRMLNODEINLINE_
#define  _VRMLNODEINLINE_

#include "VrmlNodeGroup.h"
#include "VrmlMFString.h"

class VrmlNamespace;
class VrmlNodeType;
class VrmlScene;

class VrmlNodeInline : public VrmlNodeGroup {

public:

  // Define the built in VrmlNodeType:: "Inline"
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType *nodeType() const;

  VrmlNodeInline(VrmlScene*);
  virtual ~VrmlNodeInline();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodeInline* toInline() const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual void addToScene( VrmlScene *s, const char* relativeUrl );

  virtual void setField(const char *fieldName,
			const VrmlField &fieldValue);

  void load(const char *relativeUrl);

protected:

  VrmlMFString d_url;

  VrmlNamespace *d_namespace;

  bool d_hasLoaded;

};

#endif // _VRMLNODEINLINE_

